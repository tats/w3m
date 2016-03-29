/* $Id: fb.c,v 1.16 2003/07/13 16:19:10 ukai Exp $ */
/**************************************************************************
                fb.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#if defined(__linux__)
#include <linux/fb.h>
#elif defined(__FreeBSD__)
#include <sys/fbio.h> 
#endif
#if defined(__FreeBSD__)
#include <sys/types.h>
#include <machine/param.h>
#endif

#include "fb.h"

#define FB_ENV		"FRAMEBUFFER"
#if defined(__linux__)
#define	FB_DEFDEV	"/dev/fb0"
#elif defined(__FreeBSD__)
#define	FB_DEFDEV	"/dev/ttyv0"
#endif

#define MONO_OFFSET_8BIT  0x40
#define COLORS_MONO_8BIT  0x40
#define MONO_MASK_8BIT    0xFC
#define MONO_SHIFT_8BIT   2

#define COLOR_OFFSET_8BIT 0x80
#define COLORS_8BIT       0x80
#define RED_MASK_8BIT     0xC0
#define GREEN_MASK_8BIT   0xE0
#define BLUE_MASK_8BIT    0xC0
#define RED_SHIFT_8BIT    1
#define GREEN_SHIFT_8BIT  3
#define BLUE_SHIFT_8BIT   6

#define FALSE 0
#define TRUE  1

#define IMAGE_SIZE_MAX 10000

#if defined(__linux__)
static struct fb_cmap *fb_cmap_create(struct fb_fix_screeninfo *,
				      struct fb_var_screeninfo *);
#elif defined(__FreeBSD__)
static video_color_palette_t *fb_cmap_create(video_info_t *video_info,
				video_adapter_info_t *video_adapter_info);
#endif
#if defined(__linux__)
static void fb_cmap_destroy(struct fb_cmap *cmap);
#elif defined(__FreeBSD__)
static void fb_cmap_destroy(video_color_palette_t *cmap);
#endif
#if defined(__linux__)
static int fb_fscrn_get(int fbfp, struct fb_fix_screeninfo *scinfo);
#endif
#if defined(__linux__)
static void *fb_mmap(int fbfp, struct fb_fix_screeninfo *scinfo);
#elif defined(__FreeBSD__)
static void *fb_mmap(int fbfp, video_adapter_info_t *video_adapter_info);
#endif
#if defined(__linux__)
static int fb_munmap(void *buf, struct fb_fix_screeninfo *scinfo);
#elif defined(__FreeBSD__)
static int fb_munmap(void *buf, video_adapter_info_t *video_adapter_info);
#endif
#if defined(__linux__)
static int fb_vscrn_get(int fbfp, struct fb_var_screeninfo *scinfo);
#endif
#if defined(__linux__)
static int fb_cmap_set(int fbfp, struct fb_cmap *cmap);
#elif defined(__FreeBSD__)
static int fb_cmap_set(int fbfp, video_color_palette_t *cmap);
#endif
#if defined(__linux__)
static int fb_cmap_get(int fbfp, struct fb_cmap *cmap);
#elif defined(__FreeBSD__)
static int fb_cmap_get(int fbfp, video_color_palette_t *cmap);
#endif
static int fb_cmap_init(void);
static int fb_get_cmap_index(int r, int g, int b);
static unsigned long fb_get_packed_color(int r, int g, int b);
#if defined(__FreeBSD__)
static int fb_video_mode_get(int fbfp, int *video_mode);
static int fb_video_info_get(int fbfp, video_info_t *video_info);
static int fb_video_adapter_info_get(int fbfp, video_adapter_info_t *video_adapter_info);
#endif

#if defined(__linux__)
static struct fb_fix_screeninfo fscinfo;
static struct fb_var_screeninfo vscinfo;
#elif defined(__FreeBSD__)
static video_info_t video_info;
static video_adapter_info_t video_adapter_info;
#endif
#if defined(__linux__)
static struct fb_cmap *cmap = NULL, *cmap_org = NULL;
#elif defined(__FreeBSD__)
static video_color_palette_t *cmap = NULL, *cmap_org = NULL;
#endif
static int is_open = FALSE;
static int fbfp = -1;
static size_t pixel_size = 0;
static unsigned char *buf = NULL;

int
fb_open(void)
{
    char *fbdev = { FB_DEFDEV };
#if defined(__FreeBSD__)
    int video_mode;
#endif

    if (is_open == TRUE)
	return 1;

    if (getenv(FB_ENV)) {
	fbdev = getenv(FB_ENV);
    }

    if ((fbfp = open(fbdev, O_RDWR)) == -1) {
	fprintf(stderr, "open %s error\n", fbdev);
	goto ERR_END;
    }

#if defined(__linux__)
    if (fb_fscrn_get(fbfp, &fscinfo)) {
	goto ERR_END;
    }

    if (fb_vscrn_get(fbfp, &vscinfo)) {
	goto ERR_END;
    }
#elif defined(__FreeBSD__)
    if (fb_video_mode_get(fbfp, &video_mode)) {
	goto ERR_END;
    }
    video_info.vi_mode = video_mode;

    if (fb_video_info_get(fbfp, &video_info)) {
	goto ERR_END;
    }

    if (fb_video_adapter_info_get(fbfp, &video_adapter_info)) {
	goto ERR_END;
    }
    if (!(video_info.vi_flags & V_INFO_GRAPHICS) ||
	!(video_info.vi_flags & V_INFO_LINEAR)) {
	goto ERR_END;
    }
#endif

#if defined(__linux__)
    if ((cmap = fb_cmap_create(&fscinfo, &vscinfo)) == (struct fb_cmap *)-1) {
	goto ERR_END;
    }
#elif defined(__FreeBSD__)
    if ((cmap = fb_cmap_create(&video_info, &video_adapter_info)) == (video_color_palette_t *)-1) {
	goto ERR_END;
    }
#endif

#if defined(__linux__)
    if (!(buf = fb_mmap(fbfp, &fscinfo))) {
	fprintf(stderr, "Can't allocate memory.\n");
	goto ERR_END;
    }
#elif defined(__FreeBSD__)
    if (!(buf = fb_mmap(fbfp, &video_adapter_info))) {
	fprintf(stderr, "Can't allocate memory.\n");
	goto ERR_END;
    }
#endif

#if defined(__linux__)
    if (fscinfo.type != FB_TYPE_PACKED_PIXELS) {
	fprintf(stderr, "This type of framebuffer is not supported.\n");
	goto ERR_END;
    }
#elif defined(__FreeBSD__)
    if (!(video_info.vi_mem_model == V_INFO_MM_PACKED || 
	  video_info.vi_mem_model == V_INFO_MM_DIRECT)) {
	fprintf(stderr, "This type of framebuffer is not supported.\n");
	goto ERR_END;
    }
#endif

#if defined(__linux__)
    if (fscinfo.visual == FB_VISUAL_PSEUDOCOLOR && vscinfo.bits_per_pixel == 8) {
#elif defined(__FreeBSD__)
    if (video_adapter_info.va_flags & V_ADP_PALETTE &&
	video_info.vi_mem_model == V_INFO_MM_PACKED &&
	video_info.vi_depth == 8) {
#else
    if (0) {
#endif
	if (fb_cmap_get(fbfp, cmap)) {
	    fprintf(stderr, "Can't get color map.\n");
	    fb_cmap_destroy(cmap);
	    cmap = NULL;
	    goto ERR_END;
	}

	if (fb_cmap_init())
	    goto ERR_END;

	pixel_size = 1;
    }
#if defined(__linux__)
    else if ((fscinfo.visual == FB_VISUAL_TRUECOLOR ||
	      fscinfo.visual == FB_VISUAL_DIRECTCOLOR) &&
	     (vscinfo.bits_per_pixel == 15 ||
	      vscinfo.bits_per_pixel == 16 ||
	      vscinfo.bits_per_pixel == 24 || vscinfo.bits_per_pixel == 32)) {
	pixel_size = (vscinfo.bits_per_pixel + 7) / CHAR_BIT;
    }
#elif defined(__FreeBSD__)
    else if (video_info.vi_mem_model == V_INFO_MM_DIRECT &&
	     (video_info.vi_depth == 15 ||
	      video_info.vi_depth == 16 ||
	      video_info.vi_depth == 24 || video_info.vi_depth == 32)) {
	pixel_size = (video_info.vi_depth + 7) / CHAR_BIT;
    }
#endif
    else {
	fprintf(stderr, "This type of framebuffer is not supported.\n");
	goto ERR_END;
    }

    is_open = TRUE;
    return 0;

  ERR_END:
    fb_close();
    return 1;
}

void
fb_close(void)
{
    if (is_open != TRUE)
	return;

    if (cmap != NULL) {
	fb_cmap_destroy(cmap);
	cmap = NULL;
    }
    if (cmap_org != NULL) {
	fb_cmap_set(fbfp, cmap_org);
	fb_cmap_destroy(cmap_org);
	cmap = NULL;
    }
    if (buf != NULL) {
#if defined(__linux__)
	fb_munmap(buf, &fscinfo);
#elif defined(__FreeBSD__)
	fb_munmap(buf, &video_adapter_info);
#endif
	buf = NULL;
    }

    if (fbfp >= 0) {
	close(fbfp);
    }

    is_open = FALSE;
}

/***********   fb_image_*  ***********/

FB_IMAGE *
fb_image_new(int width, int height)
{
    FB_IMAGE *image;

    if (is_open != TRUE)
	return NULL;

    if (width > IMAGE_SIZE_MAX || height > IMAGE_SIZE_MAX || width < 1
	|| height < 1)
	return NULL;

    image = malloc(sizeof(*image));
    if (image == NULL)
	return NULL;

    image->data = calloc(sizeof(*(image->data)), width * height * pixel_size);
    if (image->data == NULL) {
	free(image);
	return NULL;
    }

    image->num = 1;
    image->id = 0;
    image->delay = 0;

    image->width = width;
    image->height = height;
    image->rowstride = width * pixel_size;
    image->len = width * height * pixel_size;

    return image;
}

void
fb_image_free(FB_IMAGE * image)
{
    if (image == NULL)
	return;

    if (image->data != NULL)
	free(image->data);

    free(image);
}

void
fb_image_pset(FB_IMAGE * image, int x, int y, int r, int g, int b)
{
    unsigned long work;

    if (image == NULL || is_open != TRUE || x >= image->width
	|| y >= image->height)
	return;

    work = fb_get_packed_color(r, g, b);
    memcpy(image->data + image->rowstride * y + pixel_size * x, &work,
	   pixel_size);
}

void
fb_image_fill(FB_IMAGE * image, int r, int g, int b)
{
    unsigned long work;
    int offset;

    if (image == NULL || is_open != TRUE)
	return;

    work = fb_get_packed_color(r, g, b);

    for (offset = 0; offset < image->len; offset += pixel_size) {
	memcpy(image->data + offset, &work, pixel_size);
    }
}

int
fb_image_draw(FB_IMAGE * image, int x, int y, int sx, int sy, int width,
	      int height)
{
    int i, offset_fb, offset_img;

    if (image == NULL || is_open != TRUE ||
	sx > image->width || sy > image->height ||
	x > fb_width() || y > fb_height())
	return 1;

    if (sx + width > image->width)
	width = image->width - sx;

    if (sy + height > image->height)
	height = image->height - sy;

    if (x + width > fb_width())
	width = fb_width() - x;

    if (y + height > fb_height())
	height = fb_height() - y;

#if defined(__linux__)
    offset_fb = fscinfo.line_length * y + pixel_size * x;
#elif defined(__FreeBSD__)
    offset_fb = video_adapter_info.va_line_width * y + pixel_size * x;
#endif
    offset_img = image->rowstride * sy + pixel_size * sx;
    for (i = 0; i < height; i++) {
	memcpy(buf + offset_fb, image->data + offset_img, pixel_size * width);
#if defined(__linux__)
	offset_fb += fscinfo.line_length;
#elif defined(__FreeBSD__)
	offset_fb += video_adapter_info.va_line_width;
#endif
	offset_img += image->rowstride;
    }

    return 0;
}

void
fb_image_copy(FB_IMAGE * dest, FB_IMAGE * src)
{
    if (dest == NULL || src == NULL)
	return;

    if (dest->len != src->len)
	return;

    memcpy(dest->data, src->data, src->len);
}

/***********   fb_frame_*  ***********/

FB_IMAGE **
fb_frame_new(int w, int h, int n)
{
    FB_IMAGE **frame;
    int i, error = 0;

    if (w > IMAGE_SIZE_MAX || h > IMAGE_SIZE_MAX || w < 1 || h < 1 || n < 1)
	return NULL;

    frame = malloc(sizeof(*frame) * n);
    if (frame == NULL)
	return NULL;

    for (i = 0; i < n; i++) {
	frame[i] = fb_image_new(w, h);
	frame[i]->num = n;
	frame[i]->id = i;
	frame[i]->delay = 1000;
	if (frame[i] == NULL)
	    error = 1;
    }

    if (error) {
	fb_frame_free(frame);
	return NULL;
    }

    return frame;
}


void
fb_frame_free(FB_IMAGE ** frame)
{
    int i, n;

    if (frame == NULL)
	return;

    n = frame[0]->num;
    for (i = 0; i < n; i++) {
	fb_image_free(frame[i]);
    }
    free(frame);
}

int
fb_width(void)
{
    if (is_open != TRUE)
	return 0;

#if defined(__linux__)
    return vscinfo.xres;
#elif defined(__FreeBSD__)
    return video_info.vi_width;
#endif
}

int
fb_height(void)
{
    if (is_open != TRUE)
	return 0;

#if defined(__linux__)
    return vscinfo.yres;
#elif defined(__FreeBSD__)
    return video_info.vi_height;
#endif
}

int
fb_clear(int x, int y, int w, int h, int r, int g, int b)
{
    int i, offset_fb;
    static int rr = -1, gg = -1, bb = -1;
    static char *tmp = NULL;

    if (is_open != TRUE || x > fb_width() || y > fb_height())
	return 1;

    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;

    if (x + w > fb_width())
	w = fb_width() - x;
    if (y + h > fb_height())
	h = fb_height() - y;

    if (tmp == NULL) {
#if defined(__linux__)
	tmp = malloc(fscinfo.line_length);
#elif defined(__FreeBSD__)
	tmp = malloc(video_adapter_info.va_line_width);
#endif
	if (tmp == NULL)
	    return 1;
    }
    if (rr != r || gg != g || bb != b) {
	unsigned long work;
	int ww = fb_width();

	work = fb_get_packed_color(r, g, b);
	for (i = 0; i < ww; i++)
	    memcpy(tmp + pixel_size * i, &work, pixel_size);
	rr = r;
	gg = g;
	bb = b;
    }
#if defined(__linux__)
    offset_fb = fscinfo.line_length * y + pixel_size * x;
#elif defined(__FreeBSD__)
    offset_fb = video_adapter_info.va_line_width * y + pixel_size * x;
#endif
    for (i = 0; i < h; i++) {
	memcpy(buf + offset_fb, tmp, pixel_size * w);
#if defined(__linux__)
	offset_fb += fscinfo.line_length;
#elif defined(__FreeBSD__)
	offset_fb += video_adapter_info.va_line_width;
#endif
    }
    return 0;
}

/********* static functions **************/
static unsigned long
fb_get_packed_color(int r, int g, int b)
{
    if (pixel_size == 1) {
	return fb_get_cmap_index(r, g, b);
    }
    else {
#if defined(__linux__)
	return
	    ((r >> (CHAR_BIT - vscinfo.red.length)) << vscinfo.red.offset) +
	    ((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.
	     offset) +
	    ((b >> (CHAR_BIT - vscinfo.blue.length)) << vscinfo.blue.offset);
#elif defined(__FreeBSD__)
	return
	    ((r >> (CHAR_BIT - video_info.vi_pixel_fsizes[0])) <<
	     video_info.vi_pixel_fields[0]) +
	    ((g >> (CHAR_BIT - video_info.vi_pixel_fsizes[1])) <<
	     video_info.vi_pixel_fields[1]) +
	    ((b >> (CHAR_BIT - video_info.vi_pixel_fsizes[2])) <<
	     video_info.vi_pixel_fields[2]);
#endif
    }
}

static int
fb_get_cmap_index(int r, int g, int b)
{
    int work;
    if ((r & GREEN_MASK_8BIT) == (g & GREEN_MASK_8BIT)
	&& (g & GREEN_MASK_8BIT) == (b & GREEN_MASK_8BIT)) {
	work = (r >> MONO_SHIFT_8BIT) + MONO_OFFSET_8BIT;
    }
    else {
	work = ((r & RED_MASK_8BIT) >> RED_SHIFT_8BIT)
	    + ((g & GREEN_MASK_8BIT) >> GREEN_SHIFT_8BIT)
	    + ((b & BLUE_MASK_8BIT) >> BLUE_SHIFT_8BIT)
	    + COLOR_OFFSET_8BIT;
    }
    return work;
}

static int
fb_cmap_init(void)
{
    int lp;

    if (cmap == NULL)
	return 1;

#if defined(__linux__)
    if (cmap->len < COLOR_OFFSET_8BIT + COLORS_8BIT) {
	fprintf(stderr, "Can't allocate enough color.\n");
	return 1;
    }
#elif defined(__FreeBSD__)
    if (cmap->count < COLOR_OFFSET_8BIT + COLORS_8BIT) {
	fprintf(stderr, "Can't allocate enough color.\n");
	return 1;
    }
#endif

    if (cmap_org == NULL) {
#if defined(__linux__)
	if ((cmap_org =
	     fb_cmap_create(&fscinfo, &vscinfo)) == (struct fb_cmap *)-1) {
	    return 1;
	}
#elif defined(__FreeBSD__)
	if ((cmap_org =
	     fb_cmap_create(&video_info, &video_adapter_info)) ==
	     (video_color_palette_t *)-1) {
	    return 1;
	}
#endif

	if (fb_cmap_get(fbfp, cmap_org)) {
	    fprintf(stderr, "Can't get color map.\n");
	    fb_cmap_destroy(cmap_org);
	    cmap_org = NULL;
	    return 1;
	}
    }

#if defined(__linux__)
    cmap->start = MONO_OFFSET_8BIT;
    cmap->len = COLORS_8BIT + COLORS_MONO_8BIT;
#elif defined(__FreeBSD__)
    cmap->index = MONO_OFFSET_8BIT;
    cmap->count = COLORS_8BIT + COLORS_MONO_8BIT;
#endif

    for (lp = 0; lp < COLORS_MONO_8BIT; lp++) {
	int c;
	c = (lp << (MONO_SHIFT_8BIT + 8)) +
	    (lp ? (0xFFFF - (MONO_MASK_8BIT << 8)) : 0);
	if (cmap->red)
	    *(cmap->red + lp) = c;
	if (cmap->green)
	    *(cmap->green + lp) = c;
	if (cmap->blue)
	    *(cmap->blue + lp) = c;
    }

    for (lp = 0; lp < COLORS_8BIT; lp++) {
	int r, g, b;
	r = lp & (RED_MASK_8BIT >> RED_SHIFT_8BIT);
	g = lp & (GREEN_MASK_8BIT >> GREEN_SHIFT_8BIT);
	b = lp & (BLUE_MASK_8BIT >> BLUE_SHIFT_8BIT);
	if (cmap->red)
	    *(cmap->red + lp + COLORS_MONO_8BIT)
		= (r << (RED_SHIFT_8BIT + 8)) +
		(r ? (0xFFFF - (RED_MASK_8BIT << 8)) : 0);
	if (cmap->green)
	    *(cmap->green + lp + COLORS_MONO_8BIT)
		= (g << (GREEN_SHIFT_8BIT + 8)) +
		(g ? (0xFFFF - (GREEN_MASK_8BIT << 8)) : 0);
	if (cmap->blue)
	    *(cmap->blue + lp + COLORS_MONO_8BIT)
		= (b << (BLUE_SHIFT_8BIT + 8)) +
		(b ? (0xFFFF - (BLUE_MASK_8BIT << 8)) : 0);
    }

    if (fb_cmap_set(fbfp, cmap)) {
	fb_cmap_destroy(cmap);
	cmap = NULL;
	fprintf(stderr, "Can't set color map.\n");
	return 1;
    }
    return 0;
}

/*
 * (struct fb_cmap) Device independent colormap information.
 * 
 * fb_cmap_create()     create colormap information
 * fb_cmap_destroy()    destroy colormap information
 * fb_cmap_get()        get information
 * fb_cmap_set()        set information
 */

#define	LUT_MAX		(256)

#if defined(__linux__)
static struct fb_cmap *
fb_cmap_create(struct fb_fix_screeninfo *fscinfo,
	       struct fb_var_screeninfo *vscinfo)
#elif defined(__FreeBSD__)
static video_color_palette_t *
fb_cmap_create(video_info_t *video_info,
	       video_adapter_info_t *video_adapter_info)
#endif
{
#if defined(__linux__)
    struct fb_cmap *cmap;
#elif defined(__FreeBSD__)
    video_color_palette_t *cmap;
#endif
    int cmaplen = LUT_MAX;

    /* check the existence of colormap */
#if defined(__linux__)
    if (fscinfo->visual == FB_VISUAL_MONO01 ||
	fscinfo->visual == FB_VISUAL_MONO10 ||
	fscinfo->visual == FB_VISUAL_TRUECOLOR)
	return NULL;
#elif defined(__FreeBSD__)
    if (!(video_adapter_info->va_flags & V_ADP_PALETTE))
	return NULL;
#endif

#if defined(__linux__)
    cmap = (struct fb_cmap *)malloc(sizeof(struct fb_cmap));
#elif defined(__FreeBSD__)
    cmap = (video_color_palette_t *)malloc(sizeof(video_color_palette_t));
#endif
    if (!cmap) {
	perror("cmap malloc error\n");
#if defined(__linux__)
	return (struct fb_cmap *)-1;
#elif defined(__FreeBSD__)
	return (video_color_palette_t *)-1;
#endif
    }
#if defined(__linux__)
    memset(cmap, 0, sizeof(struct fb_cmap));
#elif defined(__FreeBSD__)
    memset(cmap, 0, sizeof(video_color_palette_t));
#endif

#if defined(__FreeBSD__)
    if (video_info->vi_mem_model == V_INFO_MM_PACKED) {
	cmap->red = (u_char *) malloc(sizeof(u_char) * cmaplen);
	if (!cmap->red) {
	    perror("red lut malloc error\n");
	    return (video_color_palette_t *)-1;
	}
	cmap->green = (u_char *) malloc(sizeof(u_char) * cmaplen);
	if (!cmap->green) {
	    perror("green lut malloc error\n");
	    free(cmap->red);
	    return (video_color_palette_t *)-1;
	}
	cmap->blue = (u_char *) malloc(sizeof(u_char) * cmaplen);
	if (!cmap->blue) {
	    perror("blue lut malloc error\n");
	    free(cmap->red);
	    free(cmap->green);
	    return (video_color_palette_t *)-1;
	}
	cmap->transparent = (u_char *) malloc(sizeof(u_char) * cmaplen);
	if (!cmap->transparent) {
	    perror("transparent lut malloc error\n");
	    free(cmap->red);
	    free(cmap->green);
	    free(cmap->blue);
	    return (video_color_palette_t *)-1;
	}
	cmap->count = cmaplen;
	return cmap;
    }
#endif

    /* Allocates memory for a colormap */
#if defined(__linux__)
    if (vscinfo->red.length) {
	cmap->red = (__u16 *) malloc(sizeof(__u16) * cmaplen);
#elif defined(__FreeBSD__)
    if (video_info->vi_pixel_fsizes[0]) {
	cmap->red = (u_char *) malloc(sizeof(u_char) * cmaplen);
#else
    if (0) {
#endif
	if (!cmap->red) {
	    perror("red lut malloc error\n");
#if defined(__linux__)
	    return (struct fb_cmap *)-1;
#elif defined(__FreeBSD__)
	    return (video_color_palette_t *)-1;
#endif
	}
    }
#if defined(__linux__)
    if (vscinfo->green.length) {
	cmap->green = (__u16 *) malloc(sizeof(__u16) * cmaplen);
#elif defined(__FreeBSD__)
    if (video_info->vi_pixel_fsizes[1]) {
	cmap->green = (u_char *) malloc(sizeof(u_char) * cmaplen);
#else
    if (0) {
#endif
	if (!cmap->green) {
#if defined(__linux__)
	    if (vscinfo->red.length)
		free(cmap->red);
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[0])
		free(cmap->red);
#endif
	    perror("green lut malloc error\n");
#if defined(__linux__)
	    return (struct fb_cmap *)-1;
#elif defined(__FreeBSD__)
	    return (video_color_palette_t *)-1;
#endif
	}
    }
#if defined(__linux__)
    if (vscinfo->blue.length) {
	cmap->blue = (__u16 *) malloc(sizeof(__u16) * cmaplen);
#elif defined(__FreeBSD__)
    if (video_info->vi_pixel_fsizes[2]) {
	cmap->blue = (u_char *) malloc(sizeof(u_char) * cmaplen);
#else
    if (0) {
#endif
	if (!cmap->blue) {
#if defined(__linux__)
	    if (vscinfo->red.length)
		free(cmap->red);
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[0])
		free(cmap->red);
#endif
#if defined(__linux__)
	    if (vscinfo->green.length)
		free(cmap->green);
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[1])
		free(cmap->green);
#endif
	    perror("blue lut malloc error\n");
#if defined(__linux__)
	    return (struct fb_cmap *)-1;
#elif defined(__FreeBSD__)
	    return (video_color_palette_t *)-1;
#endif
	}
    }
#if defined(__linux__)
    if (vscinfo->transp.length) {
	cmap->transp = (__u16 *) malloc(sizeof(__u16) * cmaplen);
#elif defined(__FreeBSD__)
    if (video_info->vi_pixel_fsizes[3]) {
	cmap->transparent = (u_char *) malloc(sizeof(u_char) * cmaplen);
#else
    if (0) {
#endif
#if defined(__linux__)
	if (!cmap->transp) {
#elif defined(__FreeBSD__)
	if (!cmap->transparent) {
#else
	if (0) {
#endif
#if defined(__linux__)
	    if (vscinfo->red.length)
		free(cmap->red);
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[0])
		free(cmap->red);
#endif
#if defined(__linux__)
	    if (vscinfo->green.length)
		free(cmap->green);
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[1])
		free(cmap->green);
#endif
#if defined(__linux__)
	    if (vscinfo->blue.length)
		free(cmap->blue);
	    perror("transp lut malloc error\n");
#elif defined(__FreeBSD__)
	    if (video_info->vi_pixel_fsizes[2])
		free(cmap->blue);
	    perror("transparent lut malloc error\n");
#endif
#if defined(__linux__)
	    return (struct fb_cmap *)-1;
#elif defined(__FreeBSD__)
	    return (video_color_palette_t *)-1;
#endif
	}
    }
#if defined(__linux__)
    cmap->len = cmaplen;
#elif defined(__FreeBSD__)
    cmap->count = cmaplen;
#endif
    return cmap;
}

#if defined(__linux__)
static void
fb_cmap_destroy(struct fb_cmap *cmap)
#elif defined(__FreeBSD__)
static void
fb_cmap_destroy(video_color_palette_t *cmap)
#endif
{
    if (cmap->red)
	free(cmap->red);
    if (cmap->green)
	free(cmap->green);
    if (cmap->blue)
	free(cmap->blue);
#if defined(__linux__)
    if (cmap->transp)
	free(cmap->transp);
#elif defined(__FreeBSD__)
    if (cmap->transparent)
	free(cmap->transparent);
#endif
    free(cmap);
}

#if defined(__linux__)
static int
fb_cmap_get(int fbfp, struct fb_cmap *cmap)
#elif defined(__FreeBSD__)
static int
fb_cmap_get(int fbfp, video_color_palette_t *cmap)
#endif
{
#if defined(__linux__)
    if (ioctl(fbfp, FBIOGETCMAP, cmap)) {
	perror("ioctl FBIOGETCMAP error\n");
	return -1;
    }
#elif defined(__FreeBSD__)
    if (ioctl(fbfp, FBIO_GETPALETTE, cmap) == -1) {
	perror("ioctl FBIO_GETPALETTE error\n");
	return -1;
    }
#endif
    return 0;
}

#if defined(__linux__)
static int
fb_cmap_set(int fbfp, struct fb_cmap *cmap)
#elif defined(__FreeBSD__)
static int
fb_cmap_set(int fbfp, video_color_palette_t *cmap)
#endif
{
#if defined(__linux__)
    if (ioctl(fbfp, FBIOPUTCMAP, cmap)) {
	perror("ioctl FBIOPUTCMAP error\n");
	return -1;
    }
#elif defined(__FreeBSD__)
    if (ioctl(fbfp, FBIO_SETPALETTE, cmap) == -1) {
	perror("ioctl FBIO_SETPALETTE error\n");
	return -1;
    }
#endif
    return 0;
}

/*
 * access to framebuffer
 * 
 * fb_mmap()            map from framebuffer into memory
 * fb_munmap()          deletes the mappings
 */

#if defined(__linux__)
static void *
fb_mmap(int fbfp, struct fb_fix_screeninfo *scinfo)
#elif defined(__FreeBSD__)
static void *
fb_mmap(int fbfp, video_adapter_info_t *video_adapter_info)
#endif
{
    void *buf;
#if defined(__linux__)
    if ((buf = (unsigned char *)
	 mmap(NULL, scinfo->smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfp,
	      (off_t) 0))
	== MAP_FAILED) {
	perror("mmap error");
	return NULL;
    }
#elif defined(__FreeBSD__)
    size_t mmap_offset;
    size_t mmap_length;
    mmap_offset = (size_t)(video_adapter_info->va_window) & (PAGE_MASK);
    mmap_length = (size_t)(video_adapter_info->va_window_size +
			   mmap_offset + PAGE_MASK) & (~PAGE_MASK);
    if ((buf = (unsigned char *)
	 mmap(NULL, mmap_length, PROT_READ | PROT_WRITE, MAP_SHARED, fbfp,
	      (off_t) 0))
	== MAP_FAILED) {
	perror("mmap error");
	return NULL;
    }
#endif
    return buf;
}

#if defined(__linux__)
static int
fb_munmap(void *buf, struct fb_fix_screeninfo *scinfo)
#elif defined(__FreeBSD__)
static int
fb_munmap(void *buf, video_adapter_info_t *video_adapter_info)
#endif
{
#if defined(__linux__)
    return munmap(buf, scinfo->smem_len);
#elif defined(__FreeBSD__)
    size_t mmap_offset;
    size_t mmap_length;
    mmap_offset = (size_t)(video_adapter_info->va_window) & (PAGE_MASK);
    mmap_length = (size_t)(video_adapter_info->va_window_size +
			   mmap_offset + PAGE_MASK) & (~PAGE_MASK);
    return munmap((void *)((u_long)buf & (~PAGE_MASK)), mmap_length);
#endif
}

/*
 * (struct fb_fix_screeninfo) device independent fixed information
 * 
 * fb_fscrn_get()               get information
 */
#if defined(__linux__)
static int
fb_fscrn_get(int fbfp, struct fb_fix_screeninfo *scinfo)
{
    if (ioctl(fbfp, FBIOGET_FSCREENINFO, scinfo)) {
	perror("ioctl FBIOGET_FSCREENINFO error\n");
	return -1;
    }
    return 0;
}
#endif

/*
 * (struct fb_var_screeninfo) device independent variable information
 * 
 * fb_vscrn_get()               get information
 */
#if defined(__linux__)
static int
fb_vscrn_get(int fbfp, struct fb_var_screeninfo *scinfo)
{
    if (ioctl(fbfp, FBIOGET_VSCREENINFO, scinfo)) {
	perror("ioctl FBIOGET_VSCREENINFO error\n");
	return -1;
    }
    return 0;
}
#endif

#if defined(__FreeBSD__)
static int
fb_video_mode_get(int fbfp, int *video_mode)
{
    if (ioctl(fbfp, FBIO_GETMODE, video_mode) == -1) {
	perror("ioctl FBIO_GETMODE error\n");
	return -1;
    }
    return 0;
}
#endif

#if defined(__FreeBSD__)
static int
fb_video_info_get(int fbfp, video_info_t *video_info)
{
    if (ioctl(fbfp, FBIO_MODEINFO, video_info) == -1) {
	perror("ioctl FBIO_MODEINFO error\n");
	return -1;
    }
    return 0;
}
#endif

#if defined(__FreeBSD__)
static int
fb_video_adapter_info_get(int fbfp, video_adapter_info_t *video_adapter_info)
{
    if (ioctl(fbfp, FBIO_ADPINFO, video_adapter_info) == -1) {
	perror("ioctl FBIO_ADPINFO error\n");
	return -1;
    }
    return 0;
}
#endif

