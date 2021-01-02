/* $Id: fb_gdkpixbuf.c,v 1.21 2004/11/08 17:14:06 ukai Exp $ */
/**************************************************************************
                fb_gdkpixbuf.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include "config.h"
#if defined(USE_GTK2)
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "fb.h"
#include "fb_img.h"

static void draw(FB_IMAGE * img, int x, int y, int w, int h,
		 GdkPixbuf * pixbuf);
static GdkPixbuf *resize_image(GdkPixbuf * pixbuf, int width, int height);

#if defined(USE_GTK2)
static int
get_animation_size(GdkPixbufAnimation * animation, int *w, int *h, int *delay)
{
    GdkPixbufAnimationIter *iter;
    int n, i, d = -1;
    GTimeVal time;

    g_get_current_time(&time);
    iter = gdk_pixbuf_animation_get_iter(animation, &time);
    *w = gdk_pixbuf_animation_get_width(animation);
    *h = gdk_pixbuf_animation_get_height(animation);
    for (i = 1; 
	 gdk_pixbuf_animation_iter_on_currently_loading_frame(iter) != TRUE; 
	 i++) {
	int tmp;
	tmp = gdk_pixbuf_animation_iter_get_delay_time(iter);
	g_time_val_add(&time, tmp * 1000);
	if (tmp > d)
	    d = tmp;
	gdk_pixbuf_animation_iter_advance(iter, &time);
    }
    if (delay)
	*delay = d;
    n = i;
    g_object_unref(G_OBJECT(iter));
    return n;
}
#else
static int
get_animation_size(GdkPixbufAnimation * animation, int *w, int *h, int *delay)
{
    GList *frames;
    int iw, ih, n, i, d = -1;

    frames = gdk_pixbuf_animation_get_frames(animation);
    n = gdk_pixbuf_animation_get_num_frames(animation);
    *w = gdk_pixbuf_animation_get_width(animation);
    *h = gdk_pixbuf_animation_get_height(animation);
    for (i = 0; i < n; i++) {
	GdkPixbufFrame *frame;
	GdkPixbuf *pixbuf;
	int tmp;

	frame = (GdkPixbufFrame *) g_list_nth_data(frames, i);
	tmp = gdk_pixbuf_frame_get_delay_time(frame);
	if (tmp > d)
	    d = tmp;
	pixbuf = gdk_pixbuf_frame_get_pixbuf(frame);
	iw = gdk_pixbuf_frame_get_x_offset(frame)
	    + gdk_pixbuf_get_width(pixbuf);
	ih = gdk_pixbuf_frame_get_y_offset(frame)
	    + gdk_pixbuf_get_height(pixbuf);
	if (iw > *w)
	    *w = iw;
	if (ih > *h)
	    *h = ih;
    }
    if (delay)
	*delay = d;
    return n;
}
#endif

void
fb_image_init()
{
#if defined(USE_GTK2)
    g_type_init();
#endif
}

int
get_image_size(char *filename, int *w, int *h)
{
    GdkPixbufAnimation *animation;
    if (filename == NULL)
	return 1;
#if defined(USE_GTK2)
    animation = gdk_pixbuf_animation_new_from_file(filename, NULL);
#else
    animation = gdk_pixbuf_animation_new_from_file(filename);
#endif
    if (animation == NULL)
	return 1;
    get_animation_size(animation, w, h, NULL);
#if defined(USE_GTK2)
    g_object_unref(G_OBJECT(animation));
#else
    gdk_pixbuf_animation_unref(animation);
#endif
    return 0;
}

FB_IMAGE **
fb_image_load(char *filename, int w, int h, int max_anim)
{
    GdkPixbufAnimation *animation;
#if defined(USE_GTK2)
    GdkPixbufAnimationIter *iter;
    GTimeVal time;
#else
    int i;
    GList *frames;
#endif
    double ratio_w, ratio_h;
    int n, j, fw, fh, frame_num, delay;
    FB_IMAGE **fb_frame = NULL, *tmp_image = NULL;

    if (filename == NULL)
	return NULL;
#if defined(USE_GTK2)
    animation = gdk_pixbuf_animation_new_from_file(filename, NULL);
#else
    animation = gdk_pixbuf_animation_new_from_file(filename);
#endif
    if (animation == NULL)
	return NULL;
    frame_num = n = get_animation_size(animation, &fw, &fh, &delay);
    if (delay <= 0)
	max_anim = -1;
    if (max_anim < 0) {
	frame_num = (-max_anim > n) ? n : -max_anim;
    }
    else if (max_anim > 0) {
	frame_num = n = (max_anim > n) ? n : max_anim;
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

    fb_frame = fb_frame_new(w, h, frame_num);
    if (fb_frame == NULL)
	goto END;

    tmp_image = fb_image_new(w, h);
    if (tmp_image == NULL) {
	fb_frame_free(fb_frame);
	fb_frame = NULL;
	goto END;
    }

    if (bg_r != 0 || bg_g != 0 || bg_b != 0) {
	fb_image_fill(tmp_image, bg_r, bg_g, bg_b);
    }

#if defined(USE_GTK2)
    g_get_current_time(&time);
    iter = gdk_pixbuf_animation_get_iter(animation, &time);

    if (max_anim < 0 && n > -max_anim) {
	max_anim = n + max_anim;
	for (j = 0; j < max_anim; j++) {
	    g_time_val_add(&time, 
			   gdk_pixbuf_animation_iter_get_delay_time(iter) * 1000);
	    gdk_pixbuf_animation_iter_advance(iter, &time);
	}
    }
    for (j = 0; j < n; j++) {
	GdkPixbuf *org_pixbuf, *pixbuf;

	org_pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(iter);
	pixbuf = resize_image(org_pixbuf, w, h);

	fb_frame[j]->delay = gdk_pixbuf_animation_iter_get_delay_time(iter);
	g_time_val_add(&time, fb_frame[j]->delay * 1000);
	draw(fb_frame[j], 0, 0, w, h, pixbuf);
	if (org_pixbuf != pixbuf)
	    g_object_unref(G_OBJECT(pixbuf));
	gdk_pixbuf_animation_iter_advance(iter, &time);
    }
#else
    frames = gdk_pixbuf_animation_get_frames(animation);

    for (j = 0; j < n; j++) {
	GdkPixbufFrame *frame;
	GdkPixbuf *org_pixbuf, *pixbuf;
	int width, height, ofstx, ofsty;

	if (max_anim < 0) {
	    i = (j - n + frame_num > 0) ? (j - n + frame_num) : 0;
	}
	else {
	    i = j;
	}
	frame = (GdkPixbufFrame *) g_list_nth_data(frames, j);
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
	fb_image_copy(fb_frame[i], tmp_image);
	draw(fb_frame[i], ofstx, ofsty, width, height, pixbuf);

	switch (gdk_pixbuf_frame_get_action(frame)) {
	case GDK_PIXBUF_FRAME_RETAIN:
	    fb_image_copy(tmp_image, fb_frame[i]);
	    break;
	case GDK_PIXBUF_FRAME_DISPOSE:
	    fb_image_fill(tmp_image, bg_r, bg_g, bg_b);
	    break;
	case GDK_PIXBUF_FRAME_REVERT:
	    fb_image_copy(tmp_image, fb_frame[0]);
	    break;
	default:
	    fb_image_copy(tmp_image, fb_frame[0]);
	}

	if (org_pixbuf != pixbuf)
	    gdk_pixbuf_finalize(pixbuf);
    }
#endif
  END:
    if (tmp_image)
	fb_image_free(tmp_image);
#if defined(USE_GTK2)
    g_object_unref(G_OBJECT(animation));
#else
    gdk_pixbuf_animation_unref(animation);
#endif
    return fb_frame;
}
static void
draw(FB_IMAGE * img, int x, int y, int w, int h, GdkPixbuf * pixbuf)
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
	    if (!alpha || pixels[offset + 3] != 0) {
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
