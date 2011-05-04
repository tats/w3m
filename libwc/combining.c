
#include "wc.h"
#ifdef USE_UNICODE
#include "ucs.h"
#endif
#include "map/iso88596_combining.map"
#include "map/iso885911_combining.map"
#include "map/cp864_combining.map"
#include "map/cp874_combining.map"
#include "map/cp1255_combining.map"
#include "map/cp1256_combining.map"
#include "map/cp1258_combining.map"
#include "map/tcvn5712_combining.map"

wc_bool
wc_is_combining(wc_wchar_t cc)
{
    switch (WC_CCS_SET(cc.ccs)) {
    case WC_CCS_ISO_8859_6:
	return iso88596_combining_map[cc.code & 0x7f];
    case WC_CCS_ISO_8859_11:
	return iso885911_combining_map[cc.code & 0x7f];
    case WC_CCS_CP864:
	return cp864_combining_map[cc.code & 0x7f];
    case WC_CCS_CP874:
	return cp874_combining_map[cc.code & 0x7f];
    case WC_CCS_CP1255:
	return cp1255_combining_map[cc.code & 0x7f];
    case WC_CCS_CP1256:
	return cp1256_combining_map[cc.code & 0x7f];
    case WC_CCS_CP1258_1:
	return cp1258_combining_map[cc.code & 0x7f];
    case WC_CCS_TCVN_5712_1:
	return tcvn5712_combining_map[cc.code & 0x7f];
#ifdef USE_UNICODE
    case WC_CCS_UCS2:
    case WC_CCS_UCS4:
    case WC_CCS_UCS_TAG:
	return wc_is_ucs_combining(cc.code);
#endif
    }
    return WC_FALSE;
}
