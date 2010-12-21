/* $Id: w3mimgdisplay.c,v 1.19 2010/12/21 10:13:55 htrb Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "config.h"
#include "w3mimg/w3mimg.h"

w3mimg_op *w_op;
static char *background = NULL;
static int offset_x = 0, offset_y = 0;
static int defined_bg = 0, defined_x = 0, defined_y = 0, defined_test = 0;
static int defined_debug = 0;
static char *defined_size = NULL;

#define MAX_IMAGE 1000
static W3MImage *imageBuf = NULL;
static int maxImage = 0, maxAnim = 100, clearMargin = 0;

static void GetOption(int argc, char **argv);
static void DrawImage(char *buf, int redraw);
static void TermImage(void);
static void ClearImage(char *buf);

int
main(int argc, char **argv)
{
    int len;
    char buf[1024 + 128];
#ifdef W3MIMGDISPLAY_SETUID
    uid_t runner_uid = getuid();
    uid_t owner_uid = geteuid();

    /* swap real and effective */
    setreuid(owner_uid, runner_uid);
#endif
    GetOption(argc, argv);
    if (!defined_debug)
	freopen(DEV_NULL_PATH, "w", stderr);

#ifdef W3MIMGDISPLAY_SETUID
    /* 
     * back real and effective
     * run w3mimg_open() in setuid privileges
     */
    setreuid(runner_uid, owner_uid);
#endif
    w_op = w3mimg_open();
#ifdef W3MIMGDISPLAY_SETUID
    /* make sure drop privileges now */
    setreuid(runner_uid, runner_uid);
#endif
    if (w_op == NULL)
	exit(1);
    if (defined_x)
	w_op->offset_x = offset_x;
    if (defined_y)
	w_op->offset_y = offset_y;

    w_op->max_anim = maxAnim;
    w_op->clear_margin = clearMargin;

    if (defined_test) {
	printf("%d %d\n", w_op->width - w_op->offset_x,
	       w_op->height - w_op->offset_y);
	w_op->close(w_op);
	exit(0);
    }

    if (defined_size) {
	if (w_op->init(w_op)) {
	    W3MImage img;
	    int w, h;
	    if (w_op->get_image_size(w_op, &img, defined_size, &w, &h))
		printf("%d %d\n", w, h);
	}
	w_op->close(w_op);
	exit(0);
    }

    w_op->set_background(w_op, background);

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
	if (!(isdigit(buf[0]) && buf[1] == ';')) {
	    fputc('\n', stdout);
	    fflush(stdout);
	    continue;
	}
	len = strlen(buf);
	if (buf[len - 1] == '\n') {
	    buf[--len] = '\0';
	    if (buf[len - 1] == '\r')
		buf[--len] = '\0';
	}
	/*
	 * w3mimg protocol
	 *  0  1  2 ....
	 * +--+--+--+--+ ...... +--+--+
	 * |op|; |args             |\n|
	 * +--+--+--+--+ .......+--+--+
	 *
	 * args is separeted by ';'
	 * op   args
	 *  0;  params          draw image
	 *  1;  params          redraw image
	 *  2;  -none-          terminate drawing
	 *  3;  -none-          sync drawing
	 *  4;  -none-          nop, sync communication
	 *                      response '\n'
	 *  5;  path            get size of image,
	 *                      response "<width> <height>\n"
	 *  6;  params(6)       clear image
	 *
	 * params
	 *      <n>;<x>;<y>;<w>;<h>;<sx>;<sy>;<sw>;<sh>;<path>
	 * params(6)
	 *      <x>;<y>;<w>;<h>
	 *   
	 */
	switch (buf[0]) {
	case '0':
	    DrawImage(&buf[2], 0);
	    break;
	case '1':
	    DrawImage(&buf[2], 1);
	    break;
	case '2':
	    TermImage();
	    break;
	case '3':
	    w_op->sync(w_op);
	    break;
	case '4':
	    fputs("\n", stdout);
	    fflush(stdout);
	    break;
	case '5':
	    if (w_op->init(w_op)) {
		W3MImage img;
		int w, h;
		if (w_op->get_image_size(w_op, &img, &buf[2], &w, &h)) {
		    fprintf(stdout, "%d %d\n", w, h);
		    fflush(stdout);
		}
		else {
		    fprintf(stdout, "\n");
		    fflush(stdout);
		}
	    }
	    else {
		fprintf(stdout, "\n");
		fflush(stdout);
	    }
	    break;
	case '6':
	    ClearImage(&buf[2]);
	    break;
	}
    }
    TermImage();
    w_op->close(w_op);
    exit(0);
}

static void
GetOption(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
	if (!strcmp("-bg", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    background = argv[i];
	    defined_bg = 1;
	}
	else if (!strcmp("-x", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    offset_x = atoi(argv[i]);
	    defined_x = 1;
	}
	else if (!strcmp("-y", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    offset_y = atoi(argv[i]);
	    defined_y = 1;
	}
	else if (!strcmp("-test", argv[i])) {
	    defined_test = 1;
	}
	else if (!strcmp("-anim", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    maxAnim = atoi(argv[i]);
	}
	else if (!strcmp("-margin", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    clearMargin = atoi(argv[i]);
	    if (clearMargin < 0)
		clearMargin = 0;
	}
	else if (!strcmp("-size", argv[i])) {
	    if (++i >= argc)
		exit(1);
	    defined_size = argv[i];
	}
	else if (!strcmp("-debug", argv[i])) {
	    defined_debug = 1;
	}
	else {
	    exit(1);
	}
    }
}

void
DrawImage(char *buf, int redraw)
{
    char *p = buf;
    int n = 0, x = 0, y = 0, w = 0, h = 0, sx = 0, sy = 0, sw = 0, sh = 0;

    if (!p)
	return;
    for (; isdigit(*p); p++)
	n = 10 * n + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	x = 10 * x + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	y = 10 * y + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	w = 10 * w + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	h = 10 * h + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	sx = 10 * sx + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	sy = 10 * sy + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	sw = 10 * sw + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	sh = 10 * sh + (*p - '0');
    if (*(p++) != ';')
	return;

    n--;
    if (n < 0 || n >= MAX_IMAGE)
	return;
    if (redraw) {
	if (!w_op->active(w_op) || n >= maxImage || !imageBuf[n].pixmap)
	    return;
	goto draw_image;
    }
    w_op->init(w_op);

    if (n >= maxImage) {
	int i = maxImage;
	maxImage = i ? (i * 2) : 2;
	if (maxImage > MAX_IMAGE)
	    maxImage = MAX_IMAGE;
	else if (n >= maxImage)
	    maxImage = n + 1;
	imageBuf = (W3MImage *) realloc((void *)imageBuf,
					sizeof(W3MImage) * maxImage);
	for (; i < maxImage; i++)
	    imageBuf[i].pixmap = NULL;
    }
    if (imageBuf[n].pixmap) {
	w_op->free_image(w_op, &imageBuf[n]);
	imageBuf[n].pixmap = NULL;
    }

    if (w_op->load_image(w_op, &imageBuf[n], p, w, h) == 0)
	imageBuf[n].pixmap = NULL;

  draw_image:
    if (imageBuf[n].pixmap)
	w_op->show_image(w_op, &imageBuf[n], sx, sy, sw, sh, x, y);
}

void
TermImage(void)
{
    w_op->finish(w_op);
    if (imageBuf) {
	int i;
	for (i = 0; i < maxImage; i++) {
	    w_op->free_image(w_op, &imageBuf[i]);
	}
	free(imageBuf);
	imageBuf = NULL;
    }
    maxImage = 0;
}

void
ClearImage(char *buf)
{
    char *p = buf;
    int x = 0, y = 0, w = 0, h = 0;

    if (!p)
	return;
    for (; isdigit(*p); p++)
	x = 10 * x + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	y = 10 * y + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	w = 10 * w + (*p - '0');
    if (*(p++) != ';')
	return;
    for (; isdigit(*p); p++)
	h = 10 * h + (*p - '0');

    w_op->clear(w_op,
		x + offset_x - w_op->clear_margin,
		y + offset_y - w_op->clear_margin,
		w + w_op->clear_margin * 2, h + w_op->clear_margin * 2);
}
