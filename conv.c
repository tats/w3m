#include <stdio.h>
#include <string.h>
#include "fm.h"

#ifdef JP_CHARSET
#include "terms.h"
#include "Str.h"

#ifdef DEBUG
#include <malloc.h>
#endif				/* DEBUG */

#define	uchar		unsigned char
#define ushort		unsigned short
#define uint		unsigned int

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#define	TRUE		1
#define	FALSE		0
#ifdef ESC_CODE
#undef ESC_CODE
#endif
#define ESC_CODE	'\033'

#define CODE_NORMAL	0x00
#define CODE_OK		0x01
#define CODE_BROKEN	0x02
#define CODE_ERROR	0x04
#define EUC_NOSTATE	0x00
#define EUC_MBYTE1	0x10
#define EUC_SS2		0x20
#define EUC_SS3		0x40
#define SJIS_NOSTATE	0x00
#define SJIS_SHIFT_L	0x10
#define SJIS_SHIFT_H	0x20
#define ISO_NOSTATE	0x00
#define ISO_ESC		0x10
#define ISO_CS94	0x20
#define ISO_MBCS	0x40
#define ISO_MBYTE1	0x80
#define CODE_STATE(c)	((c) & 0x0f)
#define EUC_STATE(c)	((c) & 0xf0)
#define SJIS_STATE(c)	((c) & 0xf0)
#define ISO_STATE(c)	((c) & 0xf0)

#define CSET_ASCII	0
#define CSET_X0208	1
#define CSET_X0201K	2
#define CSET_UNKNOWN	3

#define	JSIcode  "\033$@"
#define	JSOcode  "\033(H"
#define	J2SIcode "\033$@"
#define	J2SOcode "\033(J"
#define	NSIcode  "\033$B"
#define	NSOcode  "\033(J"
#define	N2SIcode  "\033$B"
#define	N2SOcode  "\033(B"
#define	N3SIcode "\033$@"
#define	N3SOcode "\033(B"
#define	USIcode  "\033$"
#define	USOcode  "\033+"

static char *SIcode, *SOcode;

static Str cConvEE(Str is);
static Str cConvEJ(Str is);
static Str cConvES(Str is);
static Str cConvSE(Str is);
static Str cConvJE(Str is);
char checkShiftCode(Str buf, uchar);

static char *han2zen_tab[] =
{
    "!!", "!#", "!V", "!W", "!\"", "!&", "%r", "%!",
    "%#", "%%", "%'", "%)", "%c", "%e", "%g", "%C",
    "!<", "%\"", "%$", "%&", "%(", "%*", "%+", "%-",
    "%/", "%1", "%3", "%5", "%7", "%9", "%;", "%=",
    "%?", "%A", "%D", "%F", "%H", "%J", "%K", "%L",
    "%M", "%N", "%O", "%R", "%U", "%X", "%[", "%^",
    "%_", "%`", "%a", "%b", "%d", "%f", "%h", "%i",
    "%j", "%k", "%l", "%m", "%o", "%s", "!+", "!,",
};

typedef struct _ConvRoutine {
    char key;
     Str(*routine) ();
    char *ShiftIn, *ShiftOut;
} ConvRoutine;

static ConvRoutine FromEJ[] =
{
    {CODE_JIS_J, cConvEJ, JSIcode, JSOcode},
    {CODE_JIS_N, cConvEJ, NSIcode, NSOcode},
    {CODE_JIS_n, cConvEJ, N2SIcode, N2SOcode},
    {CODE_JIS_m, cConvEJ, N3SIcode, N3SOcode},
    {CODE_JIS_j, cConvEJ, J2SIcode, J2SOcode},
    {CODE_SJIS,  cConvES, "", ""},
    {CODE_EUC,   cConvEE, "", ""},
    {'\0', NULL, NULL, NULL}
};

static ConvRoutine ToEJ[] =
{
    {CODE_JIS_J, cConvJE, JSIcode, JSOcode},
    {CODE_JIS_N, cConvJE, NSIcode, NSOcode},
    {CODE_JIS_n, cConvJE, N2SIcode, N2SOcode},
    {CODE_JIS_m, cConvJE, N3SIcode, N3SOcode},
    {CODE_JIS_j, cConvJE, J2SIcode, J2SOcode},
    {CODE_SJIS,  cConvSE, "", ""},
    {CODE_EUC,   cConvEE, "", ""},
    {'\0', NULL, NULL, NULL}
};

char *
GetSICode(char key)
{
    int i;
    for (i = 0; FromEJ[i].key != '\0' ; i++)
	if (FromEJ[i].key == key)
	    return FromEJ[i].ShiftIn;
    return "";
}

char *
GetSOCode(char key)
{
    int i;
    for (i = 0; FromEJ[i].key != '\0'; i++)
	if (FromEJ[i].key == key)
	    return FromEJ[i].ShiftOut;
    return "";
}

static void
n_impr(char s)
{
    fprintf(stderr, "conv: option %c(0x%02x) is not implemented yet... sorry\n", s, s);
    exit(1);
}

Str
conv_str(Str is, char fc, char tc)
{
    int i;
    Str os;
    static char from_code = '\0';
    static char to_code = '\0';
    static Str (*conv_from) ();
    static Str (*conv_to) ();

    if (fc == tc || fc == CODE_ASCII || tc == CODE_ASCII)
	return is;

    if (fc == CODE_INNER_EUC)
	os = is;
    else {
	if (from_code != fc) {
	    for (i = 0; ToEJ[i].key != '\0'; i++) {
		if (ToEJ[i].key == fc) {
		    from_code = fc;
		    conv_from = *ToEJ[i].routine;
		    goto next;
		}
	    }
	    n_impr(fc);
	    return NULL;
	}
    next:
	os = conv_from(is);
    }
    if (tc == CODE_INNER_EUC || tc == CODE_EUC)
	return os;
    else {
	if (to_code != tc) {
	    for (i = 0; FromEJ[i].key != '\0'; i++) {
		if (FromEJ[i].key == tc) {
		    SIcode = FromEJ[i].ShiftIn;
		    SOcode = FromEJ[i].ShiftOut;
		    to_code = tc;
		    conv_to = *FromEJ[i].routine;
		    goto next2;
		}
	    }
	    n_impr(tc);
	    return NULL;
	}
    next2:
	return conv_to(os);
    }
}

Str
conv(char *is, char fc, char tc)
{
    return conv_str(Strnew_charp(is), fc, tc);
}

static uchar
getSLb(uchar * ptr, uchar * ub)
{				/* Get Shift-JIS Lower byte */
    uchar c = *ptr;

    *ub <<= 1;
    if (c < 0x9f) {
	if (c > 0x7e)
	    c--;
	*ub -= 1;
	c -= 0x3f;
    }
    else {
	c -= 0x9e;
    }
    return c;
}

static Str
cConvSE(Str is)
{				/* Convert Shift-JIS to EUC-JP */
    uchar *p, ub, lb;
    int state = SJIS_NOSTATE;
    Str os = Strnew_size(is->length);
    uchar *endp = (uchar *) &is->ptr[is->length];

    for (p = (uchar *) is->ptr; p < endp; p++) {
	switch (state) {
	case SJIS_NOSTATE:
	    if (!(*p & 0x80))	/* ASCII */
		Strcat_char(os, (char) (*p));
	    else if (0x81 <= *p && *p <= 0x9f) {	/* JIS X 0208,
							 * 0213 */
		ub = *p & 0x7f;
		state = SJIS_SHIFT_L;
	    }
	    else if (0xe0 <= *p && *p <= 0xef) {	/* JIS X 0208 */
		/* } else if (0xe0 <= *p && *p <= 0xfc) { *//* JIS X 0213 */
		ub = (*p & 0x7f) - 0x40;
		state = SJIS_SHIFT_H;
	    }
	    else if (0xa0 <= *p && *p <= 0xdf) {	/* JIS X 0201-Kana 
							 */
		Strcat_char(os, (char) (han2zen_tab[*p - 0xa0][0] | 0x80));
		Strcat_char(os, (char) (han2zen_tab[*p - 0xa0][1] | 0x80));
	    }
	    break;
	case SJIS_SHIFT_L:
	case SJIS_SHIFT_H:
	    if ((0x40 <= *p && *p <= 0x7e) ||
		(0x80 <= *p && *p <= 0xfc)) {	/* JIS X 0208, 0213 */
		lb = getSLb(p, &ub);
		ub += 0x20;
		lb += 0x20;
		Strcat_char(os, (char) (ub | 0x80));
		Strcat_char(os, (char) (lb | 0x80));
	    }
	    else if (!(*p & 0x80))	/* broken ? */
		Strcat_char(os, (char) (*p));
	    state = SJIS_NOSTATE;
	    break;
	}
    }
    return os;
}

static Str
cConvJE(Str is)
{				/* Convert ISO-2022-JP to EUC-JP */
    uchar *p, ub;
    char cset = CSET_ASCII;
    int state = ISO_NOSTATE;
    Str os = Strnew_size(is->length);
    uchar *endp = (uchar *) &is->ptr[is->length];

    for (p = (uchar *) is->ptr; p < endp; p++) {
	switch (state) {
	case ISO_NOSTATE:
	    if (*p == ESC_CODE)	/* ESC sequence */
		state = ISO_ESC;
	    else if (cset == CSET_ASCII || *p < 0x21)
		Strcat_char(os, (char) (*p));
	    else if (cset == CSET_X0208 && *p <= 0x7e) {
		/* JIS X 0208 */
		ub = *p;
		state = ISO_MBYTE1;
	    }
	    else if (cset == CSET_X0201K && *p <= 0x5f) {
		/* JIS X 0201-Kana */
		Strcat_char(os, (char) (han2zen_tab[*p - 0x20][0] | 0x80));
		Strcat_char(os, (char) (han2zen_tab[*p - 0x20][1] | 0x80));
	    }
	    break;
	case ISO_MBYTE1:
	    if (*p == ESC_CODE)	/* ESC sequence */
		state = ISO_ESC;
	    else if (0x21 <= *p && *p <= 0x7e) {	/* JIS X 0208 */
		Strcat_char(os, (char) (ub | 0x80));
		Strcat_char(os, (char) (*p | 0x80));
		state = ISO_NOSTATE;
	    }
	    else {
		Strcat_char(os, (char) (*p));
		state = ISO_NOSTATE;
	    }
	    break;
	case ISO_ESC:
	    if (*p == '(')	/* ESC ( F */
		state = ISO_CS94;
	    else if (*p == '$')	/* ESC $ F, ESC $ ( F */
		state = ISO_MBCS;
	    else {
		Strcat_char(os, ESC_CODE);
		Strcat_char(os, (char) (*p));
		state = ISO_NOSTATE;
	    }
	    break;
	case ISO_CS94:
	    if (*p == 'B' || *p == 'J' || *p == 'H')
		cset = CSET_ASCII;
	    else if (*p == 'I')
		cset = CSET_X0201K;
	    else {
		Strcat_char(os, ESC_CODE);
		Strcat_char(os, '(');
		Strcat_char(os, (char) (*p));
	    }
	    state = ISO_NOSTATE;
	    break;
	case ISO_MBCS:
	    if (*p == '(') {	/* ESC $ ( F */
		state = ISO_MBCS | ISO_CS94;
		break;
	    }
	case ISO_MBCS | ISO_CS94:
	    if (*p == 'B' || *p == '@')
		cset = CSET_X0208;
	    else {
		Strcat_char(os, ESC_CODE);
		Strcat_char(os, '$');
		if (state == (ISO_MBCS | ISO_CS94))
		    Strcat_char(os, '(');
		Strcat_char(os, (char) (*p));
	    }
	    state = ISO_NOSTATE;
	    break;
	}
    }
    return os;
}

static Str
_cConvEE(Str is, char is_euc)
{				/* Convert EUC-JP to EUC-JP / ISO-2022-JP
				 * (no JIS X 0201-Kana, 0212, 0213-2) */
    uchar *p, ub, euc = 0;
    int state = EUC_NOSTATE;
    char cset = CSET_ASCII;
    Str os;
    uchar *endp = (uchar *) &is->ptr[is->length];

    if (is_euc) {
	os = Strnew_size(is->length);
	euc = 0x80;
    }
    else
	os = Strnew_size(is->length * 3 / 2);

    for (p = (uchar *) is->ptr; p < endp; p++) {
	switch (state) {
	case EUC_NOSTATE:
	    if (!(*p & 0x80)) {	/* ASCII */
		if (!is_euc && cset != CSET_ASCII) {
		    Strcat_charp(os, SOcode);
		    cset = CSET_ASCII;
		}
		Strcat_char(os, (char) (*p));
	    }
	    else if (0xa1 <= *p && *p <= 0xfe) {	/* JIS X 0208,
							 * 0213-1 */
		ub = *p;
		state = EUC_MBYTE1;
	    }
	    else if (*p == EUC_SS2_CODE)	/* SS2 + JIS X 0201-Kana */
		state = EUC_SS2;
	    else if (*p == EUC_SS3_CODE)	/* SS3 + JIS X 0212, 0213-2 */
		state = EUC_SS3;
	    break;
	case EUC_MBYTE1:
	    if (0xa1 <= *p && *p <= 0xfe) {	/* JIS X 0208, 0213-1 */
		if (!is_euc && cset != CSET_X0208) {
		    Strcat_charp(os, SIcode);
		    cset = CSET_X0208;
		}
		Strcat_char(os, (char) ((ub & 0x7f) | euc));
		Strcat_char(os, (char) ((*p & 0x7f) | euc));
	    }
	    else if (!(*p & 0x80)) {	/* broken ? */
		if (!is_euc && cset != CSET_ASCII) {
		    Strcat_charp(os, SOcode);
		    cset = CSET_ASCII;
		}
		Strcat_char(os, (char) (*p));
	    }
	    state = EUC_NOSTATE;
	    break;
	case EUC_SS2:
	    if (0xa0 <= *p && *p <= 0xdf) {	/* JIS X 0201-Kana */
		if (!is_euc && cset != CSET_X0208) {
		    Strcat_charp(os, SIcode);
		    cset = CSET_X0208;
		}
		Strcat_char(os, (char) (han2zen_tab[*p - 0xa0][0] | euc));
		Strcat_char(os, (char) (han2zen_tab[*p - 0xa0][1] | euc));
	    }
	    state = EUC_NOSTATE;
	    break;
	case EUC_SS3:
	    state = (EUC_SS3 | EUC_MBYTE1);
	    break;
	case EUC_SS3 | EUC_MBYTE1:
	    state = EUC_NOSTATE;
	    break;
	}
    }
    if (!is_euc && cset != CSET_ASCII)
	Strcat_charp(os, SOcode);
    return os;
}

static Str
cConvEE(Str is)
{
    return _cConvEE(is, TRUE);
}

static Str
cConvEJ(Str is)
{
    return _cConvEE(is, FALSE);
}

void
put_sjis(Str os, uchar ub, uchar lb)
{
    ub -= 0x20;
    lb -= 0x20;
    if ((ub & 1) == 0)
	lb += 94;
    ub = ((ub - 1) >> 1) + 0x81;
    lb += 0x3f;
    if (ub > 0x9f)
	ub += 0x40;
    if (lb > 0x7e)
	lb++;

    Strcat_char(os, (char) (ub));
    Strcat_char(os, (char) (lb));
}

static Str
cConvES(Str is)
{				/* Convert EUC-JP to Shift-JIS */
    uchar *p, ub;
    int state = EUC_NOSTATE;
    Str os = Strnew_size(is->length);
    uchar *endp = (uchar *) &is->ptr[is->length];

    for (p = (uchar *) is->ptr; p < endp; p++) {
	switch (state) {
	case EUC_NOSTATE:
	    if (!(*p & 0x80))	/* ASCII */
		Strcat_char(os, (char) (*p));
	    else if (0xa1 <= *p && *p <= 0xfe) {	/* JIS X 0208,
							 * 0213-1 */
		ub = *p;
		state = EUC_MBYTE1;
	    }
	    else if (*p == EUC_SS2_CODE)	/* SS2 + JIS X 0201-Kana */
		state = EUC_SS2;
	    else if (*p == EUC_SS3_CODE)	/* SS3 + JIS X 0212, 0213-2 */
		state = EUC_SS3;
	    break;
	case EUC_MBYTE1:
	    if (0xa1 <= *p && *p <= 0xfe)	/* JIS X 0208, 0213-1 */
		put_sjis(os, ub & 0x7f, *p & 0x7f);
	    else if (!(*p & 0x80))	/* broken ? */
		Strcat_char(os, (char) (*p));
	    state = EUC_NOSTATE;
	    break;
	case EUC_SS2:
	    if (0xa0 <= *p && *p <= 0xdf)	/* JIS X 0201-Kana */
		put_sjis(os, han2zen_tab[*p - 0xa0][0],
		       han2zen_tab[*p - 0xa0][1]);
	    state = EUC_NOSTATE;
	    break;
	case EUC_SS3:
	    state = (EUC_SS3 | EUC_MBYTE1);
	    break;
	case EUC_SS3 | EUC_MBYTE1:
	    state = EUC_NOSTATE;
	    break;
	}
    }
    return os;
}

/* 
 * static ushort sjis_shift[8] = { 0x7fff, 0xffff, 0x0, 0x0, 0x0,
 * 0x0, 0xffff, 0x0 }; static ushort sjis_second[16] = { 0x0, 0x0, 
 * 0x0, 0x0, 0xffff, 0xffff, 0xffff, 0xfffe, 0xffff, 0xffff, 0xffff,
 * 0xffff, 0xffff, 0xffff, 0xffff, 0xfff8 }; */

char
checkShiftCode(Str buf, uchar hint)
{
    uchar *p, si = '\0', so = '\0';
    int euc = (CODE_NORMAL | EUC_NOSTATE),
	sjis = (CODE_NORMAL | SJIS_NOSTATE), sjis_kana = CODE_NORMAL,
	iso = (CODE_NORMAL | ISO_NOSTATE), iso_kana = CODE_NORMAL;
    uchar *endp = (uchar *) &buf->ptr[buf->length];

    if (hint == CODE_INNER_EUC)
	return '\0';
    p = (uchar *) buf->ptr;
    while (1) {
	if (iso != CODE_ERROR && (si == '\0' || so == '\0')) {
	    switch (ISO_STATE(iso)) {
	    case ISO_NOSTATE:
		if (*p == ESC_CODE)	/* ESC sequence */
		    iso = (CODE_STATE(iso) | ISO_ESC);
		break;
	    case ISO_ESC:
		if (*p == '(')	/* ESC ( F */
		    iso = (CODE_STATE(iso) | ISO_CS94);
		else if (*p == '$')	/* ESC $ F, ESC $ ( F */
		    iso = (CODE_STATE(iso) | ISO_MBCS);
		else
		    iso = (CODE_STATE(iso) | ISO_NOSTATE);
		break;
	    case ISO_CS94:
		if (*p == 'B' || *p == 'J' || *p == 'H')
		    so = *p;
		else if (*p == 'I')
		    iso_kana = CODE_OK;
		iso = (CODE_STATE(iso) | ISO_NOSTATE);
		break;
	    case ISO_MBCS:
		if (*p == '(') {	/* ESC $ ( F */
		    iso = (CODE_STATE(iso) | ISO_MBCS | ISO_CS94);
		    break;
		}
	    case ISO_MBCS | ISO_CS94:
		if (*p == 'B' || *p == '@')
		    si = *p;
		iso = (CODE_STATE(iso) | ISO_NOSTATE);
		break;
	    }
	    if (*p & 0x80)
		iso = CODE_ERROR;
	}
	if (euc != CODE_ERROR) {
	    switch (EUC_STATE(euc)) {
	    case EUC_NOSTATE:
		if (!(*p & 0x80))	/* ASCII */
		    ;
		else if (0xa1 <= *p && *p <= 0xfe)	/* JIS X 0208,
							 * 0213-1 */
		    euc = (CODE_STATE(euc) | EUC_MBYTE1);
		else if (*p == EUC_SS2_CODE)	/* SS2 + JIS X 0201-Kana */
		    euc = (CODE_STATE(euc) | EUC_SS2);
		else if (*p == EUC_SS3_CODE)	/* SS3 + JIS X 0212, 0213-2 */
		    euc = (CODE_STATE(euc) | EUC_SS3);
		else
		    euc = CODE_ERROR;
		break;
	    case EUC_MBYTE1:
		if (CODE_STATE(euc) == CODE_NORMAL)
		    euc = CODE_OK;
	    case EUC_SS3 | EUC_MBYTE1:
		if (0xa1 <= *p && *p <= 0xfe)	/* JIS X 0208, 0213-1 */
		    euc = (CODE_STATE(euc) | EUC_NOSTATE);
		else if (euc & CODE_BROKEN)
		    euc = CODE_ERROR;
		else
		    euc = (CODE_BROKEN | EUC_NOSTATE);
		break;
	    case EUC_SS2:
		if (0xa0 <= *p && *p <= 0xdf)	/* JIS X 0201-Kana */
		    euc = (CODE_STATE(euc) | EUC_NOSTATE);
		else
		    euc = CODE_ERROR;
		break;
	    case EUC_SS3:
		if (0xa1 <= *p && *p <= 0xfe)	/* JIS X 0212, 0213-2 */
		    euc = (CODE_STATE(euc) | EUC_SS3 | EUC_MBYTE1);
		else
		    euc = CODE_ERROR;
		break;
	    }
	}
	if (sjis != CODE_ERROR) {
	    switch (SJIS_STATE(sjis)) {
	    case SJIS_NOSTATE:
		if (!(*p & 0x80))	/* ASCII */
		    ;
		else if (0x81 <= *p && *p <= 0x9f)
		    sjis = (CODE_STATE(sjis) | SJIS_SHIFT_L);
		else if (0xe0 <= *p && *p <= 0xef)	/* JIS X 0208 */
		    /* else if (0xe0 <= *p && *p <= 0xfc) */
		    /* JIS X 0213 */
		    sjis = (CODE_STATE(sjis) | SJIS_SHIFT_H);
		else if (0xa0 == *p)
		    sjis = (CODE_BROKEN | SJIS_NOSTATE);
		else if (0xa1 <= *p && *p <= 0xdf)	/* JIS X 0201-Kana 
							 */
		    sjis_kana = CODE_OK;
		else
		    sjis = CODE_ERROR;
		break;
	    case SJIS_SHIFT_L:
	    case SJIS_SHIFT_H:
		if (CODE_STATE(sjis) == CODE_NORMAL)
		    sjis = CODE_OK;
		if ((0x40 <= *p && *p <= 0x7e) ||
		    (0x80 <= *p && *p <= 0xfc))		/* JIS X 0208,
							 * 0213 */
		    sjis = (CODE_STATE(sjis) | SJIS_NOSTATE);
		else if (sjis & CODE_BROKEN)
		    sjis = CODE_ERROR;
		else
		    sjis = (CODE_BROKEN | SJIS_NOSTATE);
		break;
	    }
	}
	if (euc == CODE_ERROR || sjis == CODE_ERROR)
	    break;
	if (p == endp)
	    break;
	p++;
    }
    if (iso != CODE_ERROR) {
	if (si == '\0' && so == '\0' && iso_kana != CODE_OK)
	    return '\0';
	switch (si) {
	case '@':
	    switch (so) {
	    case 'H':
		return CODE_JIS_J;
	    case 'J':
		return CODE_JIS_j;
	    case 'B':
		return CODE_JIS_m;
	    default:
		return CODE_JIS_m;
	    }
	case 'B':
	    switch (so) {
	    case 'J':
		return CODE_JIS_N;
	    case 'B':
		return CODE_JIS_n;
	    default:
		return CODE_JIS_n;
	    }
	default:
	    switch (so) {
	    case 'H':
		return CODE_JIS_J;
	    case 'J':
		return CODE_JIS_N;
	    case 'B':
		return CODE_JIS_n;
	    default:
		return CODE_JIS_n;
	    }
	}
    }
    if (hint == CODE_EUC) {
	if (euc != CODE_ERROR)
	    return CODE_EUC;
    } else if (hint == CODE_SJIS) {
	if (sjis != CODE_ERROR)
	    return CODE_SJIS;
    }
    if (CODE_STATE(euc) == CODE_OK)
	return CODE_EUC;
    if (CODE_STATE(sjis) == CODE_OK)
	return CODE_SJIS;
    if (CODE_STATE(euc) == CODE_NORMAL)
	return CODE_EUC;
    if (CODE_STATE(sjis) == CODE_NORMAL)
	return CODE_SJIS;
    return CODE_EUC;
}
#endif				/* JP_CHARSET */
