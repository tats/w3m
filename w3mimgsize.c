/* $Id: w3mimgsize.c,v 1.1 2002/01/31 17:54:57 ukai Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <Imlib.h>

int
main(int argc, char **argv)
{
    Display *display;
    ImlibData *id;
    ImlibImage *im;

    fclose(stderr);
    if (argc < 2)
	exit(1);
    display = XOpenDisplay(NULL);
    if (!display)
	exit(1);
    id = Imlib_init(display);
    if (!id)
	exit(1);
    im = Imlib_load_image(id, argv[1]);
    if (!im)
	exit(1);
    printf("%d %d\n", im->rgb_width, im->rgb_height);
    /*
     * Imlib_kill_image(id, im);
     * XCloseDisplay(display);
     */
    exit(0);
}
