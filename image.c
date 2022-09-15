/* $Id: image.c,v 1.37 2010/12/21 10:13:55 htrb Exp $ */

#include "fm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_WAITPID
#include <sys/wait.h>
#endif


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

int get_pixel_per_cell(int *ppc, int *ppl);

int
getCharSize()
{
    FILE *f;
    Str tmp;
    int w = 0, h = 0;

    set_environ("W3M_TTY", ttyname_tty());

    if (enable_inline_image) {
	int ppc, ppl;

	if (get_pixel_per_cell(&ppc,&ppl)) {
	    pixel_per_char_i = ppc ;
	    pixel_per_line_i = ppl ;
	    pixel_per_char = (double)ppc;
	    pixel_per_line = (double)ppl;
	}
	else {
	    pixel_per_char_i = (int)pixel_per_char;
	    pixel_per_line_i = (int)pixel_per_line;
	}

	return  TRUE;
    }

    tmp = Strnew();
    if (!strchr(Imgdisplay, '/'))
	Strcat_m_charp(tmp, w3m_auxbin_dir(), "/", NULL);
    Strcat_m_charp(tmp, Imgdisplay, " -test 2>/dev/null", NULL);
    f = popen(tmp->ptr, "r");
    if (!f)
	return FALSE;
    while (fscanf(f, "%d %d", &w, &h) < 0) {
	if (feof(f))
	    break;
    }
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
    char *cmd;

    if (!strchr(Imgdisplay, '/'))
	cmd = Strnew_m_charp(w3m_auxbin_dir(), "/", Imgdisplay, NULL)->ptr;
    else
	cmd = Imgdisplay;
    Imgdisplay_pid = open_pipe_rw(&Imgdisplay_rf, &Imgdisplay_wf);
    if (Imgdisplay_pid < 0)
	goto err0;
    if (Imgdisplay_pid == 0) {
	/* child */
	setup_child(FALSE, 2, -1);
	myExec(cmd);
	/* XXX: ifdef __EMX__, use start /f ? */
    }
    activeImage = TRUE;
    return TRUE;
  err0:
    Imgdisplay_pid = 0;
    activeImage = FALSE;
    return FALSE;
}

static void
closeImgdisplay()
{
    if (Imgdisplay_wf)
	fclose(Imgdisplay_wf);
    if (Imgdisplay_rf) {
	/* sync with the child */
	getc(Imgdisplay_rf); /* EOF expected */
	fclose(Imgdisplay_rf);
    }
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

static void
syncImage(void)
{
    if (enable_inline_image) {
	return;
    }

    fputs("3;\n", Imgdisplay_wf);	/* XSync() */
    fputs("4;\n", Imgdisplay_wf);	/* put '\n' */
    while (fflush(Imgdisplay_wf) != 0) {
	if (ferror(Imgdisplay_wf))
	    goto err;
    }
    if (!fgetc(Imgdisplay_rf))
	goto err;
    return;
  err:
    closeImgdisplay();
    image_index += MAX_IMAGE;
    n_terminal_image = 0;
}

void put_image_osc5379(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh);
void put_image_sixel(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int n_terminal_image);
void put_image_iterm2(char *url, int x, int y, int w, int h);
void put_image_kitty(char *url, int x, int y, int w, int h, int sx, int sy, int
    sw, int sh, int c, int r);

void
drawImage()
{
    static char buf[64];
    int j, draw = FALSE;
    TerminalImage *i;
    struct stat st ;

    if (!activeImage)
	return;
    if (!n_terminal_image)
	return;
    for (j = 0; j < n_terminal_image; j++) {
	i = &terminal_image[j];

	if (enable_inline_image) {
	    /*
	     * So this shouldn't ever happen, but if it does then at least let's
	     * not have external programs fetch images from the Internet...
	     */
	    if (!i->cache->touch || stat(i->cache->file,&st))
	      return;

	    char *url = i->cache->file;

	    int x = i->x / pixel_per_char_i;
	    int y = i->y / pixel_per_line_i;

	    int w = i->cache->a_width > 0 ? (
		    (i->cache->width + i->x % pixel_per_char_i + pixel_per_char_i - 1) /
		    pixel_per_char_i) : 0;
	    int h = i->cache->a_height > 0 ? (
		    (i->cache->height + i->y % pixel_per_line_i + pixel_per_line_i - 1) /
		    pixel_per_line_i) : 0;

	    int sx = i->sx / pixel_per_char_i;
	    int sy = i->sy / pixel_per_line_i;

	    int sw = (i->width + i->sx % pixel_per_char_i + pixel_per_char_i - 1) /
		      pixel_per_char_i;
	    int sh = (i->height + i->sy % pixel_per_line_i + pixel_per_line_i - 1) /
		      pixel_per_line_i;



	    if (enable_inline_image == INLINE_IMG_SIXEL) {
		put_image_sixel(url, x, y, w, h, sx, sy, sw, sh, n_terminal_image);
	    } else if (enable_inline_image == INLINE_IMG_OSC5379) {
		put_image_osc5379(url, x, y, w, h, sx, sy, sw, sh);
	    } else if (enable_inline_image == INLINE_IMG_ITERM2) {
		put_image_iterm2(url, x, y, sw, sh);
	    } else if (enable_inline_image == INLINE_IMG_KITTY) {
		put_image_kitty(url, x, y, i->width, i->height, i->sx, i->sy, sw * pixel_per_char, sh * pixel_per_line_i, sw, sh);
	    }

	    continue ;
	}

	if (!(i->cache->loaded & IMG_FLAG_LOADED &&
	      i->width > 0 && i->height > 0))
	    continue;
	if (!(Imgdisplay_rf && Imgdisplay_wf)) {
	    if (!openImgdisplay())
		return;
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
	draw = TRUE;
    }

    if (!enable_inline_image) {
	if (!draw)
	    return;
	syncImage();
    }
    else
	n_terminal_image = 0;

    touch_cursor();
    refresh();
}

void
clearImage()
{
    static char buf[64];
    int j;
    TerminalImage *i;

    if (!activeImage)
	return;
    if (!n_terminal_image)
	return;
    if (!Imgdisplay_wf) {
	n_terminal_image = 0;
	return;
    }
    for (j = 0; j < n_terminal_image; j++) {
	i = &terminal_image[j];
	if (!(i->cache->loaded & IMG_FLAG_LOADED &&
	      i->width > 0 && i->height > 0))
	    continue;
	sprintf(buf, "6;%d;%d;%d;%d\n", i->x, i->y, i->width, i->height);
	fputs(buf, Imgdisplay_wf);
    }
    syncImage();
    n_terminal_image = 0;
}

/* load image */

#ifndef MAX_LOAD_IMAGE
#define MAX_LOAD_IMAGE 8
#endif
static int n_load_image = 0;
static Hash_sv *image_hash = NULL;
static Hash_sv *image_file = NULL;
static GeneralList *image_list = NULL;
static ImageCache **image_cache = NULL;
static Buffer *image_buffer = NULL;

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
	    !(a->image->cache->loaded & IMG_FLAG_DONT_REMOVE) &&
	    a->image->cache->index < 0)
	    unlink(a->image->cache->file);
    }
    loadImage(NULL, IMG_FLAG_STOP);
}

void
getAllImage(Buffer *buf)
{
    AnchorList *al;
    Anchor *a;
    ParsedURL *current;
    int i;

    image_buffer = buf;
    if (!buf)
	return;
    buf->image_loaded = TRUE;
    al = buf->img;
    if (!al)
	return;
    current = baseURL(buf);
    for (i = 0, a = al->anchors; i < al->nanchor; i++, a++) {
	if (a->image) {
	    a->image->cache = getImage(a->image, current, buf->image_flag);
	    if (a->image->cache &&
		a->image->cache->loaded == IMG_FLAG_UNLOADED)
		buf->image_loaded = FALSE;
	}
    }
}

void
showImageProgress(Buffer *buf)
{
    AnchorList *al;
    Anchor *a;
    int i, l, n;

    if (!buf)
	return;
    al = buf->img;
    if (!al)
	return;
    for (i = 0, l = 0, n = 0, a = al->anchors; i < al->nanchor; i++, a++) {
	if (a->image && a->hseq >= 0) {
	    n++;
	    if (a->image->cache && a->image->cache->loaded & IMG_FLAG_LOADED)
		l++;
	}
    }
    if (n) {
        if (enable_inline_image && n == l)
	    drawImage();
	message(Sprintf("%d/%d images loaded", l, n)->ptr,
		buf->cursorX + buf->rootX, buf->cursorY + buf->rootY);
	refresh();
    }
}

void
loadImage(Buffer *buf, int flag)
{
    ImageCache *cache;
    struct stat st;
    int i, draw = FALSE;
    /* int wait_st; */
#ifdef DONT_CALL_GC_AFTER_FORK
    char *loadargs[7];
#endif

    if (maxLoadImage > MAX_LOAD_IMAGE)
	maxLoadImage = MAX_LOAD_IMAGE;
    else if (maxLoadImage < 1)
	maxLoadImage = 1;
    if (n_load_image == 0)
	n_load_image = maxLoadImage;
    if (!image_cache) {
	image_cache = New_N(ImageCache *, MAX_LOAD_IMAGE);
	bzero(image_cache, sizeof(ImageCache *) * MAX_LOAD_IMAGE);
    }
    for (i = 0; i < n_load_image; i++) {
	cache = image_cache[i];
	if (!cache || !cache->touch)
	    continue;
	if (lstat(cache->touch, &st))
	    continue;
	if (cache->pid) {
	    kill(cache->pid, SIGKILL);
	    /*
	     * #ifdef HAVE_WAITPID
	     * waitpid(cache->pid, &wait_st, 0);
	     * #else
	     * wait(&wait_st);
	     * #endif
	     */
	    cache->pid = 0;
	}
	if (!stat(cache->file, &st)) {
	    cache->loaded = IMG_FLAG_LOADED;
	    if (getImageSize(cache)) {
		if (image_buffer)
		    image_buffer->need_reshape = TRUE;
	    }
	    draw = TRUE;
	}
	else
	    cache->loaded = IMG_FLAG_ERROR;
	unlink(cache->touch);
	image_cache[i] = NULL;
    }

    for (i = (buf != image_buffer) ? 0 : maxLoadImage; i < n_load_image; i++) {
	cache = image_cache[i];
	if (!cache || !cache->touch)
	    continue;
	if (cache->pid) {
	    kill(cache->pid, SIGKILL);
	    /*
	     * #ifdef HAVE_WAITPID
	     * waitpid(cache->pid, &wait_st, 0);
	     * #else
	     * wait(&wait_st);
	     * #endif
	     */
	    cache->pid = 0;
	}
	/*TODO make sure removing this didn't break anything
	unlink(cache->touch);
	*/
	image_cache[i] = NULL;
    }

    if (flag == IMG_FLAG_STOP) {
	image_list = NULL;
	image_file = NULL;
	n_load_image = maxLoadImage;
	image_buffer = NULL;
	return;
    }

    if (draw && image_buffer) {
        if (!enable_inline_image)
	    drawImage();
	showImageProgress(image_buffer);
    }

    image_buffer = buf;

    if (!image_list)
	return;
    for (i = 0; i < n_load_image; i++) {
	if (image_cache[i])
	    continue;
	while (1) {
	    cache = (ImageCache *) popValue(image_list);
	    if (!cache) {
		for (i = 0; i < n_load_image; i++) {
		    if (image_cache[i])
			return;
		}
		image_list = NULL;
		image_file = NULL;
		if (image_buffer)
		    displayBuffer(image_buffer, B_NORMAL);
		return;
	    }
	    if (cache->loaded == IMG_FLAG_UNLOADED)
		break;
	}
	image_cache[i] = cache;
	if (!cache->touch) {
	    continue;
	}

	flush_tty();
#ifdef DONT_CALL_GC_AFTER_FORK
	loadargs[0] = MyProgramName;
	loadargs[1] = "-$$getimage";
	loadargs[2] = conv_to_system(cache->url);
	loadargs[3] = conv_to_system(parsedURL2Str(cache->current)->ptr);
	loadargs[4] = cache->file;
	loadargs[5] = cache->touch;
	loadargs[6] = NULL;
	if ((cache->pid = fork()) == 0) {
	    setup_child(FALSE, 0, -1);
	    execvp(MyProgramName, loadargs);
	    exit(1);
	}
	else if (cache->pid < 0) {
	    cache->pid = 0;
	    return;
	}
#else /* !DONT_CALL_GC_AFTER_FORK */
	if ((cache->pid = fork()) == 0) {
	    Buffer *b;
	    /*
	     * setup_child(TRUE, 0, -1);
	     */
	    setup_child(FALSE, 0, -1);
	    image_source = cache->file;
	    b = loadGeneralFile(cache->url, cache->current, NULL, 0, NULL);
	    /* TODO make sure removing this didn't break anything
	    if (!b || !b->real_type || strncasecmp(b->real_type, "image/", 6))
		unlink(cache->file);
	    */
#if defined(HAVE_SYMLINK) && defined(HAVE_LSTAT)
	    symlink(cache->file, cache->touch);
#else
	    {
		FILE *f = fopen(cache->touch, "w");
		if (f)
		    fclose(f);
	    }
#endif
	    exit(0);
	}
	else if (cache->pid < 0) {
	    cache->pid = 0;
	    return;
	}
#endif /* !DONT_CALL_GC_AFTER_FORK */
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
	cache->pid = 0;
	cache->index = 0;
	cache->loaded = IMG_FLAG_UNLOADED;
	if (enable_inline_image == INLINE_IMG_OSC5379) {
	    if (image->width > 0 && image->width % pixel_per_char_i > 0)
		image->width += (pixel_per_char_i - image->width % pixel_per_char_i);

	    if (image->height > 0 && image->height % pixel_per_line_i > 0)
		image->height += (pixel_per_line_i - image->height % pixel_per_line_i);
	    if (image->height > 0 && image->width > 0) {
		cache->loaded = IMG_FLAG_LOADED;
	    }
	}
	if (cache->loaded == IMG_FLAG_UNLOADED) {
	    cache->touch = tmpfname(TMPF_DFL, NULL)->ptr;
	}
	else {
	    cache->touch = NULL;
	}

	cache->width = image->width ;
	cache->height = image->height ;
	cache->a_width = image->width;
	cache->a_height = image->height;
	putHash_sv(image_hash, key->ptr, (void *)cache);
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
    if (cache->loaded & IMG_FLAG_LOADED)
	getImageSize(cache);
    return cache;
}

static int
parseImageHeader(char *path, u_int *width, u_int *height)
{
    FILE *fp;
    u_char buf[8];

    if (!(fp = fopen(path, "r"))) return FALSE;

    if (fread(buf, 1, 2, fp) != 2) goto error;

    if (memcmp(buf, "\xff\xd8", 2) == 0) {
        /* JPEG */
	if (fseek(fp, 2, SEEK_CUR) < 0)	goto error;   /* 0xffe0 */
	while (fread(buf, 1, 2, fp) == 2) {
	    size_t len = ((buf[0] << 8) | buf[1]) - 2;
	    if (fseek(fp, len, SEEK_CUR) < 0) goto error;
	    if (fread(buf, 1, 2, fp) == 2 &&
	        /* SOF0 or SOF2 */
	        (memcmp(buf, "\xff\xc0", 2) == 0 || memcmp(buf, "\xff\xc2", 2) == 0)) {
		fseek(fp, 3, SEEK_CUR);
		if (fread(buf, 1, 2, fp) == 2) {
		    *height = (buf[0] << 8) | buf[1];
		    if (fread(buf, 1, 2, fp) == 2) {
			*width = (buf[0] << 8) | buf[1];
			goto success;
		    }
		}
		break;
	    }
	}
	goto error;
    }

    if (fread(buf + 2, 1, 1, fp) != 1) goto error;

    if (memcmp(buf, "GIF", 3) == 0) {
        /* GIF */
	if (fseek(fp, 3, SEEK_CUR) < 0) goto error;
	if (fread(buf, 1, 2, fp) == 2) {
	    *width = (buf[1] << 8) | buf[0];
	    if (fread(buf, 1, 2, fp) == 2) {
		*height = (buf[1] << 8) | buf[0];
		goto success;
	    }
	}
	goto error;
    }

    if (fread(buf + 3, 1, 5, fp) != 5) goto error;

    if (memcmp(buf, "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8) == 0) {
	/* PNG */
	if (fseek(fp, 8, SEEK_CUR) < 0) goto error;
	if (fread(buf, 1, 4, fp) == 4) {
	    *width = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	    if (fread(buf, 1, 4, fp) == 4) {
		*height = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
		goto success;
	    }
	}
	goto error;
    }

error:
    fclose(fp);
    return FALSE;

success:
    fclose(fp);
    return TRUE;
}

int
getImageSize(ImageCache * cache)
{
    Str tmp;
    FILE *f;
    int w = 0, h = 0;

    if (!activeImage)
	return FALSE;
    if (!cache || !(cache->loaded & IMG_FLAG_LOADED) ||
	(cache->width > 0 && cache->height > 0))
	return FALSE;

    if (parseImageHeader(cache->file, &w, &h))
	goto got_image_size;

    tmp = Strnew();
    if (!strchr(Imgdisplay, '/'))
	Strcat_m_charp(tmp, w3m_auxbin_dir(), "/", NULL);
    Strcat_m_charp(tmp, Imgdisplay, " -size ", shell_quote(cache->file), NULL);
    f = popen(tmp->ptr, "r");
    if (!f)
	return FALSE;
    while (fscanf(f, "%d %d", &w, &h) < 0) {
	if (feof(f))
	    break;
    }
    pclose(f);

    if (!(w > 0 && h > 0))
	return FALSE;

got_image_size:
    w = (int)(w * image_scale / 100 + 0.5);
    if (w == 0)
	w = 1;
    h = (int)(h * image_scale / 100 + 0.5);
    if (h == 0)
	h = 1;
    if (cache->width < 0 && cache->height < 0) {
	cache->width = (w > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : w;
	cache->height = (h > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : h;
    }
    else if (cache->width < 0) {
	int tmp = (int)((double)cache->height * w / h + 0.5);
	cache->a_width = cache->width = (tmp > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : tmp;
    }
    else if (cache->height < 0) {
	int tmp = (int)((double)cache->width * h / w + 0.5);
	cache->a_height = cache->height = (tmp > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : tmp;
    }
    if (cache->width == 0)
	cache->width = 1;
    if (cache->height == 0)
	cache->height = 1;
    tmp = Sprintf("%d;%d;%s", cache->width, cache->height, cache->url);
    putHash_sv(image_hash, tmp->ptr, (void *)cache);
    return TRUE;
}
