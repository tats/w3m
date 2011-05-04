/* $Id: ftp.c,v 1.42 2010/12/15 10:50:24 htrb Exp $ */
#include <stdio.h>
#ifndef __MINGW32_VERSION
#include <pwd.h>
#endif /* __MINGW32_VERSION */
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

#ifndef __MINGW32_VERSION
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif /* __MINGW32_VERSION */

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
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

static JMP_BUF AbortLoading;

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
    SIGNAL_RETURN;
}

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
    int sock, status;

    sock = openSocket(ftp->host, "ftp", 21);
    if (sock < 0)
	goto open_err;
    if (ftppass_hostnamegen && !strcmp(ftp->user, "anonymous")) {
	size_t n = strlen(ftp->pass);

	if (n > 0 && ftp->pass[n - 1] == '@') {
#ifdef INET6
	    struct sockaddr_storage sockname;
#else
	    struct sockaddr_in sockname;
#endif
	    socklen_t socknamelen = sizeof(sockname);

	    if (!getsockname(sock, (struct sockaddr *)&sockname, &socknamelen)) {
		struct hostent *sockent;
		Str tmp = Strnew_charp(ftp->pass);
#ifdef INET6
		char hostbuf[NI_MAXHOST];

		if (getnameinfo((struct sockaddr *)&sockname, socknamelen,
				hostbuf, sizeof hostbuf, NULL, 0, NI_NAMEREQD)
			== 0)
		    Strcat_charp(tmp, hostbuf);
		else if (getnameinfo((struct sockaddr *)&sockname, socknamelen,
				        hostbuf, sizeof hostbuf, NULL, 0, NI_NUMERICHOST)
			== 0)
		    Strcat_m_charp(tmp, "[", hostbuf, "]", NULL);
		else
		    Strcat_charp(tmp, "unknown");
#else

		if ((sockent = gethostbyaddr((char *)&sockname.sin_addr,
					     sizeof(sockname.sin_addr),
					     sockname.sin_family)))
		    Strcat_charp(tmp, sockent->h_name);
		else
		    Strcat_m_charp(tmp, "[", inet_ntoa(sockname.sin_addr),
				   "]", NULL);
#endif
		ftp->pass = tmp->ptr;
	    }
	}
    }
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
    int port;
    socklen_t sockaddrlen;
    unsigned char d1, d2, d3, d4;
    char abuf[INET6_ADDRSTRLEN];
#endif

#ifdef INET6
    sockaddrlen = sizeof(sockaddr);
    if (getpeername(fileno(ftp->wf),
		    (struct sockaddr *)&sockaddr, &sockaddrlen) < 0)
	return -1;
#ifdef HAVE_OLD_SS_FAMILY
    family = sockaddr.__ss_family;
#else
    family = sockaddr.ss_family;
#endif
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

static int ex_ftpdir_name_size_date(char *, char **, char **, char **,
				    char **);

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
    Str uname = NULL;
    Str pwd = NULL;
    int add_auth_cookie_flag = FALSE;
    char *realpathname = NULL;

    if (!pu->host)
	return NULL;

    if (pu->user == NULL && pu->pass == NULL) {
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
	pwd = NULL;
	find_auth_user_passwd(pu, NULL, &uname, &pwd, 0);
	if (pwd == NULL) {
	    if (fmInitialized) {
		term_raw();
		pwd = Strnew_charp(inputLine("Password: ", NULL, IN_PASSWORD));
		pwd = Str_conv_to_system(pwd);
		term_cbreak();
	    }
	    else {
#ifndef __MINGW32_VERSION
		pwd = Strnew_charp((char *)getpass("Password: "));
#else
		term_raw();
		pwd = Strnew_charp(inputLine("Password: ", NULL, IN_PASSWORD));
		pwd = Str_conv_to_system(pwd);
		term_cbreak();
#endif /* __MINGW32_VERSION */
	    }
	    add_auth_cookie_flag = TRUE;
	}
	pass = pwd->ptr;
    }
    else if (ftppasswd != NULL && *ftppasswd != '\0')
	pass = ftppasswd;
    else {
#ifndef __MINGW32_VERSION
	struct passwd *mypw = getpwuid(getuid());
	tmp = Strnew_charp(mypw ? mypw->pw_name : "anonymous");
#else
	tmp = Strnew_charp("anonymous");
#endif /* __MINGW32_VERSION */
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
	add_auth_user_passwd(pu, NULL, uname, pwd, 0);

  ftp_read:
    ftp_command(&current_ftp, "TYPE", "I", &status);
    if (ftp_pasv(&current_ftp) < 0) {
	ftp_quit(&current_ftp);
	return NULL;
    }
    if (pu->file == NULL || *pu->file == '\0' ||
	pu->file[strlen(pu->file) - 1] == '/')
	goto ftp_dir;

    realpathname = file_unquote(pu->file);
    if (*realpathname == '/' && *(realpathname + 1) == '~')
	realpathname++;
    /* Get file */
    uf->modtime = ftp_modtime(&current_ftp, realpathname);
    ftp_command(&current_ftp, "RETR", realpathname, &status);
    if (status == 125 || status == 150)
	return newFileStream(current_ftp.data, (void (*)())closeFTPdata);

  ftp_dir:
    pu->scheme = SCM_FTPDIR;
    return NULL;
}

#ifdef USE_M17N
Str
loadFTPDir(ParsedURL *pu, wc_ces * charset)
#else
Str
loadFTPDir0(ParsedURL *pu)
#endif
{
    Str FTPDIRtmp;
    Str tmp;
    int status;
    volatile int sv_type;
    char *realpathname, *fn, *q;
    char **flist;
    int i, nfile, nfile_max;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
#ifdef USE_M17N
    wc_ces doc_charset = DocumentCharset;

    *charset = WC_CES_US_ASCII;
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
	if (*realpathname == '/' && *(realpathname + 1) == '~')
	    realpathname++;
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
	convertLine(NULL, Strnew_charp(file_unquote(tmp->ptr)), RAW_MODE,
		    charset, doc_charset);
    q = html_quote(tmp->ptr);
    FTPDIRtmp = Strnew_m_charp("<html>\n<head>\n<base href=\"", fn,
			       "\">\n<title>", q,
			       "</title>\n</head>\n<body>\n<h1>Index of ", q,
			       "</h1>\n", NULL);

    if (SETJMP(AbortLoading) != 0) {
	if (sv_type == UNIXLIKE_SERVER)
	    Strcat_charp(FTPDIRtmp, "</a></pre>\n");
	else
	    Strcat_charp(FTPDIRtmp, "</a></ul>\n");
	Strcat_charp(FTPDIRtmp, "<p>Transfer Interrupted!\n");
	goto ftp_end;
    }
    TRAP_ON;

    if (sv_type == UNIXLIKE_SERVER)
	Strcat_charp(FTPDIRtmp, "<pre>\n");
    else
	Strcat_charp(FTPDIRtmp, "<ul>\n<li>");
    Strcat_charp(FTPDIRtmp, "<a href=\"..\">[Upper Directory]</a>\n");

    nfile_max = 100;
    flist = New_N(char *, nfile_max);
    nfile = 0;
    if (sv_type == UNIXLIKE_SERVER) {
	char *name, *link, *date, *size, *type_str;
	int ftype, max_len, len, j;

	max_len = 20;
	while (tmp = Strfgets(current_ftp.data), tmp->length > 0) {
	    Strchop(tmp);
	    if ((ftype =
		 ex_ftpdir_name_size_date(tmp->ptr, &name, &link, &date,
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
		type_str = " ";
	    }
	    if (max_len < len)
		max_len = len;
	    flist[nfile++] = Sprintf("%s%s\n%s  %5s%s", name, type_str, date,
				     size, link)->ptr;
	    if (nfile == nfile_max) {
		nfile_max *= 2;
		flist = New_Reuse(char *, flist, nfile_max);
	    }
	}
	qsort(flist, nfile, sizeof(char *), strCmp);
	for (j = 0; j < nfile; j++) {
	    fn = flist[j];
	    date = strchr(fn, '\n');
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
		*(date - 1) = '\0';
	    }
	    date++;
	    tmp = convertLine(NULL, Strnew_charp(fn), RAW_MODE, charset,
			      doc_charset);
	    if (ftype == FTPDIR_LINK)
		Strcat_char(tmp, '@');
	    Strcat_m_charp(FTPDIRtmp, "<a href=\"", html_quote(file_quote(fn)),
			   "\">", html_quote(tmp->ptr), "</a>", NULL);
	    for (i = get_Str_strwidth(tmp); i <= max_len; i++) {
		if ((max_len % 2 + i) % 2)
		    Strcat_char(FTPDIRtmp, '.');
		else
		    Strcat_char(FTPDIRtmp, ' ');
	    }
	    tmp = convertLine(NULL, Strnew_charp(date), RAW_MODE, charset,
			      doc_charset);
	    Strcat_m_charp(FTPDIRtmp, html_quote(tmp->ptr), "\n", NULL);
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
	    tmp = convertLine(NULL, Strnew_charp(fn), RAW_MODE, charset,
			      doc_charset);
	    Strcat_m_charp(FTPDIRtmp, "<li><a href=\"",
			   html_quote(file_quote(fn)), "\">",
			   html_quote(tmp->ptr), "</a>\n", NULL);
	}
	Strcat_charp(FTPDIRtmp, "</ul>\n");
    }

  ftp_end:
    Strcat_charp(FTPDIRtmp, "</body>\n</html>\n");
    TRAP_OFF;
    closeFTPdata(current_ftp.data);
    return FTPDIRtmp;
}

void
disconnectFTP(void)
{
    ftp_quit(&current_ftp);
}

#define EX_SKIP_SPACE(cp) {\
    while (IS_SPACE(*cp) && *cp != '\0') cp++;\
    if (*cp == '\0')\
	goto done;\
}
#define EX_SKIP_NONE_SPACE(cp) {\
    while (!IS_SPACE(*cp) && *cp != '\0') cp++;\
    if (*cp == '\0')\
	goto done;\
}
#define EX_COUNT_DIGIT(cp) {\
    size = 0;\
    while (*cp && IS_DIGIT(*cp))\
	size = size * 10 + *(cp++) - '0';\
    if (*cp == '\0')\
	goto done;\
}

static Str size_int2str(clen_t);

static int
ex_ftpdir_name_size_date(char *line, char **name, char **link, char **date,
			 char **sizep)
{
    int ftype = FTPDIR_NONE;
    char *cp = line, *p;
    clen_t size;

    if (strlen(cp) < 11)
	goto done;
    /* skip permission */
    cp += 10;
    if (!IS_SPACE(*cp))
	goto done;
    cp++;

    /* skip link count */
    EX_SKIP_SPACE(cp);
    EX_COUNT_DIGIT(cp);
    cp++;

    /* skip owner string */
    EX_SKIP_SPACE(cp);
    EX_SKIP_NONE_SPACE(cp);
    cp++;

    /* skip group string */
    EX_SKIP_SPACE(cp);
    EX_SKIP_NONE_SPACE(cp);
    cp++;

    /* extract size */
    EX_SKIP_SPACE(cp);
    p = cp;
    EX_COUNT_DIGIT(cp);
    if (*cp == ',') {		/* device file ? */
	cp++;
	EX_SKIP_SPACE(cp);
	EX_SKIP_NONE_SPACE(cp);
	*sizep = allocStr(p, cp - p);
    }
    else {
	*sizep = size_int2str(size)->ptr;
    }
    cp++;

    /* extract date */
    /* loose check for i18n server */
    p = cp;
    EX_SKIP_SPACE(cp);
    EX_SKIP_NONE_SPACE(cp);	/* month ? */
    EX_SKIP_SPACE(cp);
    EX_SKIP_NONE_SPACE(cp);	/* day ? */
    EX_SKIP_SPACE(cp);
    EX_SKIP_NONE_SPACE(cp);	/* year or time ? */
    *date = allocStr(p, cp - p);
    cp++;

    /* extract file name */
    EX_SKIP_SPACE(cp);
    switch (line[0]) {
    case 'l':
	ftype = FTPDIR_LINK;
	if ((p = strstr(cp, " -> ")) == NULL)
	    goto done;
	*name = allocStr(cp, p - cp);
	*link = allocStr(p, -1);
	*sizep = "";
	break;
    case 'd':
	ftype = FTPDIR_DIR;
	*name = allocStr(cp, -1);
	*link = "";
	*sizep = "";
	break;
    default:
	ftype = FTPDIR_FILE;
	*name = allocStr(cp, -1);
	*link = "";
	break;
    }

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
