/* $Id: func.c,v 1.13 2002/11/22 15:43:14 ukai Exp $ */
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
static char keymap_initialized;
static struct stat current_keymap_file;

void
setKeymap(char *p, int lineno, int verbose)
{
    char *s, *emsg;
    int c, f;

    s = getQWord(&p);
    c = getKey(s);
    if (c < 0) {		/* error */
	if (lineno > 0)
	    emsg = Sprintf("line %d: unknown key '%s'", lineno, s)->ptr;
	else
	    emsg = Sprintf("defkey: unknown key '%s'", s)->ptr;
	record_err_message(emsg);
	if (verbose)
	    disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
	return;
    }
    s = getWord(&p);
    f = getFuncList(s);
    if (f < 0) {
	if (lineno > 0)
	    emsg = Sprintf("line %d: invalid command '%s'", lineno, s)->ptr;
	else
	    emsg = Sprintf("defkey: invalid command '%s'", s)->ptr;
	record_err_message(emsg);
	if (verbose)
	    disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
	return;
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

void
initKeymap(int force)
{
    FILE *kf;
    Str line;
    char *p, *s, *emsg;
    int lineno;
    int verbose = 1;
    int fd;
    struct stat kstat;
    extern int str_to_bool(char *value, int old);

    if (!force && !keymap_initialized)
	return;

    if ((kf = fopen(rcFile(keymap_file), "rt")) == NULL ||
	((fd = fileno(kf)) < 0 || fstat(fd, &kstat) ||
	 (!force && keymap_initialized &&
	  kstat.st_mtime == current_keymap_file.st_mtime &&
	  kstat.st_dev == current_keymap_file.st_dev &&
	  kstat.st_ino == current_keymap_file.st_ino &&
	  kstat.st_size == current_keymap_file.st_size)))
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
	setKeymap(p, lineno, verbose);
    }
    fclose(kf);
    current_keymap_file = kstat;
    keymap_initialized = TRUE;
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

#ifdef USE_MOUSE
void
initMouseMenu(void)
{
    FILE *mf;
    Str line;
    char *p, *s;
    int f, b, x, x2;

    mouse_menu = NULL;
    if ((mf = fopen(rcFile(MOUSE_FILE), "rt")) == NULL)
	return;

    while (!feof(mf)) {
	line = Strfgets(mf);
	Strchop(line);
	Strremovefirstspaces(line);
	if (line->length == 0)
	    continue;
	p = conv_from_system(line->ptr);
	s = getWord(&p);
	if (*s == '#')		/* comment */
	    continue;
	if (!strcmp(s, "menu")) {
	    s = getQWord(&p);
	    if (!*s)
		continue;	/* error */
	    mouse_menu = New(MouseMenu);
	    mouse_menu->str = s;
	    mouse_menu->width = strlen(s);
	    mouse_menu->in_action = FALSE;
	    if (mouse_menu->width >= LIMIT_MOUSE_MENU)
		mouse_menu->width = LIMIT_MOUSE_MENU;
	    for (b = 0; b < 3; b++) {
		mouse_menu->map[b] = New_N(MouseMenuMap, mouse_menu->width);
		for (x = 0; x < mouse_menu->width; x++) {
		    mouse_menu->map[b][x].func = NULL;
		    mouse_menu->map[b][x].data = NULL;
		}
	    }
	}
	if (!mouse_menu)
	    continue;		/* "menu" is not set */
	if (strcmp(s, "button")) 
	    continue;		/* error */
	s = getWord(&p);
	b = atoi(s);
	if (!(b >= 1 && b <= 3))
	    continue;		/* error */
	s = getWord(&p);
	x = atoi(s);
	if (!(IS_DIGIT(*s) && x >= 0 && x < mouse_menu->width))
	    continue;		/* error */
	s = getWord(&p);
	x2 = atoi(s);
	if (!(IS_DIGIT(*s) && x2 >= 0 && x2 < mouse_menu->width))
	    continue;		/* error */
	s = getWord(&p);
	f = getFuncList(s);
	if (f < 0)
	    continue;		/* error */
	s = getQWord(&p);
	if (!*s)
	    s = NULL;
	for (; x <= x2; x++) {
	    mouse_menu->map[b - 1][x].func = w3mFuncList[f].func;
	    mouse_menu->map[b - 1][x].data = s;
	}
    }
    fclose(mf);
}
#endif
