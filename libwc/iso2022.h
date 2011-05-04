
#ifndef _WC_ISO2022_H
#define _WC_ISO2022_H

#define WC_C_ESC	0x1B	/* '\033' */
#define WC_C_SS2	0x4E	/* ESC 'N' */
#define WC_C_SS3	0x4F	/* ESC 'O' */
#define WC_C_LS2	0x6E	/* ESC 'n' */
#define WC_C_LS3	0x6F	/* ESC 'o' */
#define WC_C_LS1R	0x7E	/* ESC '~' */
#define WC_C_LS2R	0x7D	/* ESC '}' */
#define WC_C_LS3R	0x7C	/* ESC '|' */
#define WC_C_G0_CS94	0x28	/* ESC '(' F */
#define WC_C_G1_CS94	0x29	/* ESC ')' F */
#define WC_C_G2_CS94	0x2A	/* ESC '*' F */
#define WC_C_G3_CS94	0x2B	/* ESC '+' F */
#define WC_C_G0_CS96	0x2C	/* ESC ',' F */ /* ISO 2022 does not permit */
#define WC_C_G1_CS96	0x2D	/* ESC '-' F */
#define WC_C_G2_CS96	0x2E	/* ESC '.' F */
#define WC_C_G3_CS96	0x2F	/* ESC '/' F */
#define WC_C_MBCS	0x24	/* ESC '$' G F */
#define WC_C_CS942	0x21	/* ESC G '!' F */
#define WC_C_C0		0x21	/* ESC '!' F */
#define WC_C_C1		0x22	/* ESC '"' F */
#define WC_C_REP	0x26	/* ESC '&' F ESC '"' F */
#define WC_C_CSWSR	0x25	/* ESC '%' F */
#define WC_C_CSWOSR	0x2F	/* ESC '%' '/' F */

#define WC_C_SO		0x0E	/* '\016' */
#define WC_C_SI		0x0F	/* '\017' */
#define WC_C_SS2R	0x8E
#define WC_C_SS3R	0x8F

#define WC_F_ISO_646_US		0x42	/* 'B' */
#define WC_F_ISO_646_IRV	WC_F_ISO_646_US
#define WC_F_US_ASCII		WC_F_ISO_646_US
#define WC_F_JIS_X_0201K	0x49	/* 'I' */
#define WC_F_JIS_X_0201		0x4A	/* 'J' */
#define WC_F_GB_1988		0x54	/* 'T' */

#define WC_F_ISO_8859_1		0x41	/* 'A' */
#define WC_F_ISO_8859_2		0x42	/* 'B' */
#define WC_F_ISO_8859_3		0x43	/* 'C' */
#define WC_F_ISO_8859_4		0x44	/* 'D' */
#define WC_F_ISO_8859_5		0x4C	/* 'L' */
#define WC_F_ISO_8859_6		0x47	/* 'G' */
#define WC_F_ISO_8859_7		0x46	/* 'F' */
#define WC_F_ISO_8859_8		0x48	/* 'H' */
#define WC_F_ISO_8859_9		0x4D	/* 'M' */
#define WC_F_ISO_8859_10	0x56	/* 'V' */
#define WC_F_ISO_8859_11	0x54	/* 'T' */
#define WC_F_TIS_620		WC_F_ISO_8859_11
#define WC_F_ISO_8859_13	0x59	/* 'Y' */
#define WC_F_ISO_8859_14	0x5F	/* '_' */
#define WC_F_ISO_8859_15	0x62	/* 'b' */
#define WC_F_ISO_8859_16	0x66	/* 'f' */

#define WC_F_JIS_C_6226		0x40	/* '@' */
#define WC_F_GB_2312		0x41	/* 'A' */
#define WC_F_JIS_X_0208		0x42	/* 'B' */
#define WC_F_KS_X_1001		0x43	/* 'C' */
#define WC_F_KS_C_5601		WC_F_KS_X_1001
#define WC_F_JIS_X_0212		0x44	/* 'D' */
#define WC_F_ISO_IR_165		0x45	/* 'E' */
#define WC_F_CCITT_GB		WC_F_ISO_IR_165
#define WC_F_CNS_11643_1	0x47	/* 'G' */
#define WC_F_CNS_11643_2	0x48	/* 'H' */
#define WC_F_CNS_11643_3	0x49	/* 'I' */
#define WC_F_CNS_11643_4	0x4A	/* 'J' */
#define WC_F_CNS_11643_5	0x4B	/* 'K' */
#define WC_F_CNS_11643_6	0x4C	/* 'L' */
#define WC_F_CNS_11643_7	0x4D	/* 'M' */
#define WC_F_KPS_9566		0x4E	/* 'N' */
#define WC_F_JIS_X_0213_1	0x4F	/* 'O' */
#define WC_F_JIS_X_0213_2	0x50	/* 'P' */

#define WC_ISO_NOSTATE		0
#define WC_ISO_MBYTE1		1
#define WC_EUC_NOSTATE		0
#define WC_EUC_MBYTE1		2	/* for EUC (G1) */
#define WC_EUC_TW_SS2		3	/* for EUC_TW (G2) */
#define WC_EUC_TW_MBYTE1	4	/* for EUC_TW (G2) */
#define WC_EUC_TW_MBYTE2	5	/* for EUC_TW (G2) */
#define WC_ISO_ESC		6
#define WC_ISO_CSWSR		0x10
#define WC_ISO_CSWOSR		0x20

#define WC_ISO_MAP_CG   0xF0
#define WC_ISO_MAP_C0	0x10			/* 0x00 - 0x1F */
#define WC_ISO_MAP_GL	0x00			/* 0x21 - 0x7E */
#define WC_ISO_MAP_GL96	0x20			/* 0x20,  0x7F */
#define WC_ISO_MAP_C1	0x50			/* 0x80 - 0x9F */
#define WC_ISO_MAP_GR	0x40			/* 0xA1 - 0xFE */
#define WC_ISO_MAP_GR96	0x60			/* 0xA0,  0xFF */
#define WC_ISO_MAP_SO	(0x1 | WC_ISO_MAP_C0)	/* 0x0E */
#define WC_ISO_MAP_SI	(0x2 | WC_ISO_MAP_C0)	/* 0x0F */
#define WC_ISO_MAP_ESC	(0x3 | WC_ISO_MAP_C0)	/* 0x1B */
#define WC_ISO_MAP_SS2	(0x4 | WC_ISO_MAP_C1)	/* 0x8E */
#define WC_ISO_MAP_SS3	(0x5 | WC_ISO_MAP_C1)	/* 0x8F */
#define WC_ISO_MAP_DETECT	0x4F

#define WC_CS94WUL_N(U,L)	(((U) - 0x21) * 0x5E + (L) - 0x21)
#define WC_CS94W_N(c)	WC_CS94WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)     
#define WC_CS96WUL_N(U,L)	(((U) - 0x20) * 0x60 + (L) - 0x20)
#define WC_CS96W_N(c)	WC_CS96WUL_N(((c) >> 8) & 0x7F, (c) & 0x7F)     
#define WC_N_CS94WU(c)	((c) / 0x5E + 0x21)
#define WC_N_CS94WL(c)	((c) % 0x5E + 0x21)
#define WC_N_CS94W(c)	((WC_N_CS94WU(c) << 8) + WC_N_CS94WL(c))
#define WC_N_CS96WU(c)	((c) / 0x60 + 0x20)
#define WC_N_CS96WL(c)	((c) % 0x60 + 0x20)
#define WC_N_CS96W(c)	((WC_N_CS96WU(c) << 8) + WC_N_CS96WL(c))

extern wc_uint8 WC_ISO_MAP[];

extern Str  wc_conv_from_iso2022(Str is, wc_ces ces);
extern void wc_push_to_iso2022(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_euc(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_eucjp(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_euctw(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_iso8859(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_iso2022_end(Str os, wc_status *st);
extern int  wc_parse_iso2022_esc(wc_uchar **ptr, wc_status *st);
extern void wc_push_iso2022_esc(Str os, wc_ccs ccs, wc_uchar g, wc_uint8 invoke, wc_status *st);
extern void wc_create_gmap(wc_status *st);
extern Str  wc_char_conv_from_iso2022(wc_uchar c, wc_status *st);

#endif
