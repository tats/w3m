/**************************************************************************
                fb_gdkpixbuf.c 0.2 Copyright (C) 2002, hito
 **************************************************************************/

#include "fb.h"
#include "fb_img.h"

static void set_prm(IMAGE *img);

IMAGE *fb_load_image(char *filename, int w, int h)
{
  GdkPixbuf *pixbuf;
  IMAGE *img;

  if(filename == NULL)
    return NULL;

  img = malloc(sizeof(*img));
  if(img == NULL)
    return NULL;
  
  pixbuf = gdk_pixbuf_new_from_file(filename);
  if(pixbuf == NULL){
    free(img);
    return NULL;
  }

  img->pixbuf = pixbuf;
  set_prm(img);

  fb_resize_image(img, w, h);

  return img;
}

int fb_draw_image(IMAGE *img, int x, int y, int sx, int sy, int width, int height)
{
  int i, j, r, g, b, offset, bpp;

  if(img == NULL)
    return 1;

  bpp = img->rowstride / img->width;
  for(j = sy; j < sy + height && j < img->height; j++){
    offset = j * img->rowstride + bpp * sx;
    for(i = sx; i < sx + width && i < img->width; i++, offset += bpp){
      r = img->pixels[offset];
      g = img->pixels[offset + 1];
      b = img->pixels[offset + 2];
      if(img->alpha && img->pixels[offset + 3] == 0)
	fb_pset(i + x - sx, j + y - sy, bg_r, bg_g, bg_b);
      else
	fb_pset(i + x - sx, j + y - sy, r, g, b);
    }
  }
  return 0;
}

int fb_resize_image(IMAGE *img, int width, int height)
{
  GdkPixbuf *pixbuf;
  if(width < 1 || height < 1 || img == NULL)
    return 1;

  if(width == img->width && height == img->height)
    return 0;

  pixbuf = gdk_pixbuf_scale_simple(img->pixbuf, width, height, GDK_INTERP_HYPER);
  if(pixbuf == NULL)
    return 1;
  gdk_pixbuf_finalize(img->pixbuf);

  img->pixbuf = pixbuf;
  set_prm(img);
  return 0;
}

void fb_free_image(IMAGE *img)
{
  if(img == NULL)
    return;

  gdk_pixbuf_finalize(img->pixbuf);
  free(img);
}

IMAGE *fb_dup_image(IMAGE *img)
{
  GdkPixbuf *pixbuf;
  IMAGE *new_img;

  if(img == NULL)
    return NULL;

  new_img = malloc(sizeof(*img));
  if(new_img == NULL)
    return NULL;

  pixbuf = gdk_pixbuf_copy(img->pixbuf);
  if(pixbuf == NULL){
    free(new_img);
    return NULL;
  }

  new_img->pixbuf = pixbuf;
  set_prm(new_img);
  return new_img;
}

int fb_rotate_image(IMAGE *img, int angle)
{
  return 1;
}

static void set_prm(IMAGE *img)
{
  GdkPixbuf *pixbuf;

  if(img == NULL)
    return;
  pixbuf = img->pixbuf;

  img->pixels = gdk_pixbuf_get_pixels(pixbuf);
  img->width  = gdk_pixbuf_get_width(pixbuf);
  img->height = gdk_pixbuf_get_height(pixbuf);
  img->alpha  = gdk_pixbuf_get_has_alpha(pixbuf);
  img->rowstride = gdk_pixbuf_get_rowstride(pixbuf);
}
