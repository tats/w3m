/* $Id: win_w3mimg.cpp,v 1.2 2010/12/24 09:52:06 htrb Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include <assert.h>
#include <locale.h>

#include <new>
#include <algorithm>

#include "w3mimg/w3mimg.h"
#include <windows.h>
#include <gdiplus.h>
#include <unistd.h>
#include <sys/cygwin.h>
/* GDI+ can handle BMP, GIF, JPEG, PNG and TIFF by itself. */

#define OFFSET_X	2
#define OFFSET_Y	2
#define DEBUG

#ifdef DEBUG
#define THROW_NONE throw()
#else
#define THROW_NONE
#endif

struct win_info {
    HWND window;
    Gdiplus::ARGB background_pixel;
    ULONG_PTR gdiplus_token;
    FILE *logfile;
};

struct window_list {
    HWND *wnd;
    size_t nwnd;
    size_t capacity;
};

typedef Gdiplus::CachedBitmap *cache_handle;
class win_image {
private:
    win_image(const win_image &); // decl only
    win_image &operator=(const win_image &); // decl only

    Gdiplus::Bitmap *gpbitmap;
    unsigned int nframe;
    unsigned int current;
    unsigned long tick;
    unsigned long loopcount; // zero = infinite
    unsigned long *delay; // unit: millisecond
    cache_handle *cache;

public:
    win_image() THROW_NONE;
    ~win_image() THROW_NONE;
    int load(w3mimg_op *wop, Gdiplus::Bitmap **p_gpbitmap,
	    int *wreturn, int *hreturn) THROW_NONE;
    int show(w3mimg_op *wop, int sx, int sy, int sw, int sh, int x, int y) THROW_NONE;
    int animate(w3mimg_op *wop) THROW_NONE;
};

static int win_init(w3mimg_op * wop) THROW_NONE;
static int win_finish(w3mimg_op * wop) THROW_NONE;
static int win_active(w3mimg_op * wop) THROW_NONE;
static void win_set_background(w3mimg_op * wop, char *background) THROW_NONE;
static void win_sync(w3mimg_op * wop) THROW_NONE;
static void win_close(w3mimg_op * wop) THROW_NONE;

static int win_load_image(w3mimg_op * wop, W3MImage * img, char *fname,
	int w, int h) THROW_NONE;
static int win_show_image(w3mimg_op * wop, W3MImage * img,
	int sx, int sy, int sw, int sh, int x, int y) THROW_NONE;
static void win_free_image(w3mimg_op * wop, W3MImage * img) THROW_NONE;
static int win_get_image_size(w3mimg_op * wop, W3MImage * img,
	char *fname, int *w, int *h) THROW_NONE;
static int win_clear(w3mimg_op * wop, int x, int y, int w, int h) THROW_NONE;

static int window_alive(w3mimg_op *wop) THROW_NONE;
static Gdiplus::Bitmap *read_image_file(w3mimg_op *wop, const char *fname) THROW_NONE;
static BOOL CALLBACK store_to_window_list(HWND hWnd, LPARAM wndlist) THROW_NONE;
static void clear_window_list(struct window_list *wl) THROW_NONE;
static const char *gdip_strerror(Gdiplus::Status status) THROW_NONE;
static void gdip_perror(w3mimg_op *wop, Gdiplus::Status status, const char *func) THROW_NONE;
static char *win32_strerror_alloc(DWORD status) THROW_NONE;
static void win32_perror(w3mimg_op *wop, DWORD status, const char *func) THROW_NONE;
#if 0 /* unused */
static WCHAR *mb2wstr_alloc(const char *) THROW_NONE;
static char *wstr2mb_alloc(const WCHAR *) THROW_NONE;
#endif

#define PRELUDE(wop, xi) \
    assert(wop); \
    struct win_info *xi = static_cast<struct win_info *>(wop->priv); \
    assert(xi)

win_image::win_image() THROW_NONE
    : gpbitmap(NULL), nframe(0)
{}

win_image::~win_image() THROW_NONE
{
    if (this->cache) {
	for (size_t i = 0; i != this->nframe; ++i) {
	    delete this->cache[i];
	}
	delete[] this->cache;
    }
    delete[] this->delay;
    delete this->gpbitmap;
}

int
win_image::load(w3mimg_op *wop, Gdiplus::Bitmap **p_gpbitmap, int *wreturn, int *hreturn) THROW_NONE
{
    PRELUDE(wop, xi);
    Gdiplus::Bitmap *gpbitmap = *p_gpbitmap;
    assert(gpbitmap);
    Gdiplus::Status status = Gdiplus::Ok;
    int retval = 0;

    Gdiplus::PropertyItem *loopcountbuf = NULL;
    Gdiplus::PropertyItem *delaybuf = NULL;
    unsigned long *delay = NULL;
    cache_handle *cache = NULL;

    if (xi->logfile) {
	fprintf(xi->logfile, "win_image::load(%p, %p, %p, %p) start\n",
		wop, gpbitmap, wreturn, hreturn);
    }
    {
	unsigned int width = gpbitmap->GetWidth();
	unsigned int height = gpbitmap->GetHeight();
	unsigned int nframe = gpbitmap->GetFrameCount(&Gdiplus::FrameDimensionTime);
	unsigned long loopcount = 0;
	unsigned int first_frame = 0;

	if (xi->logfile)
	    fprintf(xi->logfile, "win_image::load(): size[0]=%ux%u\n", width, height);
	if (nframe == 0) {
	    // Not an animated picture
	    if (xi->logfile)
		fprintf(xi->logfile, "win_image::load(): zero frame count\n");
	    nframe = 1;
	    delay = new(std::nothrow) unsigned long[1];
	    if (delay == NULL)
		goto last;
	    delay[0] = 0;
	} else {
	    unsigned int loopcountsize = gpbitmap->GetPropertyItemSize(PropertyTagLoopCount);
	    unsigned int delaysize = gpbitmap->GetPropertyItemSize(PropertyTagFrameDelay);

	    // Get loop count
	    if (loopcountsize != 0) {
		loopcountbuf = (Gdiplus::PropertyItem *)malloc(loopcountsize);
		if (loopcountbuf == NULL)
		    goto last;
		status = gpbitmap->GetPropertyItem(PropertyTagLoopCount, loopcountsize, loopcountbuf);
		if (status != Gdiplus::Ok)
		    goto gdip_error;
		if (loopcountbuf->type == PropertyTagTypeShort &&
			loopcountbuf->length >= sizeof(unsigned short)) {
		    loopcount = *(unsigned short *)loopcountbuf->value;
		} else if (loopcountbuf->type == PropertyTagTypeLong &&
			loopcountbuf->length >= sizeof(unsigned long)) {
		    loopcount = *(unsigned long *)loopcountbuf->value;
		}
	    }
	    if (xi->logfile)
		fprintf(xi->logfile, "win_image::load(): loopcount=%lu\n", loopcount);
	    // Get delay times
	    if (delaysize != 0) {
		delaybuf = (Gdiplus::PropertyItem *)malloc(delaysize);
		if (delaybuf == NULL)
		    goto last;
		status = gpbitmap->GetPropertyItem(PropertyTagFrameDelay, delaysize, delaybuf);
		if (status != Gdiplus::Ok)
		    goto gdip_error;
		delay = new(std::nothrow) unsigned long[nframe];
		if (delay == NULL)
		    goto last;
		std::fill(delay, delay + nframe, 0);
		if (delaybuf->type == PropertyTagTypeShort) {
		    unsigned int count = delaybuf->length / sizeof(unsigned short);
		    for (unsigned int i = 0; i != count; ++i)
			delay[i] = ((unsigned short *)delaybuf->value)[i] * 10;
		} else if (delaybuf->type == PropertyTagTypeLong) {
		    unsigned int count = delaybuf->length / sizeof(unsigned long);
		    for (unsigned int i = 0; i != count; ++i)
			delay[i] = ((unsigned long *)delaybuf->value)[i] * 10;
		}
	    }
	    if (xi->logfile) {
		for (unsigned int i = 0; i != nframe; ++i)
		    fprintf(xi->logfile, "win_image::load(): delay[%u]=%lu\n", i, delay[i]);
	    }
	    // Get dimensions
	    for (unsigned int nextframe = 1; nextframe != nframe; ++nextframe) {
		status = gpbitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, nextframe);
		if (status != Gdiplus::Ok) {
		    if (xi->logfile)
			fprintf(xi->logfile, "win_image::load(): SelectActiveFrame() to %u failed = %d: %s\n",
				nextframe, (int)status, gdip_strerror(status));
		    goto last;
		}
		unsigned int iw = gpbitmap->GetWidth();
		unsigned int ih = gpbitmap->GetHeight();
		if (iw > width)
		    width = iw;
		if (ih > height)
		    height = ih;
		if (xi->logfile)
		    fprintf(xi->logfile, "win_image::load(): size[%u]=%ux%u\n", nextframe, iw, ih);
	    }
	    // Go to the first frame
	    first_frame = (0 < -wop->max_anim && -wop->max_anim < nframe) ? (nframe + wop->max_anim) : 0;
	    status = gpbitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, first_frame);
	    if (status != Gdiplus::Ok) {
		if (xi->logfile)
		    fprintf(xi->logfile, "win_image::load(): SelectActiveFrame() to %u frame = %d: %s\n",
			    first_frame, (int)status, gdip_strerror(status));
		goto last;
	    }
	}
	// Allocate cache array
	cache = new(std::nothrow) cache_handle[nframe];
	if (cache == NULL)
	    goto last;
	std::fill(cache, cache + nframe, (cache_handle)NULL);
	// Sanity check
	if (width > SHRT_MAX || height > SHRT_MAX) {
	    if (xi->logfile)
		fprintf(xi->logfile, "win_image::load(): too big image: %ux%u\n", width, height);
	    goto last;
	}
	// Store the results
	if (wreturn)
	    *wreturn = (int)width;
	if (hreturn)
	    *hreturn = (int)height;
	this->gpbitmap = gpbitmap;
	*p_gpbitmap = NULL; // ownership transfer
	this->nframe = nframe;
	this->current = first_frame;
	this->tick = 0;
	this->loopcount = loopcount;
	this->delay = delay;
	delay = NULL; // ownership transfer
	this->cache = cache;
	cache = NULL; // ownership transfer
	retval = 1;
    }
    goto last;
    
gdip_error:
    gdip_perror(wop, status, "win_image::load");
    goto last;
last:
    delete[] cache;
    delete[] delay;
    free(delaybuf);
    free(loopcountbuf);
    if (xi->logfile)
	fprintf(xi->logfile, "win_image::load() = %d\n", retval);
    return retval;
}

int
win_image::show(w3mimg_op *wop, int sx, int sy, int sw, int sh, int x, int y) THROW_NONE
{
    PRELUDE(wop, xi);
    int retval = 0;
    Gdiplus::Status status = Gdiplus::Ok;
    cache_handle newcache = NULL;

    if (xi->logfile)
	fprintf(xi->logfile, "win_image::show(%p, %d, %d, %d, %d, %d, %d) start current=%u\n",
		wop, sx, sy, sw, sh, x, y, this->current);
    if (!window_alive(wop))
	goto last;
    {
	int xx = x + wop->offset_x;
	int yy = y + wop->offset_y;

	// Prepare the Graphics object for painting
	Gdiplus::Graphics graphics(xi->window);
	if ((status = graphics.GetLastStatus()) != Gdiplus::Ok)
	    goto gdip_error;
	Gdiplus::Rect clip(xx, yy, sw, sh);
	status = graphics.SetClip(clip);
	if (status != Gdiplus::Ok)
	    goto gdip_error;

	unsigned int retry_count = 2;
	do {
	    if (this->cache[this->current] == NULL) {
		// Cache the image
		Gdiplus::Bitmap tmp_bitmap(sw, sh, &graphics);
		if ((status = tmp_bitmap.GetLastStatus()) != Gdiplus::Ok)
		    goto gdip_error;
		Gdiplus::Graphics tmp_graphics(&tmp_bitmap);
		if ((status = tmp_graphics.GetLastStatus()) != Gdiplus::Ok)
		    goto gdip_error;
		status = tmp_graphics.Clear(Gdiplus::Color(xi->background_pixel));
		if (status != Gdiplus::Ok)
		    goto gdip_error;
		status = tmp_graphics.DrawImage(this->gpbitmap, 0, 0, sw, sh);
		if (status != Gdiplus::Ok)
		    goto gdip_error;
		Gdiplus::CachedBitmap *newcache = new Gdiplus::CachedBitmap(&tmp_bitmap, &graphics);
		if (newcache == NULL)
		    goto last;
		if ((status = newcache->GetLastStatus()) != Gdiplus::Ok)
		    goto gdip_error;
		this->cache[this->current] = newcache;
		newcache = NULL; // ownership transfer
		--retry_count;
	    }
	    // Draw it
	    status = graphics.DrawCachedBitmap(this->cache[this->current], xx - sx, yy - sy);
	    if (status == Gdiplus::Ok)
		break;
	    // maybe the user altered the display configuration
	    if (xi->logfile)
		fprintf(xi->logfile, "win_image::show(): stale cache = %d: %s\n",
			(int)status, gdip_strerror(status));
	    delete this->cache[this->current];
	    this->cache[this->current] = NULL;
	    if (retry_count == 0)
		goto last;
	} while (1);

	retval = 1;
    }
    goto last;
gdip_error:
    gdip_perror(wop, status, "win_image::show");
    goto last;
last:
    delete newcache;
    if (xi->logfile)
	fprintf(xi->logfile, "win_image::show() = %d\n", retval);
    return retval;
}

int
win_image::animate(w3mimg_op * wop) THROW_NONE
{
    PRELUDE(wop, xi);
    int retval = 0;
    Gdiplus::Status status = Gdiplus::Ok;

    if (xi->logfile)
	fprintf(xi->logfile, "win_image::animate(%p) start\n", wop);
    {
	if (this->nframe <= 1)
	    goto animation_end;
#define UNIT_TICK 50
#define MIN_DELAY (UNIT_TICK*2)
	this->tick += UNIT_TICK;
	if (this->tick >= MIN_DELAY && this->tick >= this->delay[this->current]) {
	    this->tick = 0;
	    unsigned int nextframe = this->current + 1;
	    if (wop->max_anim == nextframe)
		goto animation_end;
	    if (nextframe >= this->nframe) {
		if (this->loopcount == 1 || wop->max_anim < 0) // end of the loop
		    goto animation_end;
		nextframe = 0;
	    }
	    status = this->gpbitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, nextframe);
	    if (status != Gdiplus::Ok)
		goto gdip_error;
	    this->current = nextframe;
	    if (nextframe == 0 && this->loopcount > 1)
		--this->loopcount;
	}
animation_end:
	retval = 1;
    }
    goto last;
gdip_error:
    gdip_perror(wop, status, "win_image::animate");
    goto last;
last:
    if (xi->logfile)
	fprintf(xi->logfile, "win_image::animate() = %d\n", retval);
    return retval;
}

static int
window_alive(w3mimg_op *wop) THROW_NONE
{
    PRELUDE(wop, xi);
    if (xi->window == NULL)
	return 0;
    if (IsWindow(xi->window))
	return 1;
    xi->window = NULL;
    fputs("w3mimgdisplay: target window disappeared\n", stderr);
    if (xi->logfile)
	fputs("w3mimgdisplay: target window disappeared\n", xi->logfile);
    return 0;
}

static int
win_init(w3mimg_op *) THROW_NONE
{
    // nothing to do
    return 1;
}

static int
win_finish(w3mimg_op *) THROW_NONE
{
    // nothing to do
    return 1;
}

static int
win_clear(w3mimg_op *wop, int x, int y, int w, int h) THROW_NONE
{
    PRELUDE(wop, xi);
    Gdiplus::Status status = Gdiplus::Ok;
    int retval = 0;

    if (xi->logfile)
	fprintf(xi->logfile, "win_clear(%p, %d, %d, %d, %d) start\n",
		wop, x, y, w, h);
    if (!window_alive(wop))
	goto last;
    {
	if (x < 0)
	    x = 0;
	if (y < 0)
	    y = 0;
	Gdiplus::SolidBrush brush(Gdiplus::Color(xi->background_pixel));
	if ((status = brush.GetLastStatus()) != Gdiplus::Ok)
	    goto gdip_error;
	Gdiplus::Graphics graphics(xi->window);
	if ((status = graphics.GetLastStatus()) != Gdiplus::Ok)
	    goto gdip_error;
	status = graphics.FillRectangle(&brush, x + wop->offset_x, y + wop->offset_y, w, h);
	if (status != Gdiplus::Ok)
	    goto gdip_error;
	retval = 1;
    }
    goto last;
gdip_error:
    gdip_perror(wop, status, "win_clear");
    goto last;
last:
    if (xi->logfile)
	fprintf(xi->logfile, "win_clear() = %d\n", retval);
    return retval;
}

static int
win_active(w3mimg_op * wop) THROW_NONE
{
    return window_alive(wop);
}

static void
win_set_background(w3mimg_op * wop, char *background) THROW_NONE
{
    PRELUDE(wop, xi);

    HDC windc = NULL;

    if (xi->logfile)
	fprintf(xi->logfile, "win_set_background(%p, \"%s\")\n", wop, background ? background : "(auto)");
    {
	// Fallback value
	// xi->background_pixel = Gdiplus::Color::White;
	xi->background_pixel = Gdiplus::Color::Black;

	// Explicit
	if (background) {
	    unsigned int r, g, b;
	    if (sscanf(background, "#%02x%02x%02x", &r, &g, &b) == 3) {
		xi->background_pixel = Gdiplus::Color::MakeARGB((BYTE)255, (BYTE)r, (BYTE)g, (BYTE)b);
		goto last;
	    }
	}

	// Auto detect
	if (xi->window == NULL || !IsWindow(xi->window))
	    goto last;
	windc = GetDC(xi->window);
	if (windc == NULL)
	    goto win32_error;
	COLORREF c = GetPixel(windc,
		    (wop->offset_x >= 1) ? (wop->offset_x - 1) : 0,
		    (wop->offset_y >= 1) ? (wop->offset_y - 1) : 0);
	xi->background_pixel = Gdiplus::Color::MakeARGB(
		(BYTE)255, GetRValue(c), GetGValue(c), GetBValue(c));
    }
    goto last;
win32_error:
    win32_perror(wop, GetLastError(), "win_set_background");
    goto last;
last:
    if (xi->logfile)
	fprintf(xi->logfile, "win_set_background() result = #%06x\n",
		(unsigned int)xi->background_pixel);
    if (windc)
	ReleaseDC(xi->window, windc);
}

static void
win_sync(w3mimg_op *) THROW_NONE
{
    // nothing to do
    return;
}

static void
win_close(w3mimg_op * wop) THROW_NONE
{
    PRELUDE(wop, xi);

    if (xi->gdiplus_token)
	Gdiplus::GdiplusShutdown(xi->gdiplus_token);
    if (xi->logfile) {
	fprintf(xi->logfile, "win_close(%p)\n", wop);
	fclose(xi->logfile);
    }
    delete xi;
    delete wop;
}

static Gdiplus::Bitmap *
read_image_file(w3mimg_op *wop, const char *fname) THROW_NONE
{
    PRELUDE(wop, xi);
    Gdiplus::Status status = Gdiplus::Ok;
    Gdiplus::Bitmap *retval = NULL;

    WCHAR *wfname = NULL;
    Gdiplus::Bitmap *gpbitmap = NULL;

    if (xi->logfile)
	fprintf(xi->logfile, "read_image_file(%p, \"%s\") start\n", wop, fname);
    {
	wfname = (WCHAR *)cygwin_create_path(CCP_POSIX_TO_WIN_W, fname);
	if (wfname == NULL)
	    goto last;
	gpbitmap = new Gdiplus::Bitmap(wfname);
	if (gpbitmap == NULL)
	    goto last;
	status = gpbitmap->GetLastStatus();
	switch (status) {
	    case Gdiplus::Ok:
		break;
	    case Gdiplus::UnknownImageFormat:
	    case Gdiplus::FileNotFound:
		goto last; // fail silently
	    default:
		goto gdip_error;
	}
	retval = gpbitmap;
	gpbitmap = NULL; // ownership transfer
    }
    goto last;
gdip_error:
    gdip_perror(wop, status, "read_image_file");
last:
    delete gpbitmap;
    free(wfname);
    if (xi->logfile)
	fprintf(xi->logfile, "read_image_file() = %p\n", retval);
    return retval;
}

static int
win_load_image(w3mimg_op * wop, W3MImage * img, char *fname, int w, int h) THROW_NONE
{
    PRELUDE(wop, xi);
    int retval = 0;
    Gdiplus::Bitmap *gpbitmap = NULL;
    win_image *wimg = NULL;

    assert(img);
    if (xi->logfile) {
	fprintf(xi->logfile, "win_load_image(%p, %p, \"%s\", %d, %d) start\n",
		wop, img, fname, w, h);
    }
    {
	gpbitmap = read_image_file(wop, fname);
	if (gpbitmap == NULL)
	    goto last;
	int iw, ih;
	wimg = new(std::nothrow) win_image;
	if (!wimg->load(wop, &gpbitmap, &iw, &ih))
	    goto last;
	img->pixmap = wimg;
	wimg = NULL; // ownership transfer
	img->width = (0 <= w && w < iw) ? w : iw;
	img->height = (0 <= h && h < ih) ? h : ih;
	retval = 1;
    }
    goto last;
last:
    delete wimg;
    delete gpbitmap;
    if (xi->logfile)
	fprintf(xi->logfile, "win_load_image() = %d\n", retval);
    return retval;
}

static int
win_show_image(w3mimg_op * wop, W3MImage * img, int sx, int sy, int sw,
	       int sh, int x, int y) THROW_NONE
{
    PRELUDE(wop, xi);
    int retval = 0;

    assert(img);
    win_image *wimg = static_cast<win_image *>(img->pixmap);
    assert(wimg);
    
    if (xi->logfile)
	fprintf(xi->logfile, "win_show_image(%p, %p, %d, %d, %d, %d, %d, %d) start\n",
		wop, img, sx, sy, sw, sh, x, y);
    int sww = sw ? sw : img->width;
    int shh = sh ? sh : img->height;
    retval = wimg->show(wop, sx, sy, sww, shh, x, y)
	&& wimg->animate(wop);
    if (xi->logfile)
	fprintf(xi->logfile, "win_show_image() = %d\n", retval);
    return retval;
}

static void
win_free_image(w3mimg_op * wop, W3MImage * img) THROW_NONE
{
    PRELUDE(wop, xi);

    assert(img);
    if (xi->logfile)
	fprintf(xi->logfile, "win_free_image(%p, %p) pixmap=%p\n", wop, img, img->pixmap);
    delete static_cast<win_image *>(img->pixmap);
    img->pixmap = NULL;
    img->width = 0;
    img->height = 0;
}

static int
win_get_image_size(w3mimg_op * wop, W3MImage *img_unused, char *fname, int *w, int *h) THROW_NONE
{
    PRELUDE(wop, xi);
    int retval = 0;
    Gdiplus::Bitmap *gpbitmap = NULL;
    win_image *wimg = NULL;

    if (xi->logfile) {
	fprintf(xi->logfile, "win_get_image_size(%p, %p, \"%s\", %p, %p) start\n",
		wop, img_unused, fname, w, h);
    }
    {
	gpbitmap = read_image_file(wop, fname);
	if (gpbitmap == NULL)
	    goto last;
	wimg = new(std::nothrow) win_image;
	if (wimg == NULL)
	    goto last;
	retval = wimg->load(wop, &gpbitmap, w, h);;
    }
    goto last;
last:
    delete wimg;
    delete gpbitmap;
    if (xi->logfile)
	fprintf(xi->logfile, "win_get_image_size() = %d\n", retval);
    return retval;
}

extern "C" w3mimg_op *
w3mimg_winopen()
{
    w3mimg_op *retval = NULL;
    Gdiplus::Status status = Gdiplus::Ok;

    w3mimg_op *wop = NULL;
    struct win_info *xi = NULL;
    struct window_list children = { NULL, 0, 0 };

    {
	// Quit if running on X
	const char *display_name;
	if ((display_name = getenv("DISPLAY")) != NULL &&
		display_name[0] && strcmp(display_name, ":0") != 0)
	    return NULL;

	// Allocate the context objects
	wop = new(std::nothrow) w3mimg_op(); // call the default ctor instead of "new w3mimg_op;"
	if (wop == NULL)
	    return NULL;
	wop->priv = xi = new(std::nothrow) win_info();
	if (xi == NULL)
	    goto last;

	// Debug logging
	const char *logging_dir;
	if ((logging_dir = getenv("W3MIMG_LOGDIR")) != NULL &&
		logging_dir[0]) {
	    size_t l = strlen(logging_dir) + sizeof "/w3mimgXXXXXXXXXX.log";
	    char *fname = (char *)malloc(l);
	    snprintf(fname, l, "%s/w3mimg%d.log", logging_dir, (int)getpid());
	    xi->logfile = fopen(fname, "a");
	    if (xi->logfile) {
		setvbuf(xi->logfile, NULL, _IONBF, 0);
		fprintf(xi->logfile, "\nw3mimg_winopen() start pid=%d\n", (int)getpid());
	    }
	}

	// Look for the window to draw the image
	xi->window = NULL;
	const char *windowid;
	if ((windowid = getenv("WINDOWID")) != NULL)
	    xi->window = FindWindowA(windowid, NULL);
	if (!xi->window)
	    xi->window = GetForegroundWindow();
	if (!xi->window)
	    goto win32_error;

	WINDOWINFO winfo = WINDOWINFO();
	winfo.cbSize = sizeof winfo;
	GetWindowInfo(xi->window, &winfo);
	wop->width = (int)(winfo.rcClient.right - winfo.rcClient.left);
	wop->height = (int)(winfo.rcClient.bottom - winfo.rcClient.top);

	// Search decendant windows and find out which is the text window
	while (1) {
	    HWND p_window = xi->window;

	    clear_window_list(&children);
	    EnumChildWindows(xi->window, &store_to_window_list, (LPARAM)&children);
	    for (unsigned int i = 0; i < children.nwnd; i++) {
		int width, height;

		GetWindowInfo(children.wnd[i], &winfo);
		width = (int)(winfo.rcClient.right - winfo.rcClient.left);
		height = (int)(winfo.rcClient.bottom - winfo.rcClient.top);
		if (width > wop->width * 0.7 &&
			height > wop->height * 0.7) {
		    /* maybe text window */
		    wop->width = width;
		    wop->height = height;
		    xi->window = children.wnd[i];
		}
	    }
	    if (p_window == xi->window)
		break;
	}

	// Terminal may leave some border pixels
	wop->offset_x = OFFSET_X;
	wop->offset_y = OFFSET_Y;

	// Start up the GDI+
	Gdiplus::GdiplusStartupInput startup_input; /// default ctor
	status = Gdiplus::GdiplusStartup(&xi->gdiplus_token, &startup_input, NULL);
	if (status != Gdiplus::Ok)
	    goto gdip_error;

	// Fill the context object
	wop->init = win_init;
	wop->finish = win_finish;
	wop->active = win_active;
	wop->set_background = win_set_background;
	wop->sync = win_sync;
	wop->close = win_close;
	wop->clear = win_clear;

	wop->load_image = win_load_image;
	wop->show_image = win_show_image;
	wop->free_image = win_free_image;
	wop->get_image_size = win_get_image_size;

	retval = wop; // take care of the object lifetime
    }
    goto last;
win32_error:
    win32_perror(wop, GetLastError(), "w3mimg_winopen");
    goto last;
gdip_error:
    gdip_perror(wop, status, "w3mimg_winopen");
    goto last;
last:
    if (xi && xi->logfile)
	fprintf(xi->logfile, "w3mimg_winopen() = %p\n", retval);
    clear_window_list(&children);
    if (!retval) {
	if (xi) {
	    if (xi->gdiplus_token)
		Gdiplus::GdiplusShutdown(xi->gdiplus_token);
	    if (xi->logfile)
		fclose(xi->logfile);
	    delete xi;
	}
	delete wop;
    }
    return retval;
}

static BOOL CALLBACK
store_to_window_list(HWND hWnd, LPARAM wndlist) THROW_NONE
{
    struct window_list *wl = (struct window_list *)wndlist;

    if (wl->nwnd >= wl->capacity) {
	size_t newsize = (wl->capacity < 4 ) ? 4 : (wl->capacity * 2);
	HWND *newbuf = (HWND *)realloc(wl->wnd, newsize * sizeof newbuf[0]);
	if (newbuf == NULL) {
	    clear_window_list(wl);
	    return FALSE;
	}
	wl->wnd = newbuf;
	wl->capacity = newsize;
    }
    wl->wnd[wl->nwnd++] = hWnd;
    return TRUE;
}

static void
clear_window_list(struct window_list *wl) THROW_NONE
{
    free(wl->wnd);
    wl->wnd = NULL;
    wl->nwnd = 0;
    wl->capacity = 0;
}

static const char *
gdip_strerror(Gdiplus::Status status) THROW_NONE
{
    size_t i;
    struct status_rec {
	Gdiplus::Status code;
	const char *str;
    };
    static const struct status_rec table[] = {
#define ERRITEM(s) { Gdiplus::s, #s }
	ERRITEM(Ok),
	ERRITEM(GenericError),
	ERRITEM(InvalidParameter),
	ERRITEM(OutOfMemory),
	ERRITEM(ObjectBusy),
	ERRITEM(InsufficientBuffer),
	ERRITEM(NotImplemented),
	ERRITEM(Win32Error),
	ERRITEM(WrongState),
	ERRITEM(Aborted),
	ERRITEM(FileNotFound),
	ERRITEM(ValueOverflow),
	ERRITEM(AccessDenied),
	ERRITEM(UnknownImageFormat),
	ERRITEM(FontFamilyNotFound),
	ERRITEM(FontStyleNotFound),
	ERRITEM(NotTrueTypeFont),
	ERRITEM(UnsupportedGdiplusVersion),
	ERRITEM(GdiplusNotInitialized),
	ERRITEM(PropertyNotFound),
	ERRITEM(PropertyNotSupported),
	ERRITEM(ProfileNotFound),
#undef ERRITEM
    };
    for (i = 0; i != sizeof table / sizeof table[0]; ++i)
	if (table[i].code == status)
	    return table[i].str;
    return "unknown";
}

static void
gdip_perror(w3mimg_op *wop, Gdiplus::Status status, const char *func) THROW_NONE
{
    const char *s = gdip_strerror(status);
    fprintf(stderr, "w3mimgdisplay: GDI+ error %d: %s\n", (int)status, s);
    if (wop && wop->priv) {
	struct win_info *xi = (struct win_info *)wop->priv;
	if (xi->logfile) {
	    fprintf(xi->logfile, "%s(): GDI+ error %d: %s\n", func, (int)status, s);
	}
    }
}

// Don't free() the result; use LocalFree() instead
static char *
win32_strerror_alloc(DWORD status) THROW_NONE
{
    char *errbuf = NULL;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL, status, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
	    (LPSTR)&errbuf, 0, NULL);
    if (errbuf) {
	size_t len = strlen(errbuf);
	if (len > 0 && errbuf[len - 1] == '\n')
	    errbuf[len - 1] = '\0';
    }
    return errbuf;
}

static void
win32_perror(w3mimg_op *wop, DWORD status, const char *func) THROW_NONE
{
    char *errbuf = win32_strerror_alloc(status);
    const char *s = errbuf ? errbuf : "(unknown)";

    fprintf(stderr, "w3mimgdisplay: Win32 error %u: %s\n", (unsigned int)status, s);
    if (wop && wop->priv) {
	struct win_info *xi = (struct win_info *)wop->priv;
	if (xi->logfile) {
	    fprintf(xi->logfile, "%s(): Win32 error %u: %s\n",
		    func, (unsigned int)status, s);
	}
    }
    LocalFree(errbuf);
}

#if 0 /* unused */
static WCHAR *
mb2wstr_alloc(const char *s) THROW_NONE
{
    int len;
    WCHAR *buf = NULL;

    len = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, s, -1, NULL, 0);
    if (len <= 0) {
	fprintf(stderr, "w3mimgdisplay: unable to convert string ecode=%u\n",
		(unsigned int)GetLastError());
	goto error;
    }
    buf = (WCHAR *)malloc(len * sizeof(WCHAR)); /* including L'\0' */
    if (!buf)
	goto error;
    len = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, s, -1, buf, len);
    if (len <= 0) {
	fprintf(stderr, "w3mimgdisplay: unable to convert string ecode=%u\n",
		(unsigned int)GetLastError());
	goto error;
    }
    return buf;
error:
    free(buf);
    return NULL;
}

static char *
wstr2mb_alloc(const WCHAR *ws) THROW_NONE
{
    int len;
    char *buf = NULL;

    len = WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, ws, -1, NULL, 0, NULL, NULL);
    if (len <= 0) {
	fprintf(stderr, "w3mimgdisplay: unable to convert string ecode=%u\n",
		(unsigned int)GetLastError());
	goto error;
    }
    buf = (char *)malloc(len); /* including '\0' */
    if (!buf)
	goto error;
    len = WideCharToMultiByte(CP_OEMCP, WC_COMPOSITECHECK, ws, -1, buf, len, NULL, NULL);
    if (len <= 0) {
	fprintf(stderr, "w3mimgdisplay: unable to convert string ecode=%u\n",
		(unsigned int)GetLastError());
	goto error;
    }
    return buf;
error:
    free(buf);
    return NULL;
}
#endif /* unused */
