/* $Id: html.h,v 1.8 2002/09/28 16:30:07 ukai Exp $ */
#ifndef _HTML_H
#define _HTML_H
#ifdef USE_SSL
#include <bio.h>
#include <x509.h>
#include <ssl.h>
#endif				/* USE_SSL */

#include "istream.h"

#define StrUFgets(f) StrISgets((f)->stream)
#define StrmyUFgets(f) StrmyISgets((f)->stream)
#define UFgetc(f) ISgetc((f)->stream)
#define UFundogetc(f) ISundogetc((f)->stream)
#define UFread(f,buf,len) ISread((f)->stream,buf,len)
#define UFclose(f) (ISclose((f)->stream), (f)->stream = NULL)

struct cmdtable {
    char *cmdname;
    int cmd;
};

struct mailcap {
    char *type;
    char *viewer;
    int flags;
    char *test;
    char *nametemplate;
    char *edit;
};

#define MAILCAP_NEEDSTERMINAL	0x01
#define MAILCAP_COPIOUSOUTPUT	0x02
#define MAILCAP_HTMLOUTPUT      0x04

#define MCSTAT_REPNAME          0x01
#define MCSTAT_REPTYPE          0x02
#define MCSTAT_REPPARAM         0x04

struct table2 {
    char *item1;
    char *item2;
};

typedef struct {
    char *referer;
    int flag;
} URLOption;

typedef struct _ParsedURL {
    int scheme;
    char *user;
    char *pass;
    char *host;
    int port;
    char *file;
    char *real_file;
    char *query;
    char *label;
    int is_nocache;
} ParsedURL;

typedef struct {
    unsigned char scheme;
    char is_cgi;
    char encoding;
    InputStream stream;
    char *ext;
    int compression;
    char *guess_type;
#ifdef USE_SSL
    char *ssl_certificate;
#endif
} URLFile;

#define CMP_NOCOMPRESS   0
#define CMP_COMPRESS     1
#define CMP_GZIP         2
#define CMP_BZIP2        3
#define CMP_DEFLATE      4

#define ENC_7BIT	0
#define ENC_BASE64	1
#define ENC_QUOTE	2
#define ENC_UUENCODE	3

#define HTML_UNKNOWN	0
#define HTML_A		1
#define HTML_N_A	2
#define HTML_H		3
#define HTML_N_H	4
#define HTML_P		5
#define HTML_BR		6
#define HTML_B		7
#define HTML_N_B	8
#define HTML_UL		9
#define HTML_N_UL	10
#define HTML_LI		11
#define HTML_OL		12
#define HTML_N_OL	13
#define HTML_TITLE	14
#define HTML_N_TITLE	15
#define HTML_HR		16
#define HTML_DL		17
#define HTML_N_DL	18
#define HTML_DT		19
#define HTML_DD		20
#define HTML_PRE	21
#define HTML_N_PRE	22
#define HTML_BLQ	23
#define HTML_N_BLQ	24
#define HTML_IMG	25
#define HTML_LISTING	26
#define HTML_N_LISTING	27
#define HTML_XMP	28
#define HTML_N_XMP	29
#define HTML_PLAINTEXT	30
#define HTML_TABLE      31
#define HTML_N_TABLE    32
#define HTML_META       33
#define HTML_N_P        34
#define HTML_FRAME      35
#define HTML_FRAMESET   36
#define HTML_N_FRAMESET 37
#define HTML_CENTER     38
#define HTML_N_CENTER   39
#define HTML_FONT       40
#define HTML_N_FONT     41
#define HTML_FORM       42
#define HTML_N_FORM     43
#define HTML_INPUT      44
#define HTML_TEXTAREA   45
#define HTML_N_TEXTAREA 46
#define HTML_SELECT     47
#define HTML_N_SELECT   48
#define HTML_OPTION     49
#define HTML_NOBR       50
#define HTML_N_NOBR     51
#define HTML_DIV        52
#define HTML_N_DIV      53
#define HTML_ISINDEX    54
#define HTML_MAP        55
#define HTML_N_MAP      56
#define HTML_AREA       57
#define HTML_SCRIPT     58
#define HTML_N_SCRIPT   59
#define HTML_BASE       60
#define HTML_DEL        61
#define HTML_N_DEL      62
#define HTML_INS        63
#define HTML_N_INS      64
#define HTML_U          65
#define HTML_N_U        66
#define HTML_STYLE      67
#define HTML_N_STYLE    68
#define HTML_WBR        69
#define HTML_EM		70
#define HTML_N_EM	71
#define HTML_BODY	72
#define HTML_N_BODY	73
#define HTML_TR         74
#define HTML_N_TR       75
#define HTML_TD         76
#define HTML_N_TD       77
#define HTML_CAPTION    78
#define HTML_N_CAPTION  79
#define HTML_TH         80
#define HTML_N_TH       81
#define HTML_THEAD      82
#define HTML_N_THEAD    83
#define HTML_TBODY      84
#define HTML_N_TBODY    85
#define HTML_TFOOT      86
#define HTML_N_TFOOT    87
#define HTML_COLGROUP   88
#define HTML_N_COLGROUP 89
#define HTML_COL        90
#define HTML_BGSOUND    91
#define HTML_APPLET     92
#define HTML_EMBED      93
#define HTML_N_OPTION   94
#define HTML_HEAD       95
#define HTML_N_HEAD     96
#define HTML_DOCTYPE    97
#define HTML_NOFRAMES   98
#define HTML_N_NOFRAMES 99


   /* pseudo tag */
#define HTML_INTERNAL       106
#define HTML_N_INTERNAL     107
#define HTML_SELECT_INT     108
#define HTML_N_SELECT_INT   109
#define HTML_OPTION_INT     110
#define HTML_TEXTAREA_INT   111
#define HTML_N_TEXTAREA_INT 112
#define HTML_TABLE_ALT      113
#define HTML_RULE           114
#define HTML_N_RULE         115
#define HTML_PRE_INT        116
#define HTML_N_PRE_INT      117
#define HTML_TITLE_ALT      118
#define HTML_FORM_INT       119
#define HTML_N_FORM_INT     120
#define HTML_DL_COMPACT     121
#define HTML_INPUT_ALT      122
#define HTML_N_INPUT_ALT    123
#define HTML_IMG_ALT        124
#define HTML_N_IMG_ALT      125
#define HTML_EOL            126
#define HTML_NOP	    127

#define MAX_HTMLTAG	    128

/* Tag attribute */

#define ATTR_UNKNOWN		0
#define ATTR_ACCEPT		1
#define ATTR_ACCEPT_CHARSET	2
#define ATTR_ACTION		3
#define ATTR_ALIGN		4
#define ATTR_ALT		5
#define ATTR_ARCHIVE		6
#define ATTR_BACKGROUND		7
#define ATTR_BORDER		8
#define ATTR_CELLPADDING	9
#define ATTR_CELLSPACING	10
#define ATTR_CHARSET		11
#define ATTR_CHECKED		12
#define ATTR_COLS		13
#define ATTR_COLSPAN		14
#define ATTR_CONTENT		15
#define ATTR_ENCTYPE		16
#define ATTR_HEIGHT		17
#define ATTR_HREF		18
#define ATTR_HTTP_EQUIV		19
#define ATTR_ID			20
#define ATTR_LINK		21
#define ATTR_MAXLENGTH		22
#define ATTR_METHOD		23
#define ATTR_MULTIPLE		24
#define ATTR_NAME		25
#define ATTR_NOWRAP		26
#define ATTR_PROMPT		27
#define ATTR_ROWS		28
#define ATTR_ROWSPAN		29
#define ATTR_SIZE		30
#define ATTR_SRC		31
#define ATTR_TARGET		32
#define ATTR_TYPE		33
#define ATTR_USEMAP		34
#define ATTR_VALIGN		35
#define ATTR_VALUE		36
#define ATTR_VSPACE		37
#define ATTR_WIDTH		38
#define ATTR_COMPACT		39
#define ATTR_START		40
#define ATTR_SELECTED		41
#define ATTR_LABEL		42
#define ATTR_READONLY		43
#define ATTR_SHAPE		44
#define ATTR_COORDS		45
#define ATTR_ISMAP		46

/* Internal attribute */
#define ATTR_XOFFSET		49
#define ATTR_YOFFSET		50
#define ATTR_TOP_MARGIN		51
#define ATTR_BOTTOM_MARGIN	52
#define ATTR_TID		53
#define ATTR_FID		54
#define ATTR_FOR_TABLE		55
#define ATTR_FRAMENAME		56
#define ATTR_HBORDER		57
#define ATTR_HSEQ		58
#define ATTR_NO_EFFECT		59
#define ATTR_REFERER		60
#define ATTR_SELECTNUMBER	61
#define ATTR_TEXTAREANUMBER	62
#define ATTR_TITLE		63

#define MAX_TAGATTR		64

/* HTML Tag Information Table */

typedef struct html_tag_info {
    char *name;
    unsigned char *accept_attribute;
    unsigned char max_attribute;
    unsigned char flag;
} TagInfo;

#define TFLG_END	1
#define TFLG_INT	2

/* HTML Tag Attribute Information Table */

typedef struct tag_attribute_info {
    char *name;
    unsigned char vtype;
    unsigned char flag;
} TagAttrInfo;

#define AFLG_INT	1

#define VTYPE_NONE	0
#define VTYPE_STR	1
#define VTYPE_NUMBER	2
#define VTYPE_LENGTH	3
#define VTYPE_ALIGN     4
#define VTYPE_VALIGN    5
#define VTYPE_ACTION    6
#define VTYPE_ENCTYPE   7
#define VTYPE_METHOD    8
#define VTYPE_MLENGTH   9
#define VTYPE_TYPE      10

#define SHAPE_UNKNOWN	0
#define SHAPE_DEFAULT	1
#define SHAPE_RECT	2
#define SHAPE_CIRCLE	3
#define SHAPE_POLY	4

extern TagInfo TagMAP[];
extern TagAttrInfo AttrMAP[];

struct environment {
    char env;
    int type;
    int count;
    char indent;
};

#define MAX_ENV_LEVEL    20
#define MAX_INDENT_LEVEL 10

#define INDENT_INCR 4

#define SCM_UNKNOWN	255
#define SCM_MISSING	254
#define SCM_HTTP	0
#define SCM_GOPHER	1
#define SCM_FTP		2
#define SCM_FTPDIR	3
#define SCM_LOCAL	4
#define SCM_LOCAL_CGI	5
#define SCM_EXEC	6
#define SCM_NNTP	7
#define SCM_NEWS	8
#define SCM_MAILTO      9
#ifdef USE_SSL
#define SCM_HTTPS       10
#endif				/* USE_SSL */

#endif				/* _HTML_H */
