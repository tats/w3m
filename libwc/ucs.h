
#ifndef _WC_UCS_H
#define _WC_UCS_H

#define WC_C_UCS2_NBSP		0xA0
#define WC_C_UCS2_BOM		0xFEFF
#define WC_C_UCS2_REPLACE	0xFFFD
#define WC_C_UCS2_END		0xFFFF
#define WC_C_UCS2_SURROGATE	0xD800
#define WC_C_UCS2_SURROGATE_LOW	0xDC00
#define WC_C_UCS2_SURROGATE_END	0xDFFF
#define WC_C_UCS2_HANGUL	0xAC00
#define WC_C_UCS2_HANGUL_END	0xD7A3
#define WC_C_UCS2_EURO		0x20AC
#define WC_C_UCS4_END		0x7FFFFFFF
#define WC_C_UCS4_ERROR		0xFFFFFFFFU
#define WC_C_UNICODE_END	0x10FFFF
#define WC_C_UNICODE_MASK	0x1FFFFF
#define WC_C_LANGUAGE_TAG0	0xE0000
#define WC_C_LANGUAGE_TAG	0xE0001
#define WC_C_TAG_SPACE		0xE0020
#define WC_C_CANCEL_TAG		0xE007F
#define WC_C_UCS4_PLANE1	0x10000
#define WC_C_UCS4_PLANE2	0x20000
#define WC_C_UCS4_PLANE3	0x30000

#define wc_ucs_tag_to_ucs(c)		((c) & WC_C_UNICODE_MASK)
#define wc_ucs_tag_to_tag(c)		((c) >> 24)
#define wc_ucs_to_ucs_tag(c,tag)	((c) | ((tag) << 24))
#define wc_ccs_ucs_to_ccs_ucs_tag(ccs)	(WC_CCS_UCS_TAG | ((ccs) & ~WC_CCS_A_SET))
#define wc_ucs_to_utf16(ucs) \
	((((((ucs) - WC_C_UCS4_PLANE1) >> 10) | WC_C_UCS2_SURROGATE) << 16) \
	| ((((ucs) - WC_C_UCS4_PLANE1) & 0x3ff) | WC_C_UCS2_SURROGATE_LOW))
#define wc_utf16_to_ucs(high, low) \
	(((((high) & 0x3ff) << 10) | ((low) & 0x3ff)) + WC_C_UCS4_PLANE1)

extern wc_table  *wc_get_ucs_table(wc_ccs ccs);
extern wc_wchar_t wc_ucs_to_any(wc_uint32 ucs, wc_table *t);
extern wc_uint32  wc_any_to_ucs(wc_wchar_t cc);
extern wc_wchar_t wc_any_to_any(wc_wchar_t cc, wc_table *t);
extern wc_wchar_t wc_ucs_to_any_list(wc_uint32 ucs, wc_table **tlist);
extern wc_wchar_t wc_any_to_any_ces(wc_wchar_t cc, wc_status *st);
extern wc_wchar_t wc_any_to_iso2022(wc_wchar_t cc, wc_status *st);
extern wc_wchar_t wc_ucs_to_iso2022(wc_uint32 ucs);
extern wc_wchar_t wc_ucs_to_iso2022w(wc_uint32 ucs);
extern wc_ccs     wc_ucs_to_ccs(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_ambiguous_width(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_wide(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_combining(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_hangul(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_alpha(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_digit(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_alnum(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_lower(wc_uint32 ucs);
extern wc_bool    wc_is_ucs_upper(wc_uint32 ucs);
extern wc_uint32  wc_ucs_toupper(wc_uint32 ucs);
extern wc_uint32  wc_ucs_tolower(wc_uint32 ucs);
extern wc_uint32  wc_ucs_totitle(wc_uint32 ucs);
extern wc_uint32  wc_ucs_precompose(wc_uint32 ucs1, wc_uint32 ucs2);
extern wc_uint32  wc_ucs_to_fullwidth(wc_uint32 ucs);
extern int        wc_ucs_put_tag(char *tag);
extern char      *wc_ucs_get_tag(int ntag);
extern void       wtf_push_ucs(Str os, wc_uint32 ucs, wc_status *st);

#endif
