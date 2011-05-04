/* $Id: search.c,v 1.31 2004/03/23 16:44:02 ukai Exp $ */
#include "fm.h"
#include "regex.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>

static void
set_mark(Line *l, int pos, int epos)
{
    for (; pos < epos && pos < l->size; pos++)
	l->propBuf[pos] |= PE_MARK;
}

#ifdef USE_MIGEMO
/* Migemo: romaji --> kana+kanji in regexp */
static FILE *migemor = NULL, *migemow = NULL;
static int migemo_running;
static int migemo_pid = 0;

void
init_migemo()
{
    migemo_active = migemo_running = use_migemo;
    if (migemor != NULL)
	fclose(migemor);
    if (migemow != NULL)
	fclose(migemow);
    migemor = migemow = NULL;
    if (migemo_pid)
	kill(migemo_pid, SIGKILL);
    migemo_pid = 0;
}

static int
open_migemo(char *migemo_command)
{
    migemo_pid = open_pipe_rw(&migemor, &migemow);
    if (migemo_pid < 0)
	goto err0;
    if (migemo_pid == 0) {
	/* child */
	setup_child(FALSE, 2, -1);
	myExec(migemo_command);
	/* XXX: ifdef __EMX__, use start /f ? */
    }
    return 1;
  err0:
    migemo_pid = 0;
    migemo_active = migemo_running = 0;
    return 0;
}

static char *
migemostr(char *str)
{
    Str tmp = NULL;
    if (migemor == NULL || migemow == NULL)
	if (open_migemo(migemo_command) == 0)
	    return str;
    fprintf(migemow, "%s\n", conv_to_system(str));
  again:
    if (fflush(migemow) != 0) {
	switch (errno) {
	case EINTR:
	    goto again;
	default:
	    goto err;
	}
    }
    tmp = Str_conv_from_system(Strfgets(migemor));
    Strchop(tmp);
    if (tmp->length == 0)
	goto err;
    return conv_search_string(tmp->ptr, SystemCharset);
  err:
    /* XXX: backend migemo is not working? */
    init_migemo();
    migemo_active = migemo_running = 0;
    return str;
}
#endif				/* USE_MIGEMO */

#ifdef USE_M17N
/* normalize search string */
char *
conv_search_string(char *str, wc_ces f_ces)
{
    if (SearchConv && !WcOption.pre_conv &&
	Currentbuf->document_charset != f_ces)
	str = wtf_conv_fit(str, Currentbuf->document_charset);
    return str;
}
#endif

int
forwardSearch(Buffer *buf, char *str)
{
    char *p, *first, *last;
    Line *l, *begin;
    int wrapped = FALSE;
    int pos;

#ifdef USE_MIGEMO
    if (migemo_active > 0) {
	if (((p = regexCompile(migemostr(str), IgnoreCase)) != NULL)
	    && ((p = regexCompile(str, IgnoreCase)) != NULL)) {
	    message(p, 0, 0);
	    return SR_NOTFOUND;
	}
    }
    else
#endif
    if ((p = regexCompile(str, IgnoreCase)) != NULL) {
	message(p, 0, 0);
	return SR_NOTFOUND;
    }
    l = buf->currentLine;
    if (l == NULL) {
	return SR_NOTFOUND;
    }
    pos = buf->pos;
    if (l->bpos) {
	pos += l->bpos;
	while (l->bpos && l->prev)
	    l = l->prev;
    }
    begin = l;
#ifdef USE_M17N
    while (pos < l->size && l->propBuf[pos] & PC_WCHAR2)
	pos++;
#endif
    if (pos < l->size && regexMatch(&l->lineBuf[pos], l->size - pos, 0) == 1) {
	matchedPosition(&first, &last);
	pos = first - l->lineBuf;
	while (pos >= l->len && l->next && l->next->bpos) {
	    pos -= l->len;
	    l = l->next;
	}
	buf->pos = pos;
	if (l != buf->currentLine)
	    gotoLine(buf, l->linenumber);
	arrangeCursor(buf);
	set_mark(l, pos, pos + last - first);
	return SR_FOUND;
    }
    for (l = l->next;; l = l->next) {
	if (l == NULL) {
	    if (buf->pagerSource) {
		l = getNextPage(buf, 1);
		if (l == NULL) {
		    if (WrapSearch && !wrapped) {
			l = buf->firstLine;
			wrapped = TRUE;
		    }
		    else {
			break;
		    }
		}
	    }
	    else if (WrapSearch) {
		l = buf->firstLine;
		wrapped = TRUE;
	    }
	    else {
		break;
	    }
	}
	if (l->bpos)
	    continue;
	if (regexMatch(l->lineBuf, l->size, 1) == 1) {
	    matchedPosition(&first, &last);
	    pos = first - l->lineBuf;
	    while (pos >= l->len && l->next && l->next->bpos) {
		pos -= l->len;
		l = l->next;
	    }
	    buf->pos = pos;
	    buf->currentLine = l;
	    gotoLine(buf, l->linenumber);
	    arrangeCursor(buf);
	    set_mark(l, pos, pos + last - first);
	    return SR_FOUND | (wrapped ? SR_WRAPPED : 0);
	}
	if (wrapped && l == begin)	/* no match */
	    break;
    }
    return SR_NOTFOUND;
}

int
backwardSearch(Buffer *buf, char *str)
{
    char *p, *q, *found, *found_last, *first, *last;
    Line *l, *begin;
    int wrapped = FALSE;
    int pos;

#ifdef USE_MIGEMO
    if (migemo_active > 0) {
	if (((p = regexCompile(migemostr(str), IgnoreCase)) != NULL)
	    && ((p = regexCompile(str, IgnoreCase)) != NULL)) {
	    message(p, 0, 0);
	    return SR_NOTFOUND;
	}
    }
    else
#endif
    if ((p = regexCompile(str, IgnoreCase)) != NULL) {
	message(p, 0, 0);
	return SR_NOTFOUND;
    }
    l = buf->currentLine;
    if (l == NULL) {
	return SR_NOTFOUND;
    }
    pos = buf->pos;
    if (l->bpos) {
	pos += l->bpos;
	while (l->bpos && l->prev)
	    l = l->prev;
    }
    begin = l;
    if (pos > 0) {
	pos--;
#ifdef USE_M17N
	while (pos > 0 && l->propBuf[pos] & PC_WCHAR2)
	    pos--;
#endif
	p = &l->lineBuf[pos];
	found = NULL;
	found_last = NULL;
	q = l->lineBuf;
	while (regexMatch(q, &l->lineBuf[l->size] - q, q == l->lineBuf) == 1) {
	    matchedPosition(&first, &last);
	    if (first <= p) {
		found = first;
		found_last = last;
	    }
	    if (q - l->lineBuf >= l->size)
		break;
	    q++;
#ifdef USE_M17N
	    while (q - l->lineBuf < l->size
		   && l->propBuf[q - l->lineBuf] & PC_WCHAR2)
		q++;
#endif
	    if (q > p)
		break;
	}
	if (found) {
	    pos = found - l->lineBuf;
	    while (pos >= l->len && l->next && l->next->bpos) {
		pos -= l->len;
		l = l->next;
	    }
	    buf->pos = pos;
	    if (l != buf->currentLine)
		gotoLine(buf, l->linenumber);
	    arrangeCursor(buf);
	    set_mark(l, pos, pos + found_last - found);
	    return SR_FOUND;
	}
    }
    for (l = l->prev;; l = l->prev) {
	if (l == NULL) {
	    if (WrapSearch) {
		l = buf->lastLine;
		wrapped = TRUE;
	    }
	    else {
		break;
	    }
	}
	found = NULL;
	found_last = NULL;
	q = l->lineBuf;
	while (regexMatch(q, &l->lineBuf[l->size] - q, q == l->lineBuf) == 1) {
	    matchedPosition(&first, &last);
	    found = first;
	    found_last = last;
	    if (q - l->lineBuf >= l->size)
		break;
	    q++;
#ifdef USE_M17N
	    while (q - l->lineBuf < l->size
		   && l->propBuf[q - l->lineBuf] & PC_WCHAR2)
		q++;
#endif
	}
	if (found) {
	    pos = found - l->lineBuf;
	    while (pos >= l->len && l->next && l->next->bpos) {
		pos -= l->len;
		l = l->next;
	    }
	    buf->pos = pos;
	    gotoLine(buf, l->linenumber);
	    arrangeCursor(buf);
	    set_mark(l, pos, pos + found_last - found);
	    return SR_FOUND | (wrapped ? SR_WRAPPED : 0);
	}
	if (wrapped && l == begin)	/* no match */
	    break;
    }
    return SR_NOTFOUND;
}
