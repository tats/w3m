/*
 * Copyright (c) 1999, NBG01720@nifty.ne.jp
 *
 * To compile this program:
 *      gcc -D__ST_MT_ERRNO__ -O2 -s -Zomf -Zmtd -lX11 scrsize.c
 *
 * When I wrote this routine, I consulted some part of the source code of the
 * xwininfo utility by X Consortium.
 *
 * Copyright (c) 1987, X Consortium
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the X Consortium shall not
 * be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the X
 * Consortium.
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>

int
main()
{
    char *cp;
    Display *dpy;
    Window window;
    XWindowAttributes win_attributes;
    XSizeHints hints;
    long longjunk;
    int dst[2];

    _scrsize(dst);
    cp = getenv("WINDOWID");
    if (cp) {
	dpy = XOpenDisplay(NULL);
	if (dpy) {
	    if (XGetWindowAttributes(dpy, window = atol(cp), &win_attributes))
		if (XGetWMNormalHints(dpy, window, &hints, &longjunk))
		    if (hints.flags & PResizeInc && hints.width_inc
			&& hints.height_inc) {
			if (hints.flags & (PBaseSize | PMinSize)) {
			    if (hints.flags & PBaseSize) {
				win_attributes.width -= hints.base_width;
				win_attributes.height -= hints.base_height;
			    }
			    else {
				win_attributes.width -= hints.min_width;
				win_attributes.height -= hints.min_height;
			    }
			}
			dst[0] = win_attributes.width / hints.width_inc;
			dst[1] = win_attributes.height / hints.height_inc;
		    }
	    XCloseDisplay(dpy);
	}
    }
    printf("%i %i\n", dst[0], dst[1]);
    return 0;
}
