/* $Id: w3mhelperpanel.c,v 1.14 2007/05/31 01:19:50 inu Exp $ */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "Str.h"
#include "indep.h"
#include "textlist.h"
#include "parsetag.h"
#include "myctype.h"

#if LANG == JA
/* FIXME: gettextize here */
#define MSG_TITLE		"外部ビューアの編集"
#define MSG_NEW_ENTRY		"新規登録"
#define MSG_TYPE		"データタイプ"
#define MSG_COMMAND		"外部コマンド"
#define MSG_REGISTER		"登録"
#define MSG_DELETE		"削除"
#define MSG_DOIT		"実行"
#else				/* LANG != JA */
#define MSG_TITLE		"External Viewers Setup"
#define MSG_NEW_ENTRY		"New Entry"
#define MSG_TYPE		"Type"
#define MSG_COMMAND		"Command"
#define MSG_REGISTER		"Register"
#define MSG_DELETE		"Delete"
#define MSG_DOIT		"Do it"
#endif				/* LANG != JA */

char *local_cookie;

void
extractMailcapEntry(char *mcap_entry, char **type, char **cmd)
{
    int j;

    while (*mcap_entry && IS_SPACE(*mcap_entry))
	mcap_entry++;
    for (j = 0;
	 mcap_entry[j] && mcap_entry[j] != ';' && !IS_SPACE(mcap_entry[j]);
	 j++) ;
    *type = allocStr(mcap_entry, j);
    if (mcap_entry[j] == ';')
	j++;
    while (mcap_entry[j] && IS_SPACE(mcap_entry[j]))
	j++;
    *cmd = allocStr(&mcap_entry[j], -1);
}

static void
bye(const char *action, const char *mailcap)
{
    printf("Content-Type: text/plain\n\n%s %s\n", action, mailcap);
    exit(1);
}

void
printMailcapPanel(char *mailcap)
{
    FILE *f;
    Str tmp;
    char *type, *viewer;

    if ((f = fopen(mailcap, "rt")) == NULL) {
	if (errno != ENOENT)
	    bye("Can't open", mailcap);

	if (!(f = fopen(mailcap, "a+")))	/* if $HOME/.mailcap is not found, make it now! */
	    bye("Can't open", mailcap);

	{
	    char *SysMailcap = getenv("SYS_MAILCAP");
	    FILE *s = fopen(SysMailcap ? SysMailcap : "/etc/mailcap", "r");
	    if (s) {
		char buffer[256];
		while (fgets(buffer, sizeof buffer, s))	/* Copy system mailcap to */
		    fputs(buffer, f);	/* users' new one         */
		fclose(s);
		rewind(f);
	    }
	}
    }
#if LANG == JA
    /* FIXME: gettextize here */
    printf("Content-Type: text/html; charset=EUC-JP\n\n");
#else
    printf("Content-Type: text/html\n\n");
#endif
    printf("<html>\n<head>\n<title>%s</title>\n</head>\n<body>\n<h1>%s</h1>\n",
	   MSG_TITLE, MSG_TITLE);
    printf("<form method=post action=\"file:///$LIB/" W3MHELPERPANEL_CMDNAME
	   "\">\n");
    printf("<input type=hidden name=mode value=edit>\n");
    printf("<input type=hidden name=cookie value=\"%s\">\n",
	   html_quote(local_cookie));
    printf("<table>\n<tr><td>%s:<td>%s=<input type=text name=newtype size=40>\n\
<tr><td><td>%s=<input type=text name=newcmd size=40>\n\
<tr><td><input type=submit name=submit value=\"%s\">\n</table>\n",
	   MSG_NEW_ENTRY, MSG_TYPE, MSG_COMMAND, MSG_REGISTER);
    printf("<p><hr width=50%%><p>\n<table border='0' cellpadding='0'>\n\
<tr><th align=left><b>%s</b><th><b>%s</b>\n",
	   MSG_TYPE, MSG_COMMAND);
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] == '#')
	    continue;
	Strchop(tmp);
	extractMailcapEntry(tmp->ptr, &type, &viewer);
	printf("<tr valign=top><td>%s<td>%s<td nowrap>", html_quote(type),
	       html_quote(viewer));
	printf("<input type=checkbox name=delete value=\"%s\">%s\n",
	       html_quote(type), MSG_DELETE);
    }
    printf("</table>\n<input type=submit name=submit value=\"%s\">\n</form>\n\
</body>\n</html>\n",
	   MSG_DOIT);
}

void
editMailcap(char *mailcap, struct parsed_tagarg *args)
{
    TextList *t = newTextList();
    TextListItem *ti;
    FILE *f;
    Str tmp;
    char *type, *viewer;
    struct parsed_tagarg *a;
    int delete_it;

    if ((f = fopen(mailcap, "rt")) == NULL)
	bye("Can't open", mailcap);

    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] == '#')
	    continue;
	Strchop(tmp);
	extractMailcapEntry(tmp->ptr, &type, &viewer);
	delete_it = 0;
	for (a = args; a != NULL; a = a->next) {
	    if (!strcmp(a->arg, "delete") && !strcmp(a->value, type)) {
		delete_it = 1;
		break;
	    }
	}
	if (!delete_it)
	    pushText(t, Sprintf("%s;\t%s\n", type, viewer)->ptr);
    }
    type = tag_get_value(args, "newtype");
    viewer = tag_get_value(args, "newcmd");
    if (type != NULL && *type != '\0' && viewer != NULL && *viewer != '\0')
	pushText(t, Sprintf("%s;\t%s\n", type, viewer)->ptr);
    fclose(f);
    if ((f = fopen(mailcap, "w")) == NULL)
	bye("Can't write to", mailcap);

    for (ti = t->first; ti != NULL; ti = ti->next)
	fputs(ti->ptr, f);
    fclose(f);
    printf("Content-Type: text/plain\n");
    printf("w3m-control: BACK\nw3m-control: BACK\n");
    printf("w3m-control: REINIT MAILCAP\n");
}

int
main(int argc, char *argv[], char **envp)
{
    Str mailcapfile;
    extern char *getenv();
    char *p;
    int length;
    Str qs = NULL;
    struct parsed_tagarg *cgiarg;
    char *mode;
    char *sent_cookie;

    GC_INIT();
    p = getenv("REQUEST_METHOD");
    if (p == NULL || strcasecmp(p, "post"))
	goto request_err;
    p = getenv("CONTENT_LENGTH");
    if (p == NULL || (length = atoi(p)) <= 0)
	goto request_err;

    qs = Strfgets(stdin);
    Strchop(qs);
    if (qs->length != length)
	goto request_err;
    cgiarg = cgistr2tagarg(qs->ptr);

    p = getenv("LOCAL_COOKIE_FILE");
    if (p) {
	FILE *f = fopen(p, "r");
	if (f) {
	    local_cookie = Strfgets(f)->ptr;
	    fclose(f);
	}
    }
    sent_cookie = tag_get_value(cgiarg, "cookie");
    if (local_cookie == NULL || sent_cookie == NULL ||
	strcmp(local_cookie, sent_cookie) != 0) {
	/* Local cookie doesn't match */
	bye("Local cookie doesn't match: It may be an illegal execution", "");
    }

    mode = tag_get_value(cgiarg, "mode");
    mailcapfile = Strnew_charp(expandPath(USER_MAILCAP));
    if (mode && !strcmp(mode, "edit")) {
	char *referer;
	/* check if I can edit my mailcap */
	if ((referer = getenv("HTTP_REFERER")) != NULL) {
	    if (strncmp(referer, "file://", 7) != 0 &&
		strncmp(referer, "exec://", 7) != 0) {
		/* referer is not file: nor exec: */
		bye("It may be an illegal execution\n referer=", referer);
	    }
	}
	/* edit mailcap */
	editMailcap(mailcapfile->ptr, cgiarg);
    }
    else {
	/* initial panel */
	printMailcapPanel(mailcapfile->ptr);
    }
    return 0;

  request_err:
    bye("Incomplete Request:", qs ? qs->ptr : "(null)");
    exit(1);
}
