/* $Id: w3mimgsize.c,v 1.3 2002/07/22 16:17:32 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "w3mimg/w3mimg.h"

int
main(int argc, char **argv)
{
    w3mimg_op *w_op = NULL;
    W3MImage img;
    int w, h;

    fclose(stderr);
    if (argc < 2)
	exit(1);
    w_op = w3mimg_open();
    if (w_op == NULL)
	exit(1);

    if (!w_op->init(w_op))
	exit(1);

    if (!w_op->get_image_size(w_op, &img, argv[1], &w, &h))
	exit(1);
    printf("%d %d\n", w, h);
    exit(0);
}
