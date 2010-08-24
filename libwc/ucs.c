
#ifdef USE_UNICODE

#include <stdlib.h>
#include "wc.h"
#include "ucs.h"
#include "search.h"
#include "big5.h"
#include "hkscs.h"
#include "sjis.h"
#include "johab.h"
#include "gbk.h"
#include "gb18030.h"
#include "uhc.h"
#include "viet.h"
#include "wtf.h"

#include "ucs.map"

#include "map/ucs_ambwidth.map"
#include "map/ucs_wide.map"
#include "map/ucs_combining.map"
#include "map/ucs_precompose.map"
#include "map/ucs_hangul.map"
#include "map/ucs_fullwidth.map"
#include "map/ucs_isalpha.map"
#include "map/ucs_isdigit.map"
#include "map/ucs_islower.map"
#include "map/ucs_isupper.map"
#include "map/ucs_case.map"

#define MAX_TAG_MAP 0x100
static int n_tag_map = 0;
static char *tag_map[ MAX_TAG_MAP ];

wc_table *
wc_get_ucs_table(wc_ccs ccs)
{
    int f = WC_CCS_INDEX(ccs);

    switch (WC_CCS_TYPE(ccs)) {
    case WC_CCS_A_CS94:
	if (f < WC_F_ISO_BASE || f > WC_F_CS94_END)
	    return NULL;
	return &ucs_cs94_table[f - WC_F_ISO_BASE];
    case WC_CCS_A_CS94W:
	if (f < WC_F_ISO_BASE || f > WC_F_CS94W_END)
	    return NULL;
	return &ucs_cs94w_table[f - WC_F_ISO_BASE];
    case WC_CCS_A_CS96:
	if (f < WC_F_ISO_BASE || f > WC_F_CS96_END)
	    return NULL;
	return &ucs_cs96_table[f - WC_F_ISO_BASE];
    case WC_CCS_A_CS96W:
	if (f < WC_F_ISO_BASE || f > WC_F_CS96W_END)
	    return NULL;
	return &ucs_cs96w_table[f - WC_F_ISO_BASE];
    case WC_CCS_A_CS942:
	if (f < WC_F_ISO_BASE || f > WC_F_CS942_END)
	    return NULL;
	return &ucs_cs942_table[f - WC_F_ISO_BASE];
    case WC_CCS_A_PCS:
	if (f < WC_F_PCS_BASE || f > WC_F_PCS_END)
	    return NULL;
	return &ucs_pcs_table[f - WC_F_PCS_BASE];
    case WC_CCS_A_PCSW:
	if (f < WC_F_PCS_BASE || f > WC_F_PCSW_END)
	    return NULL;
	return &ucs_pcsw_table[f - WC_F_PCS_BASE];
    default:
	return NULL;
    }
}

wc_wchar_t
wc_ucs_to_any(wc_uint32 ucs, wc_table *t)
{
    wc_wchar_t cc;
    wc_map *map;

    if (t && t->map && ucs && ucs <= WC_C_UCS2_END) {
	map = wc_map_search((wc_uint16)ucs, t->map, t->n);
	if (map)
	    return t->conv(t->ccs, map->code2);
    }
    if (t && (ucs & ~0xFFFF) == WC_C_UCS4_PLANE2) {
	if (t->ccs == WC_CCS_JIS_X_0213_1)
	    map = wc_map_search((wc_uint16)(ucs & 0xffff),
		ucs_p2_jisx02131_map, N_ucs_p2_jisx02131_map);
	else if (t->ccs == WC_CCS_JIS_X_0213_2)
	    map = wc_map_search((wc_uint16)(ucs & 0xffff),
		ucs_p2_jisx02132_map, N_ucs_p2_jisx02132_map);
	else if (t->ccs == WC_CCS_HKSCS ||
		 t->ccs == WC_CCS_HKSCS_1 || t->ccs == WC_CCS_HKSCS_2)
	    map = wc_map_search((wc_uint16)(ucs & 0xffff),
		ucs_p2_hkscs_map, N_ucs_p2_hkscs_map);
	else
	    map = NULL;
	if (map)
	    return t->conv(t->ccs, map->code2);
    }
    cc.ccs = WC_CCS_UNKNOWN;
    return cc;
}

wc_uint32
wc_any_to_ucs(wc_wchar_t cc)
{
    int f;
    wc_uint16 *map = NULL;
    wc_map *map2;

    f = WC_CCS_INDEX(cc.ccs);
    switch (WC_CCS_TYPE(cc.ccs)) {
    case WC_CCS_A_CS94:
	if (cc.ccs == WC_CCS_US_ASCII)
	    return cc.code;
	if (f < WC_F_ISO_BASE || f > WC_F_CS94_END)
	    return WC_C_UCS4_ERROR;
	map = cs94_ucs_map[f - WC_F_ISO_BASE];
	cc.code &= 0x7f;
	break;
    case WC_CCS_A_CS94W:
	if (cc.ccs == WC_CCS_GB_2312 && WcOption.use_gb12345_map) {
	    cc.ccs = WC_CCS_GB_12345;
	    return wc_any_to_ucs(cc);
	} else if (cc.ccs == WC_CCS_JIS_X_0213_1) {
	    map2 = wc_map_search((wc_uint16)(cc.code & 0x7f7f),
		jisx02131_ucs_p2_map, N_jisx02131_ucs_p2_map);
	    if (map2)
		return map2->code2 | WC_C_UCS4_PLANE2;
	} else if (cc.ccs == WC_CCS_JIS_X_0213_2) {
	    map2 = wc_map_search((wc_uint16)(cc.code & 0x7f7f),
		jisx02132_ucs_p2_map, N_jisx02132_ucs_p2_map);
	    if (map2)
		return map2->code2 | WC_C_UCS4_PLANE2;
	}
	if (f < WC_F_ISO_BASE || f > WC_F_CS94W_END)
	    return 0;
	map = cs94w_ucs_map[f - WC_F_ISO_BASE];
	cc.code = WC_CS94W_N(cc.code);
	break;
    case WC_CCS_A_CS96:
	if (f < WC_F_ISO_BASE || f > WC_F_CS96_END)
	    return WC_C_UCS4_ERROR;
	map = cs96_ucs_map[f - WC_F_ISO_BASE];
	cc.code &= 0x7f;
	break;
    case WC_CCS_A_CS96W:
	if (f < WC_F_ISO_BASE || f > WC_F_CS96W_END)
	    return WC_C_UCS4_ERROR;
	map = cs96w_ucs_map[f - WC_F_ISO_BASE];
	cc.code = WC_CS96W_N(cc.code);
	break;
    case WC_CCS_A_CS942:
	if (f < WC_F_ISO_BASE || f > WC_F_CS942_END)
	    return WC_C_UCS4_ERROR;
	map = cs942_ucs_map[f - WC_F_ISO_BASE];
	cc.code &= 0x7f;
	break;
    case WC_CCS_A_PCS:
	if (f < WC_F_PCS_BASE || f > WC_F_PCS_END)
	    return WC_C_UCS4_ERROR;
	switch (cc.ccs) {
	case WC_CCS_CP1258_2:
	    map2 = wc_map_search((wc_uint16)cc.code,
		cp12582_ucs_map, N_cp12582_ucs_map);
	    if (map2)
		return map2->code2;
	    return WC_C_UCS4_ERROR;
	case WC_CCS_TCVN_5712_3:
	    return wc_any_to_ucs(wc_tcvn57123_to_tcvn5712(cc));
	case WC_CCS_GBK_80:
	    return WC_C_UCS2_EURO;
	}
	map = pcs_ucs_map[f - WC_F_PCS_BASE];
	cc.code &= 0x7f;
	break;
    case WC_CCS_A_PCSW:
	if (f < WC_F_PCS_BASE || f > WC_F_PCSW_END)
	    return WC_C_UCS4_ERROR;
	map = pcsw_ucs_map[f - WC_F_PCS_BASE];
	switch (cc.ccs) {
	case WC_CCS_BIG5:
	    cc.code = WC_BIG5_N(cc.code);
	    break;
	case WC_CCS_BIG5_2:
	    cc.code = WC_CS94W_N(cc.code) + WC_C_BIG5_2_BASE;
	    break;
	case WC_CCS_HKSCS_1:
	case WC_CCS_HKSCS_2:
	    cc = wc_cs128w_to_hkscs(cc);
	case WC_CCS_HKSCS:
	    map2 = wc_map_search((wc_uint16)cc.code,
		hkscs_ucs_p2_map, N_hkscs_ucs_p2_map);
	    if (map2)
		return map2->code2 | WC_C_UCS4_PLANE2;
	    cc.code = wc_hkscs_to_N(cc.code);
	    break;
	case WC_CCS_JOHAB:
	    return wc_any_to_ucs(wc_johab_to_cs128w(cc));
	case WC_CCS_JOHAB_1:
	    return WC_CS94x128_N(cc.code) + WC_C_UCS2_HANGUL;
	case WC_CCS_JOHAB_2:
	    cc.code = WC_CS128W_N(cc.code);
	    cc.code = WC_N_JOHAB2(cc.code);
	    map2 = wc_map_search((wc_uint16)cc.code,
		johab2_ucs_map, N_johab2_ucs_map);
	    if (map2)
		return map2->code2;
	    return WC_C_UCS4_ERROR;
	case WC_CCS_JOHAB_3:
	    if ((cc.code & 0x7f7f) < 0x2121)
		return WC_C_UCS4_ERROR;
	case WC_CCS_SJIS_EXT:
	    return wc_any_to_ucs(wc_sjis_ext_to_cs94w(cc));
	case WC_CCS_SJIS_EXT_1:
	    cc.code = wc_sjis_ext1_to_N(cc.code);
	    if (cc.code == WC_C_SJIS_ERROR)
		return WC_C_UCS4_ERROR;
	    break;
	case WC_CCS_SJIS_EXT_2:
	    cc.code = wc_sjis_ext2_to_N(cc.code);
	    if (cc.code == WC_C_SJIS_ERROR)
		return WC_C_UCS4_ERROR;
	    break;
	case WC_CCS_GBK_1:
	case WC_CCS_GBK_2:
	    cc = wc_cs128w_to_gbk(cc);
	case WC_CCS_GBK:
	    cc.code = wc_gbk_to_N(cc.code);
	    break;
	case WC_CCS_GBK_EXT:
	case WC_CCS_GBK_EXT_1:
	case WC_CCS_GBK_EXT_2:
	    return wc_gb18030_to_ucs(cc);
	case WC_CCS_UHC_1:
	case WC_CCS_UHC_2:
	    cc = wc_cs128w_to_uhc(cc);
	case WC_CCS_UHC:
	    if (cc.code > WC_C_UHC_END)
		return WC_C_UCS4_ERROR;
	    cc.code = wc_uhc_to_N(cc.code);
	    break;
	default:
	    cc.code = WC_CS94W_N(cc.code);
	    break;
	}
	break;
    case WC_CCS_A_WCS16:
	switch (WC_CCS_SET(cc.ccs)) {
	case WC_CCS_UCS2:
	    return cc.code;
	}
	return WC_C_UCS4_ERROR;
    case WC_CCS_A_WCS32:
	switch (WC_CCS_SET(cc.ccs)) {
	case WC_CCS_UCS4:
	    return cc.code;
	case WC_CCS_UCS_TAG:
	    return wc_ucs_tag_to_ucs(cc.code);
	case WC_CCS_GB18030:
	    return wc_gb18030_to_ucs(cc);
	}
	return WC_C_UCS4_ERROR;
    case WC_CCS_A_UNKNOWN:
	if (cc.ccs == WC_CCS_C1)
	    return (cc.code | 0x80);
    default:
	return WC_C_UCS4_ERROR;
    }
    if (map == NULL)
	return WC_C_UCS4_ERROR;
    cc.code = map[cc.code];
    return cc.code ? cc.code : WC_C_UCS4_ERROR;
}

wc_wchar_t
wc_any_to_any(wc_wchar_t cc, wc_table *t)
{
    wc_ccs is_wide = WC_CCS_IS_WIDE(cc.ccs);
    wc_uint32 ucs = wc_any_to_ucs(cc);

    if (ucs != WC_C_UCS4_ERROR) {
	cc = wc_ucs_to_any(ucs, t);
	if (!WC_CCS_IS_UNKNOWN(cc.ccs))
	    return cc;

	ucs = wc_ucs_to_fullwidth(ucs);
	if (ucs != WC_C_UCS4_ERROR) {
	    cc = wc_ucs_to_any(ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
    }
    cc.ccs = is_wide ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
    return cc;
}

wc_wchar_t
wc_ucs_to_any_list(wc_uint32 ucs, wc_table **tlist)
{
    wc_wchar_t cc;
    wc_table **t;

    if (tlist != NULL) {
	for (t = tlist; *t != NULL; t++) {
	    if ((*t)->map == NULL)
		continue;
	    cc = wc_ucs_to_any(ucs, *t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
    }
    cc.ccs = WC_CCS_UNKNOWN;
    return cc;
}

wc_wchar_t
wc_any_to_any_ces(wc_wchar_t cc, wc_status *st)
{
    wc_uint32 ucs = wc_any_to_ucs(cc);
    wc_ccs is_wide = WC_CCS_IS_WIDE(cc.ccs);

    if (ucs < 0x80) {
	cc.ccs = WC_CCS_US_ASCII;
	cc.code = ucs;
	return cc;
    }
    if (ucs != WC_C_UCS4_ERROR) {
	if (st->ces_info->id & WC_CES_T_UTF) {
	    cc.ccs = wc_ucs_to_ccs(ucs);
	    cc.code = ucs;
	    return cc;
	} else if (st->ces_info->id == WC_CES_JOHAB) {
	    cc = wc_ucs_to_johab(ucs);
	    if (WC_CCS_IS_UNKNOWN(cc.ccs))
		cc.ccs = is_wide ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	    return cc;
	}
	cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlistw : st->tlist);
	if (!WC_CCS_IS_UNKNOWN(cc.ccs))
	    return cc;
	if (! WcOption.fix_width_conv) {
	    cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlist : st->tlistw);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
	if (st->ces_info->id == WC_CES_GB18030) {
	    cc = wc_ucs_to_gb18030(ucs);
	    if (WC_CCS_IS_UNKNOWN(cc.ccs))
		cc.ccs = is_wide ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	    return cc;
	}
	if (ucs == WC_C_UCS2_NBSP) {	/* NBSP -> SP */
	    cc.ccs = WC_CCS_US_ASCII;
	    cc.code = 0x20;
	    return cc;
	}
	if (st->ces_info->id & (WC_CES_T_ISO_8859|WC_CES_T_EUC) &&
	    0x80 <= ucs && ucs <= 0x9F) {
	    cc.ccs = WC_CCS_C1;
	    cc.code = ucs;
	    return cc;
	}

	ucs = wc_ucs_to_fullwidth(ucs);
	if (ucs != WC_C_UCS4_ERROR) {
	    cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlistw : st->tlist);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	    if (! WcOption.fix_width_conv) {
		cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlist : st->tlistw);
		if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		    return cc;
	    }
	}
    }
    cc.ccs = is_wide ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
    return cc;
}

wc_wchar_t
wc_any_to_iso2022(wc_wchar_t cc, wc_status *st)
{
    wc_uint32 ucs = wc_any_to_ucs(cc);
    wc_ccs is_wide = WC_CCS_IS_WIDE(cc.ccs);

    if (ucs < 0x80) {
	cc.ccs = WC_CCS_US_ASCII;
	cc.code = ucs;
	return cc;
    }
    if (ucs != WC_C_UCS4_ERROR) {
	cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlistw : st->tlist);
	if (!WC_CCS_IS_UNKNOWN(cc.ccs))
	    return cc;
	if (! WcOption.strict_iso2022) {
	    cc = (is_wide) ? wc_ucs_to_iso2022w(ucs) : wc_ucs_to_iso2022(ucs);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
	if (! WcOption.fix_width_conv) {
	    cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlist : st->tlistw);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	    if (! WcOption.strict_iso2022) {
		cc = (is_wide) ? wc_ucs_to_iso2022(ucs) : wc_ucs_to_iso2022w(ucs);
		if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		    return cc;
	    }
	}
	if (ucs == WC_C_UCS2_NBSP) {	/* NBSP -> SP */
	   cc.ccs = WC_CCS_US_ASCII;
	   cc.code = 0x20;
	   return cc;
	}

	ucs = wc_ucs_to_fullwidth(ucs);
	if (ucs != WC_C_UCS4_ERROR) {
	    cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlistw : st->tlist);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	    if (! WcOption.strict_iso2022) {
		cc = (is_wide) ? wc_ucs_to_iso2022w(ucs) : wc_ucs_to_iso2022(ucs);
		if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		    return cc;
	    }
	    if (! WcOption.fix_width_conv) {
		cc = wc_ucs_to_any_list(ucs, is_wide ? st->tlist : st->tlistw);
		if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		    return cc;
		if (! WcOption.strict_iso2022) {
		    cc = (is_wide) ? wc_ucs_to_iso2022(ucs) : wc_ucs_to_iso2022w(ucs);
		    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
			return cc;
		}
	    }
	}
	if (ucs == WC_C_UCS2_NBSP) {	/* NBSP -> SP */
	   cc.ccs = WC_CCS_US_ASCII;
	   cc.code = 0x20;
	   return cc;
	}
    }
    cc.ccs = is_wide ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
    return cc;
}

wc_wchar_t
wc_ucs_to_iso2022(wc_uint32 ucs)
{
    wc_table *t;
    wc_wchar_t cc;
    int f;

    if (ucs <= WC_C_UCS2_END) {
	for (f = 0; f <= WC_F_CS96_END - WC_F_ISO_BASE; f++) {
	    t = &ucs_cs96_table[f];
	    if (t->map == NULL)
		continue;
	    cc = wc_ucs_to_any((wc_uint16)ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
	for (f = 0; f <= WC_F_CS94_END - WC_F_ISO_BASE; f++) {
	    t = &ucs_cs94_table[f];
	    if (t->map == NULL)
		continue;
	    cc = wc_ucs_to_any((wc_uint16)ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
	for (f = 0; f <= WC_F_CS942_END - WC_F_ISO_BASE; f++) {
	    t = &ucs_cs942_table[f];
	    if (t->map == NULL)
		continue;
	    cc = wc_ucs_to_any((wc_uint16)ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
    }
    cc.ccs = WC_CCS_UNKNOWN;
    return cc;
}

wc_wchar_t
wc_ucs_to_iso2022w(wc_uint32 ucs)
{
    wc_table *t;
    wc_wchar_t cc;
    int f;

    if (ucs <= WC_C_UCS2_END) {
	for (f = 0; f <= WC_F_CS94W_END - WC_F_ISO_BASE; f++) {
	    t = &ucs_cs94w_table[f];
	    if (t->map == NULL)
		continue;
	    cc = wc_ucs_to_any((wc_uint16)ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
	for (f = 0; f <= WC_F_CS96W_END - WC_F_ISO_BASE; f++) {
	    t = &ucs_cs96w_table[f];
	    if (t->map == NULL)
		continue;
	    cc = wc_ucs_to_any((wc_uint16)ucs, t);
	    if (!WC_CCS_IS_UNKNOWN(cc.ccs))
		return cc;
	}
    }
    cc.ccs = WC_CCS_UNKNOWN_W;
    return cc;
}

wc_ccs
wc_ucs_to_ccs(wc_uint32 ucs)
{
    if (0x80 <= ucs && ucs <= 0x9F)
	return WC_CCS_C1;
    return ((ucs <= WC_C_UCS2_END) ? WC_CCS_UCS2 : WC_CCS_UCS4)
	| ((WcOption.east_asian_width && wc_is_ucs_ambiguous_width(ucs))
		    ? WC_CCS_A_WIDE : 0)
	| (wc_is_ucs_wide(ucs) ? WC_CCS_A_WIDE : 0)
	| (wc_is_ucs_combining(ucs) ? WC_CCS_A_COMB : 0);
}

wc_bool
wc_is_ucs_ambiguous_width(wc_uint32 ucs)
{
    if (0xa1 <= ucs && ucs <= 0xfe && WcOption.use_jisx0213)
	return 1;
    else if (ucs <= WC_C_UCS2_END)
	return (wc_map_range_search((wc_uint16)ucs,
		    ucs_ambwidth_map, N_ucs_ambwidth_map) != NULL);
    else
	return ((0xF0000 <= ucs && ucs <= 0xFFFFD)
		|| (0x100000 <= ucs && ucs <= 0x10FFFD));
}

wc_bool
wc_is_ucs_wide(wc_uint32 ucs)
{
    if (ucs <= WC_C_UCS2_END)
	return (wc_map_range_search((wc_uint16)ucs,
		ucs_wide_map, N_ucs_wide_map) != NULL);
    else
	return ((ucs & ~0xFFFF) == WC_C_UCS4_PLANE2 ||
		(ucs & ~0xFFFF) == WC_C_UCS4_PLANE3);
}

wc_bool
wc_is_ucs_combining(wc_uint32 ucs)
{
    return (WcOption.use_combining && ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_combining_map, N_ucs_combining_map) != NULL);
}

wc_bool
wc_is_ucs_hangul(wc_uint32 ucs)
{
    return (ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_hangul_map, N_ucs_hangul_map) != NULL);
}

wc_bool
wc_is_ucs_alpha(wc_uint32 ucs)
{
    return (ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_isalpha_map, N_ucs_isalpha_map) != NULL);
}

wc_bool
wc_is_ucs_digit(wc_uint32 ucs)
{
    return (ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_isdigit_map, N_ucs_isdigit_map) != NULL);
}

wc_bool
wc_is_ucs_alnum(wc_uint32 ucs)
{
    return (wc_is_ucs_alpha(ucs) || wc_is_ucs_digit(ucs));
}

wc_bool
wc_is_ucs_lower(wc_uint32 ucs)
{
    return (ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_islower_map, N_ucs_islower_map) != NULL);
}

wc_bool
wc_is_ucs_upper(wc_uint32 ucs)
{
    return (ucs <= WC_C_UCS2_END &&
	wc_map_range_search((wc_uint16)ucs,
	ucs_isupper_map, N_ucs_isupper_map) != NULL);
}

wc_uint32
wc_ucs_toupper(wc_uint32 ucs)
{
    wc_map *conv = NULL;
    if (ucs <= WC_C_UCS2_END)
	conv = wc_map_search((wc_uint16)ucs,
			     ucs_toupper_map, N_ucs_toupper_map);
    return conv ? (wc_uint32)(conv->code2) : ucs;
}

wc_uint32
wc_ucs_tolower(wc_uint32 ucs)
{
    wc_map *conv = NULL;
    if (ucs <= WC_C_UCS2_END)
	conv = wc_map_search((wc_uint16)ucs,
			     ucs_tolower_map, N_ucs_tolower_map);
    return conv ? (wc_uint32)(conv->code2) : ucs;
}

wc_uint32
wc_ucs_totitle(wc_uint32 ucs)
{
    wc_map *conv = NULL;
    if (ucs <= WC_C_UCS2_END)
	conv = wc_map_search((wc_uint16)ucs,
			     ucs_totitle_map, N_ucs_totitle_map);
    return conv ? (wc_uint32)(conv->code2) : ucs;
}

wc_uint32
wc_ucs_precompose(wc_uint32 ucs1, wc_uint32 ucs2)
{
    wc_map3 *map;

    if (WcOption.use_combining &&
	ucs1 <= WC_C_UCS2_END && ucs2 <= WC_C_UCS2_END &&
	(map = wc_map3_search((wc_uint16)ucs1, (wc_uint16)ucs2,
	ucs_precompose_map, N_ucs_precompose_map)) != NULL)
	return map->code3;
    return WC_C_UCS4_ERROR;
}

wc_uint32
wc_ucs_to_fullwidth(wc_uint32 ucs)
{
    wc_map *map;

    if (ucs <= WC_C_UCS2_END &&
	(map = wc_map_search((wc_uint16)ucs,
	ucs_fullwidth_map, N_ucs_fullwidth_map)) != NULL)
	return map->code2;
    return WC_C_UCS4_ERROR;
}

int
wc_ucs_put_tag(char *p)
{
    int i;

    if (p == NULL || *p == '\0')
	return 0;
    for (i = 1; i <= n_tag_map; i++) {
	if (!strcasecmp(p, tag_map[i]))
	    return i;
    }
    n_tag_map++;
    if (n_tag_map == MAX_TAG_MAP)
	return 0;
    tag_map[n_tag_map] = p;
    return n_tag_map;
}

char *
wc_ucs_get_tag(int ntag)
{
    if (ntag == 0 || ntag > n_tag_map)
	return NULL;
    return tag_map[ntag];
}

void
wtf_push_ucs(Str os, wc_uint32 ucs, wc_status *st)
{
    wc_ccs ccs;

    if (ucs >= WC_C_LANGUAGE_TAG0 && ucs <= WC_C_CANCEL_TAG) {
	if (! WcOption.use_language_tag)
	    return;
	if (ucs == WC_C_LANGUAGE_TAG)
	    st->tag = Strnew_size(4);
	else if (ucs == WC_C_CANCEL_TAG) {
	    st->tag = NULL;
	    st->ntag = 0;
	}  else if (st->tag && ucs >= WC_C_TAG_SPACE)
	    Strcat_char(st->tag, (char)(ucs & 0x7f));
	return;
    }
    if (st->tag) {
	st->ntag = wc_ucs_put_tag(st->tag->ptr);
	st->tag = NULL;
    }
    if (ucs < 0x80) {
	if (st->ntag)
	    wtf_push(os, WC_CCS_UCS_TAG,  wc_ucs_to_ucs_tag(ucs, st->ntag));
	else
	    Strcat_char(os, (char)ucs);
    } else {
	ccs = wc_ucs_to_ccs(ucs);
	if (st->ntag && ucs <= WC_C_UNICODE_END) {
	    ccs = wc_ccs_ucs_to_ccs_ucs_tag(ccs);
	    ucs = wc_ucs_to_ucs_tag(ucs, st->ntag);
	}
	wtf_push(os, ccs, ucs);
    }
}

#endif
