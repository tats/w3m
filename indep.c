/* $Id: indep.c,v 1.38 2007/05/23 15:06:05 inu Exp $ */
#include "fm.h"
#include <stdio.h>
#ifndef __MINGW32_VERSION
#include <pwd.h>
#endif /* __MINGW32_VERSION */
#include <sys/param.h>
#include <sys/types.h>
#include <stdlib.h>
#include "indep.h"
#include "Str.h"
#include <gc.h>
#include "myctype.h"
#include "entity.h"

unsigned char QUOTE_MAP[0x100] = {
    /* NUL SOH STX ETX EOT ENQ ACK BEL  BS  HT  LF  VT  FF  CR  SO  SI */
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    /* DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN  EM SUB ESC  FS  GS  RS  US */
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    /* SPC   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
    24, 72, 76, 40, 8, 40, 41, 72, 72, 72, 72, 40, 72, 8, 0, 64,
    /*   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 72, 74, 72, 75, 40,
    /*   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
    72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 72, 72, 72, 0,
    /*   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
    72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 72, 72, 72, 24,

    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

char *HTML_QUOTE_MAP[] = {
    NULL,
    "&amp;",
    "&lt;",
    "&gt;",
    "&quot;",
    NULL,
    NULL,
    NULL,
};

clen_t
strtoclen(const char *s)
{
#ifdef HAVE_STRTOLL
    return strtoll(s, NULL, 10);
#elif defined(HAVE_STRTOQ)
    return strtoq(s, NULL, 10);
#elif defined(HAVE_ATOLL)
    return atoll(s);
#elif defined(HAVE_ATOQ)
    return atoq(s);
#else
    return atoi(s);
#endif
}

#ifndef HAVE_BCOPY
void
bcopy(const void *src, void *dest, int len)
{
    int i;
    if (src == dest)
	return;
    if (src < dest) {
	for (i = len - 1; i >= 0; i--)
	    ((char *)dest)[i] = ((const char *)src)[i];
    }
    else {			/* src > dest */
	for (i = 0; i < len; i++)
	    ((char *)dest)[i] = ((const char *)src)[i];
    }
}

void
bzero(void *ptr, int len)
{
    int i;
    char *p = ptr;
    for (i = 0; i < len; i++)
	*(p++) = 0;
}
#endif				/* not HAVE_BCOPY */

char *
allocStr(const char *s, int len)
{
    char *ptr;

    if (s == NULL)
	return NULL;
    if (len < 0)
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
    return strcmp(*(const char **)s1, *(const char **)s2);
}

char *
currentdir()
{
    char *path;
#ifdef HAVE_GETCWD
#ifdef MAXPATHLEN
    path = NewAtom_N(char, MAXPATHLEN);
    getcwd(path, MAXPATHLEN);
#else
    path = getcwd(NULL, 0);
#endif
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

    buf = allocStr(name, -1);
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
		while (p != buf && *--p != '/') ;	/* ->foo/FOO */
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
		while (p != buf && *--p != '/') ;	/* ->foo/ */
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
    char *p;
    struct passwd *passent, *getpwnam(const char *);
    Str extpath = NULL;

    if (name == NULL)
	return NULL;
    p = name;
    if (*p == '~') {
	p++;
#ifndef __MINGW32_VERSION
	if (IS_ALPHA(*p)) {
	    char *q = strchr(p, '/');
	    if (q) {		/* ~user/dir... */
		passent = getpwnam(allocStr(p, q - p));
		p = q;
	    }
	    else {		/* ~user */
		passent = getpwnam(p);
		p = "";
	    }
	    if (!passent)
		goto rest;
	    extpath = Strnew_charp(passent->pw_dir);
	} else
#endif /* __MINGW32_VERSION */
	  if (*p == '/' || *p == '\0') {	/* ~/dir... or ~ */
	    extpath = Strnew_charp(getenv("HOME"));
	}
	else
	    goto rest;
	if (Strcmp_charp(extpath, "/") == 0 && *p == '/')
	    p++;
	Strcat_charp(extpath, p);
	return extpath->ptr;
    }
  rest:
    return name;
}

#ifndef HAVE_STRCHR
char *
strchr(const char *s, int c)
{
    while (*s) {
	if ((unsigned char)*s == c)
	    return (char *)s;
	s++;
    }
    return NULL;
}
#endif				/* not HAVE_STRCHR */

#ifndef HAVE_STRCASECMP
int
strcasecmp(const char *s1, const char *s2)
{
    int x;
    while (*s1) {
	x = TOLOWER(*s1) - TOLOWER(*s2);
	if (x != 0)
	    return x;
	s1++;
	s2++;
    }
    return -TOLOWER(*s2);
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    int x;
    while (*s1 && n) {
	x = TOLOWER(*s1) - TOLOWER(*s2);
	if (x != 0)
	    return x;
	s1++;
	s2++;
	n--;
    }
    return n ? -TOLOWER(*s2) : 0;
}
#endif				/* not HAVE_STRCASECMP */

#ifndef HAVE_STRCASESTR
/* string search using the simplest algorithm */
char *
strcasestr(const char *s1, const char *s2)
{
    int len1, len2;
    if (s2 == NULL)
	return (char *)s1;
    if (*s2 == '\0')
	return (char *)s1;
    len1 = strlen(s1);
    len2 = strlen(s2);
    while (*s1 && len1 >= len2) {
	if (strncasecmp(s1, s2, len2) == 0)
	    return (char *)s1;
	s1++;
	len1--;
    }
    return 0;
}
#endif

static int
strcasematch(char *s1, char *s2)
{
    int x;
    while (*s1) {
	if (*s2 == '\0')
	    return 1;
	x = TOLOWER(*s1) - TOLOWER(*s2);
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

    for (p = str; *p && IS_SPACE(*p); p++) ;
    for (q = p; *q; q++) ;
    for (; q > p && IS_SPACE(*(q - 1)); q--) ;
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
	s->ptr[s->length - 2] == '\r' && s->ptr[s->length - 1] == '\n') {
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
    int strict_entity = TRUE;

    if (*p == '&')
	p++;
    if (*p == '#') {
	p++;
	if (*p == 'x' || *p == 'X') {
	    p++;
	    if (!IS_XDIGIT(*p)) {
		*str = p;
		return -1;
	    }
	    for (dummy = GET_MYCDIGIT(*p), p++; IS_XDIGIT(*p); p++)
		dummy = dummy * 0x10 + GET_MYCDIGIT(*p);
	    if (*p == ';')
		p++;
	    *str = p;
	    return dummy;
	}
	else {
	    if (!IS_DIGIT(*p)) {
		*str = p;
		return -1;
	    }
	    for (dummy = GET_MYCDIGIT(*p), p++; IS_DIGIT(*p); p++)
		dummy = dummy * 10 + GET_MYCDIGIT(*p);
	    if (*p == ';')
		p++;
	    *str = p;
	    return dummy;
	}
    }
    if (!IS_ALPHA(*p)) {
	*str = p;
	return -1;
    }
    q = p;
    for (p++; IS_ALNUM(*p); p++) ;
    q = allocStr(q, p - q);
    if (strcasestr("lt gt amp quot nbsp", q) && *p != '=') {
	/* a character entity MUST be terminated with ";". However,
	 * there's MANY web pages which uses &lt , &gt or something
	 * like them as &lt;, &gt;, etc. Therefore, we treat the most
	 * popular character entities (including &#xxxx;) without
	 * the last ";" as character entities. If the trailing character
	 * is "=", it must be a part of query in an URL. So &lt=, &gt=, etc.
	 * are not regarded as character entities.
	 */
	strict_entity = FALSE;
    }
    if (*p == ';')
	p++;
    else if (strict_entity) {
	*str = p;
	return -1;
    }
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

    for (p = str; *p;) {
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

static char xdigit[0x10] = "0123456789ABCDEF";

#define url_unquote_char(pstr) \
  ((IS_XDIGIT((*(pstr))[1]) && IS_XDIGIT((*(pstr))[2])) ? \
    (*(pstr) += 3, (GET_MYCDIGIT((*(pstr))[-2]) << 4) | GET_MYCDIGIT((*(pstr))[-1])) : \
   -1)

char *
url_quote(char *str)
{
    Str tmp = NULL;
    char *p;

    for (p = str; *p; p++) {
	if (is_url_quote(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    Strcat_char(tmp, '%');
	    Strcat_char(tmp, xdigit[((unsigned char)*p >> 4) & 0xF]);
	    Strcat_char(tmp, xdigit[(unsigned char)*p & 0xF]);
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
file_quote(char *str)
{
    Str tmp = NULL;
    char *p;
    char buf[4];

    for (p = str; *p; p++) {
	if (is_file_quote(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    sprintf(buf, "%%%02X", (unsigned char)*p);
	    Strcat_charp(tmp, buf);
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
file_unquote(char *str)
{
    Str tmp = NULL;
    char *p, *q;
    int c;

    for (p = str; *p;) {
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
	}
	else if (is_url_unsafe(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
	    sprintf(buf, "%%%02X", (unsigned char)*p);
	    Strcat_charp(tmp, buf);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp;
    return x;
}


Str
Str_url_unquote(Str x, int is_form, int safe)
{
    Str tmp = NULL;
    char *p = x->ptr, *ep = x->ptr + x->length, *q;
    int c;

    for (; p < ep;) {
	if (is_form && *p == '+') {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(x->ptr, (int)(p - x->ptr));
	    Strcat_char(tmp, ' ');
	    p++;
	    continue;
	}
	else if (*p == '%') {
	    q = p;
	    c = url_unquote_char(&q);
	    if (c >= 0 && (!safe || !IS_ASCII(c) || !is_file_quote(c))) {
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

char *
shell_quote(char *str)
{
    Str tmp = NULL;
    char *p;

    for (p = str; *p; p++) {
	if (is_shell_unsafe(*p)) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    Strcat_char(tmp, '\\');
	    Strcat_char(tmp, *p);
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

static char *
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
w3m_auxbin_dir()
{
    return w3m_dir("W3M_AUXBIN_DIR", AUXBIN_DIR);
}

char *
w3m_lib_dir()
{
    /* FIXME: use W3M_CGIBIN_DIR? */
    return w3m_dir("W3M_LIB_DIR", CGIBIN_DIR);
}

char *
w3m_etc_dir()
{
    return w3m_dir("W3M_ETC_DIR", ETC_DIR);
}

char *
w3m_conf_dir()
{
    return w3m_dir("W3M_CONF_DIR", CONF_DIR);
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
