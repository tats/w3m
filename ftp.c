#include <stdio.h>
#include <pwd.h>
#include <Str.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>

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
    FILE *rcontrol;
    FILE *wcontrol;
    FILE *data;
} *FTP;

#define FtpError(status) ((status)<0)
#define FTPDATA(ftp) ((ftp)->data)

typedef int STATUS;

static FTP ftp;

static Str
read_response1(FTP ftp)
{
    char c;
    Str buf = Strnew();
    while (1) {
	c = getc(ftp->rcontrol);
	if (c == '\r') {
	    c = getc(ftp->rcontrol);
	    if (c == '\n') {
		Strcat_charp(buf, "\r\n");
		break;
	    }
	    else {
		Strcat_char(buf, '\r');
		Strcat_char(buf, c);
	    }
	}
	else if (c == '\n') {
	    Strcat_charp(buf, "\r\n");
	    break;
	}
	else if (feof(ftp->rcontrol))
	    break;
	else
	    Strcat_char(buf, c);
    }
    return buf;
}

Str
read_response(FTP ftp)
{
    Str tmp;

    tmp = read_response1(ftp);
    if (feof(ftp->rcontrol)) {
	return tmp;
    }
    if (tmp->ptr[3] == '-') {
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
	    tmp = read_response1(ftp);
	    if (feof(ftp->rcontrol)) {
		break;
	    }
	    if (IS_DIGIT(tmp->ptr[0])
		&& IS_DIGIT(tmp->ptr[1])
		&& IS_DIGIT(tmp->ptr[2])
		&& tmp->ptr[3] == ' ') {
		break;
	    }
	}
    }
    return tmp;
}

int
FtpLogin(FTP * ftp_return, char *host, char *user, char *pass)
{
    Str tmp;
    FTP ftp = New(struct _FTP);
    int fd;
    *ftp_return = ftp;
    fd = openSocket(host, "ftp", 21);
    if (fd < 0)
	return -1;
#ifdef FTPPASS_HOSTNAMEGEN
    if (!strcmp(user, "anonymous")) {
	size_t n = strlen(pass);

	if (n > 0 && pass[n - 1] == '@') {
	    struct sockaddr_in sockname;
	    int socknamelen = sizeof(sockname);

	    if (!getsockname(fd, (struct sockaddr *) &sockname, &socknamelen)) {
		struct hostent *sockent;
		Str tmp2 = Strnew_charp(pass);

		if (sockent = gethostbyaddr((char *) &sockname.sin_addr,
					    sizeof(sockname.sin_addr),
					    sockname.sin_family))
		    Strcat_charp(tmp2, sockent->h_name);
		else
		    Strcat_m_charp(tmp2, "[", inet_ntoa(sockname.sin_addr), "]", NULL);

		pass = tmp2->ptr;
	    }
	}
    }
#endif
    ftp->rcontrol = fdopen(fd, "rb");
    ftp->wcontrol = fdopen(dup(fd), "wb");
    ftp->data = NULL;
    tmp = read_response(ftp);
    if (atoi(tmp->ptr) != 220)
	return -1;
    if (fmInitialized) {
	message(Sprintf("Sending FTP username (%s) to remote server.\n", user)->ptr, 0, 0);
	refresh();
    }
    tmp = Sprintf("USER %s\r\n", user);
    fwrite(tmp->ptr, tmp->length, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    /*
     * Some ftp daemons(e.g. publicfile) return code 230 for user command.
     */
    if (atoi(tmp->ptr) == 230)
	goto succeed;
    if (atoi(tmp->ptr) != 331)
	return -1;
    if (fmInitialized) {
	message(Sprintf("Sending FTP password to remote server.\n")->ptr, 0, 0);
	refresh();
    }
    tmp = Sprintf("PASS %s\r\n", pass);
    fwrite(tmp->ptr, tmp->length, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (atoi(tmp->ptr) != 230)
	return -1;
  succeed:
    return 0;
}

int
FtpBinary(FTP ftp)
{
    Str tmp;
    fwrite("TYPE I\r\n", 8, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (atoi(tmp->ptr) != 200)
	return -1;
    return 0;
}

int
ftp_pasv(FTP ftp)
{
    int n1, n2, n3, n4, p1, p2;
    int data_s;
    char *p;
    Str tmp;
    int family;
#ifdef INET6
    struct sockaddr_storage sin;
    int sinlen, port;
    unsigned char d1, d2, d3, d4;
    char abuf[INET6_ADDRSTRLEN];
#endif

#ifdef INET6
    sinlen = sizeof(sin);
    if (getpeername(fileno(ftp->wcontrol),
		    (struct sockaddr *)&sin, &sinlen) < 0)
	return -1;
    family = sin.ss_family;
#else
    family = AF_INET;
#endif
    switch (family) {
#ifdef INET6
    case AF_INET6:
	fwrite("EPSV\r\n", 6, sizeof(char), ftp->wcontrol);
	fflush(ftp->wcontrol);
	tmp = read_response(ftp);
	if (atoi(tmp->ptr) != 229)
	    return -1;
	for (p = tmp->ptr + 4; *p && *p != '('; p++);
	if (*p == '\0')
	    return -1;
	if (sscanf(++p, "%c%c%c%d%c", &d1, &d2, &d3, &port, &d4) != 5
	    || d1 != d2 || d1 != d3 || d1 != d4)
	    return -1;
	if (getnameinfo((struct sockaddr *)&sin, sinlen,
			abuf, sizeof(abuf),
			NULL, 0, NI_NUMERICHOST) != 0)
	    return -1;
	tmp = Sprintf("%s", abuf);
	data_s = openSocket(tmp->ptr, "", port);
	break;
#endif
    case AF_INET:
	fwrite("PASV\r\n", 6, sizeof(char), ftp->wcontrol);
	fflush(ftp->wcontrol);
	tmp = read_response(ftp);
	if (atoi(tmp->ptr) != 227)
	    return -1;
	for (p = tmp->ptr + 4; *p && !IS_DIGIT(*p); p++);
	if (*p == '\0')
	    return -1;
	sscanf(p, "%d,%d,%d,%d,%d,%d", &n1, &n2, &n3, &n4, &p1, &p2);
	tmp = Sprintf("%d.%d.%d.%d", n1, n2, n3, n4);
	data_s = openSocket(tmp->ptr, "", p1 * 256 + p2);
	break;
    default:
	return -1;
    }
    if (data_s < 0)
	return -1;
    ftp->data = fdopen(data_s, "rb");
    return 0;
}

int
FtpCwd(FTP ftp, char *path)
{
    Str tmp;

    tmp = Sprintf("CWD %s\r\n", path);
    fwrite(tmp->ptr, tmp->length, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (tmp->ptr[0] == '5') {
	return -1;
    }
    return 0;
}

int
FtpOpenRead(FTP ftp, char *path)
{
    Str tmp;

    if (ftp_pasv(ftp) < 0)
	return -1;
    tmp = Sprintf("RETR %s\r\n", path);
    fwrite(tmp->ptr, tmp->length, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (tmp->ptr[0] == '5') {
	fclose(ftp->data);
	ftp->data = NULL;
	return -1;
    }
    return 0;
}

int
Ftpfclose(FILE * f)
{
    fclose(f);
    if (f == ftp->data)
	ftp->data = NULL;
    read_response(ftp);
    return 0;
}

int
FtpData(FTP ftp, char *cmd, char *arg, char *mode)
{
    Str tmp;

    if (ftp_pasv(ftp) < 0)
	return -1;
    tmp = Sprintf(cmd, arg);
    Strcat_charp(tmp, "\r\n");
    fwrite(tmp->ptr, tmp->length, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (tmp->ptr[0] == '5') {
	fclose(ftp->data);
	ftp->data = NULL;
	return -1;
    }
    return 0;
}

int
FtpClose(FTP ftp)
{
    Str tmp;

    fclose(ftp->data);
    ftp->data = NULL;
    tmp = read_response(ftp);
    if (atoi(tmp->ptr) != 226)
	return -1;
    return 0;
}

int
FtpBye(FTP ftp)
{
    Str tmp;
    int ret_val, control_closed;

    fwrite("QUIT\r\n", 6, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (atoi(tmp->ptr) != 221)
	ret_val = -1;
    else
	ret_val = 0;
    control_closed = 0;
    if (ftp->rcontrol != NULL) {
	fclose(ftp->rcontrol);
	ftp->rcontrol = NULL;
	control_closed = 1;
    }
    if (ftp->wcontrol != NULL) {
	fclose(ftp->wcontrol);
	ftp->wcontrol = NULL;
	control_closed = 1;
    }
    if (control_closed && ftp->data != NULL) {
	fclose(ftp->data);
	ftp->data = NULL;
    }
    return ret_val;
}


Str FTPDIRtmp;

static int ex_ftpdir_name_size_date(char *, char **, char **, char **);
static int ftp_system(FTP);
static char *ftp_escape_str(char *);
static char *ftp_restore_str(char *);

#define	SERVER_NONE	0
#define	UNIXLIKE_SERVER	1

#define	FTPDIR_NONE	0
#define	FTPDIR_DIR	1
#define	FTPDIR_LINK	2
#define	FTPDIR_FILE	3

FILE *
openFTP(ParsedURL * pu)
{
    Str tmp2 = Strnew();
    Str tmp3 = Strnew();
    Str host;
    STATUS s;
    Str curdir;
    char *user;
    char *pass;
    char *fn;
    char *qdir;
    char **flist;
    int i, nfile, nfile_max = 100;
    Str pwd;
    int add_auth_cookie_flag;
    char *realpath = NULL;
#ifdef JP_CHARSET
    char code = '\0', ic;
    Str pathStr;
#endif
    int sv_type;

    add_auth_cookie_flag = 0;
    if (pu->user)
	user = pu->user;
    else {
	Strcat_charp(tmp3, "anonymous");
	user = tmp3->ptr;
    }
    if (pu->pass)
	pass = pu->pass;
    else if (pu->user) {
	pwd = find_auth_cookie(pu->host, pu->user);
	if (pwd == NULL) {
	    if (fmInitialized) {
		term_raw();
		pwd = Strnew_charp(inputLine("Password: ", NULL, IN_PASSWORD));
		term_cbreak();
	    }
	    else {
		pwd = Strnew_charp((char *) getpass("Password: "));
	    }
	    add_auth_cookie_flag = 1;
	}
	pass = pwd->ptr;
    }
    else if (ftppasswd != NULL && *ftppasswd != '\0')
	pass = ftppasswd;
    else {
	struct passwd *mypw = getpwuid(getuid());
	if (mypw == NULL)
	    Strcat_charp(tmp2, "anonymous");
	else
	    Strcat_charp(tmp2, mypw->pw_name);
	Strcat_char(tmp2, '@');
	pass = tmp2->ptr;
    }
    s = FtpLogin(&ftp, pu->host, user, pass);
    if (FtpError(s))
	return NULL;
    if (add_auth_cookie_flag)
	add_auth_cookie(pu->host, pu->user, pwd);
    if (pu->file == NULL || *pu->file == '\0')
	goto ftp_dir;
    else
	realpath = ftp_restore_str(pu->file);

    /* Get file */
    FtpBinary(ftp);
    s = FtpOpenRead(ftp, realpath);
    if (!FtpError(s)) {
#ifdef JP_CHARSET
	pathStr = Strnew_charp(realpath);
	if ((ic = checkShiftCode(pathStr, code)) != '\0') {
	    pathStr = conv_str(pathStr, (code = ic), InnerCode);
	    realpath = pathStr->ptr;
	}
#endif				/* JP_CHARSET */
	pu->file = realpath;
	return FTPDATA(ftp);
    }

    /* Get directory */
  ftp_dir:
    pu->scheme = SCM_FTPDIR;
    FTPDIRtmp = Strnew();
    sv_type = ftp_system(ftp);
    if (pu->file == NULL || *pu->file == '\0') {
	if (sv_type == UNIXLIKE_SERVER) {
	    s = FtpData(ftp, "LIST", NULL, "r");
	}
	else {
	    s = FtpData(ftp, "NLST", NULL, "r");
	}
	curdir = Strnew_charp("/");
    }
    else {
	if (sv_type == UNIXLIKE_SERVER) {
	    s = FtpCwd(ftp, realpath);
	    if (!FtpError(s)) {
		s = FtpData(ftp, "LIST", NULL, "r");
	    }
	}
	else {
	    s = FtpData(ftp, "NLST %s", realpath, "r");
	}
	if (realpath[0] == '/')
	    curdir = Strnew_charp(realpath);
	else
	    curdir = Sprintf("/%s", realpath);
	if (Strlastchar(curdir) != '/')
	    Strcat_char(curdir, '/');
    }
    if (FtpError(s)) {
	FtpBye(ftp);
	return NULL;
    }
    host = Strnew_charp("ftp://");
    if (pu->user) {
	Strcat_m_charp(host, pu->user, "@", NULL);
    }
    Strcat_charp(host, pu->host);
    if (Strlastchar(host) == '/')
	Strshrink(host, 1);
    qdir = htmlquote_str(curdir->ptr);
    FTPDIRtmp = Sprintf("<html><head><title>%s%s</title></head><body><h1>Index of %s%s</h1>\n",
			host->ptr, qdir, host->ptr, qdir);
    curdir = Strnew_charp(ftp_escape_str(curdir->ptr));
    qdir = curdir->ptr;
    tmp2 = Strdup(curdir);
    if (Strcmp_charp(curdir, "/") != 0) {
	Strshrink(tmp2, 1);
	while (Strlastchar(tmp2) != '/' && tmp2->length > 0)
	    Strshrink(tmp2, 1);
    }
    if (sv_type == UNIXLIKE_SERVER) {
	Strcat_charp(FTPDIRtmp, "<pre><a href=\"");
    }
    else {
	Strcat_charp(FTPDIRtmp, "<ul><li><a href=\"");
    }
    Strcat_m_charp(FTPDIRtmp, host->ptr,
		   htmlquote_str(tmp2->ptr),
		   "\">[Upper Directory]</a>\n", NULL);

    flist = New_N(char *, nfile_max);
    nfile = 0;
    if (sv_type == UNIXLIKE_SERVER) {
	char *name, *date, *size, *type_str;
	int ftype, max_len, len, i, j;
	Str line_tmp;

	max_len = 0;
	while (tmp2 = Strfgets(FTPDATA(ftp)), tmp2->length > 0) {
	    Strchop(tmp2);
	    if ((ftype = ex_ftpdir_name_size_date(tmp2->ptr, &name, &date, &size))
		== FTPDIR_NONE) {
		continue;
	    }
	    if (!strcmp(".", name) || !strcmp("..", name)) {
		continue;
	    }
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
	    line_tmp = Sprintf("%s%s %-12.12s %6.6s", name, type_str, date, size);
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
		*(date - 1) = '\0';
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
	    Strcat_m_charp(FTPDIRtmp, "<a href=\"",
			   host->ptr,
			   qdir,
			   ftp_escape_str(fn),
			   "\">",
			   htmlquote_str(fn), NULL);
	    if (ftype == FTPDIR_DIR) {
		Strcat_charp(FTPDIRtmp, "/");
		len++;
	    }
	    else if (ftype == FTPDIR_LINK) {
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
	Strcat_charp(FTPDIRtmp, "</pre></body></html>\n");
    }
    else {
	while (tmp2 = Strfgets(FTPDATA(ftp)), tmp2->length > 0) {
	    Strchop(tmp2);
	    flist[nfile++] = mybasename(tmp2->ptr);
	    if (nfile == nfile_max) {
		nfile_max *= 2;
		flist = New_Reuse(char *, flist, nfile_max);
	    }
	}
	qsort(flist, nfile, sizeof(char *), strCmp);
	for (i = 0; i < nfile; i++) {
	    fn = flist[i];
	    Strcat_m_charp(FTPDIRtmp, "<li><a href=\"",
			   host->ptr, qdir,
			   ftp_escape_str(fn),
			   "\">",
			   htmlquote_str(fn),
			   "</a>\n", NULL);
	}
	Strcat_charp(FTPDIRtmp, "</ul></body></html>\n");
    }

    FtpClose(ftp);
    FtpBye(ftp);
    return NULL;
}

static int
ftp_system(FTP ftp)
{
    int sv_type = SERVER_NONE;
    Str tmp;

    fwrite("SYST\r\n", 6, sizeof(char), ftp->wcontrol);
    fflush(ftp->wcontrol);
    tmp = read_response(ftp);
    if (strstr(tmp->ptr, "UNIX") != NULL
	|| !strncmp(tmp->ptr + 4, "Windows_NT", 10)) {	/* :-) */
	sv_type = UNIXLIKE_SERVER;
    }

    return (sv_type);
}

static char *
ftp_escape_str(char *str)
{
    Str s = Strnew();
    char *p, buf[5];
    unsigned char c;

    for (; (c = (unsigned char) *str) != '\0'; str++) {
	p = NULL;
	if (c < '!' || c > '~'
	    || c == '#' || c == '?' || c == '+'
	    || c == '&' || c == '<' || c == '>' || c == '"' || c == '%') {
	    sprintf(buf, "%%%02X", c);
	    p = buf;
	}
	if (p)
	    Strcat_charp(s, p);
	else
	    Strcat_char(s, *str);
    }
    return s->ptr;
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

static char *
ftp_restore_str(char *str)
{
    Str s = Strnew();
    char *p;
    unsigned char c, code[2];

    for (; *str; str++) {
	p = NULL;
	if (*str == '%' && str[1] != '\0' && str[2] != '\0') {
	    c = (unsigned char) str[1];
	    XD_CTOD(c)
		code[0] = c * (unsigned char) 16;
	    c = (unsigned char) str[2];
	    XD_CTOD(c)
		code[0] += c;
	    code[1] = '\0';
	    p = (char *) code;
	    str += 2;
	}
      skip:
	if (p)
	    Strcat_charp(s, p);
	else
	    Strcat_char(s, *str);
    }
    return s->ptr;
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

static Str size_int2str(unsigned long);

static int
ex_ftpdir_name_size_date(char *line, char **name, char **date, char **sizep)
{
    int ftype = FTPDIR_NONE;
    char *cp, *endp;
    Str date_str, name_str, size_str;
    unsigned long size;

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
	    && IS_DIGIT(cp[10]) && IS_DIGIT(cp[11])
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
size_int2str(unsigned long size)
{
    Str size_str;
    int unit;
    double dtmp;
    char *size_format, *unit_str;

    dtmp = (double) size;
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

void
closeFTP(FILE * f)
{
    if (f) {
	fclose(f);
	if (f == ftp->data)
	    ftp->data = NULL;
    }
    FtpBye(ftp);
}
