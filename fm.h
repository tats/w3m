/* $Id: fm.h,v 1.75 2002/11/11 15:33:35 ukai Exp $ */
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
#endif

#include "ctrlcode.h"
#include "html.h"
#include "gc.h"
#include "Str.h"
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

#if LANG == JA
#define JP_CHARSET
#endif				/* LANG == JA */

/* 
 * Constants.
 */
#define LINELEN	4096		/* Maximum line length */
#define PAGER_MAX_LINE	10000	/* Maximum line kept as pager */
#define FNLEN 80

#ifdef USE_IMAGE
#define MAX_IMAGE 1000

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
/* Character type */
#define PC_ASCII	0x0000
#define PC_CTRL		0x2000

#ifdef JP_CHARSET
#define PC_KANJI1	0x4000
#define PC_KANJI2	0x8000
#define PC_KANJI	(PC_KANJI1|PC_KANJI2)
#define P_CHARTYPE	(PC_ASCII|PC_CTRL|PC_KANJI)
#else				/* ISO-8859-1 charset (not JP_CHARSET) */
#define P_CHARTYPE	(PC_ASCII|PC_CTRL)
#endif				/* not JP_CHARSET */
#if 0
#define GET_PCTYPE(c)   ((GET_MYCTYPE(c)&MYCTYPE_CNTRL)<<13)
#else
#define GET_PCTYPE(c)   ((GET_MYCTYPE(c)&MYCTYPE_CNTRL)?PC_CTRL:PC_ASCII)
#endif

#ifndef KANJI_SYMBOLS
#define PC_RULE         0x1000
#endif				/* not KANJI_SYMBOLS */

/* Effect ( standout/underline ) */
#define P_EFFECT	0x01ff
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
#define PE_VISITED	0x0100

#define CharType(c)	((c)&P_CHARTYPE)
#ifdef KANJI_SYMBOLS
#define CharEffect(c)	((c)&P_EFFECT)
#else				/* not KANJI_SYMBOLS */
#define CharEffect(c)	((c)&(P_EFFECT|PC_RULE))
#endif				/* not KANJI_SYMBOLS */
#define SetCharType(v,c)	((v)=(((v)&~P_CHARTYPE)|(c)))


#define COLPOS(l,c)	calcPosition(l->lineBuf,l->propBuf,l->len,c,0,CP_AUTO)
#define IS_UNPRINTABLE_CONTROL(c,m) (CharType(m)==PC_CTRL&&(c)!=CTRL_I&&(c)!=CTRL_J)
#ifdef JP_CHARSET
#define IS_UNPRINTABLE_ASCII(c,m) (!IS_ASCII(c)&&CharType(m)==PC_ASCII)
#else
#define IS_UNPRINTABLE_ASCII(c,m) (!IS_LATIN1(c))
#endif

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
#define BP_SOURCE	0x4
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
#ifdef MENU_MAP
#ifdef USE_IMAGE
    char shape;
    short *coords;
    int ncoords;
    short center_x;
    short center_y;
#endif
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
    short len;
    short width;
    long linenumber;		/* on buffer */
    long real_linenumber;	/* on file */
    unsigned short usrflags;
} Line;

typedef struct {
    int line;
    short pos;
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
    BufferPoint start;
    BufferPoint end;
    int hseq;
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
    short currentColumn;
    short cursorX;
    short cursorY;
    short pos;
    short visualpos;
    short rootX;
    short rootY;
    short COLS;
    short LINES;
    InputStream pagerSource;
    AnchorList *href;
    AnchorList *name;
    AnchorList *img;
    AnchorList *formitem;
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
    size_t linelen;
    size_t trbyte;
    char check_url;
#ifdef JP_CHARSET
    char document_code;
#endif				/* JP_CHARSET */
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
    char need_reshape;
    Anchor *submit;
} Buffer;

typedef struct _TabBuffer {
    struct _TabBuffer *nextTab;
    struct _TabBuffer *prevTab;
    Buffer *currentBuffer;
    Buffer *firstBuffer;
} TabBuffer;

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

#define FONTSTAT_SIZE 4

#define INIT_BUFFER_WIDTH (COLS - 1)

typedef struct {
    int pos;
    int len;
    int tlen;
    long flag;
    Str anchor;
    Str anchor_target;
    short anchor_hseq;
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
    int prevchar;
    long flag;
    long flag_stack[RB_STACK_SIZE];
    int flag_sp;
    int status;
    Str ignore_tag;
    short table_level;
    short nobr_level;
    Str anchor;
    Str anchor_target;
    short anchor_hseq;
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
#define in_stand fontstat[2]

#define RB_PRE		0x01
#define RB_XMPMODE	0x02
#define RB_LSTMODE	0x04
#define RB_PLAIN	0x08
#define RB_LEFT		0x80000
#define RB_CENTER	0x10
#define RB_RIGHT	0x20
#define RB_ALIGN	(RB_LEFT| RB_CENTER | RB_RIGHT)
#define RB_NOBR		0x40
#define RB_P		0x80
#define RB_PRE_INT	0x100
#define RB_PREMODE	(RB_PRE | RB_PRE_INT)
#define RB_SPECIAL	(RB_PRE|RB_XMPMODE|RB_LSTMODE|RB_PLAIN|RB_NOBR|RB_PRE_INT)
#define RB_PLAINMODE	(RB_XMPMODE|RB_LSTMODE|RB_PLAIN)

#define RB_IN_DT	0x200
#define RB_INTXTA	0x400
#define RB_INSELECT	0x800
#define RB_IGNORE	0x1000
#define RB_INSEL	0x2000
#define RB_IGNORE_P	0x4000
#define RB_TITLE	0x8000
#define RB_NFLUSHED	0x10000
#define RB_NOFRAMES	0x20000
#define RB_INTABLE	0x40000

#ifdef FORMAT_NICE
#define RB_FILL		0x200000
#endif				/* FORMAT_NICE */

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
#define R_ST_CMNT1  7		/* <!  */
#define R_ST_CMNT2  8		/* <!- */
#define R_ST_CMNT   9		/* within comment */
#define R_ST_NCMNT1 10		/* comment - */
#define R_ST_NCMNT2 11		/* comment -- */
#define R_ST_NCMNT3 12		/* comment -- space */
#define R_ST_IRRTAG 13		/* within irregular tag */

#define ST_IS_REAL_TAG(s)   ((s)==R_ST_TAG||(s)==R_ST_TAG0||(s)==R_ST_EQL)
#define ST_IS_COMMENT(s)    ((s)>=R_ST_CMNT1)
#define ST_IS_TAG(s)        ((s)!=R_ST_NORMAL&&(s)!=R_ST_AMP&&!ST_IS_COMMENT(s))

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

struct auth_cookie {
    Str host;
    int port;
    Str file;
    Str realm;
    Str cookie;
    struct auth_cookie *next;
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
#define MAX_TMPF_TYPE	4

#define set_no_proxy(domains) (NO_proxy_domains=make_domain_list(domains))

/* 
 * Globals.
 */

extern int LINES, COLS;
#if defined(__CYGWIN__) && LANG == JA
extern int LASTLINE;
#else				/* not defined(__CYGWIN__) || LANG != JA */
#define LASTLINE (LINES-1)
#endif				/* not defined(__CYGWIN__) || LANG != JA */

global int Tabstop init(8);
global int ShowEffect init(TRUE);
global int PagerMax init(PAGER_MAX_LINE);
#ifdef JP_CHARSET
global char InnerCode init(CODE_INNER_EUC);	/* use EUC-JP internally; do not change */
#endif				/* JP_CHARSET */

global char SearchHeader init(FALSE);
global char *DefaultType init(NULL);
global char RenderFrame init(FALSE);
global char TargetSelf init(FALSE);
global char PermitSaveToPipe init(FALSE);
global char DecodeCTE init(FALSE);
global char ArgvIsURL init(FALSE);
global char MetaRefresh init(FALSE);

global char fmInitialized init(FALSE);
global char QuietMessage init(FALSE);

extern unsigned char GlobalKeymap[];
extern unsigned char EscKeymap[];
extern unsigned char EscBKeymap[];
extern unsigned char EscDKeymap[];
#ifdef __EMX__
extern unsigned char PcKeymap[];
#endif
extern FuncList w3mFuncList[];

global char *HTTP_proxy init(NULL);
#ifdef USE_GOPHER
global char *GOPHER_proxy init(NULL);
#endif				/* USE_GOPHER */
global char *FTP_proxy init(NULL);
global ParsedURL HTTP_proxy_parsed;
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
global int DNS_order init(DNS_ORDER_UNSPEC);
extern int ai_family_order_table[3][3];	/* XXX */
#endif				/* INET6 */
global TextList *NO_proxy_domains;
global char NoCache init(FALSE);
global int Do_not_use_proxy init(FALSE);
global int Do_not_use_ti_te init(FALSE);

global char *document_root init(NULL);
global char *personal_document_root init(NULL);
global char *cgi_bin init(NULL);
global char *index_file init(NULL);

global char *CurrentDir;
/*
global Buffer *Currentbuf;
global Buffer *Firstbuf;
*/
global TabBuffer *CurrentTab;
global TabBuffer *FirstTab;
global TabBuffer *LastTab;
global int open_tab_blank init(FALSE);
global int close_tab_back init(FALSE);
global int nTab;
global int TabCols init(10);
#define N_TAB ((COLS - 2 > TabCols * nTab) ? nTab	\
	: (nTab - 1) / ((nTab * TabCols - 1) / (COLS - 2) + 1) + 1)
#define Currentbuf (CurrentTab->currentBuffer)
#define Firstbuf (CurrentTab->firstBuffer)
global int CurrentKey;
global char *CurrentKeyData;
global char *CurrentCmdData;
extern char *ullevel[];

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
global int use_mark init(TRUE);
#endif
#ifdef EMACS_LIKE_LINEEDIT
global int emacs_like_lineedit init(TRUE);
#endif
#ifdef VI_PREC_NUM
global int vi_prec_num init(TRUE);
#endif
#ifdef LABEL_TOPLINE
global int label_topline init(FALSE);
#endif
#ifdef NEXTPAGE_TOPLINE
global int nextpage_topline init(FALSE);
#endif
global char *displayTitleTerm init(NULL);
global int displayLink init(FALSE);
global int displayLineInfo init(FALSE);
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
#else
global int displayImage init(FALSE); /* XXX: emacs-w3m use display_image=off */
#endif
global char *Editor init(DEF_EDITOR);
#ifdef USE_W3MMAILER
global char *Mailer init(NULL);
#else
global char *Mailer init(DEF_MAILER);
#endif
global char *ExtBrowser init(DEF_EXT_BROWSER);
global char *ExtBrowser2 init(NULL);
global char *ExtBrowser3 init(NULL);
global int BackgroundExtViewer init(TRUE);
global int disable_secret_security_check init(FALSE);
global char *passwd_file init(PASSWD_FILE);
global char *pre_form_file init(PRE_FORM_FILE);
global char *ftppasswd init(NULL);
#ifdef FTPPASS_HOSTNAMEGEN
global int ftppass_hostnamegen init(TRUE);
#endif
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
global char *pauth init(NULL);
global Str proxy_auth_cookie init(NULL);
global int UseExternalDirBuffer init(TRUE);
global char *DirBufferCommand init("file:///$LIB/dirlist" CGI_EXTENSION);
#ifdef USE_DICT
global int UseDictCommand init(FALSE);
global char *DictCommand init("file:///$LIB/w3mdict" CGI_EXTENSION);
#endif				/* USE_DICT */
global int ignore_null_img_alt init(TRUE);
global int FoldTextarea init(FALSE);
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
global char *Local_cookie init(NULL);
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
global int URLHistSize init(100);
global int SaveURLHist init(TRUE);
#endif				/* USE_HISTORY */
global int multicolList init(FALSE);

global char DisplayCode init(DISPLAY_CODE);
#ifdef JP_CHARSET
global char SystemCode init(SYSTEM_CODE);
global char DocumentCode init(0);
global char UseContentCharset init(TRUE);
global char UseAutoDetect init(TRUE);
#define Str_conv_from_system(x) conv_str((x), SystemCode, InnerCode)
#define Str_conv_to_system(x) conv_str((x), InnerCode, SystemCode)
#define conv_from_system(x) conv((x), SystemCode, InnerCode)->ptr
#define conv_to_system(x) conv((x), InnerCode, SystemCode)->ptr
#define url_quote_conv(x,c) url_quote(conv((x), InnerCode, (c))->ptr)
#else
#define Str_conv_from_system(x) (x)
#define Str_conv_to_system(x) (x)
#define conv_from_system(x) (x)
#define conv_to_system(x) (x)
#define url_quote_conv(x,c) url_quote(x)
#endif				/* JP_CHARSET */
#ifndef KANJI_SYMBOLS
global int no_graphic_char init(FALSE);
extern char alt_rule[];
#endif				/* not KANJI_SYMBOLS */
extern char UseAltEntity;
global char *rc_dir;
global int rc_dir_is_tmp init(FALSE);

#ifdef USE_MOUSE
global int use_mouse init(TRUE);
extern int mouseActive;
global int reverse_mouse init(FALSE);
global int relative_wheel_scroll init(FALSE);
global int fixed_wheel_scroll_count init(5);
global int relative_wheel_scroll_ratio init(30);
#endif				/* USE_MOUSE */

#ifdef USE_COOKIE
global int default_use_cookie init(TRUE);
global int use_cookie init(FALSE);
global int accept_cookie init(FALSE);
#define ACCEPT_BAD_COOKIE_DISCARD	0
#define ACCEPT_BAD_COOKIE_ACCEPT	1
#define ACCEPT_BAD_COOKIE_ASK		2
global int accept_bad_cookie init(ACCEPT_BAD_COOKIE_DISCARD);
global char *cookie_reject_domains init(NULL);
global char *cookie_accept_domains init(NULL);
global TextList *Cookie_reject_domains;
global TextList *Cookie_accept_domains;
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

#ifdef JP_CHARSET
#define is_kanji(s)    (IS_KANJI1((s)[0])&&IS_KANJI2((s)[1]))
#define get_mctype(s)  (is_kanji(s)?PC_KANJI:GET_PCTYPE(*(s)))
#define get_mclen(m)   (((m)==PC_KANJI)?2:1)
#define mctowc(s,m) \
    (((m)==PC_KANJI)?((unsigned char)(s)[0]|((unsigned char)(s)[1]<<8)): \
                     (unsigned char)(s)[0])
#define is_wckanji(wc) ((wc)&~0xff)
#define get_wctype(wc) (is~wckanji(wc)?PC_KANJI:GET_PCTYPE(wc))
#else
#define get_mctype(s)  GET_PCTYPE(*(s))
#define get_mclen(m)   1
#define mctowc(s,m)    ((unsigned char)*(s))
#define is_wckanji(wc) ((wc)&~0xff)
#define get_wctype(wc) (is~wckanji(wc)?PC_ASCII:GET_PCTYPE(wc))
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
#define AL_IMPLICIT_DONE 4
#define AL_ONCE          8
#define AL_IMPLICIT_ONCE (AL_IMPLICIT|AL_ONCE)
#endif

/* 
 * Externals
 */

#include "table.h"
#include "proto.h"

#endif				/* not FM_H */
