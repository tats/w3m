/* $Id: map.c,v 1.6 2002/01/31 18:28:24 ukai Exp $ */
/*
 * client-side image maps
 */
#include "fm.h"
#include <math.h>

#ifdef MENU_MAP
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
#endif

char *
follow_map_menu(Buffer *buf, struct parsed_tagarg *arg, Anchor *a_img, int x,
		int y)
{
    MapList *ml;
    ListItem *al;
    MapArea *a;
    char *name;
    int i, n, selected = -1, initial;
    char **label;
#ifdef USE_IMAGE
    int px, py, map = 0;
#endif

    name = tag_get_value(arg, "link");
    if (name == NULL)
	return NULL;

    for (ml = buf->maplist; ml != NULL; ml = ml->next) {
	if (!Strcmp_charp(ml->name, name))
	    break;
    }
    if (ml == NULL || ml->area == NULL)
	return NULL;

    n = ml->area->nitem;
    if (n == 0)
	return NULL;
    label = New_N(char *, n + 1);
#ifdef USE_IMAGE
    if (getMapXY(buf, a_img, &px, &py))
	map = 1;
#endif
    initial = -n;
    for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	a = (MapArea *) al->ptr;
	if (a) {
	    label[i] = *a->alt ? a->alt : a->url;
#ifdef USE_IMAGE
	    if (initial < 0 && map && inMapArea(a, px, py)) {
		if (a->shape == SHAPE_DEFAULT) {
		    if (initial == -n)
			initial = -i;
		}
		else
		    initial = i;
	    }
#endif
	}
	else
	    label[i] = "";
    }
    label[n] = NULL;
    if (initial == -n)
	initial = 0;
    else if (initial < 0)
	initial *= -1;

    optionMenu(x, y, label, &selected, initial, NULL);

    if (selected >= 0) {
	for (i = 0, al = ml->area->first; al != NULL; i++, al = al->next) {
	    if (al->ptr && i == selected)
		return ((MapArea *) al->ptr)->url;
	}
    }
    return NULL;
}

#else
char *map1 = "<HTML><HEAD><TITLE>Image map links</TITLE></HEAD>\
<BODY><H1>Image map links</H1>";

Buffer *
follow_map_panel(Buffer *buf, struct parsed_tagarg *arg)
{
    Str mappage;
    MapList *ml;
    ListItem *al;
    MapArea *a;
    char *name;
    ParsedURL pu;

    name = tag_get_value(arg, "link");
    if (name == NULL)
	return NULL;

    for (ml = buf->maplist; ml != NULL; ml = ml->next) {
	if (!Strcmp_charp(ml->name, name))
	    break;
    }
    if (ml == NULL)
	return NULL;

    mappage = Strnew_charp(map1);
    for (al = ml->area->first; al != NULL; al = al->next) {
	a = (MapArea *) al->ptr;
	if (!a)
	    continue;
	parseURL2(a->url, &pu, baseURL(buf));
	Strcat_charp(mappage, "<a href=\"");
	Strcat_charp(mappage, html_quote(parsedURL2Str(&pu)->ptr));
	Strcat_charp(mappage, "\">");
	Strcat_charp(mappage, html_quote(a->alt));
	Strcat_charp(mappage, " ");
	Strcat_charp(mappage, html_quote(a->url));
	Strcat_charp(mappage, "</a><br>\n");
    }
    Strcat_charp(mappage, "</body></html>");

    return loadHTMLString(mappage);
}
#endif

#ifdef USE_IMAGE
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

MapArea *
newMapArea(char *url, char *alt, char *shape, char *coords)
{
    MapArea *a = New(MapArea);
#ifdef MENU_MAP
#ifdef USE_IMAGE
    char *p;
    int i, max;
#endif
#endif

    a->url = url;
    a->alt = alt ? alt : "";
#ifdef MENU_MAP
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
	if (!IS_DIGIT(*p))
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
    }
    if (a->shape == SHAPE_POLY) {
	a->ncoords = a->ncoords / 2 * 2;
	a->coords[a->ncoords] = a->coords[0];
	a->coords[a->ncoords + 1] = a->coords[1];
    }
#endif
#endif
    return a;
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
    Strcat_charp(tmp, "<tr><td nowrap>Number of line<td>");
    all = buf->allLine;
    if (all == 0 && buf->lastLine)
	all = buf->lastLine->linenumber;
    Strcat(tmp, Sprintf("%d", all));
    Strcat_charp(tmp, "<tr><td nowrap>Transferred byte<td>");
    Strcat(tmp, Sprintf("%d", buf->trbyte));

    a = retrieveCurrentAnchor(buf);
    if (a != NULL) {
	parseURL2(a->url, &pu, baseURL(buf));
	s = parsedURL2Str(&pu);
	Strcat_charp(tmp, "<tr><td nowrap>URL of current anchor<td>");
	Strcat_charp(tmp, html_quote(s->ptr));
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
	s = Strnew_charp(form2str((FormItemList *)a->url));
	Strcat_charp(tmp, "<tr><td nowrap>Method/type of current form<td>");
	Strcat_charp(tmp, html_quote(s->ptr));
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
