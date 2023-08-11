/* $Id: Str.c,v 1.8 2002/12/24 17:20:46 ukai Exp $ */
/* 
 * String manipulation library for Boehm GC
 *
 * (C) Copyright 1998-1999 by Akinori Ito
 *
 * This software may be redistributed freely for this purpose, in full 
 * or in part, provided that this entire copyright notice is included 
 * on any copies of this software and applications and derivations thereof.
 *
 * This software is provided on an "as is" basis, without warranty of any
 * kind, either expressed or implied, as to any matter including, but not
 * limited to warranty of fitness of purpose, or merchantability, or
 * results obtained from use of this software.
 */
#include <stdio.h>
#include <stdlib.h>
#include <gc.h>
#include <stdarg.h>
#include <string.h>
#ifdef __EMX__			/* or include "fm.h" for HAVE_BCOPY? */
#include <strings.h>
#endif
#include "Str.h"
#include "myctype.h"

#define INITIAL_STR_SIZE 32

#ifdef STR_DEBUG
/* This is obsolete, because "Str" can handle a '\0' character now. */
#define STR_LENGTH_CHECK(x) if (((x)->ptr==0&&(x)->length!=0)||(strlen((x)->ptr)!=(x)->length))abort();
#else				/* not STR_DEBUG */
#define STR_LENGTH_CHECK(x)
#endif				/* not STR_DEBUG */

Str
Strnew()
{
    Str x = GC_MALLOC(sizeof(struct _Str));
    if (x == NULL)
	exit(1);
    x->ptr = GC_MALLOC_ATOMIC(INITIAL_STR_SIZE);
    if (x->ptr == NULL)
	exit(1);
    x->ptr[0] = '\0';
    x->area_size = INITIAL_STR_SIZE;
    x->length = 0;
    return x;
}

Str
Strnew_size(int n)
{
    Str x = GC_MALLOC(sizeof(struct _Str));
    if (x == NULL)
	exit(1);
    if (n < 0 || n >= STR_SIZE_MAX)
	n = STR_SIZE_MAX - 1;
    else if (n + 1 < INITIAL_STR_SIZE)
	n = INITIAL_STR_SIZE - 1;
    x->ptr = GC_MALLOC_ATOMIC(n + 1);
    if (x->ptr == NULL)
	exit(1);
    x->ptr[0] = '\0';
    x->area_size = n + 1;
    x->length = 0;
    return x;
}

Str
Strnew_charp(const char *p)
{
    Str x;
    int n, len;

    if (p == NULL)
	return Strnew();
    x = GC_MALLOC(sizeof(struct _Str));
    if (x == NULL)
	exit(1);
    n = strlen(p) + 1;
    if (n <= 0 || n > STR_SIZE_MAX)
	n = STR_SIZE_MAX;
    len = n - 1;
    if (n < INITIAL_STR_SIZE)
	n = INITIAL_STR_SIZE;
    x->ptr = GC_MALLOC_ATOMIC(n);
    if (x->ptr == NULL)
	exit(1);
    x->area_size = n;
    x->length = len;
    bcopy((const void *)p, (void *)x->ptr, len);
    x->ptr[x->length] = '\0';
    return x;
}

Str
Strnew_m_charp(const char *p, ...)
{
    va_list ap;
    Str r = Strnew();

    va_start(ap, p);
    while (p != NULL) {
	Strcat_charp(r, p);
	p = va_arg(ap, char *);
    }
    va_end(ap);
    return r;
}

Str
Strnew_charp_n(const char *p, int n)
{
    Str x;
    int len;

    if (p == NULL)
	return Strnew_size(n);
    x = GC_MALLOC(sizeof(struct _Str));
    if (x == NULL)
	exit(1);
    if (n < 0 || n >= STR_SIZE_MAX)
	n = STR_SIZE_MAX - 1;
    len = n;
    if (n + 1 < INITIAL_STR_SIZE)
	n = INITIAL_STR_SIZE - 1;
    x->ptr = GC_MALLOC_ATOMIC(n + 1);
    if (x->ptr == NULL)
	exit(1);
    x->area_size = n + 1;
    x->length = len;
    bcopy((const void *)p, (void *)x->ptr, len);
    x->ptr[x->length] = '\0';
    return x;
}

Str
Strdup(Str s)
{
    Str n = Strnew_size(s->length);
    STR_LENGTH_CHECK(s);
    Strcopy(n, s);
    return n;
}

void
Strclear(Str s)
{
    s->length = 0;
    s->ptr[0] = '\0';
}

void
Strfree(Str x)
{
    GC_free(x->ptr);
    GC_free(x);
}

void
Strcopy(Str x, Str y)
{
    STR_LENGTH_CHECK(x);
    STR_LENGTH_CHECK(y);
    if (x->area_size < y->length + 1) {
	x->ptr = GC_REALLOC(x->ptr, y->length + 1);
	if (x->ptr == NULL)
	    exit(1);
	x->area_size = y->length + 1;
    }
    bcopy((const void *)y->ptr, (void *)x->ptr, y->length + 1);
    x->length = y->length;
}

void
Strcopy_charp(Str x, const char *y)
{
    int len;

    STR_LENGTH_CHECK(x);
    if (y == NULL) {
	x->length = 0;
	x->ptr[0] = '\0';
	return;
    }
    len = strlen(y);
    if (len < 0 || len >= STR_SIZE_MAX)
	len = STR_SIZE_MAX - 1;
    if (x->area_size < len + 1) {
	x->ptr = GC_REALLOC(x->ptr, len + 1);
	if (x->ptr == NULL)
	    exit(1);
	x->area_size = len + 1;
    }
    bcopy((const void *)y, (void *)x->ptr, len);
    x->ptr[len] = '\0';
    x->length = len;
}

void
Strcopy_charp_n(Str x, const char *y, int n)
{
    int len = n;

    STR_LENGTH_CHECK(x);
    if (y == NULL) {
	x->length = 0;
	x->ptr[0] = '\0';
	return;
    }
    if (len < 0 || len >= STR_SIZE_MAX)
	len = STR_SIZE_MAX - 1;
    if (x->area_size < len + 1) {
	x->ptr = GC_REALLOC(x->ptr, len + 1);
	if (x->ptr == NULL)
	    exit(1);
	x->area_size = len + 1;
    }
    bcopy((const void *)y, (void *)x->ptr, len);
    x->ptr[len] = '\0';
    x->length = len;
}

void
Strcat_charp_n(Str x, const char *y, int n)
{
    int newlen;

    STR_LENGTH_CHECK(x);
    if (y == NULL || n == 0)
	return;
    if (n < 0)
	n = STR_SIZE_MAX - 1;
    newlen = x->length + n + 1;
    if (newlen <= 0 || newlen > STR_SIZE_MAX) {
	newlen = STR_SIZE_MAX;
	n = newlen - x->length - 1;
	if (n <= 0)
	    return;
    }
    if (x->area_size < newlen) {
	newlen += newlen / 2;
	if (newlen <= 0 || newlen > STR_SIZE_MAX)
	    newlen = STR_SIZE_MAX;
	x->ptr = GC_REALLOC(x->ptr, newlen);
	if (x->ptr == NULL)
	    exit(1);
	x->area_size = newlen;
    }
    bcopy((const void *)y, (void *)&x->ptr[x->length], n);
    x->length += n;
    x->ptr[x->length] = '\0';
}

void
Strcat(Str x, Str y)
{
    STR_LENGTH_CHECK(y);
    Strcat_charp_n(x, y->ptr, y->length);
}

void
Strcat_charp(Str x, const char *y)
{
    if (y == NULL)
	return;
    Strcat_charp_n(x, y, strlen(y));
}

void
Strcat_m_charp(Str x, ...)
{
    va_list ap;
    char *p;

    va_start(ap, x);
    while ((p = va_arg(ap, char *)) != NULL)
	 Strcat_charp_n(x, p, strlen(p));
    va_end(ap);
}

void
Strgrow(Str x)
{
    int newlen, addlen;

    if (x->area_size < 8192)
	addlen = x->area_size;
    else
	addlen = x->area_size / 2;
    if (addlen < INITIAL_STR_SIZE)
	addlen = INITIAL_STR_SIZE;
    newlen = x->area_size + addlen;
    if (newlen <= 0 || newlen > STR_SIZE_MAX) {
	newlen = STR_SIZE_MAX;
	if (x->length + 1 >= newlen)
	    x->length = newlen - 2;
    }
    if (x->area_size < newlen) {
	x->ptr = GC_REALLOC(x->ptr, newlen);
	if (x->ptr == NULL)
	    exit(1);
	x->area_size = newlen;
    }
    x->ptr[x->length] = '\0';
}

Str
Strsubstr(Str s, int beg, int len)
{
    Str new_s;
    int i;

    STR_LENGTH_CHECK(s);
    new_s = Strnew();
    if (beg >= s->length)
	return new_s;
    for (i = 0; i < len && beg + i < s->length; i++)
	Strcat_char(new_s, s->ptr[beg + i]);
    return new_s;
}

void
Strlower(Str s)
{
    int i;
    STR_LENGTH_CHECK(s);
    for (i = 0; i < s->length; i++)
	s->ptr[i] = TOLOWER(s->ptr[i]);
}

void
Strupper(Str s)
{
    int i;
    STR_LENGTH_CHECK(s);
    for (i = 0; i < s->length; i++)
	s->ptr[i] = TOUPPER(s->ptr[i]);
}

void
Strchop(Str s)
{
    STR_LENGTH_CHECK(s);
    while (s->length > 0 &&
	   (s->ptr[s->length - 1] == '\n' || s->ptr[s->length - 1] == '\r')) {
	s->length--;
    }
    s->ptr[s->length] = '\0';
}

void
Strinsert_char(Str s, int pos, char c)
{
    int i;
    STR_LENGTH_CHECK(s);
    if (pos < 0 || s->length < pos)
	return;
    if (s->length + 2 > s->area_size)
	Strgrow(s);
    if (s->length < pos)
	return;
    for (i = s->length; i > pos; i--)
	s->ptr[i] = s->ptr[i - 1];
    s->ptr[++s->length] = '\0';
    s->ptr[pos] = c;
}

void
Strinsert_charp(Str s, int pos, const char *p)
{
    STR_LENGTH_CHECK(s);
    while (*p)
	Strinsert_char(s, pos++, *(p++));
}

void
Strdelete(Str s, int pos, int n)
{
    int i;
    STR_LENGTH_CHECK(s);
    if (pos < 0 || s->length < pos)
	return;
    if (n < 0)
	n = STR_SIZE_MAX - pos - 1;
    if (s->length <= pos + n) {
	s->ptr[pos] = '\0';
	s->length = pos;
	return;
    }
    for (i = pos; i < s->length - n; i++)
	s->ptr[i] = s->ptr[i + n];
    s->ptr[i] = '\0';
    s->length = i;
}

void
Strtruncate(Str s, int pos)
{
    STR_LENGTH_CHECK(s);
    if (pos < 0 || s->length < pos)
	return;
    s->ptr[pos] = '\0';
    s->length = pos;
}

void
Strshrink(Str s, int n)
{
    STR_LENGTH_CHECK(s);
    if (n >= s->length) {
	s->length = 0;
	s->ptr[0] = '\0';
    }
    else if (n > 0) {
	s->length -= n;
	s->ptr[s->length] = '\0';
    }
}

void
Strremovefirstspaces(Str s)
{
    int i;

    STR_LENGTH_CHECK(s);
    for (i = 0; i < s->length && IS_SPACE(s->ptr[i]); i++) ;
    if (i == 0)
	return;
    Strdelete(s, 0, i);
}

void
Strremovetrailingspaces(Str s)
{
    int i;

    STR_LENGTH_CHECK(s);
    for (i = s->length - 1; i >= 0 && IS_SPACE(s->ptr[i]); i--) ;
    s->length = i + 1;
    s->ptr[i + 1] = '\0';
}

Str
Stralign_left(Str s, int width)
{
    Str n;
    int i;

    STR_LENGTH_CHECK(s);
    if (s->length >= width)
	return Strdup(s);
    n = Strnew_size(width);
    Strcopy(n, s);
    for (i = s->length; i < width; i++)
	Strcat_char(n, ' ');
    return n;
}

Str
Stralign_right(Str s, int width)
{
    Str n;
    int i;

    STR_LENGTH_CHECK(s);
    if (s->length >= width)
	return Strdup(s);
    n = Strnew_size(width);
    for (i = s->length; i < width; i++)
	Strcat_char(n, ' ');
    Strcat(n, s);
    return n;
}

Str
Stralign_center(Str s, int width)
{
    Str n;
    int i, w;

    STR_LENGTH_CHECK(s);
    if (s->length >= width)
	return Strdup(s);
    n = Strnew_size(width);
    w = (width - s->length) / 2;
    for (i = 0; i < w; i++)
	Strcat_char(n, ' ');
    Strcat(n, s);
    for (i = w + s->length; i < width; i++)
	Strcat_char(n, ' ');
    return n;
}

#define SP_NORMAL 0
#define SP_PREC   1
#define SP_PREC2  2

Str
Sprintf(char *fmt, ...)
{
    int len = 0;
    int status = SP_NORMAL;
    int p = 0;
    char *f;
    Str s;
    va_list ap;

    va_start(ap, fmt);
    for (f = fmt; *f; f++) {
      redo:
	switch (status) {
	case SP_NORMAL:
	    if (*f == '%') {
		status = SP_PREC;
		p = 0;
	    }
	    else
		len++;
	    break;
	case SP_PREC:
	    if (IS_ALPHA(*f)) {
		/* conversion char. */
		int vi;
		char *vs;

		switch (*f) {
		case 'l':
		case 'h':
		case 'L':
		case 'w':
		    continue;
		case 'd':
		case 'i':
		case 'o':
		case 'x':
		case 'X':
		case 'u':
		    va_arg(ap, int);
		    len += (p > 0) ? p : 10;
		    break;
		case 'f':
		case 'g':
		case 'e':
		case 'G':
		case 'E':
		    va_arg(ap, double);
		    len += (p > 0) ? p : 15;
		    break;
		case 'c':
		    len += 1;
		    va_arg(ap, int);
		    break;
		case 's':
		    vs = va_arg(ap, char *);
		    vi = strlen(vs);
		    len += (p > vi) ? p : vi;
		    break;
		case 'p':
		    va_arg(ap, void *);
		    len += 10;
		    break;
		case 'n':
		    va_arg(ap, void *);
		    break;
		}
		status = SP_NORMAL;
	    }
	    else if (IS_DIGIT(*f))
		p = p * 10 + *f - '0';
	    else if (*f == '.')
		status = SP_PREC2;
	    else if (*f == '%') {
		status = SP_NORMAL;
		len++;
	    }
	    break;
	case SP_PREC2:
	    if (IS_ALPHA(*f)) {
		status = SP_PREC;
		goto redo;
	    }
	    break;
	}
    }
    va_end(ap);
    s = Strnew_size(len * 2);
    va_start(ap, fmt);
    vsprintf(s->ptr, fmt, ap);
    va_end(ap);
    s->length = strlen(s->ptr);
    if (s->length > len * 2) {
	fprintf(stderr, "Sprintf: string too long\n");
	exit(1);
    }
    return s;
}

Str
Strfgets(FILE * f)
{
    Str s = Strnew();
    int c;
    while ((c = fgetc(f)) != EOF) {
	Strcat_char(s, c);
	if (c == '\n')
	    break;
    }
    return s;
}

Str
Strfgetall(FILE * f)
{
    Str s = Strnew();
    int c;
    while ((c = fgetc(f)) != EOF) {
	Strcat_char(s, c);
    }
    return s;
}
