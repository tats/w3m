/* 
 * client-side image maps
 */
#include "fm.h"

#ifdef MENU_MAP
char *
follow_map_menu(Buffer * buf, struct parsed_tagarg *arg, int x, int y)
{
    MapList *ml;
    char *name;
    TextListItem *t, *s;
    int i, n, selected = -1;
    char **label;

    name = tag_get_value(arg, "link");
    if (name == NULL)
	return NULL;

    for (ml = buf->maplist; ml != NULL; ml = ml->next) {
	if (!Strcmp_charp(ml->name, name))
	    break;
    }
    if (ml == NULL)
	return NULL;

    for (n = 0, t = ml->urls->first; t != NULL; n++, t = t->next);
    if (n == 0)
	return NULL;
    label = New_N(char *, n + 1);
    for (i = 0, t = ml->urls->first, s = ml->alts->first; t != NULL;
	i++, t = t->next, s = s->next)
	label[i] = *s->ptr ? s->ptr : t->ptr;
    label[n] = NULL;

    optionMenu(x, y, label, &selected, 0, NULL);

    if (selected >= 0) {
	for (i = 0, t = ml->urls->first; t != NULL; i++, t = t->next)
	    if (i == selected)
		return t->ptr;
    }
    return NULL;
}

#else
char *map1 = "<HTML><HEAD><TITLE>Image map links</TITLE></HEAD>\
<BODY><H1>Image map links</H1>";

Buffer *
follow_map_panel(Buffer * buf, struct parsed_tagarg *arg)
{
    Str mappage;
    MapList *ml;
    char *name;
    TextListItem *t, *s;
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
    for (t = ml->urls->first, s = ml->alts->first; t != NULL;
	t = t->next, s = s->next) {
	parseURL2(t->ptr, &pu, baseURL(buf));
	Strcat_charp(mappage, "<a href=\"");
	Strcat_charp(mappage, html_quote(parsedURL2Str(&pu)->ptr));
	Strcat_charp(mappage, "\">");
	Strcat_charp(mappage, html_quote(s->ptr));
	Strcat_charp(mappage, " ");
	Strcat_charp(mappage, html_quote(t->ptr));
	Strcat_charp(mappage, "</a><br>\n");
    }
    Strcat_charp(mappage, "</body></html>");

    return loadHTMLString(mappage);
}
#endif

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
page_info_panel(Buffer * buf)
{
    Str tmp = Strnew_size(1024);
    Anchor *a;
    Str s;
    ParsedURL pu;
    TextListItem *ti;
    struct frameset *f_set = NULL;
    int all;

    Strcat_charp(tmp, "<html><head><title>Information about current page</title></head><body>");
    Strcat_charp(tmp, "<h1>Information about current page</h1><table cellpadding=0>");
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
	s = Strnew_charp(form2str((FormItemList *) a->url));
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
	     buf->nextBuffer != NULL &&
	     buf->nextBuffer->frameset != NULL)
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
