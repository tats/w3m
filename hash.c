/* $Id: hash.c,v 1.4 2001/12/10 17:02:44 ukai Exp $ */
#include <string.h>
#include "hash.h"
#include "gc.h"

static unsigned int
hashfunc(char *s)
{
    unsigned int h = 0;
    while (*s) {
	if (h & 0x80000000) {
	    h <<= 1;
	    h |= 1;
	}
	else
	    h <<= 1;
	h += *s;
	s++;
    }
    return h;
}

#define keycomp(x,y) !strcmp(x,y)

/* *INDENT-OFF* */
defhashfunc(char *, int, si)
defhashfunc(char *, char *, ss)
defhashfunc(char *, void *, sv)
defhashfunc_i(int, void *, iv)
/* *INDENT-ON* */
