/* $Id: fb_gdkpixbuf.h,v 1.3 2002/07/18 15:01:31 ukai Exp $ */
#ifndef fb_gdkpixbuf_header
#define fb_gdkpixbuf_header

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    int width;
    int height;
    int rowstride;
    int alpha;
    GdkPixbuf *pixbuf;
    guchar *pixels;
} IMAGE;

#endif
