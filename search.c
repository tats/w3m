/* $Id: search.c,v 1.12 2002/01/16 19:18:21 ukai Exp $ */
#include "fm.h"
#include "regex.h"
#include <errno.h>

static void
set_mark(Line *l, int pos, int epos)
{
    for (; pos < epos && pos < l->len; pos++)
	l->propBuf[pos] |= PE_MARK;
}

#ifdef USE_MIGEMO
/* Migemo: romaji --> kana+kanji in regexp */
static FILE *migemor, *migemow;

void
init_migemo()
{
    if (migemor != NULL)
	fclose(migemor);
    if (migemow != NULL)
	fclose(migemow);
    migemor = migemow = NULL;
}

static int
open_migemo(char *migemo_command)
{
    int fdr[2];
    int fdw[2];
    int pid;
    if (pipe(fdr) < 0)
	goto err0;
    if (pipe(fdw) < 0)
	goto err1;

    /* migemow:fdw[1] -|-> fdw[0]=0 {migemo} fdr[1]=1 -|-> fdr[0]:migemor */
    pid = fork();
    if (pid < 0)
	goto err2;
    if (pid == 0) {
	/* child */
	close(fdr[0]);
	close(fdw[1]);
	dup2(fdw[0], 0);
	dup2(fdr[1], 1);
	close(2);
	system(migemo_command);
	exit(1);
    }
    close(fdr[1]);
    close(fdw[0]);
    migemor = fdopen(fdr[0], "r");
    migemow = fdopen(fdw[1], "w");
    return 1;
  err2:
    close(fdw[0]);
    close(fdw[1]);
  err1:
    close(fdr[0]);
    close(fdr[1]);
  err0:
    use_migemo = 0;
    return 0;
}

static char *
migemostr(char *str)
{
    Str tmp = NULL;
    if (migemor == NULL || migemow == NULL)
	if (open_migemo(migemo_command) == 0)
	    return str;
    fprintf(migemow, "%s\n", str);
  again:
    if (fflush(migemow) != 0) {
	switch (errno) {
	case EINTR:
	    goto again;
	default:
	    goto err;
	}
    }
    tmp = Strfgets(migemor);
    Strchop(tmp);
    if (tmp->length == 0)
	goto err;
    return tmp->ptr;
  err:
    /* XXX: backend migemo is not working? */
    init_migemo();
    use_migemo = 0;
    return str;
}
#endif				/* USE_MIGEMO */

int
forwardSearch(Buffer *buf, char *str)
{
    char *p, *first, *last;
    Line *l, *begin;
    int wrapped = FALSE;
    int pos;

#ifdef USE_MIGEMO
    if (use_migemo) {
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
    l = begin = buf->currentLine;
    if (l == NULL) {
	return SR_NOTFOUND;
    }
    pos = buf->pos + 1;
#ifdef JP_CHARSET
    if (l->propBuf[pos] & PC_KANJI2)
	pos++;
#endif
    if (pos < l->len && regexMatch(&l->lineBuf[pos], l->len - pos, 0) == 1) {
	matchedPosition(&first, &last);
	buf->pos = first - l->lineBuf;
	arrangeCursor(buf);
	set_mark(l, buf->pos, last - l->lineBuf);
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
	if (regexMatch(l->lineBuf, l->len, 1) == 1) {
	    matchedPosition(&first, &last);
	    if (wrapped && l == begin && buf->pos == first - l->lineBuf)
		/* exactly same match */
		break;
	    buf->pos = first - l->lineBuf;
	    buf->currentLine = l;
	    gotoLine(buf, l->linenumber);
	    arrangeCursor(buf);
	    set_mark(l, buf->pos, last - l->lineBuf);
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
    char *p, *q, *found, *first, *last;
    Line *l, *begin;
    int wrapped = FALSE;
    int pos;

#ifdef USE_MIGEMO
    if (use_migemo) {
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
    l = begin = buf->currentLine;
    if (l == NULL) {
	return SR_NOTFOUND;
    }
    if (buf->pos > 0) {
	pos = buf->pos - 1;
#ifdef JP_CHARSET
	if (l->propBuf[pos] & PC_KANJI2)
	    pos--;
#endif
	p = &l->lineBuf[pos];
	found = NULL;
	q = l->lineBuf;
	while (regexMatch(q, &l->lineBuf[l->len] - q, q == l->lineBuf) == 1) {
	    matchedPosition(&first, &last);
	    if (first <= p)
		found = first;
#ifdef JP_CHARSET
	    if (l->propBuf[q - l->lineBuf] & PC_KANJI1)
		q += 2;
	    else
#endif
		q++;
	    if (q > p)
		break;
	}
	if (found) {
	    buf->pos = found - l->lineBuf;
	    arrangeCursor(buf);
	    set_mark(l, buf->pos, last - l->lineBuf);
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
	q = l->lineBuf;
	while (regexMatch(q, &l->lineBuf[l->len] - q, q == l->lineBuf) == 1) {
	    matchedPosition(&first, &last);
	    if (wrapped && l == begin && buf->pos == first - l->lineBuf)
		/* exactly same match */
		;
	    else
		found = first;
#ifdef JP_CHARSET
	    if (l->propBuf[q - l->lineBuf] & PC_KANJI1)
		q += 2;
	    else
#endif
		q++;
	}
	if (found) {
	    buf->pos = found - l->lineBuf;
	    buf->currentLine = l;
	    gotoLine(buf, l->linenumber);
	    arrangeCursor(buf);
	    set_mark(l, buf->pos, last - l->lineBuf);
	    return SR_FOUND | (wrapped ? SR_WRAPPED : 0);
	}
	if (wrapped && l == begin)	/* no match */
	    break;
    }
    return SR_NOTFOUND;
}
