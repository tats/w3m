/* $Id: x11_w3mimg.c,v 1.8 2002/09/30 17:44:30 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"

#if defined(USE_IMLIB)
#include <Imlib.h>
#elif defined(USE_GDKPIXBUF)
#include <gdk-pixbuf/gdk-pixbuf-xlib.h>
#else
#error no Imlib and GdkPixbuf support
#endif

#include "w3mimg/w3mimg.h"

#define OFFSET_X	2
#define OFFSET_Y	2

struct x11_info {
    Display *display;
    Window window, parent;
    unsigned long background_pixel;
    GC imageGC;
#if defined(USE_IMLIB)
    ImlibData *id;
#elif defined(USE_GDKPIXBUF)
    int init_flag;
#endif
};

static int
x11_init(w3mimg_op * self)
{
    struct x11_info *xi;
    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;
#if defined(USE_IMLIB)
    if (!xi->id) {
	xi->id = Imlib_init(xi->display);
	if (!xi->id)
	    return 0;
    }
#elif defined(USE_GDKPIXBUF)
    if (!xi->init_flag) {
	gdk_pixbuf_xlib_init(xi->display, 0);
	xi->init_flag = TRUE;
    }
#endif
    if (!xi->imageGC) {
	xi->imageGC = XCreateGC(xi->display, xi->parent, 0, NULL);
	if (!xi->imageGC)
	    return 0;
    }
    return 1;
}

static int
x11_finish(w3mimg_op * self)
{
    struct x11_info *xi;
    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;
    if (xi->imageGC) {
	XFreeGC(xi->display, xi->imageGC);
	xi->imageGC = NULL;
    }
    return 1;
}

static int
x11_active(w3mimg_op * self)
{
    struct x11_info *xi;
    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;
    if (!xi->imageGC)
	return 0;
    return 1;
}

static void
x11_set_background(w3mimg_op * self, char *background)
{
    XColor screen_def, exact_def;
    struct x11_info *xi;
    if (self == NULL)
	return;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return;

    if (background &&
	XAllocNamedColor(xi->display, DefaultColormap(xi->display, 0),
			 background, &screen_def, &exact_def))
	xi->background_pixel = screen_def.pixel;
    else {
	Pixmap p;
	GC gc;
	XImage *i;

	p = XCreatePixmap(xi->display, xi->window, 1, 1,
			  DefaultDepth(xi->display, 0));
	gc = XCreateGC(xi->display, xi->window, 0, NULL);
	if (!p || !gc)
	    exit(1);		/* XXX */
	XCopyArea(xi->display, xi->window, p, gc,
		  (self->offset_x >= 1) ? (self->offset_x - 1) : 0,
		  (self->offset_y >= 1) ? (self->offset_y - 1) : 0,
		  1, 1, 0, 0);
	i = XGetImage(xi->display, p, 0, 0, 1, 1, -1, ZPixmap);
	if (!i)
	    exit(1);
	xi->background_pixel = XGetPixel(i, 0, 0);
	XDestroyImage(i);
	XFreeGC(xi->display, gc);
	XFreePixmap(xi->display, p);
    }
}

static void
x11_sync(w3mimg_op * self)
{
    struct x11_info *xi;
    if (self == NULL)
	return;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return;
    XSync(xi->display, False);
}

static void
x11_close(w3mimg_op * self)
{
    /* XCloseDisplay(xi->display); */
}

static int
x11_load_image(w3mimg_op * self, W3MImage * img, char *fname, int w, int h)
{
    struct x11_info *xi;
#if defined(USE_IMLIB)
    ImlibImage *im;
#elif defined(USE_GDKPIXBUF)
    GdkPixbuf *pixbuf;
    int iw, ih;
#endif

    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;

#if defined(USE_IMLIB)
    im = Imlib_load_image(xi->id, fname);
    if (!im)
	return 0;
    if (w <= 0)
	w = im->rgb_width;
    if (h <= 0)
	h = im->rgb_height;
#elif defined(USE_GDKPIXBUF)
    pixbuf = gdk_pixbuf_new_from_file(fname);
    if (!pixbuf)
	return 0;
    iw = gdk_pixbuf_get_width(pixbuf);
    ih = gdk_pixbuf_get_height(pixbuf);
    if (w <= 0)
	w = iw;
    if (h <= 0)
	h = ih;
#endif
    img->pixmap = (void *)XCreatePixmap(xi->display, xi->parent, w, h,
					DefaultDepth(xi->display, 0));
    if (!img->pixmap)
	return 0;
    XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
    XFillRectangle(xi->display, (Pixmap) img->pixmap, xi->imageGC, 0, 0, w, h);
#if defined(USE_IMLIB)
    Imlib_paste_image(xi->id, im, (Pixmap) img->pixmap, 0, 0, w, h);
    Imlib_kill_image(xi->id, im);
#elif defined(USE_GDKPIXBUF)
    if (w != iw || h != ih) {
	GdkPixbuf *newpixbuf;
	newpixbuf = gdk_pixbuf_scale_simple(pixbuf, w, h, GDK_INTERP_BILINEAR);
	gdk_pixbuf_xlib_render_to_drawable(newpixbuf, (Drawable) img->pixmap,
					   xi->imageGC, 0, 0, 0, 0, w, h,
					   XLIB_RGB_DITHER_NORMAL, 0, 0);
	gdk_pixbuf_finalize(newpixbuf);
    } else {
	gdk_pixbuf_xlib_render_to_drawable(pixbuf, (Drawable) img->pixmap,
					   xi->imageGC, 0, 0, 0, 0, w, h,
					   XLIB_RGB_DITHER_NORMAL, 0, 0);
    }
    gdk_pixbuf_unref(pixbuf);
#endif
    img->width = w;
    img->height = h;
    return 1;
}

static int
x11_show_image(w3mimg_op * self, W3MImage * img, int sx, int sy, int sw,
	       int sh, int x, int y)
{
    struct x11_info *xi;
    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;

    XCopyArea(xi->display, (Pixmap) img->pixmap, xi->window, xi->imageGC,
	      sx, sy,
	      (sw ? sw : img->width),
	      (sh ? sh : img->height), x + self->offset_x, y + self->offset_y);
    return 1;
}

static void
x11_free_image(w3mimg_op * self, W3MImage * img)
{
    struct x11_info *xi;
    if (self == NULL)
	return;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return;
    if (img && img->pixmap) {
	XFreePixmap(xi->display, (Pixmap) img->pixmap);
	img->pixmap = NULL;
	img->width = 0;
	img->height = 0;
    }
}

static int
x11_get_image_size(w3mimg_op * self, W3MImage * img, char *fname, int *w,
		   int *h)
{
    struct x11_info *xi;
#if defined(USE_IMLIB)
    ImlibImage *im;
#elif defined(USE_GDKPIXBUF)
    GdkPixbuf *pixbuf;
#endif

    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;

#if defined(USE_IMLIB)
    im = Imlib_load_image(xi->id, fname);
    if (!im)
	return 0;

    *w = im->rgb_width;
    *h = im->rgb_height;
    Imlib_kill_image(xi->id, im);
#elif defined(USE_GDKPIXBUF)
    pixbuf = gdk_pixbuf_new_from_file(fname);
    if (!pixbuf)
	return 0;

    *w = gdk_pixbuf_get_width(pixbuf);
    *h = gdk_pixbuf_get_height(pixbuf);

    gdk_pixbuf_unref(pixbuf);
#endif
    return 1;
}

/* *INDENT-OFF* */
/*
  xterm/kterm/hanterm/cxterm
    top window (WINDOWID)
      +- text window
           +- scrollbar
  rxvt/aterm/Eterm/wterm
    top window (WINDOWID)
      +- text window
      +- scrollbar
      +- menubar (etc.)
  gnome-terminal
    top window
      +- text window (WINDOWID)
      +- scrollbar
      +- menubar
  mlterm (-s)
    top window
      +- text window (WINDOWID)
      +- scrollbar
  mlterm
    top window = text window (WINDOWID)

  powershell
    top window
      +- window
      |    +- text window
      |    +- scrollbar
      +- menubar (etc.)
  dtterm
    top window
      +- window
           +- window
           |    +- window
           |         +- text window
           |         +- scrollbar
           +- menubar
  hpterm
    top window
      +- window
           +- text window
           +- scrollbar
           +- (etc.)
*/
/* *INDENT-ON* */

w3mimg_op *
w3mimg_x11open()
{
    w3mimg_op *wop = NULL;
    struct x11_info *xi = NULL;
    char *id;
    int revert, i;
    unsigned int nchildren;
    XWindowAttributes attr;
    Window root, *children;

    wop = (w3mimg_op *) malloc(sizeof(w3mimg_op));
    if (wop == NULL)
	return NULL;
    memset(wop, 0, sizeof(w3mimg_op));

    xi = (struct x11_info *)malloc(sizeof(struct x11_info));
    if (xi == NULL)
	goto error;
    memset(xi, 0, sizeof(struct x11_info));

    xi->display = XOpenDisplay(NULL);
    if (xi->display == NULL) {
	goto error;
    }
    if ((id = getenv("WINDOWID")) != NULL)
	xi->window = (Window) atoi(id);
    else
	XGetInputFocus(xi->display, &xi->window, &revert);
    if (!xi->window)
	exit(1);

    XGetWindowAttributes(xi->display, xi->window, &attr);
    wop->width = attr.width;
    wop->height = attr.height;

    while (1) {
	Window p_window;

	XQueryTree(xi->display, xi->window, &root, &xi->parent,
		   &children, &nchildren);
	p_window = xi->window;
	for (i = 0; i < nchildren; i++) {
	    XGetWindowAttributes(xi->display, children[i], &attr);
	    if (attr.width > wop->width * 0.7 &&
		attr.height > wop->height * 0.7) {
		/* maybe text window */
		wop->width = attr.width;
		wop->height = attr.height;
		xi->window = children[i];
	    }
	}
	if (p_window == xi->window)
	    break;
    }
    wop->offset_x = OFFSET_X;
    for (i = 0; i < nchildren; i++) {
	XGetWindowAttributes(xi->display, children[i], &attr);
	if (attr.x <= 0 && attr.width < 30 && attr.height > wop->height * 0.7) {
	    /* scrollbar of xterm/kterm ? */
	    wop->offset_x += attr.x + attr.width + attr.border_width * 2;
	    break;
	}
    }
    wop->offset_y = OFFSET_Y;

    wop->priv = xi;

    wop->init = x11_init;
    wop->finish = x11_finish;
    wop->active = x11_active;
    wop->set_background = x11_set_background;
    wop->sync = x11_sync;
    wop->close = x11_close;

    wop->load_image = x11_load_image;
    wop->show_image = x11_show_image;
    wop->free_image = x11_free_image;
    wop->get_image_size = x11_get_image_size;

    return wop;
  error:
    if (xi)
	free(xi);
    free(wop);
    return NULL;
}
