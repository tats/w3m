#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#define MYBUFSIZ (BUFSIZ * 0x10)

/* cf. rfc1950.txt */
static char dummy_head[1 + 1] = {
    0x8 + 0x7 * 0x10,
    (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
};

int
main(int argc, char *argv[])
{
    int err, nfiles, i, nin, inrest, eoin, nout, outrest, eoout, ninflates,
	raw;
    z_stream d_stream;
    char *me = argv[0];
    char *inbuf, *outbuf, **filev;

    if (!(inbuf = malloc(MYBUFSIZ))) {
	fprintf(stderr, "%s: inbuf = malloc(%lu): %s\n", me,
		(unsigned long)MYBUFSIZ, strerror(errno));
	exit(1);
    }

    if (!(outbuf = malloc(MYBUFSIZ))) {
	fprintf(stderr, "%s: outbuf = malloc(%lu): %s\n", me,
		(unsigned long)MYBUFSIZ, strerror(errno));
	exit(1);
    }

    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;

    if (argc > 1) {
	nfiles = argc - 1;
	filev = &argv[1];
    }
    else {
	static char *myargv[] = { "-", NULL };

	nfiles = 1;
	filev = myargv;
    }

    if ((err = inflateInit(&d_stream)) != Z_OK) {
	fprintf(stderr, "%s: inflateInit(&d_stream): %d\n", me, err);
	exit(1);
    }

    for (raw = ninflates = i = inrest = outrest = 0, eoout = 1; i < nfiles;
	 ++i) {
	FILE *in;

	if (strcmp(filev[i], "-")) {
	    if (!(in = fopen(filev[i], "rb"))) {
		fprintf(stderr, "%s: fopen(\"%s\", \"rb\"): %s\n", me,
			filev[i], strerror(errno));
		exit(1);
	    }
	}
	else
	    in = stdin;

	for (eoin = 0;;) {
	    if ((nin =
		 fread(&inbuf[inrest], 1, MYBUFSIZ - inrest,
		       in)) < MYBUFSIZ - inrest) {
		if (ferror(in)) {
		    fprintf(stderr, "%s: fread(&inbuf[%d], 1, %d, in): %s\n",
			    me, inrest, MYBUFSIZ - inrest, strerror(errno));
		    exit(1);
		}

		eoin = 1;
	    }

	    if (nin > 0) {
	      retry:
		d_stream.next_in = inbuf;
		d_stream.avail_in = inrest + nin;

		for (eoout = 0;;) {
		    d_stream.next_out = &outbuf[outrest];
		    d_stream.avail_out = MYBUFSIZ - outrest;

		    switch (err = inflate(&d_stream, Z_NO_FLUSH)) {
		    case Z_OK:
			if (!
			    ((nout =
			      fwrite(outbuf, 1, MYBUFSIZ - d_stream.avail_out,
				     stdout)) > 0)) {
			    fprintf(stderr,
				    "%s: fwrite(outbuf, 1, %d, stdout): %s\n",
				    me, MYBUFSIZ - d_stream.avail_out,
				    strerror(errno));
			    exit(1);
			}

			memmove(outbuf, &outbuf[nout],
				MYBUFSIZ - d_stream.avail_out - nout);
			outrest = MYBUFSIZ - d_stream.avail_out - nout;
			++ninflates;
			break;
		    case Z_STREAM_END:
		    case Z_BUF_ERROR:
			ninflates = 0;
			outrest = MYBUFSIZ - d_stream.avail_out;
			eoout = 1;
			goto next_fread;
		    case Z_DATA_ERROR:
			if (!ninflates) {
			    if ((err = inflateReset(&d_stream)) != Z_OK) {
				fprintf(stderr,
					"%s: inflateReset(&d_stream): %d\n",
					me, err);
				exit(1);
			    }

			    d_stream.next_in = dummy_head;
			    d_stream.avail_in = sizeof(dummy_head);

			    if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_OK) {
				raw = ninflates = 1;
				goto retry;
			    }
			}
		    default:
			fprintf(stderr,
				"%s: inflate(&d_stream, Z_NO_FLUSH): %d\n", me,
				err);
			exit(1);
		    }
		}
	    }

	  next_fread:
	    if (d_stream.avail_in) {
		memmove(inbuf, &inbuf[inrest + nin - d_stream.avail_in],
			d_stream.avail_in);
		inrest = d_stream.avail_in;
	    }
	    else
		inrest = 0;

	    if (eoin)
		break;
	}

	if (in != stdin)
	    fclose(in);
    }

    if (!eoout && !raw) {
	fprintf(stderr, "%s: short input\n", me);
	exit(1);
    }

    if (inrest)
	fprintf(stderr, "%s: trailing garbages are ignored\n", me);

    if (outrest && fwrite(outbuf, 1, outrest, stdout) < outrest) {
	fprintf(stderr, "%s: fwrite(outbuf, 1, %d, stdout): %s\n", me, outrest,
		strerror(errno));
	exit(1);
    }

    return 0;
}
