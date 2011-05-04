/* $Id: Str.h,v 1.6 2006/04/07 13:35:35 inu Exp $ */
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
#ifndef GC_STR_H
#define GC_STR_H
#include <stdio.h>
#include <string.h>
#ifdef __EMX__
#define strcasecmp	stricmp
#define strncasecmp	strnicmp
#endif

typedef struct _Str {
    char *ptr;
    int length;
    int area_size;
} *Str;

Str Strnew(void);
Str Strnew_size(int);
Str Strnew_charp(char *);
Str Strnew_charp_n(char *, int);
Str Strnew_m_charp(char *, ...);
Str Strdup(Str);
void Strclear(Str);
void Strfree(Str);
void Strcopy(Str, Str);
void Strcopy_charp(Str, char *);
void Strcopy_charp_n(Str, char *, int);
void Strcat_charp_n(Str, char *, int);
void Strcat(Str, Str);
void Strcat_charp(Str, char *);
void Strcat_m_charp(Str, ...);
Str Strsubstr(Str, int, int);
void Strinsert_char(Str, int, char);
void Strinsert_charp(Str, int, char *);
void Strdelete(Str, int, int);
void Strtruncate(Str, int);
void Strlower(Str);
void Strupper(Str);
void Strchop(Str);
void Strshrink(Str, int);
void Strshrinkfirst(Str, int);
void Strremovefirstspaces(Str);
void Strremovetrailingspaces(Str);
Str Stralign_left(Str, int);
Str Stralign_right(Str, int);
Str Stralign_center(Str, int);

Str Sprintf(char *fmt, ...);

Str Strfgets(FILE *);
Str Strfgetall(FILE *);

void Strgrow(Str s);

#define Strcat_char(x,y) (((x)->length+1>=(x)->area_size)?Strgrow(x),0:0,(x)->ptr[(x)->length++]=(y),(x)->ptr[(x)->length]=0)
#define Strcmp(x,y)                  strcmp((x)->ptr,(y)->ptr)
#define Strcmp_charp(x,y)            strcmp((x)->ptr,(y))
#define Strncmp(x,y,n)               strncmp((x)->ptr,(y)->ptr,(n))
#define Strncmp_charp(x,y,n)         strncmp((x)->ptr,(y),(n))
#define Strcasecmp(x,y)              strcasecmp((x)->ptr,(y)->ptr)
#define Strcasecmp_charp(x,y)        strcasecmp((x)->ptr,(y))
#define Strncasecmp(x,y,n)           strncasecmp((x)->ptr,(y)->ptr,(n))
#define Strncasecmp_charp(x,y,n)     strncasecmp((x)->ptr,(y),(n))

#define Strlastchar(s)               ((s)->length>0?(s)->ptr[(s)->length-1]:'\0')
#define Strinsert(s,n,p)             Strinsert_charp((s),(n),(p)->ptr)
#define Strshrinkfirst(s,n)          Strdelete((s),0,(n))
#define Strfputs(s,f)                fwrite((s)->ptr,1,(s)->length,(f))
#endif				/* not GC_STR_H */
