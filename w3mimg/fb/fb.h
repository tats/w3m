/* $Id: fb.h,v 1.5 2002/07/29 15:25:37 ukai Exp $ */
#ifndef fb_header
#define fb_header
#include <linux/fb.h>

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
void fb_image_rotate(FB_IMAGE * image, int direction);
void fb_image_copy(FB_IMAGE * dest, FB_IMAGE * src);

FB_IMAGE **fb_frame_new(int w, int h, int num);
void fb_frame_free(FB_IMAGE ** frame);
void fb_frame_rotate(FB_IMAGE ** frame, int direction);

int fb_open(void);
void fb_close(void);
void fb_pset(int x, int y, int r, int g, int b);
int fb_get_color(int x, int y, int *r, int *g, int *b);
void fb_clear(void);
int fb_width(void);
int fb_height(void);
void fb_cmap_disp(void);
void fb_fscrn_disp(void);
void fb_vscrn_disp(void);

#endif
