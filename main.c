/* $Id: main.c,v 1.270 2010/08/24 10:11:51 htrb Exp $ */
#define MAINPROGRAM
#include "fm.h"
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#if defined(HAVE_WAITPID) || defined(HAVE_WAIT3)
#include <sys/wait.h>
#endif
#include <time.h>
#if defined(__CYGWIN__) && defined(USE_BINMODE_STREAM)
#include <io.h>
#endif
#include "terms.h"
#include "myctype.h"
#include "regex.h"
#include "wc.h"
#include "wtf.h"
#include "ucs.h"


#define DSTR_LEN	256

Hist *LoadHist;
Hist *SaveHist;
Hist *URLHist;
Hist *ShellHist;
Hist *TextHist;

typedef struct _Event {
    int cmd;
    void *data;
    struct _Event *next;
} Event;
static Event *CurrentEvent = NULL;
static Event *LastEvent = NULL;

static AlarmEvent DefaultAlarm = {
    0, AL_UNSET, FUNCNAME_nulcmd, NULL
};
static AlarmEvent *CurrentAlarm = &DefaultAlarm;
static MySignalHandler SigAlarm(SIGNAL_ARG);

static int need_resize_screen = FALSE;
static MySignalHandler resize_hook(SIGNAL_ARG);
static void resize_screen(void);

static MySignalHandler SigPipe(SIGNAL_ARG);

static char *SearchString = NULL;
int (*searchRoutine) (Buffer *, char *);

JMP_BUF IntReturn;

static void delBuffer(Buffer *buf);
static void cmd_loadfile(char *path);
static void cmd_loadURL(char *url, ParsedURL *current, char *referer,
			FormList *request);
static void cmd_loadBuffer(Buffer *buf, int prop, int linkid);
static void keyPressEventProc(int c);
int show_params_p = 0;
void show_params(FILE * fp);

static char *getCurWord(Buffer *buf, int *spos, int *epos);

static int display_ok = FALSE;
static void do_dump(Buffer *);
int prec_num = 0;
int prev_key = -1;
int on_target = 1;
static int add_download_list = FALSE;

void set_buffer_environ(Buffer *);
static void save_buffer_position(Buffer *buf);

static void _followForm(int);
static void _goLine(char *);
static void _newT(void);
static void followTab(TabBuffer * tab);
static void moveTab(TabBuffer * t, TabBuffer * t2, int right);
static void _nextA(int);
static void _prevA(int);
static int check_target = TRUE;
#define PREC_NUM (prec_num ? prec_num : 1)
#define PREC_LIMIT 10000
static int searchKeyNum(void);

#define help() fusage(stdout, 0)
#define usage() fusage(stderr, 1)

int enable_inline_image;

static void
fversion(FILE * f)
{
    fprintf(f, "w3m version %s, options %s\n", w3m_version,
#if LANG == JA
	    "lang=ja"
#else
	    "lang=en"
#endif
	    ",m17n"
	    ",image"
	    ",color"
	    ",ansi-color"
	    ",menu"
	    ",cookie"
	    ",ssl"
	    ",ssl-verify"
	    ",external-uri-loader"
#ifdef INET6
	    ",ipv6"
#endif
	    ",alarm"
	);
}

static void
fusage(FILE * f, int err)
{
    fversion(f);
    /* FIXME: gettextize? */
    fprintf(f, "usage: w3m [options] [URL or filename]\noptions:\n");
    fprintf(f, "    -t tab           set tab width\n");
    fprintf(f, "    -r               ignore backspace effect\n");
    fprintf(f, "    -l line          # of preserved line (default 10000)\n");
    fprintf(f, "    -I charset       document charset\n");
    fprintf(f, "    -O charset       display/output charset\n");
    fprintf(f, "    -B               load bookmark\n");
    fprintf(f, "    -bookmark file   specify bookmark file\n");
    fprintf(f, "    -T type          specify content-type\n");
    fprintf(f, "    -m               internet message mode\n");
    fprintf(f, "    -v               visual startup mode\n");
    fprintf(f, "    -M               monochrome display\n");
    fprintf(f,
	    "    -N               open URL of command line on each new tab\n");
    fprintf(f, "    -F               automatically render frames\n");
    fprintf(f,
	    "    -cols width      specify column width (used with -dump)\n");
    fprintf(f,
	    "    -ppc count       specify the number of pixels per character (4.0...32.0)\n");
    fprintf(f,
	    "    -ppl count       specify the number of pixels per line (4.0...64.0)\n");
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
#ifdef INET6
    fprintf(f, "    -4               IPv4 only (-o dns_order=4)\n");
    fprintf(f, "    -6               IPv6 only (-o dns_order=6)\n");
#endif
    fprintf(f, "    -insecure        use insecure SSL config options\n");
    fprintf(f,
	    "    -cookie          use cookie (-no-cookie: don't use cookie)\n");
    fprintf(f, "    -graph           use DEC special graphics for border of table and menu\n");
    fprintf(f, "    -no-graph        use ASCII character for border of table and menu\n");
    fprintf(f, "    -s               squeeze multiple blank lines\n");
    fprintf(f, "    -W               toggle search wrap mode\n");
    fprintf(f, "    -X               don't use termcap init/deinit\n");
    fprintf(f,
	    "    -title[=TERM]    set buffer name to terminal title string\n");
    fprintf(f, "    -o opt=value     assign value to config option\n");
    fprintf(f, "    -show-option     print all config options\n");
    fprintf(f, "    -config file     specify config file\n");
    fprintf(f, "    -debug           use debug mode (only for debugging)\n");
    fprintf(f, "    -reqlog          write request logfile\n");
    fprintf(f, "    -help            print this usage message\n");
    fprintf(f, "    -version         print w3m version\n");
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

		printf(msg_ring[i].msg,	(unsigned long)msg_ring[i].arg);
		sleep_till_anykey(1, 1);
	    }

	    lock = 0;
	}
    }
    else if (orig_GC_warn_proc)
	orig_GC_warn_proc(msg, arg);
    else
	fprintf(stderr, msg, (unsigned long)arg);
}

static void
sig_chld(int signo)
{
    int p_stat;
    pid_t pid;

#ifdef HAVE_WAITPID
    while ((pid = waitpid(-1, &p_stat, WNOHANG)) > 0)
#elif HAVE_WAIT3
    while ((pid = wait3(&p_stat, WNOHANG, NULL)) > 0)
#else
    if ((pid = wait(&p_stat)) > 0)
#endif
    {
	DownloadList *d;

	if (WIFEXITED(p_stat)) {
	    for (d = FirstDL; d != NULL; d = d->next) {
		if (d->pid == pid) {
		    d->err = WEXITSTATUS(p_stat);
		    break;
		}
	    }
	}
    }
    mySignal(SIGCHLD, sig_chld);
    return;
}

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
    if (!Strcasecmp_charp(hs, "user-agent"))
	override_user_agent = TRUE;
    Strcat_charp(hs, ": ");
    if (*(++p)) {		/* not null header */
	SKIP_BLANKS(p);		/* skip white spaces */
	Strcat_charp(hs, p);
    }
    Strcat_charp(hs, "\r\n");
    return hs;
}

static void *
die_oom(size_t bytes)
{
    fprintf(stderr, "Out of memory: %lu bytes unavailable!\n", (unsigned long)bytes);
    exit(1);
}

int
main(int argc, char **argv, char **envp)
{
    Buffer *newbuf = NULL;
    char *p;
    int c, i;
    InputStream redin;
    char *line_str = NULL;
    char **load_argv;
    FormList *request;
    int load_argc = 0;
    int load_bookmark = FALSE;
    int visual_start = FALSE;
    int open_new_tab = FALSE;
    char search_header = FALSE;
    char *default_type = NULL;
    char *post_file = NULL;
    Str err_msg;
    char *Locale = NULL;
    wc_uint8 auto_detect;
#if defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE)
    char **getimage_args = NULL;
#endif /* defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE) */
    if (!getenv("GC_LARGE_ALLOC_WARN_INTERVAL"))
	set_environ("GC_LARGE_ALLOC_WARN_INTERVAL", "30000");
    GC_INIT();
#if (GC_VERSION_MAJOR>7) || ((GC_VERSION_MAJOR==7) && (GC_VERSION_MINOR>=2))
    GC_set_oom_fn(die_oom);
#else
    GC_oom_fn = die_oom;
#endif
#if defined(ENABLE_NLS) || (defined(USE_M17N) && defined(HAVE_LANGINFO_CODESET))
    setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    NO_proxy_domains = newTextList();
    fileToDelete = newTextList();

    load_argv = New_N(char *, argc - 1);
    load_argc = 0;

    CurrentDir = currentdir();
    CurrentPid = (int)getpid();
#if defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE)
    if (argv[0] && *argv[0])
	MyProgramName = argv[0];
#endif /* defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE) */
    BookmarkFile = NULL;
    config_file = NULL;

    {
	char hostname[HOST_NAME_MAX + 2];
	if (gethostname(hostname, HOST_NAME_MAX + 2) == 0) {
	    size_t hostname_len;
	    /* Don't use hostname if it is truncated.  */
	    hostname[HOST_NAME_MAX + 1] = '\0';
	    hostname_len = strlen(hostname);
	    if (hostname_len <= HOST_NAME_MAX && hostname_len < STR_SIZE_MAX)
		HostName = allocStr(hostname, (int)hostname_len);
	}
    }

    /* argument search 1 */
    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') {
	    if (!strcmp("-config", argv[i])) {
		argv[i] = "-dummy";
		if (++i >= argc)
		    usage();
		config_file = argv[i];
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

    if (non_null(Locale = getenv("LC_ALL")) ||
	non_null(Locale = getenv("LC_CTYPE")) ||
	non_null(Locale = getenv("LANG"))) {
	DisplayCharset = wc_guess_locale_charset(Locale, DisplayCharset);
	DocumentCharset = wc_guess_locale_charset(Locale, DocumentCharset);
	SystemCharset = wc_guess_locale_charset(Locale, SystemCharset);
    }

    /* initializations */
    init_rc();

    LoadHist = newHist();
    SaveHist = newHist();
    ShellHist = newHist();
    TextHist = newHist();
    URLHist = newHist();

    if (FollowLocale && Locale) {
	DisplayCharset = wc_guess_locale_charset(Locale, DisplayCharset);
	SystemCharset = wc_guess_locale_charset(Locale, SystemCharset);
    }
    auto_detect = WcOption.auto_detect;
    BookmarkCharset = DocumentCharset;

    if (!non_null(HTTP_proxy) &&
	((p = getenv("HTTP_PROXY")) ||
	 (p = getenv("http_proxy")) || (p = getenv("HTTP_proxy"))))
	HTTP_proxy = p;
    if (!non_null(HTTPS_proxy) &&
	((p = getenv("HTTPS_PROXY")) ||
	 (p = getenv("https_proxy")) || (p = getenv("HTTPS_proxy"))))
	HTTPS_proxy = p;
    if (HTTPS_proxy == NULL && non_null(HTTP_proxy))
	HTTPS_proxy = HTTP_proxy;
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
	    else if (!strncmp("-I", argv[i], 2)) {
		if (argv[i][2] != '\0')
		    p = argv[i] + 2;
		else {
		    if (++i >= argc)
			usage();
		    p = argv[i];
		}
		DocumentCharset = wc_guess_charset_short(p, DocumentCharset);
		WcOption.auto_detect = WC_OPT_DETECT_OFF;
		UseContentCharset = FALSE;
	    }
	    else if (!strncmp("-O", argv[i], 2)) {
		if (argv[i][2] != '\0')
		    p = argv[i] + 2;
		else {
		    if (++i >= argc)
			usage();
		    p = argv[i];
		}
		DisplayCharset = wc_guess_charset_short(p, DisplayCharset);
	    }
	    else if (!strcmp("-graph", argv[i]))
		UseGraphicChar = GRAPHIC_CHAR_DEC;
	    else if (!strcmp("-no-graph", argv[i]))
		UseGraphicChar = GRAPHIC_CHAR_ASCII;
	    else if (!strcmp("-T", argv[i])) {
		if (++i >= argc)
		    usage();
		DefaultType = default_type = argv[i];
	    }
	    else if (!strcmp("-m", argv[i]))
		SearchHeader = search_header = TRUE;
	    else if (!strcmp("-v", argv[i]))
		visual_start = TRUE;
	    else if (!strcmp("-N", argv[i]))
		open_new_tab = TRUE;
	    else if (!strcmp("-M", argv[i]))
		useColor = FALSE;
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
		if (COLS > MAXIMUM_COLS) {
		    COLS = MAXIMUM_COLS;
		}
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
	    else if (!strcmp("-ri", argv[i])) {
	        enable_inline_image = INLINE_IMG_OSC5379;
	    }
	    else if (!strcmp("-sixel", argv[i])) {
		enable_inline_image = INLINE_IMG_SIXEL;
	    }
	    else if (!strcmp("-num", argv[i]))
		showLineNum = TRUE;
	    else if (!strcmp("-no-proxy", argv[i]))
		use_proxy = FALSE;
#ifdef INET6
	    else if (!strcmp("-4", argv[i]) || !strcmp("-6", argv[i]))
		set_param_option(Sprintf("dns_order=%c", argv[i][1])->ptr);
#endif
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
	    else if (!strcmp("-no-cookie", argv[i])) {
		use_cookie = FALSE;
		accept_cookie = FALSE;
	    }
	    else if (!strcmp("-cookie", argv[i])) {
		use_cookie = TRUE;
		accept_cookie = TRUE;
	    }
	    else if (!strcmp("-s", argv[i]))
		squeezeBlankLine = TRUE;
	    else if (!strcmp("-X", argv[i]))
		Do_not_use_ti_te = TRUE;
	    else if (!strcmp("-title", argv[i]))
		displayTitleTerm = getenv("TERM");
	    else if (!strncmp("-title=", argv[i], 7))
		displayTitleTerm = argv[i] + 7;
	    else if (!strcmp("-insecure", argv[i])) {
#ifdef OPENSSL_TLS_SECURITY_LEVEL
		set_param_option("ssl_cipher=ALL:eNULL:@SECLEVEL=0");
#else
		set_param_option("ssl_cipher=ALL:eNULL");
#endif
#ifdef SSL_CTX_set_min_proto_version
		set_param_option("ssl_min_version=all");
#endif
		set_param_option("ssl_forbid_method=");
		set_param_option("ssl_verify_server=0");
	    }
	    else if (!strcmp("-o", argv[i]) ||
		     !strcmp("-show-option", argv[i])) {
		if (!strcmp("-show-option", argv[i]) || ++i >= argc ||
		    !strcmp(argv[i], "?")) {
		    show_params(stdout);
		    exit(0);
		}
		if (!set_param_option(argv[i])) {
		    /* option set failed */
		    /* FIXME: gettextize? */
		    fprintf(stderr, "%s: bad option\n", argv[i]);
		    show_params_p = 1;
		    usage();
		}
	    }
	    else if (!strcmp("-", argv[i]) || !strcmp("-dummy", argv[i])) {
		/* do nothing */
	    }
	    else if (!strcmp("-debug", argv[i])) {
		w3m_debug = TRUE;
	    }
	    else if (!strcmp("-reqlog",argv[i])) {
		w3m_reqlog=rcFile("request.log");
	    }
#if defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE)
	    else if (!strcmp("-$$getimage", argv[i])) {
		++i;
		getimage_args = argv + i;
		i += 4;
		if (i > argc)
		    usage();
	    }
#endif /* defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE) */
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
	    COLS = DEFAULT_COLS;
    }

    if (!w3m_dump && !w3m_backend) {
	fmInit();
	mySignal(SIGWINCH, resize_hook);
    }
    else if (w3m_halfdump && displayImage)
	activeImage = TRUE;

    sync_with_option();
    initCookie();
    if (UseHistory)
	loadHistory(URLHist);

    wtf_init(DocumentCharset, DisplayCharset);
    /*  if (w3m_dump)
     *    WcOption.pre_conv = WC_TRUE;
     */

    if (w3m_backend)
	backend();
#if defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE)
    if (getimage_args) {
	char *image_url = conv_from_system(getimage_args[0]);
	char *base_url = conv_from_system(getimage_args[1]);
	ParsedURL base_pu;

	parseURL2(base_url, &base_pu, NULL);
	image_source = getimage_args[2];
	newbuf = loadGeneralFile(image_url, &base_pu, NULL, 0, NULL);
	if (!newbuf || !newbuf->real_type ||
	    strncasecmp(newbuf->real_type, "image/", 6))
	    unlink(getimage_args[2]);
#if defined(HAVE_SYMLINK) && defined(HAVE_LSTAT)
	symlink(getimage_args[2], getimage_args[3]);
#else
	{
	    FILE *f = fopen(getimage_args[3], "w");
	    if (f)
		fclose(f);
	}
#endif
	w3m_exit(0);
    }
#endif /* defined(DONT_CALL_GC_AFTER_FORK) && defined(USE_IMAGE) */

    if (w3m_dump)
	mySignal(SIGINT, SIG_IGN);
    mySignal(SIGCHLD, sig_chld);
    mySignal(SIGPIPE, SigPipe);

#if (GC_VERSION_MAJOR>7) || ((GC_VERSION_MAJOR==7) && (GC_VERSION_MINOR>=2))
    orig_GC_warn_proc = GC_get_warn_proc();
    GC_set_warn_proc(wrap_GC_warn_proc);
#else
    orig_GC_warn_proc = GC_set_warn_proc(wrap_GC_warn_proc);
#endif
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
	    /* FIXME: gettextize? */
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
	    char *url;

	    url = load_argv[i];
	    if (getURLScheme(&url) == SCM_MISSING && !ArgvIsURL)
		url = file_to_url(load_argv[i]);
	    else
		url = url_encode(conv_from_system(load_argv[i]), NULL, 0);
	    if (w3m_dump == DUMP_HEAD) {
		request = New(FormList);
		request->method = FORM_METHOD_HEAD;
		newbuf = loadGeneralFile(url, NULL, NO_REFERER, 0, request);
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
			/* FIXME: gettextize? */
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
		newbuf = loadGeneralFile(url, NULL, NO_REFERER, 0, request);
	    }
	    if (newbuf == NULL) {
		/* FIXME: gettextize? */
		Strcat(err_msg,
		       Sprintf("w3m: Can't load %s.\n", load_argv[i]));
		continue;
	    }
	    else if (newbuf == NO_BUFFER)
		continue;
	    switch (newbuf->real_scheme) {
	    case SCM_MAILTO:
		break;
	    case SCM_LOCAL:
	    case SCM_LOCAL_CGI:
		unshiftHist(LoadHist, url);
	    default:
		pushHashHist(URLHist, parsedURL2Str(&newbuf->currentURL)->ptr);
		break;
	    }
	}
	else if (newbuf == NO_BUFFER)
	    continue;
	if (newbuf->pagerSource ||
	    (newbuf->real_scheme == SCM_LOCAL && newbuf->header_source &&
	     newbuf->currentURL.file && strcmp(newbuf->currentURL.file, "-")))
	    newbuf->search_header = search_header;
	if (CurrentTab == NULL) {
	    FirstTab = LastTab = CurrentTab = newTab();
	    nTab = 1;
	    Firstbuf = Currentbuf = newbuf;
	}
	else if (open_new_tab) {
	    _newT();
	    Currentbuf->nextBuffer = newbuf;
	    delBuffer(Currentbuf);
	}
	else {
	    Currentbuf->nextBuffer = newbuf;
	    Currentbuf = newbuf;
	}
	if (!w3m_dump || w3m_dump == DUMP_BUFFER) {
	    if (Currentbuf->frameset != NULL && RenderFrame)
		rFrame();
	}
	if (w3m_dump)
	    do_dump(Currentbuf);
	else {
	    Currentbuf = newbuf;
	    saveBufferInfo();
	}
    }
    if (w3m_dump) {
	if (err_msg->length)
	    fprintf(stderr, "%s", err_msg->ptr);
	save_cookies();
	w3m_exit(0);
    }

    if (add_download_list) {
	add_download_list = FALSE;
	CurrentTab = LastTab;
	if (!FirstTab) {
	    FirstTab = LastTab = CurrentTab = newTab();
	    nTab = 1;
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
    else
	CurrentTab = FirstTab;
    if (!FirstTab || !Firstbuf || Firstbuf == NO_BUFFER) {
	if (newbuf == NO_BUFFER) {
	    if (fmInitialized)
		/* FIXME: gettextize? */
		inputChar("Hit any key to quit w3m:");
	}
	if (fmInitialized)
	    fmTerm();
	if (err_msg->length)
	    fprintf(stderr, "%s", err_msg->ptr);
	if (newbuf == NO_BUFFER) {
	    save_cookies();
	    if (!err_msg->length)
		w3m_exit(0);
	}
	w3m_exit(2);
    }
    if (err_msg->length)
	disp_message_nsec(err_msg->ptr, FALSE, 1, TRUE, FALSE);

    SearchHeader = FALSE;
    DefaultType = NULL;
    UseContentCharset = TRUE;
    WcOption.auto_detect = auto_detect;

    Currentbuf = Firstbuf;
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    if (line_str) {
	_goLine(line_str);
    }
    for (;;) {
	if (add_download_list) {
	    add_download_list = FALSE;
	    ldDL();
	}
	if (Currentbuf->submit) {
	    Anchor *a = Currentbuf->submit;
	    Currentbuf->submit = NULL;
	    gotoLine(Currentbuf, a->start.line);
	    Currentbuf->pos = a->start.pos;
	    _followForm(TRUE);
	    continue;
	}
	/* event processing */
	if (CurrentEvent) {
	    CurrentKey = -1;
	    CurrentKeyData = NULL;
	    CurrentCmdData = (char *)CurrentEvent->data;
	    w3mFuncList[CurrentEvent->cmd].func();
	    CurrentCmdData = NULL;
	    CurrentEvent = CurrentEvent->next;
	    continue;
	}
	/* get keypress event */
	if (Currentbuf->event) {
	    if (Currentbuf->event->status != AL_UNSET) {
		CurrentAlarm = Currentbuf->event;
		if (CurrentAlarm->sec == 0) {	/* refresh (0sec) */
		    Currentbuf->event = NULL;
		    CurrentKey = -1;
		    CurrentKeyData = NULL;
		    CurrentCmdData = (char *)CurrentAlarm->data;
		    w3mFuncList[CurrentAlarm->cmd].func();
		    CurrentCmdData = NULL;
		    continue;
		}
	    }
	    else
		Currentbuf->event = NULL;
	}
	if (!Currentbuf->event)
	    CurrentAlarm = &DefaultAlarm;
	if (CurrentAlarm->sec > 0) {
	    mySignal(SIGALRM, SigAlarm);
	    alarm(CurrentAlarm->sec);
	}
	mySignal(SIGWINCH, resize_hook);
	if (activeImage && displayImage && Currentbuf->img &&
	    !Currentbuf->image_loaded) {
	    do {
		if (need_resize_screen)
		    resize_screen();
		loadImage(Currentbuf, IMG_FLAG_NEXT);
	    } while (sleep_till_anykey(1, 0) <= 0);
	}
	else
	{
	    do {
		if (need_resize_screen)
		    resize_screen();
	    } while (sleep_till_anykey(1, 0) <= 0);
	}
	c = getch();
	if (CurrentAlarm->sec > 0) {
	    alarm(0);
	}
	if (IS_ASCII(c)) {	/* Ascii */
	    if (('0' <= c) && (c <= '9') &&
		(prec_num || (GlobalKeymap[c] == FUNCNAME_nulcmd))) {
		prec_num = prec_num * 10 + (int)(c - '0');
		if (prec_num > PREC_LIMIT)
		   prec_num = PREC_LIMIT;
	    }
	    else {
		set_buffer_environ(Currentbuf);
		save_buffer_position(Currentbuf);
		keyPressEventProc((int)c);
		prec_num = 0;
	    }
	}
	prev_key = CurrentKey;
	CurrentKey = -1;
	CurrentKeyData = NULL;
    }
}

static void
keyPressEventProc(int c)
{
    CurrentKey = c;
    w3mFuncList[(int)GlobalKeymap[c]].func();
}

void
pushEvent(int cmd, void *data)
{
    Event *event;

    event = New(Event);
    event->cmd = cmd;
    event->data = data;
    event->next = NULL;
    if (CurrentEvent)
	LastEvent->next = event;
    else
	CurrentEvent = event;
    LastEvent = event;
}

static void
dump_source(Buffer *buf)
{
    FILE *f;
    int c;
    if (buf->sourcefile == NULL)
	return;
    f = fopen(buf->sourcefile, "r");
    if (f == NULL)
	return;
    while ((c = fgetc(f)) != EOF) {
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
	printf("%s",
	       wc_conv_strict(ti->ptr, InnerCharset,
			      buf->document_charset)->ptr);
    }
    puts("");
}

static void
dump_extra(Buffer *buf)
{
    printf("W3m-current-url: %s\n", parsedURL2Str(&buf->currentURL)->ptr);
    if (buf->baseURL)
	printf("W3m-base-url: %s\n", parsedURL2Str(buf->baseURL)->ptr);
    printf("W3m-document-charset: %s\n",
	   wc_ces_to_charset(buf->document_charset));
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
}

static int
cmp_anchor_hseq(const void *a, const void *b)
{
    return (*((const Anchor **) a))->hseq - (*((const Anchor **) b))->hseq;
}

static void
do_dump(Buffer *buf)
{
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

    prevtrap = mySignal(SIGINT, intTrap);
    if (SETJMP(IntReturn) != 0) {
	mySignal(SIGINT, prevtrap);
	return;
    }
    if (w3m_dump & DUMP_EXTRA)
	dump_extra(buf);
    if (w3m_dump & DUMP_HEAD)
	dump_head(buf);
    if (w3m_dump & DUMP_SOURCE)
	dump_source(buf);
    if (w3m_dump == DUMP_BUFFER) {
	int i;
	saveBuffer(buf, stdout, FALSE);
	if (displayLinkNumber && buf->href) {
	    int nanchor = buf->href->nanchor;
	    printf("\nReferences:\n\n");
	    Anchor **in_order = New_N(Anchor *, buf->href->nanchor);
	    for (i = 0; i < nanchor; i++)
		in_order[i] = buf->href->anchors + i;
	    qsort(in_order, nanchor, sizeof(Anchor *), cmp_anchor_hseq);
	    for (i = 0; i < nanchor; i++) {
		ParsedURL pu;
		char *url;
		if (in_order[i]->slave)
		    continue;
		parseURL2(in_order[i]->url, &pu, baseURL(buf));
		url = url_decode2(parsedURL2Str(&pu)->ptr, Currentbuf);
		printf("[%d] %s\n", in_order[i]->hseq + 1, url);
	    }
	}
    }
    mySignal(SIGINT, prevtrap);
}

DEFUN(nulcmd, NOTHING NULL @@@, "Do nothing")
{				/* do nothing */
}

void
pcmap(void)
{
}

static void
escKeyProc(int c, int esc, unsigned char *map)
{
    if (CurrentKey >= 0 && CurrentKey & K_MULTI) {
	unsigned char **mmap;
	mmap = (unsigned char **)getKeyData(MULTI_KEY(CurrentKey));
	if (!mmap)
	    return;
	switch (esc) {
	case K_ESCD:
	    map = mmap[3];
	    break;
	case K_ESCB:
	    map = mmap[2];
	    break;
	case K_ESC:
	    map = mmap[1];
	    break;
	default:
	    map = mmap[0];
	    break;
	}
	esc |= (CurrentKey & ~0xFFFF);
    }
    CurrentKey = esc | c;
    w3mFuncList[(int)map[c]].func();
}

DEFUN(escmap, ESCMAP, "ESC map")
{
    char c;
    c = getch();
    if (IS_ASCII(c))
	escKeyProc((int)c, K_ESC, EscKeymap);
}

DEFUN(escbmap, ESCBMAP, "ESC [ map")
{
    char c;
    c = getch();
    if (IS_DIGIT(c)) {
	escdmap(c);
	return;
    }
    if (IS_ASCII(c))
	escKeyProc((int)c, K_ESCB, EscBKeymap);
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
    if (c == '~')
	escKeyProc((int)d, K_ESCD, EscDKeymap);
}

DEFUN(multimap, MULTIMAP, "multimap")
{
    char c;
    c = getch();
    if (IS_ASCII(c)) {
	CurrentKey = K_MULTI | (CurrentKey << 16) | c;
	escKeyProc((int)c, 0, NULL);
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

    deleteImage(Currentbuf);
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
    saveBufferInfo();

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

static MySignalHandler
resize_hook(SIGNAL_ARG)
{
    need_resize_screen = TRUE;
    mySignal(SIGWINCH, resize_hook);
    SIGNAL_RETURN;
}

static void
resize_screen(void)
{
    need_resize_screen = FALSE;
    setlinescols();
    setupscreen();
    if (CurrentTab)
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static MySignalHandler
SigPipe(SIGNAL_ARG)
{
    mySignal(SIGPIPE, SigPipe);
    SIGNAL_RETURN;
}

/*
 * Command functions: These functions are called with a keystroke.
 */

static void
nscroll(int n, int mode)
{
    Buffer *buf = Currentbuf;
    Line *top = buf->topLine, *cur = buf->currentLine;
    int lnum, tlnum, llnum, diff_n;

    if (buf->firstLine == NULL)
	return;
    lnum = cur->linenumber;
    buf->topLine = lineSkip(buf, top, n, FALSE);
    if (buf->topLine == top) {
	lnum += n;
	if (lnum < buf->topLine->linenumber)
	    lnum = buf->topLine->linenumber;
	else if (lnum > buf->lastLine->linenumber)
	    lnum = buf->lastLine->linenumber;
    }
    else {
	tlnum = buf->topLine->linenumber;
	llnum = buf->topLine->linenumber + buf->LINES - 1;
	if (nextpage_topline)
	    diff_n = 0;
	else
	    diff_n = n - (tlnum - top->linenumber);
	if (lnum < tlnum)
	    lnum = tlnum + diff_n;
	if (lnum > llnum)
	    lnum = llnum + diff_n;
    }
    gotoLine(buf, lnum);
    arrangeLine(buf);
    if (n > 0) {
	if (buf->currentLine->bpos &&
	    buf->currentLine->bwidth >= buf->currentColumn + buf->visualpos)
	    cursorDown(buf, 1);
	else {
	    while (buf->currentLine->next && buf->currentLine->next->bpos &&
		   buf->currentLine->bwidth + buf->currentLine->width <
		   buf->currentColumn + buf->visualpos)
		cursorDown0(buf, 1);
	}
    }
    else {
	if (buf->currentLine->bwidth + buf->currentLine->width <
	    buf->currentColumn + buf->visualpos)
	    cursorUp(buf, 1);
	else {
	    while (buf->currentLine->prev && buf->currentLine->bpos &&
		   buf->currentLine->bwidth >=
		   buf->currentColumn + buf->visualpos)
		cursorUp0(buf, 1);
	}
    }
    displayBuffer(buf, mode);
}

/* Move page forward */
DEFUN(pgFore, NEXT_PAGE, "Scroll down one page")
{
    if (vi_prec_num)
	nscroll(searchKeyNum() * (Currentbuf->LINES - 1), B_NORMAL);
    else
	nscroll(prec_num ? searchKeyNum() : searchKeyNum()
		* (Currentbuf->LINES - 1), prec_num ? B_SCROLL : B_NORMAL);
}

/* Move page backward */
DEFUN(pgBack, PREV_PAGE, "Scroll up one page")
{
    if (vi_prec_num)
	nscroll(-searchKeyNum() * (Currentbuf->LINES - 1), B_NORMAL);
    else
	nscroll(-(prec_num ? searchKeyNum() : searchKeyNum()
		  * (Currentbuf->LINES - 1)), prec_num ? B_SCROLL : B_NORMAL);
}

/* Move half page forward */
DEFUN(hpgFore, NEXT_HALF_PAGE, "Scroll down half a page")
{
	nscroll(searchKeyNum() * (Currentbuf->LINES / 2 - 1), B_NORMAL);
}

/* Move half page backward */
DEFUN(hpgBack, PREV_HALF_PAGE, "Scroll up half a page")
{
	nscroll(-searchKeyNum() * (Currentbuf->LINES / 2 - 1), B_NORMAL);
}

/* 1 line up */
DEFUN(lup1, UP, "Scroll the screen up one line")
{
    nscroll(searchKeyNum(), B_SCROLL);
}

/* 1 line down */
DEFUN(ldown1, DOWN, "Scroll the screen down one line")
{
    nscroll(-searchKeyNum(), B_SCROLL);
}

/* move cursor position to the center of screen */
DEFUN(ctrCsrV, CENTER_V, "Center on cursor line")
{
    int offsety;
    if (Currentbuf->firstLine == NULL)
	return;
    offsety = Currentbuf->LINES / 2 - Currentbuf->cursorY;
    if (offsety != 0) {
	Currentbuf->topLine =
	    lineSkip(Currentbuf, Currentbuf->topLine, -offsety, FALSE);
	arrangeLine(Currentbuf);
	displayBuffer(Currentbuf, B_NORMAL);
    }
}

DEFUN(ctrCsrH, CENTER_H, "Center on cursor column")
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
DEFUN(rdrwSc, REDRAW, "Draw the screen anew")
{
    clear();
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
clear_mark(Line *l)
{
    int pos;
    if (!l)
	return;
    for (pos = 0; pos < l->size; pos++)
	l->propBuf[pos] &= ~PE_MARK;
}

/* search by regular expression */
static int
srchcore(char *volatile str, int (*func) (Buffer *, char *))
{
    MySignalHandler(*prevtrap) ();
    volatile int i, result = SR_NOTFOUND;

    if (str != NULL && str != SearchString)
	SearchString = str;
    if (SearchString == NULL || *SearchString == '\0')
	return SR_NOTFOUND;

    str = conv_search_string(SearchString, DisplayCharset);
    prevtrap = mySignal(SIGINT, intTrap);
    crmode();
    if (SETJMP(IntReturn) == 0) {
	for (i = 0; i < PREC_NUM; i++) {
	    result = func(Currentbuf, str);
	    if (i < PREC_NUM - 1 && result & SR_FOUND)
		clear_mark(Currentbuf->currentLine);
	}
    }
    mySignal(SIGINT, prevtrap);
    term_raw();
    return result;
}

static void
disp_srchresult(int result, char *prompt, char *str)
{
    if (str == NULL)
	str = "";
    if (result & SR_NOTFOUND)
	disp_message(Sprintf("Not found: %s", str)->ptr, TRUE);
    else if (result & SR_WRAPPED)
	disp_message(Sprintf("Search wrapped: %s", str)->ptr, TRUE);
    else if (show_srch_str)
	disp_message(Sprintf("%s%s", prompt, str)->ptr, TRUE);
}

static int
dispincsrch(int ch, Str buf, Lineprop *prop)
{
    static Buffer sbuf;
    static Line *currentLine;
    static int pos;
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


    default:
	if (ch >= 0)
	    return ch;		/* use InputKeymap */
    }

    if (do_next_search) {
	if (*str) {
	    if (searchRoutine == forwardSearch)
		Currentbuf->pos += 1;
	    SAVE_BUFPOSITION(&sbuf);
	    if (srchcore(str, searchRoutine) == SR_NOTFOUND
		&& searchRoutine == forwardSearch) {
		Currentbuf->pos -= 1;
		SAVE_BUFPOSITION(&sbuf);
	    }
	    arrangeCursor(Currentbuf);
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
}

void
srch(int (*func) (Buffer *, char *), char *prompt)
{
    char *str;
    int result;
    int disp = FALSE;
    int pos;

    str = searchKeyData();
    if (str == NULL || *str == '\0') {
	str = inputStrHist(prompt, NULL, TextHist);
	if (str != NULL && *str == '\0')
	    str = SearchString;
	if (str == NULL) {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
	disp = TRUE;
    }
    pos = Currentbuf->pos;
    if (func == forwardSearch)
	Currentbuf->pos += 1;
    result = srchcore(str, func);
    if (result & SR_FOUND)
	clear_mark(Currentbuf->currentLine);
    else
	Currentbuf->pos = pos;
    displayBuffer(Currentbuf, B_NORMAL);
    if (disp)
	disp_srchresult(result, prompt, str);
    searchRoutine = func;
}

/* Search regular expression forward */

DEFUN(srchfor, SEARCH SEARCH_FORE WHEREIS, "Search forward")
{
    srch(forwardSearch, "Forward: ");
}

DEFUN(isrchfor, ISEARCH, "Incremental search forward")
{
    isrch(forwardSearch, "I-search: ");
}

/* Search regular expression backward */

DEFUN(srchbak, SEARCH_BACK, "Search backward")
{
    srch(backwardSearch, "Backward: ");
}

DEFUN(isrchbak, ISEARCH_BACK, "Incremental search backward")
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
	/* FIXME: gettextize? */
	disp_message("No previous regular expression", TRUE);
	return;
    }
    if (reverse != 0)
	reverse = 1;
    if (searchRoutine == backwardSearch)
	reverse ^= 1;
    if (reverse == 0)
	Currentbuf->pos += 1;
    result = srchcore(SearchString, routine[reverse]);
    if (result & SR_FOUND)
	clear_mark(Currentbuf->currentLine);
    else {
	if (reverse == 0)
	    Currentbuf->pos -= 1;
    }
    displayBuffer(Currentbuf, B_NORMAL);
    disp_srchresult(result, (reverse ? "Backward: " : "Forward: "),
		    SearchString);
}

/* Search next matching */
DEFUN(srchnxt, SEARCH_NEXT, "Continue search forward")
{
    srch_nxtprv(0);
}

/* Search previous matching */
DEFUN(srchprv, SEARCH_PREV, "Continue search backward")
{
    srch_nxtprv(1);
}

static void
shiftvisualpos(Buffer *buf, int shift)
{
    Line *l = buf->currentLine;
    buf->visualpos -= shift;
    if (buf->visualpos - l->bwidth >= buf->COLS)
	buf->visualpos = l->bwidth + buf->COLS - 1;
    else if (buf->visualpos - l->bwidth < 0)
	buf->visualpos = l->bwidth;
    arrangeLine(buf);
    if (buf->visualpos - l->bwidth == -shift && buf->cursorX == 0)
	buf->visualpos = l->bwidth;
}

/* Shift screen left */
DEFUN(shiftl, SHIFT_LEFT, "Shift screen left")
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
DEFUN(shiftr, SHIFT_RIGHT, "Shift screen right")
{
    int column;

    if (Currentbuf->firstLine == NULL)
	return;
    column = Currentbuf->currentColumn;
    columnSkip(Currentbuf, searchKeyNum() * (Currentbuf->COLS - 1) - 1);
    shiftvisualpos(Currentbuf, Currentbuf->currentColumn - column);
    displayBuffer(Currentbuf, B_NORMAL);
}

DEFUN(col1R, RIGHT, "Shift screen one column right")
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

DEFUN(col1L, LEFT, "Shift screen one column left")
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

DEFUN(setEnv, SETENV, "Set environment variable")
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

DEFUN(pipeBuf, PIPE_BUF, "Pipe current buffer through a shell command and display output")
{
    Buffer *buf;
    char *cmd, *tmpf;
    FILE *f;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    cmd = searchKeyData();
    if (cmd == NULL || *cmd == '\0') {
	/* FIXME: gettextize? */
	cmd = inputLineHist("Pipe buffer to: ", "", IN_COMMAND, ShellHist);
    }
    if (cmd != NULL)
	cmd = conv_to_system(cmd);
    if (cmd == NULL || *cmd == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
    f = fopen(tmpf, "w");
    if (f == NULL) {
	/* FIXME: gettextize? */
	disp_message(Sprintf("Can't save buffer to %s", cmd)->ptr, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f, TRUE);
    fclose(f);
    buf = getpipe(myExtCommand(cmd, shell_quote(tmpf), TRUE)->ptr);
    if (buf == NULL) {
	disp_message("Execution failed", TRUE);
	return;
    }
    else {
	buf->filename = cmd;
	buf->buffername = Sprintf("%s %s", PIPEBUFFERNAME,
				  conv_from_system(cmd))->ptr;
	buf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
	if (buf->type == NULL)
	    buf->type = "text/plain";
	buf->currentURL.file = "-";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Execute shell command and read output ac pipe. */
DEFUN(pipesh, PIPE_SHELL, "Execute shell command and display output")
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
	disp_message("Execution failed", TRUE);
	return;
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
DEFUN(readsh, READ_SHELL, "Execute shell command and display output")
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
    prevtrap = mySignal(SIGINT, intTrap);
    crmode();
    buf = getshell(cmd);
    mySignal(SIGINT, prevtrap);
    term_raw();
    if (buf == NULL) {
	/* FIXME: gettextize? */
	disp_message("Execution failed", TRUE);
	return;
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
DEFUN(execsh, EXEC_SHELL SHELL, "Execute shell command and display output")
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
	printf("\n");
	system(cmd);
	/* FIXME: gettextize? */
	printf("\n[Hit any key]");
	fflush(stdout);
	fmInit();
	getch();
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Load file */
DEFUN(ldfile, LOAD, "Open local file in a new buffer")
{
    char *fn;

    fn = searchKeyData();
    if (fn == NULL || *fn == '\0') {
	/* FIXME: gettextize? */
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
DEFUN(ldhelp, HELP, "Show help panel")
{
    char *lang;
    int n;
    Str tmp;

    lang = AcceptLang;
    n = strcspn(lang, ";, \t");
    tmp = Sprintf("file:///$LIB/" HELP_CGI CGI_EXTENSION "?version=%s&lang=%s",
		  Str_form_quote(Strnew_charp(w3m_version))->ptr,
		  Str_form_quote(Strnew_charp_n(lang, n))->ptr);
    cmd_loadURL(tmp->ptr, NULL, NO_REFERER, NULL);
}

static void
cmd_loadfile(char *fn)
{
    Buffer *buf;

    buf = loadGeneralFile(file_to_url(fn), NULL, NO_REFERER, 0, NULL);
    if (buf == NULL) {
	/* FIXME: gettextize? */
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

DEFUN(movL, MOVE_LEFT, "Cursor left")
{
    _movL(Currentbuf->COLS / 2);
}

DEFUN(movL1, MOVE_LEFT1, "Cursor left. With edge touched, slide")
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

DEFUN(movD, MOVE_DOWN, "Cursor down")
{
    _movD((Currentbuf->LINES + 1) / 2);
}

DEFUN(movD1, MOVE_DOWN1, "Cursor down. With edge touched, slide")
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

DEFUN(movU, MOVE_UP, "Cursor up")
{
    _movU((Currentbuf->LINES + 1) / 2);
}

DEFUN(movU1, MOVE_UP1, "Cursor up. With edge touched, slide")
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

DEFUN(movR, MOVE_RIGHT, "Cursor right")
{
    _movR(Currentbuf->COLS / 2);
}

DEFUN(movR1, MOVE_RIGHT1, "Cursor right. With edge touched, slide")
{
    _movR(1);
}

/* movLW, movRW */
/*
 * From: Takashi Nishimoto <g96p0935@mse.waseda.ac.jp> Date: Mon, 14 Jun
 * 1999 09:29:56 +0900
 */
#if defined(USE_M17N) && defined(USE_UNICODE)
#define nextChar(s, l)	do { (s)++; } while ((s) < (l)->len && (l)->propBuf[s] & PC_WCHAR2)
#define prevChar(s, l)	do { (s)--; } while ((s) > 0 && (l)->propBuf[s] & PC_WCHAR2)

static wc_uint32
getChar(char *p)
{
    return wc_any_to_ucs(wtf_parse1((wc_uchar **)&p));
}

static int
is_wordchar(wc_uint32 c)
{
    return wc_is_ucs_alnum(c);
}
#else
#define nextChar(s, l)	(s)++
#define prevChar(s, l)	(s)--
#define getChar(p)	((int)*(p))

static int
is_wordchar(int c)
{
    return IS_ALNUM(c);
}
#endif

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

DEFUN(movLW, PREV_WORD, "Move to the previous word")
{
    char *lb;
    Line *pline, *l;
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
	    l = Currentbuf->currentLine;
	    lb = l->lineBuf;
	    while (Currentbuf->pos > 0) {
		int tmp = Currentbuf->pos;
		prevChar(tmp, l);
		if (is_wordchar(getChar(&lb[tmp])))
		    break;
		Currentbuf->pos = tmp;
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

	l = Currentbuf->currentLine;
	lb = l->lineBuf;
	while (Currentbuf->pos > 0) {
	    int tmp = Currentbuf->pos;
	    prevChar(tmp, l);
	    if (!is_wordchar(getChar(&lb[tmp])))
		break;
	    Currentbuf->pos = tmp;
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

DEFUN(movRW, NEXT_WORD, "Move to the next word")
{
    char *lb;
    Line *pline, *l;
    int ppos;
    int i, n = searchKeyNum();

    if (Currentbuf->firstLine == NULL)
	return;

    for (i = 0; i < n; i++) {
	pline = Currentbuf->currentLine;
	ppos = Currentbuf->pos;

	if (next_nonnull_line(Currentbuf->currentLine) < 0)
	    goto end;

	l = Currentbuf->currentLine;
	lb = l->lineBuf;
	while (Currentbuf->pos < l->len &&
	       is_wordchar(getChar(&lb[Currentbuf->pos])))
	    nextChar(Currentbuf->pos, l);

	while (1) {
	    while (Currentbuf->pos < l->len &&
		   !is_wordchar(getChar(&lb[Currentbuf->pos])))
		nextChar(Currentbuf->pos, l);
	    if (Currentbuf->pos < l->len)
		break;
	    if (next_nonnull_line(Currentbuf->currentLine->next) < 0) {
		Currentbuf->currentLine = pline;
		Currentbuf->pos = ppos;
		goto end;
	    }
	    Currentbuf->pos = 0;
	    l = Currentbuf->currentLine;
	    lb = l->lineBuf;
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
	/* FIXME: gettextize? */
	ans = inputChar("Download process retains. "
			"Do you want to exit w3m? (y/n)");
    else if (confirm)
	/* FIXME: gettextize? */
	ans = inputChar("Do you want to exit w3m? (y/n)");
    if (!(ans && TOLOWER(*ans) == 'y')) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }

    term_title("");		/* XXX */
    if (activeImage)
	termImage();
    fmTerm();
    save_cookies();
    if (UseHistory && SaveURLHist)
	saveHistory(URLHist, URLHistSize);
    w3m_exit(0);
}

/* Quit */
DEFUN(quitfm, ABORT EXIT, "Quit at once")
{
    _quitfm(FALSE);
}

/* Question and Quit */
DEFUN(qquitfm, QUIT, "Quit with confirmation request")
{
    _quitfm(confirm_on_quit);
}

/* Select buffer */
DEFUN(selBuf, SELECT, "Display buffer-stack panel")
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
	deleteImage(buf);
	if (clear_buffer)
	    tmpClearBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* Suspend (on BSD), or run interactive shell (on SysV) */
DEFUN(susp, INTERRUPT SUSPEND, "Suspend w3m to background")
{
    char *shell;
    move(LASTLINE, 0);
    clrtoeolx();
    refresh();
    fmTerm();
    shell = getenv("SHELL");
    if (shell == NULL)
	shell = "/bin/sh";
    system(shell);
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

DEFUN(goLine, GOTO_LINE, "Go to the specified line")
{

    char *str = searchKeyData();
    if (prec_num)
	_goLine("^");
    else if (str)
	_goLine(str);
    else
	/* FIXME: gettextize? */
	_goLine(inputStr("Goto line: ", ""));
}


DEFUN(goLineF, BEGIN, "Go to the first line")
{
    _goLine("^");
}

DEFUN(goLineL, END, "Go to the last line")
{
    _goLine("$");
}

/* Go to the beginning of the line */
DEFUN(linbeg, LINE_BEGIN, "Go to the beginning of the line")
{
    if (Currentbuf->firstLine == NULL)
	return;
    while (Currentbuf->currentLine->prev && Currentbuf->currentLine->bpos)
	cursorUp0(Currentbuf, 1);
    Currentbuf->pos = 0;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* Go to the bottom of the line */
DEFUN(linend, LINE_END, "Go to the end of the line")
{
    if (Currentbuf->firstLine == NULL)
	return;
    while (Currentbuf->currentLine->next
	   && Currentbuf->currentLine->next->bpos)
	cursorDown0(Currentbuf, 1);
    Currentbuf->pos = Currentbuf->currentLine->len - 1;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

static int
cur_real_linenumber(Buffer *buf)
{
    Line *l, *cur = buf->currentLine;
    int n;

    if (!cur)
	return 1;
    n = cur->real_linenumber ? cur->real_linenumber : 1;
    for (l = buf->firstLine; l && l != cur && l->real_linenumber == 0; l = l->next) {	/* header */
	if (l->bpos == 0)
	    n++;
    }
    return n;
}

/* Run editor on the current buffer */
DEFUN(editBf, EDIT, "Edit local source")
{
    char *fn = Currentbuf->filename;
    Str cmd;

    if (fn == NULL || Currentbuf->pagerSource != NULL ||	/* Behaving as a pager */
	(Currentbuf->type == NULL && Currentbuf->edit == NULL) ||	/* Reading shell */
	Currentbuf->real_scheme != SCM_LOCAL || !strcmp(Currentbuf->currentURL.file, "-") ||	/* file is std input  */
	Currentbuf->bufferprop & BP_FRAME) {	/* Frame */
	disp_err_message("Can't edit other than local file", TRUE);
	return;
    }
    if (Currentbuf->edit)
	cmd = unquote_mailcap(Currentbuf->edit, Currentbuf->real_type, fn,
			      checkHeader(Currentbuf, "Content-Type:"), NULL);
    else
	cmd = myEditor(Editor, shell_quote(fn),
		       cur_real_linenumber(Currentbuf));
    fmTerm();
    system(cmd->ptr);
    fmInit();

    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    reload();
}

/* Run editor on the current screen */
DEFUN(editScr, EDIT_SCREEN, "Edit rendered copy of document")
{
    char *tmpf;
    FILE *f;

    tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
    f = fopen(tmpf, "w");
    if (f == NULL) {
	/* FIXME: gettextize? */
	disp_err_message(Sprintf("Can't open %s", tmpf)->ptr, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f, TRUE);
    fclose(f);
    fmTerm();
    system(myEditor(Editor, shell_quote(tmpf),
		    cur_real_linenumber(Currentbuf))->ptr);
    fmInit();
    unlink(tmpf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}


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
    const int *no_referer_ptr;

    message(Sprintf("loading %s", url)->ptr, 0, 0);
    refresh();

    no_referer_ptr = query_SCONF_NO_REFERER_FROM(&Currentbuf->currentURL);
    base = baseURL(Currentbuf);
    if ((no_referer_ptr && *no_referer_ptr) ||
	base == NULL ||
	base->scheme == SCM_LOCAL || base->scheme == SCM_LOCAL_CGI ||
	base->scheme == SCM_DATA)
	referer = NO_REFERER;
    if (referer == NULL)
	referer = parsedURL2RefererStr(&Currentbuf->currentURL)->ptr;
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

    if (do_download)		/* download (thus no need to render frames) */
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
	    if (label_topline)
		Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
					       Currentbuf->currentLine->
					       linenumber -
					       Currentbuf->topLine->linenumber,
					       FALSE);
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
	/* FIXME: gettextize? */
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
    if (label_topline)
	Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
				       Currentbuf->currentLine->linenumber
				       - Currentbuf->topLine->linenumber,
				       FALSE);
    Currentbuf->pos = al->start.pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    return;
}

static int
handleMailto(char *url)
{
    Str to;
    char *pos;

    if (strncasecmp(url, "mailto:", 7))
	return 0;
    if (!non_null(Mailer)) {
	/* FIXME: gettextize? */
	disp_err_message("no mailer is specified", TRUE);
	return 1;
    }

    /* invoke external mailer */
    if (MailtoOptions == MAILTO_OPTIONS_USE_MAILTO_URL) {
	to = Strnew_charp(html_unquote(url));
    } else {
	to = Strnew_charp(url + 7);
	if ((pos = strchr(to->ptr, '?')) != NULL)
	    Strtruncate(to, pos - to->ptr);
    }
    fmTerm();
    system(myExtCommand(Mailer, shell_quote(file_unquote(to->ptr)),
			FALSE)->ptr);
    fmInit();
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    pushHashHist(URLHist, url);
    return 1;
}

/* follow HREF link */
DEFUN(followA, GOTO_LINK, "Follow current hyperlink in a new buffer")
{
    Anchor *a;
    ParsedURL u;
    int x = 0, y = 0, map = 0;
    char *url;

    if (Currentbuf->firstLine == NULL)
	return;

    a = retrieveCurrentImg(Currentbuf);
    if (a && a->image && a->image->map) {
	_followForm(FALSE);
	return;
    }
    if (a && a->image && a->image->ismap) {
	getMapXY(Currentbuf, a, &x, &y);
	map = 1;
    }
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
    if (handleMailto(a->url))
	return;
    url = a->url;
    if (map)
	url = Sprintf("%s?%d,%d", a->url, x, y)->ptr;

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
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
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
DEFUN(followI, VIEW_IMAGE, "Display image in viewer")
{
    Anchor *a;
    Buffer *buf;

    if (Currentbuf->firstLine == NULL)
	return;

    a = retrieveCurrentImg(Currentbuf);
    if (a == NULL)
	return;
    /* FIXME: gettextize? */
    message(Sprintf("loading %s", a->url)->ptr, 0, 0);
    refresh();
    buf = loadGeneralFile(a->url, baseURL(Currentbuf), NULL, 0, NULL);
    if (buf == NULL) {
	/* FIXME: gettextize? */
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
    FormSelectOptionItem *opt;
    FormSelectOptionItem *curopt;
    FormSelectOptionItem *srcopt;

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

static Str
conv_form_encoding(Str val, FormItemList *fi, Buffer *buf)
{
    wc_ces charset = SystemCharset;

    if (fi->parent->charset)
	charset = fi->parent->charset;
    else if (buf->document_charset && buf->document_charset != WC_CES_US_ASCII)
	charset = buf->document_charset;
    return wc_Str_conv_strict(val, InnerCharset, charset);
}

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
	    Sprintf("------------------------------%d%ld%ld%ld", CurrentPid,
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
		getMapXY(Currentbuf, retrieveCurrentImg(Currentbuf), &x, &y);
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
		getMapXY(Currentbuf, retrieveCurrentImg(Currentbuf), &x, &y);
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
DEFUN(submitForm, SUBMIT, "Submit form")
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
    Anchor *a, *a2;
    char *p;
    FormItemList *fi, *f2;
    Str tmp, tmp2;
    int multipart = 0, i;

    if (Currentbuf->firstLine == NULL)
	return;

    a = retrieveCurrentForm(Currentbuf);
    if (a == NULL)
	return;
    fi = (FormItemList *)a->url;
    switch (fi->type) {
    case FORM_INPUT_TEXT:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	/* FIXME: gettextize? */
	p = inputStrHist("TEXT:", fi->value ? fi->value->ptr : NULL, TextHist);
	if (p == NULL || fi->readonly)
	    break;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept || fi->parent->nitems == 1)
	    goto do_submit;
	break;
    case FORM_INPUT_FILE:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	/* FIXME: gettextize? */
	p = inputFilenameHist("Filename:", fi->value ? fi->value->ptr : NULL,
			      NULL);
	if (p == NULL || fi->readonly)
	    break;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept || fi->parent->nitems == 1)
	    goto do_submit;
	break;
    case FORM_INPUT_PASSWORD:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    break;
	}
	/* FIXME: gettextize? */
	p = inputLine("Password:", fi->value ? fi->value->ptr : NULL,
		      IN_PASSWORD);
	if (p == NULL)
	    break;
	fi->value = Strnew_charp(p);
	formUpdateBuffer(a, Currentbuf, fi);
	if (fi->accept)
	    goto do_submit;
	break;
    case FORM_TEXTAREA:
	if (submit)
	    goto do_submit;
	if (fi->readonly)
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	input_textarea(fi);
	formUpdateBuffer(a, Currentbuf, fi);
	break;
    case FORM_INPUT_RADIO:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    break;
	}
	formRecheckRadio(a, Currentbuf, fi);
	break;
    case FORM_INPUT_CHECKBOX:
	if (submit)
	    goto do_submit;
	if (fi->readonly) {
	    /* FIXME: gettextize? */
	    disp_message_nsec("Read only field!", FALSE, 1, TRUE, FALSE);
	    break;
	}
	fi->checked = !fi->checked;
	formUpdateBuffer(a, Currentbuf, fi);
	break;
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
    case FORM_INPUT_IMAGE:
    case FORM_INPUT_SUBMIT:
    case FORM_INPUT_BUTTON:
      do_submit:
	tmp = Strnew();
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
	    if ((p = strchr(tmp2->ptr, '?')) != NULL)
		Strshrink(tmp2, (tmp2->ptr + tmp2->length) - p);
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
		f2->label = f2->init_label;
		f2->selected = f2->init_selected;
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

/* go to the top anchor */
DEFUN(topA, LINK_BEGIN, "Move to the first hyperlink")
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
DEFUN(lastA, LINK_END, "Move to the last hyperlink")
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

/* go to the nth anchor */
DEFUN(nthA, LINK_N, "Go to the nth link")
{
    HmarkerList *hl = Currentbuf->hmarklist;
	BufferPoint *po;
	Anchor *an;

	int n = searchKeyNum();
	if (n < 0 || n > hl->nmark) return;

	if (Currentbuf->firstLine == NULL)
		return;
    if (!hl || hl->nmark == 0)
		return;

	po = hl->marks + n-1;
	an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
	if (an == NULL)
		an = retrieveAnchor(Currentbuf->formitem, po->line, po->pos);
	if (an == NULL) return;

    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* go to the next anchor */
DEFUN(nextA, NEXT_LINK, "Move to the next hyperlink")
{
    _nextA(FALSE);
}

/* go to the previous anchor */
DEFUN(prevA, PREV_LINK, "Move to the previous hyperlink")
{
    _prevA(FALSE);
}

/* go to the next visited anchor */
DEFUN(nextVA, NEXT_VISITED, "Move to the next visited hyperlink")
{
    _nextA(TRUE);
}

/* go to the previous visited anchor */
DEFUN(prevVA, PREV_VISITED, "Move to the previous visited hyperlink")
{
    _prevA(TRUE);
}

/* go to the next [visited] anchor */
static void
_nextA(int visited)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an, *pan;
    int i, x, y, n = searchKeyNum();
    ParsedURL url;

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (visited != TRUE && an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    y = Currentbuf->currentLine->linenumber;
    x = Currentbuf->pos;

    if (visited == TRUE) {
	n = hl->nmark;
    }

    for (i = 0; i < n; i++) {
	pan = an;
	if (an && an->hseq >= 0) {
	    int hseq = an->hseq + 1;
	    do {
		if (hseq >= hl->nmark) {
		    if (visited == TRUE)
			return;
		    an = pan;
		    goto _end;
		}
		po = &hl->marks[hseq];
		an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
		if (visited != TRUE && an == NULL)
		    an = retrieveAnchor(Currentbuf->formitem, po->line,
					po->pos);
		hseq++;
		if (visited == TRUE && an) {
		    parseURL2(an->url, &url, baseURL(Currentbuf));
		    if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
			goto _end;
		    }
		}
	    } while (an == NULL || an == pan);
	}
	else {
	    an = closest_next_anchor(Currentbuf->href, NULL, x, y);
	    if (visited != TRUE)
		an = closest_next_anchor(Currentbuf->formitem, an, x, y);
	    if (an == NULL) {
		if (visited == TRUE)
		    return;
		an = pan;
		break;
	    }
	    x = an->start.pos;
	    y = an->start.line;
	    if (visited == TRUE) {
		parseURL2(an->url, &url, baseURL(Currentbuf));
		if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
		    goto _end;
		}
	    }
	}
    }
    if (visited == TRUE)
	return;

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
static void
_prevA(int visited)
{
    HmarkerList *hl = Currentbuf->hmarklist;
    BufferPoint *po;
    Anchor *an, *pan;
    int i, x, y, n = searchKeyNum();
    ParsedURL url;

    if (Currentbuf->firstLine == NULL)
	return;
    if (!hl || hl->nmark == 0)
	return;

    an = retrieveCurrentAnchor(Currentbuf);
    if (visited != TRUE && an == NULL)
	an = retrieveCurrentForm(Currentbuf);

    y = Currentbuf->currentLine->linenumber;
    x = Currentbuf->pos;

    if (visited == TRUE) {
	n = hl->nmark;
    }

    for (i = 0; i < n; i++) {
	pan = an;
	if (an && an->hseq >= 0) {
	    int hseq = an->hseq - 1;
	    do {
		if (hseq < 0) {
		    if (visited == TRUE)
			return;
		    an = pan;
		    goto _end;
		}
		po = hl->marks + hseq;
		an = retrieveAnchor(Currentbuf->href, po->line, po->pos);
		if (visited != TRUE && an == NULL)
		    an = retrieveAnchor(Currentbuf->formitem, po->line,
					po->pos);
		hseq--;
		if (visited == TRUE && an) {
		    parseURL2(an->url, &url, baseURL(Currentbuf));
		    if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
			goto _end;
		    }
		}
	    } while (an == NULL || an == pan);
	}
	else {
	    an = closest_prev_anchor(Currentbuf->href, NULL, x, y);
	    if (visited != TRUE)
		an = closest_prev_anchor(Currentbuf->formitem, an, x, y);
	    if (an == NULL) {
		if (visited == TRUE)
		    return;
		an = pan;
		break;
	    }
	    x = an->start.pos;
	    y = an->start.line;
	    if (visited == TRUE && an) {
		parseURL2(an->url, &url, baseURL(Currentbuf));
		if (getHashHist(URLHist, parsedURL2Str(&url)->ptr)) {
		    goto _end;
		}
	    }
	}
    }
    if (visited == TRUE)
	return;

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
DEFUN(nextL, NEXT_LEFT, "Move left to the next hyperlink")
{
    nextX(-1, 0);
}

/* go to the next left-up anchor */
DEFUN(nextLU, NEXT_LEFT_UP, "Move left or upward to the next hyperlink")
{
    nextX(-1, -1);
}

/* go to the next right anchor */
DEFUN(nextR, NEXT_RIGHT, "Move right to the next hyperlink")
{
    nextX(1, 0);
}

/* go to the next right-down anchor */
DEFUN(nextRD, NEXT_RIGHT_DOWN, "Move right or downward to the next hyperlink")
{
    nextX(1, 1);
}

/* go to the next downward anchor */
DEFUN(nextD, NEXT_DOWN, "Move downward to the next hyperlink")
{
    nextY(1);
}

/* go to the next upward anchor */
DEFUN(nextU, NEXT_UP, "Move upward to the next hyperlink")
{
    nextY(-1);
}

/* go to the next bufferr */
DEFUN(nextBf, NEXT, "Switch to the next buffer")
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
DEFUN(prevBf, PREV, "Switch to the previous buffer")
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
DEFUN(backBf, BACK, "Close current buffer and return to the one below in stack")
{
    Buffer *buf = Currentbuf->linkBuffer[LB_N_FRAME];

    if (!checkBackBuffer(Currentbuf)) {
	if (close_tab_back && nTab >= 1) {
	    deleteTab(CurrentTab);
	    displayBuffer(Currentbuf, B_FORCE_REDRAW);
	}
	else
	    /* FIXME: gettextize? */
	    disp_message("Can't go back...", TRUE);
	return;
    }

    delBuffer(Currentbuf);

    if (buf) {
	if (buf->frameQ) {
	    struct frameset *fs;
	    long linenumber = buf->frameQ->linenumber;
	    long top = buf->frameQ->top_linenumber;
	    int pos = buf->frameQ->pos;
	    int currentColumn = buf->frameQ->currentColumn;
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

DEFUN(deletePrevBuf, DELETE_PREVBUF, "Delete previous buffer (mainly for local CGI-scripts)")
{
    Buffer *buf = Currentbuf->nextBuffer;
    if (buf)
	delBuffer(buf);
}

static void
cmd_loadURL(char *url, ParsedURL *current, char *referer, FormList *request)
{
    Buffer *buf;

    if (handleMailto(url))
	return;

    refresh();
    buf = loadGeneralFile(url, current, referer, 0, request);
    if (buf == NULL) {
	/* FIXME: gettextize? */
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
    const int *no_referer_ptr;

    url = searchKeyData();
    if (url == NULL) {
	Hist *hist = copyHist(URLHist);
	Anchor *a;

	current = baseURL(Currentbuf);
	if (current) {
	    char *c_url = parsedURL2Str(current)->ptr;
	    if (DefaultURLString == DEFAULT_URL_CURRENT)
		url = url_decode2(c_url, NULL);
	    else
		pushHist(hist, c_url);
	}
	a = retrieveCurrentAnchor(Currentbuf);
	if (a) {
	    char *a_url;
	    parseURL2(a->url, &p_url, current);
	    a_url = parsedURL2Str(&p_url)->ptr;
	    if (DefaultURLString == DEFAULT_URL_LINK)
		url = url_decode2(a_url, Currentbuf);
	    else
		pushHist(hist, a_url);
	}
	url = inputLineHist(prompt, url, IN_URL, hist);
	if (url != NULL)
	    SKIP_BLANKS(url);
    }
    if (relative) {
	no_referer_ptr = query_SCONF_NO_REFERER_FROM(&Currentbuf->currentURL);
	current = baseURL(Currentbuf);
	if ((no_referer_ptr && *no_referer_ptr) ||
	    current == NULL ||
	    current->scheme == SCM_LOCAL || current->scheme == SCM_LOCAL_CGI ||
	    current->scheme == SCM_DATA)
	    referer = NO_REFERER;
	else
	    referer = parsedURL2RefererStr(&Currentbuf->currentURL)->ptr;
	url = url_encode(url, current, Currentbuf->document_charset);
    }
    else {
	current = NULL;
	referer = NULL;
	url = url_encode(url, NULL, 0);
    }
    if (url == NULL || *url == '\0') {
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    if (*url == '#') {
	gotoLabel(url + 1);
	return;
    }
    parseURL2(url, &p_url, current);
    pushHashHist(URLHist, parsedURL2Str(&p_url)->ptr);
    cmd_loadURL(url, current, referer, NULL);
    if (Currentbuf != cur_buf)	/* success */
	pushHashHist(URLHist, parsedURL2Str(&Currentbuf->currentURL)->ptr);
}

DEFUN(goURL, GOTO, "Open specified document in a new buffer")
{
    goURL0("Goto URL: ", FALSE);
}

DEFUN(goHome, GOTO_HOME, "Open home page in a new buffer")
{
    char *url;
    if ((url = getenv("HTTP_HOME")) != NULL ||
        (url = getenv("WWW_HOME")) != NULL) {
        ParsedURL p_url;
        Buffer *cur_buf = Currentbuf;
        SKIP_BLANKS(url);
        url = url_encode(url, NULL, 0);
        parseURL2(url, &p_url, NULL);
        pushHashHist(URLHist, parsedURL2Str(&p_url)->ptr);
        cmd_loadURL(url, NULL, NULL, NULL);
        if (Currentbuf != cur_buf)	/* success */
        pushHashHist(URLHist, parsedURL2Str(&Currentbuf->currentURL)->ptr);
    }
}

DEFUN(gorURL, GOTO_RELATIVE, "Go to relative address")
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
DEFUN(ldBmark, BOOKMARK VIEW_BOOKMARK, "View bookmarks")
{
    cmd_loadURL(BookmarkFile, NULL, NO_REFERER, NULL);
}


/* Add current to bookmark */
DEFUN(adBmark, ADD_BOOKMARK, "Add current page to bookmarks")
{
    Str tmp;
    FormList *request;

    tmp = Sprintf("mode=panel&cookie=%s&bmark=%s&url=%s&title=%s"
		    "&charset=%s"
		    ,
		  (Str_form_quote(localCookie()))->ptr,
		  (Str_form_quote(Strnew_charp(BookmarkFile)))->ptr,
		  (Str_form_quote(parsedURL2Str(&Currentbuf->currentURL)))->
		  ptr,
		  (Str_form_quote(wc_conv_strict(Currentbuf->buffername,
						 InnerCharset,
						 BookmarkCharset)))->ptr,
		  wc_ces_to_charset(BookmarkCharset));
    request = newFormList(NULL, "post", NULL, NULL, NULL, NULL, NULL);
    request->body = tmp->ptr;
    request->length = tmp->length;
    cmd_loadURL("file:///$LIB/" W3MBOOKMARK_CMDNAME, NULL, NO_REFERER,
		request);
}

/* option setting */
DEFUN(ldOpt, OPTIONS, "Display options setting panel")
{
    cmd_loadBuffer(load_option_panel(), BP_NO_URL, LB_NOLINK);
}

/* set an option */
DEFUN(setOpt, SET_OPTION, "Set option")
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
DEFUN(msgs, MSGS, "Display error messages")
{
    cmd_loadBuffer(message_list_panel(), BP_NO_URL, LB_NOLINK);
}

/* page info */
DEFUN(pginfo, INFO, "Display information about the current document")
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
		    parsedURL2Str(&Currentbuf->currentURL)->ptr, NULL);
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    cmd_loadURL(a->url, baseURL(Currentbuf),
		parsedURL2Str(&Currentbuf->currentURL)->ptr, NULL);
#endif
}

/* link menu */
DEFUN(linkMn, LINK_MENU, "Pop up link element menu")
{
    LinkList *l = link_menu(Currentbuf);
    ParsedURL p_url;

    if (!l || !l->url)
	return;
    if (*(l->url) == '#') {
	gotoLabel(l->url + 1);
	return;
    }
    parseURL2(l->url, &p_url, baseURL(Currentbuf));
    pushHashHist(URLHist, parsedURL2Str(&p_url)->ptr);
    cmd_loadURL(l->url, baseURL(Currentbuf),
		parsedURL2Str(&Currentbuf->currentURL)->ptr, NULL);
}

static void
anchorMn(Anchor *(*menu_func) (Buffer *), int go)
{
    Anchor *a;
    BufferPoint *po;

    if (!Currentbuf->href || !Currentbuf->hmarklist)
	return;
    a = menu_func(Currentbuf);
    if (!a || a->hseq < 0)
	return;
    po = &Currentbuf->hmarklist->marks[a->hseq];
    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
    if (go)
	followA();
}

/* accesskey */
DEFUN(accessKey, ACCESSKEY, "Pop up accesskey menu")
{
    anchorMn(accesskey_menu, TRUE);
}

/* list menu */
DEFUN(listMn, LIST_MENU, "Pop up menu for hyperlinks to browse to")
{
    anchorMn(list_menu, TRUE);
}

DEFUN(movlistMn, MOVE_LIST_MENU, "Pop up menu to navigate between hyperlinks")
{
    anchorMn(list_menu, FALSE);
}

/* link,anchor,image list */
DEFUN(linkLst, LIST, "Show all URLs referenced")
{
    Buffer *buf;

    buf = link_list_panel(Currentbuf);
    if (buf != NULL) {
	buf->document_charset = Currentbuf->document_charset;
	cmd_loadBuffer(buf, BP_NORMAL, LB_NOLINK);
    }
}

/* cookie list */
DEFUN(cooLst, COOKIE, "View cookie list")
{
    Buffer *buf;

    buf = cookie_list_panel();
    if (buf != NULL)
	cmd_loadBuffer(buf, BP_NO_URL, LB_NOLINK);
}

/* History page */
DEFUN(ldHist, HISTORY, "Show browsing history")
{
    cmd_loadBuffer(historyBuffer(URLHist), BP_NO_URL, LB_NOLINK);
}

/* download HREF link */
DEFUN(svA, SAVE_LINK, "Save hyperlink target")
{
    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    do_download = TRUE;
    followA();
    do_download = FALSE;
}

/* download IMG link */
DEFUN(svI, SAVE_IMAGE, "Save inline image")
{
    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    do_download = TRUE;
    followI();
    do_download = FALSE;
}

/* save buffer */
DEFUN(svBuf, PRINT SAVE_SCREEN, "Save rendered document")
{
    char *qfile = NULL, *file;
    FILE *f;
    int is_pipe;

    CurrentKeyData = NULL;	/* not allowed in w3m-control: */
    file = searchKeyData();
    if (file == NULL || *file == '\0') {
	/* FIXME: gettextize? */
	qfile = inputLineHist("Save buffer to: ", NULL, IN_COMMAND, SaveHist);
	if (qfile == NULL || *qfile == '\0') {
	    displayBuffer(Currentbuf, B_NORMAL);
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
	file = expandPath(file);
	if (checkOverWrite(file) < 0) {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
	f = fopen(file, "w");
	is_pipe = FALSE;
    }
    if (f == NULL) {
	/* FIXME: gettextize? */
	char *emsg = Sprintf("Can't open %s", conv_from_system(file))->ptr;
	disp_err_message(emsg, TRUE);
	return;
    }
    saveBuffer(Currentbuf, f, TRUE);
    if (is_pipe)
	pclose(f);
    else
	fclose(f);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* save source */
DEFUN(svSrc, DOWNLOAD SAVE, "Save document source")
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
    static Lineprop *p = NULL;
    Lineprop *pp;
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
    s = NULL;
    a = (only_img ? NULL : retrieveCurrentAnchor(Currentbuf));
    if (a == NULL) {
	a = (only_img ? NULL : retrieveCurrentForm(Currentbuf));
	if (a == NULL) {
	    a = retrieveCurrentImg(Currentbuf);
	    if (a == NULL)
		return;
	}
	else
	    s = Strnew_charp(form2str((FormItemList *)a->url));
    }
    if (s == NULL) {
	parseURL2(a->url, &pu, baseURL(Currentbuf));
	s = parsedURL2Str(&pu);
    }
    if (DecodeURL)
	s = Strnew_charp(url_decode2(s->ptr, Currentbuf));
    s = checkType(s, &pp, NULL);
    p = NewAtom_N(Lineprop, s->length);
    bcopy((void *)pp, (void *)p, s->length * sizeof(Lineprop));
  disp:
    n = searchKeyNum();
    if (n > 1 && s->length > (n - 1) * (COLS - 1))
	offset = (n - 1) * (COLS - 1);
    while (offset < s->length && p[offset] & PC_WCHAR2)
	offset++;
    disp_message_nomouse(&s->ptr[offset], TRUE);
}

/* peek URL */
DEFUN(peekURL, PEEK_LINK, "Show target address")
{
    _peekURL(0);
}

/* peek URL of image */
DEFUN(peekIMG, PEEK_IMG, "Show image address")
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

DEFUN(curURL, PEEK, "Show current address")
{
    static Str s = NULL;
    static Lineprop *p = NULL;
    Lineprop *pp;
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
	if (DecodeURL)
	    s = Strnew_charp(url_decode2(s->ptr, NULL));
	s = checkType(s, &pp, NULL);
	p = NewAtom_N(Lineprop, s->length);
	bcopy((void *)pp, (void *)p, s->length * sizeof(Lineprop));
    }
    n = searchKeyNum();
    if (n > 1 && s->length > (n - 1) * (COLS - 1))
	offset = (n - 1) * (COLS - 1);
    while (offset < s->length && p[offset] & PC_WCHAR2)
	offset++;
    disp_message_nomouse(&s->ptr[offset], TRUE);
}
/* view HTML source */

DEFUN(vwSrc, SOURCE VIEW, "Toggle between HTML shown or processed")
{
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
	    wc_ces old_charset;
	    wc_bool old_fix_width_conv;
	    FILE *f;
	    Str tmpf = tmpfname(TMPF_SRC, NULL);
	    f = fopen(tmpf->ptr, "w");
	    if (f == NULL)
		return;
	    old_charset = DisplayCharset;
	    old_fix_width_conv = WcOption.fix_width_conv;
	    DisplayCharset = (Currentbuf->document_charset != WC_CES_US_ASCII)
		? Currentbuf->document_charset : 0;
	    WcOption.fix_width_conv = WC_FALSE;
	    saveBufferBody(Currentbuf, f, TRUE);
	    DisplayCharset = old_charset;
	    WcOption.fix_width_conv = old_fix_width_conv;
	    fclose(f);
	    Currentbuf->sourcefile = tmpf->ptr;
	}
	else {
	    return;
	}
    }

    buf = newBuffer(INIT_BUFFER_WIDTH);

    if (is_html_type(Currentbuf->type)) {
	buf->type = "text/plain";
	if (Currentbuf->real_type &&
	    is_html_type(Currentbuf->real_type))
	    buf->real_type = "text/plain";
	else
	    buf->real_type = Currentbuf->real_type;
	buf->buffername = Sprintf("source of %s", Currentbuf->buffername)->ptr;
	buf->linkBuffer[LB_N_SOURCE] = Currentbuf;
	Currentbuf->linkBuffer[LB_SOURCE] = buf;
    }
    else if (!strcasecmp(Currentbuf->type, "text/plain")) {
	buf->type = "text/html";
	if (Currentbuf->real_type &&
	    !strcasecmp(Currentbuf->real_type, "text/plain"))
	    buf->real_type = "text/html";
	else
	    buf->real_type = Currentbuf->real_type;
	buf->buffername = Sprintf("HTML view of %s",
				  Currentbuf->buffername)->ptr;
	buf->linkBuffer[LB_SOURCE] = Currentbuf;
	Currentbuf->linkBuffer[LB_N_SOURCE] = buf;
    }
    else {
	return;
    }
    buf->currentURL = Currentbuf->currentURL;
    buf->real_scheme = Currentbuf->real_scheme;
    buf->filename = Currentbuf->filename;
    buf->sourcefile = Currentbuf->sourcefile;
    buf->header_source = Currentbuf->header_source;
    buf->search_header = Currentbuf->search_header;
    buf->document_charset = Currentbuf->document_charset;
    buf->clone = Currentbuf->clone;
    (*buf->clone)++;

    buf->need_reshape = TRUE;
    reshapeBuffer(buf);
    pushBuffer(buf);
    displayBuffer(Currentbuf, B_NORMAL);
}

/* reload */
DEFUN(reload, RELOAD, "Load current document anew")
{
    Buffer *buf, *fbuf = NULL, sbuf;
    wc_ces old_charset;
    Str url;
    FormList *request;
    int multipart;

    if (Currentbuf->bufferprop & BP_INTERNAL) {
	if (!strcmp(Currentbuf->buffername, DOWNLOAD_LIST_TITLE)) {
	    ldDL();
	    return;
	}
	/* FIXME: gettextize? */
	disp_err_message("Can't reload...", TRUE);
	return;
    }
    if (Currentbuf->currentURL.scheme == SCM_LOCAL &&
	!strcmp(Currentbuf->currentURL.file, "-")) {
	/* file is std input */
	/* FIXME: gettextize? */
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
	if (!(buf = renderFrame(fbuf, 1))) {
	    displayBuffer(Currentbuf, B_NORMAL);
	    return;
	}
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
	if (Currentbuf->firstLine) {
	    COPY_BUFROOT(Currentbuf, &sbuf);
	    restorePosition(Currentbuf, &sbuf);
	}
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
    /* FIXME: gettextize? */
    message("Reloading...", 0, 0);
    refresh();
    old_charset = DocumentCharset;
    if (Currentbuf->document_charset != WC_CES_US_ASCII)
	DocumentCharset = Currentbuf->document_charset;
    SearchHeader = Currentbuf->search_header;
    DefaultType = Currentbuf->real_type;
    buf = loadGeneralFile(url->ptr, NULL, NO_REFERER, RG_NOCACHE, request);
    DocumentCharset = old_charset;
    SearchHeader = FALSE;
    DefaultType = NULL;

    if (multipart)
	unlink(request->body);
    if (buf == NULL) {
	/* FIXME: gettextize? */
	disp_err_message("Can't reload...", TRUE);
	return;
    }
    else if (buf == NO_BUFFER) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if (fbuf != NULL)
	Firstbuf = deleteBuffer(Firstbuf, fbuf);
    repBuffer(Currentbuf, buf);
    if ((buf->type != NULL) && (sbuf.type != NULL) &&
	((!strcasecmp(buf->type, "text/plain") &&
	  is_html_type(sbuf.type)) ||
	 (is_html_type(buf->type) &&
	  !strcasecmp(sbuf.type, "text/plain")))) {
	vwSrc();
	if (Currentbuf != buf)
	    Firstbuf = deleteBuffer(Firstbuf, buf);
    }
    Currentbuf->search_header = sbuf.search_header;
    Currentbuf->form_submit = sbuf.form_submit;
    if (Currentbuf->firstLine) {
	COPY_BUFROOT(Currentbuf, &sbuf);
	restorePosition(Currentbuf, &sbuf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

/* reshape */
DEFUN(reshape, RESHAPE, "Re-render document")
{
    Currentbuf->need_reshape = TRUE;
    reshapeBuffer(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
_docCSet(wc_ces charset)
{
    if (Currentbuf->bufferprop & BP_INTERNAL)
	return;
    if (Currentbuf->sourcefile == NULL) {
	disp_message("Can't reload...", FALSE);
	return;
    }
    Currentbuf->document_charset = charset;
    Currentbuf->need_reshape = TRUE;
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
change_charset(struct parsed_tagarg *arg)
{
    Buffer *buf = Currentbuf->linkBuffer[LB_N_INFO];
    wc_ces charset;

    if (buf == NULL)
	return;
    delBuffer(Currentbuf);
    Currentbuf = buf;
    if (Currentbuf->bufferprop & BP_INTERNAL)
	return;
    charset = Currentbuf->document_charset;
    for (; arg; arg = arg->next) {
	if (!strcmp(arg->arg, "charset"))
	    charset = atoi(arg->value);
    }
    _docCSet(charset);
}

DEFUN(docCSet, CHARSET, "Change the character encoding for the current document")
{
    char *cs;
    wc_ces charset;

    cs = searchKeyData();
    if (cs == NULL || *cs == '\0')
	/* FIXME: gettextize? */
	cs = inputStr("Document charset: ",
		      wc_ces_to_charset(Currentbuf->document_charset));
    charset = wc_guess_charset_short(cs, 0);
    if (charset == 0) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    _docCSet(charset);
}

DEFUN(defCSet, DEFAULT_CHARSET, "Change the default character encoding")
{
    char *cs;
    wc_ces charset;

    cs = searchKeyData();
    if (cs == NULL || *cs == '\0')
	/* FIXME: gettextize? */
	cs = inputStr("Default document charset: ",
		      wc_ces_to_charset(DocumentCharset));
    charset = wc_guess_charset_short(cs, 0);
    if (charset != 0)
	DocumentCharset = charset;
    displayBuffer(Currentbuf, B_NORMAL);
}

/* mark URL-like patterns as anchors */
void
chkURLBuffer(Buffer *buf)
{
    static char *url_like_pat[] = {
	"https?://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./?=~_\\&+@#,\\$;]*[a-zA-Z0-9_/=\\-]",
	"file:/[a-zA-Z0-9:%\\-\\./=_\\+@#,\\$;]*",
	"ftp://[a-zA-Z0-9][a-zA-Z0-9:%\\-\\./=_+@#,\\$]*[a-zA-Z0-9_/]",
	"mailto:[^<> 	][^<> 	]*@[a-zA-Z0-9][a-zA-Z0-9\\-\\._]*[a-zA-Z0-9]",
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
    chkExternalURIBuffer(buf);
    buf->check_url |= CHK_URL;
}

DEFUN(chkURL, MARK_URL, "Turn URL-like strings into hyperlinks")
{
    chkURLBuffer(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

DEFUN(chkWORD, MARK_WORD, "Turn current word into hyperlink")
{
    char *p;
    int spos, epos;
    p = getCurWord(Currentbuf, &spos, &epos);
    if (p == NULL)
	return;
    reAnchorWord(Currentbuf, Currentbuf->currentLine, spos, epos);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}


/* render frames */
DEFUN(rFrame, FRAME, "Toggle rendering HTML frames")
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
    if (buf == NULL) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
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
	case 4:
	    browser = ExtBrowser4;
	    break;
	case 5:
	    browser = ExtBrowser5;
	    break;
	case 6:
	    browser = ExtBrowser6;
	    break;
	case 7:
	    browser = ExtBrowser7;
	    break;
	case 8:
	    browser = ExtBrowser8;
	    break;
	case 9:
	    browser = ExtBrowser9;
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
    if (browser == NULL || *browser == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }

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

DEFUN(extbrz, EXTERN, "Display using an external browser")
{
    if (Currentbuf->bufferprop & BP_INTERNAL) {
	/* FIXME: gettextize? */
	disp_err_message("Can't browse...", TRUE);
	return;
    }
    if (Currentbuf->currentURL.scheme == SCM_LOCAL &&
	!strcmp(Currentbuf->currentURL.file, "-")) {
	/* file is std input */
	/* FIXME: gettextize? */
	disp_err_message("Can't browse stdin", TRUE);
	return;
    }
    invoke_browser(parsedURL2Str(&Currentbuf->currentURL)->ptr);
}

DEFUN(linkbrz, EXTERN_LINK, "Display target using an external browser")
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
DEFUN(curlno, LINE_INFO, "Display current position in document")
{
    Line *l = Currentbuf->currentLine;
    Str tmp;
    int cur = 0, all = 0, col = 0, len = 0;

    if (l != NULL) {
	cur = l->real_linenumber;
	col = l->bwidth + Currentbuf->currentColumn + Currentbuf->cursorX + 1;
	while (l->next && l->next->bpos)
	    l = l->next;
	if (l->width < 0)
	    l->width = COLPOS(l, l->len);
	len = l->bwidth + l->width;
    }
    if (Currentbuf->lastLine)
	all = Currentbuf->lastLine->real_linenumber;
    if (Currentbuf->pagerSource && !(Currentbuf->bufferprop & BP_CLOSE))
	tmp = Sprintf("line %d col %d/%d", cur, col, len);
    else
	tmp = Sprintf("line %d/%d (%d%%) col %d/%d", cur, all,
		      (int)((double)cur * 100.0 / (double)(all ? all : 1)
			    + 0.5), col, len);
    Strcat_charp(tmp, "  ");
    Strcat_charp(tmp, wc_ces_to_charset_desc(Currentbuf->document_charset));

    disp_message(tmp->ptr, FALSE);
}

DEFUN(dispI, DISPLAY_IMAGE, "Restart loading and drawing of images")
{
    if (!displayImage)
	initImage();
    if (!activeImage)
	return;
    displayImage = TRUE;
    /*
     * if (!(Currentbuf->type && is_html_type(Currentbuf->type)))
     * return;
     */
    Currentbuf->image_flag = IMG_FLAG_AUTO;
    Currentbuf->need_reshape = TRUE;
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

DEFUN(stopI, STOP_IMAGE, "Stop loading and drawing of images")
{
    if (!activeImage)
	return;
    /*
     * if (!(Currentbuf->type && is_html_type(Currentbuf->type)))
     * return;
     */
    Currentbuf->image_flag = IMG_FLAG_SKIP;
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}


DEFUN(dispVer, VERSION, "Display the version of w3m")
{
    disp_message(Sprintf("w3m version %s", w3m_version)->ptr, TRUE);
}

DEFUN(wrapToggle, WRAP_TOGGLE, "Toggle wrapping mode in searches")
{
    if (WrapSearch) {
	WrapSearch = FALSE;
	/* FIXME: gettextize? */
	disp_message("Wrap search off", TRUE);
    }
    else {
	WrapSearch = TRUE;
	/* FIXME: gettextize? */
	disp_message("Wrap search on", TRUE);
    }
}

static char *
getCurWord(Buffer *buf, int *spos, int *epos)
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
    while (e > 0 && !is_wordchar(getChar(&p[e])))
	prevChar(e, l);
    if (!is_wordchar(getChar(&p[e])))
	return NULL;
    b = e;
    while (b > 0) {
	int tmp = b;
	prevChar(tmp, l);
	if (!is_wordchar(getChar(&p[tmp])))
	    break;
	b = tmp;
    }
    while (e < l->len && is_wordchar(getChar(&p[e])))
	nextChar(e, l);
    *spos = b;
    *epos = e;
    return &p[b];
}

static char *
GetWord(Buffer *buf)
{
    int b, e;
    char *p;

    if ((p = getCurWord(buf, &b, &e)) != NULL) {
	return Strnew_charp_n(p, e - b)->ptr;
    }
    return NULL;
}

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
	disp_message("Execution failed", TRUE);
	return;
    }
    else if (buf != NO_BUFFER) {
	buf->filename = w;
	buf->buffername = Sprintf("%s %s", DICTBUFFERNAME, word)->ptr;
	if (buf->type == NULL)
	    buf->type = "text/plain";
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

DEFUN(dictword, DICT_WORD, "Execute dictionary command (see README.dict)")
{
    execdict(inputStr("(dictionary)!", ""));
}

DEFUN(dictwordat, DICT_WORD_AT,
      "Execute dictionary command for word at cursor")
{
    execdict(GetWord(Currentbuf));
}

void
set_buffer_environ(Buffer *buf)
{
    static Buffer *prev_buf = NULL;
    static Line *prev_line = NULL;
    static int prev_pos = -1;
    Line *l;

    if (buf == NULL)
	return;
    if (buf != prev_buf) {
	set_environ("W3M_SOURCEFILE", buf->sourcefile);
	set_environ("W3M_FILENAME", buf->filename);
	set_environ("W3M_TITLE", buf->buffername);
	set_environ("W3M_URL", parsedURL2Str(&buf->currentURL)->ptr);
	set_environ("W3M_TYPE", buf->real_type ? buf->real_type : "unknown");
	set_environ("W3M_CHARSET", wc_ces_to_charset(buf->document_charset));
    }
    l = buf->currentLine;
    if (l && (buf != prev_buf || l != prev_line || buf->pos != prev_pos)) {
	Anchor *a;
	ParsedURL pu;
	char *s = GetWord(buf);
	set_environ("W3M_CURRENT_WORD", s ? s : "");
	a = retrieveCurrentAnchor(buf);
	if (a) {
	    parseURL2(a->url, &pu, baseURL(buf));
	    set_environ("W3M_CURRENT_LINK", parsedURL2Str(&pu)->ptr);
	}
	else
	    set_environ("W3M_CURRENT_LINK", "");
	a = retrieveCurrentImg(buf);
	if (a) {
	    parseURL2(a->url, &pu, baseURL(buf));
	    set_environ("W3M_CURRENT_IMG", parsedURL2Str(&pu)->ptr);
	}
	else
	    set_environ("W3M_CURRENT_IMG", "");
	a = retrieveCurrentForm(buf);
	if (a)
	    set_environ("W3M_CURRENT_FORM", form2str((FormItemList *)a->url));
	else
	    set_environ("W3M_CURRENT_FORM", "");
	set_environ("W3M_CURRENT_LINE", Sprintf("%ld",
						l->real_linenumber)->ptr);
	set_environ("W3M_CURRENT_COLUMN", Sprintf("%d",
						  buf->currentColumn +
						  buf->cursorX + 1)->ptr);
    }
    else if (!l) {
	set_environ("W3M_CURRENT_WORD", "");
	set_environ("W3M_CURRENT_LINK", "");
	set_environ("W3M_CURRENT_IMG", "");
	set_environ("W3M_CURRENT_FORM", "");
	set_environ("W3M_CURRENT_LINE", "0");
	set_environ("W3M_CURRENT_COLUMN", "0");
    }
    prev_buf = buf;
    prev_line = l;
    prev_pos = buf->pos;
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
    while ((f = popText(fileToDelete)) != NULL) {
	unlink(f);
	if (enable_inline_image == INLINE_IMG_SIXEL && strcmp(f+strlen(f)-4, ".gif") == 0) {
	    Str firstframe = Strnew_charp(f);
	    Strcat_charp(firstframe, "-1");
	    unlink(firstframe->ptr);
        }
    }
}

void
w3m_exit(int i)
{
    stopDownload();
    deleteFiles();
    free_ssl_ctx();
    disconnectFTP();
#ifdef HAVE_MKDTEMP
    if (no_rc_dir && tmp_dir != rc_dir)
	if (rmdir(tmp_dir) != 0) {
	    fprintf(stderr, "Can't remove temporary directory (%s)!\n", tmp_dir);
	    exit(1);
	}
#endif
    exit(i);
}

DEFUN(execCmd, COMMAND, "Invoke w3m function(s)")
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
	w3mFuncList[cmd].func();
	CurrentCmdData = NULL;
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

static MySignalHandler
SigAlarm(SIGNAL_ARG)
{
    char *data;

    if (CurrentAlarm->sec > 0) {
	CurrentKey = -1;
	CurrentKeyData = NULL;
	CurrentCmdData = data = (char *)CurrentAlarm->data;
	w3mFuncList[CurrentAlarm->cmd].func();
	CurrentCmdData = NULL;
	if (CurrentAlarm->status == AL_IMPLICIT_ONCE) {
	    CurrentAlarm->sec = 0;
	    CurrentAlarm->status = AL_UNSET;
	}
	if (Currentbuf->event) {
	    if (Currentbuf->event->status != AL_UNSET)
		CurrentAlarm = Currentbuf->event;
	    else
		Currentbuf->event = NULL;
	}
	if (!Currentbuf->event)
	    CurrentAlarm = &DefaultAlarm;
	if (CurrentAlarm->sec > 0) {
	    mySignal(SIGALRM, SigAlarm);
	    alarm(CurrentAlarm->sec);
	}
    }
    SIGNAL_RETURN;
}


DEFUN(setAlarm, ALARM, "Set alarm")
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
	setAlarmEvent(&DefaultAlarm, sec, AL_EXPLICIT, cmd, data);
	disp_message_nsec(Sprintf("%dsec %s %s", sec, w3mFuncList[cmd].id,
				  data)->ptr, FALSE, 1, FALSE, TRUE);
    }
    else {
	setAlarmEvent(&DefaultAlarm, 0, AL_UNSET, FUNCNAME_nulcmd, NULL);
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

AlarmEvent *
setAlarmEvent(AlarmEvent * event, int sec, short status, int cmd, void *data)
{
    if (event == NULL)
	event = New(AlarmEvent);
    event->sec = sec;
    event->status = status;
    event->cmd = cmd;
    event->data = data;
    return event;
}

DEFUN(reinit, REINIT, "Reload configuration file")
{
    char *resource = searchKeyData();

    if (resource == NULL) {
	init_rc();
	sync_with_option();
	initCookie();
	displayBuffer(Currentbuf, B_REDRAW_IMAGE);
	return;
    }

    if (!strcasecmp(resource, "CONFIG") || !strcasecmp(resource, "RC")) {
	init_rc();
	sync_with_option();
	displayBuffer(Currentbuf, B_REDRAW_IMAGE);
	return;
    }

    if (!strcasecmp(resource, "COOKIE")) {
	initCookie();
	return;
    }

    if (!strcasecmp(resource, "KEYMAP")) {
	initKeymap(TRUE);
	return;
    }

    if (!strcasecmp(resource, "MAILCAP")) {
	initMailcap();
	return;
    }


    if (!strcasecmp(resource, "MENU")) {
	initMenu();
	return;
    }

    if (!strcasecmp(resource, "MIMETYPES")) {
	initMimeTypes();
	return;
    }

    if (!strcasecmp(resource, "URIMETHODS")) {
	initURIMethods();
	return;
    }

    disp_err_message(Sprintf("Don't know how to reinitialize '%s'", resource)->
		     ptr, FALSE);
}

DEFUN(defKey, DEFINE_KEY, "Define a binding between a key stroke combination and a command")
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
}

DEFUN(newT, NEW_TAB, "Open a new tab (with current document)")
{
    _newT();
    displayBuffer(Currentbuf, B_REDRAW_IMAGE);
}

static TabBuffer *
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

void
calcTabPos(void)
{
    TabBuffer *tab;
    int lcol = 0, rcol = 0, col;
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
    buf = tab->firstBuffer;
    while (buf && buf != NO_BUFFER) {
	next = buf->nextBuffer;
	discardBuffer(buf);
	buf = next;
    }
    return FirstTab;
}

DEFUN(closeT, CLOSE_TAB, "Close tab")
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

DEFUN(nextT, NEXT_TAB, "Switch to the next tab")
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

DEFUN(prevT, PREV_TAB, "Switch to the previous tab")
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

static void
followTab(TabBuffer * tab)
{
    Buffer *buf;
    Anchor *a;

    a = retrieveCurrentImg(Currentbuf);
    if (!(a && a->image && a->image->map))
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

DEFUN(tabA, TAB_LINK, "Follow current hyperlink in a new tab")
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

DEFUN(tabURL, TAB_GOTO, "Open specified document in a new tab")
{
    tabURL0(prec_num ? numTab(PREC_NUM) : NULL,
	    "Goto URL on new tab: ", FALSE);
}

DEFUN(tabrURL, TAB_GOTO_RELATIVE, "Open relative address in a new tab")
{
    tabURL0(prec_num ? numTab(PREC_NUM) : NULL,
	    "Goto relative URL on new tab: ", TRUE);
}

static void
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
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

DEFUN(tabR, TAB_RIGHT, "Move right along the tab bar")
{
    TabBuffer *tab;
    int i;

    for (tab = CurrentTab, i = 0; tab && i < PREC_NUM;
	 tab = tab->nextTab, i++) ;
    moveTab(CurrentTab, tab ? tab : LastTab, TRUE);
}

DEFUN(tabL, TAB_LEFT, "Move left along the tab bar")
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
    d->save = expandPath(save);
    d->lock = lock;
    d->size = size;
    d->time = time(0);
    d->running = TRUE;
    d->err = 0;
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
	if (d->running && !lstat(d->lock, &st))
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
    /* FIXME: gettextize? */
    src = Strnew_charp("<html><head><title>" DOWNLOAD_LIST_TITLE
		       "</title></head>\n<body><h1 align=center>"
		       DOWNLOAD_LIST_TITLE "</h1>\n"
		       "<form method=internal action=download><hr>\n");
    for (d = LastDL; d != NULL; d = d->prev) {
	if (lstat(d->lock, &st))
	    d->running = FALSE;
	Strcat_charp(src, "<pre>\n");
	Strcat(src, Sprintf("%s\n  --&gt; %s\n  ", html_quote(d->url),
			    html_quote(conv_from_system(d->save))));
	duration = cur_time - d->time;
	if (!stat(d->save, &st)) {
	    size = st.st_size;
	    if (!d->running) {
		if (!d->err)
		    d->size = size;
		duration = st.st_mtime - d->time;
	    }
	}
	else
	    size = 0;
	if (d->size) {
	    int i, l = COLS - 6;
	    if (size < d->size)
		i = 1.0 * l * size / d->size;
	    else
		i = l;
	    l -= i;
	    while (i-- > 0)
		Strcat_char(src, '#');
	    while (l-- > 0)
		Strcat_char(src, '_');
	    Strcat_char(src, '\n');
	}
	if ((d->running || d->err) && size < d->size)
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
	    if (d->running && size < d->size && rate) {
		eta = (d->size - size) / rate;
		Strcat(src, Sprintf("  eta %02d:%02d:%02d", eta / (60 * 60),
				    (eta / 60) % 60, eta % 60));
	    }
	}
	Strcat_char(src, '\n');
	if (!d->running) {
	    Strcat(src, Sprintf("<input type=submit name=ok%d value=OK>",
				d->pid));
	    switch (d->err) {
	    case 0: if (size < d->size)
			Strcat_charp(src, " Download ended but probably not complete");
		    else
			Strcat_charp(src, " Download complete");
		    break;
	    case 1: Strcat_charp(src, " Error: could not open destination file");
		    break;
	    case 2: Strcat_charp(src, " Error: could not write to file (disk full)");
		    break;
	    default: Strcat_charp(src, " Error: unknown reason");
	    }
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
	if (!d->running)
	    continue;
	kill(d->pid, SIGKILL);
	unlink(d->lock);
    }
}

/* download panel */
DEFUN(ldDL, DOWNLOAD_LIST, "Display downloads panel")
{
    Buffer *buf;
    int replace = FALSE, new_tab = FALSE;
    int reload;

    if (Currentbuf->bufferprop & BP_INTERNAL &&
	!strcmp(Currentbuf->buffername, DOWNLOAD_LIST_TITLE))
	replace = TRUE;
    if (!FirstDL) {
	if (replace) {
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
    reload = checkDownloadList();
    buf = DownloadListBuffer();
    if (!buf) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    buf->bufferprop |= (BP_INTERNAL | BP_NO_URL);
    if (replace) {
	COPY_BUFROOT(buf, Currentbuf);
	restorePosition(buf, Currentbuf);
    }
    if (!replace && open_tab_dl_list) {
	_newT();
	new_tab = TRUE;
    }
    pushBuffer(buf);
    if (replace || new_tab)
	deletePrevBuf();
    if (reload)
	Currentbuf->event = setAlarmEvent(Currentbuf->event, 1, AL_IMPLICIT,
					  FUNCNAME_reload, NULL);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
save_buffer_position(Buffer *buf)
{
    BufferPos *b = buf->undo;

    if (!buf->firstLine)
	return;
    if (b && b->top_linenumber == TOP_LINENUMBER(buf) &&
	b->cur_linenumber == CUR_LINENUMBER(buf) &&
	b->currentColumn == buf->currentColumn && b->pos == buf->pos)
	return;
    b = New(BufferPos);
    b->top_linenumber = TOP_LINENUMBER(buf);
    b->cur_linenumber = CUR_LINENUMBER(buf);
    b->currentColumn = buf->currentColumn;
    b->pos = buf->pos;
    b->bpos = buf->currentLine ? buf->currentLine->bpos : 0;
    b->next = NULL;
    b->prev = buf->undo;
    if (buf->undo)
	buf->undo->next = b;
    buf->undo = b;
}

static void
resetPos(BufferPos * b)
{
    Buffer buf;
    Line top, cur;

    top.linenumber = b->top_linenumber;
    cur.linenumber = b->cur_linenumber;
    cur.bpos = b->bpos;
    buf.topLine = &top;
    buf.currentLine = &cur;
    buf.pos = b->pos;
    buf.currentColumn = b->currentColumn;
    restorePosition(Currentbuf, &buf);
    Currentbuf->undo = b;
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

DEFUN(undoPos, UNDO, "Cancel the last cursor movement")
{
    BufferPos *b = Currentbuf->undo;
    int i;

    if (!Currentbuf->firstLine)
	return;
    if (!b || !b->prev)
	return;
    for (i = 0; i < PREC_NUM && b->prev; i++, b = b->prev) ;
    resetPos(b);
}

DEFUN(redoPos, REDO, "Cancel the last undo")
{
    BufferPos *b = Currentbuf->undo;
    int i;

    if (!Currentbuf->firstLine)
	return;
    if (!b || !b->next)
	return;
    for (i = 0; i < PREC_NUM && b->next; i++, b = b->next) ;
    resetPos(b);
}

DEFUN(cursorTop, CURSOR_TOP, "Move cursor to the top of the screen")
{
    if (Currentbuf->firstLine == NULL)
        return;
    Currentbuf->currentLine = lineSkip(Currentbuf, Currentbuf->topLine,
                                       0, FALSE);
    arrangeLine(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

DEFUN(cursorMiddle, CURSOR_MIDDLE, "Move cursor to the middle of the screen")
{
    int offsety;
    if (Currentbuf->firstLine == NULL)
        return;
    offsety = (Currentbuf->LINES - 1) / 2;
    Currentbuf->currentLine = currentLineSkip(Currentbuf, Currentbuf->topLine,
                                              offsety, FALSE);
    arrangeLine(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}

DEFUN(cursorBottom, CURSOR_BOTTOM, "Move cursor to the bottom of the screen")
{
    int offsety;
    if (Currentbuf->firstLine == NULL)
        return;
    offsety = Currentbuf->LINES - 1;
    Currentbuf->currentLine = currentLineSkip(Currentbuf, Currentbuf->topLine,
                                              offsety, FALSE);
    arrangeLine(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
}
