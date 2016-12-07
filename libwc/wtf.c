
#include "wc.h"
#include "wtf.h"
#include "sjis.h"
#include "big5.h"
#include "hkscs.h"
#include "johab.h"
#include "jis.h"
#include "viet.h"
#include "gbk.h"
#include "gb18030.h"
#include "uhc.h"
#ifdef USE_UNICODE
#include "ucs.h"
#include "utf8.h"
#endif

wc_uint8 WTF_WIDTH_MAP[ 0x100 ] = {
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

    1,2,1,2,1,1,1,2, 1,2,1,2,1,1,1,1, 0,0,0,0,0,0,0,0, 0,0,0,0,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
};

wc_uint8 WTF_LEN_MAP[ 0x100 ] = {
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

    3,4,3,4,3,3,3,4, 4,4,6,6,1,1,1,1, 3,4,3,4,3,3,3,4, 4,4,6,6,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
};

wc_uint8 WTF_TYPE_MAP[ 0x100 ] = {
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,

    2,  0xA,2,  0xA, 2, 0x12,2,  0xA, 2,  0xA,2,  0xA, 0x20,0x20,0x20,0x20,
    4,  0xC,4,  0xC, 4, 0x20,4,  0xC, 4,  0xC,4,  0xC, 0x20,0x20,0x20,0x20,
 0x20,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
};

static wc_uint16 CCS_MAP[ 33 ] = {
    WC_CCS_A_CS94    >> 8, WC_CCS_A_CS94W    >> 8,
    WC_CCS_A_CS96    >> 8, WC_CCS_A_CS96W    >> 8,
    WC_CCS_A_CS942   >> 8, WC_CCS_A_UNKNOWN  >> 8,
    WC_CCS_A_PCS     >> 8, WC_CCS_A_PCSW     >> 8,
    WC_CCS_A_WCS16   >> 8, WC_CCS_A_WCS16W   >> 8,
    WC_CCS_A_WCS32   >> 8, WC_CCS_A_WCS32W   >> 8,
    0,                     0,
    0,                     0,
    WC_CCS_A_CS94_C  >> 8, WC_CCS_A_CS94W_C  >> 8,
    WC_CCS_A_CS96_C  >> 8, WC_CCS_A_CS96W_C  >> 8,
    WC_CCS_A_CS942_C >> 8, 0,
    WC_CCS_A_PCS_C   >> 8, WC_CCS_A_PCSW_C   >> 8,
    WC_CCS_A_WCS16_C >> 8, WC_CCS_A_WCS16W_C >> 8,
    WC_CCS_A_WCS32_C >> 8, WC_CCS_A_WCS32W_C >> 8,
    0,                     0,
    0,                     0,
    0,
};

wc_ccs wtf_gr_ccs = 0;
static wc_ces wtf_major_ces = WC_CES_US_ASCII;
static wc_status wtf_major_st;

void
wtf_init(wc_ces ces1, wc_ces ces2)
{
    int i;
    wc_gset *gset;

    if (wc_check_ces(ces2))
	wtf_major_ces = ces2;

    if (! wc_check_ces(ces1))
	return;
    gset = WcCesInfo[WC_CES_INDEX(ces1)].gset;
    if (gset == NULL || gset[1].ccs == 0 ||
	gset[1].ccs & (WC_CCS_A_WCS16|WC_CCS_A_WCS32))
	return;
    wtf_gr_ccs = gset[1].ccs;

    if (WC_CCS_IS_WIDE(wtf_gr_ccs)) {
	for (i = 0xa1; i <= 0xff; i++) {
	    WTF_WIDTH_MAP[i] = 2;
	    WTF_LEN_MAP[i] = 2;
	    WTF_TYPE_MAP[i] = WTF_TYPE_WCHAR1W;
	}
    } else {
	for (i = 0xa1; i <= 0xff; i++) {
	    WTF_WIDTH_MAP[i] = 1;
	    WTF_LEN_MAP[i] = 1;
	    WTF_TYPE_MAP[i] = WTF_TYPE_WCHAR1;
	}
    }
}

/*
int
wtf_width(wc_uchar *p)
{
    return (int)WTF_WIDTH_MAP[*p];
}
*/

int
wtf_strwidth(wc_uchar *p)
{
    int w = 0;

    while (*p) {
	w += wtf_width(p);
	p += WTF_LEN_MAP[*p];
    }
    return w;
}

/*
size_t
wtf_len1(wc_uchar *p)
{
    return (size_t)WTF_LEN_MAP[*p];
}
*/

size_t
wtf_len(wc_uchar *p)
{
    wc_uchar *q = p;

    q += WTF_LEN_MAP[*q];
    while (*q && ! WTF_WIDTH_MAP[*q])
	q += WTF_LEN_MAP[*q];
    return q - p;
}

/*
int
wtf_type(wc_uchar *p)
{
    return (int)WTF_TYPE_MAP[*p];
}
*/

#define wcs16_to_wtf(c, p) \
    ((p)[0] = (((c) >> 14) & 0x03) | 0x80), \
    ((p)[1] = (((c) >>  7) & 0x7f) | 0x80), \
    ((p)[2] = ( (c)        & 0x7f) | 0x80)
#define wcs32_to_wtf(c, p) \
    ((p)[0] = (((c) >> 28) & 0x0f) | 0x80), \
    ((p)[1] = (((c) >> 21) & 0x7f) | 0x80), \
    ((p)[2] = (((c) >> 14) & 0x7f) | 0x80), \
    ((p)[3] = (((c) >>  7) & 0x7f) | 0x80), \
    ((p)[4] = ( (c)        & 0x7f) | 0x80)
#define wtf_to_wcs16(p) \
      ((wc_uint32)((p)[0] & 0x03) << 14) \
    | ((wc_uint32)((p)[1] & 0x7f) <<  7) \
    | ((wc_uint32)((p)[2] & 0x7f)      )
#define wtf_to_wcs32(p) \
      ((wc_uint32)((p)[0] & 0x0f) << 28) \
    | ((wc_uint32)((p)[1] & 0x7f) << 21) \
    | ((wc_uint32)((p)[2] & 0x7f) << 14) \
    | ((wc_uint32)((p)[3] & 0x7f) <<  7) \
    | ((wc_uint32)((p)[4] & 0x7f)      )

void
wtf_push(Str os, wc_ccs ccs, wc_uint32 code)
{
    wc_uchar s[8];
    wc_wchar_t cc, cc2;
    size_t n;

    if (ccs == WC_CCS_US_ASCII) {
	Strcat_char(os, (char)(code & 0x7f));
	return;
    }
    cc.ccs = ccs;
    cc.code = code;
    if (WcOption.pre_conv && !(cc.ccs & WC_CCS_A_UNKNOWN)) {
	if ((ccs == WC_CCS_JOHAB || ccs == WC_CCS_JOHAB_1 ||
		ccs == WC_CCS_JOHAB_2 || ccs == WC_CCS_JOHAB_3) &&
		(wtf_major_ces == WC_CES_EUC_KR ||
		wtf_major_ces == WC_CES_ISO_2022_KR)) {
	    cc2 = wc_johab_to_ksx1001(cc);
	    if (!WC_CCS_IS_UNKNOWN(cc2.ccs))
		cc = cc2;
	} else if (ccs == WC_CCS_KS_X_1001 &&
		wtf_major_ces == WC_CES_JOHAB) {
	    cc2 = wc_ksx1001_to_johab(cc);
	    if (!WC_CCS_IS_UNKNOWN(cc2.ccs))
		cc = cc2;
	}
#ifdef USE_UNICODE
	else if (WcOption.ucs_conv) {
	    wc_bool fix_width_conv = WcOption.fix_width_conv;
	    WcOption.fix_width_conv = WC_FALSE;
	    wc_output_init(wtf_major_ces, &wtf_major_st);
	    if (! wc_ces_has_ccs(WC_CCS_SET(ccs), &wtf_major_st)) {
		cc2 = wc_any_to_any_ces(cc, &wtf_major_st);
		if (cc2.ccs == WC_CCS_US_ASCII) {
		    Strcat_char(os, (char)(cc2.code & 0x7f));
		    return;
		}
		if (!WC_CCS_IS_UNKNOWN(cc2.ccs) &&
			cc2.ccs != WC_CCS_CP1258_2 &&
			cc2.ccs != WC_CCS_TCVN_5712_3)
		    cc = cc2;
	    }
	    WcOption.fix_width_conv = fix_width_conv;
	}
#endif
    }

    switch (WC_CCS_TYPE(cc.ccs)) {
    case WC_CCS_A_CS94:
	if (cc.ccs == wtf_gr_ccs) {
	    s[0] = (cc.code & 0x7f) | 0x80;
	    n = 1;
	    break;
	}
	if (cc.ccs == WC_CCS_JIS_X_0201K && !WcOption.use_jisx0201k) {
	    cc2 = wc_jisx0201k_to_jisx0208(cc);
	    if (!WC_CCS_IS_UNKNOWN(cc2.ccs)) {
		wtf_push(os, cc2.ccs, cc2.code);
		return;
	    }
	}
	s[0] = WTF_C_CS94;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = (cc.code & 0x7f) | 0x80;
	n = 3;
	break;
    case WC_CCS_A_CS94W:
	if (cc.ccs == wtf_gr_ccs) {
	    s[0] = ((cc.code >> 8) & 0x7f) | 0x80;
	    s[1] = ( cc.code       & 0x7f) | 0x80;
	    n = 2;
	    break;
	}
	s[0] = WTF_C_CS94W;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = ((cc.code >> 8) & 0x7f) | 0x80;
	s[3] = ( cc.code       & 0x7f) | 0x80;
	n = 4;
	break;
    case WC_CCS_A_CS96:
	if (WcOption.use_combining && wc_is_combining(cc))
	    s[0] = WTF_C_CS96_C;
	else if (cc.ccs == wtf_gr_ccs && (cc.code & 0x7f) > 0x20) {
	    s[0] = (cc.code & 0x7f) | 0x80;
	    n = 1;
	    break;
	} else
	    s[0] = WTF_C_CS96;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = (cc.code & 0x7f) | 0x80;
	n = 3;
	break;
    case WC_CCS_A_CS96W:
	if (cc.ccs == wtf_gr_ccs && ((cc.code >> 8) & 0x7f) > 0x20) {
	    s[0] = ((cc.code >> 8) & 0x7f) | 0x80;
	    s[1] = ( cc.code       & 0x7f) | 0x80;
	    n = 2;
	    break;
	}
	s[0] = WTF_C_CS96W;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = ((cc.code >> 8) & 0x7f) | 0x80;
	s[3] = ( cc.code       & 0x7f) | 0x80;
	n = 4;
	break;
    case WC_CCS_A_CS942:
	if (cc.ccs == wtf_gr_ccs) {
	    s[0] = (cc.code & 0x7f) | 0x80;
	    n = 1;
	    break;
	}
	s[0] = WTF_C_CS942;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = (cc.code & 0x7f) | 0x80;
	n = 3;
	break;
    case WC_CCS_A_PCS:
	if (WcOption.use_combining && wc_is_combining(cc))
	    s[0] = WTF_C_PCS_C;
	else if (cc.ccs == wtf_gr_ccs && (cc.code & 0x7f) > 0x20) {
	    s[0] = (cc.code & 0x7f) | 0x80;
	    n = 1;
	    break;
	} else
	    s[0] = WTF_C_PCS;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = (cc.code & 0x7f) | 0x80;
	n = 3;
	break;
    case WC_CCS_A_PCSW:
	switch (cc.ccs) {
	case WC_CCS_SJIS_EXT:
	    cc = wc_sjis_ext_to_cs94w(cc);
	    break;
	case WC_CCS_GBK:
	    cc = wc_gbk_to_cs128w(cc);
	    break;
	case WC_CCS_GBK_EXT:
	    cc = wc_gbk_ext_to_cs128w(cc);
	    break;
	case WC_CCS_BIG5:
	    cc = wc_big5_to_cs94w(cc);
	    break;
	case WC_CCS_HKSCS:
	    cc = wc_hkscs_to_cs128w(cc);
	    break;
	case WC_CCS_JOHAB:
	    cc = wc_johab_to_cs128w(cc);
	    break;
	case WC_CCS_UHC:
	    cc = wc_uhc_to_cs128w(cc);
	    break;
	}
	if (cc.ccs == wtf_gr_ccs && ((cc.code >> 8) & 0x7f) > 0x20) {
	    s[0] = ((cc.code >> 8) & 0x7f) | 0x80;
	    s[1] = ( cc.code       & 0x7f) | 0x80;
	    n = 2;
	    break;
	}
	s[0] = WTF_C_PCSW;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = ((cc.code >> 8) & 0x7f) | 0x80;
	s[3] = ( cc.code       & 0x7f) | 0x80;
	n = 4;
	break;
    case WC_CCS_A_WCS16:
	s[0] = (WC_CCS_IS_WIDE(cc.ccs) ? WTF_C_WCS16W : WTF_C_WCS16)
	     | (WC_CCS_IS_COMB(cc.ccs) ? WTF_C_COMB : 0);
	wcs16_to_wtf(cc.code, s + 1);
	s[1] |= (WC_CCS_INDEX(cc.ccs) << 2);
	n = 4;
	break;
    case WC_CCS_A_WCS32:
	s[0] = (WC_CCS_IS_WIDE(cc.ccs) ? WTF_C_WCS32W : WTF_C_WCS32)
	     | (WC_CCS_IS_COMB(cc.ccs) ? WTF_C_COMB : 0);
	wcs32_to_wtf(cc.code, s + 1);
	s[1] |= (WC_CCS_INDEX(cc.ccs) << 4);
	n = 6;
	break;
    default:
	s[0] = WTF_C_UNKNOWN;
	s[1] = WC_CCS_INDEX(cc.ccs) | 0x80;
	s[2] = (cc.code & 0x7f) | 0x80;
	n = 3;
	break;
    }
    Strcat_charp_n(os, (char *)s, n);
}

void
wtf_push_unknown(Str os, wc_uchar *p, size_t len)
{
    for (; len--; p++) {
	if (*p & 0x80)
	    wtf_push(os, WC_CCS_UNKNOWN, *p);
	else
	    Strcat_char(os, (char)*p);
    }
}

wc_wchar_t
wtf_parse1(wc_uchar **p)
{
    wc_uchar *q = *p;
    wc_wchar_t cc;

    if (*q < 0x80) {
	cc.ccs = WC_CCS_US_ASCII;
	cc.code = *(q++);
    } else if (*q > 0xa0) {
	cc.ccs = wtf_gr_ccs;
	if (WC_CCS_IS_WIDE(cc.ccs)) {
	    cc.code = ((wc_uint32)*q << 8) | *(q+1);
	    q += 2;
	} else
	    cc.code = *(q++);
    } else {
	cc.ccs = (wc_uint32)CCS_MAP[*(q++) - 0x80] << 8;
	switch (WC_CCS_TYPE(cc.ccs)) {
	case WC_CCS_A_CS94:
	case WC_CCS_A_CS96:
	case WC_CCS_A_CS942:
	case WC_CCS_A_PCS:
	case WC_CCS_A_UNKNOWN:
	    cc.ccs |= *(q++) & 0x7f;
	    cc.code = *(q++);
	    break;
	case WC_CCS_A_CS94W:
	case WC_CCS_A_CS96W:
	case WC_CCS_A_PCSW:
	    cc.ccs |= *(q++) & 0x7f;
	    cc.code = ((wc_uint32)*q << 8) | *(q+1);
	    q += 2;
	    break;
	case WC_CCS_A_WCS16:
	case WC_CCS_A_WCS16W:
	    cc.ccs |= (*q & 0x7c) >> 2;
	    cc.code = wtf_to_wcs16(q);
	    q += 3;
	    break;
	case WC_CCS_A_WCS32:
	case WC_CCS_A_WCS32W:
	    cc.ccs |= (*q & 0x70) >> 4;
	    cc.code = wtf_to_wcs32(q);
	    q += 5;
	    break;
	default:
	/* case 0: */
	    cc.ccs = WC_CCS_US_ASCII;
	    cc.code = (wc_uint32)' ';
	    break;
	}
    }

    *p = q;
    switch (cc.ccs) {
    case WC_CCS_SJIS_EXT_1:
    case WC_CCS_SJIS_EXT_2:
	return wc_cs94w_to_sjis_ext(cc);
    case WC_CCS_GBK_1:
    case WC_CCS_GBK_2:
	return wc_cs128w_to_gbk(cc);
    case WC_CCS_GBK_EXT_1:
    case WC_CCS_GBK_EXT_2:
	return wc_cs128w_to_gbk_ext(cc);
    case WC_CCS_BIG5_1:
    case WC_CCS_BIG5_2:
	return wc_cs94w_to_big5(cc);
    case WC_CCS_HKSCS_1:
    case WC_CCS_HKSCS_2:
	return wc_cs128w_to_hkscs(cc);
    case WC_CCS_JOHAB_1:
    case WC_CCS_JOHAB_2:
    case WC_CCS_JOHAB_3:
	return wc_cs128w_to_johab(cc);
    case WC_CCS_UHC_1:
    case WC_CCS_UHC_2:
	return wc_cs128w_to_uhc(cc);
    }
    return cc;
}

wc_wchar_t
wtf_parse(wc_uchar **p)
{
    wc_uchar *q;
    wc_wchar_t cc, cc2;
    wc_uint32 ucs, ucs2;

    if (**p < 0x80) {
	cc.ccs = WC_CCS_US_ASCII;
	cc.code = *((*p)++);
    } else
	cc = wtf_parse1(p);
    if ((! WcOption.use_combining) || WTF_WIDTH_MAP[**p])
	return cc;

    q = *p;
    cc2 = wtf_parse1(&q);
    if ((cc.ccs == WC_CCS_US_ASCII || cc.ccs == WC_CCS_CP1258_1) &&
	WC_CCS_SET(cc2.ccs) == WC_CCS_CP1258_1) {
	cc2.code = wc_cp1258_precompose(cc.code, cc2.code);
	if (cc2.code) {
	    cc2.ccs = WC_CCS_CP1258_2;
	    *p = q;
	    return cc2;
	}
    } else if ((cc.ccs == WC_CCS_US_ASCII || cc.ccs == WC_CCS_TCVN_5712_1) &&
	WC_CCS_SET(cc2.ccs) == WC_CCS_TCVN_5712_1) {
	cc2.code = wc_tcvn5712_precompose(cc.code, cc2.code);
	if (cc2.code) {
	    cc2.ccs = WC_CCS_TCVN_5712_3;
	    *p = q;
	    return cc2;
	}
    }
#ifdef USE_UNICODE
    else if ((cc.ccs == WC_CCS_US_ASCII || cc.ccs == WC_CCS_ISO_8859_1 ||
	WC_CCS_IS_UNICODE(cc.ccs)) && WC_CCS_IS_UNICODE(cc2.ccs)) {
	while (1) {
	    ucs = (WC_CCS_SET(cc.ccs) == WC_CCS_UCS_TAG)
		? wc_ucs_tag_to_ucs(cc.code) : cc.code;
	    ucs2 = (WC_CCS_SET(cc2.ccs) == WC_CCS_UCS_TAG)
		? wc_ucs_tag_to_ucs(cc2.code) : cc2.code;
	    ucs = wc_ucs_precompose(ucs, ucs2);
	    if (ucs == WC_C_UCS4_ERROR)
		break;
	    if (WC_CCS_SET(cc.ccs) == WC_CCS_UCS_TAG)
		cc.code = wc_ucs_to_ucs_tag(ucs, wc_ucs_tag_to_tag(cc.code));
	    else {
		cc.ccs = wc_ucs_to_ccs(ucs);
		cc.code = ucs;
	    }
	    *p = q;
	    if (! WTF_WIDTH_MAP[*q])
		break;
	    cc2 = wtf_parse1(&q);
	    if (! WC_CCS_IS_UNICODE(cc2.ccs))
		break;
	}
    }
#endif
    return cc;
}

wc_ccs
wtf_get_ccs(wc_uchar *p)
{
   return wtf_parse1(&p).ccs;
}

wc_uint32
wtf_get_code(wc_uchar *p)
{
   return wtf_parse1(&p).code;
}

wc_bool
wtf_is_hangul(wc_uchar *p)
{
    if (*p > 0xa0)
	return (wtf_gr_ccs == WC_CCS_KS_X_1001 || wtf_gr_ccs == WC_CCS_JOHAB_1);
    else if (*p == WTF_C_CS94W)
	return ((*(p + 1) & 0x7f) == WC_F_KS_X_1001);
    else if (*p == WTF_C_PCSW) {
	wc_uchar f = *(p + 1) & 0x7f;
	return (f == WC_F_JOHAB_1 || f == WC_F_JOHAB_2 || f == WC_F_JOHAB_3 ||
		f == WC_F_UHC_1 || f == WC_F_UHC_2);
    }
#ifdef USE_UNICODE
    else if (*p == WTF_C_WCS16W) {
	wc_uchar f = (*(++p) & 0x7f) >> 2;
	if (f == WC_F_UCS2)
	    return wc_is_ucs_hangul(wtf_to_wcs16(p));
    } else if (*p == WTF_C_WCS32W) {
	wc_uchar f = (*(++p) & 0x7f) >> 4;
	if (f == WC_F_UCS_TAG)
	    return wc_is_ucs_hangul(wc_ucs_tag_to_ucs(wtf_to_wcs32(p)));
    }
#endif
    return WC_FALSE;
}

char *
wtf_conv_fit(char *s, wc_ces ces)
{
    wc_uchar *p;
    Str os;
    wc_wchar_t cc;
    wc_ces major_ces;
    wc_bool pre_conv, ucs_conv;

    if (ces == WC_CES_WTF || ces == WC_CES_US_ASCII)
	return s;

    for (p = (wc_uchar *)s; *p && *p < 0x80; p++)
	;
    if (! *p)
	return s;

    os = Strnew_size(strlen(s));
    if (p > (wc_uchar *)s)
	Strcopy_charp_n(os, s, (int)(p - (wc_uchar *)s));

    major_ces = wtf_major_ces;
    pre_conv = WcOption.pre_conv;
    ucs_conv = WcOption.ucs_conv;
    wtf_major_ces = ces;
    WcOption.pre_conv = WC_TRUE;
    WcOption.ucs_conv = WC_TRUE;
    while (*p) {
	cc = wtf_parse1(&p);
	wtf_push(os, cc.ccs, cc.code);
    }
    wtf_major_ces = major_ces;
    WcOption.pre_conv = pre_conv;
    WcOption.ucs_conv = ucs_conv;
    return os->ptr;
}
