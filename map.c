/* $Id: map.c,v 1.15 2002/11/19 17:48:00 ukai Exp $ */
/*
 * client-side image maps
 */
#include "fm.h"
#include <math.h>

static MapList *
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

Str
getCurrentMapLabel(Buffer *buf)
{
    Anchor *a_img, *a_form;
    FormItemList *fi;
    MapList *ml;
    ListItem *al;
    MapArea *a;
    int i, n;
    Str s;

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
	if (!(a && i == n))
	    continue;
	s = Sprintf("[%s]", a->alt);
	if (*a->alt) {
	    ParsedURL pu;
	    parseURL2(a->url, &pu, baseURL(buf));
	    Strcat_char(s, ' ');
	    Strcat(s, parsedURL2Str(&pu));
	}
	return s;
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

MapArea *
follow_map_menu(Buffer *buf, char *name, Anchor *a_img, int x, int y)
{
    MapList *ml;
    ListItem *al;
    MapArea *a;
    int i, selected = -1, initial = 0;
#ifdef MENU_MAP
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

  map_end:
    if (selected >= 0) {
	for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	    if (al->ptr && i == selected)
		return (MapArea *) al->ptr;
	}
    }
    return NULL;
}

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
    char *url;

    ml = searchMapList(buf, name);
    if (ml == NULL)
	return NULL;

    mappage = Strnew_charp(map1);
    for (al = ml->area->first; al != NULL; al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	parseURL2(a->url, &pu, baseURL(buf));
	url = html_quote(parsedURL2Str(&pu)->ptr);
	Strcat_m_charp(mappage, "<tr><td>", html_quote(a->alt),
		       "<td><a href=\"", url, "\">", url, "</a>\n", NULL);
    }
    Strcat_charp(mappage, "</table></body></html>");

    return loadHTMLString(mappage);
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
    char *url;

    ml = searchMapList(buf, fi->value ? fi->value->ptr : NULL);
    if (ml == NULL)
	return;

    Strcat_charp(tmp, "<tr><td colspan=2>Links of current image map");
    Strcat_charp(tmp, "<tr><td colspan=2><table>");
    for (al = ml->area->first; al != NULL; al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	parseURL2(a->url, &pu, baseURL(buf));
	url = html_quote(parsedURL2Str(&pu)->ptr);
	Strcat_m_charp(tmp, "<tr><td>&nbsp;&nbsp;<td>",
		       html_quote(a->alt), "<td><a href=\"", url, "\">", url,
		       "</a>\n", NULL);
    }
    Strcat_charp(tmp, "</table>");
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
		for (j = 0; j < level; j++)
		    Strcat_charp(html, "   ");
		q = html_quote(frame.body->url);
		Strcat_charp(html, "<a href=\"");
		Strcat_charp(html, q);
		Strcat_charp(html, "\">");
		if (frame.body->name) {
		    p = file_unquote(frame.body->name);
#ifdef JP_CHARSET
		    p = conv(p, buf->document_code, InnerCode)->ptr;
#endif
		    p = html_quote(p);
		    Strcat_charp(html, p);
		}
		Strcat_charp(html, " ");
		Strcat_charp(html, q);
		Strcat_charp(html, "</a>\n");
#ifdef USE_SSL
		if (frame.body->ssl_certificate)
		    Strcat_m_charp(html, "<blockquote><pre>\n",
				   frame.body->ssl_certificate,
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
    Str s;
    ParsedURL pu;
    TextListItem *ti;
    struct frameset *f_set = NULL;
    int all;

    Strcat_charp(tmp,
		 "<html><head><title>Information about current page</title></head><body>");
    Strcat_charp(tmp,
		 "<h1>Information about current page</h1><table cellpadding=0>");
    if (buf == NULL)
	goto end;
    Strcat_charp(tmp, "<tr><td nowrap>Title<td>");
    Strcat_charp(tmp, html_quote(buf->buffername));
    Strcat_charp(tmp, "<tr><td nowrap>Current URL<td>");
    Strcat_charp(tmp, html_quote(parsedURL2Str(&buf->currentURL)->ptr));
    Strcat_charp(tmp, "<tr><td nowrap>Document Type<td>");
    if (buf->real_type)
	Strcat_charp(tmp, buf->real_type);
    else
	Strcat_charp(tmp, "unknown");
    Strcat_charp(tmp, "<tr><td nowrap>Last Modified<td>");
    Strcat_charp(tmp, html_quote(last_modified(buf)));
#ifdef JP_CHARSET
    Strcat_charp(tmp, "<tr><td nowrap>Document Code<td>");
    Strcat_charp(tmp, code_to_str(buf->document_code));
#endif				/* JP_CHARSET */
    Strcat_charp(tmp, "<tr><td nowrap>Number of lines<td>");
    all = buf->allLine;
    if (all == 0 && buf->lastLine)
	all = buf->lastLine->linenumber;
    Strcat(tmp, Sprintf("%d", all));
    Strcat_charp(tmp, "<tr><td nowrap>Transferred bytes<td>");
    Strcat(tmp, Sprintf("%d", buf->trbyte));

    a = retrieveCurrentAnchor(buf);
    if (a != NULL) {
	char *aurl;
	parseURL2(a->url, &pu, baseURL(buf));
	s = parsedURL2Str(&pu);
	aurl = html_quote(s->ptr);
	Strcat_charp(tmp, "<tr><td nowrap>URL of current anchor<td>");
	Strcat_m_charp(tmp, "<a href=\"", aurl, "\">", aurl, "</a>", NULL);
    }
    a = retrieveCurrentImg(buf);
    if (a != NULL) {
	parseURL2(a->url, &pu, baseURL(buf));
	s = parsedURL2Str(&pu);
	Strcat_charp(tmp, "<tr><td nowrap>URL of current image<td>");
	Strcat_charp(tmp, "<a href=\"");
	Strcat_charp(tmp, html_quote(s->ptr));
	Strcat_charp(tmp, "\">");
	Strcat_charp(tmp, html_quote(s->ptr));
	Strcat_charp(tmp, "</a>");
    }
    a = retrieveCurrentForm(buf);
    if (a != NULL) {
	FormItemList *fi = (FormItemList *)a->url;
	s = Strnew_charp(form2str(fi));
	Strcat_charp(tmp, "<tr><td nowrap>Method/type of current form<td>");
	Strcat_charp(tmp, html_quote(s->ptr));
	if (fi->parent->method == FORM_METHOD_INTERNAL &&
	    !Strcmp_charp(fi->parent->action, "map"))
	    append_map_info(buf, tmp, fi->parent->item);
    }
    Strcat_charp(tmp, "</table>\n");
    if (buf->document_header != NULL) {
	Strcat_charp(tmp, "<hr width=50%>\n");
	Strcat_charp(tmp, "<h1>Header information</h1>\n");
	for (ti = buf->document_header->first; ti != NULL; ti = ti->next) {
	    Strcat_charp(tmp, html_quote(ti->ptr));
	    Strcat_charp(tmp, "<br>");
	}
    }
    if (buf->frameset != NULL)
	f_set = buf->frameset;
    else if (buf->bufferprop & BP_FRAME &&
	     buf->nextBuffer != NULL && buf->nextBuffer->frameset != NULL)
	f_set = buf->nextBuffer->frameset;

    if (f_set) {
	Strcat_charp(tmp, "<hr width=50%><h1>Frame information</h1><pre>");
	append_frame_info(buf, tmp, f_set, 0);
	Strcat_charp(tmp, "</pre>");
    }
#ifdef USE_SSL
    if (buf->ssl_certificate == NULL)
	goto end;
    Strcat_charp(tmp, "<h1>SSL certificate</h1>\n");
    Strcat_charp(tmp, "<pre>\n");
    Strcat_charp(tmp, buf->ssl_certificate);
    Strcat_charp(tmp, "</pre>\n");
#endif
  end:
    Strcat_charp(tmp, "</body></html>");
    return loadHTMLString(tmp);
}
