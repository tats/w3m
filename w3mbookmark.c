#ifdef __EMX__
#include <stdlib.h>
#endif
#include <stdio.h>
#include "config.h"
#include "Str.h"
#include "indep.h"
#include "textlist.h"
#include "parsetag.h"

#include "gcmain.c"

#if LANG == JA
static char *bkmark_src1 = "<html><head><title>Bookmark Registration</title>\n\
<body><h1>•÷•√•Ø•ﬁ°º•Ø§Œ≈–œø</h1>\n\n"
#ifdef __EMX__
"<form method=get action=\"file://%s/w3mbookmark.exe\">\n\n"
#else
"<form method=get action=\"file://%s/w3mbookmark\">\n\n"
#endif
"<input type=hidden name=mode value=register>\n\
<input type=hidden name=bmark value=\"%s\">\n\
<table cellpadding=0>\n";

static char *bkmark_src2 = "<tr><td>New Section:</td><td><input type=text name=newsection width=60></td></tr>\n\
<tr><td>URL:</td><td><input type=text name=url value=\"%s\" width=60></td></tr>\n\
<tr><td>Title:</td><td><input type=text name=title value=\"%s\" width=60></td></tr>\n\
<tr><td><input type=submit name=submit value=\"≈–œø\"></td>\n\
</table>\n\
<input type=hidden name=cookie value=\"%s\">\
</form>\
</body></html>\n";
static char *default_section = "Ã§ ¨Œ‡";
#else				/* LANG != JA */
static char *bkmark_src1 = "<html><head><title>Bookmark Registration</title>\n\
<body><h1>Register to my bookmark</h1>\n\n"
#ifdef __EMX__
"<form method=get action=\"file://%s/w3mbookmark.exe\">\n\n"
#else
"<form method=get action=\"file://%s/w3mbookmark\">\n\n"
#endif
"<input type=hidden name=mode value=register>\n\
<input type=hidden name=bmark value=\"%s\">\n\
<table cellpadding=0>\n";

static char *bkmark_src2 = "<tr><td>New Section:</td><td><input type=text name=newsection width=60></td></tr>\n\
<tr><td>URL:</td><td><input type=text name=url value=\"%s\" width=60></td></tr>\n\
<tr><td>Title:</td><td><input type=text name=title value=\"%s\" width=60></td></tr>\n\
<tr><td><input type=submit name=submit value=\"ADD\"></td>\n\
</table>\n\
<input type=hidden name=cookie value=\"%s\">\
</form>\
</body></html>\n";
static char *default_section = "Miscellaneous";
#endif				/* LANG != JA */

#define FALSE 0
#define T   1

static char end_section[] = "<!--End of section (do not delete this comment)-->\n";

char *Local_cookie;

#ifdef __EMX__
static char *
lib_dir()
{
    char *value = getenv("W3M_LIB_DIR");
    return value ? value : LIB_DIR;
}
#else
#define lib_dir() LIB_DIR
#endif

void
print_bookmark_panel(char *bmark, char *url, char *title)
{
    Str tmp, tmp2;
    FILE *f;
    char *p;

    printf("Content-Type: text/html\n\n");
    printf(bkmark_src1, lib_dir(), bmark);
    if ((f = fopen(bmark, "r")) != NULL) {
	printf("<tr><td>Section:<td><select name=\"section\">\n");
	while (tmp = Strfgets(f), tmp->length > 0) {
           Strremovefirstspaces(tmp);
	    if (Strncasecmp_charp(tmp, "<h2>", 4) == 0) {
		p = tmp->ptr + 4;
		tmp2 = Strnew();
		while (*p && *p != '<')
		    Strcat_char(tmp2, *p++);
		printf("<option value=\"%s\">%s</option>", tmp2->ptr, tmp2->ptr);
	    }
	}
	printf("</select>\n");
    }
    printf(bkmark_src2, html_quote(url), html_quote(title),Local_cookie);
}

/* create new bookmark */
static int
create_new_bookmark(char *bmark, char *section, char *title, char *url, char *mode)
{
    FILE *f;
    f = fopen(bmark, mode);
    if (f == NULL) {
	printf("\nCan't open bookmark %s\n",bmark);
	return FALSE;
    }
    else {
	fprintf(f, "<html><head><title>Bookmarks</title></head>\n");
	fprintf(f, "<body>\n<h1>Bookmarks</h1>\n");
	fprintf(f, "<h2>%s</h2>\n<ul>\n", section);
	fprintf(f, "<li><a href=\"%s\">%s</a>\n", url, title);
	fprintf(f, end_section);
	fprintf(f, "</ul>\n</body>\n</html>\n");
	fclose(f);
    }
    return TRUE;
}

int
insert_bookmark(char *bmark, struct parsed_tagarg *data)
{
    char *url, *title, *section;
    FILE *f;
    TextList *tl = newTextList();
    int section_found = 0;
    int bmark_added = 0;
    Str tmp, section_tmp;

    url = tag_get_value(data, "url");
    title = tag_get_value(data, "title");
    section = tag_get_value(data, "newsection");
    if (section == NULL || *section == '\0')
	section = tag_get_value(data, "section");
    if (section == NULL || *section == '\0')
        section = default_section;

    if (url == NULL || *url == '\0' ||
	title == NULL || *title == '\0') {
	/* Bookmark not added */
	return FALSE;
    }
    url = html_quote(url);
    title = html_quote(title);
    section = html_quote(section);

    f = fopen(bmark, "r");
    if (f == NULL)
	return create_new_bookmark(bmark,section,title,url,"w");

    section_tmp = Sprintf("<h2>%s</h2>\n", section);
    for (;;) {
	tmp = Strfgets(f);
	if (tmp->length == 0)
	    break;
	if (Strcasecmp(tmp, section_tmp) == 0)
	    section_found = 1;
       if (section_found && !bmark_added) {
           Strremovefirstspaces(tmp);
           if (Strcmp_charp(tmp, end_section) == 0) {
        pushText(tl, Sprintf("<li><a href=\"%s\">%s</a>\n", url, title)->ptr);
        bmark_added = 1;
           }
	}
	if (!bmark_added && Strcasecmp_charp(tmp, "</body>\n") == 0) {
	    pushText(tl, Sprintf("<h2>%s</h2>\n<ul>\n", section)->ptr);
	    pushText(tl, Sprintf("<li><a href=\"%s\">%s</a>\n", url, title)->ptr);
	    pushText(tl, end_section);
	    pushText(tl, "</ul>\n");
           bmark_added = 1;
	}
	pushText(tl, tmp->ptr);
    }
    fclose(f);
    if (!bmark_added) {
	/* Bookmark not added; perhaps the bookmark file is ill-formed */
	/* In this case, a new bookmark is appeneded after the bookmark file */
	return create_new_bookmark(bmark,section,title,url,"a");
    }
    f = fopen(bmark, "w");
    while (tl->nitem) {
	fputs(popText(tl), f);
    }
    fclose(f);
    return TRUE;
}

int
MAIN(int argc, char *argv[], char **envp)
{
    extern char *getenv();
    char *qs;
    struct parsed_tagarg *cgiarg;
    char *mode;
    char *bmark;
    char *url;
    char *title;
    char *sent_cookie;

    if ((qs = getenv("QUERY_STRING")) == NULL) {
	printf("Content-Type: text/plain\n\n");
	printf("Incomplete Request: no QUERY_STRING\n");
	exit(1);
    }

    cgiarg = cgistr2tagarg(qs);
    mode = tag_get_value(cgiarg, "mode");
    bmark = expandPath(tag_get_value(cgiarg, "bmark"));
    url = tag_get_value(cgiarg, "url");
    title = tag_get_value(cgiarg, "title");
    if (bmark == NULL || url == NULL) {
	/* incomplete request */
	printf("Content-Type: text/plain\n\n");
	printf("Incomplete Request: QUERY_STRING=%s\n",qs);
	exit(1);
    }
    Local_cookie = getenv("LOCAL_COOKIE");
    sent_cookie = tag_get_value(cgiarg,"cookie");
    if (Local_cookie == NULL) {
	/* Local cookie not provided: maybe illegal invocation */
	Local_cookie = "";
    }
    if (mode && !strcmp(mode, "panel")) {
	if (title == NULL)
	    title = "";
	print_bookmark_panel(bmark, url, title);
    }
    else if (mode && !strcmp(mode, "register")) {
	printf("Content-Type: text/plain\n");
	if (sent_cookie == NULL || Local_cookie[0] == '\0' ||
	    strcmp(sent_cookie,Local_cookie) != 0) {
	    /* local cookie doesn't match: It may be an illegal invocation */
	    printf("\nBookmark not added: local cookie doesn't match\n");
	}
	else if (insert_bookmark(bmark, cgiarg)) {
	    printf("w3m-control: BACK\n");
	    printf("w3m-control: BACK\n\n");
	}
    }
    return 0;
}
