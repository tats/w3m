/* $Id: fb_img.h,v 1.9 2004/08/04 17:32:28 ukai Exp $ */
#ifndef fb_img_header
#define fb_img_header
#include "fb.h"

void fb_image_init();
FB_IMAGE **fb_image_load(char *filename, int w, int h, int n);
void fb_image_set_bg(int r, int g, int b);
int fb_image_clear(int x, int y, int w, int h);
int get_image_size(char *filename, int *w, int *h);

#endif
