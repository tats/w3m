#include "util.h"

#include "display.h"
#include "terms.h"

#include <stdio.h>
#include <stdlib.h>

int
exec_cmd(char *cmd)
{
    int rv;

    fmTerm();
    if ((rv = system(cmd))) {
	printf("\n[Hit any key]");
	fflush(stdout);
	fmInit();
	getch();

	return rv;
    }
    fmInit();

    return 0;
}
