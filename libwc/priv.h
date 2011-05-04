
#ifndef _WC_PRIV_H
#define _WC_PRIV_H

#define WC_F_SPECIAL	0x00
#define WC_F_CP437	0x01
#define WC_F_CP737	0x02
#define WC_F_CP775	0x03
#define WC_F_CP850	0x04
#define WC_F_CP852	0x05
#define WC_F_CP855	0x06
#define WC_F_CP856	0x07
#define WC_F_CP857	0x08
#define WC_F_CP860	0x09
#define WC_F_CP861	0x0A
#define WC_F_CP862	0x0B
#define WC_F_CP863	0x0C
#define WC_F_CP864	0x0D
#define WC_F_CP865	0x0E
#define WC_F_CP866	0x0F
#define WC_F_CP869	0x10
#define WC_F_CP874	0x11
#define WC_F_CP1006	0x12
#define WC_F_CP1250	0x13
#define WC_F_CP1251	0x14
#define WC_F_CP1252	0x15
#define WC_F_CP1253	0x16
#define WC_F_CP1254	0x17
#define WC_F_CP1255	0x18
#define WC_F_CP1256	0x19
#define WC_F_CP1257	0x1A
#define WC_F_CP1258_1	0x1B
#define WC_F_CP1258_2	0x1C
#define WC_F_TCVN_5712_1	0x1D
#define WC_F_TCVN_5712_2	0x1E
#define WC_F_TCVN_5712_3	0x1F
#define WC_F_VISCII_11_1	0x20
#define WC_F_VISCII_11_2	0x21
#define WC_F_VPS_1		0x22
#define WC_F_VPS_2		0x23
#define WC_F_KOI8_R		0x24
#define WC_F_KOI8_U		0x25
#define WC_F_NEXTSTEP		0x26
#define WC_F_GBK_80		0x27
#define WC_F_RAW		0x28

#define WC_F_SPECIAL_W		0x00
#define WC_F_BIG5		0x01
#define WC_F_BIG5_1		0x02
#define WC_F_BIG5_2		0x03
#define WC_F_CNS_11643_8	0x04
#define WC_F_CNS_11643_9	0x05
#define WC_F_CNS_11643_10	0x06
#define WC_F_CNS_11643_11	0x07
#define WC_F_CNS_11643_12	0x08
#define WC_F_CNS_11643_13	0x09
#define WC_F_CNS_11643_14	0x0A
#define WC_F_CNS_11643_15	0x0B
#define WC_F_CNS_11643_16	0x0C
#define WC_F_CNS_11643_X	0x0D
#define WC_F_GB_12345		0x0E
#define WC_F_JOHAB		0x0F
#define WC_F_JOHAB_1		0x10
#define WC_F_JOHAB_2		0x11
#define WC_F_JOHAB_3		0x12
#define WC_F_SJIS_EXT		0x13
#define WC_F_SJIS_EXT_1		0x14
#define WC_F_SJIS_EXT_2		0x15
#define WC_F_GBK		0x16
#define WC_F_GBK_1		0x17
#define WC_F_GBK_2		0x18
#define WC_F_GBK_EXT		0x19
#define WC_F_GBK_EXT_1		0x1A
#define WC_F_GBK_EXT_2		0x1B
#define WC_F_UHC		0x1C
#define WC_F_UHC_1		0x1D
#define WC_F_UHC_2		0x1E
#define WC_F_HKSCS		0x1F
#define WC_F_HKSCS_1		0x20
#define WC_F_HKSCS_2		0x21

#define WC_F_UCS2		0x00
#define WC_F_UCS4		0x00
#define WC_F_UCS_TAG		0x01
#define WC_F_GB18030		0x02

#define WC_F_C1			0x01

extern Str  wc_conv_from_priv1(Str is, wc_ces ces);
extern Str  wc_char_conv_from_priv1(wc_uchar c, wc_status *st);
extern Str  wc_conv_from_ascii(Str is, wc_ces ces);
extern void wc_push_to_raw(Str os, wc_wchar_t cc, wc_status *st);

#endif
