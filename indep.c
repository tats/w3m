/* $Id: indep.c,v 1.7 2001/11/22 13:30:02 ukai Exp $ */
#include "fm.h"
#include <stdio.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdlib.h>
#include "indep.h"
#include "Str.h"
#include "gc.h"
#include "myctype.h"
#include "entity.h"

char *
allocStr(const char *s, int len)
{
    char *ptr;

    if (s == NULL)
	return NULL;
    if (len == 0)
	len = strlen(s);
    ptr = NewAtom_N(char, len + 1);
    if (ptr == NULL) {
	fprintf(stderr, "fm: Can't allocate string. Give me more memory!\n");
	exit(-1);
    }
    bcopy(s, ptr, len);
    ptr[len] = '\0';
    return ptr;
}

int
strCmp(const void *s1, const void *s2)
{
    unsigned char *p1 = *(unsigned char **) s1;
    unsigned char *p2 = *(unsigned char **) s2;

    while ((*p1 != '\0') && (*p1 == *p2)) {
	p1++;
	p2++;
    }
    return (*p1 - *p2);
}

char *
currentdir()
{
    char *path;
#ifdef HAVE_GETCWD
    path = NewAtom_N(char, MAXPATHLEN);
    getcwd(path, MAXPATHLEN);
#else				/* not HAVE_GETCWD */
#ifdef HAVE_GETWD
    path = NewAtom_N(char, 1024);
    getwd(path);
#else				/* not HAVE_GETWD */
    FILE *f;
    char *p;
    path = NewAtom_N(char, 1024);
    f = popen("pwd", "r");
    fgets(path, 1024, f);
    pclose(f);
    for (p = path; *p; p++)
	if (*p == '\n') {
	    *p = '\0';
	    break;
	}
#endif				/* not HAVE_GETWD */
#endif				/* not HAVE_GETCWD */
    return path;
}

char *
cleanupName(char *name)
{
    char *buf, *p, *q;

    buf = allocStr(name, 0);
    p = buf;
    q = name;
    while (*q != '\0') {
	if (strncmp(p, "/../", 4) == 0) {	/* foo/bar/../FOO */
	    if (p - 2 == buf && strncmp(p - 2, "..", 2) == 0) {
		/* ../../       */
		p += 3;
		q += 3;
	    }
	    else if (p - 3 >= buf && strncmp(p - 3, "/..", 3) == 0) {
		/* ../../../    */
		p += 3;
		q += 3;
	    }
	    else {
		while (p != buf && *--p != '/');	/* ->foo/FOO */
		*p = '\0';
		q += 3;
		strcat(buf, q);
	    }
	}
	else if (strcmp(p, "/..") == 0) {	/* foo/bar/..   */
	    if (p - 2 == buf && strncmp(p - 2, "..", 2) == 0) {
		/* ../..        */
	    }
	    else if (p - 3 >= buf && strncmp(p - 3, "/..", 3) == 0) {
		/* ../../..     */
	    }
	    else {
		while (p != buf && *--p != '/');	/* ->foo/ */
		*++p = '\0';
	    }
	    break;
	}
	else if (strncmp(p, "/./", 3) == 0) {	/* foo/./bar */
	    *p = '\0';		/* -> foo/bar           */
	    q += 2;
	    strcat(buf, q);
	}
	else if (strcmp(p, "/.") == 0) {	/* foo/. */
	    *++p = '\0';	/* -> foo/              */
	    break;
	}
	else if (strncmp(p, "//", 2) == 0) {	/* foo//bar */
	    /* -> foo/bar           */
	    *p = '\0';
	    q++;
	    strcat(buf, q);
	}
	else {
	    p++;
	    q++;
	}
    }
    return buf;
}

char *
expandPath(char *name)
{
    Str userName = NULL;
    char *p;
    struct passwd *passent, *getpwnam(const char *);
    Str extpath = Strnew();

    if (name == NULL)
	return NULL;
    p = name;
    if (*p == '~') {
	p++;
	if (IS_ALPHA(*p)) {
	    userName = Strnew();
	    while (IS_ALNUM(*p) || *p == '_' || *p == '-')
		Strcat_char(userName, *(p++));
	    passent = getpwnam(userName->ptr);
	    if (passent == NULL) {
		p = name;
		goto rest;
	    }
	    Strcat_charp(extpath, passent->pw_dir);
	}
	else {
	    Strcat_charp(extpath, getenv("HOME"));
	}
	if (Strcmp_charp(extpath, "/") == 0 && *p == '/')
	    p++;
    }
  rest:
    Strcat_charp(extpath, p);
    return extpath->ptr;
}

/* string search using the simplest algorithm */
char *
strcasestr(char *s1, char *s2)
{
    int len1, len2;
    len1 = strlen(s1);
    len2 = strlen(s2);
    while (*s1 && len1 >= len2) {
	if (strncasecmp(s1, s2, len2) == 0)
	    return s1;
	s1++;
	len1--;
    }
    return 0;
}

static int
strcasematch(char *s1, char *s2)
{
    int x;
    while (*s1) {
	if (*s2 == '\0')
	    return 1;
	x = tolower(*s1) - tolower(*s2);
	if (x != 0)
	    break;
	s1++;
	s2++;
    }
    return (*s2 == '\0');
}

/* search multiple strings */
int
strcasemstr(char *str, char *srch[], char **ret_ptr)
{
    int i;
    while (*str) {
	for (i = 0; srch[i]; i++) {
	    if (strcasematch(str, srch[i])) {
		if (ret_ptr)
		    *ret_ptr = str;
		return i;
	    }
	}
	str++;
    }
    return -1;
}

char *
remove_space(char *str)
{
    char *p, *q;

    for (p = str; *p && IS_SPACE(*p); p++)
	;
    for (q = p; *q; q++)
	;
    for (; q > p && IS_SPACE(*(q-1)); q--)
	;
    if (*q != '\0')
	return Strnew_charp_n(p, q - p)->ptr;
    return p;
}

int
non_null(char *s)
{
    if (s == NULL)
	return FALSE;
    while (*s) {
	if (!IS_SPACE(*s))
	    return TRUE;
	s++;
    }
    return FALSE;
}

void
cleanup_line(Str s, int mode)
{
    if (s->length >= 2 &&
	s->ptr[s->length - 2] == '\r' &&
	s->ptr[s->length - 1] == '\n') {
	Strshrink(s, 2);
	Strcat_char(s, '\n');
    }
    else if (Strlastchar(s) == '\r')
	s->ptr[s->length - 1] = '\n';
    else if (Strlastchar(s) != '\n')
	Strcat_char(s, '\n');
    if (mode != PAGER_MODE) {
	int i;
	for (i = 0; i < s->length; i++) {
	    if (s->ptr[i] == '\0')
		s->ptr[i] = ' ';
	}
    }
}

int
getescapechar(char **str)
{
    int dummy = -1;
    char *p = *str, *q;

    if (*p == '&')
	p++;
    if (*p == '#') {
	p++;
	if (*p == 'x' || *p == 'X') {
	    p++;
	    if (! IS_XDIGIT(*p)) {
		*str = p;
		return -1;
	    }
	    q = p;
	    for (p++; IS_XDIGIT(*p); p++)
		;
	    q = allocStr(q, p - q);
	    if (*p == ';')
		p++;
	    *str = p;
	    sscanf(q, "%x", &dummy);
	    return dummy;
	} else {
	    if (! IS_DIGIT(*p)) {
		*str = p;
		return -1;
	    }
	    q = p;
	    for (p++; IS_DIGIT(*p); p++)
		;
	    q = allocStr(q, p - q);
	    if (*p == ';')
		p++;
	    *str = p;
	    sscanf(q, "%d", &dummy);
	    return dummy;
	}
    }
    if (! IS_ALPHA(*p)) {
	*str = p;
	return -1;
    }
    q = p;
    for (p++; IS_ALNUM(*p); p++)
	;
    q = allocStr(q, p - q);
    if (*p == ';')
	p++;
    *str = p;
    return getHash_si(&entity, q, -1);
}

char *
getescapecmd(char **s)
{
    char *save = *s;
    Str tmp;
    int ch = getescapechar(s);

    if (ch >= 0)
	return conv_entity(ch);

    if (*save != '&')
	tmp = Strnew_charp("&");
    else
	tmp = Strnew();
    Strcat_charp_n(tmp, save, *s - save);
    return tmp->ptr;
}

char *
html_quote_char(char c)
{
    switch (c) {
    case '&':
	return "&amp;";
    case '<':
	return "&lt;";
    case '>':
	return "&gt;";
    case '"':
	return "&quot;";
    }
    return NULL;
}

char *
html_quote(char *str)
{
    Str tmp = NULL;
    char *p, *q;

    for (p = str; *p; p++) {
	q = html_quote_char(*p);
	if (q) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    Strcat_charp(tmp, q);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

char *
html_unquote(char *str)
{
    Str tmp = NULL;
    char *p, *q;

    for (p = str; *p; ) {
	if (*p == '&') {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    q = getescapecmd(&p);
	    Strcat_charp(tmp, q);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	    p++;
	}
    }

    if (tmp)
	return tmp->ptr;
    return str;
}

static int
url_unquote_char(char **str)
{
    char *p = *str;
    char buf[3];
    int n;

    if (*p != '%')
	return -1;
    p++;
    if (IS_XDIGIT(*p)) {
	buf[0] = *(p++);
	if (IS_XDIGIT(*p)) {
	    buf[1] = *(p++);
	    buf[2] = '\0';
	} else
	    buf[1] = '\0';
	if (sscanf(buf, "%x", &n)) {
	    *str = p;
	    return n;
	}
    }
    return -1;
}

char *
url_quote(char *str)
{
    Str tmp = NULL;
    char *p;
    char buf[4];

    for (p = str; *p; p++) {
	if (IS_CNTRL(*p) || *p == ' ' || ! IS_ASCII(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    sprintf(buf, "%%%02X", (unsigned char)*p);
	    Strcat_charp(tmp, buf);
	} else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

char *
url_unquote(char *str)
{
    Str tmp = NULL;
    char *p, *q;
    int c;

    for (p = str; *p; ) {
	if (*p == '%') {
	    q = p;
	    c = url_unquote_char(&q);
	    if (c >= 0 && (IS_CNTRL(c) || c == ' ' || ! IS_ASCII(c))) {
		if (tmp == NULL)
		    tmp = Strnew_charp_n(str, (int)(p - str));
		if (c != '\0' && c != '\n' && c != '\r')
		    Strcat_char(tmp, (char)c);
		p = q;
		continue;
	    }
	}
	if (tmp)
	    Strcat_char(tmp, *p);
	p++;
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

char *
file_quote(char *str)
{
    Str tmp = NULL;
    char *p;
    char buf[4];

    for (p = str; *p; p++) {
	if (IS_CNTRL(*p) || *p == ' ' || ! IS_ASCII(*p) || *p == '+' ||
	    *p == ':' || *p == '#' || *p == '?' || *p == '&' || *p == '%') {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    sprintf(buf, "%%%02X", (unsigned char)*p);
	    Strcat_charp(tmp, buf);
	} else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

char *
file_unquote(char *str)
{
    Str tmp = NULL;
    char *p, *q;
    int c;

    for (p = str; *p; ) {
	if (*p == '%') {
	    q = p;
	    c = url_unquote_char(&q);
	    if (c >= 0) {
		if (tmp == NULL)
		    tmp = Strnew_charp_n(str, (int)(p - str));
		if (c != '\0' && c != '\n' && c != '\r')
		    Strcat_char(tmp, (char)c);
		p = q;
		continue;
	    }
	}
	if (tmp)
	    Strcat_char(tmp, *p);
	p++;
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

/* rfc1808 safe */
static int
is_url_safe(char c)
{
    switch (c) {
	/* safe */
    case '$':
    case '-':
    case '_':
    case '.':
	return 1;
    default:
	return IS_ALNUM(c);
    }
}

Str
Str_form_quote(Str x)
{
    Str tmp = NULL;
    char *p = x->ptr, *ep = x->ptr + x->length;
    char buf[4];

    for (; p < ep; p++) {
	if (*p == ' ') {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
	    Strcat_char(tmp, '+');
	} else if (! is_url_safe(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
	    sprintf(buf, "%%%02X", (unsigned char)*p);
	    Strcat_charp(tmp, buf);
	} else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp;
    return x;
}

Str
Str_form_unquote(Str x)
{
    Str tmp = NULL;
    char *p = x->ptr, *ep = x->ptr + x->length, *q;
    int c;

    for (; p < ep; ) {
	if (*p == '+') {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
	    Strcat_char(tmp, ' ');
	    p++;
	    continue;
	} else if (*p == '%') {
	    q = p;
	    c = url_unquote_char(&q);
	    if (c >= 0) {
		if (tmp == NULL)
		    tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
		Strcat_char(tmp, (char)c);
		p = q;
		continue;
	    }
	}
	if (tmp)
	    Strcat_char(tmp, *p);
	p++;
    }
    if (tmp)
	return tmp;
    return x;
}

static int
is_shell_safe(char c)
{
    switch (c) {
	/* safe */
    case '/':
    case '.':
    case '_':
    case ':':
	return 1;
    default:
	return IS_ALNUM(c) || (c & 0x80);
    }
}

char *
shell_quote(char *str)
{
    Str tmp = NULL;
    char *p;

    for (p = str; *p; p++) {
        if (! is_shell_safe(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
            Strcat_char(tmp, '\\');
	    Strcat_char(tmp, *p);
	} else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

static char*
w3m_dir(const char *name, char *dft)
{
#ifdef USE_PATH_ENVVAR
    char *value = getenv(name);
    return value ? value : dft;
#else
    return dft;
#endif
}

char *
w3m_lib_dir()
{
    return w3m_dir("W3M_LIB_DIR", LIB_DIR);
}

char *
w3m_help_dir()
{
    return w3m_dir("W3M_HELP_DIR", HELP_DIR);
}
/* Local Variables:    */
/* c-basic-offset: 4   */
/* tab-width: 8        */
/* End:                */
