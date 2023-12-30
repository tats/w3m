/* $Id: terms.c,v 1.63 2010/08/20 09:34:47 htrb Exp $ */
/* 
 * An original curses library for EUC-kanji by Akinori ITO,     December 1989
 * revised by Akinori ITO, January 1995
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include <string.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifndef __MINGW32_VERSION
#include <sys/ioctl.h>
#else
#include <winsock.h>
#endif /* __MINGW32_VERSION */
#ifdef USE_MOUSE
#ifdef USE_GPM
#include <gpm.h>
#endif				/* USE_GPM */
#ifdef USE_SYSMOUSE
#include <osreldate.h>
#if (__FreeBSD_version >= 400017) || (__FreeBSD_kernel_version >= 400017)
#include <sys/consio.h>
#include <sys/fbio.h>
#else
#include <machine/console.h>
#endif
int (*sysm_handler) (int x, int y, int nbs, int obs);
static int cwidth = 8, cheight = 16;
static int xpix, ypix, nbs, obs = 0;
#endif				/* use_SYSMOUSE */

static int is_xterm = 0;

void mouse_init(void), mouse_end(void);
int mouseActive = 0;
#endif				/* USE_MOUSE */

static char *title_str = NULL;

static int tty;

#include "terms.h"
#include "fm.h"
#include "myctype.h"

#ifdef __EMX__
#define INCL_DOSNLS
#include <os2.h>
#endif				/* __EMX__ */

#if defined(__CYGWIN__)
#include <windows.h>
#include <sys/cygwin.h>
static int isWinConsole = 0;
#define TERM_CYGWIN 1
#define TERM_CYGWIN_RESERVE_IME 2
static int isLocalConsole = 0;

#if CYGWIN_VERSION_DLL_MAJOR < 1005 && defined(USE_MOUSE)
int cygwin_mouse_btn_swapped = 0;
#endif

#if defined(SUPPORT_WIN9X_CONSOLE_MBCS)
static HANDLE hConIn = INVALID_HANDLE_VALUE;
static int isWin95 = 0;
static char *ConInV;
static int iConIn, nConIn, nConInMax;

static void
check_win9x(void)
{
    OSVERSIONINFO winVersionInfo;

    winVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&winVersionInfo) == 0) {
	fprintf(stderr, "can't get Windows version information.\n");
	exit(1);
    }
    if (winVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
	isWin95 = 1;
    }
    else {
	isWin95 = 0;
    }
}

void
enable_win9x_console_input(void)
{
    if (isWin95 && isWinConsole && isLocalConsole &&
	hConIn == INVALID_HANDLE_VALUE) {
	hConIn = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_WRITE,
			    NULL, OPEN_EXISTING, 0, NULL);
	if (hConIn != INVALID_HANDLE_VALUE) {
	    getch();
	}
    }
}

void
disable_win9x_console_input(void)
{
    if (hConIn != INVALID_HANDLE_VALUE) {
	CloseHandle(hConIn);
	hConIn = INVALID_HANDLE_VALUE;
    }
}

static void
expand_win32_console_input_buffer(int n)
{
    if (nConIn + n >= nConInMax) {
	char *oldv;

	nConInMax = ((nConIn + n) / 2 + 1) * 3;
	oldv = ConInV;
	ConInV = GC_MALLOC_ATOMIC(nConInMax);
	memcpy(ConInV, oldv, nConIn);
    }
}

static int
read_win32_console_input(void)
{
    INPUT_RECORD rec;
    DWORD nevents;

    if (PeekConsoleInput(hConIn, &rec, 1, &nevents) && nevents) {
	switch (rec.EventType) {
	case KEY_EVENT:
	    expand_win32_console_input_buffer(3);

	    if (ReadConsole(hConIn, &ConInV[nConIn], 1, &nevents, NULL)) {
		nConIn += nevents;
		return nevents;
	    }

	    break;
	default:
	    break;
	}

	ReadConsoleInput(hConIn, &rec, 1, &nevents);
    }
    return 0;
}

static int
read_win32_console(char *s, int n)
{
    KEY_EVENT_RECORD *ker;

    if (hConIn == INVALID_HANDLE_VALUE)
	return read(tty, s, n);

    if (n > 0)
	for (;;) {
	    if (iConIn < nConIn) {
		if (n > nConIn - iConIn)
		    n = nConIn - iConIn;

		memcpy(s, ConInV, n);

		if ((iConIn += n) >= nConIn)
		    iConIn = nConIn = 0;

		break;
	    }

	    iConIn = nConIn = 0;

	    while (!read_win32_console_input()) ;
	}

    return n;
}

#endif				/* SUPPORT_WIN9X_CONSOLE_MBCS */

static HWND
GetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024
    HWND hwndFound;
    char pszNewWindowTitle[MY_BUFSIZE];
    char pszOldWindowTitle[MY_BUFSIZE];

    GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);
    wsprintf(pszNewWindowTitle, "%d/%d",
	     GetTickCount(), GetCurrentProcessId());
    SetConsoleTitle(pszNewWindowTitle);
    Sleep(40);
    hwndFound = FindWindow(NULL, pszNewWindowTitle);
    SetConsoleTitle(pszOldWindowTitle);
    return (hwndFound);
}

#if CYGWIN_VERSION_DLL_MAJOR < 1005 && defined(USE_MOUSE)
static unsigned long
cygwin_version(void)
{
    struct per_process *p;

    p = (struct per_process *)cygwin_internal(CW_USER_DATA);
    if (p != NULL) {
	return (p->dll_major * 1000) + p->dll_minor;
    }
    return 0;
}
#endif

static void
check_cygwin_console(void)
{
    char *term = getenv("TERM");
    char *ctype;
    HANDLE hWnd;

    if (term == NULL)
	term = DEFAULT_TERM;
    if (term && strncmp(term, "cygwin", 6) == 0) {
	isWinConsole = TERM_CYGWIN;
    }
    if (isWinConsole) {
	hWnd = GetConsoleHwnd();
	if (hWnd != INVALID_HANDLE_VALUE) {
	    if (IsWindowVisible(hWnd)) {
		isLocalConsole = 1;
	    }
	}
	if (((ctype = getenv("LC_ALL")) ||
	     (ctype = getenv("LC_CTYPE")) ||
	     (ctype = getenv("LANG"))) && strncmp(ctype, "ja", 2) == 0) {
	    isWinConsole = TERM_CYGWIN_RESERVE_IME;
	}
#ifdef SUPPORT_WIN9X_CONSOLE_MBCS
	check_win9x();
	if (isWin95 && ttyslot() != -1) {
	    isLocalConsole = 0;
	}
#endif
    }
#if CYGWIN_VERSION_DLL_MAJOR < 1005 && defined(USE_MOUSE)
    if (cygwin_version() <= 1003015) {
	/* cygwin DLL 1.3.15 or earler */
	cygwin_mouse_btn_swapped = 1;
    }
#endif
}
#endif				/* __CYGWIN__ */

char *getenv(const char *);
MySignalHandler reset_exit(SIGNAL_ARG), reset_error_exit(SIGNAL_ARG), error_dump(SIGNAL_ARG);
void setlinescols(void);
void flush_tty(void);

#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif				/* not SIGIOT */

#ifdef HAVE_TERMIO_H
#include <termio.h>
typedef struct termio TerminalMode;
#define TerminalSet(fd,x)       ioctl(fd,TCSETA,x)
#define TerminalGet(fd,x)       ioctl(fd,TCGETA,x)
#define MODEFLAG(d)     ((d).c_lflag)
#define IMODEFLAG(d)    ((d).c_iflag)
#endif				/* HAVE_TERMIO_H */

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#include <unistd.h>
typedef struct termios TerminalMode;
#define TerminalSet(fd,x)       tcsetattr(fd,TCSANOW,x)
#define TerminalGet(fd,x)       tcgetattr(fd,x)
#define MODEFLAG(d)     ((d).c_lflag)
#define IMODEFLAG(d)    ((d).c_iflag)
#endif				/* HAVE_TERMIOS_H */

#ifdef HAVE_SGTTY_H
#include <sgtty.h>
typedef struct sgttyb TerminalMode;
#define TerminalSet(fd,x)       ioctl(fd,TIOCSETP,x)
#define TerminalGet(fd,x)       ioctl(fd,TIOCGETP,x)
#define MODEFLAG(d)     ((d).sg_flags)
#endif				/* HAVE_SGTTY_H */

#ifdef __MINGW32_VERSION
/* dummy struct */
typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;

#define NCCS 32
struct termios
  {
    tcflag_t c_iflag;           /* input mode flags */
    tcflag_t c_oflag;           /* output mode flags */
    tcflag_t c_cflag;           /* control mode flags */
    tcflag_t c_lflag;           /* local mode flags */
    cc_t c_line;                        /* line discipline */
    cc_t c_cc[NCCS];            /* control characters */
    speed_t c_ispeed;           /* input speed */
    speed_t c_ospeed;           /* output speed */
  };
typedef struct termios TerminalMode;
#define TerminalSet(fd,x)       (0)
#define TerminalGet(fd,x)       (0)
#define MODEFLAG(d)     (0)

/* dummy defines */
#define SIGHUP (0)
#define SIGQUIT (0)
#define ECHO (0)
#define ISIG (0)
#define VEOF (0)
#define ICANON (0)
#define IXON (0)
#define IXOFF (0)

char *ttyname(int);
#endif /* __MINGW32_VERSION */

#define MAX_LINE        200
#define MAX_COLUMN      400

/* Screen properties */
#define S_SCREENPROP    0x0f
#define S_NORMAL        0x00
#define S_STANDOUT      0x01
#define S_UNDERLINE     0x02
#define S_BOLD          0x04
#define S_EOL           0x08

/* Sort of Character */
#define C_WHICHCHAR     0xc0
#define C_ASCII         0x00
#ifdef USE_M17N
#define C_WCHAR1        0x40
#define C_WCHAR2        0x80
#endif
#define C_CTRL          0xc0

#define CHMODE(c)       ((c)&C_WHICHCHAR)
#define SETCHMODE(var,mode)	((var) = (((var)&~C_WHICHCHAR) | mode))
#ifdef USE_M17N
#define SETCH(var,ch,len)	((var) = New_Reuse(char, (var), (len) + 1), \
				strncpy((var), (ch), (len + 1)))
#else
#define SETCH(var,ch,len)	((var) = (ch))
#endif

/* Charactor Color */
#define COL_FCOLOR      0xf00
#define COL_FBLACK      0x800
#define COL_FRED        0x900
#define COL_FGREEN      0xa00
#define COL_FYELLOW     0xb00
#define COL_FBLUE       0xc00
#define COL_FMAGENTA    0xd00
#define COL_FCYAN       0xe00
#define COL_FWHITE      0xf00
#define COL_FTERM       0x000

#define S_COLORED       0xf00

#ifdef USE_BG_COLOR
/* Background Color */
#define COL_BCOLOR      0xf000
#define COL_BBLACK      0x8000
#define COL_BRED        0x9000
#define COL_BGREEN      0xa000
#define COL_BYELLOW     0xb000
#define COL_BBLUE       0xc000
#define COL_BMAGENTA    0xd000
#define COL_BCYAN       0xe000
#define COL_BWHITE      0xf000
#define COL_BTERM       0x0000

#define S_BCOLORED      0xf000
#endif				/* USE_BG_COLOR */


#define S_GRAPHICS      0x10

#define S_DIRTY         0x20

#define SETPROP(var,prop) (var = (((var)&S_DIRTY) | prop))

/* Line status */
#define L_DIRTY         0x01
#define L_UNUSED        0x02
#define L_NEED_CE       0x04
#define L_CLRTOEOL      0x08

#define ISDIRTY(d)      ((d) & L_DIRTY)
#define ISUNUSED(d)     ((d) & L_UNUSED)
#define NEED_CE(d)      ((d) & L_NEED_CE)

typedef unsigned short l_prop;

typedef struct scline {
#ifdef USE_M17N
    char **lineimage;
#else
    char *lineimage;
#endif
    l_prop *lineprop;
    short isdirty;
    short eol;
} Screen;

static TerminalMode d_ioval;
static int tty = -1;
static FILE *ttyf = NULL;

static
char bp[1024], funcstr[256];

char *T_cd, *T_ce, *T_kr, *T_kl, *T_cr, *T_bt, *T_ta, *T_sc, *T_rc,
    *T_so, *T_se, *T_us, *T_ue, *T_cl, *T_cm, *T_al, *T_sr, *T_md, *T_me,
    *T_ti, *T_te, *T_nd, *T_as, *T_ae, *T_eA, *T_ac, *T_op;

int LINES, COLS;
#if defined(__CYGWIN__)
int LASTLINE;
#endif				/* defined(__CYGWIN__) */

static int max_LINES = 0, max_COLS = 0;
static int tab_step = 8;
static int CurLine, CurColumn;
static Screen *ScreenElem = NULL, **ScreenImage = NULL;
static l_prop CurrentMode = 0;
static int graph_enabled = 0;

static char gcmap[96];

extern int tgetent(char *, char *);
extern int tgetnum(char *);
extern int tgetflag(char *);
extern char *tgetstr(char *, char **);
extern char *tgoto(char *, int, int);
extern int tputs(char *, int, int (*)(char));
void clear(void), wrap(void), touch_line(void), touch_column(int);
#if 0
void need_clrtoeol(void);
#endif
void clrtoeol(void);		/* conflicts with curs_clear(3)? */

static int write1(char);

static void
writestr(char *s)
{
    tputs(s, 1, write1);
}

#define MOVE(line,column)       writestr(tgoto(T_cm,column,line));

#ifdef USE_IMAGE
void
put_image_osc5379(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
    Str buf;
    char *size ;

    if (w > 0 && h > 0)
	size = Sprintf("%dx%d",w,h)->ptr;
    else
	size = "";

    MOVE(y,x);
    buf = Sprintf("\x1b]5379;show_picture %s %s %dx%d+%d+%d\x07",url,size,sw,sh,sx,sy);
    writestr(buf->ptr);
    MOVE(Currentbuf->cursorY,Currentbuf->cursorX);
}


void
put_image_iterm2(char *url, int x, int y, int w, int h)
{
    Str buf;
    char *cbuf;
    FILE *fp;
    int c, i;
    struct stat st;

    if (stat(url, &st))
	return;

    fp = fopen(url, "r");
    if (!fp)
	return;

    buf = Sprintf("\x1b]1337;"
      "File="
      "name=%s;"
      "size=%d;"
      "width=%d;"
      "height=%d;"
      "preserveAspectRatio=0;"
      "inline=1"
      ":", url, st.st_size, w, h);

    MOVE(y,x);

    writestr(buf->ptr);

    cbuf = GC_MALLOC_ATOMIC(3072);
    if (!cbuf)
	goto cleanup;
    i = 0;
    while ((c = fgetc(fp)) != EOF) {
	cbuf[i++] = c;
	if (i == 3072) {
	    buf = base64_encode(cbuf, i);
	    writestr(buf->ptr);
	    i = 0;
	}
    }

    if (i) {
	buf = base64_encode(cbuf, i);
	writestr(buf->ptr);
    }

cleanup:
    fclose(fp);
    writestr("\a");
    MOVE(Currentbuf->cursorY,Currentbuf->cursorX);
}

void ttymode_set(int mode, int imode);
void ttymode_reset(int mode, int imode);

void
put_image_kitty(char *url, int x, int y, int w, int h, int sx, int sy, int sw,
    int sh, int cols, int rows)
{
    Str buf, base64;
    char *cbuf, *type, *tmpf;
    char *argv[4];
    FILE *fp;
    int c, i, j, m, t, is_anim;
    struct stat st;
    pid_t pid;
    MySignalHandler(*volatile previntr) (SIGNAL_ARG);
    MySignalHandler(*volatile prevquit) (SIGNAL_ARG);
    MySignalHandler(*volatile prevstop) (SIGNAL_ARG);

    if (!url)
	return;

    type = guessContentType(url);
    t = 100; /* always convert to png for now. */

    if(!(type && !strcasecmp(type, "image/png"))) {
	tmpf = Sprintf("%s/%s.png", tmp_dir, mybasename(url))->ptr;

	if (type && !strcasecmp(type, "image/gif")) {
	    is_anim = 1;
	} else {
	    is_anim = 0;
	}

	/* convert only if png doesn't exist yet. */

	if (stat(tmpf, &st)) {
	    if (stat(url, &st))
		return;

	    flush_tty();

	    previntr = mySignal(SIGINT, SIG_IGN);
	    prevquit = mySignal(SIGQUIT, SIG_IGN);
	    prevstop = mySignal(SIGTSTP, SIG_IGN);

	    if ((pid = fork()) == 0) {
		i = 0;

		close(STDERR_FILENO);	/* Don't output error message. */
		ttymode_set(ISIG, 0);

		if ((cbuf = getenv("W3M_KITTY_TO_PNG")))
		    argv[i++] = cbuf;
		else
		    argv[i++] = "convert";

		if (is_anim) {
		    buf = Strnew_charp(url);
		    Strcat_charp(buf, "[0]");
		    argv[i++] = buf->ptr;
		} else {
		    argv[i++] = url;
		}
		argv[i++] = tmpf;
		argv[i++] = NULL;
		execvp(argv[0],argv);
		exit(0);
	    }
	    else if (pid > 0) {
		waitpid(pid, &i, 0);
		ttymode_reset(ISIG, 0);
		mySignal(SIGINT, previntr);
		mySignal(SIGQUIT, prevquit);
		mySignal(SIGTSTP, prevstop);
	    }

	    pushText(fileToDelete, tmpf);
	}
	url = tmpf;
    }

    if (stat(url, &st))
	return;

    fp = fopen(url, "r");
    if (!fp)
	return;

    MOVE(y, x);


    cbuf = GC_MALLOC_ATOMIC(3072); /* base64-encoded chunks of 4096 bytes */
    if (!cbuf)
	goto cleanup;
    i = 0;

    while (i < 3072 && (c = fgetc(fp)) != EOF)
	cbuf[i++] = c;


    base64 = base64_encode(cbuf, i);

    if (c == EOF)
	m = 0;
    else
	m = 1;
    buf = Sprintf("\x1b_Gf=%d,s=%d,v=%d,a=T,m=%d,x=%d,y=%d,w=%d,h=%d,c=%d,r=%d;"
	  "%s\x1b\\", t, w, h, m, sx, sy, sw, sh, cols, rows, base64->ptr);
    writestr(buf->ptr);

    if (m) {
	i = 0;
	j = 0;
	while ((c = fgetc(fp)) != EOF) {
	    if (j) {
		base64 = base64_encode(cbuf, i);
		buf = Sprintf("\x1b_Gm=1;%s\x1b\\", base64->ptr);
		writestr(buf->ptr);
		i = 0;
		j = 0;
	    }
	    cbuf[i++] = c;
	    if (i == 3072)
		j = 1;
	}

	if (i) {
	    base64 = base64_encode(cbuf, i);
	    buf = Sprintf("\x1b_Gm=0;%s\x1b\\", base64->ptr);
	    writestr(buf->ptr);
	}
    }
cleanup:
    fclose(fp);
    MOVE(Currentbuf->cursorY, Currentbuf->cursorX);
}

static void
save_gif(const char *path, u_char *header, size_t  header_size, u_char *body, size_t body_size)
{
    int	fd;

    if ((fd = open(path, O_WRONLY|O_CREAT, 0600)) >= 0) {
	write(fd, header, header_size) ;
	write(fd, body, body_size) ;
	write(fd, "\x3b" , 1) ;
	close(fd) ;
    }
}

static u_char *
skip_gif_header(u_char *p)
{
    /* Header */
    p += 10;

    if (*(p) & 0x80) {
	p += (3 * (2 << ((*p) & 0x7)));
    }
    p += 3;

    return p;
}

static Str
save_first_animation_frame(const char *path)
{
    int	fd;
    struct stat	st;
    u_char *header;
    size_t header_size;
    u_char *body;
    u_char *p;
    ssize_t len;
    Str new_path;

    new_path = Strnew_charp(path);
    Strcat_charp(new_path, "-1");
    if (stat(new_path->ptr, &st) == 0) {
	return new_path;
    }

    if ((fd = open( path, O_RDONLY)) < 0) {
	return NULL;
    }

    if (fstat( fd, &st) != 0 || ! (header = GC_malloc( st.st_size))){
	close( fd);
	return NULL;
    }

    len = read(fd, header, st.st_size);
    close(fd);

    /* Header */

    if (len != st.st_size || strncmp((char *)header, "GIF89a", 6) != 0) {
	return NULL;
    }

    p = skip_gif_header(header);
    header_size = p - header;

    /* Application Extension */
    if (p[0] == 0x21 && p[1] == 0xff) {
	p += 19;
    }

    /* Other blocks */
    body = NULL;
    while (p + 2 < header + st.st_size) {
	if (*(p++) == 0x21 && *(p++) == 0xf9 && *(p++) == 0x04) {
	    if( body) {
		/* Graphic Control Extension */
		save_gif(new_path->ptr, header, header_size, body, p - 3 - body);
		return new_path;
	    }
	    else {
		/* skip the first frame. */
	    }
	    body = p - 3;
	}
    }

    return NULL;
}

void
put_image_sixel(char *url, int x, int y, int w, int h, int sx, int sy, int sw, int sh, int n_terminal_image)
{
    pid_t pid;
    int do_anim;
    MySignalHandler(*volatile previntr) (SIGNAL_ARG);
    MySignalHandler(*volatile prevquit) (SIGNAL_ARG);
    MySignalHandler(*volatile prevstop) (SIGNAL_ARG);

    MOVE(y,x);
    flush_tty();

    do_anim = (n_terminal_image == 1 && x == 0 && y == 0 && sx == 0 && sy == 0);

    previntr = mySignal(SIGINT, SIG_IGN);
    prevquit = mySignal(SIGQUIT, SIG_IGN);
    prevstop = mySignal(SIGTSTP, SIG_IGN);

    if ((pid = fork()) == 0) {
	char *env;
	int n = 0;
	char *argv[20];
	char digit[2][11+1];
	char clip[44+3+1];
	Str str_url;

	close(STDERR_FILENO);	/* Don't output error message. */
	if (do_anim) {
	    writestr("\x1b[?80h");
	}
	else if (!strstr(url, "://") && strcmp(url+strlen(url)-4, ".gif") == 0 &&
                 (str_url = save_first_animation_frame(url))) {
	    url = str_url->ptr;
	}
	ttymode_set(ISIG, 0);

	if ((env = getenv("W3M_IMG2SIXEL"))) {
	    char *p;
	    env = Strnew_charp(env)->ptr;
	    while (n < 8 && (p = strchr(env, ' '))) {
		*p = '\0';
		if (*env != '\0') {
		    argv[n++] = env;
		}
		env = p+1;
	    }
	    if (*env != '\0') {
		argv[n++] = env;
	    }
	}
	else {
		argv[n++] = "img2sixel";
	}
	argv[n++] = "-l";
	argv[n++] = do_anim ? "auto" : "disable";
	argv[n++] = "-w";
	sprintf(digit[0], "%d", w);
	argv[n++] = digit[0];
	argv[n++] = "-h";
	sprintf(digit[1], "%d", h);
	argv[n++] = digit[1];
	argv[n++] = "-c";
	sprintf(clip, "%dx%d+%d+%d", sw, sh, sx, sy);
	argv[n++] = clip;
	argv[n++] = url;
	if (getenv("TERM") && strcmp(getenv("TERM"), "screen") == 0 &&
	    (!getenv("SCREEN_VARIANT") || strcmp(getenv("SCREEN_VARIANT"), "sixel") != 0)) {
	    argv[n++] = "-P";
	}
	argv[n++] = NULL;
	execvp(argv[0],argv);
	exit(0);
    }
    else if (pid > 0) {
	int status;
	waitpid(pid, &status, 0);
	ttymode_reset(ISIG, 0);
	mySignal(SIGINT, previntr);
	mySignal(SIGQUIT, prevquit);
	mySignal(SIGTSTP, prevstop);
	if (do_anim) {
	    writestr("\x1b[?80l");
	}
    }

    MOVE(Currentbuf->cursorY,Currentbuf->cursorX);
}

int
get_pixel_per_cell(int *ppc, int *ppl)
{
    fd_set  rfd;
    struct timeval tval;
    char buf[100];
    char *p;
    ssize_t len;
    ssize_t left;
    int wp,hp,wc,hc;
    int i;

#ifdef  TIOCGWINSZ
    struct winsize ws;
    if (ioctl(tty, TIOCGWINSZ, &ws) == 0 && ws.ws_ypixel > 0 && ws.ws_row > 0 &&
        ws.ws_xpixel > 0 && ws.ws_col > 0) {
	*ppc = ws.ws_xpixel / ws.ws_col;
	*ppl = ws.ws_ypixel / ws.ws_row;
	return 1;
    }
#endif

    fputs("\x1b[14t\x1b[18t",ttyf); flush_tty();

    p = buf;
    left = sizeof(buf) - 1;
    for (i = 0; i < 10; i++) {
	tval.tv_usec = 200000;	/* 0.2 sec * 10 */
	tval.tv_sec = 0;
	FD_ZERO(&rfd);
	FD_SET(tty,&rfd);
	if (select(tty+1,&rfd,NULL,NULL,&tval) <= 0 || ! FD_ISSET(tty,&rfd))
	    continue;

	if ((len = read(tty,p,left)) <= 0)
	    continue;
	p[len] = '\0';

	if (sscanf(buf,"\x1b[4;%d;%dt\x1b[8;%d;%dt",&hp,&wp,&hc,&wc) == 4) {
	    if (wp > 0 && wc > 0 && hp > 0 && hc > 0) {
		*ppc = wp / wc;
		*ppl = hp / hc;
		return 1;
	    }
	    else {
		return 0;
	    }
	}
	p += len;
	left -= len;
    }

    return 0;
}
#endif				/* USE_IMAGE */

#ifdef USE_MOUSE
#define W3M_TERM_INFO(name, title, mouse)	name, title, mouse
#define NEED_XTERM_ON   (1)
#define NEED_XTERM_OFF  (1<<1)
#ifdef __CYGWIN__
#define NEED_CYGWIN_ON  (1<<2)
#define NEED_CYGWIN_OFF (1<<3)
#endif
#else
#define W3M_TERM_INFO(name, title, mouse)	name, title
#endif

static char XTERM_TITLE[] = "\033]0;w3m: %s\007";
static char SCREEN_TITLE[] = "\033k%s\033\134";
#ifdef __CYGWIN__
static char CYGWIN_TITLE[] = "w3m: %s";
#endif

/* *INDENT-OFF* */
static struct w3m_term_info {
    char *term;
    char *title_str;
#ifdef USE_MOUSE
    int mouse_flag;
#endif
} w3m_term_info_list[] = {
    {W3M_TERM_INFO("xterm", XTERM_TITLE, (NEED_XTERM_ON|NEED_XTERM_OFF))},
    {W3M_TERM_INFO("kterm", XTERM_TITLE, (NEED_XTERM_ON|NEED_XTERM_OFF))},
    {W3M_TERM_INFO("rxvt", XTERM_TITLE, (NEED_XTERM_ON|NEED_XTERM_OFF))},
    {W3M_TERM_INFO("Eterm", XTERM_TITLE, (NEED_XTERM_ON|NEED_XTERM_OFF))},
    {W3M_TERM_INFO("mlterm", XTERM_TITLE, (NEED_XTERM_ON|NEED_XTERM_OFF))},
    {W3M_TERM_INFO("screen", SCREEN_TITLE, 0)},
#ifdef __CYGWIN__
    {W3M_TERM_INFO("cygwin", CYGWIN_TITLE, (NEED_CYGWIN_ON|NEED_CYGWIN_OFF))},
#endif
    {W3M_TERM_INFO(NULL, NULL, 0)}
};
#undef W3M_TERM_INFO
/* *INDENT-ON * */

int
set_tty(void)
{
    char *ttyn;

    if (isatty(0))		/* stdin */
	ttyn = ttyname(0);
    else
	ttyn = DEV_TTY_PATH;
    tty = open(ttyn, O_RDWR);
    if (tty < 0) {
	/* use stderr instead of stdin... is it OK???? */
	tty = 2;
    }
    ttyf = fdopen(tty, "w");
#ifdef __CYGWIN__
    check_cygwin_console();
#endif
    TerminalGet(tty, &d_ioval);
    if (displayTitleTerm != NULL) {
	struct w3m_term_info *p;
	for (p = w3m_term_info_list; p->term != NULL; p++) {
	    if (!strncmp(displayTitleTerm, p->term, strlen(p->term))) {
		title_str = p->title_str;
		break;
	    }
	}
    }
#ifdef USE_MOUSE
    {
	char *term = getenv("TERM");
	if (term != NULL) {
	    struct w3m_term_info *p;
	    for (p = w3m_term_info_list; p->term != NULL; p++) {
		if (!strncmp(term, p->term, strlen(p->term))) {
			is_xterm = p->mouse_flag;
			break;
		    }
		}
	}
    }
#endif
    return 0;
}

void
ttymode_set(int mode, int imode)
{
#ifndef __MINGW32_VERSION
    TerminalMode ioval;

    TerminalGet(tty, &ioval);
    MODEFLAG(ioval) |= mode;
#ifndef HAVE_SGTTY_H
    IMODEFLAG(ioval) |= imode;
#endif				/* not HAVE_SGTTY_H */

    while (TerminalSet(tty, &ioval) == -1) {
	if (errno == EINTR || errno == EAGAIN)
	    continue;
	printf("Error occurred while set %x: errno=%d\n", mode, errno);
	reset_error_exit(SIGNAL_ARGLIST);
    }
#endif
}

void
ttymode_reset(int mode, int imode)
{
#ifndef __MINGW32_VERSION
    TerminalMode ioval;

    TerminalGet(tty, &ioval);
    MODEFLAG(ioval) &= ~mode;
#ifndef HAVE_SGTTY_H
    IMODEFLAG(ioval) &= ~imode;
#endif				/* not HAVE_SGTTY_H */

    while (TerminalSet(tty, &ioval) == -1) {
	if (errno == EINTR || errno == EAGAIN)
	    continue;
	printf("Error occurred while reset %x: errno=%d\n", mode, errno);
	reset_error_exit(SIGNAL_ARGLIST);
    }
#endif /* __MINGW32_VERSION */
}

#ifndef HAVE_SGTTY_H
void
set_cc(int spec, int val)
{
    TerminalMode ioval;

    TerminalGet(tty, &ioval);
    ioval.c_cc[spec] = val;
    while (TerminalSet(tty, &ioval) == -1) {
	if (errno == EINTR || errno == EAGAIN)
	    continue;
	printf("Error occurred: errno=%d\n", errno);
	reset_error_exit(SIGNAL_ARGLIST);
    }
}
#endif				/* not HAVE_SGTTY_H */

void
close_tty(void)
{
    if (tty > 2)
	close(tty);
}

char *
ttyname_tty(void)
{
    return ttyname(tty);
}

void
reset_tty(void)
{
    writestr(T_op);		/* turn off */
    writestr(T_me);
    if (!Do_not_use_ti_te) {
	if (T_te && *T_te)
	    writestr(T_te);
	else
	    writestr(T_cl);
    }
    writestr(T_se);		/* reset terminal */
    flush_tty();
    TerminalSet(tty, &d_ioval);
    if (tty != 2)
        close_tty();
}

static MySignalHandler
reset_exit_with_value(SIGNAL_ARG, int rval)
{
#ifdef USE_MOUSE
    if (mouseActive)
	mouse_end();
#endif				/* USE_MOUSE */
    reset_tty();
    w3m_exit(rval);
    SIGNAL_RETURN;
}

MySignalHandler
reset_error_exit(SIGNAL_ARG)
{
  reset_exit_with_value(SIGNAL_ARGLIST, 1);
}

MySignalHandler
reset_exit(SIGNAL_ARG)
{
  reset_exit_with_value(SIGNAL_ARGLIST, 0);
}

MySignalHandler
error_dump(SIGNAL_ARG)
{
    mySignal(SIGIOT, SIG_DFL);
    reset_tty();
    abort();
    SIGNAL_RETURN;
}

void
set_int(void)
{
    mySignal(SIGHUP, reset_exit);
    mySignal(SIGINT, reset_exit);
    mySignal(SIGQUIT, reset_exit);
    mySignal(SIGTERM, reset_exit);
    mySignal(SIGILL, error_dump);
    mySignal(SIGIOT, error_dump);
    mySignal(SIGFPE, error_dump);
#ifdef	SIGBUS
    mySignal(SIGBUS, error_dump);
#endif				/* SIGBUS */
    /* mySignal(SIGSEGV, error_dump); */
}


static void
setgraphchar(void)
{
    int c, i, n;

    for (c = 0; c < 96; c++)
	gcmap[c] = (char)(c + ' ');

    if (!T_ac)
	return;

    n = strlen(T_ac);
    for (i = 0; i < n - 1; i += 2) {
	c = (unsigned)T_ac[i] - ' ';
	if (c >= 0 && c < 96)
	    gcmap[c] = T_ac[i + 1];
    }
}

#define graphchar(c) (((unsigned)(c)>=' ' && (unsigned)(c)<128)? gcmap[(c)-' '] : (c))
#define GETSTR(v,s) {v = pt; suc = tgetstr(s,&pt); if (!suc) v = ""; else v = allocStr(suc, -1); }

void
getTCstr(void)
{
    char *ent;
    char *suc;
    char *pt = funcstr;
    int r;

    ent = getenv("TERM") ? getenv("TERM") : DEFAULT_TERM;
    if (ent == NULL) {
	fprintf(stderr, "TERM is not set\n");
	reset_error_exit(SIGNAL_ARGLIST);
    }

    r = tgetent(bp, ent);
    if (r != 1) {
	/* Can't find termcap entry */
	fprintf(stderr, "Can't find termcap entry %s\n", ent);
	reset_error_exit(SIGNAL_ARGLIST);
    }

    GETSTR(T_ce, "ce");		/* clear to the end of line */
    GETSTR(T_cd, "cd");		/* clear to the end of display */
    GETSTR(T_kr, "nd");		/* cursor right */
    if (suc == NULL)
	GETSTR(T_kr, "kr");
    if (tgetflag("bs"))
	T_kl = "\b";		/* cursor left */
    else {
	GETSTR(T_kl, "le");
	if (suc == NULL)
	    GETSTR(T_kl, "kb");
	if (suc == NULL)
	    GETSTR(T_kl, "kl");
    }
    GETSTR(T_cr, "cr");		/* carriage return */
    GETSTR(T_ta, "ta");		/* tab */
    GETSTR(T_sc, "sc");		/* save cursor */
    GETSTR(T_rc, "rc");		/* restore cursor */
    GETSTR(T_so, "so");		/* standout mode */
    GETSTR(T_se, "se");		/* standout mode end */
    GETSTR(T_us, "us");		/* underline mode */
    GETSTR(T_ue, "ue");		/* underline mode end */
    GETSTR(T_md, "md");		/* bold mode */
    GETSTR(T_me, "me");		/* bold mode end */
    GETSTR(T_cl, "cl");		/* clear screen */
    GETSTR(T_cm, "cm");		/* cursor move */
    GETSTR(T_al, "al");		/* append line */
    GETSTR(T_sr, "sr");		/* scroll reverse */
    GETSTR(T_ti, "ti");		/* terminal init */
    GETSTR(T_te, "te");		/* terminal end */
    GETSTR(T_nd, "nd");		/* move right one space */
    GETSTR(T_eA, "eA");		/* enable alternative charset */
    GETSTR(T_as, "as");		/* alternative (graphic) charset start */
    GETSTR(T_ae, "ae");		/* alternative (graphic) charset end */
    GETSTR(T_ac, "ac");		/* graphics charset pairs */
    GETSTR(T_op, "op");		/* set default color pair to its original value */
#if defined( CYGWIN ) && CYGWIN < 1
    /* for TERM=pcansi on MS-DOS prompt. */
#if 0
    T_eA = "";
    T_as = "\033[12m";
    T_ae = "\033[10m";
    T_ac = "l\001k\002m\003j\004x\005q\006n\020a\024v\025w\026u\027t\031";
#endif
    T_eA = "";
    T_as = "";
    T_ae = "";
    T_ac = "";
#endif				/* CYGWIN */

    LINES = COLS = 0;
    setlinescols();
    setgraphchar();
}

void
setlinescols(void)
{
    char *p;
    int i;
#ifdef __EMX__
    {
	int s[2];
	_scrsize(s);
	COLS = s[0];
	LINES = s[1];

	if (getenv("WINDOWID")) {
	    FILE *fd = popen("scrsize", "rt");
	    if (fd) {
		fscanf(fd, "%i %i", &COLS, &LINES);
		pclose(fd);
	    }
	}
    }
#elif defined(HAVE_TERMIOS_H) && defined(TIOCGWINSZ)
    struct winsize wins;

    i = ioctl(tty, TIOCGWINSZ, &wins);
    if (i >= 0 && wins.ws_row != 0 && wins.ws_col != 0) {
	LINES = wins.ws_row;
	COLS = wins.ws_col;
    }
#endif				/* defined(HAVE-TERMIOS_H) && defined(TIOCGWINSZ) */
    if (LINES <= 0 && (p = getenv("LINES")) != NULL && (i = atoi(p)) >= 0)
	LINES = i;
    if (COLS <= 0 && (p = getenv("COLUMNS")) != NULL && (i = atoi(p)) >= 0)
	COLS = i;
    if (LINES <= 0)
	LINES = tgetnum("li");	/* number of line */
    if (COLS <= 0)
	COLS = tgetnum("co");	/* number of column */
    if (COLS > MAX_COLUMN)
	COLS = MAX_COLUMN;
    if (LINES > MAX_LINE)
	LINES = MAX_LINE;
#if defined(__CYGWIN__)
    LASTLINE = LINES - (isWinConsole == TERM_CYGWIN_RESERVE_IME ? 2 : 1);
#endif				/* defined(__CYGWIN__) */
}

void
setupscreen(void)
{
    int i;

    if (LINES + 1 > max_LINES) {
	max_LINES = LINES + 1;
	max_COLS = 0;
	ScreenElem = New_N(Screen, max_LINES);
	ScreenImage = New_N(Screen *, max_LINES);
    }
    if (COLS + 1 > max_COLS) {
	max_COLS = COLS + 1;
	for (i = 0; i < max_LINES; i++) {
#ifdef USE_M17N
	    ScreenElem[i].lineimage = New_N(char *, max_COLS);
	    bzero((void *)ScreenElem[i].lineimage, max_COLS * sizeof(char *));
#else
	    ScreenElem[i].lineimage = New_N(char, max_COLS);
#endif
	    ScreenElem[i].lineprop = New_N(l_prop, max_COLS);
	}
    }
    for (i = 0; i < LINES; i++) {
	ScreenImage[i] = &ScreenElem[i];
	ScreenImage[i]->lineprop[0] = S_EOL;
	ScreenImage[i]->isdirty = 0;
    }
    for (; i < max_LINES; i++) {
	ScreenElem[i].isdirty = L_UNUSED;
    }

    clear();
}

/* 
 * Screen initialize
 */
int
initscr(void)
{
    if (set_tty() < 0)
	return -1;
    set_int();
    getTCstr();
    if (T_ti && !Do_not_use_ti_te)
	writestr(T_ti);
    setupscreen();
    return 0;
}

static int
write1(char c)
{
    putc(c, ttyf);
#ifdef SCREEN_DEBUG
    flush_tty();
#endif				/* SCREEN_DEBUG */
    return 0;
}

void
move(int line, int column)
{
    if (line >= 0 && line < LINES)
	CurLine = line;
    if (column >= 0 && column < COLS)
	CurColumn = column;
}

#ifdef USE_BG_COLOR
#define M_SPACE (S_SCREENPROP|S_COLORED|S_BCOLORED|S_GRAPHICS)
#else				/* not USE_BG_COLOR */
#define M_SPACE (S_SCREENPROP|S_COLORED|S_GRAPHICS)
#endif				/* not USE_BG_COLOR */

static int
#ifdef USE_M17N
need_redraw(char *c1, l_prop pr1, char *c2, l_prop pr2)
{
    if (!c1 || !c2 || strcmp(c1, c2))
	return 1;
    if (*c1 == ' ')
#else
need_redraw(char c1, l_prop pr1, char c2, l_prop pr2)
{
    if (c1 != c2)
	return 1;
    if (c1 == ' ')
#endif
	return (pr1 ^ pr2) & M_SPACE & ~S_DIRTY;

    if ((pr1 ^ pr2) & ~S_DIRTY)
	return 1;

    return 0;
}

#define M_CEOL (~(M_SPACE|C_WHICHCHAR))

#ifdef USE_M17N
#define SPACE " "
#else
#define SPACE ' '
#endif

#ifdef USE_M17N
void
addch(char c)
{
    addmch(&c, 1);
}

void
addmch(char *pc, size_t len)
#else
void
addch(char pc)
#endif
{
    l_prop *pr;
    int dest, i;
#ifdef USE_M17N
    static Str tmp = NULL;
    char **p;
    char c = *pc;
    int width = wtf_width((wc_uchar *) pc);

    if (tmp == NULL)
	tmp = Strnew();
    Strcopy_charp_n(tmp, pc, len);
    pc = tmp->ptr;
#else
    char *p;
    char c = pc;
#endif

    if (CurColumn == COLS)
	wrap();
    if (CurColumn >= COLS)
	return;
    p = ScreenImage[CurLine]->lineimage;
    pr = ScreenImage[CurLine]->lineprop;

#ifndef USE_M17N
    /* Eliminate unprintables according to * iso-8859-*.
     * Particularly 0x96 messes up T.Dickey's * (xfree-)xterm */
    if (IS_INTSPACE(c))
	c = ' ';
#endif

    if (pr[CurColumn] & S_EOL) {
	if (c == ' ' && !(CurrentMode & M_SPACE)) {
	    CurColumn++;
	    return;
	}
	for (i = CurColumn; i >= 0 && (pr[i] & S_EOL); i--) {
	    SETCH(p[i], SPACE, 1);
	    SETPROP(pr[i], (pr[i] & M_CEOL) | C_ASCII);
	}
    }

    if (c == '\t' || c == '\n' || c == '\r' || c == '\b')
	SETCHMODE(CurrentMode, C_CTRL);
#ifdef USE_M17N
    else if (len > 1)
	SETCHMODE(CurrentMode, C_WCHAR1);
#endif
    else if (!IS_CNTRL(c))
	SETCHMODE(CurrentMode, C_ASCII);
    else
	return;

    /* Required to erase bold or underlined character for some * terminal
     * emulators. */
#ifdef USE_M17N
    i = CurColumn + width - 1;
#else
    i = CurColumn;
#endif
    if (i < COLS &&
	(((pr[i] & S_BOLD) && need_redraw(p[i], pr[i], pc, CurrentMode)) ||
	 ((pr[i] & S_UNDERLINE) && !(CurrentMode & S_UNDERLINE)))) {
	touch_line();
	i++;
	if (i < COLS) {
	    touch_column(i);
	    if (pr[i] & S_EOL) {
		SETCH(p[i], SPACE, 1);
		SETPROP(pr[i], (pr[i] & M_CEOL) | C_ASCII);
	    }
#ifdef USE_M17N
	    else {
		for (i++; i < COLS && CHMODE(pr[i]) == C_WCHAR2; i++)
		    touch_column(i);
	    }
#endif
	}
    }

#ifdef USE_M17N
    if (CurColumn + width > COLS) {
	touch_line();
	for (i = CurColumn; i < COLS; i++) {
	    SETCH(p[i], SPACE, 1);
	    SETPROP(pr[i], (pr[i] & ~C_WHICHCHAR) | C_ASCII);
	    touch_column(i);
	}
	wrap();
	if (CurColumn + width > COLS)
	    return;
	p = ScreenImage[CurLine]->lineimage;
	pr = ScreenImage[CurLine]->lineprop;
    }
    if (CHMODE(pr[CurColumn]) == C_WCHAR2) {
	touch_line();
	for (i = CurColumn - 1; i >= 0; i--) {
	    l_prop l = CHMODE(pr[i]);
	    SETCH(p[i], SPACE, 1);
	    SETPROP(pr[i], (pr[i] & ~C_WHICHCHAR) | C_ASCII);
	    touch_column(i);
	    if (l != C_WCHAR2)
		break;
	}
    }
#endif
    if (CHMODE(CurrentMode) != C_CTRL) {
	if (need_redraw(p[CurColumn], pr[CurColumn], pc, CurrentMode)) {
	    SETCH(p[CurColumn], pc, len);
	    SETPROP(pr[CurColumn], CurrentMode);
	    touch_line();
	    touch_column(CurColumn);
#ifdef USE_M17N
	    SETCHMODE(CurrentMode, C_WCHAR2);
	    for (i = CurColumn + 1; i < CurColumn + width; i++) {
		SETCH(p[i], SPACE, 1);
		SETPROP(pr[i], (pr[CurColumn] & ~C_WHICHCHAR) | C_WCHAR2);
		touch_column(i);
	    }
	    for (; i < COLS && CHMODE(pr[i]) == C_WCHAR2; i++) {
		SETCH(p[i], SPACE, 1);
		SETPROP(pr[i], (pr[i] & ~C_WHICHCHAR) | C_ASCII);
		touch_column(i);
	    }
	}
	CurColumn += width;
#else
	}
	CurColumn++;
#endif
    }
    else if (c == '\t') {
	dest = (CurColumn + tab_step) / tab_step * tab_step;
	if (dest >= COLS) {
	    wrap();
	    touch_line();
	    dest = tab_step;
	    p = ScreenImage[CurLine]->lineimage;
	    pr = ScreenImage[CurLine]->lineprop;
	}
	for (i = CurColumn; i < dest; i++) {
	    if (need_redraw(p[i], pr[i], SPACE, CurrentMode)) {
		SETCH(p[i], SPACE, 1);
		SETPROP(pr[i], CurrentMode);
		touch_line();
		touch_column(i);
	    }
	}
	CurColumn = i;
    }
    else if (c == '\n') {
	wrap();
    }
    else if (c == '\r') {	/* Carriage return */
	CurColumn = 0;
    }
    else if (c == '\b' && CurColumn > 0) {	/* Backspace */
	CurColumn--;
#ifdef USE_M17N
	while (CurColumn > 0 && CHMODE(pr[CurColumn]) == C_WCHAR2)
	    CurColumn--;
#endif
    }
}

void
wrap(void)
{
    if (CurLine == LASTLINE)
	return;
    CurLine++;
    CurColumn = 0;
}

void
touch_column(int col)
{
    if (col >= 0 && col < COLS)
	ScreenImage[CurLine]->lineprop[col] |= S_DIRTY;
}

void
touch_line(void)
{
    if (!(ScreenImage[CurLine]->isdirty & L_DIRTY)) {
	int i;
	for (i = 0; i < COLS; i++)
	    ScreenImage[CurLine]->lineprop[i] &= ~S_DIRTY;
	ScreenImage[CurLine]->isdirty |= L_DIRTY;
    }

}

void
standout(void)
{
    CurrentMode |= S_STANDOUT;
}

void
standend(void)
{
    CurrentMode &= ~S_STANDOUT;
}

void
toggle_stand(void)
{
#ifdef USE_M17N
    int i;
#endif
    l_prop *pr = ScreenImage[CurLine]->lineprop;
    pr[CurColumn] ^= S_STANDOUT;
#ifdef USE_M17N
    if (CHMODE(pr[CurColumn]) != C_WCHAR2) {
	for (i = CurColumn + 1; CHMODE(pr[i]) == C_WCHAR2; i++)
	    pr[i] ^= S_STANDOUT;
    }
#endif
}

void
bold(void)
{
    CurrentMode |= S_BOLD;
}

void
boldend(void)
{
    CurrentMode &= ~S_BOLD;
}

void
underline(void)
{
    CurrentMode |= S_UNDERLINE;
}

void
underlineend(void)
{
    CurrentMode &= ~S_UNDERLINE;
}

void
graphstart(void)
{
    CurrentMode |= S_GRAPHICS;
}

void
graphend(void)
{
    CurrentMode &= ~S_GRAPHICS;
}

int
graph_ok(void)
{
    if (UseGraphicChar != GRAPHIC_CHAR_DEC)
	return 0;
    return T_as[0] != 0 && T_ae[0] != 0 && T_ac[0] != 0;
}

void
setfcolor(int color)
{
    CurrentMode &= ~COL_FCOLOR;
    if ((color & 0xf) <= 7)
	CurrentMode |= (((color & 7) | 8) << 8);
}

static char *
color_seq(int colmode)
{
    int n = 30;
    static char seqbuf[32];
#ifdef USE_COLOR
    n = highIntensityColors ? 90 : 30;
#endif
    sprintf(seqbuf, "\033[%dm", ((colmode >> 8) & 7) + n);
    return seqbuf;
}

#ifdef USE_BG_COLOR
void
setbcolor(int color)
{
    CurrentMode &= ~COL_BCOLOR;
    if ((color & 0xf) <= 7)
	CurrentMode |= (((color & 7) | 8) << 12);
}

static char *
bcolor_seq(int colmode)
{
    static char seqbuf[32];
    sprintf(seqbuf, "\033[%dm", ((colmode >> 12) & 7) + 40);
    return seqbuf;
}
#endif				/* USE_BG_COLOR */

#define RF_NEED_TO_MOVE    0
#define RF_CR_OK           1
#define RF_NONEED_TO_MOVE  2
#ifdef USE_BG_COLOR
#define M_MEND (S_STANDOUT|S_UNDERLINE|S_BOLD|S_COLORED|S_BCOLORED|S_GRAPHICS)
#else				/* not USE_BG_COLOR */
#define M_MEND (S_STANDOUT|S_UNDERLINE|S_BOLD|S_COLORED|S_GRAPHICS)
#endif				/* not USE_BG_COLOR */
void
refresh(void)
{
    int line, col, pcol;
    int pline = CurLine;
    int moved = RF_NEED_TO_MOVE;
#ifdef USE_M17N
    char **pc;
#else
    char *pc;
#endif
    l_prop *pr, mode = 0;
    l_prop color = COL_FTERM;
#ifdef USE_BG_COLOR
    l_prop bcolor = COL_BTERM;
#endif				/* USE_BG_COLOR */
    short *dirty;

#ifdef USE_M17N
    wc_putc_init(InnerCharset, DisplayCharset);
#endif
    for (line = 0; line <= LASTLINE; line++) {
	dirty = &ScreenImage[line]->isdirty;
	if (*dirty & L_DIRTY) {
	    *dirty &= ~L_DIRTY;
	    pc = ScreenImage[line]->lineimage;
	    pr = ScreenImage[line]->lineprop;
	    for (col = 0; col < COLS && !(pr[col] & S_EOL); col++) {
		if (*dirty & L_NEED_CE && col >= ScreenImage[line]->eol) {
		    if (need_redraw(pc[col], pr[col], SPACE, 0))
			break;
		}
		else {
		    if (pr[col] & S_DIRTY)
			break;
		}
	    }
	    if (*dirty & (L_NEED_CE | L_CLRTOEOL)) {
		pcol = ScreenImage[line]->eol;
		if (pcol >= COLS) {
		    *dirty &= ~(L_NEED_CE | L_CLRTOEOL);
		    pcol = col;
		}
	    }
	    else {
		pcol = col;
	    }
	    if (line < LINES - 2 && pline == line - 1 && pcol == 0) {
		switch (moved) {
		case RF_NEED_TO_MOVE:
		    MOVE(line, 0);
		    moved = RF_CR_OK;
		    break;
		case RF_CR_OK:
		    write1('\n');
		    write1('\r');
		    break;
		case RF_NONEED_TO_MOVE:
		    moved = RF_CR_OK;
		    break;
		}
	    }
	    else {
		MOVE(line, pcol);
		moved = RF_CR_OK;
	    }
	    if (*dirty & (L_NEED_CE | L_CLRTOEOL)) {
		writestr(T_ce);
		if (col != pcol)
		    MOVE(line, col);
	    }
	    pline = line;
	    pcol = col;
	    for (; col < COLS; col++) {
		if (pr[col] & S_EOL)
		    break;

		/* 
		 * some terminal emulators do linefeed when a
		 * character is put on COLS-th column. this behavior
		 * is different from one of vt100, but such terminal
		 * emulators are used as vt100-compatible
		 * emulators. This behaviour causes scroll when a
		 * character is drawn on (COLS-1,LINES-1) point.  To
		 * avoid the scroll, I prohibit to draw character on
		 * (COLS-1,LINES-1).
		 */
#if !defined(USE_BG_COLOR) || defined(__CYGWIN__)
#ifdef __CYGWIN__
		if (isWinConsole)
#endif
		    if (line == LINES - 1 && col == COLS - 1)
			break;
#endif				/* !defined(USE_BG_COLOR) || defined(__CYGWIN__) */
		if ((!(pr[col] & S_STANDOUT) && (mode & S_STANDOUT)) ||
		    (!(pr[col] & S_UNDERLINE) && (mode & S_UNDERLINE)) ||
		    (!(pr[col] & S_BOLD) && (mode & S_BOLD)) ||
		    (!(pr[col] & S_COLORED) && (mode & S_COLORED))
#ifdef USE_BG_COLOR
		    || (!(pr[col] & S_BCOLORED) && (mode & S_BCOLORED))
#endif				/* USE_BG_COLOR */
		    || (!(pr[col] & S_GRAPHICS) && (mode & S_GRAPHICS))) {
		    if ((mode & S_COLORED)
#ifdef USE_BG_COLOR
			|| (mode & S_BCOLORED)
#endif				/* USE_BG_COLOR */
			)
			writestr(T_op);
		    if (mode & S_GRAPHICS)
			writestr(T_ae);
		    writestr(T_me);
		    mode &= ~M_MEND;
		}
		if ((*dirty & L_NEED_CE && col >= ScreenImage[line]->eol) ?
		    need_redraw(pc[col], pr[col], SPACE,
				0) : (pr[col] & S_DIRTY)) {
		    if (pcol == col - 1)
			writestr(T_nd);
		    else if (pcol != col)
			MOVE(line, col);

		    if ((pr[col] & S_STANDOUT) && !(mode & S_STANDOUT)) {
			writestr(T_so);
			mode |= S_STANDOUT;
		    }
		    if ((pr[col] & S_UNDERLINE) && !(mode & S_UNDERLINE)) {
			writestr(T_us);
			mode |= S_UNDERLINE;
		    }
		    if ((pr[col] & S_BOLD) && !(mode & S_BOLD)) {
			writestr(T_md);
			mode |= S_BOLD;
		    }
		    if ((pr[col] & S_COLORED) && (pr[col] ^ mode) & COL_FCOLOR) {
			color = (pr[col] & COL_FCOLOR);
			mode = ((mode & ~COL_FCOLOR) | color);
			writestr(color_seq(color));
		    }
#ifdef USE_BG_COLOR
		    if ((pr[col] & S_BCOLORED)
			&& (pr[col] ^ mode) & COL_BCOLOR) {
			bcolor = (pr[col] & COL_BCOLOR);
			mode = ((mode & ~COL_BCOLOR) | bcolor);
			writestr(bcolor_seq(bcolor));
		    }
#endif				/* USE_BG_COLOR */
		    if ((pr[col] & S_GRAPHICS) && !(mode & S_GRAPHICS)) {
#ifdef USE_M17N
			wc_putc_end(ttyf);
#endif
			if (!graph_enabled) {
			    graph_enabled = 1;
			    writestr(T_eA);
			}
			writestr(T_as);
			mode |= S_GRAPHICS;
		    }
#ifdef USE_M17N
		    if (pr[col] & S_GRAPHICS)
			write1(graphchar(*pc[col]));
		    else if (CHMODE(pr[col]) != C_WCHAR2)
			wc_putc(pc[col], ttyf);
#else
		    write1((pr[col] & S_GRAPHICS) ? graphchar(pc[col]) :
			   pc[col]);
#endif
		    pcol = col + 1;
		}
	    }
	    if (col == COLS)
		moved = RF_NEED_TO_MOVE;
	    for (; col < COLS && !(pr[col] & S_EOL); col++)
		pr[col] |= S_EOL;
	}
	*dirty &= ~(L_NEED_CE | L_CLRTOEOL);
	if (mode & M_MEND) {
	    if (mode & (S_COLORED
#ifdef USE_BG_COLOR
			| S_BCOLORED
#endif				/* USE_BG_COLOR */
		))
		writestr(T_op);
	    if (mode & S_GRAPHICS) {
		writestr(T_ae);
#ifdef USE_M17N
		wc_putc_clear_status();
#endif
	    }
	    writestr(T_me);
	    mode &= ~M_MEND;
	}
    }
#ifdef USE_M17N
    wc_putc_end(ttyf);
#endif
    MOVE(CurLine, CurColumn);
    flush_tty();
}

void
clear(void)
{
    int i, j;
    l_prop *p;
    writestr(T_cl);
    move(0, 0);
    for (i = 0; i < LINES; i++) {
	ScreenImage[i]->isdirty = 0;
	p = ScreenImage[i]->lineprop;
	for (j = 0; j < COLS; j++) {
	    p[j] = S_EOL;
	}
    }
    CurrentMode = C_ASCII;
}

#ifdef USE_RAW_SCROLL
static void
scroll_raw(void)
{				/* raw scroll */
    MOVE(LINES - 1, 0);
    write1('\n');
}

void
scroll(int n)
{				/* scroll up */
    int cli = CurLine, cco = CurColumn;
    Screen *t;
    int i, j, k;

    i = LINES;
    j = n;
    do {
	k = j;
	j = i % k;
	i = k;
    } while (j);
    do {
	k--;
	i = k;
	j = (i + n) % LINES;
	t = ScreenImage[k];
	while (j != k) {
	    ScreenImage[i] = ScreenImage[j];
	    i = j;
	    j = (i + n) % LINES;
	}
	ScreenImage[i] = t;
    } while (k);

    for (i = 0; i < n; i++) {
	t = ScreenImage[LINES - 1 - i];
	t->isdirty = 0;
	for (j = 0; j < COLS; j++)
	    t->lineprop[j] = S_EOL;
	scroll_raw();
    }
    move(cli, cco);
}

void
rscroll(int n)
{				/* scroll down */
    int cli = CurLine, cco = CurColumn;
    Screen *t;
    int i, j, k;

    i = LINES;
    j = n;
    do {
	k = j;
	j = i % k;
	i = k;
    } while (j);
    do {
	k--;
	i = k;
	j = (LINES + i - n) % LINES;
	t = ScreenImage[k];
	while (j != k) {
	    ScreenImage[i] = ScreenImage[j];
	    i = j;
	    j = (LINES + i - n) % LINES;
	}
	ScreenImage[i] = t;
    } while (k);
    if (T_sr && *T_sr) {
	MOVE(0, 0);
	for (i = 0; i < n; i++) {
	    t = ScreenImage[i];
	    t->isdirty = 0;
	    for (j = 0; j < COLS; j++)
		t->lineprop[j] = S_EOL;
	    writestr(T_sr);
	}
	move(cli, cco);
    }
    else {
	for (i = 0; i < LINES; i++) {
	    t = ScreenImage[i];
	    t->isdirty |= L_DIRTY | L_NEED_CE;
	    for (j = 0; j < COLS; j++) {
		t->lineprop[j] |= S_DIRTY;
	    }
	}
    }
}
#endif

#if 0
void
need_clrtoeol(void)
{
    /* Clear to the end of line as the need arises */
    l_prop *lprop = ScreenImage[CurLine]->lineprop;

    if (lprop[CurColumn] & S_EOL)
	return;

    if (!(ScreenImage[CurLine]->isdirty & (L_NEED_CE | L_CLRTOEOL)) ||
	ScreenImage[CurLine]->eol > CurColumn)
	ScreenImage[CurLine]->eol = CurColumn;

    ScreenImage[CurLine]->isdirty |= L_NEED_CE;
}
#endif				/* 0 */

/* XXX: conflicts with curses's clrtoeol(3) ? */
void
clrtoeol(void)
{				/* Clear to the end of line */
    int i;
    l_prop *lprop = ScreenImage[CurLine]->lineprop;

    if (lprop[CurColumn] & S_EOL)
	return;

    if (!(ScreenImage[CurLine]->isdirty & (L_NEED_CE | L_CLRTOEOL)) ||
	ScreenImage[CurLine]->eol > CurColumn)
	ScreenImage[CurLine]->eol = CurColumn;

    ScreenImage[CurLine]->isdirty |= L_CLRTOEOL;
    touch_line();
    for (i = CurColumn; i < COLS && !(lprop[i] & S_EOL); i++) {
	lprop[i] = S_EOL | S_DIRTY;
    }
}

#ifdef USE_BG_COLOR
static void
clrtoeol_with_bcolor(void)
{
    int i, cli, cco;
    l_prop pr;

    if (!(CurrentMode & S_BCOLORED)) {
	clrtoeol();
	return;
    }
    cli = CurLine;
    cco = CurColumn;
    pr = CurrentMode;
    CurrentMode = (CurrentMode & (M_CEOL | S_BCOLORED)) | C_ASCII;
    for (i = CurColumn; i < COLS; i++)
	addch(' ');
    move(cli, cco);
    CurrentMode = pr;
}

void
clrtoeolx(void)
{
    clrtoeol_with_bcolor();
}
#else				/* not USE_BG_COLOR */

void
clrtoeolx(void)
{
    clrtoeol();
}
#endif				/* not USE_BG_COLOR */

static void
clrtobot_eol(void (*clrtoeol) (void))
{
    int l, c;

    l = CurLine;
    c = CurColumn;
    (*clrtoeol) ();
    CurColumn = 0;
    CurLine++;
    for (; CurLine < LINES; CurLine++)
	(*clrtoeol) ();
    CurLine = l;
    CurColumn = c;
}

void
clrtobot(void)
{
    clrtobot_eol(clrtoeol);
}

void
clrtobotx(void)
{
    clrtobot_eol(clrtoeolx);
}

#if 0
void
no_clrtoeol(void)
{
    int i;
    l_prop *lprop = ScreenImage[CurLine]->lineprop;

    ScreenImage[CurLine]->isdirty &= ~L_CLRTOEOL;
}
#endif				/* 0 */

void
addstr(char *s)
{
#ifdef USE_M17N
    int len;

    while (*s != '\0') {
	len = wtf_len((wc_uchar *) s);
	addmch(s, len);
	s += len;
    }
#else
    while (*s != '\0')
	addch(*(s++));
#endif
}

void
addnstr(char *s, int n)
{
    int i;
#ifdef USE_M17N
    int len, width;

    for (i = 0; *s != '\0';) {
	width = wtf_width((wc_uchar *) s);
	if (i + width > n)
	    break;
	len = wtf_len((wc_uchar *) s);
	addmch(s, len);
	s += len;
	i += width;
    }
#else
    for (i = 0; i < n && *s != '\0'; i++)
	addch(*(s++));
#endif
}

void
addnstr_sup(char *s, int n)
{
    int i;
#ifdef USE_M17N
    int len, width;

    for (i = 0; *s != '\0';) {
	width = wtf_width((wc_uchar *) s);
	if (i + width > n)
	    break;
	len = wtf_len((wc_uchar *) s);
	addmch(s, len);
	s += len;
	i += width;
    }
#else
    for (i = 0; i < n && *s != '\0'; i++)
	addch(*(s++));
#endif
    for (; i < n; i++)
	addch(' ');
}

void
crmode(void)
#ifndef HAVE_SGTTY_H
{
    ttymode_reset(ICANON, IXON);
    ttymode_set(ISIG, 0);
#ifdef HAVE_TERMIOS_H
    set_cc(VMIN, 1);
#else				/* not HAVE_TERMIOS_H */
    set_cc(VEOF, 1);
#endif				/* not HAVE_TERMIOS_H */
}
#else				/* HAVE_SGTTY_H */
{
    ttymode_set(CBREAK, 0);
}
#endif				/* HAVE_SGTTY_H */

void
nocrmode(void)
#ifndef HAVE_SGTTY_H
{
    ttymode_set(ICANON, 0);
#ifdef HAVE_TERMIOS_H
    set_cc(VMIN, 4);
#else				/* not HAVE_TERMIOS_H */
    set_cc(VEOF, 4);
#endif				/* not HAVE_TERMIOS_H */
}
#else				/* HAVE_SGTTY_H */
{
    ttymode_reset(CBREAK, 0);
}
#endif				/* HAVE_SGTTY_H */

void
term_echo(void)
{
    ttymode_set(ECHO, 0);
}

void
term_noecho(void)
{
    ttymode_reset(ECHO, 0);
}

void
term_raw(void)
#ifndef HAVE_SGTTY_H
#ifdef IEXTEN
#define TTY_MODE ISIG|ICANON|ECHO|IEXTEN
#else				/* not IEXTEN */
#define TTY_MODE ISIG|ICANON|ECHO
#endif				/* not IEXTEN */
{
    ttymode_reset(TTY_MODE, IXON | IXOFF);
#ifdef HAVE_TERMIOS_H
    set_cc(VMIN, 1);
#else				/* not HAVE_TERMIOS_H */
    set_cc(VEOF, 1);
#endif				/* not HAVE_TERMIOS_H */
}
#else				/* HAVE_SGTTY_H */
{
    ttymode_set(RAW, 0);
}
#endif				/* HAVE_SGTTY_H */

void
term_cooked(void)
#ifndef HAVE_SGTTY_H
{
#ifdef __EMX__
    /* On XFree86/OS2, some scrambled characters
     * will appear when asserting IEXTEN flag.
     */
    ttymode_set((TTY_MODE) & ~IEXTEN, 0);
#else
    ttymode_set(TTY_MODE, 0);
#endif
#ifdef HAVE_TERMIOS_H
    set_cc(VMIN, 4);
#else				/* not HAVE_TERMIOS_H */
    set_cc(VEOF, 4);
#endif				/* not HAVE_TERMIOS_H */
}
#else				/* HAVE_SGTTY_H */
{
    ttymode_reset(RAW, 0);
}
#endif				/* HAVE_SGTTY_H */

void
term_cbreak(void)
{
    term_cooked();
    term_noecho();
}

void
term_title(char *s)
{
    if (!fmInitialized)
        return;
    if (title_str != NULL) {
#ifdef __CYGWIN__
	if (isLocalConsole && title_str == CYGWIN_TITLE) {
	    Str buff;
	    buff = Sprintf(title_str, s);
	    if (buff->length > 1024) {
		Strtruncate(buff, 1024);
	    }
	    SetConsoleTitle(buff->ptr);
	}
	else if (isLocalConsole || !isWinConsole)
#endif
        fprintf(ttyf, title_str, s);
    }
}

char
getch(void)
{
    char c;

    while (
#ifdef SUPPORT_WIN9X_CONSOLE_MBCS
	      read_win32_console(&c, 1)
#else
	      read(tty, &c, 1)
#endif
	      < (int)1) {
	if (errno == EINTR || errno == EAGAIN)
	    continue;
	/* error happend on read(2) */
	quitfm();
	break;			/* unreachable */
    }
    return c;
}

#ifdef USE_MOUSE
#ifdef USE_GPM
char
wgetch(void *p)
{
    char c;

    /* read(tty, &c, 1); */
    while (read(tty, &c, 1) < (ssize_t) 1) {
	if (errno == EINTR || errno == EAGAIN)
	    continue;
	/* error happend on read(2) */
	quitfm();
	break;			/* unreachable */
    }
    return c;
}

int
do_getch(void)
{
    if (is_xterm || !gpm_handler)
	return getch();
    else
	return Gpm_Getch();
}
#endif				/* USE_GPM */

#ifdef USE_SYSMOUSE
int
sysm_getch(void)
{
    fd_set rfd;
    int key, x, y;

    FD_ZERO(&rfd);
    FD_SET(tty, &rfd);
    while (select(tty + 1, &rfd, NULL, NULL, NULL) <= 0) {
	if (errno == EINTR) {
	    x = xpix / cwidth;
	    y = ypix / cheight;
	    key = (*sysm_handler) (x, y, nbs, obs);
	    if (key != 0)
		return key;
	}
    }
    return getch();
}

int
do_getch(void)
{
    if (is_xterm || !sysm_handler)
	return getch();
    else
	return sysm_getch();
}

MySignalHandler
sysmouse(SIGNAL_ARG)
{
    struct mouse_info mi;

    mi.operation = MOUSE_GETINFO;
    if (ioctl(tty, CONS_MOUSECTL, &mi) == -1)
	return;
    xpix = mi.u.data.x;
    ypix = mi.u.data.y;
    obs = nbs;
    nbs = mi.u.data.buttons & 0x7;
    /* for cosmetic bug in syscons.c on FreeBSD 3.[34] */
    mi.operation = MOUSE_HIDE;
    ioctl(tty, CONS_MOUSECTL, &mi);
    mi.operation = MOUSE_SHOW;
    ioctl(tty, CONS_MOUSECTL, &mi);
}
#endif				/* USE_SYSMOUSE */
#endif				/* USE_MOUSE */

void
bell(void)
{
    write1(7);
}

static void
skip_escseq(void)
{
    int c;

    c = getch();
    if (c == '[' || c == 'O') {
	c = getch();
#ifdef USE_MOUSE
	if (is_xterm && c == 'M') {
	    getch();
	    getch();
	    getch();
	} else if (is_xterm && c == '<') {
	    c = getch();
	    while (IS_DIGIT(c) || c == ';')
		c = getch();
	}
	else
#endif
	    while (IS_DIGIT(c))
		c = getch();
    }
}

int
sleep_till_anykey(int sec, int purge)
{
    fd_set rfd;
    struct timeval tim;
    int er, c, ret;
    TerminalMode ioval;

    TerminalGet(tty, &ioval);
    term_raw();

    tim.tv_sec = sec;
    tim.tv_usec = 0;

    FD_ZERO(&rfd);
    FD_SET(tty, &rfd);

    ret = select(tty + 1, &rfd, 0, 0, &tim);
    if (ret > 0 && purge) {
	c = getch();
	if (c == ESC_CODE)
	    skip_escseq();
    }
    er = TerminalSet(tty, &ioval);
    if (er == -1) {
	printf("Error occurred: errno=%d\n", errno);
	reset_error_exit(SIGNAL_ARGLIST);
    }
    return ret;
}

#ifdef USE_MOUSE

#define XTERM_ON   {fputs("\033[?1001s\033[?1000h\033[?1006h",ttyf); flush_tty();}
#define XTERM_OFF  {fputs("\033[?1006l\033[?1000l\033[?1001r",ttyf); flush_tty();}
#define CYGWIN_ON  {fputs("\033[?1000h",ttyf); flush_tty();}
#define CYGWIN_OFF {fputs("\033[?1000l",ttyf); flush_tty();}

#ifdef USE_GPM
/* Linux console with GPM support */

void
mouse_init(void)
{
    Gpm_Connect conn;
    extern int gpm_process_mouse(Gpm_Event *, void *);
    int r;

    if (mouseActive)
	return;
    conn.eventMask = ~0;
    conn.defaultMask = 0;
    conn.maxMod = 0;
    conn.minMod = 0;

    gpm_handler = NULL;
    r = Gpm_Open(&conn, 0);
    if (r == -2) {
	/*
	 * If Gpm_Open() success, returns >= 0
	 * Gpm_Open() returns -2 in case of xterm.
	 * Gpm_Close() is necessary here. Otherwise,
	 * xterm is being left in the mode where the mouse clicks are
	 * passed through to the application.
	 */
	Gpm_Close();
	is_xterm = (NEED_XTERM_ON | NEED_XTERM_OFF);
    }
    else if (r >= 0) {
	gpm_handler = gpm_process_mouse;
	is_xterm = 0;
    }
    if (is_xterm) {
	XTERM_ON;
    }
    mouseActive = 1;
}

void
mouse_end(void)
{
    if (mouseActive == 0)
	return;
    if (is_xterm) {
	XTERM_OFF;
    }
    else
	Gpm_Close();
    mouseActive = 0;
}

#elif	defined(USE_SYSMOUSE)
/* *BSD console with sysmouse support */
void
mouse_init(void)
{
    mouse_info_t mi;
    extern int sysm_process_mouse();

    if (mouseActive)
	return;
    if (is_xterm) {
	XTERM_ON;
    }
    else {
#if defined(FBIO_MODEINFO) || defined(CONS_MODEINFO)	/* FreeBSD > 2.x */
#ifndef FBIO_GETMODE		/* FreeBSD 3.x */
#define FBIO_GETMODE    CONS_GET
#define FBIO_MODEINFO   CONS_MODEINFO
#endif				/* FBIO_GETMODE */
	video_info_t vi;

	if (ioctl(tty, FBIO_GETMODE, &vi.vi_mode) != -1 &&
	    ioctl(tty, FBIO_MODEINFO, &vi) != -1) {
	    cwidth = vi.vi_cwidth;
	    cheight = vi.vi_cheight;
	}
#endif				/* defined(FBIO_MODEINFO) ||
				 * defined(CONS_MODEINFO) */
	mySignal(SIGUSR2, SIG_IGN);
	mi.operation = MOUSE_MODE;
	mi.u.mode.mode = 0;
	mi.u.mode.signal = SIGUSR2;
	sysm_handler = NULL;
	if (ioctl(tty, CONS_MOUSECTL, &mi) != -1) {
	    mySignal(SIGUSR2, sysmouse);
	    mi.operation = MOUSE_SHOW;
	    ioctl(tty, CONS_MOUSECTL, &mi);
	    sysm_handler = sysm_process_mouse;
	}
    }
    mouseActive = 1;
}

void
mouse_end(void)
{
    if (mouseActive == 0)
	return;
    if (is_xterm) {
	XTERM_OFF;
    }
    else {
	mouse_info_t mi;
	mi.operation = MOUSE_MODE;
	mi.u.mode.mode = 0;
	mi.u.mode.signal = 0;
	ioctl(tty, CONS_MOUSECTL, &mi);
    }
    mouseActive = 0;
}

#else
/* not GPM nor SYSMOUSE, but use mouse with xterm */

void
mouse_init(void)
{
    if (mouseActive)
	return;
    if (is_xterm & NEED_XTERM_ON) {
	XTERM_ON;
    }
#ifdef __CYGWIN__
    else if (is_xterm & NEED_CYGWIN_ON) {
	CYGWIN_ON;
    }
#endif
    mouseActive = 1;
}

void
mouse_end(void)
{
    if (mouseActive == 0)
	return;
    if (is_xterm & NEED_XTERM_OFF) {
	XTERM_OFF;
    }
#ifdef __CYGWIN__
    else if (is_xterm & NEED_CYGWIN_OFF) {
	CYGWIN_OFF;
    }
#endif
    mouseActive = 0;
}

#endif				/* not USE_GPM nor USE_SYSMOUSE */


void
mouse_active(void)
{
    if (!mouseActive)
	mouse_init();
}

void
mouse_inactive(void)
{
    if (mouseActive && is_xterm)
	mouse_end();
}

#endif				/* USE_MOUSE */

void
flush_tty(void)
{
    if (ttyf)
	fflush(ttyf);
}

#ifdef USE_IMAGE
void
touch_cursor(void)
{
#ifdef USE_M17N
    int i;
#endif
    touch_line();
#ifdef USE_M17N
    for (i = CurColumn; i >= 0; i--) {
	touch_column(i);
	if (CHMODE(ScreenImage[CurLine]->lineprop[i]) != C_WCHAR2) 
	    break;
    }
    for (i = CurColumn + 1; i < COLS; i++) {
	if (CHMODE(ScreenImage[CurLine]->lineprop[i]) != C_WCHAR2) 
	    break;
	touch_column(i);
    }
#else
    touch_column(CurColumn);
#endif
}
#endif

#ifdef __MINGW32_VERSION

int tgetent(char *bp, char *name)
{
  return 0;
}

int tgetnum(char *id)
{
  return -1;
}

int tgetflag(char *id)
{
  return 0;
}

char *tgetstr(char *id, char **area)
{
  id = "";
}

char *tgoto(char *cap, int col, int row)
{
}

int tputs(char *str, int affcnt, int (*putc)(char))
{
}

char *ttyname(int tty)
{
  return "CON";
}

#endif /* __MINGW32_VERSION */
