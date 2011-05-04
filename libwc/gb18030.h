
#ifndef _WC_GB18030_H
#define _WC_GB18030_H

#include "gbk.h"

#define WC_C_GB18030_UCS2	0x81308130U
#define WC_C_GB18030_UCS2_END	0x8431A439U
#define WC_C_GB18030_UCS4	0x90308130U
#define WC_C_GB18030_UCS4_END	0xE3329A35U

#define WC_GB18030_NOSTATE	0
#define WC_GB18030_MBYTE1	1	/* 0x81 - 0xA0, 0xA1 - 0xFE */
#define WC_GB18030_MBYTE2	2	/* 0x30 - 0x39 */
#define WC_GB18030_MBYTE3	3	/* 0x81 - 0xA0, 0xA1 - 0xFE */

#define WC_GB18030_MAP_C0	0x0
#define WC_GB18030_MAP_GL	0x1
#define WC_GB18030_MAP_C1	0x2
#define WC_GB18030_MAP_LB	0x4
#define WC_GB18030_MAP_UB	(0x8  | WC_GB18030_MAP_LB)
#define WC_GB18030_MAP_L4	0x10

#define WC_GB18030_N(c)	\
	(((((((c) >> 24) & 0xff) - 0x81) * 0x0A \
	  + (((c) >> 16) & 0xff) - 0x30) * 0x7E \
	  + (((c) >>  8) & 0xff) - 0x81) * 0x0A \
	  + ( (c)        & 0xff) - 0x30)
#define WC_N_GB18030(c)	\
	 ((((c) / 0x0A / 0x7E / 0xA + 0x81) << 24) \
	+ (((c) / 0x0A / 0x7E % 0xA + 0x30) << 16) \
	+ (((c) / 0x0A % 0x7E       + 0x81) <<  8) \
	+   (c) % 0xA               + 0x30        )

extern wc_uchar WC_GB18030_MAP[];

extern wc_wchar_t wc_gbk_ext_to_cs128w(wc_wchar_t cc);
extern wc_wchar_t wc_cs128w_to_gbk_ext(wc_wchar_t cc);
#ifdef USE_UNICODE
extern wc_uint32  wc_gb18030_to_ucs(wc_wchar_t cc);
extern wc_wchar_t wc_ucs_to_gb18030(wc_uint32 ucs);
#endif
extern Str        wc_conv_from_gb18030(Str is, wc_ces ces);
extern void       wc_push_to_gb18030(Str os, wc_wchar_t cc, wc_status *st);
extern Str        wc_char_conv_from_gb18030(wc_uchar c, wc_status *st);

#endif
