/* $Id: func.h,v 1.2 2001/11/20 17:49:23 ukai Exp $ */
/*
 * w3m func.h
 */

#ifndef FUNC_H
#define FUNC_H

#define K_ESC  0x100
#define K_ESCB 0x200
#define K_ESCD 0x400

typedef struct _FuncList {
    char *id;
    void (*func) ();
} FuncList;

typedef struct _KeyListItem {
    int key;
    char *data;
} KeyListItem;

typedef struct _KeyList {
    KeyListItem *item;
    int nitem;
    int size;
} KeyList;

#endif				/* not FUNC_H */
