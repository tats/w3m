/* $Id: display.c,v 1.25 2002/10/29 16:19:41 ukai Exp $ */
#include <signal.h>
#include "fm.h"

#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

#ifdef USE_COLOR

#define EFFECT_ANCHOR_START       effect_anchor_start()
#define EFFECT_ANCHOR_END         effect_anchor_end()
#define EFFECT_IMAGE_START        effect_image_start()
#define EFFECT_IMAGE_END          effect_image_end()
#define EFFECT_FORM_START         effect_form_start()
#define EFFECT_FORM_END           effect_form_end()
#define EFFECT_ACTIVE_START	  effect_active_start()
#define EFFECT_ACTIVE_END	  effect_active_end()
#define EFFECT_VISITED_START      effect_visited_start()
#define EFFECT_VISITED_END        effect_visited_end()
#define EFFECT_MARK_START         effect_mark_start()
#define EFFECT_MARK_END           effect_mark_end()

/* color: *     0  black *      1  red *        2  green *      3  yellow
 * *    4  blue *       5  magenta *    6  cyan *       7  white */

#define EFFECT_ANCHOR_START_C       setfcolor(anchor_color)
#define EFFECT_IMAGE_START_C        setfcolor(image_color)
#define EFFECT_FORM_START_C         setfcolor(form_color)
#define EFFECT_ACTIVE_START_C      (setfcolor(active_color), underline())
#define EFFECT_VISITED_START_C      setfcolor(visited_color)
#ifdef USE_BG_COLOR
#define EFFECT_MARK_START_C         setbcolor(mark_color)
#else
#define EFFECT_MARK_START_C         standout()
#endif

#define EFFECT_IMAGE_END_C          setfcolor(basic_color)
#define EFFECT_ANCHOR_END_C         setfcolor(basic_color)
#define EFFECT_FORM_END_C           setfcolor(basic_color)
#define EFFECT_ACTIVE_END_C        (setfcolor(basic_color), underlineend())
#define EFFECT_VISITED_END_C        setfcolor(basic_color)
#ifdef USE_BG_COLOR
#define EFFECT_MARK_END_C           setbcolor(bg_color)
#else
#define EFFECT_MARK_END_C           standend()
#endif

#define EFFECT_ANCHOR_START_M       underline()
#define EFFECT_ANCHOR_END_M         underlineend()
#define EFFECT_IMAGE_START_M        standout()
#define EFFECT_IMAGE_END_M          standend()
#define EFFECT_FORM_START_M         standout()
#define EFFECT_FORM_END_M           standend()
#define EFFECT_ACTIVE_START_NC      underline()
#define EFFECT_ACTIVE_END_NC        underlineend()
#define EFFECT_ACTIVE_START_M       bold()
#define EFFECT_ACTIVE_END_M         boldend()
#define EFFECT_VISITED_START_M /**/
#define EFFECT_VISITED_END_M /**/
#define EFFECT_MARK_START_M         standout()
#define EFFECT_MARK_END_M           standend()
#define define_effect(name_start,name_end,color_start,color_end,mono_start,mono_end) \
static void name_start { if (useColor) { color_start; } else { mono_start; }}\
static void name_end { if (useColor) { color_end; } else { mono_end; }}
define_effect(EFFECT_ANCHOR_START, EFFECT_ANCHOR_END, EFFECT_ANCHOR_START_C,
	      EFFECT_ANCHOR_END_C, EFFECT_ANCHOR_START_M, EFFECT_ANCHOR_END_M)
define_effect(EFFECT_IMAGE_START, EFFECT_IMAGE_END, EFFECT_IMAGE_START_C,
	      EFFECT_IMAGE_END_C, EFFECT_IMAGE_START_M, EFFECT_IMAGE_END_M)
define_effect(EFFECT_FORM_START, EFFECT_FORM_END, EFFECT_FORM_START_C,
	      EFFECT_FORM_END_C, EFFECT_FORM_START_M, EFFECT_FORM_END_M)
define_effect(EFFECT_MARK_START, EFFECT_MARK_END, EFFECT_MARK_START_C,
	      EFFECT_MARK_END_C, EFFECT_MARK_START_M, EFFECT_MARK_END_M)

/*****************/
/* *INDENT-OFF* */
static void
EFFECT_ACTIVE_START
{
    if (useColor) {
	if (useActiveColor) {
#ifdef __EMX__
	    if(!getenv("WINDOWID"))
		setfcolor(active_color);
	    else
#endif
	    {
		EFFECT_ACTIVE_START_C;
	    }
	} else {
	    EFFECT_ACTIVE_START_NC;
	}
    } else {
	EFFECT_ACTIVE_START_M;
    }
}

static void
EFFECT_ACTIVE_END
{
    if (useColor) {
	if (useActiveColor) {
	    EFFECT_ACTIVE_END_C;
	} else {
	    EFFECT_ACTIVE_END_NC;
	}
    } else {
	EFFECT_ACTIVE_END_M;
    }
}

static void
EFFECT_VISITED_START
{
    if (useVisitedColor) {
	if (useColor) {
	    EFFECT_VISITED_START_C;
	} else {
	    EFFECT_VISITED_START_M;
	}
    }
}

static void
EFFECT_VISITED_END
{
    if (useVisitedColor) {
	if (useColor) {
	    EFFECT_VISITED_END_C;
	} else {
	    EFFECT_VISITED_END_M;
	}
    }
}
/* *INDENT-ON* */
#else				/* not USE_COLOR */

#define EFFECT_ANCHOR_START       underline()
#define EFFECT_ANCHOR_END         underlineend()
#define EFFECT_IMAGE_START        standout()
#define EFFECT_IMAGE_END          standend()
#define EFFECT_FORM_START         standout()
#define EFFECT_FORM_END           standend()
#define EFFECT_ACTIVE_START       bold()
#define EFFECT_ACTIVE_END         boldend()
#define EFFECT_VISITED_START /**/
#define EFFECT_VISITED_END /**/
#define EFFECT_MARK_START         standout()
#define EFFECT_MARK_END           standend()
#endif				/* not USE_COLOR */
#ifndef KANJI_SYMBOLS
static char g_rule[] = "ntwluxkavmqajaaa";
#endif				/* not KANJI_SYMBOLS */

/* 
 * Terminate routine.
 */

void
fmTerm(void)
{
    if (fmInitialized) {
	move(LASTLINE, 0);
	clrtoeolx();
	refresh();
#ifdef USE_IMAGE
	if (activeImage)
	    loadImage(IMG_FLAG_STOP);
#endif
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_end();
#endif				/* USE_MOUSE */
	reset_tty();
	fmInitialized = FALSE;
    }
}


/* 
 * Initialize routine.
 */
void
fmInit(void)
{
    if (!fmInitialized) {
	initscr();
	term_raw();
	term_noecho();
#ifdef USE_IMAGE
	if (displayImage)
	    initImage();
#endif
    }
    fmInitialized = TRUE;
}

/* 
 * Display some lines.
 */
static Line *cline = NULL;
static int ccolumn = -1;

static int ulmode = 0, somode = 0, bomode = 0;
static int anch_mode = 0, emph_mode = 0, imag_mode = 0, form_mode = 0,
    active_mode = 0, visited_mode = 0, mark_mode = 0;
#ifndef KANJI_SYMBOLS
static int graph_mode = 0;
#endif				/* not KANJI_SYMBOLS */
#ifdef USE_ANSI_COLOR
static Linecolor color_mode = 0;
#endif

#ifdef USE_BUFINFO
static Buffer *save_current_buf = NULL;
#endif

char *delayed_msg = NULL;

#ifdef USE_IMAGE
static int image_touch = 0;
static int draw_image_flag = FALSE;
static Line *redrawLineImage(Buffer *buf, Line *l, int i);
#endif

void
displayBuffer(Buffer *buf, int mode)
{
    Str msg;
    Anchor *aa = NULL;

    if (buf->topLine == NULL && readBufferCache(buf) == 0) {	/* clear_buffer */
	mode = B_FORCE_REDRAW;
    }

    if (buf->width == 0)
	buf->width = COLS;
    if (buf->height == 0)
	buf->height = LASTLINE + 1;
    if ((buf->width != INIT_BUFFER_WIDTH && buf->type &&
	 !strcmp(buf->type, "text/html")) || buf->need_reshape) {
	buf->need_reshape = TRUE;
	reshapeBuffer(buf);
    }
    if (showLineNum) {
	if (buf->lastLine && buf->lastLine->real_linenumber > 0)
	    buf->rootX = (int)(log(buf->lastLine->real_linenumber + 0.1)
			       / log(10)) + 2;
	if (buf->rootX < 5)
	    buf->rootX = 5;
	if (buf->rootX > COLS)
	    buf->rootX = COLS;
    }
    else
	buf->rootX = 0;
    buf->COLS = COLS - buf->rootX;
    if (mode == B_FORCE_REDRAW || mode == B_SCROLL ||
#ifdef USE_IMAGE
	mode == B_REDRAW_IMAGE ||
#endif
	cline != buf->topLine || ccolumn != buf->currentColumn) {
	if (
#ifdef USE_IMAGE
	       !(activeImage && displayImage && draw_image_flag) &&
#endif
	       mode == B_SCROLL && cline && buf->currentColumn == ccolumn) {
	    int n = buf->topLine->linenumber - cline->linenumber;
	    if (n > 0 && n < LASTLINE) {
		move(LASTLINE, 0);
		clrtoeolx();
		refresh();
		scroll(n);
	    }
	    else if (n < 0 && n > -LASTLINE) {
#if defined(__CYGWIN__) && LANG == JA
		move(LASTLINE + n + 1, 0);
		clrtoeolx();
		refresh();
#endif				/* defined(__CYGWIN__) && LANG == JA */
		rscroll(-n);
	    }
	    redrawNLine(buf, n);
	}
	else {
#ifdef USE_IMAGE
	    if (activeImage &&
		(mode == B_REDRAW_IMAGE ||
		 cline != buf->topLine || ccolumn != buf->currentColumn)) {
		if (draw_image_flag)
		    clear();
		clearImage();
		loadImage(IMG_FLAG_STOP);
		image_touch++;
		draw_image_flag = FALSE;
	    }
#endif
	    redrawBuffer(buf);
	}
	cline = buf->topLine;
	ccolumn = buf->currentColumn;
    }
    if (buf->topLine == NULL)
	buf->topLine = buf->firstLine;

#ifdef USE_IMAGE
    if (buf->need_reshape) {
	displayBuffer(buf, B_FORCE_REDRAW);
	return;
    }
#endif

#ifdef USE_MOUSE
    if (use_mouse)
#if LANG == JA
	msg = Strnew_charp("¢ã¢¬¢­");
#else				/* LANG != JA */
	msg = Strnew_charp("<=UpDn ");
#endif				/* LANG != JA */
    else
#endif				/* not USE_MOUSE */
	msg = Strnew();
    if (displayLineInfo && buf->currentLine != NULL && buf->lastLine != NULL) {
	int cl = buf->currentLine->real_linenumber;
	int ll = buf->lastLine->real_linenumber;
	int r = (int)((double)cl * 100.0 / (double)ll + 0.5);
	Strcat(msg, Sprintf("%d/%d (%d%%)", cl, ll, r));
    }
    else
	Strcat_charp(msg, "Viewing");
#ifdef USE_SSL
    if (buf->ssl_certificate)
	Strcat_charp(msg, "[SSL]");
#endif
    Strcat_charp(msg, " <");
    Strcat_charp(msg, buf->buffername);
    if (displayLink)
	aa = retrieveCurrentAnchor(buf);
    if (aa) {
	ParsedURL url;
	Str s;
	int l;
	parseURL2(aa->url, &url, baseURL(buf));
	s = parsedURL2Str(&url);
	l = buf->width - 2;
	if (s->length > l) {
	    if (l >= 4) {
		msg = Strsubstr(s, 0, (l - 2) / 2);
#if LANG == JA
		Strcat_charp(msg, "¡Ä");
#else				/* LANG != JA */
		Strcat_charp(msg, "..");
#endif				/* LANG != JA */
		l = buf->width - msg->length;
		Strcat(msg, Strsubstr(s, s->length - l, l));
	    }
	    else {
		msg = s;
	    }
	}
	else {
	    l -= s->length;
	    if (msg->length > l) {
#ifdef JP_CHARSET
		char *bn = msg->ptr;
		int i, j;
		for (i = 0; bn[i]; i += j) {
		    j = get_mclen(get_mctype(&bn[i]));
		    if (i + j > l)
			break;
		}
		l = i;
#endif
		Strtruncate(msg, l);
	    }
	    Strcat_charp(msg, "> ");
	    Strcat(msg, s);
	}
    }
    else {
	Strcat_charp(msg, ">");
    }
    if (buf->firstLine == NULL) {
	Strcat_charp(msg, "\tNo Line");
	clear();
    }
    if (delayed_msg != NULL) {
	disp_message(delayed_msg, FALSE);
	delayed_msg = NULL;
	refresh();
    }
    standout();
    message(msg->ptr, buf->cursorX + buf->rootX, buf->cursorY);
    standend();
    term_title(buf->buffername);
    refresh();
#ifdef USE_IMAGE
    if (activeImage && displayImage && buf->img) {
	/*
	 * loadImage(IMG_FLAG_START);
	 */
	drawImage();
    }
#endif
#ifdef USE_BUFINFO
    if (buf != save_current_buf) {
	saveBufferInfo();
	save_current_buf = buf;
    }
#endif
}

void
redrawBuffer(Buffer *buf)
{
    redrawNLine(buf, LASTLINE);
}

void
redrawNLine(Buffer *buf, int n)
{
    Line *l, *l0;
    int i;

#ifdef USE_COLOR
    if (useColor) {
	EFFECT_ANCHOR_END_C;
#ifdef USE_BG_COLOR
	setbcolor(bg_color);
#endif				/* USE_BG_COLOR */
    }
#endif				/* USE_COLOR */
    for (i = 0, l = buf->topLine; i < LASTLINE; i++) {
	if (i >= LASTLINE - n || i < -n)
	    l0 = redrawLine(buf, l, i);
	else {
	    l0 = (l) ? l->next : NULL;
	}
	if (l0 == NULL && l == NULL)
	    break;
	l = l0;
    }
    if (n > 0)
	clrtobotx();

#ifdef USE_IMAGE
    if (!(activeImage && displayImage && buf->img))
	return;
    move(buf->cursorY, buf->cursorX);
    for (i = 0, l = buf->topLine; i < LASTLINE; i++) {
	if (i >= LASTLINE - n || i < -n)
	    l0 = redrawLineImage(buf, l, i);
	else {
	    l0 = (l) ? l->next : NULL;
	}
	if (l0 == NULL && l == NULL)
	    break;
	l = l0;
    }
    getAllImage(buf);
#endif
}

#define addKanji(pc,pr) (addChar((pc)[0],(pr)[0]),addChar((pc)[1],(pr)[1]))

Line *
redrawLine(Buffer *buf, Line *l, int i)
{
    int j, pos, rcol, ncol, delta;
    int column = buf->currentColumn;
    char *p;
    Lineprop *pr;
#ifdef USE_ANSI_COLOR
    Linecolor *pc;
#endif
#ifdef USE_COLOR
    Anchor *a;
    ParsedURL url;
    int k, vpos = -1;
#endif

    if (l == NULL) {
	if (buf->pagerSource) {
	    l = getNextPage(buf, LASTLINE - i);
	    if (l == NULL)
		return NULL;
	}
	else
	    return NULL;
    }
    move(i, 0);
    if (showLineNum) {
	char tmp[16];
	if (!buf->rootX) {
	    if (buf->lastLine->real_linenumber > 0)
		buf->rootX = (int)(log(buf->lastLine->real_linenumber + 0.1)
				   / log(10)) + 2;
	    if (buf->rootX < 5)
		buf->rootX = 5;
	    if (buf->rootX > COLS)
		buf->rootX = COLS;
	    buf->COLS = COLS - buf->rootX;
	}
	if (l->real_linenumber)
	    sprintf(tmp, "%*ld:", buf->rootX - 1, l->real_linenumber);
	else
	    sprintf(tmp, "%*s ", buf->rootX - 1, "");
	addstr(tmp);
    }
    move(i, buf->rootX);
    if (l->width < 0)
	l->width = COLPOS(l, l->len);
    if (l->len == 0 || l->width - 1 < column) {
	clrtoeolx();
	return l->next;
    }
    /* need_clrtoeol(); */
    pos = columnPos(l, column);
    p = &(l->lineBuf[pos]);
    pr = &(l->propBuf[pos]);
#ifdef USE_ANSI_COLOR
    if (useColor && l->colorBuf)
	pc = &(l->colorBuf[pos]);
    else
	pc = NULL;
#endif
    rcol = COLPOS(l, pos);

#ifndef JP_CHARSET
    delta = 1;
#endif
    for (j = 0; rcol - column < buf->COLS && pos + j < l->len; j += delta) {
#ifdef USE_COLOR
	if (useVisitedColor && vpos <= pos + j && !(pr[j] & PE_VISITED)) {
	    a = retrieveAnchor(buf->href, l->linenumber, pos + j);
	    if (a) {
		parseURL2(a->url, &url, baseURL(buf));
		if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
		    for (k = a->start.pos; k < a->end.pos; k++)
			pr[k - pos] |= PE_VISITED;
		}
		vpos = a->end.pos;
	    }
	}
#endif
#ifdef JP_CHARSET
	if (CharType(pr[j]) == PC_KANJI1)
	    delta = 2;
	else
	    delta = 1;
#endif
	ncol = COLPOS(l, pos + j + delta);
	if (ncol - column > buf->COLS)
	    break;
#ifdef USE_ANSI_COLOR
	if (pc)
	    do_color(pc[j]);
#endif
	if (rcol < column) {
	    for (rcol = column; rcol < ncol; rcol++)
		addChar(' ', 0);
	    continue;
	}
	if (p[j] == '\t') {
	    for (; rcol < ncol; rcol++)
		addChar(' ', 0);
	}
#ifdef JP_CHARSET
	else if (delta == 2) {
	    addKanji(&p[j], &pr[j]);
	}
#endif
	else {
	    addChar(p[j], pr[j]);
	}
	rcol = ncol;
    }
    if (somode) {
	somode = FALSE;
	standend();
    }
    if (ulmode) {
	ulmode = FALSE;
	underlineend();
    }
    if (bomode) {
	bomode = FALSE;
	boldend();
    }
    if (emph_mode) {
	emph_mode = FALSE;
	boldend();
    }

    if (anch_mode) {
	anch_mode = FALSE;
	EFFECT_ANCHOR_END;
    }
    if (imag_mode) {
	imag_mode = FALSE;
	EFFECT_IMAGE_END;
    }
    if (form_mode) {
	form_mode = FALSE;
	EFFECT_FORM_END;
    }
    if (visited_mode) {
	visited_mode = FALSE;
	EFFECT_VISITED_END;
    }
    if (active_mode) {
	active_mode = FALSE;
	EFFECT_ACTIVE_END;
    }
    if (mark_mode) {
	mark_mode = FALSE;
	EFFECT_MARK_END;
    }
#ifndef KANJI_SYMBOLS
    if (graph_mode) {
	graph_mode = FALSE;
	graphend();
    }
#endif				/* not KANJI_SYMBOLS */
#ifdef USE_ANSI_COLOR
    if (color_mode)
	do_color(0);
#endif
    if (rcol - column < buf->COLS)
	clrtoeolx();
    return l->next;
}

#ifdef USE_IMAGE
Line *
redrawLineImage(Buffer *buf, Line *l, int i)
{
    int j, pos, rcol;
    int column = buf->currentColumn;
    Anchor *a;
    int x, y, sx, sy, w, h;

    if (l == NULL)
	return NULL;
    if (l->width < 0)
	l->width = COLPOS(l, l->len);
    if (l->len == 0 || l->width - 1 < column)
	return l->next;
    pos = columnPos(l, column);
    rcol = COLPOS(l, pos);
    for (j = 0; rcol - column < buf->COLS && pos + j < l->len; j++) {
	if (rcol - column < 0) {
	    rcol = COLPOS(l, pos + j + 1);
	    continue;
	}
	a = retrieveAnchor(buf->img, l->linenumber, pos + j);
	if (a && a->image && a->image->touch < image_touch) {
	    Image *image = a->image;
	    ImageCache *cache;

	    cache = image->cache = getImage(image, baseURL(buf),
					    buf->image_flag);
	    if (cache) {
		if ((image->width < 0 && cache->width > 0) ||
		    (image->height < 0 && cache->height > 0)) {
		    image->width = cache->width;
		    image->height = cache->height;
		    buf->need_reshape = TRUE;
		}
		x = (int)((rcol - column + buf->rootX) * pixel_per_char);
		y = (int)(i * pixel_per_line);
		sx = (int)((rcol - COLPOS(l, a->start.pos)) * pixel_per_char);
		sy = (int)((l->linenumber - image->y) * pixel_per_line);
		if (sx == 0 && x + image->xoffset >= 0)
		    x += image->xoffset;
		else
		    sx -= image->xoffset;
		if (sy == 0 && y + image->yoffset >= 0)
		    y += image->yoffset;
		else
		    sy -= image->yoffset;
		if (image->width > 0)
		    w = image->width - sx;
		else
		    w = (int)(8 * pixel_per_char - sx);
		if (image->height > 0)
		    h = image->height - sy;
		else
		    h = (int)(pixel_per_line - sy);
		if (w > (int)((buf->rootX + buf->COLS) * pixel_per_char - x))
		    w = (int)((buf->rootX + buf->COLS) * pixel_per_char - x);
		if (h > (int)(LASTLINE * pixel_per_line - y))
		    h = (int)(LASTLINE * pixel_per_line - y);
		addImage(cache, x, y, sx, sy, w, h);
		image->touch = image_touch;
		draw_image_flag = TRUE;
	    }
	}
	rcol = COLPOS(l, pos + j + 1);
    }
    return l->next;
}
#endif

int
redrawLineRegion(Buffer *buf, Line *l, int i, int bpos, int epos)
{
    int j, pos, rcol, ncol, delta;
    int column = buf->currentColumn;
    char *p;
    Lineprop *pr;
#ifdef USE_ANSI_COLOR
    Linecolor *pc;
#endif
    int bcol, ecol;
#ifdef USE_COLOR
    Anchor *a;
    ParsedURL url;
    int k, vpos = -1;
#endif

    if (l == NULL)
	return 0;
    pos = columnPos(l, column);
    p = &(l->lineBuf[pos]);
    pr = &(l->propBuf[pos]);
#ifdef USE_ANSI_COLOR
    if (useColor && l->colorBuf)
	pc = &(l->colorBuf[pos]);
    else
	pc = NULL;
#endif
    rcol = COLPOS(l, pos);
    bcol = bpos - pos;
    ecol = epos - pos;

#ifndef JP_CHARSET
    delta = 1;
#endif
    for (j = 0; rcol - column < buf->COLS && pos + j < l->len; j += delta) {
#ifdef USE_COLOR
	if (useVisitedColor && vpos <= pos + j && !(pr[j] & PE_VISITED)) {
	    a = retrieveAnchor(buf->href, l->linenumber, pos + j);
	    if (a) {
		parseURL2(a->url, &url, baseURL(buf));
		if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
		    for (k = a->start.pos; k < a->end.pos; k++)
			pr[k - pos] |= PE_VISITED;
		}
		vpos = a->end.pos;
	    }
	}
#endif
#ifdef JP_CHARSET
	if (CharType(pr[j]) == PC_KANJI1)
	    delta = 2;
	else
	    delta = 1;
#endif
	ncol = COLPOS(l, pos + j + delta);
	if (ncol - column > buf->COLS)
	    break;
#ifdef USE_ANSI_COLOR
	if (pc)
	    do_color(pc[j]);
#endif
	if (j >= bcol && j < ecol) {
	    if (rcol < column) {
		move(i, buf->rootX);
		for (rcol = column; rcol < ncol; rcol++)
		    addChar(' ', 0);
		continue;
	    }
	    move(i, rcol - column + buf->rootX);
	    if (p[j] == '\t') {
		for (; rcol < ncol; rcol++)
		    addChar(' ', 0);
	    }
#ifdef JP_CHARSET
	    else if (delta == 2) {
		addKanji(&p[j], &pr[j]);
	    }
#endif
	    else {
		addChar(p[j], pr[j]);
	    }
	}
	rcol = ncol;
    }
    if (somode) {
	somode = FALSE;
	standend();
    }
    if (ulmode) {
	ulmode = FALSE;
	underlineend();
    }
    if (bomode) {
	bomode = FALSE;
	boldend();
    }
    if (emph_mode) {
	emph_mode = FALSE;
	boldend();
    }

    if (anch_mode) {
	anch_mode = FALSE;
	EFFECT_ANCHOR_END;
    }
    if (imag_mode) {
	imag_mode = FALSE;
	EFFECT_IMAGE_END;
    }
    if (form_mode) {
	form_mode = FALSE;
	EFFECT_FORM_END;
    }
    if (visited_mode) {
	visited_mode = FALSE;
	EFFECT_VISITED_END;
    }
    if (active_mode) {
	active_mode = FALSE;
	EFFECT_ACTIVE_END;
    }
    if (mark_mode) {
	mark_mode = FALSE;
	EFFECT_MARK_END;
    }
#ifndef KANJI_SYMBOLS
    if (graph_mode) {
	graph_mode = FALSE;
	graphend();
    }
#endif				/* not KANJI_SYMBOLS */
#ifdef USE_ANSI_COLOR
    if (color_mode)
	do_color(0);
#endif
    return rcol - column;
}

#define do_effect1(effect,modeflag,action_start,action_end) \
if (m & effect) { \
    if (!modeflag) { \
	action_start; \
	modeflag = TRUE; \
    } \
}

#define do_effect2(effect,modeflag,action_start,action_end) \
if (modeflag) { \
    action_end; \
    modeflag = FALSE; \
}

void
do_effects(Lineprop m)
{
    /* effect end */
    do_effect2(PE_UNDER, ulmode, underline(), underlineend());
    do_effect2(PE_STAND, somode, standout(), standend());
    do_effect2(PE_BOLD, bomode, bold(), boldend());
    do_effect2(PE_EMPH, emph_mode, bold(), boldend());
    do_effect2(PE_ANCHOR, anch_mode, EFFECT_ANCHOR_START, EFFECT_ANCHOR_END);
    do_effect2(PE_IMAGE, imag_mode, EFFECT_IMAGE_START, EFFECT_IMAGE_END);
    do_effect2(PE_FORM, form_mode, EFFECT_FORM_START, EFFECT_FORM_END);
    do_effect2(PE_VISITED, visited_mode, EFFECT_VISITED_START,
	       EFFECT_VISITED_END);
    do_effect2(PE_ACTIVE, active_mode, EFFECT_ACTIVE_START, EFFECT_ACTIVE_END);
    do_effect2(PE_MARK, mark_mode, EFFECT_MARK_START, EFFECT_MARK_END);
#ifndef KANJI_SYMBOLS
    if (graph_mode) {
	graphend();
	graph_mode = FALSE;
    }
#endif				/* not KANJI_SYMBOLS */

    /* effect start */
    do_effect1(PE_UNDER, ulmode, underline(), underlineend());
    do_effect1(PE_STAND, somode, standout(), standend());
    do_effect1(PE_BOLD, bomode, bold(), boldend());
    do_effect1(PE_EMPH, emph_mode, bold(), boldend());
    do_effect1(PE_ANCHOR, anch_mode, EFFECT_ANCHOR_START, EFFECT_ANCHOR_END);
    do_effect1(PE_IMAGE, imag_mode, EFFECT_IMAGE_START, EFFECT_IMAGE_END);
    do_effect1(PE_FORM, form_mode, EFFECT_FORM_START, EFFECT_FORM_END);
    do_effect1(PE_VISITED, visited_mode, EFFECT_VISITED_START,
	       EFFECT_VISITED_END);
    do_effect1(PE_ACTIVE, active_mode, EFFECT_ACTIVE_START, EFFECT_ACTIVE_END);
    do_effect1(PE_MARK, mark_mode, EFFECT_MARK_START, EFFECT_MARK_END);
#ifndef KANJI_SYMBOLS
    if (m & PC_RULE) {
	if (!graph_mode && graph_ok()) {
	    graphstart();
	    graph_mode = TRUE;
	}
    }
#endif				/* not KANJI_SYMBOLS */
}

#ifdef USE_ANSI_COLOR
void
do_color(Linecolor c)
{
    if (c & 0x8)
	setfcolor(c & 0x7);
    else if (color_mode & 0x8)
	setfcolor(basic_color);
#ifdef USE_BG_COLOR
    if (c & 0x80)
	setbcolor((c >> 4) & 0x7);
    else if (color_mode & 0x80)
	setbcolor(bg_color);
#endif
    color_mode = c;
}
#endif

void
addChar(char c, Lineprop mode)
{
    Lineprop m = CharEffect(mode);

#ifdef JP_CHARSET
    if (CharType(mode) != PC_KANJI2)
#endif				/* JP_CHARSET */
	do_effects(m);
#ifndef KANJI_SYMBOLS
    if (m & PC_RULE) {
	if (graph_mode)
	    addch(g_rule[c & 0xF]);
	else
	    addch(alt_rule[c & 0xF]);
    }
    else
#endif				/* not KANJI_SYMBOLS */
    if (IS_UNPRINTABLE_ASCII(c, mode)) {
	addstr(Sprintf("\\%3o", (unsigned char)c)->ptr);
    }
    else if (c == '\t') {
	addch(c);
    }
    else if (c == DEL_CODE)
	addstr("^?");
    else if (IS_UNPRINTABLE_CONTROL(c, mode)) {	/* Control code */
	addch('^');
	addch(c + '@');
    }
    else if (c != '\n')
	addch(c);
    else			/* \n */
	addch(' ');
}

GeneralList *message_list = NULL;

void
record_err_message(char *s)
{
    if (fmInitialized) {
	if (!message_list)
	    message_list = newGeneralList();
	if (message_list->nitem >= LINES)
	    popValue(message_list);
	pushValue(message_list, allocStr(s, -1));
    }
}

/* 
 * List of error messages
 */
Buffer *
message_list_panel(void)
{
    Str tmp = Strnew_size(LINES * COLS);
    ListItem *p;

    Strcat_charp(tmp,
		 "<html><head><title>List of error messages</title></head><body>"
		 "<h1>List of error messages</h1><table cellpadding=0>\n");
    if (message_list)
	for (p = message_list->last; p; p = p->prev)
	    Strcat_m_charp(tmp, "<tr><td><pre>", html_quote(p->ptr),
			   "</pre></td></tr>\n", NULL);
    else
	Strcat_charp(tmp, "<tr><td>(no message recorded)</td></tr>\n");
    Strcat_charp(tmp, "</table></body></html>");
    return loadHTMLString(tmp);
}

void
message(char *s, int return_x, int return_y)
{
    if (!fmInitialized)
	return;
    move(LASTLINE, 0);
    addnstr(s, COLS - 1);
    clrtoeolx();
    move(return_y, return_x);
}

void
disp_message_nsec(char *s, int redraw_current, int sec, int purge, int mouse)
{
    if (QuietMessage)
	return;
    if (!fmInitialized) {
	fprintf(stderr, "%s\n", conv_to_system(s));
	return;
    }
    if (Currentbuf != NULL)
	message(s, Currentbuf->cursorX + Currentbuf->rootX,
		Currentbuf->cursorY);
    else
	message(s, LASTLINE, 0);
    refresh();
#ifdef USE_MOUSE
    if (mouse && use_mouse)
	mouse_active();
#endif
    sleep_till_anykey(sec, purge);
#ifdef USE_MOUSE
    if (mouse && use_mouse)
	mouse_inactive();
#endif
    if (Currentbuf != NULL && redraw_current)
	displayBuffer(Currentbuf, B_NORMAL);
}

void
disp_message(char *s, int redraw_current)
{
    disp_message_nsec(s, redraw_current, 10, FALSE, TRUE);
}
#ifdef USE_MOUSE
void
disp_message_nomouse(char *s, int redraw_current)
{
    disp_message_nsec(s, redraw_current, 10, FALSE, FALSE);
}
#endif

void
set_delayed_message(char *s)
{
    delayed_msg = allocStr(s, -1);
}

void
cursorUp(Buffer *buf, int n)
{
    if (buf->firstLine == NULL)
	return;
    if (buf->cursorY > 0)
	cursorUpDown(buf, -1);
    else {
	buf->topLine = lineSkip(buf, buf->topLine, -n, FALSE);
	if (buf->currentLine->prev != NULL)
	    buf->currentLine = buf->currentLine->prev;
	arrangeLine(buf);
    }
}

void
cursorDown(Buffer *buf, int n)
{
    if (buf->firstLine == NULL)
	return;
    if (buf->cursorY < LASTLINE - 1)
	cursorUpDown(buf, 1);
    else {
	buf->topLine = lineSkip(buf, buf->topLine, n, FALSE);
	if (buf->currentLine->next != NULL)
	    buf->currentLine = buf->currentLine->next;
	arrangeLine(buf);
    }
}

void
cursorUpDown(Buffer *buf, int n)
{
    Line *cl = buf->currentLine;

    if (buf->firstLine == NULL)
	return;
    if ((buf->currentLine = currentLineSkip(buf, cl, n, FALSE)) == cl)
	return;
    arrangeLine(buf);
}

void
cursorRight(Buffer *buf, int n)
{
    int i, delta = 1, cpos, vpos2;
    Line *l = buf->currentLine;
    Lineprop *p;

    if (buf->firstLine == NULL)
	return;
    if (buf->pos == l->len)
	return;
    i = buf->pos;
    p = l->propBuf;
#ifdef JP_CHARSET
    if (CharType(p[i]) == PC_KANJI1)
	delta = 2;
#endif				/* JP_CHARSET */
    if (i + delta < l->len) {
	buf->pos = i + delta;
    }
    else if (l->len == 0) {
	buf->pos = 0;
    }
    else {
	buf->pos = l->len - 1;
#ifdef JP_CHARSET
	if (CharType(p[buf->pos]) == PC_KANJI2)
	    buf->pos--;
#endif				/* JP_CHARSET */
    }
    cpos = COLPOS(l, buf->pos);
    buf->visualpos = cpos - buf->currentColumn;
    delta = 1;
#ifdef JP_CHARSET
    if (CharType(p[buf->pos]) == PC_KANJI1)
	delta = 2;
#endif				/* JP_CHARSET */
    vpos2 = COLPOS(l, buf->pos + delta) - buf->currentColumn - 1;
    if (vpos2 >= buf->COLS && n) {
	columnSkip(buf, n + (vpos2 - buf->COLS) - (vpos2 - buf->COLS) % n);
	buf->visualpos = cpos - buf->currentColumn;
    }
    buf->cursorX = buf->visualpos;
}

void
cursorLeft(Buffer *buf, int n)
{
    int i, delta = 1, cpos;
    Line *l = buf->currentLine;
    Lineprop *p;

    if (buf->firstLine == NULL)
	return;
    i = buf->pos;
    p = l->propBuf;
#ifdef JP_CHARSET
    if (i >= 2 && CharType(p[i - 1]) == PC_KANJI2)
	delta = 2;
#endif				/* JP_CHARSET */
    if (i > delta)
	buf->pos = i - delta;
    else
	buf->pos = 0;
    cpos = COLPOS(l, buf->pos);
    buf->visualpos = cpos - buf->currentColumn;
    if (buf->visualpos < 0 && n) {
	columnSkip(buf, -n + buf->visualpos - buf->visualpos % n);
	buf->visualpos = cpos - buf->currentColumn;
    }
    buf->cursorX = buf->visualpos;
}

void
cursorHome(Buffer *buf)
{
    buf->visualpos = 0;
    buf->cursorX = buf->cursorY = 0;
}


/* 
 * Arrange line,column and cursor position according to current line and
 * current position.
 */
void
arrangeCursor(Buffer *buf)
{
    int col, col2;
    int delta = 1;
    if (buf == NULL || buf->currentLine == NULL)
	return;
    /* Arrange line */
    if (buf->currentLine->linenumber - buf->topLine->linenumber >= LASTLINE ||
	buf->currentLine->linenumber < buf->topLine->linenumber) {
	buf->topLine = buf->currentLine;
    }
    /* Arrange column */
    if (buf->currentLine->len == 0)
	buf->pos = 0;
    else if (buf->pos >= buf->currentLine->len)
	buf->pos = buf->currentLine->len - 1;
#ifdef JP_CHARSET
    if (CharType(buf->currentLine->propBuf[buf->pos]) == PC_KANJI2)
	buf->pos--;
#endif				/* JP_CHARSET */
    col = COLPOS(buf->currentLine, buf->pos);
#ifdef JP_CHARSET
    if (CharType(buf->currentLine->propBuf[buf->pos]) == PC_KANJI1)
	delta = 2;
#endif				/* JP_CHARSET */
    col2 = COLPOS(buf->currentLine, buf->pos + delta);
    if (col < buf->currentColumn || col2 > buf->COLS + buf->currentColumn) {
	buf->currentColumn = 0;
	if (col2 > buf->COLS)
	    columnSkip(buf, col);
    }
    /* Arrange cursor */
    buf->cursorY = buf->currentLine->linenumber - buf->topLine->linenumber;
    buf->visualpos = buf->cursorX =
	COLPOS(buf->currentLine, buf->pos) - buf->currentColumn;
#ifdef DISPLAY_DEBUG
    fprintf(stderr,
	    "arrangeCursor: column=%d, cursorX=%d, visualpos=%d, pos=%d, len=%d\n",
	    buf->currentColumn, buf->cursorX, buf->visualpos, buf->pos,
	    buf->currentLine->len);
#endif
}

void
arrangeLine(Buffer *buf)
{
    int i, cpos;

    if (buf->firstLine == NULL)
	return;
    buf->cursorY = buf->currentLine->linenumber - buf->topLine->linenumber;
    i = columnPos(buf->currentLine, buf->currentColumn + buf->visualpos);
    cpos = COLPOS(buf->currentLine, i) - buf->currentColumn;
    if (cpos >= 0) {
	buf->cursorX = cpos;
	buf->pos = i;
    }
    else if (buf->currentLine->len > i) {
	int delta = 1;
#ifdef JP_CHARSET
	if (buf->currentLine->len > i + 1 &&
	    CharType(buf->currentLine->propBuf[i + 1]) == PC_KANJI2)
	    delta = 2;
#endif
	buf->cursorX = 0;
	buf->pos = i;
	if (COLPOS(buf->currentLine, i + delta) <= buf->currentColumn)
	    buf->pos += delta;
    }
    else {
	buf->cursorX = 0;
	buf->pos = 0;
    }
#ifdef DISPLAY_DEBUG
    fprintf(stderr,
	    "arrangeLine: column=%d, cursorX=%d, visualpos=%d, pos=%d, len=%d\n",
	    buf->currentColumn, buf->cursorX, buf->visualpos, buf->pos,
	    buf->currentLine->len);
#endif
}

void
cursorXY(Buffer *buf, int x, int y)
{
    int oldX;

    cursorUpDown(buf, y - buf->cursorY);

    if (buf->cursorX > x) {
	while (buf->cursorX > x)
	    cursorLeft(buf, buf->COLS / 2);
    }
    else if (buf->cursorX < x) {
	while (buf->cursorX < x) {
	    oldX = buf->cursorX;

	    cursorRight(buf, buf->COLS / 2);

	    if (oldX == buf->cursorX)
		break;
	}
	if (buf->cursorX > x)
	    cursorLeft(buf, buf->COLS / 2);
    }
}

void
restorePosition(Buffer *buf, Buffer *orig)
{
    buf->topLine = lineSkip(buf, buf->firstLine, TOP_LINENUMBER(orig) - 1,
			    FALSE);
    gotoLine(buf, CUR_LINENUMBER(orig));
    buf->pos = orig->pos;
    buf->currentColumn = orig->currentColumn;
    arrangeCursor(buf);
}

/* Local Variables:    */
/* c-basic-offset: 4   */
/* tab-width: 8        */
/* End:                */
