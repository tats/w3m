/* $Id: func.h,v 1.3 2001/12/10 17:02:44 ukai Exp $ */
/*
 * w3m func.h
 */

#ifndef FUNC_H
#define FUNC_H

#include "textlist.h"
#include "hash.h"

#define KEY_HASH_SIZE 127

#define K_ESC  0x100
#define K_ESCB 0x200
#define K_ESCD 0x400

typedef struct _FuncList {
    char *id;
    void (*func) ();
} FuncList;

#endif				/* not FUNC_H */
