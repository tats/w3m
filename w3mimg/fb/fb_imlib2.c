/* $Id: fb_imlib2.c,v 1.6 2002/07/22 16:17:32 ukai Exp $ */
/**************************************************************************
                fb_imlib2.c 0.3 Copyright (C) 2002, hito
 **************************************************************************/

#include <X11/Xlib.h>
#include <Imlib2.h>
#include "fb.h"
#include "fb_img.h"

static void draw(FB_IMAGE * img, Imlib_Image image);
static Imlib_Image resize_image(Imlib_Image image, int width, int height);

int
get_image_size(char *filename, int *w, int *h)
{
    Imlib_Image image;

    if (filename == NULL)
	return 1;

    image = imlib_load_image(filename);
    if (image == NULL)
	return 1;

    imlib_context_set_image(image);
    *w = imlib_image_get_width();
    *h = imlib_image_get_height();
    imlib_free_image();

    return 0;
}

FB_IMAGE *
fb_image_load(char *filename, int w, int h)
{
    Imlib_Image image;
    FB_IMAGE *img;

    if (filename == NULL)
	return NULL;

    image = imlib_load_image(filename);
    if (image == NULL)
	return NULL;

    image = resize_image(image, w, h);
    if (image == NULL)
	return NULL;

    imlib_context_set_image(image);

    w = imlib_image_get_width();
    h = imlib_image_get_height();

    img = fb_image_new(w, h);

    if (img == NULL) {
	imlib_free_image();
	return NULL;
    }

    draw(img, image);

    imlib_free_image();

    return img;
}

static void
draw(FB_IMAGE * img, Imlib_Image image)
{
    int i, j, r, g, b, a = 0, offset;
    DATA32 *data;

    if (img == NULL)
	return;

    imlib_context_set_image(image);
    data = imlib_image_get_data_for_reading_only();

    for (j = 0; j < img->height; j++) {
	offset = img->width * j;
	for (i = 0; i < img->width; i++) {
	    a = (data[offset + i] >> 24) & 0x000000ff;
	    r = (data[offset + i] >> 16) & 0x000000ff;
	    g = (data[offset + i] >> 8) & 0x000000ff;
	    b = (data[offset + i]) & 0x000000ff;

	    if (a == 0)
		fb_image_pset(img, i, j, bg_r, bg_g, bg_b);
	    else
		fb_image_pset(img, i, j, r, g, b);
	}
    }
    return;
}

static Imlib_Image
resize_image(Imlib_Image image, int width, int height)
{
    Imlib_Image resized_image;
    int w, h;

    if (image == NULL)
	return NULL;

    imlib_context_set_image(image);
    w = imlib_image_get_width();
    h = imlib_image_get_height();

    if (width < 1 || height < 1)
	return image;

    if (w == width && h == height)
	return image;

    resized_image = 
	imlib_create_cropped_scaled_image(0, 0, w, h, width, height);

    imlib_free_image();

    return resized_image;
}
