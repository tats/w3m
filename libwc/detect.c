
#include "wc.h"
#include "iso2022.h"
#include "sjis.h"
#include "big5.h"
#include "hz.h"
#include "viet.h"
#ifdef USE_UNICODE
#include "utf8.h"
#include "utf7.h"
#endif

wc_uint8 WC_DETECT_MAP[ 0x100 ] = {
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 
};

#define DETECT_NORMAL	0
#define DETECT_POSSIBLE	1
#define DETECT_OK	2
#define DETECT_BROKEN	4
#define DETECT_ERROR	8
#define SET_DETECT(x,y) ((x) |= (y))
#define SET_BROKEN_ERROR(x) ((x) = ((x) & DETECT_BROKEN) ? DETECT_ERROR : ((x) | DETECT_BROKEN))

void
wc_create_detect_map(wc_ces ces, wc_bool esc)
{
    static wc_ces detect_ces = WC_CES_US_ASCII;
    int i;

    if (ces != detect_ces) {
	if (ces & WC_CES_T_VIET) {
	    wc_uint8 *map = NULL;
	    switch (ces) {
	    case WC_CES_TCVN_5712:
		map = wc_c0_tcvn57122_map;
		break;
	    case WC_CES_VISCII_11:
		map = wc_c0_viscii112_map;
		break;
	    case WC_CES_VPS:
		map = wc_c0_vps2_map;
		break;
	    }
	    for (i = 0; i < 0x20; i++)
		WC_DETECT_MAP[i] = map[i] ? 1 : 0;
	} else {
	    for (i = 0; i < 0x20; i++)
		WC_DETECT_MAP[i] = 0;
	    WC_DETECT_MAP[WC_C_HZ_TILDA] = (ces == WC_CES_HZ_GB_2312) ? 1 : 0;
#ifdef USE_UNICODE
	    WC_DETECT_MAP[WC_C_UTF7_PLUS] = (ces == WC_CES_UTF_7) ? 1 : 0;
#endif
	}
	detect_ces = ces;
    }
    WC_DETECT_MAP[WC_C_ESC] = (esc || (ces & WC_CES_T_ISO_2022)) ? 1 : 0;
    return;
}

wc_ces
wc_auto_detect(char *is, size_t len, wc_ces hint)
{
    wc_uchar *p = (wc_uchar *)is;
    wc_uchar *ep = p + len;
    wc_uchar *q;
    wc_ces euc = 0, priv = 0;
    wc_status st;
    int euc_state = 0, sjis_state = 0, big5_state = 0, hz_state = 0;
    int iso_detect = DETECT_ERROR, euc_detect = DETECT_ERROR,
	sjis_detect = DETECT_ERROR, big5_detect = DETECT_ERROR,
	hz_detect = DETECT_ERROR, latin_detect = DETECT_ERROR,
	priv_detect = DETECT_ERROR;
    int possible = 0;
    wc_bool iso2022jp2 = WC_FALSE, iso2022jp3 = WC_FALSE,
	iso2022cn = WC_FALSE, iso2022kr = WC_FALSE, ok = WC_FALSE;
#ifdef USE_UNICODE
    int utf8_state = 0;
    int utf8_detect = DETECT_ERROR;
    int utf8_next = 0;
#endif

    wc_create_detect_map(hint, WC_TRUE);
    for (; p < ep && ! WC_DETECT_MAP[*p]; p++)
	;
    if (p == ep)
	return hint;

    switch (hint) {
    case WC_CES_ISO_2022_JP:
    case WC_CES_ISO_2022_JP_2:
    case WC_CES_ISO_2022_JP_3:
    case WC_CES_EUC_JP:
    case WC_CES_SHIFT_JIS:
    case WC_CES_SHIFT_JISX0213:
	euc = WC_CES_EUC_JP;
	euc_state = WC_EUC_NOSTATE;
	sjis_state = WC_SJIS_NOSTATE;
	iso_detect = euc_detect = sjis_detect = DETECT_NORMAL;
	possible = 3;
	break;
    case WC_CES_ISO_2022_CN:
    case WC_CES_EUC_CN:
	euc = WC_CES_EUC_CN;
	euc_state = WC_EUC_NOSTATE;
	big5_state = WC_BIG5_NOSTATE;
	iso_detect = euc_detect = big5_detect = DETECT_NORMAL;
	possible = 3;
	break;
    case WC_CES_EUC_TW:
    case WC_CES_BIG5:
	euc = WC_CES_EUC_TW;
	euc_state = WC_EUC_NOSTATE;
	big5_state = WC_BIG5_NOSTATE;
	iso_detect = euc_detect = big5_detect = DETECT_NORMAL;
	possible = 3;
	break;
    case WC_CES_HZ_GB_2312:
	euc = WC_CES_EUC_CN;
	euc_state = WC_EUC_NOSTATE;
	hz_state = WC_HZ_NOSTATE;
	iso_detect = euc_detect = big5_detect = hz_detect = DETECT_NORMAL;
	possible = 4;
	break;
    case WC_CES_ISO_2022_KR:
    case WC_CES_EUC_KR:
	euc = WC_CES_EUC_KR;
	euc_state = WC_EUC_NOSTATE;
	iso_detect = euc_detect = DETECT_NORMAL;
	possible = 3;
	break;
#ifdef USE_UNICODE
    case WC_CES_UTF_8:
	iso_detect = DETECT_NORMAL;
	possible = 1;
	break;
#endif
    case WC_CES_US_ASCII:
	iso_detect = latin_detect = DETECT_NORMAL;
	possible = 2;
	break;
    default:
	if (hint & WC_CES_T_ISO_8859) {
	    iso_detect = latin_detect = DETECT_NORMAL;
	    possible = 2;
	} else {
	    iso_detect = priv_detect = DETECT_NORMAL;
	    priv = hint;	/* for TVCN, VISCII, VPS */
	    possible = 2;
	}
	break;
    }
#ifdef USE_UNICODE
    if (priv_detect == DETECT_ERROR) {
	utf8_detect = DETECT_NORMAL;
	possible++;
    }
#endif

    wc_input_init(WC_CES_US_ASCII, &st);

    for (; p < ep; p++) {
	if (possible == 0 || (possible == 1 && ok))
	    break;
	if (iso_detect != DETECT_ERROR) {
	    switch (*p) {
	    case WC_C_ESC:
		if (*(p+1) == WC_C_MBCS) {
		    q = p;
		    if (! wc_parse_iso2022_esc(&q, &st))
			break;
		    if (st.design[0] == WC_CCS_JIS_C_6226 ||
			st.design[0] == WC_CCS_JIS_X_0208)
			;
		    else if (st.design[0] == WC_CCS_JIS_X_0213_1 ||
			     st.design[0] == WC_CCS_JIS_X_0213_2)
			iso2022jp3 = WC_TRUE;
		    else if (WC_CCS_TYPE(st.design[0]) == WC_CCS_A_CS94W)
			iso2022jp2 = WC_TRUE;
		    if (st.design[1] == WC_CCS_KS_X_1001)
			iso2022kr = WC_TRUE;
		    else if (st.design[1] == WC_CCS_GB_2312 ||
			     st.design[1] == WC_CCS_ISO_IR_165 ||
			     st.design[1] == WC_CCS_CNS_11643_1)
			iso2022cn = WC_TRUE;
		    if (WC_CCS_TYPE(st.design[2]) == WC_CCS_A_CS94W ||
			WC_CCS_TYPE(st.design[3]) == WC_CCS_A_CS94W)
			iso2022cn = WC_TRUE;
		} else if (*(p+1) == WC_C_G2_CS96) {
		    q = p;
		    if (! wc_parse_iso2022_esc(&q, &st))
			break;
		    if (WC_CCS_TYPE(st.design[2]) == WC_CCS_A_CS96)
			iso2022jp2 = WC_TRUE;
		} else if (*(p+1) == WC_C_CSWSR) {
		    q = p;
		    if (! wc_parse_iso2022_esc(&q, &st))
			break;
		    possible = 0;
		    iso_detect = DETECT_BROKEN;
		    continue;
		}
		iso_detect = DETECT_OK;
		ok = WC_TRUE;
		break;
	    case WC_C_SI:
	    case WC_C_SO:
		iso_detect = DETECT_OK;
		ok = WC_TRUE;
		iso2022cn = WC_TRUE;
		iso2022kr = WC_TRUE;
		break;
	    default:
		if (*p & 0x80) {
		    iso_detect = DETECT_ERROR;
		    possible--;
		}
		break;
	    }
	}
	if (euc_detect != DETECT_ERROR) {
	    switch (euc_state) {
	    case WC_EUC_NOSTATE:
		switch (WC_ISO_MAP[*p]) {
		case WC_ISO_MAP_GR:
		    euc_state = WC_EUC_MBYTE1;
		    break;
		case WC_ISO_MAP_SS2:
		    if (euc == WC_CES_EUC_JP)
			euc_state = WC_EUC_MBYTE1;
		    else if (euc == WC_CES_EUC_TW)
			euc_state = WC_EUC_TW_SS2;
		    else
			euc_detect = DETECT_ERROR;
		    break;
		case WC_ISO_MAP_SS3:
		    if (euc == WC_CES_EUC_JP &&
			WC_ISO_MAP[*(p+1)] == WC_ISO_MAP_GR)
			;
		    else
			euc_detect = DETECT_ERROR;
		    break;
		case WC_ISO_MAP_C1:
		case WC_ISO_MAP_GR96:
		    euc_detect = DETECT_ERROR;
		    break;
		}
		break;
	    case WC_EUC_MBYTE1:
		if (WC_ISO_MAP[*p] == WC_ISO_MAP_GR) {
		    SET_DETECT(euc_detect, DETECT_OK);
		    ok = WC_TRUE;
		} else
		    SET_BROKEN_ERROR(euc_detect);
		euc_state = WC_EUC_NOSTATE;
		break;
	    case WC_EUC_TW_SS2:
		if (!( 0xa0 <= *p && *p <= 0xb0) ||
		    WC_ISO_MAP[*(p+1)] != WC_ISO_MAP_GR)
		    euc_detect = DETECT_ERROR;
		euc_state = WC_EUC_NOSTATE;
		break;
	    }
	    if (euc_detect == DETECT_ERROR)
		possible--;
	}
	if (sjis_detect != DETECT_ERROR) {
	    switch (sjis_state) {
	    case WC_SJIS_NOSTATE:
		switch (WC_SJIS_MAP[*p]) {
		case WC_SJIS_MAP_SL:
		case WC_SJIS_MAP_SH:
		    sjis_state = WC_SJIS_SHIFT_L;
		    break;
		case WC_SJIS_MAP_SK:
		    SET_DETECT(sjis_detect, DETECT_POSSIBLE);
		    break;
		case WC_SJIS_MAP_SX:
		    if (WcOption.use_jisx0213) {
			sjis_state = WC_SJIS_SHIFT_X;
			break;
		    }
		case WC_SJIS_MAP_80:
		case WC_SJIS_MAP_A0:
		case WC_SJIS_MAP_C1:
		    sjis_detect = DETECT_ERROR;
		    break;
		}
		break;
	    case WC_SJIS_SHIFT_L:
		if (WC_SJIS_MAP[*p] & WC_SJIS_MAP_LB) {
		    SET_DETECT(sjis_detect, DETECT_OK);
		    ok = WC_TRUE;
		} else
		    SET_BROKEN_ERROR(sjis_detect);
		sjis_state = WC_SJIS_NOSTATE;
		break;
	    case WC_SJIS_SHIFT_X:
		if (WC_SJIS_MAP[*p] & WC_SJIS_MAP_LB)
		    SET_DETECT(sjis_detect, DETECT_POSSIBLE);
		else
		    sjis_detect = DETECT_ERROR;
		sjis_state = WC_SJIS_NOSTATE;
		break;
	    }
	    if (sjis_detect == DETECT_ERROR)
		possible--;
	}
	if (big5_detect != DETECT_ERROR) {
	    switch (big5_state) {
	    case WC_BIG5_NOSTATE:
		switch (WC_BIG5_MAP[*p]) {
		case WC_BIG5_MAP_UB:
		    big5_state = WC_BIG5_MBYTE1;
		    break;
		case WC_BIG5_MAP_C1:
		    big5_detect = DETECT_ERROR;
		    break;
		}
		break;
	    case WC_BIG5_MBYTE1:
		if (WC_BIG5_MAP[*p] & WC_BIG5_MAP_LB) {
		    SET_DETECT(big5_detect, DETECT_OK);
		    ok = WC_TRUE;
		} else
		    SET_BROKEN_ERROR(big5_detect);
		big5_state = WC_BIG5_NOSTATE;
		break;
	    }
	    if (big5_detect == DETECT_ERROR)
		possible--;
	}
	if (hz_detect != DETECT_ERROR) {
	  if (*p & 0x80) {
		hz_detect = DETECT_ERROR;
		possible--;
	  } else {
	    switch (hz_state) {
	    case WC_HZ_NOSTATE:
		if (*p == WC_C_HZ_TILDA)
		    hz_state = WC_HZ_TILDA;
		break;
	    case WC_HZ_TILDA:
		if (*p == WC_C_HZ_SI)
		    hz_state = WC_HZ_MBYTE;
		else
		    hz_state = WC_HZ_NOSTATE;
		break;
	    case WC_HZ_TILDA_MB:
		if (*p == WC_C_HZ_SO)
		    hz_state = WC_HZ_NOSTATE;
		else
		    hz_state = WC_HZ_MBYTE;
		break;
	    case WC_HZ_MBYTE:
		if (*p == WC_C_HZ_TILDA)
		    hz_state = WC_HZ_TILDA_MB;
		else
		    hz_state = WC_HZ_MBYTE1;
		break;
	    case WC_HZ_MBYTE1:
		hz_detect = DETECT_OK;
		ok = WC_TRUE;
		hz_state = WC_HZ_NOSTATE;
		break;
	    }
	  }
	}
	if (latin_detect != DETECT_ERROR) {
	    switch (WC_ISO_MAP[*p] & WC_ISO_MAP_CG) {
	    case WC_ISO_MAP_GR:
	    case WC_ISO_MAP_GR96:
		SET_DETECT(latin_detect, DETECT_OK);
		ok = WC_TRUE;
		break;
	    case WC_ISO_MAP_C1:
		latin_detect = DETECT_ERROR;
		break;
	    }
	    if (latin_detect == DETECT_ERROR)
		possible--;
	}
	if (priv_detect != DETECT_ERROR) {
	    if (*p != WC_C_ESC && WC_DETECT_MAP[*p]) {
		SET_DETECT(priv_detect, DETECT_OK);
		ok = WC_TRUE;
	    }
/*
	    if (priv_detect == DETECT_ERROR)
		possible--;
*/
	}
#ifdef USE_UNICODE
	if (utf8_detect != DETECT_ERROR) {
	    switch (utf8_state) {
	    case WC_UTF8_NOSTATE:
		switch (utf8_next = WC_UTF8_MAP[*p]) {
		case 1:
		case 8:
		    break;
		case 0:
		case 7:
		    utf8_detect = DETECT_ERROR;
		    break;
		default:
		    utf8_next--;
		    utf8_state = WC_UTF8_NEXT;
		    break;
		}
		break;
	    case WC_UTF8_NEXT:
		if (WC_UTF8_MAP[*p]) {
		    utf8_detect = DETECT_ERROR;
		    utf8_state = WC_UTF8_NOSTATE;
		    break;
		}
		utf8_next--;
		if (! utf8_next) {
		    SET_DETECT(utf8_detect, DETECT_OK);
		    ok = WC_TRUE;
		    utf8_state = WC_UTF8_NOSTATE;
		}
		break;
	    }
	    if (utf8_detect == DETECT_ERROR)
		possible--;
	}
#endif
    }

    if (iso_detect != DETECT_ERROR) {
	if (iso_detect == DETECT_NORMAL) {
	   if (hz_detect == DETECT_OK)
		return WC_CES_HZ_GB_2312;
	   if (priv_detect == DETECT_OK)
		return priv;
	   return WC_CES_US_ASCII;
	}
	switch (euc) {
	case WC_CES_EUC_CN:
	case WC_CES_EUC_TW:
	    if (iso2022cn)
		return WC_CES_ISO_2022_CN;
	    break;
	case WC_CES_EUC_KR:
	    if (iso2022kr)
		return WC_CES_ISO_2022_KR;
	    break;
	}
	if (iso2022jp3)
	    return WC_CES_ISO_2022_JP_3;
	if (iso2022jp2)
	    return WC_CES_ISO_2022_JP_2;
	if (iso2022cn)
	    return WC_CES_ISO_2022_CN;
	if (iso2022kr)
	    return WC_CES_ISO_2022_KR;
	return WC_CES_ISO_2022_JP;
    }
    switch (hint) {
    case WC_CES_ISO_2022_JP:
    case WC_CES_ISO_2022_JP_2:
    case WC_CES_ISO_2022_JP_3:
    case WC_CES_ISO_2022_KR:
    case WC_CES_ISO_2022_CN:
	break;
    case WC_CES_EUC_JP:
    case WC_CES_EUC_CN:
    case WC_CES_EUC_TW:
    case WC_CES_EUC_KR:
	if (euc_detect != DETECT_ERROR)
	    return hint;
	break;
    case WC_CES_SHIFT_JIS:
    case WC_CES_SHIFT_JISX0213:
	if (sjis_detect != DETECT_ERROR)
	    return hint;
	break;
    case WC_CES_BIG5:
	if (big5_detect != DETECT_ERROR)
	    return hint;
	break;
#ifdef USE_UNICODE
    case WC_CES_UTF_8:
	return hint;
#endif
    case WC_CES_US_ASCII:
#ifdef USE_UNICODE
	if (utf8_detect != DETECT_ERROR)
	    return hint;
#endif
	if (latin_detect != DETECT_ERROR)
	    return WC_CES_ISO_8859_1;
	return hint;
    default:
	if (latin_detect != DETECT_ERROR)
	    return hint;
	if (priv_detect != DETECT_ERROR)
	    return hint;
#ifdef USE_UNICODE
	if (utf8_detect != DETECT_ERROR)
	    return WC_CES_UTF_8;
#endif
	return hint;
    }
    if (euc_detect == DETECT_OK)
	return euc;
    if (sjis_detect == DETECT_OK)
	return WC_CES_SHIFT_JIS;
    if (big5_detect == DETECT_OK)
	return WC_CES_BIG5;
#ifdef USE_UNICODE
    if (utf8_detect == DETECT_OK)
	return WC_CES_UTF_8;
    if (sjis_detect & DETECT_POSSIBLE)
	return WC_CES_SHIFT_JIS;
#endif
    if (euc_detect != DETECT_ERROR)
	return euc;
    if (sjis_detect != DETECT_ERROR)
	return WC_CES_SHIFT_JIS;
    if (big5_detect != DETECT_ERROR)
	return WC_CES_BIG5;
#ifdef USE_UNICODE
    if (utf8_detect != DETECT_ERROR)
	return WC_CES_UTF_8;
#endif
    return hint;
}
