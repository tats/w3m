
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#undef BUFSIZE
#define BUFSIZE 4096

int
main(int argc, char **argv)
{
    z_stream s;
    FILE *f;
    char inbuf[BUFSIZE], outbuf[BUFSIZE];
    int status, flush;

    if (argc > 1) {
	f = fopen(argv[1], "rb");
	if (! f)
	    exit(1);
    } else
	f = stdin;

    s.zalloc = Z_NULL;
    s.zfree = Z_NULL;
    s.opaque = Z_NULL;
    status = deflateInit(&s, Z_DEFAULT_COMPRESSION);
    if (status != Z_OK)
	exit(1);
    s.avail_in = 0;
    s.next_out = outbuf;
    s.avail_out = sizeof(outbuf);
    flush = Z_NO_FLUSH; 
    while (1) {
	if (s.avail_in == 0) {
	    s.next_in = inbuf;
	    s.avail_in = fread(inbuf, 1, sizeof(inbuf), f);
	    if (s.avail_in < sizeof(inbuf))
		flush = Z_FINISH;
	}
	status = deflate(&s, flush);
	if (status == Z_STREAM_END) {
	    if (sizeof(outbuf) - s.avail_out)
		fwrite(outbuf, 1, sizeof(outbuf) - s.avail_out, stdout);
	    break;
	}
	if (status != Z_OK)
	    exit(1);
	if (s.avail_out == 0) {
	    fwrite(outbuf, 1, sizeof(outbuf), stdout);
	    s.next_out = outbuf;
	    s.avail_out = sizeof(outbuf);
	}
    }
    deflateEnd(&s);
    fclose(f);
    return 0;
}
