/* $Id: backend.c,v 1.15 2010/08/08 09:53:42 htrb Exp $ */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "fm.h"
#include <gc.h>
#include "terms.h"


/* Prototype declaration of internal functions */
#ifdef HAVE_READLINE
#include <readline/readline.h>
#else				/* ! HAVE_READLINE */
static char *readline(char *);
#endif				/* ! HAVE_READLINE */
static TextList *split(char *);


/* Prototype declaration of command functions */
static void get(TextList *);
static void post(TextList *);
static void set(TextList *);
static void show(TextList *);
static void quit(TextList *);
static void help(TextList *);


/* *INDENT-OFF* */
/* Table of command functions */
struct {
    const char *name;
    const char *option_string;
    const char *help;
    void (*func)(TextList*);
} command_table[] = {
    {"get", "[-download_only] URL", "Retrieve URL.", get},
    {"post", "[-download_only] [-target TARGET] [-charset CHARSET]"
     " [-enctype ENCTYPE] [-body BODY] [-boundary BOUNDARY] [-length LEN] URL",
     "Retrieve URL.", post},
    {"set", "VARIABLE VALUE", "Set VALUE to VARIABLE.", set},
    {"show", "VARIABLE", "Show value of VARIABLE.", show},
    {"quit", "", "Quit program.", quit},
    {"help", "", "Display help messages.", help},
    {NULL, NULL, NULL, NULL},
};
/* *INDENT-ON* */

/* Prototype declaration of functions to manipulate configuration variables */
static void set_column(TextList *);
static void show_column(TextList *);


/* *INDENT-OFF* */
/* Table of configuration variables */
struct {
    const char *name;
    void (*set_func)(TextList*);
    void (*show_func)(TextList*);
} variable_table[] = {
    {"column", set_column, show_column},
    {NULL, NULL, NULL},
};
/* *INDENT-ON* */

static void
print_headers(Buffer *buf, int len)
{
    TextListItem *tp;

    if (buf->document_header) {
	for (tp = buf->document_header->first; tp; tp = tp->next)
	    printf("%s\n", tp->ptr);
    }
    printf("w3m-current-url: %s\n", parsedURL2Str(&buf->currentURL)->ptr);
    if (buf->baseURL)
	printf("w3m-base-url: %s\n", parsedURL2Str(buf->baseURL)->ptr);
    printf("w3m-content-type: %s\n", buf->type);
#ifdef USE_M17N
    if (buf->document_charset)
	printf("w3m-content-charset: %s\n",
	       wc_ces_to_charset(buf->document_charset));
#endif
    if (len > 0)
	printf("w3m-content-length: %d\n", len);
}


static void
internal_get(char *url, int flag, FormList *request)
{
    Buffer *buf;

    backend_halfdump_buf = NULL;
    do_download = flag;
    buf = loadGeneralFile(url, NULL, NO_REFERER, 0, request);
    do_download = FALSE;
    if (buf != NULL && buf != NO_BUFFER) {
	if (is_html_type(buf->type) && backend_halfdump_buf) {
	    TextLineListItem *p;
	    Str first, last;
	    int len = 0;
	    for (p = backend_halfdump_buf->first; p; p = p->next) {
		p->ptr->line = Str_conv_to_halfdump(p->ptr->line);
		len += p->ptr->line->length + 1;
	    }
	    first = Strnew_charp("<pre>\n");
	    last = Strnew_m_charp("</pre><title>", html_quote(buf->buffername),
				  "</title>\n", NULL);
	    print_headers(buf, len + first->length + last->length);
	    printf("\n");
	    printf("%s", first->ptr);
	    for (p = backend_halfdump_buf->first; p; p = p->next)
		printf("%s\n", p->ptr->line->ptr);
	    printf("%s", last->ptr);
	}
	else {
	    if (!strcasecmp(buf->type, "text/plain")) {
		Line *lp;
		int len = 0;
		for (lp = buf->firstLine; lp; lp = lp->next) {
		    len += lp->len;
		    if (lp->lineBuf[lp->len - 1] != '\n')
			len++;
		}
		print_headers(buf, len);
		printf("\n");
		saveBuffer(buf, stdout, TRUE);
	    }
	    else {
		print_headers(buf, 0);
	    }
	}
    }
}


/* Command: get */
static void
get(TextList *argv)
{
    char *p, *url = NULL;
    int flag = FALSE;

    while ((p = popText(argv))) {
	if (!strcasecmp(p, "-download_only"))
	    flag = TRUE;
	else
	    url = p;
    }
    if (url) {
	internal_get(url, flag, NULL);
    }
}


/* Command: post */
static void
post(TextList *argv)
{
    FormList *request;
    char *p, *target = NULL, *charset = NULL,
	*enctype = NULL, *body = NULL, *boundary = NULL, *url = NULL;
    int flag = FALSE, length = 0;

    while ((p = popText(argv))) {
	if (!strcasecmp(p, "-download_only"))
	    flag = TRUE;
	else if (!strcasecmp(p, "-target"))
	    target = popText(argv);
	else if (!strcasecmp(p, "-charset"))
	    charset = popText(argv);
	else if (!strcasecmp(p, "-enctype"))
	    enctype = popText(argv);
	else if (!strcasecmp(p, "-body"))
	    body = popText(argv);
	else if (!strcasecmp(p, "-boundary"))
	    boundary = popText(argv);
	else if (!strcasecmp(p, "-length"))
	    length = atol(popText(argv));
	else
	    url = p;
    }
    if (url) {
	request =
	    newFormList(NULL, "post", charset, enctype, target, NULL, NULL);
	request->body = body;
	request->boundary = boundary;
	request->length = (length > 0) ? length : (body ? strlen(body) : 0);
	internal_get(url, flag, request);
    }
}


/* Command: set */
static void
set(TextList *argv)
{
    if (argv->nitem > 1) {
	int i;
	for (i = 0; variable_table[i].name; i++) {
	    if (!strcasecmp(variable_table[i].name, argv->first->ptr)) {
		popText(argv);
		if (variable_table[i].set_func)
		    variable_table[i].set_func(argv);
		break;
	    }
	}
    }
}


/* Command: show */
static void
show(TextList *argv)
{
    if (argv->nitem >= 1) {
	int i;
	for (i = 0; variable_table[i].name; i++) {
	    if (!strcasecmp(variable_table[i].name, argv->first->ptr)) {
		popText(argv);
		if (variable_table[i].show_func)
		    variable_table[i].show_func(argv);
		break;
	    }
	}
    }
}


/* Command: quit */
static void
quit(TextList *argv)
{
#ifdef USE_COOKIE
    save_cookies();
#endif				/* USE_COOKIE */
    w3m_exit(0);
}


/* Command: help */
static void
help(TextList *argv)
{
    int i;
    for (i = 0; command_table[i].name; i++)
	printf("%s %s\n    %s\n",
	       command_table[i].name,
	       command_table[i].option_string, command_table[i].help);
}


/* Sub command: set COLS */
static void
set_column(TextList *argv)
{
    if (argv->nitem == 1) {
	COLS = atol(argv->first->ptr);
    }
}

/* Sub command: show COLS */
static void
show_column(TextList *argv)
{
    fprintf(stdout, "column=%d\n", COLS);
}


/* Call appropriate command function based on given string */
static void
call_command_function(char *str)
{
    int i;
    TextList *argv = split(str);
    if (argv->nitem > 0) {
	for (i = 0; command_table[i].name; i++) {
	    if (!strcasecmp(command_table[i].name, argv->first->ptr)) {
		popText(argv);
		if (command_table[i].func)
		    command_table[i].func(argv);
		break;
	    }
	}
    }
}


/* Main function */
int
backend(void)
{
    char *str;

    w3m_dump = 0;
    if (COLS == 0)
	COLS = DEFAULT_COLS;
#ifdef USE_MOUSE
    use_mouse = FALSE;
#endif				/* USE_MOUSE */

    if (backend_batch_commands) {
	while ((str = popText(backend_batch_commands)))
	    call_command_function(str);
    }
    else {
	while ((str = readline("w3m> ")))
	    call_command_function(str);
    }
    quit(NULL);
    return 0;
}


/* Dummy function of readline(). */
#ifndef HAVE_READLINE
static char *
readline(char *prompt)
{
    Str s;
    fputs(prompt, stdout);
    fflush(stdout);
    s = Strfgets(stdin);
    if (feof(stdin) && (strlen(s->ptr) == 0))
	return NULL;
    else
	return s->ptr;
}
#endif				/* ! HAVE_READLINE */


/* Splits a string into a list of tokens and returns that list. */
static TextList *
split(char *p)
{
    int in_double_quote = FALSE, in_single_quote = FALSE;
    Str s = Strnew();
    TextList *tp = newTextList();

    for (; *p; p++) {
	switch (*p) {
	case '"':
	    if (in_single_quote)
		Strcat_char(s, '"');
	    else
		in_double_quote = !in_double_quote;
	    break;
	case '\'':
	    if (in_double_quote)
		Strcat_char(s, '\'');
	    else
		in_single_quote = !in_single_quote;
	    break;
	case '\\':
	    if (!in_single_quote) {
		/* Process escape characters. */
		p++;
		switch (*p) {
		case 't':
		    Strcat_char(s, '\t');
		    break;
		case 'r':
		    Strcat_char(s, '\r');
		    break;
		case 'f':
		    Strcat_char(s, '\f');
		    break;
		case 'n':
		    Strcat_char(s, '\n');
		    break;
		case '\0':
		    goto LAST;
		default:
		    Strcat_char(s, *p);
		}
	    }
	    else {
		Strcat_char(s, *p);
	    }
	    break;
	case ' ':
	case '\t':
	case '\r':
	case '\f':
	case '\n':
	    /* Separators are detected. */
	    if (in_double_quote || in_single_quote) {
		Strcat_char(s, *p);
	    }
	    else if (s->length > 0) {
		pushText(tp, s->ptr);
		s = Strnew();
	    }
	    break;
	default:
	    Strcat_char(s, *p);
	}
    }
  LAST:
    if (s->length > 0)
	pushText(tp, s->ptr);
    return tp;
}
