/* $Id: myctype.h,v 1.6 2003/09/22 21:02:20 ukai Exp $ */
#ifndef _MYCTYPE_H
#define _MYCTYPE_H

#define MYCTYPE_CNTRL 1
#define MYCTYPE_SPACE 2
#define MYCTYPE_ALPHA 4
#define MYCTYPE_DIGIT 8
#define MYCTYPE_PRINT 16
#define MYCTYPE_HEX   32
#define MYCTYPE_INTSPACE 64
#define MYCTYPE_ASCII (MYCTYPE_CNTRL|MYCTYPE_PRINT)
#define MYCTYPE_ALNUM (MYCTYPE_ALPHA|MYCTYPE_DIGIT)
#define MYCTYPE_XDIGIT (MYCTYPE_HEX|MYCTYPE_DIGIT)

#define GET_MYCTYPE(x) (MYCTYPE_MAP[(int)(unsigned char)(x)])
#define GET_MYCDIGIT(x) (MYCTYPE_DIGITMAP[(int)(unsigned char)(x)])

#define IS_CNTRL(x) (GET_MYCTYPE(x) & MYCTYPE_CNTRL)
#define IS_SPACE(x) (GET_MYCTYPE(x) & MYCTYPE_SPACE)
#define IS_ALPHA(x) (GET_MYCTYPE(x) & MYCTYPE_ALPHA)
#define IS_DIGIT(x) (GET_MYCTYPE(x) & MYCTYPE_DIGIT)
#define IS_PRINT(x) (GET_MYCTYPE(x) & MYCTYPE_PRINT)
#define IS_ASCII(x) (GET_MYCTYPE(x) & MYCTYPE_ASCII)
#define IS_ALNUM(x) (GET_MYCTYPE(x) & MYCTYPE_ALNUM)
#define IS_XDIGIT(x) (GET_MYCTYPE(x) & MYCTYPE_XDIGIT)
#define IS_INTSPACE(x) (MYCTYPE_MAP[(unsigned char)(x)] & MYCTYPE_INTSPACE)

extern unsigned char MYCTYPE_MAP[];
extern unsigned char MYCTYPE_DIGITMAP[];

#define	TOLOWER(x)	(IS_ALPHA(x) ? ((x)|0x20) : (x))
#define	TOUPPER(x)	(IS_ALPHA(x) ? ((x)&~0x20) : (x))

#endif
