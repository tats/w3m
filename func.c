/* $Id: func.c,v 1.9 2002/03/22 15:05:18 ukai Exp $ */
/*
 * w3m func.c
 */

#include <stdio.h>

#include "fm.h"
#include "func.h"
#include "myctype.h"

#include "funcname.c"
#include "functable.c"

#define KEYDATA_HASH_SIZE 16
static Hash_iv *keyData = NULL;

void
initKeymap(void)
{
    FILE *kf;
    Str line;
    char *p, *s, *emsg;
    int c;
    int f;
    int lineno;
    int verbose = 1;
    extern int str_to_bool(char *value, int old);

    if ((kf = fopen(rcFile(KEYMAP_FILE), "rt")) == NULL)
	return;

    lineno = 0;
    while (!feof(kf)) {
	line = Strfgets(kf);
	lineno++;
	Strchop(line);
	Strremovefirstspaces(line);
	if (line->length == 0)
	    continue;
	p = line->ptr;
	s = getWord(&p);
	if (*s == '#')		/* comment */
	    continue;
	if (!strcmp(s, "keymap")) ;
	else if (!strcmp(s, "verbose")) {
	    s = getWord(&p);
	    if (*s)
		verbose = str_to_bool(s, verbose);
	    continue;
	}
	else {			/* error */
	    emsg = Sprintf("line %d: syntax error '%s'", lineno, s)->ptr;
	    record_err_message(emsg);
	    if (verbose)
		disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
	    continue;
	}
	s = getQWord(&p);
	c = getKey(s);
	if (c < 0) {		/* error */
	    emsg = Sprintf("line %d: unknown key '%s'", lineno, s)->ptr;
	    record_err_message(emsg);
	    if (verbose)
		disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
	    continue;
	}
	s = getWord(&p);
	f = getFuncList(s);
	if (f < 0) {
	    emsg = Sprintf("line %d: invalid command '%s'", lineno, s)->ptr;
	    record_err_message(emsg);
	    if (verbose)
		disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
	    continue;
	}
	if (c & K_ESCD)
	    EscDKeymap[c ^ K_ESCD] = f;
	else if (c & K_ESCB)
	    EscBKeymap[c ^ K_ESCB] = f;
	else if (c & K_ESC)
	    EscKeymap[c ^ K_ESC] = f;
	else
	    GlobalKeymap[c] = f;
	s = getQWord(&p);
	if (*s) {
	    if (keyData == NULL)
		keyData = newHash_iv(KEYDATA_HASH_SIZE);
	    putHash_iv(keyData, c, (void *)s);
	}
    }
    fclose(kf);
}

int
getFuncList(char *id)
{
    return getHash_si(&functable, id, -1);
}

char *
getKeyData(int key)
{
    if (keyData == NULL)
	return NULL;
    return (char *)getHash_iv(keyData, key, NULL);
}

int
getKey(char *s)
{
    int c, esc = 0, ctrl = 0;

    if (s == NULL || *s == '\0')
	return -1;

    if (strcasecmp(s, "UP") == 0)	/* ^[[A */
	return K_ESCB | 'A';
    else if (strcasecmp(s, "DOWN") == 0)	/* ^[[B */
	return K_ESCB | 'B';
    else if (strcasecmp(s, "RIGHT") == 0)	/* ^[[C */
	return K_ESCB | 'C';
    else if (strcasecmp(s, "LEFT") == 0)	/* ^[[D */
	return K_ESCB | 'D';

    if (strncasecmp(s, "ESC-", 4) == 0 || strncasecmp(s, "ESC ", 4) == 0) {	/* ^[ */
	s += 4;
	esc = K_ESC;
    }
    else if (strncasecmp(s, "M-", 2) == 0 || strncasecmp(s, "\\E", 2) == 0) {	/* ^[ */
	s += 2;
	esc = K_ESC;
    }
    else if (*s == ESC_CODE) {	/* ^[ */
	s++;
	esc = K_ESC;
    }
    if (strncasecmp(s, "C-", 2) == 0) {	/* ^, ^[^ */
	s += 2;
	ctrl = 1;
    }
    else if (*s == '^' && *(s + 1)) {	/* ^, ^[^ */
	s++;
	ctrl = 1;
    }
    if (!esc && ctrl && *s == '[') {	/* ^[ */
	s++;
	ctrl = 0;
	esc = K_ESC;
    }
    if (esc && !ctrl) {
	if (*s == '[' || *s == 'O') {	/* ^[[, ^[O */
	    s++;
	    esc = K_ESCB;
	}
	if (strncasecmp(s, "C-", 2) == 0) {	/* ^[^, ^[[^ */
	    s += 2;
	    ctrl = 1;
	}
	else if (*s == '^' && *(s + 1)) {	/* ^[^, ^[[^ */
	    s++;
	    ctrl = 1;
	}
    }

    if (ctrl) {
	if (*s >= '@' && *s <= '_')	/* ^@ .. ^_ */
	    return esc | (*s - '@');
	else if (*s >= 'a' && *s <= 'z')	/* ^a .. ^z */
	    return esc | (*s - 'a' + 1);
	else if (*s == '?')	/* ^? */
	    return esc | DEL_CODE;
	else
	    return -1;
    }

    if (esc == K_ESCB && IS_DIGIT(*s)) {
	c = (int)(*s - '0');
	s++;
	if (IS_DIGIT(*s)) {
	    c = c * 10 + (int)(*s - '0');
	    s++;
	}
	if (*s == '~')
	    return K_ESCD | c;
	else
	    return -1;
    }

    if (strncasecmp(s, "SPC", 3) == 0)	/* ' ' */
	return esc | ' ';
    else if (strncasecmp(s, "TAB", 3) == 0)	/* ^i */
	return esc | '\t';
    else if (strncasecmp(s, "DEL", 3) == 0)	/* ^? */
	return esc | DEL_CODE;

    if (*s == '\\' && *(s + 1) != '\0') {
	switch (*(s + 1)) {
	case 'a':		/* ^g */
	    return esc | CTRL_G;
	case 'b':		/* ^h */
	    return esc | CTRL_H;
	case 't':		/* ^i */
	    return esc | CTRL_I;
	case 'n':		/* ^j */
	    return esc | CTRL_J;
	case 'r':		/* ^m */
	    return esc | CTRL_M;
	case 'e':		/* ^[ */
	    return esc | ESC_CODE;
	case '^':		/* ^ */
	    return esc | '^';
	case '\\':		/* \ */
	    return esc | '\\';
	default:
	    return -1;
	}
    }
    if (IS_ASCII(*s))		/* Ascii */
	return esc | *s;
    else
	return -1;
}

char *
getWord(char **str)
{
    char *p, *s;

    p = *str;
    SKIP_BLANKS(p);
    for (s = p; *p && !IS_SPACE(*p) && *p != ';'; p++) ;
    *str = p;
    return Strnew_charp_n(s, p - s)->ptr;
}

char *
getQWord(char **str)
{
    Str tmp = Strnew();
    char *p;
    int in_q = 0, in_dq = 0, esc = 0;

    p = *str;
    SKIP_BLANKS(p);
    for (; *p; p++) {
	if (esc) {
	    if (in_q) {
		if (*p != '\\' && *p != '\'')	/* '..\\..', '..\'..' */
		    Strcat_char(tmp, '\\');
	    }
	    else if (in_dq) {
		if (*p != '\\' && *p != '"')	/* "..\\..", "..\".." */
		    Strcat_char(tmp, '\\');
	    }
	    else {
		if (*p != '\\' && *p != '\'' &&	/* ..\\.., ..\'.. */
		    *p != '"' && !IS_SPACE(*p))	/* ..\".., ..\.. */
		    Strcat_char(tmp, '\\');
	    }
	    Strcat_char(tmp, *p);
	    esc = 0;
	}
	else if (*p == '\\') {
	    esc = 1;
	}
	else if (in_q) {
	    if (*p == '\'')
		in_q = 0;
	    else
		Strcat_char(tmp, *p);
	}
	else if (in_dq) {
	    if (*p == '"')
		in_dq = 0;
	    else
		Strcat_char(tmp, *p);
	}
	else if (*p == '\'') {
	    in_q = 1;
	}
	else if (*p == '"') {
	    in_dq = 1;
	}
	else if (IS_SPACE(*p) || *p == ';') {
	    break;
	}
	else {
	    Strcat_char(tmp, *p);
	}
    }
    *str = p;
    return tmp->ptr;
}
