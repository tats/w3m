/* $Id: indep.c,v 1.1 2001/11/08 05:15:01 a-ito Exp $ */
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

#if defined(__EMX__)&&!defined(JP_CHARSET)
int CodePage=0;
#endif

static int is_safe(int);

struct table3 {
    char *item1;
    unsigned char item2;
};

struct table3 escapetbl[] =
{
    {"lt", '<'},
    {"gt", '>'},
    {"amp", '&'},
    {"quot", '"'},
    {"LT", '<'},
    {"GT", '>'},
    {"AMP", '&'},
    {"QUOT", '"'},
/* for latin1_tbl */
    {"nbsp", 128 + 32},
    {"NBSP", 128 + 32},
    {"iexcl", 128 + 33},
    {"cent", 128 + 34},
    {"pound", 128 + 35},
    {"curren", 128 + 36},
    {"yen", 128 + 37},
    {"brvbar", 128 + 38},
    {"sect", 128 + 39},
    {"uml", 128 + 40},
    {"copy", 128 + 41},
    {"ordf", 128 + 42},
    {"laquo", 128 + 43},
    {"not", 128 + 44},
    {"shy", 128 + 45},
    {"reg", 128 + 46},
    {"macr", 128 + 47},
    {"deg", 128 + 48},
    {"plusmn", 128 + 49},
    {"sup2", 128 + 50},
    {"sup3", 128 + 51},
    {"acute", 128 + 52},
    {"micro", 128 + 53},
    {"para", 128 + 54},
    {"middot", 128 + 55},
    {"cedil", 128 + 56},
    {"sup1", 128 + 57},
    {"ordm", 128 + 58},
    {"raquo", 128 + 59},
    {"frac14", 128 + 60},
    {"frac12", 128 + 61},
    {"frac34", 128 + 62},
    {"iquest", 128 + 63},
    {"Agrave", 128 + 64},
    {"Aacute", 128 + 65},
    {"Acirc", 128 + 66},
    {"Atilde", 128 + 67},
    {"Auml", 128 + 68},
    {"Aring", 128 + 69},
    {"AElig", 128 + 70},
    {"Ccedil", 128 + 71},
    {"Egrave", 128 + 72},
    {"Eacute", 128 + 73},
    {"Ecirc", 128 + 74},
    {"Euml", 128 + 75},
    {"Igrave", 128 + 76},
    {"Iacute", 128 + 77},
    {"Icirc", 128 + 78},
    {"Iuml", 128 + 79},
    {"ETH", 128 + 80},
    {"Ntilde", 128 + 81},
    {"Ograve", 128 + 82},
    {"Oacute", 128 + 83},
    {"Ocirc", 128 + 84},
    {"Otilde", 128 + 85},
    {"Ouml", 128 + 86},
    {"times", 128 + 87},
    {"Oslash", 128 + 88},
    {"Ugrave", 128 + 89},
    {"Uacute", 128 + 90},
    {"Ucirc", 128 + 91},
    {"Uuml", 128 + 92},
    {"Yacute", 128 + 93},
    {"THORN", 128 + 94},
    {"szlig", 128 + 95},
    {"agrave", 128 + 96},
    {"aacute", 128 + 97},
    {"acirc", 128 + 98},
    {"atilde", 128 + 99},
    {"auml", 128 + 100},
    {"aring", 128 + 101},
    {"aelig", 128 + 102},
    {"ccedil", 128 + 103},
    {"egrave", 128 + 104},
    {"eacute", 128 + 105},
    {"ecirc", 128 + 106},
    {"euml", 128 + 107},
    {"igrave", 128 + 108},
    {"iacute", 128 + 109},
    {"icirc", 128 + 110},
    {"iuml", 128 + 111},
    {"eth", 128 + 112},
    {"ntilde", 128 + 113},
    {"ograve", 128 + 114},
    {"oacute", 128 + 115},
    {"ocirc", 128 + 116},
    {"otilde", 128 + 117},
    {"ouml", 128 + 118},
    {"divide", 128 + 119},
    {"oslash", 128 + 120},
    {"ugrave", 128 + 121},
    {"uacute", 128 + 122},
    {"ucirc", 128 + 123},
    {"uuml", 128 + 124},
    {"yacute", 128 + 125},
    {"thorn", 128 + 126},
    {"yuml", 128 + 127},
    {NULL, 0},
};

#ifdef JP_CHARSET
static char *latin1_tbl[128] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,	/* 0-  7 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,	/* 8- 15 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,	/* 16- 23 */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,	/* 24- 31 */
    NBSP, "!", "¡ñ", "¡ò", NULL, "¡ï", "|", "¡ø",	/* 32- 39 */
    "¡¯", "(C)", NULL, "¢ã", "¢Ì", "-", "(R)", "¡±",	/* 40- 47 */
    "¡ë", "¡Þ", "2", "3", "'", "¦Ì", "¢ù", "¡¦",	/* 48- 55 */
    ",", "1", NULL, "¢ä", "1/4", "1/2", "3/4", "?",	/* 56- 63 */
    "A`", "A'", "A^", "A~", "Ae", "¢ò", "AE", "C",	/* 64- 71 */
    "E`", "E'", "E^", "E", "I`", "I'", "I^", "I",	/* 72- 79 */
    "D", "N~", "O`", "O'", "O^", "O~", "Oe", "¡ß",	/* 80- 87 */
    "¦Õ", "U`", "U'", "U^", "Ue", "Y'", "th", "ss",	/* 88- 95 */
    "a`", "a'", "a^", "a~", "ae", "a", "ae", "c",	/* 96-103 */
    "e`", "e'", "e^", "e", "i`", "i'", "i^", "i",	/* 104-111 */
    "dh", "n~", "o`", "o'", "o^", "o~", "oe", "¡à",	/* 112-119 */
    "¦Õ", "u`", "u'", "u^", "ue", "y'", "th", "y"	/* 120-127 */
};

char *
conv_latin1(int ch)
{
    static char dummy[] = {0, 0};
    if (ch > 0xff) {
	/* it must be a unicode character; w3m can't handle it */
	return "??";
    }
    if (!IS_PRINT(ch) && !IS_CNTRL(ch)) {
	char *save;
	if (ch >= 0x80 && (save = latin1_tbl[ch - 0x80])) {
	    return save;
	}
	return "?";
    }
    else {
	dummy[0] = ch;
	return dummy;
    }
}
#else				/* not JP_CHARSET */
#ifdef __EMX__
/*
 * Character conversion table
 * ( to code page 850 from iso-8859-1 )
 *
 * Following character constants are in code page 850.
 */
static char latin1_tbl[96] = {
  ' ', '\255', '\275', '\234', '\317', '\276', '\335', '\365',
  '\371', '\270', '\246', '\256', '\252', '\360', '\251', '\356',
  '\370', '\361', '\375', '\374', '\357', '\346', '\364', '\372',
  '\367', '\373', '\247', '\257', '\254', '\253', '\363', '\250',
  '\267', '\265', '\266', '\307', '\216', '\217', '\222', '\200',
  '\324', '\220', '\322', '\323', '\336', '\326', '\327', '\330',
  '\321', '\245', '\343', '\340', '\342', '\345', '\231', '\236',
  '\235', '\353', '\351', '\352', '\232', '\355', '\350', '\341',
  '\205', '\240', '\203', '\306', '\204', '\206', '\221', '\207',
  '\212', '\202', '\210', '\211', '\215', '\241', '\214', '\213',
  '\320', '\244', '\225', '\242', '\223', '\344', '\224', '\366',
  '\233', '\227', '\243', '\226', '\201', '\354', '\347', '\230'
};
#endif

char *
conv_latin1(int ch)
{
    static char dummy[2] = {0, 0};
    if (ch > 0xff) {
	/* it must be a unicode character; w3m can't handle it */
	return "??";
    }
#ifdef __EMX__
    {
	if(CodePage==850&&ch>=160)
	    ch=latin1_tbl[(unsigned)ch-160];
    }
#endif
    dummy[0] = ch;
    return dummy;
}
#endif				/* not JP_CHARSET */

int
getescapechar(char **s)
{
    int i, dummy = 0;
    char *save;
    Str tmp = Strnew();

    save = *s;
    if (**s == '&')
	(*s)++;
    if (**s == '#') {
	if (*(*s + 1) == 'x') {
	    (*s)++;
	    sscanf(*s + 1, "%x", &dummy);
	}
	else if (*(*s + 1) == 'X') {
	    (*s)++;
	    sscanf(*s + 1, "%X", &dummy);
	}
	else {
	    sscanf(*s + 1, "%d", &dummy);
	}
	(*s)++;
	save = *s;
	while (**s && **s != ';') {
	    if (!IS_ALNUM(**s)) {
		if (*s > save)
		    break;
		else
		    goto fail;
	    }
	    (*s)++;
	}
	if (**s == ';')
	    (*s)++;
	return dummy;
    }
    while (IS_ALNUM(**s)) {
	Strcat_char(tmp, **s);
	(*s)++;
    }
    /* if (**s != ';') goto fail; */
    if (**s == ';')
	(*s)++;
    for (i = 0; escapetbl[i].item1 != NULL; i++)
	if (Strcmp_charp(tmp, escapetbl[i].item1) == 0) {
	    return escapetbl[i].item2;
	}
  fail:
    return '\0';
}

char *
getescapecmd(char **s)
{
    char *save = *s;
    Str tmp, tmp2;
    int ch = getescapechar(s);
    if (ch)
	return conv_latin1(ch);

    tmp = Strnew_charp_n(save, *s - save);
    if (tmp->ptr[0] != '&') {
	tmp2 = Strnew_charp("&");
	Strcat(tmp2, tmp);
	return tmp2->ptr;
    }
    return tmp->ptr;
}

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

void
copydicname(char *s, char *fn)
{
    strcpy(s, fn);
    for (fn = &s[strlen(s)]; s < fn; fn--) {
	if (*fn == '/') {
	    *(fn + 1) = '\0';
	    return;
	}
    }
    if (*fn == '/')
	*(fn + 1) = '\0';
    else
	*fn = '\0';
}

#ifndef __EMX__
char *
currentdir()
{
    char *path;
#ifdef GETCWD
    path = New_N(char, MAXPATHLEN);
    getcwd(path, MAXPATHLEN);
#else				/* not GETCWD */
#ifdef GETWD
    path = New_N(char, 1024);
    getwd(path);
#else				/* not GETWD */
    FILE *f;
    char *p;
    path = New_N(char, 1024);
    f = popen("pwd", "r");
    fgets(path, 1024, f);
    pclose(f);
    for (p = path; *p; p++)
	if (*p == '\n') {
	    *p = '\0';
	    break;
	}
#endif				/* not GETWD */
#endif				/* not GETCWD */
    return path;
}
#endif				/* __EMX__ */

char *
cleanupName(char *name)
{
    char *buf, *p, *q;

    buf = allocStr(name, 0);
    p = buf;
    q = name;
    while (*q != '\0' && *q != '?') {
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
#ifdef CYGWIN
	    if (p == buf) {	/* //DRIVE/foo  */
		p += 2;
		q += 2;
		continue;
	    }
#endif				/* CYGWIN */
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
cleanup_str(char *str)
{
    Str tmp = NULL;
    char *s = str, *c;

    while (*s) {
	if (*s == '&') {
	    if (tmp == NULL) {
		tmp = Strnew();
		Strcat_charp_n(tmp, str, s - str);
	    }
	    c = getescapecmd(&s);
	    Strcat_charp(tmp, c);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *s);
	    s++;
	}
    }

    if (tmp)
	return tmp->ptr;
    else
	return str;
}

char *
remove_space(char *str)
{
    Str s = Strnew();
    while (*str) {
	if (!IS_SPACE(*str))
	    Strcat_char(s, *str);
	str++;
    }
    return s->ptr;
}

char *
htmlquote_char(char c)
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
htmlquote_str(char *str)
{
    Str tmp = NULL;
    char *p, *q;
    for (p = str; *p; p++) {
	q = htmlquote_char(*p);
	if (q) {
	    if (tmp == NULL) {
		tmp = Strnew();
		Strcat_charp_n(tmp, str, p - str);
	    }
	    Strcat_charp(tmp, q);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    else
	return str;
}

Str
form_quote(Str x)
{
    Str r = Strnew();
    int i;
    char c;
    for (i = 0; i < x->length; i++) {
	c = x->ptr[i];
	if (c == ' ')
	    Strcat_char(r, '+');
	else if (IS_ALNUM(c) || is_safe(c)) {
	    Strcat_char(r, c);
	}
	else {
	    Strcat_charp(r, "%");
	    Strcat(r, Sprintf("%02X", (c & 0xff)));
	}
    }
    return r;
}

/* rfc1808 safe */
static int
is_safe(int c)
{
    switch (c) {
	/* safe */
    case '$':
    case '-':
    case '_':
    case '.':
	return 1;
    default:
	return 0;
    }
}

Str
form_unquote(Str x)
{
    Str r = Strnew();
    int i, j;
    char c;
    Str num;

    for (i = 0; i < x->length; i++) {
	c = x->ptr[i];
	if (c == '+')
	    Strcat_char(r, ' ');
	else if (c == '%') {
	    num = Strnew_charp("0");
	    if (IS_ALNUM(x->ptr[i + 1])) {
		Strcat_char(num, x->ptr[i + 1]);
		i++;
		if (IS_ALNUM(x->ptr[i + 1])) {
		    Strcat_char(num, x->ptr[i + 1]);
		    i++;
		}
	    }
	    sscanf(num->ptr, "%x", &j);
	    Strcat_char(r, (char) j);
	}
	else
	    Strcat_char(r, c);
    }
    return r;
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

Str
escape_shellchar(Str s)
{
    Str x = Strnew();
    int i;
    for (i = 0; i < s->length; i++) {
	switch (s->ptr[i]) {
	case ';':
	case '&':
	case '|':
	case '$':
	case '!':
	case '(':
	case ')':
	case '{':
	case '}':
	case '*':
	case '?':
	    Strcat_char(x, '\\');
	    /* continue to the next */
	default:
	    Strcat_char(x, s->ptr[i]);
	}
    }
    return x;
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

/* Local Variables:    */
/* c-basic-offset: 4   */
/* tab-width: 8        */
/* End:                */
