#include "fm.h"

#ifdef MOUSE
#ifdef USE_GPM
#include <gpm.h>
#endif
#if defined(USE_GPM) || defined(USE_SYSMOUSE)
extern int do_getch();
#define getch()	do_getch()
#endif				/* USE_GPM */
#endif				/* MOUSE */

#ifdef __EMX__
#include <sys/kbdscan.h>
#include <strings.h>
#endif
char *NullLine = "";
Lineprop NullProp[] =
{0};

/* 
 * Buffer creation
 */
Buffer *
newBuffer(int width)
{
    Buffer *n;

    n = New(Buffer);
    if (n == NULL)
	return NULL;
    bzero((void *) n, sizeof(Buffer));
    n->width = width;
    n->currentURL.scheme = SCM_UNKNOWN;
    n->baseURL = NULL;
    n->baseTarget = NULL;
    n->buffername = "";
    n->bufferprop = BP_NORMAL;
    n->clone = New(int);
    *n->clone = 1;
    n->linelen = 0;
    n->trbyte = 0;
#ifdef USE_SSL
    n->ssl_certificate = NULL;
#endif
    return n;
}

/* 
 * Create null buffer
 */
Buffer *
nullBuffer(void)
{
    Buffer *b;

    b = newBuffer(COLS);
    b->buffername = "*Null*";
    return b;
}

/* 
 * clearBuffer: clear buffer content
 */
void
clearBuffer(Buffer * buf)
{
    buf->firstLine = buf->topLine = buf->currentLine = buf->lastLine = NULL;
    buf->allLine = 0;
}

/* 
 * discardBuffer: free buffer structure
 */

void
discardBuffer(Buffer * buf)
{
    int i;
    Buffer *b;

    clearBuffer(buf);
    for (i = 0; i < MAX_LB; i++) {
	b = buf->linkBuffer[i];
	if (b == NULL)
	    continue;
	b->linkBuffer[REV_LB[i]] = NULL;
    }
    if (buf->savecache)
	unlink(buf->savecache);
    if (--(*buf->clone))
	return;
    if (buf->pagerSource)
	ISclose(buf->pagerSource);
    if (buf->sourcefile) {
	if (buf->real_scheme != SCM_LOCAL || buf->bufferprop & BP_FRAME)
	    unlink(buf->sourcefile);
    }
    if (buf->mailcap_source)
	unlink(buf->mailcap_source);
    while (buf->frameset) {
	deleteFrameSet(buf->frameset);
       buf->frameset = popFrameTree(&(buf->frameQ));
    }
}

/* 
 * namedBuffer: Select buffer which have specified name
 */
Buffer *
namedBuffer(Buffer * first, char *name)
{
    Buffer *buf;

    if (!strcmp(first->buffername, name)) {
	return first;
    }
    for (buf = first; buf->nextBuffer != NULL; buf = buf->nextBuffer) {
	if (!strcmp(buf->nextBuffer->buffername, name)) {
	    return buf->nextBuffer;
	}
    }
    return NULL;
}

/* 
 * deleteBuffer: delete buffer
 */
Buffer *
deleteBuffer(Buffer * first, Buffer * delbuf)
{
    Buffer *buf, *b;

    if (first == delbuf && first->nextBuffer != NULL) {
	buf = first->nextBuffer;
	discardBuffer(first);
	return buf;
    }
    if ((buf = prevBuffer(first, delbuf)) != NULL) {
	b = buf->nextBuffer;
	buf->nextBuffer = b->nextBuffer;
	discardBuffer(b);
    }
    return first;
}

/* 
 * replaceBuffer: replace buffer
 */
Buffer *
replaceBuffer(Buffer * first, Buffer * delbuf, Buffer * newbuf)
{
    Buffer *buf;

    if (delbuf == NULL) {
	newbuf->nextBuffer = first;
	return newbuf;
    }
    if (first == delbuf) {
	newbuf->nextBuffer = delbuf->nextBuffer;
	discardBuffer(delbuf);
	return newbuf;
    }
    if (delbuf && (buf = prevBuffer(first, delbuf))) {
	buf->nextBuffer = newbuf;
	newbuf->nextBuffer = delbuf->nextBuffer;
	discardBuffer(delbuf);
	return first;
    }
    newbuf->nextBuffer = first;
    return newbuf;
}

Buffer *
nthBuffer(Buffer * firstbuf, int n)
{
    int i;
    Buffer *buf = firstbuf;

    if (n < 0)
	return firstbuf;
    for (i = 0; i < n; i++) {
	if (buf == NULL)
	    return NULL;
	buf = buf->nextBuffer;
    }
    return buf;
}

static void
writeBufferName(Buffer * buf, int n)
{
    Str msg;
    int all;

    all = buf->allLine;
    if (all == 0 && buf->lastLine != NULL)
	all = buf->lastLine->linenumber;
    move(n, 0);
    msg = Sprintf("<%s> [%d lines]", buf->buffername, all);
    if (buf->filename != NULL) {
	switch (buf->currentURL.scheme) {
	case SCM_LOCAL:
	case SCM_LOCAL_CGI:
	    if (strcmp(buf->currentURL.file, "-")) {
		Strcat_char(msg, ' ');
		Strcat_charp(msg, conv_from_system(buf->currentURL.real_file));
	    }
	    break;
	case SCM_UNKNOWN:
	case SCM_MISSING:
	    break;
	default:
	    Strcat_char(msg, ' ');
	    Strcat(msg, parsedURL2Str(&buf->currentURL));
	    break;
	}
    }
    addnstr_sup(msg->ptr, COLS - 1);
}


/* 
 * gotoLine: go to line number
 */
void
gotoLine(Buffer * buf, int n)
{
    char msg[32];
    Line *l = buf->firstLine;

    if (l == NULL)
	return;
    if (buf->pagerSource && !(buf->bufferprop & BP_CLOSE)) {
	if (buf->lastLine->linenumber < n)
	    getNextPage(buf, n - buf->lastLine->linenumber);
	while ((buf->lastLine->linenumber < n) &&
	       (getNextPage(buf, 1) != NULL));
    }
    if (l->linenumber > n) {
	sprintf(msg, "First line is #%ld", l->linenumber);
	disp_message(msg, FALSE);
	buf->topLine = buf->currentLine = l;
	return;
    }
    if (buf->lastLine->linenumber < n) {
	l = buf->lastLine;
	sprintf(msg, "Last line is #%ld", buf->lastLine->linenumber);
	disp_message(msg, FALSE);
	buf->currentLine = l;
	buf->topLine = lineSkip(buf, buf->currentLine, - (LASTLINE - 1), FALSE);
	return;
    }
    for (; l != NULL; l = l->next) {
	if (l->linenumber >= n) {
	    buf->currentLine = l;
	    if (n < buf->topLine->linenumber ||
		buf->topLine->linenumber + LASTLINE <= n)
		buf->topLine = lineSkip(buf, l, -(LASTLINE + 1) / 2, FALSE);
	    break;
	}
    }
}

/* 
 * gotoRealLine: go to real line number
 */
void
gotoRealLine(Buffer * buf, int n)
{
    char msg[32];
    Line *l = buf->firstLine;

    if (l == NULL)
	return;
    if (buf->pagerSource && !(buf->bufferprop & BP_CLOSE)) {
	if (buf->lastLine->real_linenumber < n)
	    getNextPage(buf, n - buf->lastLine->real_linenumber);
	while ((buf->lastLine->real_linenumber < n) &&
	       (getNextPage(buf, 1) != NULL));
    }
    if (l->real_linenumber > n) {
	sprintf(msg, "First line is #%ld", l->real_linenumber);
	disp_message(msg, FALSE);
	buf->topLine = buf->currentLine = l;
	return;
    }
    if (buf->lastLine->real_linenumber < n) {
	l = buf->lastLine;
	sprintf(msg, "Last line is #%ld", buf->lastLine->real_linenumber);
	disp_message(msg, FALSE);
	buf->currentLine = l;
	buf->topLine = lineSkip(buf, buf->currentLine, - (LASTLINE - 1), FALSE);
	return;
    }
    for (; l != NULL; l = l->next) {
	if (l->real_linenumber >= n) {
	    buf->currentLine = l;
	    if (n < buf->topLine->real_linenumber ||
		buf->topLine->real_linenumber + LASTLINE <= n)
		buf->topLine = lineSkip(buf, l, -(LASTLINE + 1) / 2, FALSE);
	    break;
	}
    }
}


static Buffer *
listBuffer(Buffer * top, Buffer * current)
{
    int i, c = 0;
    Buffer *buf = top;

    move(0, 0);
#ifdef COLOR
    if (useColor) {
	setfcolor(basic_color);
#ifdef BG_COLOR
	setbcolor(bg_color);
#endif				/* BG_COLOR */
    }
#endif				/* COLOR */
    clrtobotx();
    for (i = 0; i < LASTLINE; i++) {
	if (buf == current) {
	    c = i;
	    standout();
	}
	writeBufferName(buf, i);
	if (buf == current) {
	    standend();
	    clrtoeolx();
	    move(i, 0);
	    toggle_stand();
	}
	else
	    clrtoeolx();
	if (buf->nextBuffer == NULL) {
	    move(i + 1, 0);
	    clrtobotx();
	    break;
	}
	buf = buf->nextBuffer;
    }
    standout();
    message("Buffer selection mode: SPC for select / D for delete buffer", 0, 0);
    standend();
/* 
 * move(LASTLINE, COLS - 1); */
    move(c, 0);
    refresh();
    return buf->nextBuffer;
}


/* 
 * Select buffer visually
 */
Buffer *
selectBuffer(Buffer * firstbuf, Buffer * currentbuf, char *selectchar)
{
    int i, cpoint,		/* Current Buffer Number */
     spoint,			/* Current Line on Screen */
     maxbuf, sclimit = LASTLINE;	/* Upper limit of line * number in 
					 * the * screen */
    Buffer *buf, *topbuf;
    char c;

    i = cpoint = 0;
    for (buf = firstbuf; buf != NULL; buf = buf->nextBuffer) {
	if (buf == currentbuf)
	    cpoint = i;
	i++;
    }
    maxbuf = i;

    if (cpoint >= sclimit) {
	spoint = sclimit / 2;
	topbuf = nthBuffer(firstbuf, cpoint - spoint);
    }
    else {
	topbuf = firstbuf;
	spoint = cpoint;
    }
    listBuffer(topbuf, currentbuf);

    for (;;) {
	if ((c = getch()) == ESC_CODE) {
	    if ((c = getch()) == '[' || c == 'O') {
		switch (c = getch()) {
		case 'A':
		    c = 'k';
		    break;
		case 'B':
		    c = 'j';
		    break;
		case 'C':
		    c = ' ';
		    break;
		case 'D':
		    c = 'B';
		    break;
		}
	    }
	}
#ifdef __EMX__
	else if(!c)
	  switch(getch()){
	    case K_UP:
		c='k';
		break;
	    case K_DOWN:
		c='j';
		break;
	    case K_RIGHT:
		c=' ';
		break;
	    case K_LEFT:
		c='B';
	}
#endif
	switch (c) {
	case CTRL_N:
	case 'j':
	    if (spoint < sclimit - 1) {
		if (currentbuf->nextBuffer == NULL)
		    continue;
		writeBufferName(currentbuf, spoint);
		currentbuf = currentbuf->nextBuffer;
		cpoint++;
		spoint++;
		standout();
		writeBufferName(currentbuf, spoint);
		standend();
		move(spoint, 0);
		toggle_stand();
	    }
	    else if (cpoint < maxbuf - 1) {
		topbuf = currentbuf;
		currentbuf = currentbuf->nextBuffer;
		cpoint++;
		spoint = 1;
		listBuffer(topbuf, currentbuf);
	    }
	    break;
	case CTRL_P:
	case 'k':
	    if (spoint > 0) {
		writeBufferName(currentbuf, spoint);
		currentbuf = nthBuffer(topbuf, --spoint);
		cpoint--;
		standout();
		writeBufferName(currentbuf, spoint);
		standend();
		move(spoint, 0);
		toggle_stand();
	    }
	    else if (cpoint > 0) {
		i = cpoint - sclimit;
		if (i < 0)
		    i = 0;
		cpoint--;
		spoint = cpoint - i;
		currentbuf = nthBuffer(firstbuf, cpoint);
		topbuf = nthBuffer(firstbuf, i);
		listBuffer(topbuf, currentbuf);
	    }
	    break;
	default:
	    *selectchar = c;
	    return currentbuf;
	}
/* 
 * move(LASTLINE, COLS - 1);
 */
	move(spoint, 0);
	refresh();
    }
}

/* 
 * Reshape HTML buffer
 */
void
reshapeBuffer(Buffer * buf)
{
    URLFile f;
    int top, linenum, cursorY, pos, currentColumn;
    AnchorList *formitem;

    if (buf->sourcefile == NULL)
       return;
    init_stream(&f, SCM_LOCAL, NULL);
    examineFile(buf->mailcap_source ? buf->mailcap_source : buf->sourcefile, &f);
    if (f.stream == NULL)
        return;

    if (buf->firstLine == NULL) {
	top = 1;
	linenum = 1;
    } else {
	top = buf->topLine->linenumber;
	linenum = buf->currentLine->linenumber;
    }
    cursorY = buf->cursorY;
    pos = buf->pos;
    currentColumn = buf->currentColumn;
    clearBuffer(buf);
    while (buf->frameset) {
	deleteFrameSet(buf->frameset);
       buf->frameset = popFrameTree(&(buf->frameQ));
    }

    formitem = buf->formitem;
    buf->href = NULL;
    buf->name = NULL;
    buf->img = NULL;
    buf->formitem = NULL;
    buf->width = INIT_BUFFER_WIDTH;

#ifdef JP_CHARSET
    UseContentCharset = FALSE;
    UseAutoDetect = FALSE;
#endif
    if (! strcasecmp(buf->type, "text/html"))
       loadHTMLBuffer(&f, buf);
    else
       loadBuffer(&f, buf);
    UFclose(&f);
#ifdef JP_CHARSET
    UseContentCharset = TRUE;
    UseAutoDetect = TRUE;
#endif

    buf->height = LASTLINE + 1;
    buf->topLine = lineSkip(buf, buf->topLine, top - 1, FALSE);
    gotoLine(buf, linenum);
    buf->pos = pos;
    buf->currentColumn = currentColumn;
    arrangeCursor(buf);
    if (buf->check_url & CHK_URL)
	chkURL();
#ifdef USE_NNTP
    if (buf->check_url & CHK_NMID)
	chkNMID();
#endif
    formResetBuffer(buf, formitem);
}

/* shallow copy */
void
copyBuffer(Buffer * a, Buffer * b)
{
    readBufferCache(b);
    bcopy((void *) b, (void *) a, sizeof(Buffer));
}

Buffer *
prevBuffer(Buffer * first, Buffer * buf)
{
    Buffer *b;

    for (b = first; b != NULL && b->nextBuffer != buf; b = b->nextBuffer);
    return b;
}

#define fwrite1(d, f) (fwrite(&d, sizeof(d), 1, f)==0)
#define fread1(d, f) (fread(&d, sizeof(d), 1, f)==0)

int
writeBufferCache(Buffer *buf)
{
    Str tmp;
    FILE *cache = NULL;
    Line *l;
#ifdef ANSI_COLOR
    int colorflag;
#endif

    if (buf->savecache)
	return -1;

    if (buf->firstLine == NULL)
	goto _error1;

    tmp = tmpfname(TMPF_CACHE, NULL);
    buf->savecache = tmp->ptr;
    cache = fopen(buf->savecache, "w");
    if (!cache)
	goto _error1;

    if (fwrite1(buf->currentLine->linenumber, cache) ||
	fwrite1(buf->topLine->linenumber, cache))
	goto _error;

    for (l = buf->firstLine; l; l = l->next) {
	if (fwrite1(l->real_linenumber, cache) ||
	    fwrite1(l->usrflags, cache) ||
	    fwrite1(l->width, cache) ||
	    fwrite1(l->len, cache) ||
	    fwrite(l->lineBuf, 1, l->len, cache) < l->len ||
	    fwrite(l->propBuf, sizeof(Lineprop), l->len, cache) < l->len)
	    goto _error;
#ifdef ANSI_COLOR
        colorflag = l->colorBuf ? 1 : 0;
	if (fwrite1(colorflag, cache))
	    goto _error;
	if (colorflag) {
	    if (fwrite(l->colorBuf, sizeof(Linecolor), l->len, cache) < l->len)
		goto _error;
	}
#endif
    }

    fclose(cache);
    return 0;
    _error:
    fclose(cache);
    unlink(buf->savecache);
    _error1:
    buf->savecache = NULL;
    return -1;
}

int
readBufferCache(Buffer *buf)
{
    FILE *cache;
    Line *l = NULL, *prevl;
    long lnum = 0, clnum, tlnum;
#ifdef ANSI_COLOR
    int colorflag;
#endif

    if (buf->savecache == NULL)
	return -1;

    cache = fopen(buf->savecache, "r");
    if (cache == NULL ||
	fread1(clnum, cache) ||
	fread1(tlnum, cache)) {
	buf->savecache = NULL;
	return -1;
    }

    while (!feof(cache)) {
	lnum++;
	prevl = l;
	l = New(Line);
	l->prev = prevl;
	if (prevl)
	    prevl->next = l;
	else
	    buf->firstLine = l;
	l->linenumber = lnum;
	if (lnum == clnum)
	    buf->currentLine = l;
	if (lnum == tlnum)
	    buf->topLine = l;
	if (fread1(l->real_linenumber, cache) ||
	    fread1(l->usrflags, cache) ||
	    fread1(l->width, cache) ||
	    fread1(l->len, cache))
	    break;
	l->lineBuf = NewAtom_N(char, l->len + 1);
	fread(l->lineBuf, 1, l->len, cache);
	l->lineBuf[l->len] = '\0';
	l->propBuf = NewAtom_N(Lineprop, l->len);
	fread(l->propBuf, sizeof(Lineprop), l->len, cache);
#ifdef ANSI_COLOR
	if (fread1(colorflag, cache))
	    break;
	if (colorflag) {
	    l->colorBuf = NewAtom_N(Linecolor, l->len);
	    fread(l->colorBuf, sizeof(Linecolor), l->len, cache);
	} else {
	    l->colorBuf = NULL;
	}
#endif
    }
    buf->lastLine = prevl;
    buf->lastLine->next = NULL;
    fclose(cache);
    unlink(buf->savecache);
    buf->savecache = NULL;
    return 0;
}
