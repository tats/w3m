/* $Id: x11_w3mimg.c,v 1.29 2004/11/08 17:14:06 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

#if defined(USE_IMLIB)
#include <Imlib.h>
#elif defined(USE_IMLIB2)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Imlib2.h>
#elif defined(USE_GDKPIXBUF)
#if defined(USE_GTK2)
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#else
#include <gdk-pixbuf/gdk-pixbuf-xlib.h>
#endif
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

#if defined(USE_GDKPIXBUF)
struct x11_image {
    int total;
    int no;
    int wait;
    int delay;
    Pixmap *pixmap;
};

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
    g_object_unref(G_OBJECT(iter));
    n = i;
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
#endif

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
#if defined(USE_GTK2)
	g_type_init();
#endif
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
x11_clear(w3mimg_op * self, int x, int y, int w, int h)
{
    struct x11_info *xi;
    if (self == NULL)
	return 0;
    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;

    if (x < 0)
	x = 0;
    if (y < 0)
	y = 0;

    XClearArea(xi->display, xi->window, x, y, w, h, False);
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

#if defined(USE_GDKPIXBUF)
static struct x11_image *
x11_img_new(struct x11_info *xi, int w, int h, int n)
{
    struct x11_image *img = NULL;
    int i;

    img = malloc(sizeof(*img));
    if (!img)
	goto ERROR;

    img->pixmap = calloc(n, sizeof(*(img->pixmap)));
    if (!img->pixmap)
	goto ERROR;

    for (i = 0; i < n; i++) {
	img->pixmap[i] = XCreatePixmap(xi->display, xi->parent, w, h,
				       DefaultDepth(xi->display, 0));
	if (!img->pixmap[i])
	    goto ERROR;

	XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
	XFillRectangle(xi->display, (Pixmap) img->pixmap[i], xi->imageGC, 0, 0,
		       w, h);
    }

    img->no = 0;
    img->total = n;
    img->wait = 0;
    img->delay = -1;

    return img;
  ERROR:
    if (img) {
	if (img->pixmap) {
	    for (i = 0; i < n; i++) {
		if (img->pixmap[i])
		    XFreePixmap(xi->display, (Pixmap) img->pixmap[i]);
	    }
	    free(img->pixmap);
	}
	free(img);
    }
    return NULL;
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
	gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    if (resized_pixbuf == NULL)
	return NULL;
    return resized_pixbuf;
}
#endif

static int
x11_load_image(w3mimg_op * self, W3MImage * img, char *fname, int w, int h)
{
    struct x11_info *xi;
#if defined(USE_IMLIB)
    ImlibImage *im;
#elif defined(USE_IMLIB2)
    Imlib_Image im;
#elif defined(USE_GDKPIXBUF)
    GdkPixbufAnimation *animation;
    int j, iw, ih, n, frame_num, delay = -1, max_anim;
    double ratio_w, ratio_h;
    struct x11_image *ximg;
    Pixmap tmp_pixmap;
#if defined(USE_GTK2)
    GdkPixbufAnimationIter *iter;
    GTimeVal time;
#else
    int i;
    GList *frames;
#endif
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
    img->pixmap = (void *)XCreatePixmap(xi->display, xi->parent, w, h,
					DefaultDepth(xi->display, 0));
    if (!img->pixmap)
	return 0;
    XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
    XFillRectangle(xi->display, (Pixmap) img->pixmap, xi->imageGC, 0, 0, w, h);
    Imlib_paste_image(xi->id, im, (Pixmap) img->pixmap, 0, 0, w, h);
    Imlib_kill_image(xi->id, im);
#elif defined(USE_IMLIB2)
    im = imlib_load_image(fname);
    if (!im)
	return 0;
    imlib_context_set_image(im);
    if (w <= 0)
	w = imlib_image_get_width();
    if (h <= 0)
	h = imlib_image_get_height();
    img->pixmap = (void *)XCreatePixmap(xi->display, xi->parent, w, h,
					DefaultDepth(xi->display, 0));
    if (!img->pixmap)
	return 0;
    XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
    XFillRectangle(xi->display, (Pixmap) img->pixmap, xi->imageGC, 0, 0, w, h);
    imlib_context_set_display(xi->display);
    imlib_context_set_visual(DefaultVisual(xi->display, 0));
    imlib_context_set_colormap(DefaultColormap(xi->display, 0));
    imlib_context_set_drawable((Drawable) img->pixmap);
    imlib_render_image_on_drawable_at_size(0, 0, w, h);
    imlib_free_image();
#elif defined(USE_GDKPIXBUF)
    max_anim = self->max_anim;
#if defined(USE_GTK2)
    animation = gdk_pixbuf_animation_new_from_file(fname, NULL);
#else
    animation = gdk_pixbuf_animation_new_from_file(fname);
#endif
    if (!animation)
	return 0;
    frame_num = n = get_animation_size(animation, &iw, &ih, &delay);
    if (delay <= 0)
	max_anim = -1;

    if (max_anim < 0) {
	frame_num = (-max_anim > n) ? n : -max_anim;
    }
    else if (max_anim > 0) {
	frame_num = n = (max_anim > n) ? n : max_anim;
    }

    if (w < 1 || h < 1) {
	w = iw;
	h = ih;
	ratio_w = ratio_h = 1;
    }
    else {
	ratio_w = 1.0 * w / iw;
	ratio_h = 1.0 * h / ih;
    }
    tmp_pixmap = XCreatePixmap(xi->display, xi->parent, w, h,
			       DefaultDepth(xi->display, 0));
    XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
    XFillRectangle(xi->display, (Pixmap) tmp_pixmap, xi->imageGC, 0, 0, w, h);
    if (!tmp_pixmap) {
#if defined(USE_GTK2)
	g_object_unref(G_OBJECT(animation));
#else
	gdk_pixbuf_animation_unref(animation);
#endif
	return 0;
    }
    ximg = x11_img_new(xi, w, h, frame_num);
    if (!ximg) {
	XFreePixmap(xi->display, tmp_pixmap);
#if defined(USE_GTK2)
	g_object_unref(G_OBJECT(animation));
#else
	gdk_pixbuf_animation_unref(animation);
#endif
	return 0;
    }
#if defined(USE_GTK2)
    g_get_current_time(&time);
    iter = gdk_pixbuf_animation_get_iter(animation, &time);

   if (max_anim < 0 && n > -max_anim) {
	max_anim = n + max_anim;
	for (j = 0; j < max_anim; j++) {
	    delay = gdk_pixbuf_animation_iter_get_delay_time(iter);
	    g_time_val_add(&time, delay * 1000);
	    gdk_pixbuf_animation_iter_advance(iter, &time);
	}
    }
    for (j = 0; j < frame_num; j++) {
	GdkPixbuf *org_pixbuf, *pixbuf;

	org_pixbuf = gdk_pixbuf_animation_iter_get_pixbuf(iter);
	delay = gdk_pixbuf_animation_iter_get_delay_time(iter);
	pixbuf = resize_image(org_pixbuf, w, h);

	if (delay > ximg->delay)
	    ximg->delay = delay;

	gdk_pixbuf_xlib_render_to_drawable_alpha(pixbuf,
						 (Drawable) ximg->pixmap[j], 0,
						 0, 0, 0, w, h,
						 GDK_PIXBUF_ALPHA_BILEVEL, 1,
						 XLIB_RGB_DITHER_NORMAL, 0, 0);
	if (org_pixbuf != pixbuf)
	    g_object_unref(G_OBJECT(pixbuf));
	g_time_val_add(&time, delay * 1000);
	gdk_pixbuf_animation_iter_advance(iter, &time);
    }
    XFreePixmap(xi->display, tmp_pixmap);
    g_object_unref(G_OBJECT(animation));

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
	delay = gdk_pixbuf_frame_get_delay_time(frame);
	width = gdk_pixbuf_get_width(org_pixbuf);
	height = gdk_pixbuf_get_height(org_pixbuf);

	if (ofstx == 0 && ofsty == 0 && width == w && height == h) {
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

	if (delay > ximg->delay)
	    ximg->delay = delay;

	XCopyArea(xi->display, tmp_pixmap, ximg->pixmap[i],
		  xi->imageGC, 0, 0, w, h, 0, 0);
	gdk_pixbuf_xlib_render_to_drawable_alpha(pixbuf,
						 (Drawable) ximg->pixmap[i], 0,
						 0, ofstx, ofsty, width,
						 height,
						 GDK_PIXBUF_ALPHA_BILEVEL, 1,
						 XLIB_RGB_DITHER_NORMAL, 0, 0);

	switch (gdk_pixbuf_frame_get_action(frame)) {
	case GDK_PIXBUF_FRAME_RETAIN:
	    XCopyArea(xi->display, ximg->pixmap[i], tmp_pixmap,
		      xi->imageGC, 0, 0, w, h, 0, 0);
	    break;
	case GDK_PIXBUF_FRAME_DISPOSE:
	    XSetForeground(xi->display, xi->imageGC, xi->background_pixel);
	    XFillRectangle(xi->display, tmp_pixmap, xi->imageGC,
			   0, 0, w, h);
	    break;
	case GDK_PIXBUF_FRAME_REVERT:
	    XCopyArea(xi->display, ximg->pixmap[0], tmp_pixmap,
		      xi->imageGC, 0, 0, w, h, 0, 0);
	    break;
	default:
	    XCopyArea(xi->display, ximg->pixmap[0], tmp_pixmap,
		      xi->imageGC, 0, 0, w, h, 0, 0);
	    break;
	}


	if (org_pixbuf != pixbuf)
	    gdk_pixbuf_finalize(pixbuf);

    }
    XFreePixmap(xi->display, tmp_pixmap);
    gdk_pixbuf_animation_unref(animation);
#endif
    img->pixmap = ximg;
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
#if defined(USE_GDKPIXBUF)
    struct x11_image *ximg = img->pixmap;
    int i;
#endif
    if (self == NULL)
	return 0;

    if (img->pixmap == NULL)
	return 0;

    xi = (struct x11_info *)self->priv;
    if (xi == NULL)
	return 0;

#if defined(USE_IMLIB) || defined(USE_IMLIB2)
    XCopyArea(xi->display, (Pixmap) img->pixmap, xi->window, xi->imageGC,
	      sx, sy,
	      (sw ? sw : img->width),
	      (sh ? sh : img->height), x + self->offset_x, y + self->offset_y);
#elif defined(USE_GDKPIXBUF)
#define WAIT_CNT 4
    if (ximg->delay <= 0)
	i = ximg->total - 1;
    else
	i = ximg->no;
    XCopyArea(xi->display, ximg->pixmap[i], xi->window, xi->imageGC,
	      sx, sy,
	      (sw ? sw : img->width),
	      (sh ? sh : img->height), x + self->offset_x, y + self->offset_y);
    if (ximg->total > 1) {
	if (ximg->wait > WAIT_CNT) {
	    ximg->wait = 0;
	    if (i < ximg->total - 1)
		ximg->no = i + 1;
	    else
		ximg->no = 0;
	}
	ximg->wait += 1;
    }
#endif
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
#if defined(USE_IMLIB) || defined(USE_IMLIB2)
    if (img && img->pixmap) {
	XFreePixmap(xi->display, (Pixmap) img->pixmap);
	img->pixmap = NULL;
	img->width = 0;
	img->height = 0;
    }
#elif defined(USE_GDKPIXBUF)
    if (img && img->pixmap) {
	struct x11_image *ximg = img->pixmap;
	int i, n;
	if (ximg->pixmap) {
	    n = ximg->total;
	    for (i = 0; i < n; i++) {
		if (ximg->pixmap[i])
		    XFreePixmap(xi->display, (Pixmap) ximg->pixmap[i]);
	    }
	    free(ximg->pixmap);
	}
	free(ximg);
	img->pixmap = NULL;
	img->width = 0;
	img->height = 0;
    }
#endif
}

static int
x11_get_image_size(w3mimg_op * self, W3MImage * img, char *fname, int *w,
		   int *h)
{
    struct x11_info *xi;
#if defined(USE_IMLIB)
    ImlibImage *im;
#elif defined(USE_IMLIB2)
    Imlib_Image im;
#elif defined(USE_GDKPIXBUF)
    GdkPixbufAnimation *animation;
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
#elif defined(USE_IMLIB2)
    im = imlib_load_image(fname);
    if (im == NULL)
	return 0;

    imlib_context_set_image(im);
    *w = imlib_image_get_width();
    *h = imlib_image_get_height();
    imlib_free_image();
#elif defined(USE_GDKPIXBUF)
#if defined(USE_GTK2)
    animation = gdk_pixbuf_animation_new_from_file(fname, NULL);
#else
    animation = gdk_pixbuf_animation_new_from_file(fname);
#endif
    if (!animation)
	return 0;

    get_animation_size(animation, w, h, NULL);
#if defined(USE_GTK2)
    g_object_unref(G_OBJECT(animation));
#else
    gdk_pixbuf_animation_unref(animation);
#endif
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
    wop->clear = x11_clear;

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
