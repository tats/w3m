#ifndef fb_img_header
#define fb_img_header
#include "config.h"

#if defined(USE_IMLIB2)
 #include "w3mimg/fb/fb_imlib2.h"
#elif defined(USE_GDKPIXBUF)
 #include "w3mimg/fb/fb_gdkpixbuf.h"
#else
#error no Imlib2 and GdkPixbuf support
#endif

IMAGE *fb_load_image(char *filename, int w, int h);
int    fb_draw_image(IMAGE *img, int x, int y, int sx, int sy, int width, int height);
int    fb_draw_image_simple(IMAGE *img, int x, int y);
int    fb_resize_image(IMAGE *img, int width, int height);
void   fb_free_image(IMAGE *img);
void   fb_set_bg(int r, int g, int b);
IMAGE *fb_dup_image(IMAGE *img);
int    fb_rotate_image(IMAGE *img, int angle);

#endif
