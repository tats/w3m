
#include "wc.h"
#include "iso2022.h"
#include "jis.h"
#include "big5.h"
#include "johab.h"
#include "wtf.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif

#define C0  WC_ISO_MAP_C0
#define C1  WC_ISO_MAP_C1
#define GL  WC_ISO_MAP_GL
#define GR  WC_ISO_MAP_GR
#define GL2 WC_ISO_MAP_GL96
#define GR2 WC_ISO_MAP_GR96
#define SO  WC_ISO_MAP_SO
#define SI  WC_ISO_MAP_SI
#define ESC WC_ISO_MAP_ESC
#define SS2 WC_ISO_MAP_SS2
#define SS3 WC_ISO_MAP_SS3

wc_uint8 WC_ISO_MAP[ 0x100 ] = {
   C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, SO, SI,
   C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, C0, ESC,C0, C0, C0, C0,
   GL2,GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
   GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
   GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
   GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
   GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL,
   GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL, GL2,

   C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, SS2,SS3,
   C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1, C1,
   GR2,GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR,
   GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR,
   GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR,
   GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR,
   GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR,
   GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR, GR2,
};

static wc_uchar cs94_gmap[ 0x80 - WC_F_ISO_BASE ];
static wc_uchar cs94w_gmap[ 0x80 - WC_F_ISO_BASE ];
static wc_uchar cs96_gmap[ 0x80 - WC_F_ISO_BASE ];
static wc_uchar cs96w_gmap[ 0x80 - WC_F_ISO_BASE ];
static wc_uchar cs942_gmap[ 0x80 - WC_F_ISO_BASE ];

static void
wtf_push_iso2022(Str os, wc_ccs ccs, wc_uint32 code)
{
    switch (ccs) {
    case WC_CCS_JIS_C_6226:
    case WC_CCS_JIS_X_0208:
    case WC_CCS_JIS_X_0213_1:
	ccs = wc_jisx0208_or_jisx02131(code);
	break;
    case WC_CCS_JIS_X_0212:
    case WC_CCS_JIS_X_0213_2:
	ccs = wc_jisx0212_or_jisx02132(code);
	break;
    case WC_CCS_JIS_X_0201:
    case WC_CCS_GB_1988:
	ccs = WC_CCS_US_ASCII;
	break;
    }
    wtf_push(os, ccs, code);
}

Str
wc_conv_from_iso2022(Str is, wc_ces ces)
{
    Str os;
    wc_uchar *sp = (wc_uchar *)is->ptr;
    wc_uchar *ep = sp + is->length;
    wc_uchar *p, *q = NULL;
    int state = WC_ISO_NOSTATE;
    wc_status st;
    wc_ccs gl_ccs, gr_ccs;

    for (p = sp; p < ep && !(WC_ISO_MAP[*p] & WC_ISO_MAP_DETECT); p++)
	;
    if (p == ep)
	return is;
    os = Strnew_size(is->length);
    if (p > sp)
	Strcat_charp_n(os, is->ptr, (int)(p - sp));

    wc_input_init(ces, &st);
    gl_ccs = st.design[st.gl];
    gr_ccs = st.design[st.gr];

    for (; p < ep; p++) {
	switch (state) {
	case WC_ISO_NOSTATE:
	    switch (WC_ISO_MAP[*p]) {
	    case GL2:
		gl_ccs = st.ss ? st.design[st.ss]
			: st.design[st.gl];
		if (!(WC_CCS_TYPE(gl_ccs) & WC_CCS_A_CS96)) {
		    Strcat_char(os, (char)*p);
		    break;
		}
	    case GL:
		gl_ccs = st.ss ? st.design[st.ss]
			: st.design[st.gl];
		if (WC_CCS_IS_WIDE(gl_ccs)) {
		    q = p;
		    state = WC_ISO_MBYTE1;
		    continue;
		} else if (gl_ccs == WC_CES_US_ASCII)
		    Strcat_char(os, (char)*p);
		else
		    wtf_push_iso2022(os, gl_ccs, (wc_uint32)*p);
		break;
	    case GR2:
		gr_ccs = st.ss ? st.design[st.ss]
			: st.design[st.gr];
		if (!(WC_CCS_TYPE(gr_ccs) & WC_CCS_A_CS96)) {
		    wtf_push_unknown(os, p, 1);
		    break;
		}
	    case GR:
		gr_ccs = st.ss ? st.design[st.ss]
			: st.design[st.gr];
		if (WC_CCS_IS_WIDE(gr_ccs)) {
		    q = p;
		    state = WC_EUC_MBYTE1;
		    continue;
		} else if (gr_ccs)
		    wtf_push_iso2022(os, gr_ccs, (wc_uint32)*p);
		else
		    wtf_push_unknown(os, p, 1);
		break;
	    case C0:
		Strcat_char(os, (char)*p);
		break;
	    case C1:
		wtf_push(os, WC_CCS_C1, (wc_uint32)*p);
		break;
	    case ESC:
		st.ss = 0;
		if (wc_parse_iso2022_esc(&p, &st))
		    state = st.state;
		else
		    Strcat_char(os, (char)*p);
		continue;
	    case SI:
		st.gl = 0;
		break;
	    case SO:
		st.gl = 1;
		break;
	    case SS2:
		if (! st.design[2]) {
		    wtf_push_unknown(os, p, 1);
		    break;
		}
		st.ss = 2;
		continue;
	    case SS3:
		if (! st.design[3]) {
		    wtf_push_unknown(os, p, 1);
		    break;
		}
		st.ss = 3;
		continue;
	    }
	    break;
	case WC_ISO_MBYTE1:
	    switch (WC_ISO_MAP[*p]) {
	    case GL2:
		if (!(WC_CCS_TYPE(gl_ccs) & WC_CCS_A_CS96)) {
		    Strcat_char(os, (char)*q);
		    Strcat_char(os, (char)*p);
		    break;
		}
	    case GL:
		wtf_push_iso2022(os, gl_ccs, ((wc_uint32)*q << 8) | *p);
		break;
	    default:
		wtf_push_unknown(os, q, 2);
		break;
	    }
	    break;
	case WC_EUC_MBYTE1:
	    switch (WC_ISO_MAP[*p]) {
	    case GR2:
		if (!(WC_CCS_TYPE(gr_ccs) & WC_CCS_A_CS96)) {
		    wtf_push_unknown(os, q, 2);
		    break;
		}
	    case GR:
		if (gr_ccs == WC_CCS_CNS_11643_X) {
		    state = WC_EUC_TW_MBYTE2;
		    continue;
		}
		wtf_push_iso2022(os, gr_ccs, ((wc_uint32)*q << 8) | *p);
		break;
	    default:
		wtf_push_unknown(os, q, 2);
		break;
	    }
	    break;
	case WC_EUC_TW_MBYTE2:
	    if (WC_ISO_MAP[*p] == GR) {
		if (0xa1 <= *q && *q <= 0xa7) {
		    wtf_push_iso2022(os, WC_CCS_CNS_11643_1 + (*q - 0xa1),
			((wc_uint32)*(q+1) << 8) | *p);
		    break;
		}
		if (0xa8 <= *q && *q <= 0xb0) {
		    wtf_push_iso2022(os, WC_CCS_CNS_11643_8 + (*q - 0xa8),
			((wc_uint32)*(q+1) << 8) | *p);
		    break;
		}
	    }
	    wtf_push_unknown(os, q, 3);
	    break;
	case WC_ISO_CSWSR:
	    if (*p == WC_C_ESC && *(p+1) == WC_C_CSWSR) {
		if (*(p+2) == WC_F_ISO_BASE) {
		    state = st.state = WC_ISO_NOSTATE;
		    p += 2;
		    continue;
		} else if (*(p+2) > WC_F_ISO_BASE && *(p+2) <= 0x7e) {
		    p += 2;
		    continue;
		}
	    }
	    wtf_push_unknown(os, p, 1);
	    continue;
	case WC_ISO_CSWOSR:
	    wtf_push_unknown(os, p, ep - p);
	    return os;
	    break;
	}
	st.ss = 0;
	state = WC_ISO_NOSTATE;
    }
    switch (state) {
    case WC_ISO_MBYTE1:
    case WC_EUC_MBYTE1:
	wtf_push_unknown(os, p-1, 1);
	break;
    case WC_EUC_TW_MBYTE1:
	wtf_push_unknown(os, p-2, 2);
	break;
    }
    return os;
}

int
wc_parse_iso2022_esc(wc_uchar **ptr, wc_status *st)
{
    wc_uchar *p = *ptr, state, f = 0, g = 0, cs = 0;

    if (*p != WC_C_ESC)
	return 0;
    state = *p;
    for (p++; *p && state; p++) {
	switch (state) {
	case WC_C_ESC:		/* ESC */
	    switch (*p) {
	    case WC_C_MBCS:	/* ESC '$' */
		state = *p;
		continue;
	    case WC_C_G0_CS94:	/* ESC '(' */
	    case WC_C_G1_CS94:	/* ESC ')' */
	    case WC_C_G2_CS94:	/* ESC '*' */
	    case WC_C_G3_CS94:	/* ESC '+' */
		state = cs = WC_C_G0_CS94;
		g = *p & 0x03;
		continue;
	    case WC_C_G0_CS96:	/* ESC ',' */ /* ISO 2022 does not permit */
	    case WC_C_G1_CS96:	/* ESC '-' */
	    case WC_C_G2_CS96:	/* ESC '.' */
	    case WC_C_G3_CS96:	/* ESC '/' */
		state = cs = WC_C_G0_CS96;
		g = *p & 0x03;
		continue;
	    case WC_C_C0:	/* ESC '!' */ /* not suported */
	    case WC_C_C1:	/* ESC '"' */ /* not suported */
	    case WC_C_REP:	/* ESC '&' */ /* not suported */
		state = cs = WC_C_C0;
		continue;
	    case WC_C_CSWSR:	/* ESC '%' */ /* not suported */
		state = cs = WC_C_CSWSR;
		continue;
	    case WC_C_SS2:	/* ESC 'N' */
		st->ss = 2; *ptr = p; return 1;
	    case WC_C_SS3:	/* ESC 'O' */
		st->ss = 3; *ptr = p; return 1;
	    case WC_C_LS2:	/* ESC 'n' */
		st->gl = 2; *ptr = p; return 1;
	    case WC_C_LS3:	/* ESC 'o' */
		st->gl = 3; *ptr = p; return 1;
	    case WC_C_LS1R:	/* ESC '~' */
		st->gr = 1; *ptr = p; return 1;
	    case WC_C_LS2R:	/* ESC '}' */
		st->gr = 2; *ptr = p; return 1;
	    case WC_C_LS3R:	/* ESC '|' */
		st->gr = 3; *ptr = p; return 1;
	    default:
		return 0;
	    }
	    break;
	case WC_C_MBCS:		/* ESC '$' */
	    switch (*p) {
	    case WC_F_JIS_C_6226:	/* ESC '$' @ */
	    case WC_F_JIS_X_0208:	/* ESC '$' B */
	    case WC_F_GB_2312:	/* ESC '$' A */
		state = 0;
		cs = WC_C_G0_CS94 | 0x80;
		g = 0;
		f = *p;
		break;
	    case WC_C_G0_CS94:	/* ESC '$' '(' */
	    case WC_C_G1_CS94:	/* ESC '$' ')' */
	    case WC_C_G2_CS94:	/* ESC '$' '*' */
	    case WC_C_G3_CS94:	/* ESC '$' '+' */
		state = cs = WC_C_G0_CS94 | 0x80;
		g = *p & 0x03;
		continue;
	    case WC_C_G0_CS96:	/* ESC '$' ',' */ /* ISO 2022 does not permit */
	    case WC_C_G1_CS96:	/* ESC '$' '-' */
	    case WC_C_G2_CS96:	/* ESC '$' '.' */
	    case WC_C_G3_CS96:	/* ESC '$' '/' */
		state = cs = WC_C_G0_CS96 | 0x80;
		g = *p & 0x03;
		continue;
	    default:
		return 0;
	    }
	    break;
	case WC_C_G0_CS94:	/* ESC [()*+] F */
	    if (*p == WC_C_CS942) {	/* ESC [()*+] '!' */
		state = cs = WC_C_CS942 | 0x80;
		g = *p & 0x03;
		continue;
	    }
	case WC_C_G0_CS96:	/* ESC [,-./] F */
	case WC_C_G0_CS94 | 0x80:	/* ESC '$' [()*+] F */
	case WC_C_G0_CS96 | 0x80:	/* ESC '$' [,-./] F */
	case WC_C_CS942 | 0x80:	/* ESC [()*+] '!' F */
	case WC_C_C0:		/* ESC [!"&] F */
	case WC_C_CSWSR | 0x80:	/* ESC '%' '/' F */
	    state = 0;
	    f = *p;
	    break;
	case WC_C_CSWSR:	/* ESC '%' F */
	    if (*p == WC_C_CSWOSR) {	/* ESC '%' '/' */
		state = cs = WC_C_CSWSR | 0x80;
		continue;
	    }
	    state = 0;
	    f = *p;
	    break;
	default:
	    return 0;
	}
    }
    if (f < WC_F_ISO_BASE || f > 0x7e)
	return 0;
    switch (cs) {
    case WC_C_G0_CS94:
	st->design[g] = WC_CCS_SET_CS94(f);
	break;
    case WC_C_G0_CS94 | 0x80:
	st->design[g] = WC_CCS_SET_CS94W(f);
	break;
    case WC_C_G0_CS96:
	st->design[g] = WC_CCS_SET_CS96(f);
	break;
    case WC_C_G0_CS96 | 0x80:
	st->design[g] = WC_CCS_SET_CS96W(f);
	break;
    case WC_C_CS942 | 0x80:
	st->design[g] = WC_CCS_SET_CS942(f);
	break;
    case WC_C_CSWSR:
	if (f == WC_F_ISO_BASE)
	    st->state = WC_ISO_NOSTATE;
	else
	    st->state = WC_ISO_CSWSR;
	break;
    case WC_C_CSWOSR:
	st->state = WC_ISO_CSWOSR;
	break;
    }
    *ptr = p - 1;
    return 1;
}

void
wc_push_to_iso2022(Str os, wc_wchar_t cc, wc_status *st)
{
    wc_uchar g = 0;
    wc_bool is_wide = WC_FALSE, retry = WC_FALSE;
    wc_wchar_t cc2;

  while (1) {
    switch (WC_CCS_TYPE(cc.ccs)) {
    case WC_CCS_A_CS94:
	if (cc.ccs == WC_CCS_US_ASCII)
	    cc.ccs = st->g0_ccs;
	g = cs94_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	break;
    case WC_CCS_A_CS94W:
	is_wide = 1;
	switch (cc.ccs) {
#ifdef USE_UNICODE
	case WC_CCS_JIS_X_0212:
	    if (!WcOption.use_jisx0212 && WcOption.use_jisx0213 &&
		WcOption.ucs_conv) {
		cc2 = wc_jisx0212_to_jisx0213(cc);
		if (cc2.ccs == WC_CCS_JIS_X_0213_1 ||
		    cc2.ccs == WC_CCS_JIS_X_0213_2) {
		    cc = cc2;
		    continue;
		}
	    }
	    break;
	case WC_CCS_JIS_X_0213_1:
	case WC_CCS_JIS_X_0213_2:
	    if (!WcOption.use_jisx0213 && WcOption.use_jisx0212 &&
		WcOption.ucs_conv) {
		cc2 = wc_jisx0213_to_jisx0212(cc);
		if (cc2.ccs == WC_CCS_JIS_X_0212) {
		    cc = cc2;
		    continue;
		}
	    }
	    break;
#endif
	}
	g = cs94w_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	break;
    case WC_CCS_A_CS96:
	g = cs96_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	break;
    case WC_CCS_A_CS96W:
	is_wide = 1;
	g = cs96w_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	break;
    case WC_CCS_A_CS942:
	g = cs942_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	break;
    case WC_CCS_A_UNKNOWN_W:
	if (WcOption.no_replace)
	    return;
	is_wide = 1;
	cc.ccs = WC_CCS_US_ASCII;
	g = cs94_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	cc.code = ((wc_uint32)WC_REPLACE_W[0] << 8) | WC_REPLACE_W[1];
	break;
    case WC_CCS_A_UNKNOWN:
	if (WcOption.no_replace)
	    return;
	cc.ccs = WC_CCS_US_ASCII;
	g = cs94_gmap[WC_CCS_INDEX(cc.ccs) - WC_F_ISO_BASE];
	cc.code = (wc_uint32)WC_REPLACE[0];
	break;
    default:
	if ((cc.ccs == WC_CCS_JOHAB || WC_CCS_JOHAB_1 ||
		cc.ccs == WC_CCS_JOHAB_2 || cc.ccs == WC_CCS_JOHAB_3) &&
		cs94w_gmap[WC_F_KS_X_1001 - WC_F_ISO_BASE]) {
	    wc_wchar_t cc2 = wc_johab_to_ksx1001(cc);
	    if (cc2.ccs == WC_CCS_KS_X_1001) {
		cc = cc2;
		continue;
	    }
	}
#ifdef USE_UNICODE
	if (WcOption.ucs_conv)
	    cc = wc_any_to_iso2022(cc, st);
	else
#endif
	    cc.ccs = WC_CCS_IS_WIDE(cc.ccs) ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	continue;
    }
    if (! g) {
#ifdef USE_UNICODE
	if (WcOption.ucs_conv && ! retry)
	    cc = wc_any_to_any_ces(cc, st);
	else
#endif
	    cc.ccs = WC_CCS_IS_WIDE(cc.ccs) ? WC_CCS_UNKNOWN_W : WC_CCS_UNKNOWN;
	retry = WC_TRUE;
	continue;
    }

    wc_push_iso2022_esc(os, cc.ccs, g, 1, st);
    if (is_wide)
	Strcat_char(os, (char)((cc.code >> 8) & 0x7f));
    Strcat_char(os, (char)(cc.code & 0x7f));
    return;
  }
}

void
wc_push_to_iso2022_end(Str os, wc_status *st)
{
    if (st->design[1] != 0 && st->design[1] != st->g1_ccs)
	wc_push_iso2022_esc(os, st->g1_ccs, WC_C_G1_CS94, 0, st);
    wc_push_iso2022_esc(os, st->g0_ccs, WC_C_G0_CS94, 1, st);
}

void
wc_push_iso2022_esc(Str os, wc_ccs ccs, wc_uchar g, wc_uint8 invoke, wc_status *st)
{
    wc_uint8 g_invoke = g & 0x03;

    if (st->design[g_invoke] != ccs) {
	Strcat_char(os, WC_C_ESC);
	if (WC_CCS_IS_WIDE(ccs)) {
	    Strcat_char(os, WC_C_MBCS);
	    if (g_invoke != 0 ||
		(ccs != WC_CCS_JIS_C_6226 &&
		 ccs != WC_CCS_JIS_X_0208 &&
		 ccs != WC_CCS_GB_2312))
		Strcat_char(os, (char)g);
	} else {
	    Strcat_char(os, (char)g);
	    if ((ccs & WC_CCS_A_ISO_2022) == WC_CCS_A_CS942)
		Strcat_char(os, WC_C_CS942);
	}
	Strcat_char(os, (char)WC_CCS_GET_F(ccs));
	st->design[g_invoke] = ccs;
    }
    if (! invoke)
	return;

    switch (g_invoke) {
    case 0:
	if (st->gl != 0) {
	    Strcat_char(os, WC_C_SI);
	    st->gl = 0;
	}
	break;
    case 1:
	if (st->gl != 1) {
	    Strcat_char(os, WC_C_SO);
	    st->gl = 1;
	}
	break;
    case 2:
	Strcat_char(os, WC_C_ESC);
	Strcat_char(os, WC_C_SS2);
	break;
    case 3:
	Strcat_char(os, WC_C_ESC);
	Strcat_char(os, WC_C_SS3);
	break;
    }
}

void
wc_push_to_euc(Str os, wc_wchar_t cc, wc_status *st)
{
    wc_ccs g1_ccs = st->ces_info->gset[1].ccs;

  while (1) {
    if (cc.ccs == g1_ccs) {
	Strcat_char(os, (char)((cc.code >> 8) | 0x80));
	Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
	return;
    }
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_C1:
	Strcat_char(os, (char)(cc.code | 0x80));
	return;
    case WC_CCS_UNKNOWN_W:
	if (!WcOption.no_replace)
	    Strcat_charp(os, WC_REPLACE_W);
	return;
    case WC_CCS_UNKNOWN:
	if (!WcOption.no_replace)
	    Strcat_charp(os, WC_REPLACE);
	return;
    case WC_CCS_JOHAB:
    case WC_CCS_JOHAB_1:
    case WC_CCS_JOHAB_2:
    case WC_CCS_JOHAB_3:
	if (st->ces_info->id == WC_CES_EUC_KR) {
	    cc = wc_johab_to_ksx1001(cc);
	    continue;
	}
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
wc_push_to_eucjp(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_JIS_X_0201K:
	if (WcOption.use_jisx0201k) {
	    Strcat_char(os, WC_C_SS2R);
	    Strcat_char(os, (char)(cc.code | 0x80));
	    return;
	} else if (WcOption.fix_width_conv)
	    cc.ccs = WC_CCS_UNKNOWN;
	else
	    cc = wc_jisx0201k_to_jisx0208(cc);
	continue;
    case WC_CCS_JIS_X_0208:
	break;
    case WC_CCS_JIS_X_0213_1:
	if (WcOption.use_jisx0213)
	    break;
#ifdef USE_UNICODE
	else if (WcOption.ucs_conv && WcOption.use_jisx0212)
	    cc = wc_jisx0213_to_jisx0212(cc);
#endif
	else
	    cc.ccs = WC_CCS_UNKNOWN_W;
	continue;
    case WC_CCS_JIS_X_0212:
	if (WcOption.use_jisx0212) {
	    Strcat_char(os, WC_C_SS3R);
	    break;
	}
#ifdef USE_UNICODE
	else if (WcOption.ucs_conv && WcOption.use_jisx0213)
	    cc = wc_jisx0212_to_jisx0213(cc);
#endif
	else
	    cc.ccs = WC_CCS_UNKNOWN_W;
	continue;
    case WC_CCS_JIS_X_0213_2:
	if (WcOption.use_jisx0213) {
	    Strcat_char(os, WC_C_SS3R);
	    break;
	}
#ifdef USE_UNICODE
	else if (WcOption.ucs_conv && WcOption.use_jisx0212)
	    cc = wc_jisx0213_to_jisx0212(cc);
#endif
	else
	    cc.ccs = WC_CCS_UNKNOWN_W;
	continue;
    case WC_CCS_C1:
	Strcat_char(os, (char)(cc.code | 0x80));
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
    Strcat_char(os, (char)((cc.code >> 8) | 0x80));
    Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
    return;
  }
}

void
wc_push_to_euctw(Str os, wc_wchar_t cc, wc_status *st)
{
  while (1) {
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_CNS_11643_1:
	break;
    case WC_CCS_CNS_11643_2:
    case WC_CCS_CNS_11643_3:
    case WC_CCS_CNS_11643_4:
    case WC_CCS_CNS_11643_5:
    case WC_CCS_CNS_11643_6:
    case WC_CCS_CNS_11643_7:
	Strcat_char(os, WC_C_SS2R);
	Strcat_char(os, (char)(0xA1 + (cc.ccs - WC_CCS_CNS_11643_1)));
	break;
    case WC_CCS_CNS_11643_8:
    case WC_CCS_CNS_11643_9:
    case WC_CCS_CNS_11643_10:
    case WC_CCS_CNS_11643_11:
    case WC_CCS_CNS_11643_12:
    case WC_CCS_CNS_11643_13:
    case WC_CCS_CNS_11643_14:
    case WC_CCS_CNS_11643_15:
    case WC_CCS_CNS_11643_16:
	Strcat_char(os, WC_C_SS2R);
	Strcat_char(os, (char)(0xA8 + (cc.ccs - WC_CCS_CNS_11643_8)));
	break;
    case WC_CCS_C1:
	Strcat_char(os, (char)(cc.code | 0x80));
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
    Strcat_char(os, (char)((cc.code >> 8) | 0x80));
    Strcat_char(os, (char)((cc.code & 0xff) | 0x80));
    return;
  }
}

void
wc_push_to_iso8859(Str os, wc_wchar_t cc, wc_status *st)
{
    wc_ccs g1_ccs = st->ces_info->gset[1].ccs;

  while (1) {
    if (cc.ccs == g1_ccs) {
	Strcat_char(os, (char)(cc.code | 0x80));
	return;
    }
    switch (cc.ccs) {
    case WC_CCS_US_ASCII:
	Strcat_char(os, (char)cc.code);
	return;
    case WC_CCS_C1:
	Strcat_char(os, (char)(cc.code | 0x80));
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
wc_create_gmap(wc_status *st)
{
    wc_gset *gset = st->ces_info->gset;
    wc_uchar *gset_ext = st->ces_info->gset_ext;
    int i, f;

    if (WcOption.strict_iso2022) {
	for (i = 0; i < WC_F_ISO_BASE; i++) {
	    cs94_gmap[i] = 0;
	    cs96_gmap[i] = 0;
	    cs94w_gmap[i] = 0;
	    cs96w_gmap[i] = 0;
	    cs942_gmap[i] = 0;
	}
    } else {
	for (i = 0; i < WC_F_ISO_BASE; i++) {
	    cs94_gmap[i] = gset_ext[0];
	    cs96_gmap[i] = gset_ext[1];
	    cs94w_gmap[i] = gset_ext[2];
	    cs96w_gmap[i] = gset_ext[3];
	    cs942_gmap[i] = gset_ext[0];
	}
    }
    for (i = 0; gset[i].ccs; i++) {
	f = WC_CCS_GET_F(gset[i].ccs) - WC_F_ISO_BASE;
	switch (WC_CCS_TYPE(gset[i].ccs)) {
	case WC_CCS_A_CS94:
	    switch (gset[i].ccs) {
	    case WC_CCS_JIS_X_0201K:
		if (!WcOption.use_jisx0201k)
		    continue;
		break;
	    }
	    cs94_gmap[f] = gset[i].g;
	    break;
	case WC_CCS_A_CS94W:
	    switch (gset[i].ccs) {
	    case WC_CCS_JIS_X_0212:
		if (!WcOption.use_jisx0212)
		    continue;
		break;
	    case WC_CCS_JIS_X_0213_1:
	    case WC_CCS_JIS_X_0213_2:
		if (!WcOption.use_jisx0213)
		    continue;
		break;
	    }
	    cs94w_gmap[f] = gset[i].g;
	    break;
	case WC_CCS_A_CS96:
	    cs96_gmap[f] = gset[i].g;
	    break;
	case WC_CCS_A_CS96W:
	    cs96w_gmap[f] = gset[i].g;
	    break;
	case WC_CCS_A_CS942:
	    cs942_gmap[f] = gset[i].g;
	    break;
	}
    }
}

Str
wc_char_conv_from_iso2022(wc_uchar c, wc_status *st)
{
    static Str os;
    static wc_uchar buf[4];
    static size_t nbuf;
    wc_uchar *p;
    wc_ccs gl_ccs, gr_ccs;

    if (st->state == -1) {
	st->state = WC_ISO_NOSTATE;
	os = Strnew_size(8);
	nbuf = 0;
    }

    gl_ccs = st->ss ? st->design[st->ss] : st->design[st->gl];
    gr_ccs = st->ss ? st->design[st->ss] : st->design[st->gr];

    switch (st->state) {
    case WC_ISO_NOSTATE:
	switch (WC_ISO_MAP[c]) {
	case GL2:
	    if (!(WC_CCS_TYPE(gl_ccs) & WC_CCS_A_CS96)) {
		Strcat_char(os, (char)c);
		break;
	    }
	case GL:
	    if (WC_CCS_IS_WIDE(gl_ccs)) {
		buf[nbuf++] = c;
		st->state = WC_ISO_MBYTE1;
		return NULL;
	    } else if (gl_ccs == WC_CES_US_ASCII)
		Strcat_char(os, (char)c);
	    else
		wtf_push_iso2022(os, gl_ccs, (wc_uint32)c);
	    break;
	case GR2:
	    if (!(WC_CCS_TYPE(gr_ccs) & WC_CCS_A_CS96))
		break;
	case GR:
	    if (WC_CCS_IS_WIDE(gr_ccs)) {
		buf[nbuf++] = c;
		st->state = WC_EUC_MBYTE1;
		return NULL;
	    } else if (gr_ccs)
		wtf_push_iso2022(os, gr_ccs, (wc_uint32)c);
	    break;
	case C0:
	    Strcat_char(os, (char)c);
	    break;
	case C1:
	    break;
	case ESC:
	    buf[nbuf++] = c;
	    st->state = WC_C_ESC;
	    return NULL;
	case SI:
	    st->gl = 0;
	    break;
	case SO:
	    st->gl = 1;
	    break;
	case SS2:
	    if (! st->design[2])
		return os;
	    st->ss = 2;
	    return NULL;
	case SS3:
	    if (! st->design[3])
		return os;
	    st->ss = 3;
	    return NULL;
	}
	break;
    case WC_ISO_MBYTE1:
	switch (WC_ISO_MAP[c]) {
	case GL2:
	    if (!(WC_CCS_TYPE(gl_ccs) & WC_CCS_A_CS96))
		break;
	case GL:
	    buf[nbuf++] = c;
	    wtf_push_iso2022(os, gl_ccs, ((wc_uint32)buf[0] << 8) | buf[1]);
	    break;
	}
	st->state = WC_ISO_NOSTATE;
	break;
    case WC_EUC_MBYTE1:
	switch (WC_ISO_MAP[c]) {
	case GR2:
	    if (!(WC_CCS_TYPE(gr_ccs) & WC_CCS_A_CS96))
		break;
	case GR:
	    if (gr_ccs == WC_CCS_CNS_11643_X) {
		buf[nbuf++] = c;
		st->state = WC_EUC_TW_MBYTE2;
		return NULL;
	    }
	    buf[nbuf++] = c;
	    wtf_push_iso2022(os, gr_ccs, ((wc_uint32)buf[0] << 8) | buf[1]);
	    break;
	}
	st->state = WC_ISO_NOSTATE;
	break;
    case WC_EUC_TW_MBYTE2:
	if (WC_ISO_MAP[c] == GR) {
	    buf[nbuf++] = c;
	    c = buf[0];
	    if (0xa1 <= c && c <= 0xa7) {
		wtf_push_iso2022(os, WC_CCS_CNS_11643_1 + (c - 0xa1),
			((wc_uint32)buf[1] << 8) | buf[2]);
		break;
	    }
	    if (0xa8 <= c && c <= 0xb0) {
		wtf_push_iso2022(os, WC_CCS_CNS_11643_8 + (c - 0xa8),
			((wc_uint32)buf[1] << 8) | buf[2]);
		break;
	    }
	}
	st->state = WC_ISO_NOSTATE;
	break;
    case WC_C_ESC:
	switch (c) {
	case WC_C_G0_CS94:
	case WC_C_G1_CS94:
	case WC_C_G2_CS94:
	case WC_C_G3_CS94:
	    buf[nbuf++] = c;
	    st->state = WC_C_G0_CS94;
	    return NULL;
	case WC_C_G0_CS96:
	case WC_C_G1_CS96:
	case WC_C_G2_CS96:
	case WC_C_G3_CS96:
	case WC_C_C0:
	case WC_C_C1:
	case WC_C_REP:
	    buf[nbuf++] = c;
	    st->state = WC_C_G0_CS96;
	    return NULL;
	case WC_C_MBCS:
	case WC_C_CSWSR:
	    buf[nbuf++] = c;
	    st->state = c;
	    return NULL;
	case WC_C_SS2:
	    st->ss = 2;
	    st->state = WC_ISO_NOSTATE;
	    return NULL;
	case WC_C_SS3:
	    st->ss = 3;
	    st->state = WC_ISO_NOSTATE;
	    return NULL;
	case WC_C_LS2:
	    st->gl = 2;
	    break;
	case WC_C_LS3:
	    st->gl = 3;
	    break;
	case WC_C_LS2R:
	    st->gr = 2;
	    break;
	case WC_C_LS3R:
	    st->gr = 3;
	    break;
	default:
	    break;
	}
	break;
    case WC_C_MBCS:
	switch (c) {
	case WC_F_JIS_C_6226:
	case WC_F_JIS_X_0208:
	case WC_F_GB_2312:
	    buf[nbuf++] = c;
	    p = buf;
	    wc_parse_iso2022_esc(&p, st);
	    break;
	case WC_C_G0_CS94:
	case WC_C_G1_CS94:
	case WC_C_G2_CS94:
	case WC_C_G3_CS94:
	case WC_C_G0_CS96:
	case WC_C_G1_CS96:
	case WC_C_G2_CS96:
	case WC_C_G3_CS96:
	    buf[nbuf++] = c;
	    st->state = WC_C_G0_CS96;
	    return NULL;
	}
	break;
    case WC_C_CSWSR:
	switch (c) {
	case WC_C_CSWOSR:
	    buf[nbuf++] = c;
	    st->state = WC_C_G1_CS94;
	    return NULL;
	}
	buf[nbuf++] = c;
	p = buf;
	wc_parse_iso2022_esc(&p, st);
	break;
    case WC_C_G0_CS94:
	switch (c) {
	case WC_C_CS942:
	    buf[nbuf++] = c;
	    st->state = WC_C_G0_CS96;
	    return NULL;
	}
    case WC_C_G0_CS96:
	buf[nbuf++] = c;
	p = buf;
	wc_parse_iso2022_esc(&p, st);
	break;
    }
    st->ss = 0;
    st->state = -1;
    return os;
}
