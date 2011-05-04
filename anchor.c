/* $Id: anchor.c,v 1.33 2006/04/08 11:33:16 inu Exp $ */
#include "fm.h"
#include "myctype.h"
#include "regex.h"

#define FIRST_ANCHOR_SIZE 30

AnchorList *
putAnchor(AnchorList *al, char *url, char *target, Anchor **anchor_return,
	  char *referer, char *title, unsigned char key, int line, int pos)
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
    a->title = title;
    a->accesskey = key;
    a->slave = FALSE;
    a->start = bp;
    a->end = bp;
    al->nanchor++;
    if (anchor_return)
	*anchor_return = a;
    return al;
}


Anchor *
registerHref(Buffer *buf, char *url, char *target, char *referer, char *title,
	     unsigned char key, int line, int pos)
{
    Anchor *a;
    buf->href = putAnchor(buf->href, url, target, &a, referer, title, key,
			  line, pos);
    return a;
}

Anchor *
registerName(Buffer *buf, char *url, int line, int pos)
{
    Anchor *a;
    buf->name = putAnchor(buf->name, url, NULL, &a, NULL, NULL, '\0', line,
			  pos);
    return a;
}

Anchor *
registerImg(Buffer *buf, char *url, char *title, int line, int pos)
{
    Anchor *a;
    buf->img = putAnchor(buf->img, url, NULL, &a, NULL, title, '\0', line,
			 pos);
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
    buf->formitem = putAnchor(buf->formitem, (char *)fi, flist->target, &a,
			      NULL, NULL, '\0', line, pos);
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
	if (a->hseq < 0)
	  continue;
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

    if (*p1 == '<') {
	p1++;
	if (*(p2 - 1) == '>')
	    p2--;
    }
    tmp = wc_Str_conv_strict(Strnew_charp_n(p1, p2 - p1), InnerCharset,
			     buf->document_charset);
    tmp = Sprintf("news:%s", file_quote(tmp->ptr));
    return registerHref(buf, tmp->ptr, NULL, NO_REFERER, NULL, '\0', line,
			pos);
}
#endif				/* USE_NNTP */

static Anchor *
_put_anchor_all(Buffer *buf, char *p1, char *p2, int line, int pos)
{
    Str tmp;

    tmp = wc_Str_conv_strict(Strnew_charp_n(p1, p2 - p1), InnerCharset,
			     buf->document_charset);
    return registerHref(buf, url_quote(tmp->ptr), NULL, NO_REFERER, NULL,
			'\0', line, pos);
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

static char *
reAnchorPos(Buffer *buf, Line *l, char *p1, char *p2,
	    Anchor *(*anchorproc) (Buffer *, char *, char *, int, int))
{
    Anchor *a;
    int spos, epos;
    int i, hseq = -2;

    spos = p1 - l->lineBuf;
    epos = p2 - l->lineBuf;
    for (i = spos; i < epos; i++) {
	if (l->propBuf[i] & (PE_ANCHOR | PE_FORM))
	    return p2;
    }
    for (i = spos; i < epos; i++)
	l->propBuf[i] |= PE_ANCHOR;
    while (spos > l->len && l->next && l->next->bpos) {
	spos -= l->len;
	epos -= l->len;
	l = l->next;
    }
    while (1) {
	a = anchorproc(buf, p1, p2, l->linenumber, spos);
	a->hseq = hseq;
	if (hseq == -2) {
	    reseq_anchor(buf);
	    hseq = a->hseq;
	}
	a->end.line = l->linenumber;
	if (epos > l->len && l->next && l->next->bpos) {
	    a->end.pos = l->len;
	    spos = 0;
	    epos -= l->len;
	    l = l->next;
	}
	else {
	    a->end.pos = epos;
	    break;
	}
    }
    return p2;
}

void
reAnchorWord(Buffer *buf, Line *l, int spos, int epos)
{
    reAnchorPos(buf, l, &l->lineBuf[spos], &l->lineBuf[epos], _put_anchor_all);
}

/* search regexp and register them as anchors */
/* returns error message if any               */
static char *
reAnchorAny(Buffer *buf, char *re,
	    Anchor *(*anchorproc) (Buffer *, char *, char *, int, int))
{
    Line *l;
    char *p = NULL, *p1, *p2;

    if (re == NULL || *re == '\0') {
	return NULL;
    }
    if ((re = regexCompile(re, 1)) != NULL) {
	return re;
    }
    for (l = MarkAllPages ? buf->firstLine : buf->topLine; l != NULL &&
	 (MarkAllPages || l->linenumber < buf->topLine->linenumber + LASTLINE);
	 l = l->next) {
	if (p && l->bpos)
	    goto next_line;
	p = l->lineBuf;
	for (;;) {
	    if (regexMatch(p, &l->lineBuf[l->size] - p, p == l->lineBuf) == 1) {
		matchedPosition(&p1, &p2);
		p = reAnchorPos(buf, l, p1, p2, anchorproc);
	    }
	    else
		break;
	}
      next_line:
	if (MarkAllPages && l->next == NULL && buf->pagerSource &&
	    !(buf->bufferprop & BP_CLOSE))
	    getNextPage(buf, PagerMax);
    }
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

char *
reAnchorNewsheader(Buffer *buf)
{
    Line *l;
    char *p, *p1, *p2;
    static char *header_mid[] = {
	"Message-Id:", "References:", "In-Reply-To:", NULL
    };
    static char *header_group[] = {
	"Newsgroups:", NULL
    };
    char **header, **q;
    int i, search = FALSE;

    if (!buf || !buf->firstLine)
	return NULL;
    for (i = 0; i <= 1; i++) {
	if (i == 0) {
	    regexCompile("<[!-;=?-~]+@[a-zA-Z0-9\\.\\-_]+>", 1);
	    header = header_mid;
	}
	else {
	    regexCompile("[a-zA-Z0-9\\.\\-_]+", 1);
	    header = header_group;
	}
	for (l = buf->firstLine; l != NULL && l->real_linenumber == 0;
	     l = l->next) {
	    if (l->bpos)
		continue;
	    p = l->lineBuf;
	    if (!IS_SPACE(*p)) {
		search = FALSE;
		for (q = header; *q; q++) {
		    if (!strncasecmp(p, *q, strlen(*q))) {
			search = TRUE;
			p = strchr(p, ':') + 1;
			break;
		    }
		}
	    }
	    if (!search)
		continue;
	    for (;;) {
		if (regexMatch(p, &l->lineBuf[l->size] - p, p == l->lineBuf)
		    == 1) {
		    matchedPosition(&p1, &p2);
		    p = reAnchorPos(buf, l, p1, p2, _put_anchor_news);
		}
		else
		    break;
	    }
	}
    }
    reseq_anchor(buf);
    return NULL;
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
    ml->marks[seq].invalid = 0;
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

void
shiftAnchorPosition(AnchorList *al, HmarkerList *hl, int line, int pos,
		    int shift)
{
    Anchor *a;
    size_t b, e, s = 0;
    int cmp;

    if (al == NULL || al->nanchor == 0)
	return;

    s = al->nanchor / 2;
    for (b = 0, e = al->nanchor - 1; b <= e; s = (b + e + 1) / 2) {
	a = &al->anchors[s];
	cmp = onAnchor(a, line, pos);
	if (cmp == 0)
	    break;
	else if (cmp > 0)
	    b = s + 1;
	else if (s == 0)
	    break;
	else
	    e = s - 1;
    }
    for (; s < al->nanchor; s++) {
	a = &al->anchors[s];
	if (a->start.line > line)
	    break;
	if (a->start.pos > pos) {
	    a->start.pos += shift;
	    if (hl->marks[a->hseq].line == line)
		hl->marks[a->hseq].pos = a->start.pos;
	}
	if (a->end.pos >= pos)
	    a->end.pos += shift;
    }
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
	    a = registerImg(buf, a_img.url, a_img.title, l->linenumber, pos);
	    a->hseq = -a_img.hseq;
	    a->slave = TRUE;
	    a->image = img;
	    a->end.pos = pos + ecol - col;
	    for (k = pos; k < a->end.pos; k++)
		l->propBuf[k] |= PE_IMAGE;
	    if (a_href.url) {
		a = registerHref(buf, a_href.url, a_href.target,
				 a_href.referer, a_href.title,
				 a_href.accesskey, l->linenumber, pos);
		a->hseq = a_href.hseq;
		a->slave = TRUE;
		a->end.pos = pos + ecol - col;
		for (k = pos; k < a->end.pos; k++)
		    l->propBuf[k] |= PE_ANCHOR;
	    }
	    if (a_form.url) {
		buf->formitem = putAnchor(buf->formitem, a_form.url,
					  a_form.target, &a, NULL, NULL, '\0',
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
				      a_form.target, &a, NULL, NULL, '\0',
				      l->linenumber, pos);
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

char *
getAnchorText(Buffer *buf, AnchorList *al, Anchor *a)
{
    int hseq, i;
    Line *l;
    Str tmp = NULL;
    char *p, *ep;

    if (!a || a->hseq < 0)
	return NULL;
    hseq = a->hseq;
    l = buf->firstLine;
    for (i = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (a->hseq != hseq)
	    continue;
	for (; l; l = l->next) {
	    if (l->linenumber == a->start.line)
		break;
	}
	if (!l)
	    break;
	p = l->lineBuf + a->start.pos;
	ep = l->lineBuf + a->end.pos;
	for (; p < ep && IS_SPACE(*p); p++) ;
	if (p == ep)
	    continue;
	if (!tmp)
	    tmp = Strnew_size(ep - p);
	else
	    Strcat_char(tmp, ' ');
	Strcat_charp_n(tmp, p, ep - p);
    }
    return tmp ? tmp->ptr : NULL;
}

Buffer *
link_list_panel(Buffer *buf)
{
    LinkList *l;
    AnchorList *al;
    Anchor *a;
    FormItemList *fi;
    int i;
    char *t, *u, *p;
    ParsedURL pu;
    /* FIXME: gettextize? */
    Str tmp = Strnew_charp("<title>Link List</title>\
<h1 align=center>Link List</h1>\n");

    if (buf->bufferprop & BP_INTERNAL ||
	(buf->linklist == NULL && buf->href == NULL && buf->img == NULL)) {
	return NULL;
    }

    if (buf->linklist) {
	Strcat_charp(tmp, "<hr><h2>Links</h2>\n<ol>\n");
	for (l = buf->linklist; l; l = l->next) {
	    if (l->url) {
		parseURL2(l->url, &pu, baseURL(buf));
		p = parsedURL2Str(&pu)->ptr;
		u = html_quote(p);
		if (DecodeURL)
		    p = html_quote(url_unquote_conv(p, buf->document_charset));
		else
		    p = u;
	    }
	    else
		u = p = "";
	    if (l->type == LINK_TYPE_REL)
		t = " [Rel]";
	    else if (l->type == LINK_TYPE_REV)
		t = " [Rev]";
	    else
		t = "";
	    t = Sprintf("%s%s\n", l->title ? l->title : "", t)->ptr;
	    t = html_quote(t);
	    Strcat_m_charp(tmp, "<li><a href=\"", u, "\">", t, "</a><br>", p,
			   "\n", NULL);
	}
	Strcat_charp(tmp, "</ol>\n");
    }

    if (buf->href) {
	Strcat_charp(tmp, "<hr><h2>Anchors</h2>\n<ol>\n");
	al = buf->href;
	for (i = 0; i < al->nanchor; i++) {
	    a = &al->anchors[i];
	    if (a->hseq < 0 || a->slave)
		continue;
	    parseURL2(a->url, &pu, baseURL(buf));
	    p = parsedURL2Str(&pu)->ptr;
	    u = html_quote(p);
	    if (DecodeURL)
		p = html_quote(url_unquote_conv(p, buf->document_charset));
	    else
		p = u;
	    t = getAnchorText(buf, al, a);
	    t = t ? html_quote(t) : "";
	    Strcat_m_charp(tmp, "<li><a href=\"", u, "\">", t, "</a><br>", p,
			   "\n", NULL);
	}
	Strcat_charp(tmp, "</ol>\n");
    }

    if (buf->img) {
	Strcat_charp(tmp, "<hr><h2>Images</h2>\n<ol>\n");
	al = buf->img;
	for (i = 0; i < al->nanchor; i++) {
	    a = &al->anchors[i];
	    if (a->slave)
		continue;
	    parseURL2(a->url, &pu, baseURL(buf));
	    p = parsedURL2Str(&pu)->ptr;
	    u = html_quote(p);
	    if (DecodeURL)
		p = html_quote(url_unquote_conv(p, buf->document_charset));
	    else
		p = u;
	    if (a->title && *a->title)
		t = html_quote(a->title);
	    else if (DecodeURL)
		t = html_quote(url_unquote_conv
			       (a->url, buf->document_charset));
	    else
		t = html_quote(a->url);
	    Strcat_m_charp(tmp, "<li><a href=\"", u, "\">", t, "</a><br>", p,
			   "\n", NULL);
	    a = retrieveAnchor(buf->formitem, a->start.line, a->start.pos);
	    if (!a)
		continue;
	    fi = (FormItemList *)a->url;
	    fi = fi->parent->item;
	    if (fi->parent->method == FORM_METHOD_INTERNAL &&
		!Strcmp_charp(fi->parent->action, "map") && fi->value) {
		MapList *ml = searchMapList(buf, fi->value->ptr);
		ListItem *mi;
		MapArea *m;
		if (!ml)
		    continue;
		Strcat_charp(tmp, "<br>\n<b>Image map</b>\n<ol>\n");
		for (mi = ml->area->first; mi != NULL; mi = mi->next) {
		    m = (MapArea *) mi->ptr;
		    if (!m)
			continue;
		    parseURL2(m->url, &pu, baseURL(buf));
		    p = parsedURL2Str(&pu)->ptr;
		    u = html_quote(p);
		    if (DecodeURL)
			p = html_quote(url_unquote_conv(p,
							buf->
							document_charset));
		    else
			p = u;
		    if (m->alt && *m->alt)
			t = html_quote(m->alt);
		    else if (DecodeURL)
			t = html_quote(url_unquote_conv(m->url,
							buf->
							document_charset));
		    else
			t = html_quote(m->url);
		    Strcat_m_charp(tmp, "<li><a href=\"", u, "\">", t,
				   "</a><br>", p, "\n", NULL);
		}
		Strcat_charp(tmp, "</ol>\n");
	    }
	}
	Strcat_charp(tmp, "</ol>\n");
    }

    return loadHTMLString(tmp);
}
