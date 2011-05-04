
#ifndef _WC_JIS_H
#define _WC_JIS_H

extern wc_uchar  *wc_jisx0212_jisx02132_map;

extern wc_wchar_t wc_jisx0201k_to_jisx0208(wc_wchar_t cc);
extern wc_wchar_t wc_jisx0212_to_jisx0213(wc_wchar_t cc);
extern wc_wchar_t wc_jisx0213_to_jisx0212(wc_wchar_t cc);
extern wc_ccs     wc_jisx0208_or_jisx02131(wc_uint16 code);
extern wc_ccs     wc_jisx0212_or_jisx02132(wc_uint16 code);

#endif
