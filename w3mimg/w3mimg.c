/* $Id: w3mimg.c,v 1.3 2002/07/19 03:24:28 ukai Exp $ */

#include <stdlib.h>
#include "w3mimg/w3mimg.h"

w3mimg_op *
w3mimg_open()
{
    w3mimg_op *w_op = NULL;
#ifdef USE_W3MIMG_X11
    if (w_op == NULL)
	w_op = w3mimg_x11open();
#endif
#ifdef USE_W3MIMG_FB
    if (w_op == NULL)
	w_op = w3mimg_fbopen();
#endif
    return w_op;
}
