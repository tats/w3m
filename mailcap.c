/* $Id: mailcap.c,v 1.13 2006/08/07 03:10:26 ukai Exp $ */
#include "fm.h"
#include "myctype.h"
#include <stdio.h>
#include <errno.h>
#include "parsetag.h"
#include "local.h"

static struct mailcap DefaultMailcap[] = {
    {"image/*", DEF_IMAGE_VIEWER " %s", 0, NULL, NULL, NULL},	/* */
    {"audio/basic", DEF_AUDIO_PLAYER " %s", 0, NULL, NULL, NULL},
    {NULL, NULL, 0, NULL, NULL, NULL}
};

static TextList *mailcap_list;
static struct mailcap **UserMailcap;

int
mailcapMatch(struct mailcap *mcap, char *type)
{
    char *cap = mcap->type, *p;
    int level;
    for (p = cap; *p != '/'; p++) {
	if (TOLOWER(*p) != TOLOWER(*type))
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
	if (TOLOWER(*p) != TOLOWER(*type))
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
		Str command =
		    unquote_mailcap(table->test, type, NULL, NULL, NULL);
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
matchMailcapAttr(char *p, char *attr, int len, Str *value)
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

static int
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
    mcap->type = allocStr(p, (k >= 0) ? k + 1 : j);
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
    mcap->viewer = allocStr(p, (k >= 0) ? k + 1 : j);
    p += j;

    while (*p == ';') {
	p++;
	SKIP_BLANKS(p);
	if (matchMailcapAttr(p, "needsterminal", 13, NULL)) {
	    mcap->flags |= MAILCAP_NEEDSTERMINAL;
	}
	else if (matchMailcapAttr(p, "copiousoutput", 13, NULL)) {
	    mcap->flags |= MAILCAP_COPIOUSOUTPUT;
	}
	else if (matchMailcapAttr(p, "x-htmloutput", 12, NULL) ||
		 matchMailcapAttr(p, "htmloutput", 10, NULL)) {
	    mcap->flags |= MAILCAP_HTMLOUTPUT;
	}
	else if (matchMailcapAttr(p, "test", 4, &tmp)) {
	    mcap->test = allocStr(tmp->ptr, tmp->length);
	}
	else if (matchMailcapAttr(p, "nametemplate", 12, &tmp)) {
	    mcap->nametemplate = allocStr(tmp->ptr, tmp->length);
	}
	else if (matchMailcapAttr(p, "edit", 4, &tmp)) {
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

static struct mailcap *
loadMailcap(char *filename)
{
    FILE *f;
    int i, n;
    Str tmp;
    struct mailcap *mcap;

    f = fopen(expandPath(filename), "r");
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

void
initMailcap()
{
    TextListItem *tl;
    int i;

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

char *
acceptableMimeTypes()
{
    static Str types = NULL;
    TextList *l;
    Hash_si *mhash;
    char *p;
    int i;

    if (types != NULL)
	return types->ptr;

    /* generate acceptable media types */
    l = newTextList();
    mhash = newHash_si(16);	/* XXX */
    /* pushText(l, "text"); */
    putHash_si(mhash, "text", 1);
    pushText(l, "image");
    putHash_si(mhash, "image", 1);
    for (i = 0; i < mailcap_list->nitem; i++) {
	struct mailcap *mp = UserMailcap[i];
	char *mt;
	if (mp == NULL)
	    continue;
	for (; mp->type; mp++) {
	    p = strchr(mp->type, '/');
	    if (p == NULL)
		continue;
	    mt = allocStr(mp->type, p - mp->type);
	    if (getHash_si(mhash, mt, 0) == 0) {
		pushText(l, mt);
		putHash_si(mhash, mt, 1);
	    }
	}
    }
    types = Strnew();
    Strcat_charp(types, "text/html, text/*;q=0.5");
    while ((p = popText(l)) != NULL) {
	Strcat_charp(types, ", ");
	Strcat_charp(types, p);
	Strcat_charp(types, "/*");
    }
    return types->ptr;
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

#define MCF_SQUOTED (1 << 0)
#define MCF_DQUOTED (1 << 1)

Str
quote_mailcap(char *s, int flag)
{
    Str d;

    d = Strnew();

    for (;; ++s)
	switch (*s) {
	case '\0':
	    goto end;
	case '$':
	case '`':
	case '"':
	case '\\':
	    if (!(flag & MCF_SQUOTED))
		Strcat_char(d, '\\');

	    Strcat_char(d, *s);
	    break;
	case '\'':
	    if (flag & MCF_SQUOTED) {
		Strcat_charp(d, "'\\''");
		break;
	    }
	default:
	    if (!flag && !IS_ALNUM(*s))
		Strcat_char(d, '\\');
	case '_':
	case '.':
	case ':':
	case '/':
	    Strcat_char(d, *s);
	    break;
	}
  end:
    return d;
}


static Str
unquote_mailcap_loop(char *qstr, char *type, char *name, char *attr,
		     int *mc_stat, int flag0)
{
    Str str, tmp, test, then;
    char *p;
    int status = MC_NORMAL, prev_status = MC_NORMAL, sp = 0, flag;

    if (mc_stat)
	*mc_stat = 0;

    if (qstr == NULL)
	return NULL;

    str = Strnew();
    tmp = test = then = NULL;

    for (flag = flag0, p = qstr; *p; p++) {
	if (status == MC_QUOTED) {
	    if (prev_status == MC_PREC2)
		Strcat_char(tmp, *p);
	    else
		Strcat_char(str, *p);
	    status = prev_status;
	    continue;
	}
	else if (*p == '\\') {
	    prev_status = status;
	    status = MC_QUOTED;
	    continue;
	}
	switch (status) {
	case MC_NORMAL:
	    if (*p == '%') {
		status = MC_PREC;
	    }
	    else {
		if (*p == '\'') {
		    if (!flag0 && flag & MCF_SQUOTED)
			flag &= ~MCF_SQUOTED;
		    else if (!flag)
			flag |= MCF_SQUOTED;
		}
		else if (*p == '"') {
		    if (!flag0 && flag & MCF_DQUOTED)
			flag &= ~MCF_DQUOTED;
		    else if (!flag)
			flag |= MCF_DQUOTED;
		}
		Strcat_char(str, *p);
	    }
	    break;
	case MC_PREC:
	    if (IS_ALPHA(*p)) {
		switch (*p) {
		case 's':
		    if (name) {
			Strcat_charp(str, quote_mailcap(name, flag)->ptr);
			if (mc_stat)
			    *mc_stat |= MCSTAT_REPNAME;
		    }
		    break;
		case 't':
		    if (type) {
			Strcat_charp(str, quote_mailcap(type, flag)->ptr);
			if (mc_stat)
			    *mc_stat |= MCSTAT_REPTYPE;
		    }
		    break;
		}
		status = MC_NORMAL;
	    }
	    else if (*p == '{') {
		status = MC_PREC2;
		test = then = NULL;
		tmp = Strnew();
	    }
	    else if (*p == '%') {
		Strcat_char(str, *p);
	    }
	    break;
	case MC_PREC2:
	    if (sp > 0 || *p == '{') {
		Strcat_char(tmp, *p);

		switch (*p) {
		case '{':
		    ++sp;
		    break;
		case '}':
		    --sp;
		    break;
		default:
		    break;
		}
	    }
	    else if (*p == '}') {
		char *q;
		if (attr && (q = strcasestr(attr, tmp->ptr)) != NULL &&
		    (q == attr || IS_SPACE(*(q - 1)) || *(q - 1) == ';') &&
		    matchattr(q, tmp->ptr, tmp->length, &tmp)) {
		    Strcat_charp(str, quote_mailcap(tmp->ptr, flag)->ptr);
		    if (mc_stat)
			*mc_stat |= MCSTAT_REPPARAM;
		}
		status = MC_NORMAL;
	    }
	    else {
		Strcat_char(tmp, *p);
	    }
	    break;
	}
    }
    return str;
}

Str
unquote_mailcap(char *qstr, char *type, char *name, char *attr, int *mc_stat)
{
    return unquote_mailcap_loop(qstr, type, name, attr, mc_stat, 0);
}
