/* $Id: fb_w3mimg.c,v 1.14 2004/08/04 17:32:28 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "w3mimg/fb/fb.h"
#include "w3mimg/fb/fb_img.h"
#include "w3mimg/w3mimg.h"

static int
w3mfb_init(w3mimg_op * self)
{
    if (self == NULL)
	return 0;
    /* XXX */
    return 1;
}

static int
w3mfb_finish(w3mimg_op * self)
{
    if (self == NULL)
	return 0;
    return 1;
}

static int
w3mfb_active(w3mimg_op * self)
{
    if (self == NULL)
	return 0;
    return 1;
}

static void
w3mfb_set_background(w3mimg_op * self, char *background)
{
    if (self == NULL)
	return;
    if (background) {
	int r, g, b;
	if (sscanf(background, "#%02x%02x%02x", &r, &g, &b) == 3)
	    fb_image_set_bg(r, g, b);
    }
}

static void
w3mfb_sync(w3mimg_op * self)
{
    return;
}

static void
w3mfb_close(w3mimg_op * self)
{
    fb_close();
}

static int
w3mfb_clear(w3mimg_op * self, int x, int y, int w, int h)
{
    if (self == NULL)
	return 0;
    fb_image_clear(x, y, w, h);
    return 1;
}

static int
w3mfb_load_image(w3mimg_op * self, W3MImage * img, char *fname, int w, int h)
{
    FB_IMAGE **im;

    if (self == NULL)
	return 0;
    im = fb_image_load(fname, w, h, self->max_anim);
    if (!im)
	return 0;
    img->pixmap = im;
    img->width = im[0]->width;
    img->height = im[0]->height;
    return 1;
}

static int
w3mfb_show_image(w3mimg_op * self, W3MImage * img, int sx, int sy,
		 int sw, int sh, int x, int y)
{
    int i;
    FB_IMAGE **frame;
#define WAIT_CNT 4

    if (self == NULL)
	return 0;

    if (img->pixmap == NULL)
	return 0;

    frame = (FB_IMAGE **) img->pixmap;
    i = frame[0]->id;
    fb_image_draw(frame[i],
		  x + self->offset_x, y + self->offset_y,
		  sx, sy, (sw ? sw : img->width), (sh ? sh : img->height));
    if (frame[0]->num > 1) {
	if (frame[1]->id > WAIT_CNT) {
	    frame[1]->id = 0;
	    if (i < frame[0]->num - 1)
		frame[0]->id = i + 1;
	    else
		frame[0]->id = 0;
	}
	frame[1]->id += 1;
    }
    return 1;
}

static void
w3mfb_free_image(w3mimg_op * self, W3MImage * img)
{
    if (self == NULL)
	return;
    if (img && img->pixmap) {
	fb_frame_free((FB_IMAGE **) img->pixmap);
	img->pixmap = NULL;
	img->width = 0;
	img->height = 0;
    }
}

static int
w3mfb_get_image_size(w3mimg_op * self, W3MImage * img,
		     char *fname, int *w, int *h)
{
    int i;

    if (self == NULL)
	return 0;
    i = get_image_size(fname, w, h);
    if (i)
	return 0;
    return 1;
}

#ifdef W3MIMGDISPLAY_SETUID
static int
check_tty_console(char *tty)
{
    if (tty == NULL || *tty == '\0')
	return 0;
    if (strncmp(tty, "/dev/", 5) == 0)
	tty += 5;
#if defined(__linux__)
    if (strncmp(tty, "tty", 3) == 0 && isdigit(*(tty + 3)))
	return 1;
    if (strncmp(tty, "vc/", 3) == 0 && isdigit(*(tty + 3)))
	return 1;
#elif defined(__FreeBSD__)
    if (strncmp(tty, "ttyv", 4) == 0 && isxdigit(*(tty + 4)))
	return 1;
#endif
    return 0;
}
#else
#define check_tty_console(tty)	1
#endif

w3mimg_op *
w3mimg_fbopen()
{
    w3mimg_op *wop = NULL;
    wop = (w3mimg_op *) malloc(sizeof(w3mimg_op));
    if (wop == NULL)
	return NULL;
    memset(wop, 0, sizeof(w3mimg_op));

    if (!check_tty_console(getenv("W3M_TTY")) &&
	strncmp("fbterm", getenv("TERM"), 6) != 0 &&
	strncmp("jfbterm", getenv("TERM"), 7) != 0) {
	fprintf(stderr, "w3mimgdisplay/fb: tty is not console\n");
	goto error;
    }

    if (fb_open())
	goto error;

    wop->width = fb_width();
    wop->height = fb_height();

    wop->init = w3mfb_init;
    wop->finish = w3mfb_finish;
    wop->active = w3mfb_active;
    wop->set_background = w3mfb_set_background;
    wop->sync = w3mfb_sync;
    wop->close = w3mfb_close;
    wop->clear = w3mfb_clear;

    wop->load_image = w3mfb_load_image;
    wop->show_image = w3mfb_show_image;
    wop->free_image = w3mfb_free_image;
    wop->get_image_size = w3mfb_get_image_size;

    /* XXX */
    fb_image_init();

    return wop;
  error:
    free(wop);
    return NULL;
}
