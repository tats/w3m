/* $Id: anchor.c,v 1.10 2002/03/29 16:39:37 ukai Exp $ */
#include "fm.h"
#include "myctype.h"
#include "regex.h"

#define FIRST_ANCHOR_SIZE 30

AnchorList *
putAnchor(AnchorList *al, char *url, char *target, Anchor **anchor_return,
	  char *referer, int line, int pos)
{
    int n, i, j;
    Anchor *a;
    BufferPoint bp;
    if (al == NULL) {
	al = New(AnchorList);
	al->anchors = NULL;
	al->nanchor = al->anchormax = 0;
	al->acache = -1;
    }
    if (al->anchormax == 0) {
	/* first time; allocate anchor buffer */
	al->anchors = New_N(Anchor, FIRST_ANCHOR_SIZE);
	al->anchormax = FIRST_ANCHOR_SIZE;
    }
    if (al->nanchor == al->anchormax) {	/* need realloc */
	al->anchormax *= 2;
	al->anchors = New_Reuse(Anchor, al->anchors, al->anchormax);
    }
    bp.line = line;
    bp.pos = pos;
    n = al->nanchor;
    if (!n || bpcmp(al->anchors[n - 1].start, bp) < 0)
	i = n;
    else
	for (i = 0; i < n; i++) {
	    if (bpcmp(al->anchors[i].start, bp) >= 0) {
		for (j = n; j > i; j--)
		    al->anchors[j] = al->anchors[j - 1];
		break;
	    }
	}
    a = &al->anchors[i];
    a->url = url;
    a->target = target;
    a->referer = referer;
    a->start = bp;
    a->end = bp;
    al->nanchor++;
    if (anchor_return)
	*anchor_return = a;
    return al;
}


Anchor *
registerHref(Buffer *buf, char *url, char *target, char *referer, int line,
	     int pos)
{
    Anchor *a;
    buf->href = putAnchor(buf->href, url, target, &a, referer, line, pos);
    return a;
}

Anchor *
registerName(Buffer *buf, char *url, int line, int pos)
{
    Anchor *a;
    buf->name = putAnchor(buf->name, url, NULL, &a, NULL, line, pos);
    return a;
}

Anchor *
registerImg(Buffer *buf, char *url, int line, int pos)
{
    Anchor *a;
    buf->img = putAnchor(buf->img, url, NULL, &a, NULL, line, pos);
    return a;
}

Anchor *
registerForm(Buffer *buf, FormList *flist, struct parsed_tag *tag, int line,
	     int pos)
{
    Anchor *a;
    FormItemList *fi;

    fi = formList_addInput(flist, tag);
    if (fi == NULL)
	return NULL;
    buf->formitem = putAnchor(buf->formitem,
			      (char *)fi, flist->target, &a, NULL, line, pos);
    return a;
}

int
onAnchor(Anchor *a, int line, int pos)
{
    BufferPoint bp;
    bp.line = line;
    bp.pos = pos;

    if (bpcmp(bp, a->start) < 0)
	return -1;
    if (bpcmp(a->end, bp) <= 0)
	return 1;
    return 0;
}

Anchor *
retrieveAnchor(AnchorList *al, int line, int pos)
{
    Anchor *a;
    size_t b, e;
    int cmp;

    if (al == NULL || al->nanchor == 0)
	return NULL;

    if (al->acache < 0 || al->acache >= al->nanchor)
	al->acache = 0;

    for (b = 0, e = al->nanchor - 1; b <= e; al->acache = (b + e) / 2) {
	a = &al->anchors[al->acache];
	cmp = onAnchor(a, line, pos);
	if (cmp == 0)
	    return a;
	else if (cmp > 0)
	    b = al->acache + 1;
	else if (al->acache == 0)
	    return NULL;
	else
	    e = al->acache - 1;
    }
    return NULL;
}

Anchor *
retrieveCurrentAnchor(Buffer *buf)
{
    if (buf->currentLine == NULL)
	return NULL;
    return retrieveAnchor(buf->href, buf->currentLine->linenumber, buf->pos);
}

Anchor *
retrieveCurrentImg(Buffer *buf)
{
    if (buf->currentLine == NULL)
	return NULL;
    return retrieveAnchor(buf->img, buf->currentLine->linenumber, buf->pos);
}

Anchor *
retrieveCurrentForm(Buffer *buf)
{
    if (buf->currentLine == NULL)
	return NULL;
    return retrieveAnchor(buf->formitem,
			  buf->currentLine->linenumber, buf->pos);
}

Anchor *
searchAnchor(AnchorList *al, char *str)
{
    int i;
    Anchor *a;
    if (al == NULL)
	return NULL;
    for (i = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (!strcmp(a->url, str))
	    return a;
    }
    return NULL;
}

Anchor *
searchURLLabel(Buffer *buf, char *url)
{
    return searchAnchor(buf->name, url);
}

#ifdef USE_NNTP
static Anchor *
_put_anchor_news(Buffer *buf, char *p1, char *p2, int line, int pos)
{
    Str tmp;

    p1++;
    if (*(p2 - 1) == '>')
	p2--;
    tmp = Strnew_size(sizeof("news:") - 1 + (p2 - p1));
    Strcat_charp_n(tmp, "news:", sizeof("news:") - 1);
    Strcat_charp_n(tmp, p1, p2 - p1);
    return registerHref(buf, tmp->ptr, NULL, NO_REFERER, line, pos);
}
#endif				/* USE_NNTP */

static Anchor *
_put_anchor_all(Buffer *buf, char *p1, char *p2, int line, int pos)
{
    return registerHref(buf, allocStr(p1, p2 - p1), NULL, NO_REFERER, line,
			pos);
}

static void
reseq_anchor0(AnchorList *al, short *seqmap)
{
    int i;
    Anchor *a;

    if (!al)
	return;

    for (i = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (a->hseq >= 0) {
	    a->hseq = seqmap[a->hseq];
	}
    }
}

/* renumber anchor */
static void
reseq_anchor(Buffer *buf)
{
    int i, j, n, nmark = (buf->hmarklist) ? buf->hmarklist->nmark : 0;
    short *seqmap;
    Anchor *a, *a1;
    HmarkerList *ml = NULL;

    if (!buf->href)
	return;

    n = nmark;
    for (i = 0; i < buf->href->nanchor; i++) {
	a = &buf->href->anchors[i];
	if (a->hseq == -2)
	    n++;
    }

    if (n == nmark)
	return;

    seqmap = NewAtom_N(short, n);

    for (i = 0; i < n; i++)
	seqmap[i] = i;

    n = nmark;
    for (i = 0; i < buf->href->nanchor; i++) {
	a = &buf->href->anchors[i];
	if (a->hseq == -2) {
	    a->hseq = n;
	    a1 = closest_next_anchor(buf->href, NULL, a->start.pos,
				     a->start.line);
	    a1 = closest_next_anchor(buf->formitem, a1, a->start.pos,
				     a->start.line);
	    if (a1 && a1->hseq >= 0) {
		seqmap[n] = seqmap[a1->hseq];
		for (j = a1->hseq; j < nmark; j++)
		    seqmap[j]++;
	    }
	    ml = putHmarker(ml, a->start.line, a->start.pos, seqmap[n]);
	    n++;
	}
    }

    for (i = 0; i < nmark; i++) {
	ml = putHmarker(ml, buf->hmarklist->marks[i].line,
			buf->hmarklist->marks[i].pos, seqmap[i]);
    }
    buf->hmarklist = ml;

    reseq_anchor0(buf->href, seqmap);
    reseq_anchor0(buf->formitem, seqmap);
}

/* search regexp and register them as anchors */
/* returns error message if any               */
static char *
reAnchorAny(Buffer *buf, char *re,
	    Anchor *(*anchorproc) (Buffer *, char *, char *, int, int))
{
    Line *l;
    char *p, *p1, *p2;
    Anchor *a;
    int i;
    int spos, epos;

    if (re == NULL || *re == '\0') {
	return NULL;
    }
    if ((re = regexCompile(re, 1)) != NULL) {
	return re;
    }
    for (l = MarkAllPages ? buf->firstLine : buf->topLine; l != NULL &&
	 (MarkAllPages || l->linenumber < buf->topLine->linenumber + LASTLINE);
	 l = l->next) {
	p = l->lineBuf;
	for (;;) {
	    if (regexMatch(p, &l->lineBuf[l->len] - p, p == l->lineBuf) == 1) {
		matchedPosition(&p1, &p2);
		spos = p1 - l->lineBuf;
		epos = p2 - l->lineBuf;
		for (i = spos; i < epos; i++) {
		    if (l->propBuf[i] & (PE_ANCHOR | PE_FORM))
			goto _next;
		}
		a = anchorproc(buf, p1, p2, l->linenumber, p1 - l->lineBuf);
		a->end.line = l->linenumber;
		a->end.pos = epos;
		a->hseq = -2;
		for (i = a->start.pos; i < a->end.pos; i++)
		    l->propBuf[i] |= PE_ANCHOR;
	      _next:
		p = p2;
	    }
	    else
		break;
	}
    }
    reseq_anchor(buf);
    return NULL;
}

char *
reAnchor(Buffer *buf, char *re)
{
    return reAnchorAny(buf, re, _put_anchor_all);
}

#ifdef USE_NNTP
char *
reAnchorNews(Buffer *buf, char *re)
{
    return reAnchorAny(buf, re, _put_anchor_news);
}
#endif				/* USE_NNTP */

#define FIRST_MARKER_SIZE 30
HmarkerList *
putHmarker(HmarkerList *ml, int line, int pos, int seq)
{
    if (ml == NULL) {
	ml = New(HmarkerList);
	ml->marks = NULL;
	ml->nmark = 0;
	ml->markmax = 0;
	ml->prevhseq = -1;
    }
    if (ml->markmax == 0) {
	ml->markmax = FIRST_MARKER_SIZE;
	ml->marks = NewAtom_N(BufferPoint, ml->markmax);
	bzero(ml->marks, sizeof(BufferPoint) * ml->markmax);
    }
    if (seq + 1 > ml->nmark)
	ml->nmark = seq + 1;
    if (ml->nmark >= ml->markmax) {
	ml->markmax = ml->nmark * 2;
	ml->marks = New_Reuse(BufferPoint, ml->marks, ml->markmax);
    }
    ml->marks[seq].line = line;
    ml->marks[seq].pos = pos;
    return ml;
}

Anchor *
closest_next_anchor(AnchorList *a, Anchor *an, int x, int y)
{
    int i;

    if (a == NULL || a->nanchor == 0)
	return an;
    for (i = 0; i < a->nanchor; i++) {
	if (a->anchors[i].hseq < 0)
	    continue;
	if (a->anchors[i].start.line > y ||
	    (a->anchors[i].start.line == y && a->anchors[i].start.pos > x)) {
	    if (an == NULL || an->start.line > a->anchors[i].start.line ||
		(an->start.line == a->anchors[i].start.line &&
		 an->start.pos > a->anchors[i].start.pos))
		an = &a->anchors[i];
	}
    }
    return an;
}

Anchor *
closest_prev_anchor(AnchorList *a, Anchor *an, int x, int y)
{
    int i;

    if (a == NULL || a->nanchor == 0)
	return an;
    for (i = 0; i < a->nanchor; i++) {
	if (a->anchors[i].hseq < 0)
	    continue;
	if (a->anchors[i].end.line < y ||
	    (a->anchors[i].end.line == y && a->anchors[i].end.pos <= x)) {
	    if (an == NULL || an->end.line < a->anchors[i].end.line ||
		(an->end.line == a->anchors[i].end.line &&
		 an->end.pos < a->anchors[i].end.pos))
		an = &a->anchors[i];
	}
    }
    return an;
}

#ifdef USE_IMAGE
void
addMultirowsImg(Buffer *buf, AnchorList *al)
{
    int i, j, k, col, ecol, pos;
    Image *img;
    Anchor a_img, a_href, a_form, *a;
    Line *l, *ls;

    if (al == NULL || al->nanchor == 0)
	return;
    for (i = 0; i < al->nanchor; i++) {
	a_img = al->anchors[i];
	img = a_img.image;
	if (a_img.hseq < 0 || !img || img->rows <= 1)
	    continue;
	for (l = buf->firstLine; l != NULL; l = l->next) {
	    if (l->linenumber == img->y)
		break;
	}
	if (!l)
	    continue;
	if (a_img.y == a_img.start.line)
	    ls = l;
	else {
	    for (ls = l; ls != NULL;
		 ls = (a_img.y < a_img.start.line) ? ls->next : ls->prev) {
		if (ls->linenumber == a_img.start.line)
		    break;
	    }
	    if (!ls)
		continue;
	}
	a = retrieveAnchor(buf->href, a_img.start.line, a_img.start.pos);
	if (a)
	    a_href = *a;
	else
	    a_href.url = NULL;
	a = retrieveAnchor(buf->formitem, a_img.start.line, a_img.start.pos);
	if (a)
	    a_form = *a;
	else
	    a_form.url = NULL;
	col = COLPOS(ls, a_img.start.pos);
	ecol = COLPOS(ls, a_img.end.pos);
	for (j = 0; l && j < img->rows; l = l->next, j++) {
	    if (a_img.start.line == l->linenumber)
		continue;
	    pos = columnPos(l, col);
	    a = registerImg(buf, a_img.url, l->linenumber, pos);
	    a->hseq = -a_img.hseq;
	    a->image = img;
	    a->end.pos = pos + ecol - col;
	    for (k = pos; k < a->end.pos; k++)
		l->propBuf[k] |= PE_IMAGE;
	    if (a_href.url) {
		a = registerHref(buf, a_href.url, a_href.target,
				 a_href.referer, l->linenumber, pos);
		a->hseq = a_href.hseq;
		a->end.pos = pos + ecol - col;
		for (k = pos; k < a->end.pos; k++)
		    l->propBuf[k] |= PE_ANCHOR;
	    }
	    if (a_form.url) {
		buf->formitem = putAnchor(buf->formitem, a_form.url,
					  a_form.target, &a, NULL,
					  l->linenumber, pos);
		a->hseq = a_form.hseq;
		a->end.pos = pos + ecol - col;
	    }
	}
	img->rows = 0;
    }
}
#endif

void
addMultirowsForm(Buffer *buf, AnchorList *al)
{
    int i, j, k, col, ecol, pos;
    Anchor a_form, *a;
    FormItemList *fi;
    Line *l, *ls;

    if (al == NULL || al->nanchor == 0)
	return;
    for (i = 0; i < al->nanchor; i++) {
	a_form = al->anchors[i];
	al->anchors[i].rows = 1;
	if (a_form.hseq < 0 || a_form.rows <= 1)
	    continue;
	for (l = buf->firstLine; l != NULL; l = l->next) {
	    if (l->linenumber == a_form.y)
		break;
	}
	if (!l)
	    continue;
	if (a_form.y == a_form.start.line)
	    ls = l;
	else {
	    for (ls = l; ls != NULL;
		 ls = (a_form.y < a_form.start.line) ? ls->next : ls->prev) {
		if (ls->linenumber == a_form.start.line)
		    break;
	    }
	    if (!ls)
		continue;
	}
	fi = (FormItemList *)a_form.url;
	col = COLPOS(ls, a_form.start.pos);
	ecol = COLPOS(ls, a_form.end.pos);
	for (j = 0; l && j < a_form.rows; l = l->next, j++) {
	    pos = columnPos(l, col);
	    if (j == 0) {
		buf->hmarklist->marks[a_form.hseq].line = l->linenumber;
		buf->hmarklist->marks[a_form.hseq].pos = pos;
	    }
	    if (a_form.start.line == l->linenumber)
		continue;
	    buf->formitem = putAnchor(buf->formitem, a_form.url,
				      a_form.target, &a, NULL, l->linenumber,
				      pos);
	    a->hseq = a_form.hseq;
	    a->y = a_form.y;
	    a->end.pos = pos + ecol - col;
	    l->lineBuf[pos - 1] = '[';
	    l->lineBuf[a->end.pos] = ']';
	    for (k = pos; k < a->end.pos; k++)
		l->propBuf[k] |= PE_FORM;
	}
    }
}
