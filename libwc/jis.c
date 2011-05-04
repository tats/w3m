
#include "wc.h"
#include "jis.h"
#include "search.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif

#include "map/jisx0201k_jisx0208.map"
#include "map/jisx0208_jisx02131.map"

wc_wchar_t
wc_jisx0201k_to_jisx0208(wc_wchar_t cc)
{
    cc.code = jisx0201k_jisx0208_map[cc.code & 0x7f];
    cc.ccs = cc.code ? WC_CCS_JIS_X_0208 : WC_CCS_UNKNOWN_W;
    return cc;
}

wc_wchar_t
wc_jisx0212_to_jisx0213(wc_wchar_t cc)
{
#ifdef USE_UNICODE
    wc_wchar_t cc2;
    static wc_table *t1 = NULL;
    static wc_table *t2 = NULL;

    if (t1 == NULL) {
	t1 = wc_get_ucs_table(WC_CCS_JIS_X_0213_1);
	t2 = wc_get_ucs_table(WC_CCS_JIS_X_0213_2);
    }
    cc2 = wc_any_to_any(cc, t2);
    if (cc2.ccs == WC_CCS_JIS_X_0212)
	return cc2;
    return wc_any_to_any(cc, t1);
#else
    cc.ccs = WC_CCS_UNKNOWN_W;
    return cc;
#endif
}

wc_wchar_t
wc_jisx0213_to_jisx0212(wc_wchar_t cc)
{
#ifdef USE_UNICODE
    static wc_table *t = NULL;

    if (t == NULL)
	t = wc_get_ucs_table(WC_CCS_JIS_X_0212);
    return wc_any_to_any(cc, t);
#else
    cc.ccs = WC_CCS_UNKNOWN_W;
    return cc;
#endif
}

wc_ccs
wc_jisx0208_or_jisx02131(wc_uint16 code)
{
    return wc_map_range_search(code & 0x7f7f,
	jisx0208_jisx02131_map, N_jisx0208_jisx02131_map)
	? WC_CCS_JIS_X_0213_1 : WC_CCS_JIS_X_0208;
}

wc_ccs
wc_jisx0212_or_jisx02132(wc_uint16 code)
{
    return wc_jisx0212_jisx02132_map[(code >> 8) & 0x7f]
	? WC_CCS_JIS_X_0213_2 : WC_CCS_JIS_X_0212;
}
