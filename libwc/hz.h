
#ifndef _WC_HZ_H
#define _WC_HZ_H

#define WC_C_HZ_TILDA	'~'
#define WC_C_HZ_SI	'{'
#define WC_C_HZ_SO	'}'

#define WC_HZ_NOSTATE	0
#define WC_HZ_TILDA	1
#define WC_HZ_TILDA_MB	2
#define WC_HZ_MBYTE	3
#define WC_HZ_MBYTE1	4
#define WC_HZ_MBYTE1_GR	5

extern Str  wc_conv_from_hz(Str is, wc_ces ces);
extern void wc_push_to_hz(Str os, wc_wchar_t cc, wc_status *st);
extern void wc_push_to_hz_end(Str os, wc_status *st);

#endif
