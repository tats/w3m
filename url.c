#include "fm.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#include <sys/stat.h>
#ifdef __EMX__
#include <io.h>
#include <strings.h>
#endif				/* __EMX__ */

#include "html.h"
#include "Str.h"
#include "myctype.h"
#include "regex.h"

#ifdef USE_SSL
#ifndef SSLEAY_VERSION_NUMBER
#include <crypto.h>		/* SSLEAY_VERSION_NUMBER may be here */
#endif
#include <err.h>
#endif

#ifdef	__WATT32__
#define	write(a,b,c)	write_s(a,b,c)
#endif				/* __WATT32__ */

#define NOPROXY_NETADDR		/* allow IP address for no_proxy */

#ifdef INET6
/* see rc.c, "dns_order" and dnsorders[] */
int ai_family_order_table[3][3] =
{
    {PF_UNSPEC, PF_UNSPEC, PF_UNSPEC},	/* 0:unspec */
    {PF_INET, PF_INET6, PF_UNSPEC},	/* 1:inet inet6 */
    {PF_INET6, PF_INET, PF_UNSPEC}	/* 2:inet6 inet */
};
#endif				/* INET6 */

static JMP_BUF AbortLoading;

/* XXX: note html.h SCM_ */
static int
 DefaultPort[] =
{
    80,				/* http */
    70,				/* gopher */
    21,				/* ftp */
    21,				/* ftpdir */
    0,				/* local - not defined */
    0,                /* local-CGI - not defined? */
    0,				/* exec - not defined? */
    119,			/* nntp */
    119,			/* news */
    0,				/* mailto - not defined */
#ifdef USE_SSL
    443,			/* https */
#endif				/* USE_SSL */
};

struct cmdtable schemetable[] =
{
    {"http", SCM_HTTP},
    {"gopher", SCM_GOPHER},
    {"ftp", SCM_FTP},
    {"local", SCM_LOCAL},
    {"file", SCM_LOCAL},
/*  {"exec", SCM_EXEC}, */
    {"nntp", SCM_NNTP},
    {"news", SCM_NEWS},
    {"mailto", SCM_MAILTO},
#ifdef USE_SSL
    {"https", SCM_HTTPS},
#endif				/* USE_SSL */
    {NULL, SCM_UNKNOWN},
};

static struct table2 DefaultGuess[] =
{
    {"html", "text/html"},
    {"HTML", "text/html"},
    {"htm", "text/html"},
    {"HTM", "text/html"},
    {"shtml", "text/html"},
    {"SHTML", "text/html"},
    {"gif", "image/gif"},
    {"GIF", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"JPEG", "image/jpeg"},
    {"JPG", "image/jpeg"},
    {"png", "image/png"},
    {"PNG", "image/png"},
    {"xbm", "image/xbm"},
    {"XBM", "image/xbm"},
    {"au", "audio/basic"},
    {"AU", "audio/basic"},
    {"gz", "application/x-gzip"},
    {"Z", "application/x-compress"},
    {"bz2", "application/x-bzip"},
    {"tar", "application/x-tar"},
    {"zip", "application/x-zip"},
    {"lha", "application/x-lha"},
    {"lzh", "application/x-lha"},
    {"LZH", "application/x-lha"},
    {"ps", "application/postscript"},
    {"pdf", "application/pdf"},
    {NULL, NULL}
};

static void add_index_file(ParsedURL * pu, URLFile *uf);

/* #define HTTP_DEFAULT_FILE    "/index.html" */

#ifndef HTTP_DEFAULT_FILE
#define HTTP_DEFAULT_FILE "/"
#endif				/* not HTTP_DEFAULT_FILE */

#ifdef SOCK_DEBUG
#include <stdarg.h>

static void
sock_log(char *message,...)
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

static char *
DefaultFile(int scheme)
{
    switch (scheme) {
    case SCM_HTTP:
#ifdef USE_SSL
    case SCM_HTTPS:
#endif				/* USE_SSL */
	return allocStr(HTTP_DEFAULT_FILE, 0);
#ifdef USE_GOPHER
    case SCM_GOPHER:
	return allocStr("1", 0);
#endif				/* USE_GOPHER */
    case SCM_LOCAL:
    case SCM_LOCAL_CGI:
    case SCM_FTP:
	return allocStr("/", 0);
    }
    return NULL;
}

static MySignalHandler
KeyAbort(SIGNAL_ARG)
{
    LONGJMP(AbortLoading, 1);
}

#ifdef USE_SSL
SSL_CTX *ssl_ctx = NULL;

void
free_ssl_ctx()
{
    if (ssl_ctx != NULL)
	SSL_CTX_free(ssl_ctx);
}

#if SSLEAY_VERSION_NUMBER >= 0x00905100
#include <rand.h>
void
init_PRNG()
{
    char buffer[256];
    const char *file;
    long l;
    if (RAND_status())
	return;
    if (file = RAND_file_name(buffer, sizeof(buffer))) {
#ifdef USE_EGD
	if (RAND_egd(file) > 0)
	    return;
#endif
	RAND_load_file(file, -1);
    }
    if (RAND_status())
	goto seeded;
    srand48((long) time(NULL));
    while (!RAND_status()) {
	l = lrand48();
	RAND_seed((unsigned char *)&l, sizeof(long));
    }
 seeded:
    if (file)
	RAND_write_file(file);
}
#endif				/* SSLEAY_VERSION_NUMBER >= 0x00905100 */

SSL *
openSSLHandle(int sock)
{
    SSL *handle;
    char *emsg;
    static char *old_ssl_forbid_method = NULL;
#ifdef USE_SSL_VERIFY
    static int old_ssl_verify_server = -1;
#endif

    if (! old_ssl_forbid_method || ! ssl_forbid_method ||
	strcmp(old_ssl_forbid_method, ssl_forbid_method)) {
	old_ssl_forbid_method = ssl_forbid_method;
#ifdef USE_SSL_VERIFY
	ssl_path_modified = 1;
#else
	free_ssl_ctx();
	ssl_ctx = NULL;
#endif
    }
#ifdef USE_SSL_VERIFY
    if (old_ssl_verify_server != ssl_verify_server) {
	old_ssl_verify_server = ssl_verify_server;
	ssl_path_modified = 1;
    }
    if (ssl_path_modified) {
	free_ssl_ctx();
	ssl_ctx = NULL;
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
	  if (strchr(ssl_forbid_method, '2')) option |= SSL_OP_NO_SSLv2;
	  if (strchr(ssl_forbid_method, '3')) option |= SSL_OP_NO_SSLv3;
	  if (strchr(ssl_forbid_method, 't')) option |= SSL_OP_NO_TLSv1;
	  if (strchr(ssl_forbid_method, 'T')) option |= SSL_OP_NO_TLSv1;
	}
	SSL_CTX_set_options(ssl_ctx, option);
#ifdef USE_SSL_VERIFY
	/* derived from openssl-0.9.5/apps/s_{client,cb}.c */
	SSL_CTX_set_verify(ssl_ctx, ssl_verify_server ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);
	if (ssl_cert_file != NULL && *ssl_cert_file != '\0') {
	    int ng = 1;
	    if (SSL_CTX_use_certificate_file(ssl_ctx, ssl_cert_file, SSL_FILETYPE_PEM) > 0) {
		char *key_file = (ssl_key_file == NULL || *ssl_key_file == '\0') ? ssl_cert_file : ssl_key_file;
		if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) > 0)
		    if (SSL_CTX_check_private_key(ssl_ctx))
			ng = 0;
	    }
	    if (ng) {
		free_ssl_ctx();
		goto eend;
	    }
	}
	if (SSL_CTX_load_verify_locations(ssl_ctx, ssl_ca_file, ssl_ca_path))
#endif				/* defined(USE_SSL_VERIFY) */
	    SSL_CTX_set_default_verify_paths(ssl_ctx);
#endif				/* SSLEAY_VERSION_NUMBER >= 0x0800 */
/*** by inu
        atexit(free_ssl_ctx);
*/
    }
    handle = SSL_new(ssl_ctx);
    SSL_set_fd(handle, sock);
#if SSLEAY_VERSION_NUMBER >= 0x00905100
    init_PRNG();
#endif				/* SSLEAY_VERSION_NUMBER >= 0x00905100 */
    if (SSL_connect(handle) <= 0)
      goto eend;
    return handle;
eend:
    emsg = Sprintf("SSL error: %s", ERR_error_string(ERR_get_error(), NULL))->ptr;
    disp_err_message(emsg, FALSE);
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
baseURL(Buffer * buf)
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
openSocket(char *hostname,
	   char *remoteport_name,
	   unsigned short remoteport_num)
{
    int sock = -1;
#ifdef INET6
    int *af;
    struct addrinfo hints, *res0, *res;
    int error;
#else				/* not INET6 */
    struct sockaddr_in hostaddr;
    struct hostent *entry;
    struct protoent *proto;
    unsigned short s_port;
    int a1, a2, a3, a4;
    unsigned long adr;
#endif				/* not INET6 */
    MySignalHandler(*trap) ();

    if (fmInitialized) {
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
    trap = signal(SIGINT, KeyAbort);
    if (fmInitialized)
	term_cbreak();
    if (hostname == NULL) {
#ifdef SOCK_DEBUG
	sock_log("openSocket() failed. reason: Bad hostname \"%s\"\n", hostname);
#endif
	goto error;
    }

#ifdef INET6
    for (af = ai_family_order_table[DNS_order];; af++) {
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = *af;
	hints.ai_socktype = SOCK_STREAM;
	if (remoteport_num != 0) {
	    Str portbuf = Sprintf("%d", remoteport_num);
	    error = getaddrinfo(hostname, portbuf->ptr, &hints, &res0);
	}
	else {
	    error = -1;
	}
	if (error && remoteport_name && remoteport_name[0] != '\0') {
	    /* try default port */
	    error = getaddrinfo(hostname, remoteport_name, &hints, &res0);
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
    bzero((char *) &hostaddr, sizeof(struct sockaddr_in));
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
    regexCompile("[0-9][0-9]*\\.[0-9][0-9]*\\.[0-9][0-9]*\\.[0-9][0-9]*", 0);
    if (regexMatch(hostname, 0, 1)) {
	sscanf(hostname, "%d.%d.%d.%d", &a1, &a2, &a3, &a4);
	adr = htonl((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);
	bcopy((void *) &adr, (void *) &hostaddr.sin_addr, sizeof(long));
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port = s_port;
	if (fmInitialized) {
	    message(Sprintf("Connecting to %s", hostname)->ptr, 0, 0);
	    refresh();
	}
	if (connect(sock, (struct sockaddr *) &hostaddr,
		    sizeof(struct sockaddr_in)) < 0) {
#ifdef SOCK_DEBUG
	    sock_log("openSocket: connect() failed. reason: %s\n", strerror(errno));
#endif
	    goto error;
	}
    }
    else {
	char **h_addr_list;
	int result;
	if (fmInitialized) {
	    message(Sprintf("Performing hostname lookup on %s", hostname)->ptr, 0, 0);
	    refresh();
	}
	if ((entry = gethostbyname(hostname)) == NULL) {
#ifdef SOCK_DEBUG
	    sock_log("openSocket: gethostbyname() failed. reason: %s\n", strerror(errno));
#endif
	    goto error;
	}
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_port = s_port;
	for (h_addr_list = entry->h_addr_list; *h_addr_list; h_addr_list++) {
	    bcopy((void *) h_addr_list[0], (void *) &hostaddr.sin_addr, entry->h_length);
#ifdef SOCK_DEBUG
	    adr = ntohl(*(long *) &hostaddr.sin_addr);
	    sock_log("openSocket: connecting %d.%d.%d.%d\n",
		     (adr >> 24) & 0xff,
		     (adr >> 16) & 0xff,
		     (adr >> 8) & 0xff,
		     adr & 0xff);
#endif
	    if (fmInitialized) {
		message(Sprintf("Connecting to %s", hostname)->ptr, 0, 0);
		refresh();
	    }
	    if ((result = connect(sock, (struct sockaddr *) &hostaddr,
				  sizeof(struct sockaddr_in))) == 0) {
		break;
	    }
#ifdef SOCK_DEBUG
	    else {
		sock_log("openSocket: connect() failed. reason: %s\n", strerror(errno));
	    }
#endif
	}
	if (result < 0) {
	    goto error;
	}
    }
#endif				/* not INET6 */

    if (fmInitialized)
	term_raw();
    signal(SIGINT, trap);
    return sock;
  error:
    if (fmInitialized)
	term_raw();
    signal(SIGINT, trap);

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
	    switch(option) {
	    case COPYPATH_SPC_ALLOW:
	      Strcat_char(tmp,*orgpath);
	      break;
	    case COPYPATH_SPC_IGNORE:
	      /* do nothing */
	      break;
	    case COPYPATH_SPC_REPLACE:
	      Strcat_charp(tmp,"%20");
	      break;
	    }
        }
	else
	    Strcat_char(tmp,*orgpath);
	orgpath++;
	length--;
    }
    return tmp->ptr;
}

void
parseURL(char *url, ParsedURL * p_url, ParsedURL * current)
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
    p_url->label = NULL;

    if (*url == '#') {		/* label only */
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
#endif         /* SUPPORT_DOS_DRIVE_PREFIX */
    /* search for scheme */
    p_url->scheme = getURLScheme(&p);
    if (p_url->scheme == SCM_MISSING) {		/* scheme not found */
       if (current) {
	    copyParsedURL(p_url, current);
           if (p_url->scheme == SCM_LOCAL_CGI)
        p_url->scheme = SCM_LOCAL;
       } else
	    p_url->scheme = SCM_LOCAL;
	p = url;
       if (!strncmp(p,"//",2)) {
           /* URL begins with // */
           /* it means that 'scheme:' is abbreviated */
#ifdef __CYGWIN__
           /* in CYGWIN, '//host/share' doesn't means 'http://host/share' */
             p_url->scheme = SCM_LOCAL;
             goto analyze_file;
#endif /* __CYGWIN__ */
           p += 2;
           goto analyze_url;
       }
	goto analyze_file;
    }
    /* get host and port */
    if (p[0] != '/' || p[1] != '/') {	/* scheme:foo or * scheme:/foo */
	p_url->host = NULL;
	if (p_url->scheme != SCM_UNKNOWN)
	    p_url->port = DefaultPort[p_url->scheme];
	else
	    p_url->port = 0;
	goto analyze_file;
    }
    if (p_url->scheme == SCM_LOCAL) {	/* file://foo           */
#ifdef __EMX__
	p += 2;
	goto analyze_file;
#else
	if (p[2] == '/' || p[2] == '~') {	/* file:///foo  */
	    p += 2;		/* file://~user */
	    goto analyze_file;
	}
#endif
#if defined( __CYGWIN__ ) && defined( SUPPORT_DOS_DRIVE_PREFIX )
       if (IS_ALPHA(p[2]) && (p[3] == ':' || p[3] == '|'))
           p += 2;
        goto analyze_file;     /* file://DRIVE/foo or
                                  file://machine/share/foo */
#endif                /* __CYGWIN__ && SUPPORT_DOS_DRIVE_PREFIX */
    }
    p += 2;			/* scheme://foo         */
  analyze_url:
    q = p;
    while (*p && *p != ':' && *p != '/' && *p != '@' && *p != '?') {
#ifdef INET6
	if (*p == '[') {        /* rfc2732 compliance */
	    char *p_colon = NULL;
	    do {
		p++;
		if ((p_colon == NULL) && (*p == ':'))
		    p_colon = p;
	    } while (*p && (IS_ALNUM(*p) || *p == ':' || *p == '.'));
	    if (*p == ']') {
		p++;
		break;
	    }
	    else if (p_colon) {
		p = p_colon;
		break;
	    }
	}
#endif
	p++;
    }
    switch (*p) {
    case '\0':			/* scheme://host                */
	/* scheme://user@host           */
	/* scheme://user:pass@host      */
	p_url->host = copyPath(q, -1, COPYPATH_SPC_IGNORE);
	p_url->port = DefaultPort[p_url->scheme];
	goto analyze_file;
    case ':':
	p_url->host = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	q = ++p;
	while (*p && *p != '/' && *p != '@')
	    p++;
	if (*p == '@') {	/* scheme://user:pass@...       */
	    p_url->pass = copyPath(q, p - q, COPYPATH_SPC_ALLOW);
	    q = ++p;
	    p_url->user = p_url->host;
	    p_url->host = NULL;
	    goto analyze_url;
	}
	tmp = Strnew_charp_n(q, p - q);
	p_url->port = atoi(tmp->ptr);
	if (*p == '\0') {	/* scheme://host:port           */
	    /* scheme://user@host:port      */
	    /* scheme://user:pass@host:port */
	    p_url->file = DefaultFile(p_url->scheme);
	    p_url->label = NULL;
	    return;
	}
	break;
    case '@':			/* scheme://user@...            */
	p_url->user = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	q = ++p;
	goto analyze_url;
    case '/':			/* scheme://host/...            */
    case '?':
	/* scheme://user@host/...       */
	/* scheme://user:pass@host/...  */
	p_url->host = copyPath(q, p - q, COPYPATH_SPC_IGNORE);
	p_url->port = DefaultPort[p_url->scheme];
	break;
    }
#ifdef INET6
    /* rfc2732 compliance */
    if (p_url->host != NULL && p_url->host[0] == '[' &&
	p_url->host[strlen(p_url->host)-1] == ']' ) {
	p_url->host[strlen(p_url->host)-1] = '\0';
	++(p_url->host);
    }
#endif
  analyze_file:
#ifndef __CYGWIN__
    if (p_url->scheme == SCM_LOCAL && p_url->user == NULL &&
	p_url->host != NULL && strcmp(p_url->host, "localhost")) {
	p_url->scheme = SCM_FTP;	/* ftp://host/... */
	if (p_url->port == 0)
	    p_url->port = DefaultPort[SCM_FTP];
    }
#endif
#ifdef SUPPORT_DOS_DRIVE_PREFIX
    if (p_url->scheme == SCM_LOCAL) {
       q = p;
       if (*q == '/')
           q++;
       if (IS_ALPHA(q[0]) && (q[1] == ':' || q[1] == '|')) {
           if (q[1] == '|') {
        p = allocStr(q, 0);
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
    if (*p == '\0') {		/* scheme://host[:port]/ */
	p_url->file = DefaultFile(p_url->scheme);
	p_url->label = NULL;
	return;
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
	while (*p && *p != '#')
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
  do_label:
    if (p_url->scheme == SCM_MISSING) {
	p_url->scheme = SCM_LOCAL;
	p_url->file = allocStr(p, 0);
	p_url->label = NULL;
    }
    else if (*p == '#')
	p_url->label = allocStr(p + 1, 0);
    else
	p_url->label = NULL;
}

#define initParsedURL(p) bzero(p,sizeof(ParsedURL))

void
copyParsedURL(ParsedURL * p, ParsedURL * q)
{
    p->scheme = q->scheme;
    p->port = q->port;
    p->is_nocache = q->is_nocache;
    if (q->user)
	p->user = allocStr(q->user, 0);
    else
	p->user = NULL;
    if (q->pass)
	p->pass = allocStr(q->pass, 0);
    else
	p->pass = NULL;
    if (q->host)
	p->host = allocStr(q->host, 0);
    else
	p->host = NULL;
    if (q->file)
	p->file = allocStr(q->file, 0);
    else
	p->file = NULL;
    if (q->real_file)
	p->real_file = allocStr(q->real_file, 0);
    else
	p->real_file = NULL;
    if (q->label)
	p->label = allocStr(q->label, 0);
    else
	p->label = NULL;
}

void
parseURL2(char *url, ParsedURL * pu, ParsedURL * current)
{
    char *p, *q;
    Str tmp;

    parseURL(url, pu, current);
    if (pu->scheme == SCM_MAILTO)
	return;

    if (pu->scheme == SCM_LOCAL)
	pu->file = expandName(pu->file);

    if (current && pu->scheme == current->scheme) {
	if (pu->user == NULL) {
	    pu->user = current->user;
	}
	if (pu->pass == NULL) {
	    pu->pass = current->pass;
	}
	if (pu->host == NULL) {
	    pu->host = current->host;
	}
	if (pu->file) {
	    if (
#ifdef USE_GOPHER
		pu->scheme != SCM_GOPHER &&
#endif				/* USE_GOPHER */
#ifdef USE_NNTP
		pu->scheme != SCM_NEWS &&
#endif				/* USE_NNTP */
		pu->file[0] != '/'
#ifdef SUPPORT_DOS_DRIVE_PREFIX
		&& !(pu->scheme == SCM_LOCAL && IS_ALPHA(pu->file[0]) && pu->file[1] == ':')
#endif
		) {
		p = pu->file;
		if (current->file) {
		    tmp = Strnew_charp(current->file);
		    if ((q = strchr(tmp->ptr, '?')) != NULL)
			Strshrink(tmp, (tmp->ptr + tmp->length) - q);
		    while (tmp->length > 0) {
			if (Strlastchar(tmp) == '/')
			    break;
			Strshrink(tmp, 1);
		    }
		    Strcat_charp(tmp, p);
		    pu->file = allocStr(tmp->ptr, 0);
		}
	    }
#ifdef USE_GOPHER
	    else if (pu->scheme == SCM_GOPHER &&
		     pu->file[0] == '/') {
		p = pu->file;
		pu->file = allocStr(p + 1, 0);
	    }
#endif				/* USE_GOPHER */
	}
	else if (pu->label) {
	    /* pu has only label */
	    pu->file = current->file;
	}
    }
    if (pu->file) {
#ifdef __EMX__
	if (pu->scheme == SCM_LOCAL) {
	    if (strncmp(pu->file, "/$LIB/", 6)) {
		char *arg, abs[_MAX_PATH], tmp[_MAX_PATH];

		if (!(arg = strchr(strcpy(tmp, pu->file), '?'))) {
		    _abspath(abs, tmp, _MAX_PATH);
		    pu->file = cleanupName(abs);
		}
		else {
		    *arg = 0;
		    _abspath(abs, tmp, _MAX_PATH);
		    *arg = '?';
		    pu->file = cleanupName(strcat(abs, arg));
		}
	    }
	}
#else
	if (pu->scheme == SCM_LOCAL && pu->file[0] != '/' &&
#if defined( __CYGWIN__ ) && defined( SUPPORT_DOS_DRIVE_PREFIX ) /* for 'drive:' */
           ! (IS_ALPHA(pu->file[0]) && pu->file[1] == ':') &&
#endif
	    strcmp(pu->file, "-")) {
	    tmp = Strnew_charp(CurrentDir);
	    if (Strlastchar(tmp) != '/')
		Strcat_char(tmp, '/');
	    Strcat_charp(tmp, pu->file);
	    pu->file = cleanupName(tmp->ptr);
	}
#endif
	else if ((pu->scheme == SCM_HTTP
#ifdef USE_SSL
		|| pu->scheme == SCM_HTTPS
#endif
		) && pu->file[0] == '/') {
		char *p = &pu->file[1];
		int s = getURLScheme(&p);
		if (s == SCM_MISSING || s == SCM_UNKNOWN)
		    pu->file = cleanupName(pu->file);
	} else if (
#ifdef USE_GOPHER
		 pu->scheme != SCM_GOPHER &&
#endif				/* USE_GOPHER */
#ifdef USE_NNTP
		 pu->scheme != SCM_NEWS &&
#endif				/* USE_NNTP */
		 pu->file[0] == '/') {
	    pu->file = cleanupName(pu->file);
	}
	if (pu->scheme == SCM_LOCAL)
	    pu->real_file = cleanupName2(file_unquote(pu->file), FALSE);
    }
}

static Str
_parsedURL2Str(ParsedURL * pu, int pass)
{
    Str tmp;
    static char *scheme_str[] =
    {
	"http", "gopher", "ftp", "ftp", "file", "file", "exec", "nntp", "news", "mailto",
#ifdef USE_SSL
	"https",
#endif				/* USE_SSL */
    };

    if (pu->scheme == SCM_UNKNOWN || pu->scheme == SCM_MISSING) {
	return Strnew_charp("???");
    }
    if (pu->host == NULL && pu->file == NULL && pu->label != NULL) {
	/* local label */
	return Sprintf("#%s", pu->label);
    }
    if (pu->scheme == SCM_LOCAL && ! strcmp(pu->file, "-")) {
	tmp = Strnew_charp("-");
	if (pu->label) {
	    Strcat_char(tmp, '#');
	    Strcat_charp(tmp, pu->label);
	}
	return tmp;
    }
    tmp = Strnew_charp(scheme_str[pu->scheme]);
    Strcat_char(tmp, ':');
    if (pu->scheme == SCM_MAILTO) {
	Strcat_charp(tmp, pu->file);
	return tmp;
    }
#ifdef USE_NNTP
    if (pu->scheme != SCM_NEWS)
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
	pu->scheme != SCM_NEWS &&
#endif				/* USE_NNTP */
#ifdef SUPPORT_DOS_DRIVE_PREFIX
	(pu->file == NULL || (pu->file[0] != '/' && pu->file[1] != ':')))
#else
	(pu->file == NULL || pu->file[0] != '/'))
#endif
	Strcat_char(tmp, '/');
    Strcat_charp(tmp, pu->file);
    if (pu->label) {
	Strcat_char(tmp, '#');
	Strcat_charp(tmp, pu->label);
    }
    return tmp;
}

Str
parsedURL2Str(ParsedURL * pu)
{
    return _parsedURL2Str(pu, FALSE);
}

int
getURLScheme(char **url)
{
    char *p = *url, *q;
    int i;
    int scheme = SCM_MISSING;

    while (*p && (IS_ALPHA(*p) || *p == '.' || *p == '+' || *p == '-'))
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
otherinfo(ParsedURL * target, ParsedURL * current, char *referer)
{
    Str s = Strnew();

    Strcat_charp(s, "User-Agent: ");
    if (UserAgent == NULL || *UserAgent == '\0')
	Strcat_charp(s, version);
    else
	Strcat_charp(s, UserAgent);
    Strcat_charp(s, "\r\n");
    Strcat_charp(s, "Accept: text/*, image/*, audio/*, video/*, application/*\r\n");
    Strcat_charp(s, "Accept-Encoding: gzip, compress, bzip, bzip2, deflate\r\n");
    Strcat_charp(s, "Accept-Language: ");
    if (AcceptLang != NULL && *AcceptLang != '\0') {
	Strcat_charp(s, AcceptLang);
	Strcat_charp(s, "\r\n");
    }
    else {
#if LANG == JA
	Strcat_charp(s, "ja; q=1.0, en; q=0.5\r\n");
#else				/* LANG != JA (must be EN) */
	Strcat_charp(s, "en; q=1.0\r\n");
#endif				/* LANG != JA */
    }
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
	if (referer == NULL && current && current->scheme != SCM_LOCAL &&
	    (current->scheme != SCM_FTP ||
	     (current->user == NULL && current->pass == NULL))) {
	    Strcat_charp(s, "Referer: ");
	    Strcat(s, parsedURL2Str(current));
	    Strcat_charp(s, "\r\n");
	}
	else if (referer != NULL && referer != NO_REFERER) {
	    Strcat_charp(s, "Referer: ");
	    Strcat_charp(s, referer);
	    Strcat_charp(s, "\r\n");
	}
    }
    return s->ptr;
}

Str
HTTPrequest(ParsedURL * pu, ParsedURL * current, HRequest * hr, TextList * extra)
{
    Str tmp;
    TextListItem *i;
#ifdef USE_COOKIE
    Str cookie;
#endif				/* USE_COOKIE */
    switch (hr->command) {
    case HR_COMMAND_CONNECT:
	tmp = Strnew_charp("CONNECT ");
	break;
    case HR_COMMAND_POST:
	tmp = Strnew_charp("POST ");
	break;
    case HR_COMMAND_HEAD:
	tmp = Strnew_charp("HEAD ");
	break;
    case HR_COMMAND_GET:
    default:
	tmp = Strnew_charp("GET ");
    }
    if (hr->command == HR_COMMAND_CONNECT) {
	Strcat_charp(tmp, pu->host);
	Strcat(tmp, Sprintf(":%d", pu->port));
    }
    else if (hr->flag & HR_FLAG_LOCAL) {
	Strcat_charp(tmp, pu->file);
    }
    else {
	Strcat(tmp, _parsedURL2Str(pu, TRUE));
    }
    Strcat_charp(tmp, " HTTP/1.0\r\n");
    if (hr->referer == NO_REFERER)
	Strcat_charp(tmp, otherinfo(pu, NULL, NULL));
    else
	Strcat_charp(tmp, otherinfo(pu, current, hr->referer));
    if (extra != NULL)
	for (i = extra->first; i != NULL; i = i->next)
	    Strcat_charp(tmp, i->ptr);
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
	    Strcat(tmp, Sprintf("Content-length: %ld\r\n", hr->request->length));
	    Strcat_charp(tmp, "\r\n");
	}
	else {
            if (!override_content_type) {
               Strcat_charp(tmp, "Content-type: application/x-www-form-urlencoded\r\n");
            }
	    Strcat(tmp, Sprintf("Content-length: %ld\r\n", hr->request->length));
            if (header_string)
               Strcat (tmp, header_string);
	    Strcat_charp(tmp, "\r\n");
           Strcat_charp_n(tmp, hr->request->body, hr->request->length);
	    Strcat_charp(tmp, "\r\n");
	}
    }
    else {
        if (header_string)
           Strcat (tmp, header_string);
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
    uf->stream = stream;
    uf->scheme = scheme;
    uf->encoding = ENC_7BIT;
    uf->is_cgi = FALSE;
    uf->compression = 0;
    uf->guess_type = NULL;
    uf->ext = NULL;
}

static InputStream
openFTPStream(ParsedURL * pu)
{
    return newFileStream(openFTP(pu), closeFTP);
}

URLFile
openURL(char *url, ParsedURL * pu, ParsedURL * current,
	URLOption * option, FormList * request, TextList * extra_header,
	URLFile * ouf, unsigned char *status)
{
    Str tmp;
    int i, sock, scheme;
    char *p, *q, *u;
    URLFile uf;
    HRequest hr;
#ifdef USE_SSL
    SSL *sslh;
#endif				/* USE_SSL */
#ifdef USE_NNTP
    FILE *fw;
    char *r;
    InputStream stream;
#endif				/* USE_NNTP */

    if (ouf) {
	uf = *ouf;
    }
    else {
	init_stream(&uf, SCM_MISSING, NULL);
    }

    u = url;
    scheme = getURLScheme(&u);
    if (current == NULL && scheme == SCM_MISSING && ! ArgvIsURL)
	u = file_to_url(url);		/* force to local file */
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
	    pu->real_file = cleanupName2(file_unquote(pu->file), FALSE);
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
    pu->is_nocache = (option->flag & RG_NOCACHE);
    uf.ext = filename_extension(pu->file, 1);

    hr.command = HR_COMMAND_GET;
    hr.flag = 0;
    hr.referer = option->referer;
    hr.request = request;

    switch (pu->scheme) {
    case SCM_LOCAL:
    case SCM_LOCAL_CGI:
	if (request && request->body) {
	    /* local CGI: POST */
	    uf.stream = newFileStream(localcgi_post(pu->real_file, request,
					option->referer), (void (*)()) pclose);
	    if (uf.stream == NULL)
		goto ordinary_local_file;
	    uf.is_cgi = TRUE;
	    uf.scheme = pu->scheme = SCM_LOCAL_CGI;
	}
	else if ((q = strchr(pu->file, '?')) != NULL) {
	    /* lodal CGI: GET */
	    p = Strnew_charp_n(pu->file, (int)(q - pu->file))->ptr;
	    pu->real_file = cleanupName2(file_unquote(p), FALSE);
	    uf.stream = newFileStream(localcgi_get(pu->real_file, q + 1,
					option->referer), (void (*)()) pclose);
	    if (uf.stream == NULL) {
		pu->file = p;
		goto ordinary_local_file;
	    }
	    uf.is_cgi = TRUE;
	    uf.scheme = pu->scheme = SCM_LOCAL_CGI;
	}
	else if ((i = strlen(pu->file)) > 4 &&
#ifdef __EMX__
		!strncmp(pu->file + i - 4, ".cmd", 4))
#else
		!strncmp(pu->file + i - 4, ".cgi", 4))
#endif
	{
	    /* lodal CGI: GET */
	    uf.stream = newFileStream(localcgi_get(pu->real_file, "",
					option->referer), (void (*)()) pclose);
	    if (uf.stream == NULL)
		goto ordinary_local_file;
	    uf.is_cgi = TRUE;
	    uf.scheme = pu->scheme = SCM_LOCAL_CGI;
	}
	else {
	  ordinary_local_file:
	    examineFile(pu->real_file, &uf);
	}
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
		q = cleanupName2(file_unquote(p), FALSE);
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
	if (pu->file == NULL)
	    pu->file = allocStr("/", 0);
	if (non_null(FTP_proxy) &&
	    !Do_not_use_proxy &&
	    pu->host != NULL &&
	    !check_no_proxy(pu->host)) {
	    sock = openSocket(FTP_proxy_parsed.host,
			    schemetable[FTP_proxy_parsed.scheme].cmdname,
			      FTP_proxy_parsed.port);
	    if (sock < 0)
		return uf;
	    uf.scheme = SCM_HTTP;
	    tmp = HTTPrequest(pu, current, &hr, extra_header);
	    write(sock, tmp->ptr, tmp->length);
	}
	else {
	    uf.stream = openFTPStream(pu);
	    uf.scheme = pu->scheme;
	    return uf;
	}
	break;
    case SCM_HTTP:
#ifdef USE_SSL
    case SCM_HTTPS:
#endif				/* USE_SSL */
	if (pu->file == NULL)
	    pu->file = allocStr("/", 0);
	if (request && request->method == FORM_METHOD_POST && request->body)
	    hr.command = HR_COMMAND_POST;
	if (request && request->method == FORM_METHOD_HEAD)
	    hr.command = HR_COMMAND_HEAD;
	if (non_null(HTTP_proxy) &&
	    !Do_not_use_proxy &&
	    pu->host != NULL &&
	    !check_no_proxy(pu->host)) {
	    char *save_label;
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS && *status == HTST_CONNECT) {
		sock = ssl_socket_of(ouf->stream);
		if (!(sslh = openSSLHandle(sock))) {
		    *status = HTST_MISSING;
		    return uf;
		}
	    }
	    else {
		sock = openSocket(HTTP_proxy_parsed.host,
			   schemetable[HTTP_proxy_parsed.scheme].cmdname,
				  HTTP_proxy_parsed.port);
		sslh = NULL;
	    }
#else
	    sock = openSocket(HTTP_proxy_parsed.host,
			   schemetable[HTTP_proxy_parsed.scheme].cmdname,
			      HTTP_proxy_parsed.port);
#endif
	    if (sock < 0) {
#ifdef SOCK_DEBUG
		sock_log("Can't open socket\n");
#endif
		return uf;
	    }
	    save_label = pu->label;
	    pu->label = NULL;
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS) {
		if (*status == HTST_NORMAL) {
		    hr.command = HR_COMMAND_CONNECT;
		    tmp = HTTPrequest(pu, current, &hr, NULL);
		    *status = HTST_CONNECT;
		}
		else {
		    hr.flag |= HR_FLAG_LOCAL;
		    tmp = HTTPrequest(pu, current, &hr, extra_header);
		    *status = HTST_NORMAL;
		}
	    }
	    else
#endif				/* USE_SSL */
	    {
		tmp = HTTPrequest(pu, current, &hr, extra_header);
		*status = HTST_NORMAL;
		pu->label = save_label;
	    }
	}
	else {
	    sock = openSocket(pu->host,
			      schemetable[pu->scheme].cmdname,
			      pu->port);
	    if (sock < 0) {
		*status = HTST_MISSING;
		return uf;
	    }
#ifdef USE_SSL
	    if (pu->scheme == SCM_HTTPS) {
		if (!(sslh = openSSLHandle(sock))) {
		    *status = HTST_MISSING;
		    return uf;
		}
	    }
#endif				/* USE_SSL */
	    hr.flag |= HR_FLAG_LOCAL;
	    tmp = HTTPrequest(pu, current, &hr, extra_header);
	    *status = HTST_NORMAL;
	}
#ifdef USE_SSL
	if (pu->scheme == SCM_HTTPS) {
	    uf.stream = newSSLStream(sslh, sock);
	    if (sslh)
		SSL_write(sslh, tmp->ptr, tmp->length);
	    else
		write(sock, tmp->ptr, tmp->length);
	    if (hr.command == HR_COMMAND_POST &&
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
#ifdef HTTP_DEBUG
	    {
		FILE *ff = fopen("zzrequest", "a");
		fwrite(tmp->ptr, sizeof(char), tmp->length, ff);
		fclose(ff);
	    }
#endif				/* HTTP_DEBUG */
	    if (hr.command == HR_COMMAND_POST &&
		request->enctype == FORM_ENCTYPE_MULTIPART)
		write_from_file(sock, request->body);
	}
	break;
#ifdef USE_GOPHER
    case SCM_GOPHER:
	if (non_null(GOPHER_proxy) &&
	    !Do_not_use_proxy &&
	    pu->host != NULL &&
	    !check_no_proxy(pu->host)) {
	    sock = openSocket(GOPHER_proxy_parsed.host,
			 schemetable[GOPHER_proxy_parsed.scheme].cmdname,
			      GOPHER_proxy_parsed.port);
	    if (sock < 0)
		return uf;
	    uf.scheme = SCM_HTTP;
	    tmp = HTTPrequest(pu, current, &hr, extra_header);
	}
	else {
	    sock = openSocket(pu->host,
			      schemetable[pu->scheme].cmdname,
			      pu->port);
	    if (sock < 0)
		return uf;
	    if (pu->file == NULL)
		pu->file = "1";
	    tmp = Strnew_charp(pu->file);
	    Strcat_char(tmp, '\n');
	}
	write(sock, tmp->ptr, tmp->length);
	break;
#endif				/* USE_GOPHER */
#ifdef USE_NNTP
    case SCM_NNTP:
	/* nntp://<host>:<port>/<newsgroup-name>/<article-number> */
    case SCM_NEWS:
	if (pu->scheme == SCM_NNTP) {
	    p = pu->host;
	} else {
	    p = getenv("NNTPSERVER");
	}
	r = getenv("NNTPMODE");
	if (p == NULL)
	    return uf;
	sock = openSocket(p, "nntp", pu->port);
	if (sock < 0)
	    return uf;
	stream = newInputStream(sock);
	fw = fdopen(sock, "wb");
	if (stream == NULL || fw == NULL)
	    return uf;
	tmp = StrISgets(stream);
	if (tmp->length == 0)
	    goto nntp_error;
	sscanf(tmp->ptr, "%d", &i);
	if (i != 200 && i != 201)
	    goto nntp_error;
	if (r && *r != '\0') {
	    fprintf(fw, "MODE %s\r\n", r);
	    fflush(fw);
	    tmp = StrISgets(stream);
	    if (tmp->length == 0)
		goto nntp_error;
	    sscanf(tmp->ptr, "%d", &i);
	    if (i != 200 && i != 201)
		goto nntp_error;
	}
	if (pu->scheme == SCM_NNTP) {
	    char *group;
	    if (pu->file == NULL || *pu->file == '\0')
		goto nntp_error;
	    /* first char of pu->file is '/' */
	    group = url_unquote(Strnew_charp(pu->file + 1)->ptr);
	    p = strchr(group, '/');
	    if (p == NULL)
		goto nntp_error;
	    *p++ = '\0';
	    fprintf(fw, "GROUP %s\r\n", group);
	    fflush(fw);
	    tmp = StrISgets(stream);
	    if (tmp->length == 0) {
		goto nntp_error;
	    }
	    sscanf(tmp->ptr, "%d", &i);
	    if (i != 211)
		goto nntp_error;
	    fprintf(fw, "ARTICLE %s\r\n", p);
	} else {
	    fprintf(fw, "ARTICLE <%s>\r\n", url_unquote(pu->file));
	}
	fflush(fw);
	tmp = StrISgets(stream);
	if (tmp->length == 0)
	    goto nntp_error;
	sscanf(tmp->ptr, "%d", &i);
	if (i != 220)
	    goto nntp_error;
	uf.scheme = SCM_NEWS; /* XXX */
	uf.stream = stream;
	return uf;
      nntp_error:
	ISclose(stream);
	fclose(fw);
	return uf;
#endif				/* USE_NNTP */
    case SCM_UNKNOWN:
    default:
	return uf;
    }
    uf.stream = newInputStream(sock);
    return uf;
}

/* add index_file if exists */
static void
add_index_file(ParsedURL * pu, URLFile *uf)
{
    char *p, *q;

    if (index_file == NULL || index_file[0] == '\0') {
	uf->stream = NULL;
	return;
    }
    p = Strnew_m_charp(pu->file, "/", file_quote(index_file), NULL)->ptr;
    p = cleanupName(p);
    q = cleanupName2(file_unquote(p), FALSE);
    examineFile(q, uf);
    if (uf->stream == NULL)
	return;
    pu->file = p;
    pu->real_file = q;
    return;
}

char *
guessContentTypeFromTable(struct table2 *table, char *filename)
{
    char *p;
    if (table == NULL)
	return NULL;
    p = &filename[strlen(filename) - 1];
    while (filename < p && *p != '.')
	p--;
    if (p == filename)
	return NULL;
    p++;
    while (table->item1) {
	if (!strcmp(p, table->item1))
	    return table->item2;
	table++;
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
	if ((ret = guessContentTypeFromTable(UserMimeTypes[i], filename)) != NULL)
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

    if (NO_proxy_domains == NULL || NO_proxy_domains->nitem == 0 ||
	domain == NULL)
	return 0;
    for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
	if (domain_match(tl->ptr, domain))
	    return 1;
    }
#ifdef NOPROXY_NETADDR
    if (!NOproxy_netaddr) {
	return 0;
    }
    /* 
     * to check noproxy by network addr
     */
    {
#ifndef INET6
	struct hostent *he;
	int n;
	unsigned char **h_addr_list;
	char addr[4 * 16], buf[5];

	he = gethostbyname(domain);
	if (!he)
	    return (0);
	for (h_addr_list = (unsigned char **) he->h_addr_list
	     ; *h_addr_list; h_addr_list++) {
	    sprintf(addr, "%d", h_addr_list[0][0]);
	    for (n = 1; n < he->h_length; n++) {
		sprintf(buf, ".%d", h_addr_list[0][n]);
		strcat(addr, buf);
	    }
	    for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
		if (strncmp(tl->ptr, addr, strlen(tl->ptr)) == 0)
		    return (1);
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
			&((struct sockaddr_in *) res->ai_addr)->sin_addr,
			      addr, sizeof(addr));
		    break;
		case AF_INET6:
		    inet_ntop(AF_INET6,
		      &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr,
			      addr, sizeof(addr));
		    break;
		default:
		    /* unknown */
		    continue;
		}
		for (tl = NO_proxy_domains->first; tl != NULL; tl = tl->next) {
		    if (strncmp(tl->ptr, addr, strlen(tl->ptr)) == 0) {
			freeaddrinfo(res0);
			return 1;
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
#endif				/* NOPROXY_NETADDR */
    return 0;
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
