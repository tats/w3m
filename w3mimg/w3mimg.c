/* $Id: w3mimg.c,v 1.6 2010/12/21 10:13:55 htrb Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "w3mimg/w3mimg.h"

w3mimg_op *
w3mimg_open()
{
    w3mimg_op *w_op = NULL;
#ifdef W3MIMGDISPLAY_SETUID
    uid_t runner_uid = getuid();
    uid_t owner_uid = geteuid();
#endif
#ifdef USE_W3MIMG_WIN
    if (w_op == NULL)
	w_op = w3mimg_winopen();
#endif
#ifdef USE_W3MIMG_X11
#ifdef W3MIMGDISPLAY_SETUID
    /* run in user privileges */
    setreuid(owner_uid, runner_uid);
#endif
    if (w_op == NULL)
	w_op = w3mimg_x11open();
#ifdef W3MIMGDISPLAY_SETUID
    setreuid(runner_uid, owner_uid);
#endif
#endif
#ifdef USE_W3MIMG_FB
    /* run in setuid privileges */
    if (w_op == NULL)
	w_op = w3mimg_fbopen();
#endif
    return w_op;
}
