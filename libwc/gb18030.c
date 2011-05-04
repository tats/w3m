
#include "wc.h"
#include "gb18030.h"
#include "search.h"
#include "wtf.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif
#include "map/gb18030_ucs.map"

#define C0 WC_GB18030_MAP_C0
#define GL WC_GB18030_MAP_GL
#define C1 WC_GB18030_MAP_C1
#define LB WC_GB18030_MAP_LB
#define UB WC_GB18030_MAP_UB
#define L4 WC_GB18030_MAP_L4

wc_uint8 WC_GB18030_MAP[ 0x100 ] = {
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    L4, L4, L4, L4, L4, L4, L4, L4, L4, L4, GL, GL, GL, GL, GL, GL,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, C0,

    LB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, C1,
};

wc_wchar_t
wc_gbk_ext_to_cs128w(wc_wchar_t cc)
{
    cc.code = WC_GBK_N(cc.code);
    if (cc.code < 0x4000)
	cc.ccs = WC_CCS_GBK_EXT_1;
    else {
	cc.ccs = WC_CCS_GBK_EXT_2;
	cc.code -= 0x4000;
    }
    cc.code = WC_N_CS128W(cc.code);
    return cc;
}

wc_wchar_t
wc_cs128w_to_gbk_ext(wc_wchar_t cc)
{
    cc.code = WC_CS128W_N(cc.code);
    if (cc.ccs == WC_CCS_GBK_EXT_2)
	cc.code += 0x4000;
    cc.ccs = WC_CCS_GBK_EXT;
    cc.code = WC_N_GBK(cc.code);
    return cc;
}

static wc_ccs
wc_gbk_or_gbk_ext(wc_uint16 code) {
    return wc_map3_range_search(code,
        gbk_ext_ucs_map, N_gbk_ext_ucs_map)
        ? WC_CCS_GBK_EXT : WC_CCS_GBK;
}

#ifdef USE_UNICODE
wc_uint32
wc_gb18030_to_ucs(wc_wchar_t cc)
{
    wc_map3 *map;

    switch (WC_CCS_SET(cc.ccs)) {
    case WC_CCS_GBK_EXT_1:
    case WC_CCS_GBK_EXT_2:
	cc = wc_cs128w_to_gbk_ext(cc);
    case WC_CCS_GBK_EXT:
	map = wc_map3_range_search((wc_uint16)cc.code,
		gbk_ext_ucs_map, N_gbk_ext_ucs_map);
	if (map)
	    return map->code3 + WC_GBK_N(cc.code) - WC_GBK_N(map->code2);
	return WC_C_UCS4_ERROR;
    case WC_CCS_GB18030:
	break;
    default:
	return wc_any_to_ucs(cc);
    }
    if (cc.code >= WC_C_GB18030_UCS2 && cc.code <= WC_C_GB18030_UCS2_END) {
	int i, min = 0, max = N_ucs_gb18030_map - 1;

	cc.code = WC_GB18030_N(cc.code) - WC_GB18030_N(WC_C_GB18030_UCS2);
	if (cc.code >= ucs_gb18030_map[max].code3)
	    i = max;
	else {
	    while(1) {
		i = (min + max) / 2;
		if (min == max)
		    break;
		if (cc.code < ucs_gb18030_map[i].code3)
		    max = i - 1;
		else if (cc.code >= ucs_gb18030_map[i+1].code3)
		    min = i + 1;
		else
		    break;
	    }
	}
	return ucs_gb18030_map[i].code + cc.code - ucs_gb18030_map[i].code3;
    }
    if (cc.code >= WC_C_GB18030_UCS4 && cc.code <= WC_C_GB18030_UCS4_END)
	return WC_GB18030_N(cc.code) - WC_GB18030_N(WC_C_GB18030_UCS4)
		+ 0x10000;
    return WC_C_UCS4_ERROR;
}

wc_wchar_t
wc_ucs_to_gb18030(wc_uint32 ucs)
{
    wc_wchar_t cc;
    wc_map3 *map;

    if (ucs <= WC_C_UCS2_END) {
	map = wc_map3_range_search((wc_uint16)ucs,
		ucs_gbk_ext_map, N_ucs_gbk_ext_map);
	if (map) {
	    cc.code = WC_GBK_N(map->code3) + ucs - map->code;
	    cc.code = WC_N_GBK(cc.code);
	    cc.ccs = WC_CCS_GBK_EXT;
	    return cc;
	}
	map = wc_map3_range_search((wc_uint16)ucs,
		ucs_gb18030_map, N_ucs_gb18030_map);
	if (map) {
	    cc.code = map->code3 + ucs - map->code + WC_GB18030_N(WC_C_GB18030_UCS2);
	    cc.code = WC_N_GB18030(cc.code);
	    if (WcOption.gb18030_as_ucs)
		cc.ccs = WC_CCS_GB18030 | (wc_ucs_to_ccs(ucs) & ~WC_CCS_A_SET);
	    else
		cc.ccs = WC_CCS_GB18030_W;
	    return cc;
	}
    } else if (ucs <= WC_C_UNICODE_END) {
	cc.code = ucs - 0x10000 + WC_GB18030_N(WC_C_GB18030_UCS4);
	cc.code = WC_N_GB18030(cc.code);
	if (WcOption.gb18030_as_ucs)
	    cc.ccs = WC_CCS_GB18030 | (wc_ucs_to_ccs(ucs) & ~WC_CCS_A_SET);
	else
	    cc.ccs = WC_CCS_GB18030_W;
	return cc;
    }
    cc.ccs = WC_CCS_UNKNOWN;
    return cc;
}
#endif

Str
wc_conv_from_gb18030(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    int state = WC_GB18030_NOSTATE;
    wc_uint32 gbk;
    wc_wchar_t cc;
#ifdef USE_UNICODE
    wc_uint32 ucs;
#endif

    for (p = sp; p < ep && *p < 0x80; p++) 
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, (char *)is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_GB18030_NOSTATE:
	    switch (WC_GB18030_MAP[*p]) {
	    case UB:
		state = WC_GB18030_MBYTE1;
		break;
	    case C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_GB18030_MBYTE1:
	    if (WC_GB18030_MAP[*p] & LB) {
		gbk = ((wc_uint32)*(p-1) << 8) | *p;
		if (wc_gbk_or_gbk_ext(gbk) == WC_CCS_GBK_EXT)
		    wtf_push(os, WC_CCS_GBK_EXT, gbk);
		else if (*(p-1) >= 0xA1 && *p >= 0xA1)
		    wtf_push(os, wc_gb2312_or_gbk(gbk), gbk);
		else
		    wtf_push(os, WC_CCS_GBK, gbk);
	    } else if (WC_GB18030_MAP[*p] == L4) {
		state = WC_GB18030_MBYTE2;
		break;
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_GB18030_NOSTATE;
	    break;
	case WC_GB18030_MBYTE2:
	    if (WC_GB18030_MAP[*p] == UB) {
		state = WC_GB18030_MBYTE3;
		break;
	    } else
		wtf_push_unknown(os, p-2, 3);
	    state = WC_GB18030_NOSTATE;
	    break;
	case WC_GB18030_MBYTE3:
	    if (WC_GB18030_MAP[*p] == L4) {
		cc.ccs = WC_CCS_GB18030_W;
		cc.code = ((wc_uint32)*(p-3) << 24)
		        | ((wc_uint32)*(p-2) << 16)
		        | ((wc_uint32)*(p-1) << 8)
		        | *p;
#ifdef USE_UNICODE
		if (WcOption.gb18030_as_ucs &&
		    (ucs = wc_gb18030_to_ucs(cc)) != WC_C_UCS4_ERROR)
		    wtf_push(os, WC_CCS_GB18030 | (wc_ucs_to_ccs(ucs) & ~WC_CCS_A_SET), cc.code);
		else
#endif
		    wtf_push(os, cc.ccs, cc.code);
	    } else
		wtf_push_unknown(os, p-3, 4);
	    state = WC_GB18030_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_GB18030_MBYTE1:
	wtf_push_unknown(os, p-1, 1);
	break;
    case WC_GB18030_MBYTE2:
	wtf_push_unknown(os, p-2, 2);
	break;
    case WC_GB18030_MBYTE3:
	wtf_push_unknown(os, p-3, 3);
	break;
    }
    return os;
}

void
wc_push_to_gb18030(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (WC_CCS_SET(cc.ccs)) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_GB_2312:
	Strcat_char(os, (char)((cc.code >> 8) | 0x80));
	Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
	return;
    case WC_CCS_GBK_1:
    case WC_CCS_GBK_2:
	cc = wc_cs128w_to_gbk(cc);
    case WC_CCS_GBK:
	Strcat_char(os, (char)(cc.code >> 8));
	Strcat_char(os, (char)(cc.code & 0xff));
	return;
    case WC_CCS_GBK_EXT_1:
    case WC_CCS_GBK_EXT_2:
	cc = wc_cs128w_to_gbk(cc);
    case WC_CCS_GBK_EXT:
	Strcat_char(os, (char)(cc.code >> 8));
	Strcat_char(os, (char)(cc.code & 0xff));
	return;
    case WC_CCS_GB18030:
	Strcat_char(os, (char)((cc.code >> 24) & 0xff));
	Strcat_char(os, (char)((cc.code >> 16) & 0xff));
	Strcat_char(os, (char)((cc.code >> 8)  & 0xff));
	Strcat_char(os, (char)(cc.code & 0xff));
	return;
    case WC_CCS_UNKNOWN_W:
	if (!WcOption.no_replace)
	    Strcat_charp(os, WC_REPLACE_W);
	return;
    case WC_CCS_UNKNOWN:
	if (!WcOption.no_replace)
	    Strcat_charp(os, WC_REPLACE);
	return;
    default:
#ifdef USE_UNICODE
	if (WcOption.ucs_conv)
	    cc = wc_any_to_any_ces(cc, st);
	else
#endif
	    cc.ccs = WC_CCS_IS_WIDE(cc.ccs) ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	continue;
    }
  }
}

Str
wc_char_conv_from_gb18030(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar gb[4];
    wc_uint32 gbk;
    wc_wchar_t cc;
#ifdef USE_UNICODE
    wc_uint32 ucs;
#endif

    if (st->state == -1) {
	st->state = WC_GB18030_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_GB18030_NOSTATE:
	switch (WC_GB18030_MAP[c]) {
	case UB:
	    gb[0] = c;
	    st->state = WC_GB18030_MBYTE1;
	    return NULL;
	case C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_GB18030_MBYTE1:
	if (WC_GB18030_MAP[c] & LB) {
	    gbk = ((wc_uint32)gb[0] << 8) | c;
	    if (wc_gbk_or_gbk_ext(gbk) == WC_CCS_GBK_EXT)
		wtf_push(os, WC_CCS_GBK_EXT, gbk);
	    else if (gb[0] >= 0xA1 && c >= 0xA1)
		wtf_push(os, wc_gb2312_or_gbk(gbk), gbk);
	    else
		wtf_push(os, WC_CCS_GBK, gbk);
	} else if (WC_GB18030_MAP[c] == L4) {
	    gb[1] = c;
	    st->state = WC_GB18030_MBYTE2;
	    return NULL;
	}
	break;
    case WC_GB18030_MBYTE2:
	if (WC_GB18030_MAP[c] == UB) {
	    gb[2] = c;
	    st->state = WC_GB18030_MBYTE3;
	    return NULL;
	}
	break;
    case WC_GB18030_MBYTE3:
	if (WC_GB18030_MAP[c] == L4) {
	    cc.ccs = WC_CCS_GB18030_W;
	    cc.code = ((wc_uint32)gb[0] << 24)
		    | ((wc_uint32)gb[1] << 16)
		    | ((wc_uint32)gb[2] << 8)
		    | c;
#ifdef USE_UNICODE
	    if (WcOption.gb18030_as_ucs &&
		(ucs = wc_gb18030_to_ucs(cc)) != WC_C_UCS4_ERROR)
		wtf_push(os, WC_CCS_GB18030 | (wc_ucs_to_ccs(ucs) & ~WC_CCS_A_SET), cc.code);
	    else
#endif
	        wtf_push(os, cc.ccs, cc.code);
	}
	break;
    }
    st->state = -1;
    return os;
}
