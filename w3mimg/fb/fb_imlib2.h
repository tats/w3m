#ifndef fb_imlib2_header
#define fb_imlib2_header

#include <X11/Xlib.h>
#include <Imlib2.h>

typedef struct {
  int width;
  int height;
  Imlib_Image image;
  DATA32 *data;
} IMAGE;

#endif
