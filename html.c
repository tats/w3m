#include "html.h"

/* Define HTML Tag Infomation Table */

#define ATTR_CORE	ATTR_ID
#define MAXA_CORE	1
unsigned char ALST_A[] =
{ATTR_NAME,ATTR_HREF,ATTR_TARGET,ATTR_HSEQ,ATTR_REFERER,ATTR_FRAMENAME,ATTR_CORE};
#define MAXA_A		MAXA_CORE + 6
unsigned char ALST_P[] = {ATTR_ALIGN,ATTR_CORE};
#define MAXA_P		MAXA_CORE + 1
unsigned char ALST_UL[] = {ATTR_START,ATTR_TYPE,ATTR_CORE};
#define MAXA_UL		MAXA_CORE + 2
unsigned char ALST_LI[] = {ATTR_TYPE,ATTR_VALUE,ATTR_CORE};
#define MAXA_LI		MAXA_CORE + 2
unsigned char ALST_HR[] = {ATTR_WIDTH,ATTR_ALIGN,ATTR_CORE};
#define MAXA_HR		MAXA_CORE + 2
unsigned char ALST_DL[] = {ATTR_COMPACT,ATTR_CORE};
#define MAXA_DL		MAXA_CORE + 1
unsigned char ALST_PRE[] = {ATTR_FOR_TABLE,ATTR_CORE};
#define MAXA_PRE	MAXA_CORE + 1
unsigned char ALST_IMG[] =
{ATTR_SRC,ATTR_ALT,ATTR_WIDTH,ATTR_HEIGHT,ATTR_USEMAP,ATTR_CORE};
#define MAXA_IMG	MAXA_CORE + 5
unsigned char ALST_TABLE[] =
{ATTR_BORDER,ATTR_WIDTH,ATTR_HBORDER,ATTR_CELLSPACING,ATTR_CELLPADDING,ATTR_VSPACE,ATTR_CORE};
#define MAXA_TABLE	MAXA_CORE + 6
unsigned char ALST_META[] = {ATTR_HTTP_EQUIV,ATTR_CONTENT,ATTR_CORE};
#define MAXA_META	MAXA_CORE + 2
unsigned char ALST_FRAME[] = {ATTR_SRC,ATTR_NAME,ATTR_CORE};
#define MAXA_FRAME	MAXA_CORE + 2
unsigned char ALST_FRAMESET[] = {ATTR_COLS,ATTR_ROWS,ATTR_CORE};
#define MAXA_FRAMESET	MAXA_CORE + 2
unsigned char ALST_FORM[] =
{ATTR_METHOD,ATTR_ACTION,ATTR_CHARSET,ATTR_ACCEPT_CHARSET,ATTR_ENCTYPE,ATTR_TARGET,ATTR_CORE};
#define MAXA_FORM	MAXA_CORE + 6
unsigned char ALST_INPUT[] =
{ATTR_TYPE,ATTR_VALUE,ATTR_NAME,ATTR_CHECKED,ATTR_ACCEPT,ATTR_SIZE,ATTR_MAXLENGTH,ATTR_ALT,ATTR_CORE};
#define MAXA_INPUT	MAXA_CORE + 8
unsigned char ALST_TEXTAREA[] = {ATTR_COLS,ATTR_ROWS,ATTR_NAME,ATTR_CORE};
#define MAXA_TEXTAREA	MAXA_CORE + 3
unsigned char ALST_SELECT[] = {ATTR_NAME,ATTR_MULTIPLE,ATTR_CORE};
#define MAXA_SELECT	MAXA_CORE + 2
unsigned char ALST_OPTION[] = {ATTR_VALUE,ATTR_LABEL,ATTR_SELECTED,ATTR_CORE};
#define MAXA_OPTION	MAXA_CORE + 3
unsigned char ALST_ISINDEX[] = {ATTR_ACTION,ATTR_PROMPT,ATTR_CORE};
#define MAXA_ISINDEX	MAXA_CORE + 2
unsigned char ALST_MAP[] = {ATTR_NAME,ATTR_CORE};
#define MAXA_MAP	MAXA_CORE + 1
unsigned char ALST_AREA[] = {ATTR_HREF,ATTR_ALT,ATTR_CORE};
#define MAXA_AREA	MAXA_CORE + 2
unsigned char ALST_BASE[] = {ATTR_HREF,ATTR_TARGET,ATTR_CORE};
#define MAXA_BASE	MAXA_CORE + 2
unsigned char ALST_BODY[] = {ATTR_BACKGROUND,ATTR_CORE};
#define MAXA_BODY	MAXA_CORE + 1
unsigned char ALST_TR[] = {ATTR_ALIGN,ATTR_VALIGN,ATTR_CORE};
#define MAXA_TR		MAXA_CORE + 2
unsigned char ALST_TD[] =
{ATTR_COLSPAN,ATTR_ROWSPAN,ATTR_ALIGN,ATTR_VALIGN,ATTR_WIDTH,ATTR_NOWRAP,ATTR_CORE};
#define MAXA_TD		MAXA_CORE + 6
unsigned char ALST_BGSOUND[] = {ATTR_SRC,ATTR_CORE};
#define MAX_BGSOUND	MAXA_CORE + 1
unsigned char ALST_APPLET[] = {ATTR_ARCHIVE,ATTR_CORE};
#define MAX_APPLET	MAXA_CORE + 1
unsigned char ALST_EMBED[] = {ATTR_SRC,ATTR_CORE};
#define MAX_EMBED	MAXA_CORE + 1

unsigned char ALST_TABLE_ALT[] = {ATTR_TID};
#define MAXA_TABLE_ALT	1
unsigned char ALST_TITLE_ALT[] = {ATTR_TITLE};
#define MAXA_TITLE_ALT	1
unsigned char ALST_INPUT_ALT[] =
{ATTR_HSEQ,ATTR_FID,ATTR_NO_EFFECT,ATTR_TYPE,ATTR_NAME,ATTR_VALUE,ATTR_CHECKED,ATTR_ACCEPT,ATTR_SIZE,ATTR_MAXLENGTH,ATTR_TEXTAREANUMBER,ATTR_SELECTNUMBER,ATTR_ROWS};
#define MAXA_INPUT_ALT	13
unsigned char ALST_IMG_ALT[] = {ATTR_SRC};
#define MAXA_IMG_ALT	1
unsigned char ALST_NOP[] = {ATTR_CORE};
#define MAXA_NOP	MAXA_CORE

TagInfo TagMAP[MAX_HTMLTAG] =
{
    {NULL,          0,             0},           /*   0 HTML_UNKNOWN    */
    {ALST_A,        MAXA_A,        0},           /*   1 HTML_A          */
    {NULL,          0,             TFLG_END},    /*   2 HTML_N_A        */
    {ALST_P,        MAXA_P,        0},           /*   3 HTML_H          */
    {NULL,          0,             TFLG_END},    /*   4 HTML_N_H        */
    {ALST_P,        MAXA_P,        0},           /*   5 HTML_P          */
    {NULL,          0,             0},           /*   6 HTML_BR         */
    {NULL,          0,             0},           /*   7 HTML_B          */
    {NULL,          0,             TFLG_END},    /*   8 HTML_N_B        */
    {ALST_UL,       MAXA_UL,       0},           /*   9 HTML_UL         */
    {NULL,          0,             TFLG_END},    /*  10 HTML_N_UL       */
    {ALST_LI,       MAXA_LI,       0},           /*  11 HTML_LI         */
    {ALST_UL,       MAXA_UL,       0},           /*  12 HTML_OL         */
    {NULL,          0,             TFLG_END},    /*  13 HTML_N_OL       */
    {NULL,          0,             0},           /*  14 HTML_TITLE      */
    {NULL,          0,             TFLG_END},    /*  15 HTML_N_TITLE    */
    {ALST_HR,       MAXA_HR,       0},           /*  16 HTML_HR         */
    {ALST_DL,       MAXA_DL,       0},           /*  17 HTML_DL         */
    {NULL,          0,             TFLG_END},    /*  18 HTML_N_DL       */
    {NULL,          0,             0},           /*  19 HTML_DT         */
    {NULL,          0,             0},           /*  20 HTML_DD         */
    {ALST_PRE,      MAXA_PRE,      0},           /*  21 HTML_PRE        */
    {NULL,          0,             TFLG_END},    /*  22 HTML_N_PRE      */
    {NULL,          0,             0},           /*  23 HTML_BLQ        */
    {NULL,          0,             TFLG_END},    /*  24 HTML_N_BLQ      */
    {ALST_IMG,      MAXA_IMG,      0},           /*  25 HTML_IMG        */
    {NULL,          0,             0},           /*  26 HTML_LISTING    */
    {NULL,          0,             TFLG_END},    /*  27 HTML_N_LISTING  */
    {NULL,          0,             0},           /*  28 HTML_XMP        */
    {NULL,          0,             TFLG_END},    /*  29 HTML_N_XMP      */
    {NULL,          0,             0},           /*  30 HTML_PLAINTEXT  */
    {ALST_TABLE,    MAXA_TABLE,    0},           /*  31 HTML_TABLE      */
    {NULL,          0,             TFLG_END},    /*  32 HTML_N_TABLE    */
    {ALST_META,     MAXA_META,     0},           /*  33 HTML_META       */
    {NULL,          0,             TFLG_END},    /*  34 HTML_N_P        */
    {ALST_FRAME,    MAXA_FRAME,    0},           /*  35 HTML_FRAME      */
    {ALST_FRAMESET, MAXA_FRAMESET, 0},           /*  36 HTML_FRAMESET   */
    {NULL,          0,             TFLG_END},    /*  37 HTML_N_FRAMESET */
    {NULL,          0,             0},           /*  38 HTML_CENTER     */
    {NULL,          0,             TFLG_END},    /*  39 HTML_N_CENTER   */
    {NULL,          0,             0},           /*  40 HTML_FONT       */
    {NULL,          0,             TFLG_END},    /*  41 HTML_N_FONT     */
    {ALST_FORM,     MAXA_FORM,     0},           /*  42 HTML_FORM       */
    {NULL,          0,             TFLG_END},    /*  43 HTML_N_FORM     */
    {ALST_INPUT,    MAXA_INPUT,    0},           /*  44 HTML_INPUT      */
    {ALST_TEXTAREA, MAXA_TEXTAREA, 0},           /*  45 HTML_TEXTAREA   */
    {NULL,          0,             TFLG_END},    /*  46 HTML_N_TEXTAREA */
    {ALST_SELECT,   MAXA_SELECT,   0},           /*  47 HTML_SELECT     */
    {NULL,          0,             TFLG_END},    /*  48 HTML_N_SELECT   */
    {ALST_OPTION,   MAXA_OPTION,   0},           /*  49 HTML_OPTION     */
    {NULL,          0,             0},           /*  50 HTML_NOBR       */
    {NULL,          0,             TFLG_END},    /*  51 HTML_N_NOBR     */
    {ALST_P,        MAXA_P,        0},           /*  52 HTML_DIV        */
    {NULL,          0,             TFLG_END},    /*  53 HTML_N_DIV      */
    {ALST_ISINDEX,  MAXA_ISINDEX,  0},           /*  54 HTML_ISINDEX    */
    {ALST_MAP,      MAXA_MAP,      0},           /*  55 HTML_MAP        */
    {NULL,          0,             TFLG_END},    /*  56 HTML_N_MAP      */
    {ALST_AREA,     MAXA_AREA,     0},           /*  57 HTML_AREA       */
    {NULL,          0,             0},           /*  58 HTML_SCRIPT     */
    {NULL,          0,             TFLG_END},    /*  59 HTML_N_SCRIPT   */
    {ALST_BASE,     MAXA_BASE,     0},           /*  60 HTML_BASE       */
    {NULL,          0,             0},           /*  61 HTML_DEL        */
    {NULL,          0,             TFLG_END},    /*  62 HTML_N_DEL      */
    {NULL,          0,             0},           /*  63 HTML_INS        */
    {NULL,          0,             TFLG_END},    /*  64 HTML_N_INS      */
    {NULL,          0,             0},           /*  65 HTML_U          */
    {NULL,          0,             TFLG_END},    /*  66 HTML_N_U        */
    {NULL,          0,             0},           /*  67 HTML_STYLE      */
    {NULL,          0,             TFLG_END},    /*  68 HTML_N_STYLE    */
    {NULL,          0,             0},           /*  69 HTML_WBR        */
    {NULL,          0,             0},           /*  70 HTML_EM         */
    {NULL,          0,             TFLG_END},    /*  71 HTML_N_EM       */
    {ALST_BODY,     MAXA_BODY,     0},           /*  72 HTML_BODY       */
    {NULL,          0,             TFLG_END},    /*  73 HTML_N_BODY     */
    {ALST_TR,       MAXA_TR,       0},           /*  74 HTML_TR         */
    {NULL,          0,             TFLG_END},    /*  75 HTML_N_TR       */
    {ALST_TD,       MAXA_TD,       0},           /*  76 HTML_TD         */
    {NULL,          0,             TFLG_END},    /*  77 HTML_N_TD       */
    {NULL,          0,             0},           /*  78 HTML_CAPTION    */
    {NULL,          0,             TFLG_END},    /*  79 HTML_N_CAPTION  */
    {ALST_TD,       MAXA_TD,       0},           /*  80 HTML_TH         */
    {NULL,          0,             TFLG_END},    /*  81 HTML_N_TH       */
    {NULL,          0,             0},           /*  82 HTML_THEAD      */
    {NULL,          0,             TFLG_END},    /*  83 HTML_N_THEAD    */
    {NULL,          0,             0},           /*  84 HTML_TBODY      */
    {NULL,          0,             TFLG_END},    /*  85 HTML_N_TBODY    */
    {NULL,          0,             0},           /*  86 HTML_TFOOT      */
    {NULL,          0,             TFLG_END},    /*  87 HTML_N_TFOOT    */
    {NULL,          0,             0},           /*  88 HTML_COLGROUP   */
    {NULL,          0,             TFLG_END},    /*  89 HTML_N_COLGROUP */
    {NULL,          0,             0},           /*  90 HTML_COL        */
    {ALST_BGSOUND,  MAX_BGSOUND,   0},           /*  91 HTML_BGSOUND    */
    {ALST_APPLET,   MAX_APPLET,    0},           /*  92 HTML_APPLET     */
    {ALST_EMBED,    MAX_EMBED,     0},           /*  93 HTML_EMBED      */
    {NULL,          0,             TFLG_END},    /*  94 HTML_N_OPTION   */
    {NULL,          0,             0},           /*  95 HTML_HEAD       */
    {NULL,          0,             TFLG_END},    /*  96 HTML_N_HEAD     */
    {NULL,          0,             0},           /*  97 HTML_DOCTYPE    */
                    				       		    
    {NULL,          0,             0},           /*  98 Undefined       */
    {NULL,          0,             0},           /*  99 Undefined       */
    {NULL,          0,             0},           /* 100 Undefined       */
    {NULL,          0,             0},           /* 101 Undefined       */
    {NULL,          0,             0},           /* 102 Undefined       */
    {NULL,          0,             0},           /* 103 Undefined       */
    {NULL,          0,             0},           /* 104 Undefined       */
    {NULL,          0,             0},           /* 105 Undefined       */
    {NULL,          0,             0},           /* 106 Undefined       */
    {NULL,          0,             0},           /* 107 Undefined       */
    {NULL,          0,             0},           /* 108 Undefined       */
    {NULL,          0,             0},           /* 109 Undefined       */
    {NULL,          0,             0},           /* 110 Undefined       */
    {NULL,          0,             0},           /* 111 Undefined       */
    {NULL,          0,             0},           /* 112 Undefined       */

    /* pseudo tag */
    {ALST_TABLE_ALT,MAXA_TABLE_ALT,TFLG_INT},    /* 113 HTML_TABLE_ALT   */
    {NULL,          0,             TFLG_INT},    /* 114 HTML_RULE        */
    {NULL,          0,    TFLG_INT|TFLG_END},    /* 115 HTML_N_RULE      */
    {NULL,          0,             TFLG_INT},    /* 116 HTML_PRE_INT     */
    {NULL,          0,    TFLG_INT|TFLG_END},    /* 117 HTML_N_PRE_INT   */
    {ALST_TITLE_ALT,MAXA_TITLE_ALT,TFLG_INT},    /* 118 HTML_TITLE_ALT   */
    {ALST_FORM,     MAXA_FORM,     TFLG_INT},    /* 119 HTML_FORM_INT    */
    {NULL,          0,    TFLG_INT|TFLG_END},    /* 120 HTML_N_FORM_INT  */
    {NULL,          0,             TFLG_INT},    /* 121 HTML_DL_COMPACT  */
    {ALST_INPUT_ALT,MAXA_INPUT_ALT,TFLG_INT},    /* 122 HTML_INPUT_ALT   */
    {NULL,          0,    TFLG_INT|TFLG_END},    /* 123 HTML_N_INPUT_ALT */
    {ALST_IMG_ALT,  MAXA_IMG_ALT,  TFLG_INT},    /* 124 HTML_IMG_ALT     */
    {NULL,          0,    TFLG_INT|TFLG_END},    /* 125 HTML_N_IMG_ALT   */
    {NULL,          0,             TFLG_INT},    /* 126 HTML_EOL         */
    {ALST_NOP,      MAXA_NOP,      TFLG_INT},    /* 127 HTML_NOP         */
};
    
TagAttrInfo AttrMAP[MAX_TAGATTR] =
{
    {NULL            ,	VTYPE_NONE,    0},        /*  0 ATTR_UNKNOWN        */
    {"accept"        ,	VTYPE_NONE,    0},        /*  1 ATTR_ACCEPT         */
    {"accept-charset",	VTYPE_STR,     0},        /*  2 ATTR_ACCEPT_CHARSET */
    {"action"        ,	VTYPE_ACTION,  0},        /*  3 ATTR_ACTION         */
    {"align"         ,	VTYPE_ALIGN,   0},        /*  4 ATTR_ALIGN          */
    {"alt"           ,	VTYPE_STR,     0},        /*  5 ATTR_ALT            */
    {"archive"       ,	VTYPE_STR,     0},        /*  6 ATTR_ARCHIVE        */
    {"background"    ,	VTYPE_STR,     0},        /*  7 ATTR_BACKGROUND     */
    {"border"        ,	VTYPE_NUMBER,  0},        /*  8 ATTR_BORDER         */
    {"cellpadding"   ,	VTYPE_NUMBER,  0},        /*  9 ATTR_CELLPADDING    */
    {"cellspacing"   ,	VTYPE_NUMBER,  0},        /* 10 ATTR_CELLSPACING    */
    {"charset"       ,	VTYPE_STR,     0},        /* 11 ATTR_CHARSET        */
    {"checked"       ,	VTYPE_NONE,    0},        /* 12 ATTR_CHECKED        */
    {"cols"          ,	VTYPE_MLENGTH, 0},        /* 13 ATTR_COLS           */
    {"colspan"       ,	VTYPE_NUMBER,  0},        /* 14 ATTR_COLSPAN        */
    {"content"       ,	VTYPE_STR,     0},        /* 15 ATTR_CONTENT        */
    {"enctype"       ,	VTYPE_ENCTYPE, 0},        /* 16 ATTR_ENCTYPE        */
    {"height"        ,	VTYPE_LENGTH,  0},        /* 17 ATTR_HEIGHT         */
    {"href"          ,	VTYPE_STR,     0},        /* 18 ATTR_HREF           */
    {"http-equiv"    ,	VTYPE_STR,     0},        /* 19 ATTR_HTTP_EQUIV     */
    {"id"            ,	VTYPE_STR,     0},        /* 20 ATTR_ID             */
    {"link"          ,	VTYPE_STR,     0},        /* 21 ATTR_LINK           */
    {"maxlength"     ,	VTYPE_NUMBER,  0},        /* 22 ATTR_MAXLENGTH      */
    {"method"        ,	VTYPE_METHOD,  0},        /* 23 ATTR_METHOD         */
    {"multiple"      ,	VTYPE_NONE,    0},        /* 24 ATTR_MULTIPLE       */
    {"name"          ,	VTYPE_STR,     0},        /* 25 ATTR_NAME           */
    {"nowrap"        ,	VTYPE_NONE,    0},        /* 26 ATTR_NOWRAP         */
    {"prompt"        ,	VTYPE_STR,     0},        /* 27 ATTR_PROMPT         */
    {"rows"          ,	VTYPE_MLENGTH, 0},        /* 28 ATTR_ROWS           */
    {"rowspan"       ,	VTYPE_NUMBER,  0},        /* 29 ATTR_ROWSPAN        */
    {"size"          ,	VTYPE_NUMBER,  0},        /* 30 ATTR_SIZE           */
    {"src"           ,	VTYPE_STR,     0},        /* 31 ATTR_SRC            */
    {"target"        ,	VTYPE_STR,     0},        /* 32 ATTR_TARGET         */
    {"type"          ,	VTYPE_TYPE,    0},        /* 33 ATTR_TYPE           */
    {"usemap"        ,	VTYPE_STR,     0},        /* 34 ATTR_USEMAP         */
    {"valign"        ,	VTYPE_VALIGN,  0},        /* 35 ATTR_VALIGN         */
    {"value"         ,	VTYPE_STR,     0},        /* 36 ATTR_VALUE          */
    {"vspace"        ,	VTYPE_NUMBER,  0},        /* 37 ATTR_VSPACE         */
    {"width"         ,	VTYPE_LENGTH,  0},        /* 38 ATTR_WIDTH          */
    {"compact"       ,	VTYPE_NONE,    0},        /* 39 ATTR_COMPACT        */
    {"start"         ,	VTYPE_NUMBER,  0},        /* 40 ATTR_START          */
    {"selected"      ,	VTYPE_NONE,    0},        /* 41 ATTR_SELECTED       */
    {"label"         ,	VTYPE_STR,     0},        /* 42 ATTR_LABEL          */
                                       		   
    {NULL            ,	VTYPE_NONE,    0},        /* 43 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 44 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 45 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 46 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 47 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 48 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 49 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 50 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 51 Undefined           */
    {NULL            ,	VTYPE_NONE,    0},        /* 52 Undefined           */
                                                           		   
    /* Internal attribute */                               		   
    {"tid"           ,	VTYPE_NUMBER,  AFLG_INT}, /* 53 ATTR_TID            */
    {"fid"           ,	VTYPE_NUMBER,  AFLG_INT}, /* 54 ATTR_FID            */
    {"for_table"     ,	VTYPE_NONE,    AFLG_INT}, /* 55 ATTR_FOR_TABLE      */
    {"framename"     ,	VTYPE_STR,     AFLG_INT}, /* 56 ATTR_FRAMENAME      */
    {"hborder"       ,	VTYPE_NONE,    0},        /* 57 ATTR_HBORDER        */
    {"hseq"          ,	VTYPE_NUMBER,  AFLG_INT}, /* 58 ATTR_HSEQ           */
    {"no_effect"     ,	VTYPE_NONE,    AFLG_INT}, /* 59 ATTR_NO_EFFECT      */
    {"referer"       ,	VTYPE_STR,     AFLG_INT}, /* 60 ATTR_REFERER        */
    {"selectnumber"  ,	VTYPE_NUMBER,  AFLG_INT}, /* 61 ATTR_SELECTNUMBER   */
    {"textareanumber",	VTYPE_NUMBER,  AFLG_INT}, /* 62 ATTR_TEXTAREANUMBER */
    {"title"         ,	VTYPE_STR,     AFLG_INT}, /* 63 ATTR_TITLE          */
};
