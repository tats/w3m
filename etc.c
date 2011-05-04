/* $Id: etc.c,v 1.81 2007/05/23 15:06:05 inu Exp $ */
#include "fm.h"
#ifndef __MINGW32_VERSION
#include <pwd.h>
#endif
#include "myctype.h"
#include "html.h"
#include "local.h"
#include "hash.h"

#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#if defined(HAVE_WAITPID) || defined(HAVE_WAIT3)
#include <sys/wait.h>
#endif
#include <signal.h>

#ifdef	__WATT32__
#define	read(a,b,c)	read_s(a,b,c)
#define	close(x)	close_s(x)
#endif				/* __WATT32__ */

struct auth_pass {
    int bad;
    int is_proxy;
    Str host;
    int port;
/*    Str file; */
    Str realm;
    Str uname;
    Str pwd;
    struct auth_pass *next;
};

struct auth_pass *passwords = NULL;

int
columnSkip(Buffer *buf, int offset)
{
    int i, maxColumn;
    int column = buf->currentColumn + offset;
    int nlines = buf->LINES + 1;
    Line *l;

    maxColumn = 0;
    for (i = 0, l = buf->topLine; i < nlines && l != NULL; i++, l = l->next) {
	if (l->width < 0)
	    l->width = COLPOS(l, l->len);
	if (l->width - 1 > maxColumn)
	    maxColumn = l->width - 1;
    }
    maxColumn -= buf->COLS - 1;
    if (column < maxColumn)
	maxColumn = column;
    if (maxColumn < 0)
	maxColumn = 0;

    if (buf->currentColumn == maxColumn)
	return 0;
    buf->currentColumn = maxColumn;
    return 1;
}

int
columnPos(Line *line, int column)
{
    int i;

    for (i = 1; i < line->len; i++) {
	if (COLPOS(line, i) > column)
	    break;
    }
#ifdef USE_M17N
    for (i--; i > 0 && line->propBuf[i] & PC_WCHAR2; i--) ;
    return i;
#else
    return i - 1;
#endif
}

Line *
lineSkip(Buffer *buf, Line *line, int offset, int last)
{
    int i;
    Line *l;

    l = currentLineSkip(buf, line, offset, last);
    if (!nextpage_topline)
	for (i = buf->LINES - 1 - (buf->lastLine->linenumber - l->linenumber);
	     i > 0 && l->prev != NULL; i--, l = l->prev) ;
    return l;
}

Line *
currentLineSkip(Buffer *buf, Line *line, int offset, int last)
{
    int i, n;
    Line *l = line;

    if (buf->pagerSource && !(buf->bufferprop & BP_CLOSE)) {
	n = line->linenumber + offset + buf->LINES;
	if (buf->lastLine->linenumber < n)
	    getNextPage(buf, n - buf->lastLine->linenumber);
	while ((last || (buf->lastLine->linenumber < n)) &&
	       (getNextPage(buf, 1) != NULL)) ;
	if (last)
	    l = buf->lastLine;
    }

    if (offset == 0)
	return l;
    if (offset > 0)
	for (i = 0; i < offset && l->next != NULL; i++, l = l->next) ;
    else
	for (i = 0; i < -offset && l->prev != NULL; i++, l = l->prev) ;
    return l;
}

#define MAX_CMD_LEN 128

int
gethtmlcmd(char **s)
{
    extern Hash_si tagtable;
    char cmdstr[MAX_CMD_LEN];
    char *p = cmdstr;
    char *save = *s;
    int cmd;

    (*s)++;
    /* first character */
    if (IS_ALNUM(**s) || **s == '_' || **s == '/') {
	*(p++) = TOLOWER(**s);
	(*s)++;
    }
    else
	return HTML_UNKNOWN;
    if (p[-1] == '/')
	SKIP_BLANKS(*s);
    while ((IS_ALNUM(**s) || **s == '_') && p - cmdstr < MAX_CMD_LEN) {
	*(p++) = TOLOWER(**s);
	(*s)++;
    }
    if (p - cmdstr == MAX_CMD_LEN) {
	/* buffer overflow: perhaps caused by bad HTML source */
	*s = save + 1;
	return HTML_UNKNOWN;
    }
    *p = '\0';

    /* hash search */
    cmd = getHash_si(&tagtable, cmdstr, HTML_UNKNOWN);
    while (**s && **s != '>')
	(*s)++;
    if (**s == '>')
	(*s)++;
    return cmd;
}

#ifdef USE_ANSI_COLOR
static int
parse_ansi_color(char **str, Lineprop *effect, Linecolor *color)
{
    char *p = *str, *q;
    Lineprop e = *effect;
    Linecolor c = *color;
    int i;

    if (*p != ESC_CODE || *(p + 1) != '[')
	return 0;
    p += 2;
    for (q = p; IS_DIGIT(*q) || *q == ';'; q++) ;
    if (*q != 'm')
	return 0;
    *str = q + 1;
    while (1) {
	if (*p == 'm') {
	    e = PE_NORMAL;
	    c = 0;
	    break;
	}
	if (IS_DIGIT(*p)) {
	    q = p;
	    for (p++; IS_DIGIT(*p); p++) ;
	    i = atoi(allocStr(q, p - q));
	    switch (i) {
	    case 0:
		e = PE_NORMAL;
		c = 0;
		break;
	    case 1:
	    case 5:
		e = PE_BOLD;
		break;
	    case 4:
		e = PE_UNDER;
		break;
	    case 7:
		e = PE_STAND;
		break;
	    case 100:		/* for EWS4800 kterm */
		c = 0;
		break;
	    case 39:
		c &= 0xf0;
		break;
	    case 49:
		c &= 0x0f;
		break;
	    default:
		if (i >= 30 && i <= 37)
		    c = (c & 0xf0) | (i - 30) | 0x08;
		else if (i >= 40 && i <= 47)
		    c = (c & 0x0f) | ((i - 40) << 4) | 0x80;
		break;
	    }
	    if (*p == 'm')
		break;
	}
	else {
	    e = PE_NORMAL;
	    c = 0;
	    break;
	}
	p++;			/* *p == ';' */
    }
    *effect = e;
    *color = c;
    return 1;
}
#endif
/* 
 * Check character type
 */

Str
checkType(Str s, Lineprop **oprop, Linecolor **ocolor)
{
    Lineprop mode;
    Lineprop effect = PE_NORMAL;
    Lineprop *prop;
    static Lineprop *prop_buffer = NULL;
    static int prop_size = 0;
    char *str = s->ptr, *endp = &s->ptr[s->length], *bs = NULL;
#ifdef USE_ANSI_COLOR
    Lineprop ceffect = PE_NORMAL;
    Linecolor cmode = 0;
    int check_color = FALSE;
    Linecolor *color = NULL;
    static Linecolor *color_buffer = NULL;
    static int color_size = 0;
    char *es = NULL;
#endif
    int do_copy = FALSE;
    int i;
    int plen = 0, clen;

    if (prop_size < s->length) {
	prop_size = (s->length > LINELEN) ? s->length : LINELEN;
	prop_buffer = New_Reuse(Lineprop, prop_buffer, prop_size);
    }
    prop = prop_buffer;

    if (ShowEffect) {
	bs = memchr(str, '\b', s->length);
#ifdef USE_ANSI_COLOR
	if (ocolor) {
	    es = memchr(str, ESC_CODE, s->length);
	    if (es) {
		if (color_size < s->length) {
		    color_size = (s->length > LINELEN) ? s->length : LINELEN;
		    color_buffer = New_Reuse(Linecolor, color_buffer,
					     color_size);
		}
		color = color_buffer;
	    }
	}
#endif
	if ((bs != NULL)
#ifdef USE_ANSI_COLOR
	    || (es != NULL)
#endif
	    ) {
	    char *sp = str, *ep;
	    s = Strnew_size(s->length);
	    do_copy = TRUE;
	    ep = bs ? (bs - 2) : endp;
#ifdef USE_ANSI_COLOR
	    if (es && ep > es - 2)
		ep = es - 2;
#endif
	    for (; str < ep && IS_ASCII(*str); str++) {
		*(prop++) = PE_NORMAL | (IS_CNTRL(*str) ? PC_CTRL : PC_ASCII);
#ifdef USE_ANSI_COLOR
		if (color)
		    *(color++) = 0;
#endif
	    }
	    Strcat_charp_n(s, sp, (int)(str - sp));
	}
    }
    if (!do_copy) {
	for (; str < endp && IS_ASCII(*str); str++)
	    *(prop++) = PE_NORMAL | (IS_CNTRL(*str) ? PC_CTRL : PC_ASCII);
    }

    while (str < endp) {
	if (prop - prop_buffer >= prop_size)
	    break;
	if (bs != NULL) {
#ifdef USE_M17N
	    if (str == bs - 2 && !strncmp(str, "__\b\b", 4)) {
		str += 4;
		effect = PE_UNDER;
		if (str < endp)
		    bs = memchr(str, '\b', endp - str);
		continue;
	    }
	    else
#endif
	    if (str == bs - 1 && *str == '_') {
		str += 2;
		effect = PE_UNDER;
		if (str < endp)
		    bs = memchr(str, '\b', endp - str);
		continue;
	    }
	    else if (str == bs) {
		if (*(str + 1) == '_') {
		    if (s->length) {
			str += 2;
#ifdef USE_M17N
			for (i = 1; i <= plen; i++)
			    *(prop - i) |= PE_UNDER;
#else
			*(prop - 1) |= PE_UNDER;
#endif
		    }
		    else {
			str++;
		    }
		}
#ifdef USE_M17N
		else if (!strncmp(str + 1, "\b__", 3)) {
		    if (s->length) {
			str += (plen == 1) ? 3 : 4;
			for (i = 1; i <= plen; i++)
			    *(prop - i) |= PE_UNDER;
		    }
		    else {
			str += 2;
		    }
		}
		else if (*(str + 1) == '\b') {
		    if (s->length) {
			clen = get_mclen(str + 2);
			if (plen == clen &&
			    !strncmp(str - plen, str + 2, plen)) {
			    for (i = 1; i <= plen; i++)
				*(prop - i) |= PE_BOLD;
			    str += 2 + clen;
			}
			else {
			    Strshrink(s, plen);
			    prop -= plen;
			    str += 2;
			}
		    }
		    else {
			str += 2;
		    }
		}
#endif
		else {
		    if (s->length) {
#ifdef USE_M17N
			clen = get_mclen(str + 1);
			if (plen == clen &&
			    !strncmp(str - plen, str + 1, plen)) {
			    for (i = 1; i <= plen; i++)
				*(prop - i) |= PE_BOLD;
			    str += 1 + clen;
			}
			else {
			    Strshrink(s, plen);
			    prop -= plen;
			    str++;
			}
#else
			if (*(str - 1) == *(str + 1)) {
			    *(prop - 1) |= PE_BOLD;
			    str += 2;
			}
			else {
			    Strshrink(s, 1);
			    prop--;
			    str++;
			}
#endif
		    }
		    else {
			str++;
		    }
		}
		if (str < endp)
		    bs = memchr(str, '\b', endp - str);
		continue;
	    }
#ifdef USE_ANSI_COLOR
	    else if (str > bs)
		bs = memchr(str, '\b', endp - str);
#endif
	}
#ifdef USE_ANSI_COLOR
	if (es != NULL) {
	    if (str == es) {
		int ok = parse_ansi_color(&str, &ceffect, &cmode);
		if (str < endp)
		    es = memchr(str, ESC_CODE, endp - str);
		if (ok) {
		    if (cmode)
			check_color = TRUE;
		    continue;
		}
	    }
	    else if (str > es)
		es = memchr(str, ESC_CODE, endp - str);
	}
#endif

	plen = get_mclen(str);
	mode = get_mctype(str) | effect;
#ifdef USE_ANSI_COLOR
	if (color) {
	    *(color++) = cmode;
	    mode |= ceffect;
	}
#endif
	*(prop++) = mode;
#ifdef USE_M17N
	if (plen > 1) {
	    mode = (mode & ~PC_WCHAR1) | PC_WCHAR2;
	    for (i = 1; i < plen; i++) {
		*(prop++) = mode;
#ifdef USE_ANSI_COLOR
		if (color)
		    *(color++) = cmode;
#endif
	    }
	    if (do_copy)
		Strcat_charp_n(s, (char *)str, plen);
	    str += plen;
	}
	else
#endif
	{
	    if (do_copy)
		Strcat_char(s, (char)*str);
	    str++;
	}
	effect = PE_NORMAL;
    }
    *oprop = prop_buffer;
#ifdef USE_ANSI_COLOR
    if (ocolor)
	*ocolor = check_color ? color_buffer : NULL;
#endif
    return s;
}

static int
nextColumn(int n, char *p, Lineprop *pr)
{
    if (*pr & PC_CTRL) {
	if (*p == '\t')
	    return (n + Tabstop) / Tabstop * Tabstop;
	else if (*p == '\n')
	    return n + 1;
	else if (*p != '\r')
	    return n + 2;
	return n;
    }
#ifdef USE_M17N
    if (*pr & PC_UNKNOWN)
	return n + 4;
    return n + wtf_width((wc_uchar *) p);
#else
    return n + 1;
#endif
}

int
calcPosition(char *l, Lineprop *pr, int len, int pos, int bpos, int mode)
{
    static int *realColumn = NULL;
    static int size = 0;
    static char *prevl = NULL;
    int i, j;

    if (l == NULL || len == 0)
	return bpos;
    if (l == prevl && mode == CP_AUTO) {
	if (pos <= len)
	    return realColumn[pos];
    }
    if (size < len + 1) {
	size = (len + 1 > LINELEN) ? (len + 1) : LINELEN;
	realColumn = New_N(int, size);
    }
    prevl = l;
    i = 0;
    j = bpos;
#ifdef USE_M17N
    if (pr[i] & PC_WCHAR2) {
	for (; i < len && pr[i] & PC_WCHAR2; i++)
	    realColumn[i] = j;
	if (i > 0 && pr[i - 1] & PC_KANJI && WcOption.use_wide)
	    j++;
    }
#endif
    while (1) {
	realColumn[i] = j;
	if (i == len)
	    break;
	j = nextColumn(j, &l[i], &pr[i]);
	i++;
#ifdef USE_M17N
	for (; i < len && pr[i] & PC_WCHAR2; i++)
	    realColumn[i] = realColumn[i - 1];
#endif
    }
    if (pos >= i)
	return j;
    return realColumn[pos];
}

int
columnLen(Line *line, int column)
{
    int i, j;

    for (i = 0, j = 0; i < line->len;) {
	j = nextColumn(j, &line->lineBuf[i], &line->propBuf[i]);
	if (j > column)
	    return i;
	i++;
#ifdef USE_M17N
	while (i < line->len && line->propBuf[i] & PC_WCHAR2)
	    i++;
#endif
    }
    return line->len;
}

char *
lastFileName(char *path)
{
    char *p, *q;

    p = q = path;
    while (*p != '\0') {
	if (*p == '/')
	    q = p + 1;
	p++;
    }

    return allocStr(q, -1);
}

#ifdef USE_INCLUDED_SRAND48
static unsigned long R1 = 0x1234abcd;
static unsigned long R2 = 0x330e;
#define A1 0x5deec
#define A2 0xe66d
#define C 0xb

void
srand48(long seed)
{
    R1 = (unsigned long)seed;
    R2 = 0x330e;
}

long
lrand48(void)
{
    R1 = (A1 * R1 << 16) + A1 * R2 + A2 * R1 + ((A2 * R2 + C) >> 16);
    R2 = (A2 * R2 + C) & 0xffff;
    return (long)(R1 >> 1);
}
#endif

char *
mybasename(char *s)
{
    char *p = s;
    while (*p)
	p++;
    while (s <= p && *p != '/')
	p--;
    if (*p == '/')
	p++;
    else
	p = s;
    return allocStr(p, -1);
}

char *
mydirname(char *s)
{
    char *p = s;
    while (*p)
	p++;
    if (s != p)
	p--;
    while (s != p && *p == '/')
	p--;
    while (s != p && *p != '/')
	p--;
    if (*p != '/')
	return ".";
    while (s != p && *p == '/')
	p--;
    return allocStr(s, strlen(s) - strlen(p) + 1);
}

#ifndef HAVE_STRERROR
char *
strerror(int errno)
{
    extern char *sys_errlist[];
    return sys_errlist[errno];
}
#endif				/* not HAVE_STRERROR */

#ifndef HAVE_SYS_ERRLIST
char **sys_errlist;

prepare_sys_errlist()
{
    int i, n;

    i = 1;
    while (strerror(i) != NULL)
	i++;
    n = i;
    sys_errlist = New_N(char *, n);
    sys_errlist[0] = "";
    for (i = 1; i < n; i++)
	sys_errlist[i] = strerror(i);
}
#endif				/* not HAVE_SYS_ERRLIST */

int
next_status(char c, int *status)
{
    switch (*status) {
    case R_ST_NORMAL:
	if (c == '<') {
	    *status = R_ST_TAG0;
	    return 0;
	}
	else if (c == '&') {
	    *status = R_ST_AMP;
	    return 1;
	}
	else
	    return 1;
	break;
    case R_ST_TAG0:
	if (c == '!') {
	    *status = R_ST_CMNT1;
	    return 0;
	}
	*status = R_ST_TAG;
	/* continues to next case */
    case R_ST_TAG:
	if (c == '>')
	    *status = R_ST_NORMAL;
	else if (c == '=')
	    *status = R_ST_EQL;
	return 0;
    case R_ST_EQL:
	if (c == '"')
	    *status = R_ST_DQUOTE;
	else if (c == '\'')
	    *status = R_ST_QUOTE;
	else if (IS_SPACE(c))
	    *status = R_ST_EQL;
	else if (c == '>')
	    *status = R_ST_NORMAL;
	else
	    *status = R_ST_VALUE;
	return 0;
    case R_ST_QUOTE:
	if (c == '\'')
	    *status = R_ST_TAG;
	return 0;
    case R_ST_DQUOTE:
	if (c == '"')
	    *status = R_ST_TAG;
	return 0;
    case R_ST_VALUE:
	if (c == '>')
	    *status = R_ST_NORMAL;
	else if (IS_SPACE(c))
	    *status = R_ST_TAG;
	return 0;
    case R_ST_AMP:
	if (c == ';') {
	    *status = R_ST_NORMAL;
	    return 0;
	}
	else if (c != '#' && !IS_ALNUM(c) && c != '_') {
	    /* something's wrong! */
	    *status = R_ST_NORMAL;
	    return 0;
	}
	else
	    return 0;
    case R_ST_CMNT1:
	switch (c) {
	case '-':
	    *status = R_ST_CMNT2;
	    break;
	case '>':
	    *status = R_ST_NORMAL;
	    break;
	default:
	    *status = R_ST_IRRTAG;
	}
	return 0;
    case R_ST_CMNT2:
	switch (c) {
	case '-':
	    *status = R_ST_CMNT;
	    break;
	case '>':
	    *status = R_ST_NORMAL;
	    break;
	default:
	    *status = R_ST_IRRTAG;
	}
	return 0;
    case R_ST_CMNT:
	if (c == '-')
	    *status = R_ST_NCMNT1;
	return 0;
    case R_ST_NCMNT1:
	if (c == '-')
	    *status = R_ST_NCMNT2;
	else
	    *status = R_ST_CMNT;
	return 0;
    case R_ST_NCMNT2:
	switch (c) {
	case '>':
	    *status = R_ST_NORMAL;
	    break;
	case '-':
	    *status = R_ST_NCMNT2;
	    break;
	default:
	    if (IS_SPACE(c))
		*status = R_ST_NCMNT3;
	    else
		*status = R_ST_CMNT;
	    break;
	}
	break;
    case R_ST_NCMNT3:
	switch (c) {
	case '>':
	    *status = R_ST_NORMAL;
	    break;
	case '-':
	    *status = R_ST_NCMNT1;
	    break;
	default:
	    if (IS_SPACE(c))
		*status = R_ST_NCMNT3;
	    else
		*status = R_ST_CMNT;
	    break;
	}
	return 0;
    case R_ST_IRRTAG:
	if (c == '>')
	    *status = R_ST_NORMAL;
	return 0;
    }
    /* notreached */
    return 0;
}

int
read_token(Str buf, char **instr, int *status, int pre, int append)
{
    char *p;
    int prev_status;

    if (!append)
	Strclear(buf);
    if (**instr == '\0')
	return 0;
    for (p = *instr; *p; p++) {
	prev_status = *status;
	next_status(*p, status);
	switch (*status) {
	case R_ST_NORMAL:
	    if (prev_status == R_ST_AMP && *p != ';') {
		p--;
		break;
	    }
	    if (prev_status == R_ST_NCMNT2 || prev_status == R_ST_NCMNT3 ||
		prev_status == R_ST_IRRTAG || prev_status == R_ST_CMNT1) {
		if (prev_status == R_ST_CMNT1 && !append && !pre)
		    Strclear(buf);
		if (pre)
		    Strcat_char(buf, *p);
		p++;
		goto proc_end;
	    }
	    Strcat_char(buf, (!pre && IS_SPACE(*p)) ? ' ' : *p);
	    if (ST_IS_REAL_TAG(prev_status)) {
		*instr = p + 1;
		if (buf->length < 2 ||
		    buf->ptr[buf->length - 2] != '<' ||
		    buf->ptr[buf->length - 1] != '>')
		    return 1;
		Strshrink(buf, 2);
	    }
	    break;
	case R_ST_TAG0:
	case R_ST_TAG:
	    if (prev_status == R_ST_NORMAL && p != *instr) {
		*instr = p;
		*status = prev_status;
		return 1;
	    }
	    if (*status == R_ST_TAG0 && !REALLY_THE_BEGINNING_OF_A_TAG(p)) {
		/* it seems that this '<' is not a beginning of a tag */
		/*
		 * Strcat_charp(buf, "&lt;");
		 */
		Strcat_char(buf, '<');
		*status = R_ST_NORMAL;
	    }
	    else
		Strcat_char(buf, *p);
	    break;
	case R_ST_EQL:
	case R_ST_QUOTE:
	case R_ST_DQUOTE:
	case R_ST_VALUE:
	case R_ST_AMP:
	    Strcat_char(buf, *p);
	    break;
	case R_ST_CMNT:
	case R_ST_IRRTAG:
	    if (pre)
		Strcat_char(buf, *p);
	    else if (!append)
		Strclear(buf);
	    break;
	case R_ST_CMNT1:
	case R_ST_CMNT2:
	case R_ST_NCMNT1:
	case R_ST_NCMNT2:
	case R_ST_NCMNT3:
	    /* do nothing */
	    if (pre)
		Strcat_char(buf, *p);
	    break;
	}
    }
  proc_end:
    *instr = p;
    return 1;
}

Str
correct_irrtag(int status)
{
    char c;
    Str tmp = Strnew();

    while (status != R_ST_NORMAL) {
	switch (status) {
	case R_ST_CMNT:	/* required "-->" */
	case R_ST_NCMNT1:	/* required "->" */
	    c = '-';
	    break;
	case R_ST_NCMNT2:
	case R_ST_NCMNT3:
	case R_ST_IRRTAG:
	case R_ST_CMNT1:
	case R_ST_CMNT2:
	case R_ST_TAG:
	case R_ST_TAG0:
	case R_ST_EQL:		/* required ">" */
	case R_ST_VALUE:
	    c = '>';
	    break;
	case R_ST_QUOTE:
	    c = '\'';
	    break;
	case R_ST_DQUOTE:
	    c = '"';
	    break;
	case R_ST_AMP:
	    c = ';';
	    break;
	default:
	    return tmp;
	}
	next_status(c, &status);
	Strcat_char(tmp, c);
    }
    return tmp;
}

/*
 * RFC2617: 1.2 Access Authentication Framework
 *
 * The realm value (case-sensitive), in combination with the canonical root
 * URL (the absoluteURI for the server whose abs_path is empty; see section
 * 5.1.2 of RFC2616 ) of the server being accessed, defines the protection
 * space. These realms allow the protected resources on a server to be
 * partitioned into a set of protection spaces, each with its own
 * authentication scheme and/or authorization database.
 *
 */
static void
add_auth_pass_entry(const struct auth_pass *ent, int netrc, int override)
{
    if ((ent->host || netrc)	/* netrc accept default (host == NULL) */
	&&(ent->is_proxy || ent->realm || netrc)
	&& ent->uname && ent->pwd) {
	struct auth_pass *newent = New(struct auth_pass);
	memcpy(newent, ent, sizeof(struct auth_pass));
	if (override) {
	    newent->next = passwords;
	    passwords = newent;
	} 
	else {
	    if (passwords == NULL)
		passwords = newent;
	    else if (passwords->next == NULL)
		passwords->next = newent;
	    else {
		struct auth_pass *ep = passwords;
		for (; ep->next; ep = ep->next) ;
		ep->next = newent;
	    }
	}
    }
    /* ignore invalid entries */
}

static struct auth_pass *
find_auth_pass_entry(char *host, int port, char *realm, char *uname, 
		     int is_proxy)
{
    struct auth_pass *ent;
    for (ent = passwords; ent != NULL; ent = ent->next) {
	if (ent->is_proxy == is_proxy
	    && (ent->bad != TRUE)
	    && (!ent->host || !Strcasecmp_charp(ent->host, host))
	    && (!ent->port || ent->port == port)
	    && (!ent->uname || !uname || !Strcmp_charp(ent->uname, uname))
	    && (!ent->realm || !realm || !Strcmp_charp(ent->realm, realm))
	    )
	    return ent;
    }
    return NULL;
}

int
find_auth_user_passwd(ParsedURL *pu, char *realm,
		      Str *uname, Str *pwd, int is_proxy)
{
    struct auth_pass *ent;

    if (pu->user && pu->pass) {
	*uname = Strnew_charp(pu->user);
	*pwd = Strnew_charp(pu->pass);
	return 1;
    }
    ent = find_auth_pass_entry(pu->host, pu->port, realm, pu->user, is_proxy);
    if (ent) {
	*uname = ent->uname;
	*pwd = ent->pwd;
	return 1;
    }
    return 0;
}

void
add_auth_user_passwd(ParsedURL *pu, char *realm, Str uname, Str pwd, 
		     int is_proxy)
{
    struct auth_pass ent;
    memset(&ent, 0, sizeof(ent));

    ent.is_proxy = is_proxy;
    ent.host = Strnew_charp(pu->host);
    ent.port = pu->port;
    ent.realm = Strnew_charp(realm);
    ent.uname = uname;
    ent.pwd = pwd;
    add_auth_pass_entry(&ent, 0, 1);
}

void
invalidate_auth_user_passwd(ParsedURL *pu, char *realm, Str uname, Str pwd, 
			    int is_proxy)
{
    struct auth_pass *ent;
    ent = find_auth_pass_entry(pu->host, pu->port, realm, NULL, is_proxy);
    if (ent) {
	ent->bad = TRUE;
    }
    return;
}

/* passwd */
/*
 * machine <host>
 * host <host>
 * port <port>
 * proxy
 * path <file>	; not used
 * realm <realm>
 * login <login>
 * passwd <passwd>
 * password <passwd>
 */

static Str
next_token(Str arg)
{
    Str narg = NULL;
    char *p, *q;
    if (arg == NULL || arg->length == 0)
	return NULL;
    p = arg->ptr;
    q = p;
    SKIP_NON_BLANKS(q);
    if (*q != '\0') {
	*q++ = '\0';
	SKIP_BLANKS(q);
	if (*q != '\0')
	    narg = Strnew_charp(q);
    }
    return narg;
}

static void
parsePasswd(FILE * fp, int netrc)
{
    struct auth_pass ent;
    Str line = NULL;

    bzero(&ent, sizeof(struct auth_pass));
    while (1) {
	Str arg = NULL;
	char *p;

	if (line == NULL || line->length == 0)
	    line = Strfgets(fp);
	if (line->length == 0)
	    break;
	Strchop(line);
	Strremovefirstspaces(line);
	p = line->ptr;
	if (*p == '#' || *p == '\0') {
	    line = NULL;
	    continue;		/* comment or empty line */
	}
	arg = next_token(line);

	if (!strcmp(p, "machine") || !strcmp(p, "host")
	    || (netrc && !strcmp(p, "default"))) {
	    add_auth_pass_entry(&ent, netrc, 0);
	    bzero(&ent, sizeof(struct auth_pass));
	    if (netrc)
		ent.port = 21;	/* XXX: getservbyname("ftp"); ? */
	    if (strcmp(p, "default") != 0) {
		line = next_token(arg);
		ent.host = arg;
	    }
	    else {
		line = arg;
	    }
	}
	else if (!netrc && !strcmp(p, "port") && arg) {
	    line = next_token(arg);
	    ent.port = atoi(arg->ptr);
	}
	else if (!netrc && !strcmp(p, "proxy")) {
	    ent.is_proxy = 1;
	    line = arg;
	}
	else if (!netrc && !strcmp(p, "path")) {
	    line = next_token(arg);
	    /* ent.file = arg; */
	}
	else if (!netrc && !strcmp(p, "realm")) {
	    /* XXX: rest of line becomes arg for realm */
	    line = NULL;
	    ent.realm = arg;
	}
	else if (!strcmp(p, "login")) {
	    line = next_token(arg);
	    ent.uname = arg;
	}
	else if (!strcmp(p, "password") || !strcmp(p, "passwd")) {
	    line = next_token(arg);
	    ent.pwd = arg;
	}
	else if (netrc && !strcmp(p, "machdef")) {
	    while ((line = Strfgets(fp))->length != 0) {
		if (*line->ptr == '\n')
		    break;
	    }
	    line = NULL;
	}
	else if (netrc && !strcmp(p, "account")) {
	    /* ignore */
	    line = next_token(arg);
	}
	else {
	    /* ignore rest of line */
	    line = NULL;
	}
    }
    add_auth_pass_entry(&ent, netrc, 0);
}

/* FIXME: gettextize? */
#define FILE_IS_READABLE_MSG "SECURITY NOTE: file %s must not be accessible by others"

FILE *
openSecretFile(char *fname)
{
    char *efname;
    struct stat st;

    if (fname == NULL)
	return NULL;
    efname = expandPath(fname);
    if (stat(efname, &st) < 0)
	return NULL;

    /* check permissions, if group or others readable or writable,
     * refuse it, because it's insecure.
     *
     * XXX: disable_secret_security_check will introduce some
     *    security issues, but on some platform such as Windows
     *    it's not possible (or feasible) to disable group|other
     *    readable and writable.
     *   [w3m-dev 03368][w3m-dev 03369][w3m-dev 03370]
     */
    if (disable_secret_security_check)
	/* do nothing */ ;
    else if ((st.st_mode & (S_IRWXG | S_IRWXO)) != 0) {
	if (fmInitialized) {
	    message(Sprintf(FILE_IS_READABLE_MSG, fname)->ptr, 0, 0);
	    refresh();
	}
	else {
	    fputs(Sprintf(FILE_IS_READABLE_MSG, fname)->ptr, stderr);
	    fputc('\n', stderr);
	}
	sleep(2);
	return NULL;
    }

    return fopen(efname, "r");
}

void
loadPasswd(void)
{
    FILE *fp;

    passwords = NULL;
    fp = openSecretFile(passwd_file);
    if (fp != NULL) {
	parsePasswd(fp, 0);
	fclose(fp);
    }

    /* for FTP */
    fp = openSecretFile("~/.netrc");
    if (fp != NULL) {
	parsePasswd(fp, 1);
	fclose(fp);
    }
    return;
}

/* get last modified time */
char *
last_modified(Buffer *buf)
{
    TextListItem *ti;
    struct stat st;

    if (buf->document_header) {
	for (ti = buf->document_header->first; ti; ti = ti->next) {
	    if (strncasecmp(ti->ptr, "Last-modified: ", 15) == 0) {
		return ti->ptr + 15;
	    }
	}
	return "unknown";
    }
    else if (buf->currentURL.scheme == SCM_LOCAL) {
	if (stat(buf->currentURL.file, &st) < 0)
	    return "unknown";
	return ctime(&st.st_mtime);
    }
    return "unknown";
}

static char roman_num1[] = {
    'i', 'x', 'c', 'm', '*',
};
static char roman_num5[] = {
    'v', 'l', 'd', '*',
};

static Str
romanNum2(int l, int n)
{
    Str s = Strnew();

    switch (n) {
    case 1:
    case 2:
    case 3:
	for (; n > 0; n--)
	    Strcat_char(s, roman_num1[l]);
	break;
    case 4:
	Strcat_char(s, roman_num1[l]);
	Strcat_char(s, roman_num5[l]);
	break;
    case 5:
    case 6:
    case 7:
    case 8:
	Strcat_char(s, roman_num5[l]);
	for (n -= 5; n > 0; n--)
	    Strcat_char(s, roman_num1[l]);
	break;
    case 9:
	Strcat_char(s, roman_num1[l]);
	Strcat_char(s, roman_num1[l + 1]);
	break;
    }
    return s;
}

Str
romanNumeral(int n)
{
    Str r = Strnew();

    if (n <= 0)
	return r;
    if (n >= 4000) {
	Strcat_charp(r, "**");
	return r;
    }
    Strcat(r, romanNum2(3, n / 1000));
    Strcat(r, romanNum2(2, (n % 1000) / 100));
    Strcat(r, romanNum2(1, (n % 100) / 10));
    Strcat(r, romanNum2(0, n % 10));

    return r;
}

Str
romanAlphabet(int n)
{
    Str r = Strnew();
    int l;
    char buf[14];

    if (n <= 0)
	return r;

    l = 0;
    while (n) {
	buf[l++] = 'a' + (n - 1) % 26;
	n = (n - 1) / 26;
    }
    l--;
    for (; l >= 0; l--)
	Strcat_char(r, buf[l]);

    return r;
}

#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif				/* not SIGIOT */

static void
reset_signals(void)
{
#ifdef SIGHUP
    mySignal(SIGHUP, SIG_DFL);	/* terminate process */
#endif
    mySignal(SIGINT, SIG_DFL);	/* terminate process */
#ifdef SIGQUIT
    mySignal(SIGQUIT, SIG_DFL);	/* terminate process */
#endif
    mySignal(SIGTERM, SIG_DFL);	/* terminate process */
    mySignal(SIGILL, SIG_DFL);	/* create core image */
    mySignal(SIGIOT, SIG_DFL);	/* create core image */
    mySignal(SIGFPE, SIG_DFL);	/* create core image */
#ifdef SIGBUS
    mySignal(SIGBUS, SIG_DFL);	/* create core image */
#endif				/* SIGBUS */
#ifdef SIGCHLD
    mySignal(SIGCHLD, SIG_IGN);
#endif
#ifdef SIGPIPE
    mySignal(SIGPIPE, SIG_IGN);
#endif
}

#ifndef FOPEN_MAX
#define FOPEN_MAX 1024		/* XXX */
#endif

static void
close_all_fds_except(int i, int f)
{
    switch (i) {		/* fall through */
    case 0:
	dup2(open(DEV_NULL_PATH, O_RDONLY), 0);
    case 1:
	dup2(open(DEV_NULL_PATH, O_WRONLY), 1);
    case 2:
	dup2(open(DEV_NULL_PATH, O_WRONLY), 2);
    }
    /* close all other file descriptors (socket, ...) */
    for (i = 3; i < FOPEN_MAX; i++) {
	if (i != f)
	    close(i);
    }
}

void
setup_child(int child, int i, int f)
{
    reset_signals();
    mySignal(SIGINT, SIG_IGN);
#ifndef __MINGW32_VERSION
    if (!child)
	SETPGRP();
#endif /* __MINGW32_VERSION */
    close_tty();
    close_all_fds_except(i, f);
    QuietMessage = TRUE;
    fmInitialized = FALSE;
    TrapSignal = FALSE;
}

#ifndef __MINGW32_VERSION
pid_t
open_pipe_rw(FILE ** fr, FILE ** fw)
{
    int fdr[2];
    int fdw[2];
    pid_t pid;

    if (fr && pipe(fdr) < 0)
	goto err0;
    if (fw && pipe(fdw) < 0)
	goto err1;

    flush_tty();
    pid = fork();
    if (pid < 0)
	goto err2;
    if (pid == 0) {
	/* child */
	if (fr) {
	    close(fdr[0]);
	    dup2(fdr[1], 1);
	}
	if (fw) {
	    close(fdw[1]);
	    dup2(fdw[0], 0);
	}
    }
    else {
	if (fr) {
	    close(fdr[1]);
	    if (*fr == stdin)
		dup2(fdr[0], 0);
	    else
		*fr = fdopen(fdr[0], "r");
	}
	if (fw) {
	    close(fdw[0]);
	    if (*fw == stdout)
		dup2(fdw[1], 1);
	    else
		*fw = fdopen(fdw[1], "w");
	}
    }
    return pid;
  err2:
    if (fw) {
	close(fdw[0]);
	close(fdw[1]);
    }
  err1:
    if (fr) {
	close(fdr[0]);
	close(fdr[1]);
    }
  err0:
    return (pid_t) - 1;
}
#endif /* __MINGW32_VERSION */

void
myExec(char *command)
{
    mySignal(SIGINT, SIG_DFL);
    execl("/bin/sh", "sh", "-c", command, NULL);
    exit(127);
}

void
mySystem(char *command, int background)
{
#ifndef __MINGW32_VERSION
    if (background) {
#ifndef __EMX__
	flush_tty();
	if (!fork()) {
	    setup_child(FALSE, 0, -1);
	    myExec(command);
	}
#else
	Str cmd = Strnew_charp("start /f ");
	Strcat_charp(cmd, command);
	system(cmd->ptr);
#endif
    }
    else
#endif /* __MINGW32_VERSION */
	system(command);
}

Str
myExtCommand(char *cmd, char *arg, int redirect)
{
    Str tmp = NULL;
    char *p;
    int set_arg = FALSE;

    for (p = cmd; *p; p++) {
	if (*p == '%' && *(p + 1) == 's' && !set_arg) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(cmd, (int)(p - cmd));
	    Strcat_charp(tmp, arg);
	    set_arg = TRUE;
	    p++;
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (!set_arg) {
	if (redirect)
	    tmp = Strnew_m_charp("(", cmd, ") < ", arg, NULL);
	else
	    tmp = Strnew_m_charp(cmd, " ", arg, NULL);
    }
    return tmp;
}

Str
myEditor(char *cmd, char *file, int line)
{
    Str tmp = NULL;
    char *p;
    int set_file = FALSE, set_line = FALSE;

    for (p = cmd; *p; p++) {
	if (*p == '%' && *(p + 1) == 's' && !set_file) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(cmd, (int)(p - cmd));
	    Strcat_charp(tmp, file);
	    set_file = TRUE;
	    p++;
	}
	else if (*p == '%' && *(p + 1) == 'd' && !set_line && line > 0) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(cmd, (int)(p - cmd));
	    Strcat(tmp, Sprintf("%d", line));
	    set_line = TRUE;
	    p++;
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (!set_file) {
	if (tmp == NULL)
	    tmp = Strnew_charp(cmd);
	if (!set_line && line > 1 && strcasestr(cmd, "vi"))
	    Strcat(tmp, Sprintf(" +%d", line));
	Strcat_m_charp(tmp, " ", file, NULL);
    }
    return tmp;
}

#ifdef __MINGW32_VERSION
char *
expandName(char *name)
{
    return getenv("HOME");
}
#else
char *
expandName(char *name)
{
    char *p;
    struct passwd *passent, *getpwnam(const char *);
    Str extpath = NULL;

    if (name == NULL)
	return NULL;
    p = name;
    if (*p == '/') {
	if ((*(p + 1) == '~' && IS_ALPHA(*(p + 2)))
	    && personal_document_root) {
	    char *q;
	    p += 2;
	    q = strchr(p, '/');
	    if (q) {		/* /~user/dir... */
		passent = getpwnam(allocStr(p, q - p));
		p = q;
	    }
	    else {		/* /~user */
		passent = getpwnam(p);
		p = "";
	    }
	    if (!passent)
		goto rest;
	    extpath = Strnew_m_charp(passent->pw_dir, "/",
				     personal_document_root, NULL);
	    if (*personal_document_root == '\0' && *p == '/')
		p++;
	}
	else
	    goto rest;
	if (Strcmp_charp(extpath, "/") == 0 && *p == '/')
	    p++;
	Strcat_charp(extpath, p);
	return extpath->ptr;
    }
    else
	return expandPath(p);
  rest:
    return name;
}
#endif

char *
file_to_url(char *file)
{
    Str tmp;
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    char *drive = NULL;
#endif
#ifdef SUPPORT_NETBIOS_SHARE
    char *host = NULL;
#endif

    file = expandPath(file);
#ifdef SUPPORT_NETBIOS_SHARE
    if (file[0] == '/' && file[1] == '/') {
	char *p;
	file += 2;
	if (*file) {
	    p = strchr(file, '/');
	    if (p != NULL && p != file) {
		host = allocStr(file, (p - file));
		file = p;
	    }
	}
    }
#endif
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    if (IS_ALPHA(file[0]) && file[1] == ':') {
	drive = allocStr(file, 2);
	file += 2;
    }
    else
#endif
    if (file[0] != '/') {
	tmp = Strnew_charp(CurrentDir);
	if (Strlastchar(tmp) != '/')
	    Strcat_char(tmp, '/');
	Strcat_charp(tmp, file);
	file = tmp->ptr;
    }
    tmp = Strnew_charp("file://");
#ifdef SUPPORT_NETBIOS_SHARE
    if (host)
	Strcat_charp(tmp, host);
#endif
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    if (drive)
	Strcat_charp(tmp, drive);
#endif
    Strcat_charp(tmp, file_quote(cleanupName(file)));
    return tmp->ptr;
}

#ifdef USE_M17N
char *
url_unquote_conv(char *url, wc_ces charset)
#else
char *
url_unquote_conv0(char *url)
#endif
{
#ifdef USE_M17N
    wc_uint8 old_auto_detect = WcOption.auto_detect;
#endif
    Str tmp;
    tmp = Str_url_unquote(Strnew_charp(url), FALSE, TRUE);
#ifdef USE_M17N
    if (!charset || charset == WC_CES_US_ASCII)
	charset = SystemCharset;
    WcOption.auto_detect = WC_OPT_DETECT_ON;
    tmp = convertLine(NULL, tmp, RAW_MODE, &charset, charset);
    WcOption.auto_detect = old_auto_detect;
#endif
    return tmp->ptr;
}

static char *tmpf_base[MAX_TMPF_TYPE] = {
    "tmp", "src", "frame", "cache", "cookie",
};
static unsigned int tmpf_seq[MAX_TMPF_TYPE];

Str
tmpfname(int type, char *ext)
{
    Str tmpf;
    tmpf = Sprintf("%s/w3m%s%d-%d%s",
		   tmp_dir,
		   tmpf_base[type],
		   CurrentPid, tmpf_seq[type]++, (ext) ? ext : "");
    pushText(fileToDelete, tmpf->ptr);
    return tmpf;
}

static char *monthtbl[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int
get_day(char **s)
{
    Str tmp = Strnew();
    int day;
    char *ss = *s;

    if (!**s)
	return -1;

    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));

    day = atoi(tmp->ptr);

    if (day < 1 || day > 31) {
	*s = ss;
	return -1;
    }
    return day;
}

static int
get_month(char **s)
{
    Str tmp = Strnew();
    int mon;
    char *ss = *s;

    if (!**s)
	return -1;

    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    if (tmp->length > 0) {
	mon = atoi(tmp->ptr);
    }
    else {
	while (**s && IS_ALPHA(**s))
	    Strcat_char(tmp, *((*s)++));
	for (mon = 1; mon <= 12; mon++) {
	    if (strncmp(tmp->ptr, monthtbl[mon - 1], 3) == 0)
		break;
	}
    }
    if (mon < 1 || mon > 12) {
	*s = ss;
	return -1;
    }
    return mon;
}

static int
get_year(char **s)
{
    Str tmp = Strnew();
    int year;
    char *ss = *s;

    if (!**s)
	return -1;

    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    if (tmp->length != 2 && tmp->length != 4) {
	*s = ss;
	return -1;
    }

    year = atoi(tmp->ptr);
    if (tmp->length == 2) {
	if (year >= 70)
	    year += 1900;
	else
	    year += 2000;
    }
    return year;
}

static int
get_time(char **s, int *hour, int *min, int *sec)
{
    Str tmp = Strnew();
    char *ss = *s;

    if (!**s)
	return -1;

    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    if (**s != ':') {
	*s = ss;
	return -1;
    }
    *hour = atoi(tmp->ptr);

    (*s)++;
    Strclear(tmp);
    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    if (**s != ':') {
	*s = ss;
	return -1;
    }
    *min = atoi(tmp->ptr);

    (*s)++;
    Strclear(tmp);
    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    *sec = atoi(tmp->ptr);

    if (*hour < 0 || *hour >= 24 ||
	*min < 0 || *min >= 60 || *sec < 0 || *sec >= 60) {
	*s = ss;
	return -1;
    }
    return 0;
}

static int
get_zone(char **s, int *z_hour, int *z_min)
{
    Str tmp = Strnew();
    int zone;
    char *ss = *s;

    if (!**s)
	return -1;

    if (**s == '+' || **s == '-')
	Strcat_char(tmp, *((*s)++));
    while (**s && IS_DIGIT(**s))
	Strcat_char(tmp, *((*s)++));
    if (!(tmp->length == 4 && IS_DIGIT(*ss)) &&
	!(tmp->length == 5 && (*ss == '+' || *ss == '-'))) {
	*s = ss;
	return -1;
    }

    zone = atoi(tmp->ptr);
    *z_hour = zone / 100;
    *z_min = zone - (zone / 100) * 100;
    return 0;
}

/* RFC 1123 or RFC 850 or ANSI C asctime() format string -> time_t */
time_t
mymktime(char *timestr)
{
    char *s;
    int day, mon, year, hour, min, sec, z_hour = 0, z_min = 0;

    if (!(timestr && *timestr))
	return -1;
    s = timestr;

#ifdef DEBUG
    fprintf(stderr, "mktime: %s\n", timestr);
#endif				/* DEBUG */

    while (*s && IS_ALPHA(*s))
	s++;
    while (*s && !IS_ALNUM(*s))
	s++;

    if (IS_DIGIT(*s)) {
	/* RFC 1123 or RFC 850 format */
	if ((day = get_day(&s)) == -1)
	    return -1;

	while (*s && !IS_ALNUM(*s))
	    s++;
	if ((mon = get_month(&s)) == -1)
	    return -1;

	while (*s && !IS_DIGIT(*s))
	    s++;
	if ((year = get_year(&s)) == -1)
	    return -1;

	while (*s && !IS_DIGIT(*s))
	    s++;
	if (!*s) {
	    hour = 0;
	    min = 0;
	    sec = 0;
	}
	else {
	    if (get_time(&s, &hour, &min, &sec) == -1)
		return -1;
	    while (*s && !IS_DIGIT(*s) && *s != '+' && *s != '-')
		s++;
	    get_zone(&s, &z_hour, &z_min);
	}
    }
    else {
	/* ANSI C asctime() format. */
	while (*s && !IS_ALNUM(*s))
	    s++;
	if ((mon = get_month(&s)) == -1)
	    return -1;

	while (*s && !IS_DIGIT(*s))
	    s++;
	if ((day = get_day(&s)) == -1)
	    return -1;

	while (*s && !IS_DIGIT(*s))
	    s++;
	if (get_time(&s, &hour, &min, &sec) == -1)
	    return -1;

	while (*s && !IS_DIGIT(*s))
	    s++;
	if ((year = get_year(&s)) == -1)
	    return -1;
    }
#ifdef DEBUG
    fprintf(stderr,
	    "year=%d month=%d day=%d hour:min:sec=%d:%d:%d zone=%d:%d\n", year,
	    mon, day, hour, min, sec, z_hour, z_min);
#endif				/* DEBUG */

    mon -= 3;
    if (mon < 0) {
	mon += 12;
	year--;
    }
    day += (year - 1968) * 1461 / 4;
    day += ((((mon * 153) + 2) / 5) - 672);
    hour -= z_hour;
    min -= z_min;
    return (time_t) ((day * 60 * 60 * 24) +
		     (hour * 60 * 60) + (min * 60) + sec);
}

#ifdef USE_COOKIE
#ifdef INET6
#include <sys/socket.h>
#endif				/* INET6 */
#ifndef __MINGW32_VERSION
#include <netdb.h>
#else
#include <winsock.h>
#endif
char *
FQDN(char *host)
{
    char *p;
#ifndef INET6
    struct hostent *entry;
#else				/* INET6 */
    int *af;
#endif				/* INET6 */

    if (host == NULL)
	return NULL;

    if (strcasecmp(host, "localhost") == 0)
	return host;

    for (p = host; *p && *p != '.'; p++) ;

    if (*p == '.')
	return host;

#ifndef INET6
    if (!(entry = gethostbyname(host)))
	return NULL;

    return allocStr(entry->h_name, -1);
#else				/* INET6 */
    for (af = ai_family_order_table[DNS_order];; af++) {
	int error;
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	char *namebuf;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = *af;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(host, NULL, &hints, &res0);
	if (error) {
	    if (*af == PF_UNSPEC) {
		/* all done */
		break;
	    }
	    /* try next address family */
	    continue;
	}
	for (res = res0; res != NULL; res = res->ai_next) {
	    if (res->ai_canonname) {
		/* found */
		namebuf = strdup(res->ai_canonname);
		freeaddrinfo(res0);
		return namebuf;
	    }
	}
	freeaddrinfo(res0);
	if (*af == PF_UNSPEC) {
	    break;
	}
    }
    /* all failed */
    return NULL;
#endif				/* INET6 */
}

#endif				/* USE_COOKIE */

void (*mySignal(int signal_number, void (*action) (int))) (int) {
#ifdef	SA_RESTART
    struct sigaction new_action, old_action;

    sigemptyset(&new_action.sa_mask);
    new_action.sa_handler = action;
    if (signal_number == SIGALRM) {
#ifdef	SA_INTERRUPT
	new_action.sa_flags = SA_INTERRUPT;
#else
	new_action.sa_flags = 0;
#endif
    }
    else {
	new_action.sa_flags = SA_RESTART;
    }
    sigaction(signal_number, &new_action, &old_action);
    return (old_action.sa_handler);
#else
    return (signal(signal_number, action));
#endif
}
