/* $Id: regex.c,v 1.7 2002/01/10 04:55:07 ukai Exp $ */
/* 
 * regex: Regular expression pattern match library
 * 
 * by A.ITO, December 1989
 * Revised by A.ITO, January 2002
 */

#ifdef REGEX_DEBUG
#include <sys/types.h>
#include <malloc.h>
#endif				/* REGEX_DEBUG */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gc.h>
#include "regex.h"
#include "config.h"

#ifndef NULL
#define NULL	0
#endif				/* not NULL */

#if LANG == JA
#define JP_CHARSET
#endif

#define RE_ITER_LIMIT   65535

#define RE_MATCHMODE	0x07
#define	RE_NORMAL	0x00
#define RE_ANY		0x01
#define RE_WHICH	0x02
#define RE_EXCEPT	0x03
#define RE_SUBREGEX     0x04
#define RE_BEGIN	0x05
#define RE_END		0x06
#define RE_ENDMARK	0x07

#define RE_OPT          0x08
#define RE_ANYTIME	0x10
#define RE_IGNCASE      0x40

#define RE_MODE(x)      ((x)->mode&RE_MATCHMODE)
#define RE_SET_MODE(x,v) ((x)->mode = (((x)->mode&~RE_MATCHMODE)|((v)&RE_MATCHMODE)))

#ifdef REGEX_DEBUG
void debugre(regexchar *);
char *lc2c(longchar *, int);
int verbose;
#endif				/* REGEX_DEBUG */

#ifndef IS_ALPHA
#define IS_ALPHA(x) (!((x)&0x80) && isalpha(x))
#define IS_KANJI1(x) ((x)&0x80)
#endif

#ifdef JP_CHARSET
#define RE_KANJI(p)	(((unsigned char)*(p) << 8) | (unsigned char)*((p)+1))
#endif

#define RE_WHICH_RANGE	0xffff

static Regex DefaultRegex;
#define CompiledRegex DefaultRegex.re
#define Cstorage DefaultRegex.storage

static int regmatch(regexchar *, char *, int, char **);
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

static Regex *
newRegex0(char **ex, int igncase, Regex *regex, char **msg, int level)
{
    char *p;
    longchar *r;
    regexchar *re;
    int m;
    longchar *st_ptr;

    if (regex == NULL)
	regex = (Regex *)GC_malloc(sizeof(Regex));
    regex->alt_regex = NULL;
    re = regex->re;
    st_ptr = regex->storage;
    for (p = *ex; *p != '\0'; p++) {
	re->mode = 0;
	switch (*p) {
	case '.':
	    re->p.pattern = NULL;
	    RE_SET_MODE(re, RE_ANY);
	    re++;
	    break;
	case '$':
	    re->p.pattern = NULL;
	    RE_SET_MODE(re, RE_END);
	    re++;
	    break;
	case '^':
	    re->p.pattern = NULL;
	    RE_SET_MODE(re, RE_BEGIN);
	    re++;
	    break;
	case '+':
	    if (re == regex->re ||
		(RE_MODE(re - 1) != RE_ANY && (re - 1)->p.pattern == NULL)) {
		if (msg)
		    *msg = "Invalid regular expression";
		return NULL;
	    }
	    *re = *(re - 1);
	    re->mode |= RE_ANYTIME;
	    re++;
	    break;
	case '*':
	    if (re == regex->re ||
		(RE_MODE(re - 1) != RE_ANY && (re - 1)->p.pattern == NULL)) {
		if (msg)
		    *msg = "Invalid regular expression";
		return NULL;
	    }
	    (re - 1)->mode |= RE_ANYTIME;
	    break;
	case '?':
	    if (re == regex->re ||
		(RE_MODE(re - 1) != RE_ANY && (re - 1)->p.pattern == NULL)) {
		if (msg)
		    *msg = "Invalid regular expression";
		return NULL;
	    }
	    (re - 1)->mode |= RE_OPT;
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
	    re->p.pattern = r;
	    RE_SET_MODE(re, m);
	    re++;
	    break;
	case '|':
	    RE_SET_MODE(re, RE_ENDMARK);
	    re++;
	    p++;
	    regex->alt_regex = newRegex0(&p, igncase, NULL, msg, level);
	    if (regex->alt_regex == NULL)
		return NULL;
	    *ex = p;
	    return regex;
	case '(':
	    RE_SET_MODE(re, RE_SUBREGEX);
	    p++;
	    re->p.sub = newRegex0(&p, igncase, NULL, msg, level + 1);
	    if (re->p.sub == NULL)
		return NULL;
	    re++;
	    break;
	case ')':
	    if (level == 0) {
		if (msg)
		    *msg = "Too many ')'";
		return NULL;
	    }
	    RE_SET_MODE(re, RE_ENDMARK);
	    re++;
	    *ex = p;
	    return regex;
	case '\\':
	    p++;
	default:
#ifdef JP_CHARSET
	    if (IS_KANJI1(*p)) {
		*(st_ptr) = RE_KANJI(p);
		p++;
	    }
	    else
#endif
		*st_ptr = (unsigned char)*p;
	    re->p.pattern = st_ptr;
	    st_ptr++;
	    RE_SET_MODE(re, RE_NORMAL);
	    if (igncase)
		re->mode |= RE_IGNCASE;
	    re++;
	}
	if (st_ptr >= &regex->storage[STORAGE_MAX] ||
	    re >= &regex->re[REGEX_MAX]) {
	    if (msg)
		*msg = "Regular expression too long";
	    return NULL;
	}
    }
    RE_SET_MODE(re, RE_ENDMARK);
    if (msg)
	*msg = NULL;
    *ex = p;
    return regex;
}

Regex *
newRegex(char *ex, int igncase, Regex *regex, char **msg)
{
    return newRegex0(&ex, igncase, regex, msg, 0);
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
RegexMatch(Regex *re, char *str, int len, int firstp)
{
    char *p, *ep;
    char *lpos;
    Regex *r;

    if (str == NULL)
	return 0;
    if (len == 0)
	len = strlen(str);
    re->position = NULL;
    ep = str + len;
    for (p = str; p < ep; p++) {
	lpos = NULL;
	re->lposition = NULL;
	for (r = re; r != NULL; r = r->alt_regex) {
	    switch (regmatch(r->re, p, firstp && (p == str), &lpos)) {
	    case 1:		/* matched */
		re->position = p;
		if (re->lposition == NULL || re->lposition < lpos)
		    re->lposition = lpos;
		break;
	    case -1:		/* error */
		re->position = NULL;
		return -1;
	    }
	}
	if (re->lposition != NULL) {
	    /* matched */
	    return 1;
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
MatchedPosition(Regex *re, char **first, char **last)
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

struct MatchingContext1 {
    int label;
    regexchar *re;
    char *lastpos;
    char *str;
    int iter_limit;
    int n_any;
    int firstp;
    char *end_p;
    Regex *sub_regex;
    struct MatchingContext1 *sub_ctx;
    struct MatchingContext2 *ctx2;
};

struct MatchingContext2 {
    int label;
    Regex *regex;
    char *lastpos;
    struct MatchingContext1 *ctx;
    struct MatchingContext2 *ctx2;
    char *str;
    int n_any;
    int firstp;
};


#define YIELD(retval,context,lnum) (context)->label = lnum; return (retval); label##lnum:

static int regmatch_iter(struct MatchingContext1 *, regexchar *, char *, int);

static int
regmatch_sub_anytime(struct MatchingContext2 *c, Regex *regex,
		     regexchar * pat2, char *str, int iter_limit, int firstp)
{
    switch (c->label) {
    case 1:
	goto label1;
    case 2:
	goto label2;
    case 3:
	goto label3;
    }
    c->ctx = GC_malloc(sizeof(struct MatchingContext1));
    c->ctx2 = GC_malloc(sizeof(struct MatchingContext2));
    c->ctx->label = 0;
    c->regex = regex;
    c->n_any = 0;
    c->str = str;
    c->firstp = firstp;
    for (;;) {
	c->ctx->label = 0;
	while (regmatch_iter(c->ctx, c->regex->re, c->str, c->firstp)) {
	    c->n_any = c->ctx->lastpos - c->str;
	    if (c->n_any <= 0)
		continue;
	    c->firstp = 0;
	    if (RE_MODE(pat2) == RE_ENDMARK) {
		c->lastpos = c->str + c->n_any;
		YIELD(1, c, 1);
	    }
	    else if (regmatch(pat2, c->str + c->n_any,
			      c->firstp, &c->lastpos) == 1) {
		YIELD(1, c, 2);
	    }
	    if (iter_limit == 1)
		continue;
	    c->ctx2->label = 0;
	    while (regmatch_sub_anytime(c->ctx2, regex, pat2,
					c->str + c->n_any, iter_limit - 1,
					c->firstp)) {

		c->lastpos = c->ctx2->lastpos;
		YIELD(1, c, 3);
	    }
	}
	if (c->regex->alt_regex == NULL)
	    break;
	c->regex = c->regex->alt_regex;
    }
    return 0;
}

static int
regmatch_iter(struct MatchingContext1 *c,
	      regexchar * re, char *str, int firstp)
{
    switch (c->label) {
    case 1:
	goto label1;
    case 2:
	goto label2;
    case 3:
	goto label3;
    case 4:
	goto label4;
    case 5:
	goto label5;
    case 6:
	goto label6;
    case 7:
	goto label7;
    }
    if (RE_MODE(re) == RE_ENDMARK)
	return 0;
    c->re = re;
    c->end_p = str + strlen(str);
    c->firstp = firstp;
    c->str = str;
    c->sub_ctx = NULL;
    while (RE_MODE(c->re) != RE_ENDMARK) {
	if (c->re->mode & (RE_ANYTIME | RE_OPT)) {
	    if (c->re->mode & RE_ANYTIME)
		c->iter_limit = RE_ITER_LIMIT;
	    else
		c->iter_limit = 1;
	    c->n_any = -1;
	    while (c->n_any < c->iter_limit) {
		if (c->str + c->n_any >= c->end_p) {
		    return 0;
		}
		if (c->n_any >= 0) {
		    if (RE_MODE(c->re) == RE_SUBREGEX) {
			c->ctx2 = GC_malloc(sizeof(struct MatchingContext2));
			c->ctx2->label = 0;
			while (regmatch_sub_anytime(c->ctx2,
						    c->re->p.sub,
						    c->re + 1,
						    c->str + c->n_any,
						    c->iter_limit,
						    c->firstp)) {
			    c->n_any = c->ctx2->lastpos - c->str;
			    c->lastpos = c->ctx2->lastpos;
			    YIELD(1, c, 1);
			}
			return 0;
		    }
#ifdef JP_CHARSET
		    else if (IS_KANJI1(c->str[c->n_any])) {
			longchar k;
			k = RE_KANJI(c->str + c->n_any);
			if (regmatch1(c->re, k)) {
			    c->n_any += 2;
			}
			else {
			    return 0;
			}
			c->firstp = 0;
		    }
#endif
		    else {
			longchar k;
			k = (unsigned char)c->str[c->n_any];
			if (regmatch1(c->re, k)) {
			    c->n_any++;
			}
			else {
			    return 0;
			}
			c->firstp = 0;
		    }
		}
		else
		    c->n_any++;
		if (RE_MODE(c->re + 1) == RE_ENDMARK) {
		    c->lastpos = c->str + c->n_any;
		    YIELD(1, c, 2);
		}
		else if (regmatch(c->re + 1, c->str + c->n_any,
				  c->firstp, &c->lastpos) == 1) {
		    YIELD(1, c, 3);
		}
	    }
	    return 0;
	}
	/* regexp other than pat*, pat+ and pat? */
	if (c->str >= c->end_p)
	    return 0;
	switch (RE_MODE(c->re)) {
	case RE_BEGIN:
	    if (!c->firstp)
		return 0;
	    c->re++;
	    break;
	case RE_END:
	    c->lastpos = c->str;
	    c->re++;
	    YIELD((c->str >= c->end_p), c, 4);
	    break;
	case RE_SUBREGEX:
	    if (c->sub_ctx == NULL) {
		c->sub_ctx = GC_malloc(sizeof(struct MatchingContext1));
	    }
	    c->sub_regex = c->re->p.sub;
	    for (;;) {
		c->sub_ctx->label = 0;
		while (regmatch_iter(c->sub_ctx, c->sub_regex->re,
				     c->str, c->firstp)) {
		    if (c->sub_ctx->lastpos != c->str)
			c->firstp = 0;
		    if (RE_MODE(c->re + 1) == RE_ENDMARK) {
			c->lastpos = c->sub_ctx->lastpos;
			YIELD(1, c, 5);
		    }
		    else if (regmatch(c->re + 1, c->sub_ctx->lastpos,
				      c->firstp, &c->lastpos) == 1) {
			YIELD(1, c, 6);
		    }
		}
		if (c->sub_regex->alt_regex == NULL)
		    break;
		c->sub_regex = c->sub_regex->alt_regex;
	    }
	    return 0;
	default:
#ifdef JP_CHARSET
	    if (IS_KANJI1(*c->str)) {
		longchar k;
		k = RE_KANJI(c->str);
		c->str += 2;
		if (!regmatch1(c->re, k))
		    return 0;
	    }
	    else
#endif
	    {
		longchar k;
		k = (unsigned char)*(c->str++);
		if (!regmatch1(c->re, k))
		    return 0;
	    }
	    c->re++;
	    c->firstp = 0;
	}
    }
    c->lastpos = c->str;
#ifdef REGEX_DEBUG
    if (verbose)
	printf("Succeed: %s %d\n", c->str, c->lastpos - c->str);
#endif
    YIELD(1, c, 7);
    return 0;
}

static int
regmatch(regexchar * re, char *str, int firstp, char **lastpos)
{
    struct MatchingContext1 contx;

    *lastpos = NULL;

    contx.label = 0;
    while (regmatch_iter(&contx, re, str, firstp)) {
#ifdef REGEX_DEBUG
	char *p;
	if (verbose) {
	    printf("regmatch: matched <");
	    for (p = str; p < contx.lastpos; p++)
		putchar(*p);
	    printf(">\n");
	}
#endif
	if (*lastpos == NULL || *lastpos < contx.lastpos)
	    *lastpos = contx.lastpos;
    }
    if (*lastpos == NULL)
	return 0;
    return 1;
}


static int
regmatch1(regexchar * re, longchar c)
{
    switch (RE_MODE(re)) {
    case RE_ANY:
#ifdef REGEX_DEBUG
	if (verbose)
	    printf("%c vs any. -> 1\n", c);
#endif				/* REGEX_DEBUG */
	return 1;
    case RE_NORMAL:
#ifdef REGEX_DEBUG
	if (verbose)
	    printf("RE=%c vs %c -> %d\n", *re->p.pattern, c,
		   *re->p.pattern == c);
#endif				/* REGEX_DEBUG */
	if (re->mode & RE_IGNCASE) {
	    if (*re->p.pattern < 127 && c < 127 &&
		IS_ALPHA(*re->p.pattern) && IS_ALPHA(c))
		return tolower(*re->p.pattern) == tolower(c);
	    else
		return *re->p.pattern == c;
	}
	else
	    return (*re->p.pattern == c);
    case RE_WHICH:
	return matchWhich(re->p.pattern, c);
    case RE_EXCEPT:
	return !matchWhich(re->p.pattern, c);
    }
    return 0;
}

static int
matchWhich(longchar * pattern, longchar c)
{
    longchar *p = pattern;
    int ans = 0;

#ifdef REGEX_DEBUG
    if (verbose)
	printf("RE pattern = %s char=%s", lc2c(pattern, 10000), lc2c(&c, 1));
#endif				/* REGEX_DEBUG */
    while (*p != '\0') {
	if (*(p + 1) == RE_WHICH_RANGE && *(p + 2) != '\0') {	/* Char class. */
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
    if (verbose)
	printf(" -> %d\n", ans);
#endif				/* REGEX_DEBUG */
    return ans;
}

#ifdef REGEX_DEBUG
char *
lc2c(longchar * x, int len)
{
    static char y[100];
    int i = 0;
    char *r;

    while (x[i] && i < len) {
	if (x[i] == RE_WHICH_RANGE)
	    y[i++] = '-';
	else if (x[i] >= 128) {
	    y[i++] = ((x[i] >> 8) & 0xff);
	    y[i++] = (x[i] & 0xff);
	}
	else
	    y[i++] = x[i];
    }
    y[i] = '\0';
    r = GC_malloc_atomic(i + 1);
    strcpy(r, y);
    return r;
}

void
debugre(regexchar * re)
{
    for (; RE_MODE(re) != RE_ENDMARK; re++) {
	switch (RE_MODE(re)) {
	case RE_BEGIN:
	    printf("Begin ");
	    continue;
	case RE_END:
	    printf("End ");
	    continue;
	}
	if (re->mode & RE_ANYTIME)
	    printf("Anytime-");
	if (re->mode & RE_OPT)
	    printf("Opt-");

	switch (RE_MODE(re)) {
	case RE_ANY:
	    printf("Any ");
	    break;
	case RE_NORMAL:
	    printf("Match-to'%c' ", *re->p.pattern);
	    break;
	case RE_WHICH:
	    printf("One-of\"%s\" ", lc2c(re->p.pattern, 10000));
	    break;
	case RE_EXCEPT:
	    printf("Other-than\"%s\" ", lc2c(re->p.pattern, 10000));
	    break;
	case RE_SUBREGEX:
	    {
		Regex *r = re->p.sub;
		printf("(");
		while (r) {
		    debugre(r->re);
		    if (r->alt_regex)
			printf(" | ");
		    r = r->alt_regex;
		}
		printf(")");
		break;
	    }
	default:
	    printf("Unknown ");
	}
    }
}

#endif				/* REGEX_DEBUG */

#ifdef REGEXTEST
int
main(int argc, char **argv)
{
    char buf[128], buf2[128];
    char *msg;
    Regex *re;
    char *fpos, *epos;
    FILE *f = stdin;
    int i = 1;

#ifdef REGEX_DEBUG
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-v") == 0)
	    verbose = 1;
	else
	    break;
    }
#endif

    if (argc > i)
	f = fopen(argv[i], "r");
    if (f == NULL) {
	fprintf(stderr, "Can't open %s\n", argv[i]);
	exit(1);
    }
    while (fscanf(f, "%s%s", buf, buf2) == 2) {
	re = newRegex(buf, 0, NULL, &msg);
	if (re == NULL) {
	    printf("Error on regexp /%s/: %s\n", buf, msg);
	    exit(1);
	}
	if (RegexMatch(re, buf2, 0, 1)) {
	    printf("/%s/\t%s\t", buf, buf2);
	    MatchedPosition(re, &fpos, &epos);
	    while (fpos < epos)
		putchar(*(fpos++));
	}
	else
	    printf("/%s/\t%s\tno_match", buf, buf2);
	putchar('\n');
    }
    /* notreatched */
    return 0;
}
#endif
