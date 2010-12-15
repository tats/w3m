/* $Id: mktable.c,v 1.16 2010/12/15 10:50:24 htrb Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include "myctype.h"
#include "config.h"
#include "hash.h"
#include "myctype.h"
#include "Str.h"
#include <gc.h>

/* *INDENT-OFF* */
defhash(HashItem_ss *, int, hss_i)
/* *INDENT-ON* */

#define keycomp(x,y) ((x)==(y))

/* XXX: we assume sizeof(unsigned long) >= sizeof(void *) */
static unsigned long
hashfunc(HashItem_ss * x)
{
    return (unsigned long)x;
}

/* *INDENT-OFF* */
defhashfunc(HashItem_ss *, int, hss_i)
/* *INDENT-ON* */

int
main(int argc, char *argv[], char **envp)
{
    FILE *f;
    Hash_ss *hash;
    HashItem_ss **hashitems, *hi;
    int size, n, i, j;
    Str s, name, fbase;
    char *p;
    Hash_hss_i *rhash;

    GC_INIT();
    if (argc != 3) {
	fprintf(stderr, "usage: %s hashsize file.tab > file.c\n", argv[0]);
	exit(1);
    }
    size = atoi(argv[1]);
    if (size <= 0) {
	fprintf(stderr, "hash size should be positive\n");
	exit(1);
    }
    if ((f = fopen(argv[2], "r")) == NULL) {
	fprintf(stderr, "Can't open %s\n", argv[2]);
	exit(1);
    }
    p = argv[2];
    if (strrchr(p, '/') != NULL)
	p = strrchr(p, '/') + 1;
    fbase = Strnew_charp(p);
    if (strchr(fbase->ptr, '.'))
	while (Strlastchar(fbase) != '.')
	    Strshrink(fbase, 1);
    Strshrink(fbase, 1);

    hash = newHash_ss(size);
    printf("#include \"hash.h\"\n");
    for (;;) {
	s = Strfgets(f);
	if (s->length == 0)
	    exit(0);
	Strremovetrailingspaces(s);
	if (Strcmp_charp(s, "%%") == 0)
	    break;
	puts(s->ptr);
    }
    n = 0;
    for (;;) {
	s = Strfgets(f);
	if (s->length == 0)
	    break;
	Strremovefirstspaces(s);
	Strremovetrailingspaces(s);
	name = Strnew();
	for (p = s->ptr; *p; p++) {
	    if (IS_SPACE(*p))
		break;
	    Strcat_char(name, *p);
	}
	while (*p && IS_SPACE(*p))
	    p++;
	putHash_ss(hash, name->ptr, p);
	n++;
    }
    fclose(f);

    hashitems = (HashItem_ss **) GC_malloc(sizeof(HashItem_ss *) * n);
    rhash = newHash_hss_i(n * 2);
    j = 0;
    for (i = 0; i < hash->size; i++) {
	for (hi = hash->tab[i]; hi != NULL; hi = hi->next) {
	    hashitems[j] = hi;
	    putHash_hss_i(rhash, hi, j);
	    j++;
	}
    }
    printf("static HashItem_si MyHashItem[] = {\n");
    for (i = 0; i < j; i++) {
	printf("    /* %d */ {\"%s\", %s, ", i,
	       hashitems[i]->key, hashitems[i]->value);
	if (hashitems[i]->next == NULL) {
	    printf("NULL},\n");
	}
	else {
	    printf("&MyHashItem[%d]},\n",
		   getHash_hss_i(rhash, hashitems[i]->next, -1));
	}
    }
    printf("};\n\nstatic HashItem_si *MyHashItemTbl[] = {\n");

    for (i = 0; i < hash->size; i++) {
	if (hash->tab[i])
	    printf("    &MyHashItem[%d],\n",
		   getHash_hss_i(rhash, hash->tab[i], -1));
	else
	    printf("    NULL,\n");
    }
    printf("};\n\n");
    printf("Hash_si %s = { %d, MyHashItemTbl };\n", fbase->ptr, hash->size);

    exit(0);
}
