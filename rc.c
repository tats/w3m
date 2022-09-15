/* $Id: rc.c,v 1.116 2010/08/20 09:47:09 htrb Exp $ */
/* 
 * Initialization file etc.
 */
#include "fm.h"
#include "myctype.h"
#include "proto.h"
#include <stdio.h>
#include <errno.h>
#include "parsetag.h"
#include "local.h"
#include "regex.h"
#include <stdlib.h>
#include <stddef.h>

struct param_ptr {
    char *name;
    int type;
    int inputtype;
    void *varptr;
    char *comment;
    void *select;
};

struct param_section {
    char *name;
    struct param_ptr *params;
};

struct rc_search_table {
    struct param_ptr *param;
    short uniq_pos;
};

static struct rc_search_table *RC_search_table;
static int RC_table_size;

#define P_INT      0
#define P_SHORT    1
#define P_CHARINT  2
#define P_CHAR     3
#define P_STRING   4
#if defined(USE_SSL) && defined(USE_SSL_VERIFY)
#define P_SSLPATH  5
#endif
#define P_COLOR    6
#define P_CODE     7
#define P_PIXELS   8
#define P_NZINT    9
#define P_SCALE    10

/* FIXME: gettextize here */
static wc_ces OptionCharset = WC_CES_US_ASCII;	/* FIXME: charset of source code */
static int OptionEncode = FALSE;

#define CMT_HELPER	 N_("External Viewer Setup")
#define CMT_TABSTOP      N_("Tab width in characters")
#define CMT_INDENT_INCR  N_("Indent for HTML rendering")
#define CMT_PIXEL_PER_CHAR N_("Number of pixels per character (4.0...32.0)")
#define CMT_PIXEL_PER_LINE N_("Number of pixels per line (4.0...64.0)")
#define CMT_PAGERLINE    N_("Number of remembered lines when used as a pager")
#define CMT_HISTORY	 N_("Use URL history")
#define CMT_HISTSIZE     N_("Number of remembered URL")
#define CMT_SAVEHIST     N_("Save URL history")
#define CMT_FRAME        N_("Render frames automatically")
#define CMT_ARGV_IS_URL  N_("Treat argument without scheme as URL")
#define CMT_TSELF        N_("Use _self as default target")
#define CMT_OPEN_TAB_BLANK N_("Open link on new tab if target is _blank or _new")
#define CMT_OPEN_TAB_DL_LIST N_("Open download list panel on new tab")
#define CMT_DISPLINK     N_("Display link URL automatically")
#define CMT_DISPLINKNUMBER N_("Display link numbers")
#define CMT_DECODE_URL   N_("Display decoded URL")
#define CMT_DISPLINEINFO N_("Display current line number")
#define CMT_DISP_IMAGE   N_("Display inline images")
#define CMT_PSEUDO_INLINES N_("Display pseudo-ALTs for inline images with no ALT or TITLE string")
#define CMT_AUTO_IMAGE   N_("Load inline images automatically")
#define CMT_MAX_LOAD_IMAGE N_("Maximum processes for parallel image loading")
#define CMT_EXT_IMAGE_VIEWER   N_("Use external image viewer")
#define CMT_IMAGE_SCALE  N_("Scale of image (%)")
#define CMT_IMGDISPLAY   N_("External command to display image")
#define CMT_IMAGE_MAP_LIST N_("Use link list of image map")
#define CMT_INLINE_IMG_PROTOCOL N_("Inline image display method")
#define CMT_MULTICOL     N_("Display file names in multi-column format")
#define CMT_ALT_ENTITY   N_("Use ASCII equivalents to display entities")
#define CMT_GRAPHIC_CHAR N_("Character type for border of table and menu")
#define CMT_DISP_BORDERS N_("Display table borders, ignore value of BORDER")
#define CMT_DISABLE_CENTER N_("Disable center alignment")
#define CMT_FOLD_TEXTAREA N_("Fold lines in TEXTAREA")
#define CMT_DISP_INS_DEL N_("Display INS, DEL, S and STRIKE element")
#define CMT_COLOR        N_("Display with color")
#define CMT_B_COLOR      N_("Color of normal character")
#define CMT_A_COLOR      N_("Color of anchor")
#define CMT_I_COLOR      N_("Color of image link")
#define CMT_F_COLOR      N_("Color of form")
#define CMT_ACTIVE_STYLE N_("Enable coloring of active link")
#define CMT_C_COLOR	 N_("Color of currently active link")
#define CMT_VISITED_ANCHOR N_("Use visited link color")
#define CMT_V_COLOR	 N_("Color of visited link")
#define CMT_BG_COLOR     N_("Color of background")
#define CMT_MARK_COLOR   N_("Color of mark")
#define CMT_USE_PROXY    N_("Use proxy")
#define CMT_HTTP_PROXY   N_("URL of HTTP proxy host")
#define CMT_HTTPS_PROXY  N_("URL of HTTPS proxy host")
#define CMT_FTP_PROXY    N_("URL of FTP proxy host")
#define CMT_NO_PROXY     N_("Domains to be accessed directly (no proxy)")
#define CMT_NOPROXY_NETADDR	N_("Check noproxy by network address")
#define CMT_NO_CACHE     N_("Disable cache")
#define CMT_DNS_ORDER	N_("Order of name resolution")
#define CMT_DROOT       N_("Directory corresponding to / (document root)")
#define CMT_PDROOT      N_("Directory corresponding to /~user")
#define CMT_CGIBIN      N_("Directory corresponding to /cgi-bin")
#define CMT_CONFIRM_QQ  N_("Confirm when quitting with q")
#define CMT_CLOSE_TAB_BACK N_("Close tab if buffer is last when back")
#define CMT_EMACS_LIKE_LINEEDIT	N_("Enable Emacs-style line editing")
#define CMT_SPACE_AUTOCOMPLETE  N_("Space key triggers file completion while editing URLs")
#define CMT_VI_PREC_NUM	 N_("Enable vi-like numeric prefix")
#define CMT_LABEL_TOPLINE N_("Move cursor to top line when going to label")
#define CMT_NEXTPAGE_TOPLINE N_("Move cursor to top line when moving to next page")
#define CMT_FOLD_LINE    N_("Fold lines of plain text file")
#define CMT_SHOW_NUM     N_("Show line numbers")
#define CMT_SHOW_SRCH_STR N_("Show search string")
#define CMT_MIMETYPES    N_("List of mime.types files")
#define CMT_MAILCAP      N_("List of mailcap files")
#define CMT_URIMETHODMAP N_("List of urimethodmap files")
#define CMT_EDITOR       N_("Editor")
#define CMT_MAILER       N_("Mailer")
#define CMT_MAILTO_OPTIONS N_("How to call Mailer for mailto URLs with options")
#define CMT_EXTBRZ       N_("External browser")
#define CMT_EXTBRZ2      N_("2nd external browser")
#define CMT_EXTBRZ3      N_("3rd external browser")
#define CMT_EXTBRZ4      N_("4th external browser")
#define CMT_EXTBRZ5      N_("5th external browser")
#define CMT_EXTBRZ6      N_("6th external browser")
#define CMT_EXTBRZ7      N_("7th external browser")
#define CMT_EXTBRZ8      N_("8th external browser")
#define CMT_EXTBRZ9      N_("9th external browser")
#define CMT_DISABLE_SECRET_SECURITY_CHECK	N_("Disable secret file security check")
#define CMT_PASSWDFILE	 N_("Password file")
#define CMT_PRE_FORM_FILE	N_("File for setting form on loading")
#define CMT_SITECONF_FILE	N_("File for preferences for each site")
#define CMT_FTPPASS      N_("Password for anonymous FTP (your mail address)")
#define CMT_FTPPASS_HOSTNAMEGEN N_("Generate domain part of password for FTP")
#define CMT_USERAGENT    N_("User-Agent identification string")
#define CMT_ACCEPTENCODING	N_("Accept-Encoding header")
#define CMT_ACCEPTMEDIA	 N_("Accept header")
#define CMT_ACCEPTLANG   N_("Accept-Language header")
#define CMT_MARK_ALL_PAGES N_("Treat URL-like strings as links in all pages")
#define CMT_WRAP         N_("Wrap search")
#define CMT_VIEW_UNSEENOBJECTS N_("Display unseen objects (e.g. bgimage tag)")
#define CMT_AUTO_UNCOMPRESS	N_("Uncompress compressed data automatically when downloading")
#define CMT_BGEXTVIEW    N_("Run external viewer in the background")
#define CMT_EXT_DIRLIST  N_("Use external program for directory listing")
#define CMT_DIRLIST_CMD  N_("URL of directory listing command")
#define CMT_USE_DICTCOMMAND  N_("Enable dictionary lookup through CGI")
#define CMT_DICTCOMMAND  N_("URL of dictionary lookup command")
#define CMT_IGNORE_NULL_IMG_ALT	N_("Display link name for images lacking ALT")
#define CMT_IFILE        N_("Index file for directories")
#define CMT_RETRY_HTTP   N_("Prepend http:// to URL automatically")
#define CMT_DEFAULT_URL  N_("Default value for open-URL command")
#define CMT_DECODE_CTE   N_("Decode Content-Transfer-Encoding when saving")
#define CMT_PRESERVE_TIMESTAMP N_("Preserve timestamp when saving")
#define CMT_CLEAR_BUF     N_("Free memory of undisplayed buffers")
#define CMT_NOSENDREFERER N_("Suppress `Referer:' header")
#define CMT_CROSSORIGINREFERER N_("Exclude pathname and query string from `Referer:' header when cross domain communication")
#define CMT_IGNORE_CASE N_("Search case-insensitively")
#define CMT_USE_LESSOPEN N_("Use LESSOPEN")
#define CMT_SSL_VERIFY_SERVER N_("Perform SSL server verification")
#define CMT_SSL_CERT_FILE N_("PEM encoded certificate file of client")
#define CMT_SSL_KEY_FILE N_("PEM encoded private key file of client")
#define CMT_SSL_CA_PATH N_("Path to directory for PEM encoded certificates of CAs")
#define CMT_SSL_CA_FILE N_("File consisting of PEM encoded certificates of CAs")
#define CMT_SSL_CA_DEFAULT N_("Use default locations for PEM encoded certificates of CAs")
#define CMT_SSL_FORBID_METHOD N_("List of forbidden SSL methods (2: SSLv2, 3: SSLv3, t: TLSv1.0, 5: TLSv1.1, 6: TLSv1.2, 7: TLSv1.3)")
#ifdef SSL_CTX_set_min_proto_version
#define CMT_SSL_MIN_VERSION N_("Minimum SSL version (all, TLSv1.0, TLSv1.1, TLSv1.2, or TLSv1.3)")
#endif
#define CMT_SSL_CIPHER N_("SSL ciphers for TLSv1.2 and below (e.g. DEFAULT:@SECLEVEL=2)")
#define CMT_USECOOKIE   N_("Enable cookie processing")
#define CMT_SHOWCOOKIE  N_("Print a message when receiving a cookie")
#define CMT_ACCEPTCOOKIE N_("Accept cookies")
#define CMT_ACCEPTBADCOOKIE N_("Action to be taken on invalid cookie")
#define CMT_COOKIE_REJECT_DOMAINS N_("Domains to reject cookies from")
#define CMT_COOKIE_ACCEPT_DOMAINS N_("Domains to accept cookies from")
#define CMT_COOKIE_AVOID_WONG_NUMBER_OF_DOTS N_("Domains to avoid [wrong number of dots]")
#define CMT_FOLLOW_REDIRECTION N_("Number of redirections to follow")
#define CMT_META_REFRESH N_("Enable processing of meta-refresh tag")
#define CMT_LOCALHOST_ONLY N_("Restrict connections only to localhost")


#define CMT_DISPLAY_CHARSET  N_("Display charset")
#define CMT_DOCUMENT_CHARSET N_("Default document charset")
#define CMT_AUTO_DETECT      N_("Automatic charset detect when loading")
#define CMT_SYSTEM_CHARSET   N_("System charset")
#define CMT_FOLLOW_LOCALE    N_("System charset follows locale(LC_CTYPE)")
#define CMT_EXT_HALFDUMP     N_("Output halfdump with display charset")
#define CMT_USE_WIDE         N_("Use multi column characters")
#define CMT_USE_COMBINING    N_("Use combining characters")
#define CMT_EAST_ASIAN_WIDTH N_("Use double width for some Unicode characters")
#define CMT_USE_LANGUAGE_TAG N_("Use Unicode language tags")
#define CMT_UCS_CONV         N_("Charset conversion using Unicode map")
#define CMT_PRE_CONV         N_("Charset conversion when loading")
#define CMT_SEARCH_CONV      N_("Adjust search string for document charset")
#define CMT_FIX_WIDTH_CONV   N_("Fix character width when conversion")
#define CMT_USE_GB12345_MAP  N_("Use GB 12345 Unicode map instead of GB 2312's")
#define CMT_USE_JISX0201     N_("Use JIS X 0201 Roman for ISO-2022-JP")
#define CMT_USE_JISC6226     N_("Use JIS C 6226:1978 for ISO-2022-JP")
#define CMT_USE_JISX0201K    N_("Use JIS X 0201 Katakana")
#define CMT_USE_JISX0212     N_("Use JIS X 0212:1990 (Supplemental Kanji)")
#define CMT_USE_JISX0213     N_("Use JIS X 0213:2000 (2000JIS)")
#define CMT_STRICT_ISO2022   N_("Strict ISO-2022-JP/KR/CN")
#define CMT_GB18030_AS_UCS   N_("Treat 4 bytes char. of GB18030 as Unicode")
#define CMT_SIMPLE_PRESERVE_SPACE N_("Simple Preserve space")

#define CMT_KEYMAP_FILE N_("keymap file")

#define PI_TEXT    0
#define PI_ONOFF   1
#define PI_SEL_C   2
#define PI_CODE    3

struct sel_c {
    int value;
    char *cvalue;
    char *text;
};

static struct sel_c colorstr[] = {
    {0, "black", N_("black")},
    {1, "red", N_("red")},
    {2, "green", N_("green")},
    {3, "yellow", N_("yellow")},
    {4, "blue", N_("blue")},
    {5, "magenta", N_("magenta")},
    {6, "cyan", N_("cyan")},
    {7, "white", N_("white")},
    {8, "terminal", N_("terminal")},
    {0, NULL, NULL}
};

#if 1				/* ANSI-C ? */
#define N_STR(x)	#x
#define N_S(x)	(x), N_STR(x)
#else				/* for traditional cpp? */
static char n_s[][2] = {
    {'0', 0},
    {'1', 0},
    {'2', 0},
};
#define N_S(x) (x), n_s[(x)]
#endif


static struct sel_c defaulturls[] = {
    {N_S(DEFAULT_URL_EMPTY), N_("none")},
    {N_S(DEFAULT_URL_CURRENT), N_("current URL")},
    {N_S(DEFAULT_URL_LINK), N_("link URL")},
    {0, NULL, NULL}
};

static struct sel_c displayinsdel[] = {
    {N_S(DISPLAY_INS_DEL_SIMPLE), N_("simple")},
    {N_S(DISPLAY_INS_DEL_NORMAL), N_("use tag")},
    {N_S(DISPLAY_INS_DEL_FONTIFY), N_("fontify")},
    {0, NULL, NULL}
};


#ifdef INET6
static struct sel_c dnsorders[] = {
    {N_S(DNS_ORDER_UNSPEC), N_("unspecified")},
    {N_S(DNS_ORDER_INET_INET6), N_("inet inet6")},
    {N_S(DNS_ORDER_INET6_INET), N_("inet6 inet")},
    {N_S(DNS_ORDER_INET_ONLY), N_("inet only")},
    {N_S(DNS_ORDER_INET6_ONLY), N_("inet6 only")},
    {0, NULL, NULL}
};
#endif				/* INET6 */

static struct sel_c badcookiestr[] = {
    {N_S(ACCEPT_BAD_COOKIE_DISCARD), N_("discard")},
    {N_S(ACCEPT_BAD_COOKIE_ASK), N_("ask")},
    {0, NULL, NULL}
};

static struct sel_c mailtooptionsstr[] = {
    {N_S(MAILTO_OPTIONS_IGNORE), N_("ignore options and use only the address")},
    {N_S(MAILTO_OPTIONS_USE_MAILTO_URL), N_("use full mailto URL")},
    {0, NULL, NULL}
};

static wc_ces_list *display_charset_str = NULL;
static wc_ces_list *document_charset_str = NULL;
static wc_ces_list *system_charset_str = NULL;
static struct sel_c auto_detect_str[] = {
    {N_S(WC_OPT_DETECT_OFF), N_("OFF")},
    {N_S(WC_OPT_DETECT_ISO_2022), N_("Only ISO 2022")},
    {N_S(WC_OPT_DETECT_ON), N_("ON")},
    {0, NULL, NULL}
};

static struct sel_c graphic_char_str[] = {
    {N_S(GRAPHIC_CHAR_ASCII), N_("ASCII")},
    {N_S(GRAPHIC_CHAR_CHARSET), N_("charset specific")},
    {N_S(GRAPHIC_CHAR_DEC), N_("DEC special graphics")},
    {0, NULL, NULL}
};

static struct sel_c inlineimgstr[] = {
    {N_S(INLINE_IMG_NONE), N_("external command")},
    {N_S(INLINE_IMG_OSC5379), N_("OSC 5379 (mlterm)")},
    {N_S(INLINE_IMG_SIXEL), N_("sixel (img2sixel)")},
    {N_S(INLINE_IMG_ITERM2), N_("OSC 1337 (iTerm2)")},
    {N_S(INLINE_IMG_KITTY), N_("kitty (ImageMagick)")},
    {0, NULL, NULL}
};

struct param_ptr params1[] = {
    {"tabstop", P_NZINT, PI_TEXT, (void *)&Tabstop, CMT_TABSTOP, NULL},
    {"indent_incr", P_NZINT, PI_TEXT, (void *)&IndentIncr, CMT_INDENT_INCR,
     NULL},
    {"pixel_per_char", P_PIXELS, PI_TEXT, (void *)&pixel_per_char,
     CMT_PIXEL_PER_CHAR, NULL},
    {"pixel_per_line", P_PIXELS, PI_TEXT, (void *)&pixel_per_line,
     CMT_PIXEL_PER_LINE, NULL},
    {"frame", P_CHARINT, PI_ONOFF, (void *)&RenderFrame, CMT_FRAME, NULL},
    {"target_self", P_CHARINT, PI_ONOFF, (void *)&TargetSelf, CMT_TSELF, NULL},
    {"open_tab_blank", P_INT, PI_ONOFF, (void *)&open_tab_blank,
     CMT_OPEN_TAB_BLANK, NULL},
    {"open_tab_dl_list", P_INT, PI_ONOFF, (void *)&open_tab_dl_list,
     CMT_OPEN_TAB_DL_LIST, NULL},
    {"display_link", P_INT, PI_ONOFF, (void *)&displayLink, CMT_DISPLINK,
     NULL},
    {"display_link_number", P_INT, PI_ONOFF, (void *)&displayLinkNumber,
     CMT_DISPLINKNUMBER, NULL},
    {"decode_url", P_INT, PI_ONOFF, (void *)&DecodeURL, CMT_DECODE_URL, NULL},
    {"display_lineinfo", P_INT, PI_ONOFF, (void *)&displayLineInfo,
     CMT_DISPLINEINFO, NULL},
    {"ext_dirlist", P_INT, PI_ONOFF, (void *)&UseExternalDirBuffer,
     CMT_EXT_DIRLIST, NULL},
    {"dirlist_cmd", P_STRING, PI_TEXT, (void *)&DirBufferCommand,
     CMT_DIRLIST_CMD, NULL},
    {"use_dictcommand", P_INT, PI_ONOFF, (void *)&UseDictCommand,
     CMT_USE_DICTCOMMAND, NULL},
    {"dictcommand", P_STRING, PI_TEXT, (void *)&DictCommand,
     CMT_DICTCOMMAND, NULL},
    {"multicol", P_INT, PI_ONOFF, (void *)&multicolList, CMT_MULTICOL, NULL},
    {"alt_entity", P_CHARINT, PI_ONOFF, (void *)&UseAltEntity, CMT_ALT_ENTITY,
     NULL},
    {"graphic_char", P_CHARINT, PI_SEL_C, (void *)&UseGraphicChar,
     CMT_GRAPHIC_CHAR, (void *)graphic_char_str},
    {"display_borders", P_CHARINT, PI_ONOFF, (void *)&DisplayBorders,
     CMT_DISP_BORDERS, NULL},
    {"disable_center", P_CHARINT, PI_ONOFF, (void *)&DisableCenter,
     CMT_DISABLE_CENTER, NULL},
    {"fold_textarea", P_CHARINT, PI_ONOFF, (void *)&FoldTextarea,
     CMT_FOLD_TEXTAREA, NULL},
    {"display_ins_del", P_INT, PI_SEL_C, (void *)&displayInsDel,
     CMT_DISP_INS_DEL, displayinsdel},
    {"ignore_null_img_alt", P_INT, PI_ONOFF, (void *)&ignore_null_img_alt,
     CMT_IGNORE_NULL_IMG_ALT, NULL},
    {"view_unseenobject", P_INT, PI_ONOFF, (void *)&view_unseenobject,
     CMT_VIEW_UNSEENOBJECTS, NULL},
    /* XXX: emacs-w3m force to off display_image even if image options off */
    {"display_image", P_INT, PI_ONOFF, (void *)&displayImage, CMT_DISP_IMAGE,
     NULL},
    {"pseudo_inlines", P_INT, PI_ONOFF, (void *)&pseudoInlines,
     CMT_PSEUDO_INLINES, NULL},
    {"auto_image", P_INT, PI_ONOFF, (void *)&autoImage, CMT_AUTO_IMAGE, NULL},
    {"max_load_image", P_INT, PI_TEXT, (void *)&maxLoadImage,
     CMT_MAX_LOAD_IMAGE, NULL},
    {"ext_image_viewer", P_INT, PI_ONOFF, (void *)&useExtImageViewer,
     CMT_EXT_IMAGE_VIEWER, NULL},
    {"image_scale", P_SCALE, PI_TEXT, (void *)&image_scale, CMT_IMAGE_SCALE,
     NULL},
    {"inline_img_protocol", P_INT, PI_SEL_C, (void *)&enable_inline_image,
     CMT_INLINE_IMG_PROTOCOL, (void *)inlineimgstr},
    {"imgdisplay", P_STRING, PI_TEXT, (void *)&Imgdisplay, CMT_IMGDISPLAY,
     NULL},
    {"image_map_list", P_INT, PI_ONOFF, (void *)&image_map_list,
     CMT_IMAGE_MAP_LIST, NULL},
    {"fold_line", P_INT, PI_ONOFF, (void *)&FoldLine, CMT_FOLD_LINE, NULL},
    {"show_lnum", P_INT, PI_ONOFF, (void *)&showLineNum, CMT_SHOW_NUM, NULL},
    {"show_srch_str", P_INT, PI_ONOFF, (void *)&show_srch_str,
     CMT_SHOW_SRCH_STR, NULL},
    {"label_topline", P_INT, PI_ONOFF, (void *)&label_topline,
     CMT_LABEL_TOPLINE, NULL},
    {"nextpage_topline", P_INT, PI_ONOFF, (void *)&nextpage_topline,
     CMT_NEXTPAGE_TOPLINE, NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params2[] = {
    {"color", P_INT, PI_ONOFF, (void *)&useColor, CMT_COLOR, NULL},
    {"basic_color", P_COLOR, PI_SEL_C, (void *)&basic_color, CMT_B_COLOR,
     (void *)colorstr},
    {"anchor_color", P_COLOR, PI_SEL_C, (void *)&anchor_color, CMT_A_COLOR,
     (void *)colorstr},
    {"image_color", P_COLOR, PI_SEL_C, (void *)&image_color, CMT_I_COLOR,
     (void *)colorstr},
    {"form_color", P_COLOR, PI_SEL_C, (void *)&form_color, CMT_F_COLOR,
     (void *)colorstr},
    {"mark_color", P_COLOR, PI_SEL_C, (void *)&mark_color, CMT_MARK_COLOR,
     (void *)colorstr},
    {"bg_color", P_COLOR, PI_SEL_C, (void *)&bg_color, CMT_BG_COLOR,
     (void *)colorstr},
    {"active_style", P_INT, PI_ONOFF, (void *)&useActiveColor,
     CMT_ACTIVE_STYLE, NULL},
    {"active_color", P_COLOR, PI_SEL_C, (void *)&active_color, CMT_C_COLOR,
     (void *)colorstr},
    {"visited_anchor", P_INT, PI_ONOFF, (void *)&useVisitedColor,
     CMT_VISITED_ANCHOR, NULL},
    {"visited_color", P_COLOR, PI_SEL_C, (void *)&visited_color, CMT_V_COLOR,
     (void *)colorstr},
    {NULL, 0, 0, NULL, NULL, NULL},
};


struct param_ptr params3[] = {
    {"pagerline", P_NZINT, PI_TEXT, (void *)&PagerMax, CMT_PAGERLINE, NULL},
    {"use_history", P_INT, PI_ONOFF, (void *)&UseHistory, CMT_HISTORY, NULL},
    {"history", P_INT, PI_TEXT, (void *)&URLHistSize, CMT_HISTSIZE, NULL},
    {"save_hist", P_INT, PI_ONOFF, (void *)&SaveURLHist, CMT_SAVEHIST, NULL},
    {"confirm_qq", P_INT, PI_ONOFF, (void *)&confirm_on_quit, CMT_CONFIRM_QQ,
     NULL},
    {"close_tab_back", P_INT, PI_ONOFF, (void *)&close_tab_back,
     CMT_CLOSE_TAB_BACK, NULL},
    {"emacs_like_lineedit", P_INT, PI_ONOFF, (void *)&emacs_like_lineedit,
     CMT_EMACS_LIKE_LINEEDIT, NULL},
    {"space_autocomplete", P_INT, PI_ONOFF, (void *)&space_autocomplete,
     CMT_SPACE_AUTOCOMPLETE, NULL},
    {"vi_prec_num", P_INT, PI_ONOFF, (void *)&vi_prec_num, CMT_VI_PREC_NUM,
     NULL},
    {"mark_all_pages", P_INT, PI_ONOFF, (void *)&MarkAllPages,
     CMT_MARK_ALL_PAGES, NULL},
    {"wrap_search", P_INT, PI_ONOFF, (void *)&WrapDefault, CMT_WRAP, NULL},
    {"ignorecase_search", P_INT, PI_ONOFF, (void *)&IgnoreCase,
     CMT_IGNORE_CASE, NULL},
    {"clear_buffer", P_INT, PI_ONOFF, (void *)&clear_buffer, CMT_CLEAR_BUF,
     NULL},
    {"decode_cte", P_CHARINT, PI_ONOFF, (void *)&DecodeCTE, CMT_DECODE_CTE,
     NULL},
    {"auto_uncompress", P_CHARINT, PI_ONOFF, (void *)&AutoUncompress,
     CMT_AUTO_UNCOMPRESS, NULL},
    {"preserve_timestamp", P_CHARINT, PI_ONOFF, (void *)&PreserveTimestamp,
     CMT_PRESERVE_TIMESTAMP, NULL},
    {"keymap_file", P_STRING, PI_TEXT, (void *)&keymap_file, CMT_KEYMAP_FILE,
     NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params4[] = {
    {"use_proxy", P_CHARINT, PI_ONOFF, (void *)&use_proxy, CMT_USE_PROXY,
     NULL},
    {"http_proxy", P_STRING, PI_TEXT, (void *)&HTTP_proxy, CMT_HTTP_PROXY,
     NULL},
    {"https_proxy", P_STRING, PI_TEXT, (void *)&HTTPS_proxy, CMT_HTTPS_PROXY,
     NULL},
    {"ftp_proxy", P_STRING, PI_TEXT, (void *)&FTP_proxy, CMT_FTP_PROXY, NULL},
    {"no_proxy", P_STRING, PI_TEXT, (void *)&NO_proxy, CMT_NO_PROXY, NULL},
    {"noproxy_netaddr", P_INT, PI_ONOFF, (void *)&NOproxy_netaddr,
     CMT_NOPROXY_NETADDR, NULL},
    {"no_cache", P_CHARINT, PI_ONOFF, (void *)&NoCache, CMT_NO_CACHE, NULL},

    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params5[] = {
    {"document_root", P_STRING, PI_TEXT, (void *)&document_root, CMT_DROOT,
     NULL},
    {"personal_document_root", P_STRING, PI_TEXT,
     (void *)&personal_document_root, CMT_PDROOT, NULL},
    {"cgi_bin", P_STRING, PI_TEXT, (void *)&cgi_bin, CMT_CGIBIN, NULL},
    {"index_file", P_STRING, PI_TEXT, (void *)&index_file, CMT_IFILE, NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params6[] = {
    {"mime_types", P_STRING, PI_TEXT, (void *)&mimetypes_files, CMT_MIMETYPES,
     NULL},
    {"mailcap", P_STRING, PI_TEXT, (void *)&mailcap_files, CMT_MAILCAP, NULL},
    {"urimethodmap", P_STRING, PI_TEXT, (void *)&urimethodmap_files,
     CMT_URIMETHODMAP, NULL},
    {"editor", P_STRING, PI_TEXT, (void *)&Editor, CMT_EDITOR, NULL},
    {"mailto_options", P_INT, PI_SEL_C, (void *)&MailtoOptions,
     CMT_MAILTO_OPTIONS, (void *)mailtooptionsstr},
    {"mailer", P_STRING, PI_TEXT, (void *)&Mailer, CMT_MAILER, NULL},
    {"extbrowser", P_STRING, PI_TEXT, (void *)&ExtBrowser, CMT_EXTBRZ, NULL},
    {"extbrowser2", P_STRING, PI_TEXT, (void *)&ExtBrowser2, CMT_EXTBRZ2,
     NULL},
    {"extbrowser3", P_STRING, PI_TEXT, (void *)&ExtBrowser3, CMT_EXTBRZ3,
     NULL},
    {"extbrowser4", P_STRING, PI_TEXT, (void *)&ExtBrowser4, CMT_EXTBRZ4,
     NULL},
    {"extbrowser5", P_STRING, PI_TEXT, (void *)&ExtBrowser5, CMT_EXTBRZ5,
     NULL},
    {"extbrowser6", P_STRING, PI_TEXT, (void *)&ExtBrowser6, CMT_EXTBRZ6,
     NULL},
    {"extbrowser7", P_STRING, PI_TEXT, (void *)&ExtBrowser7, CMT_EXTBRZ7,
     NULL},
    {"extbrowser8", P_STRING, PI_TEXT, (void *)&ExtBrowser8, CMT_EXTBRZ8,
     NULL},
    {"extbrowser9", P_STRING, PI_TEXT, (void *)&ExtBrowser9, CMT_EXTBRZ9,
     NULL},
    {"bgextviewer", P_INT, PI_ONOFF, (void *)&BackgroundExtViewer,
     CMT_BGEXTVIEW, NULL},
    {"use_lessopen", P_INT, PI_ONOFF, (void *)&use_lessopen, CMT_USE_LESSOPEN,
     NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params7[] = {
    {"ssl_forbid_method", P_STRING, PI_TEXT, (void *)&ssl_forbid_method,
     CMT_SSL_FORBID_METHOD, NULL},
#ifdef SSL_CTX_set_min_proto_version
    {"ssl_min_version", P_STRING, PI_TEXT, (void *)&ssl_min_version,
     CMT_SSL_MIN_VERSION, NULL},
#endif
    {"ssl_cipher", P_STRING, PI_TEXT, (void *)&ssl_cipher, CMT_SSL_CIPHER,
     NULL},
    {"ssl_verify_server", P_INT, PI_ONOFF, (void *)&ssl_verify_server,
     CMT_SSL_VERIFY_SERVER, NULL},
    {"ssl_cert_file", P_SSLPATH, PI_TEXT, (void *)&ssl_cert_file,
     CMT_SSL_CERT_FILE, NULL},
    {"ssl_key_file", P_SSLPATH, PI_TEXT, (void *)&ssl_key_file,
     CMT_SSL_KEY_FILE, NULL},
    {"ssl_ca_path", P_SSLPATH, PI_TEXT, (void *)&ssl_ca_path, CMT_SSL_CA_PATH,
     NULL},
    {"ssl_ca_file", P_SSLPATH, PI_TEXT, (void *)&ssl_ca_file, CMT_SSL_CA_FILE,
     NULL},
    {"ssl_ca_default", P_INT, PI_ONOFF, (void *)&ssl_ca_default,
     CMT_SSL_CA_DEFAULT, NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params8[] = {
    {"use_cookie", P_INT, PI_ONOFF, (void *)&use_cookie, CMT_USECOOKIE, NULL},
    {"show_cookie", P_INT, PI_ONOFF, (void *)&show_cookie,
     CMT_SHOWCOOKIE, NULL},
    {"accept_cookie", P_INT, PI_ONOFF, (void *)&accept_cookie,
     CMT_ACCEPTCOOKIE, NULL},
    {"accept_bad_cookie", P_INT, PI_SEL_C, (void *)&accept_bad_cookie,
     CMT_ACCEPTBADCOOKIE, (void *)badcookiestr},
    {"cookie_reject_domains", P_STRING, PI_TEXT,
     (void *)&cookie_reject_domains, CMT_COOKIE_REJECT_DOMAINS, NULL},
    {"cookie_accept_domains", P_STRING, PI_TEXT,
     (void *)&cookie_accept_domains, CMT_COOKIE_ACCEPT_DOMAINS, NULL},
    {"cookie_avoid_wrong_number_of_dots", P_STRING, PI_TEXT,
     (void *)&cookie_avoid_wrong_number_of_dots,
     CMT_COOKIE_AVOID_WONG_NUMBER_OF_DOTS, NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params9[] = {
    {"passwd_file", P_STRING, PI_TEXT, (void *)&passwd_file, CMT_PASSWDFILE,
     NULL},
    {"disable_secret_security_check", P_INT, PI_ONOFF,
     (void *)&disable_secret_security_check, CMT_DISABLE_SECRET_SECURITY_CHECK,
     NULL},
    {"ftppasswd", P_STRING, PI_TEXT, (void *)&ftppasswd, CMT_FTPPASS, NULL},
    {"ftppass_hostnamegen", P_INT, PI_ONOFF, (void *)&ftppass_hostnamegen,
     CMT_FTPPASS_HOSTNAMEGEN, NULL},
    {"pre_form_file", P_STRING, PI_TEXT, (void *)&pre_form_file,
     CMT_PRE_FORM_FILE, NULL},
    {"siteconf_file", P_STRING, PI_TEXT, (void *)&siteconf_file,
     CMT_SITECONF_FILE, NULL},
    {"user_agent", P_STRING, PI_TEXT, (void *)&UserAgent, CMT_USERAGENT, NULL},
    {"no_referer", P_INT, PI_ONOFF, (void *)&NoSendReferer, CMT_NOSENDREFERER,
     NULL},
    {"cross_origin_referer", P_INT, PI_ONOFF, (void *)&CrossOriginReferer,
     CMT_CROSSORIGINREFERER, NULL},
    {"accept_language", P_STRING, PI_TEXT, (void *)&AcceptLang, CMT_ACCEPTLANG,
     NULL},
    {"accept_encoding", P_STRING, PI_TEXT, (void *)&AcceptEncoding,
     CMT_ACCEPTENCODING,
     NULL},
    {"accept_media", P_STRING, PI_TEXT, (void *)&AcceptMedia, CMT_ACCEPTMEDIA,
     NULL},
    {"argv_is_url", P_CHARINT, PI_ONOFF, (void *)&ArgvIsURL, CMT_ARGV_IS_URL,
     NULL},
    {"retry_http", P_INT, PI_ONOFF, (void *)&retryAsHttp, CMT_RETRY_HTTP,
     NULL},
    {"default_url", P_INT, PI_SEL_C, (void *)&DefaultURLString,
     CMT_DEFAULT_URL, (void *)defaulturls},
    {"follow_redirection", P_INT, PI_TEXT, &FollowRedirection,
     CMT_FOLLOW_REDIRECTION, NULL},
    {"meta_refresh", P_CHARINT, PI_ONOFF, (void *)&MetaRefresh,
     CMT_META_REFRESH, NULL},
    {"localhost_only", P_CHARINT, PI_ONOFF, (void *)&LocalhostOnly,
     CMT_LOCALHOST_ONLY, NULL},
#ifdef INET6
    {"dns_order", P_INT, PI_SEL_C, (void *)&DNS_order, CMT_DNS_ORDER,
     (void *)dnsorders},
#endif				/* INET6 */
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_ptr params10[] = {
    {"display_charset", P_CODE, PI_CODE, (void *)&DisplayCharset,
     CMT_DISPLAY_CHARSET, (void *)&display_charset_str},
    {"document_charset", P_CODE, PI_CODE, (void *)&DocumentCharset,
     CMT_DOCUMENT_CHARSET, (void *)&document_charset_str},
    {"auto_detect", P_CHARINT, PI_SEL_C, (void *)&WcOption.auto_detect,
     CMT_AUTO_DETECT, (void *)auto_detect_str},
    {"system_charset", P_CODE, PI_CODE, (void *)&SystemCharset,
     CMT_SYSTEM_CHARSET, (void *)&system_charset_str},
    {"follow_locale", P_CHARINT, PI_ONOFF, (void *)&FollowLocale,
     CMT_FOLLOW_LOCALE, NULL},
    {"ext_halfdump", P_CHARINT, PI_ONOFF, (void *)&ExtHalfdump,
     CMT_EXT_HALFDUMP, NULL},
    {"use_wide", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_wide, CMT_USE_WIDE,
     NULL},
    {"use_combining", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_combining,
     CMT_USE_COMBINING, NULL},
    {"east_asian_width", P_CHARINT, PI_ONOFF,
     (void *)&WcOption.east_asian_width, CMT_EAST_ASIAN_WIDTH, NULL},
    {"use_language_tag", P_CHARINT, PI_ONOFF,
     (void *)&WcOption.use_language_tag, CMT_USE_LANGUAGE_TAG, NULL},
    {"ucs_conv", P_CHARINT, PI_ONOFF, (void *)&WcOption.ucs_conv, CMT_UCS_CONV,
     NULL},
    {"pre_conv", P_CHARINT, PI_ONOFF, (void *)&WcOption.pre_conv, CMT_PRE_CONV,
     NULL},
    {"search_conv", P_CHARINT, PI_ONOFF, (void *)&SearchConv, CMT_SEARCH_CONV,
     NULL},
    {"fix_width_conv", P_CHARINT, PI_ONOFF, (void *)&WcOption.fix_width_conv,
     CMT_FIX_WIDTH_CONV, NULL},
    {"use_gb12345_map", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_gb12345_map,
     CMT_USE_GB12345_MAP, NULL},
    {"use_jisx0201", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_jisx0201,
     CMT_USE_JISX0201, NULL},
    {"use_jisc6226", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_jisc6226,
     CMT_USE_JISC6226, NULL},
    {"use_jisx0201k", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_jisx0201k,
     CMT_USE_JISX0201K, NULL},
    {"use_jisx0212", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_jisx0212,
     CMT_USE_JISX0212, NULL},
    {"use_jisx0213", P_CHARINT, PI_ONOFF, (void *)&WcOption.use_jisx0213,
     CMT_USE_JISX0213, NULL},
    {"strict_iso2022", P_CHARINT, PI_ONOFF, (void *)&WcOption.strict_iso2022,
     CMT_STRICT_ISO2022, NULL},
    {"gb18030_as_ucs", P_CHARINT, PI_ONOFF, (void *)&WcOption.gb18030_as_ucs,
     CMT_GB18030_AS_UCS, NULL},
    {"simple_preserve_space", P_CHARINT, PI_ONOFF, (void *)&SimplePreserveSpace,
     CMT_SIMPLE_PRESERVE_SPACE, NULL},
    {NULL, 0, 0, NULL, NULL, NULL},
};

struct param_section sections[] = {
    {N_("Display Settings"), params1},
    {N_("Color Settings"), params2},
    {N_("Miscellaneous Settings"), params3},
    {N_("Directory Settings"), params5},
    {N_("External Program Settings"), params6},
    {N_("Network Settings"), params9},
    {N_("Proxy Settings"), params4},
    {N_("SSL Settings"), params7},
    {N_("Cookie Settings"), params8},
    {N_("Charset Settings"), params10},
    {NULL, NULL}
};

static Str to_str(struct param_ptr *p);

static int
compare_table(struct rc_search_table *a, struct rc_search_table *b)
{
    return strcmp(a->param->name, b->param->name);
}

static void
create_option_search_table()
{
    int i, j, k;
    int diff1, diff2;
    char *p, *q;

    /* count table size */
    RC_table_size = 0;
    for (j = 0; sections[j].name != NULL; j++) {
	i = 0;
	while (sections[j].params[i].name) {
	    i++;
	    RC_table_size++;
	}
    }

    RC_search_table = New_N(struct rc_search_table, RC_table_size);
    k = 0;
    for (j = 0; sections[j].name != NULL; j++) {
	i = 0;
	while (sections[j].params[i].name) {
	    RC_search_table[k].param = &sections[j].params[i];
	    k++;
	    i++;
	}
    }

    qsort(RC_search_table, RC_table_size, sizeof(struct rc_search_table),
	  (int (*)(const void *, const void *))compare_table);

    diff2 = 0;
    for (i = 0; i < RC_table_size - 1; i++) {
	p = RC_search_table[i].param->name;
	q = RC_search_table[i + 1].param->name;
	for (j = 0; p[j] != '\0' && q[j] != '\0' && p[j] == q[j]; j++) ;
	diff1 = j;
	if (diff1 > diff2)
	    RC_search_table[i].uniq_pos = diff1 + 1;
	else
	    RC_search_table[i].uniq_pos = diff2 + 1;
	diff2 = diff1;
    }
}

struct param_ptr *
search_param(char *name)
{
    size_t b, e, i;
    int cmp;
    int len = strlen(name);

    for (b = 0, e = RC_table_size - 1; b <= e;) {
	i = (b + e) / 2;
	cmp = strncmp(name, RC_search_table[i].param->name, len);

	if (!cmp) {
	    if (len >= RC_search_table[i].uniq_pos) {
		return RC_search_table[i].param;
	    }
	    else {
		while ((cmp =
			strcmp(name, RC_search_table[i].param->name)) <= 0)
		    if (!cmp)
			return RC_search_table[i].param;
		    else if (i == 0)
			return NULL;
		    else
			i--;
		/* ambiguous */
		return NULL;
	    }
	}
	else if (cmp < 0) {
	    if (i == 0)
		return NULL;
	    e = i - 1;
	}
	else
	    b = i + 1;
    }
    return NULL;
}

/* show parameter with bad options invokation */
void
show_params(FILE * fp)
{
    int i, j, l;
    const char *t = "";
    char *cmt;

#ifdef ENABLE_NLS
    OptionCharset = SystemCharset;	/* FIXME */
#endif

    fputs("\nconfiguration parameters\n", fp);
    for (j = 0; sections[j].name != NULL; j++) {
	if (!OptionEncode)
	    cmt =
		wc_conv(_(sections[j].name), OptionCharset,
			InnerCharset)->ptr;
	else
	    cmt = sections[j].name;
	fprintf(fp, "  section[%d]: %s\n", j, conv_to_system(cmt));
	i = 0;
	while (sections[j].params[i].name) {
	    switch (sections[j].params[i].type) {
	    case P_INT:
	    case P_SHORT:
	    case P_CHARINT:
	    case P_NZINT:
		t = (sections[j].params[i].inputtype ==
		     PI_ONOFF) ? "bool" : "number";
		break;
	    case P_CHAR:
		t = "char";
		break;
	    case P_STRING:
		t = "string";
		break;
#if defined(USE_SSL) && defined(USE_SSL_VERIFY)
	    case P_SSLPATH:
		t = "path";
		break;
#endif
	    case P_COLOR:
		t = "color";
		break;
	    case P_CODE:
		t = "charset";
		break;
	    case P_PIXELS:
		t = "number";
		break;
	    case P_SCALE:
		t = "percent";
		break;
	    }
	    if (!OptionEncode)
		cmt = wc_conv(_(sections[j].params[i].comment),
			      OptionCharset, InnerCharset)->ptr;
	    else
		cmt = sections[j].params[i].comment;
	    l = 30 - (strlen(sections[j].params[i].name) + strlen(t));
	    if (l < 0)
		l = 1;
	    fprintf(fp, "    -o %s=<%s>%*s%s\n",
		    sections[j].params[i].name, t, l, " ",
		    conv_to_system(cmt));
	    i++;
	}
    }
}

int
str_to_bool(char *value, int old)
{
    if (value == NULL)
	return 1;
    switch (TOLOWER(*value)) {
    case '0':
    case 'f':			/* false */
    case 'n':			/* no */
    case 'u':			/* undef */
	return 0;
    case 'o':
	if (TOLOWER(value[1]) == 'f')	/* off */
	    return 0;
	return 1;		/* on */
    case 't':
	if (TOLOWER(value[1]) == 'o')	/* toggle */
	    return !old;
	return 1;		/* true */
    case '!':
    case 'r':			/* reverse */
    case 'x':			/* exchange */
	return !old;
    }
    return 1;
}

static int
str_to_color(char *value)
{
    if (value == NULL)
	return 8;		/* terminal */
    switch (TOLOWER(*value)) {
    case '0':
	return 0;		/* black */
    case '1':
    case 'r':
	return 1;		/* red */
    case '2':
    case 'g':
	return 2;		/* green */
    case '3':
    case 'y':
	return 3;		/* yellow */
    case '4':
	return 4;		/* blue */
    case '5':
    case 'm':
	return 5;		/* magenta */
    case '6':
    case 'c':
	return 6;		/* cyan */
    case '7':
    case 'w':
	return 7;		/* white */
    case '8':
    case 't':
	return 8;		/* terminal */
    case 'b':
	if (!strncasecmp(value, "blu", 3))
	    return 4;		/* blue */
	else
	    return 0;		/* black */
    }
    return 8;			/* terminal */
}

static int
set_param(char *name, char *value)
{
    struct param_ptr *p;
    double ppc;

    if (value == NULL)
	return 0;
    p = search_param(name);
    if (p == NULL)
	return 0;
    switch (p->type) {
    case P_INT:
	if (atoi(value) >= 0)
	    *(int *)p->varptr = (p->inputtype == PI_ONOFF)
		? str_to_bool(value, *(int *)p->varptr) : atoi(value);
	break;
    case P_NZINT:
	if (atoi(value) > 0)
	    *(int *)p->varptr = atoi(value);
	break;
    case P_SHORT:
	*(short *)p->varptr = (p->inputtype == PI_ONOFF)
	    ? str_to_bool(value, *(short *)p->varptr) : atoi(value);
	break;
    case P_CHARINT:
	*(char *)p->varptr = (p->inputtype == PI_ONOFF)
	    ? str_to_bool(value, *(char *)p->varptr) : atoi(value);
	break;
    case P_CHAR:
	*(char *)p->varptr = value[0];
	break;
    case P_STRING:
	*(char **)p->varptr = value;
	break;
#if defined(USE_SSL) && defined(USE_SSL_VERIFY)
    case P_SSLPATH:
	if (value != NULL && value[0] != '\0')
	    *(char **)p->varptr = rcFile(value);
	else
	    *(char **)p->varptr = NULL;
	ssl_path_modified = 1;
	break;
#endif
    case P_COLOR:
	*(int *)p->varptr = str_to_color(value);
	break;
    case P_CODE:
	*(wc_ces *) p->varptr =
	    wc_guess_charset_short(value, *(wc_ces *) p->varptr);
	break;
    case P_PIXELS:
	ppc = atof(value);
	if (ppc >= MINIMUM_PIXEL_PER_CHAR && ppc <= MAXIMUM_PIXEL_PER_CHAR * 2)
	    *(double *)p->varptr = ppc;
	break;
    case P_SCALE:
	ppc = atof(value);
	if (ppc >= 10 && ppc <= 1000)
	    *(double *)p->varptr = ppc;
	break;
    }
    return 1;
}

int
set_param_option(char *option)
{
    Str tmp = Strnew();
    char *p = option, *q;

    while (*p && !IS_SPACE(*p) && *p != '=')
	Strcat_char(tmp, *p++);
    while (*p && IS_SPACE(*p))
	p++;
    if (*p == '=') {
	p++;
	while (*p && IS_SPACE(*p))
	    p++;
    }
    Strlower(tmp);
    if (set_param(tmp->ptr, p))
	goto option_assigned;
    q = tmp->ptr;
    if (!strncmp(q, "no", 2)) {	/* -o noxxx, -o no-xxx, -o no_xxx */
	q += 2;
	if (*q == '-' || *q == '_')
	    q++;
    }
    else if (tmp->ptr[0] == '-')	/* -o -xxx */
	q++;
    else
	return 0;
    if (set_param(q, "0"))
	goto option_assigned;
    return 0;
  option_assigned:
    return 1;
}

char *
get_param_option(char *name)
{
    struct param_ptr *p;

    p = search_param(name);
    return p ? to_str(p)->ptr : NULL;
}

static void
interpret_rc(FILE * f)
{
    Str line;
    Str tmp;
    char *p;

    for (;;) {
	line = Strfgets(f);
	if (line->length == 0)		/* end of file */
	    break;
	Strchop(line);
	if (line->length == 0)		/* blank line */
	    continue;
	Strremovefirstspaces(line);
	if (line->ptr[0] == '#')	/* comment */
	    continue;
	tmp = Strnew();
	p = line->ptr;
	while (*p && !IS_SPACE(*p))
	    Strcat_char(tmp, *p++);
	while (*p && IS_SPACE(*p))
	    p++;
	Strlower(tmp);
	set_param(tmp->ptr, p);
    }
}

void
parse_proxy()
{
    if (non_null(HTTP_proxy))
	parseURL(HTTP_proxy, &HTTP_proxy_parsed, NULL);
    if (non_null(HTTPS_proxy))
	parseURL(HTTPS_proxy, &HTTPS_proxy_parsed, NULL);
    if (non_null(FTP_proxy))
	parseURL(FTP_proxy, &FTP_proxy_parsed, NULL);
    if (non_null(NO_proxy))
	set_no_proxy(NO_proxy);
}

void
parse_cookie()
{
    if (non_null(cookie_reject_domains))
	Cookie_reject_domains = make_domain_list(cookie_reject_domains);
    if (non_null(cookie_accept_domains))
	Cookie_accept_domains = make_domain_list(cookie_accept_domains);
    if (non_null(cookie_avoid_wrong_number_of_dots))
	Cookie_avoid_wrong_number_of_dots_domains
	       	= make_domain_list(cookie_avoid_wrong_number_of_dots);
}

#define do_mkdir(dir,mode) mkdir(dir,mode)

static void loadSiteconf(void);

void
sync_with_option(void)
{
    if (PagerMax < LINES)
	PagerMax = LINES;
    WrapSearch = WrapDefault;
    parse_proxy();
    parse_cookie();
    initMailcap();
    initMimeTypes();
    initURIMethods();
    if (fmInitialized && (displayImage || enable_inline_image))
	initImage();
    loadPasswd();
    loadPreForm();
    loadSiteconf();

    if (AcceptLang == NULL || *AcceptLang == '\0') {
	/* TRANSLATORS: 
	 * AcceptLang default: this is used in Accept-Language: HTTP request 
	 * header. For example, ja.po should translate it as
	 * "ja;q=1.0, en;q=0.5" like that.
	 */
	AcceptLang = _("en;q=1.0");
    }
    if (AcceptEncoding == NULL || *AcceptEncoding == '\0')
	AcceptEncoding = acceptableEncoding();
    if (AcceptMedia == NULL || *AcceptMedia == '\0')
	AcceptMedia = acceptableMimeTypes();
    update_utf8_symbol();
    if (fmInitialized) {
	initKeymap(FALSE);
	initMenu();
    }
}

void
init_rc(void)
{
    int i;
    struct stat st;
    FILE *f;

    if (rc_dir != NULL)
	goto open_rc;

    rc_dir = expandPath(RC_DIR);
    i = strlen(rc_dir);
    if (i > 1 && rc_dir[i - 1] == '/')
	rc_dir[i - 1] = '\0';

    display_charset_str = wc_get_ces_list();
    document_charset_str = display_charset_str;
    system_charset_str = display_charset_str;

    if (stat(rc_dir, &st) < 0) {
	if (errno == ENOENT) {	/* no directory */
	    if (do_mkdir(rc_dir, 0700) < 0) {
		/* fprintf(stderr, "Can't create config directory (%s)!\n", rc_dir); */
		goto rc_dir_err;
	    }
	    else {
		stat(rc_dir, &st);
	    }
	}
	else {
	    /* fprintf(stderr, "Can't open config directory (%s)!\n", rc_dir); */
	    goto rc_dir_err;
	}
    }
    if (!S_ISDIR(st.st_mode)) {
	/* not a directory */
	/* fprintf(stderr, "%s is not a directory!\n", rc_dir); */
	goto rc_dir_err;
    }
    if (!(st.st_mode & S_IWUSR)) {
	/* fprintf(stderr, "%s is not writable!\n", rc_dir); */
	goto rc_dir_err;
    }
    no_rc_dir = FALSE;
    tmp_dir = rc_dir;

    if (config_file == NULL)
	config_file = rcFile(CONFIG_FILE);

    create_option_search_table();

  open_rc:
    /* open config file */
    if ((f = fopen(etcFile(W3MCONFIG), "rt")) != NULL) {
	interpret_rc(f);
	fclose(f);
    }
    if ((f = fopen(confFile(CONFIG_FILE), "rt")) != NULL) {
	interpret_rc(f);
	fclose(f);
    }
    if (config_file && (f = fopen(config_file, "rt")) != NULL) {
	interpret_rc(f);
	fclose(f);
    }
    return;

  rc_dir_err:
    no_rc_dir = TRUE;
    if (((tmp_dir = getenv("TMPDIR")) == NULL || *tmp_dir == '\0') &&
	((tmp_dir = getenv("TMP")) == NULL || *tmp_dir == '\0') &&
	((tmp_dir = getenv("TEMP")) == NULL || *tmp_dir == '\0'))
	tmp_dir = "/tmp";
#ifdef HAVE_MKDTEMP
    tmp_dir = mkdtemp(Strnew_m_charp(tmp_dir, "/w3m-XXXXXX", NULL)->ptr);
    if (tmp_dir == NULL)
	tmp_dir = rc_dir;
#endif
    create_option_search_table();
    goto open_rc;
}


static char optionpanel_src1[] =
    "<html><head><title>Option Setting Panel</title></head><body>\
<h1 align=center>Option Setting Panel<br>(w3m version %s)</b></h1>\
<form method=post action=\"file:///$LIB/" W3MHELPERPANEL_CMDNAME "\">\
<input type=hidden name=mode value=panel>\
<input type=hidden name=cookie value=\"%s\">\
<input type=submit value=\"%s\">\
</form><br>\
<form method=internal action=option>";

static Str optionpanel_str = NULL;

static Str
to_str(struct param_ptr *p)
{
    switch (p->type) {
    case P_INT:
    case P_COLOR:
    case P_CODE:
	return Sprintf("%d", (int)(*(wc_ces *) p->varptr));
    case P_NZINT:
	return Sprintf("%d", *(int *)p->varptr);
    case P_SHORT:
	return Sprintf("%d", *(short *)p->varptr);
    case P_CHARINT:
	return Sprintf("%d", *(char *)p->varptr);
    case P_CHAR:
	return Sprintf("%c", *(char *)p->varptr);
    case P_STRING:
#if defined(USE_SSL) && defined(USE_SSL_VERIFY)
    case P_SSLPATH:
#endif
	/*  SystemCharset -> InnerCharset */
	return Strnew_charp(conv_from_system(*(char **)p->varptr));
    case P_PIXELS:
    case P_SCALE:
	return Sprintf("%g", *(double *)p->varptr);
    }
    /* not reached */
    return NULL;
}

Buffer *
load_option_panel(void)
{
    Str src;
    struct param_ptr *p;
    struct sel_c *s;
    wc_ces_list *c;
    int x, i;
    Str tmp;
    Buffer *buf;

    if (optionpanel_str == NULL)
	optionpanel_str = Sprintf(optionpanel_src1, w3m_version,
			      html_quote(localCookie()->ptr), _(CMT_HELPER));
#ifdef ENABLE_NLS
    OptionCharset = SystemCharset;	/* FIXME */
#endif
    if (!OptionEncode) {
	optionpanel_str =
	    wc_Str_conv(optionpanel_str, OptionCharset, InnerCharset);
	for (i = 0; sections[i].name != NULL; i++) {
	    sections[i].name =
		wc_conv(_(sections[i].name), OptionCharset,
			InnerCharset)->ptr;
	    for (p = sections[i].params; p->name; p++) {
		p->comment =
		    wc_conv(_(p->comment), OptionCharset,
			    InnerCharset)->ptr;
		if (p->inputtype == PI_SEL_C
			&& p->select != colorstr
			) {
		    for (s = (struct sel_c *)p->select; s->text != NULL; s++) {
			s->text =
			    wc_conv(_(s->text), OptionCharset,
				    InnerCharset)->ptr;
		    }
		}
	    }
	}
	for (s = colorstr; s->text; s++)
	    s->text = wc_conv(_(s->text), OptionCharset,
			      InnerCharset)->ptr;
	OptionEncode = TRUE;
    }
    src = Strdup(optionpanel_str);

    Strcat_charp(src, "<table><tr><td>");
    for (i = 0; sections[i].name != NULL; i++) {
	Strcat_m_charp(src, "<h1>", sections[i].name, "</h1>", NULL);
	p = sections[i].params;
	Strcat_charp(src, "<table width=100% cellpadding=0>");
	while (p->name) {
	    Strcat_m_charp(src, "<tr><td>", p->comment, NULL);
	    Strcat(src, Sprintf("</td><td width=%d>",
				(int)(28 * pixel_per_char)));
	    switch (p->inputtype) {
	    case PI_TEXT:
		Strcat_m_charp(src, "<input type=text name=",
			       p->name,
			       " value=\"",
			       html_quote(to_str(p)->ptr), "\">", NULL);
		break;
	    case PI_ONOFF:
		x = atoi(to_str(p)->ptr);
		Strcat_m_charp(src, "<input type=radio name=",
			       p->name,
			       " value=1",
			       (x ? " checked" : ""),
			       ">YES&nbsp;&nbsp;<input type=radio name=",
			       p->name,
			       " value=0", (x ? "" : " checked"), ">NO", NULL);
		break;
	    case PI_SEL_C:
		tmp = to_str(p);
		Strcat_m_charp(src, "<select name=", p->name, ">", NULL);
		for (s = (struct sel_c *)p->select; s->text != NULL; s++) {
		    Strcat_charp(src, "<option value=");
		    Strcat(src, Sprintf("%s\n", s->cvalue));
		    if ((p->type != P_CHAR && s->value == atoi(tmp->ptr)) ||
			(p->type == P_CHAR && (char)s->value == *(tmp->ptr)))
			Strcat_charp(src, " selected");
		    Strcat_char(src, '>');
		    Strcat_charp(src, s->text);
		}
		Strcat_charp(src, "</select>");
		break;
	    case PI_CODE:
		tmp = to_str(p);
		Strcat_m_charp(src, "<select name=", p->name, ">", NULL);
		for (c = *(wc_ces_list **) p->select; c->desc != NULL; c++) {
		    Strcat_charp(src, "<option value=");
		    Strcat(src, Sprintf("%s\n", c->name));
		    if (c->id == atoi(tmp->ptr))
			Strcat_charp(src, " selected");
		    Strcat_char(src, '>');
		    Strcat_charp(src, c->desc);
		}
		Strcat_charp(src, "</select>");
		break;
	    }
	    Strcat_charp(src, "</td></tr>\n");
	    p++;
	}
	Strcat_charp(src,
		     "<tr><td></td><td><p><input type=submit value=\"OK\"></td></tr>");
	Strcat_charp(src, "</table><hr width=50%>");
    }
    Strcat_charp(src, "</table></form></body></html>");
    buf = loadHTMLString(src);
    if (buf)
	buf->document_charset = OptionCharset;
    return buf;
}

void
panel_set_option(struct parsed_tagarg *arg)
{
    FILE *f = NULL;
    char *p;
    Str s = Strnew(), tmp;

    if (config_file == NULL) {
	disp_message("There's no config file... config not saved", FALSE);
    }
    else {
	f = fopen(config_file, "wt");
	if (f == NULL) {
	    disp_message("Can't write option!", FALSE);
	}
    }
    while (arg) {
	/*  InnerCharset -> SystemCharset */
	if (arg->value) {
	    p = conv_to_system(arg->value);
	    if (set_param(arg->arg, p)) {
		tmp = Sprintf("%s %s\n", arg->arg, p);
		Strcat(tmp, s);
		s = tmp;
	    }
	}
	arg = arg->next;
    }
    if (f) {
	fputs(s->ptr, f);
	fclose(f);
    }
    sync_with_option();
    backBf();
}

char *
rcFile(char *base)
{
    if (base &&
	(base[0] == '/' ||
	 (base[0] == '.'
	  && (base[1] == '/' || (base[1] == '.' && base[2] == '/')))
	 || (base[0] == '~' && base[1] == '/')))
	/* /file, ./file, ../file, ~/file */
	return expandPath(base);
    return expandPath(Strnew_m_charp(rc_dir, "/", base, NULL)->ptr);
}

char *
auxbinFile(char *base)
{
    return expandPath(Strnew_m_charp(w3m_auxbin_dir(), "/", base, NULL)->ptr);
}

#if 0				/* not used */
char *
libFile(char *base)
{
    return expandPath(Strnew_m_charp(w3m_lib_dir(), "/", base, NULL)->ptr);
}
#endif

char *
etcFile(char *base)
{
    return expandPath(Strnew_m_charp(w3m_etc_dir(), "/", base, NULL)->ptr);
}

char *
confFile(char *base)
{
    return expandPath(Strnew_m_charp(w3m_conf_dir(), "/", base, NULL)->ptr);
}


/* siteconf */
/*
 * url "<url>"|/<re-url>/|m@<re-url>@i [exact]
 * substitute_url "<destination-url>"
 * url_charset <charset>
 * no_referer_from on|off
 * no_referer_to on|off
 * user_agent "<string>"
 * 
 * The last match wins.
 */

struct siteconf_rec {
    struct siteconf_rec *next;
    char *url;
    Regex *re_url;
    int url_exact;
    unsigned char mask[(SCONF_N_FIELD + 7) >> 3];

    char *substitute_url;
    char *user_agent;
    wc_ces url_charset;
    int no_referer_from;
    int no_referer_to;
};
#define SCONF_TEST(ent, f) ((ent)->mask[(f)>>3] & (1U<<((f)&7)))
#define SCONF_SET(ent, f) ((ent)->mask[(f)>>3] |= (1U<<((f)&7)))
#define SCONF_CLEAR(ent, f) ((ent)->mask[(f)>>3] &= ~(1U<<((f)&7)))

static struct siteconf_rec *siteconf_head = NULL;
static struct siteconf_rec *newSiteconfRec(void);

static struct siteconf_rec *
newSiteconfRec(void)
{
    struct siteconf_rec *ent;

    ent = New(struct siteconf_rec);
    ent->next = NULL;
    ent->url = NULL;
    ent->re_url = NULL;
    ent->url_exact = FALSE;
    memset(ent->mask, 0, sizeof(ent->mask));

    ent->substitute_url = NULL;
    ent->user_agent = NULL;
    ent->url_charset = 0;
    return ent;
}

static void
loadSiteconf(void)
{
    char *efname;
    FILE *fp;
    Str line;
    struct siteconf_rec *ent = NULL;

    siteconf_head = NULL;
    if (!siteconf_file)
	return;
    if ((efname = expandPath(siteconf_file)) == NULL)
	return;
    fp = fopen(efname, "r");
    if (fp == NULL)
	return;
    while (line = Strfgets(fp), line->length > 0) {
	char *p, *s;

	Strchop(line);
	p = line->ptr;
	SKIP_BLANKS(p);
	if (*p == '#' || *p == '\0')
	    continue;
	s = getWord(&p);

	/* The "url" begins a new record. */
	if (strcmp(s, "url") == 0) {
	    char *url, *opt;
	    struct siteconf_rec *newent;

	    /* First, register the current record. */
	    if (ent) {
		ent->next = siteconf_head;
		siteconf_head = ent;
		ent = NULL;
	    }

	    /* Second, create a new record. */
	    newent = newSiteconfRec();
	    url = getRegexWord((const char **)&p, &newent->re_url);
	    opt = getWord(&p);
	    SKIP_BLANKS(p);
	    if (!newent->re_url) {
		ParsedURL pu;
		if (!url || !*url)
		    continue;
		parseURL2(url, &pu, NULL);
		newent->url = parsedURL2Str(&pu)->ptr;
	    }
	    /* If we have an extra or unknown option, ignore this record
	     * for future extensions. */
	    if (strcmp(opt, "exact") == 0) {
		newent->url_exact = TRUE;
	    }
	    else if (*opt != 0)
		    continue;
	    if (*p)
		continue;
	    ent = newent;
	    continue;
	}

	/* If the current record is broken, skip to the next "url". */
	if (!ent)
	    continue;

	/* Fill the new record. */
	if (strcmp(s, "substitute_url") == 0) {
	    ent->substitute_url = getQWord(&p);
	    SCONF_SET(ent, SCONF_SUBSTITUTE_URL);
	}
	if (strcmp(s, "user_agent") == 0) {
	    ent->user_agent = getQWord(&p);
	    SCONF_SET(ent, SCONF_USER_AGENT);
	}
	else if (strcmp(s, "url_charset") == 0) {
	    char *charset = getWord(&p);
	    ent->url_charset = (charset && *charset) ?
		wc_charset_to_ces(charset) : 0;
	    SCONF_SET(ent, SCONF_URL_CHARSET);
	}
	else if (strcmp(s, "no_referer_from") == 0) {
	    ent->no_referer_from = str_to_bool(getWord(&p), 0);
	    SCONF_SET(ent, SCONF_NO_REFERER_FROM);
	}
	else if (strcmp(s, "no_referer_to") == 0) {
	    ent->no_referer_to = str_to_bool(getWord(&p), 0);
	    SCONF_SET(ent, SCONF_NO_REFERER_TO);
	}
    }
    if (ent) {
	ent->next = siteconf_head;
	siteconf_head = ent;
	ent = NULL;
    }
    fclose(fp);
}

const void *
querySiteconf(const ParsedURL *query_pu, int field)
{
    const struct siteconf_rec *ent;
    Str u;
    char *firstp, *lastp;

    if (field < 0 || field >= SCONF_N_FIELD)
	return NULL;
    if (!query_pu || IS_EMPTY_PARSED_URL(query_pu))
	return NULL;
    u = parsedURL2Str((ParsedURL *)query_pu);
    if (u->length == 0)
	return NULL;

    for (ent = siteconf_head; ent; ent = ent->next) {
	if (!SCONF_TEST(ent, field))
	    continue;
	if (ent->re_url) {
	    if (RegexMatch(ent->re_url, u->ptr, u->length, 1)) {
		MatchedPosition(ent->re_url, &firstp, &lastp);
		if (!ent->url_exact)
		    goto url_found;
		if (firstp != u->ptr || lastp == firstp)
		    continue;
		if (*lastp == 0 || *lastp == '?' || *(lastp - 1) == '?' ||
		    *lastp == '#' || *(lastp - 1) == '#')
		    goto url_found;
	    }
	} else {
	    int matchlen = strmatchlen(ent->url, u->ptr, u->length);
	    if (matchlen == 0 || ent->url[matchlen] != 0)
		continue;
	    firstp = u->ptr;
	    lastp = u->ptr + matchlen;
	    if (*lastp == 0 || *lastp == '?' || *(lastp - 1) == '?' ||
		*lastp == '#' || *(lastp - 1) == '#')
		goto url_found;
	    if (!ent->url_exact && (*lastp == '/' || *(lastp - 1) == '/'))
		goto url_found;
	}
    }
    return NULL;

url_found:
    switch (field) {
    case SCONF_SUBSTITUTE_URL:
	if (ent->substitute_url && *ent->substitute_url) {
	    Str tmp = Strnew_charp_n(u->ptr, firstp - u->ptr);
	    Strcat_charp(tmp, ent->substitute_url);
	    Strcat_charp(tmp, lastp);
	    return tmp->ptr;
	}
	return NULL;
    case SCONF_USER_AGENT:
	if (ent->user_agent && *ent->user_agent) {
	    return ent->user_agent;
	}
	return NULL;
    case SCONF_URL_CHARSET:
	return &ent->url_charset;
    case SCONF_NO_REFERER_FROM:
	return &ent->no_referer_from;
    case SCONF_NO_REFERER_TO:
	return &ent->no_referer_to;
    }
    return NULL;
}
