
#ifndef _WC_UTF8_H
#define _WC_UTF8_H

#define WC_C_UTF8_L2	0x80
#define WC_C_UTF8_L3	0x800
#define WC_C_UTF8_L4	0x10000
#define WC_C_UTF8_L5	0x200000
#define WC_C_UTF8_L6	0x4000000

#define WC_UTF8_NOSTATE	0
#define WC_UTF8_NEXT	1

extern wc_uint8 WC_UTF8_MAP[];

extern size_t    wc_ucs_to_utf8(wc_uint32 ucs, wc_uchar *utf8);
extern wc_uint32 wc_utf8_to_ucs(wc_uchar *utf8);
extern Str       wc_conv_from_utf8(Str is, wc_ces ces);
extern void      wc_push_to_utf8(Str os, wc_wchar_t cc, wc_status *st);
extern void      wc_push_to_utf8_end(Str os, wc_status *st);
extern Str       wc_char_conv_from_utf8(wc_uchar c, wc_status *st);

#endif
