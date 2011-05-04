/* $Id: fb_img.c,v 1.6 2003/07/07 15:48:17 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "config.h"
#include "fb.h"
#include "fb_img.h"

static int bg_r = 0, bg_g = 0, bg_b = 0;

#if defined(USE_IMLIB2)
#include "w3mimg/fb/fb_imlib2.c"
#elif defined(USE_GDKPIXBUF)
#include "w3mimg/fb/fb_gdkpixbuf.c"
#else
#error no Imlib2 and GdkPixbuf support
#endif

void
fb_image_set_bg(int r, int g, int b)
{
    bg_r = r;
    bg_g = g;
    bg_b = b;
}

int
fb_image_clear(int x, int y, int w, int h)
{
    return fb_clear(x, y, w, h, bg_r, bg_g, bg_b);
}
