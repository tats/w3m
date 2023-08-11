/* $Id: terms.h,v 1.10 2004/07/15 16:32:39 ukai Exp $ */
#ifndef TERMS_H
#define TERMS_H

extern int LINES, COLS;
#if defined(__CYGWIN__)
extern int LASTLINE;
#endif

void clear(void);
void flush_tty(void);
void setlinescols(void);

#ifdef USE_MOUSE
/* Addition:mouse event */
#define MOUSE_BTN1_DOWN 0
#define MOUSE_BTN2_DOWN 1
#define MOUSE_BTN3_DOWN 2
#define MOUSE_BTN4_DOWN_RXVT 3
#define MOUSE_BTN5_DOWN_RXVT 4
#define MOUSE_BTN4_DOWN_XTERM 64
#define MOUSE_BTN5_DOWN_XTERM 65
#define MOUSE_BTN_UP 3
#define MOUSE_BTN_RESET -1

void mouse_end(void);
#endif

#ifdef __CYGWIN__
#if CYGWIN_VERSION_DLL_MAJOR < 1005 && defined(USE_MOUSE)
extern int cygwin_mouse_btn_swapped;
#endif
#ifdef SUPPORT_WIN9X_CONSOLE_MBCS
extern void enable_win9x_console_input(void);
extern void disable_win9x_console_input(void);
#endif
#endif

#ifdef USE_IMAGE
extern void put_image_osc5379(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh);
extern void put_image_sixel(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int n_terminal_image);
extern void put_image_iterm2(char *url, int x, int y, int w, int h);
extern void put_image_kitty(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int c, int r);
extern int get_pixel_per_cell(int *ppc, int *ppl);
#endif

char getch(void);

#endif				/* not TERMS_H */
