
#ifndef _WC_UHC_H
#define _WC_UHC_H

#define WC_C_UHC_END	0xC6FE

#define WC_UHC_NOSTATE	0
#define WC_UHC_MBYTE1	1	/* 0x81 - 0xA0, 0xA1 - 0xFE */

#define WC_UHC_MAP_C0	0x0
#define WC_UHC_MAP_GL	0x1
#define WC_UHC_MAP_C1	0x2
#define WC_UHC_MAP_LB	0x4
#define WC_UHC_MAP_UB	(0x8 | WC_UHC_MAP_LB)

#define WC_UHCUL_N(U,L)		(((U) - 0x81) * 0xB2 + (L) - (((L) < 0x61) ? 0x41 : (((L) < 0x81) ? 0x47 : 0x4D)))
#define WC_UHC_N(c)		WC_UHCUL_N(((c) >> 8) & 0xFF, (c) & 0xFF)
#define WC_N_UHCU(c)		((c) / 0xB2 + 0x81)
#define WC_N_UHCL(c)		((c) % 0xB2 + (((c) % 0xB2 < 0x1A) ? 0x41 : (((c) % 0xB2 < 0x34) ? 0x47 : 0x4D)))
#define WC_N_UHC(c)		((WC_N_UHCU(c) << 8) + WC_N_UHCL(c))
#ifndef WC_CS128W_N
#define WC_CS128WUL_N(U,L)	((U) * 0x80 + (L))
#define WC_CS128W_N(c)		WC_CS128WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)
#define WC_N_CS128WU(c)		((c) / 0x80)
#define WC_N_CS128WL(c)		((c) % 0x80)
#define WC_N_CS128W(c)		((WC_N_CS128WU(c) << 8) + WC_N_CS128WL(c))
#endif

extern wc_uchar WC_UHC_MAP[];

extern wc_wchar_t wc_uhc_to_cs128w(wc_wchar_t cc);
extern wc_wchar_t wc_cs128w_to_uhc(wc_wchar_t cc);
extern wc_uint32  wc_uhc_to_N(wc_uint32 c);
extern Str        wc_conv_from_uhc(Str is, wc_ces ces);
extern void       wc_push_to_uhc(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_uhc(wc_uchar c, wc_status *st);

#endif
