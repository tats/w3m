
#include "fm.h"
#include <stdio.h>
#include "indep.h"
#include "Str.h"
#include "gc.h"

typedef struct {
    short ucs;
    char *ptr;
} entity_map;

#ifdef JP_CHARSET
#include "ucs_eucjp.h"

static char *latin1_eucjp_map[ 96 ] =
{
    NBSP, "!",  "¡ñ", "¡ò", "CUR","¡ï", "|",  "¡ø",	/* 32- 39 */
    "¡¯", "(C)","-a", "¢ã", "¢Ì", "-",  "(R)","¡±",	/* 40- 47 */
    "¡ë", "¡Þ", "^2", "^3", "'",  "¦Ì", "¢ù", "¡¦",	/* 48- 55 */
    ",",  "^1", "-o", "¢ä", "1/4","1/2","3/4","?",	/* 56- 63 */
    "A`", "A'", "A^", "A~", "A:", "¢ò",  "AE","C,",	/* 64- 71 */
    "E`", "E'", "E^", "E", "I`",  "I'", "I^", "I:",	/* 72- 79 */
    "D-", "N~", "O`", "O'", "O^", "O~", "Oe", "¡ß",	/* 80- 87 */
    "¦Õ", "U`", "U'", "U^", "U:", "Y'", "th", "ss",	/* 88- 95 */
    "a`", "a'", "a^", "a~", "a:", "a",  "ae", "c",	/* 96-103 */
    "e`", "e'", "e^", "e:", "i`", "i'", "i^", "i:",	/* 104-111 */
    "d-", "n~", "o`", "o'", "o^", "o~", "oe", "¡à",	/* 112-119 */
    "¦Õ", "u`", "u'", "u^", "u:", "y'", "th", "y:"	/* 120-127 */
};

#else
#ifdef __EMX__
/*
 * Character conversion table
 * ( to code page 850 from iso-8859-1 )
 *
 * Following character constants are in code page 850.
 */
static char *latin1_cp850_map[ 96 ] = {
  NBSP,   "\255", "\275", "\234", "\317", "\276", "\335", "\365",
  "\371", "\270", "\246", "\256", "\252", "\360", "\251", "\356",
  "\370", "\361", "\375", "\374", "\357", "\346", "\364", "\372",
  "\367", "\373", "\247", "\257", "\254", "\253", "\363", "\250",
  "\267", "\265", "\266", "\307", "\216", "\217", "\222", "\200",
  "\324", "\220", "\322", "\323", "\336", "\326", "\327", "\330",
  "\321", "\245", "\343", "\340", "\342", "\345", "\231", "\236",
  "\235", "\353", "\351", "\352", "\232", "\355", "\350", "\341",
  "\205", "\240", "\203", "\306", "\204", "\206", "\221", "\207",
  "\212", "\202", "\210", "\211", "\215", "\241", "\214", "\213",
  "\320", "\244", "\225", "\242", "\223", "\344", "\224", "\366",
  "\233", "\227", "\243", "\226", "\201", "\354", "\347", "\230"
};
#endif
#endif
#include "ucs_latin1.h"

static char *latin1_ascii_map[ 96 ] =
{
    NBSP, "!",  "-c-","-L-","CUR","=Y=","|",  "S:",	/* 32- 39 */
    "\"", "(C)","-a", "<<", "NOT","-",  "(R)","¡±",	/* 40- 47 */
    "DEG","+-", "^2", "^3", "'",  "u",  "P:", ".",	/* 48- 55 */
    ",",  "^1", "-o", ">>", "1/4","1/2","3/4","?",	/* 56- 63 */
    "A`", "A'", "A^", "A~", "A:", "AA", "AE", "C,",	/* 64- 71 */
    "E`", "E'", "E^", "E", "I`",  "I'", "I^", "I:",	/* 72- 79 */
    "D-", "N~", "O`", "O'", "O^", "O~", "Oe", "x",	/* 80- 87 */
    "O/", "U`", "U'", "U^", "U:", "Y'", "th", "ss",	/* 88- 95 */
    "a`", "a'", "a^", "a~", "a:", "a",  "ae", "c",	/* 96-103 */
    "e`", "e'", "e^", "e:", "i`", "i'", "i^", "i:",	/* 104-111 */
    "d-", "n~", "o`", "o'", "o^", "o~", "oe", "-:",	/* 112-119 */
    "o/", "u`", "u'", "u^", "u:", "y'", "th", "y:"	/* 120-127 */
};

char UseAltEntity = FALSE;

static int
map_cmp(const void *a, const void *b)
{
    return *(int *)a - ((entity_map *)b)->ucs;
}

static char *
map_search(int c, entity_map *map, size_t n)
{
    entity_map *m;

    m = (entity_map *)bsearch((void *)&c, (void *)map, n,
	sizeof(entity_map), map_cmp);
    return m ? m->ptr : NULL;
}

char *
conv_entity(int c)
{
    static char buf[] = {0, 0};
    char *p;

    if (c < 0)			/* error */
	return "?";
    if (c < 0x80) {		/* US-ASCII */
	buf[0] = (char)c;
	return buf;
    }
    if (c < 0xa0)		/* C1 */
	return "?";
    if (c == 0xa0)		/* NBSP */
	return NBSP;
    if (c < 0x100) {		/* Latin 1 (ISO-8859-1) */
	if (UseAltEntity)
	    return latin1_ascii_map[c - 0xa0];
#ifdef JP_CHARSET
	return latin1_eucjp_map[c - 0xa0];
#else
#ifdef __EMX__
	if (CodePage == 850)
	    return latin1_cp850_map[c - 0xa0];
#endif
	buf[0] = (char)c;
	return buf;
#endif
    }
				/* Unicode */
#ifdef JP_CHARSET
    if (! UseAltEntity) {
	p = map_search(c, ucs_eucjp_map,
		sizeof(ucs_eucjp_map) / sizeof(entity_map));
	return p ? p : "?";
    }
#endif
    p = map_search(c, ucs_latin1_map,
	sizeof(ucs_latin1_map) / sizeof(entity_map));
    if (p && *p & 0x80)		/* ISO-8859-1 */
	return conv_entity((int)(*p & 0xff));
    return p ? p : "?";
}
