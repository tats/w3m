/* $Id: fb.c,v 1.6 2002/07/30 16:03:01 ukai Exp $ */
/**************************************************************************
                fb.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
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

static struct fb_fix_screeninfo fscinfo;
static struct fb_var_screeninfo vscinfo;
static struct fb_cmap *cmap = NULL;
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

    if (!((fscinfo.visual == FB_VISUAL_TRUECOLOR ||
	   fscinfo.visual == FB_VISUAL_DIRECTCOLOR) &&
	  (vscinfo.bits_per_pixel == 15 ||
	   vscinfo.bits_per_pixel == 16 ||
	   vscinfo.bits_per_pixel == 24 || vscinfo.bits_per_pixel == 32))) {
	fprintf(stderr, "This type of framebuffer is not supported.\n");
	goto ERR_END;
    }

    pixel_size = (vscinfo.bits_per_pixel + 7) / CHAR_BIT;

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
    int offset;

    if (image == NULL || is_open != TRUE || x >= image->width
	|| y >= image->height)
	return;

    offset = image->rowstride * y + pixel_size * x;

    work =
	((r >> (CHAR_BIT - vscinfo.red.length)) << vscinfo.red.offset) +
	((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.offset) +
	((b >> (CHAR_BIT - vscinfo.blue.length)) << vscinfo.blue.offset);

    memcpy(image->data + offset, &work, pixel_size);
}

void
fb_image_fill(FB_IMAGE * image, int r, int g, int b)
{
    unsigned long work;
    int offset;

    if (image == NULL || is_open != TRUE)
	return;

    work =
	((r >> (CHAR_BIT - vscinfo.red.length)) << vscinfo.red.offset) +
	((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.offset) +
	((b >> (CHAR_BIT - vscinfo.blue.length)) << vscinfo.blue.offset);

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
fb_image_rotate(FB_IMAGE * image, int direction)
{
    unsigned char *src, *dest, *tmp;
    int x, y, i, ofst;

    if (image == NULL)
	return;

    tmp = malloc(image->len);
    if (tmp == NULL)
	return;

    src = image->data;
    dest = tmp;

    if (direction) {
	int ofst2 = image->rowstride * (image->height - 1);
	for (x = 0; x < image->rowstride; x += pixel_size) {
	    ofst = ofst2 + x;
	    for (y = image->height - 1; y >= 0; y--) {
		memcpy(dest, src + ofst, pixel_size);
		dest += pixel_size;
		ofst -= image->rowstride;
	    }
	}
    } else {
	for (x = image->rowstride - pixel_size; x >= 0; x -= pixel_size) {
	    ofst = x;
	    for (y = 0; y < image->height; y++) {
		memcpy(dest, src + ofst, pixel_size);
		dest += pixel_size;
		ofst += image->rowstride;
	    }
	}
    }
    memcpy(src, tmp, image->len);
    i = image->width;
    image->width = image->height;
    image->height = i;
    image->rowstride = image->width * pixel_size;
    free(tmp);
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

void
fb_frame_rotate(FB_IMAGE ** frame, int direction)
{
    int i, n;

    n = frame[0]->num;
    for (i = 0; i < n; i++) {
	fb_image_rotate(frame[i], direction);
    }
}

void
fb_pset(int x, int y, int r, int g, int b)
{
    unsigned long work;
    int offset;

    if (is_open != TRUE || x >= vscinfo.xres || y >= vscinfo.yres)
	return;

    offset = fscinfo.line_length * y + pixel_size * x;

    if (offset >= fscinfo.smem_len)
	return;

    work =
	((r >> (CHAR_BIT - vscinfo.red.length)) << vscinfo.red.offset) +
	((g >> (CHAR_BIT - vscinfo.green.length)) << vscinfo.green.offset) +
	((b >> (CHAR_BIT - vscinfo.blue.length)) << vscinfo.blue.offset);
    memcpy(buf + offset, &work, pixel_size);
}

int
fb_get_color(int x, int y, int *r, int *g, int *b)
{
    unsigned long work = 0;
    int offset;

    if (is_open != TRUE || x >= vscinfo.xres || y >= vscinfo.yres)
	return 1;

    offset = fscinfo.line_length * y + pixel_size * x;

    if (offset >= fscinfo.smem_len)
	return 1;

    memcpy(&work, buf + offset, pixel_size);

    *r = ((work >> vscinfo.red.
	   offset) & (0x000000ff >> (CHAR_BIT - vscinfo.red.length)))
	<< (CHAR_BIT - vscinfo.red.length);
    *g = ((work >> vscinfo.green.
	   offset) & (0x000000ff >> (CHAR_BIT - vscinfo.green.length)))
	<< (CHAR_BIT - vscinfo.green.length);
    *b = ((work >> vscinfo.blue.
	   offset) & (0x000000ff >> (CHAR_BIT - vscinfo.blue.length)))
	<< (CHAR_BIT - vscinfo.blue.length);
    return 0;
}

void
fb_clear(void)
{
    if (is_open != TRUE)
	return;

    memset(buf, 0,
	   (vscinfo.xres * vscinfo.yres * vscinfo.bits_per_pixel) / CHAR_BIT);
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

void
fb_cmap_disp(void)
{
    int lp;

    if (is_open != TRUE)
	return;

    printf("cmap DUMP\n");
    printf("start		:[%08x]\n", cmap->start);
    printf("len		:[%08x]\n", cmap->len);
    printf("red		:[%8p]\n", cmap->red);
    if (cmap->red) {
	for (lp = 0; lp < cmap->len; lp++) {
	    if ((lp + 1) % 16 == 0)
		printf("%04x\n", *(cmap->red + lp));
	    else
		printf("%04x ", *(cmap->red + lp));
	}
	if (lp % 16)
	    printf("\n");
    }
    printf("green		:[%8p]\n", cmap->green);
    if (cmap->green) {
	for (lp = 0; lp < cmap->len; lp++) {
	    if ((lp + 1) % 16 == 0)
		printf("%04x\n", *(cmap->green + lp));
	    else
		printf("%04x ", *(cmap->green + lp));
	}
	if (lp % 16)
	    printf("\n");
    }
    printf("blue		:[%8p]\n", cmap->blue);
    if (cmap->blue) {
	for (lp = 0; lp < cmap->len; lp++) {
	    if ((lp + 1) % 16 == 0)
		printf("%04x\n", *(cmap->blue + lp));
	    else
		printf("%04x ", *(cmap->blue + lp));
	}
	if (lp % 16)
	    printf("\n");
    }
    printf("transp		:[%8p]\n", cmap->transp);
    if (cmap->transp) {
	for (lp = 0; lp < cmap->len; lp++) {
	    if ((lp + 1) % 16 == 0)
		printf("%04x\n", *(cmap->transp + lp));
	    else
		printf("%04x ", *(cmap->transp + lp));
	}
	if (lp % 16)
	    printf("\n");
    }
    return;
}


void
fb_fscrn_disp(void)
{
    if (is_open != TRUE)
	return;

    printf("scinfo[%8p] DUMP\n", &fscinfo);
    printf("id		:[%s]\n", fscinfo.id);
    printf("smem_start	:[%08lx]\n", fscinfo.smem_start);
    printf("smem_len	:[%d]\n", fscinfo.smem_len);
    printf("type		:[%d] ", fscinfo.type);
    switch (fscinfo.type) {
    case FB_TYPE_PACKED_PIXELS:
	printf("FB_TYPE_PACKED_PIXELS\n");
	break;
    case FB_TYPE_PLANES:
	printf("FB_TYPE_PLANES\n");
	break;
    case FB_TYPE_INTERLEAVED_PLANES:
	printf("FB_TYPE_INTERLEAVED_PLANES\n");
	break;
    case FB_TYPE_TEXT:
	printf("FB_TYPE_TEXT\n");
	break;
    default:
	printf("Unknown type.\n");
    }
    printf("type_aux	:[%d] ", fscinfo.type_aux);
    switch (fscinfo.type_aux) {
    case FB_AUX_TEXT_MDA:
	printf("FB_AUX_TEXT_MDA\n");
	break;
    case FB_AUX_TEXT_CGA:
	printf("FB_AUX_TEXT_CGA\n");
	break;
    case FB_AUX_TEXT_S3_MMIO:
	printf("FB_AUX_TEXT_S3_MMIO\n");
	break;
    case FB_AUX_TEXT_MGA_STEP16:
	printf("FB_AUX_TEXT_MGA_STEP16\n");
	break;
    case FB_AUX_TEXT_MGA_STEP8:
	printf("FB_AUX_TEXT_MGA_STEP8\n");
	break;
    default:
	printf("Unknown type_aux.\n");
    }
    printf("visual		:[%d] ", fscinfo.visual);
    switch (fscinfo.visual) {
    case FB_VISUAL_MONO01:
	printf("FB_VISUAL_MONO01\n");
	break;
    case FB_VISUAL_MONO10:
	printf("FB_VISUAL_MONO10\n");
	break;
    case FB_VISUAL_TRUECOLOR:
	printf("FB_VISUAL_TRUECOLOR\n");
	break;
    case FB_VISUAL_PSEUDOCOLOR:
	printf("FB_VISUAL_PSEUDOCOLOR\n");
	break;
    case FB_VISUAL_DIRECTCOLOR:
	printf("FB_VISUAL_DIRECTCOLOR\n");
	break;
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
	printf("FB_VISUAL_STATIC_PSEUDOCOLOR\n");
	break;
    default:
	printf("Unknown Visual mode.\n");
    }
    printf("xpanstep	:[%d]\n", fscinfo.xpanstep);
    printf("ypanstep	:[%d]\n", fscinfo.ypanstep);
    printf("ywrapstep	:[%d]\n", fscinfo.ywrapstep);
    printf("line_length	:[%d]\n", fscinfo.line_length);
    printf("mmio_start	:[%08lx]\n", fscinfo.mmio_start);
    printf("mmio_len	:[%d]\n", fscinfo.mmio_len);
    printf("accel		:[%d] ", fscinfo.accel);
    switch (fscinfo.accel) {
    case FB_ACCEL_NONE:
	printf("FB_ACCEL_NONE\n");
	break;
    case FB_ACCEL_ATARIBLITT:
	printf("FB_ACCEL_ATARIBLITT\n");
	break;
    case FB_ACCEL_AMIGABLITT:
	printf("FB_ACCEL_AMIGABLITT\n");
	break;
    case FB_ACCEL_S3_TRIO64:
	printf("FB_ACCEL_S3_TRIO64\n");
	break;
    case FB_ACCEL_NCR_77C32BLT:
	printf("FB_ACCEL_NCR_77C32BLT\n");
	break;
    case FB_ACCEL_S3_VIRGE:
	printf("FB_ACCEL_S3_VIRGE\n");
	break;
    case FB_ACCEL_ATI_MACH64GX:
	printf("FB_ACCEL_ATI_MACH64GX\n");
	break;
    case FB_ACCEL_DEC_TGA:
	printf("FB_ACCEL_DEC_TGA\n");
	break;
    case FB_ACCEL_ATI_MACH64CT:
	printf("FB_ACCEL_ATI_MACH64CT\n");
	break;
    case FB_ACCEL_ATI_MACH64VT:
	printf("FB_ACCEL_ATI_MACH64VT\n");
	break;
    case FB_ACCEL_ATI_MACH64GT:
	printf("FB_ACCEL_ATI_MACH64GT\n");
	break;
    case FB_ACCEL_SUN_CREATOR:
	printf("FB_ACCEL_SUN_CREATOR\n");
	break;
    case FB_ACCEL_SUN_CGSIX:
	printf("FB_ACCEL_SUN_CGSIX\n");
	break;
    case FB_ACCEL_SUN_LEO:
	printf("FB_ACCEL_SUN_LEO\n");
	break;
    case FB_ACCEL_IMS_TWINTURBO:
	printf("FB_ACCEL_IMS_TWINTURBO\n");
	break;
    case FB_ACCEL_3DLABS_PERMEDIA2:
	printf("FB_ACCEL_3DLABS_PERMEDIA2\n");
	break;
    case FB_ACCEL_MATROX_MGA2064W:
	printf("FB_ACCEL_MATROX_MGA2064W\n");
	break;
    case FB_ACCEL_MATROX_MGA1064SG:
	printf("FB_ACCEL_MATROX_MGA1064SG\n");
	break;
    case FB_ACCEL_MATROX_MGA2164W:
	printf("FB_ACCEL_MATROX_MGA2164W\n");
	break;
    case FB_ACCEL_MATROX_MGA2164W_AGP:
	printf("FB_ACCEL_MATROX_MGA2164W_AGP\n");
	break;
    case FB_ACCEL_MATROX_MGAG100:
	printf("FB_ACCEL_MATROX_MGAG100\n");
	break;
    case FB_ACCEL_MATROX_MGAG200:
	printf("FB_ACCEL_MATROX_MGAG200\n");
	break;
    case FB_ACCEL_SUN_CG14:
	printf("FB_ACCEL_SUN_CG14\n");
	break;
    case FB_ACCEL_SUN_BWTWO:
	printf("FB_ACCEL_SUN_BWTWO\n");
	break;
    case FB_ACCEL_SUN_CGTHREE:
	printf("FB_ACCEL_SUN_CGTHREE\n");
	break;
    case FB_ACCEL_SUN_TCX:
	printf("FB_ACCEL_SUN_TCX\n");
	break;
    default:
	printf("Unknown Visual mode.\n");
    }
    return;
}

void
fb_vscrn_disp(void)
{
    if (is_open != TRUE)
	return;
    printf("vscinfo DUMP\n");
    printf("xres		:[%d]\n", vscinfo.xres);
    printf("yres		:[%d]\n", vscinfo.yres);
    printf("xres_virtual	:[%d]\n", vscinfo.xres_virtual);
    printf("yres_virtual	:[%d]\n", vscinfo.yres_virtual);
    printf("xoffset		:[%d]\n", vscinfo.xoffset);
    printf("yoffset		:[%d]\n", vscinfo.yoffset);
    printf("bits_per_pixel	:[%d]\n", vscinfo.bits_per_pixel);
    printf("grayscale	:[%d]\n", vscinfo.grayscale);
    printf("red.offset	:[%d]\n", vscinfo.red.offset);
    printf("red.length	:[%d]\n", vscinfo.red.length);
    printf("red.msb_right	:[%d]\n", vscinfo.red.msb_right);
    printf("green.offset	:[%d]\n", vscinfo.green.offset);
    printf("green.length	:[%d]\n", vscinfo.green.length);
    printf("green.msb_right	:[%d]\n", vscinfo.green.msb_right);
    printf("blue.offset	:[%d]\n", vscinfo.blue.offset);
    printf("blue.length	:[%d]\n", vscinfo.blue.length);
    printf("blue.msb_right	:[%d]\n", vscinfo.blue.msb_right);
    printf("transp.offset	:[%d]\n", vscinfo.transp.offset);
    printf("transp.length	:[%d]\n", vscinfo.transp.length);
    printf("transp.msb_right:[%d]\n", vscinfo.transp.msb_right);
    printf("nonstd		:[%d]\n", vscinfo.nonstd);
    printf("activate	:[%d]\n", vscinfo.activate);
    printf("height		:[%d]\n", vscinfo.height);
    printf("width		:[%d]\n", vscinfo.width);
    printf("accel_flags	:[%d]\n", vscinfo.accel_flags);
    printf("pixclock	:[%d]\n", vscinfo.pixclock);
    printf("left_margin	:[%d]\n", vscinfo.left_margin);
    printf("right_margin	:[%d]\n", vscinfo.right_margin);
    printf("upper_margin	:[%d]\n", vscinfo.upper_margin);
    printf("lower_margin	:[%d]\n", vscinfo.lower_margin);
    printf("hsync_len	:[%d]\n", vscinfo.hsync_len);
    printf("vsync_len	:[%d]\n", vscinfo.vsync_len);
    printf("sync		:[%d]\n", vscinfo.sync);
    printf("vmode		:[%d]\n", vscinfo.vmode);
    return;
}

/********* static functions **************/

/*
 * (struct fb_cmap)デバイスに依存しないカラーマップ情報
 * 
 * fb_cmap_create()     新規のカラーマップ情報
 * fb_cmap_destroy()    カラーマップ情報の破棄
 * fb_cmap_disp()               情報の表示
 * fb_cmap_get()                情報の獲得
 * fb_cmap_set()                情報の設定
 */

#define	LUT_MAX		(256)

static struct fb_cmap *
fb_cmap_create(struct fb_fix_screeninfo *fscinfo,
	       struct fb_var_screeninfo *vscinfo)
{
    struct fb_cmap *cmap;
    int cmaplen = LUT_MAX;

    /* カラーマップの存在チェック */
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

    /* 各分色が存在しそうだったらカラーマップ用の領域を確保 */
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

#if 0
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
#endif
/*
 * フレームバッファに対するアクセス
 * 
 * fb_mmap()            フレームバッファのメモリ上へのマップ
 * fb_munmap()          フレームバッファのメモリ上からのアンマップ
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
 * (struct fb_fix_screeninfo)デバイスに依存しない固定された情報
 * 
 * fb_fscrn_disp()              情報の表示
 * fb_fscrn_get()               情報の獲得
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
 * (struct fb_var_screeninfo)デバイスに依存しない変更可能な情報
 * 
 * fb_vscrn_disp()              情報の表示
 * fb_vscrn_get()               情報の獲得
 * fb_vscrn_set()               情報の設定
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

#if 0
static int
fb_vscrn_set(int fbfp, struct fb_var_screeninfo *scinfo)
{
    if (ioctl(fbfp, FBIOPUT_VSCREENINFO, scinfo)) {
	perror("ioctl FBIOPUT_VSCREENINFO error\n");
	return -1;
    }
    return 0;
}
#endif
