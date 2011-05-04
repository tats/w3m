
#include <stdlib.h>
#include <ctype.h>
#include <gc.h>
#define New_N(type,n) ((type*)GC_MALLOC((n)*sizeof(type)))

#include "wc.h"

#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

wc_locale WcLocale = 0;

static struct {
  char *lang;
  wc_ces ces;
} lang_ces_table[] = {
  { "cs", WC_CES_ISO_8859_2 },	/* cs_CZ */
  { "el", WC_CES_ISO_8859_7 },	/* el_GR */
  { "iw", WC_CES_ISO_8859_8 },	/* iw_IL */
  { "ja", WC_CES_EUC_JP },	/* ja_JP */
  { "ko", WC_CES_EUC_KR },	/* ko_KR */
  { "hu", WC_CES_ISO_8859_2 },	/* hu_HU */
  { "pl", WC_CES_ISO_8859_2 },	/* pl_PL */
  { "ro", WC_CES_ISO_8859_2 },	/* ro_RO */
  { "ru", WC_CES_ISO_8859_5 },	/* ru_SU */
  { "sk", WC_CES_ISO_8859_2 },	/* sk_SK */
  { "sl", WC_CES_ISO_8859_2 },	/* sl_CS */
  { "tr", WC_CES_ISO_8859_9 },	/* tr_TR */
  { "zh", WC_CES_EUC_CN },	/* zh_CN */
  { NULL, 0 }
};

static wc_ces
wc_codepage(int n)
{
	switch (n) {
	case 437: return WC_CES_CP437;
	case 737: return WC_CES_CP737;
	case 775: return WC_CES_CP775;
	case 850: return WC_CES_CP850;
	case 852: return WC_CES_CP852;
	case 855: return WC_CES_CP855;
	case 856: return WC_CES_CP856;
	case 857: return WC_CES_CP857;
	case 860: return WC_CES_CP860;
	case 861: return WC_CES_CP861;
	case 862: return WC_CES_CP862;
	case 863: return WC_CES_CP863;
	case 864: return WC_CES_CP864;
	case 865: return WC_CES_CP865;
	case 866: return WC_CES_CP866;
	case 869: return WC_CES_CP869;
	case 874: return WC_CES_CP874;
	case 932: return WC_CES_CP932;		/* CP932 = Shift_JIS */
	case 936: return WC_CES_CP936;		/* CP936 = GBK > EUC_CN */
	case 943: return WC_CES_CP943;		/* CP943 = Shift_JIS */
	case 949: return WC_CES_CP949;		/* CP949 = UHC > EUC_KR */
	case 950: return WC_CES_CP950;		/* CP950 = Big5 */
	case 1006: return WC_CES_CP1006;
	case 1250: return WC_CES_CP1250;
	case 1251: return WC_CES_CP1251;
	case 1252: return WC_CES_CP1252;
	case 1253: return WC_CES_CP1253;
	case 1254: return WC_CES_CP1254;
	case 1255: return WC_CES_CP1255;
	case 1256: return WC_CES_CP1256;
	case 1257: return WC_CES_CP1257;
	case 1258: return WC_CES_CP1258;
	}
	return 0;
}

wc_ces
wc_guess_charset(char *charset, wc_ces orig)
{
    wc_ces guess;

    if (charset == NULL || *charset == '\0')
	return orig;
    guess = wc_charset_to_ces(charset);
    return guess ? guess : orig;
}

wc_ces
wc_guess_charset_short(char *charset, wc_ces orig)
{
    wc_ces guess;

    if (charset == NULL || *charset == '\0')
	return orig;
    guess = wc_charset_short_to_ces(charset);
    return guess ? guess : orig;
}

wc_ces
wc_guess_locale_charset(char *locale, wc_ces orig)
{
    wc_ces guess;

    if (locale == NULL || *locale == '\0')
	return orig;
    guess = wc_locale_to_ces(locale);
    return guess ? guess : orig;
}

wc_ces
wc_charset_to_ces(char *charset)
{
    char *p = charset;
    char buf[16];
    int n;

    if (tolower(*p) == 'x' && *(p+1) == '-')
	p += 2;
    for (n = 0; *p && n < 15; p++) {
	if ((unsigned char)*p > 0x20 && *p != '_' && *p != '-')
	    buf[n++] = tolower(*p);
    }
    buf[n] = 0;
    p = buf;
    switch (*p) {
    case 'e':
	if (! strncmp(p, "euc", 3)) {
	    p += 3;
	    switch (*p) {
	    case 'j': return WC_CES_EUC_JP;
	    case 'c': return WC_CES_EUC_CN;
	    case 't': return WC_CES_EUC_TW;
	    case 'k': return WC_CES_EUC_KR;
	    }
	    switch (WcLocale) {
	    case WC_LOCALE_JA_JP: return WC_CES_EUC_JP;
	    case WC_LOCALE_ZH_CN: return WC_CES_EUC_CN;
	    case WC_LOCALE_ZH_TW: return WC_CES_EUC_TW;
	    case WC_LOCALE_ZH_HK: return WC_CES_EUC_CN;
	    case WC_LOCALE_KO_KR: return WC_CES_EUC_KR;
	    }
	    return WC_CES_EUC_JP;
        }
	break;
    case 'i':
	if (! strncmp(p, "iso2022", 7)) {
	    p += 7;
	    switch (*p) {
	    case 'j':
		if (! strncmp(p, "jp2", 3))
		    return WC_CES_ISO_2022_JP_2;
		if (! strncmp(p, "jp3", 3))
		    return WC_CES_ISO_2022_JP_3;
		return WC_CES_ISO_2022_JP;
	    case 'c': return WC_CES_ISO_2022_CN;
	    case 'k': return WC_CES_ISO_2022_KR;
	    }
	    return WC_CES_ISO_2022_JP;
	} else if (! strncmp(p, "iso8859", 7)) {
	    n = atoi(p + 7);
	    if (n >= 1 && n <= 16 && n != 12)
		return (WC_CES_E_ISO_8859 | n);
	    return WC_CES_ISO_8859_1;
	} else if (! strncmp(p, "ibm", 3)) {
	    p += 3;
	    if (*p >= '1' && *p <= '9')
	    	return wc_codepage(atoi(p));
	    return wc_charset_to_ces(p);
	}
	break;
    case 'j':
	if (! strncmp(p, "johab", 5))
	    return WC_CES_JOHAB;
	if (! strncmp(p, "jis", 3))
	    return WC_CES_ISO_2022_JP;
	break;
    case 's':
	if (! strncmp(p, "shiftjisx0213", 13) ||
	    ! strncmp(p, "sjisx0213", 9))
	    return WC_CES_SHIFT_JISX0213;
	if (! strncmp(p, "shiftjis", 8) ||
	    ! strncmp(p, "sjis", 4))
	    return WC_CES_SHIFT_JIS;
	break;
    case 'p':
	if (! strncmp(p, "pck", 3))
	    return WC_CES_SHIFT_JIS;
	break;
    case 'g':
	if (! strncmp(p, "gb18030", 7) ||
	    ! strncmp(p, "gbk2k", 5))
	    return WC_CES_GB18030;
	if (! strncmp(p, "gbk", 3))
	    return WC_CES_GBK;
	if (! strncmp(p, "gb2312", 6))
	    return WC_CES_EUC_CN;
	break;
    case 'b':
	if (! strncmp(p, "big5hkscs", 9))
	    return WC_CES_HKSCS;
	if (! strncmp(p, "big5", 4))
	    return WC_CES_BIG5;
	break;
    case 'h':
	if (! strncmp(p, "hz", 2))
	    return WC_CES_HZ_GB_2312;
	if (! strncmp(p, "hkscs", 5))
	    return WC_CES_HKSCS;
	break;
    case 'k':
	if (! strncmp(p, "koi8r", 5))
	    return WC_CES_KOI8_R;
	if (! strncmp(p, "koi8u", 5))
	    return WC_CES_KOI8_U;
	if (! strncmp(p, "ksx1001", 7))
	    return WC_CES_EUC_KR;
	if (! strncmp(p, "ksc5601", 7))
	    return WC_CES_EUC_KR;
	break;
    case 't':
	if (! strncmp(p, "tis620", 6))
	    return WC_CES_TIS_620;
	if (! strncmp(p, "tcvn", 4))
	    return WC_CES_TCVN_5712;
	break;
    case 'n':
	if (! strncmp(p, "next", 4))
	    return WC_CES_NEXTSTEP;
	break;
    case 'v':
	if (! strncmp(p, "viet", 4)) {
	    p += 4;
	    if (! strncmp(p, "tcvn", 4))
		return WC_CES_TCVN_5712;
	}
	if (! strncmp(p, "viscii", 6))
	    return WC_CES_VISCII_11;
	if (! strncmp(p, "vps", 3))
	    return WC_CES_VPS;
	break;
    case 'u':
#ifdef USE_UNICODE
	if (! strncmp(p, "utf8", 4))
	    return WC_CES_UTF_8;
	if (! strncmp(p, "utf7", 4))
	    return WC_CES_UTF_7;
#endif
	if (! strncmp(p, "uhc", 3))
	    return WC_CES_UHC;
	if (! strncmp(p, "ujis", 4))
	    return WC_CES_EUC_JP;
	if (! strncmp(p, "usascii", 7))
	    return WC_CES_US_ASCII;
	break;
    case 'a':
	if (! strncmp(p, "ascii", 5))
	    return WC_CES_US_ASCII;
	break;
    case 'c':
	if (! strncmp(p, "cngb", 4))
	    return WC_CES_EUC_CN;
	if (*(p+1) != 'p')
	    break;
	p += 2;
	if (*p >= '1' &&  *p <= '9')
	    return wc_codepage(atoi(p));
	break;
    case 'w':
	if (strncmp(p, "windows", 7))
	    break;
	p += 7;
	if (! strncmp(p, "31j", 3))
	    return WC_CES_CP932;
	if (*p >= '1' &&  *p <= '9')
	    return wc_codepage(atoi(p));
	break;
    }
    return 0;
}

wc_ces
wc_charset_short_to_ces(char *charset)
{
    char *p = charset;
    char buf[16];
    wc_ces ces;
    int n;

    ces = wc_charset_to_ces(charset);
    if (ces)
	return ces;

    for (n = 0; *p && n < 15; p++) {
	if ((unsigned char)*p > 0x20 && *p != '_' && *p != '-')
	    buf[n++] = tolower(*p);
    }
    buf[n] = 0;
    p = buf;
    switch (*p) {
    case 'e':
	switch (*(p+1)) {
	case 'j': return WC_CES_EUC_JP;
	case 'c': return WC_CES_EUC_CN;
	case 't': return WC_CES_EUC_TW;
	case 'k': return WC_CES_EUC_KR;
	}
	return WC_CES_EUC_JP;
    case 'j':
	p++;
	if (*p == 'o')
	    return WC_CES_JOHAB;
	if (*p == 'p')
	   p++;
	if (*p == '2')
	   return WC_CES_ISO_2022_JP_2;
	if (*p == '3')
	   return WC_CES_ISO_2022_JP_3;
	return WC_CES_ISO_2022_JP;
    case 's':
	return WC_CES_SHIFT_JIS;
    case 'g':
	return WC_CES_EUC_CN;
    case 'b':
	return WC_CES_BIG5;
    case 'h':
	if (*(p+1) == 'k')
	    return WC_CES_HKSCS;
	return WC_CES_HZ_GB_2312;
    case 'k':
	if (*(p+1) == 'o')
	    return WC_CES_KOI8_R;
	return WC_CES_ISO_2022_KR;
    case 'l':
	n = atoi(p + 1);
	if (n >= 1 && n <= 16 && n != 12)
	    return (WC_CES_E_ISO_8859 | n);
	return WC_CES_ISO_8859_1;
    case 't':
	if (*(p+1) == 'c')
	    return WC_CES_TCVN_5712;
	return WC_CES_TIS_620;
    case 'n':
	return WC_CES_NEXTSTEP;
    case 'v':
	if (*(p+1) == 'p')
	    return WC_CES_VPS;
	return WC_CES_VISCII_11;
#ifdef USE_UNICODE
    case 'u':
	if (*(p+1) == '7')
	    return WC_CES_UTF_7;
	return WC_CES_UTF_8;
#endif
    case 'a':
	return WC_CES_US_ASCII;
    case 'c':
	return WC_CES_ISO_2022_CN;
    case 'w':
	p++;
	if (*p >= '1' &&  *p <= '9')
	    return wc_codepage(atoi(p));
	break;
    case 'r':
	return WC_CES_RAW;
    }
    return 0;
}

wc_ces
wc_locale_to_ces(char *locale)
{
    char *p = locale;
    char buf[8];
    int n;

    if (*p == 'C' && *(p+1) == '\0')
	return WC_CES_US_ASCII;
#ifdef HAVE_LANGINFO_CODESET
    {
	char *cs = nl_langinfo(CODESET);
	if (cs && strcmp(cs, "US-ASCII"))
	    return wc_charset_to_ces(cs);
    }
#endif
    for (n = 0; *p && *p != '.' && n < 7; p++) {
	if ((unsigned char)*p > 0x20)
	    buf[n++] = tolower(*p);
    }
    buf[n] = 0;
    if (*p == '.') {
	p++;
	if (! strcasecmp(p, "euc")) {
	    switch (buf[0]) {
	    case 'j':
		WcLocale = WC_LOCALE_JA_JP;
		break;
	    case 'k':
		WcLocale = WC_LOCALE_KO_KR;
		break;
	    case 'z':
	        if (!strcmp(buf, "zh_tw"))
		    WcLocale = WC_LOCALE_ZH_TW;
	        else if (!strcmp(buf, "zh_hk"))
		    WcLocale = WC_LOCALE_ZH_HK;
		else
		    WcLocale = WC_LOCALE_ZH_CN;
		break;
	    default:
		WcLocale = 0;
		break;
	    }
	}
	return wc_charset_to_ces(p);
    }

    if (!strcmp(buf, "japanese"))
	return WC_CES_SHIFT_JIS;
    if (!strcmp(buf, "zh_tw") ||
	!strcmp(buf, "zh_hk"))
	return WC_CES_BIG5;
    for (n = 0; lang_ces_table[n].lang; n++) {
	if (!strncmp(buf, lang_ces_table[n].lang, 2))
	    return lang_ces_table[n].ces;
    }
    return WC_CES_ISO_8859_1;
}

char *
wc_ces_to_charset(wc_ces ces)
{
    if (ces == WC_CES_WTF)
	return "WTF";
    return WcCesInfo[WC_CES_INDEX(ces)].name;
}

char *
wc_ces_to_charset_desc(wc_ces ces)
{
    if (ces == WC_CES_WTF)
	return "W3M Transfer Format";
    return WcCesInfo[WC_CES_INDEX(ces)].desc;
}

wc_ces
wc_guess_8bit_charset(wc_ces orig)
{
    switch (orig) {
    case WC_CES_ISO_2022_JP:
    case WC_CES_ISO_2022_JP_2:
    case WC_CES_ISO_2022_JP_3:
	return WC_CES_EUC_JP;
    case WC_CES_ISO_2022_KR:
	return WC_CES_EUC_KR;
    case WC_CES_ISO_2022_CN:
    case WC_CES_HZ_GB_2312:
	return WC_CES_EUC_CN;
    case WC_CES_US_ASCII:
	return WC_CES_ISO_8859_1;
    }
    return orig;
}

wc_bool
wc_check_ces(wc_ces ces)
{
    size_t i = WC_CES_INDEX(ces);

    return (i <= WC_CES_END && WcCesInfo[i].id == ces);
}

static int
wc_ces_list_cmp(const void *a, const void *b)
{
    return strcasecmp(((wc_ces_list *)a)->desc, ((wc_ces_list *)b)->desc);
}

static wc_ces_list *list = NULL;

wc_ces_list *
wc_get_ces_list(void)
{
    wc_ces_info *info;
    size_t n;

    if (list)
	return list;
    for (info = WcCesInfo, n = 0; info->id; info++) {
	if (info->name != NULL)
	    n++;
    }
    list = New_N(wc_ces_list, n + 1);
    for (info = WcCesInfo, n = 0; info->id; info++) {
	if (info->name != NULL) {
	    list[n].id = info->id;
	    list[n].name = info->name;
	    list[n].desc = info->desc;
	    n++;
	}
    }
    list[n].id = 0;
    list[n].name = NULL;
    list[n].desc = NULL;
    qsort(list, n, sizeof(wc_ces_list), wc_ces_list_cmp);
    return list;
}
