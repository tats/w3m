#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "Str.h"
#include "indep.h"
#include "textlist.h"
#include "parsetag.h"
#include "myctype.h"

#include "gcmain.c"

#if LANG == JA
#define MSG_TITLE		"外部ビューアの編集"
#define MSG_NEW_ENTRY		"新規登録"
#define MSG_TYPE		"データタイプ"
#define MSG_COMMAND		"外部コマンド"
#define MSG_REGISTER		"登録"
#define MSG_DELETE		"削除"
#define MSG_DOIT		"実行"
#else				/* LANG != JA */
#define MSG_TITLE		"External Viewers"
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
    int j, k;

    while (*mcap_entry && IS_SPACE(*mcap_entry))
	mcap_entry++;
    for (j = 0; mcap_entry[j] && mcap_entry[j] != ';' && !IS_SPACE(mcap_entry[j]);
	 j++);
    *type = allocStr(mcap_entry, j);
    if (mcap_entry[j] == ';')
	j++;
    while (mcap_entry[j] && IS_SPACE(mcap_entry[j]))
	j++;
    *cmd = allocStr(&mcap_entry[j], 0);
}

static void
bye(const char*action,const char*mailcap)
{   printf("Content-Type: text/plain\n\n%s %s\n", action, mailcap);
    exit(1);
}

void
printMailcapPanel(char *mailcap)
{
    FILE *f;
    Str tmp;
    char *type, *viewer;

    if ((f = fopen(mailcap, "rt")) == NULL) {
	if(errno!=ENOENT)
	    bye("Can't open",mailcap);

	if(!(f=fopen(mailcap,"a+")))	/* if $HOME/.mailcap is not found, make it now! */
	    bye("Can't open",mailcap);

	{   char* SysMailcap=getenv("SYS_MAILCAP");
	    FILE* s = fopen(SysMailcap?SysMailcap:"/etc/mailcap","r");
	    if (s) {
		char buffer[256];
		while(fgets(buffer,sizeof buffer,s))	/* Copy system mailcap to */
		    fputs(buffer,f);			/* users' new one         */
		fclose(s);
		rewind(f);
	    }
	}
    }
    printf("Content-Type: text/html\n\n");
    printf("<html><head><title>External Viewer Setup</title></head><body><h1>%s</h1>\n", MSG_TITLE);
#ifdef __EMX__
    printf("<form method=get action=\"file:///$LIB/w3mhelperpanel.exe\">\n");
#else
    printf("<form method=get action=\"file:///$LIB/w3mhelperpanel\">\n");
#endif
    printf("<input type=hidden name=mode value=edit>\n");
    printf("<input type=hidden name=cookie value=\"%s\">\n",local_cookie);
    printf("%s: %s=<input type=text name=newtype><br>%s=<input type=text name=newcmd><br><input type=submit name=submit value=\"%s\">\n",
	   MSG_NEW_ENTRY, MSG_TYPE, MSG_COMMAND, MSG_REGISTER);
    printf("<p><hr width=50%%><p><table border='0' cellpadding='0'><tr><th>&nbsp;&nbsp;<th><b>%s</b><th><b>%s</b>\n",
	   MSG_TYPE, MSG_COMMAND);
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] == '#')
	    continue;
	Strchop(tmp);
	extractMailcapEntry(tmp->ptr, &type, &viewer);
	printf("<tr valign=top><td><td>%s<td>%s<td>", htmlquote_str(type), htmlquote_str(viewer));
	printf("<input type=checkbox name=delete value=\"%s\">%s\n",
	       htmlquote_str(type), MSG_DELETE);
    }
    printf("</table><input type=submit name=submit value=\"%s\"></form></body></html>\n",
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
	bye("Can't open",mailcap);

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
	bye("Can't write to",mailcap);

    for (ti = t->first; ti != NULL; ti = ti->next)
	fputs(ti->ptr, f);
    fclose(f);
    printf("Content-Type: text/plain\n");
    printf("w3m-control: BACK\nw3m-control: BACK\n");
    printf("w3m-control: INIT_MAILCAP\n");
}

int
MAIN(int argc, char *argv[], char **envp)
{
    Str mailcapfile;
    extern char *getenv();
    char *qs;
    struct parsed_tagarg *cgiarg;
    char *mode;
    char *sent_cookie;

    if ((qs = getenv("QUERY_STRING")) == NULL)
	exit(1);

    cgiarg = cgistr2tagarg(qs);
    mode = tag_get_value(cgiarg, "mode");
    local_cookie = getenv("LOCAL_COOKIE");
    mailcapfile = Strnew_charp(expandPath(RC_DIR));
    Strcat_charp(mailcapfile, "/mailcap");

    if (mode && !strcmp(mode, "edit")) {
	char *referer;
	/* check if I can edit my mailcap */
	if ((referer = getenv("HTTP_REFERER")) != NULL) {
	    if (strncmp(referer,"file://",7) != 0 &&
		strncmp(referer,"exec://",7) != 0) {
		/* referer is not file: nor exec: */
		bye("It may be an illegal execution\n referer=",referer);
	    }
	}
	sent_cookie = tag_get_value(cgiarg,"cookie");
	if (local_cookie == NULL || sent_cookie == NULL ||
	    strcmp(local_cookie,sent_cookie) != 0) {
	    /* Local cookie doesn't match */
	    bye("Local cookie doesn't match: It may be an illegal execution","");
	}
	/* edit mailcap */
	editMailcap(mailcapfile->ptr, cgiarg);
    }
    else {
	/* initial panel */
	printMailcapPanel(mailcapfile->ptr);
    }
    return 0;
}
