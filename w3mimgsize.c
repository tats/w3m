/* $Id: w3mimgsize.c,v 1.2 2002/07/17 20:58:48 ukai Exp $ */
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

    fclose(stderr);
    if (argc < 2)
	exit(1);
    w_op = w3mimg_open();
    if (w_op == NULL)
	exit(1);

    if (!w_op->init(w_op))
	exit(1);

    if (!w_op->load_image(w_op, &img, argv[1], -1, -1))
	exit(1);
    printf("%d %d\n", img.width, img.height);
    exit(0);
}
