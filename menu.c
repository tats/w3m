
/* 
 * w3m menu.c
 */
#include <stdio.h>

#include "fm.h"
#include "menu.h"
#include "func.h"
#include "myctype.h"
#include "regex.h"

#ifdef MOUSE
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
#endif				/* defined(USE_GPM) || * * * * * *
				 * defined(USE_SYSMOUSE) */
#endif				/* MOUSE */

#ifdef MENU

#ifdef KANJI_SYMBOLS
static char *FRAME[] =
{
#ifdef MENU_THIN_FRAME
    "┌", "─", "┐",
    "│", "  ", "│",
    "└", "─", "┘",
#else				/* not MENU_THIN_FRAME */
    "┏", "━", "┓",
    "┃", "  ", "┃",
    "┗", "━", "┛",
#endif				/* not MENU_THIN_FRAME */
    "：", "："};
#define FRAME_WIDTH 2

#define G_start
/**/
#define G_end    /**/

#else				/* not KANJI_SYMBOLS */
static char *N_FRAME[] =
{
    "+", "-", "+",
    "|", " ", "|",
    "+", "-", "+",
    ":", ":"};
#define FRAME_WIDTH 1

static char *G_FRAME[] =
{
    "l", "q", "k",
    "x", " ", "x",
    "m", "q", "j",
    ":", ":"};

static char **FRAME = NULL;
static int graph_mode = FALSE;

#define G_start  {if (graph_mode) graphstart();}
#define G_end    {if (graph_mode) graphend();}
#endif				/* not KANJI_SYMBOLS */

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
static int	mPc(char c);
#endif

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
    mUp,    mNull,  mNull,  mNull,  mNull,  mNull,  mNext,  mNull,
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
    mNull,  mNull,  mNull,  mNull,  mNull,  mNull,  mSrchP, mNull,
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

/* --- SelectMenu --- */

static Menu SelectMenu;
static int SelectV = 0;
static void initSelectMenu(void);
static void smChBuf(void);
static int smDelBuf(char c);

/* --- SelectMenu (END) --- */

/* --- MainMenu --- */

static Menu MainMenu;
#if LANG == JA
static MenuItem MainMenuItem[] =
{
/* type        label         variabel value func     popup keys data  */
    {MENU_FUNC, "戻る         (b)", NULL, 0, backBf, NULL, "b", NULL},
    {MENU_FUNC, "バッファ選択 (s)", NULL, 0, selBuf, NULL, "s", NULL},
    {MENU_FUNC, "ソースを表示 (v)", NULL, 0, vwSrc, NULL, "vV", NULL},
    {MENU_FUNC, "ソースを編集 (e)", NULL, 0, editBf, NULL, "eE", NULL},
    {MENU_FUNC, "ソースを保存 (S)", NULL, 0, svSrc, NULL, "S", NULL},
    {MENU_FUNC, "再読み込み   (r)", NULL, 0, reload, NULL, "rR", NULL},
    {MENU_NOP, "────────", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, "リンクを表示 (a)", NULL, 0, followA, NULL, "a", NULL},
    {MENU_FUNC, "リンクを保存 (A)", NULL, 0, svA, NULL, "A", NULL},
    {MENU_FUNC, "画像を表示   (i)", NULL, 0, followI, NULL, "i", NULL},
    {MENU_FUNC, "画像を保存   (I)", NULL, 0, svI, NULL, "I", NULL},
    {MENU_FUNC, "フレーム表示 (f)", NULL, 0, rFrame, NULL, "fF", NULL},
    {MENU_NOP, "────────", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, "ブックマーク (B)", NULL, 0, ldBmark, NULL, "B", NULL},
    {MENU_FUNC, "ヘルプ       (h)", NULL, 0, ldhelp, NULL, "hH", NULL},
    {MENU_FUNC, "オプション   (o)", NULL, 0, ldOpt, NULL, "oO", NULL},
    {MENU_NOP, "────────", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, "終了         (q)", NULL, 0, qquitfm, NULL, "qQ", NULL},
    {MENU_END, "", NULL, 0, nulcmd, NULL, "", NULL},
};
#else				/* LANG != JA */
static MenuItem MainMenuItem[] =
{
/* type        label           variable value func     popup keys data  */
    {MENU_FUNC, " Back         (b) ", NULL, 0, backBf, NULL, "b", NULL},
    {MENU_FUNC, " Select Buffer(s) ", NULL, 0, selBuf, NULL, "s", NULL},
    {MENU_FUNC, " View Source  (v) ", NULL, 0, vwSrc, NULL, "vV", NULL},
    {MENU_FUNC, " Edit Source  (e) ", NULL, 0, editBf, NULL, "eE", NULL},
    {MENU_FUNC, " Save Source  (S) ", NULL, 0, svSrc, NULL, "S", NULL},
    {MENU_FUNC, " Reload       (r) ", NULL, 0, reload, NULL, "rR", NULL},
    {MENU_NOP, " ---------------- ", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, " Go Link      (a) ", NULL, 0, followA, NULL, "a", NULL},
    {MENU_FUNC, " Save Link    (A) ", NULL, 0, svA, NULL, "A", NULL},
    {MENU_FUNC, " View Image   (i) ", NULL, 0, followI, NULL, "i", NULL},
    {MENU_FUNC, " Save Image   (I) ", NULL, 0, svI, NULL, "I", NULL},
    {MENU_FUNC, " View Frame   (f) ", NULL, 0, rFrame, NULL, "fF", NULL},
    {MENU_NOP, " ---------------- ", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, " Bookmark     (B) ", NULL, 0, ldBmark, NULL, "B", NULL},
    {MENU_FUNC, " Help         (h) ", NULL, 0, ldhelp, NULL, "hH", NULL},
    {MENU_FUNC, " Option       (o) ", NULL, 0, ldOpt, NULL, "oO", NULL},
    {MENU_NOP, " ---------------- ", NULL, 0, nulcmd, NULL, "", NULL},
    {MENU_FUNC, " Quit         (q) ", NULL, 0, qquitfm, NULL, "qQ", NULL},
    {MENU_END, "", NULL, 0, nulcmd, NULL, "", NULL},
};
#endif				/* LANG != JA  */

/* --- MainMenu (END) --- */

extern int w3mNFuncList;
extern FuncList w3mFuncList[];
static MenuList *w3mMenuList;

static Menu *CurrentMenu = NULL;

#define mvaddch(y, x, c)        (move(y, x), addch(c))
#define mvaddstr(y, x, str)     (move(y, x), addstr(str))
#define mvaddnstr(y, x, str, n) (move(y, x), addnstr_sup(str, n))

void
new_menu(Menu * menu, MenuItem * item)
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

    for (i = 0; item[i].type != MENU_END; i++);
    menu->nitem = i;
    menu->height = menu->nitem;
    for (i = 0; i < 128; i++)
	menu->keymap[i] = MenuKeymap[i];
    menu->width = 0;
    for (i = 0; i < menu->nitem; i++) {
	if ((p = item[i].keys) != NULL) {
	    while (*p) {
		if (IS_ASCII(*p)) {
		    menu->keymap[(int) *p] = mSelect;
		    menu->keyselect[(int) *p] = i;
		}
		p++;
	    }
	}
	l = strlen(item[i].label);
	if (l > menu->width)
	    menu->width = l;
    }
}

void
geom_menu(Menu * menu, int x, int y, int select)
{
    int win_x, win_y, win_w, win_h;

    menu->select = select;

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

    win_y = menu->y - select - 1;
    win_h = menu->height + 2;
    if (win_y + win_h > LASTLINE)
	win_y = LASTLINE - win_h;
    if (win_y < 0) {
	win_y = 0;
	if (win_y + win_h > LASTLINE) {
	    win_h = LASTLINE - win_y;
	    menu->height = win_h - 2;
	    if (menu->height <= select)
		menu->offset = select - menu->height + 1;
	}
    }
    menu->y = win_y + 1;
}

void
draw_all_menu(Menu * menu)
{
    if (menu->parent != NULL)
	draw_all_menu(menu->parent);
    draw_menu(menu);
}

void
draw_menu(Menu * menu)
{
    int x, y, w;
    int i, j;

    x = menu->x - FRAME_WIDTH;
    w = menu->width + 2 * FRAME_WIDTH;
    y = menu->y - 1;

#ifndef KANJI_SYMBOLS
    if (FRAME == NULL) {
	if (graph_ok()) {
	    graph_mode = TRUE;
	    FRAME = G_FRAME;
	}
	else {
	    FRAME = N_FRAME;
	}
    }
#endif				/* not KANJI_SYMBOLS */

    if (menu->offset == 0) {
	G_start;
	mvaddstr(y, x, FRAME[0]);
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[1]);
	mvaddstr(y, x + i, FRAME[2]);
	G_end;
    }
    else {
	G_start;
	mvaddstr(y, x, FRAME[3]);
	G_end;
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[4]);
	G_start;
	mvaddstr(y, x + i, FRAME[5]);
	G_end;
	i = (w / 2 - 1) / FRAME_WIDTH * FRAME_WIDTH;
	mvaddstr(y, x + i, FRAME[9]);
    }

    for (j = 0; j < menu->height; j++) {
	y++;
	G_start;
	mvaddstr(y, x, FRAME[3]);
	G_end;
	draw_menu_item(menu, menu->offset + j);
	G_start;
	mvaddstr(y, x + w - FRAME_WIDTH, FRAME[5]);
	G_end;
    }
    y++;
    if (menu->offset + menu->height == menu->nitem) {
	G_start;
	mvaddstr(y, x, FRAME[6]);
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[7]);
	mvaddstr(y, x + i, FRAME[8]);
	G_end;
    }
    else {
	G_start;
	mvaddstr(y, x, FRAME[3]);
	G_end;
	for (i = FRAME_WIDTH; i < w - FRAME_WIDTH; i += FRAME_WIDTH)
	    mvaddstr(y, x + i, FRAME[4]);
	G_start;
	mvaddstr(y, x + i, FRAME[5]);
	G_end;
	i = (w / 2 - 1) / FRAME_WIDTH * FRAME_WIDTH;
	mvaddstr(y, x + i, FRAME[10]);
    }
}

void
draw_menu_item(Menu * menu, int select)
{
    mvaddnstr(menu->y + select - menu->offset, menu->x,
	      menu->item[select].label, menu->width);
}

int
select_menu(Menu * menu, int select)
{
    if (select < 0 || select >= menu->nitem)
	return (MENU_NOTHING);
    if (select < menu->offset)
	up_menu(menu, menu->offset - select);
    else if (select >= menu->offset + menu->height)
	down_menu(menu, select - menu->offset - menu->height + 1);

    if (menu->select >= menu->offset &&
	menu->select < menu->offset + menu->height)
	draw_menu_item(menu, menu->select);
    menu->select = select;
    standout();
    draw_menu_item(menu, menu->select);
    standend();
/* 
 * move(menu->cursorY, menu->cursorX); */
    move(menu->y + select - menu->offset, menu->x);
    toggle_stand();
    refresh();

    return (menu->select);
}

void
goto_menu(Menu * menu, int select, int down)
{
    int select_in;
    if (select >= menu->nitem)
	select = menu->nitem - 1;
    else if (select < 0)
	select = 0;
    select_in = select;
    while (menu->item[select].type == MENU_NOP) {
	if (down > 0) {
	    if (++select >= menu->nitem)
	    {
		down_menu(menu, select_in - menu->select);
		select = menu->select;
		break;
	    }
	}
	else if (down < 0) {
	    if (--select < 0)
	    {
		up_menu(menu, menu->select - select_in);
		select = menu->select;
		break;
	    }
	}
	else {
	    return;
	}
    }
    select_menu(menu, select);
}

void
up_menu(Menu * menu, int n)
{
    if (n < 0 || menu->offset == 0)
	return;
    menu->offset -= n;
    if (menu->offset < 0)
	menu->offset = 0;

    draw_menu(menu);
}

void
down_menu(Menu * menu, int n)
{
    if (n < 0 || menu->offset + menu->height == menu->nitem)
	return;
    menu->offset += n;
    if (menu->offset + menu->height > menu->nitem)
	menu->offset = menu->nitem - menu->height;

    draw_menu(menu);
}

int
action_menu(Menu * menu)
{
    char c;
    int select;
    MenuItem item;

    if (menu->active == 0) {
	if (menu->parent != NULL)
	    menu->parent->active = 0;
	return (0);
    }
    draw_all_menu(menu);
    select_menu(menu, menu->select);

    while (1) {
#ifdef MOUSE
	if (use_mouse)
	    mouse_active();
#endif				/* MOUSE */
	c = getch();
#ifdef MOUSE
	if (use_mouse)
	    mouse_inactive();
#if defined(USE_GPM) || defined(USE_SYSMOUSE)
	if (c == X_MOUSE_SELECTED) {
	    select = X_Mouse_Selection;
	    if (select != MENU_NOTHING)
		break;
	}
#endif				/* defined(USE_GPM) || * * * * * *
				 * defined(USE_SYSMOUSE) */
#endif				/* MOUSE */
	if (IS_ASCII(c)) {	/* Ascii */
	    select = (*menu->keymap[(int) c]) (c);
	    if (select != MENU_NOTHING)
		break;
	}
    }
    if (select >= 0 && select < menu->nitem) {
	item = menu->item[select];
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
	    CurrentMenuData = item.data;
	    (*item.func) ();
	    CurrentMenuData = NULL;
	}
    }
    else if (select == MENU_CLOSE) {
	if (menu->parent != NULL)
	    menu->parent->active = 0;
    }
    return (0);
}

void
popup_menu(Menu * parent, Menu * menu)
{
    int active = 1;

    if (menu->item == NULL || menu->nitem == 0)
	return;
    if (menu->active)
	return;

#ifdef MOUSE
#ifdef USE_GPM
    gpm_handler = gpm_process_menu_mouse;
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
    sysm_handler = sysm_process_menu_mouse;
#endif				/* USE_SYSMOUSE */
#endif				/* MOUSE */
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
#ifdef MOUSE
#ifdef USE_GPM
    if (CurrentMenu == NULL)
	gpm_handler = gpm_process_mouse;
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
    if (CurrentMenu == NULL)
	sysm_handler = sysm_process_mouse;
#endif				/* USE_SYSMOUSE */
#endif				/* MOUSE */
}

void
guess_menu_xy(Menu * parent, int width, int *x, int *y)
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
new_option_menu(Menu * menu, char **label, int *variable, void (*func) ())
{
    int i, nitem;
    char **p;
    MenuItem *item;

    if (label == NULL || *label == NULL)
	return;

    for (i = 0, p = label; *p != NULL; i++, p++);
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

/* --- MenuFunctions --- */

#ifdef __EMX__
static int
mPc(char c)
{
  c = getch();
  return(MenuPcKeymap[(int)c](c));
}
#endif

static int
mEsc(char c)
{
    c = getch();
    return (MenuEscKeymap[(int) c] (c));
}

static int
mEscB(char c)
{
    c = getch();
    if (IS_DIGIT(c))
	return (mEscD(c));
    else
	return (MenuEscBKeymap[(int) c] (c));
}

static int
mEscD(char c)
{
    int d;

    d = (int) c - (int) '0';
    c = getch();
    if (IS_DIGIT(c)) {
	d = d * 10 + (int) c - (int) '0';
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
	return (select_menu(CurrentMenu, CurrentMenu->keyselect[(int) c]));
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
    int select = CurrentMenu->select + CurrentMenu->height;

    if (select >= CurrentMenu->nitem)
	return mLast(c);
    down_menu(CurrentMenu, CurrentMenu->height);
    goto_menu(CurrentMenu, select, -1);
    return (MENU_NOTHING);
}

static int
mPrev(char c)
{
    int select = CurrentMenu->select - CurrentMenu->height;

    if (select < 0)
	return mTop(c);
    up_menu(CurrentMenu, CurrentMenu->height);
    goto_menu(CurrentMenu, select, 1);
    return (MENU_NOTHING);
}

static int
mFore(char c)
{
    if (CurrentMenu->select >= CurrentMenu->nitem - 1)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, (CurrentMenu->select + CurrentMenu->height - 1), (CurrentMenu->height + 1));
    return (MENU_NOTHING);
}

static int
mBack(char c)
{
    if (CurrentMenu->select <= 0)
	return (MENU_NOTHING);
    goto_menu(CurrentMenu, (CurrentMenu->select - CurrentMenu->height + 1), (-1 - CurrentMenu->height));
    return (MENU_NOTHING);
}

static int
mOk(char c)
{
    int select = CurrentMenu->select;

    if (CurrentMenu->item[select].type == MENU_NOP)
	return (MENU_NOTHING);
    return (select);
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
menuForwardSearch (Menu* menu, char* str, int from)
{
    int i;
    char* p;
    if ((p = regexCompile (str, IgnoreCase)) != NULL) {
	message (p, 0, 0);
	return -1;
    }
    for (i = from; i < menu->nitem; i++)
	if (regexMatch (menu->item[i].label, 0, 0) == 1)
            return i;
    return -1;
}

int
menu_search_forward (Menu* menu, int from)
{
    char *str;
    int found;
    str = inputStrHist("Forward: ", NULL, TextHist);
    if (str != NULL && *str == '\0')
	str = SearchString;
    if (str == NULL || *str == '\0')
	return (MENU_NOTHING);
    SearchString = str;
    found = menuForwardSearch (menu, SearchString, from);
    if (WrapSearch && found == -1)
        found = menuForwardSearch (menu, SearchString, 0);
    menuSearchRoutine = menuForwardSearch;
    if (found >= 0)
        return found;
    return from;
}

static int
mSrchF (char c)
{
    int select;
    select = menu_search_forward (CurrentMenu, CurrentMenu->select);
    goto_menu (CurrentMenu, select, 1);
    return (MENU_NOTHING);
}

static int
menuBackwardSearch (Menu* menu, char* str, int from)
{
    int i;
    char* p;
    if ((p = regexCompile (str, IgnoreCase)) != NULL) {
	message (p, 0, 0);
	return -1;
    }
    for (i = from; i >= 0 ; i--)
	if (regexMatch (menu->item[i].label, 0, 0) == 1)
            return i;
    return -1;
}

int
menu_search_backward (Menu* menu, int from)
{
    char *str;
    int found;
    str = inputStrHist("Backward: ", NULL, TextHist);
    if (str != NULL && *str == '\0')
	str = SearchString;
    if (str == NULL || *str == '\0')
	return (MENU_NOTHING);
    SearchString = str;
    found = menuBackwardSearch (menu, SearchString, from);
    if (WrapSearch && found == -1)
        found = menuBackwardSearch (menu, SearchString, 0);
    menuSearchRoutine = menuBackwardSearch;
    if (found >= 0)
        return found;
    return from;
}

static int
mSrchB (char c)
{
    int select;
    select = menu_search_backward (CurrentMenu, CurrentMenu->select);
    goto_menu (CurrentMenu, select, -1);
    return (MENU_NOTHING);
}

static int
menu_search_next_previous (Menu* menu, int from, int reverse)
{
    int found;
    int new_from;
    static int (*routine[2]) (Menu *, char *, int) =
    {
	menuForwardSearch, menuBackwardSearch
    };

    if (menuSearchRoutine == NULL) {
	disp_message ("No previous regular expression", TRUE);
	return from;
    }
    addstr(menuSearchRoutine == menuForwardSearch ? "Forward: " : "Backward: ");
    addstr(SearchString);
    if (reverse != 0)
	reverse = 1;
    if (menuSearchRoutine == menuBackwardSearch)
	reverse ^= 1;
    new_from = from - reverse * 2 + 1;
    if (new_from >=0 && new_from < menu->nitem)
	found = (*routine[reverse]) (menu, SearchString, new_from);
    else
	found = (*routine[reverse]) (menu, SearchString, from);
    if (WrapSearch && found == -1) {
        found = (*routine[reverse]) (menu, SearchString, reverse * menu->nitem);
    }
    if (found >= 0)
        return found;
    return from;
}

static int
mSrchN (char c)
{
    int select;
    select = menu_search_next_previous (CurrentMenu, CurrentMenu->select, 0);
    goto_menu (CurrentMenu, select, 1);
    return (MENU_NOTHING);
}

static int
mSrchP (char c)
{
    int select;
    select = menu_search_next_previous (CurrentMenu, CurrentMenu->select, 1);
    goto_menu (CurrentMenu, select, -1);
    return (MENU_NOTHING);
}

#ifdef MOUSE
#define MOUSE_BTN1_DOWN 0
#define MOUSE_BTN2_DOWN 1
#define MOUSE_BTN3_DOWN 2
#define MOUSE_BTN4_DOWN_RXVT 3
#define MOUSE_BTN5_DOWN_RXVT 4
#define MOUSE_BTN4_DOWN_XTERM 64
#define MOUSE_BTN5_DOWN_XTERM 65
#define MOUSE_BTN_UP 3
#define MOUSE_BTN_RESET -1
#define MOUSE_SCROLL_LINE 5

static int
process_mMouse(int btn, int x, int y)
{
    Menu *menu;
    int select;
    static int press_btn = MOUSE_BTN_RESET, press_x, press_y;
    char c = ' ';

    menu = CurrentMenu;

    if (x < 0 || x >= COLS || y < 0 || y > LASTLINE)
	return (MENU_NOTHING);

    if (btn == MOUSE_BTN_UP) {
	if (press_btn == MOUSE_BTN1_DOWN ||
	    press_btn == MOUSE_BTN3_DOWN) {
	    if (x < menu->x - FRAME_WIDTH ||
		x >= menu->x + menu->width + FRAME_WIDTH ||
		y < menu->y - 1 ||
		y >= menu->y + menu->height + 1) {
		return (MENU_CANCEL);
	    }
	    else if ((x >= menu->x - FRAME_WIDTH &&
		      x < menu->x) ||
		     (x >= menu->x + menu->width &&
		      x < menu->x + menu->width + FRAME_WIDTH)) {
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
		select = y - menu->y + menu->offset;
		if (menu->item[select].type == MENU_NOP)
		    return (MENU_NOTHING);
		return (select_menu(menu, select));
	    }
	}
    }
    else {
	press_btn = btn;
	press_x = x;
	press_y = y;
    }
    return (MENU_NOTHING);
}

static int
mMouse(char c)
{
    int btn, x, y;

    btn = (unsigned char) getch() - 32;
    x = (unsigned char) getch() - 33;
    if (x < 0)
	x += 0x100;
    y = (unsigned char) getch() - 33;
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
    int btn, x, y;
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
#else				/* not MOUSE */
static int
mMouse(char c)
{
    return (MENU_NOTHING);
}
#endif				/* not MOUSE */

/* --- MenuFunctions (END) --- */

/* --- MainMenu --- */

void
popupMenu(int x, int y, Menu *menu)
{
    initSelectMenu();

    menu->cursorX = Currentbuf->cursorX;
    menu->cursorY = Currentbuf->cursorY;
    menu->x = x + FRAME_WIDTH + 1;
    menu->y = y + 2;

    popup_menu(NULL, menu);
}

void
mainMenu(int x, int y)
{
    popupMenu(x, y, &MainMenu);
}

void
mainMn(void)
{
    Menu *menu = &MainMenu;
    char *data;
    int n;

    data = searchKeyData();
    if (data != NULL) {
	n = getMenuN(w3mMenuList, data);
	if (n < 0)
	    return;
	menu = w3mMenuList[n].menu;
    }
    popupMenu(Currentbuf->cursorX, Currentbuf->cursorY, menu);
}

/* --- MainMenu (END) --- */

/* --- SelectMenu --- */

static void
initSelectMenu(void)
{
    int i, nitem, len = 0, l;
    Buffer *buf;
    Str str;
    char **label;
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
	    case SCM_LOCAL_CGI:
		if (strcmp(buf->currentURL.file, "-")) {
		    Strcat_char(str, ' ');
		    Strcat_charp(str, buf->filename);
		}
		break;
	    case SCM_UNKNOWN:
	    case SCM_MISSING:
		break;
	    default:
		Strcat_char(str, ' ');
		Strcat(str, parsedURL2Str(&buf->currentURL));
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

    new_option_menu(&SelectMenu, label, &SelectV, smChBuf);
    SelectMenu.initial = SelectV;
    SelectMenu.cursorX = Currentbuf->cursorX;
    SelectMenu.cursorY = Currentbuf->cursorY;
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
    for (i = 0, buf = Firstbuf; i < SelectV; i++, buf = buf->nextBuffer);
    Currentbuf = buf;
    if (clear_buffer) {
	for (buf = Firstbuf; buf != NULL; buf = buf->nextBuffer)
	    tmpClearBuffer(buf);
    }
}

static int
smDelBuf(char c) {
    int i, x, y, select;
    Buffer *buf;

    if (CurrentMenu->select < 0 || CurrentMenu->select >= SelectMenu.nitem)
	return (MENU_NOTHING);
    for (i = 0, buf = Firstbuf; i < CurrentMenu->select; i++, buf = buf->nextBuffer);
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
    select = CurrentMenu->select;

    initSelectMenu();

    CurrentMenu->x = x;
    CurrentMenu->y = y;

    geom_menu(CurrentMenu, x, y, 0);

    CurrentMenu->select = (select <= CurrentMenu->nitem - 2) ? select
	: (CurrentMenu->nitem - 2);

    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    draw_all_menu(CurrentMenu);
    select_menu(CurrentMenu, CurrentMenu->select);
    return (MENU_NOTHING);
}

/* --- SelectMenu (END) --- */

/* --- OptionMenu --- */

void
optionMenu(int x, int y, char **label, int *variable, int initial, void (*func) ())
{
    Menu menu;

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

void
initMenu(void)
{
    FILE *mf;
    Str line;
    char *p, *s;
    int in_menu, nmenu, nitem, type;
    MenuItem *item;
    MenuList *list;

    w3mMenuList = New_N(MenuList, 3);
    w3mMenuList[0].id = "Main";
    w3mMenuList[0].menu = &MainMenu;
    w3mMenuList[0].item = MainMenuItem;
    w3mMenuList[1].id = "Select";
    w3mMenuList[1].menu = &SelectMenu;
    w3mMenuList[1].item = NULL;
    w3mMenuList[2].id = NULL;

    if ((mf = fopen(rcFile(MENU_FILE), "rt")) == NULL)
	goto create_menu;

    if (!w3mNFuncList)
	w3mNFuncList = countFuncList(w3mFuncList);

    in_menu = 0;
    while (!feof(mf)) {
	line = Strfgets(mf);
	Strchop(line);
	Strremovefirstspaces(line);
	if (line->length == 0)
	    continue;
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
	else {
	    if (strcmp(s, "menu"))	/* error */
		continue;
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
    }
    fclose(mf);

  create_menu:
    for (list = w3mMenuList; list->id != NULL; list++) {
	if (list->item == NULL)
	    continue;
	new_menu(list->menu, list->item);
    }
}

int
setMenuItem(MenuItem * item, char *type, char *line)
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
	f = getFuncList(func, w3mFuncList, w3mNFuncList);
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
addMenuList(MenuList ** mlist, char *id)
{
    int n;
    MenuList *list = *mlist;

    for (n = 0; list->id != NULL; list++, n++);
    *mlist = New_Reuse(MenuList, *mlist, (n + 2));
    list = *mlist + n;
    list->id = id;
    list->menu = New(Menu);
    list->item = New(MenuItem);
    (list + 1)->id = NULL;
    return n;
}

int
getMenuN(MenuList * list, char *id)
{
    int n;

    for (n = 0; list->id != NULL; list++, n++) {
	if (strcmp(id, list->id) == 0)
	    return n;
    }
    return -1;
}

/* --- InitMenu (END) --- */

#endif				/* MENU */
