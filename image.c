/* $Id: image.c,v 1.2 2002/01/31 18:28:24 ukai Exp $ */

#include "fm.h"
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_WAITPID
#include <sys/wait.h>
#endif

#ifdef USE_IMAGE

static int image_index = 0;

/* display image */

typedef struct _termialImage {
    ImageCache *cache;
    short x;
    short y;
    short sx;
    short sy;
    short width;
    short height;
} TerminalImage;

static TerminalImage *terminal_image = NULL;
static int n_terminal_image = 0;
static int max_terminal_image = 0;
static FILE *Imgdisplay_rf = NULL, *Imgdisplay_wf = NULL;
static pid_t Imgdisplay_pid = 0;
static int openImgdisplay();
static void closeImgdisplay();
int getCharSize();

void
initImage()
{
    if (activeImage)
	return;
    if (getCharSize())
	activeImage = TRUE;
}

int
getCharSize()
{
    FILE *f;
    Str tmp;
    int w = 0, h = 0;

    tmp = Strnew();
    if (!strchr(Imgdisplay, '/'))
	Strcat_m_charp(tmp, LIB_DIR, "/", NULL);
    Strcat_m_charp(tmp, Imgdisplay, " -test 2> /dev/null", NULL);
    f = popen(tmp->ptr, "r");
    if (!f)
	return FALSE;
    fscanf(f, "%d %d", &w, &h);
    pclose(f);
    if (!(w > 0 && h > 0))
	return FALSE;
    if (!set_pixel_per_char)
	pixel_per_char = (int)(1.0 * w / COLS + 0.5);
    if (!set_pixel_per_line)
	pixel_per_line = (int)(1.0 * h / LINES + 0.5);
    return TRUE;
}

void
termImage()
{
    if (!activeImage)
	return;
    clearImage();
    if (Imgdisplay_wf) {
	fputs("2;\n", Imgdisplay_wf);	/* ClearImage() */
	fflush(Imgdisplay_wf);
    }
    closeImgdisplay();
}

static int
openImgdisplay()
{
    int fdr[2], fdw[2];
    char *cmd;

    if (pipe(fdr) < 0)
	goto err0;
    if (pipe(fdw) < 0)
	goto err1;

    flush_tty();
    Imgdisplay_pid = fork();
    if (Imgdisplay_pid < 0)
	goto err2;
    if (Imgdisplay_pid == 0) {
	/* child */
	reset_signals();
	signal(SIGINT, SIG_IGN);
#ifdef HAVE_SETPGRP
	setpgrp();
#endif
	close_tty();
	close(fdr[0]);
	close(fdw[1]);
	dup2(fdw[0], 0);
	dup2(fdr[1], 1);
	close(2);
	if (!strchr(Imgdisplay, '/'))
	    cmd = Strnew_m_charp(LIB_DIR, "/", Imgdisplay, NULL)->ptr;
	else
	    cmd = Imgdisplay;
	execl("/bin/sh", "sh", "-c", cmd, NULL);
	exit(1);
    }
    close(fdr[1]);
    close(fdw[0]);
    Imgdisplay_rf = fdopen(fdr[0], "r");
    Imgdisplay_wf = fdopen(fdw[1], "w");
    activeImage = TRUE;
    return TRUE;
  err2:
    close(fdw[0]);
    close(fdw[1]);
  err1:
    close(fdr[0]);
    close(fdr[1]);
  err0:
    Imgdisplay_rf = NULL;
    Imgdisplay_wf = NULL;
    Imgdisplay_pid = 0;
    activeImage = FALSE;
    return FALSE;
}

static void
closeImgdisplay()
{
    if (Imgdisplay_rf)
	fclose(Imgdisplay_rf);
    if (Imgdisplay_wf)
	fclose(Imgdisplay_wf);
    if (Imgdisplay_pid)
	kill(Imgdisplay_pid, SIGKILL);
    Imgdisplay_rf = NULL;
    Imgdisplay_wf = NULL;
    Imgdisplay_pid = 0;
}

void
addImage(ImageCache * cache, int x, int y, int sx, int sy, int w, int h)
{
    TerminalImage *i;

    if (!activeImage)
	return;
    if (n_terminal_image >= max_terminal_image) {
	max_terminal_image = max_terminal_image ? (2 * max_terminal_image) : 8;
	terminal_image = New_Reuse(TerminalImage, terminal_image,
				   max_terminal_image);
    }
    i = &terminal_image[n_terminal_image];
    i->cache = cache;
    i->x = x;
    i->y = y;
    i->sx = sx;
    i->sy = sy;
    i->width = w;
    i->height = h;
    n_terminal_image++;
}

void
drawImage()
{
    static char buf[64];
    int j, draw = FALSE;
    TerminalImage *i;

    if (!activeImage)
	return;
    if (!n_terminal_image)
	return;
    for (j = 0; j < n_terminal_image; j++) {
	i = &terminal_image[j];
	if (!(i->cache->loaded == IMG_FLAG_LOADED &&
	      i->width > 0 && i->height > 0))
	    continue;
	if (!(Imgdisplay_rf && Imgdisplay_wf)) {
	    if (!openImgdisplay())
		return;
	}
	if (!draw) {
	    fputs("3;\n", Imgdisplay_wf);	/* XSync() */
	    draw = TRUE;
	}
	if (i->cache->index > 0) {
	    i->cache->index *= -1;
	    fputs("0;", Imgdisplay_wf);	/* DrawImage() */
	}
	else
	    fputs("1;", Imgdisplay_wf);	/* DrawImage(redraw) */
	sprintf(buf, "%d;%d;%d;%d;%d;%d;%d;%d;%d;",
		(-i->cache->index - 1) % MAX_IMAGE + 1, i->x, i->y,
		(i->cache->width > 0) ? i->cache->width : 0,
		(i->cache->height > 0) ? i->cache->height : 0,
		i->sx, i->sy, i->width, i->height);
	fputs(buf, Imgdisplay_wf);
	fputs(i->cache->file, Imgdisplay_wf);
	fputs("\n", Imgdisplay_wf);
	fputs("4;\n", Imgdisplay_wf);	/* put '\n' */
      again:
	if (fflush(Imgdisplay_wf) != 0) {
	    switch (errno) {
	    case EINTR:
		goto again;
	    default:
		goto err;
	    }
	}
	if (!fgetc(Imgdisplay_rf))
	    goto err;
    }
    if (!draw)
	return;
    fputs("3;\n", Imgdisplay_wf);	/* XSync() */
    fputs("4;\n", Imgdisplay_wf);	/* put '\n' */
  again2:
    if (fflush(Imgdisplay_wf) != 0) {
	switch (errno) {
	case EINTR:
	    goto again2;
	default:
	    goto err;
	}
    }
    if (!fgetc(Imgdisplay_rf))
	goto err;
    /*
     * touch_line();
     * touch_column(CurColumn);
     * #ifdef JP_CHARSET
     * if (CurColumn > 0 &&
     * CHMODE(ScreenImage[CurLine]->lineprop[CurColumn]) == C_WCHAR2)
     * touch_column(CurColumn - 1);
     * else if (CurColumn < COLS - 1 &&
     * CHMODE(ScreenImage[CurLine]->lineprop[CurColumn]) == C_WCHAR1)
     * touch_column(CurColumn + 1);
     * #endif
     */
    touch_cursor();
    refresh();
    return;
  err:
    closeImgdisplay();
    image_index += MAX_IMAGE;
}

void
clearImage()
{
    if (!activeImage)
	return;
    n_terminal_image = 0;
}

/* load image */

static Hash_sv *image_hash = NULL;
static Hash_sv *image_file = NULL;
static GeneralList *image_list = NULL;
static ImageCache *image_cache = NULL;
static pid_t image_pid = 0;
static int need_load_image = FALSE;

static MySignalHandler
load_image_handler(SIGNAL_ARG)
{
    need_load_image = TRUE;
    SIGNAL_RETURN;
}

static MySignalHandler
load_image_next(SIGNAL_ARG)
{
    need_load_image = TRUE;
    loadImage(IMG_FLAG_NEXT);
    SIGNAL_RETURN;
}

void
deleteImage(Buffer *buf)
{
    AnchorList *al;
    Anchor *a;
    int i;

    if (!buf)
	return;
    al = buf->img;
    if (!al)
	return;
    for (i = 0, a = al->anchors; i < al->nanchor; i++, a++) {
	if (a->image && a->image->cache &&
	    a->image->cache->loaded != IMG_FLAG_UNLOADED &&
	    a->image->cache->index < 0)
	    unlink(a->image->cache->file);
    }
    loadImage(IMG_FLAG_STOP);
}

void
getAllImage(Buffer *buf)
{
    AnchorList *al;
    Anchor *a;
    ParsedURL *current;
    int i;

    if (!buf)
	return;
    al = buf->img;
    if (!al)
	return;
    current = baseURL(buf);
    for (i = 0, a = al->anchors; i < al->nanchor; i++, a++) {
	if (a->image)
	    a->image->cache = getImage(a->image, current, buf->image_flag);
    }
}

void
loadImage(int flag)
{
    int wait_st;

    if (flag == IMG_FLAG_STOP) {
	if (image_pid) {
	    kill(image_pid, SIGKILL);
#ifdef HAVE_WAITPID
	    waitpid(image_pid, &wait_st, 0);
#else
	    wait(&wait_st);
#endif
	    image_pid = 0;
	}
	image_cache = NULL;
	image_list = NULL;
	image_file = NULL;
	need_load_image = FALSE;
	return;
    }

    if (flag == IMG_FLAG_NEXT && need_load_image) {
	struct stat st;
	if (image_pid) {
#ifdef HAVE_WAITPID
	    waitpid(image_pid, &wait_st, 0);
#else
	    wait(&wait_st);
#endif
	    image_pid = 0;
	}
	if (!stat(image_cache->file, &st)) {
	    image_cache->loaded = IMG_FLAG_LOADED;
	    if (getImageSize(image_cache)) {
		if (Currentbuf)
		    Currentbuf->need_reshape = TRUE;
	    }
	}
	else {
	    image_cache->loaded = IMG_FLAG_ERROR;
	}
	image_cache = NULL;
	drawImage();
    }

    need_load_image = FALSE;
    if (flag == IMG_FLAG_START)
	signal(SIGUSR1, load_image_handler);
    else
	signal(SIGUSR1, load_image_next);

    if (image_cache)
	return;

    image_pid = 0;
    if (!image_list)
	return;
    while (1) {
	image_cache = (ImageCache *) popValue(image_list);
	if (!image_cache) {
	    if (Currentbuf && Currentbuf->need_reshape)
		displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
	if (image_cache->loaded == IMG_FLAG_UNLOADED)
	    break;
    }

    flush_tty();
    if ((image_pid = fork()) == 0) {
	Buffer *b;

	reset_signals();
	signal(SIGINT, SIG_IGN);
	close_tty();
	QuietMessage = TRUE;
	fmInitialized = FALSE;
	image_source = image_cache->file;
	b = loadGeneralFile(image_cache->url, image_cache->current, NULL, 0,
			    NULL);
	if (!b || !b->real_type || strncasecmp(b->real_type, "image/", 6))
	    unlink(image_cache->file);
	kill(getppid(), SIGUSR1);
	exit(0);
    }
}

ImageCache *
getImage(Image * image, ParsedURL *current, int flag)
{
    Str key = NULL;
    ImageCache *cache;

    if (!activeImage)
	return NULL;
    if (!image_hash)
	image_hash = newHash_sv(100);
    if (image->cache)
	cache = image->cache;
    else {
	key = Sprintf("%d;%d;%s", image->width, image->height, image->url);
	cache = (ImageCache *) getHash_sv(image_hash, key->ptr, NULL);
    }
    if (cache && cache->index && abs(cache->index) <= image_index - MAX_IMAGE) {
	struct stat st;
	if (stat(cache->file, &st))
	    cache->loaded = IMG_FLAG_UNLOADED;
	cache->index = 0;
    }

    if (!cache) {
	if (flag == IMG_FLAG_SKIP)
	    return NULL;

	cache = New(ImageCache);
	cache->url = image->url;
	cache->current = current;
	cache->file = tmpfname(TMPF_DFL, image->ext)->ptr;
	cache->index = 0;
	cache->loaded = IMG_FLAG_UNLOADED;
	cache->width = image->width;
	cache->height = image->height;
	putHash_sv(image_hash, key->ptr, (void *)cache);
	pushText(fileToDelete, cache->file);
    }
    if (flag != IMG_FLAG_SKIP) {
	if (cache->loaded == IMG_FLAG_UNLOADED) {
	    if (!image_file)
		image_file = newHash_sv(100);
	    if (!getHash_sv(image_file, cache->file, NULL)) {
		putHash_sv(image_file, cache->file, (void *)cache);
		if (!image_list)
		    image_list = newGeneralList();
		pushValue(image_list, (void *)cache);
	    }
	}
	if (!cache->index)
	    cache->index = ++image_index;
    }
    if (cache->loaded == IMG_FLAG_LOADED)
	getImageSize(cache);
    return cache;
}

int
getImageSize(ImageCache * cache)
{
    Str tmp;
    FILE *f;
    int w = 0, h = 0;

    if (!activeImage)
	return 0;
    if (!cache || cache->loaded != IMG_FLAG_LOADED ||
	(cache->width > 0 && cache->height > 0))
	return 0;
    tmp = Strnew();
    if (!strchr(Imgsize, '/'))
	Strcat_m_charp(tmp, LIB_DIR, "/", NULL);
    Strcat_m_charp(tmp, Imgsize, " ", shell_quote(cache->file),
		   " 2> /dev/null", NULL);
    f = popen(tmp->ptr, "r");
    if (!f)
	return 0;
    fscanf(f, "%d %d", &w, &h);
    pclose(f);

    if (!(w > 0 && h > 0))
	return 0;
    w = (int)(w * image_scale / 100 + 0.5);
    if (w == 0)
	w = 1;
    h = (int)(h * image_scale / 100 + 0.5);
    if (h == 0)
	h = 1;
    if (cache->width < 0 && cache->height < 0) {
	cache->width = w;
	cache->height = h;
    }
    else if (cache->width < 0) {
	cache->width = (int)((double)cache->height * w / h + 0.5);
    }
    else if (cache->height < 0) {
	cache->height = (int)((double)cache->width * h / w + 0.5);
    }
    if (cache->width == 0)
	cache->width = 1;
    if (cache->height == 0)
	cache->height = 1;
    tmp = Sprintf("%d;%d;%s", cache->width, cache->height, cache->url);
    putHash_sv(image_hash, tmp->ptr, (void *)cache);
    return 1;
}
#endif
