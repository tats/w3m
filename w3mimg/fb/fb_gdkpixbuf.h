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
