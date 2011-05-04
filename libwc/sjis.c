
#include "wc.h"
#include "sjis.h"
#include "jis.h"
#include "wtf.h"
#include "ucs.h"

#include "map/jisx02132_sjis.map"
wc_uchar *wc_jisx0212_jisx02132_map = jisx02132_sjis_map;

#define C0  WC_SJIS_MAP_C0
#define GL  WC_SJIS_MAP_GL
#define LB  WC_SJIS_MAP_LB
#define S80 WC_SJIS_MAP_80
#define SK  WC_SJIS_MAP_SK
#define SL  WC_SJIS_MAP_SL
#define SH  WC_SJIS_MAP_SH
#define SX  WC_SJIS_MAP_SX
#define C1  WC_SJIS_MAP_C1
#define SA0 WC_SJIS_MAP_A0

wc_uint8 WC_SJIS_MAP[ 0x100 ] = {
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, 
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, 
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, 
    LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, LB, C0,

    S80,SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL,
    SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL, SL,
    SA0,SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK,
    SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK,
    SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK,
    SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK, SK,
    SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH, SH,
    SX, SX, SX, SX, SX, SX, SX, SX, SX, SX, SX, SX, SX, C1, C1, C1,
};

#define sjis_to_jisx0208(ub, lb) \
{ \
    ub -= (ub < 0xa0) ? 0x81 : 0xc1; \
    ub = (ub << 1) + 0x21; \
    if (lb < 0x9f) { \
	lb -= (lb > 0x7e) ? 0x20 : 0x1f; \
    } else { \
	ub++; \
	lb -= 0x7e; \
    } \
}
#define sjis_to_jisx02132(ub, lb) \
{ \
    if (lb < 0x9f) { \
	ub = sjis1_jisx02132_map[ub - 0xf0]; \
	lb -= (lb > 0x7e) ? 0x20 : 0x1f; \
    } else { \
	ub = sjis2_jisx02132_map[ub - 0xf0]; \
	lb -= 0x7e; \
    } \
}
#define jisx0208_to_sjis(ub, lb) \
{ \
    lb += (ub & 1) ? 0x1f : 0x7d; \
    if (lb > 0x7e) \
	lb++; \
    ub = (ub - 0x21) >> 1; \
    ub += (ub < 0x1f) ? 0x81 : 0xc1; \
}
#define jisx02132_to_sjis(ub, lb) \
{ \
    lb += (ub & 1) ? 0x1f : 0x7d; \
    if (lb > 0x7e) \
	lb++; \
    ub = jisx02132_sjis_map[ ub ]; \
}

wc_wchar_t
wc_sjis_to_jis(wc_wchar_t cc)
{
    wc_uchar ub, lb;

    ub = cc.code >> 8;
    lb = cc.code & 0xff;
    if (ub < 0xf0) {
	sjis_to_jisx0208(ub, lb);
	cc.ccs = WC_CCS_JIS_X_0208;
    } else {
	sjis_to_jisx02132(ub, lb);
	cc.ccs = WC_CCS_JIS_X_0213_2;
    }
    cc.code = ((wc_uint32)ub << 8) | lb;
    return cc;
}

wc_wchar_t
wc_jis_to_sjis(wc_wchar_t cc)
{
    wc_uchar ub, lb;

    ub = (cc.code >> 8) & 0x7f;
    lb = cc.code & 0x7f;
    if (cc.ccs == WC_CCS_JIS_X_0213_2) {
	jisx02132_to_sjis(ub, lb);
	if (! ub) {
	    cc.ccs = WC_CCS_UNKNOWN_W;
	    return cc;
	}
    } else {
	jisx0208_to_sjis(ub, lb);
    }
    cc.code = ((wc_uint32)ub << 8) | lb;
    return cc;
}

wc_wchar_t
wc_sjis_ext_to_cs94w(wc_wchar_t cc)
{
    wc_uchar ub, lb;

    ub = cc.code >> 8;
    lb = cc.code & 0xff;
    sjis_to_jisx0208(ub, lb);
    if (ub <= 0x7e) {
	cc.ccs = WC_CCS_SJIS_EXT_1;
    } else {
	ub -= 0x5e;
	cc.ccs = WC_CCS_SJIS_EXT_2;
    }
    cc.code = ((wc_uint32)ub << 8) | lb;
    return cc;
}

wc_wchar_t
wc_cs94w_to_sjis_ext(wc_wchar_t cc)
{
    wc_uchar ub, lb;

    ub = (cc.code >> 8) & 0x7f;
    lb = cc.code & 0x7f;
    if (cc.ccs == WC_CCS_SJIS_EXT_2)
	ub += 0x5e;
    jisx0208_to_sjis(ub, lb);
    cc.ccs = WC_CCS_SJIS_EXT;
    cc.code = ((wc_uint32)ub << 8) | lb;
    return cc;
}

wc_uint32
wc_sjis_ext1_to_N(wc_uint32 c)
{
    wc_uchar ub;

    ub = (c >> 8) & 0x7f;
    switch(ub) {
    case 0x2D:	/* 0x8740 - */
	ub = 0;
	break;
    case 0x79:	/* 0xED40 - */
    case 0x7A:	/* 0xED9F - */
    case 0x7B:	/* 0xEE40 - */
    case 0x7C:	/* 0xEE9F - */
	ub -= 0x78;
	break;
    default:
	return WC_C_SJIS_ERROR;
    }
    return ub * 0x5e + (c & 0x7f) - 0x21;
}

wc_uint32
wc_sjis_ext2_to_N(wc_uint32 c)
{
    wc_uchar ub;

    ub = (c >> 8) & 0x7f;
    switch(ub) {
    case 0x35:	/* 0xFA40 - */
    case 0x36:	/* 0xFA9F - */
    case 0x37:	/* 0xFB40 - */
    case 0x38:	/* 0xFB9F - */
    case 0x39:	/* 0xFC40 - */
	ub -= 0x30;
	break;
    default:
	return WC_C_SJIS_ERROR;
    }
    return ub * 0x5e + (c & 0x7f) - 0x21;
}

Str
wc_conv_from_sjis(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    wc_uchar jis[2];
    int state = WC_SJIS_NOSTATE;
    wc_wchar_t cc;

    for (p = sp; p < ep && *p < 0x80; p++)
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_SJIS_NOSTATE:
	    switch (WC_SJIS_MAP[*p]) {
	    case SL:
		state = WC_SJIS_SHIFT_L;
		break;
	    case SH:
		state = WC_SJIS_SHIFT_H;
		break;
	    case SX:
		state = WC_SJIS_SHIFT_X;
		break;
	    case SK:
		wtf_push(os, WC_CCS_JIS_X_0201K, (wc_uint32)*p);
		break;
	    case S80:
	    case SA0:
	    case C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_SJIS_SHIFT_L:
	case WC_SJIS_SHIFT_H:
	    if (WC_SJIS_MAP[*p] & LB) {
		jis[0] = *(p-1);
		jis[1] = *p;
		sjis_to_jisx0208(jis[0], jis[1]);
		cc.code = ((wc_uint32)jis[0] << 8) | jis[1];
		cc.ccs = wc_jisx0208_or_jisx02131(cc.code);
		if (cc.ccs == WC_CCS_JIS_X_0208)
		    wtf_push(os, cc.ccs, cc.code);
		else
		    wtf_push(os, WC_CCS_SJIS_EXT, ((wc_uint32)*(p-1) << 8) | *p);
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_SJIS_NOSTATE;
	    break;
	case WC_SJIS_SHIFT_X:
	    if (WC_SJIS_MAP[*p] & LB)
		wtf_push(os, WC_CCS_SJIS_EXT, ((wc_uint32)*(p-1) << 8) | *p);
	    else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_SJIS_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_SJIS_SHIFT_L:
    case WC_SJIS_SHIFT_H:
    case WC_SJIS_SHIFT_X:
	wtf_push_unknown(os, p-1, 1);
	break;
    }
    return os;
}

Str
wc_conv_from_sjisx0213(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    wc_uchar jis[2];
    int state = WC_SJIS_NOSTATE;
    wc_wchar_t cc;

    for (p = sp; p < ep && *p < 0x80; p++)
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, is->ptr, (int)(p - sp));

    for (; p < ep; p++) {
	switch (state) {
	case WC_SJIS_NOSTATE:
	    switch (WC_SJIS_MAP[*p]) {
	    case SL:
		state = WC_SJIS_SHIFT_L;
		break;
	    case SH:
		state = WC_SJIS_SHIFT_H;
		break;
	    case SX:
		state = WC_SJIS_SHIFT_X;
		break;
	    case SK:
		wtf_push(os, WC_CCS_JIS_X_0201K, (wc_uint32)*p);
		break;
	    case S80:
	    case SA0:
	    case C1:
		wtf_push_unknown(os, p, 1);
		break;
	    default:
		Strcat_char(os, (char)*p);
		break;
	    }
	    break;
	case WC_SJIS_SHIFT_L:
	case WC_SJIS_SHIFT_H:
	    if (WC_SJIS_MAP[*p] & LB) {
		jis[0] = *(p-1);
		jis[1] = *p;
		sjis_to_jisx0208(jis[0], jis[1]);
		cc.code = ((wc_uint32)jis[0] << 8) | jis[1];
		cc.ccs = wc_jisx0208_or_jisx02131(cc.code);
		wtf_push(os, cc.ccs, cc.code);
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_SJIS_NOSTATE;
	    break;
	case WC_SJIS_SHIFT_X:
	    if (WC_SJIS_MAP[*p] & LB) {
		jis[0] = *(p-1);
		jis[1] = *p;
		sjis_to_jisx02132(jis[0], jis[1]);
		wtf_push(os, WC_CCS_JIS_X_0213_2, ((wc_uint32)jis[0] << 8) | jis[1]);
	    } else
		wtf_push_unknown(os, p-1, 2);
	    state = WC_SJIS_NOSTATE;
	    break;
	}
    }
    switch (state) {
    case WC_SJIS_SHIFT_L:
    case WC_SJIS_SHIFT_H:
    case WC_SJIS_SHIFT_X:
	wtf_push_unknown(os, p-1, 1);
	break;
    }
    return os;
}

void
wc_push_to_sjis(Str os, wc_wchar_t cc, wc_status *st)
{
    wc_uchar ub, lb;

  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, cc.code);
	return;
    case WC_CCS_JIS_X_0201K:
	if (WcOption.use_jisx0201k) {
	    Strcat_char(os, cc.code | 0x80);
	    return;
	} else if (WcOption.fix_width_conv)
	    cc.ccs = WC_CCS_UNKNOWN;
	else
	    cc = wc_jisx0201k_to_jisx0208(cc);
	continue;
    case WC_CCS_JIS_X_0208:
	ub = (cc.code >> 8) & 0x7f;
	lb = cc.code & 0x7f;
	jisx0208_to_sjis(ub, lb);
	Strcat_char(os, ub);
	Strcat_char(os, lb);
	return;
    case WC_CCS_SJIS_EXT_1:
    case WC_CCS_SJIS_EXT_2:
	cc = wc_cs94w_to_sjis_ext(cc);
    case WC_CCS_SJIS_EXT:
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

void
wc_push_to_sjisx0213(Str os, wc_wchar_t cc, wc_status *st)
{
    wc_uchar ub, lb;

  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, cc.code);
	return;
    case WC_CCS_JIS_X_0201K:
	if (WcOption.use_jisx0201k) {
	    Strcat_char(os, cc.code | 0x80);
	    return;
	} else if (WcOption.fix_width_conv)
	    cc.ccs = WC_CCS_UNKNOWN;
	else
	    cc = wc_jisx0201k_to_jisx0208(cc);
	continue;
    case WC_CCS_JIS_X_0213_1:
	if (! WcOption.use_jisx0213) {
	    cc.ccs = WC_CCS_UNKNOWN_W;
	    continue;
	}
    case WC_CCS_JIS_X_0208:
	ub = (cc.code >> 8) & 0x7f;
	lb = cc.code & 0x7f;
	jisx0208_to_sjis(ub, lb);
	Strcat_char(os, ub);
	Strcat_char(os, lb);
	return;
    case WC_CCS_JIS_X_0213_2:
	if (! WcOption.use_jisx0213) {
	    cc.ccs = WC_CCS_UNKNOWN_W;
	    continue;
	}
	ub = (cc.code >> 8) & 0x7f;
	lb = cc.code & 0x7f;
	jisx02132_to_sjis(ub, lb);
	if (ub) {
	    Strcat_char(os, ub);
	    Strcat_char(os, lb);
	    return;
	}
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
wc_char_conv_from_sjis(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar jis[2];
    wc_wchar_t cc;

    if (st->state == -1) {
	st->state = WC_SJIS_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_SJIS_NOSTATE:
	switch (WC_SJIS_MAP[c]) {
	case SL:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_L;
	    return NULL;
	case SH:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_H;
	    return NULL;
	case SX:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_X;
	    return NULL;
	case SK:
	    wtf_push(os, WC_CCS_JIS_X_0201K, (wc_uint32)c);
	    break;
	case S80:
	case SA0:
	case C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_SJIS_SHIFT_L:
    case WC_SJIS_SHIFT_H:
	if (WC_SJIS_MAP[c] & LB) {
	    jis[1] = c;
	    sjis_to_jisx0208(jis[0], jis[1]);
	    cc.code = ((wc_uint32)jis[0] << 8) | jis[1];
	    cc.ccs = wc_jisx0208_or_jisx02131(cc.code);
	    if (cc.ccs == WC_CCS_JIS_X_0208)
		wtf_push(os, cc.ccs, cc.code);
	    else
	        wtf_push(os, WC_CCS_SJIS_EXT, ((wc_uint32)jis[0] << 8) | jis[1]);
	}
	st->state = WC_SJIS_NOSTATE;
	break;
    case WC_SJIS_SHIFT_X:
	if (WC_SJIS_MAP[c] & LB) {
	    jis[1] = c;
	    wtf_push(os, WC_CCS_SJIS_EXT, ((wc_uint32)jis[0] << 8) | jis[1]);
	}
	st->state = WC_SJIS_NOSTATE;
	break;
    }
    st->state = -1;
    return os;
}

Str
wc_char_conv_from_sjisx0213(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar jis[2];
    wc_wchar_t cc;

    if (st->state == -1) {
	st->state = WC_SJIS_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_SJIS_NOSTATE:
	switch (WC_SJIS_MAP[c]) {
	case SL:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_L;
	    return NULL;
	case SH:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_H;
	    return NULL;
	case SX:
	    jis[0] = c;
	    st->state = WC_SJIS_SHIFT_X;
	    return NULL;
	case SK:
	    wtf_push(os, WC_CCS_JIS_X_0201K, (wc_uint32)c);
	    break;
	case S80:
	case SA0:
	case C1:
	    break;
	default:
	    Strcat_char(os, (char)c);
	    break;
	}
	break;
    case WC_SJIS_SHIFT_L:
    case WC_SJIS_SHIFT_H:
	if (WC_SJIS_MAP[c] & LB) {
	    jis[1] = c;
	    sjis_to_jisx0208(jis[0], jis[1]);
	    cc.code = ((wc_uint32)jis[0] << 8) | jis[1];
	    cc.ccs = wc_jisx0208_or_jisx02131(cc.code);
	    wtf_push(os, cc.ccs, cc.code);
	}
	st->state = WC_SJIS_NOSTATE;
	break;
    case WC_SJIS_SHIFT_X:
	if (WC_SJIS_MAP[c] & LB) {
	    jis[1] = c;
	    sjis_to_jisx02132(jis[0], jis[1]);
	    wtf_push(os, WC_CCS_JIS_X_0213_2, ((wc_uint32)jis[0] << 8) | jis[1]);
	}
	st->state = WC_SJIS_NOSTATE;
	break;
    }
    st->state = -1;
    return os;
}
