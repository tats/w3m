#ifndef fb_header
#define fb_header
#include <linux/fb.h>

int fb_open(void);
void fb_close(void);
void fb_pset(int x, int y, int r, int g, int b);
void fb_clear(void);
int fb_width(void);
int fb_height(void);
void fb_cmap_disp(void);
void fb_fscrn_disp(void);
void fb_vscrn_disp(void);
int fb_get_color(int x, int y, int *r, int *g, int *b);

#endif
