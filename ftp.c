/* $Id: ftp.c,v 1.26 2003/01/15 16:24:25 ukai Exp $ */
#include <stdio.h>
#include <pwd.h>
#include <Str.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>

#include "fm.h"
#include "html.h"
#include "myctype.h"

#ifdef DEBUG
#include <malloc.h>
#endif				/* DEBUG */

#include <sys/socket.h>
#if defined(FTPPASS_HOSTNAMEGEN) || defined(INET6)
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

typedef struct _FTP {
    char *host;
    int port;
    char *user;
    char *pass;
    InputStream rf;
    FILE *wf;
    FILE *data;
} *FTP;

static struct _FTP current_ftp = {
    NULL, 0, NULL, NULL, NULL, NULL, NULL
};

static Str
ftp_command(FTP ftp, char *cmd, char *arg, int *status)
{
    Str tmp;

    if (!ftp->host)
	return NULL;
    if (cmd) {
	if (arg)
	    tmp = Sprintf("%s %s\r\n", cmd, arg);
	else
	    tmp = Sprintf("%s\r\n", cmd);
	fwrite(tmp->ptr, sizeof(char), tmp->length, ftp->wf);
	fflush(ftp->wf);
    }
    if (!status)
	return NULL;
    *status = -1;		/* error */
    tmp = StrISgets(ftp->rf);
    if (IS_DIGIT(tmp->ptr[0]) && IS_DIGIT(tmp->ptr[1]) &&
	IS_DIGIT(tmp->ptr[2]) && tmp->ptr[3] == ' ')
	sscanf(tmp->ptr, "%d", status);

    if (tmp->ptr[3] != '-')
	return tmp;
    /* RFC959 4.2 FTP REPLIES */
    /* multi-line response start */
    /* 
     * Thus the format for multi-line replies is that the
     * first line will begin with the exact required reply
     * code, followed immediately by a Hyphen, "-" (also known 
     * as Minus), followed by text.  The last line will begin
     * with the same code, followed immediately by Space <SP>, 
     * optionally some text, and the Telnet end-of-line code. */
    while (1) {
	tmp = StrISgets(ftp->rf);
	if (IS_DIGIT(tmp->ptr[0]) && IS_DIGIT(tmp->ptr[1]) &&
	    IS_DIGIT(tmp->ptr[2]) && tmp->ptr[3] == ' ') {
	    sscanf(tmp->ptr, "%d", status);
	    break;
	}
    }
    return tmp;
}

static void
ftp_close(FTP ftp)
{
    if (!ftp->host)
	return;
    if (ftp->rf) {
	IStype(ftp->rf) &= ~IST_UNCLOSE;
	ISclose(ftp->rf);
	ftp->rf = NULL;
    }
    if (ftp->wf) {
	fclose(ftp->wf);
	ftp->wf = NULL;
    }
    if (ftp->data) {
	fclose(ftp->data);
	ftp->data = NULL;
    }
    ftp->host = NULL;
    return;
}

static int
ftp_login(FTP ftp)
{
    Str tmp;
    int sock, status;

    sock = openSocket(ftp->host, "ftp", 21);
    if (sock < 0)
	goto open_err;
#ifdef FTPPASS_HOSTNAMEGEN
    if (ftppass_hostnamegen && !strcmp(ftp->user, "anonymous")) {
	size_t n = strlen(ftp->pass);

	if (n > 0 && ftp->pass[n - 1] == '@') {
	    struct sockaddr_in sockname;
	    int socknamelen = sizeof(sockname);

	    if (!getsockname(sock, (struct sockaddr *)&sockname, &socknamelen)) {
		struct hostent *sockent;
		tmp = Strnew_charp(ftp->pass);

		if ((sockent = gethostbyaddr((char *)&sockname.sin_addr,
					     sizeof(sockname.sin_addr),
					     sockname.sin_family)))
		    Strcat_charp(tmp, sockent->h_name);
		else
		    Strcat_m_charp(tmp, "[", inet_ntoa(sockname.sin_addr),
				   "]", NULL);

		ftp->pass = tmp->ptr;
	    }
	}
    }
#endif
    ftp->rf = newInputStream(sock);
    ftp->wf = fdopen(dup(sock), "wb");
    if (!ftp->rf || !ftp->wf)
	goto open_err;
    IStype(ftp->rf) |= IST_UNCLOSE;
    ftp_command(ftp, NULL, NULL, &status);
    if (status != 220)
	goto open_err;
    if (fmInitialized) {
	message(Sprintf("Sending FTP username (%s) to remote server.",
			ftp->user)->ptr, 0, 0);
	refresh();
    }
    ftp_command(ftp, "USER", ftp->user, &status);
    /*
     * Some ftp daemons(e.g. publicfile) return code 230 for user command.
     */
    if (status == 230)
	goto succeed;
    if (status != 331)
	goto open_err;
    if (fmInitialized) {
	message("Sending FTP password to remote server.", 0, 0);
	refresh();
    }
    ftp_command(ftp, "PASS", ftp->pass, &status);
    if (status != 230)
	goto open_err;
  succeed:
    return TRUE;
  open_err:
    ftp_close(ftp);
    return FALSE;
}

static int
ftp_pasv(FTP ftp)
{
    int status;
    int n1, n2, n3, n4, p1, p2;
    int data;
    char *p;
    Str tmp;
    int family;
#ifdef INET6
    struct sockaddr_storage sockaddr;
    int sockaddrlen, port;
    unsigned char d1, d2, d3, d4;
    char abuf[INET6_ADDRSTRLEN];
#endif

#ifdef INET6
    sockaddrlen = sizeof(sockaddr);
    if (getpeername(fileno(ftp->wf),
		    (struct sockaddr *)&sockaddr, &sockaddrlen) < 0)
	return -1;
    family = sockaddr.ss_family;
#else
    family = AF_INET;
#endif
    switch (family) {
#ifdef INET6
    case AF_INET6:
	tmp = ftp_command(ftp, "EPSV", NULL, &status);
	if (status != 229)
	    return -1;
	for (p = tmp->ptr + 4; *p && *p != '('; p++) ;
	if (*p == '\0')
	    return -1;
	if (sscanf(++p, "%c%c%c%d%c", &d1, &d2, &d3, &port, &d4) != 5
	    || d1 != d2 || d1 != d3 || d1 != d4)
	    return -1;
	if (getnameinfo((struct sockaddr *)&sockaddr, sockaddrlen,
			abuf, sizeof(abuf), NULL, 0, NI_NUMERICHOST) != 0)
	    return -1;
	data = openSocket(abuf, "", port);
	break;
#endif
    case AF_INET:
	tmp = ftp_command(ftp, "PASV", NULL, &status);
	if (status != 227)
	    return -1;
	for (p = tmp->ptr + 4; *p && !IS_DIGIT(*p); p++) ;
	if (*p == '\0')
	    return -1;
	sscanf(p, "%d,%d,%d,%d,%d,%d", &n1, &n2, &n3, &n4, &p1, &p2);
	tmp = Sprintf("%d.%d.%d.%d", n1, n2, n3, n4);
	data = openSocket(tmp->ptr, "", p1 * 256 + p2);
	break;
    default:
	return -1;
    }
    if (data < 0)
	return -1;
    ftp->data = fdopen(data, "rb");
    return 0;
}

static time_t
ftp_modtime(FTP ftp, char *path)
{
    int status;
    Str tmp;
    char *p;
    struct tm tm;
    time_t t, lt, gt;

    tmp = ftp_command(ftp, "MDTM", path, &status);
    if (status != 213)
	return -1;
    for (p = tmp->ptr + 4; *p && *p == ' '; p++) ;
    memset(&tm, 0, sizeof(struct tm));
    if (sscanf(p, "%04d%02d%02d%02d%02d%02d",
	       &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
	       &tm.tm_hour, &tm.tm_min, &tm.tm_sec) < 6)
	return -1;
    tm.tm_year -= 1900;
    tm.tm_mon--;
    t = mktime(&tm);
    lt = mktime(localtime(&t));
    gt = mktime(gmtime(&t));
    return t + (lt - gt);
}

static int
ftp_quit(FTP ftp)
{
    /*
     * int status;
     * ftp_command(ftp, "QUIT", NULL, &status);
     * ftp_close(ftp);
     * if (status != 221)
     * return -1;
     */
    ftp_command(ftp, "QUIT", NULL, NULL);
    ftp_close(ftp);
    return 0;
}

static int ex_ftpdir_name_size_date(char *, char **, char **, char **);

#define	SERVER_NONE	0
#define	UNIXLIKE_SERVER	1

#define	FTPDIR_NONE	0
#define	FTPDIR_DIR	1
#define	FTPDIR_LINK	2
#define	FTPDIR_FILE	3

static void
closeFTPdata(FILE * f)
{
    int status;
    if (f) {
	fclose(f);
	if (f == current_ftp.data)
	    current_ftp.data = NULL;
    }
    ftp_command(&current_ftp, NULL, NULL, &status);
    /* status == 226 */
}

void
closeFTP(void)
{
    ftp_close(&current_ftp);
}

InputStream
openFTPStream(ParsedURL *pu, URLFile *uf)
{
    Str tmp;
    int status;
    char *user = NULL;
    char *pass = NULL;
    Str pwd = NULL;
    int add_auth_cookie_flag = FALSE;
    char *realpathname = NULL;

    if (!pu->host)
	return NULL;

    if (pu->user == NULL && pu->pass == NULL) {
	Str uname, pwd;
	if (find_auth_user_passwd(pu, NULL, &uname, &pwd, 0)) {
	    if (uname)
		user = uname->ptr;
	    if (pwd)
		pass = pwd->ptr;
	}
    }
    if (user)
	/* do nothing */ ;
    else if (pu->user)
	user = pu->user;
    else
	user = "anonymous";

    if (current_ftp.host) {
	if (!strcmp(current_ftp.host, pu->host) &&
	    current_ftp.port == pu->port && !strcmp(current_ftp.user, user)) {
	    ftp_command(&current_ftp, "NOOP", NULL, &status);
	    if (status != 200)
		ftp_close(&current_ftp);
	    else
		goto ftp_read;
	}
	else
	    ftp_quit(&current_ftp);
    }

    if (pass)
	/* do nothing */ ;
    else if (pu->pass)
	pass = pu->pass;
    else if (pu->user) {
	pwd = find_auth_cookie(pu->host, pu->port, pu->file, pu->user);
	if (pwd == NULL) {
	    if (fmInitialized) {
		term_raw();
		pwd = Strnew_charp(inputLine("Password: ", NULL, IN_PASSWORD));
		pwd = Str_conv_to_system(pwd);
		term_cbreak();
	    }
	    else {
		pwd = Strnew_charp((char *)getpass("Password: "));
	    }
	    add_auth_cookie_flag = TRUE;
	}
	pass = pwd->ptr;
    }
    else if (ftppasswd != NULL && *ftppasswd != '\0')
	pass = ftppasswd;
    else {
	struct passwd *mypw = getpwuid(getuid());
	tmp = Strnew_charp(mypw ? mypw->pw_name : "anonymous");
	Strcat_char(tmp, '@');
	pass = tmp->ptr;
    }

    if (!current_ftp.host) {
	current_ftp.host = allocStr(pu->host, -1);
	current_ftp.port = pu->port;
	current_ftp.user = allocStr(user, -1);
	current_ftp.pass = allocStr(pass, -1);
	if (!ftp_login(&current_ftp))
	    return NULL;
    }
    if (add_auth_cookie_flag)
	add_auth_cookie(pu->host, pu->port, pu->file, pu->user, pwd);

  ftp_read:
    ftp_command(&current_ftp, "TYPE", "I", &status);
    if (ftp_pasv(&current_ftp) < 0) {
	ftp_quit(&current_ftp);
	return NULL;
    }
    if (pu->file == NULL || *pu->file == '\0')
	goto ftp_dir;
    else
	realpathname = file_unquote(pu->file);
    if (pu->file[strlen(pu->file) - 1] == '/')
	goto ftp_dir;
    /* Get file */
    uf->modtime = ftp_modtime(&current_ftp, realpathname);
    ftp_command(&current_ftp, "RETR", realpathname, &status);
    if (status == 125 || status == 150)
	return newFileStream(current_ftp.data, (void (*)())closeFTPdata);

  ftp_dir:
    pu->scheme = SCM_FTPDIR;
    return NULL;
}

Str
loadFTPDir(ParsedURL *pu, char *code)
{
    Str FTPDIRtmp;
    Str tmp;
    int status, sv_type;
    char *realpathname, *fn, *q;
    char **flist;
    int i, nfile, nfile_max = 100;

#ifdef JP_CHARSET
    *code = DocumentCode;
#endif
    if (current_ftp.data == NULL)
	return NULL;
    tmp = ftp_command(&current_ftp, "SYST", NULL, &status);
    if (strstr(tmp->ptr, "UNIX") != NULL || !strncmp(tmp->ptr + 4, "Windows_NT", 10))	/* :-) */
	sv_type = UNIXLIKE_SERVER;
    else
	sv_type = SERVER_NONE;
    if (pu->file == NULL || *pu->file == '\0') {
	if (sv_type == UNIXLIKE_SERVER)
	    ftp_command(&current_ftp, "LIST", NULL, &status);
	else
	    ftp_command(&current_ftp, "NLST", NULL, &status);
	pu->file = "/";
    }
    else {
	realpathname = file_unquote(pu->file);
	if (sv_type == UNIXLIKE_SERVER) {
	    ftp_command(&current_ftp, "CWD", realpathname, &status);
	    if (status == 250)
		ftp_command(&current_ftp, "LIST", NULL, &status);
	}
	else
	    ftp_command(&current_ftp, "NLST", realpathname, &status);
    }
    if (status != 125 && status != 150) {
	fclose(current_ftp.data);
	current_ftp.data = NULL;
	return NULL;
    }
    tmp = parsedURL2Str(pu);
    if (Strlastchar(tmp) != '/')
	Strcat_char(tmp, '/');
    fn = html_quote(tmp->ptr);
    tmp =
	convertLine(NULL, Strnew_charp(file_unquote(tmp->ptr)), code,
		    RAW_MODE);
    q = html_quote(tmp->ptr);
    FTPDIRtmp = Strnew_m_charp("<html>\n<head>\n<base href=\"", fn,
			       "\">\n<title>", q,
			       "</title>\n</head>\n<body>\n<h1>Index of ", q,
			       "</h1>\n", NULL);
    if (sv_type == UNIXLIKE_SERVER)
	Strcat_charp(FTPDIRtmp, "<pre>\n");
    else
	Strcat_charp(FTPDIRtmp, "<ul>\n<li>");
    Strcat_charp(FTPDIRtmp, "<a href=\"..\">[Upper Directory]</a>\n");

    flist = New_N(char *, nfile_max);
    nfile = 0;
    if (sv_type == UNIXLIKE_SERVER) {
	char *name, *date, *size, *type_str;
	int ftype, max_len, len, j;
	Str line_tmp;

	max_len = 0;
	while (tmp = Strfgets(current_ftp.data), tmp->length > 0) {
	    Strchop(tmp);
	    if ((ftype = ex_ftpdir_name_size_date(tmp->ptr, &name, &date,
						  &size)) == FTPDIR_NONE)
		continue;
	    if (!strcmp(".", name) || !strcmp("..", name))
		continue;
	    len = strlen(name);
	    if (!len)
		continue;
	    if (ftype == FTPDIR_DIR) {
		len++;
		type_str = "/";
	    }
	    else if (ftype == FTPDIR_LINK) {
		len++;
		type_str = "@";
	    }
	    else {
		type_str = "";
	    }
	    if (max_len < len)
		max_len = len;
	    line_tmp =
		Sprintf("%s%s %-12.12s %6.6s", name, type_str, date, size);
	    flist[nfile++] = line_tmp->ptr;
	    if (nfile == nfile_max) {
		nfile_max *= 2;
		flist = New_Reuse(char *, flist, nfile_max);
	    }
	}
	qsort(flist, nfile, sizeof(char *), strCmp);
	for (j = 0; j < nfile; j++) {
	    fn = flist[j];
	    date = fn + strlen(fn) - 20;
	    if (*(date - 1) == '/') {
		ftype = FTPDIR_DIR;
		*date = '\0';
	    }
	    else if (*(date - 1) == '@') {
		ftype = FTPDIR_LINK;
		*(date - 1) = '\0';
	    }
	    else {
		ftype = FTPDIR_FILE;
		*date = '\0';
	    }
	    date++;
	    len = strlen(fn);
	    tmp = convertLine(NULL, Strnew_charp(fn), code, RAW_MODE);
	    Strcat_m_charp(FTPDIRtmp, "<a href=\"", html_quote(file_quote(fn)),
			   "\">", html_quote(tmp->ptr), NULL);
	    if (ftype == FTPDIR_LINK) {
		Strcat_charp(FTPDIRtmp, "@");
		len++;
	    }
	    Strcat_charp(FTPDIRtmp, "</a>");
	    for (i = len; i <= max_len; i++) {
		if ((max_len % 2 + i) % 2) {
		    Strcat_charp(FTPDIRtmp, ".");
		}
		else {
		    Strcat_charp(FTPDIRtmp, " ");
		}
	    }
	    Strcat_m_charp(FTPDIRtmp, date, "\n", NULL);
	}
	Strcat_charp(FTPDIRtmp, "</pre>\n");
    }
    else {
	while (tmp = Strfgets(current_ftp.data), tmp->length > 0) {
	    Strchop(tmp);
	    flist[nfile++] = mybasename(tmp->ptr);
	    if (nfile == nfile_max) {
		nfile_max *= 2;
		flist = New_Reuse(char *, flist, nfile_max);
	    }
	}
	qsort(flist, nfile, sizeof(char *), strCmp);
	for (i = 0; i < nfile; i++) {
	    fn = flist[i];
	    tmp = convertLine(NULL, Strnew_charp(fn), code, RAW_MODE);
	    Strcat_m_charp(FTPDIRtmp, "<li><a href=\"",
			   html_quote(file_quote(fn)), "\">",
			   html_quote(tmp->ptr), "</a>\n", NULL);
	}
	Strcat_charp(FTPDIRtmp, "</ul>\n");
    }
    Strcat_charp(FTPDIRtmp, "</body>\n</html>\n");

    closeFTPdata(current_ftp.data);
    return FTPDIRtmp;
}

void
disconnectFTP(void)
{
    ftp_quit(&current_ftp);
}

#define XD_CTOD(c) {\
  if (c >= '0' && c <= '9') {\
    c -= (unsigned char)'0';\
  } else if (c >= 'a' && c <= 'f') {\
    c = c - (unsigned char)'a' + (unsigned char)10;\
  } else if (c >= 'A' && c <= 'F') {\
    c = c - (unsigned char)'A' + (unsigned char)10;\
  } else {\
    goto skip;\
  }\
}

#define EX_SKIP_SPACE(cp) {\
  while (IS_SPACE(*cp) && *cp != '\0') cp++;\
  if (*cp == '\0') {\
    goto done;\
  }\
}
#define EX_SKIP_NONE_SPACE(cp) {\
  while (!IS_SPACE(*cp) && *cp != '\0') cp++;\
  if (*cp == '\0') {\
    goto done;\
  }\
}

static Str size_int2str(clen_t);

static int
ex_ftpdir_name_size_date(char *line, char **name, char **date, char **sizep)
{
    int ftype = FTPDIR_NONE;
    char *cp, *endp;
    Str date_str, name_str, size_str;
    clen_t size;

    if (strlen(line) < 11) {
	goto done;
    }
    /* skip permission */
    if (!IS_SPACE(line[10])) {
	goto done;
    }
    cp = line + 11;

    /* skip link count */
    EX_SKIP_SPACE(cp)
	while (IS_DIGIT(*cp) && *cp != '\0')
	cp++;
    if (!IS_SPACE(*cp) || *cp == '\0') {
	goto done;
    }
    cp++;

    /* skip owner string */
    EX_SKIP_SPACE(cp)
	EX_SKIP_NONE_SPACE(cp)
	cp++;

    /* skip group string */
    EX_SKIP_SPACE(cp)
	EX_SKIP_NONE_SPACE(cp)
	cp++;

    /* extract size */
    EX_SKIP_SPACE(cp)
	size = 0;
    while (*cp && IS_DIGIT(*cp)) {
	size = size * 10 + *(cp++) - '0';
    }
    if (*cp == '\0') {
	goto done;
    }

    /* extract date */
    EX_SKIP_SPACE(cp)
	if (IS_ALPHA(cp[0]) && IS_ALPHA(cp[1]) && IS_ALPHA(cp[2])
	    && IS_SPACE(cp[3])
	    && (IS_SPACE(cp[4]) || IS_DIGIT(cp[4])) && IS_DIGIT(cp[5])
	    && IS_SPACE(cp[6])
	    && (IS_SPACE(cp[7]) || IS_DIGIT(cp[7])) && IS_DIGIT(cp[8])
	    && (cp[9] == ':' || IS_DIGIT(cp[9]))
	    && IS_DIGIT(cp[10]) && (IS_DIGIT(cp[11]) || IS_SPACE(cp[11]))
	    && IS_SPACE(cp[12])) {
	cp[12] = '\0';
	date_str = Strnew_charp(cp);
	cp += 13;
    }
    else {
	goto done;
    }

    /* extract file name */
    EX_SKIP_SPACE(cp)
	if (line[0] == 'l') {
	if ((endp = strstr(cp, " -> ")) == NULL) {
	    goto done;
	}
	*endp = '\0';
	size_str = Strnew_charp("-");
	ftype = FTPDIR_LINK;
    }
    else if (line[0] == 'd') {
	size_str = Strnew_charp("-");
	ftype = FTPDIR_DIR;
    }
    else {
	size_str = size_int2str(size);
	ftype = FTPDIR_FILE;
    }
    name_str = Strnew_charp(cp);
    *date = date_str->ptr;
    *name = name_str->ptr;
    *sizep = size_str->ptr;

  done:
    return (ftype);
}

static Str
size_int2str(clen_t size)
{
    Str size_str;
    int unit;
    double dtmp;
    char *size_format, *unit_str;

    dtmp = (double)size;
    for (unit = 0; unit < 3; unit++) {
	if (dtmp < 1024) {
	    break;
	}
	dtmp /= 1024;
    }
    if (!unit || dtmp > 100) {
	size_format = "%.0f%s";
    }
    else if (dtmp > 10) {
	size_format = "%.1f%s";
    }
    else {
	size_format = "%.2f%s";
    }
    switch (unit) {
    case 3:
	unit_str = "G";
	break;
    case 2:
	unit_str = "M";
	break;
    case 1:
	unit_str = "K";
	break;
    default:
	unit_str = "";
	break;
    }
    size_str = Sprintf(size_format, dtmp, unit_str);

    return (size_str);
}
