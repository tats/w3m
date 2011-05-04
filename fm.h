/* $Id: fm.h,v 1.149 2010/08/20 09:47:09 htrb Exp $ */
/* 
 * w3m: WWW wo Miru utility
 * 
 * by A.ITO  Feb. 1995
 * 
 * You can use,copy,modify and distribute this program without any permission.
 */

#ifndef FM_H
#define FM_H


#ifndef _GNU_SOURCE
#define _GNU_SOURCE		/* strcasestr() */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"
#include "history.h"

#ifdef USE_MENU
#define MENU_SELECT
#define MENU_MAP
#endif				/* USE_MENU */

#ifndef USE_COLOR
#undef USE_ANSI_COLOR
#undef USE_BG_COLOR
#endif

#include "ctrlcode.h"
#include "html.h"
#include <gc.h>
#include "Str.h"
#ifdef USE_M17N
#include "wc.h"
#include "wtf.h"
#else
typedef int wc_ces;	/* XXX: not used */
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#if !HAVE_SETLOCALE
#define setlocale(category, locale)	/* empty */
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext (String)
#define N_(String) (String)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory)	/* empty */
# undef textdomain
# define textdomain(Domain)	/* empty */
# define _(Text) Text
# define N_(Text) Text
# define gettext(Text) Text
#endif

#include "form.h"
#include "frame.h"
#include "parsetag.h"
#include "parsetagx.h"
#include "func.h"
#include "menu.h"
#include "textlist.h"
#include "funcname1.h"
#include "terms.h"

#ifndef HAVE_BCOPY
void bcopy(const void *, void *, int);
void bzero(void *, int);
#endif				/* HAVE_BCOPY */
#ifdef __EMX__
#include <strings.h>		/* for bzero() and bcopy() */
#endif

#ifdef MAINPROGRAM
#define global
#define init(x) =(x)
#else				/* not MAINPROGRAM */
#define global extern
#define init(x)
#endif				/* not MAINPROGRAM */

#define DEFUN(funcname, macroname, docstring) void funcname(void)

/* 
 * Constants.
 */
#define LINELEN	256		/* Initial line length */
#define PAGER_MAX_LINE	10000	/* Maximum line kept as pager */

#define MAXIMUM_COLS 1024
#define DEFAULT_COLS 80

#ifdef USE_IMAGE
#define MAX_IMAGE 1000
#define MAX_IMAGE_SIZE 2048

#define DEFAULT_PIXEL_PER_CHAR  7.0	/* arbitrary */
#define DEFAULT_PIXEL_PER_LINE  14.0	/* arbitrary */
#else
#define DEFAULT_PIXEL_PER_CHAR  8.0	/* arbitrary */
#endif
#define MINIMUM_PIXEL_PER_CHAR  4.0
#define MAXIMUM_PIXEL_PER_CHAR  32.0

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#define FALSE 0
#define TRUE   1

#define SHELLBUFFERNAME	"*Shellout*"
#define PIPEBUFFERNAME	"*stream*"
#define CPIPEBUFFERNAME	"*stream(closed)*"
#ifdef USE_DICT
#define DICTBUFFERNAME "*dictionary*"
#endif				/* USE_DICT */

/* 
 * Line Property
 */

#define P_CHARTYPE	0x3f00
#ifdef USE_M17N
#define PC_ASCII	(WTF_TYPE_ASCII << 8)
#define PC_CTRL		(WTF_TYPE_CTRL << 8)
#define PC_WCHAR1	(WTF_TYPE_WCHAR1 << 8)
#define PC_WCHAR2	(WTF_TYPE_WCHAR2 << 8)
#define PC_KANJI	(WTF_TYPE_WIDE << 8)
#define PC_KANJI1	(PC_WCHAR1 | PC_KANJI)
#define PC_KANJI2	(PC_WCHAR2 | PC_KANJI)
#define PC_UNKNOWN	(WTF_TYPE_UNKNOWN << 8)
#define PC_UNDEF	(WTF_TYPE_UNDEF << 8)
#else
#define PC_ASCII	0x0000
#define PC_CTRL		0x0100
#endif
#define PC_SYMBOL       0x8000

/* Effect ( standout/underline ) */
#define P_EFFECT	0x40ff
#define PE_NORMAL	0x00
#define PE_MARK		0x01
#define PE_UNDER	0x02
#define PE_STAND	0x04
#define PE_BOLD		0x08
#define PE_ANCHOR       0x10
#define PE_EMPH         0x08
#define PE_IMAGE        0x20
#define PE_FORM         0x40
#define PE_ACTIVE	0x80
#define PE_VISITED	0x4000

/* Extra effect */
#define PE_EX_ITALIC	0x01
#define PE_EX_INSERT	0x02
#define PE_EX_STRIKE	0x04

#define PE_EX_ITALIC_E	PE_UNDER
#define PE_EX_INSERT_E	PE_UNDER
#define PE_EX_STRIKE_E	PE_STAND

#define CharType(c)	((c)&P_CHARTYPE)
#define CharEffect(c)	((c)&(P_EFFECT|PC_SYMBOL))
#define SetCharType(v,c)	((v)=(((v)&~P_CHARTYPE)|(c)))


#define COLPOS(l,c)	calcPosition(l->lineBuf,l->propBuf,l->len,c,0,CP_AUTO)

/* Flags for displayBuffer() */
#define B_NORMAL	0
#define B_FORCE_REDRAW	1
#define B_REDRAW	2
#define B_SCROLL        3
#define B_REDRAW_IMAGE	4

/* Buffer Property */
#define BP_NORMAL	0x0
#define BP_PIPE		0x1
#define BP_FRAME	0x2
#define BP_INTERNAL	0x8
#define BP_NO_URL	0x10
#define BP_REDIRECTED   0x20
#define BP_CLOSE        0x40

/* Link Buffer */
#define LB_NOLINK	-1
#define LB_FRAME	0	/* rFrame() */
#define LB_N_FRAME	1
#define LB_INFO		2	/* pginfo() */
#define LB_N_INFO	3
#define LB_SOURCE	4	/* vwSrc() */
#define LB_N_SOURCE	LB_SOURCE
#define MAX_LB		5

/* Search Result */
#define SR_FOUND       0x1
#define SR_NOTFOUND    0x2
#define SR_WRAPPED     0x4

#ifdef MAINPROGRAM
int REV_LB[MAX_LB] = {
    LB_N_FRAME, LB_FRAME, LB_N_INFO, LB_INFO, LB_N_SOURCE,
};
#else				/* not MAINPROGRAM */
extern int REV_LB[];
#endif				/* not MAINPROGRAM */

/* mark URL, Message-ID */
#define CHK_URL                1
#define CHK_NMID       2

/* Flags for calcPosition() */
#define CP_AUTO		0
#define CP_FORCE	1

/* Completion status. */
#define CPL_OK		0
#define CPL_AMBIG	1
#define CPL_FAIL	2
#define CPL_MENU	3

#define CPL_NEVER	0x0
#define CPL_OFF		0x1
#define CPL_ON		0x2
#define CPL_ALWAYS	0x4
#define CPL_URL		0x8

/* Flags for inputLine() */
#define IN_STRING	0x10
#define IN_FILENAME	0x20
#define IN_PASSWORD	0x40
#define IN_COMMAND	0x80
#define IN_URL		0x100
#define IN_CHAR		0x200

#define IMG_FLAG_SKIP	1
#define IMG_FLAG_AUTO	2

#define IMG_FLAG_START	0
#define IMG_FLAG_STOP	1
#define IMG_FLAG_NEXT	2

#define IMG_FLAG_UNLOADED	0
#define IMG_FLAG_LOADED		1
#define IMG_FLAG_ERROR		2
#define IMG_FLAG_DONT_REMOVE	4

/* 
 * Macros.
 */

#define inputLineHist(p,d,f,h)	inputLineHistSearch(p,d,f,h,NULL)
#define inputLine(p,d,f)	inputLineHist(p,d,f,NULL)
#define inputStr(p,d)		inputLine(p,d,IN_STRING)
#define inputStrHist(p,d,h)	inputLineHist(p,d,IN_STRING,h)
#define inputFilename(p,d)	inputLine(p,d,IN_FILENAME)
#define inputFilenameHist(p,d,h)	inputLineHist(p,d,IN_FILENAME,h)
#define inputChar(p)		inputLine(p,"",IN_CHAR)

#define free(x)  GC_free(x)	/* let GC do it. */

#ifdef __EMX__
#define HAVE_STRCASECMP
#define strcasecmp	stricmp
#define strncasecmp	strnicmp
#endif				/* __EMX__ */


#define SKIP_BLANKS(p) {while(*(p)&&IS_SPACE(*(p)))(p)++;}
#define SKIP_NON_BLANKS(p) {while(*(p)&&!IS_SPACE(*(p)))(p)++;}
#define IS_ENDL(c) ((c)=='\0'||(c)=='\r'||(c)=='\n')
#define IS_ENDT(c) (IS_ENDL(c)||(c)==';')

#define bpcmp(a,b) \
  (((a).line - (b).line) ? ((a).line - (b).line) : ((a).pos - (b).pos))

#define RELATIVE_WIDTH(w)   (((w)>=0)?(int)((w)/pixel_per_char):(w))
#define REAL_WIDTH(w,limit) (((w)>=0)?(int)((w)/pixel_per_char):-(w)*(limit)/100)

#define EOL(l) (&(l)->ptr[(l)->length])
#define IS_EOL(p,l) ((p)==&(l)->ptr[(l)->length])

/* 
 * Types.
 */

typedef unsigned short Lineprop;
#ifdef USE_ANSI_COLOR
typedef unsigned char Linecolor;
#endif

typedef struct _MapArea {
    char *url;
    char *target;
    char *alt;
#ifdef USE_IMAGE
    char shape;
    short *coords;
    int ncoords;
    short center_x;
    short center_y;
#endif
} MapArea;

typedef struct _MapList {
    Str name;
    GeneralList *area;
    struct _MapList *next;
} MapList;

typedef struct _Line {
    char *lineBuf;
    Lineprop *propBuf;
#ifdef USE_ANSI_COLOR
    Linecolor *colorBuf;
#endif
    struct _Line *next;
    struct _Line *prev;
    int len;
    int width;
    long linenumber;		/* on buffer */
    long real_linenumber;	/* on file */
    unsigned short usrflags;
    int size;
    int bpos;
    int bwidth;
} Line;

typedef struct {
    int line;
    int pos;
    int invalid;
} BufferPoint;

#ifdef USE_IMAGE
typedef struct _imageCache {
    char *url;
    ParsedURL *current;
    char *file;
    char *touch;
    pid_t pid;
    char loaded;
    int index;
    short width;
    short height;
} ImageCache;

typedef struct _image {
    char *url;
    char *ext;
    short width;
    short height;
    short xoffset;
    short yoffset;
    short y;
    short rows;
    char *map;
    char ismap;
    int touch;
    ImageCache *cache;
} Image;
#endif

typedef struct _anchor {
    char *url;
    char *target;
    char *referer;
    char *title;
    unsigned char accesskey;
    BufferPoint start;
    BufferPoint end;
    int hseq;
    char slave;
    short y;
    short rows;
#ifdef USE_IMAGE
    Image *image;
#endif
} Anchor;

#define NO_REFERER ((char*)-1)

typedef struct _anchorList {
    Anchor *anchors;
    int nanchor;
    int anchormax;
    int acache;
} AnchorList;

typedef struct {
    BufferPoint *marks;
    int nmark;
    int markmax;
    int prevhseq;
} HmarkerList;

#define LINK_TYPE_NONE 0
#define LINK_TYPE_REL  1
#define LINK_TYPE_REV  2
typedef struct _LinkList {
    char *url;
    char *title;		/* Next, Contents, ... */
    char *ctype;		/* Content-Type */
    char type;			/* Rel, Rev */
    struct _LinkList *next;
} LinkList;

typedef struct _Buffer {
    char *filename;
    char *buffername;
    Line *firstLine;
    Line *topLine;
    Line *currentLine;
    Line *lastLine;
    struct _Buffer *nextBuffer;
    struct _Buffer *linkBuffer[MAX_LB];
    short width;
    short height;
    char *type;
    char *real_type;
    int allLine;
    short bufferprop;
    int currentColumn;
    short cursorX;
    short cursorY;
    int pos;
    int visualpos;
    short rootX;
    short rootY;
    short COLS;
    short LINES;
    InputStream pagerSource;
    AnchorList *href;
    AnchorList *name;
    AnchorList *img;
    AnchorList *formitem;
    LinkList *linklist;
    FormList *formlist;
    MapList *maplist;
    HmarkerList *hmarklist;
    HmarkerList *imarklist;
    ParsedURL currentURL;
    ParsedURL *baseURL;
    char *baseTarget;
    int real_scheme;
    char *sourcefile;
    struct frameset *frameset;
    struct frameset_queue *frameQ;
    int *clone;
    size_t trbyte;
    char check_url;
#ifdef USE_M17N
    wc_ces document_charset;
    wc_uint8 auto_detect;
#endif
    TextList *document_header;
    FormItemList *form_submit;
    char *savecache;
    char *edit;
    struct mailcap *mailcap;
    char *mailcap_source;
    char *header_source;
    char search_header;
#ifdef USE_SSL
    char *ssl_certificate;
#endif
    char image_flag;
    char image_loaded;
    char need_reshape;
    Anchor *submit;
    struct _BufferPos *undo;
#ifdef USE_ALARM
    struct _AlarmEvent *event;
#endif
} Buffer;

typedef struct _BufferPos {
    long top_linenumber;
    long cur_linenumber;
    int currentColumn;
    int pos;
    int bpos;
    struct _BufferPos *next;
    struct _BufferPos *prev;
} BufferPos;

typedef struct _TabBuffer {
    struct _TabBuffer *nextTab;
    struct _TabBuffer *prevTab;
    Buffer *currentBuffer;
    Buffer *firstBuffer;
    short x1;
    short x2;
    short y;
} TabBuffer;

typedef struct _DownloadList {
    pid_t pid;
    char *url;
    char *save;
    char *lock;
    clen_t size;
    time_t time;
    int running;
    int err;
    struct _DownloadList *next;
    struct _DownloadList *prev;
} DownloadList;
#define DOWNLOAD_LIST_TITLE "Download List Panel"

#define COPY_BUFROOT(dstbuf, srcbuf) {\
 (dstbuf)->rootX = (srcbuf)->rootX; \
 (dstbuf)->rootY = (srcbuf)->rootY; \
 (dstbuf)->COLS = (srcbuf)->COLS; \
 (dstbuf)->LINES = (srcbuf)->LINES; \
}

#define COPY_BUFPOSITION(dstbuf, srcbuf) {\
 (dstbuf)->topLine = (srcbuf)->topLine; \
 (dstbuf)->currentLine = (srcbuf)->currentLine; \
 (dstbuf)->pos = (srcbuf)->pos; \
 (dstbuf)->cursorX = (srcbuf)->cursorX; \
 (dstbuf)->cursorY = (srcbuf)->cursorY; \
 (dstbuf)->visualpos = (srcbuf)->visualpos; \
 (dstbuf)->currentColumn = (srcbuf)->currentColumn; \
}
#define SAVE_BUFPOSITION(sbufp) COPY_BUFPOSITION(sbufp, Currentbuf)
#define RESTORE_BUFPOSITION(sbufp) COPY_BUFPOSITION(Currentbuf, sbufp)
#define TOP_LINENUMBER(buf) ((buf)->topLine ? (buf)->topLine->linenumber : 1)
#define CUR_LINENUMBER(buf) ((buf)->currentLine ? (buf)->currentLine->linenumber : 1)

#define NO_BUFFER ((Buffer*)1)

#define RB_STACK_SIZE 10

#define TAG_STACK_SIZE 10

#define FONT_STACK_SIZE 5

#define FONTSTAT_SIZE 7

#define _INIT_BUFFER_WIDTH (COLS - (showLineNum ? 6 : 1))
#define INIT_BUFFER_WIDTH ((_INIT_BUFFER_WIDTH > 0) ? _INIT_BUFFER_WIDTH : 0)
#define FOLD_BUFFER_WIDTH (FoldLine ? (INIT_BUFFER_WIDTH + 1) : -1)

typedef struct {
    int pos;
    int len;
    int tlen;
    long flag;
    Anchor anchor;
    Str img_alt;
    char fontstat[FONTSTAT_SIZE];
    short nobr_level;
    Lineprop prev_ctype;
    char init_flag;
    short top_margin;
    short bottom_margin;
} Breakpoint;

struct readbuffer {
    Str line;
    Lineprop cprop;
    short pos;
    Str prevchar;
    long flag;
    long flag_stack[RB_STACK_SIZE];
    int flag_sp;
    int status;
    unsigned char end_tag;
    short table_level;
    short nobr_level;
    Anchor anchor;
    Str img_alt;
    char fontstat[FONTSTAT_SIZE];
    char fontstat_stack[FONT_STACK_SIZE][FONTSTAT_SIZE];
    int fontstat_sp;
    Lineprop prev_ctype;
    Breakpoint bp;
    struct cmdtable *tag_stack[TAG_STACK_SIZE];
    int tag_sp;
    short top_margin;
    short bottom_margin;
};

#define in_bold fontstat[0]
#define in_under fontstat[1]
#define in_italic fontstat[2]
#define in_strike fontstat[3]
#define in_ins fontstat[4]
#define in_stand fontstat[5]

#define RB_PRE		0x01
#define RB_SCRIPT	0x02
#define RB_STYLE	0x04
#define RB_PLAIN	0x08
#define RB_LEFT		0x10
#define RB_CENTER	0x20
#define RB_RIGHT	0x40
#define RB_ALIGN	(RB_LEFT| RB_CENTER | RB_RIGHT)
#define RB_NOBR		0x80
#define RB_P		0x100
#define RB_PRE_INT	0x200
#define RB_IN_DT	0x400
#define RB_INTXTA	0x800
#define RB_INSELECT	0x1000
#define RB_IGNORE_P	0x2000
#define RB_TITLE	0x4000
#define RB_NFLUSHED	0x8000
#define RB_NOFRAMES	0x10000
#define RB_INTABLE	0x20000
#define RB_PREMODE	(RB_PRE | RB_PRE_INT | RB_SCRIPT | RB_STYLE | RB_PLAIN | RB_INTXTA)
#define RB_SPECIAL	(RB_PRE | RB_PRE_INT | RB_SCRIPT | RB_STYLE | RB_PLAIN | RB_NOBR)
#define RB_PLAIN_PRE	0x40000

#ifdef FORMAT_NICE
#define RB_FILL		0x80000
#endif				/* FORMAT_NICE */
#define RB_DEL		0x100000
#define RB_S		0x200000

#define RB_GET_ALIGN(obuf) ((obuf)->flag&RB_ALIGN)
#define RB_SET_ALIGN(obuf,align) {(obuf)->flag &= ~RB_ALIGN; (obuf)->flag |= (align); }
#define RB_SAVE_FLAG(obuf) {\
  if ((obuf)->flag_sp < RB_STACK_SIZE) \
    (obuf)->flag_stack[(obuf)->flag_sp++] = RB_GET_ALIGN(obuf); \
}
#define RB_RESTORE_FLAG(obuf) {\
  if ((obuf)->flag_sp > 0) \
   RB_SET_ALIGN(obuf,(obuf)->flag_stack[--(obuf)->flag_sp]); \
}

/* status flags */
#define R_ST_NORMAL 0		/* normal */
#define R_ST_TAG0   1		/* within tag, just after < */
#define R_ST_TAG    2		/* within tag */
#define R_ST_QUOTE  3		/* within single quote */
#define R_ST_DQUOTE 4		/* within double quote */
#define R_ST_EQL    5		/* = */
#define R_ST_AMP    6		/* within ampersand quote */
#define R_ST_EOL    7		/* end of file */
#define R_ST_CMNT1  8		/* <!  */
#define R_ST_CMNT2  9		/* <!- */
#define R_ST_CMNT   10		/* within comment */
#define R_ST_NCMNT1 11		/* comment - */
#define R_ST_NCMNT2 12		/* comment -- */
#define R_ST_NCMNT3 13		/* comment -- space */
#define R_ST_IRRTAG 14		/* within irregular tag */
#define R_ST_VALUE  15		/* within tag attribule value */

#define ST_IS_REAL_TAG(s)   ((s)==R_ST_TAG||(s)==R_ST_TAG0||(s)==R_ST_EQL||(s)==R_ST_VALUE)

/* is this '<' really means the beginning of a tag? */
#define REALLY_THE_BEGINNING_OF_A_TAG(p) \
	  (IS_ALPHA(p[1]) || p[1] == '/' || p[1] == '!' || p[1] == '?' || p[1] == '\0' || p[1] == '_')

/* flags for loadGeneralFile */
#define RG_NOCACHE   1
#define RG_FRAME     2
#define RG_FRAME_SRC 4

struct html_feed_environ {
    struct readbuffer *obuf;
    TextLineList *buf;
    FILE *f;
    Str tagbuf;
    int limit;
    int maxlimit;
    struct environment *envs;
    int nenv;
    int envc;
    int envc_real;
    char *title;
    int blank_lines;
};

#ifdef USE_COOKIE
struct portlist {
    unsigned short port;
    struct portlist *next;
};

struct cookie {
    ParsedURL url;
    Str name;
    Str value;
    time_t expires;
    Str path;
    Str domain;
    Str comment;
    Str commentURL;
    struct portlist *portl;
    char version;
    char flag;
    struct cookie *next;
};
#define COO_USE		1
#define COO_SECURE	2
#define COO_DOMAIN	4
#define COO_PATH	8
#define COO_DISCARD	16
#define COO_OVERRIDE	32	/* user chose to override security checks */

#define COO_OVERRIDE_OK	32	/* flag to specify that an error is overridable */
						/* version 0 refers to the original cookie_spec.html */
						/* version 1 refers to RFC 2109 */
						/* version 1' refers to the Internet draft to obsolete RFC 2109 */
#define COO_EINTERNAL	(1)	/* unknown error; probably forgot to convert "return 1" in cookie.c */
#define COO_ETAIL	(2 | COO_OVERRIDE_OK)	/* tail match failed (version 0) */
#define COO_ESPECIAL	(3)	/* special domain check failed (version 0) */
#define COO_EPATH	(4)	/* Path attribute mismatch (version 1 case 1) */
#define COO_ENODOT	(5 | COO_OVERRIDE_OK)	/* no embedded dots in Domain (version 1 case 2.1) */
#define COO_ENOTV1DOM	(6 | COO_OVERRIDE_OK)	/* Domain does not start with a dot (version 1 case 2.2) */
#define COO_EDOM	(7 | COO_OVERRIDE_OK)	/* domain-match failed (version 1 case 3) */
#define COO_EBADHOST	(8 | COO_OVERRIDE_OK)	/* dot in matched host name in FQDN (version 1 case 4) */
#define COO_EPORT	(9)	/* Port match failed (version 1' case 5) */
#define COO_EMAX	COO_EPORT
#endif				/* USE_COOKIE */

/* modes for align() */

#define ALIGN_CENTER 0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_MIDDLE 4
#define ALIGN_TOP    5
#define ALIGN_BOTTOM 6

#define VALIGN_MIDDLE 0
#define VALIGN_TOP    1
#define VALIGN_BOTTOM 2

typedef struct http_request {
    char command;
    char flag;
    char *referer;
    FormList *request;
} HRequest;

#define HR_COMMAND_GET		0
#define HR_COMMAND_POST		1
#define HR_COMMAND_CONNECT	2
#define HR_COMMAND_HEAD		3

#define HR_FLAG_LOCAL		1
#define HR_FLAG_PROXY		2

#define HTST_UNKNOWN		255
#define HTST_MISSING		254
#define HTST_NORMAL		0
#define HTST_CONNECT		1

#define TMPF_DFL	0
#define TMPF_SRC	1
#define TMPF_FRAME	2
#define TMPF_CACHE	3
#define TMPF_COOKIE	4
#define MAX_TMPF_TYPE	5

#define set_no_proxy(domains) (NO_proxy_domains=make_domain_list(domains))

/* 
 * Globals.
 */

extern int LINES, COLS;
#if defined(__CYGWIN__)
extern int LASTLINE;
#else				/* not defined(__CYGWIN__) */
#define LASTLINE (LINES-1)
#endif				/* not defined(__CYGWIN__) */

global int Tabstop init(8);
global int IndentIncr init(4);
global int ShowEffect init(TRUE);
global int PagerMax init(PAGER_MAX_LINE);

global char SearchHeader init(FALSE);
global char *DefaultType init(NULL);
global char RenderFrame init(FALSE);
global char TargetSelf init(FALSE);
global char PermitSaveToPipe init(FALSE);
global char DecodeCTE init(FALSE);
global char AutoUncompress init(FALSE);
global char PreserveTimestamp init(TRUE);
global char ArgvIsURL init(FALSE);
global char MetaRefresh init(FALSE);

global char fmInitialized init(FALSE);
global char QuietMessage init(FALSE);
global char TrapSignal init(TRUE);
#define TRAP_ON if (TrapSignal) { \
    prevtrap = mySignal(SIGINT, KeyAbort); \
    if (fmInitialized) \
	term_cbreak(); \
}
#define TRAP_OFF if (TrapSignal) { \
    if (fmInitialized) \
	term_raw(); \
    if (prevtrap) \
	mySignal(SIGINT, prevtrap); \
}

extern unsigned char GlobalKeymap[];
extern unsigned char EscKeymap[];
extern unsigned char EscBKeymap[];
extern unsigned char EscDKeymap[];
#ifdef __EMX__
extern unsigned char PcKeymap[];
#endif
extern FuncList w3mFuncList[];

global char *HTTP_proxy init(NULL);
#ifdef USE_SSL
global char *HTTPS_proxy init(NULL);
#endif				/* USE_SSL */
#ifdef USE_GOPHER
global char *GOPHER_proxy init(NULL);
#endif				/* USE_GOPHER */
global char *FTP_proxy init(NULL);
global ParsedURL HTTP_proxy_parsed;
#ifdef USE_SSL
global ParsedURL HTTPS_proxy_parsed;
#endif				/* USE_SSL */
#ifdef USE_GOPHER
global ParsedURL GOPHER_proxy_parsed;
#endif				/* USE_GOPHER */
global ParsedURL FTP_proxy_parsed;
global char *NO_proxy init(NULL);
global int NOproxy_netaddr init(TRUE);
#ifdef INET6
#define DNS_ORDER_UNSPEC     0
#define DNS_ORDER_INET_INET6 1
#define DNS_ORDER_INET6_INET 2
#define DNS_ORDER_INET_ONLY  4
#define DNS_ORDER_INET6_ONLY 6
global int DNS_order init(DNS_ORDER_UNSPEC);
extern int ai_family_order_table[7][3];	/* XXX */
#endif				/* INET6 */
global TextList *NO_proxy_domains;
global char NoCache init(FALSE);
global char use_proxy init(TRUE);
#define Do_not_use_proxy (!use_proxy)
global int Do_not_use_ti_te init(FALSE);
#ifdef USE_NNTP
global char *NNTP_server init(NULL);
global char *NNTP_mode init(NULL);
global int MaxNewsMessage init(50);
#endif

global char *document_root init(NULL);
global char *personal_document_root init(NULL);
global char *cgi_bin init(NULL);
global char *index_file init(NULL);

global char *CurrentDir;
global int CurrentPid;
/*
 * global Buffer *Currentbuf;
 * global Buffer *Firstbuf;
 */
global TabBuffer *CurrentTab;
global TabBuffer *FirstTab;
global TabBuffer *LastTab;
global int open_tab_blank init(FALSE);
global int open_tab_dl_list init(FALSE);
global int close_tab_back init(FALSE);
global int nTab;
global int TabCols init(10);
#define NO_TABBUFFER ((TabBuffer *)1)
#define Currentbuf (CurrentTab->currentBuffer)
#define Firstbuf (CurrentTab->firstBuffer)
global DownloadList *FirstDL init(NULL);
global DownloadList *LastDL init(NULL);
global int CurrentKey;
global char *CurrentKeyData;
global char *CurrentCmdData;
global char *w3m_reqlog;
extern char *w3m_version;

#define DUMP_BUFFER   0x01
#define DUMP_HEAD     0x02
#define DUMP_SOURCE   0x04
#define DUMP_EXTRA    0x08
#define DUMP_HALFDUMP 0x10
#define DUMP_FRAME    0x20
global int w3m_debug;
global int w3m_dump init(0);
#define w3m_halfdump (w3m_dump & DUMP_HALFDUMP)
global int w3m_halfload init(FALSE);
global Str header_string init(NULL);
global int override_content_type init(FALSE);

#ifdef USE_COLOR
global int useColor init(TRUE);
global int basic_color init(8);	/* don't change */
global int anchor_color init(4);	/* blue  */
global int image_color init(2);	/* green */
global int form_color init(1);	/* red   */
#ifdef USE_BG_COLOR
global int bg_color init(8);	/* don't change */
global int mark_color init(6);	/* cyan */
#endif				/* USE_BG_COLOR */
global int useActiveColor init(FALSE);
global int active_color init(6);	/* cyan */
global int useVisitedColor init(FALSE);
global int visited_color init(5);	/* magenta  */
#endif				/* USE_COLOR */
global int confirm_on_quit init(TRUE);
#ifdef USE_MARK
global int use_mark init(FALSE);
#endif
global int emacs_like_lineedit init(FALSE);
global int vi_prec_num init(FALSE);
global int label_topline init(FALSE);
global int nextpage_topline init(FALSE);
global char *displayTitleTerm init(NULL);
global int displayLink init(FALSE);
global int displayLinkNumber init(FALSE);
global int displayLineInfo init(FALSE);
global int DecodeURL init(FALSE);
global int retryAsHttp init(TRUE);
global int showLineNum init(FALSE);
global int show_srch_str init(TRUE);
#ifdef USE_IMAGE
global char *Imgdisplay init(IMGDISPLAY);
global int activeImage init(FALSE);
global int displayImage init(TRUE);
global int autoImage init(TRUE);
global int useExtImageViewer init(TRUE);
global int maxLoadImage init(4);
global int image_map_list init(TRUE);
#else
global int displayImage init(FALSE);	/* XXX: emacs-w3m use display_image=off */
#endif
global int pseudoInlines init(TRUE);
global char *Editor init(DEF_EDITOR);
#ifdef USE_W3MMAILER
global char *Mailer init(NULL);
#else
global char *Mailer init(DEF_MAILER);
#endif
#ifdef USE_W3MMAILER
#define MAILTO_OPTIONS_USE_W3MMAILER 0
#endif
#define MAILTO_OPTIONS_IGNORE 1
#define MAILTO_OPTIONS_USE_MAILTO_URL 2
global int MailtoOptions init(MAILTO_OPTIONS_IGNORE);
global char *ExtBrowser init(DEF_EXT_BROWSER);
global char *ExtBrowser2 init(NULL);
global char *ExtBrowser3 init(NULL);
global int BackgroundExtViewer init(TRUE);
global int disable_secret_security_check init(FALSE);
global char *passwd_file init(PASSWD_FILE);
global char *pre_form_file init(PRE_FORM_FILE);
global char *ftppasswd init(NULL);
global int ftppass_hostnamegen init(TRUE);
global int do_download init(FALSE);
#ifdef USE_IMAGE
global char *image_source init(NULL);
#endif
global char *UserAgent init(NULL);
global int NoSendReferer init(FALSE);
global char *AcceptLang init(NULL);
global char *AcceptEncoding init(NULL);
global char *AcceptMedia init(NULL);
global int WrapDefault init(FALSE);
global int IgnoreCase init(TRUE);
global int WrapSearch init(FALSE);
global int squeezeBlankLine init(FALSE);
global char *BookmarkFile init(NULL);
global int UseExternalDirBuffer init(TRUE);
global char *DirBufferCommand init("file:///$LIB/dirlist" CGI_EXTENSION);
#ifdef USE_DICT
global int UseDictCommand init(FALSE);
global char *DictCommand init("file:///$LIB/w3mdict" CGI_EXTENSION);
#endif				/* USE_DICT */
global int ignore_null_img_alt init(TRUE);
#define DISPLAY_INS_DEL_SIMPLE	0
#define DISPLAY_INS_DEL_NORMAL	1
#define DISPLAY_INS_DEL_FONTIFY	2
global int displayInsDel init(DISPLAY_INS_DEL_NORMAL);
global int FoldTextarea init(FALSE);
global int FoldLine init(FALSE);
#define DEFAULT_URL_EMPTY	0
#define DEFAULT_URL_CURRENT	1
#define DEFAULT_URL_LINK	2
global int DefaultURLString init(DEFAULT_URL_EMPTY);
global int MarkAllPages init(FALSE);

#ifdef USE_MIGEMO
global int use_migemo init(FALSE);
global int migemo_active init(0);
global char *migemo_command init(DEF_MIGEMO_COMMAND);
#endif				/* USE_MIGEMO */

global struct auth_cookie *Auth_cookie init(NULL);
#ifdef USE_COOKIE
global struct cookie *First_cookie init(NULL);
#endif				/* USE_COOKIE */

global char *mailcap_files init(USER_MAILCAP ", " SYS_MAILCAP);
global char *mimetypes_files init(USER_MIMETYPES ", " SYS_MIMETYPES);
#ifdef USE_EXTERNAL_URI_LOADER
global char *urimethodmap_files init(USER_URIMETHODMAP ", " SYS_URIMETHODMAP);
#endif

global TextList *fileToDelete;

extern Hist *LoadHist;
extern Hist *SaveHist;
extern Hist *URLHist;
extern Hist *ShellHist;
extern Hist *TextHist;
#ifdef USE_HISTORY
global int UseHistory init(TRUE);
global int URLHistSize init(100);
global int SaveURLHist init(TRUE);
#endif				/* USE_HISTORY */
global int multicolList init(FALSE);

#ifdef USE_M17N
global wc_ces InnerCharset init(WC_CES_WTF);	/* Don't change */
global wc_ces DisplayCharset init(DISPLAY_CHARSET);
global wc_ces DocumentCharset init(DOCUMENT_CHARSET);
global wc_ces SystemCharset init(SYSTEM_CHARSET);
global wc_ces BookmarkCharset init(SYSTEM_CHARSET);
global char ExtHalfdump init(FALSE);
global char FollowLocale init(TRUE);
global char UseContentCharset init(TRUE);
global char SearchConv init(TRUE);
global char SimplePreserveSpace init(FALSE);
#define Str_conv_from_system(x) wc_Str_conv((x), SystemCharset, InnerCharset)
#define Str_conv_to_system(x) wc_Str_conv_strict((x), InnerCharset, SystemCharset)
#define Str_conv_to_halfdump(x) (ExtHalfdump ? wc_Str_conv((x), InnerCharset, DisplayCharset) : (x))
#define conv_from_system(x) wc_conv((x), SystemCharset, InnerCharset)->ptr
#define conv_to_system(x) wc_conv_strict((x), InnerCharset, SystemCharset)->ptr
#define url_quote_conv(x,c) url_quote(wc_conv_strict((x), InnerCharset, (c))->ptr)
#else
#define Str_conv_from_system(x) (x)
#define Str_conv_to_system(x) (x)
#define Str_conv_to_halfdump(x) (x)
#define conv_from_system(x) (x)
#define conv_to_system(x) (x)
#define url_quote_conv(x,c) url_quote(x)
#define wc_Str_conv(x,charset0,charset1) (x)
#define wc_Str_conv_strict(x,charset0,charset1) (x)
#endif
global char UseAltEntity init(TRUE);
#define GRAPHIC_CHAR_ASCII 2
#define GRAPHIC_CHAR_DEC 1
#define GRAPHIC_CHAR_CHARSET 0
global char UseGraphicChar init(GRAPHIC_CHAR_CHARSET);
extern char *graph_symbol[];
extern char *graph2_symbol[];
extern int symbol_width;
extern int symbol_width0;
#define N_GRAPH_SYMBOL 32
#define SYMBOL_BASE 0x20
global int no_rc_dir init(FALSE);
global char *rc_dir init(NULL);
global char *tmp_dir;
global char *config_file init(NULL);

#ifdef USE_MOUSE
global int use_mouse init(TRUE);
extern int mouseActive;
global int reverse_mouse init(FALSE);
global int relative_wheel_scroll init(FALSE);
global int fixed_wheel_scroll_count init(5);
global int relative_wheel_scroll_ratio init(30);
typedef struct _MouseActionMap {
    void (*func) ();
    char *data;
} MouseActionMap;
typedef struct _MouseAction {
    char *menu_str;
    char *lastline_str;
    int menu_width;
    int lastline_width;
    int in_action;
    int cursorX;
    int cursorY;
    MouseActionMap default_map[3];
    MouseActionMap anchor_map[3];
    MouseActionMap active_map[3];
    MouseActionMap tab_map[3];
    MouseActionMap *menu_map[3];
    MouseActionMap *lastline_map[3];
} MouseAction;
global MouseAction mouse_action;
#define LIMIT_MOUSE_MENU 100
#endif				/* USE_MOUSE */

#ifdef USE_COOKIE
global int default_use_cookie init(TRUE);
global int use_cookie init(FALSE);
global int show_cookie init(TRUE);
global int accept_cookie init(FALSE);
#define ACCEPT_BAD_COOKIE_DISCARD	0
#define ACCEPT_BAD_COOKIE_ACCEPT	1
#define ACCEPT_BAD_COOKIE_ASK		2
global int accept_bad_cookie init(ACCEPT_BAD_COOKIE_DISCARD);
global char *cookie_reject_domains init(NULL);
global char *cookie_accept_domains init(NULL);
global char *cookie_avoid_wrong_number_of_dots init(NULL);
global TextList *Cookie_reject_domains;
global TextList *Cookie_accept_domains;
global TextList *Cookie_avoid_wrong_number_of_dots_domains;
#endif				/* USE_COOKIE */

#ifdef USE_IMAGE
global int view_unseenobject init(FALSE);
#else
global int view_unseenobject init(TRUE);
#endif

#if defined(USE_SSL) && defined(USE_SSL_VERIFY)
global int ssl_verify_server init(FALSE);
global char *ssl_cert_file init(NULL);
global char *ssl_key_file init(NULL);
global char *ssl_ca_path init(NULL);
global char *ssl_ca_file init(NULL);
global int ssl_path_modified init(FALSE);
#endif				/* defined(USE_SSL) &&
				 * defined(USE_SSL_VERIFY) */
#ifdef USE_SSL
global char *ssl_forbid_method init(NULL);
#endif

global int is_redisplay init(FALSE);
global int clear_buffer init(TRUE);
global double pixel_per_char init(DEFAULT_PIXEL_PER_CHAR);
global int set_pixel_per_char init(FALSE);
#ifdef USE_IMAGE
global double pixel_per_line init(DEFAULT_PIXEL_PER_LINE);
global int set_pixel_per_line init(FALSE);
global double image_scale init(100);
#endif
global int use_lessopen init(FALSE);

global char *keymap_file init(KEYMAP_FILE);

#ifdef USE_M17N
#define get_mctype(c) ((Lineprop)wtf_type((wc_uchar *)(c)) << 8)
#define get_mclen(c) wtf_len1((wc_uchar *)(c))
#define get_mcwidth(c) wtf_width((wc_uchar *)(c))
#define get_strwidth(c) wtf_strwidth((wc_uchar *)(c))
#define get_Str_strwidth(c) wtf_strwidth((wc_uchar *)((c)->ptr))
#else
#define get_mctype(c) (IS_CNTRL(*(c)) ? PC_CTRL : PC_ASCII)
#define get_mclen(c) 1
#define get_mcwidth(c) 1
#define get_strwidth(c) strlen(c)
#define get_Str_strwidth(c) ((c)->length)
#endif

global int FollowRedirection init(10);

global int w3m_backend init(FALSE);
global TextLineList *backend_halfdump_buf;
global TextList *backend_batch_commands init(NULL);
int backend(void);
extern void deleteFiles(void);
void w3m_exit(int i);

#ifdef USE_ALARM
#define AL_UNSET         0
#define AL_EXPLICIT      1
#define AL_IMPLICIT      2
#define AL_IMPLICIT_ONCE 3

typedef struct _AlarmEvent {
    int sec;
    short status;
    int cmd;
    void *data;
} AlarmEvent;
#endif

/* 
 * Externals
 */

#include "table.h"
#include "proto.h"

#endif				/* not FM_H */
