
#ifndef _WC_HKSCS_H
#define _WC_HKSCS_H

#define WC_HKSCS_NOSTATE	0
#define WC_HKSCS_MBYTE1	1	/* 0x88 - 0xFE */

#define WC_HKSCS_MAP_C0	0x0
#define WC_HKSCS_MAP_GL	0x1
#define WC_HKSCS_MAP_C1	0x2
#define WC_HKSCS_MAP_LB	0x4
#define WC_HKSCS_MAP_UB	(0x8  | WC_HKSCS_MAP_LB)
#define WC_HKSCS_MAP_UH	0x10

#define WC_HKSCSUL_N(U,L)	(((U) - 0x88) * 0x9D \
				+ (L) - (((L) < 0xA1) ? 0x40 : 0x62))
#define WC_HKSCS_N(c)	WC_HKSCSUL_N(((c) >> 8) & 0xFF, (c) & 0xFF)
#define WC_N_HKSCSU(c)	((c) / 0x9D + 0x88)
#define WC_N_HKSCSL(c)	((c) % 0x9D + (((c) % 0x9D < 0x3F) ? 0x40 : 0x62))
#define WC_N_HKSCS(c)	((WC_N_HKSCSU(c) << 8) + WC_N_HKSCSL(c))
#ifndef WC_CS128W_N
#define WC_CS128WUL_N(U,L)	((U) * 0x80 + (L))
#define WC_CS128W_N(c)		WC_CS128WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)
#define WC_N_CS128WU(c)		((c) / 0x80)
#define WC_N_CS128WL(c)		((c) % 0x80)
#define WC_N_CS128W(c)		((WC_N_CS128WU(c) << 8) + WC_N_CS128WL(c))
#endif

extern wc_uchar WC_HKSCS_MAP[];

extern wc_wchar_t wc_hkscs_to_cs128w(wc_wchar_t cc);
extern wc_wchar_t wc_cs128w_to_hkscs(wc_wchar_t cc);
extern wc_uint32  wc_hkscs_to_N(wc_uint32 c);
extern Str        wc_conv_from_hkscs(Str is, wc_ces ces);
extern void       wc_push_to_hkscs(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_hkscs(wc_uchar c, wc_status *st);

#endif
