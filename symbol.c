
#include "fm.h"

#include "Symbols/alt.sym"
#include "Symbols/graph.sym"
#ifdef USE_M17N
#include "Symbols/eucjp.sym"
#include "Symbols/euckr.sym"
#include "Symbols/euccn.sym"
#include "Symbols/euctw.sym"
#include "Symbols/big5.sym"
#ifdef USE_UNICODE
#include "Symbols/utf8.sym"
#endif
#include "Symbols/cp850.sym"

typedef struct {
    wc_ces ces;
    char width;
    char **item;
    char **conved_item;
} symbol_set;

typedef struct {
    wc_ces charset;
    symbol_set *symbol;
} charset_symbol_set;

/* *INDENT-OFF* */
static symbol_set alt_symbol_set   = { WC_CES_US_ASCII, 1, alt_symbol,   alt_symbol };
static symbol_set alt2_symbol_set  = { WC_CES_US_ASCII, 2, alt2_symbol,  alt2_symbol };
static symbol_set eucjp_symbol_set = { WC_CES_EUC_JP,   2, eucjp_symbol, NULL };
static symbol_set euckr_symbol_set = { WC_CES_EUC_KR,   2, euckr_symbol, NULL };
static symbol_set euccn_symbol_set = { WC_CES_EUC_CN,   2, euccn_symbol, NULL };
static symbol_set euctw_symbol_set = { WC_CES_EUC_TW,   2, euctw_symbol, NULL };
static symbol_set big5_symbol_set  = { WC_CES_BIG5,     2, big5_symbol,  NULL };
#ifdef USE_UNICODE
static symbol_set utf8_symbol_set  = { WC_CES_UTF_8,    1, utf8_symbol,  NULL };
#endif
static symbol_set cp850_symbol_set = { WC_CES_CP850,    1, cp850_symbol, NULL };

static charset_symbol_set charset_symbol_list[] = {
    { WC_CES_EUC_JP,        &eucjp_symbol_set },
    { WC_CES_SHIFT_JIS,     &eucjp_symbol_set },
    { WC_CES_ISO_2022_JP,   &eucjp_symbol_set },
    { WC_CES_ISO_2022_JP_2, &eucjp_symbol_set },
    { WC_CES_ISO_2022_JP_3, &eucjp_symbol_set },
    { WC_CES_EUC_KR,        &euckr_symbol_set },
    { WC_CES_ISO_2022_KR,   &euckr_symbol_set },
    { WC_CES_JOHAB,         &euckr_symbol_set },
    { WC_CES_UHC,           &euckr_symbol_set },
    { WC_CES_EUC_CN,        &euccn_symbol_set },
    { WC_CES_GBK,           &euccn_symbol_set },
    { WC_CES_GB18030,       &euccn_symbol_set },
    { WC_CES_HZ_GB_2312,    &euccn_symbol_set },
    { WC_CES_ISO_2022_CN,   &euccn_symbol_set },
    { WC_CES_EUC_TW,        &euctw_symbol_set },
    { WC_CES_BIG5,          &big5_symbol_set  },
    { WC_CES_HKSCS,         &big5_symbol_set  },
#ifdef USE_UNICODE
    { WC_CES_UTF_8,         &utf8_symbol_set  },
#endif
    { WC_CES_CP850,         &cp850_symbol_set },
    { 0, NULL },
};
/* *INDENT-ON* */

static wc_ces save_charset = 0;
static symbol_set *save_symbol = NULL;

static void
encode_symbol(symbol_set * s)
{
    int i;

    for (i = 0; s->item[i]; i++) ;
    s->conved_item = New_N(char *, i);
    for (i = 0; s->item[i]; i++) {
	if (*(s->item[i]))
	    s->conved_item[i] = wc_conv(s->item[i], s->ces, InnerCharset)->ptr;
    }
}

char **
get_symbol(wc_ces charset, int *width)
{
    charset_symbol_set *p;
    symbol_set *s = NULL;

    if (UseGraphicChar != GRAPHIC_CHAR_ASCII) {
	if (charset == save_charset && save_symbol != NULL &&
	    *width == save_symbol->width) {
	    return save_symbol->conved_item;
	}
	save_charset = charset;
	for (p = charset_symbol_list; p->charset; p++) {
	    if (charset == p->charset &&
		(*width == 0 || *width == p->symbol->width)) {
		s = p->symbol;
		break;
	    }
	}
	if (s == NULL)
	    s = (*width == 2) ? &alt2_symbol_set : &alt_symbol_set;
	if (s != save_symbol) {
	    if (!s->conved_item)
		encode_symbol(s);
	    save_symbol = s;
	}
    } else {
	if (save_symbol != NULL && *width == save_symbol->width)
	    return save_symbol->conved_item;
	s = (*width == 2) ? &alt2_symbol_set : &alt_symbol_set;
	save_symbol = s;
    }
    *width = s->width;
    return s->conved_item;
}

char **
set_symbol(int width)
{
    static char **symbol_buf = NULL;
    static int save_width = -1;
    symbol_set *s = &alt_symbol_set;
    int i;
    Str tmp;

    if (width == save_width)
	return symbol_buf;
    if (symbol_buf == NULL) {
	for (i = 0; s->item[i]; i++) ;
	symbol_buf = New_N(char *, i);
    }
    for (i = 0; s->item[i]; i++) {
	tmp = Strnew_size(4);
	if (width == 2)
	    wtf_push(tmp, WC_CCS_SPECIAL_W, (wc_uint32) (SYMBOL_BASE + i));
	else
	    wtf_push(tmp, WC_CCS_SPECIAL, (wc_uint32) (SYMBOL_BASE + i));
	symbol_buf[i] = tmp->ptr;
    }
    save_width = width;
    return symbol_buf;
}

#ifdef USE_UNICODE
void
update_utf8_symbol(void)
{
    charset_symbol_set *p;
    utf8_symbol_set.width = WcOption.east_asian_width ? 2 : 1;
    for (p = charset_symbol_list; p->charset; p++) {
	if (p->charset == WC_CES_UTF_8) {
	    encode_symbol(p->symbol);
	    break;
	}
    }
}
#endif

#else

char **
get_symbol(void)
{
    return alt_symbol;
}
#endif

void
push_symbol(Str str, char symbol, int width, int n)
{
    char buf[2], *p;
    int i;

#ifdef USE_M17N
    if (width == 2)
	p = alt2_symbol[(int)symbol];
    else
#endif
	p = alt_symbol[(int)symbol];
    for (i = 0; i < 2 && *p; i++, p++)
	buf[i] = (*p == ' ') ? NBSP_CODE : *p;

    Strcat(str, Sprintf("<_SYMBOL TYPE=%d>", symbol));
    for (; n > 0; n--)
	Strcat_charp_n(str, buf, i);
    Strcat_charp(str, "</_SYMBOL>");
}
