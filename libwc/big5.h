
#ifndef _WC_BIG5_H
#define _WC_BIG5_H

#define WC_C_BIG5_2_BASE	(0x28 * 0x9D)

#define WC_BIG5_NOSTATE	0
#define WC_BIG5_MBYTE1	1	/* 0xA1 - 0xFE */

#define WC_BIG5_MAP_C0	0x0
#define WC_BIG5_MAP_GL	0x1
#define WC_BIG5_MAP_C1	0x2
#define WC_BIG5_MAP_LB	0x4
#define WC_BIG5_MAP_UB	(0x3 | WC_BIG5_MAP_LB)

#define WC_BIG5UL_N(U,L)	(((U) - 0xA1) * 0x9D \
				+ (L) - (((L) < 0xA1) ? 0x40 : 0x62))
#define WC_BIG5_N(c)	WC_BIG5UL_N(((c) >> 8) & 0xFF, (c) & 0xFF)
#define WC_N_BIG5U(c)	((c) / 0x9D + 0xA1)
#define WC_N_BIG5L(c)	((c) % 0x9D + (((c) % 0x9D < 0x3F) ? 0x40 : 0x62))
#define WC_N_BIG5(c)	((WC_N_BIG5U(c) << 8) + WC_N_BIG5L(c))

extern wc_uchar WC_BIG5_MAP[];

extern wc_wchar_t wc_big5_to_cs94w(wc_wchar_t cc);
extern wc_wchar_t wc_cs94w_to_big5(wc_wchar_t cc);
extern Str        wc_conv_from_big5(Str is, wc_ces ces);
extern void       wc_push_to_big5(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_big5(wc_uchar c, wc_status *st);

#endif
