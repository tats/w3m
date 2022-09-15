/* $Id: istream.c,v 1.27 2010/07/18 13:43:23 htrb Exp $ */
#include "fm.h"
#include "myctype.h"
#include "istream.h"
#include <signal.h>
#include <openssl/x509v3.h>

#define	uchar		unsigned char

#define STREAM_BUF_SIZE 8192
#define SSL_BUF_SIZE	1536

#define MUST_BE_UPDATED(bs) ((bs)->stream.cur==(bs)->stream.next)

#define POP_CHAR(bs) ((bs)->iseos?'\0':(bs)->stream.buf[(bs)->stream.cur++])

static void basic_close(int *handle);
static int basic_read(int *handle, char *buf, int len);

static void file_close(struct io_file_handle *handle);
static int file_read(struct io_file_handle *handle, char *buf, int len);

static int str_read(Str handle, char *buf, int len);

static void ssl_close(struct ssl_handle *handle);
static int ssl_read(struct ssl_handle *handle, char *buf, int len);

static int ens_read(struct ens_handle *handle, char *buf, int len);
static void ens_close(struct ens_handle *handle);

static void memchop(char *p, int *len);

static void
do_update(BaseStream base)
{
    int len;
    base->stream.cur = base->stream.next = 0;
    len = (*base->read) (base->handle, base->stream.buf, base->stream.size);
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
    sb->buf = NewWithoutGC_N(uchar, bufsize);
    if (buf) {
	memcpy(sb->buf, buf, bufsize);
	sb->next = bufsize;
    }
    else {
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
    stream = NewWithoutGC(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->base.type = IST_BASIC;
    stream->base.handle = NewWithoutGC(int);
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
    stream = NewWithoutGC(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->file.type = IST_FILE;
    stream->file.handle = NewWithoutGC(struct io_file_handle);
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
    stream = NewWithoutGC(union input_stream);
    init_str_stream(&stream->base, s);
    stream->str.type = IST_STR;
    stream->str.handle = NULL;
    stream->str.read = (int (*)())str_read;
    stream->str.close = NULL;
    return stream;
}

InputStream
newSSLStream(SSL * ssl, int sock)
{
    InputStream stream;
    if (sock < 0)
	return NULL;
    stream = NewWithoutGC(union input_stream);
    init_base_stream(&stream->base, SSL_BUF_SIZE);
    stream->ssl.type = IST_SSL;
    stream->ssl.handle = NewWithoutGC(struct ssl_handle);
    stream->ssl.handle->ssl = ssl;
    stream->ssl.handle->sock = sock;
    stream->ssl.read = (int (*)())ssl_read;
    stream->ssl.close = (void (*)())ssl_close;
    return stream;
}

InputStream
newEncodedStream(InputStream is, char encoding)
{
    InputStream stream;
    if (is == NULL || (encoding != ENC_QUOTE && encoding != ENC_BASE64 &&
		       encoding != ENC_UUENCODE))
	return is;
    stream = NewWithoutGC(union input_stream);
    init_base_stream(&stream->base, STREAM_BUF_SIZE);
    stream->ens.type = IST_ENCODED;
    stream->ens.handle = NewWithoutGC(struct ens_handle);
    stream->ens.handle->is = is;
    stream->ens.handle->pos = 0;
    stream->ens.handle->encoding = encoding;
    growbuf_init_without_GC(&stream->ens.handle->gb);
    stream->ens.read = (int (*)())ens_read;
    stream->ens.close = (void (*)())ens_close;
    return stream;
}

int
ISclose(InputStream stream)
{
    MySignalHandler(*prevtrap) ();
    if (stream == NULL)
        return -1;
    if (stream->base.close != NULL) {
        if (stream->base.type & IST_UNCLOSE) {
            return -1;
        }
        prevtrap = mySignal(SIGINT, SIG_IGN);
        stream->base.close (stream->base.handle);
        mySignal(SIGINT, prevtrap);
    }
    xfree(stream->base.stream.buf);
    xfree(stream);
    return 0;
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

Str
StrISgets2(InputStream stream, char crnl)
{
    struct growbuf gb;

    if (stream == NULL)
	return NULL;
    growbuf_init(&gb);
    ISgets_to_growbuf(stream, &gb, crnl);
    return growbuf_to_Str(&gb);
}

void
ISgets_to_growbuf(InputStream stream, struct growbuf *gb, char crnl)
{
    BaseStream base = &stream->base;
    StreamBuffer sb = &base->stream;
    int i;

    gb->length = 0;

    while (!base->iseos) {
	if (MUST_BE_UPDATED(base)) {
	    do_update(base);
	    continue;
	}
	if (crnl && gb->length > 0  && gb->ptr[gb->length - 1] == '\r') {
	    if (sb->buf[sb->cur] == '\n') {
		GROWBUF_ADD_CHAR(gb, '\n');
		++sb->cur;
	    }
	    break;
	}
	for (i = sb->cur; i < sb->next; ++i) {
	    if (sb->buf[i] == '\n' || (crnl && sb->buf[i] == '\r')) {
		++i;
		break;
	    }
	}
	growbuf_append(gb, &sb->buf[sb->cur], i - sb->cur);
	sb->cur = i;
	if (gb->length > 0 && gb->ptr[gb->length - 1] == '\n')
	    break;
    }

    growbuf_reserve(gb, gb->length + 1);
    gb->ptr[gb->length] = '\0';
    return;
}

#ifdef unused
int
ISread(InputStream stream, Str buf, int count)
{
    int len;

    if (count + 1 > buf->area_size) {
	char *newptr = GC_MALLOC_ATOMIC(count + 1);
	memcpy(newptr, buf->ptr, buf->length);
	newptr[buf->length] = '\0';
	buf->ptr = newptr;
	buf->area_size = count + 1;
    }
    len = ISread_n(stream, buf->ptr, count);
    buf->length = (len > 0) ? len : 0;
    buf->ptr[buf->length] = '\0';
    return (len > 0) ? 1 : 0;
}
#endif

int
ISread_n(InputStream stream, char *dst, int count)
{
    int len, l;
    BaseStream base;

    if (stream == NULL || count <= 0)
	return -1;
    if ((base = &stream->base)->iseos)
	return 0;

    len = buffer_read(&base->stream, dst, count);
    if (MUST_BE_UPDATED(base)) {
	l = (*base->read) (base->handle, &dst[len], count - len);
	if (l <= 0) {
	    base->iseos = TRUE;
	} else {
	    len += l;
	}
    }
    return len;
}

int
ISfileno(InputStream stream)
{
    if (stream == NULL)
	return -1;
    switch (IStype(stream) & ~IST_UNCLOSE) {
    case IST_BASIC:
	return *(int *)stream->base.handle;
    case IST_FILE:
	return fileno(stream->file.handle->f);
    case IST_SSL:
	return stream->ssl.handle->sock;
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

static Str accept_this_site;

void
ssl_accept_this_site(char *hostname)
{
    if (hostname)
	accept_this_site = Strnew_charp(hostname);
    else
	accept_this_site = NULL;
}

static int
ssl_match_cert_ident(char *ident, int ilen, char *hostname)
{
    /* RFC2818 3.1.  Server Identity
     * Names may contain the wildcard
     * character * which is considered to match any single domain name
     * component or component fragment. E.g., *.a.com matches foo.a.com but
     * not bar.foo.a.com. f*.com matches foo.com but not bar.com.
     */
    int hlen = strlen(hostname);
    int i, c;

    /* Is this an exact match? */
    if ((ilen == hlen) && strncasecmp(ident, hostname, hlen) == 0)
	return TRUE;

    for (i = 0; i < ilen; i++) {
	if (ident[i] == '*' && ident[i + 1] == '.') {
	    while ((c = *hostname++) != '\0')
		if (c == '.')
		    break;
	    i++;
	}
	else {
	    if (ident[i] != *hostname++)
		return FALSE;
	}
    }
    return *hostname == '\0';
}

static Str
ssl_check_cert_ident(X509 * x, char *hostname)
{
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
    i = X509_get_ext_by_NID(x, NID_subject_alt_name, -1);
    if (i >= 0) {
	X509_EXTENSION *ex;
	STACK_OF(GENERAL_NAME) * alt;

	ex = X509_get_ext(x, i);
	alt = X509V3_EXT_d2i(ex);
	if (alt) {
	    int n;
	    GENERAL_NAME *gn;
	    X509V3_EXT_METHOD *method;
	    Str seen_dnsname = NULL;

	    n = sk_GENERAL_NAME_num(alt);
	    for (i = 0; i < n; i++) {
		gn = sk_GENERAL_NAME_value(alt, i);
		if (gn->type == GEN_DNS) {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
		    unsigned char *sn = ASN1_STRING_data(gn->d.ia5);
#else
		    const unsigned char *sn = ASN1_STRING_get0_data(gn->d.ia5);
#endif
		    int sl = ASN1_STRING_length(gn->d.ia5);

		    /*
		     * sn is a pointer to internal data and not guaranteed to
		     * be null terminated. Ensure we have a null terminated
		     * string that we can modify.
		     */
		    char *asn = GC_MALLOC(sl + 1);
		    if (!asn)
			exit(1);
		    bcopy(sn, asn, sl);
		    asn[sl] = '\0';

		    if (!seen_dnsname)
			seen_dnsname = Strnew();
		    /* replace \0 to make full string visible to user */
		    if (sl != strlen(asn)) {
			int i;
			for (i = 0; i < sl; ++i) {
			    if (!asn[i])
				asn[i] = '!';
			}
		    }
		    Strcat_m_charp(seen_dnsname, asn, " ", NULL);
		    if (sl == strlen(asn) /* catch \0 in SAN */
			&& ssl_match_cert_ident(asn, sl, hostname))
			break;
		}
	    }
	    method = X509V3_EXT_get(ex);
	    sk_GENERAL_NAME_free(alt);
	    if (i < n)		/* Found a match */
		match_ident = TRUE;
	    else if (seen_dnsname)
		/* FIXME: gettextize? */
		ret = Sprintf("Bad cert ident from %s: dNSName=%s", hostname,
			      seen_dnsname->ptr);
	}
    }

    if (match_ident == FALSE && ret == NULL) {
	X509_NAME *xn;
	char buf[2048];
	int slen;

	xn = X509_get_subject_name(x);

	slen = X509_NAME_get_text_by_NID(xn, NID_commonName, buf, sizeof(buf));
	if ( slen == -1)
	    /* FIXME: gettextize? */
	    ret = Strnew_charp("Unable to get common name from peer cert");
	else if (slen != strlen(buf)
		|| !ssl_match_cert_ident(buf, strlen(buf), hostname)) {
	    /* replace \0 to make full string visible to user */
	    if (slen != strlen(buf)) {
		int i;
		for (i = 0; i < slen; ++i) {
		    if (!buf[i])
			buf[i] = '!';
		}
	    }
	    /* FIXME: gettextize? */
	    ret = Sprintf("Bad cert ident %s from %s", buf, hostname);
	}
    }
    return ret;
}

Str
ssl_get_certificate(SSL * ssl, char *hostname)
{
    BIO *bp;
    X509 *x;
    X509_NAME *xn;
    char *p;
    int len;
    Str s;
    char buf[2048];
    Str amsg = NULL;
    Str emsg;
    char *ans;

    if (ssl == NULL)
	return NULL;
    x = SSL_get_peer_certificate(ssl);
    if (x == NULL) {
	if (accept_this_site
	    && strcasecmp(accept_this_site->ptr, hostname) == 0)
	    ans = "y";
	else {
	    /* FIXME: gettextize? */
	    emsg = Strnew_charp("No SSL peer certificate: accept? (y/n)");
	    ans = inputAnswer(emsg->ptr);
	}
	if (ans && TOLOWER(*ans) == 'y')
	    /* FIXME: gettextize? */
	    amsg = Strnew_charp
		("Accept SSL session without any peer certificate");
	else {
	    /* FIXME: gettextize? */
	    char *e = "This SSL session was rejected "
		"to prevent security violation: no peer certificate";
	    disp_err_message(e, FALSE);
	    free_ssl_ctx();
	    return NULL;
	}
	if (amsg)
	    disp_err_message(amsg->ptr, FALSE);
	ssl_accept_this_site(hostname);
	/* FIXME: gettextize? */
	s = amsg ? amsg : Strnew_charp("valid certificate");
	return s;
    }
    /* check the cert chain.
     * The chain length is automatically checked by OpenSSL when we
     * set the verify depth in the ctx.
     */
    if (ssl_verify_server) {
	long verr;
	if ((verr = SSL_get_verify_result(ssl))
	    != X509_V_OK) {
	    const char *em = X509_verify_cert_error_string(verr);
	    if (accept_this_site
		&& strcasecmp(accept_this_site->ptr, hostname) == 0)
		ans = "y";
	    else {
		/* FIXME: gettextize? */
		emsg = Sprintf("%s: accept? (y/n)", em);
		ans = inputAnswer(emsg->ptr);
	    }
	    if (ans && TOLOWER(*ans) == 'y') {
		/* FIXME: gettextize? */
		amsg = Sprintf("Accept unsecure SSL session: "
			       "unverified: %s", em);
	    }
	    else {
		/* FIXME: gettextize? */
		char *e =
		    Sprintf("This SSL session was rejected: %s", em)->ptr;
		disp_err_message(e, FALSE);
		free_ssl_ctx();
		return NULL;
	    }
	}
    }
    emsg = ssl_check_cert_ident(x, hostname);
    if (emsg != NULL) {
	if (accept_this_site
	    && strcasecmp(accept_this_site->ptr, hostname) == 0)
	    ans = "y";
	else {
	    Str ep = Strdup(emsg);
	    if (ep->length > COLS - 16)
		Strshrink(ep, ep->length - (COLS - 16));
	    Strcat_charp(ep, ": accept? (y/n)");
	    ans = inputAnswer(ep->ptr);
	}
	if (ans && TOLOWER(*ans) == 'y') {
	    /* FIXME: gettextize? */
	    amsg = Strnew_charp("Accept unsecure SSL session:");
	    Strcat(amsg, emsg);
	}
	else {
	    /* FIXME: gettextize? */
	    char *e = "This SSL session was rejected "
		"to prevent security violation";
	    disp_err_message(e, FALSE);
	    free_ssl_ctx();
	    return NULL;
	}
    }
    if (amsg)
	disp_err_message(amsg->ptr, FALSE);
    ssl_accept_this_site(hostname);
    /* FIXME: gettextize? */
    s = amsg ? amsg : Strnew_charp("valid certificate");
    Strcat_charp(s, "\n");
    xn = X509_get_subject_name(x);
    if (X509_NAME_get_text_by_NID(xn, NID_commonName, buf, sizeof(buf)) == -1)
	Strcat_charp(s, " subject=<unknown>");
    else
	Strcat_m_charp(s, " subject=", buf, NULL);
    xn = X509_get_issuer_name(x);
    if (X509_NAME_get_text_by_NID(xn, NID_commonName, buf, sizeof(buf)) == -1)
	Strcat_charp(s, ": issuer=<unknown>");
    else
	Strcat_m_charp(s, ": issuer=", buf, NULL);
    Strcat_charp(s, "\n\n");

    bp = BIO_new(BIO_s_mem());
    X509_print(bp, x);
    len = (int)BIO_ctrl(bp, BIO_CTRL_INFO, 0, (char *)&p);
    Strcat_charp_n(s, p, len);
    BIO_free_all(bp);
    X509_free(x);
    return s;
}

/* Raw level input stream functions */

static void
basic_close(int *handle)
{
    close(*(int *)handle);
    xfree(handle);
}

static int
basic_read(int *handle, char *buf, int len)
{
    return read(*(int *)handle, buf, len);
}

static void
file_close(struct io_file_handle *handle)
{
    handle->close(handle->f);
    xfree(handle);
}

static int
file_read(struct io_file_handle *handle, char *buf, int len)
{
    return fread(buf, 1, len, handle->f);
}

static int
str_read(Str handle, char *buf, int len)
{
    return 0;
}

static void
ssl_close(struct ssl_handle *handle)
{
    close(handle->sock);
    if (handle->ssl)
	SSL_free(handle->ssl);
    xfree(handle);
}

static int
ssl_read(struct ssl_handle *handle, char *buf, int len)
{
    int status;
    if (handle->ssl) {
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
    }
    else
	status = read(handle->sock, buf, len);
    return status;
}

static void
ens_close(struct ens_handle *handle)
{
    ISclose(handle->is);
    growbuf_clear(&handle->gb);
    xfree(handle);
}

static int
ens_read(struct ens_handle *handle, char *buf, int len)
{
    if (handle->pos == handle->gb.length) {
	char *p;
	struct growbuf gbtmp;

	ISgets_to_growbuf(handle->is, &handle->gb, TRUE);
	if (handle->gb.length == 0)
	    return 0;
	if (handle->encoding == ENC_BASE64)
	    memchop(handle->gb.ptr, &handle->gb.length);
	else if (handle->encoding == ENC_UUENCODE) {
	    if (handle->gb.length >= 5 &&
		!strncmp(handle->gb.ptr, "begin", 5))
		ISgets_to_growbuf(handle->is, &handle->gb, TRUE);
	    memchop(handle->gb.ptr, &handle->gb.length);
	}
	growbuf_init_without_GC(&gbtmp);
	p = handle->gb.ptr;
	if (handle->encoding == ENC_QUOTE)
	    decodeQP_to_growbuf(&gbtmp, &p);
	else if (handle->encoding == ENC_BASE64)
	    decodeB_to_growbuf(&gbtmp, &p);
	else if (handle->encoding == ENC_UUENCODE)
	    decodeU_to_growbuf(&gbtmp, &p);
	growbuf_clear(&handle->gb);
	handle->gb = gbtmp;
	handle->pos = 0;
    }

    if (len > handle->gb.length - handle->pos)
	len = handle->gb.length - handle->pos;

    memcpy(buf, &handle->gb.ptr[handle->pos], len);
    handle->pos += len;
    return len;
}

static void
memchop(char *p, int *len)
{
    char *q;

    for (q = p + *len; q > p; --q) {
	if (q[-1] != '\n' && q[-1] != '\r')
	    break;
    }
    if (q != p + *len)
	*q = '\0';
    *len = q - p;
    return;
}
