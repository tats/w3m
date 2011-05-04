
#ifndef _WC_WC_H
#define _WC_WC_H

#include <Str.h>
#include "wc_types.h"
#include "ces.h"

#define WC_FALSE 0
#define WC_TRUE 1

#define WC_OPT_DETECT_OFF	0
#define WC_OPT_DETECT_ISO_2022	1
#define WC_OPT_DETECT_ON	2

#define WC_LOCALE_JA_JP	1
#define WC_LOCALE_ZH_CN	2
#define WC_LOCALE_ZH_TW	3
#define WC_LOCALE_ZH_HK	4
#define WC_LOCALE_KO_KR	5

extern wc_uint8 WC_DETECT_MAP[];

extern wc_ces_info WcCesInfo[];
extern wc_option WcOption;
extern wc_locale WcLocale;
extern char *WcReplace;
extern char *WcReplaceW;
#define WC_REPLACE   WcReplace
#define WC_REPLACE_W WcReplaceW

#define wc_conv(is, f_ces, t_ces) \
	wc_Str_conv(Strnew_charp((is)), (f_ces), (t_ces))
#define wc_conv_n(is, n, f_ces, t_ces) \
	wc_Str_conv(Strnew_charp_n((is), (n)), (f_ces), (t_ces))
#define wc_conv_strict(is, f_ces, t_ces) \
	wc_Str_conv_strict(Strnew_charp((is)), (f_ces), (t_ces))
#define wc_conv_n_strict(is, n, f_ces, t_ces)\
	wc_Str_conv_strict(Strnew_charp_n((is), (n)), (f_ces), (t_ces))
#define wc_conv_with_detect(is, f_ces, hint, t_ces) \
	wc_Str_conv_with_detect(Strnew_charp((is)), (f_ces), (hint), (t_ces))
#define wc_conv_n_with_detect(is, n, f_ces, hint, t_ces)\
	wc_Str_conv_with_detect(Strnew_charp_n((is), (n)), (f_ces), (hint), (t_ces))

extern Str wc_Str_conv(Str is, wc_ces f_ces, wc_ces t_ces);
extern Str wc_Str_conv_strict(Str is, wc_ces f_ces, wc_ces t_ces);
extern Str wc_Str_conv_with_detect(Str is, wc_ces *f_ces, wc_ces hint, wc_ces t_ces);

extern void wc_input_init(wc_ces ces, wc_status *st);
extern void wc_output_init(wc_ces ces, wc_status *st);
extern void wc_push_end(Str os, wc_status *st);
extern wc_bool wc_ces_has_ccs(wc_ccs ccs, wc_status *st);

extern void wc_char_conv_init(wc_ces f_ces, wc_ces t_ces);
extern Str  wc_char_conv(char c);

extern void wc_putc_init(wc_ces f_ces, wc_ces t_ces);
extern void wc_putc(char *c, FILE *f);
extern void wc_putc_end(FILE *f);
extern void wc_putc_clear_status(void);

extern void   wc_create_detect_map(wc_ces ces, wc_bool esc);
extern wc_ces wc_auto_detect(char *is, size_t len, wc_ces hint);

extern wc_ces       wc_guess_charset(char *charset, wc_ces orig);
extern wc_ces       wc_guess_charset_short(char *charset, wc_ces orig);
extern wc_ces       wc_guess_locale_charset(char *locale, wc_ces orig);
extern wc_ces       wc_charset_to_ces(char *charset);
extern wc_ces       wc_charset_short_to_ces(char *charset);
extern wc_ces       wc_locale_to_ces(char *locale);
extern wc_ces       wc_guess_8bit_charset(wc_ces orig);
extern char        *wc_ces_to_charset(wc_ces ces);
extern char        *wc_ces_to_charset_desc(wc_ces ces);
extern wc_bool      wc_check_ces(wc_ces ces);
extern wc_ces_list *wc_get_ces_list(void);

#endif
