/* $Id: istream.c,v 1.9 2001/12/26 18:46:33 ukai Exp $ */
#include "fm.h"
#include "istream.h"
#include <signal.h>
#ifdef USE_SSL
#include <x509v3.h>
#endif

#define	uchar		unsigned char

#define STREAM_BUF_SIZE 8192
#define SSL_BUF_SIZE	1536

#define MUST_BE_UPDATED(bs) ((bs)->stream.cur==(bs)->stream.next)

#define POP_CHAR(bs) ((bs)->iseos?'\0':(bs)->stream.buf[(bs)->stream.cur++])

static void basic_close(int *handle);
static int basic_read(int *handle, char *buf, int len);

static void file_close(struct file_handle *handle);
static int file_read(struct file_handle *handle, char *buf, int len);

static int str_read(Str handle, char *buf, int len);

#ifdef USE_SSL
static void ssl_close(struct ssl_handle *handle);
static int ssl_read(struct ssl_handle *handle, char *buf, int len);
#endif

static int ens_read(struct ens_handle *handle, char *buf, int len);
static void ens_close(struct ens_handle *handle);

static void
do_update(BaseStream base)
{
    int len;
    base->stream.cur = base->stream.next = 0;
    len = base->read(base->handle, base->stream.buf, base->stream.size);
    if (len <= 0)
	base->iseos = TRUE;
    else
	base->stream.next += len;
}

static int
buffer_read(StreamBuffer sb, char *obuf, int count)
{
    int len = sb->next - sb->cur;
    if (len > 0) {
	if (len > count)
	    len = count;
	bcopy((const void *)&sb->buf[sb->cur], obuf, len);
	sb->cur += len;
    }
    return len;
}

static void
init_buffer(BaseStream base, char *buf, int bufsize)
{
    StreamBuffer sb = &base->stream;
    sb->size = bufsize;
    sb->cur = 0;
    if (buf) {
	sb->buf = (uchar *) buf;
	sb->next = bufsize;
    }
    else {
	sb->buf = NewAtom_N(uchar, bufsize);
	sb->next = 0;
    }
    base->iseos = FALSE;
}

static void
init_base_stream(BaseStream base, int bufsize)
{
    init_buffer(base, NULL, bufsize);
}

static void
init_str_stream(BaseStream base, Str s)
{
    init_buffer(base, s->ptr, s->length);
}

InputStream
newInputStream(int des)
{
    InputStream stream;
    if (des < 0)
	return NULL;
    stream = New(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->base.type = IST_BASIC;
    stream->base.handle = New(int);
    *(int *)stream->base.handle = des;
    stream->base.read = (int (*)())basic_read;
    stream->base.close = (void (*)())basic_close;
    return stream;
}

InputStream
newFileStream(FILE * f, void (*closep) ())
{
    InputStream stream;
    if (f == NULL)
	return NULL;
    stream = New(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->file.type = IST_FILE;
    stream->file.handle = New(struct file_handle);
    stream->file.handle->f = f;
    if (closep)
	stream->file.handle->close = closep;
    else
	stream->file.handle->close = (void (*)())fclose;
    stream->file.read = (int (*)())file_read;
    stream->file.close = (void (*)())file_close;
    return stream;
}

InputStream
newStrStream(Str s)
{
    InputStream stream;
    if (s == NULL)
	return NULL;
    stream = New(union input_stream);
    init_str_stream(&stream->base, s);
    stream->str.type = IST_STR;
    stream->str.handle = s;
    stream->str.read = (int (*)())str_read;
    stream->str.close = NULL;
    return stream;
}

#ifdef USE_SSL
InputStream
newSSLStream(SSL * ssl, int sock)
{
    InputStream stream;
    if (sock < 0)
	return NULL;
    stream = New(union input_stream);
    init_base_stream(&stream->base, SSL_BUF_SIZE);
    stream->ssl.type = IST_SSL;
    stream->ssl.handle = New(struct ssl_handle);
    stream->ssl.handle->ssl = ssl;
    stream->ssl.handle->sock = sock;
    stream->ssl.read = (int (*)())ssl_read;
    stream->ssl.close = (void (*)())ssl_close;
    return stream;
}
#endif

InputStream
newEncodedStream(InputStream is, char encoding)
{
    InputStream stream;
    if (is == NULL || (encoding != ENC_QUOTE && encoding != ENC_BASE64 &&
		       encoding != ENC_UUENCODE))
	return is;
    stream = New(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->ens.type = IST_ENCODED;
    stream->ens.handle = New(struct ens_handle);
    stream->ens.handle->is = is;
    stream->ens.handle->pos = 0;
    stream->ens.handle->encoding = encoding;
    stream->ens.handle->s = NULL;
    stream->ens.read = (int (*)())ens_read;
    stream->ens.close = (void (*)())ens_close;
    return stream;
}

void
ISclose(InputStream stream)
{
    MySignalHandler(*prevtrap) ();
    if (stream == NULL || stream->base.close == NULL)
	return;
    prevtrap = signal(SIGINT, SIG_IGN);
    stream->base.close(stream->base.handle);
    signal(SIGINT, prevtrap);
}

int
ISgetc(InputStream stream)
{
    BaseStream base;
    if (stream == NULL)
	return '\0';
    base = &stream->base;
    if (!base->iseos && MUST_BE_UPDATED(base))
	do_update(base);
    return POP_CHAR(base);
}

int
ISundogetc(InputStream stream)
{
    StreamBuffer sb;
    if (stream == NULL)
	return -1;
    sb = &stream->base.stream;
    if (sb->cur > 0) {
	sb->cur--;
	return 0;
    }
    return -1;
}

#define MARGIN_STR_SIZE 10
Str
StrISgets(InputStream stream)
{
    BaseStream base;
    StreamBuffer sb;
    Str s = NULL;
    uchar *p;
    int len;

    if (stream == NULL)
	return '\0';
    base = &stream->base;
    sb = &base->stream;

    while (!base->iseos) {
	if (MUST_BE_UPDATED(base)) {
	    do_update(base);
	}
	else {
	    if ((p = memchr(&sb->buf[sb->cur], '\n', sb->next - sb->cur))) {
		len = p - &sb->buf[sb->cur] + 1;
		if (s == NULL)
		    s = Strnew_size(len);
		Strcat_charp_n(s, (char *)&sb->buf[sb->cur], len);
		sb->cur += len;
		return s;
	    }
	    else {
		if (s == NULL)
		    s = Strnew_size(sb->next - sb->cur + MARGIN_STR_SIZE);
		Strcat_charp_n(s, (char *)&sb->buf[sb->cur],
			       sb->next - sb->cur);
		sb->cur = sb->next;
	    }
	}
    }

    if (s == NULL)
	return Strnew();
    return s;
}

Str
StrmyISgets(InputStream stream)
{
    BaseStream base;
    StreamBuffer sb;
    Str s = NULL;
    int i, len;

    if (stream == NULL)
	return '\0';
    base = &stream->base;
    sb = &base->stream;

    while (!base->iseos) {
	if (MUST_BE_UPDATED(base)) {
	    do_update(base);
	}
	else {
	    if (s && Strlastchar(s) == '\r') {
		if (sb->buf[sb->cur] == '\n')
		    Strcat_char(s, (char)sb->buf[sb->cur++]);
		return s;
	    }
	    for (i = sb->cur;
		 i < sb->next && sb->buf[i] != '\n' && sb->buf[i] != '\r';
		 i++) ;
	    if (i < sb->next) {
		len = i - sb->cur + 1;
		if (s == NULL)
		    s = Strnew_size(len + MARGIN_STR_SIZE);
		Strcat_charp_n(s, (char *)&sb->buf[sb->cur], len);
		sb->cur = i + 1;
		if (sb->buf[i] == '\n')
		    return s;
	    }
	    else {
		if (s == NULL)
		    s = Strnew_size(sb->next - sb->cur + MARGIN_STR_SIZE);
		Strcat_charp_n(s, (char *)&sb->buf[sb->cur],
			       sb->next - sb->cur);
		sb->cur = sb->next;
	    }
	}
    }

    if (s == NULL)
	return Strnew();
    return s;
}

int
ISread(InputStream stream, Str buf, int count)
{
    int rest, len;
    BaseStream base;

    if (stream == NULL || (base = &stream->base)->iseos)
	return 0;

    len = buffer_read(&base->stream, buf->ptr, count);
    rest = count - len;
    if (MUST_BE_UPDATED(base)) {
	len = base->read(base->handle, &buf->ptr[len], rest);
	if (len <= 0) {
	    base->iseos = TRUE;
	    len = 0;
	}
	rest -= len;
    }
    Strtruncate(buf, count - rest);
    if (buf->length > 0)
	return 1;
    return 0;
}

int
ISfileno(InputStream stream)
{
    if (stream == NULL)
	return -1;
    switch (IStype(stream)) {
    case IST_BASIC:
	return *(int *)stream->base.handle;
    case IST_FILE:
	return fileno(stream->file.handle->f);
    case IST_ENCODED:
	return ISfileno(stream->ens.handle->is);
    default:
	return -1;
    }
}

int
ISeos(InputStream stream)
{
    BaseStream base = &stream->base;
    if (!base->iseos && MUST_BE_UPDATED(base))
	do_update(base);
    return base->iseos;
}

#ifdef USE_SSL
static Str ssl_certificate_validity;

void
ssl_set_certificate_validity(Str msg)
{
    ssl_certificate_validity = msg;
}

Str
ssl_get_certificate(InputStream stream)
{
    BIO *bp;
    X509 *x;
    char *p;
    int len;
    Str s;
    if (stream == NULL)
	return NULL;
    if (IStype(stream) != IST_SSL)
	return NULL;
    if (stream->ssl.handle == NULL)
	return NULL;
    x = SSL_get_peer_certificate(stream->ssl.handle->ssl);
    if (x == NULL)
	return NULL;
    bp = BIO_new(BIO_s_mem());
    X509_print(bp, x);
    len = (int)BIO_ctrl(bp, BIO_CTRL_INFO, 0, (char *)&p);
    s = ssl_certificate_validity ? Strdup(ssl_certificate_validity)
	: Strnew_charp("valid certificate");
    Strcat_charp(s, "\n");
    Strcat_charp_n(s, p, len);
    BIO_free_all(bp);
    X509_free(x);
    return s;
}

Str
ssl_check_cert_ident(SSL * handle, char *hostname)
{
    X509 *x;
    int i;
    Str ret = NULL;
    int match_ident = FALSE;
    /*
     * All we need to do here is check that the CN matches.
     *
     * From RFC2818 3.1 Server Identity:
     * If a subjectAltName extension of type dNSName is present, that MUST
     * be used as the identity. Otherwise, the (most specific) Common Name
     * field in the Subject field of the certificate MUST be used. Although
     * the use of the Common Name is existing practice, it is deprecated and
     * Certification Authorities are encouraged to use the dNSName instead.
     */
    x = SSL_get_peer_certificate(handle);
    if (!x) {
	ret = Strnew_charp("Unable to get peer certificate");
	return ret;
    }

    i = X509_get_ext_by_NID(x, NID_subject_alt_name, -1);
    if (i >= 0) {
	X509_EXTENSION *ex;
	STACK_OF(GENERAL_NAME) * alt;

	ex = X509_get_ext(x, i);
	alt = X509V3_EXT_d2i(ex);
	if (alt) {
	    int n, len1, len2 = 0;
	    char *domain;
	    GENERAL_NAME *gn;
	    X509V3_EXT_METHOD *method;
	    Str seen_dnsname = NULL;

	    len1 = strlen(hostname);
	    n = sk_GENERAL_NAME_num(alt);
	    domain = strchr(hostname, '.');
	    if (domain)
		len2 = len1 - (domain - hostname);
	    for (i = 0; i < n; i++) {
		gn = sk_GENERAL_NAME_value(alt, i);
		if (gn->type == GEN_DNS) {
		    char *sn = ASN1_STRING_data(gn->d.ia5);
		    int sl = ASN1_STRING_length(gn->d.ia5);

		    if (!seen_dnsname)
			seen_dnsname = Strnew();
		    Strcat_m_charp(seen_dnsname, sn, " ");
		    /* Is this an exact match? */
		    if ((len1 == sl) && !strncasecmp(hostname, sn, len1))
			break;

		    /* Is this a wildcard match? */
		    if ((*sn == '*') && domain && (len2 == sl - 1) &&
			!strncasecmp(domain, sn + 1, len2))
			break;
		}
	    }
	    method = X509V3_EXT_get(ex);
	    method->ext_free(alt);
	    if (i < n)		/* Found a match */
		match_ident = TRUE;
	    else if (seen_dnsname)
		ret = Sprintf("Bad cert ident from %s: dNSName=%s", hostname,
			      seen_dnsname->ptr);
	}
    }

    if (match_ident == FALSE && ret == NULL) {
	X509_NAME *xn;
	char buf[2048];

	xn = X509_get_subject_name(x);

	if (X509_NAME_get_text_by_NID(xn, NID_commonName,
				      buf, sizeof(buf)) == -1)
	    ret = Strnew_charp("Unable to get common name from peer cert");
	else if (strcasecmp(hostname, buf))
	    ret = Sprintf("Bad cert ident %s from %s", buf, hostname);
	else
	    match_ident = TRUE;
    }
    X509_free(x);
    return ret;
}
#endif

/* Raw level input stream functions */

static void
basic_close(int *handle)
{
    close(*(int *)handle);
}

static int
basic_read(int *handle, char *buf, int len)
{
    return read(*(int *)handle, buf, len);
}

static void
file_close(struct file_handle *handle)
{
    handle->close(handle->f);
}

static int
file_read(struct file_handle *handle, char *buf, int len)
{
    return fread(buf, 1, len, handle->f);
}

static int
str_read(Str handle, char *buf, int len)
{
    return 0;
}

#ifdef USE_SSL
static void
ssl_close(struct ssl_handle *handle)
{
    close(handle->sock);
    if (handle->ssl)
	SSL_free(handle->ssl);
}

static int
ssl_read(struct ssl_handle *handle, char *buf, int len)
{
    int status;
    if (handle->ssl) {
#ifdef USE_SSL_VERIFY
	for (;;) {
	    status = SSL_read(handle->ssl, buf, len);
	    if (status > 0)
		break;
	    switch (SSL_get_error(handle->ssl, status)) {
	    case SSL_ERROR_WANT_READ:
	    case SSL_ERROR_WANT_WRITE:	/* reads can trigger write errors; see SSL_get_error(3) */
		continue;
	    default:
		break;
	    }
	    break;
	}
#else				/* if !defined(USE_SSL_VERIFY) */
	status = SSL_read(handle->ssl, buf, len);
#endif				/* !defined(USE_SSL_VERIFY) */
    }
    else
	status = read(handle->sock, buf, len);
    return status;
}
#endif				/* USE_SSL */

static void
ens_close(struct ens_handle *handle)
{
    ISclose(handle->is);
}

static int
ens_read(struct ens_handle *handle, char *buf, int len)
{
    if (handle->s == NULL || handle->pos == handle->s->length) {
	char *p;
	handle->s = StrmyISgets(handle->is);
	if (handle->s->length == 0)
	    return 0;
	cleanup_line(handle->s, PAGER_MODE);
	if (handle->encoding == ENC_BASE64)
	    Strchop(handle->s);
	else if (handle->encoding == ENC_UUENCODE) {
	    if (!strncmp(handle->s->ptr, "begin", 5))
		handle->s = StrmyISgets(handle->is);
	    Strchop(handle->s);
	}
	p = handle->s->ptr;
	if (handle->encoding == ENC_QUOTE)
	    handle->s = decodeQP(&p);
	else if (handle->encoding == ENC_BASE64)
	    handle->s = decodeB(&p);
	else if (handle->encoding == ENC_UUENCODE)
	    handle->s = decodeU(&p);
	handle->pos = 0;
    }

    if (len > handle->s->length - handle->pos)
	len = handle->s->length - handle->pos;

    bcopy(&handle->s->ptr[handle->pos], buf, len);
    handle->pos += len;
    return len;
}
