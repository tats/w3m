
#ifndef _WC_SJIS_H
#define _WC_SJIS_H

#define WC_C_SJIS_ERROR		0xFFFFFFFFU

#define WC_SJIS_NOSTATE	0
#define WC_SJIS_SHIFT_L	1	/* 0xA1 - 0xBF */
#define WC_SJIS_SHIFT_H	2	/* 0xE0 - 0xEF */
#define WC_SJIS_SHIFT_X	3	/* 0xF0 - 0xFC (JIS X 0213-2) */

#define WC_SJIS_MAP_C0	0x0
#define WC_SJIS_MAP_GL	0x1
#define WC_SJIS_MAP_LB	0x10
#define WC_SJIS_MAP_UB	0x20
#define WC_SJIS_MAP_80	(0x2 | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_SK	(0x3 | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_SL	(0x4 | WC_SJIS_MAP_UB | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_SH	(0x5 | WC_SJIS_MAP_UB | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_SX	(0x6 | WC_SJIS_MAP_UB | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_A0	(0x7 | WC_SJIS_MAP_LB)
#define WC_SJIS_MAP_C1	0x40

extern wc_uint8 WC_SJIS_MAP[];

extern wc_wchar_t wc_sjis_to_jis(wc_wchar_t cc);
extern wc_wchar_t wc_jis_to_sjis(wc_wchar_t cc);
extern wc_wchar_t wc_sjis_ext_to_cs94w(wc_wchar_t cc);
extern wc_wchar_t wc_cs94w_to_sjis_ext(wc_wchar_t cc);
extern wc_uint32  wc_sjis_ext1_to_N(wc_uint32 cc);
extern wc_uint32  wc_sjis_ext2_to_N(wc_uint32 cc);
extern Str        wc_conv_from_sjis(Str is, wc_ces ces);
extern Str        wc_conv_from_sjisx0213(Str is, wc_ces ces);
extern void       wc_push_to_sjis(Str os, wc_wchar_t cc, wc_status *st);
extern void       wc_push_to_sjisx0213(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_sjis(wc_uchar c, wc_status *st);
extern Str        wc_char_conv_from_sjisx0213(wc_uchar c, wc_status *st);


#endif
