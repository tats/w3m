/* $Id: fb_gdkpixbuf.c,v 1.6 2002/07/22 16:17:32 ukai Exp $ */
/**************************************************************************
                fb_gdkpixbuf.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include <gdk-pixbuf/gdk-pixbuf.h>
#include "fb.h"
#include "fb_img.h"

static void draw(FB_IMAGE * img, GdkPixbuf * pixbuf);
static GdkPixbuf *resize_image(GdkPixbuf * pixbuf, int width, int height);

int
get_image_size(char *filename, int *w, int *h)
{
    GdkPixbuf *pixbuf;

    if (filename == NULL)
	return 1;

    pixbuf = gdk_pixbuf_new_from_file(filename);
    if (pixbuf == NULL)
	return 1;

    *w = gdk_pixbuf_get_width(pixbuf);
    *h = gdk_pixbuf_get_height(pixbuf);

    gdk_pixbuf_finalize(pixbuf);
    return 0;
}

FB_IMAGE *
fb_image_load(char *filename, int w, int h)
{
    GdkPixbuf *pixbuf;
    FB_IMAGE *img;

    if (filename == NULL)
	return NULL;

    pixbuf = gdk_pixbuf_new_from_file(filename);
    if (pixbuf == NULL)
	return NULL;

    pixbuf = resize_image(pixbuf, w, h);
    if (pixbuf == NULL)
	return NULL;

    w = gdk_pixbuf_get_width(pixbuf);
    h = gdk_pixbuf_get_height(pixbuf);

    img = fb_image_new(w, h);

    if (img == NULL) {
	gdk_pixbuf_finalize(pixbuf);
	return NULL;
    }

    draw(img, pixbuf);

    gdk_pixbuf_finalize(pixbuf);

    return img;
}

void
draw(FB_IMAGE * img, GdkPixbuf * pixbuf)
{
    int i, j, r, g, b, offset, bpp, rowstride;
    guchar *pixels;
    gboolean alpha;

    if (img == NULL || pixbuf == NULL)
	return;

    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);
    alpha = gdk_pixbuf_get_has_alpha(pixbuf);

    bpp = rowstride / img->width;
    for (j = 0; j < img->height; j++) {
	offset = j * rowstride;
	for (i = 0; i < img->width; i++, offset += bpp) {
	    r = pixels[offset];
	    g = pixels[offset + 1];
	    b = pixels[offset + 2];
	    if (alpha && pixels[offset + 3] == 0)
		fb_image_pset(img, i, j, bg_r, bg_g, bg_b);
	    else
		fb_image_pset(img, i, j, r, g, b);
	}
    }
    return;
}

static GdkPixbuf *
resize_image(GdkPixbuf * pixbuf, int width, int height)
{
    GdkPixbuf * resized_pixbuf;
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

    gdk_pixbuf_finalize(pixbuf);

    if (resized_pixbuf == NULL)
	return NULL;

    return resized_pixbuf;
}
