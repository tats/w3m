/* $Id: fb_img.h,v 1.4 2002/07/22 16:17:32 ukai Exp $ */
#ifndef fb_img_header
#define fb_img_header
#include "fb.h"

FB_IMAGE *fb_image_load(char *filename, int w, int h);
int fb_image_draw_simple(FB_IMAGE * img, int x, int y);
void fb_image_set_bg(int r, int g, int b);
int get_image_size(char *filename, int *w, int *h);

#endif
