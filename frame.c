/* $Id: frame.c,v 1.34 2003/09/26 17:59:51 ukai Exp $ */
#include "fm.h"
#include "parsetagx.h"
#include "myctype.h"
#include <signal.h>
#include <setjmp.h>

static JMP_BUF AbortLoading;
struct frameset *renderFrameSet = NULL;

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
}

static int
parseFrameSetLength(char *s, char ***ret)
{
    int i, len;
    char *p, *q, **lv;

    i = 1;

    if (s)
	for (p = s; (p = strchr(p, ',')); ++p)
	    ++i;
    else
	s = "*";

    lv = New_N(char *, i);

    for (i = 0, p = s;; ++p) {
	SKIP_BLANKS(p);
	len = strtol(p, &q, 10);

	switch (*q) {
	case '%':
	    lv[i++] = Sprintf("%d%%", len)->ptr;
	    break;
	case '*':
	    lv[i++] = "*";
	    break;
	default:
	    lv[i++] = Sprintf("%d", len)->ptr;
	    break;
	}

	if (!(p = strchr(q, ',')))
	    break;
    }

    *ret = lv;
    return i;
}

struct frameset *
newFrameSet(struct parsed_tag *tag)
{
    struct frameset *f;
    int i;
    char *cols = NULL, *rows = NULL;

    f = New(struct frameset);
    f->attr = F_FRAMESET;
    f->name = NULL;
    f->currentURL = NULL;
    parsedtag_get_value(tag, ATTR_COLS, &cols);
    parsedtag_get_value(tag, ATTR_ROWS, &rows);
    f->col = parseFrameSetLength(cols, &f->width);
    f->row = parseFrameSetLength(rows, &f->height);
    f->i = 0;
    i = f->row * f->col;
    f->frame = New_N(union frameset_element, i);
    do {
	f->frame[--i].element = NULL;
    } while (i);
    return f;
}

struct frame_body *
newFrame(struct parsed_tag *tag, Buffer *buf)
{
    struct frame_body *body;
    char *p;

    body = New(struct frame_body);
    bzero((void *)body, sizeof(*body));
    body->attr = F_UNLOADED;
    body->flags = 0;
    body->baseURL = baseURL(buf);
    if (tag) {
	if (parsedtag_get_value(tag, ATTR_SRC, &p))
	    body->url = url_quote_conv(remove_space(p), buf->document_charset);
	if (parsedtag_get_value(tag, ATTR_NAME, &p) && *p != '_')
	    body->name = url_quote_conv(p, buf->document_charset);
    }
    return body;
}

static void
unloadFrame(struct frame_body *b)
{
    b->attr = F_UNLOADED;
}

void
deleteFrame(struct frame_body *b)
{
    if (b == NULL)
	return;
    unloadFrame(b);
    bzero((void *)b, sizeof(*b));
}

void
addFrameSetElement(struct frameset *f, union frameset_element element)
{
    int i;

    if (f == NULL)
	return;
    i = f->i;
    if (i >= f->col * f->row)
	return;
    f->frame[i] = element;
    f->i++;
}

void
deleteFrameSet(struct frameset *f)
{
    int i;

    if (f == NULL)
	return;
    for (i = 0; i < f->col * f->row; i++) {
	deleteFrameSetElement(f->frame[i]);
    }
    f->name = NULL;
    f->currentURL = NULL;
    return;
}

void
deleteFrameSetElement(union frameset_element e)
{
    if (e.element == NULL)
	return;
    switch (e.element->attr) {
    case F_UNLOADED:
	break;
    case F_BODY:
	deleteFrame(e.body);
	break;
    case F_FRAMESET:
	deleteFrameSet(e.set);
	break;
    default:
	break;
    }
    return;
}

static struct frame_body *
copyFrame(struct frame_body *ob)
{
    struct frame_body *rb;

    rb = New(struct frame_body);
    bcopy((const void *)ob, (void *)rb, sizeof(struct frame_body));
    return rb;
}

struct frameset *
copyFrameSet(struct frameset *of)
{
    struct frameset *rf;
    int n;

    rf = New(struct frameset);
    n = of->col * of->row;
    bcopy((const void *)of, (void *)rf, sizeof(struct frameset));
    rf->width = New_N(char *, rf->col);
    bcopy((const void *)of->width,
	  (void *)rf->width, sizeof(char *) * rf->col);
    rf->height = New_N(char *, rf->row);
    bcopy((const void *)of->height,
	  (void *)rf->height, sizeof(char *) * rf->row);
    rf->frame = New_N(union frameset_element, n);
    while (n) {
	n--;
	if (!of->frame[n].element)
	    goto attr_default;
	switch (of->frame[n].element->attr) {
	case F_UNLOADED:
	case F_BODY:
	    rf->frame[n].body = copyFrame(of->frame[n].body);
	    break;
	case F_FRAMESET:
	    rf->frame[n].set = copyFrameSet(of->frame[n].set);
	    break;
	default:
	  attr_default:
	    rf->frame[n].element = NULL;
	    break;
	}
    }
    return rf;
}

void
flushFrameSet(struct frameset *fs)
{
    int n = fs->i;

    while (n) {
	n--;
	if (!fs->frame[n].element)
	    goto attr_default;
	switch (fs->frame[n].element->attr) {
	case F_UNLOADED:
	case F_BODY:
	    fs->frame[n].body->nameList = NULL;
	    break;
	case F_FRAMESET:
	    flushFrameSet(fs->frame[n].set);
	    break;
	default:
	  attr_default:
	    /* nothing to do */
	    break;
	}
    }
}

void
pushFrameTree(struct frameset_queue **fqpp, struct frameset *fs, Buffer *buf)
{
    struct frameset_queue *rfq, *cfq = *fqpp;

    if (!fs)
	return;

    rfq = New(struct frameset_queue);
    rfq->linenumber = (buf
		       && buf->currentLine) ? buf->currentLine->linenumber : 1;
    rfq->top_linenumber = (buf && buf->topLine) ? buf->topLine->linenumber : 1;
    rfq->pos = buf ? buf->pos : 0;
    rfq->currentColumn = buf ? buf->currentColumn : 0;
    rfq->formitem = buf ? buf->formitem : NULL;

    rfq->back = cfq;
    if (cfq) {
	rfq->next = cfq->next;
	if (cfq->next)
	    cfq->next->back = rfq;
	cfq->next = rfq;
    }
    else
	rfq->next = cfq;
    rfq->frameset = fs;
    *fqpp = rfq;
    return;
}

struct frameset *
popFrameTree(struct frameset_queue **fqpp)
{
    struct frameset_queue *rfq = NULL, *cfq = *fqpp;
    struct frameset *rfs = NULL;

    if (!cfq)
	return rfs;

    rfs = cfq->frameset;
    if (cfq->next) {
	(rfq = cfq->next)->back = cfq->back;
    }
    if (cfq->back) {
	(rfq = cfq->back)->next = cfq->next;
    }
    *fqpp = rfq;
    bzero((void *)cfq, sizeof(struct frameset_queue));
    return rfs;
}

void
resetFrameElement(union frameset_element *f_element,
		  Buffer *buf, char *referer, FormList *request)
{
    char *f_name;
    struct frame_body *f_body;

    f_name = f_element->element->name;
    if (buf->frameset) {
	/* frame cascade */
	deleteFrameSetElement(*f_element);
	f_element->set = buf->frameset;
	f_element->set->currentURL = New(ParsedURL);
	copyParsedURL(f_element->set->currentURL, &buf->currentURL);
	buf->frameset = popFrameTree(&(buf->frameQ));
	f_element->set->name = f_name;
    }
    else {
	f_body = newFrame(NULL, buf);
	f_body->attr = F_BODY;
	f_body->name = f_name;
	f_body->url = parsedURL2Str(&buf->currentURL)->ptr;
	f_body->source = buf->sourcefile;
	buf->sourcefile = NULL;
	if (buf->mailcap_source) {
	    f_body->source = buf->mailcap_source;
	    buf->mailcap_source = NULL;
	}
	f_body->type = buf->type;
	f_body->referer = referer;
	f_body->request = request;
	deleteFrameSetElement(*f_element);
	f_element->body = f_body;
    }
}

static struct frameset *
frame_download_source(struct frame_body *b, ParsedURL *currentURL,
		      ParsedURL *baseURL, int flag)
{
    Buffer *buf;
    struct frameset *ret_frameset = NULL;
    ParsedURL url;

    if (b == NULL || b->url == NULL || b->url[0] == '\0')
	return NULL;
    if (b->baseURL)
	baseURL = b->baseURL;
    parseURL2(b->url, &url, currentURL);
    switch (url.scheme) {
    case SCM_LOCAL:
#if 0
	b->source = url.real_file;
#endif
	b->flags = 0;
    default:
	is_redisplay = TRUE;
	w3m_dump |= DUMP_FRAME;
	buf = loadGeneralFile(b->url,
			      baseURL ? baseURL : currentURL,
			      b->referer, flag | RG_FRAME_SRC, b->request);
#ifdef USE_SSL
	/* XXX certificate? */
	if (buf && buf != NO_BUFFER)
	    b->ssl_certificate = buf->ssl_certificate;
#endif
	w3m_dump &= ~DUMP_FRAME;
	is_redisplay = FALSE;
	break;
    }

    if (buf == NULL || buf == NO_BUFFER) {
	b->source = NULL;
	b->flags = (buf == NO_BUFFER) ? FB_NO_BUFFER : 0;
	return NULL;
    }
    b->url = parsedURL2Str(&buf->currentURL)->ptr;
    b->type = buf->type;
    b->source = buf->sourcefile;
    buf->sourcefile = NULL;
    if (buf->mailcap_source) {
	b->source = buf->mailcap_source;
	buf->mailcap_source = NULL;
    }
    b->attr = F_BODY;
    if (buf->frameset) {
	ret_frameset = buf->frameset;
	ret_frameset->name = b->name;
	ret_frameset->currentURL = New(ParsedURL);
	copyParsedURL(ret_frameset->currentURL, &buf->currentURL);
	buf->frameset = popFrameTree(&(buf->frameQ));
    }
    discardBuffer(buf);
    return ret_frameset;
}

#define CASE_TABLE_TAG \
	case HTML_TR:\
	case HTML_N_TR:\
	case HTML_TD:\
	case HTML_N_TD:\
	case HTML_TH:\
	case HTML_N_TH:\
	case HTML_THEAD:\
	case HTML_N_THEAD:\
	case HTML_TBODY:\
	case HTML_N_TBODY:\
	case HTML_TFOOT:\
	case HTML_N_TFOOT:\
	case HTML_COLGROUP:\
	case HTML_N_COLGROUP:\
	case HTML_COL

static int
createFrameFile(struct frameset *f, FILE * f1, Buffer *current, int level,
		int force_reload)
{
    int r, c, t_stack;
    URLFile f2;
#ifdef USE_M17N
    wc_ces charset, doc_charset;
#endif
    char *d_target, *p_target, *s_target, *t_target;
    ParsedURL *currentURL, base;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
    int flag;

    if (f == NULL)
	return -1;

    if (level == 0) {
	if (SETJMP(AbortLoading) != 0) {
	    TRAP_OFF;
	    return -1;
	}
	TRAP_ON;
	f->name = "_top";
    }

    if (level > 7) {
	fputs("Too many frameset tasked.\n", f1);
	return -1;
    }

    if (level == 0) {
	fprintf(f1, "<html><head><title>%s</title></head><body>\n",
		html_quote(current->buffername));
	fputs("<table hborder width=\"100%\">\n", f1);
    }
    else
	fputs("<table hborder>\n", f1);

    currentURL = f->currentURL ? f->currentURL : &current->currentURL;
    for (r = 0; r < f->row; r++) {
	fputs("<tr valign=top>\n", f1);
	for (c = 0; c < f->col; c++) {
	    union frameset_element frame;
	    struct frameset *f_frameset;
	    int i = c + r * f->col;
	    char *p = "";
	    int status = R_ST_NORMAL;
	    Str tok = Strnew();
	    int pre_mode = 0;
	    int end_tag = 0;

	    frame = f->frame[i];

	    if (frame.element == NULL) {
		fputs("<td>\n</td>\n", f1);
		continue;
	    }

	    fputs("<td", f1);
	    if (frame.element->name)
		fprintf(f1, " id=\"_%s\"", html_quote(frame.element->name));
	    if (!r)
		fprintf(f1, " width=\"%s\"", f->width[c]);
	    fputs(">\n", f1);

	    flag = 0;
	    if (force_reload) {
		flag |= RG_NOCACHE;
		if (frame.element->attr == F_BODY)
		    unloadFrame(frame.body);
	    }
	    switch (frame.element->attr) {
	    default:
		/* FIXME: gettextize? */
		fprintf(f1, "Frameset \"%s\" frame %d: type unrecognized",
			html_quote(f->name), i + 1);
		break;
	    case F_UNLOADED:
		if (!frame.body->name && f->name) {
		    frame.body->name = Sprintf("%s_%d", f->name, i)->ptr;
		}
		fflush(f1);
		f_frameset = frame_download_source(frame.body,
						   currentURL,
						   current->baseURL, flag);
		if (f_frameset) {
		    deleteFrame(frame.body);
		    f->frame[i].set = frame.set = f_frameset;
		    goto render_frameset;
		}
		/* fall through */
	    case F_BODY:
		init_stream(&f2, SCM_LOCAL, NULL);
		if (frame.body->source) {
		    fflush(f1);
		    examineFile(frame.body->source, &f2);
		}
		if (f2.stream == NULL) {
		    frame.body->attr = F_UNLOADED;
		    if (frame.body->flags & FB_NO_BUFFER)
			/* FIXME: gettextize? */
			fprintf(f1, "Open %s with other method",
				html_quote(frame.body->url));
		    else if (frame.body->url)
			/* FIXME: gettextize? */
			fprintf(f1, "Can't open %s",
				html_quote(frame.body->url));
		    else
			/* FIXME: gettextize? */
			fprintf(f1,
				"This frame (%s) contains no src attribute",
				frame.body->name ? html_quote(frame.body->name)
				: "(no name)");
		    break;
		}
		parseURL2(frame.body->url, &base, currentURL);
		p_target = f->name;
		s_target = frame.body->name;
		t_target = "_blank";
		d_target = TargetSelf ? s_target : t_target;
#ifdef USE_M17N
		charset = WC_CES_US_ASCII;
		if (current->document_charset != WC_CES_US_ASCII)
		    doc_charset = current->document_charset;
		else
		    doc_charset = DocumentCharset;
#endif
		t_stack = 0;
		if (frame.body->type &&
		    !strcasecmp(frame.body->type, "text/plain")) {
		    Str tmp;
		    fprintf(f1, "<pre>\n");
		    while ((tmp = StrmyUFgets(&f2))->length) {
			tmp = convertLine(NULL, tmp, HTML_MODE, &charset,
					  doc_charset);
			fprintf(f1, "%s", html_quote(tmp->ptr));
		    }
		    fprintf(f1, "</pre>\n");
		    UFclose(&f2);
		    break;
		}
		do {
		    int is_tag = FALSE;
		    char *q;
		    struct parsed_tag *tag;

		    do {
			if (*p == '\0') {
			    Str tmp = StrmyUFgets(&f2);
			    if (tmp->length == 0)
				break;
			    tmp = convertLine(NULL, tmp, HTML_MODE, &charset,
					      doc_charset);
			    p = tmp->ptr;
			}
			read_token(tok, &p, &status, 1, status != R_ST_NORMAL);
		    } while (status != R_ST_NORMAL);

		    if (tok->length == 0)
			continue;

		    if (tok->ptr[0] == '<') {
			if (tok->ptr[1] &&
			    REALLY_THE_BEGINNING_OF_A_TAG(tok->ptr))
			    is_tag = TRUE;
			else if (!(pre_mode & (RB_PLAIN | RB_INTXTA |
					       RB_SCRIPT | RB_STYLE))) {
			    p = Strnew_m_charp(tok->ptr + 1, p, NULL)->ptr;
			    tok = Strnew_charp("&lt;");
			}
		    }
		    if (is_tag) {
			if (pre_mode & (RB_PLAIN | RB_INTXTA | RB_SCRIPT |
					RB_STYLE)) {
			    q = tok->ptr;
			    if ((tag = parse_tag(&q, FALSE)) &&
				tag->tagid == end_tag) {
				if (pre_mode & RB_PLAIN) {
				    fputs("</PRE_PLAIN>", f1);
				    pre_mode = 0;
				    end_tag = 0;
				    goto token_end;
				}
				pre_mode = 0;
				end_tag = 0;
				goto proc_normal;
			    }
			    if (strncmp(tok->ptr, "<!--", 4) &&
				(q = strchr(tok->ptr + 1, '<'))) {
				tok = Strnew_charp_n(tok->ptr, q - tok->ptr);
				p = Strnew_m_charp(q, p, NULL)->ptr;
				status = R_ST_NORMAL;
			    }
			    is_tag = FALSE;
			}
			else if (pre_mode & RB_INSELECT) {
			    q = tok->ptr;
			    if ((tag = parse_tag(&q, FALSE))) {
				if ((tag->tagid == end_tag) ||
				    (tag->tagid == HTML_N_FORM)) {
				    if (tag->tagid == HTML_N_FORM)
					fputs("</SELECT>", f1);
				    pre_mode = 0;
				    end_tag = 0;
				    goto proc_normal;
				}
				if (t_stack) {
				    switch (tag->tagid) {
				    case HTML_TABLE:
				    case HTML_N_TABLE:
				      CASE_TABLE_TAG:
					fputs("</SELECT>", f1);
					pre_mode = 0;
					end_tag = 0;
					goto proc_normal;
				    }
				}
			    }
			}
		    }

		  proc_normal:
		    if (is_tag) {
			char *q = tok->ptr;
			int j, a_target = 0;
			ParsedURL url;

			if (!(tag = parse_tag(&q, FALSE)))
			    goto token_end;

			switch (tag->tagid) {
			case HTML_TITLE:
			    fputs("<!-- title:", f1);
			    goto token_end;
			case HTML_N_TITLE:
			    fputs("-->", f1);
			    goto token_end;
			case HTML_BASE:
			    /* "BASE" is prohibit tag */
			    if (parsedtag_get_value(tag, ATTR_HREF, &q)) {
				q = url_quote_conv(remove_space(q), charset);
				parseURL(q, &base, NULL);
			    }
			    if (parsedtag_get_value(tag, ATTR_TARGET, &q)) {
				if (!strcasecmp(q, "_self"))
				    d_target = s_target;
				else if (!strcasecmp(q, "_parent"))
				    d_target = p_target;
				else
				    d_target = url_quote_conv(q, charset);
			    }
			    Strshrinkfirst(tok, 1);
			    Strshrink(tok, 1);
			    fprintf(f1, "<!-- %s -->", html_quote(tok->ptr));
			    goto token_end;
			case HTML_META:
			    if (parsedtag_get_value(tag, ATTR_HTTP_EQUIV, &q)
				&& !strcasecmp(q, "refresh")) {
				if (parsedtag_get_value(tag, ATTR_CONTENT, &q)
				    ) {
				    Str s_tmp = NULL;
				    int refresh_interval =
					getMetaRefreshParam(q, &s_tmp);
				    if (s_tmp) {
					q = html_quote(s_tmp->ptr);
					fprintf(f1,
						"Refresh (%d sec) <a href=\"%s\">%s</a>\n",
						refresh_interval, q, q);
				    }
				}
			    }
#ifdef USE_M17N
			    if (UseContentCharset &&
				parsedtag_get_value(tag, ATTR_HTTP_EQUIV, &q)
				&& !strcasecmp(q, "Content-Type")
				&& parsedtag_get_value(tag, ATTR_CONTENT, &q)
				&& (q = strcasestr(q, "charset")) != NULL) {
				q += 7;
				SKIP_BLANKS(q);
				if (*q == '=') {
				    wc_ces c;
				    q++;
				    SKIP_BLANKS(q);
				    if ((c = wc_guess_charset(q, 0)) != 0) {
					doc_charset = c;
					charset = WC_CES_US_ASCII;
				    }
				}
			    }
#endif
			    /* fall thru, "META" is prohibit tag */
			case HTML_HEAD:
			case HTML_N_HEAD:
			case HTML_BODY:
			case HTML_N_BODY:
			case HTML_DOCTYPE:
			    /* prohibit_tags */
			    Strshrinkfirst(tok, 1);
			    Strshrink(tok, 1);
			    fprintf(f1, "<!-- %s -->", html_quote(tok->ptr));
			    goto token_end;
			case HTML_TABLE:
			    t_stack++;
			    break;
			case HTML_N_TABLE:
			    t_stack--;
			    if (t_stack < 0) {
				t_stack = 0;
				Strshrinkfirst(tok, 1);
				Strshrink(tok, 1);
				fprintf(f1,
					"<!-- table stack underflow: %s -->",
					html_quote(tok->ptr));
				goto token_end;
			    }
			    break;
			  CASE_TABLE_TAG:
			    /* table_tags MUST be in table stack */
			    if (!t_stack) {
				Strshrinkfirst(tok, 1);
				Strshrink(tok, 1);
				fprintf(f1, "<!-- %s -->",
					html_quote(tok->ptr));
				goto token_end;

			    }
			    break;
			case HTML_SELECT:
			    pre_mode = RB_INSELECT;
			    end_tag = HTML_N_SELECT;
			    break;
			case HTML_TEXTAREA:
			    pre_mode = RB_INTXTA;
			    end_tag = HTML_N_TEXTAREA;
			    break;
			case HTML_SCRIPT:
			    pre_mode = RB_SCRIPT;
			    end_tag = HTML_N_SCRIPT;
			    break;
			case HTML_STYLE:
			    pre_mode = RB_STYLE;
			    end_tag = HTML_N_STYLE;
			    break;
			case HTML_LISTING:
			    pre_mode = RB_PLAIN;
			    end_tag = HTML_N_LISTING;
			    fputs("<PRE_PLAIN>", f1);
			    goto token_end;
			case HTML_XMP:
			    pre_mode = RB_PLAIN;
			    end_tag = HTML_N_XMP;
			    fputs("<PRE_PLAIN>", f1);
			    goto token_end;
			case HTML_PLAINTEXT:
			    pre_mode = RB_PLAIN;
			    end_tag = MAX_HTMLTAG;
			    fputs("<PRE_PLAIN>", f1);
			    goto token_end;
			default:
			    break;
			}
			for (j = 0; j < TagMAP[tag->tagid].max_attribute; j++) {
			    switch (tag->attrid[j]) {
			    case ATTR_SRC:
			    case ATTR_HREF:
			    case ATTR_ACTION:
				if (!tag->value[j])
				    break;
				tag->value[j] =
				    url_quote_conv(remove_space(tag->value[j]),
						   charset);
				tag->need_reconstruct = TRUE;
				parseURL2(tag->value[j], &url, &base);
				if (url.scheme == SCM_UNKNOWN ||
#ifndef USE_W3MMAILER
				    url.scheme == SCM_MAILTO ||
#endif
				    url.scheme == SCM_MISSING)
				    break;
				a_target |= 1;
				tag->value[j] = parsedURL2Str(&url)->ptr;
				parsedtag_set_value(tag,
						    ATTR_REFERER,
						    parsedURL2Str(&base)->ptr);
#ifdef USE_M17N
				if (tag->attrid[j] == ATTR_ACTION &&
				    charset != WC_CES_US_ASCII)
				    parsedtag_set_value(tag,
							ATTR_CHARSET,
							wc_ces_to_charset
							(charset));
#endif
				break;
			    case ATTR_TARGET:
				if (!tag->value[j])
				    break;
				a_target |= 2;
				if (!strcasecmp(tag->value[j], "_self")) {
				    parsedtag_set_value(tag,
							ATTR_TARGET, s_target);
				}
				else if (!strcasecmp(tag->value[j], "_parent")) {
				    parsedtag_set_value(tag,
							ATTR_TARGET, p_target);
				}
				break;
			    case ATTR_NAME:
			    case ATTR_ID:
				if (!tag->value[j])
				    break;
				parsedtag_set_value(tag,
						    ATTR_FRAMENAME, s_target);
				break;
			    }
			}
			if (a_target == 1) {
			    /* there is HREF attribute and no TARGET
			     * attribute */
			    parsedtag_set_value(tag, ATTR_TARGET, d_target);
			}
			if (parsedtag_need_reconstruct(tag))
			    tok = parsedtag2str(tag);
			Strfputs(tok, f1);
		    }
		    else {
			if (pre_mode & RB_PLAIN)
			    fprintf(f1, "%s", html_quote(tok->ptr));
			else if (pre_mode & RB_INTXTA)
			    fprintf(f1, "%s",
				    html_quote(html_unquote(tok->ptr)));
			else
			    Strfputs(tok, f1);
		    }
		  token_end:
		    Strclear(tok);
		} while (*p != '\0' || !iseos(f2.stream));
		if (pre_mode & RB_PLAIN)
		    fputs("</PRE_PLAIN>\n", f1);
		else if (pre_mode & RB_INTXTA)
		    fputs("</TEXTAREA></FORM>\n", f1);
		else if (pre_mode & RB_INSELECT)
		    fputs("</SELECT></FORM>\n", f1);
		else if (pre_mode & (RB_SCRIPT | RB_STYLE)) {
		    if (status != R_ST_NORMAL)
			fputs(correct_irrtag(status)->ptr, f1);
		    if (pre_mode & RB_SCRIPT)
			fputs("</SCRIPT>\n", f1);
		    else if (pre_mode & RB_STYLE)
			fputs("</STYLE>\n", f1);
		}
		while (t_stack--)
		    fputs("</TABLE>\n", f1);
		UFclose(&f2);
		break;
	    case F_FRAMESET:
	      render_frameset:
		if (!frame.set->name && f->name) {
		    frame.set->name = Sprintf("%s_%d", f->name, i)->ptr;
		}
		createFrameFile(frame.set, f1, current, level + 1,
				force_reload);
		break;
	    }
	    fputs("</td>\n", f1);
	}
	fputs("</tr>\n", f1);
    }

    fputs("</table>\n", f1);
    if (level == 0) {
	fputs("</body></html>\n", f1);
	TRAP_OFF;
    }
    return 0;
}

Buffer *
renderFrame(Buffer *Cbuf, int force_reload)
{
    Str tmp;
    FILE *f;
    Buffer *buf;
    int flag;
    struct frameset *fset;
#ifdef USE_M17N
    wc_ces doc_charset = DocumentCharset;
#endif

    tmp = tmpfname(TMPF_FRAME, ".html");
    f = fopen(tmp->ptr, "w");
    if (f == NULL)
	return NULL;
    /* 
     * if (Cbuf->frameQ != NULL) fset = Cbuf->frameQ->frameset; else */
    fset = Cbuf->frameset;
    if (fset == NULL || createFrameFile(fset, f, Cbuf, 0, force_reload) < 0)
	return NULL;
    fclose(f);
    flag = RG_FRAME;
    if ((Cbuf->currentURL).is_nocache)
	flag |= RG_NOCACHE;
    renderFrameSet = Cbuf->frameset;
    flushFrameSet(renderFrameSet);
#ifdef USE_M17N
    DocumentCharset = InnerCharset;
#endif
    buf = loadGeneralFile(tmp->ptr, NULL, NULL, flag, NULL);
#ifdef USE_M17N
    DocumentCharset = doc_charset;
#endif
    renderFrameSet = NULL;
    if (buf == NULL || buf == NO_BUFFER)
	return NULL;
    buf->sourcefile = tmp->ptr;
#ifdef USE_M17N
    buf->document_charset = Cbuf->document_charset;
#endif
    copyParsedURL(&buf->currentURL, &Cbuf->currentURL);
    preFormUpdateBuffer(buf);
    return buf;
}

union frameset_element *
search_frame(struct frameset *fset, char *name)
{
    int i;
    union frameset_element *e = NULL;

    for (i = 0; i < fset->col * fset->row; i++) {
	e = &(fset->frame[i]);
	if (e->element != NULL) {
	    if (e->element->name && !strcmp(e->element->name, name)) {
		return e;
	    }
	    else if (e->element->attr == F_FRAMESET &&
		     (e = search_frame(e->set, name))) {
		return e;
	    }
	}
    }
    return NULL;
}
