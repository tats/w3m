/* $Id: search.c,v 1.5 2001/12/23 14:44:00 ukai Exp $ */
#include "fm.h"
#include "regex.h"

int
forwardSearch(Buffer *buf, char *str)
{
    char *p, *first, *last;
    Line *l, *begin;
    int wrapped = FALSE;
    int pos;

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
    if (regexMatch(&l->lineBuf[pos], l->len - pos, 0) == 1) {
	matchedPosition(&first, &last);
	buf->pos = first - l->lineBuf;
	arrangeCursor(buf);
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
	    return SR_FOUND | (wrapped ? SR_WRAPPED : 0);
	}
	if (wrapped && l == begin)	/* no match */
	    break;
    }
    return SR_NOTFOUND;
}
