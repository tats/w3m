/* $Id: news.c,v 1.4 2002/12/27 16:31:49 ukai Exp $ */
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
news_command(News * news, char *command, int *status)
{
    Str tmp;

    if (!news->host)
	return NULL;
    if (command) {
	fprintf(news->wf, "%s\r\n", command);
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
    news_command(news, NULL, &status);
    if (status != 200 && status != 201)
	goto open_err;
    if (news->mode) {
	news_command(news, Sprintf("MODE %s", news->mode)->ptr, &status);
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
    news_command(news, "QUIT", NULL);
    news_close(news);
}

static char *
name_from_address(char *str, int n)
{
    char *s, *p;
    int i, l, space = TRUE;

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
    for (p = s, l = 0; *p; p += i) {
	i = get_mclen(get_mctype(p));
	if (IS_SPACE(*p)) {
	    if (space)
		continue;
	    space = TRUE;
	}
	else
	    space = FALSE;
	l += i;
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
		 char *mid)
{
    time_t t;
    struct tm *tm;

    name = name_from_address(name, 16);
    t = mymktime(date);
    tm = localtime(&t);
    Strcat(str,
	   Sprintf
	   ("<tr valign=top><td>%d<td nowrap>(%02d/%02d)<td nowrap>%s<td><a href=\"news:%s\">%s</a>\n",
	    index, tm->tm_mon + 1, tm->tm_mday, html_quote_s(name),
	    html_quote(file_quote(mid)), html_quote(subject)));
}

InputStream
openNewsStream(ParsedURL *pu)
{
    char *host, *mode, *group, *p;
    Str tmp;
    int port, status;

    if (pu->file == NULL || *pu->file == '\0')
	return NULL;
    if (pu->scheme == SCM_NNTP)
	host = pu->host;
    else
	host = NNTP_server;
    if (!host || *host == '\0')
	return NULL;
    if (pu->scheme != SCM_NNTP && (p = strchr(host, ':'))) {
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
	    tmp = Sprintf("MODE %s", mode ? mode : "READER");
	    tmp = news_command(&current_news, tmp->ptr, &status);
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
    if (pu->scheme == SCM_NNTP) {
	/* first char of pu->file is '/' */
	group = file_unquote(Strnew_charp(pu->file + 1)->ptr);
	p = strchr(group, '/');
	if (p == NULL)
	    return NULL;
	*p++ = '\0';
	news_command(&current_news, Sprintf("GROUP %s", group)->ptr, &status);
	if (status != 211)
	    return NULL;
	news_command(&current_news, Sprintf("ARTICLE %s", p)->ptr, &status);
	if (status != 220)
	    return NULL;
	return current_news.rf;
    }
    else if (pu->scheme == SCM_NEWS) {
	tmp = Sprintf("ARTICLE <%s>", url_unquote(pu->file));
	news_command(&current_news, tmp->ptr, &status);
	if (status != 220)
	    return NULL;
	return current_news.rf;
    }
    return NULL;
}

Str
readNewsgroup(ParsedURL *pu)
{
    volatile Str page;
    Str tmp;
    URLFile f;
    Buffer *buf;
    char *group, *p, *q, *s, *t, *n;
    char *volatile qgroup;
    int status, i, first, last;
    volatile int flag = 0, start = 0, end = 0;
#ifdef JP_CHARSET
    char code = '\0';
#endif
    MySignalHandler(*volatile trap) (SIGNAL_ARG) = NULL;

    if (current_news.host == NULL || !pu->file || *pu->file == '\0')
	return NULL;
    group = file_unquote(pu->file);
    qgroup = html_quote(group);
    page = Strnew();

    if (fmInitialized) {
	message(Sprintf("Reading newsgroup %s...", group)->ptr, 0, 0);
	refresh();
    }
    if (SETJMP(AbortLoading) != 0) {
	news_close(&current_news);
	Strcat_charp(page, "</table><p>Transfer Interrupted!\n");
	goto news_end;
    }
    trap = signal(SIGINT, KeyAbort);
    if (fmInitialized)
	term_cbreak();

    page =
	Sprintf
	("<title>Newsgroup: %s</title>\n<h1>Newsgroup:&nbsp;%s</h1>\n<hr>\n",
	 qgroup, qgroup);

    qgroup = html_quote(file_quote(group));	/* URL */
    tmp =
	news_command(&current_news, Sprintf("GROUP %s", group)->ptr, &status);
    if (status != 211)
	goto news_list;
    if (sscanf(tmp->ptr, "%d %d %d %d", &status, &i, &first, &last) != 4)
	goto news_list;
    if (pu->label) {
	start = atoi(pu->label);
	if (start > 0) {
	    if (start < first)
		start = first;
	    end = start + MaxNewsMessage;
	}
    }
    if (start <= 0) {
	start = first;
	end = last + 1;
	if (end - start > MaxNewsMessage)
	    start = end - MaxNewsMessage;
    }
    if (start > first) {
	i = start - MaxNewsMessage;
	if (i < first)
	    i = first;
	Strcat(page, Sprintf("<a href=\"news:%s#%d\">[%d-%d]</a>\n",
			     qgroup, i, i, start - 1));
    }

    Strcat_charp(page, "<table>\n");
    news_command(&current_news, Sprintf("XOVER %d-%d", start, end - 1)->ptr,
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
	    s = convertLine(&f, decodeMIME(s), &code, HEADER_MODE)->ptr;
	    n = convertLine(&f, decodeMIME(n), &code, HEADER_MODE)->ptr;
	    add_news_message(page, i, t, n, s, p);
	}
    }
    else {
	init_stream(&f, SCM_NEWS, current_news.rf);
	buf = newBuffer(INIT_BUFFER_WIDTH);
	for (i = start; i < end && i <= last; i++) {
	    news_command(&current_news, Sprintf("HEAD %d", i)->ptr, &status);
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
	    add_news_message(page, i, t, n, s, p);
	}
    }
    Strcat_charp(page, "</table>\n");

    if (end <= last) {
	i = end + MaxNewsMessage - 1;
	if (i > last)
	    i = last;
	Strcat(page, Sprintf("<a href=\"news:%s#%d\">[%d-%d]</a>\n",
			     qgroup, end, end, i));
    }
    flag = 1;

  news_list:
    news_command(&current_news, Sprintf("LIST ACTIVE %s.*", group)->ptr,
		 &status);
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
	i = 0;
	if (sscanf(q, "%d %d", &last, &first) == 2 && last >= first)
	    i = last - first + 1;
	Strcat(page,
	       Sprintf
	       ("<tr><td align=right>%d<td><a href=\"news:%s\">%s</a>\n", i,
		html_quote(file_quote(p)), html_quote(p)));
    }
    if (flag == 2)
	Strcat_charp(page, "</table>\n");

  news_end:
    if (fmInitialized)
	term_raw();
    signal(SIGINT, trap);
    return page;
}

void
disconnectNews(void)
{
    news_quit(&current_news);
}

#endif				/* USE_NNTP */
