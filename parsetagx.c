/* $Id: parsetagx.c,v 1.18 2006/06/07 03:52:03 inu Exp $ */
#include "fm.h"
#include "myctype.h"
#include "indep.h"
#include "Str.h"
#include "parsetagx.h"
#include "hash.h"

#include "html.c"

/* parse HTML tag */

static int noConv(char *, void *);
static int toNumber(char *, void *);
static int toLength(char *, void *);
static int toAlign(char *, void *);
static int toVAlign(char *, void *);

/* *INDENT-OFF* */
static int (*toValFunc[]) (char *, void *) = {
    noConv,		/* VTYPE_NONE    */
    noConv,		/* VTYPE_STR     */
    toNumber,		/* VTYPE_NUMBER  */
    toLength,		/* VTYPE_LENGTH  */
    toAlign,		/* VTYPE_ALIGN   */
    toVAlign,		/* VTYPE_VALIGN  */
    noConv,		/* VTYPE_ACTION  */
    noConv,		/* VTYPE_ENCTYPE */
    noConv,		/* VTYPE_METHOD  */
    noConv,		/* VTYPE_MLENGTH */
    noConv,		/* VTYPE_TYPE    */
};
/* *INDENT-ON* */

static int
noConv(char *oval, void *str)
{
    *(char **)str = oval;
    return 1;
}

static int
toNumber(char *oval, void *num)
{
    char *ep;
    int x;

    x = strtol(oval, &ep, 10);

    if (ep > oval) {
	*(int *)num = x;
	return 1;
    }
    else
	return 0;
}

static int
toLength(char *oval, void *len)
{
    int w;
    if (!IS_DIGIT(oval[0]))
	return 0;
    w = atoi(oval);
    if (w < 0)
	return 0;
    if (w == 0)
	w = 1;
    if (oval[strlen(oval) - 1] == '%')
	*(int *)len = -w;
    else
	*(int *)len = w;
    return 1;
}

static int
toAlign(char *oval, void *align)
{
    if (strcasecmp(oval, "left") == 0)
	*(int *)align = ALIGN_LEFT;
    else if (strcasecmp(oval, "right") == 0)
	*(int *)align = ALIGN_RIGHT;
    else if (strcasecmp(oval, "center") == 0)
	*(int *)align = ALIGN_CENTER;
    else if (strcasecmp(oval, "top") == 0)
	*(int *)align = ALIGN_TOP;
    else if (strcasecmp(oval, "bottom") == 0)
	*(int *)align = ALIGN_BOTTOM;
    else if (strcasecmp(oval, "middle") == 0)
	*(int *)align = ALIGN_MIDDLE;
    else
	return 0;
    return 1;
}

static int
toVAlign(char *oval, void *valign)
{
    if (strcasecmp(oval, "top") == 0 || strcasecmp(oval, "baseline") == 0)
	*(int *)valign = VALIGN_TOP;
    else if (strcasecmp(oval, "bottom") == 0)
	*(int *)valign = VALIGN_BOTTOM;
    else if (strcasecmp(oval, "middle") == 0)
	*(int *)valign = VALIGN_MIDDLE;
    else
	return 0;
    return 1;
}

extern Hash_si tagtable;
#define MAX_TAG_LEN 64

struct parsed_tag *
parse_tag(char **s, int internal)
{
    struct parsed_tag *tag = NULL;
    int tag_id;
    char tagname[MAX_TAG_LEN], attrname[MAX_TAG_LEN];
    char *p, *q;
    int i, attr_id = 0, nattr;

    /* Parse tag name */
    tagname[0] = '\0';
    q = (*s) + 1;
    p = tagname;
    if (*q == '/') {
	*(p++) = *(q++);
	SKIP_BLANKS(q);
    }
    while (*q && !IS_SPACE(*q) && !(tagname[0] != '/' && *q == '/') &&
	   *q != '>' && p - tagname < MAX_TAG_LEN - 1) {
	*(p++) = TOLOWER(*q);
	q++;
    }
    *p = '\0';
    while (*q && !IS_SPACE(*q) && !(tagname[0] != '/' && *q == '/') &&
	   *q != '>')
	q++;

    tag_id = getHash_si(&tagtable, tagname, HTML_UNKNOWN);

    if (tag_id == HTML_UNKNOWN ||
	(!internal && TagMAP[tag_id].flag & TFLG_INT))
	goto skip_parse_tagarg;

    tag = New(struct parsed_tag);
    bzero(tag, sizeof(struct parsed_tag));
    tag->tagid = tag_id;

    if ((nattr = TagMAP[tag_id].max_attribute) > 0) {
	tag->attrid = NewAtom_N(unsigned char, nattr);
	tag->value = New_N(char *, nattr);
	tag->map = NewAtom_N(unsigned char, MAX_TAGATTR);
	memset(tag->map, MAX_TAGATTR, MAX_TAGATTR);
	memset(tag->attrid, ATTR_UNKNOWN, nattr);
	for (i = 0; i < nattr; i++)
	    tag->map[TagMAP[tag_id].accept_attribute[i]] = i;
    }

    /* Parse tag arguments */
    SKIP_BLANKS(q);
    while (1) {
       Str value = NULL, value_tmp = NULL;
	if (*q == '>' || *q == '\0')
	    goto done_parse_tag;
	p = attrname;
	while (*q && *q != '=' && !IS_SPACE(*q) &&
	       *q != '>' && p - attrname < MAX_TAG_LEN - 1) {
	    *(p++) = TOLOWER(*q);
	    q++;
	}
	*p = '\0';
	while (*q && *q != '=' && !IS_SPACE(*q) && *q != '>')
	    q++;
	SKIP_BLANKS(q);
	if (*q == '=') {
	    /* get value */
	    value_tmp = Strnew();
	    q++;
	    SKIP_BLANKS(q);
	    if (*q == '"') {
		q++;
		while (*q && *q != '"') {
		    Strcat_char(value_tmp, *q);
		    if (!tag->need_reconstruct && is_html_quote(*q))
			tag->need_reconstruct = TRUE;
		    q++;
		}
		if (*q == '"')
		    q++;
	    }
	    else if (*q == '\'') {
		q++;
		while (*q && *q != '\'') {
		    Strcat_char(value_tmp, *q);
		    if (!tag->need_reconstruct && is_html_quote(*q))
			tag->need_reconstruct = TRUE;
		    q++;
		}
		if (*q == '\'')
		    q++;
	    }
	    else if (*q) {
		while (*q && !IS_SPACE(*q) && *q != '>') {
                   Strcat_char(value_tmp, *q);
		    if (!tag->need_reconstruct && is_html_quote(*q))
			tag->need_reconstruct = TRUE;
		    q++;
		}
	    }
	}
	for (i = 0; i < nattr; i++) {
	    if ((tag)->attrid[i] == ATTR_UNKNOWN &&
		strcmp(AttrMAP[TagMAP[tag_id].accept_attribute[i]].name,
		       attrname) == 0) {
		attr_id = TagMAP[tag_id].accept_attribute[i];
		break;
	    }
	}

       if (value_tmp) {
         int j, hidden=FALSE;
         for (j=0; j<i; j++) {
           if (tag->attrid[j] == ATTR_TYPE &&
               tag->value[j] &&
               strcmp("hidden",tag->value[j]) == 0) {
             hidden=TRUE;
             break;
           }
         }
         if ((tag_id == HTML_INPUT || tag_id == HTML_INPUT_ALT) &&
             attr_id == ATTR_VALUE && hidden) {
           value = value_tmp;
         } else {
           char *x;
           value = Strnew();
           for (x = value_tmp->ptr; *x; x++) {
             if (*x != '\n')
               Strcat_char(value, *x);
           }
         }
       }

	if (i != nattr) {
	    if (!internal &&
		((AttrMAP[attr_id].flag & AFLG_INT) ||
		 (value && AttrMAP[attr_id].vtype == VTYPE_METHOD &&
		  !strcasecmp(value->ptr, "internal")))) {
		tag->need_reconstruct = TRUE;
		continue;
	    }
	    tag->attrid[i] = attr_id;
	    if (value)
		tag->value[i] = html_unquote(value->ptr);
	    else
		tag->value[i] = NULL;
	}
	else {
	    tag->need_reconstruct = TRUE;
	}
    }

  skip_parse_tagarg:
    while (*q != '>' && *q)
	q++;
  done_parse_tag:
    if (*q == '>')
	q++;
    *s = q;
    return tag;
}

int
parsedtag_set_value(struct parsed_tag *tag, int id, char *value)
{
    int i;

    if (!parsedtag_accepts(tag, id))
	return 0;

    i = tag->map[id];
    tag->attrid[i] = id;
    if (value)
	tag->value[i] = allocStr(value, -1);
    else
	tag->value[i] = NULL;
    tag->need_reconstruct = TRUE;
    return 1;
}

int
parsedtag_get_value(struct parsed_tag *tag, int id, void *value)
{
    int i;
    if (!parsedtag_exists(tag, id) || !tag->value[i = tag->map[id]])
	return 0;
    return toValFunc[AttrMAP[id].vtype] (tag->value[i], value);
}

Str
parsedtag2str(struct parsed_tag *tag)
{
    int i;
    int tag_id = tag->tagid;
    int nattr = TagMAP[tag_id].max_attribute;
    Str tagstr = Strnew();
    Strcat_char(tagstr, '<');
    Strcat_charp(tagstr, TagMAP[tag_id].name);
    for (i = 0; i < nattr; i++) {
	if (tag->attrid[i] != ATTR_UNKNOWN) {
	    Strcat_char(tagstr, ' ');
	    Strcat_charp(tagstr, AttrMAP[tag->attrid[i]].name);
	    if (tag->value[i])
		Strcat(tagstr, Sprintf("=\"%s\"", html_quote(tag->value[i])));
	}
    }
    Strcat_char(tagstr, '>');
    return tagstr;
}
