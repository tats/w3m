/* $Id: terms.h,v 1.10 2004/07/15 16:32:39 ukai Exp $ */
#ifndef TERMS_H
#define TERMS_H

extern int LINES, COLS;
#if defined(__CYGWIN__)
extern int LASTLINE;
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

#endif				/* not TERMS_H */
