/* $Id: inflate.c,v 1.4 2002/01/31 09:43:15 ukai Exp $ */
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
	if (!f) {
	    fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[1]);
	    exit(1);
	}
    }
    else
	f = stdin;

    s.zalloc = Z_NULL;
    s.zfree = Z_NULL;
    s.opaque = Z_NULL;
    status = inflateInit(&s);
    if (status != Z_OK) {
	fprintf(stderr, "%s: inflateInit() %s\n", argv[0], zError(status));
	exit(1);
    }
    s.avail_in = 0;
    s.next_out = (Bytef *) outbuf;
    s.avail_out = sizeof(outbuf);
    flush = Z_NO_FLUSH;
    while (1) {
	if (s.avail_in == 0) {
	    s.next_in = (Bytef *) inbuf;
	    s.avail_in = fread(inbuf, 1, sizeof(inbuf), f);
	}
	status = inflate(&s, flush);
	if (status == Z_STREAM_END) {
	    if (sizeof(outbuf) - s.avail_out)
		fwrite(outbuf, 1, sizeof(outbuf) - s.avail_out, stdout);
	    break;
	}
	if (status != Z_OK) {
	    fprintf(stderr, "%s: inflate() %s\n", argv[0], zError(status));
	    exit(1);
	}
	if (s.avail_out == 0) {
	    fwrite(outbuf, 1, sizeof(outbuf), stdout);
	    s.next_out = (Bytef *) outbuf;
	    s.avail_out = sizeof(outbuf);
	}
    }
    inflateEnd(&s);
    fclose(f);
    return 0;
}
