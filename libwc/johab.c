
#include "wc.h"
#include "johab.h"
#include "wtf.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif

#define C0 WC_JOHAB_MAP_C0
#define GL WC_JOHAB_MAP_GL
#define C1 WC_JOHAB_MAP_C1
#define GH WC_JOHAB_MAP_GH
#define GB WC_JOHAB_MAP_GB
#define JJ WC_JOHAB_MAP_JJ
#define JB WC_JOHAB_MAP_JB
#define HB WC_JOHAB_MAP_HB
#define CJ WC_JOHAB_MAP_CJ
#define CB WC_JOHAB_MAP_CB

/*
  00-1F 20-30 31-40 41-7E 7F 80 81-83 84-90 91-D3 D4-D7 D8-DE DF E0-F9 FA-FE FF
  C0    GL    GL    GL    C0 -  -     J     J     -     H     -  H     -     -
  -     -     J     B     -  -  J     J     B     B     B     B  B     B     -

  C0    GL    GH    GB    C0 C1 CJ    JJ    JB    CB    HB    CB HB    CB    C1 
*/

wc_uint8 WC_JOHAB_MAP[ 0x100 ] = {
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
/*  20 */
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
/*  30  31 */
    GL, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH, GH,
/*  40  41 */
    GH, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, 
    GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, 
    GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, 
    GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, GB, C0,

/*  80          83  84 */
    C1, CJ, CJ, CJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ, JJ,
/*  90  91 */
    JJ, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, 
    JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, 
    JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, 
    JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, JB, 
/*              D3  D4          D7  D8                          DF */
    JB, JB, JB, JB, CB, CB, CB, CB, HB, HB, HB, HB, HB, HB, HB, CB, 
    HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, HB,
/*                                      F9  FA              FE  FF */
    HB, HB, HB, HB, HB, HB, HB, HB, HB, HB, CB, CB, CB, CB, CB, C1,
};

static wc_uint8 johab1_N_map[ 3 ][ 32 ] = {
  { 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
   15,16,17,18,19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 1, 2, 3, 4, 5, 0, 0, 6, 7, 8, 9,10,11,
    0, 0,12,13,14,15,16,17, 0, 0,18,19,20,21, 0, 0 },
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
   16,17, 0,18,19,20,21,22,23,24,25,26,27,28, 0, 0 }
};

static wc_uint8 N_johab1_map[ 3 ][ 32 ] = {
  { 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,
   18,19,20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 3, 4, 5, 6, 7,10,11,12,13,14,15,18,19,20,21,22,
   23,26,27,28,29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
   17,19,20,21,22,23,24,25,26,27,28,29, 0, 0, 0, 0 }
};

wc_wchar_t
wc_johab_to_ksx1001(wc_wchar_t cc)
{
#ifdef USE_UNICODE
    static wc_table *t = NULL;
#endif

    switch (cc.ccs) {
    case WC_CCS_JOHAB:
	return wc_johab_to_ksx1001(wc_johab_to_cs128w(cc));
    case WC_CCS_JOHAB_1:
    case WC_CCS_JOHAB_2:
#ifdef USE_UNICODE
	if (WcOption.ucs_conv) {
	    if (t == NULL)
		t = wc_get_ucs_table(WC_CCS_KS_X_1001);
	    cc = wc_any_to_any(cc, t);
	} else
#endif
	    cc.ccs = WC_CCS_UNKNOWN_W;
	break;
    case WC_CCS_JOHAB_3:
	if (cc.code >= 0x2121)
	    cc.ccs = WC_CCS_KS_X_1001;
	else
	    cc.ccs = WC_CCS_UNKNOWN_W;
	break;
    }
    return cc;
}

wc_wchar_t
wc_ksx1001_to_johab(wc_wchar_t cc)
{
    cc.code &= 0x7f7f;
    if ((cc.code >= 0x2121 && cc.code <  0x2421) ||
	(cc.code >  0x2453 && cc.code <= 0x2C7E) ||
	(cc.code >= 0x4A21 && cc.code <= 0x7D7E)) {
	cc.ccs = WC_CCS_JOHAB_3;
	return cc;
    }
#ifdef USE_UNICODE
    if (WcOption.ucs_conv)
	cc = wc_ucs_to_johab(wc_any_to_ucs(cc));
    else
#endif
	cc.ccs = WC_CCS_UNKNOWN_W;
    return cc;
}

#ifdef USE_UNICODE
wc_wchar_t
wc_ucs_to_johab(wc_uint32 ucs)
{
    wc_table *t;
    wc_wchar_t cc;

    if (ucs >= WC_C_UCS2_HANGUL && ucs <= WC_C_UCS2_HANGUL_END) {
	ucs -= WC_C_UCS2_HANGUL;
	cc.code = WC_N_JOHAB1(ucs);
	cc.ccs = WC_CCS_JOHAB;
    } else if (ucs >= 0x3131 && ucs <= 0x3163) {
	t = wc_get_ucs_table(WC_CCS_JOHAB_2);
	cc = wc_ucs_to_any(ucs, t);
    } else {
	t = wc_get_ucs_table(WC_CCS_JOHAB_3);
	cc = wc_ucs_to_any(ucs, t);
    }
    return cc;
}
#endif

wc_uint32
wc_johab1_to_N(wc_uint32 code)
{
    wc_uint32 a, b, c;

    a = johab1_N_map[0][(code >> 10) & 0x1F];
    b = johab1_N_map[1][(code >> 5)  & 0x1F];
    c = johab1_N_map[2][ code        & 0x1F];
    if (a && b && c)
	return ((a - 1) * 21 + (b - 1)) * 28 + (c - 1);
    return WC_C_JOHAB_ERROR;
}

wc_uint32
wc_N_to_johab1(wc_uint32 code)
{
    wc_uint32 a, b, c;

    a = N_johab1_map[0][(code / 28) / 21];
    b = N_johab1_map[1][(code / 28) % 21];
    c = N_johab1_map[2][ code % 28      ];
    return 0x8000 | (a << 10) | (b << 5) | c;
}

/* 0x1F21 - 0x2C7E, 0x4A21 - 0x7C7E
  (0x1F21 - 0x207E are not in KS X 1001) */
#define johab3_to_ksx1001(ub, lb) \
{ \
    if (ub < 0xe0) { \
	ub = ((ub - 0xd8) << 1) + 0x1f; \
    } else { \
	ub = ((ub - 0xe0) << 1) + 0x4a; \
    } \
    if (lb < 0xa1) { \
	lb -= (lb < 0x91) ? 0x10 : 0x22; \
    } else { \
	ub++; \
	lb -= 0x80; \
    } \
}

#define ksx1001_to_johab3(ub, lb) \
{ \
    if (ub < 0x4a) { \
	ub -= 0x1f; \
	lb += (ub & 0x1) ? 0x80 : ((lb < 0x6f) ? 0x10 : 0x22); \
	ub = (ub >> 1) + 0xd8; \
    } else { \
	ub -= 0x4a; \
	lb += (ub & 0x1) ? 0x80 : ((lb < 0x6f) ? 0x10 : 0x22); \
	ub = (ub >> 1) + 0xe0; \
    } \
}

wc_wchar_t
wc_johab_to_cs128w(wc_wchar_t cc)
{
    wc_uint32 n;
    wc_uchar ub, lb;

    if (cc.code < 0xD800) {
	n = WC_JOHAB1_N(cc.code);
	if (n != WC_C_JOHAB_ERROR) {
	    cc.code = WC_N_CS94x128(n);
	    cc.ccs = WC_CCS_JOHAB_1;
	} else {
	    n = WC_JOHAB2_N(cc.code);
	    cc.code = WC_N_CS128W(n);
	    cc.ccs = WC_CCS_JOHAB_2;
	}
    } else {
	ub = cc.code >> 8;
	lb = cc.code & 0xff;
	johab3_to_ksx1001(ub, lb);
	cc.code = ((wc_uint32)ub << 8) | lb;
	cc.ccs = WC_CCS_JOHAB_3;
    }
    return cc;
}

wc_wchar_t
wc_cs128w_to_johab(wc_wchar_t cc)
{
    wc_uint32 n;
    wc_uchar ub, lb;

    switch (cc.ccs) {
    case WC_CCS_JOHAB_1:
	n = WC_CS94x128_N(cc.code);
	cc.code = WC_N_JOHAB1(n);
	break;
    case WC_CCS_JOHAB_2:
	n = WC_CS128W_N(cc.code);
	cc.code = WC_N_JOHAB2(n);
	break;
    case WC_CCS_JOHAB_3:
	ub = (cc.code >> 8) & 0x7f;
	lb = cc.code & 0x7f;
	ksx1001_to_johab3(ub, lb);
	cc.code = ((wc_uint32)ub << 8) | lb;
    }
    cc.ccs = WC_CCS_JOHAB;
    return cc;
}

Str
wc_conv_from_johab(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    int state = WC_JOHAB_NOSTATE;

    for (p = sp; p < ep && *p < 0x80; p++)
        ;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_JOHAB_NOSTATE:
	    switch (WC_JOHAB_MAP[*p] & WC_JOHAB_MAP_1) {
	    case WC_JOHAB_MAP_UJ:
		state = WC_JOHAB_HANGUL1;
		break;
	    case WC_JOHAB_MAP_UH:
		state = WC_JOHAB_HANJA1;
		break;
	    case WC_JOHAB_MAP_C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_JOHAB_HANGUL1:
	    if (WC_JOHAB_MAP[*p] & WC_JOHAB_MAP_LJ) 
		wtf_push(os, WC_CCS_JOHAB, ((wc_uint32)*(p-1) << 8) | *p);
	    else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_JOHAB_NOSTATE;
	    break;
	case WC_JOHAB_HANJA1:
	    if (WC_JOHAB_MAP[*p] & WC_JOHAB_MAP_LH)
		wtf_push(os, WC_CCS_JOHAB, ((wc_uint32)*(p-1) << 8) | *p);
	    else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_JOHAB_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_JOHAB_HANGUL1:
    case WC_JOHAB_HANJA1:
	wtf_push_unknown(os, p-1, 1);
	break;
    }
    return os;
}

void
wc_push_to_johab(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_JOHAB_1:
    case WC_CCS_JOHAB_2:
    case WC_CCS_JOHAB_3:
	cc = wc_cs128w_to_johab(cc);
    case WC_CCS_JOHAB:
	Strcat_char(os, (char)(cc.code >> 8));
	Strcat_char(os, (char)(cc.code & 0xff));
	return;
    case WC_CCS_KS_X_1001:
	cc = wc_ksx1001_to_johab(cc);
	continue;
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
wc_char_conv_from_johab(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar johabu;

    if (st->state == -1) {
	st->state = WC_JOHAB_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_JOHAB_NOSTATE:
	switch (WC_JOHAB_MAP[c] & WC_JOHAB_MAP_1) {
	case WC_JOHAB_MAP_UJ:
	    johabu = c;
	    st->state = WC_JOHAB_HANGUL1;
	    return NULL;
	case WC_JOHAB_MAP_UH:
	    johabu = c;
	    st->state = WC_JOHAB_HANJA1;
	    return NULL;
	case WC_JOHAB_MAP_C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_JOHAB_HANGUL1:
	if (WC_JOHAB_MAP[c] & WC_JOHAB_MAP_LJ)
	    wtf_push(os, WC_CCS_JOHAB, ((wc_uint32)johabu << 8) | c);
	break;
    case WC_JOHAB_HANJA1:
	if (WC_JOHAB_MAP[c] & WC_JOHAB_MAP_LH)
	    wtf_push(os, WC_CCS_JOHAB, ((wc_uint32)johabu << 8) | c);
	break;
    }
    st->state = -1;
    return os;
}
