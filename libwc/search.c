
#include <stdlib.h>
#include "wc.h"

static int
map_cmp(const void *a, const void *b)
{
    return *(wc_uint16 *)a - ((wc_map *)b)->code;
}

static int
map3_cmp(const void *a, const void *b)
{
    return *(wc_uint32 *)a - (((wc_uint32)((wc_map3 *)b)->code << 16) | ((wc_map3 *)b)->code2);
}

static int
map_range_cmp(const void *a, const void *b)
{
    return (*(wc_uint16 *)a < ((wc_map *)b)->code) ? -1
	: ((*(wc_uint16 *)a > ((wc_map *)b)->code2) ? 1 : 0);
}

static int
map2_range_cmp(const void *a, const void *b)
{
    return (*(wc_uint16 *)a < ((wc_map *)b)->code) ? -1
	: ((*(wc_uint16 *)a >= ((wc_map *)b + 1)->code) ? 1 : 0);
}

static int
map3_range_cmp(const void *a, const void *b)
{
    return (*(wc_uint16 *)a < ((wc_map3 *)b)->code) ? -1
	: ((*(wc_uint16 *)a > ((wc_map3 *)b)->code2) ? 1 : 0);
}

wc_map *
wc_map_search(wc_uint16 code, wc_map *map, size_t n)
{
    return (wc_map *)bsearch((void *)&code, (void *)map, n, sizeof(wc_map),
	map_cmp);
}

wc_map3 *
wc_map3_search(wc_uint16 c1, wc_uint16 c2, wc_map3 *map, size_t n)
{
    wc_uint32 code = ((wc_uint32)c1 << 16) | c2;
    return (wc_map3 *)bsearch((void *)&code, (void *)map, n, sizeof(wc_map3),
	map3_cmp);
}

wc_map *
wc_map_range_search(wc_uint16 code, wc_map *map, int n)
{
    return (wc_map *)bsearch((void *)&code, (void *)map, n, sizeof(wc_map),
	map_range_cmp);
}

wc_map *
wc_map2_range_search(wc_uint16 code, wc_map *map, size_t n)
{
    return (wc_map *)bsearch((void *)&code, (void *)map, n, sizeof(wc_map),
	map2_range_cmp);
}

wc_map3 *
wc_map3_range_search(wc_uint16 code, wc_map3 *map, size_t n)
{
    return (wc_map3 *)bsearch((void *)&code, (void *)map, n, sizeof(wc_map3),
	map3_range_cmp);
}
