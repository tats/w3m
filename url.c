/* $Id: url.c,v 1.100 2010/12/15 10:50:24 htrb Exp $ */
#include "fm.h"
#ifndef __MINGW32_VERSION
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock.h>
#endif /* __MINGW32_VERSION */

#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#include <sys/stat.h>
#ifdef __EMX__
#include <io.h>			/* ?? */
#endif				/* __EMX__ */

#include "html.h"
#include "Str.h"
#include "myctype.h"
#include "regex.h"

#ifdef USE_SSL
#ifndef SSLEAY_VERSION_NUMBER
#include <openssl/crypto.h>		/* SSLEAY_VERSION_NUMBER may be here */
#endif
#include <openssl/err.h>
#endif

#ifdef	__WATT32__
#define	write(a,b,c)	write_s(a,b,c)
#endif				/* __WATT32__ */

#ifdef __MINGW32_VERSION
#define	write(a,b,c)	send(a,b,c, 0)
#define close(fd)	closesocket(fd)
#endif

#ifdef INET6
/* see rc.c, "dns_order" and dnsorders[] */
int ai_family_order_table[7][3] = {
    {PF_UNSPEC, PF_UNSPEC, PF_UNSPEC},	/* 0:unspec */
    {PF_INET, PF_INET6, PF_UNSPEC},	/* 1:inet inet6 */
    {PF_INET6, PF_INET, PF_UNSPEC},	/* 2:inet6 inet */
    {PF_UNSPEC, PF_UNSPEC, PF_UNSPEC},  /* 3: --- */
    {PF_INET, PF_UNSPEC, PF_UNSPEC},    /* 4:inet */
    {PF_UNSPEC, PF_UNSPEC, PF_UNSPEC},  /* 5: --- */
    {PF_INET6, PF_UNSPEC, PF_UNSPEC},   /* 6:inet6 */
};
#endif				/* INET6 */

static JMP_BUF AbortLoading;

/* XXX: note html.h SCM_ */
static int
 DefaultPort[] = {
    80,				/* http */
    70,				/* gopher */
    21,				/* ftp */
    21,				/* ftpdir */
    0,				/* local - not defined */
    0,				/* local-CGI - not defined? */
    0,				/* exec - not defined? */
    119,			/* nntp */
    119,			/* nntp group */
    119,			/* news */
    119,			/* news group */
    0,				/* data - not defined */
    0,				/* mailto - not defined */
#ifdef USE_SSL
    443,			/* https */
#endif				/* USE_SSL */
};

struct cmdtable schemetable[] = {
    {"http", SCM_HTTP},
    {"gopher", SCM_GOPHER},
    {"ftp", SCM_FTP},
    {"local", SCM_LOCAL},
    {"file", SCM_LOCAL},
    /*  {"exec", SCM_EXEC}, */
    {"nntp", SCM_NNTP},
    /*  {"nntp", SCM_NNTP_GROUP}, */
    {"news", SCM_NEWS},
    /*  {"news", SCM_NEWS_GROUP}, */
    {"data", SCM_DATA},
#ifndef USE_W3MMAILER
    {"mailto", SCM_MAILTO},
#endif
#ifdef USE_SSL
    {"https", SCM_HTTPS},
#endif				/* USE_SSL */
    {NULL, SCM_UNKNOWN},
};

static struct table2 DefaultGuess[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"shtml", "text/html"},
    {"xhtml", "application/xhtml+xml"},
    {"gif", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"xbm", "image/xbm"},
    {"au", "audio/basic"},
    {"gz", "application/x-gzip"},
    {"Z", "application/x-compress"},
    {"bz2", "application/x-bzip"},
    {"tar", "application/x-tar"},
    {"zip", "application/x-zip"},
    {"lha", "application/x-lha"},
    {"lzh", "application/x-lha"},
    {"ps", "application/postscript"},
    {"pdf", "application/pdf"},
    {NULL, NULL}
};

static void add_index_file(ParsedURL *pu, URLFile *uf);

/* #define HTTP_DEFAULT_FILE    "/index.html" */

#ifndef HTTP_DEFAULT_FILE
#define HTTP_DEFAULT_FILE "/"
#endif				/* not HTTP_DEFAULT_FILE */

#ifdef SOCK_DEBUG
#include <stdarg.h>

static void
sock_log(char *message, ...)
{
    FILE *f = fopen("zzzsocklog", "a");
    va_list va;

    if (f == NULL)
	return;
    va_start(va, message);
    vfprintf(f, message, va);
    fclose(f);
}

#endif

static TextList *mimetypes_list;
static struct table2 **UserMimeTypes;

static struct table2 *
loadMimeTypes(char *filename)
{
    FILE *f;
    char *d, *type;
    int i, n;
    Str tmp;
    struct table2 *mtypes;

    f = fopen(expandPath(filename), "r");
    if (f == NULL)
	return NULL;
    n = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	d = tmp->ptr;
	if (d[0] != '#') {
	    d = strtok(d, " \t\n\r");
	    if (d != NULL) {
		d = strtok(NULL, " \t\n\r");
		for (i = 0; d != NULL; i++)
		    d = strtok(NULL, " \t\n\r");
		n += i;
	    }
	}
    }
    fseek(f, 0, 0);
    mtypes = New_N(struct table2, n + 1);
    i = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	d = tmp->ptr;
	if (d[0] == '#')
	    continue;
	type = strtok(d, " \t\n\r");
	if (type == NULL)
	    continue;
	while (1) {
	    d = strtok(NULL, " \t\n\r");
	    if (d == NULL)
		break;
	    mtypes[i].item1 = Strnew_charp(d)->ptr;
	    mtypes[i].item2 = Strnew_charp(type)->ptr;
	    i++;
	}
    }
    mtypes[i].item1 = NULL;
    mtypes[i].item2 = NULL;
    fclose(f);
    return mtypes;
}

void
initMimeTypes()
{
    int i;
    TextListItem *tl;

    if (non_null(mimetypes_files))
	mimetypes_list = make_domain_list(mimetypes_files);
    else
	mimetypes_list = NULL;
    if (mimetypes_list == NULL)
	return;
    UserMimeTypes = New_N(struct table2 *, mimetypes_list->nitem);
    for (i = 0, tl = mimetypes_list->first; tl; i++, tl = tl->next)
	UserMimeTypes[i] = loadMimeTypes(tl->ptr);
}

static char *
DefaultFile(int scheme)
{
    switch (scheme) {
    case SCM_HTTP:
#ifdef USE_SSL
    case SCM_HTTPS:
#endif				/* USE_SSL */
	return allocStr(HTTP_DEFAULT_FILE, -1);
#ifdef USE_GOPHER
    case SCM_GOPHER:
	return allocStr("1", -1);
#endif				/* USE_GOPHER */
    case SCM_LOCAL:
    case SCM_LOCAL_CGI:
    case SCM_FTP:
    case SCM_FTPDIR:
	return allocStr("/", -1);
    }
    return NULL;
}

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
    SIGNAL_RETURN;
}

#ifdef USE_SSL
SSL_CTX *ssl_ctx = NULL;

void
free_ssl_ctx()
{
    if (ssl_ctx != NULL)
	SSL_CTX_free(ssl_ctx);
    ssl_ctx = NULL;
    ssl_accept_this_site(NULL);
}

#if SSLEAY_VERSION_NUMBER >= 0x00905100
#include <openssl/rand.h>
static void
init_PRNG()
{
    char buffer[256];
    const char *file;
    long l;
    if (RAND_status())
	return;
    if ((file = RAND_file_name(buffer, sizeof(buffer)))) {
#ifdef USE_EGD
	if (RAND_egd(file) > 0)
	    return;
#endif
	RAND_load_file(file, -1);
    }
    if (RAND_status())
	goto seeded;
    srand48((long)time(NULL));
    while (!RAND_status()) {
	l = lrand48();
	RAND_seed((unsigned char *)&l, sizeof(long));
    }
  seeded:
    if (file)
	RAND_write_file(file);
}
#endif				/* SSLEAY_VERSION_NUMBER >= 0x00905100 */

static SSL *
openSSLHandle(int sock, char *hostname, char **p_cert)
{
    SSL *handle = NULL;
    static char *old_ssl_forbid_method = NULL;
#ifdef USE_SSL_VERIFY
    static int old_ssl_verify_server = -1;
#endif

    if (old_ssl_forbid_method != ssl_forbid_method
	&& (!old_ssl_forbid_method || !ssl_forbid_method ||
	    strcmp(old_ssl_forbid_method, ssl_forbid_method))) {
	old_ssl_forbid_method = ssl_forbid_method;
#ifdef USE_SSL_VERIFY
	ssl_path_modified = 1;
#else
	free_ssl_ctx();
#endif
    }
#ifdef USE_SSL_VERIFY
    if (old_ssl_verify_server != ssl_verify_server) {
	old_ssl_verify_server = ssl_verify_server;
	ssl_path_modified = 1;
    }
    if (ssl_path_modified) {
	free_ssl_ctx();
	ssl_path_modified = 0;
    }
#endif				/* defined(USE_SSL_VERIFY) */
    if (ssl_ctx == NULL) {
	int option;
#if SSLEAY_VERSION_NUMBER < 0x0800
	ssl_ctx = SSL_CTX_new();
	X509_set_default_verify_paths(ssl_ctx->cert);
#else				/* SSLEAY_VERSION_NUMBER >= 0x0800 */
	SSLeay_add_ssl_algorithms();
	SSL_load_error_strings();
	if (!(ssl_ctx = SSL_CTX_new(SSLv23_client_method())))
	    goto eend;
	option = SSL_OP_ALL;
	if (ssl_forbid_method) {
	    if (strchr(ssl_forbid_method, '2'))
		option |= SSL_OP_NO_SSLv2;
	    if (strchr(ssl_forbid_method, '3'))
		option |= SSL_OP_NO_SSLv3;
	    if (strchr(ssl_forbid_method, 't'))
		option |= SSL_OP_NO_TLSv1;
	    if (strchr(ssl_forbid_method, 'T'))
		option |= SSL_OP_NO_TLSv1;
	}
	SSL_CTX_set_options(ssl_ctx, option);
#ifdef USE_SSL_VERIFY
	/* derived from openssl-0.9.5/apps/s_{client,cb}.c */
#if 1				/* use SSL_get_verify_result() to verify cert */
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
#else
	SSL_CTX_set_verify(ssl_ctx,
			   ssl_verify_server ? SSL_VERIFY_PEER :
			   SSL_VERIFY_NONE, NULL);
#endif
	if (ssl_cert_file != NULL && *ssl_cert_file != '\0') {
	    int ng = 1;
	    if (SSL_CTX_use_certificate_file
		(ssl_ctx, ssl_cert_file, SSL_FILETYPE_PEM) > 0) {
		char *key_file = (ssl_key_file == NULL
				  || *ssl_key_file ==
				  '\0') ? ssl_cert_file : ssl_key_file;
		if (SSL_CTX_use_PrivateKey_file
		    (ssl_ctx, key_file, SSL_FILETYPE_PEM) > 0)
		    if (SSL_CTX_check_private_key(ssl_ctx))
			ng = 0;
	    }
	    if (ng) {
		free_ssl_ctx();
		goto eend;
	    }
	}
	if ((!ssl_ca_file && !ssl_ca_path)
	    || SSL_CTX_load_verify_locations(ssl_ctx, ssl_ca_file, ssl_ca_path))
#endif				/* defined(USE_SSL_VERIFY) */
	    SSL_CTX_set_default_verify_paths(ssl_ctx);
#endif				/* SSLEAY_VERSION_NUMBER >= 0x0800 */
    }
    handle = SSL_new(ssl_ctx);
    SSL_set_fd(handle, sock);
#if SSLEAY_VERSION_NUMBER >= 0x00905100
    init_PRNG();
#endif				/* SSLEAY_VERSION_NUMBER >= 0x00905100 */
#if (SSLEAY_VERSION_NUMBER >= 0x00908070) && !defined(OPENSSL_NO_TLSEXT)
    SSL_set_tlsext_host_name(handle,hostname);
#endif				/* (SSLEAY_VERSION_NUMBER >= 0x00908070) && !defined(OPENSSL_NO_TLSEXT) */
    if (SSL_connect(handle) > 0) {
	Str serv_cert = ssl_get_certificate(handle, hostname);
	if (serv_cert) {
	    *p_cert = serv_cert->ptr;
	    return handle;
	}
	close(sock);
	SSL_free(handle);
	return NULL;
    }
  eend:
    close(sock);
    if (handle)
	SSL_free(handle);
    /* FIXME: gettextize? */
    disp_err_message(Sprintf
		     ("SSL error: %s",
		      ERR_error_string(ERR_get_error(), NULL))->ptr, FALSE);
    return NULL;
}

static void
SSL_write_from_file(SSL * ssl, char *file)
{
    FILE *fd;
    int c;
    char buf[1];
    fd = fopen(file, "r");
    if (fd != NULL) {
	while ((c = fgetc(fd)) != EOF) {
	    buf[0] = c;
	    SSL_write(ssl, buf, 1);
	}
	fclose(fd);
    }
}

#endif				/* USE_SSL */

static void
write_from_file(int sock, char *file)
{
    FILE *fd;
    int c;
    char buf[1];
    fd = fopen(file, "r");
    if (fd != NULL) {
	while ((c = fgetc(fd)) != EOF) {
	    buf[0] = c;
	    write(sock, buf, 1);
	}
	fclose(fd);
    }
}

ParsedURL *
baseURL(Buffer *buf)
{
    if (buf->bufferprop & BP_NO_URL) {
	/* no URL is defined for the buffer */
	return NULL;
    }
    if (buf->baseURL != NULL) {
	/* <BASE> tag is defined in the document */
	return buf->baseURL;
    }
    else
	return &buf->currentURL;
}

int
openSocket(char *const hostname,
	   char *remoteport_name, unsigned short remoteport_num)
{
    volatile int sock = -1;
#ifdef INET6
    int *af;
    struct addrinfo hints, *res0, *res;
    int error;
    char *hname;
#else				/* not INET6 */
    struct sockaddr_in hostaddr;
    struct hostent *entry;
    struct protoent *proto;
    unsigned short s_port;
    int a1, a2, a3, a4;
    unsigned long adr;
#endif				/* not INET6 */
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

    if (fmInitialized) {
	/* FIXME: gettextize? */
	message(Sprintf("Opening socket...")->ptr, 0, 0);
	refresh();
    }
    if (SETJMP(AbortLoading) != 0) {
#ifdef SOCK_DEBUG
	sock_log("openSocket() failed. reason: user abort\n");
#endif
	if (sock >= 0)
	    close(sock);
	goto error;
    }
    TRAP_ON;
    if (hostname == NULL) {
#ifdef SOCK_DEBUG
	sock_log("openSocket() failed. reason: Bad hostname \"%s\"\n",
		 hostname);
#endif
	goto error;
    }

#ifdef INET6
    /* rfc2732 compliance */
    hname = hostname;
    if (hname != NULL && hname[0] == '[' && hname[strlen(hname) - 1] == ']') {
	hname = allocStr(hostname + 1, -1);
	hname[strlen(hname) - 1] = '\0';
	if (strspn(hname, "0123456789abcdefABCDEF:.") != strlen(hname))
	    goto error;
    }
    for (af = ai_family_order_table[DNS_order];; af++) {
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = *af;
	hints.ai_socktype = SOCK_STREAM;
	if (remoteport_num != 0) {
	    Str portbuf = Sprintf("%d", remoteport_num);
	    error = getaddrinfo(hname, portbuf->ptr, &hints, &res0);
	}
	else {
	    error = -1;
	}
	if (error && remoteport_name && remoteport_name[0] != '\0') {
	    /* try default port */
	    error = getaddrinfo(hname, remoteport_name, &hints, &res0);
	}
	if (error) {
	    if (*af == PF_UNSPEC) {
		goto error;
	    }
	    /* try next ai family */
	    continue;
	}
	sock = -1;
	for (res = res0; res; res = res->ai_next) {
	    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	    if (sock < 0) {
		continue;
	    }
	    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
		close(sock);
		sock = -1;
		continue;
	    }
	    break;
	}
	if (sock < 0) {
	    freeaddrinfo(res0);
	    if (*af == PF_UNSPEC) {
		goto error;
	    }
	    /* try next ai family */
	    continue;
	}
	freeaddrinfo(res0);
	break;
    }
#else				/* not INET6 */
    s_port = htons(remoteport_num);
    bzero((char *)&hostaddr, sizeof(struct sockaddr_in));
    if ((proto = getprotobyname("tcp")) == NULL) {
	/* protocol number of TCP is 6 */
	proto = New(struct protoent);
	proto->p_proto = 6;
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0) {
#ifdef SOCK_DEBUG
	sock_log("openSocket: socket() failed. reason: %s\n", strerror(errno));
#endif
	goto error;
    }
    regexCompile("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$", 0);
    if (regexMatch(hostname, -1, 1)) {
	sscanf(hostname, "%d.%d.%d.%d", &a1, &a2, &a3, &a4);
	adr = htonl((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);
	bcopy((void *)&adr, (void *)&hostaddr.sin_addr, sizeof(long));
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port = s_port;
	if (fmInitialized) {
	    message(Sprintf("Connecting to %s", hostname)->ptr, 0, 0);
	    refresh();
	}
	if (connect(sock, (struct sockaddr *)&hostaddr,
		    sizeof(struct sockaddr_in)) < 0) {
#ifdef SOCK_DEBUG
	    sock_log("openSocket: connect() failed. reason: %s\n",
		     strerror(errno));
#endif
	    goto error;
	}
    }
    else {
	char **h_addr_list;
	int result = -1;
	if (fmInitialized) {
	    message(Sprintf("Performing hostname lookup on %s", hostname)->ptr,
		    0, 0);
	    refresh();
	}
	if ((entry = gethostbyname(hostname)) == NULL) {
#ifdef SOCK_DEBUG
	    sock_log("openSocket: gethostbyname() failed. reason: %s\n",
		     strerror(errno));
#endif
	    goto error;
	}
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port = s_port;
	for (h_addr_list = entry->h_addr_list; *h_addr_list; h_addr_list++) {
	    bcopy((void *)h_addr_list[0], (void *)&hostaddr.sin_addr,
		  entry->h_length);
#ifdef SOCK_DEBUG
	    adr = ntohl(*(long *)&hostaddr.sin_addr);
	    sock_log("openSocket: connecting %d.%d.%d.%d\n",
		     (adr >> 24) & 0xff,
		     (adr >> 16) & 0xff, (adr >> 8) & 0xff, adr & 0xff);
#endif
	    if (fmInitialized) {
		message(Sprintf("Connecting to %s", hostname)->ptr, 0, 0);
		refresh();
	    }
	    if ((result = connect(sock, (struct sockaddr *)&hostaddr,
				  sizeof(struct sockaddr_in))) == 0) {
		break;
	    }
#ifdef SOCK_DEBUG
	    else {
		sock_log("openSocket: connect() failed. reason: %s\n",
			 strerror(errno));
	    }
#endif
	}
	if (result < 0) {
	    goto error;
	}
    }
#endif				/* not INET6 */

    TRAP_OFF;
    return sock;
  error:
    TRAP_OFF;
    return -1;

}


#define COPYPATH_SPC_ALLOW 0
#define COPYPATH_SPC_IGNORE 1
#define COPYPATH_SPC_REPLACE 2

static char *
copyPath(char *orgpath, int length, int option)
{
    Str tmp = Strnew();
    while (*orgpath && length != 0) {
	if (IS_SPACE(*orgpath)) {
	    switch (option) {
	    case COPYPATH_SPC_ALLOW:
		Strcat_char(tmp, *orgpath);
		break;
	    case COPYPATH_SPC_IGNORE:
		/* do nothing */
		break;
	    case COPYPATH_SPC_REPLACE:
		Strcat_charp(tmp, "%20");
		break;
	    }
	}
	else
	    Strcat_char(tmp, *orgpath);
	orgpath++;
	length--;
    }
    return tmp->ptr;
}

void
parseURL(char *url, ParsedURL *p_url, ParsedURL *current)
{
    char *p, *q;
    Str tmp;

    url = url_quote(url);	/* quote 0x01-0x20, 0x7F-0xFF */

    p = url;
    p_url->scheme = SCM_MISSING;
    p_url->port = 0;
    p_url->user = NULL;
    p_url->pass = NULL;
    p_url->host = NULL;
    p_url->is_nocache = 0;
    p_url->file = NULL;
    p_url->real_file = NULL;
    p_url->query = NULL;
    p_url->label = NULL;

    /* RFC1808: Relative Uniform Resource Locators
     * 4.  Resolving Relative URLs
     */
    if (*url == '\0' || *url == '#') {
	if (current)
	    copyParsedURL(p_url, current);
	goto do_label;
    }
#if defined( __EMX__ ) || defined( __CYGWIN__ )
    if (!strncmp(url, "file://localhost/", 17)) {
	p_url->scheme = SCM_LOCAL;
	p += 17 - 1;
	url += 17 - 1;
    }
#endif
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    if (IS_ALPHA(*p) && (p[1] == ':' || p[1] == '|')) {
	p_url->scheme = SCM_LOCAL;
	goto analyze_file;
    }
#endif				/* SUPPORT_DOS_DRIVE_PREFIX */
    /* search for scheme */
    p_url->scheme = getURLScheme(&p);
    if (p_url->scheme == SCM_MISSING) {
	/* scheme part is not found in the url. This means either
	 * (a) the url is relative to the current or (b) the url
	 * denotes a filename (therefore the scheme is SCM_LOCAL).
	 */
	if (current) {
	    switch (current->scheme) {
	    case SCM_LOCAL:
	    case SCM_LOCAL_CGI:
		p_url->scheme = SCM_LOCAL;
		break;
	    case SCM_FTP:
	    case SCM_FTPDIR:
		p_url->scheme = SCM_FTP;
		break;
#ifdef USE_NNTP
	    case SCM_NNTP:
	    case SCM_NNTP_GROUP:
		p_url->scheme = SCM_NNTP;
		break;
	    case SCM_NEWS:
	    case SCM_NEWS_GROUP:
		p_url->scheme = SCM_NEWS;
		break;
#endif
	    default:
		p_url->scheme = current->scheme;
		break;
	    }
	}
	else
	    p_url->scheme = SCM_LOCAL;
	p = url;
	if (!strncmp(p, "//", 2)) {
	    /* URL begins with // */
	    /* it means that 'scheme:' is abbreviated */
	    p += 2;
	    goto analyze_url;
	}
	/* the url doesn't begin with '//' */
	goto analyze_file;
    }
    /* scheme part has been found */
    if (p_url->scheme == SCM_UNKNOWN) {
	p_url->file = allocStr(url, -1);
	return;
    }
    /* get host and port */
    if (p[0] != '/' || p[1] != '/') {	/* scheme:foo or scheme:/foo */
	p_url->host = NULL;
	if (p_url->scheme != SCM_UNKNOWN)
	    p_url->port = DefaultPort[p_url->scheme];
	else
	    p_url->port = 0;
	goto analyze_file;
    }
    /* after here, p begins with // */
    if (p_url->scheme == SCM_LOCAL) {	/* file://foo           */
#ifdef __EMX__
	p += 2;
	goto analyze_file;
#else
	if (p[2] == '/' || p[2] == '~'
	    /* <A HREF="file:///foo">file:///foo</A>  or <A HREF="file://~user">file://~user</A> */
#ifdef SUPPORT_DOS_DRIVE_PREFIX
	    || (IS_ALPHA(p[2]) && (p[3] == ':' || p[3] == '|'))
	    /* <A HREF="file://DRIVE/foo">file://DRIVE/foo</A> */
#endif				/* SUPPORT_DOS_DRIVE_PREFIX */
	    ) {
	    p += 2;
	    goto analyze_file;
	}
#endif				/* __EMX__ */
    }
    p += 2;			/* scheme://foo         */
    /*          ^p is here  */
  analyze_url:
    q = p;
#ifdef INET6
    if (*q == '[') {		/* rfc2732,rfc2373 compliance */
	p++;
	while (IS_XDIGIT(*p) || *p == ':' || *p == '.')
	    p++;
	if (*p != ']' || (*(p + 1) && strchr(":/?#", *(p + 1)) == NULL))
	    p = q;
    }
#endif
    while (*p && strchr(":/@?#", *p) == NULL)
	p++;
    switch (*p) {
    case ':':
	/* scheme://user:pass@host or
	 * scheme://host:port
	 */
	p_url->host = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	q = ++p;
	while (*p && strchr("@/?#", *p) == NULL)
	    p++;
	if (*p == '@') {
	    /* scheme://user:pass@...       */
	    p_url->pass = copyPath(q, p - q, COPYPATH_SPC_ALLOW);
	    q = ++p;
	    p_url->user = p_url->host;
	    p_url->host = NULL;
	    goto analyze_url;
	}
	/* scheme://host:port/ */
	tmp = Strnew_charp_n(q, p - q);
	p_url->port = atoi(tmp->ptr);
	/* *p is one of ['\0', '/', '?', '#'] */
	break;
    case '@':
	/* scheme://user@...            */
	p_url->user = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	q = ++p;
	goto analyze_url;
    case '\0':
	/* scheme://host                */
    case '/':
    case '?':
    case '#':
	p_url->host = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	p_url->port = DefaultPort[p_url->scheme];
	break;
    }
  analyze_file:
#ifndef SUPPORT_NETBIOS_SHARE
    if (p_url->scheme == SCM_LOCAL && p_url->user == NULL &&
	p_url->host != NULL && *p_url->host != '\0' &&
	strcmp(p_url->host, "localhost")) {
	/*
	 * In the environments other than CYGWIN, a URL like 
	 * file://host/file is regarded as ftp://host/file.
	 * On the other hand, file://host/file on CYGWIN is
	 * regarded as local access to the file //host/file.
	 * `host' is a netbios-hostname, drive, or any other
	 * name; It is CYGWIN system call who interprets that.
	 */

	p_url->scheme = SCM_FTP;	/* ftp://host/... */
	if (p_url->port == 0)
	    p_url->port = DefaultPort[SCM_FTP];
    }
#endif
    if ((*p == '\0' || *p == '#' || *p == '?') && p_url->host == NULL) {
	p_url->file = "";
	goto do_query;
    }
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    if (p_url->scheme == SCM_LOCAL) {
	q = p;
	if (*q == '/')
	    q++;
	if (IS_ALPHA(q[0]) && (q[1] == ':' || q[1] == '|')) {
	    if (q[1] == '|') {
		p = allocStr(q, -1);
		p[1] = ':';
	    }
	    else
		p = q;
	}
    }
#endif

    q = p;
#ifdef USE_GOPHER
    if (p_url->scheme == SCM_GOPHER) {
	if (*q == '/')
	    q++;
	if (*q && q[0] != '/' && q[1] != '/' && q[2] == '/')
	    q++;
    }
#endif				/* USE_GOPHER */
    if (*p == '/')
	p++;
    if (*p == '\0' || *p == '#' || *p == '?') {	/* scheme://host[:port]/ */
	p_url->file = DefaultFile(p_url->scheme);
	goto do_query;
    }
#ifdef USE_GOPHER
    if (p_url->scheme == SCM_GOPHER && *p == 'R') {
	p++;
	tmp = Strnew();
	Strcat_char(tmp, *(p++));
	while (*p && *p != '/')
	    p++;
	Strcat_charp(tmp, p);
	while (*p)
	    p++;
	p_url->file = copyPath(tmp->ptr, -1, COPYPATH_SPC_IGNORE);
    }
    else
#endif				/* USE_GOPHER */
    {
	char *cgi = strchr(p, '?');
      again:
	while (*p && *p != '#' && p != cgi)
	    p++;
	if (*p == '#' && p_url->scheme == SCM_LOCAL) {
	    /* 
	     * According to RFC2396, # means the beginning of
	     * URI-reference, and # should be escaped.  But,
	     * if the scheme is SCM_LOCAL, the special
	     * treatment will apply to # for convinience.
	     */
	    if (p > q && *(p - 1) == '/' && (cgi == NULL || p < cgi)) {
		/* 
		 * # comes as the first character of the file name
		 * that means, # is not a label but a part of the file
		 * name.
		 */
		p++;
		goto again;
	    }
	    else if (*(p + 1) == '\0') {
		/* 
		 * # comes as the last character of the file name that
		 * means, # is not a label but a part of the file
		 * name.
		 */
		p++;
	    }
	}
	if (p_url->scheme == SCM_LOCAL || p_url->scheme == SCM_MISSING)
	    p_url->file = copyPath(q, p - q, COPYPATH_SPC_ALLOW);
	else
	    p_url->file = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
    }

  do_query:
    if (*p == '?') {
	q = ++p;
	while (*p && *p != '#')
	    p++;
	p_url->query = copyPath(q, p - q, COPYPATH_SPC_ALLOW);
    }
  do_label:
    if (p_url->scheme == SCM_MISSING) {
	p_url->scheme = SCM_LOCAL;
	p_url->file = allocStr(p, -1);
	p_url->label = NULL;
    }
    else if (*p == '#')
	p_url->label = allocStr(p + 1, -1);
    else
	p_url->label = NULL;
}

#define initParsedURL(p) bzero(p,sizeof(ParsedURL))
#define ALLOC_STR(s) ((s)==NULL?NULL:allocStr(s,-1))

void
copyParsedURL(ParsedURL *p, ParsedURL *q)
{
    p->scheme = q->scheme;
    p->port = q->port;
    p->is_nocache = q->is_nocache;
    p->user = ALLOC_STR(q->user);
    p->pass = ALLOC_STR(q->pass);
    p->host = ALLOC_STR(q->host);
    p->file = ALLOC_STR(q->file);
    p->real_file = ALLOC_STR(q->real_file);
    p->label = ALLOC_STR(q->label);
    p->query = ALLOC_STR(q->query);
}

void
parseURL2(char *url, ParsedURL *pu, ParsedURL *current)
{
    char *p;
    Str tmp;
    int relative_uri = FALSE;

    parseURL(url, pu, current);
#ifndef USE_W3MMAILER
    if (pu->scheme == SCM_MAILTO)
	return;
#endif
    if (pu->scheme == SCM_DATA)
	return;
    if (pu->scheme == SCM_NEWS || pu->scheme == SCM_NEWS_GROUP) {
	if (pu->file && !strchr(pu->file, '@') &&
	    (!(p = strchr(pu->file, '/')) || strchr(p + 1, '-') ||
	     *(p + 1) == '\0'))
	    pu->scheme = SCM_NEWS_GROUP;
	else
	    pu->scheme = SCM_NEWS;
	return;
    }
    if (pu->scheme == SCM_NNTP || pu->scheme == SCM_NNTP_GROUP) {
	if (pu->file && *pu->file == '/')
	    pu->file = allocStr(pu->file + 1, -1);
	if (pu->file && !strchr(pu->file, '@') &&
	    (!(p = strchr(pu->file, '/')) || strchr(p + 1, '-') ||
	     *(p + 1) == '\0'))
	    pu->scheme = SCM_NNTP_GROUP;
	else
	    pu->scheme = SCM_NNTP;
	if (current && (current->scheme == SCM_NNTP ||
			current->scheme == SCM_NNTP_GROUP)) {
	    if (pu->host == NULL) {
		pu->host = current->host;
		pu->port = current->port;
	    }
	}
	return;
    }
    if (pu->scheme == SCM_LOCAL) {
	char *q = expandName(file_unquote(pu->file));
#ifdef SUPPORT_DOS_DRIVE_PREFIX
	Str drive;
	if (IS_ALPHA(q[0]) && q[1] == ':') {
	    drive = Strnew_charp_n(q, 2);
	    Strcat_charp(drive, file_quote(q+2));
	    pu->file = drive->ptr;
	}
	else
#endif
	    pu->file = file_quote(q);
    }

    if (current && (pu->scheme == current->scheme ||
		    (pu->scheme == SCM_FTP && current->scheme == SCM_FTPDIR) ||
		    (pu->scheme == SCM_LOCAL &&
		     current->scheme == SCM_LOCAL_CGI))
	&& pu->host == NULL) {
	/* Copy omitted element from the current URL */
	pu->user = current->user;
	pu->pass = current->pass;
	pu->host = current->host;
	pu->port = current->port;
	if (pu->file && *pu->file) {
#ifdef USE_EXTERNAL_URI_LOADER
	    if (pu->scheme == SCM_UNKNOWN
		&& strchr(pu->file, ':') == NULL
		&& current && (p = strchr(current->file, ':')) != NULL) {
		pu->file = Sprintf("%s:%s",
				   allocStr(current->file,
					    p - current->file), pu->file)->ptr;
	    }
	    else
#endif
		if (
#ifdef USE_GOPHER
		       pu->scheme != SCM_GOPHER &&
#endif				/* USE_GOPHER */
		       pu->file[0] != '/'
#ifdef SUPPORT_DOS_DRIVE_PREFIX
		       && !(pu->scheme == SCM_LOCAL && IS_ALPHA(pu->file[0])
			    && pu->file[1] == ':')
#endif
		) {
		/* file is relative [process 1] */
		p = pu->file;
		if (current->file) {
		    tmp = Strnew_charp(current->file);
		    while (tmp->length > 0) {
			if (Strlastchar(tmp) == '/')
			    break;
			Strshrink(tmp, 1);
		    }
		    Strcat_charp(tmp, p);
		    pu->file = tmp->ptr;
		    relative_uri = TRUE;
		}
	    }
#ifdef USE_GOPHER
	    else if (pu->scheme == SCM_GOPHER && pu->file[0] == '/') {
		p = pu->file;
		pu->file = allocStr(p + 1, -1);
	    }
#endif				/* USE_GOPHER */
	}
	else {			/* scheme:[?query][#label] */
	    pu->file = current->file;
	    if (!pu->query)
		pu->query = current->query;
	}
	/* comment: query part need not to be completed
	 * from the current URL. */
    }
    if (pu->file) {
#ifdef __EMX__
	if (pu->scheme == SCM_LOCAL) {
	    if (strncmp(pu->file, "/$LIB/", 6)) {
		char abs[_MAX_PATH];

		_abspath(abs, file_unquote(pu->file), _MAX_PATH);
		pu->file = file_quote(cleanupName(abs));
	    }
	}
#else
	if (pu->scheme == SCM_LOCAL && pu->file[0] != '/' &&
#ifdef SUPPORT_DOS_DRIVE_PREFIX	/* for 'drive:' */
	    !(IS_ALPHA(pu->file[0]) && pu->file[1] == ':') &&
#endif
	    strcmp(pu->file, "-")) {
	    /* local file, relative path */
	    tmp = Strnew_charp(CurrentDir);
	    if (Strlastchar(tmp) != '/')
		Strcat_char(tmp, '/');
	    Strcat_charp(tmp, file_unquote(pu->file));
	    pu->file = file_quote(cleanupName(tmp->ptr));
	}
#endif
	else if (pu->scheme == SCM_HTTP
#ifdef USE_SSL
		 || pu->scheme == SCM_HTTPS
#endif
	    ) {
	    if (relative_uri) {
		/* In this case, pu->file is created by [process 1] above.
		 * pu->file may contain relative path (for example, 
		 * "/foo/../bar/./baz.html"), cleanupName() must be applied.
		 * When the entire abs_path is given, it still may contain
		 * elements like `//', `..' or `.' in the pu->file. It is 
		 * server's responsibility to canonicalize such path.
		 */
		pu->file = cleanupName(pu->file);
	    }
	}
	else if (
#ifdef USE_GOPHER
		    pu->scheme != SCM_GOPHER &&
#endif				/* USE_GOPHER */
		    pu->file[0] == '/') {
	    /*
	     * this happens on the following conditions:
	     * (1) ftp scheme (2) local, looks like absolute path.
	     * In both case, there must be no side effect with
	     * cleanupName(). (I hope so...)
	     */
	    pu->file = cleanupName(pu->file);
	}
	if (pu->scheme == SCM_LOCAL) {
#ifdef SUPPORT_NETBIOS_SHARE
	    if (pu->host && strcmp(pu->host, "localhost") != 0) {
		Str tmp = Strnew_charp("//");
		Strcat_m_charp(tmp, pu->host,
			       cleanupName(file_unquote(pu->file)), NULL);
		pu->real_file = tmp->ptr;
	    }
	    else
#endif
		pu->real_file = cleanupName(file_unquote(pu->file));
	}
    }
}

static Str
_parsedURL2Str(ParsedURL *pu, int pass)
{
    Str tmp;
    static char *scheme_str[] = {
	"http", "gopher", "ftp", "ftp", "file", "file", "exec", "nntp", "nntp",
	"news", "news", "data", "mailto",
#ifdef USE_SSL
	"https",
#endif				/* USE_SSL */
    };

    if (pu->scheme == SCM_MISSING) {
	return Strnew_charp("???");
    }
    else if (pu->scheme == SCM_UNKNOWN) {
	return Strnew_charp(pu->file);
    }
    if (pu->host == NULL && pu->file == NULL && pu->label != NULL) {
	/* local label */
	return Sprintf("#%s", pu->label);
    }
    if (pu->scheme == SCM_LOCAL && !strcmp(pu->file, "-")) {
	tmp = Strnew_charp("-");
	if (pu->label) {
	    Strcat_char(tmp, '#');
	    Strcat_charp(tmp, pu->label);
	}
	return tmp;
    }
    tmp = Strnew_charp(scheme_str[pu->scheme]);
    Strcat_char(tmp, ':');
#ifndef USE_W3MMAILER
    if (pu->scheme == SCM_MAILTO) {
	Strcat_charp(tmp, pu->file);
	if (pu->query) {
	    Strcat_char(tmp, '?');
	    Strcat_charp(tmp, pu->query);
	}
	return tmp;
    }
#endif
    if (pu->scheme == SCM_DATA) {
	Strcat_charp(tmp, pu->file);
	return tmp;
    }
#ifdef USE_NNTP
    if (pu->scheme != SCM_NEWS && pu->scheme != SCM_NEWS_GROUP)
#endif				/* USE_NNTP */
    {
	Strcat_charp(tmp, "//");
    }
    if (pu->user) {
	Strcat_charp(tmp, pu->user);
	if (pass && pu->pass) {
	    Strcat_char(tmp, ':');
	    Strcat_charp(tmp, pu->pass);
	}
	Strcat_char(tmp, '@');
    }
    if (pu->host) {
	Strcat_charp(tmp, pu->host);
	if (pu->port != DefaultPort[pu->scheme]) {
	    Strcat_char(tmp, ':');
	    Strcat(tmp, Sprintf("%d", pu->port));
	}
    }
    if (
#ifdef USE_NNTP
	   pu->scheme != SCM_NEWS && pu->scheme != SCM_NEWS_GROUP &&
#endif				/* USE_NNTP */
	   (pu->file == NULL || (pu->file[0] != '/'
#ifdef SUPPORT_DOS_DRIVE_PREFIX
				 && !(IS_ALPHA(pu->file[0])
				      && pu->file[1] == ':'
				      && pu->host == NULL)
#endif
	    )))
	Strcat_char(tmp, '/');
    Strcat_charp(tmp, pu->file);
    if (pu->scheme == SCM_FTPDIR && Strlastchar(tmp) != '/')
	Strcat_char(tmp, '/');
    if (pu->query) {
	Strcat_char(tmp, '?');
	Strcat_charp(tmp, pu->query);
    }
    if (pu->label) {
	Strcat_char(tmp, '#');
	Strcat_charp(tmp, pu->label);
    }
    return tmp;
}

Str
parsedURL2Str(ParsedURL *pu)
{
    return _parsedURL2Str(pu, FALSE);
}

int
getURLScheme(char **url)
{
    char *p = *url, *q;
    int i;
    int scheme = SCM_MISSING;

    while (*p && (IS_ALNUM(*p) || *p == '.' || *p == '+' || *p == '-'))
	p++;
    if (*p == ':') {		/* scheme found */
	scheme = SCM_UNKNOWN;
	for (i = 0; (q = schemetable[i].cmdname) != NULL; i++) {
	    int len = strlen(q);
	    if (!strncasecmp(q, *url, len) && (*url)[len] == ':') {
		scheme = schemetable[i].cmd;
		*url = p + 1;
		break;
	    }
	}
    }
    return scheme;
}

static char *
otherinfo(ParsedURL *target, ParsedURL *current, char *referer)
{
    Str s = Strnew();

    Strcat_charp(s, "User-Agent: ");
    if (UserAgent == NULL || *UserAgent == '\0')
	Strcat_charp(s, w3m_version);
    else
	Strcat_charp(s, UserAgent);
    Strcat_charp(s, "\r\n");

    Strcat_m_charp(s, "Accept: ", AcceptMedia, "\r\n", NULL);
    Strcat_m_charp(s, "Accept-Encoding: ", AcceptEncoding, "\r\n", NULL);
    Strcat_m_charp(s, "Accept-Language: ", AcceptLang, "\r\n", NULL);

    if (target->host) {
	Strcat_charp(s, "Host: ");
	Strcat_charp(s, target->host);
	if (target->port != DefaultPort[target->scheme])
	    Strcat(s, Sprintf(":%d", target->port));
	Strcat_charp(s, "\r\n");
    }
    if (target->is_nocache || NoCache) {
	Strcat_charp(s, "Pragma: no-cache\r\n");
	Strcat_charp(s, "Cache-control: no-cache\r\n");
    }
    if (!NoSendReferer) {
#ifdef USE_SSL
        if (current && current->scheme == SCM_HTTPS && target->scheme != SCM_HTTPS) {
	  /* Don't send Referer: if https:// -> http:// */
	}
	else
#endif
	if (referer == NULL && current && current->scheme != SCM_LOCAL &&
	    (current->scheme != SCM_FTP ||
	     (current->user == NULL && current->pass == NULL))) {
	    char *p = current->label;
	    Strcat_charp(s, "Referer: ");
	    current->label = NULL;
	    Strcat(s, parsedURL2Str(current));
	    current->label = p;
	    Strcat_charp(s, "\r\n");
	}
	else if (referer != NULL && referer != NO_REFERER) {
	    char *p = strchr(referer, '#');
	    Strcat_charp(s, "Referer: ");
	    if (p)
		Strcat_charp_n(s, referer, p - referer);
	    else
		Strcat_charp(s, referer);
	    Strcat_charp(s, "\r\n");
	}
    }
    return s->ptr;
}

Str
HTTPrequestMethod(HRequest *hr)
{
    switch (hr->command) {
    case HR_COMMAND_CONNECT:
	return Strnew_charp("CONNECT");
    case HR_COMMAND_POST:
	return Strnew_charp("POST");
	break;
    case HR_COMMAND_HEAD:
	return Strnew_charp("HEAD");
	break;
    case HR_COMMAND_GET:
    default:
	return Strnew_charp("GET");
    }
    return NULL;
}

Str
HTTPrequestURI(ParsedURL *pu, HRequest *hr)
{
    Str tmp = Strnew();
    if (hr->command == HR_COMMAND_CONNECT) {
	Strcat_charp(tmp, pu->host);
	Strcat(tmp, Sprintf(":%d", pu->port));
    }
    else if (hr->flag & HR_FLAG_LOCAL) {
	Strcat_charp(tmp, pu->file);
	if (pu->query) {
	    Strcat_char(tmp, '?');
	    Strcat_charp(tmp, pu->query);
	}
    }
    else {
	char *save_label = pu->label;
	pu->label = NULL;
	Strcat(tmp, _parsedURL2Str(pu, TRUE));
	pu->label = save_label;
    }
    return tmp;
}

static Str
HTTPrequest(ParsedURL *pu, ParsedURL *current, HRequest *hr, TextList *extra)
{
    Str tmp;
    TextListItem *i;
    int seen_www_auth = 0;
#ifdef USE_COOKIE
    Str cookie;
#endif				/* USE_COOKIE */
    tmp = HTTPrequestMethod(hr);
    Strcat_charp(tmp, " ");
    Strcat_charp(tmp, HTTPrequestURI(pu, hr)->ptr);
    Strcat_charp(tmp, " HTTP/1.0\r\n");
    if (hr->referer == NO_REFERER)
	Strcat_charp(tmp, otherinfo(pu, NULL, NULL));
    else
	Strcat_charp(tmp, otherinfo(pu, current, hr->referer));
    if (extra != NULL)
	for (i = extra->first; i != NULL; i = i->next) {
	    if (strncasecmp(i->ptr, "Authorization:",
			    sizeof("Authorization:") - 1) == 0) {
		seen_www_auth = 1;
#ifdef USE_SSL
		if (hr->command == HR_COMMAND_CONNECT)
		    continue;
#endif
	    }
	    if (strncasecmp(i->ptr, "Proxy-Authorization:",
			    sizeof("Proxy-Authorization:") - 1) == 0) {
#ifdef USE_SSL
		if (pu->scheme == SCM_HTTPS
		    && hr->command != HR_COMMAND_CONNECT)
		    continue;
#endif
	    }
	    Strcat_charp(tmp, i->ptr);
	}

#ifdef USE_COOKIE
    if (hr->command != HR_COMMAND_CONNECT &&
	use_cookie && (cookie = find_cookie(pu))) {
	Strcat_charp(tmp, "Cookie: ");
	Strcat(tmp, cookie);
	Strcat_charp(tmp, "\r\n");
	/* [DRAFT 12] s. 10.1 */
	if (cookie->ptr[0] != '$')
	    Strcat_charp(tmp, "Cookie2: $Version=\"1\"\r\n");
    }
#endif				/* USE_COOKIE */
    if (hr->command == HR_COMMAND_POST) {
	if (hr->request->enctype == FORM_ENCTYPE_MULTIPART) {
	    Strcat_charp(tmp, "Content-type: multipart/form-data; boundary=");
	    Strcat_charp(tmp, hr->request->boundary);
	    Strcat_charp(tmp, "\r\n");
	    Strcat(tmp,
		   Sprintf("Content-length: %ld\r\n", hr->request->length));
	    Strcat_charp(tmp, "\r\n");
	}
	else {
	    if (!override_content_type) {
		Strcat_charp(tmp,
			     "Content-type: application/x-www-form-urlencoded\r\n");
	    }
	    Strcat(tmp,
		   Sprintf("Content-length: %ld\r\n", hr->request->length));
	    if (header_string)
		Strcat(tmp, header_string);
	    Strcat_charp(tmp, "\r\n");
	    Strcat_charp_n(tmp, hr->request->body, hr->request->length);
	    Strcat_charp(tmp, "\r\n");
	}
    }
    else {
	if (header_string)
	    Strcat(tmp, header_string);
	Strcat_charp(tmp, "\r\n");
    }
#ifdef DEBUG
    fprintf(stderr, "HTTPrequest: [ %s ]\n\n", tmp->ptr);
#endif				/* DEBUG */
    return tmp;
}

void
init_stream(URLFile *uf, int scheme, InputStream stream)
{
    memset(uf, 0, sizeof(URLFile));
    uf->stream = stream;
    uf->scheme = scheme;
    uf->encoding = ENC_7BIT;
    uf->is_cgi = FALSE;
    uf->compression = CMP_NOCOMPRESS;
    uf->content_encoding = CMP_NOCOMPRESS;
    uf->guess_type = NULL;
    uf->ext = NULL;
    uf->modtime = -1;
}

URLFile
openURL(char *url, ParsedURL *pu, ParsedURL *current,
	URLOption *option, FormList *request, TextList *extra_header,
	URLFile *ouf, HRequest *hr, unsigned char *status)
{
    Str tmp;
    int sock, scheme;
    char *p, *q, *u;
    URLFile uf;
    HRequest hr0;
#ifdef USE_SSL
    SSL *sslh = NULL;
#endif				/* USE_SSL */

    if (hr == NULL)
	hr = &hr0;

    if (ouf) {
	uf = *ouf;
    }
    else {
	init_stream(&uf, SCM_MISSING, NULL);
    }

    u = url;
    scheme = getURLScheme(&u);
    if (current == NULL && scheme == SCM_MISSING && !ArgvIsURL)
	u = file_to_url(url);	/* force to local file */
    else
	u = url;
  retry:
    parseURL2(u, pu, current);
    if (pu->scheme == SCM_LOCAL && pu->file == NULL) {
	if (pu->label != NULL) {
	    /* #hogege is not a label but a filename */
	    Str tmp2 = Strnew_charp("#");
	    Strcat_charp(tmp2, pu->label);
	    pu->file = tmp2->ptr;
	    pu->real_file = cleanupName(file_unquote(pu->file));
	    pu->label = NULL;
	}
	else {
	    /* given URL must be null string */
#ifdef SOCK_DEBUG
	    sock_log("given URL must be null string\n");
#endif
	    return uf;
	}
    }

    uf.scheme = pu->scheme;
    uf.url = parsedURL2Str(pu)->ptr;
    pu->is_nocache = (option->flag & RG_NOCACHE);
    uf.ext = filename_extension(pu->file, 1);

    hr->command = HR_COMMAND_GET;
    hr->flag = 0;
    hr->referer = option->referer;
    hr->request = request;

    switch (pu->scheme) {
    case SCM_LOCAL:
    case SCM_LOCAL_CGI:
	if (request && request->body)
	    /* local CGI: POST */
	    uf.stream = newFileStream(localcgi_post(pu->real_file, pu->query,
						    request, option->referer),
				      (void (*)())fclose);
	else
	    /* lodal CGI: GET */
	    uf.stream = newFileStream(localcgi_get(pu->real_file, pu->query,
						   option->referer),
				      (void (*)())fclose);
	if (uf.stream) {
	    uf.is_cgi = TRUE;
	    uf.scheme = pu->scheme = SCM_LOCAL_CGI;
	    return uf;
	}
	examineFile(pu->real_file, &uf);
	if (uf.stream == NULL) {
	    if (dir_exist(pu->real_file)) {
		add_index_file(pu, &uf);
		if (uf.stream == NULL)
		    return uf;
	    }
	    else if (document_root != NULL) {
		tmp = Strnew_charp(document_root);
		if (Strlastchar(tmp) != '/' && pu->file[0] != '/')
		    Strcat_char(tmp, '/');
		Strcat_charp(tmp, pu->file);
		p = cleanupName(tmp->ptr);
		q = cleanupName(file_unquote(p));
		if (dir_exist(q)) {
		    pu->file = p;
		    pu->real_file = q;
		    add_index_file(pu, &uf);
		    if (uf.stream == NULL) {
			return uf;
		    }
		}
		else {
		    examineFile(q, &uf);
		    if (uf.stream) {
			pu->file = p;
			pu->real_file = q;
		    }
		}
	    }
	}
	if (uf.stream == NULL && retryAsHttp && url[0] != '/') {
	    if (scheme == SCM_MISSING || scheme == SCM_UNKNOWN) {
		/* retry it as "http://" */
		u = Strnew_m_charp("http://", url, NULL)->ptr;
		goto retry;
	    }
	}
	return uf;
    case SCM_FTP:
    case SCM_FTPDIR:
	if (pu->file == NULL)
	    pu->file = allocStr("/", -1);
	if (non_null(FTP_proxy) &&
	    !Do_not_use_proxy &&
	    pu->host != NULL && !check_no_proxy(pu->host)) {
	    hr->flag |= HR_FLAG_PROXY;
	    sock = openSocket(FTP_proxy_parsed.host,
			      schemetable[FTP_proxy_parsed.scheme].cmdname,
			      FTP_proxy_parsed.port);
	    if (sock < 0)
		return uf;
	    uf.scheme = SCM_HTTP;
	    tmp = HTTPrequest(pu, current, hr, extra_header);
	    write(sock, tmp->ptr, tmp->length);
	}
	else {
	    uf.stream = openFTPStream(pu, &uf);
	    uf.scheme = pu->scheme;
	    return uf;
	}
	break;
    case SCM_HTTP:
#ifdef USE_SSL
    case SCM_HTTPS:
#endif				/* USE_SSL */
	if (pu->file == NULL)
	    pu->file = allocStr("/", -1);
	if (request && request->method == FORM_METHOD_POST && request->body)
	    hr->command = HR_COMMAND_POST;
	if (request && request->method == FORM_METHOD_HEAD)
	    hr->command = HR_COMMAND_HEAD;
	if ((
#ifdef USE_SSL
		(pu->scheme == SCM_HTTPS) ? non_null(HTTPS_proxy) :
#endif				/* USE_SSL */
		non_null(HTTP_proxy)) && !Do_not_use_proxy &&
	    pu->host != NULL && !check_no_proxy(pu->host)) {
	    hr->flag |= HR_FLAG_PROXY;
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS && *status == HTST_CONNECT) {
		sock = ssl_socket_of(ouf->stream);
		if (!(sslh = openSSLHandle(sock, pu->host,
					   &uf.ssl_certificate))) {
		    *status = HTST_MISSING;
		    return uf;
		}
	    }
	    else if (pu->scheme == SCM_HTTPS) {
		sock = openSocket(HTTPS_proxy_parsed.host,
				  schemetable[HTTPS_proxy_parsed.scheme].
				  cmdname, HTTPS_proxy_parsed.port);
		sslh = NULL;
	    }
	    else {
#endif				/* USE_SSL */
		sock = openSocket(HTTP_proxy_parsed.host,
				  schemetable[HTTP_proxy_parsed.scheme].
				  cmdname, HTTP_proxy_parsed.port);
#ifdef USE_SSL
		sslh = NULL;
	    }
#endif				/* USE_SSL */
	    if (sock < 0) {
#ifdef SOCK_DEBUG
		sock_log("Can't open socket\n");
#endif
		return uf;
	    }
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS) {
		if (*status == HTST_NORMAL) {
		    hr->command = HR_COMMAND_CONNECT;
		    tmp = HTTPrequest(pu, current, hr, extra_header);
		    *status = HTST_CONNECT;
		}
		else {
		    hr->flag |= HR_FLAG_LOCAL;
		    tmp = HTTPrequest(pu, current, hr, extra_header);
		    *status = HTST_NORMAL;
		}
	    }
	    else
#endif				/* USE_SSL */
	    {
		tmp = HTTPrequest(pu, current, hr, extra_header);
		*status = HTST_NORMAL;
	    }
	}
	else {
	    sock = openSocket(pu->host,
			      schemetable[pu->scheme].cmdname, pu->port);
	    if (sock < 0) {
		*status = HTST_MISSING;
		return uf;
	    }
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS) {
		if (!(sslh = openSSLHandle(sock, pu->host,
					   &uf.ssl_certificate))) {
		    *status = HTST_MISSING;
		    return uf;
		}
	    }
#endif				/* USE_SSL */
	    hr->flag |= HR_FLAG_LOCAL;
	    tmp = HTTPrequest(pu, current, hr, extra_header);
	    *status = HTST_NORMAL;
	}
#ifdef USE_SSL
	if (pu->scheme == SCM_HTTPS) {
	    uf.stream = newSSLStream(sslh, sock);
	    if (sslh)
		SSL_write(sslh, tmp->ptr, tmp->length);
	    else
		write(sock, tmp->ptr, tmp->length);
	    if(w3m_reqlog){
		FILE *ff = fopen(w3m_reqlog, "a");
		if (sslh)
		    fputs("HTTPS: request via SSL\n", ff);
		else
		    fputs("HTTPS: request without SSL\n", ff);
		fwrite(tmp->ptr, sizeof(char), tmp->length, ff);
		fclose(ff);
	    }
	    if (hr->command == HR_COMMAND_POST &&
		request->enctype == FORM_ENCTYPE_MULTIPART) {
		if (sslh)
		    SSL_write_from_file(sslh, request->body);
		else
		    write_from_file(sock, request->body);
	    }
	    return uf;
	}
	else
#endif				/* USE_SSL */
	{
	    write(sock, tmp->ptr, tmp->length);
	    if(w3m_reqlog){
		FILE *ff = fopen(w3m_reqlog, "a");
		fwrite(tmp->ptr, sizeof(char), tmp->length, ff);
		fclose(ff);
	    }
	    if (hr->command == HR_COMMAND_POST &&
		request->enctype == FORM_ENCTYPE_MULTIPART)
		write_from_file(sock, request->body);
	}
	break;
#ifdef USE_GOPHER
    case SCM_GOPHER:
	if (non_null(GOPHER_proxy) &&
	    !Do_not_use_proxy &&
	    pu->host != NULL && !check_no_proxy(pu->host)) {
	    hr->flag |= HR_FLAG_PROXY;
	    sock = openSocket(GOPHER_proxy_parsed.host,
			      schemetable[GOPHER_proxy_parsed.scheme].cmdname,
			      GOPHER_proxy_parsed.port);
	    if (sock < 0)
		return uf;
	    uf.scheme = SCM_HTTP;
	    tmp = HTTPrequest(pu, current, hr, extra_header);
	}
	else {
	    sock = openSocket(pu->host,
			      schemetable[pu->scheme].cmdname, pu->port);
	    if (sock < 0)
		return uf;
	    if (pu->file == NULL)
		pu->file = "1";
	    tmp = Strnew_charp(file_unquote(pu->file));
	    Strcat_char(tmp, '\n');
	}
	write(sock, tmp->ptr, tmp->length);
	break;
#endif				/* USE_GOPHER */
#ifdef USE_NNTP
    case SCM_NNTP:
    case SCM_NNTP_GROUP:
    case SCM_NEWS:
    case SCM_NEWS_GROUP:
	if (pu->scheme == SCM_NNTP || pu->scheme == SCM_NEWS)
	    uf.scheme = SCM_NEWS;
	else
	    uf.scheme = SCM_NEWS_GROUP;
	uf.stream = openNewsStream(pu);
	return uf;
#endif				/* USE_NNTP */
    case SCM_DATA:
	if (pu->file == NULL)
	    return uf;
	p = Strnew_charp(pu->file)->ptr;
	q = strchr(p, ',');
	if (q == NULL)
	    return uf;
	*q++ = '\0';
	tmp = Strnew_charp(q);
	q = strrchr(p, ';');
	if (q != NULL && !strcmp(q, ";base64")) {
	    *q = '\0';
	    uf.encoding = ENC_BASE64;
	}
	else
	    tmp = Str_url_unquote(tmp, FALSE, FALSE);
	uf.stream = newStrStream(tmp);
	uf.guess_type = (*p != '\0') ? p : "text/plain";
	return uf;
    case SCM_UNKNOWN:
    default:
	return uf;
    }
    uf.stream = newInputStream(sock);
    return uf;
}

/* add index_file if exists */
static void
add_index_file(ParsedURL *pu, URLFile *uf)
{
    char *p, *q;
    TextList *index_file_list = NULL;
    TextListItem *ti;

    if (non_null(index_file))
	index_file_list = make_domain_list(index_file);
    if (index_file_list == NULL) {
	uf->stream = NULL;
	return;
    }
    for (ti = index_file_list->first; ti; ti = ti->next) {
	p = Strnew_m_charp(pu->file, "/", file_quote(ti->ptr), NULL)->ptr;
	p = cleanupName(p);
	q = cleanupName(file_unquote(p));
	examineFile(q, uf);
	if (uf->stream != NULL) {
	    pu->file = p;
	    pu->real_file = q;
	    return;
	}
    }
}

static char *
guessContentTypeFromTable(struct table2 *table, char *filename)
{
    struct table2 *t;
    char *p;
    if (table == NULL)
	return NULL;
    p = &filename[strlen(filename) - 1];
    while (filename < p && *p != '.')
	p--;
    if (p == filename)
	return NULL;
    p++;
    for (t = table; t->item1; t++) {
	if (!strcmp(p, t->item1))
	    return t->item2;
    }
    for (t = table; t->item1; t++) {
	if (!strcasecmp(p, t->item1))
	    return t->item2;
    }
    return NULL;
}

char *
guessContentType(char *filename)
{
    char *ret;
    int i;

    if (filename == NULL)
	return NULL;
    if (mimetypes_list == NULL)
	goto no_user_mimetypes;

    for (i = 0; i < mimetypes_list->nitem; i++) {
	if ((ret =
	     guessContentTypeFromTable(UserMimeTypes[i], filename)) != NULL)
	    return ret;
    }

  no_user_mimetypes:
    return guessContentTypeFromTable(DefaultGuess, filename);
}

TextList *
make_domain_list(char *domain_list)
{
    char *p;
    Str tmp;
    TextList *domains = NULL;

    p = domain_list;
    tmp = Strnew_size(64);
    while (*p) {
	while (*p && IS_SPACE(*p))
	    p++;
	Strclear(tmp);
	while (*p && !IS_SPACE(*p) && *p != ',')
	    Strcat_char(tmp, *p++);
	if (tmp->length > 0) {
	    if (domains == NULL)
		domains = newTextList();
	    pushText(domains, tmp->ptr);
	}
	while (*p && IS_SPACE(*p))
	    p++;
	if (*p == ',')
	    p++;
    }
    return domains;
}

static int
domain_match(char *pat, char *domain)
{
    if (domain == NULL)
	return 0;
    if (*pat == '.')
	pat++;
    for (;;) {
	if (!strcasecmp(pat, domain))
	    return 1;
	domain = strchr(domain, '.');
	if (domain == NULL)
	    return 0;
	domain++;
    }
}

int
check_no_proxy(char *domain)
{
    TextListItem *tl;
    volatile int ret = 0;
    MySignalHandler(*volatile prevtrap) (SIGNAL_ARG) = NULL;

    if (NO_proxy_domains == NULL || NO_proxy_domains->nitem == 0 ||
	domain == NULL)
	return 0;
    for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
	if (domain_match(tl->ptr, domain))
	    return 1;
    }
    if (!NOproxy_netaddr) {
	return 0;
    }
    /* 
     * to check noproxy by network addr
     */
    if (SETJMP(AbortLoading) != 0) {
	ret = 0;
	goto end;
    }
    TRAP_ON;
    {
#ifndef INET6
	struct hostent *he;
	int n;
	unsigned char **h_addr_list;
	char addr[4 * 16], buf[5];

	he = gethostbyname(domain);
	if (!he) {
	    ret = 0;
	    goto end;
	}
	for (h_addr_list = (unsigned char **)he->h_addr_list; *h_addr_list;
	     h_addr_list++) {
	    sprintf(addr, "%d", h_addr_list[0][0]);
	    for (n = 1; n < he->h_length; n++) {
		sprintf(buf, ".%d", h_addr_list[0][n]);
		strcat(addr, buf);
	    }
	    for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
		if (strncmp(tl->ptr, addr, strlen(tl->ptr)) == 0) {
		    ret = 1;
		    goto end;
		}
	    }
	}
#else				/* INET6 */
	int error;
	struct addrinfo hints;
	struct addrinfo *res, *res0;
	char addr[4 * 16];
	int *af;

	for (af = ai_family_order_table[DNS_order];; af++) {
	    memset(&hints, 0, sizeof(hints));
	    hints.ai_family = *af;
	    error = getaddrinfo(domain, NULL, &hints, &res0);
	    if (error) {
		if (*af == PF_UNSPEC) {
		    break;
		}
		/* try next */
		continue;
	    }
	    for (res = res0; res != NULL; res = res->ai_next) {
		switch (res->ai_family) {
		case AF_INET:
		    inet_ntop(AF_INET,
			      &((struct sockaddr_in *)res->ai_addr)->sin_addr,
			      addr, sizeof(addr));
		    break;
		case AF_INET6:
		    inet_ntop(AF_INET6,
			      &((struct sockaddr_in6 *)res->ai_addr)->
			      sin6_addr, addr, sizeof(addr));
		    break;
		default:
		    /* unknown */
		    continue;
		}
		for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
		    if (strncmp(tl->ptr, addr, strlen(tl->ptr)) == 0) {
			freeaddrinfo(res0);
			ret = 1;
			goto end;
		    }
		}
	    }
	    freeaddrinfo(res0);
	    if (*af == PF_UNSPEC) {
		break;
	    }
	}
#endif				/* INET6 */
    }
  end:
    TRAP_OFF;
    return ret;
}

char *
filename_extension(char *path, int is_url)
{
    char *last_dot = "", *p = path;
    int i;

    if (path == NULL)
	return last_dot;
    if (*p == '.')
	p++;
    for (; *p; p++) {
	if (*p == '.') {
	    last_dot = p;
	}
	else if (is_url && *p == '?')
	    break;
    }
    if (*last_dot == '.') {
	for (i = 1; last_dot[i] && i < 8; i++) {
	    if (is_url && !IS_ALNUM(last_dot[i]))
		break;
	}
	return allocStr(last_dot, i);
    }
    else
	return last_dot;
}

#ifdef USE_EXTERNAL_URI_LOADER
static struct table2 **urimethods;
static struct table2 default_urimethods[] = {
    {"mailto", "file:///$LIB/w3mmail.cgi?%s"},
    {NULL, NULL}
};

static struct table2 *
loadURIMethods(char *filename)
{
    FILE *f;
    int i, n;
    Str tmp;
    struct table2 *um;
    char *up, *p;

    f = fopen(expandPath(filename), "r");
    if (f == NULL)
	return NULL;
    i = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] != '#')
	    i++;
    }
    fseek(f, 0, 0);
    n = i;
    um = New_N(struct table2, n + 1);
    i = 0;
    while (tmp = Strfgets(f), tmp->length > 0) {
	if (tmp->ptr[0] == '#')
	    continue;
	while (IS_SPACE(Strlastchar(tmp)))
	    Strshrink(tmp, 1);
	for (up = p = tmp->ptr; *p != '\0'; p++) {
	    if (*p == ':') {
		um[i].item1 = Strnew_charp_n(up, p - up)->ptr;
		p++;
		break;
	    }
	}
	if (*p == '\0')
	    continue;
	while (*p != '\0' && IS_SPACE(*p))
	    p++;
	um[i].item2 = Strnew_charp(p)->ptr;
	i++;
    }
    um[i].item1 = NULL;
    um[i].item2 = NULL;
    fclose(f);
    return um;
}

void
initURIMethods()
{
    TextList *methodmap_list = NULL;
    TextListItem *tl;
    int i;

    if (non_null(urimethodmap_files))
	methodmap_list = make_domain_list(urimethodmap_files);
    if (methodmap_list == NULL)
	return;
    urimethods = New_N(struct table2 *, (methodmap_list->nitem + 1));
    for (i = 0, tl = methodmap_list->first; tl; tl = tl->next) {
	urimethods[i] = loadURIMethods(tl->ptr);
	if (urimethods[i])
	    i++;
    }
    urimethods[i] = NULL;
}

Str
searchURIMethods(ParsedURL *pu)
{
    struct table2 *ump;
    int i;
    Str scheme = NULL;
    Str url;
    char *p;

    if (pu->scheme != SCM_UNKNOWN)
	return NULL;		/* use internal */
    if (urimethods == NULL)
	return NULL;
    url = parsedURL2Str(pu);
    for (p = url->ptr; *p != '\0'; p++) {
	if (*p == ':') {
	    scheme = Strnew_charp_n(url->ptr, p - url->ptr);
	    break;
	}
    }
    if (scheme == NULL)
	return NULL;

    /*
     * RFC2396 3.1. Scheme Component
     * For resiliency, programs interpreting URI should treat upper case
     * letters as equivalent to lower case in scheme names (e.g., allow
     * "HTTP" as well as "http").
     */
    for (i = 0; (ump = urimethods[i]) != NULL; i++) {
	for (; ump->item1 != NULL; ump++) {
	    if (strcasecmp(ump->item1, scheme->ptr) == 0) {
		return Sprintf(ump->item2, url_quote(url->ptr));
	    }
	}
    }
    for (ump = default_urimethods; ump->item1 != NULL; ump++) {
	if (strcasecmp(ump->item1, scheme->ptr) == 0) {
	    return Sprintf(ump->item2, url_quote(url->ptr));
	}
    }
    return NULL;
}

/*
 * RFC2396: Uniform Resource Identifiers (URI): Generic Syntax
 * Appendix A. Collected BNF for URI
 * uric          = reserved | unreserved | escaped
 * reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
 *                 "$" | ","
 * unreserved    = alphanum | mark
 * mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
 *                  "(" | ")"
 * escaped       = "%" hex hex
 */

#define URI_PATTERN	"([-;/?:@&=+$,a-zA-Z0-9_.!~*'()]|%[0-9A-Fa-f][0-9A-Fa-f])*"
void
chkExternalURIBuffer(Buffer *buf)
{
    int i;
    struct table2 *ump;

    for (i = 0; (ump = urimethods[i]) != NULL; i++) {
	for (; ump->item1 != NULL; ump++) {
	    reAnchor(buf, Sprintf("%s:%s", ump->item1, URI_PATTERN)->ptr);
	}
    }
    for (ump = default_urimethods; ump->item1 != NULL; ump++) {
	reAnchor(buf, Sprintf("%s:%s", ump->item1, URI_PATTERN)->ptr);
    }
}
#endif

ParsedURL *
schemeToProxy(int scheme)
{
    ParsedURL *pu = NULL;	/* for gcc */
    switch (scheme) {
    case SCM_HTTP:
	pu = &HTTP_proxy_parsed;
	break;
#ifdef USE_SSL
    case SCM_HTTPS:
	pu = &HTTPS_proxy_parsed;
	break;
#endif
    case SCM_FTP:
	pu = &FTP_proxy_parsed;
	break;
#ifdef USE_GOPHER
    case SCM_GOPHER:
	pu = &GOPHER_proxy_parsed;
	break;
#endif
#ifdef DEBUG
    default:
	abort();
#endif
    }
    return pu;
}
