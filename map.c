/* $Id: map.c,v 1.30 2003/09/24 18:49:00 ukai Exp $ */
/*
 * client-side image maps
 */
#include "fm.h"
#include <math.h>

MapList *
searchMapList(Buffer *buf, char *name)
{
    MapList *ml;

    if (name == NULL)
	return NULL;
    for (ml = buf->maplist; ml != NULL; ml = ml->next) {
	if (!Strcmp_charp(ml->name, name))
	    break;
    }
    return ml;
}

#ifdef USE_IMAGE
static int
inMapArea(MapArea * a, int x, int y)
{
    int i;
    double r1, r2, s, c, t;

    if (!a)
	return FALSE;
    switch (a->shape) {
    case SHAPE_RECT:
	if (x >= a->coords[0] && y >= a->coords[1] &&
	    x <= a->coords[2] && y <= a->coords[3])
	    return TRUE;
	break;
    case SHAPE_CIRCLE:
	if ((x - a->coords[0]) * (x - a->coords[0])
	    + (y - a->coords[1]) * (y - a->coords[1])
	    <= a->coords[2] * a->coords[2])
	    return TRUE;
	break;
    case SHAPE_POLY:
	for (t = 0, i = 0; i < a->ncoords; i += 2) {
	    r1 = sqrt((double)(x - a->coords[i]) * (x - a->coords[i])
		      + (double)(y - a->coords[i + 1]) * (y -
							  a->coords[i + 1]));
	    r2 = sqrt((double)(x - a->coords[i + 2]) * (x - a->coords[i + 2])
		      + (double)(y - a->coords[i + 3]) * (y -
							  a->coords[i + 3]));
	    if (r1 == 0 || r2 == 0)
		return TRUE;
	    s = ((double)(x - a->coords[i]) * (y - a->coords[i + 3])
		 - (double)(x - a->coords[i + 2]) * (y -
						     a->coords[i +
							       1])) / r1 / r2;
	    c = ((double)(x - a->coords[i]) * (x - a->coords[i + 2])
		 + (double)(y - a->coords[i + 1]) * (y -
						     a->coords[i +
							       3])) / r1 / r2;
	    t += atan2(s, c);
	}
	if (fabs(t) > 2 * 3.14)
	    return TRUE;
	break;
    case SHAPE_DEFAULT:
	return TRUE;
    default:
	break;
    }
    return FALSE;
}

static int
nearestMapArea(MapList *ml, int x, int y)
{
    ListItem *al;
    MapArea *a;
    int i, l, n = -1, min = -1, limit = pixel_per_char * pixel_per_char
	+ pixel_per_line * pixel_per_line;

    if (!ml || !ml->area)
	return n;
    for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	a = (MapArea *) al->ptr;
	if (a) {
	    l = (a->center_x - x) * (a->center_x - x)
		+ (a->center_y - y) * (a->center_y - y);
	    if ((min < 0 || l < min) && l < limit) {
		n = i;
		min = l;
	    }
	}
    }
    return n;
}

static int
searchMapArea(Buffer *buf, MapList *ml, Anchor *a_img)
{
    ListItem *al;
    MapArea *a;
    int i, n;
    int px, py;

    if (!(ml && ml->area && ml->area->nitem))
	return -1;
    if (!getMapXY(buf, a_img, &px, &py))
	return -1;
    n = -ml->area->nitem;
    for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	if (n < 0 && inMapArea(a, px, py)) {
	    if (a->shape == SHAPE_DEFAULT) {
		if (n == -ml->area->nitem)
		    n = -i;
	    }
	    else
		n = i;
	}
    }
    if (n == -ml->area->nitem)
	return nearestMapArea(ml, px, py);
    else if (n < 0)
	return -n;
    return n;
}

MapArea *
retrieveCurrentMapArea(Buffer *buf)
{
    Anchor *a_img, *a_form;
    FormItemList *fi;
    MapList *ml;
    ListItem *al;
    MapArea *a;
    int i, n;

    a_img = retrieveCurrentImg(buf);
    if (!(a_img && a_img->image && a_img->image->map))
	return NULL;
    a_form = retrieveCurrentForm(buf);
    if (!(a_form && a_form->url))
	return NULL;
    fi = (FormItemList *)a_form->url;
    if (!(fi && fi->parent && fi->parent->item))
	return NULL;
    fi = fi->parent->item;
    ml = searchMapList(buf, fi->value ? fi->value->ptr : NULL);
    if (!ml)
	return NULL;
    n = searchMapArea(buf, ml, a_img);
    if (n < 0)
	return NULL;
    for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	a = (MapArea *) al->ptr;
	if (a && i == n)
	    return a;
    }
    return NULL;
}

int
getMapXY(Buffer *buf, Anchor *a, int *x, int *y)
{
    if (!buf || !a || !a->image || !x || !y)
	return 0;
    *x = (int)((buf->currentColumn + buf->cursorX
		- COLPOS(buf->currentLine, a->start.pos) + 0.5)
	       * pixel_per_char) - a->image->xoffset;
    *y = (int)((buf->currentLine->linenumber - a->image->y + 0.5)
	       * pixel_per_line) - a->image->yoffset;
    if (*x <= 0)
	*x = 1;
    if (*y <= 0)
	*y = 1;
    return 1;
}
#endif

Anchor *
retrieveCurrentMap(Buffer *buf)
{
    Anchor *a;
    FormItemList *fi;

    a = retrieveCurrentForm(buf);
    if (!a || !a->url)
	return NULL;
    fi = (FormItemList *)a->url;
    if (fi->parent->method == FORM_METHOD_INTERNAL &&
	!Strcmp_charp(fi->parent->action, "map"))
	return a;
    return NULL;
}

#if defined(USE_IMAGE) || defined(MENU_MAP)
MapArea *
follow_map_menu(Buffer *buf, char *name, Anchor *a_img, int x, int y)
{
    MapList *ml;
    ListItem *al;
    int i, selected = -1;
    int initial = 0;
#ifdef MENU_MAP
    MapArea *a;
    char **label;
#endif

    ml = searchMapList(buf, name);
    if (ml == NULL || ml->area == NULL || ml->area->nitem == 0)
	return NULL;

#ifdef USE_IMAGE
    initial = searchMapArea(buf, ml, a_img);
    if (initial < 0)
	initial = 0;
    else if (!image_map_list) {
	selected = initial;
	goto map_end;
    }
#endif

#ifdef MENU_MAP
    label = New_N(char *, ml->area->nitem + 1);
    for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	a = (MapArea *) al->ptr;
	if (a)
	    label[i] = *a->alt ? a->alt : a->url;
	else
	    label[i] = "";
    }
    label[ml->area->nitem] = NULL;

    optionMenu(x, y, label, &selected, initial, NULL);
#endif

#ifdef USE_IMAGE
  map_end:
#endif
    if (selected >= 0) {
	for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	    if (al->ptr && i == selected)
		return (MapArea *) al->ptr;
	}
    }
    return NULL;
}
#endif

#ifndef MENU_MAP
char *map1 = "<HTML><HEAD><TITLE>Image map links</TITLE></HEAD>\
<BODY><H1>Image map links</H1>\
<table>";

Buffer *
follow_map_panel(Buffer *buf, char *name)
{
    Str mappage;
    MapList *ml;
    ListItem *al;
    MapArea *a;
    ParsedURL pu;
    char *p, *q;
    Buffer *newbuf;

    ml = searchMapList(buf, name);
    if (ml == NULL)
	return NULL;

    mappage = Strnew_charp(map1);
    for (al = ml->area->first; al != NULL; al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	parseURL2(a->url, &pu, baseURL(buf));
	p = parsedURL2Str(&pu)->ptr;
	q = html_quote(p);
	if (DecodeURL)
	    p = html_quote(url_unquote_conv(p, buf->document_charset));
	else
	    p = q;
	Strcat_m_charp(mappage, "<tr valign=top><td><a href=\"", q, "\">",
		       html_quote(*a->alt ? a->alt : mybasename(a->url)),
		       "</a><td>", p, NULL);
    }
    Strcat_charp(mappage, "</table></body></html>");

    newbuf = loadHTMLString(mappage);
#ifdef USE_M17N
    if (newbuf)
	newbuf->document_charset = buf->document_charset;
#endif
    return newbuf;
}
#endif

MapArea *
newMapArea(char *url, char *target, char *alt, char *shape, char *coords)
{
    MapArea *a = New(MapArea);
#ifdef USE_IMAGE
    char *p;
    int i, max;
#endif

    a->url = url;
    a->target = target;
    a->alt = alt ? alt : "";
#ifdef USE_IMAGE
    a->shape = SHAPE_RECT;
    if (shape) {
	if (!strcasecmp(shape, "default"))
	    a->shape = SHAPE_DEFAULT;
	else if (!strncasecmp(shape, "rect", 4))
	    a->shape = SHAPE_RECT;
	else if (!strncasecmp(shape, "circ", 4))
	    a->shape = SHAPE_CIRCLE;
	else if (!strncasecmp(shape, "poly", 4))
	    a->shape = SHAPE_POLY;
	else
	    a->shape = SHAPE_UNKNOWN;
    }
    a->coords = NULL;
    a->ncoords = 0;
    a->center_x = 0;
    a->center_y = 0;
    if (a->shape == SHAPE_UNKNOWN || a->shape == SHAPE_DEFAULT)
	return a;
    if (!coords) {
	a->shape = SHAPE_UNKNOWN;
	return a;
    }
    if (a->shape == SHAPE_RECT) {
	a->coords = New_N(short, 4);
	a->ncoords = 4;
    }
    else if (a->shape == SHAPE_CIRCLE) {
	a->coords = New_N(short, 3);
	a->ncoords = 3;
    }
    max = a->ncoords;
    for (i = 0, p = coords; (a->shape == SHAPE_POLY || i < a->ncoords) && *p;) {
	while (IS_SPACE(*p))
	    p++;
	if (!IS_DIGIT(*p) && *p != '-' && *p != '+')
	    break;
	if (a->shape == SHAPE_POLY) {
	    if (max <= i) {
		max = i ? i * 2 : 6;
		a->coords = New_Reuse(short, a->coords, max + 2);
	    }
	    a->ncoords++;
	}
	a->coords[i] = (short)atoi(p);
	i++;
	if (*p == '-' || *p == '+')
	    p++;
	while (IS_DIGIT(*p))
	    p++;
	if (*p != ',' && !IS_SPACE(*p))
	    break;
	while (IS_SPACE(*p))
	    p++;
	if (*p == ',')
	    p++;
    }
    if (i != a->ncoords || (a->shape == SHAPE_POLY && a->ncoords < 6)) {
	a->shape = SHAPE_UNKNOWN;
	a->coords = NULL;
	a->ncoords = 0;
	return a;
    }
    if (a->shape == SHAPE_POLY) {
	a->ncoords = a->ncoords / 2 * 2;
	a->coords[a->ncoords] = a->coords[0];
	a->coords[a->ncoords + 1] = a->coords[1];
    }
    if (a->shape == SHAPE_CIRCLE) {
	a->center_x = a->coords[0];
	a->center_y = a->coords[1];
    }
    else {
	for (i = 0; i < a->ncoords / 2; i++) {
	    a->center_x += a->coords[2 * i];
	    a->center_y += a->coords[2 * i + 1];
	}
	a->center_x /= a->ncoords / 2;
	a->center_y /= a->ncoords / 2;
    }
#endif
    return a;
}

/* append image map links */
static void
append_map_info(Buffer *buf, Str tmp, FormItemList *fi)
{
    MapList *ml;
    ListItem *al;
    MapArea *a;
    ParsedURL pu;
    char *p, *q;

    ml = searchMapList(buf, fi->value ? fi->value->ptr : NULL);
    if (ml == NULL)
	return;

    Strcat_m_charp(tmp,
		   "<tr valign=top><td colspan=2>Links of current image map",
		   "<tr valign=top><td colspan=2><table>", NULL);
    for (al = ml->area->first; al != NULL; al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	parseURL2(a->url, &pu, baseURL(buf));
	q = html_quote(parsedURL2Str(&pu)->ptr);
	if (DecodeURL)
	    p = html_quote(url_unquote_conv(a->url, buf->document_charset));
	else
	    p = html_quote(a->url);
	Strcat_m_charp(tmp, "<tr valign=top><td>&nbsp;&nbsp;<td><a href=\"",
		       q, "\">",
		       html_quote(*a->alt ? a->alt : mybasename(a->url)),
		       "</a><td>", p, "\n", NULL);
    }
    Strcat_charp(tmp, "</table>");
}

/* append links */
static void
append_link_info(Buffer *buf, Str html, LinkList * link)
{
    LinkList *l;
    ParsedURL pu;
    char *url;

    if (!link)
	return;

    Strcat_charp(html, "<hr width=50%><h1>Link information</h1><table>\n");
    for (l = link; l; l = l->next) {
	if (l->url) {
	    parseURL2(l->url, &pu, baseURL(buf));
	    url = html_quote(parsedURL2Str(&pu)->ptr);
	}
	else
	    url = "(empty)";
	Strcat_m_charp(html, "<tr valign=top><td><a href=\"", url, "\">",
		       l->title ? html_quote(l->title) : "(empty)", "</a><td>",
		       NULL);
	if (l->type == LINK_TYPE_REL)
	    Strcat_charp(html, "[Rel]");
	else if (l->type == LINK_TYPE_REV)
	    Strcat_charp(html, "[Rev]");
	if (!l->url)
	    url = "(empty)";
	else if (DecodeURL)
	    url = html_quote(url_unquote_conv(l->url, buf->document_charset));
	else
	    url = html_quote(l->url);
	Strcat_m_charp(html, "<td>", url, NULL);
	if (l->ctype)
	    Strcat_m_charp(html, " (", html_quote(l->ctype), ")", NULL);
	Strcat_charp(html, "\n");
    }
    Strcat_charp(html, "</table>\n");
}

/* append frame URL */
static void
append_frame_info(Buffer *buf, Str html, struct frameset *set, int level)
{
    char *p, *q;
    int i, j;

    if (!set)
	return;

    for (i = 0; i < set->col * set->row; i++) {
	union frameset_element frame = set->frame[i];
	if (frame.element != NULL) {
	    switch (frame.element->attr) {
	    case F_UNLOADED:
	    case F_BODY:
		if (frame.body->url == NULL)
		    break;
		Strcat_charp(html, "<pre_int>");
		for (j = 0; j < level; j++)
		    Strcat_charp(html, "   ");
		q = html_quote(frame.body->url);
		Strcat_m_charp(html, "<a href=\"", q, "\">", NULL);
		if (frame.body->name) {
		    p = html_quote(url_unquote_conv(frame.body->name,
						    buf->document_charset));
		    Strcat_charp(html, p);
		}
		if (DecodeURL)
		    p = html_quote(url_unquote_conv(frame.body->url,
						    buf->document_charset));
		else
		    p = q;
		Strcat_m_charp(html, " ", p, "</a></pre_int><br>\n", NULL);
#ifdef USE_SSL
		if (frame.body->ssl_certificate)
		    Strcat_m_charp(html,
				   "<blockquote><h2>SSL certificate</h2><pre>\n",
				   html_quote(frame.body->ssl_certificate),
				   "</pre></blockquote>\n", NULL);
#endif
		break;
	    case F_FRAMESET:
		append_frame_info(buf, html, frame.set, level + 1);
		break;
	    }
	}
    }
}

/* 
 * information of current page and link 
 */
Buffer *
page_info_panel(Buffer *buf)
{
    Str tmp = Strnew_size(1024);
    Anchor *a;
    ParsedURL pu;
    TextListItem *ti;
    struct frameset *f_set = NULL;
    int all;
    char *p, *q;
#ifdef USE_M17N
    wc_ces_list *list;
    char charset[16];
#endif
    Buffer *newbuf;

    Strcat_charp(tmp, "<html><head>\
<title>Information about current page</title>\
</head><body>\
<h1>Information about current page</h1>\n");
    if (buf == NULL)
	goto end;
    all = buf->allLine;
    if (all == 0 && buf->lastLine)
	all = buf->lastLine->linenumber;
#ifdef USE_M17N
    Strcat_charp(tmp, "<form method=internal action=charset>");
#endif
    p = parsedURL2Str(&buf->currentURL)->ptr;
    if (DecodeURL)
	p = url_unquote_conv(p, 0);
    Strcat_m_charp(tmp, "<table cellpadding=0>",
		   "<tr valign=top><td nowrap>Title<td>",
		   html_quote(buf->buffername),
		   "<tr valign=top><td nowrap>Current URL<td>",
		   html_quote(p),
		   "<tr valign=top><td nowrap>Document Type<td>",
		   buf->real_type ? html_quote(buf->real_type) : "unknown",
		   "<tr valign=top><td nowrap>Last Modified<td>",
		   html_quote(last_modified(buf)), NULL);
#ifdef USE_M17N
    if (buf->document_charset != InnerCharset) {
	list = wc_get_ces_list();
	Strcat_charp(tmp,
		     "<tr><td nowrap>Document Charset<td><select name=charset>");
	for (; list->name != NULL; list++) {
	    sprintf(charset, "%d", (unsigned int)list->id);
	    Strcat_m_charp(tmp, "<option value=", charset,
			   (buf->document_charset == list->id) ? " selected>"
			   : ">", list->desc, NULL);
	}
	Strcat_charp(tmp, "</select>");
	Strcat_charp(tmp, "<tr><td><td><input type=submit value=Change>");
    }
#endif
    Strcat_m_charp(tmp,
		   "<tr valign=top><td nowrap>Number of lines<td>",
		   Sprintf("%d", all)->ptr,
		   "<tr valign=top><td nowrap>Transferred bytes<td>",
		   Sprintf("%d", buf->trbyte)->ptr, NULL);

    a = retrieveCurrentAnchor(buf);
    if (a != NULL) {
	parseURL2(a->url, &pu, baseURL(buf));
	p = parsedURL2Str(&pu)->ptr;
	q = html_quote(p);
	if (DecodeURL)
	    p = html_quote(url_unquote_conv(p, buf->document_charset));
	else
	    p = q;
	Strcat_m_charp(tmp,
		       "<tr valign=top><td nowrap>URL of current anchor<td><a href=\"",
		       q, "\">", p, "</a>", NULL);
    }
    a = retrieveCurrentImg(buf);
    if (a != NULL) {
	parseURL2(a->url, &pu, baseURL(buf));
	p = parsedURL2Str(&pu)->ptr;
	q = html_quote(p);
	if (DecodeURL)
	    p = html_quote(url_unquote_conv(p, buf->document_charset));
	else
	    p = q;
	Strcat_m_charp(tmp,
		       "<tr valign=top><td nowrap>URL of current image<td><a href=\"",
		       q, "\">", p, "</a>", NULL);
    }
    a = retrieveCurrentForm(buf);
    if (a != NULL) {
	FormItemList *fi = (FormItemList *)a->url;
	p = form2str(fi);
	if (DecodeURL)
	    p = html_quote(url_unquote_conv(p, buf->document_charset));
	else
	    p = html_quote(p);
	Strcat_m_charp(tmp,
		       "<tr valign=top><td nowrap>Method/type of current form&nbsp;<td>",
		       p, NULL);
	if (fi->parent->method == FORM_METHOD_INTERNAL
	    && !Strcmp_charp(fi->parent->action, "map"))
	    append_map_info(buf, tmp, fi->parent->item);
    }
    Strcat_charp(tmp, "</table>\n");
#ifdef USE_M17N
    Strcat_charp(tmp, "</form>");
#endif

    append_link_info(buf, tmp, buf->linklist);

    if (buf->document_header != NULL) {
	Strcat_charp(tmp, "<hr width=50%><h1>Header information</h1><pre>\n");
	for (ti = buf->document_header->first; ti != NULL; ti = ti->next)
	    Strcat_m_charp(tmp, "<pre_int>", html_quote(ti->ptr),
			   "</pre_int>\n", NULL);
	Strcat_charp(tmp, "</pre>\n");
    }

    if (buf->frameset != NULL)
	f_set = buf->frameset;
    else if (buf->bufferprop & BP_FRAME &&
	     buf->nextBuffer != NULL && buf->nextBuffer->frameset != NULL)
	f_set = buf->nextBuffer->frameset;

    if (f_set) {
	Strcat_charp(tmp, "<hr width=50%><h1>Frame information</h1>\n");
	append_frame_info(buf, tmp, f_set, 0);
    }
#ifdef USE_SSL
    if (buf->ssl_certificate)
	Strcat_m_charp(tmp, "<h1>SSL certificate</h1><pre>\n",
		       html_quote(buf->ssl_certificate), "</pre>\n", NULL);
#endif
  end:
    Strcat_charp(tmp, "</body></html>");
    newbuf = loadHTMLString(tmp);
#ifdef USE_M17N
    if (newbuf)
	newbuf->document_charset = buf->document_charset;
#endif
    return newbuf;
}
