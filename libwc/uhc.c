
#include "wc.h"
#include "uhc.h"
#include "wtf.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif

#define C0 WC_UHC_MAP_C0
#define GL WC_UHC_MAP_GL
#define C1 WC_UHC_MAP_C1
#define LB WC_UHC_MAP_LB
#define UB WC_UHC_MAP_UB

wc_uint8 WC_UHC_MAP[ 0x100 ] = {
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    GL, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, GL, GL, GL, GL, GL,
    GL, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, GL, GL, GL, GL, C0,

    C1, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB,
    UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, UB, C1,
};

wc_wchar_t
wc_uhc_to_cs128w(wc_wchar_t cc)
{
    cc.code = WC_UHC_N(cc.code);
    if (cc.code < 0x4000)
	cc.ccs = WC_CCS_UHC_1;
    else {
	cc.ccs = WC_CCS_UHC_2;
	cc.code -= 0x4000;
    }
    cc.code = WC_N_CS128W(cc.code);
    return cc;
}

wc_wchar_t
wc_cs128w_to_uhc(wc_wchar_t cc)
{
    cc.code = WC_CS128W_N(cc.code);
    if (cc.ccs == WC_CCS_UHC_2)
	cc.code += 0x4000;
    cc.ccs = WC_CCS_UHC;
    cc.code = WC_N_UHC(cc.code);
    return cc;
}

wc_uint32
wc_uhc_to_N(wc_uint32 c)
{
    if (c <= 0xA1A0)	/* 0x8141 - 0xA1A0 */
	return WC_UHC_N(c);
    if (c <= 0xA2A0)	/* 0xA240 - 0xA2A0 */
	return WC_UHC_N(c) - 0x5E;
    if (c <= 0xA2E7)	/* 0xA2E6 - 0xA2E7 */
	return WC_UHC_N(0xA2A0) - 0x5E + c - 0xA2E5;
			/* 0xA340 - 0xFEA0 */
    return WC_UHC_N(c) - ((c >> 8) - 0xA1) * 0x5E + 2;
}

Str
wc_conv_from_uhc(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    int state = WC_UHC_NOSTATE;
    wc_uint32 uhc;

    for (p = sp; p < ep && *p < 0x80; p++) 
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, (char *)is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_UHC_NOSTATE:
	    switch (WC_UHC_MAP[*p]) {
	    case UB:
		state = WC_UHC_MBYTE1;
		break;
	    case C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_UHC_MBYTE1:
	    if (WC_UHC_MAP[*p] & LB) {
		uhc = ((wc_uint32)*(p-1) << 8) | *p;
		if (*(p-1) >= 0xA1 && *p >= 0xA1 &&
		    uhc != 0xA2E6 && uhc != 0xA2E7)
		    wtf_push(os, WC_CCS_KS_X_1001, uhc);
		else
		    wtf_push(os, WC_CCS_UHC, uhc);
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_UHC_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_UHC_MBYTE1:
	wtf_push_unknown(os, p-1, 1);
	break;
    }
    return os;
}

void
wc_push_to_uhc(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_KS_X_1001:
	Strcat_char(os, (char)((cc.code >> 8) | 0x80));
	Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
	return;
    case WC_CCS_UHC_1:
    case WC_CCS_UHC_2:
	cc = wc_cs128w_to_uhc(cc);
    case WC_CCS_UHC:
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
wc_char_conv_from_uhc(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar uhcu;
    wc_uint32 uhc;

    if (st->state == -1) {
	st->state = WC_UHC_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_UHC_NOSTATE:
	switch (WC_UHC_MAP[c]) {
	case UB:
	    uhcu = c;
	    st->state = WC_UHC_MBYTE1;
	    return NULL;
	case C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_UHC_MBYTE1:
	if (WC_UHC_MAP[c] & LB) {
	    uhc = ((wc_uint32)uhcu << 8) | c;
	    if (uhcu >= 0xA1 && c >= 0xA1 &&
		uhc != 0xA2E6 && uhc != 0xA2E7)
		wtf_push(os, WC_CCS_KS_X_1001, uhc);
	    else
		wtf_push(os, WC_CCS_UHC, uhc);
	}
	break;
    }
    st->state = -1;
    return os;
}
