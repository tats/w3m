/* $Id: menu.c,v 1.46 2007/05/23 12:34:20 inu Exp $ */
/* 
 * w3m menu.c
 */
#include <stdio.h>

#include "fm.h"
#include "menu.h"
#include "func.h"
#include "myctype.h"
#include "regex.h"

#ifdef USE_MOUSE
#ifdef USE_GPM
#include <gpm.h>
static int gpm_process_menu_mouse(Gpm_Event * event, void *data);
extern int gpm_process_mouse(Gpm_Event *, void *);
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
extern int (*sysm_handler) (int x, int y, int nbs, int obs);
static int sysm_process_menu_mouse(int, int, int, int);
extern int sysm_process_mouse(int, int, int, int);
#endif				/* USE_SYSMOUSE */
#if defined(USE_GPM) || defined(USE_SYSMOUSE)
#define X_MOUSE_SELECTED (char)0xff
static int X_Mouse_Selection;
extern int do_getch();
#define getch()	do_getch()
#endif				/* defined(USE_GPM) || defined(USE_SYSMOUSE) */
#endif				/* USE_MOUSE */

#ifdef USE_MENU

static char **FRAME;
static int FRAME_WIDTH;
static int graph_mode = FALSE;
#define G_start  {if (graph_mode) graphstart();}
#define G_end    {if (graph_mode) graphend();}

static int mEsc(char c);
static int mEscB(char c);
static int mEscD(char c);
static int mNull(char c);
static int mSelect(char c);
static int mDown(char c);
static int mUp(char c);
static int mLast(char c);
static int mTop(char c);
static int mNext(char c);
static int mPrev(char c);
static int mFore(char c);
static int mBack(char c);
static int mLineU(char c);
static int mLineD(char c);
static int mOk(char c);
static int mCancel(char c);
static int mClose(char c);
static int mSusp(char c);
static int mMouse(char c);
static int mSrchF(char c);
static int mSrchB(char c);
static int mSrchN(char c);
static int mSrchP(char c);
#ifdef __EMX__
static int mPc(char c);
#endif

/* *INDENT-OFF* */
static int (*MenuKeymap[128]) (char c) = {
/*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
#ifdef __EMX__
    mPc,    mTop,   mPrev,  mClose, mNull,  mLast,  mNext,  mNull,
#else
    mNull,  mTop,   mPrev,  mClose, mNull,  mLast,  mNext,  mNull,
#endif
/*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    mCancel,mNull,  mOk,    mNull,  mNull,  mOk,    mDown,  mNull,
/*  C-p     C-q     C-r     C-s     C-t     C-u     C-v     C-w      */
    mUp,    mNull,  mSrchB, mSrchF, mNull,  mNull,  mNext,  mNull,
/*  C-x     C-y     C-z     C-[     C-\     C-]     C-^     C-_      */
    mNull,  mNull,  mSusp,  mEsc,   mNull,  mNull,  mNull,  mNull,
/*  SPC     !       "       #       $       %       &       '        */
    mOk,    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  (       )       *       +       ,       -       .       /        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mSrchF,
/*  0       1       2       3       4       5       6       7        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull , mNull,  mNull,
/*  8       9       :       ;       <       =       >       ?        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mSrchB,
/*  @       A       B       C       D       E       F       G        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  H       I       J       K       L       M       N       O        */
    mNull,  mNull,  mLineU, mLineD, mNull,  mNull,  mSrchP, mNull,
/*  P       Q       R       S       T       U       V       W        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  X       Y       Z       [       \       ]       ^       _        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  `       a       b       c       d       e       f       g        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  h       i       j       k       l       m       n       o        */
    mCancel,mNull,  mDown,  mUp,    mOk,    mNull,  mSrchN, mNull,
/*  p       q       r       s       t       u       v       w        */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  x       y       z       {       |       }       ~       DEL      */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mCancel,
};
static int (*MenuEscKeymap[128]) (char c) = {
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  O     */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mEscB,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  [                                     */
    mNull,  mNull,  mNull,  mEscB,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  v             */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mPrev,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
};
static int (*MenuEscBKeymap[128]) (char c) = {
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  A       B       C       D       E                     */
    mNull,  mUp,    mDown,  mOk,    mCancel,mClose, mNull, mNull,
/*  L       M                     */
    mNull,  mNull,  mNull,  mNull,  mClose, mMouse, mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
};
static int (*MenuEscDKeymap[128]) (char c) = {
/*  0       1       INS     3       4       PgUp,   PgDn    7     */
    mNull,  mNull,  mClose, mNull,  mNull,  mBack,  mFore,  mNull,
/*  8       9       10      F1      F2      F3      F4      F5       */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  16      F6      F7      F8      F9      F10     22      23       */
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
/*  24      25      26      27      HELP    29      30      31       */
    mNull,  mNull,  mNull,  mNull,  mClose, mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,

    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
};

#ifdef __EMX__
static int (*MenuPcKeymap[256])(char c)={
//			  Null
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
//							  S-Tab
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-q	  A-w	  A-E	  A-r	  A-t	  A-y	  A-u	  A-i
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-o	  A-p	  A-[	  A-]			  A-a	  A-s
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-d	  A-f	  A-g	  A-h	  A-j	  A-k	  A-l	  A-;
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-'    A-'		  A-\		  A-x	  A-c	  A-v
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mPrev,
// A-b	  A-n	  A-m	  A-,	  A-.	  A-/		  A-+
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
//			  F1	  F2	  F3	  F4	  F5
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// F6	  F7	  F8	  F9	  F10			  Home
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mTop,
// Up	  PgUp	  A-/	  Left	  5	  Right	  C-*	  End
  mUp,	  mUp,	  mNull,  mCancel,mNull,  mOk,	  mNull,  mLast,
// Down	  PgDn	  Ins	  Del	  S-F1	  S-F2	  S-F3	  S-F4
  mDown,  mDown,  mClose, mCancel,mNull,  mNull,  mNull,  mNull,
// S-F5	  S-F6	  S-F7	  S-F8	  S-F9	  S-F10	  C-F1	  C-F2
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// C-F3	  C-F4	  C-F5	  C-F6	  C-F7	  C-F8	  C-F9	  C-F10
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-F1	  A-F2	  A-F3	  A-F4	  A-F5	  A-F6	  A-F7	  A-F8
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-F9	  A-F10	  PrtSc	  C-Left  C-Right C-End	  C-PgDn  C-Home
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-1	  A-2	  A-3	  A-4	  A-5	  A-6	  A-7/8	  A-9
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// A-0	  A -	  A-=		  C-PgUp  F11	  F12	  S-F11
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// S-F12  C-F11	  C-F12	  A-F11	  A-F12	  C-Up	  C-/	  C-5
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
// S-*	  C-Down  C-Ins	  C-Del	  C-Tab	  C -	  C-+
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,
//				  A -	  A-Tab	  A-Enter
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 160
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 168
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 176
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 184
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 192
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 200
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 208
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 216
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 224
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 232
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,   // 240
  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mNull	   // 248
};
#endif
/* *INDENT-ON* */
/* --- SelectMenu --- */

static Menu SelectMenu;
static int SelectV = 0;
static void initSelectMenu(void);
static void smChBuf(void);
static int smDelBuf(char c);

/* --- SelectMenu (END) --- */

/* --- SelTabMenu --- */

static Menu SelTabMenu;
static int SelTabV = 0;
static void initSelTabMenu(void);
static void smChTab(void);
static int smDelTab(char c);

/* --- SelTabMenu (END) --- */

/* --- MainMenu --- */

static Menu MainMenu;
#ifdef USE_M17N
/* FIXME: gettextize here */
static wc_ces MainMenuCharset = WC_CES_US_ASCII;	/* FIXME: charset of source code */
static int MainMenuEncode = FALSE;
#endif

static MenuItem MainMenuItem[] = {
    /* type        label           variable value func     popup keys data  */
    {MENU_FUNC, N_(" Back         (b) "), NULL, 0, backBf, NULL, "b", NULL},
    {MENU_POPUP, N_(" Select Buffer(s) "), NULL, 0, NULL, &SelectMenu, "s",
     NULL},
    {MENU_POPUP, N_(" Select Tab   (t) "), NULL, 0, NULL, &SelTabMenu, "tT",
     NULL},
    {MENU_FUNC, N_(" View Source  (v) "), NULL, 0, vwSrc, NULL, "vV", NULL},
    {MENU_FUNC, N_(" Edit Source  (e) "), NULL, 0, editBf, NULL, "eE", NULL},
    {MENU_FUNC, N_(" Save Source  (S) "), NULL, 0, svSrc, NULL, "S", NULL},
    {MENU_FUNC, N_(" Reload       (r) "), NULL, 0, reload, NULL, "rR", NULL},
    {MENU_NOP, N_(" ---------------- "), NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, N_(" Go Link      (a) "), NULL, 0, followA, NULL, "a", NULL},
    {MENU_FUNC, N_("   on New Tab (n) "), NULL, 0, tabA, NULL, "nN", NULL},
    {MENU_FUNC, N_(" Save Link    (A) "), NULL, 0, svA, NULL, "A", NULL},
    {MENU_FUNC, N_(" View Image   (i) "), NULL, 0, followI, NULL, "i", NULL},
    {MENU_FUNC, N_(" Save Image   (I) "), NULL, 0, svI, NULL, "I", NULL},
    {MENU_FUNC, N_(" View Frame   (f) "), NULL, 0, rFrame, NULL, "fF", NULL},
    {MENU_NOP, N_(" ---------------- "), NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, N_(" Bookmark     (B) "), NULL, 0, ldBmark, NULL, "B", NULL},
    {MENU_FUNC, N_(" Help         (h) "), NULL, 0, ldhelp, NULL, "hH", NULL},
    {MENU_FUNC, N_(" Option       (o) "), NULL, 0, ldOpt, NULL, "oO", NULL},
    {MENU_NOP, N_(" ---------------- "), NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, N_(" Quit         (q) "), NULL, 0, qquitfm, NULL, "qQ", NULL},
    {MENU_END, "", NULL, 0, nulcmd, NULL, "", NULL},
};

/* --- MainMenu (END) --- */

static MenuList *w3mMenuList;

static Menu *CurrentMenu = NULL;

#define mvaddch(y, x, c)        (move(y, x), addch(c))
#define mvaddstr(y, x, str)     (move(y, x), addstr(str))
#define mvaddnstr(y, x, str, n) (move(y, x), addnstr_sup(str, n))

void
new_menu(Menu *menu, MenuItem *item)
{
    int i, l;
    char *p;

    menu->cursorX = 0;
    menu->cursorY = 0;
    menu->x = 0;
    menu->y = 0;
    menu->nitem = 0;
    menu->item = item;
    menu->initial = 0;
    menu->select = 0;
    menu->offset = 0;
    menu->active = 0;

    if (item == NULL)
	return;

    for (i = 0; item[i].type != MENU_END; i++) ;
    menu->nitem = i;
    menu->height = menu->nitem;
    for (i = 0; i < 128; i++)
	menu->keymap[i] = MenuKeymap[i];
    menu->width = 0;
    for (i = 0; i < menu->nitem; i++) {
	if ((p = item[i].keys) != NULL) {
	    while (*p) {
		if (IS_ASCII(*p)) {
		    menu->keymap[(int)*p] = mSelect;
		    menu->keyselect[(int)*p] = i;
		}
		p++;
	    }
	}
	l = get_strwidth(item[i].label);
	if (l > menu->width)
	    menu->width = l;
    }
}

void
geom_menu(Menu *menu, int x, int y, int mselect)
{
    int win_x, win_y, win_w, win_h;

    menu->select = mselect;

    if (menu->width % FRAME_WIDTH)
	menu->width = (menu->width / FRAME_WIDTH + 1) * FRAME_WIDTH;
    win_x = menu->x - FRAME_WIDTH;
    win_w = menu->width + 2 * FRAME_WIDTH;
    if (win_x + win_w > COLS)
	win_x = COLS - win_w;
    if (win_x < 0) {
	win_x = 0;
	if (win_w > COLS) {
	    menu->width = COLS - 2 * FRAME_WIDTH;
	    menu->width -= menu->width % FRAME_WIDTH;
	    win_w = menu->width + 2 * FRAME_WIDTH;
	}
    }
    menu->x = win_x + FRAME_WIDTH;

    win_y = menu->y - mselect - 1;
    win_h = menu->height + 2;
    if (win_y + win_h > LASTLINE)
	win_y = LASTLINE - win_h;
    if (win_y < 0) {
	win_y = 0;
	if (win_y + win_h > LASTLINE) {
	    win_h = LASTLINE - win_y;
	    menu->height = win_h - 2;
	    if (menu->height <= mselect)
		menu->offset = mselect - menu->height + 1;
	}
    }
    menu->y = win_y + 1;
}

void
draw_all_menu(Menu *menu)
{
    if (menu->parent != NULL)
	draw_all_menu(menu->parent);
    draw_menu(menu);
}

void
draw_menu(Menu *menu)
{
    int x, y, w;
    int i, j;

    x = menu->x - FRAME_WIDTH;
    w = menu->width + 2 * FRAME_WIDTH;
    y = menu->y - 1;

    if (menu->offset == 0) {
	G_start;
	mvaddstr(y, x, FRAME[3]);
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[10]);
	mvaddstr(y, x + i, FRAME[6]);
	G_end;
    }
    else {
	G_start;
	mvaddstr(y, x, FRAME[5]);
	G_end;
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i++)
	    mvaddstr(y, x + i, " ");
	G_start;
	mvaddstr(y, x + i, FRAME[5]);
	G_end;
	i = (w / 2 - 1) / FRAME_WIDTH * FRAME_WIDTH;
	mvaddstr(y, x + i, ":");
    }

    for (j = 0; j < menu->height; j++) {
	y++;
	G_start;
	mvaddstr(y, x, FRAME[5]);
	G_end;
	draw_menu_item(menu, menu->offset + j);
	G_start;
	mvaddstr(y, x + w - FRAME_WIDTH, FRAME[5]);
	G_end;
    }
    y++;
    if (menu->offset + menu->height == menu->nitem) {
	G_start;
	mvaddstr(y, x, FRAME[9]);
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[10]);
	mvaddstr(y, x + i, FRAME[12]);
	G_end;
    }
    else {
	G_start;
	mvaddstr(y, x, FRAME[5]);
	G_end;
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i++)
	    mvaddstr(y, x + i, " ");
	G_start;
	mvaddstr(y, x + i, FRAME[5]);
	G_end;
	i = (w / 2 - 1) / FRAME_WIDTH * FRAME_WIDTH;
	mvaddstr(y, x + i, ":");
    }
}

void
draw_menu_item(Menu *menu, int mselect)
{
    mvaddnstr(menu->y + mselect - menu->offset, menu->x,
	      menu->item[mselect].label, menu->width);
}

int
select_menu(Menu *menu, int mselect)
{
    if (mselect < 0 || mselect >= menu->nitem)
	return (MENU_NOTHING);
    if (mselect < menu->offset)
	up_menu(menu, menu->offset - mselect);
    else if (mselect >= menu->offset + menu->height)
	down_menu(menu, mselect - menu->offset - menu->height + 1);

    if (menu->select >= menu->offset &&
	menu->select < menu->offset + menu->height)
	draw_menu_item(menu, menu->select);
    menu->select = mselect;
    standout();
    draw_menu_item(menu, menu->select);
    standend();
    /* 
     * move(menu->cursorY, menu->cursorX); */
    move(menu->y + mselect - menu->offset, menu->x);
    toggle_stand();
    refresh();

    return (menu->select);
}

void
goto_menu(Menu *menu, int mselect, int down)
{
    int select_in;
    if (mselect >= menu->nitem)
	mselect = menu->nitem - 1;
    else if (mselect < 0)
	mselect = 0;
    select_in = mselect;
    while (menu->item[mselect].type == MENU_NOP) {
	if (down > 0) {
	    if (++mselect >= menu->nitem) {
		down_menu(menu, select_in - menu->select);
		mselect = menu->select;
		break;
	    }
	}
	else if (down < 0) {
	    if (--mselect < 0) {
		up_menu(menu, menu->select - select_in);
		mselect = menu->select;
		break;
	    }
	}
	else {
	    return;
	}
    }
    select_menu(menu, mselect);
}

void
up_menu(Menu *menu, int n)
{
    if (n < 0 || menu->offset == 0)
	return;
    menu->offset -= n;
    if (menu->offset < 0)
	menu->offset = 0;

    draw_menu(menu);
}

void
down_menu(Menu *menu, int n)
{
    if (n < 0 || menu->offset + menu->height == menu->nitem)
	return;
    menu->offset += n;
    if (menu->offset + menu->height > menu->nitem)
	menu->offset = menu->nitem - menu->height;

    draw_menu(menu);
}

int
action_menu(Menu *menu)
{
    char c;
    int mselect;
    MenuItem item;

    if (menu->active == 0) {
	if (menu->parent != NULL)
	    menu->parent->active = 0;
	return (0);
    }
    draw_all_menu(menu);
    select_menu(menu, menu->select);

    while (1) {
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_active();
#endif				/* USE_MOUSE */
	c = getch();
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_inactive();
#if defined(USE_GPM) || defined(USE_SYSMOUSE)
	if (c == X_MOUSE_SELECTED) {
	    mselect = X_Mouse_Selection;
	    if (mselect != MENU_NOTHING)
		break;
	}
#endif				/* defined(USE_GPM) || defined(USE_SYSMOUSE) */
#endif				/* USE_MOUSE */
	if (IS_ASCII(c)) {	/* Ascii */
	    mselect = (*menu->keymap[(int)c]) (c);
	    if (mselect != MENU_NOTHING)
		break;
	}
    }
    if (mselect >= 0 && mselect < menu->nitem) {
	item = menu->item[mselect];
	if (item.type & MENU_POPUP) {
	    popup_menu(menu, item.popup);
	    return (1);
	}
	if (menu->parent != NULL)
	    menu->parent->active = 0;
	if (item.type & MENU_VALUE)
	    *item.variable = item.value;
	if (item.type & MENU_FUNC) {
	    CurrentKey = -1;
	    CurrentKeyData = NULL;
	    CurrentCmdData = item.data;
	    (*item.func) ();
	    CurrentCmdData = NULL;
	}
    }
    else if (mselect == MENU_CLOSE) {
	if (menu->parent != NULL)
	    menu->parent->active = 0;
    }
    return (0);
}

void
popup_menu(Menu *parent, Menu *menu)
{
    int active = 1;

    if (menu->item == NULL || menu->nitem == 0)
	return;
    if (menu->active)
	return;

#ifdef USE_MOUSE
#ifdef USE_GPM
    gpm_handler = gpm_process_menu_mouse;
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
    sysm_handler = sysm_process_menu_mouse;
#endif				/* USE_SYSMOUSE */
#endif				/* USE_MOUSE */
    menu->parent = parent;
    menu->select = menu->initial;
    menu->offset = 0;
    menu->active = 1;
    if (parent != NULL) {
	menu->cursorX = parent->cursorX;
	menu->cursorY = parent->cursorY;
	guess_menu_xy(parent, menu->width, &menu->x, &menu->y);
    }
    geom_menu(menu, menu->x, menu->y, menu->select);

    CurrentMenu = menu;
    while (active) {
	active = action_menu(CurrentMenu);
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
    }
    menu->active = 0;
    CurrentMenu = parent;
#ifdef USE_MOUSE
#ifdef USE_GPM
    if (CurrentMenu == NULL)
	gpm_handler = gpm_process_mouse;
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
    if (CurrentMenu == NULL)
	sysm_handler = sysm_process_mouse;
#endif				/* USE_SYSMOUSE */
#endif				/* USE_MOUSE */
}

void
guess_menu_xy(Menu *parent, int width, int *x, int *y)
{
    *x = parent->x + parent->width + FRAME_WIDTH - 1;
    if (*x + width + FRAME_WIDTH > COLS) {
	*x = COLS - width - FRAME_WIDTH;
	if ((parent->x + parent->width / 2 > *x) &&
	    (parent->x + parent->width / 2 > COLS / 2))
	    *x = parent->x - width - FRAME_WIDTH + 1;
    }
    *y = parent->y + parent->select - parent->offset;
}

void
new_option_menu(Menu *menu, char **label, int *variable, void (*func) ())
{
    int i, nitem;
    char **p;
    MenuItem *item;

    if (label == NULL || *label == NULL)
	return;

    for (i = 0, p = label; *p != NULL; i++, p++) ;
    nitem = i;

    item = New_N(MenuItem, nitem + 1);

    for (i = 0, p = label; i < nitem; i++, p++) {
	if (func != NULL)
	    item[i].type = MENU_VALUE | MENU_FUNC;
	else
	    item[i].type = MENU_VALUE;
	item[i].label = *p;
	item[i].variable = variable;
	item[i].value = i;
	item[i].func = func;
	item[i].popup = NULL;
	item[i].keys = "";
    }
    item[nitem].type = MENU_END;

    new_menu(menu, item);
}

static void
set_menu_frame(void)
{
    if (graph_ok()) {
	graph_mode = TRUE;
	FRAME_WIDTH = 1;
	FRAME = graph_symbol;
    }
    else {
	graph_mode = FALSE;
#ifdef USE_M17N
	FRAME_WIDTH = 0;
	FRAME = get_symbol(DisplayCharset, &FRAME_WIDTH);
	if (!WcOption.use_wide)
	    FRAME_WIDTH = 1;
#else
	FRAME_WIDTH = 1;
	FRAME = get_symbol();
#endif
    }
}

/* --- MenuFunctions --- */

#ifdef __EMX__
static int
mPc(char c)
{
    c = getch();
    return (MenuPcKeymap[(int)c] (c));
}
#endif

static int
mEsc(char c)
{
    c = getch();
    return (MenuEscKeymap[(int)c] (c));
}

static int
mEscB(char c)
{
    c = getch();
    if (IS_DIGIT(c))
	return (mEscD(c));
    else
	return (MenuEscBKeymap[(int)c] (c));
}

static int
mEscD(char c)
{
    int d;

    d = (int)c - (int)'0';
    c = getch();
    if (IS_DIGIT(c)) {
	d = d * 10 + (int)c - (int)'0';
	c = getch();
    }
    if (c == '~')
	return (MenuEscDKeymap[d] (c));
    else
	return (MENU_NOTHING);
}

static int
mNull(char c)
{
    return (MENU_NOTHING);
}

static int
mSelect(char c)
{
    if (IS_ASCII(c))
	return (select_menu(CurrentMenu, CurrentMenu->keyselect[(int)c]));
    else
	return (MENU_NOTHING);
}

static int
mDown(char c)
{
    if (CurrentMenu->select >= CurrentMenu->nitem - 1)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, CurrentMenu->select + 1, 1);
    return (MENU_NOTHING);
}

static int
mUp(char c)
{
    if (CurrentMenu->select <= 0)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, CurrentMenu->select - 1, -1);
    return (MENU_NOTHING);
}

static int
mLast(char c)
{
    goto_menu(CurrentMenu, CurrentMenu->nitem - 1, -1);
    return (MENU_NOTHING);
}

static int
mTop(char c)
{
    goto_menu(CurrentMenu, 0, 1);
    return (MENU_NOTHING);
}

static int
mNext(char c)
{
    int mselect = CurrentMenu->select + CurrentMenu->height;

    if (mselect >= CurrentMenu->nitem)
	return mLast(c);
    down_menu(CurrentMenu, CurrentMenu->height);
    goto_menu(CurrentMenu, mselect, -1);
    return (MENU_NOTHING);
}

static int
mPrev(char c)
{
    int mselect = CurrentMenu->select - CurrentMenu->height;

    if (mselect < 0)
	return mTop(c);
    up_menu(CurrentMenu, CurrentMenu->height);
    goto_menu(CurrentMenu, mselect, 1);
    return (MENU_NOTHING);
}

static int
mFore(char c)
{
    if (CurrentMenu->select >= CurrentMenu->nitem - 1)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, (CurrentMenu->select + CurrentMenu->height - 1),
	      (CurrentMenu->height + 1));
    return (MENU_NOTHING);
}

static int
mBack(char c)
{
    if (CurrentMenu->select <= 0)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, (CurrentMenu->select - CurrentMenu->height + 1),
	      (-1 - CurrentMenu->height));
    return (MENU_NOTHING);
}

static int
mLineU(char c)
{
    int mselect = CurrentMenu->select;

    if (mselect >= CurrentMenu->nitem)
	return mLast(c);
    if (CurrentMenu->offset + CurrentMenu->height >= CurrentMenu->nitem)
	mselect++;
    else {
	down_menu(CurrentMenu, 1);
	if (mselect < CurrentMenu->offset)
	    mselect++;
    }
    goto_menu(CurrentMenu, mselect, 1);
    return (MENU_NOTHING);
}

static int
mLineD(char c)
{
    int mselect = CurrentMenu->select;

    if (mselect <= 0)
	return mTop(c);
    if (CurrentMenu->offset <= 0)
	mselect--;
    else {
	up_menu(CurrentMenu, 1);
	if (mselect >= CurrentMenu->offset + CurrentMenu->height)
	    mselect--;
    }
    goto_menu(CurrentMenu, mselect, -1);
    return (MENU_NOTHING);
}

static int
mOk(char c)
{
    int mselect = CurrentMenu->select;

    if (CurrentMenu->item[mselect].type == MENU_NOP)
	return (MENU_NOTHING);
    return (mselect);
}

static int
mCancel(char c)
{
    return (MENU_CANCEL);
}

static int
mClose(char c)
{
    return (MENU_CLOSE);
}

static int
mSusp(char c)
{
    susp();
    draw_all_menu(CurrentMenu);
    select_menu(CurrentMenu, CurrentMenu->select);
    return (MENU_NOTHING);
}

static char *SearchString = NULL;

int (*menuSearchRoutine) (Menu *, char *, int);

static int
menuForwardSearch(Menu *menu, char *str, int from)
{
    int i;
    char *p;
    if ((p = regexCompile(str, IgnoreCase)) != NULL) {
	message(p, 0, 0);
	return -1;
    }
    if (from < 0)
	from = 0;
    for (i = from; i < menu->nitem; i++)
	if (menu->item[i].type != MENU_NOP &&
	    regexMatch(menu->item[i].label, -1, 1) == 1)
	    return i;
    return -1;
}

static int
menu_search_forward(Menu *menu, int from)
{
    char *str;
    int found;
    str = inputStrHist("Forward: ", NULL, TextHist);
    if (str != NULL && *str == '\0')
	str = SearchString;
    if (str == NULL || *str == '\0')
	return -1;
    SearchString = str;
    str = conv_search_string(str, DisplayCharset);
    menuSearchRoutine = menuForwardSearch;
    found = menuForwardSearch(menu, str, from + 1);
    if (WrapSearch && found == -1)
	found = menuForwardSearch(menu, str, 0);
    if (found >= 0)
	return found;
    disp_message("Not found", TRUE);
    return -1;
}

static int
mSrchF(char c)
{
    int mselect;
    mselect = menu_search_forward(CurrentMenu, CurrentMenu->select);
    if (mselect >= 0)
	goto_menu(CurrentMenu, mselect, 1);
    return (MENU_NOTHING);
}

static int
menuBackwardSearch(Menu *menu, char *str, int from)
{
    int i;
    char *p;
    if ((p = regexCompile(str, IgnoreCase)) != NULL) {
	message(p, 0, 0);
	return -1;
    }
    if (from >= menu->nitem)
	from = menu->nitem - 1;
    for (i = from; i >= 0; i--)
	if (menu->item[i].type != MENU_NOP &&
	    regexMatch(menu->item[i].label, -1, 1) == 1)
	    return i;
    return -1;
}

static int
menu_search_backward(Menu *menu, int from)
{
    char *str;
    int found;
    str = inputStrHist("Backward: ", NULL, TextHist);
    if (str != NULL && *str == '\0')
	str = SearchString;
    if (str == NULL || *str == '\0')
	return -1;
    SearchString = str;
    str = conv_search_string(str, DisplayCharset);
    menuSearchRoutine = menuBackwardSearch;
    found = menuBackwardSearch(menu, str, from - 1);
    if (WrapSearch && found == -1)
	found = menuBackwardSearch(menu, str, menu->nitem);
    if (found >= 0)
	return found;
    disp_message("Not found", TRUE);
    return -1;
}

static int
mSrchB(char c)
{
    int mselect;
    mselect = menu_search_backward(CurrentMenu, CurrentMenu->select);
    if (mselect >= 0)
	goto_menu(CurrentMenu, mselect, -1);
    return (MENU_NOTHING);
}

static int
menu_search_next_previous(Menu *menu, int from, int reverse)
{
    int found;
    static int (*routine[2]) (Menu *, char *, int) = {
    menuForwardSearch, menuBackwardSearch};
    char *str;

    if (menuSearchRoutine == NULL) {
	disp_message("No previous regular expression", TRUE);
	return -1;
    }
    str = conv_search_string(SearchString, DisplayCharset);
    if (reverse != 0)
	reverse = 1;
    if (menuSearchRoutine == menuBackwardSearch)
	reverse ^= 1;
    from += reverse ? -1 : 1;
    found = (*routine[reverse]) (menu, str, from);
    if (WrapSearch && found == -1)
	found = (*routine[reverse]) (menu, str, reverse * menu->nitem);
    if (found >= 0)
	return found;
    disp_message("Not found", TRUE);
    return -1;
}

static int
mSrchN(char c)
{
    int mselect;
    mselect = menu_search_next_previous(CurrentMenu, CurrentMenu->select, 0);
    if (mselect >= 0)
	goto_menu(CurrentMenu, mselect, 1);
    return (MENU_NOTHING);
}

static int
mSrchP(char c)
{
    int mselect;
    mselect = menu_search_next_previous(CurrentMenu, CurrentMenu->select, 1);
    if (mselect >= 0)
	goto_menu(CurrentMenu, mselect, -1);
    return (MENU_NOTHING);
}

#ifdef USE_MOUSE
#define MOUSE_BTN1_DOWN 0
#define MOUSE_BTN2_DOWN 1
#define MOUSE_BTN3_DOWN 2
#define MOUSE_BTN4_DOWN_RXVT 3
#define MOUSE_BTN5_DOWN_RXVT 4
#define MOUSE_BTN4_DOWN_XTERM 64
#define MOUSE_BTN5_DOWN_XTERM 65
#define MOUSE_BTN_UP 3
#define MOUSE_BTN_RESET -1

static int
mMouse_scroll_line(void)
{
    int i = 0;
    if (relative_wheel_scroll)
	i = (relative_wheel_scroll_ratio * CurrentMenu->height + 99) / 100;
    else
	i = fixed_wheel_scroll_count;
    return i ? i : 1;
}

static int
process_mMouse(int btn, int x, int y)
{
    Menu *menu;
    int mselect, i;
    static int press_btn = MOUSE_BTN_RESET, press_x, press_y;
    char c = ' ';

    menu = CurrentMenu;

    if (x < 0 || x >= COLS || y < 0 || y > LASTLINE)
	return (MENU_NOTHING);

    if (btn == MOUSE_BTN_UP) {
	switch (press_btn) {
	case MOUSE_BTN1_DOWN:
	case MOUSE_BTN3_DOWN:
	    if (x < menu->x - FRAME_WIDTH ||
		x >= menu->x + menu->width + FRAME_WIDTH ||
		y < menu->y - 1 || y >= menu->y + menu->height + 1) {
		return (MENU_CANCEL);
	    }
	    else if ((x >= menu->x - FRAME_WIDTH &&
		      x < menu->x) ||
		     (x >= menu->x + menu->width &&
		      x < menu->x + menu->width + FRAME_WIDTH)) {
		return (MENU_NOTHING);
	    }
	    else if (press_y > y) {
		for (i = 0; i < press_y - y; i++)
		    mLineU(c);
		return (MENU_NOTHING);
	    }
	    else if (press_y < y) {
		for (i = 0; i < y - press_y; i++)
		    mLineD(c);
		return (MENU_NOTHING);
	    }
	    else if (y == menu->y - 1) {
		mPrev(c);
		return (MENU_NOTHING);
	    }
	    else if (y == menu->y + menu->height) {
		mNext(c);
		return (MENU_NOTHING);
	    }
	    else {
		mselect = y - menu->y + menu->offset;
		if (menu->item[mselect].type == MENU_NOP)
		    return (MENU_NOTHING);
		return (select_menu(menu, mselect));
	    }
	    break;
	case MOUSE_BTN4_DOWN_RXVT:
	    for (i = 0; i < mMouse_scroll_line(); i++)
		mLineD(c);
	    break;
	case MOUSE_BTN5_DOWN_RXVT:
	    for (i = 0; i < mMouse_scroll_line(); i++)
		mLineU(c);
	    break;
	}
    }
    else if (btn == MOUSE_BTN4_DOWN_XTERM) {
	for (i = 0; i < mMouse_scroll_line(); i++)
	    mLineD(c);
    }
    else if (btn == MOUSE_BTN5_DOWN_XTERM) {
	for (i = 0; i < mMouse_scroll_line(); i++)
	    mLineU(c);
    }

    if (btn != MOUSE_BTN4_DOWN_RXVT || press_btn == MOUSE_BTN_RESET) {
	press_btn = btn;
	press_x = x;
	press_y = y;
    }
    else {
	press_btn = MOUSE_BTN_RESET;
    }
    return (MENU_NOTHING);
}

static int
mMouse(char c)
{
    int btn, x, y;

    btn = (unsigned char)getch() - 32;
#if defined(__CYGWIN__) && CYGWIN_VERSION_DLL_MAJOR < 1005
    if (cygwin_mouse_btn_swapped) {
	if (btn == MOUSE_BTN2_DOWN)
	    btn = MOUSE_BTN3_DOWN;
	else if (btn == MOUSE_BTN3_DOWN)
	    btn = MOUSE_BTN2_DOWN;
    }
#endif
    x = (unsigned char)getch() - 33;
    if (x < 0)
	x += 0x100;
    y = (unsigned char)getch() - 33;
    if (y < 0)
	y += 0x100;

    /* 
     * if (x < 0 || x >= COLS || y < 0 || y > LASTLINE) return; */
    return process_mMouse(btn, x, y);
}

#ifdef USE_GPM
static int
gpm_process_menu_mouse(Gpm_Event * event, void *data)
{
    int btn = MOUSE_BTN_RESET, x, y;
    if (event->type & GPM_UP)
	btn = MOUSE_BTN_UP;
    else if (event->type & GPM_DOWN) {
	switch (event->buttons) {
	case GPM_B_LEFT:
	    btn = MOUSE_BTN1_DOWN;
	    break;
	case GPM_B_MIDDLE:
	    btn = MOUSE_BTN2_DOWN;
	    break;
	case GPM_B_RIGHT:
	    btn = MOUSE_BTN3_DOWN;
	    break;
	}
    }
    else {
	GPM_DRAWPOINTER(event);
	return 0;
    }
    x = event->x;
    y = event->y;
    X_Mouse_Selection = process_mMouse(btn, x - 1, y - 1);
    return X_MOUSE_SELECTED;
}
#endif				/* USE_GPM */

#ifdef USE_SYSMOUSE
static int
sysm_process_menu_mouse(int x, int y, int nbs, int obs)
{
    int btn;
    int bits;

    if (obs & ~nbs)
	btn = MOUSE_BTN_UP;
    else if (nbs & ~obs) {
	bits = nbs & ~obs;
	btn = bits & 0x1 ? MOUSE_BTN1_DOWN :
	    (bits & 0x2 ? MOUSE_BTN2_DOWN :
	     (bits & 0x4 ? MOUSE_BTN3_DOWN : 0));
    }
    else			/* nbs == obs */
	return 0;
    X_Mouse_Selection = process_mMouse(btn, x, y);
    return X_MOUSE_SELECTED;
}
#endif				/* USE_SYSMOUSE */
#else				/* not USE_MOUSE */
static int
mMouse(char c)
{
    return (MENU_NOTHING);
}
#endif				/* not USE_MOUSE */

/* --- MenuFunctions (END) --- */

/* --- MainMenu --- */

void
popupMenu(int x, int y, Menu *menu)
{
    set_menu_frame();

    initSelectMenu();
    initSelTabMenu();

    menu->cursorX = Currentbuf->cursorX + Currentbuf->rootX;
    menu->cursorY = Currentbuf->cursorY + Currentbuf->rootY;
    menu->x = x + FRAME_WIDTH + 1;
    menu->y = y + 2;

    popup_menu(NULL, menu);
}

void
mainMenu(int x, int y)
{
    popupMenu(x, y, &MainMenu);
}

DEFUN(mainMn, MAIN_MENU MENU, "Popup menu")
{
    Menu *menu = &MainMenu;
    char *data;
    int n;
    int x = Currentbuf->cursorX + Currentbuf->rootX,
	y = Currentbuf->cursorY + Currentbuf->rootY;

    data = searchKeyData();
    if (data != NULL) {
	n = getMenuN(w3mMenuList, data);
	if (n < 0)
	    return;
	menu = w3mMenuList[n].menu;
    }
#ifdef USE_MOUSE
    if (mouse_action.in_action) {
	x = mouse_action.cursorX;
	y = mouse_action.cursorY;
    }
#endif
    popupMenu(x, y, menu);
}

/* --- MainMenu (END) --- */

/* --- SelectMenu --- */

DEFUN(selMn, SELECT_MENU, "Popup buffer selection menu")
{
    int x = Currentbuf->cursorX + Currentbuf->rootX,
	y = Currentbuf->cursorY + Currentbuf->rootY;

#ifdef USE_MOUSE
    if (mouse_action.in_action) {
	x = mouse_action.cursorX;
	y = mouse_action.cursorY;
    }
#endif
    popupMenu(x, y, &SelectMenu);
}

static void
initSelectMenu(void)
{
    int i, nitem, len = 0, l;
    Buffer *buf;
    Str str;
    char **label;
    char *p;
    static char *comment = " SPC for select / D for delete buffer ";

    SelectV = -1;
    for (i = 0, buf = Firstbuf; buf != NULL; i++, buf = buf->nextBuffer) {
	if (buf == Currentbuf)
	    SelectV = i;
    }
    nitem = i;

    label = New_N(char *, nitem + 2);
    for (i = 0, buf = Firstbuf; i < nitem; i++, buf = buf->nextBuffer) {
	str = Sprintf("<%s>", buf->buffername);
	if (buf->filename != NULL) {
	    switch (buf->currentURL.scheme) {
	    case SCM_LOCAL:
		if (strcmp(buf->currentURL.file, "-")) {
		    Strcat_char(str, ' ');
		    Strcat_charp(str,
				 conv_from_system(buf->currentURL.real_file));
		}
		break;
		/* case SCM_UNKNOWN: */
	    case SCM_MISSING:
		break;
	    default:
		Strcat_char(str, ' ');
		p = parsedURL2Str(&buf->currentURL)->ptr;
		if (DecodeURL)
		    p = url_unquote_conv(p, 0);
		Strcat_charp(str, p);
		break;
	    }
	}
	label[i] = str->ptr;
	if (len < str->length)
	    len = str->length;
    }
    l = get_strwidth(comment);
    if (len < l + 4)
	len = l + 4;
    if (len > COLS - 2 * FRAME_WIDTH)
	len = COLS - 2 * FRAME_WIDTH;
    len = (len > 1) ? ((len - l + 1) / 2) : 0;
    str = Strnew();
    for (i = 0; i < len; i++)
	Strcat_char(str, '-');
    Strcat_charp(str, comment);
    for (i = 0; i < len; i++)
	Strcat_char(str, '-');
    label[nitem] = str->ptr;
    label[nitem + 1] = NULL;

    new_option_menu(&SelectMenu, label, &SelectV, smChBuf);
    SelectMenu.initial = SelectV;
    SelectMenu.cursorX = Currentbuf->cursorX + Currentbuf->rootX;
    SelectMenu.cursorY = Currentbuf->cursorY + Currentbuf->rootY;
    SelectMenu.keymap['D'] = smDelBuf;
    SelectMenu.item[nitem].type = MENU_NOP;
}

static void
smChBuf(void)
{
    int i;
    Buffer *buf;

    if (SelectV < 0 || SelectV >= SelectMenu.nitem)
	return;
    for (i = 0, buf = Firstbuf; i < SelectV; i++, buf = buf->nextBuffer) ;
    Currentbuf = buf;
    for (buf = Firstbuf; buf != NULL; buf = buf->nextBuffer) {
	if (buf == Currentbuf)
	    continue;
#ifdef USE_IMAGE
	deleteImage(buf);
#endif
	if (clear_buffer)
	    tmpClearBuffer(buf);
    }
}

static int
smDelBuf(char c)
{
    int i, x, y, mselect;
    Buffer *buf;

    if (CurrentMenu->select < 0 || CurrentMenu->select >= SelectMenu.nitem)
	return (MENU_NOTHING);
    for (i = 0, buf = Firstbuf; i < CurrentMenu->select;
	 i++, buf = buf->nextBuffer) ;
    if (Currentbuf == buf)
	Currentbuf = buf->nextBuffer;
    Firstbuf = deleteBuffer(Firstbuf, buf);
    if (!Currentbuf)
	Currentbuf = nthBuffer(Firstbuf, i - 1);;
    if (Firstbuf == NULL) {
	Firstbuf = nullBuffer();
	Currentbuf = Firstbuf;
    }

    x = CurrentMenu->x;
    y = CurrentMenu->y;
    mselect = CurrentMenu->select;

    initSelectMenu();

    CurrentMenu->x = x;
    CurrentMenu->y = y;

    geom_menu(CurrentMenu, x, y, 0);

    CurrentMenu->select = (mselect <= CurrentMenu->nitem - 2) ? mselect
	: (CurrentMenu->nitem - 2);

    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    draw_all_menu(CurrentMenu);
    select_menu(CurrentMenu, CurrentMenu->select);
    return (MENU_NOTHING);
}

/* --- SelectMenu (END) --- */

/* --- SelTabMenu --- */

DEFUN(tabMn, TAB_MENU, "Popup tab selection menu")
{
    int x = Currentbuf->cursorX + Currentbuf->rootX,
	y = Currentbuf->cursorY + Currentbuf->rootY;

#ifdef USE_MOUSE
    if (mouse_action.in_action) {
	x = mouse_action.cursorX;
	y = mouse_action.cursorY;
    }
#endif
    popupMenu(x, y, &SelTabMenu);
}

static void
initSelTabMenu(void)
{
    int i, nitem, len = 0, l;
    TabBuffer *tab;
    Buffer *buf;
    Str str;
    char **label;
    char *p;
    static char *comment = " SPC for select / D for delete tab ";

    SelTabV = -1;
    for (i = 0, tab = LastTab; tab != NULL; i++, tab = tab->prevTab) {
	if (tab == CurrentTab)
	    SelTabV = i;
    }
    nitem = i;

    label = New_N(char *, nitem + 2);
    for (i = 0, tab = LastTab; i < nitem; i++, tab = tab->prevTab) {
	buf = tab->currentBuffer;
	str = Sprintf("<%s>", buf->buffername);
	if (buf->filename != NULL) {
	    switch (buf->currentURL.scheme) {
	    case SCM_LOCAL:
		if (strcmp(buf->currentURL.file, "-")) {
		    Strcat_char(str, ' ');
		    Strcat_charp(str,
				 conv_from_system(buf->currentURL.real_file));
		}
		break;
		/* case SCM_UNKNOWN: */
	    case SCM_MISSING:
		break;
	    default:
		p = parsedURL2Str(&buf->currentURL)->ptr;
		if (DecodeURL)
		    p = url_unquote_conv(p, 0);
		Strcat_charp(str, p);
		break;
	    }
	}
	label[i] = str->ptr;
	if (len < str->length)
	    len = str->length;
    }
    l = strlen(comment);
    if (len < l + 4)
	len = l + 4;
    if (len > COLS - 2 * FRAME_WIDTH)
	len = COLS - 2 * FRAME_WIDTH;
    len = (len > 1) ? ((len - l + 1) / 2) : 0;
    str = Strnew();
    for (i = 0; i < len; i++)
	Strcat_char(str, '-');
    Strcat_charp(str, comment);
    for (i = 0; i < len; i++)
	Strcat_char(str, '-');
    label[nitem] = str->ptr;
    label[nitem + 1] = NULL;

    new_option_menu(&SelTabMenu, label, &SelTabV, smChTab);
    SelTabMenu.initial = SelTabV;
    SelTabMenu.cursorX = Currentbuf->cursorX + Currentbuf->rootX;
    SelTabMenu.cursorY = Currentbuf->cursorY + Currentbuf->rootY;
    SelTabMenu.keymap['D'] = smDelTab;
    SelTabMenu.item[nitem].type = MENU_NOP;
}

static void
smChTab(void)
{
    int i;
    TabBuffer *tab;
    Buffer *buf;

    if (SelTabV < 0 || SelTabV >= SelTabMenu.nitem)
	return;
    for (i = 0, tab = LastTab; i < SelTabV && tab != NULL;
	 i++, tab = tab->prevTab) ;
    CurrentTab = tab;
    for (tab = LastTab; tab != NULL; tab = tab->prevTab) {
	if (tab == CurrentTab)
	    continue;
	buf = tab->currentBuffer;
#ifdef USE_IMAGE
	deleteImage(buf);
#endif
	if (clear_buffer)
	    tmpClearBuffer(buf);
    }
}

static int
smDelTab(char c)
{
    int i, x, y, mselect;
    TabBuffer *tab;

    if (CurrentMenu->select < 0 || CurrentMenu->select >= SelTabMenu.nitem)
	return (MENU_NOTHING);
    for (i = 0, tab = LastTab; i < CurrentMenu->select && tab != NULL;
	 i++, tab = tab->prevTab) ;
    deleteTab(tab);

    x = CurrentMenu->x;
    y = CurrentMenu->y;
    mselect = CurrentMenu->select;

    initSelTabMenu();

    CurrentMenu->x = x;
    CurrentMenu->y = y;

    geom_menu(CurrentMenu, x, y, 0);

    CurrentMenu->select = (mselect <= CurrentMenu->nitem - 2) ? mselect
	: (CurrentMenu->nitem - 2);

    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    draw_all_menu(CurrentMenu);
    select_menu(CurrentMenu, CurrentMenu->select);
    return (MENU_NOTHING);
}

/* --- SelectMenu (END) --- */

/* --- OptionMenu --- */

void
optionMenu(int x, int y, char **label, int *variable, int initial,
	   void (*func) ())
{
    Menu menu;

    set_menu_frame();

    new_option_menu(&menu, label, variable, func);
    menu.cursorX = COLS - 1;
    menu.cursorY = LASTLINE;
    menu.x = x;
    menu.y = y;
    menu.initial = initial;

    popup_menu(NULL, &menu);
}

/* --- OptionMenu (END) --- */

/* --- InitMenu --- */

static void
interpret_menu(FILE * mf)
{
    Str line;
    char *p, *s;
    int in_menu = 0, nmenu = 0, nitem = 0, type;
    MenuItem *item = NULL;
#ifdef USE_M17N
    wc_ces charset = SystemCharset;
#endif

    while (!feof(mf)) {
	line = Strfgets(mf);
	Strchop(line);
	Strremovefirstspaces(line);
	if (line->length == 0)
	    continue;
#ifdef USE_M17N
	line = wc_Str_conv(line, charset, InnerCharset);
#endif
	p = line->ptr;
	s = getWord(&p);
	if (*s == '#')		/* comment */
	    continue;
	if (in_menu) {
	    type = setMenuItem(&item[nitem], s, p);
	    if (type == -1)
		continue;	/* error */
	    if (type == MENU_END)
		in_menu = 0;
	    else {
		nitem++;
		item = New_Reuse(MenuItem, item, (nitem + 1));
		w3mMenuList[nmenu].item = item;
		item[nitem].type = MENU_END;
	    }
	}
	else if (!strcmp(s, "menu")) {
	    s = getQWord(&p);
	    if (*s == '\0')	/* error */
		continue;
	    in_menu = 1;
	    if ((nmenu = getMenuN(w3mMenuList, s)) != -1)
		w3mMenuList[nmenu].item = New(MenuItem);
	    else
		nmenu = addMenuList(&w3mMenuList, s);
	    item = w3mMenuList[nmenu].item;
	    nitem = 0;
	    item[nitem].type = MENU_END;
	}
#ifdef USE_M17N
	else if (!strcmp(s, "charset") || !strcmp(s, "encoding")) {
	    s = getQWord(&p);
	    if (*s == '\0')	/* error */
		continue;
	    charset = wc_guess_charset(s, charset);
	}
#endif
    }
}

void
initMenu(void)
{
    FILE *mf;
    MenuList *list;

    w3mMenuList = New_N(MenuList, 3);
    w3mMenuList[0].id = "Main";
    w3mMenuList[0].menu = &MainMenu;
    w3mMenuList[0].item = MainMenuItem;
    w3mMenuList[1].id = "Select";
    w3mMenuList[1].menu = &SelectMenu;
    w3mMenuList[1].item = NULL;
    w3mMenuList[2].id = "SelectTab";
    w3mMenuList[2].menu = &SelTabMenu;
    w3mMenuList[2].item = NULL;
    w3mMenuList[3].id = NULL;

#ifdef USE_M17N
    if (!MainMenuEncode) {
	MenuItem *item;
#ifdef ENABLE_NLS
	/* FIXME: charset that gettext(3) returns */
	MainMenuCharset = SystemCharset;
#endif
	for (item = MainMenuItem; item->type != MENU_END; item++)
	    item->label =
		wc_conv(_(item->label), MainMenuCharset,
			InnerCharset)->ptr;
	MainMenuEncode = TRUE;
    }
#endif
    if ((mf = fopen(confFile(MENU_FILE), "rt")) != NULL) {
	interpret_menu(mf);
	fclose(mf);
    }
    if ((mf = fopen(rcFile(MENU_FILE), "rt")) != NULL) {
	interpret_menu(mf);
	fclose(mf);
    }

    for (list = w3mMenuList; list->id != NULL; list++) {
	if (list->item == NULL)
	    continue;
	new_menu(list->menu, list->item);
    }
}

int
setMenuItem(MenuItem *item, char *type, char *line)
{
    char *label, *func, *popup, *keys, *data;
    int f;
    int n;

    if (type == NULL || *type == '\0')	/* error */
	return -1;
    if (strcmp(type, "end") == 0) {
	item->type = MENU_END;
	return MENU_END;
    }
    else if (strcmp(type, "nop") == 0) {
	item->type = MENU_NOP;
	item->label = getQWord(&line);
	return MENU_NOP;
    }
    else if (strcmp(type, "func") == 0) {
	label = getQWord(&line);
	func = getWord(&line);
	keys = getQWord(&line);
	data = getQWord(&line);
	if (*func == '\0')	/* error */
	    return -1;
	item->type = MENU_FUNC;
	item->label = label;
	f = getFuncList(func);
	item->func = w3mFuncList[(f >= 0) ? f : FUNCNAME_nulcmd].func;
	item->keys = keys;
	item->data = data;
	return MENU_FUNC;
    }
    else if (strcmp(type, "popup") == 0) {
	label = getQWord(&line);
	popup = getQWord(&line);
	keys = getQWord(&line);
	if (*popup == '\0')	/* error */
	    return -1;
	item->type = MENU_POPUP;
	item->label = label;
	if ((n = getMenuN(w3mMenuList, popup)) == -1)
	    n = addMenuList(&w3mMenuList, popup);
	item->popup = w3mMenuList[n].menu;
	item->keys = keys;
	return MENU_POPUP;
    }
    return -1;			/* error */
}

int
addMenuList(MenuList **mlist, char *id)
{
    int n;
    MenuList *list = *mlist;

    for (n = 0; list->id != NULL; list++, n++) ;
    *mlist = New_Reuse(MenuList, *mlist, (n + 2));
    list = *mlist + n;
    list->id = id;
    list->menu = New(Menu);
    list->item = New(MenuItem);
    (list + 1)->id = NULL;
    return n;
}

int
getMenuN(MenuList *list, char *id)
{
    int n;

    for (n = 0; list->id != NULL; list++, n++) {
	if (strcmp(id, list->id) == 0)
	    return n;
    }
    return -1;
}

/* --- InitMenu (END) --- */

LinkList *
link_menu(Buffer *buf)
{
    Menu menu;
    LinkList *l;
    int i, nitem, len = 0, linkV = -1;
    char **label;
    Str str;
    char *p;

    if (!buf->linklist)
	return NULL;

    for (i = 0, l = buf->linklist; l; i++, l = l->next) ;
    nitem = i;

    label = New_N(char *, nitem + 1);
    for (i = 0, l = buf->linklist; l; i++, l = l->next) {
	str = Strnew_charp(l->title ? l->title : "(empty)");
	if (l->type == LINK_TYPE_REL)
	    Strcat_charp(str, " [Rel] ");
	else if (l->type == LINK_TYPE_REV)
	    Strcat_charp(str, " [Rev] ");
	else
	    Strcat_charp(str, " ");
	if (!l->url)
	    p = "";
	else if (DecodeURL)
	    p = url_unquote_conv(l->url, buf->document_charset);
	else
	    p = l->url;
	Strcat_charp(str, p);
	label[i] = str->ptr;
	if (len < str->length)
	    len = str->length;
    }
    label[nitem] = NULL;

    set_menu_frame();
    new_option_menu(&menu, label, &linkV, NULL);

    menu.initial = 0;
    menu.cursorX = buf->cursorX + buf->rootX;
    menu.cursorY = buf->cursorY + buf->rootY;
    menu.x = menu.cursorX + FRAME_WIDTH + 1;
    menu.y = menu.cursorY + 2;

    popup_menu(NULL, &menu);

    if (linkV < 0)
	return NULL;
    for (i = 0, l = buf->linklist; l; i++, l = l->next) {
	if (i == linkV)
	    return l;
    }
    return NULL;
}

/* --- LinkMenu (END) --- */

Anchor *
accesskey_menu(Buffer *buf)
{
    Menu menu;
    AnchorList *al = buf->href;
    Anchor *a;
    Anchor **ap;
    int i, n, nitem = 0, key = -1;
    char **label;
    char *t;
    unsigned char c;

    if (!al)
	return NULL;
    for (i = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (!a->slave && a->accesskey && IS_ASCII(a->accesskey))
	    nitem++;
    }
    if (!nitem)
	return NULL;

    label = New_N(char *, nitem + 1);
    ap = New_N(Anchor *, nitem);
    for (i = 0, n = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (!a->slave && a->accesskey && IS_ASCII(a->accesskey)) {
	    t = getAnchorText(buf, al, a);
	    label[n] = Sprintf("%c: %s", a->accesskey, t ? t : "")->ptr;
	    ap[n] = a;
	    n++;
	}
    }
    label[nitem] = NULL;

    new_option_menu(&menu, label, &key, NULL);

    menu.initial = 0;
    menu.cursorX = buf->cursorX + buf->rootX;
    menu.cursorY = buf->cursorY + buf->rootY;
    menu.x = menu.cursorX + FRAME_WIDTH + 1;
    menu.y = menu.cursorY + 2;
    for (i = 0; i < 128; i++)
	menu.keyselect[i] = -1;
    for (i = 0; i < nitem; i++) {
	c = ap[i]->accesskey;
	menu.keymap[(int)c] = mSelect;
	menu.keyselect[(int)c] = i;
    }
    for (i = 0; i < nitem; i++) {
	c = ap[i]->accesskey;
	if (!IS_ALPHA(c) || menu.keyselect[n] >= 0)
	    continue;
	c = TOLOWER(c);
	menu.keymap[(int)c] = mSelect;
	menu.keyselect[(int)c] = i;
	c = TOUPPER(c);
	menu.keymap[(int)c] = mSelect;
	menu.keyselect[(int)c] = i;
    }

    a = retrieveCurrentAnchor(buf);
    if (a && a->accesskey && IS_ASCII(a->accesskey)) {
	for (i = 0; i < nitem; i++) {
	    if (a->hseq == ap[i]->hseq) {
		menu.initial = i;
		break;
	    }
	}
    }

    popup_menu(NULL, &menu);

    return (key >= 0) ? ap[key] : NULL;
}

static char lmKeys[] = "abcdefgimopqrstuvwxyz";
static char lmKeys2[] = "1234567890ABCDEFGHILMOPQRSTUVWXYZ";
#define nlmKeys (sizeof(lmKeys) - 1)
#define nlmKeys2 (sizeof(lmKeys2) - 1)

static int
lmGoto(char c)
{
    if (IS_ASCII(c) && CurrentMenu->keyselect[(int)c] >= 0) {
	goto_menu(CurrentMenu, CurrentMenu->nitem - 1, -1);
	goto_menu(CurrentMenu, CurrentMenu->keyselect[(int)c] * nlmKeys, 1);
    }
    return (MENU_NOTHING);
}

static int
lmSelect(char c)
{
    if (IS_ASCII(c))
	return select_menu(CurrentMenu, (CurrentMenu->select / nlmKeys) *
			   nlmKeys + CurrentMenu->keyselect[(int)c]);
    else
	return (MENU_NOTHING);
}

Anchor *
list_menu(Buffer *buf)
{
    Menu menu;
    AnchorList *al = buf->href;
    Anchor *a;
    Anchor **ap;
    int i, n, nitem = 0, key = -1, two = FALSE;
    char **label;
    char *t;
    unsigned char c;

    if (!al)
	return NULL;
    for (i = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (!a->slave)
	    nitem++;
    }
    if (!nitem)
	return NULL;

    if (nitem >= nlmKeys)
	two = TRUE;
    label = New_N(char *, nitem + 1);
    ap = New_N(Anchor *, nitem);
    for (i = 0, n = 0; i < al->nanchor; i++) {
	a = &al->anchors[i];
	if (!a->slave) {
	    t = getAnchorText(buf, al, a);
	    if (!t)
		t = "";
	    if (two && n >= nlmKeys2 * nlmKeys)
		label[n] = Sprintf("  : %s", t)->ptr;
	    else if (two)
		label[n] = Sprintf("%c%c: %s", lmKeys2[n / nlmKeys],
				   lmKeys[n % nlmKeys], t)->ptr;
	    else
		label[n] = Sprintf("%c: %s", lmKeys[n], t)->ptr;
	    ap[n] = a;
	    n++;
	}
    }
    label[nitem] = NULL;

    set_menu_frame();
    set_menu_frame();
    new_option_menu(&menu, label, &key, NULL);

    menu.initial = 0;
    menu.cursorX = buf->cursorX + buf->rootX;
    menu.cursorY = buf->cursorY + buf->rootY;
    menu.x = menu.cursorX + FRAME_WIDTH + 1;
    menu.y = menu.cursorY + 2;
    for (i = 0; i < 128; i++)
	menu.keyselect[i] = -1;
    if (two) {
	for (i = 0; i < nlmKeys2; i++) {
	    c = lmKeys2[i];
	    menu.keymap[(int)c] = lmGoto;
	    menu.keyselect[(int)c] = i;
	}
	for (i = 0; i < nlmKeys; i++) {
	    c = lmKeys[i];
	    menu.keymap[(int)c] = lmSelect;
	    menu.keyselect[(int)c] = i;
	}
    }
    else {
	for (i = 0; i < nitem; i++) {
	    c = lmKeys[i];
	    menu.keymap[(int)c] = mSelect;
	    menu.keyselect[(int)c] = i;
	}
    }

    a = retrieveCurrentAnchor(buf);
    if (a) {
	for (i = 0; i < nitem; i++) {
	    if (a->hseq == ap[i]->hseq) {
		menu.initial = i;
		break;
	    }
	}
    }

    popup_menu(NULL, &menu);

    return (key >= 0) ? ap[key] : NULL;
}

#endif				/* USE_MENU */
