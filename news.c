/* $Id: news.c,v 1.17 2003/10/05 18:52:51 ukai Exp $ */
#include "fm.h"
#include "myctype.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <setjmp.h>

#ifdef USE_NNTP

#define NEWS_ENDLINE(p) \
    ((*(p) == '.' && ((p)[1] == '\n' || (p)[1] == '\r' || (p)[1] == '\0')) || \
    *(p) == '\n' || *(p) == '\r' || *(p) == '\0')

typedef struct _News {
    char *host;
    int port;
    char *mode;
    InputStream rf;
    FILE *wf;
} News;

static News current_news = { NULL, 0, NULL, NULL, NULL };

static JMP_BUF AbortLoading;

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
    SIGNAL_RETURN;
}

static Str
news_command(News * news, char *cmd, char *arg, int *status)
{
    Str tmp;

    if (!news->host)
	return NULL;
    if (cmd) {
	if (arg)
	    tmp = Sprintf("%s %s\r\n", cmd, arg);
	else
	    tmp = Sprintf("%s\r\n", cmd);
	fwrite(tmp->ptr, sizeof(char), tmp->length, news->wf);
	fflush(news->wf);
    }
    if (!status)
	return NULL;
    *status = -1;
    tmp = StrISgets(news->rf);
    if (tmp->length)
	sscanf(tmp->ptr, "%d", status);
    return tmp;
}

static void
news_close(News * news)
{
    if (!news->host)
	return;
    if (news->rf) {
	IStype(news->rf) &= ~IST_UNCLOSE;
	ISclose(news->rf);
	news->rf = NULL;
    }
    if (news->wf) {
	fclose(news->wf);
	news->wf = NULL;
    }
    news->host = NULL;
}

static int
news_open(News * news)
{
    int sock, status;

    sock = openSocket(news->host, "nntp", news->port);
    if (sock < 0)
	goto open_err;
    news->rf = newInputStream(sock);
    news->wf = fdopen(dup(sock), "wb");
    if (!news->rf || !news->wf)
	goto open_err;
    IStype(news->rf) |= IST_UNCLOSE;
    news_command(news, NULL, NULL, &status);
    if (status != 200 && status != 201)
	goto open_err;
    if (news->mode) {
	news_command(news, "MODE", news->mode, &status);
	if (status != 200 && status != 201)
	    goto open_err;
    }
    return TRUE;
  open_err:
    news_close(news);
    return FALSE;
}

static void
news_quit(News * news)
{
    news_command(news, "QUIT", NULL, NULL);
    news_close(news);
}

static char *
name_from_address(char *str, int n)
{
    char *s, *p;
    int l, space = TRUE;

    s = allocStr(str, -1);
    SKIP_BLANKS(s);
    if (*s == '<' && (p = strchr(s, '>'))) {
	*p++ = '\0';
	SKIP_BLANKS(p);
	if (*p == '\0')		/* <address> */
	    s++;
	else			/* <address> name ? */
	    s = p;
    }
    else if ((p = strchr(s, '<')))	/* name <address> */
	*p = '\0';
    else if ((p = strchr(s, '(')))	/* address (name) */
	s = p;
    if (*s == '"' && (p = strchr(s + 1, '"'))) {	/* "name" */
	*p = '\0';
	s++;
    }
    else if (*s == '(' && (p = strchr(s + 1, ')'))) {	/* (name) */
	*p = '\0';
	s++;
    }
    for (p = s, l = 0; *p; p += get_mclen(p)) {
	if (IS_SPACE(*p)) {
	    if (space)
		continue;
	    space = TRUE;
	}
	else
	    space = FALSE;
	l += get_mcwidth(p);
	if (l > n)
	    break;
    }
    *p = '\0';
    return s;
}

static char *
html_quote_s(char *str)
{
    Str tmp = NULL;
    char *p, *q;
    int space = TRUE;

    for (p = str; *p; p++) {
	if (IS_SPACE(*p)) {
	    if (space)
		continue;
	    q = "&nbsp;";
	    space = TRUE;
	}
	else {
	    q = html_quote_char(*p);
	    space = FALSE;
	}
	if (q) {
	    if (tmp == NULL)
		tmp = Strnew_charp_n(str, (int)(p - str));
	    Strcat_charp(tmp, q);
	}
	else {
	    if (tmp)
		Strcat_char(tmp, *p);
	}
    }
    if (tmp)
	return tmp->ptr;
    return str;
}

static void
add_news_message(Str str, int index, char *date, char *name, char *subject,
		 char *mid, char *scheme, char *group)
{
    time_t t;
    struct tm *tm;

    name = name_from_address(name, 16);
    t = mymktime(date);
    tm = localtime(&t);
    Strcat(str,
	   Sprintf("<tr valign=top><td>%d<td nowrap>(%02d/%02d)<td nowrap>%s",
		   index, tm->tm_mon + 1, tm->tm_mday, html_quote_s(name)));
    if (group)
	Strcat(str, Sprintf("<td><a href=\"%s%s/%d\">%s</a>\n", scheme, group,
			    index, html_quote(subject)));
    else
	Strcat(str, Sprintf("<td><a href=\"%s%s\">%s</a>\n", scheme,
			    html_quote(file_quote(mid)), html_quote(subject)));
}

/*
 * [News article]
 *  * RFC 1738
 *    nntp://<host>:<port>/<newsgroup-name>/<article-number>
 *    news:<message-id>
 *
 *  * Extension
 *    nntp://<host>:<port>/<newsgroup-name>/<message-id>
 *    nntp://<host>:<port>/<message-id>
 *    news:<newsgroup-name>/<article-number>
 *    news:<newsgroup-name>/<message-id>
 *
 * [News group]
 *  * RFC 1738
 *    news:<newsgroup-name>
 *
 *  * Extension
 *    nntp://<host>:<port>/<newsgroup-name>
 *    nntp://<host>:<port>/<newsgroup-name>/<start-number>-<end-number>
 *    news:<newsgroup-name>/<start-number>-<end-number>
 *
 * <message-id> = <unique>@<full_domain_name>
 */

InputStream
openNewsStream(ParsedURL *pu)
{
    char *host, *mode, *group, *p;
    Str tmp;
    int port, status;

    if (pu->file == NULL || *pu->file == '\0')
	return NULL;
    if (pu->scheme == SCM_NNTP || pu->scheme == SCM_NNTP_GROUP)
	host = pu->host;
    else
	host = NNTP_server;
    if (!host || *host == '\0') {
	if (current_news.host)
	    news_quit(&current_news);
	return NULL;
    }
    if (pu->scheme != SCM_NNTP && pu->scheme != SCM_NNTP_GROUP &&
	(p = strchr(host, ':'))) {
	host = allocStr(host, p - host);
	port = atoi(p + 1);
    }
    else
	port = pu->port;
    if (NNTP_mode && *NNTP_mode)
	mode = NNTP_mode;
    else
	mode = NULL;
    if (current_news.host) {
	if (!strcmp(current_news.host, host) && current_news.port == port) {
	    tmp = news_command(&current_news, "MODE", mode ? mode : "READER",
			       &status);
	    if (status != 200 && status != 201)
		news_close(&current_news);
	}
	else
	    news_quit(&current_news);
    }
    if (!current_news.host) {
	current_news.host = allocStr(host, -1);
	current_news.port = port;
	current_news.mode = mode ? allocStr(mode, -1) : NULL;
	if (!news_open(&current_news))
	    return NULL;
    }
    if (pu->scheme == SCM_NNTP || pu->scheme == SCM_NEWS) {
	/* News article */
	group = file_unquote(allocStr(pu->file, -1));
	p = strchr(group, '/');
	if (p == NULL) {	/* <message-id> */
	    if (!strchr(group, '@'))
		return NULL;
	    p = group;
	}
	else {			/* <newsgroup>/<message-id or article-number> */
	    *p++ = '\0';
	    news_command(&current_news, "GROUP", group, &status);
	    if (status != 211)
		return NULL;
	}
	if (strchr(p, '@'))	/* <message-id> */
	    news_command(&current_news, "ARTICLE", Sprintf("<%s>", p)->ptr,
			 &status);
	else			/* <article-number> */
	    news_command(&current_news, "ARTICLE", p, &status);
	if (status != 220)
	    return NULL;
	return current_news.rf;
    }
    return NULL;
}


#ifdef USE_M17N
Str
loadNewsgroup(ParsedURL *pu, wc_ces * charset)
#else
Str
loadNewsgroup0(ParsedURL *pu)
#endif
{
    volatile Str page;
    Str tmp;
    URLFile f;
    Buffer *buf;
    char *qgroup, *p, *q, *s, *t, *n;
    char *volatile scheme, *volatile group, *volatile list;
    int status, i, first, last;
    volatile int flag = 0, start = 0, end = 0;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
#ifdef USE_M17N
    wc_ces doc_charset = DocumentCharset, mime_charset;

    *charset = WC_CES_US_ASCII;
#endif
    if (current_news.host == NULL || !pu->file || *pu->file == '\0')
	return NULL;
    group = allocStr(pu->file, -1);
    if (pu->scheme == SCM_NNTP_GROUP)
	scheme = "/";
    else
	scheme = "news:";
    if ((list = strchr(group, '/'))) {
	/* <newsgroup>/<start-number>-<end-number> */
	*list++ = '\0';
    }
    if (fmInitialized) {
	message(Sprintf("Reading newsgroup %s...", group)->ptr, 0, 0);
	refresh();
    }
    qgroup = html_quote(group);
    group = file_unquote(group);
    page = Strnew_m_charp("<html>\n<head>\n<base href=\"",
			  parsedURL2Str(pu)->ptr, "\">\n<title>Newsgroup: ",
			  qgroup, "</title>\n</head>\n<body>\n<h1>Newsgroup: ",
			  qgroup, "</h1>\n<hr>\n", NULL);

    if (SETJMP(AbortLoading) != 0) {
	news_close(&current_news);
	Strcat_charp(page, "</table>\n<p>Transfer Interrupted!\n");
	goto news_end;
    }
    TRAP_ON;

    tmp = news_command(&current_news, "GROUP", group, &status);
    if (status != 211)
	goto news_list;
    if (sscanf(tmp->ptr, "%d %d %d %d", &status, &i, &first, &last) != 4)
	goto news_list;
    if (list && *list) {
	if ((p = strchr(list, '-'))) {
	    *p++ = '\0';
	    end = atoi(p);
	}
	start = atoi(list);
	if (start > 0) {
	    if (start < first)
		start = first;
	    if (end <= 0)
		end = start + MaxNewsMessage - 1;
	}
    }
    if (start <= 0) {
	start = first;
	end = last;
	if (end - start > MaxNewsMessage - 1)
	    start = end - MaxNewsMessage + 1;
    }
    page = Sprintf("<html>\n<head>\n<base href=\"%s\">\n\
<title>Newsgroup: %s %d-%d</title>\n\
</head>\n<body>\n<h1>Newsgroup: %s %d-%d</h1>\n<hr>\n", parsedURL2Str(pu)->ptr, qgroup, start, end, qgroup, start, end);
    if (start > first) {
	i = start - MaxNewsMessage;
	if (i < first)
	    i = first;
	Strcat(page, Sprintf("<a href=\"%s%s/%d-%d\">[%d-%d]</a>\n", scheme,
			     qgroup, i, start - 1, i, start - 1));
    }

    Strcat_charp(page, "<table>\n");
    news_command(&current_news, "XOVER", Sprintf("%d-%d", start, end)->ptr,
		 &status);
    if (status == 224) {
	f.scheme = SCM_NEWS;
	while (1) {
	    tmp = StrISgets(current_news.rf);
	    if (NEWS_ENDLINE(tmp->ptr))
		break;
	    if (sscanf(tmp->ptr, "%d", &i) != 1)
		continue;
	    if (!(s = strchr(tmp->ptr, '\t')))
		continue;
	    s++;
	    if (!(n = strchr(s, '\t')))
		continue;
	    *n++ = '\0';
	    if (!(t = strchr(n, '\t')))
		continue;
	    *t++ = '\0';
	    if (!(p = strchr(t, '\t')))
		continue;
	    *p++ = '\0';
	    if (*p == '<')
		p++;
	    if (!(q = strchr(p, '>')) && !(q = strchr(p, '\t')))
		continue;
	    *q = '\0';
	    tmp = decodeMIME(Strnew_charp(s), &mime_charset);
	    s = convertLine(&f, tmp, HEADER_MODE,
			    mime_charset ? &mime_charset : charset,
			    mime_charset ? mime_charset : doc_charset)->ptr;
	    tmp = decodeMIME(Strnew_charp(n), &mime_charset);
	    n = convertLine(&f, tmp, HEADER_MODE,
			    mime_charset ? &mime_charset : charset,
			    mime_charset ? mime_charset : doc_charset)->ptr;
	    add_news_message(page, i, t, n, s, p, scheme,
			     pu->scheme == SCM_NNTP_GROUP ? qgroup : NULL);
	}
    }
    else {
	init_stream(&f, SCM_NEWS, current_news.rf);
	buf = newBuffer(INIT_BUFFER_WIDTH);
	for (i = start; i <= end && i <= last; i++) {
	    news_command(&current_news, "HEAD", Sprintf("%d", i)->ptr,
			 &status);
	    if (status != 221)
		continue;
	    readHeader(&f, buf, FALSE, NULL);
	    if (!(p = checkHeader(buf, "Message-ID:")))
		continue;
	    if (*p == '<')
		p++;
	    if (!(q = strchr(p, '>')) && !(q = strchr(p, '\t')))
		*q = '\0';
	    if (!(s = checkHeader(buf, "Subject:")))
		continue;
	    if (!(n = checkHeader(buf, "From:")))
		continue;
	    if (!(t = checkHeader(buf, "Date:")))
		continue;
	    add_news_message(page, i, t, n, s, p, scheme,
			     pu->scheme == SCM_NNTP_GROUP ? qgroup : NULL);
	}
    }
    Strcat_charp(page, "</table>\n");

    if (end < last) {
	i = end + MaxNewsMessage;
	if (i > last)
	    i = last;
	Strcat(page, Sprintf("<a href=\"%s%s/%d-%d\">[%d-%d]</a>\n", scheme,
			     qgroup, end + 1, i, end + 1, i));
    }
    flag = 1;

  news_list:
    tmp = Sprintf("ACTIVE %s", group);
    if (!strchr(group, '*'))
	Strcat_charp(tmp, ".*");
    news_command(&current_news, "LIST", tmp->ptr, &status);
    if (status != 215)
	goto news_end;
    while (1) {
	tmp = StrISgets(current_news.rf);
	if (NEWS_ENDLINE(tmp->ptr))
	    break;
	if (flag < 2) {
	    if (flag == 1)
		Strcat_charp(page, "<hr>\n");
	    Strcat_charp(page, "<table>\n");
	    flag = 2;
	}
	p = tmp->ptr;
	for (q = p; *q && !IS_SPACE(*q); q++) ;
	*(q++) = '\0';
	if (sscanf(q, "%d %d", &last, &first) == 2 && last >= first)
	    i = last - first + 1;
	else
	    i = 0;
	Strcat(page,
	       Sprintf
	       ("<tr><td align=right>%d<td><a href=\"%s%s\">%s</a>\n", i,
		scheme, html_quote(file_quote(p)), html_quote(p)));
    }
    if (flag == 2)
	Strcat_charp(page, "</table>\n");

  news_end:
    Strcat_charp(page, "</body>\n</html>\n");
    TRAP_OFF;
    return page;
}

void
closeNews(void)
{
    news_close(&current_news);
}

void
disconnectNews(void)
{
    news_quit(&current_news);
}

#endif				/* USE_NNTP */
