/* $Id: main.c,v 1.149 2002/11/21 17:05:47 ukai Exp $ */
#define MAINPROGRAM
#include "fm.h"
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#if defined(HAVE_WAITPID) || defined(HAVE_WAIT3)
#include <sys/wait.h>
#endif
#include <time.h>
#include "terms.h"
#include "myctype.h"
#include "regex.h"
#ifdef USE_MOUSE
#ifdef USE_GPM
#include <gpm.h>
#endif				/* USE_GPM */
#if defined(USE_GPM) || defined(USE_SYSMOUSE)
extern int do_getch();
#define getch()	do_getch()
#endif				/* defined(USE_GPM) || defined(USE_SYSMOUSE) */
#endif

#define DSTR_LEN	256

static char *config_filename = NULL;

Hist *LoadHist;
Hist *SaveHist;
Hist *URLHist;
Hist *ShellHist;
Hist *TextHist;

typedef struct {
    int cmd;
    void *user_data;
} Event;
#define N_EVENT_QUEUE 10
static Event eventQueue[N_EVENT_QUEUE];
static int n_event_queue;

#ifdef USE_ALARM
typedef struct {
    int sec;
    int cmd;
    void *data;
    short status;
    Buffer *buffer;
} AlarmEvent;
static AlarmEvent CurrentAlarm = {
    0, FUNCNAME_nulcmd, NULL, AL_UNSET, NULL
};
static AlarmEvent PrevAlarm = {
    0, FUNCNAME_nulcmd, NULL, AL_UNSET, NULL
};
static MySignalHandler SigAlarm(SIGNAL_ARG);
#endif

#ifdef SIGWINCH
static int need_resize_screen = FALSE;
static MySignalHandler resize_hook(SIGNAL_ARG);
static MySignalHandler resize_handler(SIGNAL_ARG);
static void resize_screen(void);
#endif

#ifdef SIGPIPE
static MySignalHandler SigPipe(SIGNAL_ARG);
#endif

#ifdef USE_MARK
static char *MarkString = NULL;
#endif
static char *SearchString = NULL;
int (*searchRoutine) (Buffer *, char *);

JMP_BUF IntReturn;

static void cmd_loadfile(char *path);
static void cmd_loadURL(char *url, ParsedURL *current, char *referer);
static void cmd_loadBuffer(Buffer *buf, int prop, int linkid);
static void keyPressEventProc(int c);
int show_params_p = 0;
void show_params(FILE * fp);

static char *getCurWord(Buffer *buf, int *spos, int *epos,
			const char *badchars);

static int display_ok = FALSE;
static void dump_source(Buffer *);
static void dump_head(Buffer *);
static void dump_extra(Buffer *);
int prec_num = 0;
int prev_key = -1;
int on_target = 1;
static int add_download_list = FALSE;

void set_buffer_environ(Buffer *);

static void _followForm(int);
static void _goLine(char *);
static void _newT(void);
static void calcTabPos(void);
static void followTab(TabBuffer * tab);
static void moveTab(TabBuffer * t, TabBuffer * t2, int right);
static int check_target = TRUE;
#define PREC_NUM (prec_num ? prec_num : 1)
#define PREC_LIMIT 10000
static int searchKeyNum(void);

#define help() fusage(stdout, 0)
#define usage() fusage(stderr, 1)

static void
fversion(FILE * f)
{
    fprintf(f, "w3m version %s, options %s\n", w3m_version,
#if LANG == JA
	    "lang=ja"
#ifdef KANJI_SYMBOLS
	    ",kanji-symbols"
#endif
#else
	    "lang=en"
#endif
#ifdef USE_IMAGE
	    ",image"
#endif
#ifdef USE_COLOR
	    ",color"
#ifdef USE_ANSI_COLOR
	    ",ansi-color"
#endif
#endif
#ifdef USE_MOUSE
	    ",mouse"
#ifdef USE_GPM
	    ",gpm"
#endif
#ifdef USE_SYSMOUSE
	    ",sysmouse"
#endif
#endif
#ifdef USE_MENU
	    ",menu"
#endif
#ifdef USE_COOKIE
	    ",cookie"
#endif
#ifdef USE_SSL
	    ",ssl"
#ifdef USE_SSL_VERIFY
	    ",ssl-verify"
#endif
#endif
#ifdef USE_EXTERNAL_URI_LOADER
	    ",external-uri-loader"
#endif
#ifdef USE_W3MMAILER
	    ",w3mmailer"
#endif
#ifdef USE_NNTP
	    ",nntp"
#endif
#ifdef USE_GOPHER
	    ",gopher"
#endif
#ifdef INET6
	    ",ipv6"
#endif
#ifdef USE_ALARM
	    ",alarm"
#endif
#ifdef USE_MARK
	    ",mark"
#endif
#ifdef USE_MIGEMO
	    ",migemo"
#endif
	);
}

static void
fusage(FILE * f, int err)
{
    fversion(f);
    fprintf(f, "usage: w3m [options] [URL or filename]\noptions:\n");
    fprintf(f, "    -t tab           set tab width\n");
    fprintf(f, "    -r               ignore backspace effect\n");
    fprintf(f, "    -l line          # of preserved line (default 10000)\n");
#ifdef JP_CHARSET
#ifndef DEBIAN			/* disabled by ukai: -s is used for squeeze multi lines */
    fprintf(f, "    -e               EUC-JP\n");
    fprintf(f, "    -s               Shift_JIS\n");
    fprintf(f, "    -j               JIS\n");
#endif
    fprintf(f, "    -O e|s|j|N|m|n   display code\n");
    fprintf(f, "    -I e|s           document code\n");
#endif				/* JP_CHARSET */
    fprintf(f, "    -B               load bookmark\n");
    fprintf(f, "    -bookmark file   specify bookmark file\n");
    fprintf(f, "    -T type          specify content-type\n");
    fprintf(f, "    -m               internet message mode\n");
    fprintf(f, "    -v               visual startup mode\n");
#ifdef USE_COLOR
    fprintf(f, "    -M               monochrome display\n");
#endif				/* USE_COLOR */
    fprintf(f, "    -F               automatically render frame\n");
    fprintf(f,
	    "    -cols width      specify column width (used with -dump)\n");
    fprintf(f,
	    "    -ppc count       specify the number of pixels per character (4.0...32.0)\n");
#ifdef USE_IMAGE
    fprintf(f,
	    "    -ppl count       specify the number of pixels per line (4.0...64.0)\n");
#endif
    fprintf(f, "    -dump            dump formatted page into stdout\n");
    fprintf(f,
	    "    -dump_head       dump response of HEAD request into stdout\n");
    fprintf(f, "    -dump_source     dump page source into stdout\n");
    fprintf(f, "    -dump_both       dump HEAD and source into stdout\n");
    fprintf(f,
	    "    -dump_extra      dump HEAD, source, and extra information into stdout\n");
    fprintf(f, "    -post file       use POST method with file content\n");
    fprintf(f, "    -header string   insert string as a header\n");
    fprintf(f, "    +<num>           goto <num> line\n");
    fprintf(f, "    -num             show line number\n");
    fprintf(f, "    -no-proxy        don't use proxy\n");
#ifdef USE_MOUSE
    fprintf(f, "    -no-mouse        don't use mouse\n");
#endif				/* USE_MOUSE */
#ifdef USE_COOKIE
    fprintf(f,
	    "    -cookie          use cookie (-no-cookie: don't use cookie)\n");
#endif				/* USE_COOKIE */
    fprintf(f, "    -pauth user:pass proxy authentication\n");
#ifndef KANJI_SYMBOLS
    fprintf(f, "    -no-graph        don't use graphic character\n");
#endif				/* not KANJI_SYMBOLS */
#ifdef DEBIAN			/* replaced by ukai: pager requires -s */
    fprintf(f, "    -s               squeeze multiple blank lines\n");
#else
    fprintf(f, "    -S               squeeze multiple blank lines\n");
#endif
    fprintf(f, "    -W               toggle wrap search mode\n");
    fprintf(f, "    -X               don't use termcap init/deinit\n");
    fprintf(f,
	    "    -title[=TERM]    set buffer name to terminal title string\n");
    fprintf(f, "    -o opt=value     assign value to config option\n");
    fprintf(f, "    -show-option     print all config options\n");
    fprintf(f, "    -config file     specify config file\n");
    fprintf(f, "    -help            print this usage message\n");
    fprintf(f, "    -version         print w3m version\n");
    fprintf(f, "    -debug           DO NOT USE\n");
    if (show_params_p)
	show_params(f);
    exit(err);
}

static GC_warn_proc orig_GC_warn_proc = NULL;
#define GC_WARN_KEEP_MAX (20)

static void
wrap_GC_warn_proc(char *msg, GC_word arg)
{
    if (fmInitialized) {
	/* *INDENT-OFF* */
	static struct {
	    char *msg;
	    GC_word arg;
	} msg_ring[GC_WARN_KEEP_MAX];
	/* *INDENT-ON* */
	static int i = 0;
	static int n = 0;
	static int lock = 0;
	int j;

	j = (i + n) % (sizeof(msg_ring) / sizeof(msg_ring[0]));
	msg_ring[j].msg = msg;
	msg_ring[j].arg = arg;

	if (n < sizeof(msg_ring) / sizeof(msg_ring[0]))
	    ++n;
	else
	    ++i;

	if (!lock) {
	    lock = 1;

	    for (; n > 0; --n, ++i) {
		i %= sizeof(msg_ring) / sizeof(msg_ring[0]);
		disp_message_nsec(Sprintf
				  (msg_ring[i].msg,
				   (unsigned long)msg_ring[i].arg)->ptr, FALSE,
				  1, TRUE, FALSE);
	    }

	    lock = 0;
	}
    }
    else if (orig_GC_warn_proc)
	orig_GC_warn_proc(msg, arg);
    else
	fprintf(stderr, msg, (unsigned long)arg);
}

#ifdef SIGCHLD
static void
sig_chld(int signo)
{
    int p_stat;
#ifdef HAVE_WAITPID
    pid_t pid;

    while ((pid = waitpid(-1, &p_stat, WNOHANG)) > 0) {
	;
    }
#elif HAVE_WAIT3
    int pid;

    while ((pid = wait3(&p_stat, WNOHANG, NULL)) > 0) {
	;
    }
#else
    wait(&p_stat);
#endif
    signal(SIGCHLD, sig_chld);
    return;
}
#endif

Str
make_optional_header_string(char *s)
{
    char *p;
    Str hs;

    if (strchr(s, '\n') || strchr(s, '\r'))
	return NULL;
    for (p = s; *p && *p != ':'; p++) ;
    if (*p != ':' || p == s)
	return NULL;
    hs = Strnew_size(strlen(s) + 3);
    Strcopy_charp_n(hs, s, p - s);
    if (!Strcasecmp_charp(hs, "content-type"))
	override_content_type = TRUE;
    Strcat_charp(hs, ": ");
    if (*(++p)) {		/* not null header */
	SKIP_BLANKS(p);		/* skip white spaces */
	Strcat_charp(hs, p);
    }
    Strcat_charp(hs, "\r\n");
    return hs;
}

int
main(int argc, char **argv, char **envp)
{
    Buffer *newbuf = NULL;
    char *p, c;
    int i;
    InputStream redin;
    char *line_str = NULL;
    char **load_argv;
    FormList *request;
    int load_argc = 0;
    int load_bookmark = FALSE;
    int visual_start = FALSE;
    char search_header = FALSE;
    char *default_type = NULL;
    char *post_file = NULL;
    Str err_msg;

#ifndef HAVE_SYS_ERRLIST
    prepare_sys_errlist();
#endif				/* not HAVE_SYS_ERRLIST */

    srand48(time(0));

    NO_proxy_domains = newTextList();
    fileToDelete = newTextList();

    load_argv = New_N(char *, argc - 1);
    load_argc = 0;

    CurrentDir = currentdir();
    BookmarkFile = NULL;
    rc_dir = expandName(RC_DIR);
    i = strlen(rc_dir);
    if (i > 1 && rc_dir[i - 1] == '/')
	rc_dir[i - 1] = '\0';
    config_filename = rcFile(CONFIG_FILE);
    create_option_search_table();

    /* argument search 1 */
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') {
	    if (!strcmp("-config", argv[i])) {
		argv[i] = "-dummy";
		if (++i >= argc)
		    usage();
		config_filename = argv[i];
		argv[i] = "-dummy";
	    }
	    else if (!strcmp("-h", argv[i]) || !strcmp("-help", argv[i]))
		help();
	    else if (!strcmp("-V", argv[i]) || !strcmp("-version", argv[i])) {
		fversion(stdout);
		exit(0);
	    }
	}
    }

    /* initializations */
    init_rc(config_filename);

    LoadHist = newHist();
    SaveHist = newHist();
    ShellHist = newHist();
    TextHist = newHist();
    URLHist = newHist();
#ifdef USE_HISTORY
    loadHistory(URLHist);
#endif				/* not USE_HISTORY */

    if (!non_null(HTTP_proxy) &&
	((p = getenv("HTTP_PROXY")) ||
	 (p = getenv("http_proxy")) || (p = getenv("HTTP_proxy"))))
	HTTP_proxy = p;
#ifdef USE_SSL
    if (!non_null(HTTPS_proxy) &&
	((p = getenv("HTTPS_PROXY")) ||
	 (p = getenv("https_proxy")) || (p = getenv("HTTPS_proxy"))))
	HTTPS_proxy = p;
    if (HTTPS_proxy == NULL && non_null(HTTP_proxy))
	HTTPS_proxy = HTTP_proxy;
#endif				/* USE_SSL */
#ifdef USE_GOPHER
    if (!non_null(GOPHER_proxy) &&
	((p = getenv("GOPHER_PROXY")) ||
	 (p = getenv("gopher_proxy")) || (p = getenv("GOPHER_proxy"))))
	GOPHER_proxy = p;
#endif				/* USE_GOPHER */
    if (!non_null(FTP_proxy) &&
	((p = getenv("FTP_PROXY")) ||
	 (p = getenv("ftp_proxy")) || (p = getenv("FTP_proxy"))))
	FTP_proxy = p;
    if (!non_null(NO_proxy) &&
	((p = getenv("NO_PROXY")) ||
	 (p = getenv("no_proxy")) || (p = getenv("NO_proxy"))))
	NO_proxy = p;

    if (!non_null(Editor) && (p = getenv("EDITOR")) != NULL)
	Editor = p;
    if (!non_null(Mailer) && (p = getenv("MAILER")) != NULL)
	Mailer = p;

    /* argument search 2 */
    i = 1;
    while (i < argc) {
	if (*argv[i] == '-') {
	    if (!strcmp("-t", argv[i])) {
		if (++i >= argc)
		    usage();
		if (atoi(argv[i]) > 0)
		    Tabstop = atoi(argv[i]);
	    }
	    else if (!strcmp("-r", argv[i]))
		ShowEffect = FALSE;
	    else if (!strcmp("-l", argv[i])) {
		if (++i >= argc)
		    usage();
		if (atoi(argv[i]) > 0)
		    PagerMax = atoi(argv[i]);
	    }
#ifdef JP_CHARSET
#ifndef DEBIAN			/* XXX: use -o kanjicode={S|J|E} */
	    else if (!strcmp("-s", argv[i]))
		DisplayCode = CODE_SJIS;
	    else if (!strcmp("-j", argv[i]))
		DisplayCode = CODE_JIS_n;
	    else if (!strcmp("-e", argv[i]))
		DisplayCode = CODE_EUC;
#endif
	    else if (!strncmp("-I", argv[i], 2)) {
		if (argv[i][2])
		    argv[i] += 2;
		else if (++i >= argc)
		    usage();
		c = str_to_code(argv[i]);
		switch (c) {
		case CODE_EUC:
		case CODE_SJIS:
		case CODE_INNER_EUC:
		    DocumentCode = c;
		    UseContentCharset = FALSE;
		    UseAutoDetect = FALSE;
		    break;
		default:
		    DocumentCode = '\0';
		    UseContentCharset = TRUE;
		    UseAutoDetect = TRUE;
		    break;
		}
	    }
	    else if (!strncmp("-O", argv[i], 2)) {
		if (argv[i][2])
		    argv[i] += 2;
		else if (++i >= argc)
		    usage();
		c = str_to_code(argv[i]);
		if (c != CODE_INNER_EUC && c != CODE_ASCII)
		    DisplayCode = c;
	    }
#endif				/* JP_CHARSET */
#ifndef KANJI_SYMBOLS
	    else if (!strcmp("-no-graph", argv[i]))
		no_graphic_char = TRUE;
#endif				/* not KANJI_SYMBOLS */
	    else if (!strcmp("-T", argv[i])) {
		if (++i >= argc)
		    usage();
		DefaultType = default_type = argv[i];
	    }
	    else if (!strcmp("-m", argv[i]))
		SearchHeader = search_header = TRUE;
	    else if (!strcmp("-v", argv[i]))
		visual_start = TRUE;
#ifdef USE_COLOR
	    else if (!strcmp("-M", argv[i]))
		useColor = FALSE;
#endif				/* USE_COLOR */
	    else if (!strcmp("-B", argv[i]))
		load_bookmark = TRUE;
	    else if (!strcmp("-bookmark", argv[i])) {
		if (++i >= argc)
		    usage();
		BookmarkFile = argv[i];
		if (BookmarkFile[0] != '~' && BookmarkFile[0] != '/') {
		    Str tmp = Strnew_charp(CurrentDir);
		    if (Strlastchar(tmp) != '/')
			Strcat_char(tmp, '/');
		    Strcat_charp(tmp, BookmarkFile);
		    BookmarkFile = cleanupName(tmp->ptr);
		}
	    }
	    else if (!strcmp("-F", argv[i]))
		RenderFrame = TRUE;
	    else if (!strcmp("-W", argv[i])) {
		if (WrapDefault)
		    WrapDefault = FALSE;
		else
		    WrapDefault = TRUE;
	    }
	    else if (!strcmp("-dump", argv[i]))
		w3m_dump = DUMP_BUFFER;
	    else if (!strcmp("-dump_source", argv[i]))
		w3m_dump = DUMP_SOURCE;
	    else if (!strcmp("-dump_head", argv[i]))
		w3m_dump = DUMP_HEAD;
	    else if (!strcmp("-dump_both", argv[i]))
		w3m_dump = (DUMP_HEAD | DUMP_SOURCE);
	    else if (!strcmp("-dump_extra", argv[i]))
		w3m_dump = (DUMP_HEAD | DUMP_SOURCE | DUMP_EXTRA);
	    else if (!strcmp("-halfdump", argv[i]))
		w3m_dump = DUMP_HALFDUMP;
	    else if (!strcmp("-halfload", argv[i])) {
		w3m_dump = 0;
		w3m_halfload = TRUE;
		DefaultType = default_type = "text/html";
	    }
	    else if (!strcmp("-backend", argv[i])) {
		w3m_backend = TRUE;
	    }
	    else if (!strcmp("-backend_batch", argv[i])) {
		w3m_backend = TRUE;
		if (++i >= argc)
		    usage();
		if (!backend_batch_commands)
		    backend_batch_commands = newTextList();
		pushText(backend_batch_commands, argv[i]);
	    }
	    else if (!strcmp("-cols", argv[i])) {
		if (++i >= argc)
		    usage();
		COLS = atoi(argv[i]);
	    }
	    else if (!strcmp("-ppc", argv[i])) {
		double ppc;
		if (++i >= argc)
		    usage();
		ppc = atof(argv[i]);
		if (ppc >= MINIMUM_PIXEL_PER_CHAR &&
		    ppc <= MAXIMUM_PIXEL_PER_CHAR) {
		    pixel_per_char = ppc;
		    set_pixel_per_char = TRUE;
		}
	    }
#ifdef USE_IMAGE
	    else if (!strcmp("-ppl", argv[i])) {
		double ppc;
		if (++i >= argc)
		    usage();
		ppc = atof(argv[i]);
		if (ppc >= MINIMUM_PIXEL_PER_CHAR &&
		    ppc <= MAXIMUM_PIXEL_PER_CHAR * 2) {
		    pixel_per_line = ppc;
		    set_pixel_per_line = TRUE;
		}
	    }
#endif
	    else if (!strcmp("-num", argv[i]))
		showLineNum = TRUE;
	    else if (!strcmp("-no-proxy", argv[i]))
		Do_not_use_proxy = TRUE;
	    else if (!strcmp("-post", argv[i])) {
		if (++i >= argc)
		    usage();
		post_file = argv[i];
	    }
	    else if (!strcmp("-header", argv[i])) {
		Str hs;
		if (++i >= argc)
		    usage();
		if ((hs = make_optional_header_string(argv[i])) != NULL) {
		    if (header_string == NULL)
			header_string = hs;
		    else
			Strcat(header_string, hs);
		}
		while (argv[i][0]) {
		    argv[i][0] = '\0';
		    argv[i]++;
		}
	    }
#ifdef USE_MOUSE
	    else if (!strcmp("-no-mouse", argv[i])) {
		use_mouse = FALSE;
	    }
#endif				/* USE_MOUSE */
#ifdef USE_COOKIE
	    else if (!strcmp("-no-cookie", argv[i])) {
		use_cookie = FALSE;
		accept_cookie = FALSE;
	    }
	    else if (!strcmp("-cookie", argv[i])) {
		use_cookie = TRUE;
		accept_cookie = TRUE;
	    }
#endif				/* USE_COOKIE */
	    else if (!strcmp("-pauth", argv[i])) {
		if (++i >= argc)
		    usage();
		proxy_auth_cookie = Strnew_m_charp("Basic ",
						   encodeB(argv[i])->ptr,
						   NULL);
		while (argv[i][0]) {
		    argv[i][0] = '\0';
		    argv[i]++;
		}
	    }
#ifdef DEBIAN
	    else if (!strcmp("-s", argv[i]))
#else
	    else if (!strcmp("-S", argv[i]))
#endif
		squeezeBlankLine = TRUE;
	    else if (!strcmp("-X", argv[i]))
		Do_not_use_ti_te = TRUE;
	    else if (!strcmp("-title", argv[i]))
		displayTitleTerm = getenv("TERM");
	    else if (!strncmp("-title=", argv[i], 7))
		displayTitleTerm = argv[i] + 7;
	    else if (!strcmp("-o", argv[i]) ||
		     !strcmp("-show-option", argv[i])) {
		if (!strcmp("-show-option", argv[i]) || ++i >= argc ||
		    !strcmp(argv[i], "?")) {
		    show_params(stdout);
		    exit(0);
		}
		if (!set_param_option(argv[i])) {
		    /* option set failed */
		    fprintf(stderr, "%s: bad option\n", argv[i]);
		    show_params_p = 1;
		    usage();
		}
	    }
	    else if (!strcmp("-dummy", argv[i])) {
		/* do nothing */
	    }
	    else if (!strcmp("-debug", argv[i]))
		w3m_debug = TRUE;
	    else {
		usage();
	    }
	}
	else if (*argv[i] == '+') {
	    line_str = argv[i] + 1;
	}
	else {
	    load_argv[load_argc++] = argv[i];
	}
	i++;
    }

    sync_with_option();
#ifdef USE_COOKIE
    initCookie();
#endif				/* USE_COOKIE */
    setLocalCookie();		/* setup cookie for local CGI */

#ifdef	__WATT32__
    if (w3m_debug)
	dbug_init();
    sock_init();
#endif

    FirstTab = NULL;
    LastTab = NULL;
    nTab = 0;
    CurrentTab = NULL;
    CurrentKey = -1;
    if (BookmarkFile == NULL)
	BookmarkFile = rcFile(BOOKMARK);

    if (!isatty(1) && !w3m_dump) {
	/* redirected output */
	w3m_dump = DUMP_BUFFER;
    }
    if (w3m_dump) {
	if (COLS == 0)
	    COLS = 80;
    }

#ifdef USE_BINMODE_STREAM
    setmode(fileno(stdout), O_BINARY);
#endif
    if (w3m_backend)
	backend();
    if (!w3m_dump) {
	fmInit();
#ifdef SIGWINCH
	signal(SIGWINCH, resize_hook);
#else				/* not SIGWINCH */
	setlinescols();
	setupscreen();
#endif				/* not SIGWINCH */
	initKeymap(TRUE);
#ifdef USE_MOUSE
	initMouseMenu();
#endif				/* MOUSE */
#ifdef USE_MENU
	initMenu();
#endif				/* MENU */
    }
#ifdef USE_IMAGE
    else if (w3m_halfdump && displayImage)
	activeImage = TRUE;
#endif
#ifdef SIGCHLD
    signal(SIGCHLD, sig_chld);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SigPipe);
#endif

    orig_GC_warn_proc = GC_set_warn_proc(wrap_GC_warn_proc);
    err_msg = Strnew();
    if (load_argc == 0) {
	/* no URL specified */
	if (!isatty(0)) {
	    redin = newFileStream(fdopen(dup(0), "rb"), (void (*)())pclose);
	    newbuf = openGeneralPagerBuffer(redin);
	    dup2(1, 0);
	}
	else if (load_bookmark) {
	    newbuf = loadGeneralFile(BookmarkFile, NULL, NO_REFERER, 0, NULL);
	    if (newbuf == NULL)
		Strcat_charp(err_msg, "w3m: Can't load bookmark.\n");
	}
	else if (visual_start) {
	    Str s_page;
	    s_page =
		Strnew_charp
		("<title>W3M startup page</title><center><b>Welcome to ");
	    Strcat_charp(s_page, "<a href='http://w3m.sourceforge.net/'>");
	    Strcat_m_charp(s_page,
			   "w3m</a>!<p><p>This is w3m version ",
			   w3m_version,
			   "<br>Written by <a href='mailto:aito@fw.ipsj.or.jp'>Akinori Ito</a>",
			   NULL);
#ifdef DEBIAN
	    Strcat_m_charp(s_page,
			   "<p>Debian package is maintained by <a href='mailto:ukai@debian.or.jp'>Fumitoshi UKAI</a>.",
			   "You can read <a href='file:///usr/share/doc/w3m/'>w3m documents on your local system</a>.",
			   NULL);
#endif				/* DEBIAN */
	    newbuf = loadHTMLString(s_page);
	    if (newbuf == NULL)
		Strcat_charp(err_msg, "w3m: Can't load string.\n");
	    else if (newbuf != NO_BUFFER)
		newbuf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
	}
	else if ((p = getenv("HTTP_HOME")) != NULL ||
		 (p = getenv("WWW_HOME")) != NULL) {
	    newbuf = loadGeneralFile(p, NULL, NO_REFERER, 0, NULL);
	    if (newbuf == NULL)
		Strcat(err_msg, Sprintf("w3m: Can't load %s.\n", p));
	    else if (newbuf != NO_BUFFER)
		pushHashHist(URLHist, parsedURL2Str(&newbuf->currentURL)->ptr);
	}
	else {
	    if (fmInitialized)
		fmTerm();
	    usage();
	}
	if (newbuf == NULL) {
	    if (fmInitialized)
		fmTerm();
	    if (err_msg->length)
		fprintf(stderr, "%s", err_msg->ptr);
	    w3m_exit(2);
	}
	i = -1;
    }
    else {
	i = 0;
    }
    for (; i < load_argc; i++) {
	if (i >= 0) {
	    SearchHeader = search_header;
	    DefaultType = default_type;
	    if (w3m_dump == DUMP_HEAD) {
		request = New(FormList);
		request->method = FORM_METHOD_HEAD;
		newbuf =
		    loadGeneralFile(load_argv[i], NULL, NO_REFERER, 0,
				    request);
	    }
	    else {
		if (post_file && i == 0) {
		    FILE *fp;
		    Str body;
		    if (!strcmp(post_file, "-"))
			fp = stdin;
		    else
			fp = fopen(post_file, "r");
		    if (fp == NULL) {
			Strcat(err_msg,
			       Sprintf("w3m: Can't open %s.\n", post_file));
			continue;
		    }
		    body = Strfgetall(fp);
		    if (fp != stdin)
			fclose(fp);
		    request =
			newFormList(NULL, "post", NULL, NULL, NULL, NULL,
				    NULL);
		    request->body = body->ptr;
		    request->boundary = NULL;
		    request->length = body->length;
		}
		else {
		    request = NULL;
		}
		newbuf =
		    loadGeneralFile(load_argv[i], NULL, NO_REFERER, 0,
				    request);
	    }
	    if (newbuf == NULL) {
		Strcat(err_msg,
		       Sprintf("w3m: Can't load %s.\n", load_argv[i]));
		continue;
	    }
	    else if (newbuf == NO_BUFFER)
		continue;
	    switch (newbuf->real_scheme) {
#ifdef USE_NNTP
	    case SCM_NNTP:
	    case SCM_NEWS:
#endif				/* USE_NNTP */
	    case SCM_MAILTO:
		break;
	    case SCM_LOCAL:
	    case SCM_LOCAL_CGI:
		unshiftHist(LoadHist, conv_from_system(load_argv[i]));
	    default:
		pushHashHist(URLHist, parsedURL2Str(&newbuf->currentURL)->ptr);
		break;
	    }
	}
	else if (newbuf == NO_BUFFER)
	    continue;
	newbuf->search_header = search_header;
	if (CurrentTab == NULL) {
	    FirstTab = LastTab = CurrentTab = newTab();
	    nTab = 1;
	    calcTabPos();
	    Firstbuf = Currentbuf = newbuf;
	}
	else {
	    Currentbuf->nextBuffer = newbuf;
	    Currentbuf = newbuf;
	}
	if (w3m_dump) {
	    if (w3m_dump & DUMP_EXTRA)
		dump_extra(Currentbuf);
	    if (w3m_dump & DUMP_HEAD)
		dump_head(Currentbuf);
	    if (w3m_dump & DUMP_SOURCE)
		dump_source(Currentbuf);
	    if (w3m_dump == DUMP_BUFFER) {
		if (Currentbuf->frameset != NULL && RenderFrame)
		    rFrame();
		saveBuffer(Currentbuf, stdout);
	    }
	}
	else {
	    if (Currentbuf->frameset != NULL && RenderFrame)
		rFrame();
	    Currentbuf = newbuf;
#ifdef USE_BUFINFO
	    saveBufferInfo();
#endif
	}
    }
    if (w3m_dump) {
#ifdef USE_COOKIE
	save_cookies();
#endif				/* USE_COOKIE */
	w3m_exit(0);
    }

    if (add_download_list) {
	add_download_list = FALSE;
	if (!FirstTab) {
	    FirstTab = LastTab = CurrentTab = newTab();
	    nTab = 1;
	    calcTabPos();
	}
	if (!Firstbuf || Firstbuf == NO_BUFFER) {
	    Firstbuf = Currentbuf = newBuffer(INIT_BUFFER_WIDTH);
	    Currentbuf->bufferprop = BP_INTERNAL | BP_NO_URL;
	    Currentbuf->buffername = DOWNLOAD_LIST_TITLE;
	}
	else
	    Currentbuf = Firstbuf;
	ldDL();
    }
    if (!FirstTab || !Firstbuf || Firstbuf == NO_BUFFER) {
	if (newbuf == NO_BUFFER) {
	    if (fmInitialized)
		inputChar("Hit any key to quit w3m:");
	}
	if (fmInitialized)
	    fmTerm();
	if (err_msg->length)
	    fprintf(stderr, "%s", err_msg->ptr);
	if (newbuf == NO_BUFFER) {
#ifdef USE_COOKIE
	    save_cookies();
#endif				/* USE_COOKIE */
	    if (!err_msg->length)
		w3m_exit(0);
	}
	w3m_exit(2);
    }
    if (err_msg->length)
	disp_message_nsec(err_msg->ptr, FALSE, 1, TRUE, FALSE);

    SearchHeader = FALSE;
    DefaultType = NULL;
#ifdef JP_CHARSET
    UseContentCharset = TRUE;
    UseAutoDetect = TRUE;
#endif

    Currentbuf = Firstbuf;
    displayBuffer(Currentbuf, B_NORMAL);
    if (line_str) {
	_goLine(line_str);
    }
    onA();
    for (;;) {
	if (Currentbuf->submit) {
	    Anchor *a = Currentbuf->submit;
	    Currentbuf->submit = NULL;
	    gotoLine(Currentbuf, a->start.line);
	    Currentbuf->pos = a->start.pos;
	    _followForm(TRUE);
	}
	/* event processing */
	if (n_event_queue > 0) {
	    for (i = 0; i < n_event_queue; i++) {
		CurrentKey = -1;
		CurrentKeyData = eventQueue[i].user_data;
		CurrentCmdData = NULL;
		w3mFuncList[eventQueue[i].cmd].func();
	    }
	    n_event_queue = 0;
	}
	CurrentKeyData = NULL;
	/* get keypress event */
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_active();
#endif				/* USE_MOUSE */
#ifdef USE_ALARM
	if (Currentbuf->bufferprop & BP_RELOAD)
	    setAlarmEvent(1, AL_IMPLICIT, FUNCNAME_reload, NULL);
	if (CurrentAlarm.status & AL_IMPLICIT) {
	    CurrentAlarm.buffer = Currentbuf;
	    CurrentAlarm.status = AL_IMPLICIT_DONE
		| (CurrentAlarm.status & AL_ONCE);
	}
	else if (CurrentAlarm.status & AL_IMPLICIT_DONE &&
		 CurrentAlarm.buffer != Currentbuf) {
	    setAlarmEvent(0, AL_RESTORE, FUNCNAME_nulcmd, NULL);
	}
	if (CurrentAlarm.sec > 0) {
	    signal(SIGALRM, SigAlarm);
	    alarm(CurrentAlarm.sec);
	}
#endif
#ifdef SIGWINCH
	if (need_resize_screen) {
	    need_resize_screen = FALSE;
	    resize_screen();
	}
	signal(SIGWINCH, resize_handler);
#endif
#ifdef USE_IMAGE
	if (activeImage && displayImage)
	    loadImage(IMG_FLAG_NEXT);
#endif
	c = getch();
#ifdef USE_IMAGE
	if (activeImage && displayImage)
	    loadImage(IMG_FLAG_START);
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, resize_hook);
#endif
#ifdef USE_ALARM
	if (CurrentAlarm.sec > 0) {
	    alarm(0);
	}
#endif
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_inactive();
#endif				/* USE_MOUSE */
	if (IS_ASCII(c)) {	/* Ascii */
	    if (((prec_num && c == '0') || '1' <= c) && (c <= '9')) {
		prec_num = prec_num * 10 + (int)(c - '0');
		if (prec_num > PREC_LIMIT)
		    prec_num = PREC_LIMIT;
	    }
	    else {
		set_buffer_environ(Currentbuf);
		keyPressEventProc((int)c);
		prec_num = 0;
		if (add_download_list) {
		    add_download_list = FALSE;
		    ldDL();
		}
	    }
	}
	prev_key = CurrentKey;
	CurrentKey = -1;
    }
}

static void
keyPressEventProc(int c)
{
    CurrentKey = c;
    w3mFuncList[(int)GlobalKeymap[c]].func();
    onA();
}

void
pushEvent(int event, void *user_data)
{
    if (n_event_queue < N_EVENT_QUEUE) {
	eventQueue[n_event_queue].cmd = event;
	eventQueue[n_event_queue].user_data = user_data;
	n_event_queue++;
    }
}

static void
dump_source(Buffer *buf)
{
    FILE *f;
    char c;
    if (buf->sourcefile == NULL)
	return;
    f = fopen(buf->sourcefile, "r");
    if (f == NULL)
	return;
    while (c = fgetc(f), !feof(f)) {
	putchar(c);
    }
    fclose(f);
}

static void
dump_head(Buffer *buf)
{
    TextListItem *ti;

    if (buf->document_header == NULL) {
	if (w3m_dump & DUMP_EXTRA)
	    printf("\n");
	return;
    }
    for (ti = buf->document_header->first; ti; ti = ti->next) {
	printf("%s", ti->ptr);
    }
    puts("");
}

static void
dump_extra(Buffer *buf)
{
    printf("W3m-current-url: %s\n", parsedURL2Str(&buf->currentURL)->ptr);
    if (buf->baseURL)
	printf("W3m-base-url: %s\n", parsedURL2Str(buf->baseURL)->ptr);
#ifdef JP_CHARSET
    printf("W3m-document-charset: %s\n", code_to_str(buf->document_code));
#endif
#ifdef USE_SSL
    if (buf->ssl_certificate) {
	Str tmp = Strnew();
	char *p;
	for (p = buf->ssl_certificate; *p; p++) {
	    Strcat_char(tmp, *p);
	    if (*p == '\n') {
		for (; *(p + 1) == '\n'; p++) ;
		if (*(p + 1))
		    Strcat_char(tmp, '\t');
	    }
	}
	if (Strlastchar(tmp) != '\n')
	    Strcat_char(tmp, '\n');
	printf("W3m-ssl-certificate: %s", tmp->ptr);
    }
#endif
}

void
nulcmd(void)
{				/* do nothing */
}

#ifdef __EMX__
void
pcmap(void)
{
    w3mFuncList[(int)PcKeymap[(int)getch()]].func();
}
#else				/* not __EMX__ */
void
pcmap(void)
{
}
#endif

void
escmap(void)
{
    char c;
    c = getch();
    if (IS_ASCII(c)) {
	CurrentKey = K_ESC | c;
	w3mFuncList[(int)EscKeymap[(int)c]].func();
    }
}

void
escbmap(void)
{
    char c;
    c = getch();

    if (IS_DIGIT(c))
	escdmap(c);
    else if (IS_ASCII(c)) {
	CurrentKey = K_ESCB | c;
	w3mFuncList[(int)EscBKeymap[(int)c]].func();
    }
}

void
escdmap(char c)
{
    int d;

    d = (int)c - (int)'0';
    c = getch();
    if (IS_DIGIT(c)) {
	d = d * 10 + (int)c - (int)'0';
	c = getch();
    }
    if (c == '~') {
	CurrentKey = K_ESCD | d;
	w3mFuncList[(int)EscDKeymap[d]].func();
    }
}

void
tmpClearBuffer(Buffer *buf)
{
    if (buf->pagerSource == NULL && writeBufferCache(buf) == 0) {
	buf->firstLine = NULL;
	buf->topLine = NULL;
	buf->currentLine = NULL;
	buf->lastLine = NULL;
    }
}

static Str currentURL(void);

void
saveBufferInfo()
{
    FILE *fp;

    if (w3m_dump)
	return;
    if ((fp = fopen(rcFile("bufinfo"), "w")) == NULL) {
	return;
    }
    fprintf(fp, "%s\n", currentURL()->ptr);
    fclose(fp);
}

static void
pushBuffer(Buffer *buf)
{
    Buffer *b;

#ifdef USE_IMAGE
    deleteImage(Currentbuf);
#endif
    if (clear_buffer)
	tmpClearBuffer(Currentbuf);
    if (Firstbuf == Currentbuf) {
	buf->nextBuffer = Firstbuf;
	Firstbuf = Currentbuf = buf;
    }
    else if ((b = prevBuffer(Firstbuf, Currentbuf)) != NULL) {
	b->nextBuffer = buf;
	buf->nextBuffer = Currentbuf;
	Currentbuf = buf;
    }
#ifdef USE_BUFINFO
    saveBufferInfo();
#endif

}

static void
delBuffer(Buffer *buf)
{
    if (buf == NULL)
	return;
    if (Currentbuf == buf)
	Currentbuf = buf->nextBuffer;
    Firstbuf = deleteBuffer(Firstbuf, buf);
    if (!Currentbuf)
	Currentbuf = Firstbuf;
}

static void
repBuffer(Buffer *oldbuf, Buffer *buf)
{
    Firstbuf = replaceBuffer(Firstbuf, oldbuf, buf);
    Currentbuf = buf;
}


MySignalHandler
intTrap(SIGNAL_ARG)
{				/* Interrupt catcher */
    LONGJMP(IntReturn, 0);
    SIGNAL_RETURN;
}

#ifdef SIGWINCH
static MySignalHandler
resize_hook(SIGNAL_ARG)
{
    need_resize_screen = TRUE;
    signal(SIGWINCH, resize_hook);
    SIGNAL_RETURN;
}

static MySignalHandler
resize_handler(SIGNAL_ARG)
{
    resize_screen();
    signal(SIGWINCH, resize_handler);
    SIGNAL_RETURN;
}

static void
resize_screen(void)
{
    setlinescols();
    setupscreen();
    if (Currentbuf)
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
}
#endif				/* SIGWINCH */

#ifdef SIGPIPE
static MySignalHandler
SigPipe(SIGNAL_ARG)
{
#ifdef USE_MIGEMO
    init_migemo();
#endif
    signal(SIGPIPE, SigPipe);
    SIGNAL_RETURN;
}
#endif

/* 
 * Command functions: These functions are called with a keystroke.
 */

#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

static void
nscroll(int n, int mode)
{
    Line *curtop = Currentbuf->topLine;
    int lnum, tlnum, llnum, diff_n;

    if (Currentbuf->firstLine == NULL)
	return;
    lnum = Currentbuf->currentLine->linenumber;
    Currentbuf->topLine = lineSkip(Currentbuf, curtop, n, FALSE);
    if (Currentbuf->topLine == curtop) {
	lnum += n;
	if (lnum < Currentbuf->topLine->linenumber)
	    lnum = Currentbuf->topLine->linenumber;
	else if (lnum > Currentbuf->lastLine->linenumber)
	    lnum = Currentbuf->lastLine->linenumber;
    }
    else {
	tlnum = Currentbuf->topLine->linenumber;
	llnum = Currentbuf->topLine->linenumber + Currentbuf->LINES - 1;
#ifdef NEXTPAGE_TOPLINE
	if (nextpage_topline)
	    diff_n = 0;
	else
#endif
	    diff_n = n - (tlnum - curtop->linenumber);
	if (lnum < tlnum)
	    lnum = tlnum + diff_n;
	if (lnum > llnum)
	    lnum = llnum + diff_n;
    }
    gotoLine(Currentbuf, lnum);
    arrangeLine(Currentbuf);
    displayBuffer(Currentbuf, mode);
}

/* Move page forward */
void
pgFore(void)
{
#ifdef VI_PREC_NUM
    if (vi_prec_num)
	nscroll(searchKeyNum() * (Currentbuf->LINES - 1), B_NORMAL);
    else
#endif
	nscroll(prec_num ? searchKeyNum() : searchKeyNum()
		* (Currentbuf->LINES - 1), prec_num ? B_SCROLL : B_NORMAL);
}

/* Move page backward */
void
pgBack(void)
{
#ifdef VI_PREC_NUM
    if (vi_prec_num)
	nscroll(-searchKeyNum() * (Currentbuf->LINES - 1), B_NORMAL);
    else
#endif
	nscroll(-(prec_num ? searchKeyNum() : searchKeyNum()
		  * (Currentbuf->LINES - 1)), prec_num ? B_SCROLL : B_NORMAL);
}

/* 1 line up */
void
lup1(void)
{
    nscroll(searchKeyNum(), B_SCROLL);
}

/* 1 line down */
void
ldown1(void)
{
    nscroll(-searchKeyNum(), B_SCROLL);
}

/* move cursor position to the center of screen */
void
ctrCsrV(void)
{
    int offsety;
    if (Currentbuf->firstLine == NULL)
	return;
    offsety = Currentbuf->LINES / 2 - Currentbuf->cursorY;
    if (offsety != 0) {
#if 0
	Currentbuf->currentLine = lineSkip(Currentbuf,
					   Currentbuf->currentLine, offsety,
					   FALSE);
#endif
	Currentbuf->topLine =
	    lineSkip(Currentbuf, Currentbuf->topLine, -offsety, FALSE);
	arrangeLine(Currentbuf);
	displayBuffer(Currentbuf, B_NORMAL);
    }
}

void
ctrCsrH(void)
{
    int offsetx;
    if (Currentbuf->firstLine == NULL)
	return;
    offsetx = Currentbuf->cursorX - Currentbuf->COLS / 2;
    if (offsetx != 0) {
	columnSkip(Currentbuf, offsetx);
	arrangeCursor(Currentbuf);
	displayBuffer(Currentbuf, B_NORMAL);
    }
}

/* Redraw screen */
void
rdrwSc(void)
{
    clear();
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
clear_mark(Line *l)
{
    short pos;
    if (!l)
	return;
    for (pos = 0; pos < l->len; pos++)
	l->propBuf[pos] &= ~PE_MARK;
}

/* search by regular expression */
static int
srchcore(char *str, int (*func) (Buffer *, char *))
{
    MySignalHandler(*prevtrap) ();
    volatile int i, result = SR_NOTFOUND;

    if (str != NULL && str != SearchString)
	SearchString = str;
    if (SearchString == NULL || *SearchString == '\0')
	return SR_NOTFOUND;

    prevtrap = signal(SIGINT, intTrap);
    crmode();
    if (SETJMP(IntReturn) == 0)
	for (i = 0; i < PREC_NUM; i++)
	    result = func(Currentbuf, SearchString);
    signal(SIGINT, prevtrap);
    term_raw();
    return result;
}

void
disp_srchresult(int result, char *prompt, char *str)
{
    if (str == NULL)
	str = "";
    if (result & SR_NOTFOUND)
	disp_message(Sprintf("Not found: %s", str)->ptr, FALSE);
    else if (result & SR_WRAPPED)
	disp_message(Sprintf("Search wrapped: %s", str)->ptr, FALSE);
    else if (show_srch_str)
	disp_message(Sprintf("%s%s", prompt, str)->ptr, FALSE);
}

static int
dispincsrch(int ch, Str buf, Lineprop *prop)
{
    static Buffer sbuf;
    static Line *currentLine;
    static short pos;
    char *str;
    int do_next_search = FALSE;

    if (ch == 0 && buf == NULL) {
	SAVE_BUFPOSITION(&sbuf);	/* search starting point */
	currentLine = sbuf.currentLine;
	pos = sbuf.pos;
	return -1;
    }

    str = buf->ptr;
    switch (ch) {
    case 022:			/* C-r */
	searchRoutine = backwardSearch;
	do_next_search = TRUE;
	break;
    case 023:			/* C-s */
	searchRoutine = forwardSearch;
	do_next_search = TRUE;
	break;

#ifdef USE_MIGEMO
    case 034:
	migemo_active = -migemo_active;
	goto done;
#endif

    default:
	if (ch >= 0)
	    return ch;		/* use InputKeymap */
    }

    if (do_next_search) {
	if (*str) {
	    SAVE_BUFPOSITION(&sbuf);
	    srchcore(str, searchRoutine);
	    arrangeCursor(Currentbuf);
	    if (Currentbuf->currentLine == currentLine
		&& Currentbuf->pos == pos) {
		SAVE_BUFPOSITION(&sbuf);
		srchcore(str, searchRoutine);
		arrangeCursor(Currentbuf);
	    }
	    displayBuffer(Currentbuf, B_FORCE_REDRAW);
	    clear_mark(Currentbuf->currentLine);
	    return -1;
	}
	else
	    return 020;		/* _prev completion for C-s C-s */
    }
    else if (*str) {
	RESTORE_BUFPOSITION(&sbuf);
	arrangeCursor(Currentbuf);
	srchcore(str, searchRoutine);
	arrangeCursor(Currentbuf);
	currentLine = Currentbuf->currentLine;
	pos = Currentbuf->pos;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    clear_mark(Currentbuf->currentLine);
#ifdef USE_MIGEMO
  done:
    while (*str++ != '\0') {
	if (migemo_active > 0)
	    *prop++ |= PE_UNDER;
	else
	    *prop++ &= ~PE_UNDER;
    }
#endif
    return -1;
}

void
isrch(int (*func) (Buffer *, char *), char *prompt)
{
    char *str;
    Buffer sbuf;
    SAVE_BUFPOSITION(&sbuf);
    dispincsrch(0, NULL, NULL);	/* initialize incremental search state */

    searchRoutine = func;
    str = inputLineHistSearch(prompt, NULL, IN_STRING, TextHist, dispincsrch);
    if (str == NULL) {
	RESTORE_BUFPOSITION(&sbuf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    onA();
}

void
srch(int (*func) (Buffer *, char *), char *prompt)
{
    char *str;
    int result;

    str = inputStrHist(prompt, NULL, TextHist);
    if (str != NULL && *str == '\0')
	str = SearchString;
    if (str == NULL)
	return;
    result = srchcore(str, func);
    if (result & SR_FOUND)
	clear_mark(Currentbuf->currentLine);
    displayBuffer(Currentbuf, B_NORMAL);
    onA();
    disp_srchresult(result, prompt, str);
    searchRoutine = func;
}

/* Search regular expression forward */

void
srchfor(void)
{
    srch(forwardSearch, "Forward: ");
}

void
isrchfor(void)
{
    isrch(forwardSearch, "I-search: ");
}

/* Search regular expression backward */

void
srchbak(void)
{
    srch(backwardSearch, "Backward: ");
}

void
isrchbak(void)
{
    isrch(backwardSearch, "I-search backward: ");
}

static void
srch_nxtprv(int reverse)
{
    int result;
    /* *INDENT-OFF* */
    static int (*routine[2]) (Buffer *, char *) = {
	forwardSearch, backwardSearch
    };
    /* *INDENT-ON* */

    if (searchRoutine == NULL) {
	disp_message("No previous regular expression", TRUE);
	return;
    }
    if (reverse != 0)
	reverse = 1;
    if (searchRoutine == backwardSearch)
	reverse ^= 1;
    result = srchcore(SearchString, routine[reverse]);
    if (result & SR_FOUND)
	clear_mark(Currentbuf->currentLine);
    displayBuffer(Currentbuf, B_NORMAL);
    onA();
    disp_srchresult(result, (reverse ? "Backward: " : "Forward: "),
		    SearchString);
}

/* Search next matching */
void
srchnxt(void)
{
    srch_nxtprv(0);
}

/* Search previous matching */
void
srchprv(void)
{
    srch_nxtprv(1);
}

static void
shiftvisualpos(Buffer *buf, int shift)
{
    buf->visualpos -= shift;
    if (buf->visualpos >= buf->COLS)
	buf->visualpos = buf->COLS - 1;
    else if (buf->visualpos < 0)
	buf->visualpos = 0;
    arrangeLine(buf);
    if (buf->visualpos == -shift && buf->cursorX == 0)
	buf->visualpos = 0;
}

/* Shift screen left */
void
shiftl(void)
{
    int column;

    if (Currentbuf->firstLine == NULL)
	return;
    column = Currentbuf->currentColumn;
    columnSkip(Currentbuf, searchKeyNum() * (-Currentbuf->COLS + 1) + 1);
    shiftvisualpos(Currentbuf, Currentbuf->currentColumn - column);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* Shift screen right */
void
shiftr(void)
{
    int column;

    if (Currentbuf->firstLine == NULL)
	return;
    column = Currentbuf->currentColumn;
    columnSkip(Currentbuf, searchKeyNum() * (Currentbuf->COLS - 1) - 1);
    shiftvisualpos(Currentbuf, Currentbuf->currentColumn - column);
    displayBuffer(Currentbuf, B_NORMAL);
}

void
col1R(void)
{
    Buffer *buf = Currentbuf;
    Line *l = buf->currentLine;
    int j, column, n = searchKeyNum();

    if (l == NULL)
	return;
    for (j = 0; j < n; j++) {
	column = buf->currentColumn;
	columnSkip(Currentbuf, 1);
	if (column == buf->currentColumn)
	    break;
	shiftvisualpos(Currentbuf, 1);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

void
col1L(void)
{
    Buffer *buf = Currentbuf;
    Line *l = buf->currentLine;
    int j, n = searchKeyNum();

    if (l == NULL)
	return;
    for (j = 0; j < n; j++) {
	if (buf->currentColumn == 0)
	    break;
	columnSkip(Currentbuf, -1);
	shiftvisualpos(Currentbuf, -1);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

void
setEnv(void)
{
    char *env;
    char *var, *value;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    env = searchKeyData();
    if (env == NULL || *env == '\0' || strchr(env, '=') == NULL) {
	if (env != NULL && *env != '\0')
	    env = Sprintf("%s=", env)->ptr;
	env = inputStrHist("Set environ: ", env, TextHist);
	if (env == NULL || *env == '\0') {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    if ((value = strchr(env, '=')) != NULL && value > env) {
	var = allocStr(env, value - env);
	value++;
	set_environ(var, value);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

void
pipeBuf(void)
{
    Buffer *buf;
    char *cmd, *tmpf;
    FILE *f;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    cmd = searchKeyData();
    if (cmd == NULL || *cmd == '\0') {
	cmd = inputLineHist("Pipe buffer to: ", "", IN_COMMAND, ShellHist);
	if (cmd == NULL || *cmd == '\0') {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
    f = fopen(tmpf, "w");
    if (f == NULL) {
	disp_message(Sprintf("Can't save buffer to %s", cmd)->ptr, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f);
    fclose(f);
    pushText(fileToDelete, tmpf);
    buf = getpipe(myExtCommand(cmd, tmpf, TRUE)->ptr);
    if (buf == NULL) {
	disp_message("Execution failed", FALSE);
    }
    else {
	buf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
	if (buf->type == NULL)
	    buf->type = "text/plain";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Execute shell command and read output ac pipe. */
void
pipesh(void)
{
    Buffer *buf;
    char *cmd;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    cmd = searchKeyData();
    if (cmd == NULL || *cmd == '\0') {
	cmd = inputLineHist("(read shell[pipe])!", "", IN_COMMAND, ShellHist);
    }
    if (cmd != NULL)
	cmd = conv_to_system(cmd);
    if (cmd == NULL || *cmd == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    buf = getpipe(cmd);
    if (buf == NULL) {
	disp_message("Execution failed", FALSE);
    }
    else {
	buf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
	if (buf->type == NULL)
	    buf->type = "text/plain";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Execute shell command and load entire output to buffer */
void
readsh(void)
{
    Buffer *buf;
    MySignalHandler(*prevtrap) ();
    char *cmd;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    cmd = searchKeyData();
    if (cmd == NULL || *cmd == '\0') {
	cmd = inputLineHist("(read shell)!", "", IN_COMMAND, ShellHist);
    }
    if (cmd != NULL)
	cmd = conv_to_system(cmd);
    if (cmd == NULL || *cmd == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    prevtrap = signal(SIGINT, intTrap);
    crmode();
    buf = getshell(cmd);
    signal(SIGINT, prevtrap);
    term_raw();
    if (buf == NULL) {
	disp_message("Execution failed", FALSE);
    }
    else {
	buf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
	if (buf->type == NULL)
	    buf->type = "text/plain";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Execute shell command */
void
execsh(void)
{
    char *cmd;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    cmd = searchKeyData();
    if (cmd == NULL || *cmd == '\0') {
	cmd = inputLineHist("(exec shell)!", "", IN_COMMAND, ShellHist);
    }
    if (cmd != NULL)
	cmd = conv_to_system(cmd);
    if (cmd != NULL && *cmd != '\0') {
	fmTerm();
	system(cmd);
	printf("\n[Hit any key]");
	fflush(stdout);
	fmInit();
	getch();
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Load file */
void
ldfile(void)
{
    char *fn;

    fn = searchKeyData();
    if (fn == NULL || *fn == '\0') {
	fn = inputFilenameHist("(Load)Filename? ", NULL, LoadHist);
    }
    if (fn != NULL)
	fn = conv_to_system(fn);
    if (fn == NULL || *fn == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    cmd_loadfile(fn);
}

/* Load help file */
void
ldhelp(void)
{
#ifdef USE_HELP_CGI
    char *lang;
    int n;

    lang = AcceptLang;
    n = strcspn(lang, ";, \t");
    cmd_loadURL(Sprintf("file:///$LIB/" HELP_CGI CGI_EXTENSION
			"?version=%s&lang=%s",
			Str_form_quote(Strnew_charp(w3m_version))->ptr,
			Str_form_quote(Strnew_charp_n(lang, n))->ptr)->ptr,
		NULL, NO_REFERER);
#else
    cmd_loadURL(helpFile(HELP_FILE), NULL, NO_REFERER);
#endif
}

static void
cmd_loadfile(char *fn)
{
    Buffer *buf;

    buf = loadGeneralFile(file_to_url(fn), NULL, NO_REFERER, 0, NULL);
    if (buf == NULL) {
	char *emsg = Sprintf("%s not found", conv_from_system(fn))->ptr;
	disp_err_message(emsg, FALSE);
    }
    else if (buf != NO_BUFFER) {
	pushBuffer(buf);
	if (RenderFrame && Currentbuf->frameset != NULL)
	    rFrame();
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

/* Move cursor left */
static void
_movL(int n)
{
    int i, m = searchKeyNum();
    if (Currentbuf->firstLine == NULL)
	return;
    for (i = 0; i < m; i++)
	cursorLeft(Currentbuf, n);
    displayBuffer(Currentbuf, B_NORMAL);
}

void
movL(void)
{
    _movL(Currentbuf->COLS / 2);
}

void
movL1(void)
{
    _movL(1);
}

/* Move cursor downward */
static void
_movD(int n)
{
    int i, m = searchKeyNum();
    if (Currentbuf->firstLine == NULL)
	return;
    for (i = 0; i < m; i++)
	cursorDown(Currentbuf, n);
    displayBuffer(Currentbuf, B_NORMAL);
}

void
movD(void)
{
    _movD((Currentbuf->LINES + 1) / 2);
}

void
movD1(void)
{
    _movD(1);
}

/* move cursor upward */
static void
_movU(int n)
{
    int i, m = searchKeyNum();
    if (Currentbuf->firstLine == NULL)
	return;
    for (i = 0; i < m; i++)
	cursorUp(Currentbuf, n);
    displayBuffer(Currentbuf, B_NORMAL);
}

void
movU(void)
{
    _movU((Currentbuf->LINES + 1) / 2);
}

void
movU1(void)
{
    _movU(1);
}

/* Move cursor right */
static void
_movR(int n)
{
    int i, m = searchKeyNum();
    if (Currentbuf->firstLine == NULL)
	return;
    for (i = 0; i < m; i++)
	cursorRight(Currentbuf, n);
    displayBuffer(Currentbuf, B_NORMAL);
}

void
movR(void)
{
    _movR(Currentbuf->COLS / 2);
}

void
movR1(void)
{
    _movR(1);
}

/* movLW, movRW */
/* 
 * From: Takashi Nishimoto <g96p0935@mse.waseda.ac.jp> Date: Mon, 14 Jun
 * 1999 09:29:56 +0900 */

#define IS_WORD_CHAR(c,p) (IS_ALNUM(c) && CharType(p) == PC_ASCII)

static int
prev_nonnull_line(Line *line)
{
    Line *l;

    for (l = line; l != NULL && l->len == 0; l = l->prev) ;
    if (l == NULL || l->len == 0)
	return -1;

    Currentbuf->currentLine = l;
    if (l != line)
	Currentbuf->pos = Currentbuf->currentLine->len;
    return 0;
}

void
movLW(void)
{
    char *lb;
    Lineprop *pb;
    Line *pline;
    int ppos;
    int i, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;

    for (i = 0; i < n; i++) {
	pline = Currentbuf->currentLine;
	ppos = Currentbuf->pos;

	if (prev_nonnull_line(Currentbuf->currentLine) < 0)
	    goto end;

	while (1) {
	    lb = Currentbuf->currentLine->lineBuf;
	    pb = Currentbuf->currentLine->propBuf;
	    while (Currentbuf->pos > 0 &&
		   !IS_WORD_CHAR(lb[Currentbuf->pos - 1],
				 pb[Currentbuf->pos - 1])) {
		Currentbuf->pos--;
	    }
	    if (Currentbuf->pos > 0)
		break;
	    if (prev_nonnull_line(Currentbuf->currentLine->prev) < 0) {
		Currentbuf->currentLine = pline;
		Currentbuf->pos = ppos;
		goto end;
	    }
	    Currentbuf->pos = Currentbuf->currentLine->len;
	}

	lb = Currentbuf->currentLine->lineBuf;
	pb = Currentbuf->currentLine->propBuf;
	while (Currentbuf->pos > 0 &&
	       IS_WORD_CHAR(lb[Currentbuf->pos - 1],
			    pb[Currentbuf->pos - 1])) {
	    Currentbuf->pos--;
	}
    }
  end:
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

static int
next_nonnull_line(Line *line)
{
    Line *l;

    for (l = line; l != NULL && l->len == 0; l = l->next) ;

    if (l == NULL || l->len == 0)
	return -1;

    Currentbuf->currentLine = l;
    if (l != line)
	Currentbuf->pos = 0;
    return 0;
}

void
movRW(void)
{
    char *lb;
    Lineprop *pb;
    Line *pline;
    int ppos;
    int i, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;

    for (i = 0; i < n; i++) {
	pline = Currentbuf->currentLine;
	ppos = Currentbuf->pos;

	if (next_nonnull_line(Currentbuf->currentLine) < 0)
	    goto end;

	lb = Currentbuf->currentLine->lineBuf;
	pb = Currentbuf->currentLine->propBuf;

	while (lb[Currentbuf->pos] &&
	       IS_WORD_CHAR(lb[Currentbuf->pos], pb[Currentbuf->pos]))
	    Currentbuf->pos++;

	while (1) {
	    while (lb[Currentbuf->pos] &&
		   !IS_WORD_CHAR(lb[Currentbuf->pos], pb[Currentbuf->pos]))
		Currentbuf->pos++;
	    if (lb[Currentbuf->pos])
		break;
	    if (next_nonnull_line(Currentbuf->currentLine->next) < 0) {
		Currentbuf->currentLine = pline;
		Currentbuf->pos = ppos;
		goto end;
	    }
	    Currentbuf->pos = 0;
	    lb = Currentbuf->currentLine->lineBuf;
	    pb = Currentbuf->currentLine->propBuf;
	}
    }
  end:
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

static void
_quitfm(int confirm)
{
    char *ans = "y";

    if (checkDownloadList())
	ans = inputChar("Download process retains. "
			"Do you want to exit w3m? (y/n)");
    else if (confirm)
	ans = inputChar("Do you want to exit w3m? (y/n)");
    if (!(ans && tolower(*ans) == 'y')) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }

    term_title("");		/* XXX */
#ifdef USE_IMAGE
    if (activeImage)
	termImage();
#endif
    fmTerm();
#ifdef USE_COOKIE
    save_cookies();
#endif				/* USE_COOKIE */
#ifdef USE_HISTORY
    if (SaveURLHist)
	saveHistory(URLHist, URLHistSize);
#endif				/* USE_HISTORY */
    w3m_exit(0);
}

/* Quit */
void
quitfm(void)
{
    _quitfm(FALSE);
}

/* Question and Quit */
void
qquitfm(void)
{
    _quitfm(confirm_on_quit);
}

/* Select buffer */
void
selBuf(void)
{
    Buffer *buf;
    int ok;
    char cmd;

    ok = FALSE;
    do {
	buf = selectBuffer(Firstbuf, Currentbuf, &cmd);
	switch (cmd) {
	case 'B':
	    ok = TRUE;
	    break;
	case '\n':
	case ' ':
	    Currentbuf = buf;
	    ok = TRUE;
	    break;
	case 'D':
	    delBuffer(buf);
	    if (Firstbuf == NULL) {
		/* No more buffer */
		Firstbuf = nullBuffer();
		Currentbuf = Firstbuf;
	    }
	    break;
	case 'q':
	    qquitfm();
	    break;
	case 'Q':
	    quitfm();
	    break;
	}
    } while (!ok);

    for (buf = Firstbuf; buf != NULL; buf = buf->nextBuffer) {
	if (buf == Currentbuf)
	    continue;
#ifdef USE_IMAGE
	deleteImage(buf);
#endif
	if (clear_buffer)
	    tmpClearBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Suspend (on BSD), or run interactive shell (on SysV) */
void
susp(void)
{
#ifndef SIGSTOP
    char *shell;
#endif				/* not SIGSTOP */
    move(LASTLINE, 0);
    clrtoeolx();
    refresh();
    fmTerm();
#ifndef SIGSTOP
    shell = getenv("SHELL");
    if (shell == NULL)
	shell = "/bin/sh";
    system(shell);
#else				/* SIGSTOP */
    kill((pid_t) 0, SIGSTOP);
#endif				/* SIGSTOP */
    fmInit();
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Go to specified line */
static void
_goLine(char *l)
{
    if (l == NULL || *l == '\0' || Currentbuf->currentLine == NULL) {
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    Currentbuf->pos = 0;
    if (((*l == '^') || (*l == '$')) && prec_num) {
	gotoRealLine(Currentbuf, prec_num);
    }
    else if (*l == '^') {
	Currentbuf->topLine = Currentbuf->currentLine = Currentbuf->firstLine;
    }
    else if (*l == '$') {
	Currentbuf->topLine =
	    lineSkip(Currentbuf, Currentbuf->lastLine,
		     -(Currentbuf->LINES + 1) / 2, TRUE);
	Currentbuf->currentLine = Currentbuf->lastLine;
    }
    else
	gotoRealLine(Currentbuf, atoi(l));
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
goLine(void)
{
    if (prec_num)
	_goLine("^");
    else
	_goLine(inputStr("Goto line: ", ""));
}

void
goLineF(void)
{
    _goLine("^");
}

void
goLineL(void)
{
    _goLine("$");
}

/* Go to the beginning of the line */
void
linbeg(void)
{
    if (Currentbuf->firstLine == NULL)
	return;
    Currentbuf->pos = 0;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* Go to the bottom of the line */
void
linend(void)
{
    if (Currentbuf->firstLine == NULL)
	return;
    Currentbuf->pos = Currentbuf->currentLine->len - 1;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* Run editor on the current buffer */
void
editBf(void)
{
    char *fn = Currentbuf->filename;
    Buffer *buf, *fbuf = NULL, sbuf;
#ifdef JP_CHARSET
    char old_code;
#endif
    Str cmd;

    if (fn == NULL || Currentbuf->pagerSource != NULL ||	/* Behaving as a pager */
	(Currentbuf->type == NULL && Currentbuf->edit == NULL) ||	/* Reading shell */
	Currentbuf->real_scheme != SCM_LOCAL || !strcmp(Currentbuf->currentURL.file, "-") ||	/* file is std input  */
	Currentbuf->bufferprop & BP_FRAME) {	/* Frame */
	disp_err_message("Can't edit other than local file", TRUE);
	return;
    }
    if (Currentbuf->frameset != NULL)
	fbuf = Currentbuf->linkBuffer[LB_FRAME];
    copyBuffer(&sbuf, Currentbuf);
    if (Currentbuf->edit)
	cmd = unquote_mailcap(Currentbuf->edit, Currentbuf->real_type, fn,
			      checkHeader(Currentbuf, "Content-Type:"), NULL);
    else
	cmd = myEditor(Editor, shell_quote(fn), CUR_LINENUMBER(Currentbuf));
    fmTerm();
    system(cmd->ptr);
    fmInit();

#ifdef JP_CHARSET
    old_code = DocumentCode;
    DocumentCode = Currentbuf->document_code;
#endif
    SearchHeader = Currentbuf->search_header;
    DefaultType = Currentbuf->real_type;
    buf = loadGeneralFile(file_to_url(fn), NULL, NO_REFERER, 0, NULL);
#ifdef JP_CHARSET
    DocumentCode = old_code;
#endif
    SearchHeader = FALSE;
    DefaultType = NULL;

    if (buf == NULL) {
	disp_err_message("Re-loading failed", FALSE);
	buf = nullBuffer();
    }
    else if (buf == NO_BUFFER) {
	buf = nullBuffer();
    }
    if (fbuf != NULL)
	Firstbuf = deleteBuffer(Firstbuf, fbuf);
    repBuffer(Currentbuf, buf);
    if ((buf->type != NULL) && (sbuf.type != NULL) &&
	((!strcasecmp(buf->type, "text/plain") &&
	  !strcasecmp(sbuf.type, "text/html")) ||
	 (!strcasecmp(buf->type, "text/html") &&
	  !strcasecmp(sbuf.type, "text/plain")))) {
	vwSrc();
	if (Currentbuf != buf)
	    Firstbuf = deleteBuffer(Firstbuf, buf);
    }
    Currentbuf->search_header = sbuf.search_header;
    if (Currentbuf->firstLine)
	restorePosition(Currentbuf, &sbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Run editor on the current screen */
void
editScr(void)
{
    char *tmpf;
    FILE *f;

    tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
    f = fopen(tmpf, "w");
    if (f == NULL) {
	disp_err_message(Sprintf("Can't open %s", tmpf)->ptr, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f);
    fclose(f);
    fmTerm();
    system(myEditor(Editor, tmpf, CUR_LINENUMBER(Currentbuf))->ptr);
    fmInit();
    unlink(tmpf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

#ifdef USE_MARK

/* Set / unset mark */
void
_mark(void)
{
    Line *l;
    if (!use_mark)
	return;
    if (Currentbuf->firstLine == NULL)
	return;
    l = Currentbuf->currentLine;
    l->propBuf[Currentbuf->pos] ^= PE_MARK;
    redrawLine(Currentbuf, l, l->linenumber - Currentbuf->topLine->linenumber
	       + Currentbuf->rootY);
}

/* Go to next mark */
void
nextMk(void)
{
    Line *l;
    int i;

    if (!use_mark)
	return;
    if (Currentbuf->firstLine == NULL)
	return;
    i = Currentbuf->pos + 1;
    l = Currentbuf->currentLine;
    if (i >= l->len) {
	i = 0;
	l = l->next;
    }
    while (l != NULL) {
	for (; i < l->len; i++) {
	    if (l->propBuf[i] & PE_MARK) {
		Currentbuf->currentLine = l;
		Currentbuf->pos = i;
		arrangeCursor(Currentbuf);
		displayBuffer(Currentbuf, B_NORMAL);
		return;
	    }
	}
	l = l->next;
	i = 0;
    }
    disp_message("No mark exist after here", TRUE);
}

/* Go to previous mark */
void
prevMk(void)
{
    Line *l;
    int i;

    if (!use_mark)
	return;
    if (Currentbuf->firstLine == NULL)
	return;
    i = Currentbuf->pos - 1;
    l = Currentbuf->currentLine;
    if (i < 0) {
	l = l->prev;
	if (l != NULL)
	    i = l->len - 1;
    }
    while (l != NULL) {
	for (; i >= 0; i--) {
	    if (l->propBuf[i] & PE_MARK) {
		Currentbuf->currentLine = l;
		Currentbuf->pos = i;
		arrangeCursor(Currentbuf);
		displayBuffer(Currentbuf, B_NORMAL);
		return;
	    }
	}
	l = l->prev;
	if (l != NULL)
	    i = l->len - 1;
    }
    disp_message("No mark exist before here", TRUE);
}

/* Mark place to which the regular expression matches */
void
reMark(void)
{
    Line *l;
    char *str;
    char *p, *p1, *p2;

    if (!use_mark)
	return;
    str = inputStrHist("(Mark)Regexp: ", MarkString, TextHist);
    if (str == NULL || *str == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if ((p = regexCompile(str, 1)) != NULL) {
	disp_message(p, TRUE);
	return;
    }
    MarkString = str;
    for (l = Currentbuf->firstLine; l != NULL; l = l->next) {
	p = l->lineBuf;
	for (;;) {
	    if (regexMatch(p, &l->lineBuf[l->len] - p, p == l->lineBuf) == 1) {
		matchedPosition(&p1, &p2);
		l->propBuf[p1 - l->lineBuf] |= PE_MARK;
		p = p2;
	    }
	    else
		break;
	}
    }

    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}
#endif				/* USE_MARK */

static Buffer *
loadNormalBuf(Buffer *buf, int renderframe)
{
    pushBuffer(buf);
    if (renderframe && RenderFrame && Currentbuf->frameset != NULL)
	rFrame();
    return buf;
}

static Buffer *
loadLink(char *url, char *target, char *referer, FormList *request)
{
    Buffer *buf, *nfbuf;
    union frameset_element *f_element = NULL;
    int flag = 0;
    ParsedURL *base, pu;

    message(Sprintf("loading %s", url)->ptr, 0, 0);
    refresh();

    base = baseURL(Currentbuf);
    if (base == NULL ||
	base->scheme == SCM_LOCAL || base->scheme == SCM_LOCAL_CGI)
	referer = NO_REFERER;
    if (referer == NULL)
	referer = parsedURL2Str(&Currentbuf->currentURL)->ptr;
    buf = loadGeneralFile(url, baseURL(Currentbuf), referer, flag, request);
    if (buf == NULL) {
	char *emsg = Sprintf("Can't load %s", url)->ptr;
	disp_err_message(emsg, FALSE);
	return NULL;
    }

    parseURL2(url, &pu, base);
    pushHashHist(URLHist, parsedURL2Str(&pu)->ptr);

    if (buf == NO_BUFFER) {
	return NULL;
    }
    if (!on_target)		/* open link as an indivisual page */
	return loadNormalBuf(buf, TRUE);

    if (do_download)		/* download (thus no need to render frame) */
	return loadNormalBuf(buf, FALSE);

    if (target == NULL ||	/* no target specified (that means this page is not a frame page) */
	!strcmp(target, "_top") ||	/* this link is specified to be opened as an indivisual * page */
	!(Currentbuf->bufferprop & BP_FRAME)	/* This page is not a frame page */
	) {
	return loadNormalBuf(buf, TRUE);
    }
    nfbuf = Currentbuf->linkBuffer[LB_N_FRAME];
    if (nfbuf == NULL) {
	/* original page (that contains <frameset> tag) doesn't exist */
	return loadNormalBuf(buf, TRUE);
    }

    f_element = search_frame(nfbuf->frameset, target);
    if (f_element == NULL) {
	/* specified target doesn't exist in this frameset */
	return loadNormalBuf(buf, TRUE);
    }

    /* frame page */

    /* stack current frameset */
    pushFrameTree(&(nfbuf->frameQ), copyFrameSet(nfbuf->frameset), Currentbuf);
    /* delete frame view buffer */
    delBuffer(Currentbuf);
    Currentbuf = nfbuf;
    /* nfbuf->frameset = copyFrameSet(nfbuf->frameset); */
    resetFrameElement(f_element, buf, referer, request);
    discardBuffer(buf);
    rFrame();
    {
	Anchor *al = NULL;
	char *label = pu.label;

	if (label && f_element->element->attr == F_BODY) {
	    al = searchAnchor(f_element->body->nameList, label);
	}
	if (!al) {
	    label = Strnew_m_charp("_", target, NULL)->ptr;
	    al = searchURLLabel(Currentbuf, label);
	}
	if (al) {
	    gotoLine(Currentbuf, al->start.line);
#ifdef LABEL_TOPLINE
	    if (label_topline)
		Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
					       Currentbuf->currentLine->
					       linenumber -
					       Currentbuf->topLine->linenumber,
					       FALSE);
#endif
	    Currentbuf->pos = al->start.pos;
	    arrangeCursor(Currentbuf);
	}
    }
    displayBuffer(Currentbuf, B_NORMAL);
    return buf;
}

static void
gotoLabel(char *label)
{
    Buffer *buf;
    Anchor *al;
    int i;

    al = searchURLLabel(Currentbuf, label);
    if (al == NULL) {
	disp_message(Sprintf("%s is not found", label)->ptr, TRUE);
	return;
    }
    buf = newBuffer(Currentbuf->width);
    copyBuffer(buf, Currentbuf);
    for (i = 0; i < MAX_LB; i++)
	buf->linkBuffer[i] = NULL;
    buf->currentURL.label = allocStr(label, -1);
    pushHashHist(URLHist, parsedURL2Str(&buf->currentURL)->ptr);
    (*buf->clone)++;
    pushBuffer(buf);
    gotoLine(Currentbuf, al->start.line);
#ifdef LABEL_TOPLINE
    if (label_topline)
	Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
				       Currentbuf->currentLine->linenumber
				       - Currentbuf->topLine->linenumber,
				       FALSE);
#endif
    Currentbuf->pos = al->start.pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    return;
}

/* follow HREF link */
void
followA(void)
{
    Line *l;
    Anchor *a;
    ParsedURL u;
#ifdef USE_IMAGE
    int x = 0, y = 0, map = 0;
#endif
    char *url;

    if (Currentbuf->firstLine == NULL)
	return;
    l = Currentbuf->currentLine;

#ifdef USE_IMAGE
    a = retrieveCurrentImg(Currentbuf);
    if (a && a->image && a->image->map) {
	_followForm(FALSE);
	return;
    }
    if (a && a->image && a->image->ismap) {
	getMapXY(Currentbuf, a, &x, &y);
	map = 1;
    }
#else
    a = retrieveCurrentMap(Currentbuf);
    if (a) {
	_followForm(FALSE);
	return;
    }
#endif
    a = retrieveCurrentAnchor(Currentbuf);
    if (a == NULL) {
	_followForm(FALSE);
	return;
    }
    if (*a->url == '#') {	/* index within this buffer */
	gotoLabel(a->url + 1);
	return;
    }
    parseURL2(a->url, &u, baseURL(Currentbuf));
    if (Strcmp(parsedURL2Str(&u), parsedURL2Str(&Currentbuf->currentURL)) == 0) {
	/* index within this buffer */
	if (u.label) {
	    gotoLabel(u.label);
	    return;
	}
    }
    if (!strncasecmp(a->url, "mailto:", 7)
#ifdef USE_W3MMAILER
	&& non_null(Mailer) && strchr(a->url, '?') == NULL
#endif
	) {
	/* invoke external mailer */
	Str to = Strnew_charp(a->url + 7);
#ifndef USE_W3MMAILER
	char *pos;
	if (!non_null(Mailer)) {
	    disp_err_message("no mailer is specified", TRUE);
	    return;
	}
	if ((pos = strchr(to->ptr, '?')) != NULL)
	    Strtruncate(to, pos - to->ptr);
#endif
	fmTerm();
	system(myExtCommand(Mailer, shell_quote(url_unquote(to->ptr)),
			    FALSE)->ptr);
	fmInit();
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	pushHashHist(URLHist, a->url);
	return;
    }
#ifdef USE_NNTP
    else if (!strncasecmp(a->url, "news:", 5) && strchr(a->url, '@') == NULL) {
	/* news:newsgroup is not supported */
	disp_err_message("news:newsgroup_name is not supported", TRUE);
	return;
    }
#endif				/* USE_NNTP */
    url = a->url;
#ifdef USE_IMAGE
    if (map)
	url = Sprintf("%s?%d,%d", a->url, x, y)->ptr;
#endif

    if (check_target && open_tab_blank && a->target &&
	(!strcasecmp(a->target, "_new") || !strcasecmp(a->target, "_blank"))) {
	Buffer *buf;

	_newT();
	buf = Currentbuf;
	loadLink(url, a->target, a->referer, NULL);
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    loadLink(url, a->target, a->referer, NULL);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* follow HREF link in the buffer */
void
bufferA(void)
{
    on_target = FALSE;
    followA();
    on_target = TRUE;
}

/* view inline image */
void
followI(void)
{
    Line *l;
    Anchor *a;
    Buffer *buf;

    if (Currentbuf->firstLine == NULL)
	return;
    l = Currentbuf->currentLine;

    a = retrieveCurrentImg(Currentbuf);
    if (a == NULL)
	return;
    message(Sprintf("loading %s", a->url)->ptr, 0, 0);
    refresh();
    buf = loadGeneralFile(a->url, baseURL(Currentbuf), NULL, 0, NULL);
    if (buf == NULL) {
	char *emsg = Sprintf("Can't load %s", a->url)->ptr;
	disp_err_message(emsg, FALSE);
    }
    else if (buf != NO_BUFFER) {
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

static FormItemList *
save_submit_formlist(FormItemList *src)
{
    FormList *list;
    FormList *srclist;
    FormItemList *srcitem;
    FormItemList *item;
    FormItemList *ret = NULL;
#ifdef MENU_SELECT
    FormSelectOptionItem *opt;
    FormSelectOptionItem *curopt;
    FormSelectOptionItem *srcopt;
#endif				/* MENU_SELECT */

    if (src == NULL)
	return NULL;
    srclist = src->parent;
    list = New(FormList);
    list->method = srclist->method;
    list->action = Strdup(srclist->action);
    list->charset = srclist->charset;
    list->enctype = srclist->enctype;
    list->nitems = srclist->nitems;
    list->body = srclist->body;
    list->boundary = srclist->boundary;
    list->length = srclist->length;

    for (srcitem = srclist->item; srcitem; srcitem = srcitem->next) {
	item = New(FormItemList);
	item->type = srcitem->type;
	item->name = Strdup(srcitem->name);
	item->value = Strdup(srcitem->value);
	item->checked = srcitem->checked;
	item->accept = srcitem->accept;
	item->size = srcitem->size;
	item->rows = srcitem->rows;
	item->maxlength = srcitem->maxlength;
	item->readonly = srcitem->readonly;
#ifdef MENU_SELECT
	opt = curopt = NULL;
	for (srcopt = srcitem->select_option; srcopt; srcopt = srcopt->next) {
	    if (!srcopt->checked)
		continue;
	    opt = New(FormSelectOptionItem);
	    opt->value = Strdup(srcopt->value);
	    opt->label = Strdup(srcopt->label);
	    opt->checked = srcopt->checked;
	    if (item->select_option == NULL) {
		item->select_option = curopt = opt;
	    }
	    else {
		curopt->next = opt;
		curopt = curopt->next;
	    }
	}
	item->select_option = opt;
	if (srcitem->label)
	    item->label = Strdup(srcitem->label);
#endif				/* MENU_SELECT */
	item->parent = list;
	item->next = NULL;

	if (list->lastitem == NULL) {
	    list->item = list->lastitem = item;
	}
	else {
	    list->lastitem->next = item;
	    list->lastitem = item;
	}

	if (srcitem == src)
	    ret = item;
    }

    return ret;
}

#ifdef JP_CHARSET
static Str
conv_form_encoding(Str val, FormItemList *fi, Buffer *buf)
{
    return conv_str(val, InnerCode, fi->parent->charset ? fi->parent->charset
		    : (buf->document_code ? buf->document_code : CODE_EUC));
}
#else
#define conv_form_encoding(val, fi, buf) (val)
#endif

static void
query_from_followform(Str *query, FormItemList *fi, int multipart)
{
    FormItemList *f2;
    FILE *body = NULL;

    if (multipart) {
	*query = tmpfname(TMPF_DFL, NULL);
	body = fopen((*query)->ptr, "w");
	if (body == NULL) {
	    return;
	}
	fi->parent->body = (*query)->ptr;
	fi->parent->boundary =
	    Sprintf("------------------------------%d%ld%ld%ld", getpid(),
		    fi->parent, fi->parent->body, fi->parent->boundary)->ptr;
    }
    *query = Strnew();
    for (f2 = fi->parent->item; f2; f2 = f2->next) {
	if (f2->name == NULL)
	    continue;
	/* <ISINDEX> is translated into single text form */
	if (f2->name->length == 0 &&
	    (multipart || f2->type != FORM_INPUT_TEXT))
	    continue;
	switch (f2->type) {
	case FORM_INPUT_RESET:
	    /* do nothing */
	    continue;
	case FORM_INPUT_SUBMIT:
	case FORM_INPUT_IMAGE:
	    if (f2 != fi || f2->value == NULL)
		continue;
	    break;
	case FORM_INPUT_RADIO:
	case FORM_INPUT_CHECKBOX:
	    if (!f2->checked)
		continue;
	}
	if (multipart) {
	    if (f2->type == FORM_INPUT_IMAGE) {
		int x = 0, y = 0;
#ifdef USE_IMAGE
		getMapXY(Currentbuf, retrieveCurrentImg(Currentbuf), &x, &y);
#endif
		*query = Strdup(conv_form_encoding(f2->name, fi, Currentbuf));
		Strcat_charp(*query, ".x");
		form_write_data(body, fi->parent->boundary, (*query)->ptr,
				Sprintf("%d", x)->ptr);
		*query = Strdup(conv_form_encoding(f2->name, fi, Currentbuf));
		Strcat_charp(*query, ".y");
		form_write_data(body, fi->parent->boundary, (*query)->ptr,
				Sprintf("%d", y)->ptr);
	    }
	    else if (f2->name && f2->name->length > 0 && f2->value != NULL) {
		/* not IMAGE */
		*query = conv_form_encoding(f2->value, fi, Currentbuf);
		if (f2->type == FORM_INPUT_FILE)
		    form_write_from_file(body, fi->parent->boundary,
					 conv_form_encoding(f2->name, fi,
							    Currentbuf)->ptr,
					 (*query)->ptr,
					 Str_conv_to_system(f2->value)->ptr);
		else
		    form_write_data(body, fi->parent->boundary,
				    conv_form_encoding(f2->name, fi,
						       Currentbuf)->ptr,
				    (*query)->ptr);
	    }
	}
	else {
	    /* not multipart */
	    if (f2->type == FORM_INPUT_IMAGE) {
		int x = 0, y = 0;
#ifdef USE_IMAGE
		getMapXY(Currentbuf, retrieveCurrentImg(Currentbuf), &x, &y);
#endif
		Strcat(*query,
		       Str_form_quote(conv_form_encoding
				      (f2->name, fi, Currentbuf)));
		Strcat(*query, Sprintf(".x=%d&", x));
		Strcat(*query,
		       Str_form_quote(conv_form_encoding
				      (f2->name, fi, Currentbuf)));
		Strcat(*query, Sprintf(".y=%d", y));
	    }
	    else {
		/* not IMAGE */
		if (f2->name && f2->name->length > 0) {
		    Strcat(*query,
			   Str_form_quote(conv_form_encoding
					  (f2->name, fi, Currentbuf)));
		    Strcat_char(*query, '=');
		}
		if (f2->value != NULL) {
		    if (fi->parent->method == FORM_METHOD_INTERNAL)
			Strcat(*query, Str_form_quote(f2->value));
		    else {
			Strcat(*query,
			       Str_form_quote(conv_form_encoding
					      (f2->value, fi, Currentbuf)));
		    }
		}
	    }
	    if (f2->next)
		Strcat_char(*query, '&');
	}
    }
    if (multipart) {
	fprintf(body, "--%s--\r\n", fi->parent->boundary);
	fclose(body);
    }
    else {
	/* remove trailing & */
	while (Strlastchar(*query) == '&')
	    Strshrink(*query, 1);
    }
}

/* submit form */
void
submitForm(void)
{
    _followForm(TRUE);
}

/* process form */
void
followForm(void)
{
    _followForm(FALSE);
}

static void
_followForm(int submit)
{
    Line *l;
    Anchor *a, *a2;
    char *p;
    FormItemList *fi, *f2;
    Str tmp, tmp2;
    int multipart = 0, i;

    if (Currentbuf->firstLine == NULL)
	return;
    l = Currentbuf->currentLine;

    a = retrieveCurrentForm(Currentbuf);
    if (a == NULL)
	return;
    fi = (FormItemList *)a->url;
    switch (fi->type) {
    case FORM_INPUT_TEXT:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	p = inputStrHist("TEXT:", fi->value ? fi->value->ptr : NULL, TextHist);
	if (p == NULL || fi->readonly)
	    return;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept || fi->parent->nitems == 1)
	    goto do_submit;
	break;
    case FORM_INPUT_FILE:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	p = inputFilenameHist("Filename:", fi->value ? fi->value->ptr : NULL,
			      NULL);
	if (p == NULL || fi->readonly)
	    return;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept || fi->parent->nitems == 1)
	    goto do_submit;
	break;
    case FORM_INPUT_PASSWORD:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    return;
	}
	p = inputLine("Password:", fi->value ? fi->value->ptr : NULL,
		      IN_PASSWORD);
	if (p == NULL)
	    return;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept)
	    goto do_submit;
	break;
    case FORM_TEXTAREA:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	input_textarea(fi);
	formUpdateBuffer(a, Currentbuf, fi);
	break;
    case FORM_INPUT_RADIO:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    return;
	}
	formRecheckRadio(a, Currentbuf, fi);
	break;
    case FORM_INPUT_CHECKBOX:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    return;
	}
	fi->checked = !fi->checked;
	formUpdateBuffer(a, Currentbuf, fi);
	break;
#ifdef MENU_SELECT
    case FORM_SELECT:
	if (submit)
	    goto do_submit;
	if (!formChooseOptionByMenu(fi,
				    Currentbuf->cursorX - Currentbuf->pos +
				    a->start.pos + Currentbuf->rootX,
				    Currentbuf->cursorY + Currentbuf->rootY))
	    break;
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->parent->nitems == 1)
	    goto do_submit;
	break;
#endif				/* MENU_SELECT */
    case FORM_INPUT_IMAGE:
    case FORM_INPUT_SUBMIT:
    case FORM_INPUT_BUTTON:
      do_submit:
	tmp = Strnew();
	tmp2 = Strnew();
	multipart = (fi->parent->method == FORM_METHOD_POST &&
		     fi->parent->enctype == FORM_ENCTYPE_MULTIPART);
	query_from_followform(&tmp, fi, multipart);

	tmp2 = Strdup(fi->parent->action);
	if (!Strcmp_charp(tmp2, "!CURRENT_URL!")) {
	    /* It means "current URL" */
	    tmp2 = parsedURL2Str(&Currentbuf->currentURL);
	    if ((p = strchr(tmp2->ptr, '?')) != NULL)
		Strshrink(tmp2, (tmp2->ptr + tmp2->length) - p);
	}

	if (fi->parent->method == FORM_METHOD_GET) {
	    Strcat_charp(tmp2, "?");
	    Strcat(tmp2, tmp);
	    loadLink(tmp2->ptr, a->target, NULL, NULL);
	}
	else if (fi->parent->method == FORM_METHOD_POST) {
	    Buffer *buf;
	    if (multipart) {
		struct stat st;
		stat(fi->parent->body, &st);
		fi->parent->length = st.st_size;
	    }
	    else {
		fi->parent->body = tmp->ptr;
		fi->parent->length = tmp->length;
	    }
	    buf = loadLink(tmp2->ptr, a->target, NULL, fi->parent);
	    if (multipart) {
		unlink(fi->parent->body);
	    }
	    if (buf && !(buf->bufferprop & BP_REDIRECTED)) {	/* buf must be Currentbuf */
		/* BP_REDIRECTED means that the buffer is obtained through
		 * Location: header. In this case, buf->form_submit must not be set
		 * because the page is not loaded by POST method but GET method.
		 */
		buf->form_submit = save_submit_formlist(fi);
	    }
	}
	else if ((fi->parent->method == FORM_METHOD_INTERNAL && (!Strcmp_charp(fi->parent->action, "map") || !Strcmp_charp(fi->parent->action, "none"))) || Currentbuf->bufferprop & BP_INTERNAL) {	/* internal */
	    do_internal(tmp2->ptr, tmp->ptr);
	}
	else {
	    disp_err_message("Can't send form because of illegal method.",
			     FALSE);
	}
	break;
    case FORM_INPUT_RESET:
	for (i = 0; i < Currentbuf->formitem->nanchor; i++) {
	    a2 = &Currentbuf->formitem->anchors[i];
	    f2 = (FormItemList *)a2->url;
	    if (f2->parent == fi->parent &&
		f2->name && f2->value &&
		f2->type != FORM_INPUT_SUBMIT &&
		f2->type != FORM_INPUT_HIDDEN &&
		f2->type != FORM_INPUT_RESET) {
		f2->value = f2->init_value;
		f2->checked = f2->init_checked;
#ifdef MENU_SELECT
		f2->label = f2->init_label;
		f2->selected = f2->init_selected;
#endif				/* MENU_SELECT */
		formUpdateBuffer(a2, Currentbuf, f2);
	    }
	}
	break;
    case FORM_INPUT_HIDDEN:
    default:
	break;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
drawAnchorCursor0(Buffer *buf, AnchorList *al, int hseq, int prevhseq,
		  int tline, int eline, int active)
{
    int i, j;
    Line *l;
    Anchor *an;

    l = buf->topLine;
    for (j = 0; j < al->nanchor; j++) {
	an = &al->anchors[j];
	if (an->start.line < tline)
	    continue;
	if (an->start.line >= eline)
	    return;
	for (;; l = l->next) {
	    if (l == NULL)
		return;
	    if (l->linenumber == an->start.line)
		break;
	}
	if (hseq >= 0 && an->hseq == hseq) {
	    for (i = an->start.pos; i < an->end.pos; i++) {
		if (l->propBuf[i] & (PE_IMAGE | PE_ANCHOR | PE_FORM)) {
		    if (active)
			l->propBuf[i] |= PE_ACTIVE;
		    else
			l->propBuf[i] &= ~PE_ACTIVE;
		}
	    }
	    if (active)
		redrawLineRegion(buf, l, l->linenumber - tline + buf->rootY,
				 an->start.pos, an->end.pos);
	}
	else if (prevhseq >= 0 && an->hseq == prevhseq) {
	    if (active)
		redrawLineRegion(buf, l, l->linenumber - tline + buf->rootY,
				 an->start.pos, an->end.pos);
	}
    }
}


void
drawAnchorCursor(Buffer *buf)
{
    Anchor *an;
    int hseq, prevhseq;
    int tline, eline;

    if (!buf->firstLine || !buf->hmarklist)
	return;
    if (!buf->href && !buf->formitem)
	return;

    an = retrieveCurrentAnchor(buf);
    if (!an)
	an = retrieveCurrentMap(buf);
    if (an)
	hseq = an->hseq;
    else
	hseq = -1;
    tline = buf->topLine->linenumber;
    eline = tline + buf->LINES;
    prevhseq = buf->hmarklist->prevhseq;

    if (buf->href) {
	drawAnchorCursor0(buf, buf->href, hseq, prevhseq, tline, eline, 1);
	drawAnchorCursor0(buf, buf->href, hseq, -1, tline, eline, 0);
    }
    if (buf->formitem) {
	drawAnchorCursor0(buf, buf->formitem, hseq, prevhseq, tline, eline, 1);
	drawAnchorCursor0(buf, buf->formitem, hseq, -1, tline, eline, 0);
    }
    buf->hmarklist->prevhseq = hseq;
}

/* underline an anchor if cursor is on the anchor. */
void
onA(void)
{
    drawAnchorCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the top anchor */
void
topA(void)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an;
    int hseq = 0;

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    if (prec_num > hl->nmark)
	hseq = hl->nmark - 1;
    else if (prec_num > 0)
	hseq = prec_num - 1;
    do {
	if (hseq >= hl->nmark)
	    return;
	po = hl->marks + hseq;
	an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
	if (an == NULL)
	    an = retrieveAnchor(Currentbuf->formitem, po->line, po->pos);
	hseq++;
    } while (an == NULL);

    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the last anchor */
void
lastA(void)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an;
    int hseq;

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    if (prec_num >= hl->nmark)
	hseq = 0;
    else if (prec_num > 0)
	hseq = hl->nmark - prec_num;
    else
	hseq = hl->nmark - 1;
    do {
	if (hseq < 0)
	    return;
	po = hl->marks + hseq;
	an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
	if (an == NULL)
	    an = retrieveAnchor(Currentbuf->formitem, po->line, po->pos);
	hseq--;
    } while (an == NULL);

    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the next anchor */
void
nextA(void)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an, *pan;
    int i, x, y, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    y = Currentbuf->currentLine->linenumber;
    x = Currentbuf->pos;

    for (i = 0; i < n; i++) {
	pan = an;
	if (an && an->hseq >= 0) {
	    int hseq = an->hseq + 1;
	    do {
		if (hseq >= hl->nmark) {
		    pan = an;
		    goto _end;
		}
		po = &hl->marks[hseq];
		an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
		if (an == NULL)
		    an = retrieveAnchor(Currentbuf->formitem, po->line,
					po->pos);
		hseq++;
	    } while (an == NULL || an == pan);
	}
	else {
	    an = closest_next_anchor(Currentbuf->href, NULL, x, y);
	    an = closest_next_anchor(Currentbuf->formitem, an, x, y);
	    if (an == NULL) {
		an = pan;
		break;
	    }
	    x = an->start.pos;
	    y = an->start.line;
	}
    }

  _end:
    if (an == NULL || an->hseq < 0)
	return;
    po = &hl->marks[an->hseq];
    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the previous anchor */
void
prevA(void)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an, *pan;
    int i, x, y, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    y = Currentbuf->currentLine->linenumber;
    x = Currentbuf->pos;

    for (i = 0; i < n; i++) {
	pan = an;
	if (an && an->hseq >= 0) {
	    int hseq = an->hseq - 1;
	    do {
		if (hseq < 0) {
		    an = pan;
		    goto _end;
		}
		po = hl->marks + hseq;
		an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
		if (an == NULL)
		    an = retrieveAnchor(Currentbuf->formitem, po->line,
					po->pos);
		hseq--;
	    } while (an == NULL || an == pan);
	}
	else {
	    an = closest_prev_anchor(Currentbuf->href, NULL, x, y);
	    an = closest_prev_anchor(Currentbuf->formitem, an, x, y);
	    if (an == NULL) {
		an = pan;
		break;
	    }
	    x = an->start.pos;
	    y = an->start.line;
	}
    }

  _end:
    if (an == NULL || an->hseq < 0)
	return;
    po = hl->marks + an->hseq;
    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the next left/right anchor */
static void
nextX(int d, int dy)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    Anchor *an, *pan;
    Line *l;
    int i, x, y, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    l = Currentbuf->currentLine;
    x = Currentbuf->pos;
    y = l->linenumber;
    pan = NULL;
    for (i = 0; i < n; i++) {
	if (an)
	    x = (d > 0) ? an->end.pos : an->start.pos - 1;
	an = NULL;
	while (1) {
	    for (; x >= 0 && x < l->len; x += d) {
		an = retrieveAnchor(Currentbuf->href, y, x);
		if (!an)
		    an = retrieveAnchor(Currentbuf->formitem, y, x);
		if (an) {
		    pan = an;
		    break;
		}
	    }
	    if (!dy || an)
		break;
	    l = (dy > 0) ? l->next : l->prev;
	    if (!l)
		break;
	    x = (d > 0) ? 0 : l->len - 1;
	    y = l->linenumber;
	}
	if (!an)
	    break;
    }

    if (pan == NULL)
	return;
    gotoLine(Currentbuf, y);
    Currentbuf->pos = pan->start.pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the next downward/upward anchor */
static void
nextY(int d)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    Anchor *an, *pan;
    int i, x, y, n = searchKeyNum();
    int hseq;

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    x = Currentbuf->pos;
    y = Currentbuf->currentLine->linenumber + d;
    pan = NULL;
    hseq = -1;
    for (i = 0; i < n; i++) {
	if (an)
	    hseq = abs(an->hseq);
	an = NULL;
	for (; y >= 0 && y <= Currentbuf->lastLine->linenumber; y += d) {
	    an = retrieveAnchor(Currentbuf->href, y, x);
	    if (!an)
		an = retrieveAnchor(Currentbuf->formitem, y, x);
	    if (an && hseq != abs(an->hseq)) {
		pan = an;
		break;
	    }
	}
	if (!an)
	    break;
    }

    if (pan == NULL)
	return;
    gotoLine(Currentbuf, pan->start.line);
    arrangeLine(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the next left anchor */
void
nextL(void)
{
    nextX(-1, 0);
}

/* go to the next left-up anchor */
void
nextLU(void)
{
    nextX(-1, -1);
}

/* go to the next right anchor */
void
nextR(void)
{
    nextX(1, 0);
}

/* go to the next right-down anchor */
void
nextRD(void)
{
    nextX(1, 1);
}

/* go to the next downward anchor */
void
nextD(void)
{
    nextY(1);
}

/* go to the next upward anchor */
void
nextU(void)
{
    nextY(-1);
}

/* go to the next bufferr */
void
nextBf(void)
{
    Buffer *buf;
    int i;

    for (i = 0; i < PREC_NUM; i++) {
	buf = prevBuffer(Firstbuf, Currentbuf);
	if (!buf) {
	    if (i == 0)
		return;
	    break;
	}
	Currentbuf = buf;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* go to the previous bufferr */
void
prevBf(void)
{
    Buffer *buf;
    int i;

    for (i = 0; i < PREC_NUM; i++) {
	buf = Currentbuf->nextBuffer;
	if (!buf) {
	    if (i == 0)
		return;
	    break;
	}
	Currentbuf = buf;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static int
checkBackBuffer(Buffer *buf)
{
    Buffer *fbuf = buf->linkBuffer[LB_N_FRAME];

    if (fbuf) {
	if (fbuf->frameQ)
	    return TRUE;	/* Currentbuf has stacked frames */
	/* when no frames stacked and next is frame source, try next's
	 * nextBuffer */
	if (RenderFrame && fbuf == buf->nextBuffer) {
	    if (fbuf->nextBuffer != NULL)
		return TRUE;
	    else
		return FALSE;
	}
    }

    if (buf->nextBuffer)
	return TRUE;

    return FALSE;
}

/* delete current buffer and back to the previous buffer */
void
backBf(void)
{
    Buffer *buf = Currentbuf->linkBuffer[LB_N_FRAME];

    if (!checkBackBuffer(Currentbuf)) {
	if (close_tab_back && nTab >= 1) {
	    deleteTab(CurrentTab);
	    displayBuffer(Currentbuf, B_FORCE_REDRAW);
	}
	else
	    disp_message("Can't back...", TRUE);
	return;
    }

    delBuffer(Currentbuf);

    if (buf) {
	if (buf->frameQ) {
	    struct frameset *fs;
	    long linenumber = buf->frameQ->linenumber;
	    long top = buf->frameQ->top_linenumber;
	    short pos = buf->frameQ->pos;
	    short currentColumn = buf->frameQ->currentColumn;
	    AnchorList *formitem = buf->frameQ->formitem;

	    fs = popFrameTree(&(buf->frameQ));
	    deleteFrameSet(buf->frameset);
	    buf->frameset = fs;

	    if (buf == Currentbuf) {
		rFrame();
		Currentbuf->topLine = lineSkip(Currentbuf,
					       Currentbuf->firstLine, top - 1,
					       FALSE);
		gotoLine(Currentbuf, linenumber);
		Currentbuf->pos = pos;
		Currentbuf->currentColumn = currentColumn;
		arrangeCursor(Currentbuf);
		formResetBuffer(Currentbuf, formitem);
	    }
	}
	else if (RenderFrame && buf == Currentbuf) {
	    delBuffer(Currentbuf);
	}
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
deletePrevBuf()
{
    Buffer *buf = Currentbuf->nextBuffer;
    if (buf)
	delBuffer(buf);
}

static void
cmd_loadURL(char *url, ParsedURL *current, char *referer)
{
    Buffer *buf;

    if (!strncasecmp(url, "mailto:", 7)
#ifdef USE_W3MMAILER
	&& non_null(Mailer) && strchr(url, '?') == NULL
#endif
	) {
	/* invoke external mailer */
	Str to = Strnew_charp(url + 7);
#ifndef USE_W3MMAILER
	char *pos;
	if (!non_null(Mailer)) {
	    disp_err_message("no mailer is specified", TRUE);
	    return;
	}
	if ((pos = strchr(to->ptr, '?')) != NULL)
	    Strtruncate(to, pos - to->ptr);
#endif
	fmTerm();
	system(myExtCommand(Mailer, shell_quote(url_unquote(to->ptr)),
			    FALSE)->ptr);
	fmInit();
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	pushHashHist(URLHist, url);
	return;
    }
#ifdef USE_NNTP
    if (!strncasecmp(url, "news:", 5) && strchr(url, '@') == NULL) {
	/* news:newsgroup is not supported */
	disp_err_message("news:newsgroup_name is not supported", TRUE);
	return;
    }
#endif				/* USE_NNTP */

    refresh();
    buf = loadGeneralFile(url, current, referer, 0, NULL);
    if (buf == NULL) {
	char *emsg = Sprintf("Can't load %s", conv_from_system(url))->ptr;
	disp_err_message(emsg, FALSE);
    }
    else if (buf != NO_BUFFER) {
	pushBuffer(buf);
	if (RenderFrame && Currentbuf->frameset != NULL)
	    rFrame();
    }
    displayBuffer(Currentbuf, B_NORMAL);
}


/* go to specified URL */
static void
goURL0(char *prompt, int relative)
{
    char *url, *referer;
    ParsedURL p_url, *current;
    Buffer *cur_buf = Currentbuf;

    url = searchKeyData();
    if (url == NULL) {
	Hist *hist = copyHist(URLHist);
	Anchor *a;

	current = baseURL(Currentbuf);
	if (current) {
	    char *c_url = parsedURL2Str(current)->ptr;
	    if (DefaultURLString == DEFAULT_URL_CURRENT)
		url = c_url;
	    else
		pushHist(hist, c_url);
	}
	a = retrieveCurrentAnchor(Currentbuf);
	if (a) {
	    char *a_url;
	    parseURL2(a->url, &p_url, current);
	    a_url = parsedURL2Str(&p_url)->ptr;
	    if (DefaultURLString == DEFAULT_URL_LINK)
		url = a_url;
	    else
		pushHist(hist, a_url);
	}
	url = inputLineHist(prompt, url, IN_URL, hist);
	if (url != NULL)
	    SKIP_BLANKS(url);
    }
#ifdef JP_CHARSET
    if (url != NULL) {
	if (Currentbuf->document_code)
	    url = conv(url, InnerCode, Currentbuf->document_code)->ptr;
	else
	    url = conv_to_system(url);
    }
#endif
    if (url == NULL || *url == '\0') {
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    if (*url == '#') {
	gotoLabel(url + 1);
	return;
    }
    if (relative) {
	current = baseURL(Currentbuf);
	referer = parsedURL2Str(&Currentbuf->currentURL)->ptr;
    }
    else {
	current = NULL;
	referer = NULL;
    }
    parseURL2(url, &p_url, current);
    pushHashHist(URLHist, parsedURL2Str(&p_url)->ptr);
    cmd_loadURL(url, current, referer);
    if (Currentbuf != cur_buf)	/* success */
	pushHashHist(URLHist, parsedURL2Str(&Currentbuf->currentURL)->ptr);
}

void
goURL(void)
{
    goURL0("Goto URL: ", FALSE);
}

void
gorURL(void)
{
    goURL0("Goto relative URL: ", TRUE);
}

static void
cmd_loadBuffer(Buffer *buf, int prop, int linkid)
{
    if (buf == NULL) {
	disp_err_message("Can't load string", FALSE);
    }
    else if (buf != NO_BUFFER) {
	buf->bufferprop |= (BP_INTERNAL | prop);
	if (!(buf->bufferprop & BP_NO_URL))
	    copyParsedURL(&buf->currentURL, &Currentbuf->currentURL);
	if (linkid != LB_NOLINK) {
	    buf->linkBuffer[REV_LB[linkid]] = Currentbuf;
	    Currentbuf->linkBuffer[linkid] = buf;
	}
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* load bookmark */
void
ldBmark(void)
{
    cmd_loadURL(BookmarkFile, NULL, NO_REFERER);
}


/* Add current to bookmark */
void
adBmark(void)
{
    Str tmp;

    tmp = Sprintf("file://%s/" W3MBOOKMARK_CMDNAME
		  "?mode=panel&bmark=%s&url=%s&title=%s",
		  w3m_lib_dir(),
		  (Str_form_quote(Strnew_charp(BookmarkFile)))->ptr,
		  (Str_form_quote(parsedURL2Str(&Currentbuf->currentURL)))->
		  ptr,
		  (Str_form_quote(Strnew_charp(Currentbuf->buffername)))->ptr);
    cmd_loadURL(tmp->ptr, NULL, NO_REFERER);
}

/* option setting */
void
ldOpt(void)
{
    cmd_loadBuffer(load_option_panel(), BP_NO_URL, LB_NOLINK);
}

/* set an option */
void
setOpt(void)
{
    char *opt;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    opt = searchKeyData();
    if (opt == NULL || *opt == '\0' || strchr(opt, '=') == NULL) {
	if (opt != NULL && *opt != '\0') {
	    char *v = get_param_option(opt);
	    opt = Sprintf("%s=%s", opt, v ? v : "")->ptr;
	}
	opt = inputStrHist("Set option: ", opt, TextHist);
	if (opt == NULL || *opt == '\0') {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    if (set_param_option(opt))
	sync_with_option();
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

/* error message list */
void
msgs(void)
{
    cmd_loadBuffer(message_list_panel(), BP_NO_URL, LB_NOLINK);
}

/* page info */
void
pginfo(void)
{
    Buffer *buf;

    if ((buf = Currentbuf->linkBuffer[LB_N_INFO]) != NULL) {
	Currentbuf = buf;
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if ((buf = Currentbuf->linkBuffer[LB_INFO]) != NULL)
	delBuffer(buf);
    buf = page_info_panel(Currentbuf);
#ifdef JP_CHARSET
    if (buf != NULL)
	buf->document_code = Currentbuf->document_code;
#endif				/* JP_CHARSET */
    cmd_loadBuffer(buf, BP_NORMAL, LB_INFO);
}

void
follow_map(struct parsed_tagarg *arg)
{
    char *name = tag_get_value(arg, "link");
#if defined(MENU_MAP) || defined(USE_IMAGE)
    Anchor *an;
    MapArea *a;
    int x, y;
    ParsedURL p_url;

    an = retrieveCurrentImg(Currentbuf);
    x = Currentbuf->cursorX + Currentbuf->rootX;
    y = Currentbuf->cursorY + Currentbuf->rootY;
    a = follow_map_menu(Currentbuf, name, an, x, y);
    if (a == NULL || a->url == NULL || *(a->url) == '\0') {
#endif
#ifndef MENU_MAP
	Buffer *buf = follow_map_panel(Currentbuf, name);

	if (buf != NULL) {
#ifdef JP_CHARSET
	    buf->document_code = Currentbuf->document_code;
#endif				/* JP_CHARSET */
	    cmd_loadBuffer(buf, BP_NORMAL, LB_NOLINK);
	}
#endif
#if defined(MENU_MAP) || defined(USE_IMAGE)
	return;
    }
    if (*(a->url) == '#') {
	gotoLabel(a->url + 1);
	return;
    }
    parseURL2(a->url, &p_url, baseURL(Currentbuf));
    pushHashHist(URLHist, parsedURL2Str(&p_url)->ptr);
    if (check_target && open_tab_blank && a->target &&
	(!strcasecmp(a->target, "_new") || !strcasecmp(a->target, "_blank"))) {
	Buffer *buf;

	_newT();
	buf = Currentbuf;
	cmd_loadURL(a->url, baseURL(Currentbuf),
		    parsedURL2Str(&Currentbuf->currentURL)->ptr);
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    cmd_loadURL(a->url, baseURL(Currentbuf),
		parsedURL2Str(&Currentbuf->currentURL)->ptr);
#endif
}

#ifdef USE_COOKIE
/* cookie list */
void
cooLst(void)
{
    Buffer *buf;

    buf = cookie_list_panel();
    if (buf != NULL)
	cmd_loadBuffer(buf, BP_NO_URL, LB_NOLINK);
}
#endif				/* USE_COOKIE */

#ifdef USE_HISTORY
/* History page */
void
ldHist(void)
{
    cmd_loadBuffer(historyBuffer(URLHist), BP_NO_URL, LB_NOLINK);
}
#endif				/* USE_HISTORY */

/* download HREF link */
void
svA(void)
{
    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    do_download = TRUE;
    followA();
    do_download = FALSE;
}

/* download IMG link */
void
svI(void)
{
    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    do_download = TRUE;
    followI();
    do_download = FALSE;
}

/* save buffer */
void
svBuf(void)
{
    char *qfile = NULL, *file;
    FILE *f;
    int is_pipe;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    file = searchKeyData();
    if (file == NULL || *file == '\0') {
	qfile = inputLineHist("Save buffer to: ", NULL, IN_COMMAND, SaveHist);
	if (qfile == NULL || *qfile == '\0') {
	    displayBuffer(Currentbuf, B_FORCE_REDRAW);
	    return;
	}
    }
    file = conv_to_system(qfile ? qfile : file);
    if (*file == '|') {
	is_pipe = TRUE;
	f = popen(file + 1, "w");
    }
    else {
	if (qfile) {
	    file = unescape_spaces(Strnew_charp(qfile))->ptr;
	    file = conv_to_system(file);
	}
	file = expandName(file);
	if (checkOverWrite(file) < 0)
	    return;
	f = fopen(file, "w");
	is_pipe = FALSE;
    }
    if (f == NULL) {
	char *emsg = Sprintf("Can't open %s", conv_from_system(file))->ptr;
	disp_err_message(emsg, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f);
    if (is_pipe)
	pclose(f);
    else
	fclose(f);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* save source */
void
svSrc(void)
{
    char *file;

    if (Currentbuf->sourcefile == NULL)
	return;
    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    PermitSaveToPipe = TRUE;
    if (Currentbuf->real_scheme == SCM_LOCAL)
	file = conv_from_system(guess_save_name(NULL,
						Currentbuf->currentURL.
						real_file));
    else
	file = guess_save_name(Currentbuf, Currentbuf->currentURL.file);
    doFileCopy(Currentbuf->sourcefile, file);
    PermitSaveToPipe = FALSE;
    displayBuffer(Currentbuf, B_NORMAL);
}

static void
_peekURL(int only_img)
{

    Anchor *a;
    ParsedURL pu;
    static Str s = NULL;
    static int offset = 0, n;

    if (Currentbuf->firstLine == NULL)
	return;
    if (CurrentKey == prev_key && s != NULL) {
	if (s->length - offset >= COLS)
	    offset++;
	else if (s->length <= offset)	/* bug ? */
	    offset = 0;
	goto disp;
    }
    else {
	offset = 0;
    }
    a = (only_img ? NULL : retrieveCurrentAnchor(Currentbuf));
    if (a == NULL) {
	a = (only_img ? NULL : retrieveCurrentForm(Currentbuf));
	if (a == NULL) {
	    a = retrieveCurrentImg(Currentbuf);
	    if (a == NULL) {
		s = NULL;
		return;
	    }
	}
	else {
	    s = Strnew_charp(form2str((FormItemList *)a->url));
	    goto disp;
	}
    }
    parseURL2(a->url, &pu, baseURL(Currentbuf));
    s = parsedURL2Str(&pu);
  disp:
    n = searchKeyNum();
    if (n > 1 && s->length > (n - 1) * (COLS - 1))
	disp_message_nomouse(&s->ptr[(n - 1) * (COLS - 1)], TRUE);
    else
	disp_message_nomouse(&s->ptr[offset], TRUE);
}

/* peek URL */
void
peekURL(void)
{
    _peekURL(0);
}

/* peek URL of image */
void
peekIMG(void)
{
    _peekURL(1);
}

/* show current URL */
static Str
currentURL(void)
{
    if (Currentbuf->bufferprop & BP_INTERNAL)
	return Strnew_size(0);
    return parsedURL2Str(&Currentbuf->currentURL);
}

void
curURL(void)
{
    static Str s = NULL;
    static int offset = 0, n;

    if (Currentbuf->bufferprop & BP_INTERNAL)
	return;
    if (CurrentKey == prev_key && s != NULL) {
	if (s->length - offset >= COLS)
	    offset++;
	else if (s->length <= offset)	/* bug ? */
	    offset = 0;
    }
    else {
	offset = 0;
	s = currentURL();
    }
    n = searchKeyNum();
    if (n > 1 && s->length > (n - 1) * (COLS - 1))
	disp_message_nomouse(&s->ptr[(n - 1) * (COLS - 1)], TRUE);
    else
	disp_message_nomouse(&s->ptr[offset], TRUE);
}

/* view HTML source */
void
vwSrc(void)
{
    char *fn;
    Buffer *buf;

    if (Currentbuf->type == NULL || Currentbuf->bufferprop & BP_FRAME)
	return;
    if ((buf = Currentbuf->linkBuffer[LB_SOURCE]) != NULL ||
	(buf = Currentbuf->linkBuffer[LB_N_SOURCE]) != NULL) {
	Currentbuf = buf;
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if (Currentbuf->sourcefile == NULL) {
	if (Currentbuf->pagerSource &&
	    !strcasecmp(Currentbuf->type, "text/plain")) {
	    FILE *f;
	    Str tmpf = tmpfname(TMPF_SRC, NULL);
	    pushText(fileToDelete, tmpf->ptr);
	    f = fopen(tmpf->ptr, "w");
	    if (f == NULL)
		return;
	    saveBuffer(Currentbuf, f);
	    fclose(f);
	    fn = tmpf->ptr;
	}
	else {
	    return;
	}
    }
    else if (Currentbuf->real_scheme == SCM_LOCAL) {
	fn = Currentbuf->filename;
    }
    else {
	fn = Currentbuf->sourcefile;
    }
    if (!strcasecmp(Currentbuf->type, "text/html")) {
#ifdef JP_CHARSET
	char old_code = DocumentCode;
	DocumentCode = Currentbuf->document_code;
#endif
	buf = loadFile(fn);
#ifdef JP_CHARSET
	DocumentCode = old_code;
#endif
	if (buf == NULL)
	    return;
	buf->type = "text/plain";
	if (Currentbuf->real_type &&
	    !strcasecmp(Currentbuf->real_type, "text/html"))
	    buf->real_type = "text/plain";
	else
	    buf->real_type = Currentbuf->real_type;
	buf->bufferprop |= BP_SOURCE;
	buf->buffername = Sprintf("source of %s", Currentbuf->buffername)->ptr;
	buf->linkBuffer[LB_N_SOURCE] = Currentbuf;
	Currentbuf->linkBuffer[LB_SOURCE] = buf;
    }
    else if (!strcasecmp(Currentbuf->type, "text/plain")) {
	DefaultType = "text/html";
	buf = loadGeneralFile(file_to_url(fn), NULL, NO_REFERER, 0, NULL);
	DefaultType = NULL;
	if (buf == NULL || buf == NO_BUFFER)
	    return;
	if (Currentbuf->real_type &&
	    !strcasecmp(Currentbuf->real_type, "text/plain"))
	    buf->real_type = "text/html";
	else
	    buf->real_type = Currentbuf->real_type;
	if (!strcmp(buf->buffername, conv_from_system(lastFileName(fn))))
	    buf->buffername =
		Sprintf("HTML view of %s", Currentbuf->buffername)->ptr;
	buf->linkBuffer[LB_SOURCE] = Currentbuf;
	Currentbuf->linkBuffer[LB_N_SOURCE] = buf;
    }
    else {
	return;
    }
    buf->currentURL = Currentbuf->currentURL;
    buf->real_scheme = Currentbuf->real_scheme;
    buf->sourcefile = Currentbuf->sourcefile;
    buf->clone = Currentbuf->clone;
    (*buf->clone)++;
    pushBuffer(buf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* reload */
void
reload(void)
{
    Buffer *buf, *fbuf = NULL, sbuf;
#ifdef JP_CHARSET
    char old_code;
#endif
    Str url;
    FormList *request;
    int multipart;

    if (Currentbuf->bufferprop & BP_INTERNAL) {
	if (!strcmp(Currentbuf->buffername, DOWNLOAD_LIST_TITLE)) {
	    ldDL();
	    return;
	}
	disp_err_message("Can't reload...", FALSE);
	return;
    }
    if (Currentbuf->currentURL.scheme == SCM_LOCAL &&
	!strcmp(Currentbuf->currentURL.file, "-")) {
	/* file is std input */
	disp_err_message("Can't reload stdin", TRUE);
	return;
    }
    copyBuffer(&sbuf, Currentbuf);
    if (Currentbuf->bufferprop & BP_FRAME &&
	(fbuf = Currentbuf->linkBuffer[LB_N_FRAME])) {
	if (fmInitialized) {
	    message("Rendering frame", 0, 0);
	    refresh();
	}
	if (!(buf = renderFrame(fbuf, 1)))
	    return;
	if (fbuf->linkBuffer[LB_FRAME]) {
	    if (buf->sourcefile &&
		fbuf->linkBuffer[LB_FRAME]->sourcefile &&
		!strcmp(buf->sourcefile,
			fbuf->linkBuffer[LB_FRAME]->sourcefile))
		fbuf->linkBuffer[LB_FRAME]->sourcefile = NULL;
	    delBuffer(fbuf->linkBuffer[LB_FRAME]);
	}
	fbuf->linkBuffer[LB_FRAME] = buf;
	buf->linkBuffer[LB_N_FRAME] = fbuf;
	pushBuffer(buf);
	Currentbuf = buf;
	if (Currentbuf->firstLine)
	    restorePosition(Currentbuf, &sbuf);
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    else if (Currentbuf->frameset != NULL)
	fbuf = Currentbuf->linkBuffer[LB_FRAME];
    multipart = 0;
    if (Currentbuf->form_submit) {
	request = Currentbuf->form_submit->parent;
	if (request->method == FORM_METHOD_POST
	    && request->enctype == FORM_ENCTYPE_MULTIPART) {
	    Str query;
	    struct stat st;
	    multipart = 1;
	    query_from_followform(&query, Currentbuf->form_submit, multipart);
	    stat(request->body, &st);
	    request->length = st.st_size;
	}
    }
    else {
	request = NULL;
    }
    url = parsedURL2Str(&Currentbuf->currentURL);
    message("Reloading...", 0, 0);
    refresh();

#ifdef JP_CHARSET
    old_code = DocumentCode;
    DocumentCode = Currentbuf->document_code;
#endif
    SearchHeader = Currentbuf->search_header;
    DefaultType = Currentbuf->real_type;
    buf = loadGeneralFile(url->ptr, NULL, NO_REFERER, RG_NOCACHE, request);
#ifdef JP_CHARSET
    DocumentCode = old_code;
#endif
    SearchHeader = FALSE;
    DefaultType = NULL;

    if (multipart)
	unlink(request->body);
    if (buf == NULL) {
	disp_err_message("Can't reload...", FALSE);
	return;
    }
    else if (buf == NO_BUFFER) {
	return;
    }
    if (fbuf != NULL)
	Firstbuf = deleteBuffer(Firstbuf, fbuf);
    repBuffer(Currentbuf, buf);
    if ((buf->type != NULL) && (sbuf.type != NULL) &&
	((!strcasecmp(buf->type, "text/plain") &&
	  !strcasecmp(sbuf.type, "text/html")) ||
	 (!strcasecmp(buf->type, "text/html") &&
	  !strcasecmp(sbuf.type, "text/plain")))) {
	vwSrc();
	if (Currentbuf != buf)
	    Firstbuf = deleteBuffer(Firstbuf, buf);
    }
    Currentbuf->search_header = sbuf.search_header;
    Currentbuf->form_submit = sbuf.form_submit;
    if (Currentbuf->firstLine)
	restorePosition(Currentbuf, &sbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* mark URL-like patterns as anchors */
void
chkURLBuffer(Buffer *buf)
{
    static char *url_like_pat[] = {
	"https?://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./?=~_\\&+@#,\\$;]*[a-zA-Z0-9_/=]",
	"file:/[a-zA-Z0-9:%\\-\\./=_\\+@#,\\$;]*",
#ifdef USE_GOPHER
	"gopher://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./_]*",
#endif				/* USE_GOPHER */
	"ftp://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./=_+@#,\\$]*[a-zA-Z0-9_/]",
#ifdef USE_NNTP
	"news:[^<> 	][^<> 	]*",
	"nntp://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./_]*",
#endif				/* USE_NNTP */
#ifndef USE_W3MMAILER		/* see also chkExternalURIBuffer() */
	"mailto:[^<> 	][^<> 	]*@[a-zA-Z0-9][a-zA-Z0-9\\-\\._]*[a-zA-Z0-9]",
#endif
#ifdef INET6
	"https?://[a-zA-Z0-9:%\\-\\./_@]*\\[[a-fA-F0-9:][a-fA-F0-9:\\.]*\\][a-zA-Z0-9:%\\-\\./?=~_\\&+@#,\\$;]*",
	"ftp://[a-zA-Z0-9:%\\-\\./_@]*\\[[a-fA-F0-9:][a-fA-F0-9:\\.]*\\][a-zA-Z0-9:%\\-\\./=_+@#,\\$]*",
#endif				/* INET6 */
	NULL
    };
    int i;
    for (i = 0; url_like_pat[i]; i++) {
	reAnchor(buf, url_like_pat[i]);
    }
#ifdef USE_EXTERNAL_URI_LOADER
    chkExternalURIBuffer(buf);
#endif
    buf->check_url |= CHK_URL;
}

void
chkURL(void)
{
    chkURLBuffer(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
chkWORD(void)
{
    char *p;
    int spos, epos;
    p = getCurWord(Currentbuf, &spos, &epos, ":\"\'`<>");
    if (p == NULL)
	return;
    reAnchorWord(Currentbuf, Currentbuf->currentLine, spos, epos);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

#ifdef USE_NNTP
/* mark Message-ID-like patterns as NEWS anchors */
void
chkNMIDBuffer(Buffer *buf)
{
    static char *url_like_pat[] = {
	"<[^<> 	][^<> 	]*@[A-z0-9\\.\\-_]+>",
	NULL,
    };
    int i;
    for (i = 0; url_like_pat[i]; i++) {
	reAnchorNews(buf, url_like_pat[i]);
    }
    buf->check_url |= CHK_NMID;
}

void
chkNMID(void)
{
    chkNMIDBuffer(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}
#endif				/* USE_NNTP */

/* render frame */
void
rFrame(void)
{
    Buffer *buf;

    if ((buf = Currentbuf->linkBuffer[LB_FRAME]) != NULL) {
	Currentbuf = buf;
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if (Currentbuf->frameset == NULL) {
	if ((buf = Currentbuf->linkBuffer[LB_N_FRAME]) != NULL) {
	    Currentbuf = buf;
	    displayBuffer(Currentbuf, B_NORMAL);
	}
	return;
    }
    if (fmInitialized) {
	message("Rendering frame", 0, 0);
	refresh();
    }
    buf = renderFrame(Currentbuf, 0);
    if (buf == NULL)
	return;
    buf->linkBuffer[LB_N_FRAME] = Currentbuf;
    Currentbuf->linkBuffer[LB_FRAME] = buf;
    pushBuffer(buf);
    if (fmInitialized && display_ok)
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* spawn external browser */
static void
invoke_browser(char *url)
{
    Str cmd;
    char *browser = NULL;
    int bg = 0, len;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    browser = searchKeyData();
    if (browser == NULL || *browser == '\0') {
	switch (prec_num) {
	case 0:
	case 1:
	    browser = ExtBrowser;
	    break;
	case 2:
	    browser = ExtBrowser2;
	    break;
	case 3:
	    browser = ExtBrowser3;
	    break;
	}
	if (browser == NULL || *browser == '\0') {
	    browser = inputStr("Browse command: ", NULL);
	    if (browser != NULL)
		browser = conv_to_system(browser);
	}
    }
    else {
	browser = conv_to_system(browser);
    }
    if (browser == NULL || *browser == '\0')
	return;

    if ((len = strlen(browser)) >= 2 && browser[len - 1] == '&' &&
	browser[len - 2] != '\\') {
	browser = allocStr(browser, len - 2);
	bg = 1;
    }
    cmd = myExtCommand(browser, shell_quote(url), FALSE);
    Strremovetrailingspaces(cmd);
    fmTerm();
    mySystem(cmd->ptr, bg);
    fmInit();
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
extbrz()
{
    if (Currentbuf->bufferprop & BP_INTERNAL) {
	disp_err_message("Can't browse...", FALSE);
	return;
    }
    if (Currentbuf->currentURL.scheme == SCM_LOCAL &&
	!strcmp(Currentbuf->currentURL.file, "-")) {
	/* file is std input */
	disp_err_message("Can't browse stdin", TRUE);
	return;
    }
    invoke_browser(parsedURL2Str(&Currentbuf->currentURL)->ptr);
}

void
linkbrz()
{
    Anchor *a;
    ParsedURL pu;

    if (Currentbuf->firstLine == NULL)
	return;
    a = retrieveCurrentAnchor(Currentbuf);
    if (a == NULL)
	return;
    parseURL2(a->url, &pu, baseURL(Currentbuf));
    invoke_browser(parsedURL2Str(&pu)->ptr);
}

/* show current line number and number of lines in the entire document */
void
curlno()
{
    Str tmp;
    int cur = 0, all, col, len = 0;

    if (Currentbuf->currentLine != NULL) {
	Line *l = Currentbuf->currentLine;
	cur = l->real_linenumber;
	if (l->width < 0)
	    l->width = COLPOS(l, l->len);
	len = l->width;
    }
    col = Currentbuf->currentColumn + Currentbuf->cursorX + 1;
    all =
	(Currentbuf->lastLine ? Currentbuf->lastLine->
	 real_linenumber : Currentbuf->allLine);
    if (all == 0 && Currentbuf->lastLine != NULL)
	all = Currentbuf->currentLine->real_linenumber;
    if (all == 0)
	all = 1;
    if (Currentbuf->pagerSource && !(Currentbuf->bufferprop & BP_CLOSE))
	tmp = Sprintf("line %d col %d/%d", cur, col, len);
    else
	tmp = Sprintf("line %d/%d (%d%%) col %d/%d",
		      cur, all, cur * 100 / all, col, len);
#ifdef JP_CHARSET
    Strcat_charp(tmp, "  ");
    Strcat_charp(tmp, code_to_str(Currentbuf->document_code));
#endif				/* not JP_CHARSET */

    disp_message(tmp->ptr, FALSE);
}

#ifdef USE_IMAGE
void
dispI(void)
{
    if (!displayImage)
	initImage();
    if (!activeImage)
	return;
    displayImage = TRUE;
    /*
     * if (!(Currentbuf->type && !strcmp(Currentbuf->type, "text/html")))
     * return;
     */
    Currentbuf->image_flag = IMG_FLAG_AUTO;
    Currentbuf->need_reshape = TRUE;
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

void
stopI(void)
{
    if (!activeImage)
	return;
    /*
     * if (!(Currentbuf->type && !strcmp(Currentbuf->type, "text/html")))
     * return;
     */
    Currentbuf->image_flag = IMG_FLAG_SKIP;
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}
#endif

#ifdef USE_MOUSE

static int
mouse_scroll_line(void)
{
    if (relative_wheel_scroll)
	return (relative_wheel_scroll_ratio * LASTLINE + 99) / 100;
    else
	return fixed_wheel_scroll_count;
}

static TabBuffer *
posTab(int x, int y)
{
    TabBuffer *tab;

    for (tab = FirstTab; tab; tab = tab->nextTab) {
	if (tab->x1 <= x && x <= tab->x2 && tab->y == y)
	    return tab;
    }
    return NULL;
}

static void
mouse_menu_action(int btn, int x)
{
    switch (btn) {
    case MOUSE_BTN1_DOWN:
	btn = 0;
	break;
    case MOUSE_BTN2_DOWN:
	btn = 1;
	break;
    case MOUSE_BTN3_DOWN:
	btn = 2;
	break;
    default:
	return;
    }
    if (x >= 0 && x <= 9 && mouse_menu_map[btn][x].func) {
	CurrentKey = -1;
	CurrentKeyData = NULL;
	CurrentCmdData = mouse_menu_map[btn][x].data;
	(*mouse_menu_map[btn][x].func) ();
	CurrentCmdData = NULL;
    }
}

static void
process_mouse(int btn, int x, int y)
{
    int delta_x, delta_y, i;
    static int press_btn = MOUSE_BTN_RESET, press_x, press_y;
    TabBuffer *t;
    int ny = 0;

    if (nTab2 > 1)
	ny = LastTab->y + 1;
    if (btn == MOUSE_BTN_UP) {
	switch (press_btn) {
	case MOUSE_BTN1_DOWN:
	    if (ny && y < ny) {
		if (press_y == y && press_x == x) {
		    if (y == 0 && x >= COLS - 2) {
			deleteTab(CurrentTab);
			displayBuffer(Currentbuf, B_FORCE_REDRAW);
			return;
		    }
		    t = posTab(x, y);
		    if (t == NULL)
			return;
		    if (t == NO_TABBUFFER) {
			mouse_menu_action(press_btn, x);
			return;
		    }
		    CurrentTab = t;
		    displayBuffer(Currentbuf, B_FORCE_REDRAW);
		    return;
		}
		else if (press_y < ny) {
		    moveTab(posTab(press_x, press_y), posTab(x, y),
			    (press_y == y) ? (press_x < x) : (press_y < y));
		    return;
		}
		else if (press_x >= Currentbuf->rootX) {
		    Buffer *buf = Currentbuf;
		    int cx = Currentbuf->cursorX, cy = Currentbuf->cursorY;

		    t = posTab(x, y);
		    if (t == NULL)
			return;
		    if (t == NO_TABBUFFER)
			t = NULL;	/* open new tab */
		    cursorXY(Currentbuf, press_x - Currentbuf->rootX,
			     press_y - Currentbuf->rootY);
		    if (Currentbuf->cursorY == press_y - Currentbuf->rootY &&
			(Currentbuf->cursorX == press_x - Currentbuf->rootX
#ifdef JP_CHARSET
			 || (Currentbuf->currentLine != NULL &&
			     (Currentbuf->currentLine->propBuf[Currentbuf->pos]
			      & PC_KANJI1) && Currentbuf->cursorX == press_x
			     - Currentbuf->rootX - 1)
#endif
			)) {
			onA();
			followTab(t);
		    }
		    if (buf == Currentbuf)
			cursorXY(Currentbuf, cx, cy);
		}
		return;
	    }
	    if (press_x != x || press_y != y) {
		delta_x = x - press_x;
		delta_y = y - press_y;

		if (abs(delta_x) < abs(delta_y) / 3)
		    delta_x = 0;
		if (abs(delta_y) < abs(delta_x) / 3)
		    delta_y = 0;
		if (reverse_mouse) {
		    delta_y = -delta_y;
		    delta_x = -delta_x;
		}
		if (delta_y > 0) {
		    prec_num = delta_y;
		    ldown1();
		}
		else if (delta_y < 0) {
		    prec_num = -delta_y;
		    lup1();
		}
		if (delta_x > 0) {
		    prec_num = delta_x;
		    col1L();
		}
		else if (delta_x < 0) {
		    prec_num = -delta_x;
		    col1R();
		}
	    }
	    else {
		if (y == LASTLINE) {
		    switch (x) {
		    case 0:
		    case 1:
			backBf();
			break;
		    case 2:
		    case 3:
			pgBack();
			break;
		    case 4:
		    case 5:
			pgFore();
			break;
		    }
		    return;
		}
		if (y == Currentbuf->cursorY + Currentbuf->rootY &&
		    (x == Currentbuf->cursorX + Currentbuf->rootX
#ifdef JP_CHARSET
		     || (Currentbuf->currentLine != NULL &&
			 (Currentbuf->currentLine->
			  propBuf[Currentbuf->pos] & PC_KANJI1)
			 && x == Currentbuf->cursorX + Currentbuf->rootX + 1)
#endif				/* JP_CHARSET */
		    )) {
		    followA();
		    return;
		}
		if (x >= Currentbuf->rootX)
		    cursorXY(Currentbuf, x - Currentbuf->rootX,
			     y - Currentbuf->rootY);
		displayBuffer(Currentbuf, B_NORMAL);

	    }
	    break;
	case MOUSE_BTN2_DOWN:
	    if (ny && y < ny) {
		if (press_y == y && press_x == x) {
		    t = posTab(x, y);
		    if (t == NO_TABBUFFER) {
			mouse_menu_action(press_btn, x);
			return;
		    }
		    if (t) {
			deleteTab(t);
			displayBuffer(Currentbuf, B_FORCE_REDRAW);
		    }
		}
		return;
	    }
	    backBf();
	    break;
	case MOUSE_BTN3_DOWN:
	    if (nTab2 > 1 && y < ny) {
		if (press_y == y && press_x == x) {
		    t = posTab(x, y);
		    if (t == NO_TABBUFFER) {
			mouse_menu_action(press_btn, x);
			return;
		    }
		}
	    }
#ifdef USE_MENU
	    if (x >= Currentbuf->rootX && y > ny)
		cursorXY(Currentbuf, x - Currentbuf->rootX,
			 y - Currentbuf->rootY);
	    onA();
	    mainMenu(x, y);
#endif				/* USE_MENU */
	    break;
	case MOUSE_BTN4_DOWN_RXVT:
	    for (i = 0; i < mouse_scroll_line(); i++)
		ldown1();
	    break;
	case MOUSE_BTN5_DOWN_RXVT:
	    for (i = 0; i < mouse_scroll_line(); i++)
		lup1();
	    break;
	}
    }
    else if (btn == MOUSE_BTN4_DOWN_XTERM) {
	for (i = 0; i < mouse_scroll_line(); i++)
	    ldown1();
    }
    else if (btn == MOUSE_BTN5_DOWN_XTERM) {
	for (i = 0; i < mouse_scroll_line(); i++)
	    lup1();
    }

    if (btn != MOUSE_BTN4_DOWN_RXVT || press_btn == MOUSE_BTN_RESET) {
	press_btn = btn;
	press_x = x;
	press_y = y;
    }
    else {
	press_btn = MOUSE_BTN_RESET;
    }
}

void
msToggle(void)
{
    if (use_mouse) {
	use_mouse = FALSE;
    }
    else {
	use_mouse = TRUE;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
mouse()
{
    int btn, x, y;

    btn = (unsigned char)getch() - 32;
#if defined(__CYGWIN__)
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

    if (x < 0 || x >= COLS || y < 0 || y > LASTLINE)
	return;
    process_mouse(btn, x, y);
}

#ifdef USE_GPM
int
gpm_process_mouse(Gpm_Event * event, void *data)
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
    process_mouse(btn, x - 1, y - 1);
    return 0;
}
#endif				/* USE_GPM */

#ifdef USE_SYSMOUSE
int
sysm_process_mouse(int x, int y, int nbs, int obs)
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
    process_mouse(btn, x, y);
    return 0;
}
#endif				/* USE_SYSMOUSE */
#endif				/* USE_MOUSE */

void
dispVer()
{
    disp_message(Sprintf("w3m version %s", w3m_version)->ptr, FALSE);
}

void
wrapToggle(void)
{
    if (WrapSearch) {
	WrapSearch = FALSE;
	disp_message("Wrap search off", FALSE);
    }
    else {
	WrapSearch = TRUE;
	disp_message("Wrap search on", FALSE);
    }
}

static int
is_wordchar(int c, const char *badchars)
{
    if (badchars)
	return !(IS_SPACE(c) || strchr(badchars, c));
    else
	return IS_ALPHA(c);
}

static char *
getCurWord(Buffer *buf, int *spos, int *epos, const char *badchars)
{
    char *p;
    Line *l = buf->currentLine;
    int b, e;

    *spos = 0;
    *epos = 0;
    if (l == NULL)
	return NULL;
    p = l->lineBuf;
    e = buf->pos;
    while (e > 0 && !is_wordchar(p[e], badchars))
	e--;
    if (!is_wordchar(p[e], badchars))
	return NULL;
    b = e;
    while (b > 0 && is_wordchar(p[b - 1], badchars))
	b--;
    while (e < l->len && is_wordchar(p[e], badchars))
	e++;
    *spos = b;
    *epos = e;
    return &p[b];
}

static char *
GetWord(Buffer *buf)
{
    int b, e;
    char *p;

    if ((p = getCurWord(buf, &b, &e, 0)) != NULL) {
	return Strnew_charp_n(p, e - b)->ptr;
    }
    return NULL;
}

#ifdef USE_DICT
static void
execdict(char *word)
{
    char *w, *dictcmd;
    Buffer *buf;

    if (!UseDictCommand || word == NULL || *word == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    w = conv_to_system(word);
    if (*w == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    dictcmd = Sprintf("%s?%s", DictCommand,
		      Str_form_quote(Strnew_charp(w))->ptr)->ptr;
    buf = loadGeneralFile(dictcmd, NULL, NO_REFERER, 0, NULL);
    if (buf == NULL) {
	disp_message("Execution failed", FALSE);
    }
    else {
	buf->filename = w;
	buf->buffername = Sprintf("%s %s", DICTBUFFERNAME, word)->ptr;
	if (buf->type == NULL)
	    buf->type = "text/plain";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
dictword(void)
{
    execdict(inputStr("(dictionary)!", ""));
}

void
dictwordat(void)
{
    execdict(GetWord(Currentbuf));
}
#endif				/* USE_DICT */

void
set_buffer_environ(Buffer *buf)
{
    Anchor *a;
    Str s;
    ParsedURL pu;

    if (buf == NULL)
	return;
    set_environ("W3M_SOURCEFILE", buf->sourcefile);
    set_environ("W3M_FILENAME", buf->filename);
    set_environ("W3M_CURRENT_WORD", GetWord(buf));
    set_environ("W3M_TITLE", buf->buffername);
    set_environ("W3M_URL", parsedURL2Str(&buf->currentURL)->ptr);
    if (buf->real_type)
	set_environ("W3M_TYPE", buf->real_type);
    else
	set_environ("W3M_TYPE", "unknown");
#ifdef JP_CHARSET
    set_environ("W3M_CHARSET", code_to_str(buf->document_code));
#endif				/* JP_CHARSET */
    a = retrieveCurrentAnchor(buf);
    if (a == NULL) {
	set_environ("W3M_CURRENT_LINK", "");
    }
    else {
	parseURL2(a->url, &pu, baseURL(buf));
	s = parsedURL2Str(&pu);
	set_environ("W3M_CURRENT_LINK", s->ptr);
    }
    a = retrieveCurrentImg(buf);
    if (a == NULL) {
	set_environ("W3M_CURRENT_IMG", "");
    }
    else {
	parseURL2(a->url, &pu, baseURL(buf));
	s = parsedURL2Str(&pu);
	set_environ("W3M_CURRENT_IMG", s->ptr);
    }
    a = retrieveCurrentForm(buf);
    if (a == NULL) {
	set_environ("W3M_CURRENT_FORM", "");
    }
    else {
	s = Strnew_charp(form2str((FormItemList *)a->url));
	set_environ("W3M_CURRENT_FORM", s->ptr);
    }
}

char *
searchKeyData(void)
{
    char *data = NULL;

    if (CurrentKeyData != NULL && *CurrentKeyData != '\0')
	data = CurrentKeyData;
    else if (CurrentCmdData != NULL && *CurrentCmdData != '\0')
	data = CurrentCmdData;
    else if (CurrentKey >= 0)
	data = getKeyData(CurrentKey);
    CurrentKeyData = NULL;
    CurrentCmdData = NULL;
    if (data == NULL || *data == '\0')
	return NULL;
    return allocStr(data, -1);
}

static int
searchKeyNum(void)
{
    char *d;
    int n = 1;

    d = searchKeyData();
    if (d != NULL)
	n = atoi(d);
    return n * PREC_NUM;
}

void
deleteFiles()
{
    Buffer *buf;
    char *f;

    for (CurrentTab = FirstTab; CurrentTab; CurrentTab = CurrentTab->nextTab) {
	while (Firstbuf && Firstbuf != NO_BUFFER) {
	    buf = Firstbuf->nextBuffer;
	    discardBuffer(Firstbuf);
	    Firstbuf = buf;
	}
    }
    while ((f = popText(fileToDelete)) != NULL)
	unlink(f);
}

void
w3m_exit(int i)
{
#ifdef USE_MIGEMO
    init_migemo();		/* close pipe to migemo */
#endif
    stopDownload();
    deleteFiles();
#ifdef USE_SSL
    free_ssl_ctx();
#endif
    exit(i);
}

void
execCmd(void)
{
    char *data, *p;
    int cmd;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    data = searchKeyData();
    if (data == NULL || *data == '\0') {
	data = inputStrHist("command [; ...]: ", "", TextHist);
	if (data == NULL) {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    /* data: FUNC [DATA] [; FUNC [DATA] ...] */
    while (*data) {
	SKIP_BLANKS(data);
	if (*data == ';') {
	    data++;
	    continue;
	}
	p = getWord(&data);
	cmd = getFuncList(p);
	if (cmd < 0)
	    break;
	p = getQWord(&data);
	CurrentKey = -1;
	CurrentKeyData = NULL;
	CurrentCmdData = *p ? p : NULL;
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_inactive();
#endif
	w3mFuncList[cmd].func();
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_active();
#endif
	CurrentCmdData = NULL;
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

#ifdef USE_ALARM
static MySignalHandler
SigAlarm(SIGNAL_ARG)
{
    char *data;

    if (CurrentAlarm.sec > 0) {
	CurrentKey = -1;
	CurrentKeyData = NULL;
	CurrentCmdData = data = (char *)CurrentAlarm.data;
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_inactive();
#endif
	w3mFuncList[CurrentAlarm.cmd].func();
#ifdef USE_MOUSE
	if (use_mouse)
	    mouse_active();
#endif
	CurrentCmdData = NULL;
	onA();
	if (CurrentAlarm.status & AL_IMPLICIT) {
	    CurrentAlarm.buffer = Currentbuf;
	    CurrentAlarm.status = AL_IMPLICIT_DONE
		| (CurrentAlarm.status & AL_ONCE);
	}
	else if (CurrentAlarm.status & AL_IMPLICIT_DONE
		 && (CurrentAlarm.buffer != Currentbuf ||
		     CurrentAlarm.status & AL_ONCE)) {
	    setAlarmEvent(0, AL_RESTORE, FUNCNAME_nulcmd, NULL);
	}
	if (CurrentAlarm.sec > 0) {
	    signal(SIGALRM, SigAlarm);
	    alarm(CurrentAlarm.sec);
	}
    }
    SIGNAL_RETURN;
}

static void
copyAlarmEvent(AlarmEvent * src, AlarmEvent * dst)
{
    if (!src || !dst)
	return;
    dst->sec = src->sec;
    dst->cmd = src->cmd;
    dst->data = src->data;
    dst->status = src->status;
    dst->buffer = src->buffer;
}

void
setAlarm(void)
{
    char *data;
    int sec = 0, cmd = -1;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    data = searchKeyData();
    if (data == NULL || *data == '\0') {
	data = inputStrHist("(Alarm)sec command: ", "", TextHist);
	if (data == NULL) {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    if (*data != '\0') {
	sec = atoi(getWord(&data));
	if (sec > 0)
	    cmd = getFuncList(getWord(&data));
    }
    if (cmd >= 0) {
	data = getQWord(&data);
	setAlarmEvent(sec, AL_EXPLICIT, cmd, data);
	disp_message_nsec(Sprintf("%dsec %s %s", sec, w3mFuncList[cmd].id,
				  data)->ptr, FALSE, 1, FALSE, TRUE);
    }
    else {
	setAlarmEvent(0, AL_UNSET, FUNCNAME_nulcmd, NULL);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

void
setAlarmEvent(int sec, short status, int cmd, void *data)
{
    if (status == AL_RESTORE) {
	copyAlarmEvent(&PrevAlarm, &CurrentAlarm);
	PrevAlarm.sec = 0;
	PrevAlarm.status = AL_UNSET;
	return;
    }
    if (CurrentAlarm.status == AL_EXPLICIT &&
	(status == AL_IMPLICIT || status == AL_IMPLICIT_ONCE))
	copyAlarmEvent(&CurrentAlarm, &PrevAlarm);
    CurrentAlarm.sec = sec;
    CurrentAlarm.cmd = cmd;
    CurrentAlarm.data = data;
    CurrentAlarm.status = status;
    CurrentAlarm.buffer = NULL;
}
#endif

void
reinit()
{
    char *resource = searchKeyData();

    if (resource == NULL) {
	init_rc(config_filename);
	sync_with_option();
#ifdef USE_COOKIE
	initCookie();
#endif
	initKeymap(TRUE);
#ifdef USE_MOUSE
	initMouseMenu();
#endif
#ifdef USE_MENU
	initMenu();
#endif
	return;
    }

    if (!strcasecmp(resource, "CONFIG") || !strcasecmp(resource, "RC")) {
	init_rc(config_filename);
	sync_with_option();
	return;
    }

#ifdef USE_COOKIE
    if (!strcasecmp(resource, "COOKIE")) {
	initCookie();
	return;
    }
#endif

    if (!strcasecmp(resource, "KEYMAP")) {
	initKeymap(TRUE);
	return;
    }

    if (!strcasecmp(resource, "MAILCAP")) {
	initMailcap();
	return;
    }

#ifdef USE_MOUSE
    if (!strcasecmp(resource, "MOUSE_MENU")) {
	initMouseMenu();
	return;
    }
#endif

#ifdef USE_MENU
    if (!strcasecmp(resource, "MENU")) {
	initMenu();
	return;
    }
#endif

    if (!strcasecmp(resource, "MIMETYPES")) {
	initMimeTypes();
	return;
    }

#ifdef USE_EXTERNAL_URI_LOADER
    if (!strcasecmp(resource, "URIMETHODS")) {
	initURIMethods();
	return;
    }
#endif

    disp_err_message(Sprintf("Don't know how to reinitialize '%s'", resource)->
		     ptr, FALSE);
}

void
defKey(void)
{
    char *data;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    data = searchKeyData();
    if (data == NULL || *data == '\0') {
	data = inputStrHist("Key definition: ", "", TextHist);
	if (data == NULL || *data == '\0') {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
    }
    setKeymap(allocStr(data, -1), -1, TRUE);
    displayBuffer(Currentbuf, B_NORMAL);
}

TabBuffer *
newTab(void)
{
    TabBuffer *n;

    n = New(TabBuffer);
    if (n == NULL)
	return NULL;
    n->nextTab = NULL;
    n->currentBuffer = NULL;
    n->firstBuffer = NULL;
    return n;
}

static void
_newT(void)
{
    TabBuffer *tag;
    Buffer *buf;
    int i;

    tag = newTab();
    if (!tag)
	return;

    buf = newBuffer(Currentbuf->width);
    copyBuffer(buf, Currentbuf);
    buf->nextBuffer = NULL;
    for (i = 0; i < MAX_LB; i++)
	buf->linkBuffer[i] = NULL;
    (*buf->clone)++;
    tag->firstBuffer = tag->currentBuffer = buf;

    tag->nextTab = CurrentTab->nextTab;
    tag->prevTab = CurrentTab;
    if (CurrentTab->nextTab)
	CurrentTab->nextTab->prevTab = tag;
    else
	LastTab = tag;
    CurrentTab->nextTab = tag;
    CurrentTab = tag;
    nTab++;
    calcTabPos();
}

void
newT(void)
{
    _newT();
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

int
nTabLine(void)
{
    int n = nTab2;

    if (COLS - 2 > TabCols * n)
	return n;
    n = (n - 1) / ((n * TabCols - 1) / (COLS - 2) + 1) + 1;
    if (n > (COLS - 2) / TabCols)
	n = (COLS - 2) / TabCols;
    return n ? n : 1;
}

TabBuffer *
numTab(int n)
{
    TabBuffer *tab;
    int i;

    if (n == 0)
	return CurrentTab;
    if (n == 1)
	return FirstTab;
    if (nTab <= 1)
	return NULL;
    for (tab = FirstTab, i = 1; tab && i < n; tab = tab->nextTab, i++) ;
    return tab;
}

static void
calcTabPos(void)
{
    TabBuffer *tab;
    int lcol = 0, rcol = 2, col;
    int n1, n2, na, nx, ny, ix, iy;

    if (nTab <= 0)
	return;
    n1 = (COLS - rcol - lcol) / TabCols;
    if (n1 >= nTab) {
	n2 = 1;
	ny = 1;
    }
    else {
	if (n1 < 0)
	    n1 = 0;
	n2 = COLS / TabCols;
	if (n2 == 0)
	    n2 = 1;
	ny = (nTab - n1 - 1) / n2 + 2;
    }
    na = n1 + n2 * (ny - 1);
    n1 -= (na - nTab) / ny;
    if (n1 < 0)
	n1 = 0;
    na = n1 + n2 * (ny - 1);
    tab = FirstTab;
    for (iy = 0; iy < ny && tab; iy++) {
	if (iy == 0) {
	    nx = n1;
	    col = COLS - rcol - lcol;
	}
	else {
	    nx = n2 - (na - nTab + (iy - 1)) / (ny - 1);
	    col = COLS;
	}
	for (ix = 0; ix < nx && tab; ix++, tab = tab->nextTab) {
	    tab->x1 = col * ix / nx;
	    tab->x2 = col * (ix + 1) / nx - 1;
	    tab->y = iy;
	    if (iy == 0) {
		tab->x1 += lcol;
		tab->x2 += lcol;
	    }
	}
    }
}

TabBuffer *
deleteTab(TabBuffer * tab)
{
    Buffer *buf, *next;

    if (nTab <= 1)
	return FirstTab;
    if (tab->prevTab) {
	if (tab->nextTab)
	    tab->nextTab->prevTab = tab->prevTab;
	else
	    LastTab = tab->prevTab;
	tab->prevTab->nextTab = tab->nextTab;
	if (tab == CurrentTab)
	    CurrentTab = tab->prevTab;
    }
    else {			/* tab == FirstTab */
	tab->nextTab->prevTab = NULL;
	FirstTab = tab->nextTab;
	if (tab == CurrentTab)
	    CurrentTab = tab->nextTab;
    }
    nTab--;
    calcTabPos();
    buf = tab->firstBuffer;
    while (buf && buf != NO_BUFFER) {
	next = buf->nextBuffer;
	discardBuffer(buf);
	buf = next;
    }
    return FirstTab;
}

void
closeT(void)
{
    TabBuffer *tab;

    if (nTab <= 1)
	return;
    if (prec_num)
	tab = numTab(PREC_NUM);
    else
	tab = CurrentTab;
    if (tab)
	deleteTab(tab);
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

void
nextT(void)
{
    int i;

    if (nTab <= 1)
	return;
    for (i = 0; i < PREC_NUM; i++) {
	if (CurrentTab->nextTab)
	    CurrentTab = CurrentTab->nextTab;
	else
	    CurrentTab = FirstTab;
    }
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

void
prevT(void)
{
    int i;

    if (nTab <= 1)
	return;
    for (i = 0; i < PREC_NUM; i++) {
	if (CurrentTab->prevTab)
	    CurrentTab = CurrentTab->prevTab;
	else
	    CurrentTab = LastTab;
    }
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

void
followTab(TabBuffer * tab)
{
    Buffer *buf;
    Anchor *a;

#ifdef USE_IMAGE
    a = retrieveCurrentImg(Currentbuf);
    if (!(a && a->image && a->image->map))
#endif
	a = retrieveCurrentAnchor(Currentbuf);
    if (a == NULL)
	return;

    if (tab == CurrentTab) {
	check_target = FALSE;
	followA();
	check_target = TRUE;
	return;
    }
    _newT();
    buf = Currentbuf;
    check_target = FALSE;
    followA();
    check_target = TRUE;
    if (tab == NULL) {
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
    }
    else if (buf != Currentbuf) {
	/* buf <- p <- ... <- Currentbuf = c */
	Buffer *c, *p;

	c = Currentbuf;
	p = prevBuffer(c, buf);
	p->nextBuffer = NULL;
	Firstbuf = buf;
	deleteTab(CurrentTab);
	CurrentTab = tab;
	for (buf = p; buf; buf = p) {
	    p = prevBuffer(c, buf);
	    pushBuffer(buf);
	}
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
tabA(void)
{
    followTab(prec_num ? numTab(PREC_NUM) : NULL);
}

static void
tabURL0(TabBuffer * tab, char *prompt, int relative)
{
    Buffer *buf;

    if (tab == CurrentTab) {
	goURL0(prompt, relative);
	return;
    }
    _newT();
    buf = Currentbuf;
    goURL0(prompt, relative);
    if (tab == NULL) {
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
    }
    else if (buf != Currentbuf) {
	/* buf <- p <- ... <- Currentbuf = c */
	Buffer *c, *p;

	c = Currentbuf;
	p = prevBuffer(c, buf);
	p->nextBuffer = NULL;
	Firstbuf = buf;
	deleteTab(CurrentTab);
	CurrentTab = tab;
	for (buf = p; buf; buf = p) {
	    p = prevBuffer(c, buf);
	    pushBuffer(buf);
	}
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
tabURL(void)
{
    tabURL0(prec_num ? numTab(PREC_NUM) : NULL,
	    "Goto URL on new tab: ", FALSE);
}

void
tabrURL(void)
{
    tabURL0(prec_num ? numTab(PREC_NUM) : NULL,
	    "Goto relative URL on new tab: ", TRUE);
}

void
moveTab(TabBuffer * t, TabBuffer * t2, int right)
{
    if (t2 == NO_TABBUFFER)
	t2 = FirstTab;
    if (!t || !t2 || t == t2 || t == NO_TABBUFFER)
	return;
    if (t->prevTab) {
	if (t->nextTab)
	    t->nextTab->prevTab = t->prevTab;
	else
	    LastTab = t->prevTab;
	t->prevTab->nextTab = t->nextTab;
    }
    else {
	t->nextTab->prevTab = NULL;
	FirstTab = t->nextTab;
    }
    if (right) {
	t->nextTab = t2->nextTab;
	t->prevTab = t2;
	if (t2->nextTab)
	    t2->nextTab->prevTab = t;
	else
	    LastTab = t;
	t2->nextTab = t;
    }
    else {
	t->prevTab = t2->prevTab;
	t->nextTab = t2;
	if (t2->prevTab)
	    t2->prevTab->nextTab = t;
	else
	    FirstTab = t;
	t2->prevTab = t;
    }
    calcTabPos();
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
tabR(void)
{
    TabBuffer *tab;
    int i;

    for (tab = CurrentTab, i = 0; tab && i < PREC_NUM;
	 tab = tab->nextTab, i++) ;
    moveTab(CurrentTab, tab ? tab : LastTab, TRUE);
}

void
tabL(void)
{
    TabBuffer *tab;
    int i;

    for (tab = CurrentTab, i = 0; tab && i < PREC_NUM;
	 tab = tab->prevTab, i++) ;
    moveTab(CurrentTab, tab ? tab : FirstTab, FALSE);
}

void
addDownloadList(pid_t pid, char *url, char *save, char *lock, clen_t size)
{
    DownloadList *d;

    d = New(DownloadList);
    d->pid = pid;
    d->url = url;
    if (save[0] != '/' && save[0] != '~')
	save = Strnew_m_charp(CurrentDir, "/", save, NULL)->ptr;
    d->save = expandName(save);
    d->lock = lock;
    d->size = size;
    d->time = time(0);
    d->ok = FALSE;
    d->next = NULL;
    d->prev = LastDL;
    if (LastDL)
	LastDL->next = d;
    else
	FirstDL = d;
    LastDL = d;
    add_download_list = TRUE;
}

int
checkDownloadList(void)
{
    DownloadList *d;
    struct stat st;

    if (!FirstDL)
	return FALSE;
    for (d = FirstDL; d != NULL; d = d->next) {
#ifdef HAVE_LSTAT
	if (!d->ok && !lstat(d->lock, &st))
#else
	if (!d->ok && !stat(d->lock, &st))
#endif
	    return TRUE;
    }
    return FALSE;
}

static char *
convert_size3(clen_t size)
{
    Str tmp = Strnew();
    int n;

    do {
	n = size % 1000;
	size /= 1000;
	tmp = Sprintf(size ? ",%.3d%s" : "%d%s", n, tmp->ptr);
    } while (size);
    return tmp->ptr;
}

static Buffer *
DownloadListBuffer(void)
{
    DownloadList *d;
    Str src = NULL;
    struct stat st;
    time_t cur_time;
    int duration, rate, eta;
    size_t size;

    if (!FirstDL)
	return NULL;
    cur_time = time(0);
    src = Strnew_charp("<html><head><title>" DOWNLOAD_LIST_TITLE
		       "</title></head>\n<body><h1 align=center>"
		       DOWNLOAD_LIST_TITLE "</h1>\n"
		       "<form method=internal action=download><hr>\n");
    for (d = LastDL; d != NULL; d = d->prev) {
#ifdef HAVE_LSTAT
	if (lstat(d->lock, &st))
#else
	if (stat(d->lock, &st))
#endif
	    d->ok = TRUE;
	Strcat_charp(src, "<pre>\n");
	Strcat(src, Sprintf("%s\n  --&gt; %s\n  ", html_quote(d->url),
			    html_quote(conv_from_system(d->save))));
	duration = cur_time - d->time;
	if (!stat(d->save, &st)) {
	    size = st.st_size;
	    if (d->ok) {
		d->size = size;
		duration = st.st_mtime - d->time;
	    }
	}
	else
	    size = 0;
	if (d->size) {
	    int i, l = COLS - 6;
	    if (size < d->size)
		i = l * size / d->size;
	    else
		i = l;
	    l -= i;
	    while (i-- > 0)
		Strcat_char(src, '#');
	    while (l-- > 0)
		Strcat_char(src, '_');
	    Strcat_char(src, '\n');
	}
	if (!d->ok && size < d->size)
	    Strcat(src, Sprintf("  %s / %s bytes (%d%%)",
				convert_size3(size), convert_size3(d->size),
				(int)(100.0 * size / d->size)));
	else
	    Strcat(src, Sprintf("  %s bytes loaded", convert_size3(size)));
	if (duration > 0) {
	    rate = size / duration;
	    Strcat(src, Sprintf("  %02d:%02d:%02d  rate %s/sec",
				duration / (60 * 60), (duration / 60) % 60,
				duration % 60, convert_size(rate, 1)));
	    if (!d->ok && size < d->size && rate) {
		eta = (d->size - size) / rate;
		Strcat(src, Sprintf("  eta %02d:%02d:%02d", eta / (60 * 60),
				    (eta / 60) % 60, eta % 60));
	    }
	}
	Strcat_char(src, '\n');
	if (d->ok) {
	    Strcat(src, Sprintf("<input type=submit name=ok%d value=OK>",
				d->pid));
	    if (size < d->size)
		Strcat_charp(src, " Download incompleted");
	    else
		Strcat_charp(src, " Download completed");
	}
	else
	    Strcat(src, Sprintf("<input type=submit name=stop%d value=STOP>",
				d->pid));
	Strcat_charp(src, "\n</pre><hr>\n");
    }
    Strcat_charp(src, "</form></body></html>");
    return loadHTMLString(src);
}

void
download_action(struct parsed_tagarg *arg)
{
    DownloadList *d;
    pid_t pid;

    for (; arg; arg = arg->next) {
	if (!strncmp(arg->arg, "stop", 4)) {
	    pid = (pid_t) atoi(&arg->arg[4]);
	    kill(pid, SIGKILL);
	}
	else if (!strncmp(arg->arg, "ok", 2))
	    pid = (pid_t) atoi(&arg->arg[2]);
	else
	    continue;
	for (d = FirstDL; d; d = d->next) {
	    if (d->pid == pid) {
		unlink(d->lock);
		if (d->prev)
		    d->prev->next = d->next;
		else
		    FirstDL = d->next;
		if (d->next)
		    d->next->prev = d->prev;
		else
		    LastDL = d->prev;
		break;
	    }
	}
    }
    ldDL();
}

void
stopDownload(void)
{
    DownloadList *d;

    if (!FirstDL)
	return;
    for (d = FirstDL; d != NULL; d = d->next) {
	if (d->ok)
	    continue;
	kill(d->pid, SIGKILL);
	unlink(d->lock);
    }
}

/* download panel */
void
ldDL(void)
{
    Buffer *prev = Currentbuf;
    int delete = FALSE, new_tab = FALSE;
#ifdef USE_ALARM
    int reload;
#endif

    if (Currentbuf->bufferprop & BP_INTERNAL &&
	!strcmp(Currentbuf->buffername, DOWNLOAD_LIST_TITLE))
	delete = TRUE;
    if (!FirstDL) {
	if (delete) {
	    Currentbuf->bufferprop &= ~BP_RELOAD;
	    if (Currentbuf == Firstbuf && Currentbuf->nextBuffer == NULL) {
		if (nTab > 1)
		    deleteTab(CurrentTab);
	    }
	    else
		delBuffer(Currentbuf);
	    displayBuffer(Currentbuf, B_FORCE_REDRAW);
	}
	return;
    }
    if (!delete && open_tab_dl_list) {
	_newT();
	prev = Currentbuf;
	delete = TRUE;
	new_tab = TRUE;
    }
#ifdef USE_ALARM
    reload = checkDownloadList();
#endif
    cmd_loadBuffer(DownloadListBuffer(), BP_NO_URL, LB_NOLINK);
    if (Currentbuf == prev) {
	if (new_tab)
	    deleteTab(CurrentTab);
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    if (delete)
	deletePrevBuf();
#ifdef USE_ALARM
    if (reload) {
	Currentbuf->bufferprop |= BP_RELOAD;
	setAlarmEvent(1, AL_IMPLICIT, FUNCNAME_reload, NULL);
    }
#endif
}
