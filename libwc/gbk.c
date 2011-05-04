
#include "wc.h"
#include "gbk.h"
#include "search.h"
#include "wtf.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif

#include "map/gb2312_gbk.map"

#define C0 WC_GBK_MAP_C0
#define GL WC_GBK_MAP_GL
#define C1 WC_GBK_MAP_C1
#define LB WC_GBK_MAP_LB
#define UB WC_GBK_MAP_UB
#define C80 WC_GBK_MAP_80

wc_uint8 WC_GBK_MAP[ 0x100 ] = {
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, C0,

    C80,UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, C1,
};

wc_ccs
wc_gb2312_or_gbk(wc_uint16 code) {
    return wc_map_range_search(code,
	gb2312_gbk_map, N_gb2312_gbk_map)
	? WC_CCS_GBK : WC_CCS_GB_2312;
}

wc_wchar_t
wc_gbk_to_cs128w(wc_wchar_t cc)
{
    cc.code = WC_GBK_N(cc.code);
    if (cc.code < 0x4000)
	cc.ccs = WC_CCS_GBK_1;
    else {
	cc.ccs = WC_CCS_GBK_2;
	cc.code -= 0x4000;
    }
    cc.code = WC_N_CS128W(cc.code);
    return cc;
}

wc_wchar_t
wc_cs128w_to_gbk(wc_wchar_t cc)
{
    cc.code = WC_CS128W_N(cc.code);
    if (cc.ccs == WC_CCS_GBK_2)
	cc.code += 0x4000;
    cc.ccs = WC_CCS_GBK;
    cc.code = WC_N_GBK(cc.code);
    return cc;
}

wc_uint32
wc_gbk_to_N(wc_uint32 c)
{
    if (c <= 0xA1A0)	/* 0x8140 - 0xA1A0 */
	return WC_GBK_N(c);
    if (c <= 0xA2AA)	/* 0xA240 - 0xA2A0, 0xA2A1 - 0xA2AA */
	return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E;
    if (c <= 0xA6A0)	/* 0xA240 - 0xA6A0 */
	return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E + 0x0A;
    if (c <= 0xA6F5)	/* 0xA6E0 - 0xA6F5 */
	return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E + 0x0A - 0x3F;
    if (c <= 0xA8A0)	/* 0xA7A0 - 0xA8A0 */
	return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E + 0x0A + 0x16;
    if (c <= 0xA8C0)	/* 0xA8BB - 0xA8C0 */
	return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E + 0x0A + 0x16 - 0x1A;
			/* 0xA940 - 0xFEA0 */
    return WC_GBK_N(c) - ((c >> 8) - 0xA1) * 0x5E + 0x0A + 0x16 + 0x06;
}

Str
wc_conv_from_gbk(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    int state = WC_GBK_NOSTATE;
    wc_uint32 gbk;

    for (p = sp; p < ep && *p < 0x80; p++) 
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, (char *)is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_GBK_NOSTATE:
	    switch (WC_GBK_MAP[*p]) {
	    case UB:
		state = WC_GBK_MBYTE1;
		break;
	    case C80:
		wtf_push(os, WC_CCS_GBK_80, *p);
		break;
	    case C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_GBK_MBYTE1:
	    if (WC_GBK_MAP[*p] & LB) {
		gbk = ((wc_uint32)*(p-1) << 8) | *p;
		if (*(p-1) >= 0xA1 && *p >= 0xA1)
		    wtf_push(os, wc_gb2312_or_gbk(gbk), gbk);
		else
		    wtf_push(os, WC_CCS_GBK, gbk);
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_GBK_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_GBK_MBYTE1:
	wtf_push_unknown(os, p-1, 1);
	break;
    }
    return os;
}

void
wc_push_to_gbk(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_GB_2312:
	Strcat_char(os, (char)((cc.code >> 8) | 0x80));
	Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
	return;
    case WC_CCS_GBK_80:
	Strcat_char(os, (char)(cc.code | 0x80));
	return;
    case WC_CCS_GBK_1:
    case WC_CCS_GBK_2:
	cc = wc_cs128w_to_gbk(cc);
    case WC_CCS_GBK:
	Strcat_char(os, (char)(cc.code >> 8));
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
wc_char_conv_from_gbk(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar gbku;
    wc_uint32 gbk;

    if (st->state == -1) {
	st->state = WC_GBK_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_GBK_NOSTATE:
	switch (WC_GBK_MAP[c]) {
	case UB:
	    gbku = c;
	    st->state = WC_GBK_MBYTE1;
	    return NULL;
	case C80:
	    wtf_push(os, WC_CCS_GBK_80, c);
	    break;
	case C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_GBK_MBYTE1:
	if (WC_GBK_MAP[c] & LB) {
	    gbk = ((wc_uint32)gbku << 8) | c;
	    if (gbku >= 0xA1 && c >= 0xA1)
		wtf_push(os, wc_gb2312_or_gbk(gbk), gbk);
	    else
		wtf_push(os, WC_CCS_GBK, gbk);
	}
	break;
    }
    st->state = -1;
    return os;
}
