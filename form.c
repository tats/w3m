/* $Id: form.c,v 1.12 2002/02/04 15:18:42 ukai Exp $ */
/* 
 * HTML forms
 */
#include "fm.h"
#include "parsetag.h"
#include "parsetagx.h"
#include "myctype.h"
#include "local.h"

#ifdef __EMX__
/* lstat is identical to stat, only the link itself is statted, not the file
 * that is obtained by tracing the links. But on OS/2 systems, there is no
 * differences. */
#define lstat stat
#endif				/* __EMX__ */

extern Str *textarea_str;
#ifdef MENU_SELECT
extern FormSelectOption *select_option;
#include "menu.h"
#endif				/* MENU_SELECT */

/* *INDENT-OFF* */
struct {
    char *action;
    void (*rout)(struct parsed_tagarg *);
} internal_action[] = {
    {"map", follow_map}, 
    {"option", panel_set_option},
#ifdef USE_COOKIE
    {"cookie", set_cookie_flag},
#endif				/* USE_COOKIE */
    {"none", NULL},
    {NULL, NULL},
};
/* *INDENT-ON* */

struct form_list *
newFormList(char *action, char *method, char *charset, char *enctype,
	    char *target, char *name, struct form_list *_next)
{
    struct form_list *l;
    Str a = Strnew_charp(action);
    int m = FORM_METHOD_GET;
    int e = FORM_ENCTYPE_URLENCODED;
    int c = 0;

    if (method == NULL || !strcasecmp(method, "get"))
	m = FORM_METHOD_GET;
    else if (!strcasecmp(method, "post"))
	m = FORM_METHOD_POST;
    else if (!strcasecmp(method, "internal"))
	m = FORM_METHOD_INTERNAL;
    /* unknown method is regarded as 'get' */

    if (enctype != NULL && !strcasecmp(enctype, "multipart/form-data")) {
	e = FORM_ENCTYPE_MULTIPART;
	if (m == FORM_METHOD_GET)
	    m = FORM_METHOD_POST;
    }

    if (charset != NULL)
	c = *charset;

    l = New(struct form_list);
    l->item = l->lastitem = NULL;
    l->action = a;
    l->method = m;
    l->charset = c;
    l->enctype = e;
    l->target = target;
    l->name = name;
    l->next = _next;
    l->nitems = 0;
    l->body = NULL;
    l->length = 0;
    return l;
}

/* 
 * add <input> element to form_list
 */
struct form_item_list *
formList_addInput(struct form_list *fl, struct parsed_tag *tag)
{
    struct form_item_list *item;
    char *p;
    int i;

    /* if not in <form>..</form> environment, just ignore <input> tag */
    if (fl == NULL)
	return NULL;

    item = New(struct form_item_list);
    item->type = FORM_UNKNOWN;
    item->size = -1;
    item->rows = 0;
    item->checked = item->init_checked = 0;
    item->accept = 0;
    item->name = NULL;
    item->value = item->init_value = NULL;
    item->readonly = 0;
    if (parsedtag_get_value(tag, ATTR_TYPE, &p)) {
	item->type = formtype(p);
	if (item->size < 0 &&
	    (item->type == FORM_INPUT_TEXT ||
	     item->type == FORM_INPUT_FILE ||
	     item->type == FORM_INPUT_PASSWORD))
	    item->size = FORM_I_TEXT_DEFAULT_SIZE;
    }
    if (parsedtag_get_value(tag, ATTR_NAME, &p))
	item->name = Strnew_charp(p);
    if (parsedtag_get_value(tag, ATTR_VALUE, &p))
	item->value = item->init_value = Strnew_charp(p);
    item->checked = item->init_checked = parsedtag_exists(tag, ATTR_CHECKED);
    item->accept = parsedtag_exists(tag, ATTR_ACCEPT);
    parsedtag_get_value(tag, ATTR_SIZE, &item->size);
    parsedtag_get_value(tag, ATTR_MAXLENGTH, &item->maxlength);
    item->readonly = parsedtag_exists(tag, ATTR_READONLY);
    if (parsedtag_get_value(tag, ATTR_TEXTAREANUMBER, &i))
	item->value = item->init_value = textarea_str[i];
#ifdef MENU_SELECT
    if (parsedtag_get_value(tag, ATTR_SELECTNUMBER, &i))
	item->select_option = select_option[i].first;
#endif				/* MENU_SELECT */
    if (parsedtag_get_value(tag, ATTR_ROWS, &p))
	item->rows = atoi(p);
    if (item->type == FORM_UNKNOWN) {
	/* type attribute is missing. Ignore the tag. */
	return NULL;
    }
#ifdef MENU_SELECT
    if (item->type == FORM_SELECT) {
	chooseSelectOption(item, item->select_option);
	item->init_selected = item->selected;
	item->init_value = item->value;
	item->init_label = item->label;
    }
#endif				/* MENU_SELECT */
    if (item->type == FORM_INPUT_FILE && item->value && item->value->length) {
	/* security hole ! */
	return NULL;
    }
    item->parent = fl;
    item->next = NULL;
    if (fl->item == NULL) {
	fl->item = fl->lastitem = item;
    }
    else {
	fl->lastitem->next = item;
	fl->lastitem = item;
    }
    if (item->type == FORM_INPUT_HIDDEN)
	return NULL;
    fl->nitems++;
    return item;
}

static char *_formtypetbl[] = {
    "text", "password", "checkbox", "radio", "submit",
    "reset", "hidden", "image", "select", "textarea", "button", "file", 0,
};

static char *_formmethodtbl[] = {
    "GET", "POST", "INTERNAL", "HEAD"
};

char *
form2str(FormItemList *fi)
{
    Str tmp;
    if (fi->type == FORM_INPUT_SUBMIT ||
	fi->type == FORM_INPUT_IMAGE || fi->type == FORM_INPUT_BUTTON) {
	tmp = Strnew_charp(_formmethodtbl[fi->parent->method]);
	Strcat_char(tmp, ' ');
	Strcat(tmp, fi->parent->action);
	return tmp->ptr;
    }
    else
	return _formtypetbl[fi->type];
}

int
formtype(char *typestr)
{
    int i;
    for (i = 0; _formtypetbl[i]; i++) {
	if (!strcasecmp(typestr, _formtypetbl[i]))
	    return i;
    }
    return FORM_UNKNOWN;
}

void
formRecheckRadio(Anchor *a, Buffer *buf, FormItemList *fi)
{
    int i;
    Anchor *a2;
    FormItemList *f2;

    for (i = 0; i < buf->formitem->nanchor; i++) {
	a2 = &buf->formitem->anchors[i];
	f2 = (FormItemList *)a2->url;
	if (f2->parent == fi->parent && f2 != fi &&
	    f2->type == FORM_INPUT_RADIO && Strcmp(f2->name, fi->name) == 0) {
	    f2->checked = 0;
	    formUpdateBuffer(a2, buf, f2);
	}
    }
    fi->checked = 1;
    formUpdateBuffer(a, buf, fi);
}

void
formResetBuffer(Buffer *buf, AnchorList *formitem)
{
    int i;
    Anchor *a;
    FormItemList *f1, *f2;

    if (buf == NULL || buf->formitem == NULL || formitem == NULL)
	return;
    for (i = 0; i < buf->formitem->nanchor && i < formitem->nanchor; i++) {
	a = &buf->formitem->anchors[i];
	f1 = (FormItemList *)a->url;
	f2 = (FormItemList *)formitem->anchors[i].url;
	if (f1->type != f2->type ||
	    strcmp(((f1->name == NULL) ? "" : f1->name->ptr),
		   ((f2->name == NULL) ? "" : f2->name->ptr)))
	    break;		/* What's happening */
	switch (f1->type) {
	case FORM_INPUT_TEXT:
	case FORM_INPUT_PASSWORD:
	case FORM_INPUT_FILE:
	case FORM_TEXTAREA:
	    f1->value = f2->value;
	    break;
	case FORM_INPUT_CHECKBOX:
	case FORM_INPUT_RADIO:
	    f1->checked = f2->checked;
	    break;
	case FORM_SELECT:
#ifdef MENU_SELECT
	    f1->select_option = f2->select_option;
	    f1->label = f2->label;
#endif				/* MENU_SELECT */
	    break;
	default:
	    continue;
	}
	formUpdateBuffer(a, buf, f1);
    }
}

void
formUpdateBuffer(Anchor *a, Buffer *buf, FormItemList *form)
{
    int i, j, k;
    Buffer save;
    char *p;
    int spos, epos, c_len, rows, c_rows, pos, col = 0;
    Lineprop c_type;
    Line *l;

    copyBuffer(&save, buf);
    gotoLine(buf, a->start.line);
    switch (form->type) {
    case FORM_TEXTAREA:
    case FORM_INPUT_TEXT:
    case FORM_INPUT_FILE:
    case FORM_INPUT_PASSWORD:
    case FORM_INPUT_CHECKBOX:
    case FORM_INPUT_RADIO:
#ifdef MENU_SELECT
    case FORM_SELECT:
#endif				/* MENU_SELECT */
	spos = a->start.pos - 1;
	epos = a->end.pos;
	break;
    default:
	spos = a->start.pos;
	epos = a->end.pos - 1;
    }
    switch (form->type) {
    case FORM_INPUT_CHECKBOX:
    case FORM_INPUT_RADIO:
	if (form->checked)
	    buf->currentLine->lineBuf[spos + 1] = '*';
	else
	    buf->currentLine->lineBuf[spos + 1] = ' ';
	break;
    case FORM_INPUT_TEXT:
    case FORM_INPUT_FILE:
    case FORM_INPUT_PASSWORD:
    case FORM_TEXTAREA:
#ifdef MENU_SELECT
    case FORM_SELECT:
	if (form->type == FORM_SELECT) {
	    p = form->label->ptr;
	    updateSelectOption(form, form->select_option);
	}
	else
#endif				/* MENU_SELECT */
	    p = form->value->ptr;
	j = 0;
	l = buf->currentLine;
	if (form->type == FORM_TEXTAREA) {
	    int n = a->y - buf->currentLine->linenumber;
	    if (n > 0)
		for (; l && n; l = l->prev, n--) ;
	    else if (n < 0)
		for (; l && n; l = l->prev, n++) ;
	    if (!l)
		break;
	}
	rows = form->rows ? form->rows : 1;
	if (rows > 1)
	    col = COLPOS(l, a->start.pos);
	for (c_rows = 0; c_rows < rows; c_rows++, l = l->next) {
	    if (rows > 1) {
		pos = columnPos(l, col);
		a = retrieveAnchor(buf->formitem, l->linenumber, pos);
		if (a == NULL)
		    break;
		spos = a->start.pos - 1;
		epos = a->end.pos;
	    }
	    i = spos + 1;
	    while (p[j]) {
		if (rows > 1 && (p[j] == '\r' || p[j] == '\n'))
		    break;
		if (p[j] == '\r') {
		    j++;
		    continue;
		}
		c_type = get_mctype(&p[j]);
		c_len = get_mclen(c_type);
		k = i + c_len;
		if (k > epos)
		    break;
#ifdef JP_CHARSET
		if (c_type == PC_KANJI && form->type != FORM_INPUT_PASSWORD) {
		    SetCharType(l->propBuf[i], PC_KANJI1);
		    SetCharType(l->propBuf[i + 1], PC_KANJI2);
		}
		else
#endif				/* JP_CHARSET */
		    SetCharType(l->propBuf[i], PC_ASCII);

		for (; i < k; i++, j++) {
		    if (form->type == FORM_INPUT_PASSWORD)
			l->lineBuf[i] = '*';
		    else if (c_type == PC_CTRL ||
			     IS_UNPRINTABLE_ASCII(p[j], c_type))
			l->lineBuf[i] = ' ';
		    else
			l->lineBuf[i] = p[j];
		}
	    }
	    if (rows > 1) {
		if (!FoldTextarea) {
		    while (p[j] && p[j] != '\r' && p[j] != '\n')
			j++;
		}
		if (p[j] == '\r')
		    j++;
		if (p[j] == '\n')
		    j++;
	    }
	    for (; i < epos; i++) {
		l->lineBuf[i] = ' ';
		SetCharType(l->propBuf[i], PC_ASCII);
	    }
	}
	break;
    }
    copyBuffer(buf, &save);
}


Str
textfieldrep(Str s, int width)
{
    Lineprop c_type;
    Str n = Strnew_size(width + 2);
    int i, j, k, c_len;

    j = 0;
    for (i = 0; i < s->length; i += c_len) {
	c_type = get_mctype(&s->ptr[i]);
	c_len = get_mclen(c_type);
	if (s->ptr[i] == '\r') {
	    continue;
	}
	k = j + c_len;
	if (k > width)
	    break;
	if (IS_CNTRL(s->ptr[i])) {
	    Strcat_char(n, ' ');
	}
	else if (s->ptr[i] == '&')
	    Strcat_charp(n, "&amp;");
	else if (s->ptr[i] == '<')
	    Strcat_charp(n, "&lt;");
	else if (s->ptr[i] == '>')
	    Strcat_charp(n, "&gt;");
	else
	    Strcat_charp_n(n, &s->ptr[i], c_len);
	j = k;
    }
    for (; j < width; j++)
	Strcat_char(n, ' ');
    return n;
}

static void
form_fputs_decode(Str s, FILE * f)
{
    char *p;
    Str z = Strnew();

    for (p = s->ptr; *p;) {
	switch (*p) {
	case '<':
	    if (!strncasecmp(p, "<eol>", 5)) {
		Strcat_char(z, '\n');
		p += 5;
	    }
	    else {
		Strcat_char(z, *p);
		p++;
	    }
	    break;
#if !defined( __CYGWIN__ ) && !defined( __EMX__ )
	case '\r':
	    if (*(p + 1) == '\n')
		p++;
	    /* continue to the next label */
#endif				/* !defined( __CYGWIN__ ) && !defined( __EMX__ 
				 * ) */
	default:
	    Strcat_char(z, *p);
	    p++;
	    break;
	}
    }
#ifdef JP_CHARSET
    fputs(conv_str(z, InnerCode, DisplayCode)->ptr, f);
#else				/* not JP_CHARSET */
    fputs(z->ptr, f);
#endif				/* not JP_CHARSET */
}


void
input_textarea(FormItemList *fi)
{
    char *tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
    Str tmp;
    FILE *f;
#ifdef JP_CHARSET
    char code = DisplayCode, ic;
#endif

    f = fopen(tmpf, "w");
    if (f == NULL) {
	disp_err_message("Can't open temporary file", FALSE);
	return;
    }
    if (fi->value)
	form_fputs_decode(fi->value, f);
    fclose(f);

    fmTerm();
    system(myEditor(Editor, tmpf, 1)->ptr);
    fmInit();

    if (fi->readonly)
	return;
    f = fopen(tmpf, "r");
    if (f == NULL) {
	disp_err_message("Can't open temporary file", FALSE);
	return;
    }
    fi->value = Strnew();
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->length == 1 && tmp->ptr[tmp->length - 1] == '\n') {
	    /* null line with bare LF */
	    tmp = Strnew_charp("\r\n");
	}
	else if (tmp->length > 1 && tmp->ptr[tmp->length - 1] == '\n' &&
		 tmp->ptr[tmp->length - 2] != '\r') {
	    Strshrink(tmp, 1);
	    Strcat_charp(tmp, "\r\n");
	}
#ifdef JP_CHARSET
	if ((ic = checkShiftCode(tmp, code)) != '\0')
	    tmp = conv_str(tmp, (code = ic), InnerCode);
#endif				/* not JP_CHARSET */
	Strcat(fi->value, tmp);
    }
    fclose(f);
    unlink(tmpf);
}

void
do_internal(char *action, char *data)
{
    int i;

    for (i = 0; internal_action[i].action; i++) {
	if (strcasecmp(internal_action[i].action, action) == 0) {
	    if (internal_action[i].rout)
		internal_action[i].rout(cgistr2tagarg(data));
	    return;
	}
    }
}

#ifdef MENU_SELECT
void
addSelectOption(FormSelectOption *fso, Str value, Str label, int chk)
{
    FormSelectOptionItem *o;
    o = New(FormSelectOptionItem);
    if (value == NULL)
	value = label;
    o->value = value;
    Strremovefirstspaces(label);
    Strremovetrailingspaces(label);
    o->label = label;
    o->checked = chk;
    o->next = NULL;
    if (fso->first == NULL)
	fso->first = fso->last = o;
    else {
	fso->last->next = o;
	fso->last = o;
    }
}

void
chooseSelectOption(FormItemList *fi, FormSelectOptionItem *item)
{
    FormSelectOptionItem *opt;
    int i;

    fi->selected = 0;
    if (item == NULL) {
	fi->value = Strnew_size(0);
	fi->label = Strnew_size(0);
	return;
    }
    fi->value = item->value;
    fi->label = item->label;
    for (i = 0, opt = item; opt != NULL; i++, opt = opt->next) {
	if (opt->checked) {
	    fi->value = opt->value;
	    fi->label = opt->label;
	    fi->selected = i;
	    break;
	}
    }
    updateSelectOption(fi, item);
}

void
updateSelectOption(FormItemList *fi, FormSelectOptionItem *item)
{
    int i;

    if (fi == NULL || item == NULL)
	return;
    for (i = 0; item != NULL; i++, item = item->next) {
	if (i == fi->selected)
	    item->checked = TRUE;
	else
	    item->checked = FALSE;
    }
}

int
formChooseOptionByMenu(struct form_item_list *fi, int x, int y)
{
    int i, n, selected = -1, init_select = fi->selected;
    FormSelectOptionItem *opt;
    char **label;

    for (n = 0, opt = fi->select_option; opt != NULL; n++, opt = opt->next) ;
    label = New_N(char *, n + 1);
    for (i = 0, opt = fi->select_option; opt != NULL; i++, opt = opt->next)
	label[i] = opt->label->ptr;
    label[n] = NULL;

    optionMenu(x, y, label, &selected, init_select, NULL);

    if (selected < 0)
	return 0;
    for (i = 0, opt = fi->select_option; opt != NULL; i++, opt = opt->next) {
	if (i == selected) {
	    fi->selected = selected;
	    fi->value = opt->value;
	    fi->label = opt->label;
	    break;
	}
    }
    updateSelectOption(fi, fi->select_option);
    return 1;
}
#endif				/* MENU_SELECT */

void
form_write_data(FILE * f, char *boundary, char *name, char *value)
{
    fprintf(f, "--%s\r\n", boundary);
    fprintf(f, "Content-Disposition: form-data; name=\"%s\"\r\n\r\n", name);
    fprintf(f, "%s\r\n", value);
}

void
form_write_from_file(FILE * f, char *boundary, char *name, char *filename,
		     char *file)
{
    FILE *fd;
    struct stat st;
    int c;
    char *type;

    fprintf(f, "--%s\r\n", boundary);
    fprintf(f,
	    "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",
	    name, mybasename(filename));
    type = guessContentType(file);
    fprintf(f, "Content-Type: %s\r\n\r\n",
	    type ? type : "application/octet-stream");

#ifdef HAVE_LSTAT
    if (lstat(file, &st) < 0)
	goto write_end;
#endif				/* HAVE_LSTAT */
    if (S_ISDIR(st.st_mode))
	goto write_end;
    fd = fopen(file, "r");
    if (fd != NULL) {
	while ((c = fgetc(fd)) != EOF)
	    fputc(c, f);
	fclose(fd);
    }
  write_end:
    fprintf(f, "\r\n");
}
