/**************************************************************************
                fb_imlib2.c 0.2 Copyright (C) 2002, hito
 **************************************************************************/

#include "fb.h"
#include "fb_img.h"

static void set_prm(IMAGE *img);

IMAGE *fb_load_image(char *filename, int w, int h)
{
  Imlib_Image image;
  IMAGE *img;

  if(filename == NULL)
    return NULL;

  img = malloc(sizeof(*img));
  if(img == NULL)
    return NULL;

  image = imlib_load_image(filename);
  if(image == NULL){
    free(img);
    return NULL;
  }

  imlib_context_set_image(image);

  img->image = image;
  set_prm(img);

  fb_resize_image(img, w, h);

  return img;
}

int fb_draw_image(IMAGE *img, int x, int y, int sx, int sy, int width, int height)
{
  int i, j, r, g, b, a = 0, offset;

  if(img == NULL)
    return 1;

  for(j = sy; j < sy + height && j < img->height; j++){
    offset = j * img->width;
    for(i = sx; i < sx + width && i < img->width; i++){
      a = (img->data[offset + i] >> 24) & 0x000000ff;
      r = (img->data[offset + i] >> 16) & 0x000000ff;
      g = (img->data[offset + i] >>  8) & 0x000000ff;
      b = (img->data[offset + i]      ) & 0x000000ff;

      if(a == 0)
	fb_pset(i + x - sx, j + y - sy, bg_r, bg_g, bg_b);
      else
	fb_pset(i + x - sx, j + y - sy, r, g, b);
    }
  }
  return 0;
}

int fb_resize_image(IMAGE *img, int width, int height)
{
  Imlib_Image image;

  if(width < 1 || height < 1 || img == NULL)
    return 1;

  if(width == img->width && height == img->height)
    return 0;

  image = imlib_create_cropped_scaled_image(0, 0, img->width, img->height, width, height);
  if(image == NULL)
    return 1;

  imlib_context_set_image(img->image);
  imlib_free_image();

  img->image = image;
  set_prm(img);
  return 0;
}

void fb_free_image(IMAGE *img)
{
  if(img == NULL)
    return;

  imlib_context_set_image(img->image);
  imlib_free_image();
  free(img);
}

IMAGE *fb_dup_image(IMAGE *img)
{
  Imlib_Image image;
  IMAGE *new_img;

  if(img == NULL)
    return NULL;

  new_img = malloc(sizeof(*img));
  if(new_img == NULL)
    return NULL;

  imlib_context_set_image(img->image);
  image = imlib_clone_image();

  if(image == NULL){
    free(new_img);
    return NULL;
  }

  new_img->image = image;
  set_prm(new_img);
  return new_img;
}

int fb_rotate_image(IMAGE *img, int angle)
{
  int orientation;

  if(img == NULL)
    return 1;

  imlib_context_set_image(img->image);

  if(angle == 90){
    orientation = 1;
  }else if(angle == -90){
    orientation = 3;
  }else{
    return 1;
  }

  imlib_image_orientate(orientation);
  set_prm(img);
  return 0;
}

static void set_prm(IMAGE *img)
{
  if(img == NULL)
    return;

  imlib_context_set_image(img->image);
  img->data   = imlib_image_get_data_for_reading_only();
  img->width  = imlib_image_get_width();
  img->height = imlib_image_get_height();
}
