
#ifndef _WC_GBK_H
#define _WC_GBK_H

#define WC_GBK_NOSTATE	0
#define WC_GBK_MBYTE1	1	/* 0x81 - 0xA0, 0xA1 - 0xFE */

#define WC_GBK_MAP_C0	0x0
#define WC_GBK_MAP_GL	0x1
#define WC_GBK_MAP_C1	0x2
#define WC_GBK_MAP_LB	0x4
#define WC_GBK_MAP_UB	(0x8  | WC_GBK_MAP_LB)
#define WC_GBK_MAP_80	(0x10 | WC_GBK_MAP_LB)

#define WC_GBKUL_N(U,L)		(((U) - 0x81) * 0xBE + (L) - (((L) < 0x80) ? 0x40 : 0x41))
#define WC_GBK_N(c)		WC_GBKUL_N(((c) >> 8) & 0xFF, (c) & 0xFF)
#define WC_N_GBKU(c)		((c) / 0xBE + 0x81)
#define WC_N_GBKL(c)		((c) % 0xBE + (((c) % 0xBE < 0x3F) ? 0x40 : 0x41))
#define WC_N_GBK(c)		((WC_N_GBKU(c) << 8) + WC_N_GBKL(c))
#ifndef WC_CS128W_N
#define WC_CS128WUL_N(U,L)	((U) * 0x80 + (L))
#define WC_CS128W_N(c)		WC_CS128WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)
#define WC_N_CS128WU(c)		((c) / 0x80)
#define WC_N_CS128WL(c)		((c) % 0x80)
#define WC_N_CS128W(c)		((WC_N_CS128WU(c) << 8) + WC_N_CS128WL(c))
#endif

extern wc_uchar WC_GBK_MAP[];

extern wc_ccs     wc_gb2312_or_gbk(wc_uint16 code);
extern wc_wchar_t wc_gbk_to_cs128w(wc_wchar_t cc);
extern wc_wchar_t wc_cs128w_to_gbk(wc_wchar_t cc);
extern wc_uint32  wc_gbk_to_N(wc_uint32 c);
extern Str        wc_conv_from_gbk(Str is, wc_ces ces);
extern void       wc_push_to_gbk(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_gbk(wc_uchar c, wc_status *st);

#endif
