/* $Id: mailcap.c,v 1.1 2001/11/08 05:15:04 a-ito Exp $ */
#include "fm.h"
#include "myctype.h"
#include <stdio.h>
#include <errno.h>
#include "parsetag.h"
#include "local.h"

static struct mailcap DefaultMailcap[] =
{
    {"image/*", "xv %s", 0, NULL, NULL, NULL},	/* */
    {"audio/basic", "showaudio %s", 0, NULL, NULL, NULL},
    {NULL, NULL, 0, NULL, NULL, NULL}
};

void
initMailcap()
{
    int i;
    TextListItem *tl;

    if (non_null(mailcap_files))
	mailcap_list = make_domain_list(mailcap_files);
    else
	mailcap_list = NULL;
    if (mailcap_list == NULL)
	return;
    UserMailcap = New_N(struct mailcap *, mailcap_list->nitem);
    for (i = 0, tl = mailcap_list->first; tl; i++, tl = tl->next)
	UserMailcap[i] = loadMailcap(tl->ptr);
}

int
mailcapMatch(struct mailcap *mcap, char *type)
{
    char *cap = mcap->type, *p;
    int level;
    for (p = cap; *p != '/'; p++) {
	if (tolower(*p) != tolower(*type))
	    return 0;
	type++;
    }
    if (*type != '/')
	return 0;
    p++;
    type++;
    if (mcap->flags & MAILCAP_HTMLOUTPUT)
	level = 1;
    else
	level = 0;
    if (*p == '*')
	return 10 + level;
    while (*p) {
	if (tolower(*p) != tolower(*type))
	    return 0;
	p++;
	type++;
    }
    if (*type != '\0')
	return 0;
    return 20 + level;
}

struct mailcap *
searchMailcap(struct mailcap *table, char *type)
{
    int level = 0;
    struct mailcap *mcap = NULL;
    int i;

    if (table == NULL)
	return NULL;
    for (; table->type; table++) {
	i = mailcapMatch(table, type);
	if (i > level) {
	    if (table->test) {
		Str command = unquote_mailcap(table->test, type, NULL, NULL);
		if (system(command->ptr) != 0)
		    continue;
	    }
	    level = i;
	    mcap = table;
	}
    }
    return mcap;
}

static int
matchattr(char *p, char *attr, int len, Str * value)
{
    int quoted;
    char *q = NULL;

    if (strncasecmp(p, attr, len) == 0) {
	p += len;
	SKIP_BLANKS(p);
	if (value) {
	    *value = Strnew();
	    if (*p == '=') {
		p++;
		SKIP_BLANKS(p);
		quoted = 0;
		while (*p && (quoted || *p != ';')) {
		    if (quoted || !IS_SPACE(*p))
			q = p;
		    if (quoted)
			quoted = 0;
		    else if (*p == '\\')
			quoted = 1;
		    Strcat_char(*value, *p);
		    p++;
		}
		if (q)
		    Strshrink(*value, p - q - 1);
	    }
	    return 1;
	}
	else {
	    if (*p == '\0' || *p == ';') {
		return 1;
	    }
	}
    }
    return 0;
}

int
extractMailcapEntry(char *mcap_entry, struct mailcap *mcap)
{
    int j, k;
    char *p;
    int quoted;
    Str tmp;

    bzero(mcap, sizeof(struct mailcap));
    p = mcap_entry;
    SKIP_BLANKS(p);
    k = -1;
    for (j = 0; p[j] && p[j] != ';'; j++) {
	if (!IS_SPACE(p[j]))
	    k = j;
    }
    mcap->type = allocStr(p, (k >= 0)? k + 1 : j);
    if (!p[j])
	return 0;
    p += j + 1;

    SKIP_BLANKS(p);
    k = -1;
    quoted = 0;
    for (j = 0; p[j] && (quoted || p[j] != ';'); j++) {
	if (quoted || !IS_SPACE(p[j]))
	    k = j;
	if (quoted)
	    quoted = 0;
	else if (p[j] == '\\')
	    quoted = 1;
    }
    mcap->viewer = allocStr(p, (k >= 0)? k + 1 : j);
    p += j;

    while (*p == ';') {
	p++;
	SKIP_BLANKS(p);
	if (matchattr(p, "needsterminal", 13, NULL)) {
	    mcap->flags |= MAILCAP_NEEDSTERMINAL;
	}
	else if (matchattr(p, "copiousoutput", 13, NULL)) {
	    mcap->flags |= MAILCAP_COPIOUSOUTPUT;
	}
	else if (matchattr(p, "htmloutput", 10, NULL)) {
	    mcap->flags |= MAILCAP_HTMLOUTPUT;
	}
	else if (matchattr(p, "test", 4, &tmp)) {
	    mcap->test = allocStr(tmp->ptr, tmp->length);
	}
	else if (matchattr(p, "nametemplate", 12, &tmp)) {
	    mcap->nametemplate = allocStr(tmp->ptr, tmp->length);
	}
	else if (matchattr(p, "edit", 4, &tmp)) {
	    mcap->edit = allocStr(tmp->ptr, tmp->length);
	}
	quoted = 0;
	while (*p && (quoted || *p != ';')) {
	    if (quoted)
		quoted = 0;
	    else if (*p == '\\')
		quoted = 1;
	    p++;
	}
    }
    return 1;
}

struct mailcap *
loadMailcap(char *filename)
{
    FILE *f;
    int i, n;
    Str tmp;
    struct mailcap *mcap;

    f = fopen(expandName(filename), "r");
    if (f == NULL)
	return NULL;
    i = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] != '#')
	    i++;
    }
    fseek(f, 0, 0);
    n = i;
    mcap = New_N(struct mailcap, n + 1);
    i = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] == '#')
	    continue;
      redo:
	while (IS_SPACE(Strlastchar(tmp)))
	    Strshrink(tmp, 1);
	if (Strlastchar(tmp) == '\\') {
	    /* continuation */
	    Strshrink(tmp, 1);
	    Strcat(tmp, Strfgets(f));
	    goto redo;
	}
	if (extractMailcapEntry(tmp->ptr, &mcap[i]))
	    i++;
    }
    bzero(&mcap[i], sizeof(struct mailcap));
    fclose(f);
    return mcap;
}

struct mailcap *
searchExtViewer(char *type)
{
    struct mailcap *p;
    int i;

    if (mailcap_list == NULL)
	goto no_user_mailcap;
    
    for (i = 0; i < mailcap_list->nitem; i++) {
	if ((p = searchMailcap(UserMailcap[i], type)) != NULL)
	    return p;
    }

 no_user_mailcap:
    return searchMailcap(DefaultMailcap, type);
}

#define MC_NORMAL 0
#define MC_PREC   1
#define MC_PREC2  2
#define MC_QUOTED 3

Str
unquote_mailcap(char *qstr, char *type, char *name, int *stat)
{
    Str str = Strnew();
    char *p;
    int status = MC_NORMAL;

    if (stat)
	*stat = 0;
    
    if (qstr == NULL)
	return NULL;

    for (p = qstr; *p; p++) {
	if (status == MC_QUOTED) {
	    Strcat_char(str, *p);
	    status = MC_NORMAL;
	    continue;
	}
	else if (*p == '\\') {
	    status = MC_QUOTED;
	    continue;
	}
	switch (status) {
	case MC_NORMAL:
	    if (*p == '%') {
		status = MC_PREC;
	    }
	    else
		Strcat_char(str, *p);
	    break;
	case MC_PREC:
	    if (IS_ALPHA(*p)) {
		switch (*p) {
		case 's':
		    if (name) {
			Strcat_charp(str, name);
			if (stat)
			    *stat |= MCSTAT_REPNAME;
		    }
		    break;
		case 't':
		    if (type) {
			Strcat_charp(str, type);
			if (stat)
			    *stat |= MCSTAT_REPTYPE;
		    }
		    break;
		}
		status = MC_NORMAL;
	    }
	    else if (*p == '{') {
		status = MC_PREC2;
	    }
	    else if (*p == '%') {
		Strcat_char(str, *p);
	    }
	    break;
	case MC_PREC2:
	    if (*p == '}') {
		status = MC_NORMAL;
	    }
	    break;
	}
    }
    return str;
}
