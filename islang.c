/*
 * Copyright (c) 2000, NBG01720@nifty.ne.jp
 *
 * To compile this program:
 *      gcc -Zomf -Zcrtdll -O2 -Wall -s islang.c
 */
#define INCL_DOSNLS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int
main(int argc, char **argv)
{
    if (argc <= 1)
	return 1;

    if (isdigit((int)*argv[1])) {
	unsigned long CpList[8], CpSize;
	APIRET rc = DosQueryCp(sizeof(CpList), CpList, &CpSize);
	if (rc)
	    return rc;
	while (--argc > 0)
	    if (*CpList == atoi(argv[argc]))
		return 0;
    }
    else {
	char *lang = getenv("LANG");
	if (!lang || !*lang) {
	    lang = getenv("LANGUAGE");
	    if (!lang || !*lang)
		return 1;
	}
	if (!strnicmp(lang, argv[1], 2))
	    return 0;
    }
    return 1;
}
