/* $Id: fb.h,v 1.4 2002/07/22 16:17:32 ukai Exp $ */
#ifndef fb_header
#define fb_header
#include <linux/fb.h>

typedef struct{
      unsigned char *data;
  int width;
  int height;
  int rowstride;
  int len;
} FB_IMAGE;

FB_IMAGE *fb_image_new(int width, int height);
void   fb_image_pset(FB_IMAGE *image, int x, int y, int r, int g, int b);
int    fb_image_draw(FB_IMAGE *image, int x, int y, int sx, int sy, int width, int height);
void   fb_image_free(FB_IMAGE *image);
void   fb_image_rotete(FB_IMAGE *image, int direction);

int  fb_open(void);
void fb_close(void);
void fb_pset(int x, int y, int r, int g, int b);
int  fb_get_color(int x, int y, int *r, int *g, int *b);
void fb_clear(void);
int fb_width(void);
int fb_height(void);
void fb_cmap_disp(void);
void fb_fscrn_disp(void);
void fb_vscrn_disp(void);

#endif
