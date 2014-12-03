/* $Id: fb.h,v 1.7 2003/07/07 15:48:17 ukai Exp $ */
#ifndef fb_header
#define fb_header
#if defined(__linux__)
#include <linux/fb.h>
#elif defined(__FreeBSD__)
#include <sys/fbio.h> 
#endif

typedef struct {
    int num;
    int id;
    int delay;
    int width;
    int height;
    int rowstride;
    int len;
    unsigned char *data;
} FB_IMAGE;

FB_IMAGE *fb_image_new(int width, int height);
void fb_image_pset(FB_IMAGE * image, int x, int y, int r, int g, int b);
void fb_image_fill(FB_IMAGE * image, int r, int g, int b);
int fb_image_draw(FB_IMAGE * image, int x, int y, int sx, int sy, int width,
		  int height);
void fb_image_free(FB_IMAGE * image);
void fb_image_copy(FB_IMAGE * dest, FB_IMAGE * src);

FB_IMAGE **fb_frame_new(int w, int h, int num);
void fb_frame_free(FB_IMAGE ** frame);

int fb_open(void);
void fb_close(void);
int fb_width(void);
int fb_height(void);
int fb_clear(int x, int y, int w, int h, int r, int g, int b);

#endif
