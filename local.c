#include "fm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef READLINK
#include <unistd.h>
#endif				/* READLINK */
#include "local.h"

#define CGIFN_NORMAL     0
#define CGIFN_DROOT      1
#define CGIFN_CGIBIN     2

/* setup cookie for local CGI */
void
setLocalCookie()
{
    Str buf;
    char hostname[256];
    gethostname(hostname,256);
    buf = Sprintf("%d.%ld@%s",getpid(),lrand48(),hostname);
    Local_cookie = buf->ptr;
}

Buffer *
dirBuffer(char *dname)
{
    Str tmp;
    DIR *d;
    Directory *dir;
    struct stat st;
    char **flist;
    char *p, *qdir;
    Str fbuf = Strnew();
#ifdef READLINK
    struct stat lst;
    char lbuf[1024];
#endif				/* READLINK */
    int i, l, nrow, n = 0, maxlen = 0;
    int nfile, nfile_max = 100;
    Str dirname;

    d = opendir(dname);
    if (d == NULL)
	return NULL;
    dirname = Strnew_charp(dname);
    qdir = htmlquote_str(dirname->ptr);
    tmp = Sprintf("<title>Directory list of %s</title><h1>Directory list of %s</h1>\n", qdir, qdir);
    flist = New_N(char *, nfile_max);
    nfile = 0;
    while ((dir = readdir(d)) != NULL) {
	flist[nfile++] = allocStr(dir->d_name, 0);
	if (nfile == nfile_max) {
	    nfile_max *= 2;
	    flist = New_Reuse(char *, flist, nfile_max);
	}
	if (multicolList) {
	    l = strlen(dir->d_name);
	    if (l > maxlen)
		maxlen = l;
	    n++;
	}
    }

    if (multicolList) {
	l = COLS / (maxlen + 2);
	if (!l)
	    l = 1;
	nrow = (n + l - 1) / l;
	n = 1;
	Strcat_charp(tmp, "<TABLE CELLPADDING=0><TR VALIGN=TOP>\n");
    }
    qsort((void *) flist, nfile, sizeof(char *), strCmp);
    for (i = 0; i < nfile; i++) {
	p = flist[i];
	if (strcmp(p, ".") == 0)
	    continue;
	Strcopy(fbuf, dirname);
	if (Strlastchar(fbuf) != '/')
	    Strcat_char(fbuf, '/');
	Strcat_charp(fbuf, p);
#ifdef READLINK
	if (lstat(fbuf->ptr, &lst) < 0)
	    continue;
#endif				/* READLINK */
	if (stat(fbuf->ptr, &st) < 0)
	    continue;
	if (multicolList) {
	    if (n == 1)
		Strcat_charp(tmp, "<TD><NOBR>");
	}
	else {
	    if (S_ISDIR(st.st_mode))
		Strcat_charp(tmp, "[DIR]&nbsp; ");
#ifdef READLINK
	    else if (S_ISLNK(lst.st_mode))
		Strcat_charp(tmp, "[LINK] ");
#endif				/* READLINE */
	    else
		Strcat_charp(tmp, "[FILE] ");
	}
	Strcat_m_charp(tmp, "<A HREF=\"", htmlquote_str(fbuf->ptr), NULL);
	if (S_ISDIR(st.st_mode))
	    Strcat_char(tmp, '/');
	Strcat_m_charp(tmp, "\">", htmlquote_str(p), NULL);
	if (S_ISDIR(st.st_mode))
	    Strcat_char(tmp, '/');
	Strcat_charp(tmp, "</a>");
	if (multicolList) {
	    if (n++ == nrow) {
		Strcat_charp(tmp, "</NOBR></TD>\n");
		n = 1;
	    }
	    else {
		Strcat_charp(tmp, "<BR>\n");
	    }
	}
	else {
#ifdef READLINK
	    if (S_ISLNK(lst.st_mode)) {
		if ((l = readlink(fbuf->ptr, lbuf, sizeof(lbuf))) > 0) {
		    lbuf[l] = '\0';
		    Strcat_m_charp(tmp, " -> ", htmlquote_str(lbuf), NULL);
		    if (S_ISDIR(st.st_mode))
			Strcat_char(tmp, '/');
		}
	    }
#endif				/* READLINK */
	    Strcat_charp(tmp, "<br>\n");
	}
    }
    if (multicolList) {
	Strcat_charp(tmp, "</TR></TABLE>\n");
    }

    return loadHTMLString(tmp);
}

#ifdef __EMX__
char *
get_os2_dft(const char *name, char *dft)
{
    char *value = getenv(name);
    return value ? value : dft;
}

#define lib_dir get_os2_dft("W3M_LIB_DIR",LIB_DIR)
#else				/* not __EMX__ */
#define lib_dir LIB_DIR
#endif				/* not __EMX__ */

static int
check_local_cgi(char *file, int status)
{
    struct stat st;

#ifdef __EMX__
    if (status != CGIFN_CGIBIN) {
	char tmp[_MAX_PATH];

	_abspath(tmp, lib_dir, _MAX_PATH);	/* Translate '\\'  to  '/' 
						 * 
						 */
	if (strnicmp(file, tmp, strlen(tmp)))	/* and ignore case  */
	    return -1;
    }
#else				/* not __EMX__ */
    if (status != CGIFN_CGIBIN &&
	strncmp(file, lib_dir, strlen(lib_dir)) != 0) {
	/* 
	 * a local-CGI script should be located on either
	 * /cgi-bin/ directory or LIB_DIR (typically /usr/local/lib/w3m).
	 */
	return -1;
    }
#endif				/* not __EMX__ */
    if (stat(file, &st) < 0)
	return -1;
    if ((st.st_uid == geteuid() && (st.st_mode & S_IXUSR)) ||
	(st.st_gid == getegid() && (st.st_mode & S_IXGRP)) ||
	(st.st_mode & S_IXOTH)) {	/* executable */
	return 0;
    }
    return -1;
}

void
set_environ(char *var, char *value)
{
#ifdef HAVE_SETENV
    setenv(var, value, 1);
#else				/* not HAVE_SETENV */
#ifdef HAVE_PUTENV
    Str tmp = Strnew_m_charp(var, "=", value, NULL);
    putenv(tmp->ptr);
#else				/* not HAVE_PUTENV */
    extern char **environ;
    char **ne;
    char *p;
    int i, l, el;
    char **e, **newenv;

    /* I have no setenv() nor putenv() */
    /* This part is taken from terms.c of skkfep */
    l = strlen(var);
    for (e = environ, i = 0; *e != NULL; e++, i++) {
	if (strncmp(e, var, l) == 0 && (*e)[l] == '=') {
	    el = strlen(*e) - l - 1;
	    if (el >= strlen(value)) {
		strcpy(*e + l + 1, value);
		return 0;
	    }
	    else {
		for (; *e != NULL; e++, i++) {
		    *e = *(e + 1);
		}
		i--;
		break;
	    }
	}
    }
    newenv = (char **) GC_malloc((i + 2) * sizeof(char *));
    if (newenv == NULL)
	return;
    for (e = environ, ne = newenv; *e != NULL; *(ne++) = *(e++));
    *(ne++) = p;
    *ne = NULL;
    environ = newenv;
#endif				/* not HAVE_PUTENV */
#endif				/* not HAVE_SETENV */
}

static void
set_cgi_environ(char *name, char *fn, char *req_uri)
{
    set_environ("SERVER_SOFTWARE", version);
    set_environ("SERVER_PROTOCOL", "HTTP/1.0");
    set_environ("SERVER_NAME", "localhost");
    set_environ("SERVER_PORT", "80");	/* dummy */
    set_environ("REMOTE_HOST", "localhost");
    set_environ("REMOTE_ADDR", "127.0.0.1");
    set_environ("GATEWAY_INTERFACE", "CGI/1.1");

    set_environ("SCRIPT_NAME", name);
    set_environ("SCRIPT_FILENAME", fn);
    set_environ("REQUEST_URI", req_uri);
    set_environ("LOCAL_COOKIE",Local_cookie);
}

static Str
checkPath(char *fn, char *path)
{
    Str tmp;
    struct stat st;
    while (*path) {
	tmp = Strnew();
	while (*path && *path != ':')
	    Strcat_char(tmp, *path++);
	if (*path == ':')
	    path++;
	if (Strlastchar(tmp) != '/')
	    Strcat_char(tmp, '/');
	Strcat_charp(tmp, fn);
	if (stat(tmp->ptr, &st) == 0)
	    return tmp;
    }
    return NULL;
}

static char *
cgi_filename(char *fn, int *status)
{
    Str tmp;
    struct stat st;
    if (cgi_bin != NULL && strncmp(fn, "/cgi-bin/", 9) == 0) {
	*status = CGIFN_CGIBIN;
	tmp = checkPath(fn + 9, cgi_bin);
	if (tmp == NULL)
	    return fn;
	return tmp->ptr;
    }
    if (strncmp(fn, "/$LIB/", 6) == 0) {
	*status = CGIFN_NORMAL;
	tmp = Strnew_charp(lib_dir);
	fn += 5;
	if (Strlastchar(tmp) == '/')
	    fn++;
	Strcat_charp(tmp, fn);
	return tmp->ptr;
    }
    if (*fn == '/' && document_root != NULL && stat(fn, &st) < 0) {
	*status = CGIFN_DROOT;
	tmp = Strnew_charp(document_root);
	if (Strlastchar(tmp) != '/')
	    Strcat_char(tmp, '/');
	Strcat_charp(tmp, fn);
	return tmp->ptr;
    }
    *status = CGIFN_NORMAL;
    return fn;
}

FILE *
localcgi_post(char *file, FormList * request, char *referer)
{
    FILE *f;
    Str tmp1, tmp2;
    int status;

    tmp1 = Strnew_charp(file);
    file = cgi_filename(file, &status);
    if (check_local_cgi(file, status) < 0)
	return NULL;
    set_cgi_environ(tmp1->ptr, file, tmp1->ptr);
    set_environ("REQUEST_METHOD", "POST");
    set_environ("CONTENT_LENGTH", Sprintf("%d", request->length)->ptr);
    if (referer && referer != NO_REFERER)
        set_environ("HTTP_REFERER",referer);
    if (request->enctype == FORM_ENCTYPE_MULTIPART) {
	set_environ("CONTENT_TYPE",
		    Sprintf("multipart/form-data; boundary=%s", request->boundary)->ptr);
    }
    else {
	set_environ("CONTENT_TYPE", "application/x-www-form-urlencoded");
    }
    tmp1 = tmpfname(TMPF_DFL, NULL);
    f = fopen(tmp1->ptr, "w");
    if (f == NULL)
	return NULL;
    pushText(fileToDelete, tmp1->ptr);
    if (request->enctype == FORM_ENCTYPE_MULTIPART) {
	FILE *fd;
	int c;
	fd = fopen(request->body, "r");
	if (fd != NULL) {
	    while ((c = fgetc(fd)) != EOF)
		fputc(c, f);
	    fclose(fd);
	}
    }
    else {
	fputs(request->body, f);
    }
    fclose(f);
    tmp2 = Sprintf("%s < %s", file, tmp1->ptr);
#ifdef __EMX__
    f = popen(tmp2->ptr, "r");
#else
    tmp1 = Strnew_charp(CurrentDir);
    chdir(mydirname(file));
    f = popen(tmp2->ptr, "r");
    chdir(tmp1->ptr);
#endif
    return f;
}

FILE *
localcgi_get(char *file, char *request, char *referer)
{
    FILE *f;
    Str tmp1, tmp2;
    int status;
    
    tmp1 = Strnew_charp(file);
    file = cgi_filename(file, &status);
    if (check_local_cgi(file, status) < 0)
	return NULL;
    if (!strcmp(request, "")) {
        set_cgi_environ(tmp1->ptr, file, tmp1->ptr);
    } else {
        set_cgi_environ(tmp1->ptr, file,
			Strnew_m_charp(tmp1->ptr, "?", request, NULL)->ptr);
    }
    if (referer && referer != NO_REFERER)
        set_environ("HTTP_REFERER",referer);
    set_environ("REQUEST_METHOD", "GET");
    set_environ("QUERY_STRING", request);
    tmp2 = Sprintf("%s", file);
#ifdef __EMX__
    f = popen(tmp2->ptr, "r");
#else
    tmp1 = Strnew_charp(CurrentDir);
    chdir(mydirname(file));
    f = popen(tmp2->ptr, "r");
    chdir(tmp1->ptr);
#endif
    return f;
}
