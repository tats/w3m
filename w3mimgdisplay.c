/* $Id: w3mimgdisplay.c,v 1.1 2002/01/31 17:54:57 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <Imlib.h>

static Display *display;
static Window window, parent;
static unsigned long background_pixel;
static char *background = NULL;
static int offset_x = 2, offset_y = 2;
static int defined_bg = 0, defined_x = 0, defined_y = 0, defined_test = 0;
static int defined_debug = 0;

#define MAX_IMAGE 1000
typedef struct {
    Pixmap pixmap;
    int width;
    int height;
} Image;
static Image *imageBuf = NULL;
static int maxImage = 0;
static GC imageGC = NULL;

static void GetOption(int argc, char **argv);
static void DrawImage(char *buf, int redraw);
static void ClearImage(void);

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

int
main(int argc, char **argv)
{
    Window root, *children;
    XWindowAttributes attr;
    XColor screen_def, exact_def;
    int revert, nchildren, len, width, height, i;
    char buf[1024 + 128];
    char *id;

    GetOption(argc, argv);
    if (!defined_debug)
	fclose(stderr);

    display = XOpenDisplay(NULL);
    if (!display)
	exit(1);
    if ((id = getenv("WINDOWID")) != NULL)
	window = (Window) atoi(id);
    else
	XGetInputFocus(display, &window, &revert);
    if (!window)
	exit(1);

    XGetWindowAttributes(display, window, &attr);
    width = attr.width;
    height = attr.height;
    while (1) {
	Window p_window;

	XQueryTree(display, window, &root, &parent, &children, &nchildren);
	if (defined_debug)
	    fprintf(stderr,
		    "window=%x root=%x parent=%x nchildren=%d width=%d height=%d\n",
		    window, root, parent, nchildren, width, height);
	p_window = window;
	for (i = 0; i < nchildren; i++) {
	    XGetWindowAttributes(display, children[i], &attr);
	    if (defined_debug)
		fprintf(stderr,
			"children[%d]=%x x=%d y=%d width=%d height=%d\n", i,
			children[i], attr.x, attr.y, attr.width, attr.height);
	    if (attr.width > width * 0.7 && attr.height > height * 0.7) {
		/* maybe text window */
		width = attr.width;
		height = attr.height;
		window = children[i];
	    }
	}
	if (p_window == window)
	    break;
    }
    if (!defined_x) {
	for (i = 0; i < nchildren; i++) {
	    XGetWindowAttributes(display, children[i], &attr);
	    if (attr.x <= 0 && attr.width < 30 && attr.height > height * 0.7) {
		if (defined_debug)
		    fprintf(stderr,
			    "children[%d]=%x x=%d y=%d width=%d height=%d\n",
			    i, children[i], attr.x, attr.y, attr.width,
			    attr.height);
		/* scrollbar of xterm/kterm ? */
		offset_x += attr.x + attr.width + attr.border_width * 2;
		break;
	    }
	}
    }

    if (defined_test) {
	printf("%d %d\n", width - offset_x, height - offset_y);
	exit(0);
    }

    if (defined_bg && XAllocNamedColor(display, DefaultColormap(display, 0),
				       background, &screen_def, &exact_def))
	background_pixel = screen_def.pixel;
    else {
	Pixmap p;
	GC gc;
	XImage *i;

	p = XCreatePixmap(display, window, 1, 1, DefaultDepth(display, 0));
	gc = XCreateGC(display, window, 0, NULL);
	if (!p || !gc)
	    exit(1);
	XCopyArea(display, window, p, gc, (offset_x >= 1) ? (offset_x - 1) : 0,
		  (offset_y >= 1) ? (offset_y - 1) : 0, 1, 1, 0, 0);
	i = XGetImage(display, p, 0, 0, 1, 1, -1, ZPixmap);
	if (!i)
	    exit(1);
	background_pixel = XGetPixel(i, 0, 0);
	XDestroyImage(i);
	XFreeGC(display, gc);
	XFreePixmap(display, p);
/*
	background_pixel = WhitePixel(display, 0);
*/
    }

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
	switch (buf[0]) {
	case '0':
	    DrawImage(&buf[2], 0);
	    break;
	case '1':
	    DrawImage(&buf[2], 1);
	    break;
	case '2':
	    ClearImage();
	    break;
	case '3':
	    XSync(display, False);
	    break;
	case '4':
	    fputs("\n", stdout);
	    fflush(stdout);
	    break;
	}
    }
    ClearImage();
    /*
     * XCloseDisplay(display);
     */
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
    static ImlibData *id = NULL;
    ImlibImage *im;
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
	if (!imageGC || n >= maxImage || !imageBuf[n].pixmap)
	    return;
	goto draw_image;
    }
    if (!id) {
	id = Imlib_init(display);
	if (!id)
	    return;
    }
    if (!imageGC) {
	imageGC = XCreateGC(display, parent, 0, NULL);
	if (!imageGC)
	    return;
    }
    if (n >= maxImage) {
	int i = maxImage;
	maxImage = i ? (i * 2) : 2;
	if (maxImage > MAX_IMAGE)
	    maxImage = MAX_IMAGE;
	else if (n >= maxImage)
	    maxImage = n + 1;
	imageBuf = (Image *) realloc((void *)imageBuf,
				     sizeof(Image) * maxImage);
	for (; i < maxImage; i++)
	    imageBuf[i].pixmap = NULL;
    }
    if (imageBuf[n].pixmap) {
	XFreePixmap(display, imageBuf[n].pixmap);
	imageBuf[n].pixmap = NULL;
    }

    im = Imlib_load_image(id, p);
    if (!im)
	return;
    if (!w)
	w = im->rgb_width;
    if (!h)
	h = im->rgb_height;
    imageBuf[n].pixmap = XCreatePixmap(display, parent, w, h,
				       DefaultDepth(display, 0));
    if (!imageBuf[n].pixmap)
	return;
    XSetForeground(display, imageGC, background_pixel);
    XFillRectangle(display, imageBuf[n].pixmap, imageGC, 0, 0, w, h);
    Imlib_paste_image(id, im, imageBuf[n].pixmap, 0, 0, w, h);
    Imlib_kill_image(id, im);
    imageBuf[n].width = w;
    imageBuf[n].height = h;
  draw_image:
    XCopyArea(display, imageBuf[n].pixmap, window, imageGC,
	      sx, sy, (sw ? sw : imageBuf[n].width),
	      (sh ? sh : imageBuf[n].height), x + offset_x, y + offset_y);
}

void
ClearImage(void)
{
    if (imageGC) {
	XFreeGC(display, imageGC);
	imageGC = NULL;
    }
    if (imageBuf) {
	int i;
	for (i = 0; i < maxImage; i++) {
	    if (imageBuf[i].pixmap)
		XFreePixmap(display, imageBuf[i].pixmap);
	}
	free(imageBuf);
	imageBuf = NULL;
    }
    maxImage = 0;
}
