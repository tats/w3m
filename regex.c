/* 
 * regex: Regular expression pattern match library
 * 
 * by A.ITO, December 1989
 */

#ifdef REGEX_DEBUG
#include <sys/types.h>
#include <malloc.h>
#endif				/* REGEX_DEBUG */
#include <ctype.h>
#include <gc.h>
#ifdef __EMX__
#include <strings.h>
#endif
#include "fm.h"
#include "regex.h"

#ifdef JP_CHARSET
#define RE_KANJI(p)	(((unsigned char)*(p) << 8) | (unsigned char)*((p)+1))
#endif

#define RE_WHICH_RANGE	0xffff

static Regex DefaultRegex;
#define CompiledRegex DefaultRegex.re
#define Cstorage DefaultRegex.storage

static longchar *st_ptr;

static int regmatch(regexchar *, char *, int, int, char **);
static int regmatch1(regexchar *, longchar);
static int matchWhich(longchar *, longchar);


/* 
 * regexCompile: compile regular expression
 */
char *
regexCompile(char *ex, int igncase)
{
    char *msg;
    newRegex(ex, igncase, &DefaultRegex, &msg);
    return msg;
}

Regex *
newRegex(char *ex, int igncase, Regex * regex, char **msg)
{
    char *p;
    longchar *r;
    regexchar *re = regex->re - 1;
    int m;

    if (regex == 0)
	regex = (Regex *) GC_malloc_atomic(sizeof(Regex));
    st_ptr = regex->storage;
    for (p = ex; *p != '\0'; p++) {
	switch (*p) {
	case '.':
	    re++;
	    re->pattern = NULL;
	    re->mode = RE_ANY;
	    break;
	case '$':
	    re++;
	    re->pattern = NULL;
	    re->mode = RE_END;
	    break;
	case '^':
	    re++;
	    re->pattern = NULL;
	    re->mode = RE_BEGIN;
	    break;
	case '*':
	    if (!(re->mode & RE_ANY) && re->pattern == NULL) {
		if (msg)
		    *msg = "Invalid regular expression";
		return NULL;
	    }
	    re->mode |= RE_ANYTIME;
	    break;
	case '[':
	    r = st_ptr;
	    if (*++p == '^') {
		p++;
		m = RE_EXCEPT;
	    }
	    else
		m = RE_WHICH;
	    while (*p != ']') {
		if (*p == '\\') {
		    *(st_ptr++) = *(p + 1);
		    p += 2;
		}
		else if (*p == '-') {
		    *(st_ptr++) = RE_WHICH_RANGE;
		    p++;
		}
		else if (*p == '\0') {
		    if (msg)
			*msg = "Missing ]";
		    return NULL;
		}
#ifdef JP_CHARSET
		else if (IS_KANJI1(*p)) {
		    *(st_ptr++) = RE_KANJI(p);
		    p += 2;
		}
#endif
		else
		    *(st_ptr++) = (unsigned char)*(p++);
	    }
	    *(st_ptr++) = '\0';
	    re++;
	    re->pattern = r;
	    re->mode = m;
	    break;
	case '\\':
	    p++;
	default:
	    re++;
#ifdef JP_CHARSET
	    if (IS_KANJI1(*p)) {
		*(st_ptr) = RE_KANJI(p);
		p++;
	    }
	    else
#endif
		*st_ptr = (unsigned char)*p;
	    re->pattern = st_ptr;
	    st_ptr++;
	    re->mode = RE_NORMAL;
	    if (igncase)
		re->mode |= RE_IGNCASE;
	}
	if (st_ptr >= &Cstorage[STORAGE_MAX] ||
	    re >= &CompiledRegex[REGEX_MAX]) {
	    if (msg)
		*msg = "Regular expression too long";
	    return NULL;
	}
    }
    re++;
    re->mode = RE_ENDMARK;
    if (msg)
	*msg = NULL;
    return regex;
}

/* 
 * regexMatch: match regular expression
 */
int
regexMatch(char *str, int len, int firstp)
{
    return RegexMatch(&DefaultRegex, str, len, firstp);
}

int
RegexMatch(Regex * re, char *str, int len, int firstp)
{
    char *p, *ep;

    if (str == NULL)
	return 0;
    re->position = NULL;
    ep = str + ((len == 0) ? strlen(str) : len);
    for (p = str; p < ep; p++) {
	switch (regmatch(re->re, p, ep - p, firstp && (p == str), &re->lposition)) {
	case 1:
	    re->position = p;
	    return 1;
	case -1:
	    re->position = NULL;
	    return -1;
	}
#ifdef JP_CHARSET
	if (IS_KANJI1(*p))
	    p++;
#endif
    }
    return 0;
}

/* 
 * matchedPosition: last matched position
 */
void
MatchedPosition(Regex * re, char **first, char **last)
{
    *first = re->position;
    *last = re->lposition;
}

void
matchedPosition(char **first, char **last)
{
    *first = DefaultRegex.position;
    *last = DefaultRegex.lposition;
}

/* 
 * Intermal routines
 */
static int
regmatch(regexchar * re, char *str, int len, int firstp, char **lastpos)
{
    char *p = str, *ep = str + len;
    char *lpos, *llpos = NULL;
    longchar k;

    *lastpos = NULL;
#ifdef REGEX_DEBUG
    debugre(re, str);
#endif				/* REGEX_DEBUG */
    while ((re->mode & RE_ENDMARK) == 0) {
	if (re->mode & RE_BEGIN) {
	    if (!firstp)
		return 0;
	    re++;
	}
	else if (re->mode & RE_ANYTIME) {
           short matched, ok = 0;
           for (;;) {
        matched = 0;
		if (regmatch(re + 1, p, ep - p, firstp, &lpos) == 1) {
		    llpos = lpos;
		    matched = 1;
		    ok = 1;
		}
        if (p >= ep)
		    break;
#ifdef JP_CHARSET
		if (IS_KANJI1(*p)) {
		    k = RE_KANJI(p);
		    if (regmatch1(re, k)) {
			if (lastpos != NULL)
			    *lastpos = llpos;
			p += 2;
		    }
		    else
			break;
		}
		else
#endif
		{
		    k = (unsigned char)*p;
		    if (regmatch1(re, k)) {
			p++;
			if (lastpos != NULL)
			    *lastpos = llpos;
		    }
		    else
              break;
		}
           } 
	    if (lastpos != NULL)
		*lastpos = llpos;
	    return ok;
	}
	else if (re->mode & RE_END) {
	    if (lastpos != NULL)
		*lastpos = p;
	    return (p >= ep);
	}
	else {
	    int a;
#ifdef JP_CHARSET
	    if (IS_KANJI1(*p)) {
		k = RE_KANJI(p);
		p += 2;
		a = regmatch1(re, k);
	    }
	    else 
#endif
	    {
		k = (unsigned char)*(p++);
		a = regmatch1(re, k);
	    }
	    if (!a)
		return 0;
	    else
		re++;
	}
    }
    if (lastpos != NULL)
	*lastpos = p;
    return 1;
}

static int
regmatch1(regexchar * re, longchar c)
{
    switch (re->mode & RE_MATCHMODE) {
    case RE_ANY:
#ifdef REGEX_DEBUG
	printf("%c vs any. -> 1\n", c);
#endif				/* REGEX_DEBUG */
	return 1;
    case RE_NORMAL:
#ifdef REGEX_DEBUG
	printf("RE=%c vs %c -> %d\n", *re->pattern, c, *re->pattern == c);
#endif				/* REGEX_DEBUG */
	if (re->mode & RE_IGNCASE) {
	    if (*re->pattern < 127 && c < 127 &&
		IS_ALPHA(*re->pattern) && IS_ALPHA(c))
		return tolower(*re->pattern) == tolower(c);
	    else
		return *re->pattern == c;
	}
	else
	    return (*re->pattern == c);
    case RE_WHICH:
	return matchWhich(re->pattern, c);
    case RE_EXCEPT:
	return !matchWhich(re->pattern, c);
    }
    return 0;
}

static int
matchWhich(longchar * pattern, longchar c)
{
    longchar *p = pattern;
    int ans = 0;

#ifdef REGEX_DEBUG
    printf("RE pattern = %s char=%c", pattern, c);
#endif				/* REGEX_DEBUG */
    while (*p != '\0') {
	if (*(p + 1) == RE_WHICH_RANGE && *(p + 2) != '\0') {	/* Char  * 
								 * 
								 * *  * *
								 * * * * * 
								 * * * * 
								 * * *  *
								 * *  * *
								 * *  * *
								 * * *  *
								 * * *  *
								 * * * * * 
								 * * * * * 
								 * * *
								 * class. 
								 * * * *  * 
								 * * * * *
								 * *  * * * 
								 */
	    if (*p <= c && c <= *(p + 2)) {
		ans = 1;
		break;
	    }
	    p += 3;
	}
	else {
	    if (*p == c) {
		ans = 1;
		break;
	    }
	    p++;
	}
    }
#ifdef REGEX_DEBUG
    printf(" -> %d\n", ans);
#endif				/* REGEX_DEBUG */
    return ans;
}

#ifdef REGEX_DEBUG
char *
lc2c(longchar * x)
{
    static char y[100];
    int i = 0;

    while (x[i]) {
	if (x[i] == RE_WHICH_RANGE)
	    y[i] = '-';
	else
	    y[i] = x[i];
	i++;
    }
    y[i] = '\0';
    return y;
}

void
debugre(re, s)
    regexchar *re;
    char *s;
{
    for (; !(re->mode & RE_ENDMARK); re++) {
	if (re->mode & RE_BEGIN) {
	    printf("Begin ");
	    continue;
	}
	else if (re->mode & RE_END) {
	    printf("End ");
	    continue;
	}
	if (re->mode & RE_ANYTIME)
	    printf("Anytime-");

	switch (re->mode & RE_MATCHMODE) {
	case RE_ANY:
	    printf("Any ");
	    break;
	case RE_NORMAL:
	    printf("Match-to'%c' ", *re->pattern);
	    break;
	case RE_WHICH:
	    printf("One-of\"%s\" ", lc2c(re->pattern));
	    break;
	case RE_EXCEPT:
	    printf("Other-than\"%s\" ", lc2c(re->pattern));
	    break;
	default:
	    printf("Unknown ");
	}
    }
    putchar('\n');
}

#endif				/* REGEX_DEBUG */
