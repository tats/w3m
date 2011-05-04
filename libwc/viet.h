
#ifndef _WC_VIET_H
#define _WC_VIET_H

extern wc_uint8 wc_c0_tcvn57122_map[];
extern wc_uint8 wc_c0_viscii112_map[];
extern wc_uint8 wc_c0_vps2_map[];

extern Str       wc_conv_from_viet(Str is, wc_ces ces);
extern void      wc_push_to_viet(Str os, wc_wchar_t cc, wc_status *st);
extern void      wc_push_to_cp1258(Str os, wc_wchar_t cc, wc_status *st);
extern wc_wchar_t wc_tcvn57123_to_tcvn5712(wc_wchar_t cc);
extern wc_uint32 wc_tcvn5712_precompose(wc_uchar c1, wc_uchar c2);
extern wc_uint32 wc_cp1258_precompose(wc_uchar c1, wc_uchar c2);
extern Str       wc_char_conv_from_viet(wc_uchar c, wc_status *st);

#endif
