
#ifndef _WC_JOHAB_H
#define _WC_JOHAB_H

#define WC_C_JOHAB_ERROR	0xFFFFFFFFU

#define WC_JOHAB_NOSTATE	0
#define WC_JOHAB_HANGUL1	1	/* 0x84 - 0xD3 */
#define WC_JOHAB_HANJA1		2	/* 0xD8 - 0xDE, 0xE0 - 0xF9 */

#define WC_JOHAB_MAP_C0	0x0
#define WC_JOHAB_MAP_GL	0x1
#define WC_JOHAB_MAP_UJ	0x2
#define WC_JOHAB_MAP_UH	0x4
#define WC_JOHAB_MAP_C1	0x8
#define WC_JOHAB_MAP_LJ	0x10
#define WC_JOHAB_MAP_LH	0x20

#define WC_JOHAB_MAP_GH	(WC_JOHAB_MAP_GL|WC_JOHAB_MAP_LH)
#define WC_JOHAB_MAP_GB	(WC_JOHAB_MAP_GL|WC_JOHAB_MAP_LJ|WC_JOHAB_MAP_LH)
#define WC_JOHAB_MAP_JJ	(WC_JOHAB_MAP_UJ|WC_JOHAB_MAP_LJ)
#define WC_JOHAB_MAP_JB	(WC_JOHAB_MAP_UJ|WC_JOHAB_MAP_LJ|WC_JOHAB_MAP_LH)
#define WC_JOHAB_MAP_HB	(WC_JOHAB_MAP_UH|WC_JOHAB_MAP_LJ|WC_JOHAB_MAP_LH)
#define WC_JOHAB_MAP_CJ	(WC_JOHAB_MAP_C1|WC_JOHAB_MAP_LJ)
#define WC_JOHAB_MAP_CB	(WC_JOHAB_MAP_C1|WC_JOHAB_MAP_LJ|WC_JOHAB_MAP_LH)

#define WC_JOHAB_MAP_1	0xF
#define WC_JOHAB_MAP_2	0x30

#define WC_JOHAB1_N(c)	wc_johab1_to_N(c)
#define WC_N_JOHAB1(c)	wc_N_to_johab1(c)
#define WC_CS94x128UL_N(U,L)	(((U) - 0x21) * 0x80 + (L))
#define WC_CS94x128_N(c)	WC_CS94x128UL_N(((c) >> 8) & 0x7F, (c) & 0x7F)
#define WC_N_CS94x128U(c)	((c) / 0x80 + 0x21)
#define WC_N_CS94x128L(c)	((c) % 0x80)
#define WC_N_CS94x128(c)	((WC_N_CS94x128U(c) << 8) + WC_N_CS94x128L(c))
#define WC_JOHAB2UL_N(U,L)	(((U) - 0x84) * 0xBC + (L) - (((L) < 0x81) ? 0x41 : 0x43))
#define WC_JOHAB2_N(c)		WC_JOHAB2UL_N(((c) >> 8) & 0xFF, (c) & 0xFF)
#define WC_N_JOHAB2U(c)		((c) / 0xBC + 0x84)
#define WC_N_JOHAB2L(c)		((c) % 0xBC + (((c) % 0xBC < 0x3E) ? 0x41 : 0x43))
#define WC_N_JOHAB2(c)		((WC_N_JOHAB2U(c) << 8) + WC_N_JOHAB2L(c))
#ifndef WC_CS128W_N
#define WC_CS128WUL_N(U,L)	((U) * 0x80 + (L))
#define WC_CS128W_N(c)		WC_CS128WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)
#define WC_N_CS128WU(c)		((c) / 0x80)
#define WC_N_CS128WL(c)		((c) % 0x80)
#define WC_N_CS128W(c)		((WC_N_CS128WU(c) << 8) + WC_N_CS128WL(c))
#endif

extern wc_uchar WC_JOHAB_MAP[];

extern wc_wchar_t wc_johab_to_ksx1001(wc_wchar_t cc);
extern wc_wchar_t wc_ksx1001_to_johab(wc_wchar_t cc);
extern wc_wchar_t wc_ucs_to_johab(wc_uint32 ucs);
extern wc_uint32  wc_johab1_to_N(wc_uint32 cc);
extern wc_uint32  wc_N_to_johab1(wc_uint32 ucs);
extern wc_wchar_t wc_johab_to_cs128w(wc_wchar_t cc);
extern wc_wchar_t wc_cs128w_to_johab(wc_wchar_t cc);
extern Str        wc_conv_from_johab(Str is, wc_ces ces);
extern void       wc_push_to_johab(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_johab(wc_uchar c, wc_status *st);

#endif
