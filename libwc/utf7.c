#ifdef USE_UNICODE

#include "wc.h"
#include "ucs.h"
#include "utf7.h"
#include "wtf.h"

#define SD WC_UTF7_MAP_SET_D
#define SO WC_UTF7_MAP_SET_O
#define SB WC_UTF7_MAP_SET_B
#define BB WC_UTF7_MAP_BASE64
#define BP WC_UTF7_MAP_PLUS
#define BM WC_UTF7_MAP_MINUS
#define CD (WC_UTF7_MAP_SET_D | WC_UTF7_MAP_C0)
#define CB (WC_UTF7_MAP_SET_B | WC_UTF7_MAP_C0)
#define C1 WC_UTF7_MAP_C1

wc_uint8 WC_UTF7_MAP[ 0x100 ] = {
/*                                       TAB NL          CR          */
    CB, CB, CB, CB, CB, CB, CB, CB,  CB, CD, CD, CB, CB, CD, CB, CB,
/*                                                                  */
    CB, CB, CB, CB, CB, CB, CB, CB,  CB, CB, CB, CB, CB, CB, CB, CB,
/*  SP  !   "   #   $   %   &   '    (   )   *   +   ,   -   .   /   */
    SD, SO, SO, SO, SO, SO, SO, SD,  SD, SD, SO, BP, SD, BM, SD, BB,
/*  0   1   2   3   4   5   6   7    8   9   :   ;   <   =   >   ?   */
    BB, BB, BB, BB, BB, BB, BB, BB,  BB, BB, SD, SO, SO, SO, SO, SD,
/*  @   A   B   C   D   E   F   G    H   I   J   K   L   M   N   O   */
    BB, BB, BB, BB, BB, BB, BB, BB,  BB, BB, BB, BB, BB, BB, BB, BB, 
/*  P   Q   R   S   T   U   V   W    X   Y   Z   [   \   ]   ^   _   */
    BB, BB, BB, BB, BB, BB, BB, BB,  BB, BB, BB, SO, SB, SO, SO, SO, 
/*  `   a   b   c   d   e   f   g    h   i   j   k   l   m   n   o   */
    SO, BB, BB, BB, BB, BB, BB, BB,  BB, BB, BB, BB, BB, BB, BB, BB, 
/*  p   q   r   s   t   u   v   w    x   y   z   {   |   }   ~   DEL */
    BB, BB, BB, BB, BB, BB, BB, BB,  BB, BB, BB, SO, SO, SO, SB, CB, 

    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
    C1, C1, C1, C1, C1, C1, C1, C1,  C1, C1, C1, C1, C1, C1, C1, C1,
};

static char c_base64_map[ 0x60 ] = {
    -1, -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59,  60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,   7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,  23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32,  33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48,  49, 50, 51, -1, -1, -1, -1, -1,
};

static char base64_c_map[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BASE64_C(x) base64_c_map[(x)]
#define C_BASE64(x) c_base64_map[(x) - 0x20]

Str
wc_conv_from_utf7(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p;
    int state = WC_UTF7_NOSTATE;
    wc_uint32 b, high = 0;
    wc_status st;

    for (p = sp; p < ep && *p < 0x80 && *p != WC_C_UTF7_PLUS; p++)
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length * 4 / 3);
    if (p > sp)
	Strcat_charp_n(os, is->ptr, (int)(p - sp));

    st.tag = NULL;
    st.ntag = 0;
    for (; p < ep; p++) {
	switch (state) {
	case WC_UTF7_NOSTATE:
	    if (*p == WC_C_UTF7_PLUS) {
		state = WC_UTF7_PLUS;
		st.shift = 16;
		st.base = 0;
		high = 0;
		continue;
	    }
	    break;
	case WC_UTF7_PLUS:
	    if (*p == WC_C_UTF7_MINUS)
		wtf_push_ucs(os, (wc_uint32)WC_C_UTF7_PLUS, &st);
	case WC_UTF7_BASE64:
	    switch (WC_UTF7_MAP[*p]) {
	    case BB:	/* [A-Za-z0-9/] */
	    case BP:	/* '+' */
		b = C_BASE64(*p);
		st.shift -= 6;
		if (st.shift <= 0) {
		    st.base |= b >> (- st.shift);
		    if (st.base >= WC_C_UCS2_SURROGATE &&
			st.base < WC_C_UCS2_SURROGATE_LOW) {
			if (! high)
			    high = st.base;
			else
			    high = 0;	/* error */
		    } else if (st.base >= WC_C_UCS2_SURROGATE_LOW &&
			st.base <= WC_C_UCS2_SURROGATE_END) {
			if (high)
			    wtf_push_ucs(os, wc_utf16_to_ucs(high, st.base), &st);
			/* else; */	/* error */
			high = 0;
		    } else if (st.base != WC_C_UCS2_BOM)
			wtf_push_ucs(os, st.base, &st);
		    st.shift += 16;
		    st.base = 0;
		}
		st.base |= (b << st.shift) & 0xffff;
		state = WC_UTF7_BASE64;
		continue;
	    case BM:	/* '-' */
		state = WC_UTF7_NOSTATE;
		continue;
	    }
	}
	switch (WC_UTF7_MAP[*p]) {
	case CD:
	case CB:
	    Strcat_char(os, (char)*p);
	    break;
	case C1:
	    wtf_push_unknown(os, p, 1);
	    break;
	default:
	    wtf_push_ucs(os, (wc_uint32)*p, &st);
	    break;
	}
    }
    return os;
}

static void
wc_push_ucs_to_utf7(Str os, wc_uint32 ucs, wc_status *st)
{
    if (ucs > WC_C_UNICODE_END)
	return;
    if (ucs > WC_C_UCS2_END) {
	ucs = wc_ucs_to_utf16(ucs);
	wc_push_ucs_to_utf7(os, ucs >> 16, st);
	wc_push_ucs_to_utf7(os, ucs & 0xffff, st);
	return;
    }
    if (ucs < 0x80) {
	switch (WC_UTF7_MAP[ucs]) {
	case BB:
	case BM:
	case SD:
	case CD:
	    if (st->state == WC_UTF7_BASE64) {
		Strcat_char(os, BASE64_C(st->base));
		Strcat_char(os, WC_C_UTF7_MINUS);
		st->state = WC_UTF7_NOSTATE;
	    }
	    Strcat_char(os, (char)ucs);
	    return;
	case BP:
	    if (st->state == WC_UTF7_BASE64) {
		Strcat_char(os, BASE64_C(st->base));
		Strcat_char(os, WC_C_UTF7_MINUS);
		st->state = WC_UTF7_NOSTATE;
	    }
	    Strcat_char(os, WC_C_UTF7_PLUS);
	    Strcat_char(os, WC_C_UTF7_MINUS);
	    return;
	}
    }
    if (st->state == WC_UTF7_BASE64 && st->shift) {
	st->shift += 16;
	st->base |= ucs >> st->shift;
	Strcat_char(os, BASE64_C(st->base));
    } else {
	if (st->state != WC_UTF7_BASE64) {
	    Strcat_char(os, WC_C_UTF7_PLUS);
	    st->state = WC_UTF7_BASE64;
	}
	st->shift = 16;
	st->base = 0;
    }
    st->shift -= 6;
    Strcat_char(os, BASE64_C((ucs >> st->shift) & 0x3f));
    st->shift -= 6;
    Strcat_char(os, BASE64_C((ucs >> st->shift) & 0x3f));
    if (st->shift) {
	st->shift -= 6;
	st->base = (ucs << (- st->shift)) & 0x3f;
    }
    return;
}

static int
wc_push_tag_to_utf7(Str os, int ntag, wc_status *st)
{
    char *p;

    if (ntag) {
	p = wc_ucs_get_tag(ntag);
	if (p == NULL)
	    ntag = 0;
    }
    if (ntag) {
	wc_push_ucs_to_utf7(os, WC_C_LANGUAGE_TAG, st);
	for (; *p; p++)
	    wc_push_ucs_to_utf7(os, WC_C_LANGUAGE_TAG0 | *p, st);
    } else
	wc_push_ucs_to_utf7(os, WC_C_CANCEL_TAG, st);
    return ntag;
}

void
wc_push_to_utf7(Str os, wc_wchar_t cc, wc_status *st)
{
    char *p;

  while (1) {
    switch (WC_CCS_SET(cc.ccs)) {
    case WC_CCS_UCS4:
	if (cc.code > WC_C_UNICODE_END) {
	    cc.ccs = WC_CCS_IS_WIDE(cc.ccs) ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	    continue;
	}
    case WC_CCS_US_ASCII:
    case WC_CCS_UCS2:
	if (st->ntag)
	    st->ntag = wc_push_tag_to_utf7(os, 0, st);
	wc_push_ucs_to_utf7(os, cc.code, st);
	return;
    case WC_CCS_UCS_TAG:
	if (WcOption.use_language_tag && wc_ucs_tag_to_tag(cc.code) != st->ntag)
	    st->ntag = wc_push_tag_to_utf7(os, wc_ucs_tag_to_tag(cc.code), st);
	wc_push_ucs_to_utf7(os, wc_ucs_tag_to_ucs(cc.code), st);
	return;
    case WC_CCS_ISO_8859_1:
	if (st->ntag)
	    st->ntag = wc_push_tag_to_utf7(os, 0, st);
	wc_push_ucs_to_utf7(os, cc.code | 0x80, st);
	return;
    case WC_CCS_UNKNOWN_W:
	if (!WcOption.no_replace) {
	    if (st->ntag)
	        st->ntag = wc_push_tag_to_utf7(os, 0, st);
	    for (p = WC_REPLACE_W; *p; p++)
		wc_push_ucs_to_utf7(os, (wc_uint32)*p, st);
	}
	return;
    case WC_CCS_UNKNOWN:
	if (!WcOption.no_replace) {
	    if (st->ntag)
	        st->ntag = wc_push_tag_to_utf7(os, 0, st);
	    for (p = WC_REPLACE; *p; p++)
		wc_push_ucs_to_utf7(os, (wc_uint32)*p, st);
	}
	return;
    default:
	if (WcOption.ucs_conv &&
		(cc.code = wc_any_to_ucs(cc)) != WC_C_UCS4_ERROR)
	    cc.ccs = WC_CCS_UCS2;
	else
	    cc.ccs = WC_CCS_IS_WIDE(cc.ccs) ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	continue;
    }
  }
}

void
wc_push_to_utf7_end(Str os, wc_status *st)
{
    if (st->ntag)
	st->ntag = wc_push_tag_to_utf7(os, 0, st);
    if (st->state == WC_UTF7_BASE64) {
	if (st->shift)
	    Strcat_char(os, BASE64_C(st->base));
	Strcat_char(os, WC_C_UTF7_MINUS);
    }
    return;
}

Str
wc_char_conv_from_utf7(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uint32 high;
    wc_uint32 b;

    if (st->state == -1) {
	st->state = WC_UTF7_NOSTATE;
	os = Strnew_size(8);
    }

    switch (st->state) {
    case WC_UTF7_NOSTATE:
	if (c == WC_C_UTF7_PLUS) {
	    st->state = WC_UTF7_PLUS;
	    st->shift = 16;
	    st->base = 0;
	    high = 0;
	    return NULL;
	}
	break;
    case WC_UTF7_PLUS:
	if (c == WC_C_UTF7_MINUS) {
	    wtf_push_ucs(os, (wc_uint32)WC_C_UTF7_PLUS, st);
	    st->state = -1;
	    return os;
	}
    case WC_UTF7_BASE64:
	switch (WC_UTF7_MAP[c]) {
	case BB:	/* [A-Za-z0-9/] */
	case BP:	/* '+' */
	    b = C_BASE64(c);
	    st->shift -= 6;
	    if (st->shift <= 0) {
		st->base |= b >> (- st->shift);
		if (st->base >= WC_C_UCS2_SURROGATE &&
		    st->base < WC_C_UCS2_SURROGATE_LOW) {
		    if (! high)
			high = st->base;
		    else
			high = 0;	/* error */
		} else if (st->base >= WC_C_UCS2_SURROGATE_LOW &&
		    st->base <= WC_C_UCS2_SURROGATE_END) {
		    if (high)
			wtf_push_ucs(os, wc_utf16_to_ucs(high, st->base), st);
		    /* else; */		/* error */
		    high = 0;
		} else if (st->base != WC_C_UCS2_BOM)
		    wtf_push_ucs(os, st->base, st);
		st->shift += 16;
		st->base = 0;
	    }
	    st->base |= (b << st->shift) & 0xffff;
	    st->state = WC_UTF7_BASE64;
	    return os;
	case BM:	/* '-' */
	    st->state = -1;
	    return NULL;
	}
    }
    switch (WC_UTF7_MAP[c]) {
    case CD:
    case CB:
	Strcat_char(os, (char)c);
	break;
    case C1:
	break;
    default:
	wtf_push_ucs(os, (wc_uint32)c, st);
	break;
    }
    st->state = -1;
    return os;
}

#endif


