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
#include <linux/fb.h>

#include "fb.h"

#define FB_ENV		"FRAMEBUFFER"
#define	FB_DEFDEV	"/dev/fb0"

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

static struct fb_cmap *fb_cmap_create(struct fb_fix_screeninfo *,
				      struct fb_var_screeninfo *);
static void fb_cmap_destroy(struct fb_cmap *cmap);
static int fb_fscrn_get(int fbfp, struct fb_fix_screeninfo *scinfo);
static void *fb_mmap(int fbfp, struct fb_fix_screeninfo *scinfo);
static int fb_munmap(void *buf, struct fb_fix_screeninfo *scinfo);
static int fb_vscrn_get(int fbfp, struct fb_var_screeninfo *scinfo);
static int fb_cmap_set(int fbfp, struct fb_cmap *cmap);
static int fb_cmap_get(int fbfp, struct fb_cmap *cmap);
static int fb_cmap_init(void);
static int fb_get_cmap_index(int r, int g, int b);
static unsigned long fb_get_packed_color(int r, int g, int b);

static struct fb_fix_screeninfo fscinfo;
static struct fb_var_screeninfo vscinfo;
static struct fb_cmap *cmap = NULL, *cmap_org = NULL;
static int is_open = FALSE;
static int fbfp = -1;
static size_t pixel_size = 0;
static unsigned char *buf = NULL;

int
fb_open(void)
{
    char *fbdev = { FB_DEFDEV };

    if (is_open == TRUE)
	return 1;

    if (getenv(FB_ENV)) {
	fbdev = getenv(FB_ENV);
    }

    if ((fbfp = open(fbdev, O_RDWR)) == -1) {
	fprintf(stderr, "open %s error\n", fbdev);
	goto ERR_END;
    }

    if (fb_fscrn_get(fbfp, &fscinfo)) {
	goto ERR_END;
    }

    if (fb_vscrn_get(fbfp, &vscinfo)) {
	goto ERR_END;
    }

    if ((cmap = fb_cmap_create(&fscinfo, &vscinfo)) == (struct fb_cmap *)-1) {
	goto ERR_END;
    }

    if (!(buf = fb_mmap(fbfp, &fscinfo))) {
	fprintf(stderr, "Can't allocate memory.\n");
	goto ERR_END;
    }

    if (fscinfo.type != FB_TYPE_PACKED_PIXELS) {
	fprintf(stderr, "This type of framebuffer is not supported.\n");
	goto ERR_END;
    }

    if (fscinfo.visual == FB_VISUAL_PSEUDOCOLOR && vscinfo.bits_per_pixel == 8) {
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
    else if ((fscinfo.visual == FB_VISUAL_TRUECOLOR ||
	      fscinfo.visual == FB_VISUAL_DIRECTCOLOR) &&
	     (vscinfo.bits_per_pixel == 15 ||
	      vscinfo.bits_per_pixel == 16 ||
	      vscinfo.bits_per_pixel == 24 || vscinfo.bits_per_pixel == 32)) {
	pixel_size = (vscinfo.bits_per_pixel + 7) / CHAR_BIT;
    }
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
	fb_munmap(buf, &fscinfo);
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

    offset_fb = fscinfo.line_length * y + pixel_size * x;
    offset_img = image->rowstride * sy + pixel_size * sx;
    for (i = 0; i < height; i++) {
	memcpy(buf + offset_fb, image->data + offset_img, pixel_size * width);
	offset_fb += fscinfo.line_length;
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

    return vscinfo.xres;
}

int
fb_height(void)
{
    if (is_open != TRUE)
	return 0;

    return vscinfo.yres;
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
	tmp = malloc(fscinfo.line_length);
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
    offset_fb = fscinfo.line_length * y + pixel_size * x;
    for (i = 0; i < h; i++) {
	memcpy(buf + offset_fb, tmp, pixel_size * w);
	offset_fb += fscinfo.line_length;
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
	return
	    ((r >> (CHAR_BIT - vscinfo.red.length)) << vscinfo.red.offset) +
	    ((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.
	     offset) +
	    ((b >> (CHAR_BIT - vscinfo.blue.length)) << vscinfo.blue.offset);
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

    if (cmap->len < COLOR_OFFSET_8BIT + COLORS_8BIT) {
	fprintf(stderr, "Can't allocate enough color.\n");
	return 1;
    }

    if (cmap_org == NULL) {
	if ((cmap_org =
	     fb_cmap_create(&fscinfo, &vscinfo)) == (struct fb_cmap *)-1) {
	    return 1;
	}

	if (fb_cmap_get(fbfp, cmap_org)) {
	    fprintf(stderr, "Can't get color map.\n");
	    fb_cmap_destroy(cmap_org);
	    cmap_org = NULL;
	    return 1;
	}
    }

    cmap->start = MONO_OFFSET_8BIT;
    cmap->len = COLORS_8BIT + COLORS_MONO_8BIT;

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

static struct fb_cmap *
fb_cmap_create(struct fb_fix_screeninfo *fscinfo,
	       struct fb_var_screeninfo *vscinfo)
{
    struct fb_cmap *cmap;
    int cmaplen = LUT_MAX;

    /* check the existence of colormap */
    if (fscinfo->visual == FB_VISUAL_MONO01 ||
	fscinfo->visual == FB_VISUAL_MONO10 ||
	fscinfo->visual == FB_VISUAL_TRUECOLOR)
	return NULL;

    cmap = (struct fb_cmap *)malloc(sizeof(struct fb_cmap));
    if (!cmap) {
	perror("cmap malloc error\n");
	return (struct fb_cmap *)-1;
    }
    memset(cmap, 0, sizeof(struct fb_cmap));

    /* Allocates memory for a colormap */
    if (vscinfo->red.length) {
	cmap->red = (__u16 *) malloc(sizeof(__u16) * cmaplen);
	if (!cmap->red) {
	    perror("red lut malloc error\n");
	    return (struct fb_cmap *)-1;
	}
    }
    if (vscinfo->green.length) {
	cmap->green = (__u16 *) malloc(sizeof(__u16) * cmaplen);
	if (!cmap->green) {
	    if (vscinfo->red.length)
		free(cmap->red);
	    perror("green lut malloc error\n");
	    return (struct fb_cmap *)-1;
	}
    }
    if (vscinfo->blue.length) {
	cmap->blue = (__u16 *) malloc(sizeof(__u16) * cmaplen);
	if (!cmap->blue) {
	    if (vscinfo->red.length)
		free(cmap->red);
	    if (vscinfo->green.length)
		free(cmap->green);
	    perror("blue lut malloc error\n");
	    return (struct fb_cmap *)-1;
	}
    }
    if (vscinfo->transp.length) {
	cmap->transp = (__u16 *) malloc(sizeof(__u16) * cmaplen);
	if (!cmap->transp) {
	    if (vscinfo->red.length)
		free(cmap->red);
	    if (vscinfo->green.length)
		free(cmap->green);
	    if (vscinfo->blue.length)
		free(cmap->blue);
	    perror("transp lut malloc error\n");
	    return (struct fb_cmap *)-1;
	}
    }
    cmap->len = cmaplen;
    return cmap;
}

static void
fb_cmap_destroy(struct fb_cmap *cmap)
{
    if (cmap->red)
	free(cmap->red);
    if (cmap->green)
	free(cmap->green);
    if (cmap->blue)
	free(cmap->blue);
    if (cmap->transp)
	free(cmap->transp);
    free(cmap);
}

static int
fb_cmap_get(int fbfp, struct fb_cmap *cmap)
{
    if (ioctl(fbfp, FBIOGETCMAP, cmap)) {
	perror("ioctl FBIOGETCMAP error\n");
	return -1;
    }
    return 0;
}

static int
fb_cmap_set(int fbfp, struct fb_cmap *cmap)
{
    if (ioctl(fbfp, FBIOPUTCMAP, cmap)) {
	perror("ioctl FBIOPUTCMAP error\n");
	return -1;
    }
    return 0;
}

/*
 * access to framebuffer
 * 
 * fb_mmap()            map from framebuffer into memory
 * fb_munmap()          deletes the mappings
 */

static void *
fb_mmap(int fbfp, struct fb_fix_screeninfo *scinfo)
{
    void *buf;
    if ((buf = (unsigned char *)
	 mmap(NULL, scinfo->smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfp,
	      (off_t) 0))
	== MAP_FAILED) {
	perror("mmap error");
	return NULL;
    }
    return buf;
}

static int
fb_munmap(void *buf, struct fb_fix_screeninfo *scinfo)
{
    return munmap(buf, scinfo->smem_len);
}

/*
 * (struct fb_fix_screeninfo) device independent fixed information
 * 
 * fb_fscrn_get()               get information
 */
static int
fb_fscrn_get(int fbfp, struct fb_fix_screeninfo *scinfo)
{
    if (ioctl(fbfp, FBIOGET_FSCREENINFO, scinfo)) {
	perror("ioctl FBIOGET_FSCREENINFO error\n");
	return -1;
    }
    return 0;
}

/*
 * (struct fb_var_screeninfo) device independent variable information
 * 
 * fb_vscrn_get()               get information
 */
static int
fb_vscrn_get(int fbfp, struct fb_var_screeninfo *scinfo)
{
    if (ioctl(fbfp, FBIOGET_VSCREENINFO, scinfo)) {
	perror("ioctl FBIOGET_VSCREENINFO error\n");
	return -1;
    }
    return 0;
}
