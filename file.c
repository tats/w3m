/* $Id: file.c,v 1.266 2012/05/22 09:45:56 inu Exp $ */
/* vi: set sw=4 ts=8 ai sm noet : */
#include "fm.h"
#include <sys/types.h>
#include "myctype.h"
#include <signal.h>
#include <setjmp.h>
#if defined(HAVE_WAITPID) || defined(HAVE_WAIT3)
#include <sys/wait.h>
#endif
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
/* foo */

#include "html.h"
#include "parsetagx.h"
#include "local.h"
#include "regex.h"

#ifndef max
#define max(a,b)        ((a) > (b) ? (a) : (b))
#endif				/* not max */
#ifndef min
#define min(a,b)        ((a) > (b) ? (b) : (a))
#endif				/* not min */

#define MAX_INPUT_SIZE 80 /* TODO - max should be screen line length */

static int frame_source = 0;

static char *guess_filename(char *file);
static int _MoveFile(char *path1, char *path2);
static void uncompress_stream(URLFile *uf, char **src);
static FILE *lessopen_stream(char *path);
static Buffer *loadcmdout(char *cmd,
			  Buffer *(*loadproc) (URLFile *, Buffer *),
			  Buffer *defaultbuf);
#ifndef USE_ANSI_COLOR
#define addnewline(a,b,c,d,e,f,g) _addnewline(a,b,c,e,f,g)
#endif
static void addnewline(Buffer *buf, char *line, Lineprop *prop,
		       Linecolor *color, int pos, int width, int nlines);
static void addLink(Buffer *buf, struct parsed_tag *tag);

static JMP_BUF AbortLoading;

static struct table *tables[MAX_TABLE];
static struct table_mode table_mode[MAX_TABLE];

#if defined(USE_M17N) || defined(USE_IMAGE)
static ParsedURL *cur_baseURL = NULL;
#endif
#ifdef USE_M17N
static wc_ces cur_document_charset = 0;
#endif

static Str cur_title;
static Str cur_select;
static Str select_str;
static int select_is_multiple;
static int n_selectitem;
static Str cur_option;
static Str cur_option_value;
static Str cur_option_label;
static int cur_option_selected;
static int cur_status;
#ifdef MENU_SELECT
/* menu based <select>  */
FormSelectOption *select_option;
int max_select = MAX_SELECT;
static int n_select;
static int cur_option_maxwidth;
#endif				/* MENU_SELECT */

static Str cur_textarea;
Str *textarea_str;
static int cur_textarea_size;
static int cur_textarea_rows;
static int cur_textarea_readonly;
static int n_textarea;
static int ignore_nl_textarea;
int max_textarea = MAX_TEXTAREA;

static int http_response_code;

#ifdef USE_M17N
static wc_ces content_charset = 0;
static wc_ces meta_charset = 0;
static char *check_charset(char *p);
static char *check_accept_charset(char *p);
#endif

#define set_prevchar(x,y,n) Strcopy_charp_n((x),(y),(n))
#define set_space_to_prevchar(x) Strcopy_charp_n((x)," ",1)

struct link_stack {
    int cmd;
    short offset;
    short pos;
    struct link_stack *next;
};

static struct link_stack *link_stack = NULL;

#define FORMSTACK_SIZE 10
#define FRAMESTACK_SIZE 10

#ifdef USE_NNTP
#define Str_news_endline(s) ((s)->ptr[0]=='.'&&((s)->ptr[1]=='\n'||(s)->ptr[1]=='\r'||(s)->ptr[1]=='\0'))
#endif				/* USE_NNTP */

#define INITIAL_FORM_SIZE 10
static FormList **forms;
static int *form_stack;
static int form_max = -1;
static int forms_size = 0;
#define cur_form_id ((form_sp >= 0)? form_stack[form_sp] : -1)
static int form_sp = 0;

static clen_t current_content_length;

static int cur_hseq;
#ifdef USE_IMAGE
static int cur_iseq;
#endif

#define MAX_UL_LEVEL	9
#define UL_SYMBOL(x)	(N_GRAPH_SYMBOL + (x))
#define UL_SYMBOL_DISC		UL_SYMBOL(9)
#define UL_SYMBOL_CIRCLE	UL_SYMBOL(10)
#define UL_SYMBOL_SQUARE	UL_SYMBOL(11)
#define IMG_SYMBOL		UL_SYMBOL(12)
#define HR_SYMBOL	26

#ifdef USE_COOKIE
/* This array should be somewhere else */
/* FIXME: gettextize? */
char *violations[COO_EMAX] = {
    "internal error",
    "tail match failed",
    "wrong number of dots",
    "RFC 2109 4.3.2 rule 1",
    "RFC 2109 4.3.2 rule 2.1",
    "RFC 2109 4.3.2 rule 2.2",
    "RFC 2109 4.3.2 rule 3",
    "RFC 2109 4.3.2 rule 4",
    "RFC XXXX 4.3.2 rule 5"
};
#endif

/* *INDENT-OFF* */
static struct compression_decoder {
    int type;
    char *ext;
    char *mime_type;
    int auxbin_p;
    char *cmd;
    char *name;
    char *encoding;
    char *encodings[4];
    int use_d_arg;
} compression_decoders[] = {
    { CMP_COMPRESS, ".gz", "application/x-gzip",
      0, GUNZIP_CMDNAME, GUNZIP_NAME, "gzip", 
      {"gzip", "x-gzip", NULL}, 0 }, 
    { CMP_COMPRESS, ".Z", "application/x-compress",
      0, GUNZIP_CMDNAME, GUNZIP_NAME, "compress",
      {"compress", "x-compress", NULL}, 0 }, 
    { CMP_BZIP2, ".bz2", "application/x-bzip",
      0, BUNZIP2_CMDNAME, BUNZIP2_NAME, "bzip, bzip2",
      {"x-bzip", "bzip", "bzip2", NULL}, 0 }, 
    { CMP_DEFLATE, ".deflate", "application/x-deflate",
      1, INFLATE_CMDNAME, INFLATE_NAME, "deflate",
      {"deflate", "x-deflate", NULL}, 0 }, 
    { CMP_BROTLI, ".br", "application/x-br",
      0, BROTLI_CMDNAME, BROTLI_NAME, "br",
      {"br", "x-br", NULL}, 1 }, 
    { CMP_NOCOMPRESS, NULL, NULL, 0, NULL, NULL, NULL, {NULL}, 0},
};
/* *INDENT-ON* */

#define SAVE_BUF_SIZE 1536

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
    SIGNAL_RETURN;
}

static void
UFhalfclose(URLFile *f)
{
    switch (f->scheme) {
    case SCM_FTP:
	closeFTP();
	break;
#ifdef USE_NNTP
    case SCM_NEWS:
    case SCM_NNTP:
	closeNews();
	break;
#endif
    default:
	UFclose(f);
	break;
    }
}

int
currentLn(Buffer *buf)
{
    if (buf->currentLine)
	/*     return buf->currentLine->real_linenumber + 1;      */
	return buf->currentLine->linenumber + 1;
    else
	return 1;
}

static Buffer *
loadSomething(URLFile *f,
	      Buffer *(*loadproc) (URLFile *, Buffer *), Buffer *defaultbuf)
{
    Buffer *buf;

    if ((buf = loadproc(f, defaultbuf)) == NULL)
	return NULL;

    if (buf->buffername == NULL || buf->buffername[0] == '\0') {
	buf->buffername = checkHeader(buf, "Subject:");
	if (buf->buffername == NULL && buf->filename != NULL)
	    buf->buffername = conv_from_system(lastFileName(buf->filename));
    }
    if (buf->currentURL.scheme == SCM_UNKNOWN)
	buf->currentURL.scheme = f->scheme;
    if (f->scheme == SCM_LOCAL && buf->sourcefile == NULL)
	buf->sourcefile = buf->filename;
    if (loadproc == loadHTMLBuffer
#ifdef USE_IMAGE
	|| loadproc == loadImageBuffer
#endif
       )
	buf->type = "text/html";
    else
	buf->type = "text/plain";
    return buf;
}

int
dir_exist(char *path)
{
    struct stat stbuf;

    if (path == NULL || *path == '\0')
	return 0;
    if (stat(path, &stbuf) == -1)
	return 0;
    return IS_DIRECTORY(stbuf.st_mode);
}

static int
is_dump_text_type(char *type)
{
    struct mailcap *mcap;
    return (type && (mcap = searchExtViewer(type)) &&
	    (mcap->flags & (MAILCAP_HTMLOUTPUT | MAILCAP_COPIOUSOUTPUT)));
}

static int
is_text_type(char *type)
{
    return (type == NULL || type[0] == '\0' ||
	    strncasecmp(type, "text/", 5) == 0 ||
	    (strncasecmp(type, "application/", 12) == 0 &&
		strstr(type, "xhtml") != NULL) ||
	    strncasecmp(type, "message/", sizeof("message/") - 1) == 0);
}

static int
is_plain_text_type(char *type)
{
    return ((type && strcasecmp(type, "text/plain") == 0) ||
	    (is_text_type(type) && !is_dump_text_type(type)));
}

int
is_html_type(char *type)
{
    return (type && (strcasecmp(type, "text/html") == 0 ||
		     strcasecmp(type, "application/xhtml+xml") == 0));
}

static void
check_compression(char *path, URLFile *uf)
{
    int len;
    struct compression_decoder *d;

    if (path == NULL)
	return;

    len = strlen(path);
    uf->compression = CMP_NOCOMPRESS;
    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
	int elen;
	if (d->ext == NULL)
	    continue;
	elen = strlen(d->ext);
	if (len > elen && strcasecmp(&path[len - elen], d->ext) == 0) {
	    uf->compression = d->type;
	    uf->guess_type = d->mime_type;
	    break;
	}
    }
}

static char *
compress_application_type(int compression)
{
    struct compression_decoder *d;

    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
	if (d->type == compression)
	    return d->mime_type;
    }
    return NULL;
}

static char *
uncompressed_file_type(char *path, char **ext)
{
    int len, slen;
    Str fn;
    char *t0;
    struct compression_decoder *d;

    if (path == NULL)
	return NULL;

    slen = 0;
    len = strlen(path);
    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
	if (d->ext == NULL)
	    continue;
	slen = strlen(d->ext);
	if (len > slen && strcasecmp(&path[len - slen], d->ext) == 0)
	    break;
    }
    if (d->type == CMP_NOCOMPRESS)
	return NULL;

    fn = Strnew_charp(path);
    Strshrink(fn, slen);
    if (ext)
	*ext = filename_extension(fn->ptr, 0);
    t0 = guessContentType(fn->ptr);
    if (t0 == NULL)
	t0 = "text/plain";
    return t0;
}

static int
setModtime(char *path, time_t modtime)
{
    struct utimbuf t;
    struct stat st;

    if (stat(path, &st) == 0)
	t.actime = st.st_atime;
    else
	t.actime = time(NULL);
    t.modtime = modtime;
    return utime(path, &t);
}

void
examineFile(char *path, URLFile *uf)
{
    struct stat stbuf;

    uf->guess_type = NULL;
    if (path == NULL || *path == '\0' ||
	stat(path, &stbuf) == -1 || NOT_REGULAR(stbuf.st_mode)) {
	uf->stream = NULL;
	return;
    }
    uf->stream = openIS(path);
    if (!do_download) {
	if (use_lessopen && getenv("LESSOPEN") != NULL) {
	    FILE *fp;
	    uf->guess_type = guessContentType(path);
	    if (uf->guess_type == NULL)
		uf->guess_type = "text/plain";
	    if (is_html_type(uf->guess_type))
		return;
	    if ((fp = lessopen_stream(path))) {
		UFclose(uf);
		uf->stream = newFileStream(fp, (void (*)())pclose);
		uf->guess_type = "text/plain";
		return;
	    }
	}
	check_compression(path, uf);
	if (uf->compression != CMP_NOCOMPRESS) {
	    char *ext = uf->ext;
	    char *t0 = uncompressed_file_type(path, &ext);
	    uf->guess_type = t0;
	    uf->ext = ext;
	    uncompress_stream(uf, NULL);
	    return;
	}
    }
}

#define S_IXANY	(S_IXUSR|S_IXGRP|S_IXOTH)

int
check_command(char *cmd, int auxbin_p)
{
    static char *path = NULL;
    Str dirs;
    char *p, *np;
    Str pathname;
    struct stat st;

    if (path == NULL)
	path = getenv("PATH");
    if (auxbin_p)
	dirs = Strnew_charp(w3m_auxbin_dir());
    else
	dirs = Strnew_charp(path);
    for (p = dirs->ptr; p != NULL; p = np) {
	np = strchr(p, PATH_SEPARATOR);
	if (np)
	    *np++ = '\0';
	pathname = Strnew();
	Strcat_charp(pathname, p);
	Strcat_char(pathname, '/');
	Strcat_charp(pathname, cmd);
	if (stat(pathname->ptr, &st) == 0 && S_ISREG(st.st_mode)
	    && (st.st_mode & S_IXANY) != 0)
	    return 1;
    }
    return 0;
}

char *
acceptableEncoding()
{
    static Str encodings = NULL;
    struct compression_decoder *d;
    TextList *l;
    char *p;

    if (encodings != NULL)
	return encodings->ptr;
    l = newTextList();
    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
	if (check_command(d->cmd, d->auxbin_p)) {
	    pushText(l, d->encoding);
	}
    }
    encodings = Strnew();
    while ((p = popText(l)) != NULL) {
	if (encodings->length)
	    Strcat_charp(encodings, ", ");
	Strcat_charp(encodings, p);
    }
    return encodings->ptr;
}

/* 
 * convert line
 */
#ifdef USE_M17N
Str
convertLine(URLFile *uf, Str line, int mode, wc_ces * charset,
	    wc_ces doc_charset)
#else
Str
convertLine0(URLFile *uf, Str line, int mode)
#endif
{
#ifdef USE_M17N
    line = wc_Str_conv_with_detect(line, charset, doc_charset, InnerCharset);
#endif
    if (mode != RAW_MODE)
	cleanup_line(line, mode);
#ifdef USE_NNTP
    if (uf && uf->scheme == SCM_NEWS)
	Strchop(line);
#endif				/* USE_NNTP */
    return line;
}

int
matchattr(char *p, char *attr, int len, Str *value)
{
    int quoted;
    char *q = NULL;

    if (strncasecmp(p, attr, len) == 0) {
	p += len;
	SKIP_BLANKS(p);
	if (value) {
	    *value = Strnew();
	    if (*p == '=') {
		p++;
		SKIP_BLANKS(p);
		quoted = 0;
		while (!IS_ENDL(*p) && (quoted || *p != ';')) {
		    if (!IS_SPACE(*p))
			q = p;
		    if (*p == '"')
			quoted = (quoted) ? 0 : 1;
		    else
			Strcat_char(*value, *p);
		    p++;
		}
		if (q)
		    Strshrink(*value, p - q - 1);
	    }
	    return 1;
	}
	else {
	    if (IS_ENDT(*p)) {
		return 1;
	    }
	}
    }
    return 0;
}

#ifdef USE_IMAGE
#ifdef USE_XFACE
static char *
xface2xpm(char *xface)
{
    Image image;
    ImageCache *cache;
    FILE *f;
    struct stat st;

    SKIP_BLANKS(xface);
    image.url = xface;
    image.ext = ".xpm";
    image.width = 48;
    image.height = 48;
    image.cache = NULL;
    cache = getImage(&image, NULL, IMG_FLAG_AUTO);
    if (cache->loaded & IMG_FLAG_LOADED && !stat(cache->file, &st))
	return cache->file;
    cache->loaded = IMG_FLAG_ERROR;

    f = popen(Sprintf("%s > %s", shell_quote(auxbinFile(XFACE2XPM)),
		      shell_quote(cache->file))->ptr, "w");
    if (!f)
	return NULL;
    fputs(xface, f);
    pclose(f);
    if (stat(cache->file, &st) || !st.st_size)
	return NULL;
    cache->loaded = IMG_FLAG_LOADED | IMG_FLAG_DONT_REMOVE;
    cache->index = 0;
    return cache->file;
}
#endif
#endif

void
readHeader(URLFile *uf, Buffer *newBuf, int thru, ParsedURL *pu)
{
    char *p, *q;
#ifdef USE_COOKIE
    char *emsg;
#endif
    char c;
    Str lineBuf2 = NULL;
    Str tmp;
    TextList *headerlist;
#ifdef USE_M17N
    wc_ces charset = WC_CES_US_ASCII, mime_charset;
#endif
    char *tmpf;
    FILE *src = NULL;
    Lineprop *propBuffer;

    headerlist = newBuf->document_header = newTextList();
    if (uf->scheme == SCM_HTTP
#ifdef USE_SSL
	|| uf->scheme == SCM_HTTPS
#endif				/* USE_SSL */
	)
	http_response_code = -1;
    else
	http_response_code = 0;

    if (thru && !newBuf->header_source
#ifdef USE_IMAGE
	&& !image_source
#endif
	) {
	tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
	src = fopen(tmpf, "w");
	if (src)
	    newBuf->header_source = tmpf;
    }
    while ((tmp = StrmyUFgets(uf))->length) {
#ifdef USE_NNTP
	if (uf->scheme == SCM_NEWS && tmp->ptr[0] == '.')
	    Strshrinkfirst(tmp, 1);
#endif
	if(w3m_reqlog){
	    FILE *ff;
	    ff = fopen(w3m_reqlog, "a");
            if(ff){
	        Strfputs(tmp, ff);
	        fclose(ff);
            }
	}
	if (src)
	    Strfputs(tmp, src);
	cleanup_line(tmp, HEADER_MODE);
	if (tmp->ptr[0] == '\n' || tmp->ptr[0] == '\r' || tmp->ptr[0] == '\0') {
	    if (!lineBuf2)
		/* there is no header */
		break;
	    /* last header */
	}
	else if (!(w3m_dump & DUMP_HEAD)) {
	    if (lineBuf2) {
		Strcat(lineBuf2, tmp);
	    }
	    else {
		lineBuf2 = tmp;
	    }
	    c = UFgetc(uf);
	    UFundogetc(uf);
	    if (c == ' ' || c == '\t')
		/* header line is continued */
		continue;
	    lineBuf2 = decodeMIME(lineBuf2, &mime_charset);
	    lineBuf2 = convertLine(NULL, lineBuf2, RAW_MODE,
				   mime_charset ? &mime_charset : &charset,
				   mime_charset ? mime_charset
				   : DocumentCharset);
	    /* separated with line and stored */
	    tmp = Strnew_size(lineBuf2->length);
	    for (p = lineBuf2->ptr; *p; p = q) {
		for (q = p; *q && *q != '\r' && *q != '\n'; q++) ;
		lineBuf2 = checkType(Strnew_charp_n(p, q - p), &propBuffer,
				     NULL);
		Strcat(tmp, lineBuf2);
		if (thru)
		    addnewline(newBuf, lineBuf2->ptr, propBuffer, NULL,
			       lineBuf2->length, FOLD_BUFFER_WIDTH, -1);
		for (; *q && (*q == '\r' || *q == '\n'); q++) ;
	    }
#ifdef USE_IMAGE
	    if (thru && activeImage && displayImage) {
		Str src = NULL;
		if (!strncasecmp(tmp->ptr, "X-Image-URL:", 12)) {
		    tmpf = &tmp->ptr[12];
		    SKIP_BLANKS(tmpf);
		    src = Strnew_m_charp("<img src=\"", html_quote(tmpf),
					 "\" alt=\"X-Image-URL\">", NULL);
		}
#ifdef USE_XFACE
		else if (!strncasecmp(tmp->ptr, "X-Face:", 7)) {
		    tmpf = xface2xpm(&tmp->ptr[7]);
		    if (tmpf)
			src = Strnew_m_charp("<img src=\"file:",
					     html_quote(tmpf),
					     "\" alt=\"X-Face\"",
					     " width=48 height=48>", NULL);
		}
#endif
		if (src) {
		    URLFile f;
		    Line *l;
#ifdef USE_M17N
		    wc_ces old_charset = newBuf->document_charset;
#endif
		    init_stream(&f, SCM_LOCAL, newStrStream(src));
		    loadHTMLstream(&f, newBuf, NULL, TRUE);
		    UFclose(&f);
		    for (l = newBuf->lastLine; l && l->real_linenumber;
			 l = l->prev)
			l->real_linenumber = 0;
#ifdef USE_M17N
		    newBuf->document_charset = old_charset;
#endif
		}
	    }
#endif
	    lineBuf2 = tmp;
	}
	else {
	    lineBuf2 = tmp;
	}
	if ((uf->scheme == SCM_HTTP
#ifdef USE_SSL
	     || uf->scheme == SCM_HTTPS
#endif				/* USE_SSL */
	    ) && http_response_code == -1) {
	    p = lineBuf2->ptr;
	    while (*p && !IS_SPACE(*p))
		p++;
	    while (*p && IS_SPACE(*p))
		p++;
	    http_response_code = atoi(p);
	    if (fmInitialized) {
		message(lineBuf2->ptr, 0, 0);
		refresh();
	    }
	}
	if (!strncasecmp(lineBuf2->ptr, "content-transfer-encoding:", 26)) {
	    p = lineBuf2->ptr + 26;
	    while (IS_SPACE(*p))
		p++;
	    if (!strncasecmp(p, "base64", 6))
		uf->encoding = ENC_BASE64;
	    else if (!strncasecmp(p, "quoted-printable", 16))
		uf->encoding = ENC_QUOTE;
	    else if (!strncasecmp(p, "uuencode", 8) ||
		     !strncasecmp(p, "x-uuencode", 10))
		uf->encoding = ENC_UUENCODE;
	    else
		uf->encoding = ENC_7BIT;
	}
	else if (!strncasecmp(lineBuf2->ptr, "content-encoding:", 17)) {
	    struct compression_decoder *d;
	    p = lineBuf2->ptr + 17;
	    while (IS_SPACE(*p))
		p++;
	    uf->compression = CMP_NOCOMPRESS;
	    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
		char **e;
		for (e = d->encodings; *e != NULL; e++) {
		    if (strncasecmp(p, *e, strlen(*e)) == 0) {
			uf->compression = d->type;
			break;
		    }
		}
		if (uf->compression != CMP_NOCOMPRESS)
		    break;
	    }
	    uf->content_encoding = uf->compression;
	}
#ifdef USE_COOKIE
	else if (use_cookie && accept_cookie &&
		 pu && check_cookie_accept_domain(pu->host) &&
		 (!strncasecmp(lineBuf2->ptr, "Set-Cookie:", 11) ||
		  !strncasecmp(lineBuf2->ptr, "Set-Cookie2:", 12))) {
	    Str name = Strnew(), value = Strnew(), domain = NULL, path = NULL,
		comment = NULL, commentURL = NULL, port = NULL, tmp2;
	    int version, quoted, flag = 0;
	    time_t expires = (time_t) - 1;

	    q = NULL;
	    if (lineBuf2->ptr[10] == '2') {
		p = lineBuf2->ptr + 12;
		version = 1;
	    }
	    else {
		p = lineBuf2->ptr + 11;
		version = 0;
	    }
#ifdef DEBUG
	    fprintf(stderr, "Set-Cookie: [%s]\n", p);
#endif				/* DEBUG */
	    SKIP_BLANKS(p);
	    while (*p != '=' && !IS_ENDT(*p))
		Strcat_char(name, *(p++));
	    Strremovetrailingspaces(name);
	    if (*p == '=') {
		p++;
		SKIP_BLANKS(p);
		quoted = 0;
		while (!IS_ENDL(*p) && (quoted || *p != ';')) {
		    if (!IS_SPACE(*p))
			q = p;
		    if (*p == '"')
			quoted = (quoted) ? 0 : 1;
		    Strcat_char(value, *(p++));
		}
		if (q)
		    Strshrink(value, p - q - 1);
	    }
	    while (*p == ';') {
		p++;
		SKIP_BLANKS(p);
		if (matchattr(p, "expires", 7, &tmp2)) {
		    /* version 0 */
		    expires = mymktime(tmp2->ptr);
		}
		else if (matchattr(p, "max-age", 7, &tmp2)) {
		    /* XXX Is there any problem with max-age=0? (RFC 2109 ss. 4.2.1, 4.2.2 */
		    expires = time(NULL) + atol(tmp2->ptr);
		}
		else if (matchattr(p, "domain", 6, &tmp2)) {
		    domain = tmp2;
		}
		else if (matchattr(p, "path", 4, &tmp2)) {
		    path = tmp2;
		}
		else if (matchattr(p, "secure", 6, NULL)) {
		    flag |= COO_SECURE;
		}
		else if (matchattr(p, "comment", 7, &tmp2)) {
		    comment = tmp2;
		}
		else if (matchattr(p, "version", 7, &tmp2)) {
		    version = atoi(tmp2->ptr);
		}
		else if (matchattr(p, "port", 4, &tmp2)) {
		    /* version 1, Set-Cookie2 */
		    port = tmp2;
		}
		else if (matchattr(p, "commentURL", 10, &tmp2)) {
		    /* version 1, Set-Cookie2 */
		    commentURL = tmp2;
		}
		else if (matchattr(p, "discard", 7, NULL)) {
		    /* version 1, Set-Cookie2 */
		    flag |= COO_DISCARD;
		}
		quoted = 0;
		while (!IS_ENDL(*p) && (quoted || *p != ';')) {
		    if (*p == '"')
			quoted = (quoted) ? 0 : 1;
		    p++;
		}
	    }
	    if (pu && name->length > 0) {
		int err;
		if (show_cookie) {
		    if (flag & COO_SECURE)
		        disp_message_nsec("Received a secured cookie", FALSE, 1,
				      TRUE, FALSE);
		    else
		        disp_message_nsec(Sprintf("Received cookie: %s=%s",
					      name->ptr, value->ptr)->ptr,
				      FALSE, 1, TRUE, FALSE);
		}
		err =
		    add_cookie(pu, name, value, expires, domain, path, flag,
			       comment, version, port, commentURL);
		if (err) {
		    char *ans = (accept_bad_cookie == ACCEPT_BAD_COOKIE_ACCEPT)
			? "y" : NULL;
		    if (fmInitialized && (err & COO_OVERRIDE_OK) &&
			accept_bad_cookie == ACCEPT_BAD_COOKIE_ASK) {
			Str msg = Sprintf("Accept bad cookie from %s for %s?",
					  pu->host,
					  ((domain && domain->ptr)
					   ? domain->ptr : "<localdomain>"));
			if (msg->length > COLS - 10)
			    Strshrink(msg, msg->length - (COLS - 10));
			Strcat_charp(msg, " (y/n)");
			ans = inputAnswer(msg->ptr);
		    }
		    if (ans == NULL || TOLOWER(*ans) != 'y' ||
			(err =
			 add_cookie(pu, name, value, expires, domain, path,
				    flag | COO_OVERRIDE, comment, version,
				    port, commentURL))) {
			err = (err & ~COO_OVERRIDE_OK) - 1;
			if (err >= 0 && err < COO_EMAX)
			    emsg = Sprintf("This cookie was rejected "
					   "to prevent security violation. [%s]",
					   violations[err])->ptr;
			else
			    emsg =
				"This cookie was rejected to prevent security violation.";
			record_err_message(emsg);
			if (show_cookie)
			    disp_message_nsec(emsg, FALSE, 1, TRUE, FALSE);
		    }
		    else
			if (show_cookie)
			    disp_message_nsec(Sprintf
					  ("Accepting invalid cookie: %s=%s",
					   name->ptr, value->ptr)->ptr, FALSE,
					  1, TRUE, FALSE);
		}
	    }
	}
#endif				/* USE_COOKIE */
	else if (!strncasecmp(lineBuf2->ptr, "w3m-control:", 12) &&
		 uf->scheme == SCM_LOCAL_CGI) {
	    Str funcname = Strnew();
	    int f;

	    p = lineBuf2->ptr + 12;
	    SKIP_BLANKS(p);
	    while (*p && !IS_SPACE(*p))
		Strcat_char(funcname, *(p++));
	    SKIP_BLANKS(p);
	    f = getFuncList(funcname->ptr);
	    if (f >= 0) {
		tmp = Strnew_charp(p);
		Strchop(tmp);
		pushEvent(f, tmp->ptr);
	    }
	}
	if (headerlist)
	    pushText(headerlist, lineBuf2->ptr);
	Strfree(lineBuf2);
	lineBuf2 = NULL;
    }
    if (thru)
	addnewline(newBuf, "", propBuffer, NULL, 0, -1, -1);
    if (src)
	fclose(src);
}

char *
checkHeader(Buffer *buf, char *field)
{
    int len;
    TextListItem *i;
    char *p;

    if (buf == NULL || field == NULL || buf->document_header == NULL)
	return NULL;
    len = strlen(field);
    for (i = buf->document_header->first; i != NULL; i = i->next) {
	if (!strncasecmp(i->ptr, field, len)) {
	    p = i->ptr + len;
	    return remove_space(p);
	}
    }
    return NULL;
}

char *
checkContentType(Buffer *buf)
{
    char *p;
    Str r;
    p = checkHeader(buf, "Content-Type:");
    if (p == NULL)
	return NULL;
    r = Strnew();
    while (*p && *p != ';' && !IS_SPACE(*p))
	Strcat_char(r, *p++);
#ifdef USE_M17N
    if ((p = strcasestr(p, "charset")) != NULL) {
	p += 7;
	SKIP_BLANKS(p);
	if (*p == '=') {
	    p++;
	    SKIP_BLANKS(p);
	    if (*p == '"')
		p++;
	    content_charset = wc_guess_charset(p, 0);
	}
    }
#endif
    return r->ptr;
}

struct auth_param {
    char *name;
    Str val;
};

struct http_auth {
    int pri;
    char *scheme;
    struct auth_param *param;
    Str (*cred) (struct http_auth * ha, Str uname, Str pw, ParsedURL *pu,
		 HRequest *hr, FormList *request);
};

enum {
    AUTHCHR_NUL,
    AUTHCHR_SEP,
    AUTHCHR_TOKEN,
};

static int
skip_auth_token(char **pp)
{
    char *p;
    int first = AUTHCHR_NUL, typ;

    for (p = *pp ;; ++p) {
	switch (*p) {
	case '\0':
	    goto endoftoken;
	default:
	    if ((unsigned char)*p > 037) {
		typ = AUTHCHR_TOKEN;
		break;
	    }
	    /* thru */
	case '\177':
	case '[':
	case ']':
	case '(':
	case ')':
	case '<':
	case '>':
	case '@':
	case ';':
	case ':':
	case '\\':
	case '"':
	case '/':
	case '?':
	case '=':
	case ' ':
	case '\t':
	case ',':
	    typ = AUTHCHR_SEP;
	    break;
	}

	if (!first)
	    first = typ;
	else if (first != typ)
	    break;
    }
endoftoken:
    *pp = p;
    return first;
}

static Str
extract_auth_val(char **q)
{
    unsigned char *qq = *(unsigned char **)q;
    int quoted = 0;
    Str val = Strnew();

    SKIP_BLANKS(qq);
    if (*qq == '"') {
	quoted = TRUE;
	Strcat_char(val, *qq++);
    }
    while (*qq != '\0') {
	if (quoted && *qq == '"') {
	    Strcat_char(val, *qq++);
	    break;
	}
	if (!quoted) {
	    switch (*qq) {
	    case '[':
	    case ']':
	    case '(':
	    case ')':
	    case '<':
	    case '>':
	    case '@':
	    case ';':
	    case ':':
	    case '\\':
	    case '"':
	    case '/':
	    case '?':
	    case '=':
	    case ' ':
	    case '\t':
		qq++;
	    case ',':
		goto end_token;
	    default:
		if (*qq <= 037 || *qq == 0177) {
		    qq++;
		    goto end_token;
		}
	    }
	}
	else if (quoted && *qq == '\\')
	    Strcat_char(val, *qq++);
	Strcat_char(val, *qq++);
    }
  end_token:
    *q = (char *)qq;
    return val;
}

static Str
qstr_unquote(Str s)
{
    char *p;

    if (s == NULL)
	return NULL;
    p = s->ptr;
    if (*p == '"') {
	Str tmp = Strnew();
	for (p++; *p != '\0'; p++) {
	    if (*p == '\\')
		p++;
	    Strcat_char(tmp, *p);
	}
	if (Strlastchar(tmp) == '"')
	    Strshrink(tmp, 1);
	return tmp;
    }
    else
	return s;
}

static char *
extract_auth_param(char *q, struct auth_param *auth)
{
    struct auth_param *ap;
    char *p;

    for (ap = auth; ap->name != NULL; ap++) {
	ap->val = NULL;
    }

    while (*q != '\0') {
	SKIP_BLANKS(q);
	for (ap = auth; ap->name != NULL; ap++) {
	    size_t len;

	    len = strlen(ap->name);
	    if (strncasecmp(q, ap->name, len) == 0 &&
		(IS_SPACE(q[len]) || q[len] == '=')) {
		p = q + len;
		SKIP_BLANKS(p);
		if (*p != '=')
		    return q;
		q = p + 1;
		ap->val = extract_auth_val(&q);
		break;
	    }
	}
	if (ap->name == NULL) {
	    /* skip unknown param */
	    int token_type;
	    p = q;
	    if ((token_type = skip_auth_token(&q)) == AUTHCHR_TOKEN &&
		(IS_SPACE(*q) || *q == '=')) {
		SKIP_BLANKS(q);
		if (*q != '=')
		    return p;
		q++;
		extract_auth_val(&q);
	    }
	    else
		return p;
	}
	if (*q != '\0') {
	    SKIP_BLANKS(q);
	    if (*q == ',')
		q++;
	    else
		break;
	}
    }
    return q;
}

static Str
get_auth_param(struct auth_param *auth, char *name)
{
    struct auth_param *ap;
    for (ap = auth; ap->name != NULL; ap++) {
	if (strcasecmp(name, ap->name) == 0)
	    return ap->val;
    }
    return NULL;
}

static Str
AuthBasicCred(struct http_auth *ha, Str uname, Str pw, ParsedURL *pu,
	      HRequest *hr, FormList *request)
{
    Str s = Strdup(uname);
    Strcat_char(s, ':');
    Strcat(s, pw);
    char *base64 = base64_encode(s->ptr, s->length);
    if (!base64)
	return Strnew_charp("Basic ");
    else
	return Strnew_m_charp("Basic ", base64, NULL);
}

#ifdef USE_DIGEST_AUTH
#include <openssl/md5.h>

/* RFC2617: 3.2.2 The Authorization Request Header
 * 
 * credentials      = "Digest" digest-response
 * digest-response  = 1#( username | realm | nonce | digest-uri
 *                    | response | [ algorithm ] | [cnonce] |
 *                     [opaque] | [message-qop] |
 *                         [nonce-count]  | [auth-param] )
 *
 * username         = "username" "=" username-value
 * username-value   = quoted-string
 * digest-uri       = "uri" "=" digest-uri-value
 * digest-uri-value = request-uri   ; As specified by HTTP/1.1
 * message-qop      = "qop" "=" qop-value
 * cnonce           = "cnonce" "=" cnonce-value
 * cnonce-value     = nonce-value
 * nonce-count      = "nc" "=" nc-value
 * nc-value         = 8LHEX
 * response         = "response" "=" request-digest
 * request-digest = <"> 32LHEX <">
 * LHEX             =  "0" | "1" | "2" | "3" |
 *                     "4" | "5" | "6" | "7" |
 *                     "8" | "9" | "a" | "b" |
 *                     "c" | "d" | "e" | "f"
 */

static Str
digest_hex(unsigned char *p)
{
    char *h = "0123456789abcdef";
    Str tmp = Strnew_size(MD5_DIGEST_LENGTH * 2 + 1);
    int i;
    for (i = 0; i < MD5_DIGEST_LENGTH; i++, p++) {
	Strcat_char(tmp, h[(*p >> 4) & 0x0f]);
	Strcat_char(tmp, h[*p & 0x0f]);
    }
    return tmp;
}

enum {
    QOP_NONE,
    QOP_AUTH,
    QOP_AUTH_INT,
};

static Str
AuthDigestCred(struct http_auth *ha, Str uname, Str pw, ParsedURL *pu,
	       HRequest *hr, FormList *request)
{
    Str tmp, a1buf, a2buf, rd, s;
    unsigned char md5[MD5_DIGEST_LENGTH + 1];
    Str uri = HTTPrequestURI(pu, hr);
    char nc[] = "00000001";
    FILE *fp;

    Str algorithm = qstr_unquote(get_auth_param(ha->param, "algorithm"));
    Str nonce = qstr_unquote(get_auth_param(ha->param, "nonce"));
    Str cnonce /* = qstr_unquote(get_auth_param(ha->param, "cnonce")) */;
    /* cnonce is what client should generate. */
    Str qop = qstr_unquote(get_auth_param(ha->param, "qop"));

    static union {
	int r[4];
	unsigned char s[sizeof(int) * 4];
    } cnonce_seed;
    int qop_i = QOP_NONE;

    cnonce_seed.r[0] = rand();
    cnonce_seed.r[1] = rand();
    cnonce_seed.r[2] = rand();
    MD5(cnonce_seed.s, sizeof(cnonce_seed.s), md5);
    cnonce = digest_hex(md5);
    cnonce_seed.r[3]++;

    if (qop) {
	char *p;
	size_t i;

	p = qop->ptr;
	SKIP_BLANKS(p);

	for (;;) {
	    if ((i = strcspn(p, " \t,")) > 0) {
		if (i == sizeof("auth-int") - sizeof("") && !strncasecmp(p, "auth-int", i)) {
		    if (qop_i < QOP_AUTH_INT)
			qop_i = QOP_AUTH_INT;
		}
		else if (i == sizeof("auth") - sizeof("") && !strncasecmp(p, "auth", i)) {
		    if (qop_i < QOP_AUTH)
			qop_i = QOP_AUTH;
		}
	    }

	    if (p[i]) {
		p += i + 1;
		SKIP_BLANKS(p);
	    }
	    else
		break;
	}
    }

    /* A1 = unq(username-value) ":" unq(realm-value) ":" passwd */
    tmp = Strnew_m_charp(uname->ptr, ":",
			 qstr_unquote(get_auth_param(ha->param, "realm"))->ptr,
			 ":", pw->ptr, NULL);
    MD5(tmp->ptr, strlen(tmp->ptr), md5);
    a1buf = digest_hex(md5);

    if (algorithm) {
	if (strcasecmp(algorithm->ptr, "MD5-sess") == 0) {
	    /* A1 = H(unq(username-value) ":" unq(realm-value) ":" passwd)
	     *      ":" unq(nonce-value) ":" unq(cnonce-value)
	     */
	    if (nonce == NULL)
		return NULL;
	    tmp = Strnew_m_charp(a1buf->ptr, ":",
				 qstr_unquote(nonce)->ptr,
				 ":", qstr_unquote(cnonce)->ptr, NULL);
	    MD5(tmp->ptr, strlen(tmp->ptr), md5);
	    a1buf = digest_hex(md5);
	}
	else if (strcasecmp(algorithm->ptr, "MD5") == 0)
	    /* ok default */
	    ;
	else
	    /* unknown algorithm */
	    return NULL;
    }

    /* A2 = Method ":" digest-uri-value */
    tmp = Strnew_m_charp(HTTPrequestMethod(hr)->ptr, ":", uri->ptr, NULL);
    if (qop_i == QOP_AUTH_INT) {
	/*  A2 = Method ":" digest-uri-value ":" H(entity-body) */
	if (request && request->body) {
	    if (request->method == FORM_METHOD_POST && request->enctype == FORM_ENCTYPE_MULTIPART) {
		fp = fopen(request->body, "r");
		if (fp != NULL) {
		    Str ebody;
		    ebody = Strfgetall(fp);
		    fclose(fp);
		    MD5(ebody->ptr, strlen(ebody->ptr), md5);
		}
		else {
		    MD5("", 0, md5);
		}
	    }
	    else {
		MD5(request->body, request->length, md5);
	    }
	}
	else {
	    MD5("", 0, md5);
	}
	Strcat_char(tmp, ':');
	Strcat(tmp, digest_hex(md5));
    }
    MD5(tmp->ptr, strlen(tmp->ptr), md5);
    a2buf = digest_hex(md5);

    if (qop_i >= QOP_AUTH) {
	/* request-digest  = <"> < KD ( H(A1),     unq(nonce-value)
	 *                      ":" nc-value
	 *                      ":" unq(cnonce-value)
	 *                      ":" unq(qop-value)
	 *                      ":" H(A2)
	 *                      ) <">
	 */
	if (nonce == NULL)
	    return NULL;
	tmp = Strnew_m_charp(a1buf->ptr, ":", qstr_unquote(nonce)->ptr,
			     ":", nc,
			     ":", qstr_unquote(cnonce)->ptr,
			     ":", qop_i == QOP_AUTH ? "auth" : "auth-int",
			     ":", a2buf->ptr, NULL);
	MD5(tmp->ptr, strlen(tmp->ptr), md5);
	rd = digest_hex(md5);
    }
    else {
	/* compatibility with RFC 2069
	 * request_digest = KD(H(A1),  unq(nonce), H(A2))
	 */
	tmp = Strnew_m_charp(a1buf->ptr, ":",
			     qstr_unquote(get_auth_param(ha->param, "nonce"))->
			     ptr, ":", a2buf->ptr, NULL);
	MD5(tmp->ptr, strlen(tmp->ptr), md5);
	rd = digest_hex(md5);
    }

    /*
     * digest-response  = 1#( username | realm | nonce | digest-uri
     *                          | response | [ algorithm ] | [cnonce] |
     *                          [opaque] | [message-qop] |
     *                          [nonce-count]  | [auth-param] )
     */

    tmp = Strnew_m_charp("Digest username=\"", uname->ptr, "\"", NULL);
    Strcat_m_charp(tmp, ", realm=",
		   get_auth_param(ha->param, "realm")->ptr, NULL);
    Strcat_m_charp(tmp, ", nonce=",
		   get_auth_param(ha->param, "nonce")->ptr, NULL);
    Strcat_m_charp(tmp, ", uri=\"", uri->ptr, "\"", NULL);
    Strcat_m_charp(tmp, ", response=\"", rd->ptr, "\"", NULL);

    if (algorithm)
	Strcat_m_charp(tmp, ", algorithm=",
		       get_auth_param(ha->param, "algorithm")->ptr, NULL);

    if (cnonce)
	Strcat_m_charp(tmp, ", cnonce=\"", cnonce->ptr, "\"", NULL);

    if ((s = get_auth_param(ha->param, "opaque")) != NULL)
	Strcat_m_charp(tmp, ", opaque=", s->ptr, NULL);

    if (qop_i >= QOP_AUTH) {
	Strcat_m_charp(tmp, ", qop=",
		       qop_i == QOP_AUTH ? "auth" : "auth-int",
		       NULL);
	/* XXX how to count? */
	/* Since nonce is unique up to each *-Authenticate and w3m does not re-use *-Authenticate: headers,
	   nonce-count should be always "00000001". */
	Strcat_m_charp(tmp, ", nc=", nc, NULL);
    }

    return tmp;
}
#endif

/* *INDENT-OFF* */
struct auth_param none_auth_param[] = {
    {NULL, NULL}
};

struct auth_param basic_auth_param[] = {
    {"realm", NULL},
    {NULL, NULL}
};

#ifdef USE_DIGEST_AUTH
/* RFC2617: 3.2.1 The WWW-Authenticate Response Header
 * challenge        =  "Digest" digest-challenge
 * 
 * digest-challenge  = 1#( realm | [ domain ] | nonce |
 *                       [ opaque ] |[ stale ] | [ algorithm ] |
 *                        [ qop-options ] | [auth-param] )
 *
 * domain            = "domain" "=" <"> URI ( 1*SP URI ) <">
 * URI               = absoluteURI | abs_path
 * nonce             = "nonce" "=" nonce-value
 * nonce-value       = quoted-string
 * opaque            = "opaque" "=" quoted-string
 * stale             = "stale" "=" ( "true" | "false" )
 * algorithm         = "algorithm" "=" ( "MD5" | "MD5-sess" |
 *                        token )
 * qop-options       = "qop" "=" <"> 1#qop-value <">
 * qop-value         = "auth" | "auth-int" | token
 */
struct auth_param digest_auth_param[] = {
    {"realm", NULL},
    {"domain", NULL},
    {"nonce", NULL},
    {"opaque", NULL},
    {"stale", NULL},
    {"algorithm", NULL},
    {"qop", NULL},
    {NULL, NULL}
};
#endif
/* for RFC2617: HTTP Authentication */
struct http_auth www_auth[] = {
    { 1, "Basic ", basic_auth_param, AuthBasicCred },
#ifdef USE_DIGEST_AUTH
    { 10, "Digest ", digest_auth_param, AuthDigestCred },
#endif
    { 0, NULL, NULL, NULL,}
};
/* *INDENT-ON* */

static struct http_auth *
findAuthentication(struct http_auth *hauth, Buffer *buf, char *auth_field)
{
    struct http_auth *ha;
    int len = strlen(auth_field), slen;
    TextListItem *i;
    char *p0, *p;

    bzero(hauth, sizeof(struct http_auth));
    for (i = buf->document_header->first; i != NULL; i = i->next) {
	if (strncasecmp(i->ptr, auth_field, len) == 0) {
	    for (p = i->ptr + len; p != NULL && *p != '\0';) {
		SKIP_BLANKS(p);
		p0 = p;
		for (ha = &www_auth[0]; ha->scheme != NULL; ha++) {
		    slen = strlen(ha->scheme);
		    if (strncasecmp(p, ha->scheme, slen) == 0) {
			p += slen;
			SKIP_BLANKS(p);
			if (hauth->pri < ha->pri) {
			    *hauth = *ha;
			    p = extract_auth_param(p, hauth->param);
			    break;
			}
			else {
			    /* weak auth */
			    p = extract_auth_param(p, none_auth_param);
			}
		    }
		}
		if (p0 == p) {
		    /* all unknown auth failed */
		    int token_type;
		    if ((token_type = skip_auth_token(&p)) == AUTHCHR_TOKEN && IS_SPACE(*p)) {
			SKIP_BLANKS(p);
			p = extract_auth_param(p, none_auth_param);
		    }
		    else
			break;
		}
	    }
	}
    }
    return hauth->scheme ? hauth : NULL;
}

static void
getAuthCookie(struct http_auth *hauth, char *auth_header,
	      TextList *extra_header, ParsedURL *pu, HRequest *hr,
	      FormList *request,
	      volatile Str *uname, volatile Str *pwd)
{
    Str ss = NULL;
    Str tmp;
    TextListItem *i;
    int a_found;
    int auth_header_len = strlen(auth_header);
    char *realm = NULL;
    int proxy;

    if (hauth)
	realm = qstr_unquote(get_auth_param(hauth->param, "realm"))->ptr;

    if (!realm)
	return;

    a_found = FALSE;
    for (i = extra_header->first; i != NULL; i = i->next) {
	if (!strncasecmp(i->ptr, auth_header, auth_header_len)) {
	    a_found = TRUE;
	    break;
	}
    }
    proxy = !strncasecmp("Proxy-Authorization:", auth_header,
			 auth_header_len);
    if (a_found) {
	/* This means that *-Authenticate: header is received after
	 * Authorization: header is sent to the server. 
	 */
	if (fmInitialized) {
	    message("Wrong username or password", 0, 0);
	    refresh();
	}
	else
	    fprintf(stderr, "Wrong username or password\n");
	sleep(1);
	/* delete Authenticate: header from extra_header */
	delText(extra_header, i);
	invalidate_auth_user_passwd(pu, realm, *uname, *pwd, proxy);
    }
    *uname = NULL;
    *pwd = NULL;

    if (!a_found && find_auth_user_passwd(pu, realm, (Str*)uname, (Str*)pwd, 
					  proxy)) {
	/* found username & password in passwd file */ ;
    }
    else {
	if (QuietMessage)
	    return;
	/* input username and password */
	sleep(2);
	if (fmInitialized) {
	    char *pp;
	    term_raw();
	    /* FIXME: gettextize? */
	    if ((pp = inputStr(Sprintf("Username for %s: ", realm)->ptr,
			       NULL)) == NULL)
		return;
	    *uname = Str_conv_to_system(Strnew_charp(pp));
	    if ((pp = inputLine(Sprintf("Password for %s: ", realm)->ptr, NULL,
				IN_PASSWORD)) == NULL) {
		*uname = NULL;
		return;
	    }
	    *pwd = Str_conv_to_system(Strnew_charp(pp));
	    term_cbreak();
	}
	else {
	    /*
	     * If post file is specified as '-', stdin is closed at this
	     * point.
	     * In this case, w3m cannot read username from stdin.
	     * So exit with error message.
	     * (This is same behavior as lwp-request.)
	     */
	    if (feof(stdin) || ferror(stdin)) {
		/* FIXME: gettextize? */
		fprintf(stderr, "w3m: Authorization required for %s\n",
			realm);
		exit(1);
	    }
	    
	    /* FIXME: gettextize? */
	    printf(proxy ? "Proxy Username for %s: " : "Username for %s: ",
		   realm);
	    fflush(stdout);
	    *uname = Strfgets(stdin);
	    Strchop(*uname);
#ifdef HAVE_GETPASSPHRASE
	    *pwd = Strnew_charp((char *)
				getpassphrase(proxy ? "Proxy Password: " :
					      "Password: "));
#else
#ifndef __MINGW32_VERSION
	    *pwd = Strnew_charp((char *)
				getpass(proxy ? "Proxy Password: " :
					"Password: "));
#else
	    term_raw();
	    *pwd = Strnew_charp((char *)
				inputLine(proxy ? "Proxy Password: " :
					  "Password: ", NULL, IN_PASSWORD));
	    term_cbreak();
#endif /* __MINGW32_VERSION */
#endif
	}
    }
    ss = hauth->cred(hauth, *uname, *pwd, pu, hr, request);
    if (ss) {
	tmp = Strnew_charp(auth_header);
	Strcat_m_charp(tmp, " ", ss->ptr, "\r\n", NULL);
	pushText(extra_header, tmp->ptr);
    }
    else {
	*uname = NULL;
	*pwd = NULL;
    }
    return;
}

static int
same_url_p(ParsedURL *pu1, ParsedURL *pu2)
{
    return (pu1->scheme == pu2->scheme && pu1->port == pu2->port &&
	    (pu1->host ? pu2->host ? !strcasecmp(pu1->host, pu2->host) : 0 : 1)
	    && (pu1->file ? pu2->
		file ? !strcmp(pu1->file, pu2->file) : 0 : 1));
}

static int
checkRedirection(ParsedURL *pu)
{
    static ParsedURL *puv = NULL;
    static int nredir = 0;
    static int nredir_size = 0;
    Str tmp;

    if (pu == NULL) {
	nredir = 0;
	nredir_size = 0;
	puv = NULL;
	return TRUE;
    }
    if (nredir >= FollowRedirection) {
	/* FIXME: gettextize? */
	tmp = Sprintf("Number of redirections exceeded %d at %s",
		      FollowRedirection, parsedURL2Str(pu)->ptr);
	disp_err_message(tmp->ptr, FALSE);
	return FALSE;
    }
    else if (nredir_size > 0 &&
	     (same_url_p(pu, &puv[(nredir - 1) % nredir_size]) ||
	      (!(nredir % 2)
	       && same_url_p(pu, &puv[(nredir / 2) % nredir_size])))) {
	/* FIXME: gettextize? */
	tmp = Sprintf("Redirection loop detected (%s)",
		      parsedURL2Str(pu)->ptr);
	disp_err_message(tmp->ptr, FALSE);
	return FALSE;
    }
    if (!puv) {
	nredir_size = FollowRedirection / 2 + 1;
	puv = New_N(ParsedURL, nredir_size);
	memset(puv, 0, sizeof(ParsedURL) * nredir_size);
    }
    copyParsedURL(&puv[nredir % nredir_size], pu);
    nredir++;
    return TRUE;
}

Str
getLinkNumberStr(int correction)
{
    return Sprintf("[%d]", cur_hseq + correction);
}

/* 
 * loadGeneralFile: load file to buffer
 */
#define DO_EXTERNAL ((Buffer *(*)(URLFile *, Buffer *))doExternal)
Buffer *
loadGeneralFile(char *path, ParsedURL *volatile current, char *referer,
		int flag, FormList *volatile request)
{
    URLFile f, *volatile of = NULL;
    ParsedURL pu;
    Buffer *b = NULL;
    Buffer *(*volatile proc)(URLFile *, Buffer *) = loadBuffer;
    char *volatile tpath;
    char *volatile t = "text/plain", *p, *volatile real_type = NULL;
    Buffer *volatile t_buf = NULL;
    int volatile searchHeader = SearchHeader;
    int volatile searchHeader_through = TRUE;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
    TextList *extra_header = newTextList();
    volatile Str uname = NULL;
    volatile Str pwd = NULL;
    volatile Str realm = NULL;
    int volatile add_auth_cookie_flag;
    unsigned char status = HTST_NORMAL;
    URLOption url_option;
    Str tmp;
    Str volatile page = NULL;
#ifdef USE_GOPHER
    int gopher_download = FALSE;
#endif
#ifdef USE_M17N
    wc_ces charset = WC_CES_US_ASCII;
#endif
    HRequest hr;
    ParsedURL *volatile auth_pu;

    tpath = path;
    prevtrap = NULL;
    add_auth_cookie_flag = 0;

    checkRedirection(NULL);

  load_doc:
    {
	const char *sc_redirect;
	parseURL2(tpath, &pu, current);
	sc_redirect = query_SCONF_SUBSTITUTE_URL(&pu);
	if (sc_redirect && *sc_redirect && checkRedirection(&pu)) {
	    tpath = (char *)sc_redirect;
	    request = NULL;
	    add_auth_cookie_flag = 0;
	    current = New(ParsedURL);
	    *current = pu;
	    status = HTST_NORMAL;
	    goto load_doc;
	}
    }
    TRAP_OFF;
    url_option.referer = referer;
    url_option.flag = flag;
    f = openURL(tpath, &pu, current, &url_option, request, extra_header, of,
		&hr, &status);
    of = NULL;
#ifdef USE_M17N
    content_charset = 0;
#endif
    if (f.stream == NULL) {
	switch (f.scheme) {
	case SCM_LOCAL:
	    {
		struct stat st;
		if (stat(pu.real_file, &st) < 0)
		    return NULL;
		if (S_ISDIR(st.st_mode)) {
		    if (UseExternalDirBuffer) {
			Str cmd = Sprintf("%s?dir=%s#current",
					  DirBufferCommand, pu.file);
			b = loadGeneralFile(cmd->ptr, NULL, NO_REFERER, 0,
					    NULL);
			if (b != NULL && b != NO_BUFFER) {
			    copyParsedURL(&b->currentURL, &pu);
			    b->filename = b->currentURL.real_file;
			}
			return b;
		    }
		    else {
			page = loadLocalDir(pu.real_file);
			t = "local:directory";
#ifdef USE_M17N
			charset = SystemCharset;
#endif
		    }
		}
	    }
	    break;
	case SCM_FTPDIR:
	    page = loadFTPDir(&pu, &charset);
	    t = "ftp:directory";
	    break;
#ifdef USE_NNTP
	case SCM_NEWS_GROUP:
	    page = loadNewsgroup(&pu, &charset);
	    t = "news:group";
	    break;
#endif
	case SCM_UNKNOWN:
#ifdef USE_EXTERNAL_URI_LOADER
	    tmp = searchURIMethods(&pu);
	    if (tmp != NULL) {
		b = loadGeneralFile(tmp->ptr, current, referer, flag, request);
		if (b != NULL && b != NO_BUFFER)
		    copyParsedURL(&b->currentURL, &pu);
		return b;
	    }
#endif
	    /* FIXME: gettextize? */
	    disp_err_message(Sprintf("Unknown URI: %s",
				     parsedURL2Str(&pu)->ptr)->ptr, FALSE);
	    break;
	}
	if (page && page->length > 0)
	    goto page_loaded;
	return NULL;
    }

    if (status == HTST_MISSING) {
	TRAP_OFF;
	UFclose(&f);
	return NULL;
    }

    /* openURL() succeeded */
    if (SETJMP(AbortLoading) != 0) {
	/* transfer interrupted */
	TRAP_OFF;
	if (b)
	    discardBuffer(b);
	UFclose(&f);
	return NULL;
    }

    b = NULL;
    if (f.is_cgi) {
	/* local CGI */
	searchHeader = TRUE;
	searchHeader_through = FALSE;
    }
    if (header_string)
	header_string = NULL;
    TRAP_ON;
    if (pu.scheme == SCM_HTTP ||
#ifdef USE_SSL
	pu.scheme == SCM_HTTPS ||
#endif				/* USE_SSL */
	((
#ifdef USE_GOPHER
	     (pu.scheme == SCM_GOPHER && non_null(GOPHER_proxy)) ||
#endif				/* USE_GOPHER */
	     (pu.scheme == SCM_FTP && non_null(FTP_proxy))
	 ) && !Do_not_use_proxy && !check_no_proxy(pu.host))) {

	if (fmInitialized) {
	    term_cbreak();
	    /* FIXME: gettextize? */
	    message(Sprintf("%s contacted. Waiting for reply...", pu.host)->
		    ptr, 0, 0);
	    refresh();
	}
	if (t_buf == NULL)
	    t_buf = newBuffer(INIT_BUFFER_WIDTH);
#if 0				/* USE_SSL */
	if (IStype(f.stream) == IST_SSL) {
	    Str s = ssl_get_certificate(f.stream, pu.host);
	    if (s == NULL)
		return NULL;
	    else
		t_buf->ssl_certificate = s->ptr;
	}
#endif
	readHeader(&f, t_buf, FALSE, &pu);
	if (((http_response_code >= 301 && http_response_code <= 303)
	     || http_response_code == 307)
	    && (p = checkHeader(t_buf, "Location:")) != NULL
	    && checkRedirection(&pu)) {
	    /* document moved */
	    /* 301: Moved Permanently */
	    /* 302: Found */
	    /* 303: See Other */
	    /* 307: Temporary Redirect (HTTP/1.1) */
	    tpath = url_encode(p, NULL, 0);
	    request = NULL;
	    UFclose(&f);
	    current = New(ParsedURL);
	    copyParsedURL(current, &pu);
	    t_buf = newBuffer(INIT_BUFFER_WIDTH);
	    t_buf->bufferprop |= BP_REDIRECTED;
	    status = HTST_NORMAL;
	    goto load_doc;
	}
	t = checkContentType(t_buf);
	if (t == NULL && pu.file != NULL) {
	    if (!((http_response_code >= 400 && http_response_code <= 407) ||
		  (http_response_code >= 500 && http_response_code <= 505)))
		t = guessContentType(pu.file);
	}
	if (t == NULL)
	    t = "text/plain";
	if (add_auth_cookie_flag && realm && uname && pwd) {
	    /* If authorization is required and passed */
	    add_auth_user_passwd(&pu, qstr_unquote(realm)->ptr, uname, pwd, 
				  0);
	    add_auth_cookie_flag = 0;
	}
	if ((p = checkHeader(t_buf, "WWW-Authenticate:")) != NULL &&
	    http_response_code == 401) {
	    /* Authentication needed */
	    struct http_auth hauth;
	    if (findAuthentication(&hauth, t_buf, "WWW-Authenticate:") != NULL
		&& (realm = get_auth_param(hauth.param, "realm")) != NULL) {
		auth_pu = &pu;
		getAuthCookie(&hauth, "Authorization:", extra_header,
			      auth_pu, &hr, request, &uname, &pwd);
		if (uname == NULL) {
		    /* abort */
		    TRAP_OFF;
		    goto page_loaded;
		}
		UFclose(&f);
		add_auth_cookie_flag = 1;
		status = HTST_NORMAL;
		goto load_doc;
	    }
	}
	if ((p = checkHeader(t_buf, "Proxy-Authenticate:")) != NULL &&
	    http_response_code == 407) {
	    /* Authentication needed */
	    struct http_auth hauth;
	    if (findAuthentication(&hauth, t_buf, "Proxy-Authenticate:")
		!= NULL
		&& (realm = get_auth_param(hauth.param, "realm")) != NULL) {
		auth_pu = schemeToProxy(pu.scheme);
		getAuthCookie(&hauth, "Proxy-Authorization:",
			      extra_header, auth_pu, &hr, request, 
			      &uname, &pwd);
		if (uname == NULL) {
		    /* abort */
		    TRAP_OFF;
		    goto page_loaded;
		}
		UFclose(&f);
		add_auth_cookie_flag = 1;
		status = HTST_NORMAL;
		add_auth_user_passwd(auth_pu, qstr_unquote(realm)->ptr, uname, pwd, 1);
		goto load_doc;
	    }
	}
	/* XXX: RFC2617 3.2.3 Authentication-Info: ? */

	if (status == HTST_CONNECT) {
	    of = &f;
	    goto load_doc;
	}

	f.modtime = mymktime(checkHeader(t_buf, "Last-Modified:"));
    }
#ifdef USE_NNTP
    else if (pu.scheme == SCM_NEWS || pu.scheme == SCM_NNTP) {
	if (t_buf == NULL)
	    t_buf = newBuffer(INIT_BUFFER_WIDTH);
	readHeader(&f, t_buf, TRUE, &pu);
	t = checkContentType(t_buf);
	if (t == NULL)
	    t = "text/plain";
    }
#endif				/* USE_NNTP */
#ifdef USE_GOPHER
    else if (pu.scheme == SCM_GOPHER) {
	p = pu.file;
	while(*p == '/')
	    ++p;
	switch (*p) {
	case '0':
	    t = "text/plain";
	    break;
	case '1':
	case 'm':
	    page = loadGopherDir(&f, &pu, &charset);
	    t = "gopher:directory";
	    TRAP_OFF;
	    goto page_loaded;
	case '7':
	    if(pu.query != NULL) {
		page = loadGopherDir(&f, &pu, &charset);
		t = "gopher:directory";
	    } else {
		page = loadGopherSearch(&f, &pu, &charset);
		t = "gopher:search";
	    }
	    TRAP_OFF;
	    goto page_loaded;
	case 's':
	    t = "audio/basic";
	    break;
	case 'g':
	    t = "image/gif";
	    break;
	case 'h':
	    t = "text/html";
	    break;
	case 'I':
	    t = guessContentType(pu.file);
	    if(strncasecmp(t, "image/", 6) != 0) {
		t = "image/png";
	    }
	    break;
	case '5':
	case '9':
	    gopher_download = TRUE;
	    break;
	}
    }
#endif				/* USE_GOPHER */
    else if (pu.scheme == SCM_FTP) {
	check_compression(path, &f);
	if (f.compression != CMP_NOCOMPRESS) {
	    char *t1 = uncompressed_file_type(pu.file, NULL);
	    real_type = f.guess_type;
#if 0
	    if (t1 && strncasecmp(t1, "application/", 12) == 0) {
		f.compression = CMP_NOCOMPRESS;
		t = real_type;
	    }
	    else
#endif
	    if (t1)
		t = t1;
	    else
		t = real_type;
	}
	else {
	    real_type = guessContentType(pu.file);
	    if (real_type == NULL)
		real_type = "text/plain";
	    t = real_type;
	}
#if 0
	if (!strncasecmp(t, "application/", 12)) {
	    char *tmpf = tmpfname(TMPF_DFL, NULL)->ptr;
	    current_content_length = 0;
	    if (save2tmp(f, tmpf) < 0)
		UFclose(&f);
	    else {
		UFclose(&f);
		TRAP_OFF;
		doFileMove(tmpf, guess_save_name(t_buf, pu.file));
	    }
	    return NO_BUFFER;
	}
#endif
    }
    else if (pu.scheme == SCM_DATA) {
	t = f.guess_type;
    }
    else if (searchHeader) {
	searchHeader = SearchHeader = FALSE;
	if (t_buf == NULL)
	    t_buf = newBuffer(INIT_BUFFER_WIDTH);
	readHeader(&f, t_buf, searchHeader_through, &pu);
	if (f.is_cgi && (p = checkHeader(t_buf, "Location:")) != NULL &&
	    checkRedirection(&pu)) {
	    /* document moved */
	    tpath = url_encode(remove_space(p), NULL, 0);
	    request = NULL;
	    UFclose(&f);
	    add_auth_cookie_flag = 0;
	    current = New(ParsedURL);
	    copyParsedURL(current, &pu);
	    t_buf = newBuffer(INIT_BUFFER_WIDTH);
	    t_buf->bufferprop |= BP_REDIRECTED;
	    status = HTST_NORMAL;
	    goto load_doc;
	}
#ifdef AUTH_DEBUG
	if ((p = checkHeader(t_buf, "WWW-Authenticate:")) != NULL) {
	    /* Authentication needed */
	    struct http_auth hauth;
	    if (findAuthentication(&hauth, t_buf, "WWW-Authenticate:") != NULL
		&& (realm = get_auth_param(hauth.param, "realm")) != NULL) {
		auth_pu = &pu;
		getAuthCookie(&hauth, "Authorization:", extra_header,
			      auth_pu, &hr, request, &uname, &pwd);
		if (uname == NULL) {
		    /* abort */
		    TRAP_OFF;
		    goto page_loaded;
		}
		UFclose(&f);
		add_auth_cookie_flag = 1;
		status = HTST_NORMAL;
		goto load_doc;
	    }
	}
#endif /* defined(AUTH_DEBUG) */
	t = checkContentType(t_buf);
	if (t == NULL)
	    t = "text/plain";
    }
    else if (DefaultType) {
	t = DefaultType;
	DefaultType = NULL;
    }
    else {
	t = guessContentType(pu.file);
	if (t == NULL)
	    t = "text/plain";
	real_type = t;
	if (f.guess_type)
	    t = f.guess_type;
    }

    /* XXX: can we use guess_type to give the type to loadHTMLstream
     *      to support default utf8 encoding for XHTML here? */
    f.guess_type = t;
    
  page_loaded:
    if (page) {
	FILE *src;
#ifdef USE_IMAGE
	if (image_source)
	    return NULL;
#endif
	tmp = tmpfname(TMPF_SRC, ".html");
	src = fopen(tmp->ptr, "w");
	if (src) {
	    Str s;
	    s = wc_Str_conv_strict(page, InnerCharset, charset);
	    Strfputs(s, src);
	    fclose(src);
	}
#ifdef USE_GOPHER
	if (do_download || gopher_download) {
#else
	if (do_download) {
#endif
	    char *file;
	    if (!src)
		return NULL;
	    file = guess_filename(pu.file);
#ifdef USE_GOPHER
	    if (f.scheme == SCM_GOPHER)
		file = Sprintf("%s.html", file)->ptr;
#endif
#ifdef USE_NNTP
	    if (f.scheme == SCM_NEWS_GROUP)
		file = Sprintf("%s.html", file)->ptr;
#endif
	    doFileMove(tmp->ptr, file);
	    return NO_BUFFER;
	}
	b = loadHTMLString(page);
	if (b) {
	    copyParsedURL(&b->currentURL, &pu);
	    b->real_scheme = pu.scheme;
	    b->real_type = t;
	    if (src)
		b->sourcefile = tmp->ptr;
#ifdef USE_M17N
	    b->document_charset = charset;
#endif
	}
	return b;
    }

    if (real_type == NULL)
	real_type = t;
    proc = loadBuffer;

    current_content_length = 0;
    if ((p = checkHeader(t_buf, "Content-Length:")) != NULL)
	current_content_length = strtoclen(p);
#ifdef USE_GOPHER
    if (do_download || gopher_download) {
#else
    if (do_download) {
#endif
	/* download only */
	char *file;
	TRAP_OFF;
	if (DecodeCTE && IStype(f.stream) != IST_ENCODED)
	    f.stream = newEncodedStream(f.stream, f.encoding);
	if (pu.scheme == SCM_LOCAL) {
	    struct stat st;
	    if (PreserveTimestamp && !stat(pu.real_file, &st))
		f.modtime = st.st_mtime;
	    file = conv_from_system(guess_save_name(NULL, pu.real_file));
	}
	else
	    file = guess_save_name(t_buf, pu.file);
	if (doFileSave(f, file) == 0)
	    UFhalfclose(&f);
	else
	    UFclose(&f);
	return NO_BUFFER;
    }

    if ((f.content_encoding != CMP_NOCOMPRESS) && AutoUncompress
	&& !(w3m_dump & DUMP_EXTRA)) {
	uncompress_stream(&f, &pu.real_file);
    }
    else if (f.compression != CMP_NOCOMPRESS) {
	if (!(w3m_dump & DUMP_SOURCE) &&
	    (w3m_dump & ~DUMP_FRAME || is_text_type(t)
	     || searchExtViewer(t))) {
	    if (t_buf == NULL)
		t_buf = newBuffer(INIT_BUFFER_WIDTH);
	    uncompress_stream(&f, &t_buf->sourcefile);
	    uncompressed_file_type(pu.file, &f.ext);
	}
	else {
	    t = compress_application_type(f.compression);
	    f.compression = CMP_NOCOMPRESS;
	}
    }
#ifdef USE_IMAGE
    if (image_source) {
	Buffer *b = NULL;
	if (IStype(f.stream) != IST_ENCODED)
	    f.stream = newEncodedStream(f.stream, f.encoding);
	if (save2tmp(f, image_source) == 0) {
	    b = newBuffer(INIT_BUFFER_WIDTH);
	    b->sourcefile = image_source;
	    b->real_type = t;
	}
	UFclose(&f);
	TRAP_OFF;
	return b;
    }
#endif

    if (is_html_type(t))
	proc = loadHTMLBuffer;
    else if (is_plain_text_type(t))
	proc = loadBuffer;
#ifdef USE_IMAGE
    else if (activeImage && displayImage && !useExtImageViewer &&
	     !(w3m_dump & ~DUMP_FRAME) && !strncasecmp(t, "image/", 6))
	proc = loadImageBuffer;
#endif
    else if (w3m_backend) ;
    else if (!(w3m_dump & ~DUMP_FRAME) || is_dump_text_type(t)) {
	if (!do_download && 
#ifdef USE_GOPHER
		!gopher_download &&
#endif
		searchExtViewer(t) != NULL) {
	    proc = DO_EXTERNAL;
	}
	else {
	    TRAP_OFF;
	    if (pu.scheme == SCM_LOCAL) {
		UFclose(&f);
		_doFileCopy(pu.real_file,
			    conv_from_system(guess_save_name
					     (NULL, pu.real_file)), TRUE);
	    }
	    else {
		if (DecodeCTE && IStype(f.stream) != IST_ENCODED)
		    f.stream = newEncodedStream(f.stream, f.encoding);
		if (doFileSave(f, guess_save_name(t_buf, pu.file)) == 0)
		    UFhalfclose(&f);
		else
		    UFclose(&f);
	    }
	    return NO_BUFFER;
	}
    }
    else if (w3m_dump & DUMP_FRAME)
	return NULL;

    if (t_buf == NULL)
	t_buf = newBuffer(INIT_BUFFER_WIDTH);
    copyParsedURL(&t_buf->currentURL, &pu);
    t_buf->filename = pu.real_file ? pu.real_file :
	pu.file ? conv_to_system(pu.file) : NULL;
    if (flag & RG_FRAME) {
	t_buf->bufferprop |= BP_FRAME;
    }
#ifdef USE_SSL
    t_buf->ssl_certificate = f.ssl_certificate;
#endif
    frame_source = flag & RG_FRAME_SRC;
    if (proc == DO_EXTERNAL) {
	b = doExternal(f, t, t_buf);
    } else {
	b = loadSomething(&f, proc, t_buf);
    }
    UFclose(&f);
    frame_source = 0;
    if (b && b != NO_BUFFER) {
	b->real_scheme = f.scheme;
	b->real_type = real_type;
	if (w3m_backend)
	    b->type = allocStr(t, -1);
	if (pu.label) {
	    if (proc == loadHTMLBuffer) {
		Anchor *a;
		a = searchURLLabel(b, pu.label);
		if (a != NULL) {
		    gotoLine(b, a->start.line);
		    if (label_topline)
			b->topLine = lineSkip(b, b->topLine,
					      b->currentLine->linenumber
					      - b->topLine->linenumber, FALSE);
		    b->pos = a->start.pos;
		    arrangeCursor(b);
		}
	    }
	    else {		/* plain text */
		int l = atoi(pu.label);
		gotoRealLine(b, l);
		b->pos = 0;
		arrangeCursor(b);
	    }
	}
    }
    if (header_string)
	header_string = NULL;
#ifdef USE_NNTP
    if (b && b != NO_BUFFER && (f.scheme == SCM_NNTP || f.scheme == SCM_NEWS))
	reAnchorNewsheader(b);
#endif
    if (b && b != NO_BUFFER)
	preFormUpdateBuffer(b);
    TRAP_OFF;
    return b;
}

#define TAG_IS(s,tag,len)\
  (strncasecmp(s,tag,len)==0&&(s[len] == '>' || IS_SPACE((int)s[len])))

static char *
has_hidden_link(struct readbuffer *obuf, int cmd)
{
    Str line = obuf->line;
    struct link_stack *p;

    if (Strlastchar(line) != '>')
	return NULL;

    for (p = link_stack; p; p = p->next)
	if (p->cmd == cmd)
	    break;
    if (!p)
	return NULL;

    if (obuf->pos == p->pos)
	return line->ptr + p->offset;

    return NULL;
}

static void
push_link(int cmd, int offset, int pos)
{
    struct link_stack *p;
    p = New(struct link_stack);
    p->cmd = cmd;
    p->offset = (short)offset;
    if (p->offset < 0)
	p->offset = 0;
    p->pos = (short)pos;
    if (p->pos < 0)
	p->pos = 0;
    p->next = link_stack;
    link_stack = p;
}

static int
is_period_char(unsigned char *ch)
{
    switch (*ch) {
    case ',':
    case '.':
    case ':':
    case ';':
    case '?':
    case '!':
    case ')':
    case ']':
    case '}':
    case '>':
	return 1;
    default:
	return 0;
    }
}

static int
is_beginning_char(unsigned char *ch)
{
    switch (*ch) {
    case '(':
    case '[':
    case '{':
    case '`':
    case '<':
	return 1;
    default:
	return 0;
    }
}

static int
is_word_char(unsigned char *ch)
{
    Lineprop ctype = get_mctype(ch);

#ifdef USE_M17N
    if (ctype & (PC_CTRL | PC_KANJI | PC_UNKNOWN))
	return 0;
    if (ctype & (PC_WCHAR1 | PC_WCHAR2))
	return 1;
#else
    if (ctype == PC_CTRL)
	return 0;
#endif

    if (IS_ALNUM(*ch))
	return 1;

    switch (*ch) {
    case ',':
    case '.':
    case ':':
    case '\"':			/* " */
    case '\'':
    case '$':
    case '%':
    case '*':
    case '+':
    case '-':
    case '@':
    case '~':
    case '_':
	return 1;
    }
#ifdef USE_M17N
    if (*ch == NBSP_CODE)
	return 1;
#else
    if (*ch == TIMES_CODE || *ch == DIVIDE_CODE || *ch == ANSP_CODE)
	return 0;
    if (*ch >= AGRAVE_CODE || *ch == NBSP_CODE)
	return 1;
#endif
    return 0;
}

#ifdef USE_M17N
static int
is_combining_char(unsigned char *ch)
{
    Lineprop ctype = get_mctype(ch);

    if (ctype & PC_WCHAR2)
	return 1;
    return 0;
}
#endif

int
is_boundary(unsigned char *ch1, unsigned char *ch2)
{
    if (!*ch1 || !*ch2)
	return 1;

    if (*ch1 == ' ' && *ch2 == ' ')
	return 0;

    if (*ch1 != ' ' && is_period_char(ch2))
	return 0;

    if (*ch2 != ' ' && is_beginning_char(ch1))
	return 0;

#ifdef USE_M17N
    if (is_combining_char(ch2))
	return 0;
#endif
    if (is_word_char(ch1) && is_word_char(ch2))
	return 0;

    return 1;
}


static void
set_breakpoint(struct readbuffer *obuf, int tag_length)
{
    obuf->bp.len = obuf->line->length;
    obuf->bp.pos = obuf->pos;
    obuf->bp.tlen = tag_length;
    obuf->bp.flag = obuf->flag;
#ifdef FORMAT_NICE
    obuf->bp.flag &= ~RB_FILL;
#endif				/* FORMAT_NICE */
    obuf->bp.top_margin = obuf->top_margin;
    obuf->bp.bottom_margin = obuf->bottom_margin;

    if (!obuf->bp.init_flag)
	return;

    bcopy((void *)&obuf->anchor, (void *)&obuf->bp.anchor,
	  sizeof(obuf->anchor));
    obuf->bp.img_alt = obuf->img_alt;
    obuf->bp.input_alt = obuf->input_alt;
    obuf->bp.in_bold = obuf->in_bold;
    obuf->bp.in_italic = obuf->in_italic;
    obuf->bp.in_under = obuf->in_under;
    obuf->bp.in_strike = obuf->in_strike;
    obuf->bp.in_ins = obuf->in_ins;
    obuf->bp.nobr_level = obuf->nobr_level;
    obuf->bp.prev_ctype = obuf->prev_ctype;
    obuf->bp.init_flag = 0;
}

static void
back_to_breakpoint(struct readbuffer *obuf)
{
    obuf->flag = obuf->bp.flag;
    bcopy((void *)&obuf->bp.anchor, (void *)&obuf->anchor,
	  sizeof(obuf->anchor));
    obuf->img_alt = obuf->bp.img_alt;
    obuf->input_alt = obuf->bp.input_alt;
    obuf->in_bold = obuf->bp.in_bold;
    obuf->in_italic = obuf->bp.in_italic;
    obuf->in_under = obuf->bp.in_under;
    obuf->in_strike = obuf->bp.in_strike;
    obuf->in_ins = obuf->bp.in_ins;
    obuf->prev_ctype = obuf->bp.prev_ctype;
    obuf->pos = obuf->bp.pos;
    obuf->top_margin = obuf->bp.top_margin;
    obuf->bottom_margin = obuf->bp.bottom_margin;
    if (obuf->flag & RB_NOBR)
	obuf->nobr_level = obuf->bp.nobr_level;
}

static void
append_tags(struct readbuffer *obuf)
{
    int i;
    int len = obuf->line->length;
    int set_bp = 0;

    for (i = 0; i < obuf->tag_sp; i++) {
	switch (obuf->tag_stack[i]->cmd) {
	case HTML_A:
	case HTML_IMG_ALT:
	case HTML_B:
	case HTML_U:
	case HTML_I:
	case HTML_S:
	    push_link(obuf->tag_stack[i]->cmd, obuf->line->length, obuf->pos);
	    break;
	}
	Strcat_charp(obuf->line, obuf->tag_stack[i]->cmdname);
	switch (obuf->tag_stack[i]->cmd) {
	case HTML_NOBR:
	    if (obuf->nobr_level > 1)
		break;
	case HTML_WBR:
	    set_bp = 1;
	    break;
	}
    }
    obuf->tag_sp = 0;
    if (set_bp)
	set_breakpoint(obuf, obuf->line->length - len);
}

static void
push_tag(struct readbuffer *obuf, char *cmdname, int cmd)
{
    obuf->tag_stack[obuf->tag_sp] = New(struct cmdtable);
    obuf->tag_stack[obuf->tag_sp]->cmdname = allocStr(cmdname, -1);
    obuf->tag_stack[obuf->tag_sp]->cmd = cmd;
    obuf->tag_sp++;
    if (obuf->tag_sp >= TAG_STACK_SIZE || obuf->flag & (RB_SPECIAL & ~RB_NOBR))
	append_tags(obuf);
}

static void
push_nchars(struct readbuffer *obuf, int width,
	    char *str, int len, Lineprop mode)
{
    append_tags(obuf);
    Strcat_charp_n(obuf->line, str, len);
    obuf->pos += width;
    if (width > 0) {
	set_prevchar(obuf->prevchar, str, len);
	obuf->prev_ctype = mode;
    }
    obuf->flag |= RB_NFLUSHED;
}

#define push_charp(obuf, width, str, mode)\
push_nchars(obuf, width, str, strlen(str), mode)

#define push_str(obuf, width, str, mode)\
push_nchars(obuf, width, str->ptr, str->length, mode)

static void
check_breakpoint(struct readbuffer *obuf, int pre_mode, char *ch)
{
    int tlen, len = obuf->line->length;

    append_tags(obuf);
    if (pre_mode)
	return;
    tlen = obuf->line->length - len;
    if (tlen > 0
	|| is_boundary((unsigned char *)obuf->prevchar->ptr,
		       (unsigned char *)ch))
	set_breakpoint(obuf, tlen);
}

static void
push_char(struct readbuffer *obuf, int pre_mode, char ch)
{
    check_breakpoint(obuf, pre_mode, &ch);
    Strcat_char(obuf->line, ch);
    obuf->pos++;
    set_prevchar(obuf->prevchar, &ch, 1);
    if (ch != ' ')
	obuf->prev_ctype = PC_ASCII;
    obuf->flag |= RB_NFLUSHED;
}

#define PUSH(c) push_char(obuf, obuf->flag & RB_SPECIAL, c)

static void
push_spaces(struct readbuffer *obuf, int pre_mode, int width)
{
    int i;

    if (width <= 0)
	return;
    check_breakpoint(obuf, pre_mode, " ");
    for (i = 0; i < width; i++)
	Strcat_char(obuf->line, ' ');
    obuf->pos += width;
    set_space_to_prevchar(obuf->prevchar);
    obuf->flag |= RB_NFLUSHED;
}

static void
proc_mchar(struct readbuffer *obuf, int pre_mode,
	   int width, char **str, Lineprop mode)
{
    check_breakpoint(obuf, pre_mode, *str);
    obuf->pos += width;
    Strcat_charp_n(obuf->line, *str, get_mclen(*str));
    if (width > 0) {
	set_prevchar(obuf->prevchar, *str, 1);
	if (**str != ' ')
	    obuf->prev_ctype = mode;
    }
    (*str) += get_mclen(*str);
    obuf->flag |= RB_NFLUSHED;
}

void
push_render_image(Str str, int width, int limit,
		  struct html_feed_environ *h_env)
{
    struct readbuffer *obuf = h_env->obuf;
    int indent = h_env->envs[h_env->envc].indent;

    push_spaces(obuf, 1, (limit - width) / 2);
    push_str(obuf, width, str, PC_ASCII);
    push_spaces(obuf, 1, (limit - width + 1) / 2);
    if (width > 0)
	flushline(h_env, obuf, indent, 0, h_env->limit);
}

static int
sloppy_parse_line(char **str)
{
    if (**str == '<') {
	while (**str && **str != '>')
	    (*str)++;
	if (**str == '>')
	    (*str)++;
	return 1;
    }
    else {
	while (**str && **str != '<')
	    (*str)++;
	return 0;
    }
}

static void
passthrough(struct readbuffer *obuf, char *str, int back)
{
    int cmd;
    Str tok = Strnew();
    char *str_bak;

    if (back) {
	Str str_save = Strnew_charp(str);
	Strshrink(obuf->line, obuf->line->ptr + obuf->line->length - str);
	str = str_save->ptr;
    }
    while (*str) {
	str_bak = str;
	if (sloppy_parse_line(&str)) {
	    char *q = str_bak;
	    cmd = gethtmlcmd(&q);
	    if (back) {
		struct link_stack *p;
		for (p = link_stack; p; p = p->next) {
		    if (p->cmd == cmd) {
			link_stack = p->next;
			break;
		    }
		}
		back = 0;
	    }
	    else {
		Strcat_charp_n(tok, str_bak, str - str_bak);
		push_tag(obuf, tok->ptr, cmd);
		Strclear(tok);
	    }
	}
	else {
	    push_nchars(obuf, 0, str_bak, str - str_bak, obuf->prev_ctype);
	}
    }
}

#if 0
int
is_blank_line(char *line, int indent)
{
    int i, is_blank = 0;

    for (i = 0; i < indent; i++) {
	if (line[i] == '\0') {
	    is_blank = 1;
	}
	else if (line[i] != ' ') {
	    break;
	}
    }
    if (i == indent && line[i] == '\0')
	is_blank = 1;
    return is_blank;
}
#endif

void
fillline(struct readbuffer *obuf, int indent)
{
    push_spaces(obuf, 1, indent - obuf->pos);
    obuf->flag &= ~RB_NFLUSHED;
}

void
flushline(struct html_feed_environ *h_env, struct readbuffer *obuf, int indent,
	  int force, int width)
{
    TextLineList *buf = h_env->buf;
    FILE *f = h_env->f;
    Str line = obuf->line, pass = NULL;
    char *hidden_anchor = NULL, *hidden_img = NULL, *hidden_bold = NULL,
	*hidden_under = NULL, *hidden_italic = NULL, *hidden_strike = NULL,
	*hidden_ins = NULL, *hidden_input = NULL, *hidden = NULL;

#ifdef DEBUG
    if (w3m_debug) {
	FILE *df = fopen("zzzproc1", "a");
	fprintf(df, "flushline(%s,%d,%d,%d)\n", obuf->line->ptr, indent, force,
		width);
	if (buf) {
	    TextLineListItem *p;
	    for (p = buf->first; p; p = p->next) {
		fprintf(df, "buf=\"%s\"\n", p->ptr->line->ptr);
	    }
	}
	fclose(df);
    }
#endif

    if (!(obuf->flag & (RB_SPECIAL & ~RB_NOBR)) && Strlastchar(line) == ' ') {
	Strshrink(line, 1);
	obuf->pos--;
    }

    append_tags(obuf);

    if (obuf->anchor.url)
	hidden = hidden_anchor = has_hidden_link(obuf, HTML_A);
    if (obuf->img_alt) {
	if ((hidden_img = has_hidden_link(obuf, HTML_IMG_ALT)) != NULL) {
	    if (!hidden || hidden_img < hidden)
		hidden = hidden_img;
	}
    }
    if (obuf->input_alt.in) {
	if ((hidden_input = has_hidden_link(obuf, HTML_INPUT_ALT)) != NULL) {
	    if (!hidden || hidden_input < hidden)
		hidden = hidden_input;
	}
    }
    if (obuf->in_bold) {
	if ((hidden_bold = has_hidden_link(obuf, HTML_B)) != NULL) {
	    if (!hidden || hidden_bold < hidden)
		hidden = hidden_bold;
	}
    }
    if (obuf->in_italic) {
	if ((hidden_italic = has_hidden_link(obuf, HTML_I)) != NULL) {
	    if (!hidden || hidden_italic < hidden)
		hidden = hidden_italic;
	}
    }
    if (obuf->in_under) {
	if ((hidden_under = has_hidden_link(obuf, HTML_U)) != NULL) {
	    if (!hidden || hidden_under < hidden)
		hidden = hidden_under;
	}
    }
    if (obuf->in_strike) {
	if ((hidden_strike = has_hidden_link(obuf, HTML_S)) != NULL) {
	    if (!hidden || hidden_strike < hidden)
		hidden = hidden_strike;
	}
    }
    if (obuf->in_ins) {
	if ((hidden_ins = has_hidden_link(obuf, HTML_INS)) != NULL) {
	    if (!hidden || hidden_ins < hidden)
		hidden = hidden_ins;
	}
    }
    if (hidden) {
	pass = Strnew_charp(hidden);
	Strshrink(line, line->ptr + line->length - hidden);
    }

    if (!(obuf->flag & (RB_SPECIAL & ~RB_NOBR)) && obuf->pos > width) {
	char *tp = &line->ptr[obuf->bp.len - obuf->bp.tlen];
	char *ep = &line->ptr[line->length];

	if (obuf->bp.pos == obuf->pos && tp <= ep &&
	    tp > line->ptr && tp[-1] == ' ') {
	    bcopy(tp, tp - 1, ep - tp + 1);
	    line->length--;
	    obuf->pos--;
	}
    }

    if (obuf->anchor.url && !hidden_anchor)
	Strcat_charp(line, "</a>");
    if (obuf->img_alt && !hidden_img)
	Strcat_charp(line, "</img_alt>");
    if (obuf->input_alt.in && !hidden_input)
	Strcat_charp(line, "</input_alt>");
    if (obuf->in_bold && !hidden_bold)
	Strcat_charp(line, "</b>");
    if (obuf->in_italic && !hidden_italic)
	Strcat_charp(line, "</i>");
    if (obuf->in_under && !hidden_under)
	Strcat_charp(line, "</u>");
    if (obuf->in_strike && !hidden_strike)
	Strcat_charp(line, "</s>");
    if (obuf->in_ins && !hidden_ins)
	Strcat_charp(line, "</ins>");

    if (obuf->top_margin > 0) {
	int i;
	struct html_feed_environ h;
	struct readbuffer o;
	struct environment e[1];

	init_henv(&h, &o, e, 1, NULL, width, indent);
	o.line = Strnew_size(width + 20);
	o.pos = obuf->pos;
	o.flag = obuf->flag;
	o.top_margin = -1;
	o.bottom_margin = -1;
	Strcat_charp(o.line, "<pre_int>");
	for (i = 0; i < o.pos; i++)
	    Strcat_char(o.line, ' ');
	Strcat_charp(o.line, "</pre_int>");
	for (i = 0; i < obuf->top_margin; i++)
	    flushline(h_env, &o, indent, force, width);
    }

    if (force == 1 || obuf->flag & RB_NFLUSHED) {
	TextLine *lbuf = newTextLine(line, obuf->pos);
	if (RB_GET_ALIGN(obuf) == RB_CENTER) {
	    align(lbuf, width, ALIGN_CENTER);
	}
	else if (RB_GET_ALIGN(obuf) == RB_RIGHT) {
	    align(lbuf, width, ALIGN_RIGHT);
	}
	else if (RB_GET_ALIGN(obuf) == RB_LEFT && obuf->flag & RB_INTABLE) {
	    align(lbuf, width, ALIGN_LEFT);
	}
#ifdef FORMAT_NICE
	else if (obuf->flag & RB_FILL) {
	    char *p;
	    int rest, rrest;
	    int nspace, d, i;

	    rest = width - get_Str_strwidth(line);
	    if (rest > 1) {
		nspace = 0;
		for (p = line->ptr + indent; *p; p++) {
		    if (*p == ' ')
			nspace++;
		}
		if (nspace > 0) {
		    int indent_here = 0;
		    d = rest / nspace;
		    p = line->ptr;
		    while (IS_SPACE(*p)) {
			p++;
			indent_here++;
		    }
		    rrest = rest - d * nspace;
		    line = Strnew_size(width + 1);
		    for (i = 0; i < indent_here; i++)
			Strcat_char(line, ' ');
		    for (; *p; p++) {
			Strcat_char(line, *p);
			if (*p == ' ') {
			    for (i = 0; i < d; i++)
				Strcat_char(line, ' ');
			    if (rrest > 0) {
				Strcat_char(line, ' ');
				rrest--;
			    }
			}
		    }
		    lbuf = newTextLine(line, width);
		}
	    }
	}
#endif				/* FORMAT_NICE */
#ifdef TABLE_DEBUG
	if (w3m_debug) {
	    FILE *f = fopen("zzzproc1", "a");
	    fprintf(f, "pos=%d,%d, maxlimit=%d\n",
		    visible_length(lbuf->line->ptr), lbuf->pos,
		    h_env->maxlimit);
	    fclose(f);
	}
#endif
	if (lbuf->pos > h_env->maxlimit)
	    h_env->maxlimit = lbuf->pos;
	if (buf)
	    pushTextLine(buf, lbuf);
	else if (f) {
	    Strfputs(Str_conv_to_halfdump(lbuf->line), f);
	    fputc('\n', f);
	}
	if (obuf->flag & RB_SPECIAL || obuf->flag & RB_NFLUSHED)
	    h_env->blank_lines = 0;
	else
	    h_env->blank_lines++;
    }
    else {
	char *p = line->ptr, *q;
	Str tmp = Strnew(), tmp2 = Strnew();

#define APPEND(str) \
	if (buf) \
	    appendTextLine(buf,(str),0); \
	else if (f) \
	    Strfputs((str),f)

	while (*p) {
	    q = p;
	    if (sloppy_parse_line(&p)) {
		Strcat_charp_n(tmp, q, p - q);
		if (force == 2) {
		    APPEND(tmp);
		}
		else
		    Strcat(tmp2, tmp);
		Strclear(tmp);
	    }
	}
	if (force == 2) {
	    if (pass) {
		APPEND(pass);
	    }
	    pass = NULL;
	}
	else {
	    if (pass)
		Strcat(tmp2, pass);
	    pass = tmp2;
	}
    }

    if (obuf->bottom_margin > 0) {
	int i;
	struct html_feed_environ h;
	struct readbuffer o;
	struct environment e[1];

	init_henv(&h, &o, e, 1, NULL, width, indent);
	o.line = Strnew_size(width + 20);
	o.pos = obuf->pos;
	o.flag = obuf->flag;
	o.top_margin = -1;
	o.bottom_margin = -1;
	Strcat_charp(o.line, "<pre_int>");
	for (i = 0; i < o.pos; i++)
	    Strcat_char(o.line, ' ');
	Strcat_charp(o.line, "</pre_int>");
	for (i = 0; i < obuf->bottom_margin; i++)
	    flushline(h_env, &o, indent, force, width);
    }
    if (obuf->top_margin < 0 || obuf->bottom_margin < 0)
	return;

    obuf->line = Strnew_size(256);
    obuf->pos = 0;
    obuf->top_margin = 0;
    obuf->bottom_margin = 0;
    set_space_to_prevchar(obuf->prevchar);
    obuf->bp.init_flag = 1;
    obuf->flag &= ~RB_NFLUSHED;
    set_breakpoint(obuf, 0);
    obuf->prev_ctype = PC_ASCII;
    link_stack = NULL;
    fillline(obuf, indent);
    if (pass)
	passthrough(obuf, pass->ptr, 0);
    if (!hidden_anchor && obuf->anchor.url) {
	Str tmp;
	if (obuf->anchor.hseq > 0)
	    obuf->anchor.hseq = -obuf->anchor.hseq;
	tmp = Sprintf("<A HSEQ=\"%d\" HREF=\"", obuf->anchor.hseq);
	Strcat_charp(tmp, html_quote(obuf->anchor.url));
	if (obuf->anchor.target) {
	    Strcat_charp(tmp, "\" TARGET=\"");
	    Strcat_charp(tmp, html_quote(obuf->anchor.target));
	}
	if (obuf->anchor.referer) {
	    Strcat_charp(tmp, "\" REFERER=\"");
	    Strcat_charp(tmp, html_quote(obuf->anchor.referer));
	}
	if (obuf->anchor.title) {
	    Strcat_charp(tmp, "\" TITLE=\"");
	    Strcat_charp(tmp, html_quote(obuf->anchor.title));
	}
	if (obuf->anchor.accesskey) {
	    char *c = html_quote_char(obuf->anchor.accesskey);
	    Strcat_charp(tmp, "\" ACCESSKEY=\"");
	    if (c)
		Strcat_charp(tmp, c);
	    else
		Strcat_char(tmp, obuf->anchor.accesskey);
	}
	Strcat_charp(tmp, "\">");
	push_tag(obuf, tmp->ptr, HTML_A);
    }
    if (!hidden_img && obuf->img_alt) {
	Str tmp = Strnew_charp("<IMG_ALT SRC=\"");
	Strcat_charp(tmp, html_quote(obuf->img_alt->ptr));
	Strcat_charp(tmp, "\">");
	push_tag(obuf, tmp->ptr, HTML_IMG_ALT);
    }
    if (!hidden_input && obuf->input_alt.in) {
	Str tmp;
	if (obuf->input_alt.hseq > 0)
	    obuf->input_alt.hseq = - obuf->input_alt.hseq;
	tmp = Sprintf("<INPUT_ALT hseq=\"%d\" fid=\"%d\" name=\"%s\" type=\"%s\" value=\"%s\">",
		     obuf->input_alt.hseq,
		     obuf->input_alt.fid,
		     obuf->input_alt.name ? obuf->input_alt.name->ptr : "",
		     obuf->input_alt.type ? obuf->input_alt.type->ptr : "",
		     obuf->input_alt.value ? obuf->input_alt.value->ptr : "");
	push_tag(obuf, tmp->ptr, HTML_INPUT_ALT);
    }
    if (!hidden_bold && obuf->in_bold)
	push_tag(obuf, "<B>", HTML_B);
    if (!hidden_italic && obuf->in_italic)
	push_tag(obuf, "<I>", HTML_I);
    if (!hidden_under && obuf->in_under)
	push_tag(obuf, "<U>", HTML_U);
    if (!hidden_strike && obuf->in_strike)
	push_tag(obuf, "<S>", HTML_S);
    if (!hidden_ins && obuf->in_ins)
	push_tag(obuf, "<INS>", HTML_INS);
}

void
do_blankline(struct html_feed_environ *h_env, struct readbuffer *obuf,
	     int indent, int indent_incr, int width)
{
    if (h_env->blank_lines == 0)
	flushline(h_env, obuf, indent, 1, width);
}

void
purgeline(struct html_feed_environ *h_env)
{
    char *p, *q;
    Str tmp;

    if (h_env->buf == NULL || h_env->blank_lines == 0)
	return;

    p = rpopTextLine(h_env->buf)->line->ptr;
    tmp = Strnew();
    while (*p) {
	q = p;
	if (sloppy_parse_line(&p)) {
	    Strcat_charp_n(tmp, q, p - q);
	}
    }
    appendTextLine(h_env->buf, tmp, 0);
    h_env->blank_lines--;
}

static int
close_effect0(struct readbuffer *obuf, int cmd)
{
    int i;
    char *p;

    for (i = obuf->tag_sp - 1; i >= 0; i--) {
	if (obuf->tag_stack[i]->cmd == cmd)
	    break;
    }
    if (i >= 0) {
	obuf->tag_sp--;
	bcopy(&obuf->tag_stack[i + 1], &obuf->tag_stack[i],
	      (obuf->tag_sp - i) * sizeof(struct cmdtable *));
	return 1;
    }
    else if ((p = has_hidden_link(obuf, cmd)) != NULL) {
	passthrough(obuf, p, 1);
	return 1;
    }
    return 0;
}

static void
close_anchor(struct html_feed_environ *h_env, struct readbuffer *obuf)
{
    if (obuf->anchor.url) {
	int i;
	char *p = NULL;
	int is_erased = 0;

	for (i = obuf->tag_sp - 1; i >= 0; i--) {
	    if (obuf->tag_stack[i]->cmd == HTML_A)
		break;
	}
	if (i < 0 && obuf->anchor.hseq > 0 && Strlastchar(obuf->line) == ' ') {
	    Strshrink(obuf->line, 1);
	    obuf->pos--;
	    is_erased = 1;
	}

	if (i >= 0 || (p = has_hidden_link(obuf, HTML_A))) {
	    if (obuf->anchor.hseq > 0) {
		HTMLlineproc1(ANSP, h_env);
		set_space_to_prevchar(obuf->prevchar);
	    }
	    else {
		if (i >= 0) {
		    obuf->tag_sp--;
		    bcopy(&obuf->tag_stack[i + 1], &obuf->tag_stack[i],
			  (obuf->tag_sp - i) * sizeof(struct cmdtable *));
		}
		else {
		    passthrough(obuf, p, 1);
		}
		bzero((void *)&obuf->anchor, sizeof(obuf->anchor));
		return;
	    }
	    is_erased = 0;
	}
	if (is_erased) {
	    Strcat_char(obuf->line, ' ');
	    obuf->pos++;
	}

	push_tag(obuf, "</a>", HTML_N_A);
    }
    bzero((void *)&obuf->anchor, sizeof(obuf->anchor));
}

void
save_fonteffect(struct html_feed_environ *h_env, struct readbuffer *obuf)
{
    if (obuf->fontstat_sp < FONT_STACK_SIZE)
	bcopy(obuf->fontstat, obuf->fontstat_stack[obuf->fontstat_sp],
	      FONTSTAT_SIZE);
    obuf->fontstat_sp++;
    if (obuf->in_bold)
	push_tag(obuf, "</b>", HTML_N_B);
    if (obuf->in_italic)
	push_tag(obuf, "</i>", HTML_N_I);
    if (obuf->in_under)
	push_tag(obuf, "</u>", HTML_N_U);
    if (obuf->in_strike)
	push_tag(obuf, "</s>", HTML_N_S);
    if (obuf->in_ins)
	push_tag(obuf, "</ins>", HTML_N_INS);
    bzero(obuf->fontstat, FONTSTAT_SIZE);
}

void
restore_fonteffect(struct html_feed_environ *h_env, struct readbuffer *obuf)
{
    if (obuf->fontstat_sp > 0)
	obuf->fontstat_sp--;
    if (obuf->fontstat_sp < FONT_STACK_SIZE)
	bcopy(obuf->fontstat_stack[obuf->fontstat_sp], obuf->fontstat,
	      FONTSTAT_SIZE);
    if (obuf->in_bold)
	push_tag(obuf, "<b>", HTML_B);
    if (obuf->in_italic)
	push_tag(obuf, "<i>", HTML_I);
    if (obuf->in_under)
	push_tag(obuf, "<u>", HTML_U);
    if (obuf->in_strike)
	push_tag(obuf, "<s>", HTML_S);
    if (obuf->in_ins)
	push_tag(obuf, "<ins>", HTML_INS);
}

static Str
process_title(struct parsed_tag *tag)
{
    cur_title = Strnew();
    return NULL;
}

static Str
process_n_title(struct parsed_tag *tag)
{
    Str tmp;

    if (!cur_title)
	return NULL;
    Strremovefirstspaces(cur_title);
    Strremovetrailingspaces(cur_title);
    tmp = Strnew_m_charp("<title_alt title=\"",
			 html_quote(cur_title->ptr), "\">", NULL);
    cur_title = NULL;
    return tmp;
}

static void
feed_title(char *str)
{
    if (!cur_title)
	return;
    while (*str) {
	if (*str == '&')
	    Strcat_charp(cur_title, getescapecmd(&str));
	else if (*str == '\n' || *str == '\r') {
	    Strcat_char(cur_title, ' ');
	    str++;
	}
	else
	    Strcat_char(cur_title, *(str++));
    }
}

Str
process_img(struct parsed_tag *tag, int width)
{
    char *p, *q, *r, *r2 = NULL, *s, *t;
#ifdef USE_IMAGE
    int w, i, nw, ni = 1, n, w0 = -1, i0 = -1;
    int align, xoffset, yoffset, top, bottom, ismap = 0;
    int use_image = activeImage && displayImage;
#else
    int w, i, nw, n;
#endif
    int pre_int = FALSE, ext_pre_int = FALSE;
    Str tmp = Strnew();

    if (!parsedtag_get_value(tag, ATTR_SRC, &p))
	return tmp;
    p = url_encode(remove_space(p), cur_baseURL, cur_document_charset);
    q = NULL;
    parsedtag_get_value(tag, ATTR_ALT, &q);
    if (!pseudoInlines && (q == NULL || (*q == '\0' && ignore_null_img_alt)))
	return tmp;
    t = q;
    parsedtag_get_value(tag, ATTR_TITLE, &t);
    w = -1;
    if (parsedtag_get_value(tag, ATTR_WIDTH, &w)) {
	if (w < 0) {
	    if (width > 0)
		w = (int)(-width * pixel_per_char * w / 100 + 0.5);
	    else
		w = -1;
	}
#ifdef USE_IMAGE
	if (use_image) {
	    if (w > 0) {
		w = (int)(w * image_scale / 100 + 0.5);
		if (w == 0)
		    w = 1;
		else if (w > MAX_IMAGE_SIZE)
		    w = MAX_IMAGE_SIZE;
	    }
	}
#endif
    }
    i = -1;
#ifdef USE_IMAGE
    if (use_image) {
	if (parsedtag_get_value(tag, ATTR_HEIGHT, &i)) {
	    if (i > 0) {
		i = (int)(i * image_scale / 100 + 0.5);
		if (i == 0)
		    i = 1;
		else if (i > MAX_IMAGE_SIZE)
		    i = MAX_IMAGE_SIZE;
	    }
	    else {
		i = -1;
	    }
	}
	align = -1;
	parsedtag_get_value(tag, ATTR_ALIGN, &align);
	ismap = 0;
	if (parsedtag_exists(tag, ATTR_ISMAP))
	    ismap = 1;
    }
    else
#endif
	parsedtag_get_value(tag, ATTR_HEIGHT, &i);
    r = NULL;
    parsedtag_get_value(tag, ATTR_USEMAP, &r);
    if (parsedtag_exists(tag, ATTR_PRE_INT))
	ext_pre_int = TRUE;

    tmp = Strnew_size(128);
#ifdef USE_IMAGE
    if (use_image) {
	switch (align) {
	case ALIGN_LEFT:
	    Strcat_charp(tmp, "<div_int align=left>");
	    break;
	case ALIGN_CENTER:
	    Strcat_charp(tmp, "<div_int align=center>");
	    break;
	case ALIGN_RIGHT:
	    Strcat_charp(tmp, "<div_int align=right>");
	    break;
	}
    }
#endif
    if (r) {
	Str tmp2;
	r2 = strchr(r, '#');
	s = "<form_int method=internal action=map>";
	tmp2 = process_form(parse_tag(&s, TRUE));
	if (tmp2)
	    Strcat(tmp, tmp2);
	Strcat(tmp, Sprintf("<input_alt fid=\"%d\" "
			    "type=hidden name=link value=\"", cur_form_id));
	Strcat_charp(tmp, html_quote((r2) ? r2 + 1 : r));
	Strcat(tmp, Sprintf("\"><input_alt hseq=\"%d\" fid=\"%d\" "
			    "type=submit no_effect=true>",
			    cur_hseq++, cur_form_id));
    }
#ifdef USE_IMAGE
    if (use_image) {
	w0 = w;
	i0 = i;
	if (w < 0 || i < 0) {
	    Image image;
	    ParsedURL u;

	    parseURL2(p, &u, cur_baseURL);
	    image.url = parsedURL2Str(&u)->ptr;
	    if (!uncompressed_file_type(u.file, &image.ext))
		image.ext = filename_extension(u.file, TRUE);
	    image.cache = NULL;
	    image.width = w;
	    image.height = i;

	    image.cache = getImage(&image, cur_baseURL, IMG_FLAG_SKIP);
	    if (image.cache && image.cache->width > 0 &&
		image.cache->height > 0) {
		w = w0 = image.cache->width;
		i = i0 = image.cache->height;
	    }
	    if (w < 0)
		w = 8 * pixel_per_char;
	    if (i < 0)
		i = pixel_per_line;
	}
	if (enable_inline_image) {
	    nw = (w > 1) ? ((w - 1) / pixel_per_char_i + 1) : 1 ;
	    ni = (i > 1) ? ((i - 1) / pixel_per_line_i + 1) : 1 ;
	}
	else {
	    nw = (w > 3) ? (int)((w - 3) / pixel_per_char + 1) : 1;
	    ni = (i > 3) ? (int)((i - 3) / pixel_per_line + 1) : 1;
	}
	Strcat(tmp,
	       Sprintf("<pre_int><img_alt hseq=\"%d\" src=\"", cur_iseq++));
	pre_int = TRUE;
    }
    else
#endif
    {
	if (w < 0)
	    w = 12 * pixel_per_char;
	nw = w ? (int)((w - 1) / pixel_per_char + 1) : 1;
	if (r) {
	    Strcat_charp(tmp, "<pre_int>");
	    pre_int = TRUE;
	}
	Strcat_charp(tmp, "<img_alt src=\"");
    }
    Strcat_charp(tmp, html_quote(p));
    Strcat_charp(tmp, "\"");
    if (t) {
	Strcat_charp(tmp, " title=\"");
	Strcat_charp(tmp, html_quote(t));
	Strcat_charp(tmp, "\"");
    }
#ifdef USE_IMAGE
    if (use_image) {
	if (w0 >= 0)
	    Strcat(tmp, Sprintf(" width=%d", w0));
	if (i0 >= 0)
	    Strcat(tmp, Sprintf(" height=%d", i0));
	switch (align) {
	case ALIGN_MIDDLE:
	    if (!enable_inline_image) {
		top = ni / 2;
		bottom = top;
		if (top * 2 == ni)
		    yoffset = (int)(((ni + 1) * pixel_per_line - i) / 2);
		else
		    yoffset = (int)((ni * pixel_per_line - i) / 2);
		break;
	    }
	case ALIGN_TOP:
	    top = 0;
	    bottom = ni - 1;
	    yoffset = 0;
	    break;
	case ALIGN_BOTTOM:
	    top = ni - 1;
	    bottom = 0;
	    yoffset = (int)(ni * pixel_per_line - i);
	    break;
	default:
	    top = ni - 1;
	    bottom = 0;
	    if (ni == 1 && ni * pixel_per_line > i)
		yoffset = 0;
	    else {
		yoffset = (int)(ni * pixel_per_line - i);
		if (yoffset <= -2)
		    yoffset++;
	    }
	    break;
	}

	if (enable_inline_image)
	    xoffset = 0;
	else
	    xoffset = (int)((nw * pixel_per_char - w) / 2);

	if (xoffset)
	    Strcat(tmp, Sprintf(" xoffset=%d", xoffset));
	if (yoffset)
	    Strcat(tmp, Sprintf(" yoffset=%d", yoffset));
	if (top)
	    Strcat(tmp, Sprintf(" top_margin=%d", top));
	if (bottom)
	    Strcat(tmp, Sprintf(" bottom_margin=%d", bottom));
	if (r) {
	    Strcat_charp(tmp, " usemap=\"");
	    Strcat_charp(tmp, html_quote((r2) ? r2 + 1 : r));
	    Strcat_charp(tmp, "\"");
	}
	if (ismap)
	    Strcat_charp(tmp, " ismap");
    }
#endif
    Strcat_charp(tmp, ">");
    if (q != NULL && *q == '\0' && ignore_null_img_alt)
	q = NULL;
    if (q != NULL) {
	n = get_strwidth(q);
#ifdef USE_IMAGE
	if (use_image) {
	    if (n > nw) {
		char *r;
		for (r = q, n = 0; *r; r += get_mclen(r), n += get_mcwidth(r)) {
		    if (n + get_mcwidth(r) > nw)
			break;
		}
		Strcat_charp(tmp, html_quote(Strnew_charp_n(q, r - q)->ptr));
	    }
	    else
		Strcat_charp(tmp, html_quote(q));
	}
	else
#endif
	    Strcat_charp(tmp, html_quote(q));
	goto img_end;
    }
    if (w > 0 && i > 0) {
	/* guess what the image is! */
	if (w < 32 && i < 48) {
	    /* must be an icon or space */
	    n = 1;
	    if (strcasestr(p, "space") || strcasestr(p, "blank"))
		Strcat_charp(tmp, "_");
	    else {
		if (w * i < 8 * 16)
		    Strcat_charp(tmp, "*");
		else {
		    if (!pre_int) {
			Strcat_charp(tmp, "<pre_int>");
			pre_int = TRUE;
		    }
		    push_symbol(tmp, IMG_SYMBOL, symbol_width, 1);
		    n = symbol_width;
		}
	    }
	    goto img_end;
	}
	if (w > 200 && i < 13) {
	    /* must be a horizontal line */
	    if (!pre_int) {
		Strcat_charp(tmp, "<pre_int>");
		pre_int = TRUE;
	    }
	    w = w / pixel_per_char / symbol_width;
	    if (w <= 0)
		w = 1;
	    push_symbol(tmp, HR_SYMBOL, symbol_width, w);
	    n = w * symbol_width;
	    goto img_end;
	}
    }
    for (q = p; *q; q++) ;
    while (q > p && *q != '/')
	q--;
    if (*q == '/')
	q++;
    Strcat_char(tmp, '[');
    n = 1;
    p = q;
    for (; *q; q++) {
	if (!IS_ALNUM(*q) && *q != '_' && *q != '-') {
	    break;
	}
	Strcat_char(tmp, *q);
	n++;
	if (n + 1 >= nw)
	    break;
    }
    Strcat_char(tmp, ']');
    n++;
  img_end:
#ifdef USE_IMAGE
    if (use_image) {
	for (; n < nw; n++)
	    Strcat_char(tmp, ' ');
    }
#endif
    Strcat_charp(tmp, "</img_alt>");
    if (pre_int && !ext_pre_int)
	Strcat_charp(tmp, "</pre_int>");
    if (r) {
	Strcat_charp(tmp, "</input_alt>");
	process_n_form();
    }
#ifdef USE_IMAGE
    if (use_image) {
	switch (align) {
	case ALIGN_RIGHT:
	case ALIGN_CENTER:
	case ALIGN_LEFT:
	    Strcat_charp(tmp, "</div_int>");
	    break;
	}
    }
#endif
    return tmp;
}

Str
process_anchor(struct parsed_tag *tag, char *tagbuf)
{
    if (parsedtag_need_reconstruct(tag)) {
	parsedtag_set_value(tag, ATTR_HSEQ, Sprintf("%d", cur_hseq++)->ptr);
	return parsedtag2str(tag);
    }
    else {
	Str tmp = Sprintf("<a hseq=\"%d\"", cur_hseq++);
	Strcat_charp(tmp, tagbuf + 2);
	return tmp;
    }
}

Str
process_input(struct parsed_tag *tag)
{
    int i = 20, v, x, y, z, iw, ih, size = 20;
    char *q, *p, *r, *p2, *s;
    Str tmp = NULL;
    char *qq = "";
    int qlen = 0;

    if (cur_form_id < 0) {
	char *s = "<form_int method=internal action=none>";
	tmp = process_form(parse_tag(&s, TRUE));
    }
    if (tmp == NULL)
	tmp = Strnew();

    p = "text";
    parsedtag_get_value(tag, ATTR_TYPE, &p);
    q = NULL;
    parsedtag_get_value(tag, ATTR_VALUE, &q);
    r = "";
    parsedtag_get_value(tag, ATTR_NAME, &r);
    parsedtag_get_value(tag, ATTR_SIZE, &size);
    if (size > MAX_INPUT_SIZE)
	    size = MAX_INPUT_SIZE;
    parsedtag_get_value(tag, ATTR_MAXLENGTH, &i);
    p2 = NULL;
    parsedtag_get_value(tag, ATTR_ALT, &p2);
    x = parsedtag_exists(tag, ATTR_CHECKED);
    y = parsedtag_exists(tag, ATTR_ACCEPT);
    z = parsedtag_exists(tag, ATTR_READONLY);

    v = formtype(p);
    if (v == FORM_UNKNOWN)
	return NULL;

    if (!q) {
	switch (v) {
	case FORM_INPUT_IMAGE:
	case FORM_INPUT_SUBMIT:
	case FORM_INPUT_BUTTON:
	    q = "SUBMIT";
	    break;
	case FORM_INPUT_RESET:
	    q = "RESET";
	    break;
	    /* if no VALUE attribute is specified in 
	     * <INPUT TYPE=CHECKBOX> tag, then the value "on" is used 
	     * as a default value. It is not a part of HTML4.0 
	     * specification, but an imitation of Netscape behaviour. 
	     */
	case FORM_INPUT_CHECKBOX:
	    q = "on";
	}
    }
    /* VALUE attribute is not allowed in <INPUT TYPE=FILE> tag. */
    if (v == FORM_INPUT_FILE)
	q = NULL;
    if (q) {
	qq = html_quote(q);
	qlen = get_strwidth(q);
    }

    Strcat_charp(tmp, "<pre_int>");
    switch (v) {
    case FORM_INPUT_PASSWORD:
    case FORM_INPUT_TEXT:
    case FORM_INPUT_FILE:
    case FORM_INPUT_CHECKBOX:
	if (displayLinkNumber)
	    Strcat(tmp, getLinkNumberStr(0));
	Strcat_char(tmp, '[');
	break;
    case FORM_INPUT_RADIO:
	if (displayLinkNumber)
	    Strcat(tmp, getLinkNumberStr(0));
	Strcat_char(tmp, '(');
    }
    Strcat(tmp, Sprintf("<input_alt hseq=\"%d\" fid=\"%d\" type=\"%s\" "
			"name=\"%s\" width=%d maxlength=%d value=\"%s\"",
			cur_hseq++, cur_form_id, html_quote(p),
			html_quote(r), size, i, qq));
    if (x)
	Strcat_charp(tmp, " checked");
    if (y)
	Strcat_charp(tmp, " accept");
    if (z)
	Strcat_charp(tmp, " readonly");
    Strcat_char(tmp, '>');

    if (v == FORM_INPUT_HIDDEN)
	Strcat_charp(tmp, "</input_alt></pre_int>");
    else {
	switch (v) {
	case FORM_INPUT_PASSWORD:
	case FORM_INPUT_TEXT:
	case FORM_INPUT_FILE:
	    Strcat_charp(tmp, "<u>");
	    break;
	case FORM_INPUT_IMAGE:
	    s = NULL;
	    parsedtag_get_value(tag, ATTR_SRC, &s);
	    if (s) {
		Strcat(tmp, Sprintf("<img src=\"%s\"", html_quote(s)));
		if (p2)
		    Strcat(tmp, Sprintf(" alt=\"%s\"", html_quote(p2)));
		if (parsedtag_get_value(tag, ATTR_WIDTH, &iw))
		    Strcat(tmp, Sprintf(" width=\"%d\"", iw));
		if (parsedtag_get_value(tag, ATTR_HEIGHT, &ih))
		    Strcat(tmp, Sprintf(" height=\"%d\"", ih));
		Strcat_charp(tmp, " pre_int>");
		Strcat_charp(tmp, "</input_alt></pre_int>");
		return tmp;
	    }
	case FORM_INPUT_SUBMIT:
	case FORM_INPUT_BUTTON:
	case FORM_INPUT_RESET:
	    if (displayLinkNumber)
		Strcat(tmp, getLinkNumberStr(-1));
	    Strcat_charp(tmp, "[");
	    break;
	}
	switch (v) {
	case FORM_INPUT_PASSWORD:
	    i = 0;
	    if (q) {
		for (; i < qlen && i < size; i++)
		    Strcat_char(tmp, '*');
	    }
	    for (; i < size; i++)
		Strcat_char(tmp, ' ');
	    break;
	case FORM_INPUT_TEXT:
	case FORM_INPUT_FILE:
	    if (q)
		Strcat(tmp, textfieldrep(Strnew_charp(q), size));
	    else {
		for (i = 0; i < size; i++)
		    Strcat_char(tmp, ' ');
	    }
	    break;
	case FORM_INPUT_SUBMIT:
	case FORM_INPUT_BUTTON:
	    if (p2)
		Strcat_charp(tmp, html_quote(p2));
	    else
		Strcat_charp(tmp, qq);
	    break;
	case FORM_INPUT_RESET:
	    Strcat_charp(tmp, qq);
	    break;
	case FORM_INPUT_RADIO:
	case FORM_INPUT_CHECKBOX:
	    if (x)
		Strcat_char(tmp, '*');
	    else
		Strcat_char(tmp, ' ');
	    break;
	}
	switch (v) {
	case FORM_INPUT_PASSWORD:
	case FORM_INPUT_TEXT:
	case FORM_INPUT_FILE:
	    Strcat_charp(tmp, "</u>");
	    break;
	case FORM_INPUT_IMAGE:
	case FORM_INPUT_SUBMIT:
	case FORM_INPUT_BUTTON:
	case FORM_INPUT_RESET:
	    Strcat_charp(tmp, "]");
	}
	Strcat_charp(tmp, "</input_alt>");
	switch (v) {
	case FORM_INPUT_PASSWORD:
	case FORM_INPUT_TEXT:
	case FORM_INPUT_FILE:
	case FORM_INPUT_CHECKBOX:
	    Strcat_char(tmp, ']');
	    break;
	case FORM_INPUT_RADIO:
	    Strcat_char(tmp, ')');
	}
	Strcat_charp(tmp, "</pre_int>");
    }
    return tmp;
}

Str
process_button(struct parsed_tag *tag)
{
    Str tmp = NULL;
    char *p, *q, *r, *qq = "";
    int qlen, v;

    if (cur_form_id < 0) {
       char *s = "<form_int method=internal action=none>";
       tmp = process_form(parse_tag(&s, TRUE));
    }
    if (tmp == NULL)
       tmp = Strnew();

    p = "submit";
    parsedtag_get_value(tag, ATTR_TYPE, &p);
    q = NULL;
    parsedtag_get_value(tag, ATTR_VALUE, &q);
    r = "";
    parsedtag_get_value(tag, ATTR_NAME, &r);

    v = formtype(p);
    if (v == FORM_UNKNOWN)
       return NULL;

    switch (v) {
    case FORM_INPUT_SUBMIT:
    case FORM_INPUT_BUTTON:
    case FORM_INPUT_RESET:
	break;
    default:
	p = "submit";
	v = FORM_INPUT_SUBMIT;
	break;
    }

    if (!q) {
       switch (v) {
       case FORM_INPUT_SUBMIT:
       case FORM_INPUT_BUTTON:
           q = "SUBMIT";
           break;
       case FORM_INPUT_RESET:
           q = "RESET";
           break;
       }
    }
    if (q) {
       qq = html_quote(q);
       qlen = strlen(q);
    }

    /*    Strcat_charp(tmp, "<pre_int>"); */
    Strcat(tmp, Sprintf("<input_alt hseq=\"%d\" fid=\"%d\" type=\"%s\" "
                       "name=\"%s\" value=\"%s\">",
                       cur_hseq++, cur_form_id, html_quote(p),
                       html_quote(r), qq));
    return tmp;
}

Str
process_n_button(void)
{
    Str tmp = Strnew();
    Strcat_charp(tmp, "</input_alt>");
    /*    Strcat_charp(tmp, "</pre_int>"); */
    return tmp;
}

Str
process_select(struct parsed_tag *tag)
{
    Str tmp = NULL;
    char *p;

    if (cur_form_id < 0) {
	char *s = "<form_int method=internal action=none>";
	tmp = process_form(parse_tag(&s, TRUE));
    }

    p = "";
    parsedtag_get_value(tag, ATTR_NAME, &p);
    cur_select = Strnew_charp(p);
    select_is_multiple = parsedtag_exists(tag, ATTR_MULTIPLE);

#ifdef MENU_SELECT
    if (!select_is_multiple) {
	select_str = Strnew_charp("<pre_int>");
	if (displayLinkNumber)
	    Strcat(select_str, getLinkNumberStr(0));
	Strcat(select_str, Sprintf("[<input_alt hseq=\"%d\" "
			     "fid=\"%d\" type=select name=\"%s\" selectnumber=%d",
			     cur_hseq++, cur_form_id, html_quote(p), n_select));
	Strcat_charp(select_str, ">");
	if (n_select == max_select) {
	    max_select *= 2;
	    select_option =
		New_Reuse(FormSelectOption, select_option, max_select);
	}
	select_option[n_select].first = NULL;
	select_option[n_select].last = NULL;
	cur_option_maxwidth = 0;
    }
    else
#endif				/* MENU_SELECT */
	select_str = Strnew();
    cur_option = NULL;
    cur_status = R_ST_NORMAL;
    n_selectitem = 0;
    return tmp;
}

Str
process_n_select(void)
{
    if (cur_select == NULL)
	return NULL;
    process_option();
#ifdef MENU_SELECT
    if (!select_is_multiple) {
	if (select_option[n_select].first) {
	    FormItemList sitem;
	    chooseSelectOption(&sitem, select_option[n_select].first);
	    Strcat(select_str, textfieldrep(sitem.label, cur_option_maxwidth));
	}
	Strcat_charp(select_str, "</input_alt>]</pre_int>");
	n_select++;
    }
    else
#endif				/* MENU_SELECT */
	Strcat_charp(select_str, "<br>");
    cur_select = NULL;
    n_selectitem = 0;
    return select_str;
}

void
feed_select(char *str)
{
    Str tmp = Strnew();
    int prev_status = cur_status;
    static int prev_spaces = -1;
    char *p;

    if (cur_select == NULL)
	return;
    while (read_token(tmp, &str, &cur_status, 0, 0)) {
	if (cur_status != R_ST_NORMAL || prev_status != R_ST_NORMAL)
	    continue;
	p = tmp->ptr;
	if (tmp->ptr[0] == '<' && Strlastchar(tmp) == '>') {
	    struct parsed_tag *tag;
	    char *q;
	    if (!(tag = parse_tag(&p, FALSE)))
		continue;
	    switch (tag->tagid) {
	    case HTML_OPTION:
		process_option();
		cur_option = Strnew();
		if (parsedtag_get_value(tag, ATTR_VALUE, &q))
		    cur_option_value = Strnew_charp(q);
		else
		    cur_option_value = NULL;
		if (parsedtag_get_value(tag, ATTR_LABEL, &q))
		    cur_option_label = Strnew_charp(q);
		else
		    cur_option_label = NULL;
		cur_option_selected = parsedtag_exists(tag, ATTR_SELECTED);
		prev_spaces = -1;
		break;
	    case HTML_N_OPTION:
		/* do nothing */
		break;
	    default:
		/* never happen */
		break;
	    }
	}
	else if (cur_option) {
	    while (*p) {
		if (IS_SPACE(*p) && prev_spaces != 0) {
		    p++;
		    if (prev_spaces > 0)
			prev_spaces++;
		}
		else {
		    if (IS_SPACE(*p))
			prev_spaces = 1;
		    else
			prev_spaces = 0;
		    if (*p == '&')
			Strcat_charp(cur_option, getescapecmd(&p));
		    else
			Strcat_char(cur_option, *(p++));
		}
	    }
	}
    }
}

void
process_option(void)
{
    char begin_char = '[', end_char = ']';
    int len;

    if (cur_select == NULL || cur_option == NULL)
	return;
    while (cur_option->length > 0 && IS_SPACE(Strlastchar(cur_option)))
	Strshrink(cur_option, 1);
    if (cur_option_value == NULL)
	cur_option_value = cur_option;
    if (cur_option_label == NULL)
	cur_option_label = cur_option;
#ifdef MENU_SELECT
    if (!select_is_multiple) {
	len = get_Str_strwidth(cur_option_label);
	if (len > cur_option_maxwidth)
	    cur_option_maxwidth = len;
	addSelectOption(&select_option[n_select],
			cur_option_value,
			cur_option_label, cur_option_selected);
	return;
    }
#endif				/* MENU_SELECT */
    if (!select_is_multiple) {
	begin_char = '(';
	end_char = ')';
    }
    Strcat(select_str, Sprintf("<br><pre_int>%c<input_alt hseq=\"%d\" "
			       "fid=\"%d\" type=%s name=\"%s\" value=\"%s\"",
			       begin_char, cur_hseq++, cur_form_id,
			       select_is_multiple ? "checkbox" : "radio",
			       html_quote(cur_select->ptr),
			       html_quote(cur_option_value->ptr)));
    if (cur_option_selected)
	Strcat_charp(select_str, " checked>*</input_alt>");
    else
	Strcat_charp(select_str, "> </input_alt>");
    Strcat_char(select_str, end_char);
    Strcat_charp(select_str, html_quote(cur_option_label->ptr));
    Strcat_charp(select_str, "</pre_int>");
    n_selectitem++;
}

Str
process_textarea(struct parsed_tag *tag, int width)
{
    Str tmp = NULL;
    char *p;
#define TEXTAREA_ATTR_COL_MAX 4096
#define TEXTAREA_ATTR_ROWS_MAX 4096

    if (cur_form_id < 0) {
	char *s = "<form_int method=internal action=none>";
	tmp = process_form(parse_tag(&s, TRUE));
    }

    p = "";
    parsedtag_get_value(tag, ATTR_NAME, &p);
    cur_textarea = Strnew_charp(p);
    cur_textarea_size = 20;
    if (parsedtag_get_value(tag, ATTR_COLS, &p)) {
	cur_textarea_size = atoi(p);
	if (strlen(p) > 0 && p[strlen(p) - 1] == '%')
	    cur_textarea_size = width * cur_textarea_size / 100 - 2;
	if (cur_textarea_size <= 0) {
	    cur_textarea_size = 20;
	} else if (cur_textarea_size > TEXTAREA_ATTR_COL_MAX) {
	    cur_textarea_size = TEXTAREA_ATTR_COL_MAX;
	}
    }
    cur_textarea_rows = 1;
    if (parsedtag_get_value(tag, ATTR_ROWS, &p)) {
	cur_textarea_rows = atoi(p);
	if (cur_textarea_rows <= 0) {
	    cur_textarea_rows = 1;
	} else if (cur_textarea_rows > TEXTAREA_ATTR_ROWS_MAX) {
	    cur_textarea_rows = TEXTAREA_ATTR_ROWS_MAX;
	}
    }
    cur_textarea_readonly = parsedtag_exists(tag, ATTR_READONLY);
    if (n_textarea >= max_textarea) {
	max_textarea *= 2;
	textarea_str = New_Reuse(Str, textarea_str, max_textarea);
    }
    textarea_str[n_textarea] = Strnew();
    ignore_nl_textarea = TRUE;

    return tmp;
}

Str
process_n_textarea(void)
{
    Str tmp;
    int i;

    if (cur_textarea == NULL)
	return NULL;

    tmp = Strnew();
    Strcat(tmp, Sprintf("<pre_int>[<input_alt hseq=\"%d\" fid=\"%d\" "
			"type=textarea name=\"%s\" size=%d rows=%d "
			"top_margin=%d textareanumber=%d",
			cur_hseq, cur_form_id,
			html_quote(cur_textarea->ptr),
			cur_textarea_size, cur_textarea_rows,
			cur_textarea_rows - 1, n_textarea));
    if (cur_textarea_readonly)
	Strcat_charp(tmp, " readonly");
    Strcat_charp(tmp, "><u>");
    for (i = 0; i < cur_textarea_size; i++)
	Strcat_char(tmp, ' ');
    Strcat_charp(tmp, "</u></input_alt>]</pre_int>\n");
    cur_hseq++;
    n_textarea++;
    cur_textarea = NULL;

    return tmp;
}

void
feed_textarea(char *str)
{
    if (cur_textarea == NULL)
	return;
    if (ignore_nl_textarea) {
	if (*str == '\r')
	    str++;
	if (*str == '\n')
	    str++;
    }
    ignore_nl_textarea = FALSE;
    while (*str) {
	if (*str == '&')
	    Strcat_charp(textarea_str[n_textarea], getescapecmd(&str));
	else if (*str == '\n') {
	    Strcat_charp(textarea_str[n_textarea], "\r\n");
	    str++;
	}
	else if (*str == '\r')
	    str++;
	else
	    Strcat_char(textarea_str[n_textarea], *(str++));
    }
}

Str
process_hr(struct parsed_tag *tag, int width, int indent_width)
{
    Str tmp = Strnew_charp("<nobr>");
    int w = 0;
    int x = ALIGN_CENTER;
#define HR_ATTR_WIDTH_MAX 65535

    if (width > indent_width)
	width -= indent_width;
    if (parsedtag_get_value(tag, ATTR_WIDTH, &w)) {
	if (w > HR_ATTR_WIDTH_MAX) {
	    w = HR_ATTR_WIDTH_MAX;
	}
	w = REAL_WIDTH(w, width);
    } else {
	w = width;
    }

    parsedtag_get_value(tag, ATTR_ALIGN, &x);
    switch (x) {
    case ALIGN_CENTER:
	Strcat_charp(tmp, "<div_int align=center>");
	break;
    case ALIGN_RIGHT:
	Strcat_charp(tmp, "<div_int align=right>");
	break;
    case ALIGN_LEFT:
	Strcat_charp(tmp, "<div_int align=left>");
	break;
    }
    w /= symbol_width;
    if (w <= 0)
	w = 1;
    push_symbol(tmp, HR_SYMBOL, symbol_width, w);
    Strcat_charp(tmp, "</div_int></nobr>");
    return tmp;
}

#ifdef USE_M17N
static char *
check_charset(char *p)
{
    return wc_guess_charset(p, 0) ? p : NULL;
}

static char *
check_accept_charset(char *ac)
{
    char *s = ac, *e;

    while (*s) {
	while (*s && (IS_SPACE(*s) || *s == ','))
	    s++;
	if (!*s)
	    break;
	e = s;
	while (*e && !(IS_SPACE(*e) || *e == ','))
	    e++;
	if (wc_guess_charset(Strnew_charp_n(s, e - s)->ptr, 0))
	    return ac;
	s = e;
    }
    return NULL;
}
#endif

static Str
process_form_int(struct parsed_tag *tag, int fid)
{
    char *p, *q, *r, *s, *tg, *n;

    p = "get";
    parsedtag_get_value(tag, ATTR_METHOD, &p);
    q = "!CURRENT_URL!";
    parsedtag_get_value(tag, ATTR_ACTION, &q);
    q = url_encode(remove_space(q), cur_baseURL, cur_document_charset);
    r = NULL;
#ifdef USE_M17N
    if (parsedtag_get_value(tag, ATTR_ACCEPT_CHARSET, &r))
	r = check_accept_charset(r);
    if (!r && parsedtag_get_value(tag, ATTR_CHARSET, &r))
	r = check_charset(r);
#endif
    s = NULL;
    parsedtag_get_value(tag, ATTR_ENCTYPE, &s);
    tg = NULL;
    parsedtag_get_value(tag, ATTR_TARGET, &tg);
    n = NULL;
    parsedtag_get_value(tag, ATTR_NAME, &n);

    if (fid < 0) {
	form_max++;
	form_sp++;
	fid = form_max;
    }
    else {			/* <form_int> */
	if (form_max < fid)
	    form_max = fid;
	form_sp = fid;
    }
    if (forms_size == 0) {
	forms_size = INITIAL_FORM_SIZE;
	forms = New_N(FormList *, forms_size);
	form_stack = NewAtom_N(int, forms_size);
    }
    if (forms_size <= form_max) {
	forms_size += form_max;
	forms = New_Reuse(FormList *, forms, forms_size);
	form_stack = New_Reuse(int, form_stack, forms_size);
    }
    form_stack[form_sp] = fid;

    if (w3m_halfdump) {
	Str tmp = Sprintf("<form_int fid=\"%d\" action=\"%s\" method=\"%s\"",
			  fid, html_quote(q), html_quote(p));
	if (s)
	    Strcat(tmp, Sprintf(" enctype=\"%s\"", html_quote(s)));
	if (tg)
	    Strcat(tmp, Sprintf(" target=\"%s\"", html_quote(tg)));
	if (n)
	    Strcat(tmp, Sprintf(" name=\"%s\"", html_quote(n)));
#ifdef USE_M17N
	if (r)
	    Strcat(tmp, Sprintf(" accept-charset=\"%s\"", html_quote(r)));
#endif
	Strcat_charp(tmp, ">");
	return tmp;
    }

    forms[fid] = newFormList(q, p, r, s, tg, n, NULL);
    return NULL;
}

Str
process_form(struct parsed_tag *tag)
{
    return process_form_int(tag, -1);
}

Str
process_n_form(void)
{
    if (form_sp >= 0)
	form_sp--;
    return NULL;
}

static void
clear_ignore_p_flag(int cmd, struct readbuffer *obuf)
{
    static int clear_flag_cmd[] = {
	HTML_HR, HTML_UNKNOWN
    };
    int i;

    for (i = 0; clear_flag_cmd[i] != HTML_UNKNOWN; i++) {
	if (cmd == clear_flag_cmd[i]) {
	    obuf->flag &= ~RB_IGNORE_P;
	    return;
	}
    }
}

static void
set_alignment(struct readbuffer *obuf, struct parsed_tag *tag)
{
    long flag = -1;
    int align;

    if (parsedtag_get_value(tag, ATTR_ALIGN, &align)) {
	switch (align) {
	case ALIGN_CENTER:
	    flag = RB_CENTER;
	    break;
	case ALIGN_RIGHT:
	    flag = RB_RIGHT;
	    break;
	case ALIGN_LEFT:
	    flag = RB_LEFT;
	}
    }
    RB_SAVE_FLAG(obuf);
    if (flag != -1) {
	RB_SET_ALIGN(obuf, flag);
    }
}

#ifdef ID_EXT
static void
process_idattr(struct readbuffer *obuf, int cmd, struct parsed_tag *tag)
{
    char *id = NULL, *framename = NULL;
    Str idtag = NULL;

    /* 
     * HTML_TABLE is handled by the other process.
     */
    if (cmd == HTML_TABLE)
	return;

    parsedtag_get_value(tag, ATTR_ID, &id);
    parsedtag_get_value(tag, ATTR_FRAMENAME, &framename);
    if (id == NULL)
	return;
    if (framename)
	idtag = Sprintf("<_id id=\"%s\" framename=\"%s\">",
			html_quote(id), html_quote(framename));
    else
	idtag = Sprintf("<_id id=\"%s\">", html_quote(id));
    push_tag(obuf, idtag->ptr, HTML_NOP);
}
#endif				/* ID_EXT */

#define CLOSE_P if (obuf->flag & RB_P) { \
      flushline(h_env, obuf, envs[h_env->envc].indent,0,h_env->limit);\
      RB_RESTORE_FLAG(obuf);\
      obuf->flag &= ~RB_P;\
    }

#define HTML5_CLOSE_A do { \
	if (obuf->flag & RB_HTML5) { \
	    close_anchor(h_env, obuf); \
	} \
    } while (0)

#define CLOSE_A do { \
	CLOSE_P; \
	if (!(obuf->flag & RB_HTML5)) { \
	    close_anchor(h_env, obuf); \
	} \
    } while (0)

#define CLOSE_DT \
    if (obuf->flag & RB_IN_DT) { \
      obuf->flag &= ~RB_IN_DT; \
      HTMLlineproc1("</b>", h_env); \
    }

#define PUSH_ENV(cmd) \
    if (++h_env->envc_real < h_env->nenv) { \
      ++h_env->envc; \
      envs[h_env->envc].env = cmd; \
      envs[h_env->envc].count = 0; \
      if (h_env->envc <= MAX_INDENT_LEVEL) \
        envs[h_env->envc].indent = envs[h_env->envc - 1].indent + INDENT_INCR; \
      else \
        envs[h_env->envc].indent = envs[h_env->envc - 1].indent; \
    }

#define POP_ENV \
    if (h_env->envc_real-- < h_env->nenv) \
      h_env->envc--;

static int
ul_type(struct parsed_tag *tag, int default_type)
{
    char *p;
    if (parsedtag_get_value(tag, ATTR_TYPE, &p)) {
	if (!strcasecmp(p, "disc"))
	    return (int)'d';
	else if (!strcasecmp(p, "circle"))
	    return (int)'c';
	else if (!strcasecmp(p, "square"))
	    return (int)'s';
    }
    return default_type;
}

int
getMetaRefreshParam(char *q, Str *refresh_uri)
{
    int refresh_interval;
    char *r;
    Str s_tmp = NULL;

    if (q == NULL || refresh_uri == NULL)
	return 0;

    refresh_interval = atoi(q);
    if (refresh_interval < 0)
	return 0;

    while (*q) {
	if (!strncasecmp(q, "url=", 4)) {
	    q += 4;
	    if (*q == '\"' || *q == '\'')	/* " or ' */
		q++;
	    r = q;
	    while (*r && !IS_SPACE(*r) && *r != ';')
		r++;
	    s_tmp = Strnew_charp_n(q, r - q);

	    if (s_tmp->length > 0 &&
	        (s_tmp->ptr[s_tmp->length - 1] == '\"' ||	/* " */
		 s_tmp->ptr[s_tmp->length - 1] == '\'')) {	/* ' */
		s_tmp->length--;
		s_tmp->ptr[s_tmp->length] = '\0';
	    }
	    q = r;
	}
	while (*q && *q != ';')
	    q++;
	if (*q == ';')
	    q++;
	while (*q && *q == ' ')
	    q++;
    }
    *refresh_uri = s_tmp;
    return refresh_interval;
}

int
HTMLtagproc1(struct parsed_tag *tag, struct html_feed_environ *h_env)
{
    char *p, *q, *r;
    int i, w, x, y, z, count, width;
    struct readbuffer *obuf = h_env->obuf;
    struct environment *envs = h_env->envs;
    Str tmp;
    int hseq;
    int cmd;
#ifdef ID_EXT
    char *id = NULL;
#endif				/* ID_EXT */

    cmd = tag->tagid;

    if (obuf->flag & RB_PRE) {
	switch (cmd) {
	case HTML_NOBR:
	case HTML_N_NOBR:
	case HTML_PRE_INT:
	case HTML_N_PRE_INT:
	    return 1;
	}
    }

    switch (cmd) {
    case HTML_B:
	obuf->in_bold++;
	if (obuf->in_bold > 1)
	    return 1;
	return 0;
    case HTML_N_B:
	if (obuf->in_bold == 1 && close_effect0(obuf, HTML_B))
	    obuf->in_bold = 0;
	if (obuf->in_bold > 0) {
	    obuf->in_bold--;
	    if (obuf->in_bold == 0)
		return 0;
	}
	return 1;
    case HTML_I:
	obuf->in_italic++;
	if (obuf->in_italic > 1)
	    return 1;
	return 0;
    case HTML_N_I:
	if (obuf->in_italic == 1 && close_effect0(obuf, HTML_I))
	    obuf->in_italic = 0;
	if (obuf->in_italic > 0) {
	    obuf->in_italic--;
	    if (obuf->in_italic == 0)
		return 0;
	}
	return 1;
    case HTML_U:
	obuf->in_under++;
	if (obuf->in_under > 1)
	    return 1;
	return 0;
    case HTML_N_U:
	if (obuf->in_under == 1 && close_effect0(obuf, HTML_U))
	    obuf->in_under = 0;
	if (obuf->in_under > 0) {
	    obuf->in_under--;
	    if (obuf->in_under == 0)
		return 0;
	}
	return 1;
    case HTML_EM:
	HTMLlineproc1("<i>", h_env);
	return 1;
    case HTML_N_EM:
	HTMLlineproc1("</i>", h_env);
	return 1;
    case HTML_STRONG:
	HTMLlineproc1("<b>", h_env);
	return 1;
    case HTML_N_STRONG:
	HTMLlineproc1("</b>", h_env);
	return 1;
    case HTML_Q:
#ifdef USE_M17N
#ifdef USE_UNICODE
	if (DisplayCharset != WC_CES_US_ASCII) {
	    HTMLlineproc1((obuf->q_level & 1 ? "&lsquo;": "&ldquo;"), h_env);
	    obuf->q_level += 1;
	}
	else
#endif
#endif
	HTMLlineproc1("`", h_env);
	return 1;
    case HTML_N_Q:
#ifdef USE_M17N
#ifdef USE_UNICODE
	if (DisplayCharset != WC_CES_US_ASCII) {
	    obuf->q_level -= 1;
	    HTMLlineproc1((obuf->q_level & 1 ? "&rsquo;": "&rdquo;"), h_env);
	}
	else
#endif
#endif
	HTMLlineproc1("'", h_env);
	return 1;
    case HTML_FIGURE:
    case HTML_N_FIGURE:
    case HTML_P:
    case HTML_N_P:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 1, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	}
	obuf->flag |= RB_IGNORE_P;
	if (cmd == HTML_P) {
	    set_alignment(obuf, tag);
	    obuf->flag |= RB_P;
	}
	return 1;
    case HTML_FIGCAPTION:
    case HTML_N_FIGCAPTION:
    case HTML_BR:
	flushline(h_env, obuf, envs[h_env->envc].indent, 1, h_env->limit);
	h_env->blank_lines = 0;
	return 1;
    case HTML_H:
	if (!(obuf->flag & (RB_PREMODE | RB_IGNORE_P))) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	}
	HTMLlineproc1("<b>", h_env);
	set_alignment(obuf, tag);
	return 1;
    case HTML_N_H:
	HTMLlineproc1("</b>", h_env);
	if (!(obuf->flag & RB_PREMODE)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	}
	do_blankline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	RB_RESTORE_FLAG(obuf);
	close_anchor(h_env, obuf);
	obuf->flag |= RB_IGNORE_P;
	return 1;
    case HTML_UL:
    case HTML_OL:
    case HTML_BLQ:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    if (!(obuf->flag & RB_PREMODE) &&
		(h_env->envc == 0 || cmd == HTML_BLQ))
		do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			     h_env->limit);
	}
	PUSH_ENV(cmd);
	if (cmd == HTML_UL || cmd == HTML_OL) {
	    if (parsedtag_get_value(tag, ATTR_START, &count)) {
		envs[h_env->envc].count = count - 1;
	    }
	}
	if (cmd == HTML_OL) {
	    envs[h_env->envc].type = '1';
	    if (parsedtag_get_value(tag, ATTR_TYPE, &p)) {
		envs[h_env->envc].type = (int)*p;
	    }
	}
	if (cmd == HTML_UL)
	    envs[h_env->envc].type = ul_type(tag, 0);
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	return 1;
    case HTML_N_UL:
    case HTML_N_OL:
    case HTML_N_DL:
    case HTML_N_BLQ:
	CLOSE_DT;
	CLOSE_A;
	if (h_env->envc > 0) {
	    flushline(h_env, obuf, envs[h_env->envc - 1].indent, 0,
		      h_env->limit);
	    POP_ENV;
	    if (!(obuf->flag & RB_PREMODE) &&
		(h_env->envc == 0 || cmd == HTML_N_DL || cmd == HTML_N_BLQ)) {
		do_blankline(h_env, obuf,
			     envs[h_env->envc].indent,
			     INDENT_INCR, h_env->limit);
		obuf->flag |= RB_IGNORE_P;
	    }
	}
	close_anchor(h_env, obuf);
	return 1;
    case HTML_DL:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    if (!(obuf->flag & RB_PREMODE))
		do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			     h_env->limit);
	}
	PUSH_ENV(cmd);
	if (parsedtag_exists(tag, ATTR_COMPACT))
	    envs[h_env->envc].env = HTML_DL_COMPACT;
	obuf->flag |= RB_IGNORE_P;
	return 1;
    case HTML_LI:
	CLOSE_A;
	CLOSE_DT;
	if (h_env->envc > 0) {
	    Str num;
	    flushline(h_env, obuf,
		      envs[h_env->envc - 1].indent, 0, h_env->limit);
	    envs[h_env->envc].count++;
	    if (parsedtag_get_value(tag, ATTR_VALUE, &p)) {
		count = atoi(p);
		if (count > 0)
		    envs[h_env->envc].count = count;
		else
		    envs[h_env->envc].count = 0;
	    }
	    switch (envs[h_env->envc].env) {
	    case HTML_UL:
		envs[h_env->envc].type = ul_type(tag, envs[h_env->envc].type);
		for (i = 0; i < INDENT_INCR - 3; i++)
		    push_charp(obuf, 1, NBSP, PC_ASCII);
		tmp = Strnew();
		switch (envs[h_env->envc].type) {
		case 'd':
		    push_symbol(tmp, UL_SYMBOL_DISC, symbol_width, 1);
		    break;
		case 'c':
		    push_symbol(tmp, UL_SYMBOL_CIRCLE, symbol_width, 1);
		    break;
		case 's':
		    push_symbol(tmp, UL_SYMBOL_SQUARE, symbol_width, 1);
		    break;
		default:
		    push_symbol(tmp,
				UL_SYMBOL((h_env->envc_real -
					   1) % MAX_UL_LEVEL), symbol_width,
				1);
		    break;
		}
		if (symbol_width == 1)
		    push_charp(obuf, 1, NBSP, PC_ASCII);
		push_str(obuf, symbol_width, tmp, PC_ASCII);
		push_charp(obuf, 1, NBSP, PC_ASCII);
		set_space_to_prevchar(obuf->prevchar);
		break;
	    case HTML_OL:
		if (parsedtag_get_value(tag, ATTR_TYPE, &p))
		    envs[h_env->envc].type = (int)*p;
		switch ((envs[h_env->envc].count > 0)? envs[h_env->envc].type: '1') {
		case 'i':
		    num = romanNumeral(envs[h_env->envc].count);
		    break;
		case 'I':
		    num = romanNumeral(envs[h_env->envc].count);
		    Strupper(num);
		    break;
		case 'a':
		    num = romanAlphabet(envs[h_env->envc].count);
		    break;
		case 'A':
		    num = romanAlphabet(envs[h_env->envc].count);
		    Strupper(num);
		    break;
		default:
		    num = Sprintf("%d", envs[h_env->envc].count);
		    break;
		}
		if (INDENT_INCR >= 4)
		    Strcat_charp(num, ". ");
		else
		    Strcat_char(num, '.');
		push_spaces(obuf, 1, INDENT_INCR - num->length);
		push_str(obuf, num->length, num, PC_ASCII);
		if (INDENT_INCR >= 4)
		    set_space_to_prevchar(obuf->prevchar);
		break;
	    default:
		push_spaces(obuf, 1, INDENT_INCR);
		break;
	    }
	}
	else {
	    flushline(h_env, obuf, 0, 0, h_env->limit);
	}
	obuf->flag |= RB_IGNORE_P;
	return 1;
    case HTML_DT:
	CLOSE_A;
	if (h_env->envc == 0 ||
	    (h_env->envc_real < h_env->nenv &&
	     envs[h_env->envc].env != HTML_DL &&
	     envs[h_env->envc].env != HTML_DL_COMPACT)) {
	    PUSH_ENV(HTML_DL);
	}
	if (h_env->envc > 0) {
	    flushline(h_env, obuf,
		      envs[h_env->envc - 1].indent, 0, h_env->limit);
	}
	if (!(obuf->flag & RB_IN_DT)) {
	    HTMLlineproc1("<b>", h_env);
	    obuf->flag |= RB_IN_DT;
	}
	obuf->flag |= RB_IGNORE_P;
	return 1;
    case HTML_DD:
	CLOSE_A;
	CLOSE_DT;
	if (h_env->envc == 0 ||
	    (h_env->envc_real < h_env->nenv &&
	     envs[h_env->envc].env != HTML_DL &&
	     envs[h_env->envc].env != HTML_DL_COMPACT)) {
	    PUSH_ENV(HTML_DL);
	}
	if (envs[h_env->envc].env == HTML_DL_COMPACT) {
	    if (obuf->pos > envs[h_env->envc].indent)
		flushline(h_env, obuf, envs[h_env->envc].indent, 0,
			  h_env->limit);
	    else
		push_spaces(obuf, 1, envs[h_env->envc].indent - obuf->pos);
	}
	else
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	/* obuf->flag |= RB_IGNORE_P; */
	return 1;
    case HTML_TITLE:
	close_anchor(h_env, obuf);
	process_title(tag);
	obuf->flag |= RB_TITLE;
	obuf->end_tag = HTML_N_TITLE;
	return 1;
    case HTML_N_TITLE:
	if (!(obuf->flag & RB_TITLE))
	    return 1;
	obuf->flag &= ~RB_TITLE;
	obuf->end_tag = 0;
	tmp = process_n_title(tag);
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_TITLE_ALT:
	if (parsedtag_get_value(tag, ATTR_TITLE, &p))
	    h_env->title = html_unquote(p);
	return 0;
    case HTML_FRAMESET:
	PUSH_ENV(cmd);
	push_charp(obuf, 9, "--FRAME--", PC_ASCII);
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	return 0;
    case HTML_N_FRAMESET:
	if (h_env->envc > 0) {
	    POP_ENV;
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	}
	return 0;
    case HTML_NOFRAMES:
	CLOSE_A;
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	obuf->flag |= (RB_NOFRAMES | RB_IGNORE_P);
	/* istr = str; */
	return 1;
    case HTML_N_NOFRAMES:
	CLOSE_A;
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	obuf->flag &= ~RB_NOFRAMES;
	return 1;
    case HTML_FRAME:
	q = r = NULL;
	parsedtag_get_value(tag, ATTR_SRC, &q);
	parsedtag_get_value(tag, ATTR_NAME, &r);
	if (q) {
	    q = html_quote(q);
	    push_tag(obuf, Sprintf("<a hseq=\"%d\" href=\"%s\">",
				   cur_hseq++, q)->ptr, HTML_A);
	    if (r)
		q = html_quote(r);
	    push_charp(obuf, get_strwidth(q), q, PC_ASCII);
	    push_tag(obuf, "</a>", HTML_N_A);
	}
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	return 0;
    case HTML_HR:
	close_anchor(h_env, obuf);
	tmp = process_hr(tag, h_env->limit, envs[h_env->envc].indent);
	HTMLlineproc1(tmp->ptr, h_env);
	set_space_to_prevchar(obuf->prevchar);
	return 1;
    case HTML_PRE:
	x = parsedtag_exists(tag, ATTR_FOR_TABLE);
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    if (!x)
		do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			     h_env->limit);
	}
	else
	    fillline(obuf, envs[h_env->envc].indent);
	obuf->flag |= (RB_PRE | RB_IGNORE_P);
	/* istr = str; */
	return 1;
    case HTML_N_PRE:
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	if (!(obuf->flag & RB_IGNORE_P)) {
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	    obuf->flag |= RB_IGNORE_P;
	    h_env->blank_lines++;
	}
	obuf->flag &= ~RB_PRE;
	close_anchor(h_env, obuf);
	return 1;
    case HTML_PRE_INT:
	i = obuf->line->length;
	append_tags(obuf);
	if (!(obuf->flag & RB_SPECIAL)) {
	    set_breakpoint(obuf, obuf->line->length - i);
	}
	obuf->flag |= RB_PRE_INT;
	return 0;
    case HTML_N_PRE_INT:
	push_tag(obuf, "</pre_int>", HTML_N_PRE_INT);
	obuf->flag &= ~RB_PRE_INT;
	if (!(obuf->flag & RB_SPECIAL) && obuf->pos > obuf->bp.pos) {
	    set_prevchar(obuf->prevchar, "", 0);
	    obuf->prev_ctype = PC_CTRL;
	}
	return 1;
    case HTML_NOBR:
	obuf->flag |= RB_NOBR;
	obuf->nobr_level++;
	return 0;
    case HTML_N_NOBR:
	if (obuf->nobr_level > 0)
	    obuf->nobr_level--;
	if (obuf->nobr_level == 0)
	    obuf->flag &= ~RB_NOBR;
	return 0;
    case HTML_PRE_PLAIN:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	}
	obuf->flag |= (RB_PRE | RB_IGNORE_P);
	return 1;
    case HTML_N_PRE_PLAIN:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	    obuf->flag |= RB_IGNORE_P;
	}
	obuf->flag &= ~RB_PRE;
	return 1;
    case HTML_LISTING:
    case HTML_XMP:
    case HTML_PLAINTEXT:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	}
	obuf->flag |= (RB_PLAIN | RB_IGNORE_P);
	switch (cmd) {
	case HTML_LISTING:
	    obuf->end_tag = HTML_N_LISTING;
	    break;
	case HTML_XMP:
	    obuf->end_tag = HTML_N_XMP;
	    break;
	case HTML_PLAINTEXT:
	    obuf->end_tag = MAX_HTMLTAG;
	    break;
	}
	return 1;
    case HTML_N_LISTING:
    case HTML_N_XMP:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P)) {
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	    do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			 h_env->limit);
	    obuf->flag |= RB_IGNORE_P;
	}
	obuf->flag &= ~RB_PLAIN;
	obuf->end_tag = 0;
	return 1;
    case HTML_SCRIPT:
	obuf->flag |= RB_SCRIPT;
	obuf->end_tag = HTML_N_SCRIPT;
	return 1;
    case HTML_STYLE:
	obuf->flag |= RB_STYLE;
	obuf->end_tag = HTML_N_STYLE;
	return 1;
    case HTML_N_SCRIPT:
	obuf->flag &= ~RB_SCRIPT;
	obuf->end_tag = 0;
	return 1;
    case HTML_N_STYLE:
	obuf->flag &= ~RB_STYLE;
	obuf->end_tag = 0;
	return 1;
    case HTML_A:
	if (obuf->anchor.url)
	    close_anchor(h_env, obuf);

	hseq = 0;

	if (parsedtag_get_value(tag, ATTR_HREF, &p))
	    obuf->anchor.url = Strnew_charp(p)->ptr;
	if (parsedtag_get_value(tag, ATTR_TARGET, &p))
	    obuf->anchor.target = Strnew_charp(p)->ptr;
	if (parsedtag_get_value(tag, ATTR_REFERER, &p))
	    obuf->anchor.referer = Strnew_charp(p)->ptr;
	if (parsedtag_get_value(tag, ATTR_TITLE, &p))
	    obuf->anchor.title = Strnew_charp(p)->ptr;
	if (parsedtag_get_value(tag, ATTR_ACCESSKEY, &p))
	    obuf->anchor.accesskey = (unsigned char)*p;
	if (parsedtag_get_value(tag, ATTR_HSEQ, &hseq))
	    obuf->anchor.hseq = hseq;

	if (hseq == 0 && obuf->anchor.url) {
	    obuf->anchor.hseq = cur_hseq;
	    tmp = process_anchor(tag, h_env->tagbuf->ptr);
	    push_tag(obuf, tmp->ptr, HTML_A);
	    if (displayLinkNumber)
		HTMLlineproc1(getLinkNumberStr(-1)->ptr, h_env);
	    return 1;
	}
	return 0;
    case HTML_N_A:
	close_anchor(h_env, obuf);
	return 1;
    case HTML_IMG:
	if (parsedtag_exists(tag, ATTR_USEMAP))
	    HTML5_CLOSE_A;
	tmp = process_img(tag, h_env->limit);
	HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_IMG_ALT:
	if (parsedtag_get_value(tag, ATTR_SRC, &p))
	    obuf->img_alt = Strnew_charp(p);
#ifdef USE_IMAGE
	i = 0;
	if (parsedtag_get_value(tag, ATTR_TOP_MARGIN, &i)) {
	    if ((short)i > obuf->top_margin)
		obuf->top_margin = (short)i;
	}
	i = 0;
	if (parsedtag_get_value(tag, ATTR_BOTTOM_MARGIN, &i)) {
	    if ((short)i > obuf->bottom_margin)
		obuf->bottom_margin = (short)i;
	}
#endif
	return 0;
    case HTML_N_IMG_ALT:
	if (obuf->img_alt) {
	    if (!close_effect0(obuf, HTML_IMG_ALT))
		push_tag(obuf, "</img_alt>", HTML_N_IMG_ALT);
	    obuf->img_alt = NULL;
	}
	return 1;
    case HTML_INPUT_ALT:
	i = 0;
	if (parsedtag_get_value(tag, ATTR_TOP_MARGIN, &i)) {
	    if ((short)i > obuf->top_margin)
		obuf->top_margin = (short)i;
	}
	i = 0;
	if (parsedtag_get_value(tag, ATTR_BOTTOM_MARGIN, &i)) {
	    if ((short)i > obuf->bottom_margin)
		obuf->bottom_margin = (short)i;
	}
	if (parsedtag_get_value(tag, ATTR_HSEQ, &hseq)) {
	    obuf->input_alt.hseq = hseq;
	}
	if (parsedtag_get_value(tag, ATTR_FID, &i)) {
	    obuf->input_alt.fid = i;
	}
	if (parsedtag_get_value(tag, ATTR_TYPE, &p)) {
	    obuf->input_alt.type = Strnew_charp(p);
	}
	if (parsedtag_get_value(tag, ATTR_VALUE, &p)) {
	    obuf->input_alt.value = Strnew_charp(p);
	}
	if (parsedtag_get_value(tag, ATTR_NAME, &p)) {
	    obuf->input_alt.name = Strnew_charp(p);
	}
	obuf->input_alt.in = 1;
	return 0;
    case HTML_N_INPUT_ALT:
	if (obuf->input_alt.in) {
	    if (!close_effect0(obuf, HTML_INPUT_ALT))
		push_tag(obuf, "</input_alt>", HTML_N_INPUT_ALT);
	    obuf->input_alt.hseq = 0;
	    obuf->input_alt.fid = -1;
	    obuf->input_alt.in = 0;
	    obuf->input_alt.type = NULL;
	    obuf->input_alt.name = NULL;
	    obuf->input_alt.value = NULL;
	}
	return 1;
    case HTML_TABLE:
	close_anchor(h_env, obuf);
	obuf->table_level++;
	if (obuf->table_level >= MAX_TABLE)
	    break;
	w = BORDER_NONE;
	/* x: cellspacing, y: cellpadding */
	x = 2;
	y = 1;
	z = 0;
	width = 0;
	if (parsedtag_exists(tag, ATTR_BORDER)) {
	    if (parsedtag_get_value(tag, ATTR_BORDER, &w)) {
		if (w > 2)
		    w = BORDER_THICK;
		else if (w < 0) {	/* weird */
		    w = BORDER_THIN;
		}
	    }
	    else
		w = BORDER_THIN;
	}
	if (DisplayBorders && w == BORDER_NONE)
	    w = BORDER_THIN;
	if (parsedtag_get_value(tag, ATTR_WIDTH, &i)) {
	    if (obuf->table_level == 0)
		width = REAL_WIDTH(i, h_env->limit - envs[h_env->envc].indent);
	    else
		width = RELATIVE_WIDTH(i);
	}
	if (parsedtag_exists(tag, ATTR_HBORDER))
	    w = BORDER_NOWIN;
#define MAX_CELLSPACING 1000
#define MAX_CELLPADDING 1000
#define MAX_VSPACE 1000
	parsedtag_get_value(tag, ATTR_CELLSPACING, &x);
	parsedtag_get_value(tag, ATTR_CELLPADDING, &y);
	parsedtag_get_value(tag, ATTR_VSPACE, &z);
	if (x < 0)
	    x = 0;
	if (y < 0)
	    y = 0;
	if (z < 0)
	    z = 0;
	if (x > MAX_CELLSPACING)
	    x = MAX_CELLSPACING;
	if (y > MAX_CELLPADDING)
	    y = MAX_CELLPADDING;
	if (z > MAX_VSPACE)
	    z = MAX_VSPACE;
#ifdef ID_EXT
	parsedtag_get_value(tag, ATTR_ID, &id);
#endif				/* ID_EXT */
	tables[obuf->table_level] = begin_table(w, x, y, z);
#ifdef ID_EXT
	if (id != NULL)
	    tables[obuf->table_level]->id = Strnew_charp(id);
#endif				/* ID_EXT */
	table_mode[obuf->table_level].pre_mode = 0;
	table_mode[obuf->table_level].indent_level = 0;
	table_mode[obuf->table_level].nobr_level = 0;
	table_mode[obuf->table_level].caption = 0;
	table_mode[obuf->table_level].end_tag = 0;	/* HTML_UNKNOWN */
#ifndef TABLE_EXPAND
	tables[obuf->table_level]->total_width = width;
#else
	tables[obuf->table_level]->real_width = width;
	tables[obuf->table_level]->total_width = 0;
#endif
	return 1;
    case HTML_N_TABLE:
	/* should be processed in HTMLlineproc() */
	return 1;
    case HTML_CENTER:
	CLOSE_A;
	if (!(obuf->flag & (RB_PREMODE | RB_IGNORE_P)))
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	RB_SAVE_FLAG(obuf);
	RB_SET_ALIGN(obuf, RB_CENTER);
	return 1;
    case HTML_N_CENTER:
	CLOSE_A;
	if (!(obuf->flag & RB_PREMODE))
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	RB_RESTORE_FLAG(obuf);
	return 1;
    case HTML_DIV:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P))
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	set_alignment(obuf, tag);
	return 1;
    case HTML_N_DIV:
	CLOSE_A;
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	RB_RESTORE_FLAG(obuf);
	return 1;
    case HTML_DIV_INT:
	CLOSE_P;
	if (!(obuf->flag & RB_IGNORE_P))
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	set_alignment(obuf, tag);
	return 1;
    case HTML_N_DIV_INT:
	CLOSE_P;
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	RB_RESTORE_FLAG(obuf);
	return 1;
    case HTML_FORM:
	CLOSE_A;
	if (!(obuf->flag & RB_IGNORE_P))
	    flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	tmp = process_form(tag);
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_N_FORM:
	CLOSE_A;
	flushline(h_env, obuf, envs[h_env->envc].indent, 0, h_env->limit);
	obuf->flag |= RB_IGNORE_P;
	process_n_form();
	return 1;
    case HTML_INPUT:
	close_anchor(h_env, obuf);
	tmp = process_input(tag);
       if (tmp)
           HTMLlineproc1(tmp->ptr, h_env);
       return 1;
    case HTML_BUTTON:
       HTML5_CLOSE_A;
       tmp = process_button(tag);
       if (tmp)
           HTMLlineproc1(tmp->ptr, h_env);
       return 1;
    case HTML_N_BUTTON:
       tmp = process_n_button();
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_SELECT:
	close_anchor(h_env, obuf);
	tmp = process_select(tag);
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	obuf->flag |= RB_INSELECT;
	obuf->end_tag = HTML_N_SELECT;
	return 1;
    case HTML_N_SELECT:
	obuf->flag &= ~RB_INSELECT;
	obuf->end_tag = 0;
	tmp = process_n_select();
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_OPTION:
	/* nothing */
	return 1;
    case HTML_TEXTAREA:
	close_anchor(h_env, obuf);
	tmp = process_textarea(tag, h_env->limit);
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	obuf->flag |= RB_INTXTA;
	obuf->end_tag = HTML_N_TEXTAREA;
	return 1;
    case HTML_N_TEXTAREA:
	obuf->flag &= ~RB_INTXTA;
	obuf->end_tag = 0;
	tmp = process_n_textarea();
	if (tmp)
	    HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_ISINDEX:
	p = "";
	q = "!CURRENT_URL!";
	parsedtag_get_value(tag, ATTR_PROMPT, &p);
	parsedtag_get_value(tag, ATTR_ACTION, &q);
	tmp = Strnew_m_charp("<form method=get action=\"",
			     html_quote(q),
			     "\">",
			     html_quote(p),
			     "<input type=text name=\"\" accept></form>",
			     NULL);
	HTMLlineproc1(tmp->ptr, h_env);
	return 1;
    case HTML_DOCTYPE:
	if (!parsedtag_exists(tag, ATTR_PUBLIC)) {
	    obuf->flag |= RB_HTML5;
	}
	return 1;
    case HTML_META:
	p = q = r = NULL;
	parsedtag_get_value(tag, ATTR_HTTP_EQUIV, &p);
	parsedtag_get_value(tag, ATTR_CONTENT, &q);
#ifdef USE_M17N
	parsedtag_get_value(tag, ATTR_CHARSET, &r);
	if (r) {
	    /* <meta charset=""> */
	    SKIP_BLANKS(r);
	    meta_charset = wc_guess_charset(r, 0);
	}
	else
	if (p && q && !strcasecmp(p, "Content-Type") &&
	    (q = strcasestr(q, "charset")) != NULL) {
	    q += 7;
	    SKIP_BLANKS(q);
	    if (*q == '=') {
		q++;
		SKIP_BLANKS(q);
		meta_charset = wc_guess_charset(q, 0);
	    }
	}
	else
#endif
	if (p && q && !strcasecmp(p, "refresh")) {
	    int refresh_interval;
	    tmp = NULL;
	    refresh_interval = getMetaRefreshParam(q, &tmp);
	    if (tmp) {
		q = html_quote(tmp->ptr);
		tmp = Sprintf("Refresh (%d sec) <a href=\"%s\">%s</a>",
			      refresh_interval, q, q);
	    }
	    else if (refresh_interval > 0)
		tmp = Sprintf("Refresh (%d sec)", refresh_interval);
	    if (tmp) {
		HTMLlineproc1(tmp->ptr, h_env);
		do_blankline(h_env, obuf, envs[h_env->envc].indent, 0,
			     h_env->limit);
		if (!is_redisplay &&
		    !((obuf->flag & RB_NOFRAMES) && RenderFrame)) {
		    tag->need_reconstruct = TRUE;
		    return 0;
		}
	    }
	}
	return 1;
    case HTML_BASE:
#if defined(USE_M17N) || defined(USE_IMAGE)
	p = NULL;
	if (parsedtag_get_value(tag, ATTR_HREF, &p)) {
	    cur_baseURL = New(ParsedURL);
	    parseURL(p, cur_baseURL, NULL);
	}
#endif
    case HTML_MAP:
    case HTML_N_MAP:
    case HTML_AREA:
	return 0;
    case HTML_DEL:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    obuf->flag |= RB_DEL;
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>[DEL:</U>", h_env);
	    break;
	case DISPLAY_INS_DEL_FONTIFY:
	    obuf->in_strike++;
	    if (obuf->in_strike == 1) {
		push_tag(obuf, "<s>", HTML_S);
	    }
	    break;
	}
	return 1;
    case HTML_N_DEL:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    obuf->flag &= ~RB_DEL;
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>:DEL]</U>", h_env);
	case DISPLAY_INS_DEL_FONTIFY:
	    if (obuf->in_strike == 0)
		return 1;
	    if (obuf->in_strike == 1 && close_effect0(obuf, HTML_S))
		obuf->in_strike = 0;
	    if (obuf->in_strike > 0) {
		obuf->in_strike--;
		if (obuf->in_strike == 0) {
		    push_tag(obuf, "</s>", HTML_N_S);
		}
	    }
	    break;
	}
	return 1;
    case HTML_S:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    obuf->flag |= RB_S;
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>[S:</U>", h_env);
	    break;
	case DISPLAY_INS_DEL_FONTIFY:
	    obuf->in_strike++;
	    if (obuf->in_strike == 1) {
		push_tag(obuf, "<s>", HTML_S);
	    }
	    break;
	}
	return 1;
    case HTML_N_S:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    obuf->flag &= ~RB_S;
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>:S]</U>", h_env);
	    break;
	case DISPLAY_INS_DEL_FONTIFY:
	    if (obuf->in_strike == 0)
		return 1;
	    if (obuf->in_strike == 1 && close_effect0(obuf, HTML_S))
		obuf->in_strike = 0;
	    if (obuf->in_strike > 0) {
		obuf->in_strike--;
		if (obuf->in_strike == 0) {
		    push_tag(obuf, "</s>", HTML_N_S);
		}
	    }
	}
	return 1;
    case HTML_INS:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>[INS:</U>", h_env);
	    break;
	case DISPLAY_INS_DEL_FONTIFY:
	    obuf->in_ins++;
	    if (obuf->in_ins == 1) {
		push_tag(obuf, "<ins>", HTML_INS);
	    }
	    break;
	}
	return 1;
    case HTML_N_INS:
	switch (displayInsDel) {
	case DISPLAY_INS_DEL_SIMPLE:
	    break;
	case DISPLAY_INS_DEL_NORMAL:
	    HTMLlineproc1("<U>:INS]</U>", h_env);
	    break;
	case DISPLAY_INS_DEL_FONTIFY:
	    if (obuf->in_ins == 0)
		return 1;
	    if (obuf->in_ins == 1 && close_effect0(obuf, HTML_INS))
		obuf->in_ins = 0;
	    if (obuf->in_ins > 0) {
		obuf->in_ins--;
		if (obuf->in_ins == 0) {
		    push_tag(obuf, "</ins>", HTML_N_INS);
		}
	    }
	    break;
	}
	return 1;
    case HTML_SUP:
	if (!(obuf->flag & (RB_DEL | RB_S)))
	    HTMLlineproc1("^", h_env);
	return 1;
    case HTML_N_SUP:
	return 1;
    case HTML_SUB:
	if (!(obuf->flag & (RB_DEL | RB_S)))
	    HTMLlineproc1("[", h_env);
	return 1;
    case HTML_N_SUB:
	if (!(obuf->flag & (RB_DEL | RB_S)))
	    HTMLlineproc1("]", h_env);
	return 1;
    case HTML_FONT:
    case HTML_N_FONT:
    case HTML_NOP:
	return 1;
    case HTML_BGSOUND:
	if (view_unseenobject) {
	    if (parsedtag_get_value(tag, ATTR_SRC, &p)) {
		Str s;
		q = html_quote(p);
		s = Sprintf("<A HREF=\"%s\">bgsound(%s)</A>", q, q);
		HTMLlineproc1(s->ptr, h_env);
	    }
	}
	return 1;
    case HTML_EMBED:
	HTML5_CLOSE_A;
	if (view_unseenobject) {
	    if (parsedtag_get_value(tag, ATTR_SRC, &p)) {
		Str s;
		q = html_quote(p);
		s = Sprintf("<A HREF=\"%s\">embed(%s)</A>", q, q);
		HTMLlineproc1(s->ptr, h_env);
	    }
	}
	return 1;
    case HTML_APPLET:
	if (view_unseenobject) {
	    if (parsedtag_get_value(tag, ATTR_ARCHIVE, &p)) {
		Str s;
		q = html_quote(p);
		s = Sprintf("<A HREF=\"%s\">applet archive(%s)</A>", q, q);
		HTMLlineproc1(s->ptr, h_env);
	    }
	}
	return 1;
    case HTML_BODY:
	if (view_unseenobject) {
	    if (parsedtag_get_value(tag, ATTR_BACKGROUND, &p)) {
		Str s;
		q = html_quote(p);
		s = Sprintf("<IMG SRC=\"%s\" ALT=\"bg image(%s)\"><BR>", q, q);
		HTMLlineproc1(s->ptr, h_env);
	    }
	}
    case HTML_N_HEAD:
	if (obuf->flag & RB_TITLE)
	    HTMLlineproc1("</title>", h_env);
    case HTML_HEAD:
    case HTML_N_BODY:
	return 1;
    default:
	/* obuf->prevchar = '\0'; */
	return 0;
    }
    /* not reached */
    return 0;
}

#define PPUSH(p,c) {outp[pos]=(p);outc[pos]=(c);pos++;}
#define PSIZE	\
    if (out_size <= pos + 1) {	\
	out_size = pos * 3 / 2;	\
	outc = New_Reuse(char, outc, out_size);	\
	outp = New_Reuse(Lineprop, outp, out_size);	\
    }

static TextLineListItem *_tl_lp2;

static Str
textlist_feed()
{
    TextLine *p;
    if (_tl_lp2 != NULL) {
	p = _tl_lp2->ptr;
	_tl_lp2 = _tl_lp2->next;
	return p->line;
    }
    return NULL;
}

static int
ex_efct(int ex)
{
    int effect = 0;

    if (! ex)
	return 0;

    if (ex & PE_EX_ITALIC)
	effect |= PE_EX_ITALIC_E;

    if (ex & PE_EX_INSERT)
	effect |= PE_EX_INSERT_E;

    if (ex & PE_EX_STRIKE)
	effect |= PE_EX_STRIKE_E;

    return effect;
}

static void
HTMLlineproc2body(Buffer *buf, Str (*feed) (), int llimit)
{
    static char *outc = NULL;
    static Lineprop *outp = NULL;
    static int out_size = 0;
    Anchor *a_href = NULL, *a_img = NULL, *a_form = NULL;
    char *p, *q, *r, *s, *t, *str;
    Lineprop mode, effect, ex_effect;
    int pos;
    int nlines;
#ifdef DEBUG
    FILE *debug = NULL;
#endif
    struct frameset *frameset_s[FRAMESTACK_SIZE];
    int frameset_sp = -1;
    union frameset_element *idFrame = NULL;
    char *id = NULL;
    int hseq, form_id;
    Str line;
    char *endp;
    char symbol = '\0';
    int internal = 0;
    Anchor **a_textarea = NULL;
#ifdef MENU_SELECT
    Anchor **a_select = NULL;
#endif
#if defined(USE_M17N) || defined(USE_IMAGE)
    ParsedURL *base = baseURL(buf);
#endif
#ifdef USE_M17N
    wc_ces name_charset = url_to_charset(NULL, &buf->currentURL,
					 buf->document_charset);
#endif

    if (out_size == 0) {
	out_size = LINELEN;
	outc = NewAtom_N(char, out_size);
	outp = NewAtom_N(Lineprop, out_size);
    }

    n_textarea = -1;
    if (!max_textarea) {	/* halfload */
	max_textarea = MAX_TEXTAREA;
	textarea_str = New_N(Str, max_textarea);
	a_textarea = New_N(Anchor *, max_textarea);
    }
#ifdef MENU_SELECT
    n_select = -1;
    if (!max_select) {		/* halfload */
	max_select = MAX_SELECT;
	select_option = New_N(FormSelectOption, max_select);
	a_select = New_N(Anchor *, max_select);
    }
#endif

#ifdef DEBUG
    if (w3m_debug)
	debug = fopen("zzzerr", "a");
#endif

    effect = 0;
    ex_effect = 0;
    nlines = 0;
    while ((line = feed()) != NULL) {
#ifdef DEBUG
	if (w3m_debug) {
	    Strfputs(line, debug);
	    fputc('\n', debug);
	}
#endif
	if (n_textarea >= 0 && *(line->ptr) != '<') {	/* halfload */
	    Strcat(textarea_str[n_textarea], line);
	    continue;
	}
      proc_again:
	if (++nlines == llimit)
	    break;
	pos = 0;
#ifdef ENABLE_REMOVE_TRAILINGSPACES
	Strremovetrailingspaces(line);
#endif
	str = line->ptr;
	endp = str + line->length;
	while (str < endp) {
	    PSIZE;
	    mode = get_mctype(str);
	    if ((effect | ex_efct(ex_effect)) & PC_SYMBOL && *str != '<') {
#ifdef USE_M17N
		char **buf = set_symbol(symbol_width0);
		int len;

		p = buf[(int)symbol];
		len = get_mclen(p);
		mode = get_mctype(p);
		PPUSH(mode | effect | ex_efct(ex_effect), *(p++));
		if (--len) {
		    mode = (mode & ~PC_WCHAR1) | PC_WCHAR2;
		    while (len--) {
			PSIZE;
			PPUSH(mode | effect | ex_efct(ex_effect), *(p++));
		    }
		}
#else
		PPUSH(PC_ASCII | effect | ex_efct(ex_effect), SYMBOL_BASE + symbol);
#endif
		str += symbol_width;
	    }
#ifdef USE_M17N
	    else if (mode == PC_CTRL || mode == PC_UNDEF) {
#else
	    else if (mode == PC_CTRL || IS_INTSPACE(*str)) {
#endif
		PPUSH(PC_ASCII | effect | ex_efct(ex_effect), ' ');
		str++;
	    }
#ifdef USE_M17N
	    else if (mode & PC_UNKNOWN) {
		PPUSH(PC_ASCII | effect | ex_efct(ex_effect), ' ');
		str += get_mclen(str);
	    }
#endif
	    else if (*str != '<' && *str != '&') {
#ifdef USE_M17N
		int len = get_mclen(str);
#endif
		PPUSH(mode | effect | ex_efct(ex_effect), *(str++));
#ifdef USE_M17N
		if (--len) {
		    mode = (mode & ~PC_WCHAR1) | PC_WCHAR2;
		    while (len--) {
			PSIZE;
			PPUSH(mode | effect | ex_efct(ex_effect), *(str++));
		    }
		}
#endif
	    }
	    else if (*str == '&') {
		/* 
		 * & escape processing
		 */
		p = getescapecmd(&str);
		while (*p) {
		    PSIZE;
		    mode = get_mctype((unsigned char *)p);
#ifdef USE_M17N
		    if (mode == PC_CTRL || mode == PC_UNDEF) {
#else
		    if (mode == PC_CTRL || IS_INTSPACE(*str)) {
#endif
			PPUSH(PC_ASCII | effect | ex_efct(ex_effect), ' ');
			p++;
		    }
#ifdef USE_M17N
		    else if (mode & PC_UNKNOWN) {
			PPUSH(PC_ASCII | effect | ex_efct(ex_effect), ' ');
			p += get_mclen(p);
		    }
#endif
		    else {
#ifdef USE_M17N
			int len = get_mclen(p);
#endif
			PPUSH(mode | effect | ex_efct(ex_effect), *(p++));
#ifdef USE_M17N
			if (--len) {
			    mode = (mode & ~PC_WCHAR1) | PC_WCHAR2;
			    while (len--) {
				PSIZE;
				PPUSH(mode | effect | ex_efct(ex_effect), *(p++));
			    }
			}
#endif
		    }
		}
	    }
	    else {
		/* tag processing */
		struct parsed_tag *tag;
		if (!(tag = parse_tag(&str, TRUE)))
		    continue;
		switch (tag->tagid) {
		case HTML_B:
		    effect |= PE_BOLD;
		    break;
		case HTML_N_B:
		    effect &= ~PE_BOLD;
		    break;
		case HTML_I:
		    ex_effect |= PE_EX_ITALIC;
		    break;
		case HTML_N_I:
		    ex_effect &= ~PE_EX_ITALIC;
		    break;
		case HTML_INS:
		    ex_effect |= PE_EX_INSERT;
		    break;
		case HTML_N_INS:
		    ex_effect &= ~PE_EX_INSERT;
		    break;
		case HTML_U:
		    effect |= PE_UNDER;
		    break;
		case HTML_N_U:
		    effect &= ~PE_UNDER;
		    break;
		case HTML_S:
		    ex_effect |= PE_EX_STRIKE;
		    break;
		case HTML_N_S:
		    ex_effect &= ~PE_EX_STRIKE;
		    break;
		case HTML_A:
		    if (renderFrameSet &&
			parsedtag_get_value(tag, ATTR_FRAMENAME, &p)) {
			p = url_quote_conv(p, buf->document_charset);
			if (!idFrame || strcmp(idFrame->body->name, p)) {
			    idFrame = search_frame(renderFrameSet, p);
			    if (idFrame && idFrame->body->attr != F_BODY)
				idFrame = NULL;
			}
		    }
		    p = r = s = NULL;
		    q = buf->baseTarget;
		    t = "";
		    hseq = 0;
		    id = NULL;
		    if (parsedtag_get_value(tag, ATTR_NAME, &id)) {
			id = url_quote_conv(id, name_charset);
			registerName(buf, id, currentLn(buf), pos);
		    }
		    if (parsedtag_get_value(tag, ATTR_HREF, &p))
			p = url_encode(remove_space(p), base,
				       buf->document_charset);
		    if (parsedtag_get_value(tag, ATTR_TARGET, &q))
			q = url_quote_conv(q, buf->document_charset);
		    if (parsedtag_get_value(tag, ATTR_REFERER, &r))
			r = url_encode(r, base,
				       buf->document_charset);
		    parsedtag_get_value(tag, ATTR_TITLE, &s);
		    parsedtag_get_value(tag, ATTR_ACCESSKEY, &t);
		    parsedtag_get_value(tag, ATTR_HSEQ, &hseq);
		    if (hseq > 0)
			buf->hmarklist =
			    putHmarker(buf->hmarklist, currentLn(buf),
				       pos, hseq - 1);
		    else if (hseq < 0) {
			int h = -hseq - 1;
			if (buf->hmarklist &&
			    h < buf->hmarklist->nmark &&
			    buf->hmarklist->marks[h].invalid) {
			    buf->hmarklist->marks[h].pos = pos;
			    buf->hmarklist->marks[h].line = currentLn(buf);
			    buf->hmarklist->marks[h].invalid = 0;
			    hseq = -hseq;
			}
		    }
		    if (id && idFrame)
			idFrame->body->nameList =
			    putAnchor(idFrame->body->nameList, id, NULL,
				      (Anchor **)NULL, NULL, NULL, '\0',
				      currentLn(buf), pos);
		    if (p) {
			effect |= PE_ANCHOR;
			a_href = registerHref(buf, p, q, r, s,
					      *t, currentLn(buf), pos);
			a_href->hseq = ((hseq > 0) ? hseq : -hseq) - 1;
			a_href->slave = (hseq > 0) ? FALSE : TRUE;
		    }
		    break;
		case HTML_N_A:
		    effect &= ~PE_ANCHOR;
		    if (a_href) {
			a_href->end.line = currentLn(buf);
			a_href->end.pos = pos;
			if (a_href->start.line == a_href->end.line &&
			    a_href->start.pos == a_href->end.pos) {
			    if (buf->hmarklist && a_href->hseq >= 0 &&
				a_href->hseq < buf->hmarklist->nmark)
				buf->hmarklist->marks[a_href->hseq].invalid = 1;
			    a_href->hseq = -1;
			}
			a_href = NULL;
		    }
		    break;

		case HTML_LINK:
		    addLink(buf, tag);
		    break;

		case HTML_IMG_ALT:
		    if (parsedtag_get_value(tag, ATTR_SRC, &p)) {
#ifdef USE_IMAGE
			int w = -1, h = -1, iseq = 0, ismap = 0;
			int xoffset = 0, yoffset = 0, top = 0, bottom = 0;
			parsedtag_get_value(tag, ATTR_HSEQ, &iseq);
			parsedtag_get_value(tag, ATTR_WIDTH, &w);
			parsedtag_get_value(tag, ATTR_HEIGHT, &h);
			parsedtag_get_value(tag, ATTR_XOFFSET, &xoffset);
			parsedtag_get_value(tag, ATTR_YOFFSET, &yoffset);
			parsedtag_get_value(tag, ATTR_TOP_MARGIN, &top);
			parsedtag_get_value(tag, ATTR_BOTTOM_MARGIN, &bottom);
			if (parsedtag_exists(tag, ATTR_ISMAP))
			    ismap = 1;
			q = NULL;
			parsedtag_get_value(tag, ATTR_USEMAP, &q);
			if (iseq > 0) {
			    buf->imarklist = putHmarker(buf->imarklist,
							currentLn(buf), pos,
							iseq - 1);
			}
#endif
			s = NULL;
			parsedtag_get_value(tag, ATTR_TITLE, &s);
			p = url_quote_conv(remove_space(p),
					   buf->document_charset);
			a_img = registerImg(buf, p, s, currentLn(buf), pos);
#ifdef USE_IMAGE
			a_img->hseq = iseq;
			a_img->image = NULL;
			if (iseq > 0) {
			    ParsedURL u;
			    Image *image;

			    parseURL2(a_img->url, &u, base);
			    a_img->image = image = New(Image);
			    image->url = parsedURL2Str(&u)->ptr;
			    if (!uncompressed_file_type(u.file, &image->ext))
				image->ext = filename_extension(u.file, TRUE);
			    image->cache = NULL;
			    image->width =
				(w > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : w;
			    image->height =
				(h > MAX_IMAGE_SIZE) ? MAX_IMAGE_SIZE : h;
			    image->xoffset = xoffset;
			    image->yoffset = yoffset;
			    image->y = currentLn(buf) - top;
			    if (image->xoffset < 0 && pos == 0)
				image->xoffset = 0;
			    if (image->yoffset < 0 && image->y == 1)
				image->yoffset = 0;
			    image->rows = 1 + top + bottom;
			    image->map = q;
			    image->ismap = ismap;
			    image->touch = 0;
			    image->cache = getImage(image, base,
						    IMG_FLAG_SKIP);
			}
			else if (iseq < 0) {
			    BufferPoint *po = buf->imarklist->marks - iseq - 1;
			    Anchor *a = retrieveAnchor(buf->img,
						       po->line, po->pos);
			    if (a) {
				a_img->url = a->url;
				a_img->image = a->image;
			    }
			}
#endif
		    }
		    effect |= PE_IMAGE;
		    break;
		case HTML_N_IMG_ALT:
		    effect &= ~PE_IMAGE;
		    if (a_img) {
			a_img->end.line = currentLn(buf);
			a_img->end.pos = pos;
		    }
		    a_img = NULL;
		    break;
		case HTML_INPUT_ALT:
		    {
			FormList *form;
			int top = 0, bottom = 0;
			int textareanumber = -1;
#ifdef MENU_SELECT
			int selectnumber = -1;
#endif
			hseq = 0;
			form_id = -1;

			parsedtag_get_value(tag, ATTR_HSEQ, &hseq);
			parsedtag_get_value(tag, ATTR_FID, &form_id);
			parsedtag_get_value(tag, ATTR_TOP_MARGIN, &top);
			parsedtag_get_value(tag, ATTR_BOTTOM_MARGIN, &bottom);
			if (form_id < 0 || form_id > form_max ||
			    forms == NULL || forms[form_id] == NULL)
			    break;	/* outside of <form>..</form> */
			form = forms[form_id];
			if (hseq > 0) {
			    int hpos = pos;
			    if (*str == '[')
				hpos++;
			    buf->hmarklist =
				putHmarker(buf->hmarklist, currentLn(buf),
					   hpos, hseq - 1);
			}
			else if (hseq < 0) {
			    int h = -hseq - 1;
			    int hpos = pos;
			    if (*str == '[')
				hpos++;
			    if (buf->hmarklist &&
				h < buf->hmarklist->nmark &&
				buf->hmarklist->marks[h].invalid) {
				buf->hmarklist->marks[h].pos = hpos;
				buf->hmarklist->marks[h].line = currentLn(buf);
				buf->hmarklist->marks[h].invalid = 0;
				hseq = -hseq;
			    }
			}

			if (!form->target)
			    form->target = buf->baseTarget;
			if (a_textarea &&
			    parsedtag_get_value(tag, ATTR_TEXTAREANUMBER,
						&textareanumber)) {
			    if (textareanumber >= max_textarea) {
				max_textarea = 2 * textareanumber;
				textarea_str = New_Reuse(Str, textarea_str,
							 max_textarea);
				a_textarea = New_Reuse(Anchor *, a_textarea,
						       max_textarea);
			    }
			}
#ifdef MENU_SELECT
			if (a_select &&
			    parsedtag_get_value(tag, ATTR_SELECTNUMBER,
						&selectnumber)) {
			    if (selectnumber >= max_select) {
				max_select = 2 * selectnumber;
				select_option = New_Reuse(FormSelectOption,
							  select_option,
							  max_select);
				a_select = New_Reuse(Anchor *, a_select,
						     max_select);
			    }
			}
#endif
			a_form =
			    registerForm(buf, form, tag, currentLn(buf), pos);
			if (a_textarea && textareanumber >= 0)
			    a_textarea[textareanumber] = a_form;
#ifdef MENU_SELECT
			if (a_select && selectnumber >= 0)
			    a_select[selectnumber] = a_form;
#endif
			if (a_form) {
			    a_form->hseq = hseq - 1;
			    a_form->y = currentLn(buf) - top;
			    a_form->rows = 1 + top + bottom;
			    if (!parsedtag_exists(tag, ATTR_NO_EFFECT))
				effect |= PE_FORM;
			    break;
			}
		    }
		case HTML_N_INPUT_ALT:
		    effect &= ~PE_FORM;
		    if (a_form) {
			a_form->end.line = currentLn(buf);
			a_form->end.pos = pos;
			if (a_form->start.line == a_form->end.line &&
			    a_form->start.pos == a_form->end.pos)
			    a_form->hseq = -1;
		    }
		    a_form = NULL;
		    break;
		case HTML_MAP:
		    if (parsedtag_get_value(tag, ATTR_NAME, &p)) {
			MapList *m = New(MapList);
			m->name = Strnew_charp(p);
			m->area = newGeneralList();
			m->next = buf->maplist;
			buf->maplist = m;
		    }
		    break;
		case HTML_N_MAP:
		    /* nothing to do */
		    break;
		case HTML_AREA:
		    if (buf->maplist == NULL)	/* outside of <map>..</map> */
			break;
		    if (parsedtag_get_value(tag, ATTR_HREF, &p)) {
			MapArea *a;
			p = url_encode(remove_space(p), base,
				       buf->document_charset);
			t = NULL;
			parsedtag_get_value(tag, ATTR_TARGET, &t);
			q = "";
			parsedtag_get_value(tag, ATTR_ALT, &q);
			r = NULL;
			s = NULL;
#ifdef USE_IMAGE
			parsedtag_get_value(tag, ATTR_SHAPE, &r);
			parsedtag_get_value(tag, ATTR_COORDS, &s);
#endif
			a = newMapArea(p, t, q, r, s);
			pushValue(buf->maplist->area, (void *)a);
		    }
		    break;
		case HTML_FRAMESET:
		    frameset_sp++;
		    if (frameset_sp >= FRAMESTACK_SIZE)
			break;
		    frameset_s[frameset_sp] = newFrameSet(tag);
		    if (frameset_s[frameset_sp] == NULL)
			break;
		    if (frameset_sp == 0) {
			if (buf->frameset == NULL) {
			    buf->frameset = frameset_s[frameset_sp];
			}
			else
			    pushFrameTree(&(buf->frameQ),
					  frameset_s[frameset_sp], NULL);
		    }
		    else
			addFrameSetElement(frameset_s[frameset_sp - 1],
					   *(union frameset_element *)
					   &frameset_s[frameset_sp]);
		    break;
		case HTML_N_FRAMESET:
		    if (frameset_sp >= 0)
			frameset_sp--;
		    break;
		case HTML_FRAME:
		    if (frameset_sp >= 0 && frameset_sp < FRAMESTACK_SIZE) {
			union frameset_element element;

			element.body = newFrame(tag, buf);
			addFrameSetElement(frameset_s[frameset_sp], element);
		    }
		    break;
		case HTML_BASE:
		    if (parsedtag_get_value(tag, ATTR_HREF, &p)) {
			p = url_encode(remove_space(p), NULL,
				       buf->document_charset);
			if (!buf->baseURL)
			    buf->baseURL = New(ParsedURL);
			parseURL2(p, buf->baseURL, &buf->currentURL);
#if defined(USE_M17N) || defined(USE_IMAGE)
			base = buf->baseURL;
#endif
		    }
		    if (parsedtag_get_value(tag, ATTR_TARGET, &p))
			buf->baseTarget =
			    url_quote_conv(p, buf->document_charset);
		    break;
		case HTML_META:
		    p = q = NULL;
		    parsedtag_get_value(tag, ATTR_HTTP_EQUIV, &p);
		    parsedtag_get_value(tag, ATTR_CONTENT, &q);
		    if (p && q && !strcasecmp(p, "refresh") && MetaRefresh) {
			Str tmp = NULL;
			int refresh_interval = getMetaRefreshParam(q, &tmp);
#ifdef USE_ALARM
			if (tmp) {
			    p = url_encode(remove_space(tmp->ptr), base,
					   buf->document_charset);
			    buf->event = setAlarmEvent(buf->event,
						       refresh_interval,
						       AL_IMPLICIT_ONCE,
						       FUNCNAME_gorURL, p);
			}
			else if (refresh_interval > 0)
			    buf->event = setAlarmEvent(buf->event,
						       refresh_interval,
						       AL_IMPLICIT,
						       FUNCNAME_reload, NULL);
#else
			if (tmp && refresh_interval == 0) {
			    p = url_encode(remove_space(tmp->ptr), base,
					   buf->document_charset);
			    pushEvent(FUNCNAME_gorURL, p);
			}
#endif
		    }
		    break;
		case HTML_INTERNAL:
		    internal = HTML_INTERNAL;
		    break;
		case HTML_N_INTERNAL:
		    internal = HTML_N_INTERNAL;
		    break;
		case HTML_FORM_INT:
		    if (parsedtag_get_value(tag, ATTR_FID, &form_id))
			process_form_int(tag, form_id);
		    break;
		case HTML_TEXTAREA_INT:
		    if (parsedtag_get_value(tag, ATTR_TEXTAREANUMBER,
					    &n_textarea)
			&& n_textarea >= 0 && n_textarea < max_textarea) {
			textarea_str[n_textarea] = Strnew();
		    }
		    else
			n_textarea = -1;
		    break;
		case HTML_N_TEXTAREA_INT:
		    if (a_textarea && n_textarea >= 0) {
			FormItemList *item =
			    (FormItemList *)a_textarea[n_textarea]->url;
			item->init_value = item->value =
			    textarea_str[n_textarea];
		    }
		    break;
#ifdef MENU_SELECT
		case HTML_SELECT_INT:
		    if (parsedtag_get_value(tag, ATTR_SELECTNUMBER, &n_select)
			&& n_select >= 0 && n_select < max_select) {
			select_option[n_select].first = NULL;
			select_option[n_select].last = NULL;
		    }
		    else
			n_select = -1;
		    break;
		case HTML_N_SELECT_INT:
		    if (a_select && n_select >= 0) {
			FormItemList *item =
			    (FormItemList *)a_select[n_select]->url;
			item->select_option = select_option[n_select].first;
			chooseSelectOption(item, item->select_option);
			item->init_selected = item->selected;
			item->init_value = item->value;
			item->init_label = item->label;
		    }
		    break;
		case HTML_OPTION_INT:
		    if (n_select >= 0) {
			int selected;
			q = "";
			parsedtag_get_value(tag, ATTR_LABEL, &q);
			p = q;
			parsedtag_get_value(tag, ATTR_VALUE, &p);
			selected = parsedtag_exists(tag, ATTR_SELECTED);
			addSelectOption(&select_option[n_select],
					Strnew_charp(p), Strnew_charp(q),
					selected);
		    }
		    break;
#endif
		case HTML_TITLE_ALT:
		    if (parsedtag_get_value(tag, ATTR_TITLE, &p))
			buf->buffername = html_unquote(p);
		    break;
		case HTML_SYMBOL:
		    effect |= PC_SYMBOL;
		    if (parsedtag_get_value(tag, ATTR_TYPE, &p))
			symbol = (char)atoi(p);
		    break;
		case HTML_N_SYMBOL:
		    effect &= ~PC_SYMBOL;
		    break;
		}
#ifdef	ID_EXT
		id = NULL;
		if (parsedtag_get_value(tag, ATTR_ID, &id)) {
		    id = url_quote_conv(id, name_charset);
		    registerName(buf, id, currentLn(buf), pos);
		}
		if (renderFrameSet &&
		    parsedtag_get_value(tag, ATTR_FRAMENAME, &p)) {
		    p = url_quote_conv(p, buf->document_charset);
		    if (!idFrame || strcmp(idFrame->body->name, p)) {
			idFrame = search_frame(renderFrameSet, p);
			if (idFrame && idFrame->body->attr != F_BODY)
			    idFrame = NULL;
		    }
		}
		if (id && idFrame)
		    idFrame->body->nameList =
			putAnchor(idFrame->body->nameList, id, NULL,
				  (Anchor **)NULL, NULL, NULL, '\0',
				  currentLn(buf), pos);
#endif				/* ID_EXT */
	    }
	}
	/* end of processing for one line */
	if (!internal)
	    addnewline(buf, outc, outp, NULL, pos, -1, nlines);
	if (internal == HTML_N_INTERNAL)
	    internal = 0;
	if (str != endp) {
	    line = Strsubstr(line, str - line->ptr, endp - str);
	    goto proc_again;
	}
    }
#ifdef DEBUG
    if (w3m_debug)
	fclose(debug);
#endif
    for (form_id = 1; form_id <= form_max; form_id++)
	if (forms[form_id])
	    forms[form_id]->next = forms[form_id - 1];
    buf->formlist = (form_max >= 0) ? forms[form_max] : NULL;
    if (n_textarea)
	addMultirowsForm(buf, buf->formitem);
#ifdef USE_IMAGE
    addMultirowsImg(buf, buf->img);
#endif
}

static void
addLink(Buffer *buf, struct parsed_tag *tag)
{
    char *href = NULL, *title = NULL, *ctype = NULL, *rel = NULL, *rev = NULL;
    char type = LINK_TYPE_NONE;
    LinkList *l;

    parsedtag_get_value(tag, ATTR_HREF, &href);
    if (href)
	href = url_encode(remove_space(href), baseURL(buf),
			  buf->document_charset);
    parsedtag_get_value(tag, ATTR_TITLE, &title);
    parsedtag_get_value(tag, ATTR_TYPE, &ctype);
    parsedtag_get_value(tag, ATTR_REL, &rel);
    if (rel != NULL) {
	/* forward link type */
	type = LINK_TYPE_REL;
	if (title == NULL)
	    title = rel;
    }
    parsedtag_get_value(tag, ATTR_REV, &rev);
    if (rev != NULL) {
	/* reverse link type */
	type = LINK_TYPE_REV;
	if (title == NULL)
	    title = rev;
    }

    l = New(LinkList);
    l->url = href;
    l->title = title;
    l->ctype = ctype;
    l->type = type;
    l->next = NULL;
    if (buf->linklist) {
	LinkList *i;
	for (i = buf->linklist; i->next; i = i->next) ;
	i->next = l;
    }
    else
	buf->linklist = l;
}

void
HTMLlineproc2(Buffer *buf, TextLineList *tl)
{
    _tl_lp2 = tl->first;
    HTMLlineproc2body(buf, textlist_feed, -1);
}

static InputStream _file_lp2;

static Str
file_feed()
{
    Str s;
    s = StrISgets(_file_lp2);
    if (s->length == 0) {
	ISclose(_file_lp2);
	return NULL;
    }
    return s;
}

void
HTMLlineproc3(Buffer *buf, InputStream stream)
{
    _file_lp2 = stream;
    HTMLlineproc2body(buf, file_feed, -1);
}

static void
proc_escape(struct readbuffer *obuf, char **str_return)
{
    char *str = *str_return, *estr;
    int ech = getescapechar(str_return);
    int width, n_add = *str_return - str;
    Lineprop mode = PC_ASCII;

    if (ech < 0) {
	*str_return = str;
	proc_mchar(obuf, obuf->flag & RB_SPECIAL, 1, str_return, PC_ASCII);
	return;
    }
    mode = IS_CNTRL(ech) ? PC_CTRL : PC_ASCII;

    estr = conv_entity(ech);
    check_breakpoint(obuf, obuf->flag & RB_SPECIAL, estr);
    width = get_strwidth(estr);
    if (width == 1 && ech == (unsigned char)*estr &&
	ech != '&' && ech != '<' && ech != '>') {
	if (IS_CNTRL(ech))
	    mode = PC_CTRL;
	push_charp(obuf, width, estr, mode);
    }
    else
	push_nchars(obuf, width, str, n_add, mode);
    set_prevchar(obuf->prevchar, estr, strlen(estr));
    obuf->prev_ctype = mode;
}


static int
need_flushline(struct html_feed_environ *h_env, struct readbuffer *obuf,
	       Lineprop mode)
{
    char ch;

    if (obuf->flag & RB_PRE_INT) {
	if (obuf->pos > h_env->limit)
	    return 1;
	else
	    return 0;
    }

    ch = Strlastchar(obuf->line);
    /* if (ch == ' ' && obuf->tag_sp > 0) */
    if (ch == ' ')
	return 0;

    if (obuf->pos > h_env->limit)
	return 1;

    return 0;
}

static int
table_width(struct html_feed_environ *h_env, int table_level)
{
    int width;
    if (table_level < 0)
	return 0;
    width = tables[table_level]->total_width;
    if (table_level > 0 || width > 0)
	return width;
    return h_env->limit - h_env->envs[h_env->envc].indent;
}

/* HTML processing first pass */
void
HTMLlineproc0(char *line, struct html_feed_environ *h_env, int internal)
{
    Lineprop mode;
    int cmd;
    struct readbuffer *obuf = h_env->obuf;
    int indent, delta;
    struct parsed_tag *tag;
    Str tokbuf;
    struct table *tbl = NULL;
    struct table_mode *tbl_mode = NULL;
    int tbl_width = 0;
#ifdef USE_M17N
    int is_hangul, prev_is_hangul = 0;
#endif

#ifdef DEBUG
    if (w3m_debug) {
	FILE *f = fopen("zzzproc1", "a");
	fprintf(f, "%c%c%c%c",
		(obuf->flag & RB_PREMODE) ? 'P' : ' ',
		(obuf->table_level >= 0) ? 'T' : ' ',
		(obuf->flag & RB_INTXTA) ? 'X' : ' ',
		(obuf->flag & (RB_SCRIPT | RB_STYLE)) ? 'S' : ' ');
	fprintf(f, "HTMLlineproc1(\"%s\",%d,%lx)\n", line, h_env->limit,
		(unsigned long)h_env);
	fclose(f);
    }
#endif

    tokbuf = Strnew();

  table_start:
    if (obuf->table_level >= 0) {
	int level = min(obuf->table_level, MAX_TABLE - 1);
	tbl = tables[level];
	tbl_mode = &table_mode[level];
	tbl_width = table_width(h_env, level);
    }

    while (*line != '\0') {
	char *str, *p;
	int is_tag = FALSE;
	int pre_mode = (obuf->table_level >= 0 && tbl_mode) ?
	    tbl_mode->pre_mode : obuf->flag;
	int end_tag = (obuf->table_level >= 0 && tbl_mode) ?
	    tbl_mode->end_tag : obuf->end_tag;

	if (*line == '<' || obuf->status != R_ST_NORMAL) {
	    /* 
	     * Tag processing
	     */
	    if (obuf->status == R_ST_EOL)
		obuf->status = R_ST_NORMAL;
	    else {
		read_token(h_env->tagbuf, &line, &obuf->status,
			   pre_mode & RB_PREMODE, obuf->status != R_ST_NORMAL);
		if (obuf->status != R_ST_NORMAL)
		    return;
	    }
	    if (h_env->tagbuf->length == 0)
		continue;
	    str = Strdup(h_env->tagbuf)->ptr;
	    if (*str == '<') {
		if (str[1] && REALLY_THE_BEGINNING_OF_A_TAG(str))
		    is_tag = TRUE;
		else if (!(pre_mode & (RB_PLAIN | RB_INTXTA | RB_INSELECT |
				       RB_SCRIPT | RB_STYLE | RB_TITLE))) {
		    line = Strnew_m_charp(str + 1, line, NULL)->ptr;
		    str = "&lt;";
		}
	    }
	}
	else {
	    read_token(tokbuf, &line, &obuf->status, pre_mode & RB_PREMODE, 0);
	    if (obuf->status != R_ST_NORMAL)	/* R_ST_AMP ? */
		obuf->status = R_ST_NORMAL;
	    str = tokbuf->ptr;
	}

	if (pre_mode & (RB_PLAIN | RB_INTXTA | RB_INSELECT | RB_SCRIPT |
			RB_STYLE | RB_TITLE)) {
	    if (is_tag) {
		p = str;
		if ((tag = parse_tag(&p, internal))) {
		    if (tag->tagid == end_tag ||
			(pre_mode & RB_INSELECT && tag->tagid == HTML_N_FORM)
			|| (pre_mode & RB_TITLE
			    && (tag->tagid == HTML_N_HEAD
				|| tag->tagid == HTML_BODY)))
			goto proc_normal;
		}
	    }
	    /* title */
	    if (pre_mode & RB_TITLE) {
		feed_title(str);
		continue;
	    }
	    /* select */
	    if (pre_mode & RB_INSELECT) {
		if (obuf->table_level >= 0)
		    goto proc_normal;
		feed_select(str);
		continue;
	    }
	    if (is_tag) {
		if (strncmp(str, "<!--", 4) && (p = strchr(str + 1, '<'))) {
		    str = Strnew_charp_n(str, p - str)->ptr;
		    line = Strnew_m_charp(p, line, NULL)->ptr;
		}
		is_tag = FALSE;
	    }
	    if (obuf->table_level >= 0)
		goto proc_normal;
	    /* textarea */
	    if (pre_mode & RB_INTXTA) {
		feed_textarea(str);
		continue;
	    }
	    /* script */
	    if (pre_mode & RB_SCRIPT)
		continue;
	    /* style */
	    if (pre_mode & RB_STYLE)
		continue;
	}

      proc_normal:
	if (obuf->table_level >= 0 && tbl && tbl_mode) {
	    /* 
	     * within table: in <table>..</table>, all input tokens
	     * are fed to the table renderer, and then the renderer
	     * makes HTML output.
	     */
	    switch (feed_table(tbl, str, tbl_mode, tbl_width, internal)) {
	    case 0:
		/* </table> tag */
		obuf->table_level--;
		if (obuf->table_level >= MAX_TABLE - 1)
		    continue;
		end_table(tbl);
		if (obuf->table_level >= 0) {
		    struct table *tbl0 = tables[obuf->table_level];
		    str = Sprintf("<table_alt tid=%d>", tbl0->ntable)->ptr;
		    if (tbl0->row < 0)
			continue;
		    pushTable(tbl0, tbl);
		    tbl = tbl0;
		    tbl_mode = &table_mode[obuf->table_level];
		    tbl_width = table_width(h_env, obuf->table_level);
		    feed_table(tbl, str, tbl_mode, tbl_width, TRUE);
		    continue;
		    /* continue to the next */
		}
		if (obuf->flag & RB_DEL)
		    continue;
		/* all tables have been read */
		if (tbl->vspace > 0 && !(obuf->flag & RB_IGNORE_P)) {
		    int indent = h_env->envs[h_env->envc].indent;
		    flushline(h_env, obuf, indent, 0, h_env->limit);
		    do_blankline(h_env, obuf, indent, 0, h_env->limit);
		}
		save_fonteffect(h_env, obuf);
		initRenderTable();
		renderTable(tbl, tbl_width, h_env);
		restore_fonteffect(h_env, obuf);
		obuf->flag &= ~RB_IGNORE_P;
		if (tbl->vspace > 0) {
		    int indent = h_env->envs[h_env->envc].indent;
		    do_blankline(h_env, obuf, indent, 0, h_env->limit);
		    obuf->flag |= RB_IGNORE_P;
		}
		set_space_to_prevchar(obuf->prevchar);
		continue;
	    case 1:
		/* <table> tag */
		break;
	    default:
		continue;
	    }
	}

	if (is_tag) {
/*** Beginning of a new tag ***/
	    if ((tag = parse_tag(&str, internal)))
		cmd = tag->tagid;
	    else
		continue;
	    /* process tags */
	    if (HTMLtagproc1(tag, h_env) == 0) {
		/* preserve the tag for second-stage processing */
		if (parsedtag_need_reconstruct(tag))
		    h_env->tagbuf = parsedtag2str(tag);
		push_tag(obuf, h_env->tagbuf->ptr, cmd);
	    }
#ifdef ID_EXT
	    else {
		process_idattr(obuf, cmd, tag);
	    }
#endif				/* ID_EXT */
	    obuf->bp.init_flag = 1;
	    clear_ignore_p_flag(cmd, obuf);
	    if (cmd == HTML_TABLE)
		goto table_start;
	    else
		continue;
	}

	if (obuf->flag & (RB_DEL | RB_S))
	    continue;
	while (*str) {
	    mode = get_mctype(str);
	    delta = get_mcwidth(str);
	    if (obuf->flag & (RB_SPECIAL & ~RB_NOBR)) {
		char ch = *str;
		if (!(obuf->flag & RB_PLAIN) && (*str == '&')) {
		    char *p = str;
		    int ech = getescapechar(&p);
		    if (ech == '\n' || ech == '\r') {
			ch = '\n';
			str = p - 1;
		    }
		    else if (ech == '\t') {
			ch = '\t';
			str = p - 1;
		    }
		}
		if (ch != '\n')
		    obuf->flag &= ~RB_IGNORE_P;
		if (ch == '\n') {
		    str++;
		    if (obuf->flag & RB_IGNORE_P) {
			obuf->flag &= ~RB_IGNORE_P;
			continue;
		    }
		    if (obuf->flag & RB_PRE_INT)
			PUSH(' ');
		    else
			flushline(h_env, obuf, h_env->envs[h_env->envc].indent,
				  1, h_env->limit);
		}
		else if (ch == '\t') {
		    do {
			PUSH(' ');
		    } while ((h_env->envs[h_env->envc].indent + obuf->pos)
			     % Tabstop != 0);
		    str++;
		}
		else if (obuf->flag & RB_PLAIN) {
		    char *p = html_quote_char(*str);
		    if (p) {
			push_charp(obuf, 1, p, PC_ASCII);
			str++;
		    }
		    else {
			proc_mchar(obuf, 1, delta, &str, mode);
		    }
		}
		else {
		    if (*str == '&')
			proc_escape(obuf, &str);
		    else
			proc_mchar(obuf, 1, delta, &str, mode);
		}
		if (obuf->flag & (RB_SPECIAL & ~RB_PRE_INT))
		    continue;
	    }
	    else {
		if (!IS_SPACE(*str))
		    obuf->flag &= ~RB_IGNORE_P;
		if ((mode == PC_ASCII || mode == PC_CTRL) && IS_SPACE(*str)) {
		    if (*obuf->prevchar->ptr != ' ') {
			PUSH(' ');
		    }
		    str++;
		}
		else {
#ifdef USE_M17N
		    if (mode == PC_KANJI1)
			is_hangul = wtf_is_hangul((wc_uchar *) str);
		    else
			is_hangul = 0;
		    if (!SimplePreserveSpace && mode == PC_KANJI1 &&
			!is_hangul && !prev_is_hangul &&
			obuf->pos > h_env->envs[h_env->envc].indent &&
			Strlastchar(obuf->line) == ' ') {
			while (obuf->line->length >= 2 &&
			       !strncmp(obuf->line->ptr + obuf->line->length -
					2, "  ", 2)
			       && obuf->pos >= h_env->envs[h_env->envc].indent) {
			    Strshrink(obuf->line, 1);
			    obuf->pos--;
			}
			if (obuf->line->length >= 3 &&
			    obuf->prev_ctype == PC_KANJI1 &&
			    Strlastchar(obuf->line) == ' ' &&
			    obuf->pos >= h_env->envs[h_env->envc].indent) {
			    Strshrink(obuf->line, 1);
			    obuf->pos--;
			}
		    }
		    prev_is_hangul = is_hangul;
#endif
		    if (*str == '&')
			proc_escape(obuf, &str);
		    else
			proc_mchar(obuf, obuf->flag & RB_SPECIAL, delta, &str,
				   mode);
		}
	    }
	    if (need_flushline(h_env, obuf, mode)) {
		char *bp = obuf->line->ptr + obuf->bp.len;
		char *tp = bp - obuf->bp.tlen;
		int i = 0;

		if (tp > obuf->line->ptr && tp[-1] == ' ')
		    i = 1;

		indent = h_env->envs[h_env->envc].indent;
		if (obuf->bp.pos - i > indent) {
		    Str line;
		    append_tags(obuf);	/* may reallocate the buffer */
		    bp = obuf->line->ptr + obuf->bp.len;
		    line = Strnew_charp(bp);
		    Strshrink(obuf->line, obuf->line->length - obuf->bp.len);
#ifdef FORMAT_NICE
		    if (obuf->pos - i > h_env->limit)
			obuf->flag |= RB_FILL;
#endif				/* FORMAT_NICE */
		    back_to_breakpoint(obuf);
		    flushline(h_env, obuf, indent, 0, h_env->limit);
#ifdef FORMAT_NICE
		    obuf->flag &= ~RB_FILL;
#endif				/* FORMAT_NICE */
		    HTMLlineproc1(line->ptr, h_env);
		}
	    }
	}
    }
    if (!(obuf->flag & (RB_SPECIAL | RB_INTXTA | RB_INSELECT))) {
	char *tp;
	int i = 0;

	if (obuf->bp.pos == obuf->pos) {
	    tp = &obuf->line->ptr[obuf->bp.len - obuf->bp.tlen];
	}
	else {
	    tp = &obuf->line->ptr[obuf->line->length];
	}

	if (tp > obuf->line->ptr && tp[-1] == ' ')
	    i = 1;
	indent = h_env->envs[h_env->envc].indent;
	if (obuf->pos - i > h_env->limit) {
#ifdef FORMAT_NICE
	    obuf->flag |= RB_FILL;
#endif				/* FORMAT_NICE */
	    flushline(h_env, obuf, indent, 0, h_env->limit);
#ifdef FORMAT_NICE
	    obuf->flag &= ~RB_FILL;
#endif				/* FORMAT_NICE */
	}
    }
}

extern char *NullLine;
extern Lineprop NullProp[];

#ifndef USE_ANSI_COLOR
#define addnewline2(a,b,c,d,e,f) _addnewline2(a,b,c,e,f)
#endif
static void
addnewline2(Buffer *buf, char *line, Lineprop *prop, Linecolor *color, int pos,
	    int nlines)
{
    Line *l;
    l = New(Line);
    l->next = NULL;
    l->lineBuf = line;
    l->propBuf = prop;
#ifdef USE_ANSI_COLOR
    l->colorBuf = color;
#endif
    l->len = pos;
    l->width = -1;
    l->size = pos;
    l->bpos = 0;
    l->bwidth = 0;
    l->prev = buf->currentLine;
    if (buf->currentLine) {
	l->next = buf->currentLine->next;
	buf->currentLine->next = l;
    }
    else
	l->next = NULL;
    if (buf->lastLine == NULL || buf->lastLine == buf->currentLine)
	buf->lastLine = l;
    buf->currentLine = l;
    if (buf->firstLine == NULL)
	buf->firstLine = l;
    l->linenumber = ++buf->allLine;
    if (nlines < 0) {
	/*     l->real_linenumber = l->linenumber;     */
	l->real_linenumber = 0;
    }
    else {
	l->real_linenumber = nlines;
    }
    l = NULL;
}

static void
addnewline(Buffer *buf, char *line, Lineprop *prop, Linecolor *color, int pos,
	   int width, int nlines)
{
    char *s;
    Lineprop *p;
#ifdef USE_ANSI_COLOR
    Linecolor *c;
#endif
    Line *l;
    int i, bpos, bwidth;

    if (pos > 0) {
	s = allocStr(line, pos);
	p = NewAtom_N(Lineprop, pos);
	bcopy((void *)prop, (void *)p, pos * sizeof(Lineprop));
    }
    else {
	s = NullLine;
	p = NullProp;
    }
#ifdef USE_ANSI_COLOR
    if (pos > 0 && color) {
	c = NewAtom_N(Linecolor, pos);
	bcopy((void *)color, (void *)c, pos * sizeof(Linecolor));
    }
    else {
	c = NULL;
    }
#endif
    addnewline2(buf, s, p, c, pos, nlines);
    if (pos <= 0 || width <= 0)
	return;
    bpos = 0;
    bwidth = 0;
    while (1) {
	l = buf->currentLine;
	l->bpos = bpos;
	l->bwidth = bwidth;
	i = columnLen(l, width);
	if (i == 0) {
	    i++;
#ifdef USE_M17N
	    while (i < l->len && p[i] & PC_WCHAR2)
		i++;
#endif
	}
	l->len = i;
	l->width = COLPOS(l, l->len);
	if (pos <= i)
	    return;
	bpos += l->len;
	bwidth += l->width;
	s += i;
	p += i;
#ifdef USE_ANSI_COLOR
	if (c)
	    c += i;
#endif
	pos -= i;
	addnewline2(buf, s, p, c, pos, nlines);
    }
}

/* 
 * loadHTMLBuffer: read file and make new buffer
 */
Buffer *
loadHTMLBuffer(URLFile *f, Buffer *newBuf)
{
    FILE *src = NULL;
    Str tmp;

    if (newBuf == NULL)
	newBuf = newBuffer(INIT_BUFFER_WIDTH);
    if (newBuf->sourcefile == NULL &&
	(f->scheme != SCM_LOCAL || newBuf->mailcap)) {
	tmp = tmpfname(TMPF_SRC, ".html");
	src = fopen(tmp->ptr, "w");
	if (src)
	    newBuf->sourcefile = tmp->ptr;
    }

    loadHTMLstream(f, newBuf, src, newBuf->bufferprop & BP_FRAME);

    newBuf->topLine = newBuf->firstLine;
    newBuf->lastLine = newBuf->currentLine;
    newBuf->currentLine = newBuf->firstLine;
    if (n_textarea)
	formResetBuffer(newBuf, newBuf->formitem);
    if (src)
	fclose(src);

    return newBuf;
}

static char *_size_unit[] = { "b", "kb", "Mb", "Gb", "Tb",
    "Pb", "Eb", "Zb", "Bb", "Yb", NULL
};

char *
convert_size(clen_t size, int usefloat)
{
    float csize;
    int sizepos = 0;
    char **sizes = _size_unit;

    csize = (float)size;
    while (csize >= 999.495 && sizes[sizepos + 1]) {
	csize = csize / 1024.0;
	sizepos++;
    }
    return Sprintf(usefloat ? "%.3g%s" : "%.0f%s",
		   floor(csize * 100.0 + 0.5) / 100.0, sizes[sizepos])->ptr;
}

char *
convert_size2(clen_t size1, clen_t size2, int usefloat)
{
    char **sizes = _size_unit;
    float csize, factor = 1;
    int sizepos = 0;

    csize = (float)((size1 > size2) ? size1 : size2);
    while (csize / factor >= 999.495 && sizes[sizepos + 1]) {
	factor *= 1024.0;
	sizepos++;
    }
    return Sprintf(usefloat ? "%.3g/%.3g%s" : "%.0f/%.0f%s",
		   floor(size1 / factor * 100.0 + 0.5) / 100.0,
		   floor(size2 / factor * 100.0 + 0.5) / 100.0,
		   sizes[sizepos])->ptr;
}

void
showProgress(clen_t * linelen, clen_t * trbyte)
{
    int i, j, rate, duration, eta, pos;
    static time_t last_time, start_time;
    time_t cur_time;
    Str messages;
    char *fmtrbyte, *fmrate;

    if (!fmInitialized)
	return;

    if (*linelen < 1024)
	return;
    if (current_content_length > 0) {
	double ratio;
	cur_time = time(0);
	if (*trbyte == 0) {
	    move(LASTLINE, 0);
	    clrtoeolx();
	    start_time = cur_time;
	}
	*trbyte += *linelen;
	*linelen = 0;
	if (cur_time == last_time)
	    return;
	last_time = cur_time;
	move(LASTLINE, 0);
	ratio = 100.0 * (*trbyte) / current_content_length;
	fmtrbyte = convert_size2(*trbyte, current_content_length, 1);
	duration = cur_time - start_time;
	if (duration) {
	    rate = *trbyte / duration;
	    fmrate = convert_size(rate, 1);
	    eta = rate ? (current_content_length - *trbyte) / rate : -1;
	    messages = Sprintf("%11s %3.0f%% "
			       "%7s/s "
			       "eta %02d:%02d:%02d     ",
			       fmtrbyte, ratio,
			       fmrate,
			       eta / (60 * 60), (eta / 60) % 60, eta % 60);
	}
	else {
	    messages = Sprintf("%11s %3.0f%%                          ",
			       fmtrbyte, ratio);
	}
	addstr(messages->ptr);
	pos = 42;
	i = pos + (COLS - pos - 1) * (*trbyte) / current_content_length;
	move(LASTLINE, pos);
	standout();
	addch(' ');
	for (j = pos + 1; j <= i; j++)
	    addch('|');
	standend();
	/* no_clrtoeol(); */
	refresh();
    }
    else {
	cur_time = time(0);
	if (*trbyte == 0) {
	    move(LASTLINE, 0);
	    clrtoeolx();
	    start_time = cur_time;
	}
	*trbyte += *linelen;
	*linelen = 0;
	if (cur_time == last_time)
	    return;
	last_time = cur_time;
	move(LASTLINE, 0);
	fmtrbyte = convert_size(*trbyte, 1);
	duration = cur_time - start_time;
	if (duration) {
	    fmrate = convert_size(*trbyte / duration, 1);
	    messages = Sprintf("%7s loaded %7s/s", fmtrbyte, fmrate);
	}
	else {
	    messages = Sprintf("%7s loaded", fmtrbyte);
	}
	message(messages->ptr, 0, 0);
	refresh();
    }
}

void
init_henv(struct html_feed_environ *h_env, struct readbuffer *obuf,
	  struct environment *envs, int nenv, TextLineList *buf,
	  int limit, int indent)
{
    envs[0].indent = indent;

    obuf->line = Strnew();
    obuf->cprop = 0;
    obuf->pos = 0;
    obuf->prevchar = Strnew_size(8);
    set_space_to_prevchar(obuf->prevchar);
    obuf->flag = RB_IGNORE_P;
    obuf->flag_sp = 0;
    obuf->status = R_ST_NORMAL;
    obuf->table_level = -1;
    obuf->nobr_level = 0;
    obuf->q_level = 0;
    bzero((void *)&obuf->anchor, sizeof(obuf->anchor));
    obuf->img_alt = 0;
    obuf->input_alt.hseq = 0;
    obuf->input_alt.fid = -1;
    obuf->input_alt.in = 0;
    obuf->input_alt.type = NULL;
    obuf->input_alt.name = NULL;
    obuf->input_alt.value = NULL;
    obuf->in_bold = 0;
    obuf->in_italic = 0;
    obuf->in_under = 0;
    obuf->in_strike = 0;
    obuf->in_ins = 0;
    obuf->prev_ctype = PC_ASCII;
    obuf->tag_sp = 0;
    obuf->fontstat_sp = 0;
    obuf->top_margin = 0;
    obuf->bottom_margin = 0;
    obuf->bp.init_flag = 1;
    set_breakpoint(obuf, 0);

    h_env->buf = buf;
    h_env->f = NULL;
    h_env->obuf = obuf;
    h_env->tagbuf = Strnew();
    h_env->limit = limit;
    h_env->maxlimit = 0;
    h_env->envs = envs;
    h_env->nenv = nenv;
    h_env->envc = 0;
    h_env->envc_real = 0;
    h_env->title = NULL;
    h_env->blank_lines = 0;
}

void
completeHTMLstream(struct html_feed_environ *h_env, struct readbuffer *obuf)
{
    close_anchor(h_env, obuf);
    if (obuf->img_alt) {
	push_tag(obuf, "</img_alt>", HTML_N_IMG_ALT);
	obuf->img_alt = NULL;
    }
    if (obuf->input_alt.in) {
	push_tag(obuf, "</input_alt>", HTML_N_INPUT_ALT);
	obuf->input_alt.hseq = 0;
	obuf->input_alt.fid = -1;
	obuf->input_alt.in = 0;
	obuf->input_alt.type = NULL;
	obuf->input_alt.name = NULL;
	obuf->input_alt.value = NULL;
    }
    if (obuf->in_bold) {
	push_tag(obuf, "</b>", HTML_N_B);
	obuf->in_bold = 0;
    }
    if (obuf->in_italic) {
	push_tag(obuf, "</i>", HTML_N_I);
	obuf->in_italic = 0;
    }
    if (obuf->in_under) {
	push_tag(obuf, "</u>", HTML_N_U);
	obuf->in_under = 0;
    }
    if (obuf->in_strike) {
	push_tag(obuf, "</s>", HTML_N_S);
	obuf->in_strike = 0;
    }
    if (obuf->in_ins) {
	push_tag(obuf, "</ins>", HTML_N_INS);
	obuf->in_ins = 0;
    }
    if (obuf->flag & RB_INTXTA)
	HTMLlineproc1("</textarea>", h_env);
    /* for unbalanced select tag */
    if (obuf->flag & RB_INSELECT)
	HTMLlineproc1("</select>", h_env);
    if (obuf->flag & RB_TITLE)
	HTMLlineproc1("</title>", h_env);

    /* for unbalanced table tag */
    if (obuf->table_level >= MAX_TABLE)
	obuf->table_level = MAX_TABLE - 1;

    while (obuf->table_level >= 0) {
	int tmp = obuf->table_level;
	table_mode[obuf->table_level].pre_mode
	    &= ~(TBLM_SCRIPT | TBLM_STYLE | TBLM_PLAIN);
	HTMLlineproc1("</table>", h_env);
	if (obuf->table_level >= tmp)
	    break;
    }
}

static void
print_internal_information(struct html_feed_environ *henv)
{
    int i;
    Str s;
    TextLineList *tl = newTextLineList();

    s = Strnew_charp("<internal>");
    pushTextLine(tl, newTextLine(s, 0));
    if (henv->title) {
	s = Strnew_m_charp("<title_alt title=\"",
			   html_quote(henv->title), "\">", NULL);
	pushTextLine(tl, newTextLine(s, 0));
    }
#if 0
    if (form_max >= 0) {
	FormList *fp;
	for (i = 0; i <= form_max; i++) {
	    if (forms[i] == NULL)
		continue;
	    fp = forms[i];
	    s = Sprintf("<form_int fid=\"%d\" action=\"%s\" method=\"%s\"",
			i, html_quote(fp->action->ptr),
			(fp->method == FORM_METHOD_POST) ? "post"
			: ((fp->method ==
			    FORM_METHOD_INTERNAL) ? "internal" : "get"));
	    if (fp->target)
		Strcat(s, Sprintf(" target=\"%s\"", html_quote(fp->target)));
	    if (fp->enctype == FORM_ENCTYPE_MULTIPART)
		Strcat_charp(s, " enctype=\"multipart/form-data\"");
#ifdef USE_M17N
	    if (fp->charset)
		Strcat(s, Sprintf(" accept-charset=\"%s\"",
				  html_quote(fp->charset)));
#endif
	    Strcat_charp(s, ">");
	    pushTextLine(tl, newTextLine(s, 0));
	}
    }
#endif
#ifdef MENU_SELECT
    if (n_select > 0) {
	FormSelectOptionItem *ip;
	for (i = 0; i < n_select; i++) {
	    s = Sprintf("<select_int selectnumber=%d>", i);
	    pushTextLine(tl, newTextLine(s, 0));
	    for (ip = select_option[i].first; ip; ip = ip->next) {
		s = Sprintf("<option_int value=\"%s\" label=\"%s\"%s>",
			    html_quote(ip->value ? ip->value->ptr :
				       ip->label->ptr),
			    html_quote(ip->label->ptr),
			    ip->checked ? " selected" : "");
		pushTextLine(tl, newTextLine(s, 0));
	    }
	    s = Strnew_charp("</select_int>");
	    pushTextLine(tl, newTextLine(s, 0));
	}
    }
#endif				/* MENU_SELECT */
    if (n_textarea > 0) {
	for (i = 0; i < n_textarea; i++) {
	    s = Sprintf("<textarea_int textareanumber=%d>", i);
	    pushTextLine(tl, newTextLine(s, 0));
	    s = Strnew_charp(html_quote(textarea_str[i]->ptr));
	    Strcat_charp(s, "</textarea_int>");
	    pushTextLine(tl, newTextLine(s, 0));
	}
    }
    s = Strnew_charp("</internal>");
    pushTextLine(tl, newTextLine(s, 0));

    if (henv->buf)
	appendTextLineList(henv->buf, tl);
    else if (henv->f) {
	TextLineListItem *p;
	for (p = tl->first; p; p = p->next)
	    fprintf(henv->f, "%s\n", Str_conv_to_halfdump(p->ptr->line)->ptr);
    }
}

void
loadHTMLstream(URLFile *f, Buffer *newBuf, FILE * src, int internal)
{
    struct environment envs[MAX_ENV_LEVEL];
    clen_t linelen = 0;
    clen_t trbyte = 0;
    Str lineBuf2 = Strnew();
#ifdef USE_M17N
    wc_ces charset = WC_CES_US_ASCII;
    wc_ces volatile doc_charset = DocumentCharset;
#endif
    struct html_feed_environ htmlenv1;
    struct readbuffer obuf;
#ifdef USE_IMAGE
    int volatile image_flag;
#endif
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

#ifdef USE_M17N
    if (fmInitialized && graph_ok()) {
	symbol_width = symbol_width0 = 1;
    }
    else {
	symbol_width0 = 0;
	get_symbol(DisplayCharset, &symbol_width0);
	symbol_width = WcOption.use_wide ? symbol_width0 : 1;
    }
#else
    symbol_width = symbol_width0 = 1;
#endif

    cur_title = NULL;
    n_textarea = 0;
    cur_textarea = NULL;
    max_textarea = MAX_TEXTAREA;
    textarea_str = New_N(Str, max_textarea);
#ifdef MENU_SELECT
    n_select = 0;
    max_select = MAX_SELECT;
    select_option = New_N(FormSelectOption, max_select);
#endif				/* MENU_SELECT */
    cur_select = NULL;
    form_sp = -1;
    form_max = -1;
    forms_size = 0;
    forms = NULL;
    cur_hseq = 1;
#ifdef USE_IMAGE
    cur_iseq = 1;
    if (newBuf->image_flag)
	image_flag = newBuf->image_flag;
    else if (activeImage && displayImage && autoImage)
	image_flag = IMG_FLAG_AUTO;
    else
	image_flag = IMG_FLAG_SKIP;
#endif

    if (w3m_halfload) {
	newBuf->buffername = "---";
#ifdef USE_M17N
	newBuf->document_charset = InnerCharset;
#endif
	max_textarea = 0;
#ifdef MENU_SELECT
	max_select = 0;
#endif
	HTMLlineproc3(newBuf, f->stream);
	w3m_halfload = FALSE;
	return;
    }

    init_henv(&htmlenv1, &obuf, envs, MAX_ENV_LEVEL, NULL, newBuf->width, 0);

    if (w3m_halfdump)
	htmlenv1.f = stdout;
    else
	htmlenv1.buf = newTextLineList();
#if defined(USE_M17N) || defined(USE_IMAGE)
    cur_baseURL = baseURL(newBuf);
#endif

    if (SETJMP(AbortLoading) != 0) {
	HTMLlineproc1("<br>Transfer Interrupted!<br>", &htmlenv1);
	goto phase2;
    }
    TRAP_ON;

#ifdef USE_M17N
    if (newBuf != NULL) {
	if (newBuf->bufferprop & BP_FRAME)
	    charset = InnerCharset;
	else if (newBuf->document_charset)
	    charset = doc_charset = newBuf->document_charset;
    }
    if (content_charset && UseContentCharset)
	doc_charset = content_charset;
    else if (f->guess_type && !strcasecmp(f->guess_type, "application/xhtml+xml"))
	doc_charset = WC_CES_UTF_8;
    meta_charset = 0;
#endif
#if	0
    do_blankline(&htmlenv1, &obuf, 0, 0, htmlenv1.limit);
    obuf.flag = RB_IGNORE_P;
#endif
    if (IStype(f->stream) != IST_ENCODED)
	f->stream = newEncodedStream(f->stream, f->encoding);
    while ((lineBuf2 = StrmyUFgets(f))->length) {
#ifdef USE_NNTP
	if (f->scheme == SCM_NEWS && lineBuf2->ptr[0] == '.') {
	    Strshrinkfirst(lineBuf2, 1);
	    if (lineBuf2->ptr[0] == '\n' || lineBuf2->ptr[0] == '\r' ||
		lineBuf2->ptr[0] == '\0') {
		/*
		 * iseos(f->stream) = TRUE;
		 */
		break;
	    }
	}
#endif				/* USE_NNTP */
	if (src)
	    Strfputs(lineBuf2, src);
	linelen += lineBuf2->length;
	if (w3m_dump & DUMP_EXTRA)
	    printf("W3m-in-progress: %s\n", convert_size2(linelen, current_content_length, TRUE));
	if (w3m_dump & DUMP_SOURCE)
	    continue;
	showProgress(&linelen, &trbyte);
	/*
	 * if (frame_source)
	 * continue;
	 */
#ifdef USE_M17N
	if (meta_charset) {	/* <META> */
	    if (content_charset == 0 && UseContentCharset) {
		doc_charset = meta_charset;
		charset = WC_CES_US_ASCII;
	    }
	    meta_charset = 0;
	}
#endif
	lineBuf2 = convertLine(f, lineBuf2, HTML_MODE, &charset, doc_charset);
#ifdef USE_M17N
	cur_document_charset = charset;
#endif
	HTMLlineproc0(lineBuf2->ptr, &htmlenv1, internal);
    }
    if (obuf.status != R_ST_NORMAL) {
	HTMLlineproc0("\n", &htmlenv1, internal);
    }
    obuf.status = R_ST_NORMAL;
    completeHTMLstream(&htmlenv1, &obuf);
    flushline(&htmlenv1, &obuf, 0, 2, htmlenv1.limit);
#if defined(USE_M17N) || defined(USE_IMAGE)
    cur_baseURL = NULL;
#endif
#ifdef USE_M17N
    cur_document_charset = 0;
#endif
    if (htmlenv1.title)
	newBuf->buffername = htmlenv1.title;
    if (w3m_halfdump) {
	TRAP_OFF;
	print_internal_information(&htmlenv1);
	return;
    }
    if (w3m_backend) {
	TRAP_OFF;
	print_internal_information(&htmlenv1);
	backend_halfdump_buf = htmlenv1.buf;
	return;
    }
  phase2:
    newBuf->trbyte = trbyte + linelen;
    TRAP_OFF;
#ifdef USE_M17N
    if (!(newBuf->bufferprop & BP_FRAME))
	newBuf->document_charset = charset;
#endif
#ifdef USE_IMAGE
    newBuf->image_flag = image_flag;
#endif
    HTMLlineproc2(newBuf, htmlenv1.buf);
}

/* 
 * loadHTMLString: read string and make new buffer
 */
Buffer *
loadHTMLString(Str page)
{
    URLFile f;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
    Buffer *newBuf;

    init_stream(&f, SCM_LOCAL, newStrStream(page));

    newBuf = newBuffer(INIT_BUFFER_WIDTH);
    if (SETJMP(AbortLoading) != 0) {
	TRAP_OFF;
	discardBuffer(newBuf);
	UFclose(&f);
	return NULL;
    }
    TRAP_ON;

#ifdef USE_M17N
    newBuf->document_charset = InnerCharset;
#endif
    loadHTMLstream(&f, newBuf, NULL, TRUE);
#ifdef USE_M17N
    newBuf->document_charset = WC_CES_US_ASCII;
#endif

    TRAP_OFF;
    UFclose(&f);
    newBuf->topLine = newBuf->firstLine;
    newBuf->lastLine = newBuf->currentLine;
    newBuf->currentLine = newBuf->firstLine;
    newBuf->type = "text/html";
    newBuf->real_type = newBuf->type;
    if (n_textarea)
	formResetBuffer(newBuf, newBuf->formitem);
    return newBuf;
}

#ifdef USE_GOPHER

/* 
 * loadGopherDir: get gopher directory
 */
#ifdef USE_M17N
Str
loadGopherDir(URLFile *uf, ParsedURL *pu, wc_ces * charset)
#else
Str
loadGopherDir0(URLFile *uf, ParsedURL *pu)
#endif
{
    Str volatile tmp;
    Str lbuf, name, file, host, port, type;
    char *volatile p, *volatile q;
    int link, pre;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
#ifdef USE_M17N
    wc_ces doc_charset = DocumentCharset;
#endif

    tmp = parsedURL2Str(pu);
    p = html_quote(tmp->ptr);
    tmp =
	convertLine(NULL, Strnew_charp(file_unquote(tmp->ptr)), RAW_MODE,
		    charset, doc_charset);
    q = html_quote(tmp->ptr);
    tmp = Strnew_m_charp("<html>\n<head>\n<base href=\"", p, "\">\n<title>", q,
			 "</title>\n</head>\n<body>\n<h1>Index of ", q,
			 "</h1>\n<table>\n", NULL);

    if (SETJMP(AbortLoading) != 0)
	goto gopher_end;
    TRAP_ON;

    pre = 0;
    while (1) {
	if (lbuf = StrUFgets(uf), lbuf->length == 0)
	    break;
	if (lbuf->ptr[0] == '.' &&
	    (lbuf->ptr[1] == '\n' || lbuf->ptr[1] == '\r'))
	    break;
	lbuf = convertLine(uf, lbuf, HTML_MODE, charset, doc_charset);
	p = lbuf->ptr;
	for (q = p; *q && *q != '\t'; q++) ;
	name = Strnew_charp_n(p, q - p);
	if (!*q)
	    continue;
	p = q + 1;
	for (q = p; *q && *q != '\t'; q++) ;
	file = Strnew_charp_n(p, q - p);
	if (!*q)
	    continue;
	p = q + 1;
	for (q = p; *q && *q != '\t'; q++) ;
	host = Strnew_charp_n(p, q - p);
	if (!*q)
	    continue;
	p = q + 1;
	for (q = p; *q && *q != '\t' && *q != '\r' && *q != '\n'; q++) ;
	port = Strnew_charp_n(p, q - p);

	link = 1;
	switch (name->ptr[0]) {
	case '0':
	    p = "[text file]";
	    break;
	case '1':
	    p = "[directory]";
	    break;
	case '5':
	    p = "[DOS binary]";
	    break;
	case '7':
	    p = "[search]";
	    break;
	case 'm':
	    p = "[message]";
	    break;
	case 's':
	    p = "[sound]";
	    break;
	case 'g':
	    p = "[gif]";
	    break;
	case 'h':
	    p = "[HTML]";
	    break;
	case 'i':
	    link = 0;
	    break;
	case 'I':
	    p = "[image]";
	    break;
	case '9':
	    p = "[binary]";
	    break;
	default:
	    p = "[unsupported]";
	    break;
	}
	type = Strsubstr(name, 0, 1);
	q = Strnew_m_charp("gopher://", host->ptr, ":", port->ptr, "/", type->ptr, file->ptr, NULL)->ptr;
	if(link) {
	    if(pre) {
		Strcat_charp(tmp, "</pre>");
		pre = 0;
	    }
	    Strcat_m_charp(tmp, "<a href=\"",
			   html_quote(url_encode(q, NULL, *charset)),
			   "\">", p, " ", html_quote(name->ptr + 1), "</a><br>\n", NULL);
	} else {
	    if(!pre) {
		Strcat_charp(tmp, "<pre>");
		pre = 1;
	    }

	    Strcat_m_charp(tmp, html_quote(name->ptr + 1), "\n", NULL);
	}
    }

  gopher_end:
    TRAP_OFF;

    if(pre)
	Strcat_charp(tmp, "</pre>");
    Strcat_charp(tmp, "</table>\n</body>\n</html>\n");
    return tmp;
}

#ifdef USE_M17N
Str
loadGopherSearch(URLFile *uf, ParsedURL *pu, wc_ces * charset)
#else
Str
loadGopherSearch0(URLFile *uf, ParsedURL *pu)
#endif
{
    Str tmp;
    char *volatile p, *volatile q;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
#ifdef USE_M17N
    wc_ces doc_charset = DocumentCharset;
#endif

    tmp = parsedURL2Str(pu);
    p = html_quote(tmp->ptr);
    tmp =
	convertLine(NULL, Strnew_charp(file_unquote(tmp->ptr)), RAW_MODE,
		    charset, doc_charset);
    q = html_quote(tmp->ptr);
    tmp = Strnew_m_charp("<html>\n<head>\n<base href=\"", p, "\">\n<title>", q,
			 "</title>\n</head>\n<body>\n<h1>Search ", q,
			 "</h1>\n<form role=\"search\">\n<div>\n"
			 "<input type=\"search\" name=\"\">"
			 "</div>\n</form>\n</body>", NULL);

    return tmp;
}
#endif				/* USE_GOPHER */

/* 
 * loadBuffer: read file and make new buffer
 */
Buffer *
loadBuffer(URLFile *uf, Buffer *volatile newBuf)
{
    FILE *volatile src = NULL;
#ifdef USE_M17N
    wc_ces charset = WC_CES_US_ASCII;
    wc_ces volatile doc_charset = DocumentCharset;
#endif
    Str lineBuf2;
    volatile char pre_lbuf = '\0';
    int nlines;
    Str tmpf;
    clen_t linelen = 0, trbyte = 0;
    Lineprop *propBuffer = NULL;
#ifdef USE_ANSI_COLOR
    Linecolor *colorBuffer = NULL;
#endif
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

    if (newBuf == NULL)
	newBuf = newBuffer(INIT_BUFFER_WIDTH);

    if (SETJMP(AbortLoading) != 0) {
	goto _end;
    }
    TRAP_ON;

    if (newBuf->sourcefile == NULL &&
	(uf->scheme != SCM_LOCAL || newBuf->mailcap)) {
	tmpf = tmpfname(TMPF_SRC, NULL);
	src = fopen(tmpf->ptr, "w");
	if (src)
	    newBuf->sourcefile = tmpf->ptr;
    }
#ifdef USE_M17N
    if (newBuf->document_charset)
	charset = doc_charset = newBuf->document_charset;
    if (content_charset && UseContentCharset)
	doc_charset = content_charset;
#endif

    nlines = 0;
    if (IStype(uf->stream) != IST_ENCODED)
	uf->stream = newEncodedStream(uf->stream, uf->encoding);
    while ((lineBuf2 = StrmyISgets(uf->stream))->length) {
#ifdef USE_NNTP
	if (uf->scheme == SCM_NEWS && lineBuf2->ptr[0] == '.') {
	    Strshrinkfirst(lineBuf2, 1);
	    if (lineBuf2->ptr[0] == '\n' || lineBuf2->ptr[0] == '\r' ||
		lineBuf2->ptr[0] == '\0') {
		/*
		 * iseos(uf->stream) = TRUE;
		 */
		break;
	    }
	}
#endif				/* USE_NNTP */
	if (src)
	    Strfputs(lineBuf2, src);
	linelen += lineBuf2->length;
	if (w3m_dump & DUMP_EXTRA)
	    printf("W3m-in-progress: %s\n", convert_size2(linelen, current_content_length, TRUE));
	if (w3m_dump & DUMP_SOURCE)
	    continue;
	showProgress(&linelen, &trbyte);
	if (frame_source)
	    continue;
	lineBuf2 =
	    convertLine(uf, lineBuf2, PAGER_MODE, &charset, doc_charset);
	if (squeezeBlankLine) {
	    if (lineBuf2->ptr[0] == '\n' && pre_lbuf == '\n') {
		++nlines;
		continue;
	    }
	    pre_lbuf = lineBuf2->ptr[0];
	}
	++nlines;
	Strchop(lineBuf2);
	lineBuf2 = checkType(lineBuf2, &propBuffer, NULL);
	addnewline(newBuf, lineBuf2->ptr, propBuffer, colorBuffer,
		   lineBuf2->length, FOLD_BUFFER_WIDTH, nlines);
    }
  _end:
    TRAP_OFF;
    newBuf->topLine = newBuf->firstLine;
    newBuf->lastLine = newBuf->currentLine;
    newBuf->currentLine = newBuf->firstLine;
    newBuf->trbyte = trbyte + linelen;
#ifdef USE_M17N
    newBuf->document_charset = charset;
#endif
    if (src)
	fclose(src);

    return newBuf;
}

#ifdef USE_IMAGE
Buffer *
loadImageBuffer(URLFile *uf, Buffer *newBuf)
{
    Image image;
    ImageCache *cache;
    Str tmp, tmpf;
    FILE *src = NULL;
    URLFile f;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
    struct stat st;
    const ParsedURL *pu = newBuf ? &newBuf->currentURL : NULL;

    loadImage(newBuf, IMG_FLAG_STOP);
    image.url = uf->url;
    image.ext = uf->ext;
    image.width = -1;
    image.height = -1;
    image.cache = NULL;
    cache = getImage(&image, (ParsedURL *)pu, IMG_FLAG_AUTO);
    if (!(pu && pu->is_nocache) && cache->loaded & IMG_FLAG_LOADED &&
	!stat(cache->file, &st))
	goto image_buffer;

    if (IStype(uf->stream) != IST_ENCODED)
	uf->stream = newEncodedStream(uf->stream, uf->encoding);
    TRAP_ON;
    if (save2tmp(*uf, cache->file) < 0) {
	TRAP_OFF;
	return NULL;
    }
    TRAP_OFF;

    cache->loaded = IMG_FLAG_LOADED;
    cache->index = 0;

  image_buffer:
    if (newBuf == NULL)
	newBuf = newBuffer(INIT_BUFFER_WIDTH);
    cache->loaded |= IMG_FLAG_DONT_REMOVE;
    if (newBuf->sourcefile == NULL && uf->scheme != SCM_LOCAL)
	newBuf->sourcefile = cache->file;

    tmp = Sprintf("<img src=\"%s\"><br><br>", html_quote(image.url));
    tmpf = tmpfname(TMPF_SRC, ".html");
    src = fopen(tmpf->ptr, "w");
    if (src == NULL)
        return NULL;
    newBuf->mailcap_source = tmpf->ptr;

    init_stream(&f, SCM_LOCAL, newStrStream(tmp));
    loadHTMLstream(&f, newBuf, src, TRUE);
    UFclose(&f);
    if (src)
	fclose(src);

    newBuf->topLine = newBuf->firstLine;
    newBuf->lastLine = newBuf->currentLine;
    newBuf->currentLine = newBuf->firstLine;
    newBuf->image_flag = IMG_FLAG_AUTO;
    return newBuf;
}
#endif

static Str
conv_symbol(Line *l)
{
    Str tmp = NULL;
    char *p = l->lineBuf, *ep = p + l->len;
    Lineprop *pr = l->propBuf;
#ifdef USE_M17N
    int w;
    char **symbol = NULL;
#else
    char **symbol = get_symbol();
#endif

    for (; p < ep; p++, pr++) {
	if (*pr & PC_SYMBOL) {
#ifdef USE_M17N
	    char c = ((char)wtf_get_code((wc_uchar *) p) & 0x7f) - SYMBOL_BASE;
	    int len = get_mclen(p);
#else
	    char c = *p - SYMBOL_BASE;
#endif
	    if (tmp == NULL) {
		tmp = Strnew_size(l->len);
		Strcopy_charp_n(tmp, l->lineBuf, p - l->lineBuf);
#ifdef USE_M17N
		w = (*pr & PC_KANJI) ? 2 : 1;
		symbol = get_symbol(DisplayCharset, &w);
#endif
	    }
	    Strcat_charp(tmp, symbol[(unsigned char)c % N_SYMBOL]);
#ifdef USE_M17N
	    p += len - 1;
	    pr += len - 1;
#endif
	}
	else if (tmp != NULL)
	    Strcat_char(tmp, *p);
    }
    if (tmp)
	return tmp;
    else
	return Strnew_charp_n(l->lineBuf, l->len);
}

/* 
 * saveBuffer: write buffer to file
 */
static void
_saveBuffer(Buffer *buf, Line *l, FILE * f, int cont)
{
    Str tmp;
    int is_html = FALSE;
#ifdef USE_M17N
    int set_charset = !DisplayCharset;
    wc_ces charset = DisplayCharset ? DisplayCharset : WC_CES_US_ASCII;
#endif

    is_html = is_html_type(buf->type);

  pager_next:
    for (; l != NULL; l = l->next) {
	if (is_html)
	    tmp = conv_symbol(l);
	else
	    tmp = Strnew_charp_n(l->lineBuf, l->len);
	tmp = wc_Str_conv(tmp, InnerCharset, charset);
	Strfputs(tmp, f);
	if (Strlastchar(tmp) != '\n' && !(cont && l->next && l->next->bpos))
	    putc('\n', f);
    }
    if (buf->pagerSource && !(buf->bufferprop & BP_CLOSE)) {
	l = getNextPage(buf, PagerMax);
#ifdef USE_M17N
	if (set_charset)
	    charset = buf->document_charset;
#endif
	goto pager_next;
    }
}

void
saveBuffer(Buffer *buf, FILE * f, int cont)
{
    _saveBuffer(buf, buf->firstLine, f, cont);
}

void
saveBufferBody(Buffer *buf, FILE * f, int cont)
{
    Line *l = buf->firstLine;

    while (l != NULL && l->real_linenumber == 0)
	l = l->next;
    _saveBuffer(buf, l, f, cont);
}

static Buffer *
loadcmdout(char *cmd,
	   Buffer *(*loadproc) (URLFile *, Buffer *), Buffer *defaultbuf)
{
    FILE *f, *popen(const char *, const char *);
    Buffer *buf;
    URLFile uf;

    if (cmd == NULL || *cmd == '\0')
	return NULL;
    f = popen(cmd, "r");
    if (f == NULL)
	return NULL;
    init_stream(&uf, SCM_UNKNOWN, newFileStream(f, (void (*)())pclose));
    buf = loadproc(&uf, defaultbuf);
    UFclose(&uf);
    return buf;
}

/* 
 * getshell: execute shell command and get the result into a buffer
 */
Buffer *
getshell(char *cmd)
{
    Buffer *buf;

    buf = loadcmdout(cmd, loadBuffer, NULL);
    if (buf == NULL)
	return NULL;
    buf->filename = cmd;
    buf->buffername = Sprintf("%s %s", SHELLBUFFERNAME,
			      conv_from_system(cmd))->ptr;
    return buf;
}

/* 
 * getpipe: execute shell command and connect pipe to the buffer
 */
Buffer *
getpipe(char *cmd)
{
    FILE *f, *popen(const char *, const char *);
    Buffer *buf;

    if (cmd == NULL || *cmd == '\0')
	return NULL;
    f = popen(cmd, "r");
    if (f == NULL)
	return NULL;
    buf = newBuffer(INIT_BUFFER_WIDTH);
    buf->pagerSource = newFileStream(f, (void (*)())pclose);
    buf->filename = cmd;
    buf->buffername = Sprintf("%s %s", PIPEBUFFERNAME,
			      conv_from_system(cmd))->ptr;
    buf->bufferprop |= BP_PIPE;
#ifdef USE_M17N
    buf->document_charset = WC_CES_US_ASCII;
#endif
    return buf;
}

/* 
 * Open pager buffer
 */
Buffer *
openPagerBuffer(InputStream stream, Buffer *buf)
{

    if (buf == NULL)
	buf = newBuffer(INIT_BUFFER_WIDTH);
    buf->pagerSource = stream;
    buf->buffername = getenv("MAN_PN");
    if (buf->buffername == NULL)
	buf->buffername = PIPEBUFFERNAME;
    else
	buf->buffername = conv_from_system(buf->buffername);
    buf->bufferprop |= BP_PIPE;
#ifdef USE_M17N
    if (content_charset && UseContentCharset)
	buf->document_charset = content_charset;
    else
	buf->document_charset = WC_CES_US_ASCII;
#endif
    buf->currentLine = buf->firstLine;

    return buf;
}

Buffer *
openGeneralPagerBuffer(InputStream stream)
{
    Buffer *buf;
    char *t = "text/plain";
    Buffer *t_buf = NULL;
    URLFile uf;

    init_stream(&uf, SCM_UNKNOWN, stream);

#ifdef USE_M17N
    content_charset = 0;
#endif
    t_buf = newBuffer(INIT_BUFFER_WIDTH);
    copyParsedURL(&t_buf->currentURL, NULL);
    t_buf->currentURL.scheme = SCM_LOCAL;
    t_buf->currentURL.file = "-";
    if (SearchHeader) {
	readHeader(&uf, t_buf, TRUE, NULL);
	t = checkContentType(t_buf);
	if (t == NULL)
	    t = "text/plain";
	if (t_buf) {
	    t_buf->topLine = t_buf->firstLine;
	    t_buf->currentLine = t_buf->lastLine;
	}
	SearchHeader = FALSE;
    }
    else if (DefaultType) {
	t = DefaultType;
	DefaultType = NULL;
    }
    if (is_html_type(t)) {
	buf = loadHTMLBuffer(&uf, t_buf);
	buf->type = "text/html";
    }
    else if (is_plain_text_type(t)) {
	if (IStype(stream) != IST_ENCODED)
	    stream = newEncodedStream(stream, uf.encoding);
	buf = openPagerBuffer(stream, t_buf);
	buf->type = "text/plain";
    }
#ifdef USE_IMAGE
    else if (activeImage && displayImage && !useExtImageViewer &&
	     !(w3m_dump & ~DUMP_FRAME) && !strncasecmp(t, "image/", 6)) {
	buf = loadImageBuffer(&uf, t_buf);
	buf->type = "text/html";
    }
#endif
    else {
	if (searchExtViewer(t)) {
	    buf = doExternal(uf, t, t_buf);
	    UFclose(&uf);
	    if (buf == NULL || buf == NO_BUFFER)
		return buf;
	}
	else {			/* unknown type is regarded as text/plain */
	    if (IStype(stream) != IST_ENCODED)
		stream = newEncodedStream(stream, uf.encoding);
	    buf = openPagerBuffer(stream, t_buf);
	    buf->type = "text/plain";
	}
    }
    buf->real_type = t;
    return buf;
}

Line *
getNextPage(Buffer *buf, int plen)
{
    Line *volatile top = buf->topLine, *volatile last = buf->lastLine,
	*volatile cur = buf->currentLine;
    int i;
    int volatile nlines = 0;
    clen_t linelen = 0, trbyte = buf->trbyte;
    Str lineBuf2;
    char volatile pre_lbuf = '\0';
    URLFile uf;
#ifdef USE_M17N
    wc_ces charset;
    wc_ces volatile doc_charset = DocumentCharset;
    wc_uint8 old_auto_detect = WcOption.auto_detect;
#endif
    int volatile squeeze_flag = FALSE;
    Lineprop *propBuffer = NULL;

#ifdef USE_ANSI_COLOR
    Linecolor *colorBuffer = NULL;
#endif
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

    if (buf->pagerSource == NULL)
	return NULL;

    if (last != NULL) {
	nlines = last->real_linenumber;
	pre_lbuf = *(last->lineBuf);
	if (pre_lbuf == '\0')
	    pre_lbuf = '\n';
	buf->currentLine = last;
    }

#ifdef USE_M17N
    charset = buf->document_charset;
    if (buf->document_charset != WC_CES_US_ASCII)
	doc_charset = buf->document_charset;
    else if (UseContentCharset) {
	content_charset = 0;
	checkContentType(buf);
	if (content_charset)
	    doc_charset = content_charset;
    }
    WcOption.auto_detect = buf->auto_detect;
#endif

    if (SETJMP(AbortLoading) != 0) {
	goto pager_end;
    }
    TRAP_ON;

    init_stream(&uf, SCM_UNKNOWN, NULL);
    for (i = 0; i < plen; i++) {
	lineBuf2 = StrmyISgets(buf->pagerSource);
	if (lineBuf2->length == 0) {
	    /* Assume that `cmd == buf->filename' */
	    if (buf->filename)
		buf->buffername = Sprintf("%s %s",
					  CPIPEBUFFERNAME,
					  conv_from_system(buf->filename))->
		    ptr;
	    else if (getenv("MAN_PN") == NULL)
		buf->buffername = CPIPEBUFFERNAME;
	    buf->bufferprop |= BP_CLOSE;
	    break;
	}
	linelen += lineBuf2->length;
	showProgress(&linelen, &trbyte);
	lineBuf2 =
	    convertLine(&uf, lineBuf2, PAGER_MODE, &charset, doc_charset);
	if (squeezeBlankLine) {
	    squeeze_flag = FALSE;
	    if (lineBuf2->ptr[0] == '\n' && pre_lbuf == '\n') {
		++nlines;
		--i;
		squeeze_flag = TRUE;
		continue;
	    }
	    pre_lbuf = lineBuf2->ptr[0];
	}
	++nlines;
	Strchop(lineBuf2);
	lineBuf2 = checkType(lineBuf2, &propBuffer, &colorBuffer);
	addnewline(buf, lineBuf2->ptr, propBuffer, colorBuffer,
		   lineBuf2->length, FOLD_BUFFER_WIDTH, nlines);
	if (!top) {
	    top = buf->firstLine;
	    cur = top;
	}
	if (buf->lastLine->real_linenumber - buf->firstLine->real_linenumber
	    >= PagerMax) {
	    Line *l = buf->firstLine;
	    do {
		if (top == l)
		    top = l->next;
		if (cur == l)
		    cur = l->next;
		if (last == l)
		    last = NULL;
		l = l->next;
	    } while (l && l->bpos);
	    buf->firstLine = l;
	    buf->firstLine->prev = NULL;
	}
    }
  pager_end:
    TRAP_OFF;

    buf->trbyte = trbyte + linelen;
#ifdef USE_M17N
    buf->document_charset = charset;
    WcOption.auto_detect = old_auto_detect;
#endif
    buf->topLine = top;
    buf->currentLine = cur;
    if (!last)
	last = buf->firstLine;
    else if (last && (last->next || !squeeze_flag))
	last = last->next;
    return last;
}

int
save2tmp(URLFile uf, char *tmpf)
{
    FILE *ff;
    int check;
    clen_t linelen = 0, trbyte = 0;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;
    static JMP_BUF env_bak;
    volatile int retval = 0;
    char *volatile buf = NULL;

    ff = fopen(tmpf, "wb");
    if (ff == NULL) {
	/* fclose(f); */
	return -1;
    }
    bcopy(AbortLoading, env_bak, sizeof(JMP_BUF));
    if (SETJMP(AbortLoading) != 0) {
	goto _end;
    }
    TRAP_ON;
    check = 0;
#ifdef USE_NNTP
    if (uf.scheme == SCM_NEWS) {
	char c;
	while (c = UFgetc(&uf), !iseos(uf.stream)) {
	    if (c == '\n') {
		if (check == 0)
		    check++;
		else if (check == 3)
		    break;
	    }
	    else if (c == '.' && check == 1)
		check++;
	    else if (c == '\r' && check == 2)
		check++;
	    else
		check = 0;
	    putc(c, ff);
	    linelen += sizeof(c);
	    showProgress(&linelen, &trbyte);
	}
    }
    else
#endif				/* USE_NNTP */
    {
	int count;

	buf = NewWithoutGC_N(char, SAVE_BUF_SIZE);
	while ((count = ISread_n(uf.stream, buf, SAVE_BUF_SIZE)) > 0) {
	    if (fwrite(buf, 1, count, ff) != count) {
		retval = -2;
		goto _end;
	    }
	    linelen += count;
	    showProgress(&linelen, &trbyte);
	}
    }
  _end:
    bcopy(env_bak, AbortLoading, sizeof(JMP_BUF));
    TRAP_OFF;
    xfree(buf);
    fclose(ff);
    current_content_length = 0;
    return retval;
}

Buffer *
doExternal(URLFile uf, char *type, Buffer *defaultbuf)
{
    Str tmpf, command;
    struct mailcap *mcap;
    int mc_stat;
    Buffer *buf = NULL;
    char *header, *src = NULL, *ext = uf.ext;

    if (!(mcap = searchExtViewer(type)))
	return NULL;

    if (mcap->nametemplate) {
	tmpf = unquote_mailcap(mcap->nametemplate, NULL, "", NULL, NULL);
	if (tmpf->ptr[0] == '.')
	    ext = tmpf->ptr;
    }
    tmpf = tmpfname(TMPF_DFL, (ext && *ext) ? ext : NULL);

    if (IStype(uf.stream) != IST_ENCODED)
	uf.stream = newEncodedStream(uf.stream, uf.encoding);
    header = checkHeader(defaultbuf, "Content-Type:");
    if (header)
	header = conv_to_system(header);
    command = unquote_mailcap(mcap->viewer, type, tmpf->ptr, header, &mc_stat);
#ifndef __EMX__
    if (!(mc_stat & MCSTAT_REPNAME)) {
	Str tmp = Sprintf("(%s) < %s", command->ptr, shell_quote(tmpf->ptr));
	command = tmp;
    }
#endif

#ifdef HAVE_SETPGRP
    if (!(mcap->flags & (MAILCAP_HTMLOUTPUT | MAILCAP_COPIOUSOUTPUT)) &&
	!(mcap->flags & MAILCAP_NEEDSTERMINAL) && BackgroundExtViewer) {
	flush_tty();
	if (!fork()) {
	    setup_child(FALSE, 0, UFfileno(&uf));
	    if (save2tmp(uf, tmpf->ptr) < 0)
		exit(1);
	    UFclose(&uf);
	    myExec(command->ptr);
	}
	return NO_BUFFER;
    }
    else
#endif
    {
	if (save2tmp(uf, tmpf->ptr) < 0) {
	    return NULL;
	}
    }
    if (mcap->flags & (MAILCAP_HTMLOUTPUT | MAILCAP_COPIOUSOUTPUT)) {
	if (defaultbuf == NULL)
	    defaultbuf = newBuffer(INIT_BUFFER_WIDTH);
	if (defaultbuf->sourcefile)
	    src = defaultbuf->sourcefile;
	else
	    src = tmpf->ptr;
	defaultbuf->sourcefile = NULL;
	defaultbuf->mailcap = mcap;
    }
    if (mcap->flags & MAILCAP_HTMLOUTPUT) {
	buf = loadcmdout(command->ptr, loadHTMLBuffer, defaultbuf);
	if (buf && buf != NO_BUFFER) {
	    buf->type = "text/html";
	    buf->mailcap_source = buf->sourcefile;
	    buf->sourcefile = src;
	}
    }
    else if (mcap->flags & MAILCAP_COPIOUSOUTPUT) {
	buf = loadcmdout(command->ptr, loadBuffer, defaultbuf);
	if (buf && buf != NO_BUFFER) {
	    buf->type = "text/plain";
	    buf->mailcap_source = buf->sourcefile;
	    buf->sourcefile = src;
	}
    }
    else {
	if (mcap->flags & MAILCAP_NEEDSTERMINAL || !BackgroundExtViewer) {
	    fmTerm();
	    mySystem(command->ptr, 0);
	    fmInit();
	    if (CurrentTab && Currentbuf)
		displayBuffer(Currentbuf, B_FORCE_REDRAW);
	}
	else {
	    mySystem(command->ptr, 1);
	}
	buf = NO_BUFFER;
    }
    if (buf && buf != NO_BUFFER) {
	if ((buf->buffername == NULL || buf->buffername[0] == '\0') &&
	    buf->filename)
	    buf->buffername = conv_from_system(lastFileName(buf->filename));
	buf->edit = mcap->edit;
	buf->mailcap = mcap;
    }
    return buf;
}

static int
_MoveFile(char *path1, char *path2)
{
    InputStream f1;
    FILE *f2;
    int is_pipe;
    clen_t linelen = 0, trbyte = 0;
    char *buf = NULL;
    int count;

    f1 = openIS(path1);
    if (f1 == NULL)
	return -1;
    if (*path2 == '|' && PermitSaveToPipe) {
	is_pipe = TRUE;
	f2 = popen(path2 + 1, "w");
    }
    else {
	is_pipe = FALSE;
	f2 = fopen(path2, "wb");
    }
    if (f2 == NULL) {
	ISclose(f1);
	return -1;
    }
    current_content_length = 0;
    buf = NewWithoutGC_N(char, SAVE_BUF_SIZE);
    while ((count = ISread_n(f1, buf, SAVE_BUF_SIZE)) > 0) {
	fwrite(buf, 1, count, f2);
	linelen += count;
	showProgress(&linelen, &trbyte);
    }
    xfree(buf);
    ISclose(f1);
    if (is_pipe)
	pclose(f2);
    else
	fclose(f2);
    return 0;
}

int
_doFileCopy(char *tmpf, char *defstr, int download)
{
#ifndef __MINGW32_VERSION
    Str msg;
    Str filen;
    char *p, *q = NULL;
    pid_t pid;
    char *lock;
#if !(defined(HAVE_SYMLINK) && defined(HAVE_LSTAT))
    FILE *f;
#endif
    struct stat st;
    clen_t size = 0;
    int is_pipe = FALSE;

    if (fmInitialized) {
	p = searchKeyData();
	if (p == NULL || *p == '\0') {
	    /* FIXME: gettextize? */
	    q = inputLineHist("(Download)Save file to: ",
			      defstr, IN_COMMAND, SaveHist);
	    if (q == NULL || *q == '\0')
		return FALSE;
	    p = conv_to_system(q);
	}
	if (*p == '|' && PermitSaveToPipe)
	    is_pipe = TRUE;
	else {
	    if (q) {
		p = unescape_spaces(Strnew_charp(q))->ptr;
		p = conv_to_system(p);
	    }
	    p = expandPath(p);
	    if (checkOverWrite(p) < 0)
		return -1;
	}
	if (checkCopyFile(tmpf, p) < 0) {
	    /* FIXME: gettextize? */
	    msg = Sprintf("Can't copy. %s and %s are identical.",
			  conv_from_system(tmpf), conv_from_system(p));
	    disp_err_message(msg->ptr, FALSE);
	    return -1;
	}
	if (!download) {
	    if (_MoveFile(tmpf, p) < 0) {
		/* FIXME: gettextize? */
		msg = Sprintf("Can't save to %s", conv_from_system(p));
		disp_err_message(msg->ptr, FALSE);
	    }
	    return -1;
	}
	lock = tmpfname(TMPF_DFL, ".lock")->ptr;
#if defined(HAVE_SYMLINK) && defined(HAVE_LSTAT)
	symlink(p, lock);
#else
	f = fopen(lock, "w");
	if (f)
	    fclose(f);
#endif
	flush_tty();
	pid = fork();
	if (!pid) {
	    setup_child(FALSE, 0, -1);
	    if (!_MoveFile(tmpf, p) && PreserveTimestamp && !is_pipe &&
		!stat(tmpf, &st))
		setModtime(p, st.st_mtime);
	    unlink(lock);
	    exit(0);
	}
	if (!stat(tmpf, &st))
	    size = st.st_size;
	addDownloadList(pid, conv_from_system(tmpf), p, lock, size);
    }
    else {
	q = searchKeyData();
	if (q == NULL || *q == '\0') {
	    /* FIXME: gettextize? */
	    printf("(Download)Save file to: ");
	    fflush(stdout);
	    filen = Strfgets(stdin);
	    if (filen->length == 0)
		return -1;
	    q = filen->ptr;
	}
	for (p = q + strlen(q) - 1; IS_SPACE(*p); p--) ;
	*(p + 1) = '\0';
	if (*q == '\0')
	    return -1;
	p = q;
	if (*p == '|' && PermitSaveToPipe)
	    is_pipe = TRUE;
	else {
	    p = expandPath(p);
	    if (checkOverWrite(p) < 0)
		return -1;
	}
	if (checkCopyFile(tmpf, p) < 0) {
	    /* FIXME: gettextize? */
	    printf("Can't copy. %s and %s are identical.", tmpf, p);
	    return -1;
	}
	if (_MoveFile(tmpf, p) < 0) {
	    /* FIXME: gettextize? */
	    printf("Can't save to %s\n", p);
	    return -1;
	}
	if (PreserveTimestamp && !is_pipe && !stat(tmpf, &st))
	    setModtime(p, st.st_mtime);
    }
#endif /* __MINGW32_VERSION */
    return 0;
}

int
doFileMove(char *tmpf, char *defstr)
{
    int ret = doFileCopy(tmpf, defstr);
    unlink(tmpf);
    return ret;
}

int
doFileSave(URLFile uf, char *defstr)
{
#ifndef __MINGW32_VERSION
    Str msg;
    Str filen;
    char *p, *q;
    pid_t pid;
    char *lock;
    char *tmpf = NULL; 
#if !(defined(HAVE_SYMLINK) && defined(HAVE_LSTAT))
    FILE *f;
#endif

    if (fmInitialized) {
	p = searchKeyData();
	if (p == NULL || *p == '\0') {
	    /* FIXME: gettextize? */
	    p = inputLineHist("(Download)Save file to: ",
			      defstr, IN_FILENAME, SaveHist);
	    if (p == NULL || *p == '\0')
		return -1;
	    p = conv_to_system(p);
	}
	if (checkOverWrite(p) < 0)
	    return -1;
	if (checkSaveFile(uf.stream, p) < 0) {
	    /* FIXME: gettextize? */
	    msg = Sprintf("Can't save. Load file and %s are identical.",
			  conv_from_system(p));
	    disp_err_message(msg->ptr, FALSE);
	    return -1;
	}
	/*
	 * if (save2tmp(uf, p) < 0) {
	 * msg = Sprintf("Can't save to %s", conv_from_system(p));
	 * disp_err_message(msg->ptr, FALSE);
	 * }
	 */
	lock = tmpfname(TMPF_DFL, ".lock")->ptr;
#if defined(HAVE_SYMLINK) && defined(HAVE_LSTAT)
	symlink(p, lock);
#else
	f = fopen(lock, "w");
	if (f)
	    fclose(f);
#endif
	flush_tty();
	pid = fork();
	if (!pid) {
	    int err;
	    if ((uf.content_encoding != CMP_NOCOMPRESS) && AutoUncompress) {
		uncompress_stream(&uf, &tmpf);
		if (tmpf)
		    unlink(tmpf);
	    }
	    setup_child(FALSE, 0, UFfileno(&uf));
	    err = save2tmp(uf, p);
	    if (err == 0 && PreserveTimestamp && uf.modtime != -1)
		setModtime(p, uf.modtime);
	    UFclose(&uf);
	    unlink(lock);
	    if (err != 0)
		exit(-err);
	    exit(0);
	}
	addDownloadList(pid, uf.url, p, lock, current_content_length);
    }
    else {
	q = searchKeyData();
	if (q == NULL || *q == '\0') {
	    /* FIXME: gettextize? */
	    printf("(Download)Save file to: ");
	    fflush(stdout);
	    filen = Strfgets(stdin);
	    if (filen->length == 0)
		return -1;
	    q = filen->ptr;
	}
	for (p = q + strlen(q) - 1; IS_SPACE(*p); p--) ;
	*(p + 1) = '\0';
	if (*q == '\0')
	    return -1;
	p = expandPath(q);
	if (checkOverWrite(p) < 0)
	    return -1;
	if (checkSaveFile(uf.stream, p) < 0) {
	    /* FIXME: gettextize? */
	    printf("Can't save. Load file and %s are identical.", p);
	    return -1;
	}
	if (uf.content_encoding != CMP_NOCOMPRESS && AutoUncompress) {
	    uncompress_stream(&uf, &tmpf);
	    if (tmpf)
		unlink(tmpf);
	}
	if (save2tmp(uf, p) < 0) {
	    /* FIXME: gettextize? */
	    printf("Can't save to %s\n", p);
	    return -1;
	}
	if (PreserveTimestamp && uf.modtime != -1)
	    setModtime(p, uf.modtime);
    }
#endif /* __MINGW32_VERSION */
    return 0;
}

int
checkCopyFile(char *path1, char *path2)
{
    struct stat st1, st2;

    if (*path2 == '|' && PermitSaveToPipe)
	return 0;
    if ((stat(path1, &st1) == 0) && (stat(path2, &st2) == 0))
	if (st1.st_ino == st2.st_ino)
	    return -1;
    return 0;
}

int
checkSaveFile(InputStream stream, char *path2)
{
    struct stat st1, st2;
    int des = ISfileno(stream);

    if (des < 0)
	return 0;
    if (*path2 == '|' && PermitSaveToPipe)
	return 0;
    if ((fstat(des, &st1) == 0) && (stat(path2, &st2) == 0))
	if (st1.st_ino == st2.st_ino)
	    return -1;
    return 0;
}

int
checkOverWrite(char *path)
{
    struct stat st;
    char *ans;

    if (stat(path, &st) < 0)
	return 0;
    /* FIXME: gettextize? */
    ans = inputAnswer("File exists. Overwrite? (y/n)");
    if (ans && TOLOWER(*ans) == 'y')
	return 0;
    else
	return -1;
}

char *
inputAnswer(char *prompt)
{
    char *ans;

    if (QuietMessage)
	return "n";
    if (fmInitialized) {
	term_raw();
	ans = inputChar(prompt);
    }
    else {
	printf("%s", prompt);
	fflush(stdout);
	ans = Strfgets(stdin)->ptr;
    }
    return ans;
}

static void
uncompress_stream(URLFile *uf, char **src)
{
#ifndef __MINGW32_VERSION
    pid_t pid1;
    FILE *f1;
    char *expand_cmd = GUNZIP_CMDNAME;
    char *expand_name = GUNZIP_NAME;
    char *tmpf = NULL;
    char *ext = NULL;
    struct compression_decoder *d;
    int use_d_arg = 0;

    if (IStype(uf->stream) != IST_ENCODED) {
	uf->stream = newEncodedStream(uf->stream, uf->encoding);
	uf->encoding = ENC_7BIT;
    }
    for (d = compression_decoders; d->type != CMP_NOCOMPRESS; d++) {
	if (uf->compression == d->type) {
	    if (d->auxbin_p)
		expand_cmd = auxbinFile(d->cmd);
	    else
		expand_cmd = d->cmd;
	    expand_name = d->name;
	    ext = d->ext;
	    use_d_arg = d->use_d_arg;
	    break;
	}
    }
    uf->compression = CMP_NOCOMPRESS;

    if (uf->scheme != SCM_LOCAL
#ifdef USE_IMAGE
	&& !image_source
#endif
	) {
	tmpf = tmpfname(TMPF_DFL, ext)->ptr;
    }

    /* child1 -- stdout|f1=uf -> parent */
    pid1 = open_pipe_rw(&f1, NULL);
    if (pid1 < 0) {
	UFclose(uf);
	return;
    }
    if (pid1 == 0) {
	/* child */
	pid_t pid2;
	FILE *f2 = stdin;

	/* uf -> child2 -- stdout|stdin -> child1 */
	pid2 = open_pipe_rw(&f2, NULL);
	if (pid2 < 0) {
	    UFclose(uf);
	    exit(1);
	}
	if (pid2 == 0) {
	    /* child2 */
	    char *buf = NewWithoutGC_N(char, SAVE_BUF_SIZE);
	    int count;
	    FILE *f = NULL;

	    setup_child(TRUE, 2, UFfileno(uf));
	    if (tmpf)
		f = fopen(tmpf, "wb");
	    while ((count = ISread_n(uf->stream, buf, SAVE_BUF_SIZE)) > 0) {
		if (fwrite(buf, 1, count, stdout) != count)
		    break;
		if (f && fwrite(buf, 1, count, f) != count)
		    break;
	    }
	    UFclose(uf);
	    if (f)
		fclose(f);
	    xfree(buf);
	    exit(0);
	}
	/* child1 */
	dup2(1, 2);		/* stderr>&stdout */
	setup_child(TRUE, -1, -1);
	if (use_d_arg)
	    execlp(expand_cmd, expand_name, "-d", NULL);
	else
	    execlp(expand_cmd, expand_name, NULL);
	exit(1);
    }
    if (tmpf) {
	if (src)
	    *src = tmpf;
	else
	    uf->scheme = SCM_LOCAL;
    }
    UFhalfclose(uf);
    uf->stream = newFileStream(f1, (void (*)())fclose);
#endif /* __MINGW32_VERSION */
}

static FILE *
lessopen_stream(char *path)
{
    char *lessopen;
    FILE *fp;

    lessopen = getenv("LESSOPEN");
    if (lessopen == NULL) {
	return NULL;
    }
    if (lessopen[0] == '\0') {
	return NULL;
    }

    if (lessopen[0] == '|') {
	/* pipe mode */
	Str tmpf;
	int c;

	++lessopen;
	tmpf = Sprintf(lessopen, shell_quote(path));
	fp = popen(tmpf->ptr, "r");
	if (fp == NULL) {
	    return NULL;
	}
	c = getc(fp);
	if (c == EOF) {
	    pclose(fp);
	    return NULL;
	}
	ungetc(c, fp);
    }
    else {
	/* filename mode */
	/* not supported m(__)m */
	fp = NULL;
    }
    return fp;
}

#if 0
void
reloadBuffer(Buffer *buf)
{
    URLFile uf;

    if (buf->sourcefile == NULL || buf->pagerSource != NULL)
	return;
    init_stream(&uf, SCM_UNKNOWN, NULL);
    examineFile(buf->mailcap_source ? buf->mailcap_source : buf->sourcefile,
		&uf);
    if (uf.stream == NULL)
	return;
    is_redisplay = TRUE;
    buf->allLine = 0;
    buf->href = NULL;
    buf->name = NULL;
    buf->img = NULL;
    buf->formitem = NULL;
    buf->linklist = NULL;
    buf->maplist = NULL;
    if (buf->hmarklist)
	buf->hmarklist->nmark = 0;
    if (buf->imarklist)
	buf->imarklist->nmark = 0;
    if (is_html_type(buf->type))
	loadHTMLBuffer(&uf, buf);
    else
	loadBuffer(&uf, buf);
    UFclose(&uf);
    is_redisplay = FALSE;
}
#endif

static char *
guess_filename(char *file)
{
    char *p = NULL, *s;

    if (file != NULL)
	p = mybasename(file);
    if (p == NULL || *p == '\0')
	return DEF_SAVE_FILE;
    s = p;
    if (*p == '#')
	p++;
    while (*p != '\0') {
	if ((*p == '#' && *(p + 1) != '\0') || *p == '?') {
	    *p = '\0';
	    break;
	}
	p++;
    }
    return s;
}

char *
guess_save_name(Buffer *buf, char *path)
{
    if (buf && buf->document_header) {
	Str name = NULL;
	char *p, *q;
	if ((p = checkHeader(buf, "Content-Disposition:")) != NULL &&
	    (q = strcasestr(p, "filename")) != NULL &&
	    (q == p || IS_SPACE(*(q - 1)) || *(q - 1) == ';') &&
	    matchattr(q, "filename", 8, &name))
	    path = name->ptr;
	else if ((p = checkHeader(buf, "Content-Type:")) != NULL &&
		 (q = strcasestr(p, "name")) != NULL &&
		 (q == p || IS_SPACE(*(q - 1)) || *(q - 1) == ';') &&
		 matchattr(q, "name", 4, &name))
	    path = name->ptr;
    }
    return guess_filename(path);
}

/* Local Variables:    */
/* c-basic-offset: 4   */
/* tab-width: 8        */
/* End:                */
