/* $Id: fb_gdkpixbuf.c,v 1.11 2003/03/26 15:14:23 ukai Exp $ */
/**************************************************************************
                fb_gdkpixbuf.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "fb.h"
#include "fb_img.h"

static void draw(FB_IMAGE * img, int bg, int x, int y, int w, int h,
		 GdkPixbuf * pixbuf);
static GdkPixbuf *resize_image(GdkPixbuf * pixbuf, int width, int height);

static void
get_animation_size(GdkPixbufAnimation *animation, int *w, int *h)
{
    GList *frames;
    int iw, ih, n, i;

    frames = gdk_pixbuf_animation_get_frames(animation);
    n = gdk_pixbuf_animation_get_num_frames(animation);
    *w = gdk_pixbuf_animation_get_width(animation);
    *h = gdk_pixbuf_animation_get_height(animation);
    for (i = 0; i < n; i++) {
	GdkPixbufFrame *frame;
	GdkPixbuf *pixbuf;

	frame = (GdkPixbufFrame *) g_list_nth_data(frames, i);
	pixbuf = gdk_pixbuf_frame_get_pixbuf(frame);
	iw = gdk_pixbuf_frame_get_x_offset(frame)
	  +gdk_pixbuf_get_width(pixbuf);
	ih = gdk_pixbuf_frame_get_y_offset(frame)
	  + gdk_pixbuf_get_height(pixbuf);
	if (iw > *w)
	    *w = iw;
	if (ih > *h)
	    *h = ih;
    }
}

int
get_image_size(char *filename, int *w, int *h)
{
    GdkPixbufAnimation *animation;
    if (filename == NULL)
	return 1;
    animation = gdk_pixbuf_animation_new_from_file(filename);
    if (animation == NULL)
	return 1;
    get_animation_size(animation, w, h);
    gdk_pixbuf_animation_unref(animation);
    return 0;
}

FB_IMAGE **
fb_image_load(char *filename, int w, int h, int max_anim)
{
    GdkPixbufAnimation *animation;
    GList *frames;
    double ratio_w, ratio_h;
    int n, i, fw, fh;
    FB_IMAGE **fb_frame;
    GdkPixbufFrameAction action = GDK_PIXBUF_FRAME_REVERT;
    if (filename == NULL)
	return NULL;
    animation = gdk_pixbuf_animation_new_from_file(filename);
    if (animation == NULL)
	return NULL;
    frames = gdk_pixbuf_animation_get_frames(animation);
    get_animation_size(animation, &fw, &fh);
    n = gdk_pixbuf_animation_get_num_frames(animation);
    if (max_anim > 0) {
	n = (max_anim > n) ? n : max_anim;
    }
    if (w < 1 || h < 1) {
	w = fw;
	h = fh;
	ratio_w = ratio_h = 1;
    }
    else {
	ratio_w = 1.0 * w / fw;
	ratio_h = 1.0 * h / fh;
    }
    fb_frame = fb_frame_new(w, h, n);
    if (fb_frame == NULL)
	goto END;
    for (i = 0; i < n; i++) {
	GdkPixbufFrame *frame;
	GdkPixbuf *org_pixbuf, *pixbuf;
	int width, height, ofstx, ofsty;
	frame = (GdkPixbufFrame *) g_list_nth_data(frames, i);
	org_pixbuf = gdk_pixbuf_frame_get_pixbuf(frame);
	ofstx = gdk_pixbuf_frame_get_x_offset(frame);
	ofsty = gdk_pixbuf_frame_get_y_offset(frame);
	width = gdk_pixbuf_get_width(org_pixbuf);
	height = gdk_pixbuf_get_height(org_pixbuf);
	if (ofstx == 0 && ofsty == 0 && width == fw && height == fh) {
	    pixbuf = resize_image(org_pixbuf, w, h);
	}
	else {
	    pixbuf =
		resize_image(org_pixbuf, width * ratio_w, height * ratio_h);
	    ofstx *= ratio_w;
	    ofsty *= ratio_h;
	}
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	fb_frame[i]->delay = gdk_pixbuf_frame_get_delay_time(frame);
	if (i > 0) {
	    switch (action) {
	    case GDK_PIXBUF_FRAME_RETAIN:
		fb_image_copy(fb_frame[i], fb_frame[i - 1]);
		break;
	    case GDK_PIXBUF_FRAME_DISPOSE:
		if (bg_r != 0 || bg_g != 0 || bg_b != 0) {
		    fb_image_fill(fb_frame[i], bg_r, bg_g, bg_b);
		}
		break;
	    case GDK_PIXBUF_FRAME_REVERT:
		fb_image_copy(fb_frame[i], fb_frame[0]);
		break;
	    default:
		fb_image_copy(fb_frame[i], fb_frame[0]);
	    }
	}
	action = gdk_pixbuf_frame_get_action(frame);
	draw(fb_frame[i], !i, ofstx, ofsty, width, height, pixbuf);
	if (org_pixbuf != pixbuf)
	    gdk_pixbuf_finalize(pixbuf);
    }
  END:
    gdk_pixbuf_animation_unref(animation);
    return fb_frame;
}
static void
draw(FB_IMAGE * img, int bg, int x, int y, int w, int h, GdkPixbuf * pixbuf)
{
    int i, j, r, g, b, offset, bpp, rowstride;
    guchar *pixels;
    gboolean alpha;
    if (img == NULL || pixbuf == NULL)
	return;
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);
    alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    bpp = rowstride / w;
    for (j = 0; j < h; j++) {
	offset = j * rowstride;
	for (i = 0; i < w; i++, offset += bpp) {
	    r = pixels[offset];
	    g = pixels[offset + 1];
	    b = pixels[offset + 2];
	    if (alpha && pixels[offset + 3] == 0) {
		if (bg)
		    fb_image_pset(img, i + x, j + y, bg_r, bg_g, bg_b);
	    }
	    else {
		fb_image_pset(img, i + x, j + y, r, g, b);
	    }
	}
    }
    return;
}
static GdkPixbuf *
resize_image(GdkPixbuf * pixbuf, int width, int height)
{
    GdkPixbuf *resized_pixbuf;
    int w, h;
    if (pixbuf == NULL)
	return NULL;
    w = gdk_pixbuf_get_width(pixbuf);
    h = gdk_pixbuf_get_height(pixbuf);
    if (width < 1 || height < 1)
	return pixbuf;
    if (w == width && h == height)
	return pixbuf;
    resized_pixbuf =
	gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_HYPER);
    if (resized_pixbuf == NULL)
	return NULL;
    return resized_pixbuf;
}
