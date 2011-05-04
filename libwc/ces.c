
#include "wc.h"
#include "iso2022.h"
#include "sjis.h"
#include "hz.h"
#include "big5.h"
#include "hkscs.h"
#include "johab.h"
#include "gbk.h"
#include "gb18030.h"
#include "uhc.h"
#include "viet.h"
#include "priv.h"
#ifdef USE_UNICODE
#include "utf8.h"
#include "utf7.h"
#endif

static wc_gset gset_usascii[] = {
    { WC_CCS_US_ASCII, WC_C_G0_CS94, 1 },
    { 0, 0, 0 },
};

#define gset_iso8859(no) \
static wc_gset gset_iso8859##no[] = { \
    { WC_CCS_US_ASCII,      WC_C_G0_CS94, 1 }, \
    { WC_CCS_ISO_8859_##no, WC_C_G1_CS96 | 0x80, 1 }, \
    { 0, 0, 0 }, \
}
gset_iso8859(1); gset_iso8859(2); gset_iso8859(3); gset_iso8859(4);
gset_iso8859(5); gset_iso8859(6); gset_iso8859(7); gset_iso8859(8);
gset_iso8859(9); gset_iso8859(10); gset_iso8859(11);
gset_iso8859(13); gset_iso8859(14); gset_iso8859(15); gset_iso8859(16);

#define gset_cp(no) gset_priv1(CP##no, cp##no)
#define gset_priv1(ccs, ces) \
static wc_gset gset_##ces[] = { \
    { WC_CCS_US_ASCII, 0, 1 }, \
    { WC_CCS_##ccs,    0x80, 1 }, \
    { 0, 0, 0 }, \
}
gset_cp(437); gset_cp(737); gset_cp(775); gset_cp(850); gset_cp(852);
gset_cp(855); gset_cp(856); gset_cp(857); gset_cp(860); gset_cp(861);
gset_cp(862); gset_cp(863); gset_cp(864); gset_cp(865); gset_cp(866);
gset_cp(869); gset_cp(874); gset_cp(1006);
gset_cp(1250); gset_cp(1251); gset_cp(1252); gset_cp(1253); gset_cp(1254);
gset_cp(1255); gset_cp(1256); gset_cp(1257);
gset_priv1(KOI8_R, koi8r);
gset_priv1(KOI8_U, koi8u);
gset_priv1(NEXTSTEP, nextstep);

static wc_gset gset_iso2022jp[] = {
    { WC_CCS_US_ASCII,     WC_C_G0_CS94, 1 },
    { WC_CCS_JIS_X_0208,   WC_C_G0_CS94, 0 },
    { 0, 0, 0 },
};
static wc_gset gset_iso2022jp2[] = {
    { WC_CCS_US_ASCII,     WC_C_G0_CS94, 1 },
    { WC_CCS_JIS_X_0208,   WC_C_G0_CS94, 0 },
    { WC_CCS_JIS_X_0212,   WC_C_G0_CS94, 0 },
    { WC_CCS_GB_2312,      WC_C_G0_CS94, 0 },
    { WC_CCS_KS_X_1001,    WC_C_G0_CS94, 0 },
    { WC_CCS_ISO_8859_1,   WC_C_G2_CS96, 0 },
    { WC_CCS_ISO_8859_7,   WC_C_G2_CS96, 0 },
    { 0, 0, 0 },
};
static wc_gset gset_iso2022jp3[] = {
    { WC_CCS_US_ASCII,     WC_C_G0_CS94, 1 },
    { WC_CCS_JIS_X_0208,   WC_C_G0_CS94, 0 },
    { WC_CCS_JIS_X_0213_1, WC_C_G0_CS94, 0 },
    { WC_CCS_JIS_X_0213_2, WC_C_G0_CS94, 0 },
    { 0, 0, 0 },
};
static wc_gset gset_iso2022cn[] = {
    { WC_CCS_US_ASCII,    WC_C_G0_CS94, 1 },
    { WC_CCS_GB_2312,     WC_C_G1_CS94, 1 },
    { WC_CCS_ISO_IR_165,  WC_C_G1_CS94, 0 },
    { WC_CCS_CNS_11643_1, WC_C_G1_CS94, 0 },
    { WC_CCS_CNS_11643_2, WC_C_G2_CS94, 0 },
    { WC_CCS_CNS_11643_3, WC_C_G3_CS94, 0 },
    { WC_CCS_CNS_11643_4, WC_C_G3_CS94, 0 },
    { WC_CCS_CNS_11643_5, WC_C_G3_CS94, 0 },
    { WC_CCS_CNS_11643_6, WC_C_G3_CS94, 0 },
    { WC_CCS_CNS_11643_7, WC_C_G3_CS94, 0 },
    { 0, 0, 0 },
};
static wc_gset gset_iso2022kr[] = {
    { WC_CCS_US_ASCII,  WC_C_G0_CS94, 1 },
    { WC_CCS_KS_X_1001, WC_C_G1_CS94, 1 },
    { 0, 0, 0 },
};
static wc_uchar gset_ext_iso2022jp[] = {
    WC_C_G0_CS94, WC_C_G2_CS96, WC_C_G0_CS94, WC_C_G2_CS96
};
static wc_uchar gset_ext_iso2022cn[] = {
    WC_C_G2_CS94, WC_C_G2_CS96, WC_C_G2_CS94, WC_C_G2_CS96
};
static wc_uchar gset_ext_iso2022kr[] = {
    WC_C_G1_CS94, WC_C_G1_CS96, WC_C_G1_CS94, WC_C_G1_CS96
};
static wc_gset gset_eucjp[] = {
    { WC_CCS_US_ASCII,     WC_C_G0_CS94, 1 },
    { WC_CCS_JIS_X_0208,   WC_C_G1_CS94 | 0x80, 1 },
    { WC_CCS_JIS_X_0201K,  WC_C_G2_CS94 | 0x80, 1 },
    { WC_CCS_JIS_X_0213_1, WC_C_G1_CS94 | 0x80, 0 },
    { WC_CCS_JIS_X_0213_2, WC_C_G3_CS94 | 0x80, 0 },
    { WC_CCS_JIS_X_0212,   WC_C_G3_CS94 | 0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_euccn[] = {
    { WC_CCS_US_ASCII, WC_C_G0_CS94, 1 },
    { WC_CCS_GB_2312,  WC_C_G1_CS94 | 0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_euctw[] = {
    { WC_CCS_US_ASCII,     WC_C_G0_CS94, 1 },
    { WC_CCS_CNS_11643_1,  WC_C_G1_CS94 | 0x80, 1 },
    { WC_CCS_CNS_11643_X,  WC_C_G2_CS94 | 0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_euckr[] = {
    { WC_CCS_US_ASCII,  WC_C_G0_CS94, 1 },
    { WC_CCS_KS_X_1001, WC_C_G1_CS94 | 0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_sjis[] = {
    { WC_CCS_US_ASCII,     0, 1 },
    { WC_CCS_JIS_X_0208,   0x80, 1 },
    { WC_CCS_JIS_X_0201K,  0x80, 1 },
    { WC_CCS_SJIS_EXT_1,   0x80, 1 },
    { WC_CCS_SJIS_EXT_2,   0x80, 1 },
    { WC_CCS_SJIS_EXT,     0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_sjisx0213[] = {
    { WC_CCS_US_ASCII,     0, 1 },
    { WC_CCS_JIS_X_0208,   0x80, 1 },
    { WC_CCS_JIS_X_0201K,  0x80, 1 },
    { WC_CCS_JIS_X_0213_1, 0x80, 1 },
    { WC_CCS_JIS_X_0213_2, 0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_hz[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_GB_2312,  0, 0 },
    { 0, 0, 0 },
};
static wc_gset gset_big5[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_BIG5_1,   0x80, 1 },
    { WC_CCS_BIG5_2,   0x80, 1 },
    { WC_CCS_BIG5,     0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_hkscs[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_BIG5_1,   0x80, 1 },
    { WC_CCS_BIG5_2,   0x80, 1 },
    { WC_CCS_BIG5,     0x80, 1 },
    { WC_CCS_HKSCS_1,  0x80, 1 },
    { WC_CCS_HKSCS_2,  0x80, 1 },
    { WC_CCS_HKSCS,    0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_johab[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_JOHAB_1,  0x80, 1 },
    { WC_CCS_JOHAB_2,  0x80, 1 },
    { WC_CCS_JOHAB_3,  0x80, 1 },
    { WC_CCS_JOHAB,    0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_gbk[] = {
    { WC_CCS_US_ASCII,  0, 1 },
    { WC_CCS_GB_2312,   0x80, 1 },
    { WC_CCS_GBK_80,    0x80, 1 },
    { WC_CCS_GBK_1,     0x80, 1 },
    { WC_CCS_GBK_2,     0x80, 1 },
    { WC_CCS_GBK,       0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_gb18030[] = {
    { WC_CCS_US_ASCII,  0, 1 },
    { WC_CCS_GB_2312,   0x80, 1 },
    { WC_CCS_GBK_1,     0x80, 1 },
    { WC_CCS_GBK_2,     0x80, 1 },
    { WC_CCS_GBK,       0x80, 1 },
    { WC_CCS_GBK_EXT_1, 0x80, 1 },
    { WC_CCS_GBK_EXT_2, 0x80, 1 },
    { WC_CCS_GBK_EXT,   0x80, 1 },
    { WC_CCS_GB18030,   0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_uhc[] = {
    { WC_CCS_US_ASCII,  0, 1 },
    { WC_CCS_KS_X_1001, 0x80, 1 },
    { WC_CCS_UHC_1,     0x80, 1 },
    { WC_CCS_UHC_2,     0x80, 1 },
    { WC_CCS_UHC,       0x80, 1 },
    { 0, 0, 0 },
};
#define gset_priv2(ccs, ces) \
static wc_gset gset_##ces[] = { \
    { WC_CCS_US_ASCII, 0, 1 }, \
    { WC_CCS_##ccs##_1, 0x80, 1 }, \
    { WC_CCS_##ccs##_2, 0x80, 1 }, \
    { 0, 0, 0 }, \
}
gset_priv2(CP1258, cp1258);
gset_priv2(VISCII_11, viscii11);
gset_priv2(VPS, vps);
static wc_gset gset_tcvn5712[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_TCVN_5712_1, 0x80, 1 },
    { WC_CCS_TCVN_5712_2, 0x80, 1 },
    { WC_CCS_TCVN_5712_3, 0x80, 1 },
    { 0, 0, 0 },
};

#ifdef USE_UNICODE
static wc_gset gset_utf8[] = {
    { WC_CCS_US_ASCII,  0, 1 },
    { WC_CCS_UCS2,      0x80, 1 },
    { WC_CCS_UCS4,      0x80, 1 },
    { WC_CCS_UCS_TAG,   0x80, 1 },
    { 0, 0, 0 },
};
static wc_gset gset_utf7[] = {
    { WC_CCS_US_ASCII,  0, 1 },
    { WC_CCS_UCS2,      0x80, 1 },
    { WC_CCS_UCS4,      0x80, 1 },
    { WC_CCS_UCS_TAG,   0x80, 1 },
    { 0, 0, 0 },
};
#endif

static wc_gset gset_raw[] = {
    { WC_CCS_US_ASCII, 0, 1 },
    { WC_CCS_RAW,      0x80, 1 },
    { 0, 0, 0 },
};

#define ces_ascii(id,name,desc) \
    { WC_CES_##id, name, desc, gset_usascii, NULL, \
	(void *)wc_conv_from_ascii, (void *)wc_push_to_iso8859, \
	(void *)wc_char_conv_from_iso2022 }
#define ces_iso8859(id,name,desc,no) \
    { WC_CES_##id, name, desc, gset_iso8859##no, NULL, \
	(void *)wc_conv_from_iso2022, (void *)wc_push_to_iso8859, \
	(void *)wc_char_conv_from_iso2022 }
#define ces_priv1(id,name,desc,ces) \
    { WC_CES_##id, name, desc, gset_##ces, NULL, \
	(void *)wc_conv_from_priv1, (void *)wc_push_to_priv1, \
	(void *)wc_char_conv_from_priv1 }
#define ces_iso2022(id,name,desc,terr) \
    { WC_CES_##id, name, desc, gset_iso2022##terr, gset_ext_iso2022##terr, \
	(void *)wc_conv_from_iso2022, (void *)wc_push_to_iso2022, \
	(void *)wc_char_conv_from_iso2022 }
#define ces_euc(id,name,desc,terr) \
    { WC_CES_##id, name, desc, gset_euc##terr, NULL, \
	(void *)wc_conv_from_iso2022, (void *)wc_push_to_euc##terr, \
	(void *)wc_char_conv_from_iso2022 }
#define ces_priv2(id,name,desc,ces) \
    { WC_CES_##id, name, desc, gset_##ces, NULL, \
	(void *)wc_conv_from_##ces, (void *)wc_push_to_##ces, \
	(void *)wc_char_conv_from_##ces }

#define gset_ext_iso2022jp2	gset_ext_iso2022jp
#define gset_ext_iso2022jp3	gset_ext_iso2022jp
#define wc_push_to_euckr	wc_push_to_euc
#define wc_push_to_euccn	wc_push_to_euc
#define wc_push_to_priv1	wc_push_to_iso8859
#define wc_push_to_cp1258	wc_push_to_viet
#define wc_push_to_tcvn5712	wc_push_to_viet
#define wc_push_to_viscii11	wc_push_to_viet
#define wc_push_to_vps		wc_push_to_viet
#define wc_conv_from_cp1258	wc_conv_from_priv1
#define wc_conv_from_tcvn5712	wc_conv_from_viet
#define wc_conv_from_viscii11	wc_conv_from_viet
#define wc_conv_from_vps	wc_conv_from_viet
#define wc_conv_from_raw	wc_conv_from_priv1
#define wc_char_conv_from_hz	wc_char_conv_from_iso2022
#define wc_char_conv_from_cp1258	wc_char_conv_from_priv1
#define wc_char_conv_from_tcvn5712	wc_char_conv_from_viet
#define wc_char_conv_from_viscii11	wc_char_conv_from_viet
#define wc_char_conv_from_vps	wc_char_conv_from_viet
#define wc_char_conv_from_raw	wc_char_conv_from_priv1

wc_ces_info WcCesInfo[] = {
    ces_ascii(US_ASCII, "US-ASCII", "Latin (US-ASCII)"),

    ces_iso8859(ISO_8859_1,  "ISO-8859-1",  "Latin 1 (ISO-8859-1)",        1),
    ces_iso8859(ISO_8859_2,  "ISO-8859-2",  "Latin 2 (ISO-8859-2)",        2),
    ces_iso8859(ISO_8859_3,  "ISO-8859-3",  "Latin 3 (ISO-8859-3)",        3),
    ces_iso8859(ISO_8859_4,  "ISO-8859-4",  "Latin 4 (ISO-8859-4)",        4),
    ces_iso8859(ISO_8859_5,  "ISO-8859-5",  "Cyrillic (ISO-8859-5)",       5),
    ces_iso8859(ISO_8859_6,  "ISO-8859-6",  "Arabic (ISO-8859-6)",         6),
    ces_iso8859(ISO_8859_7,  "ISO-8859-7",  "Greek (ISO-8859-7)",          7),
    ces_iso8859(ISO_8859_8,  "ISO-8859-8",  "Hebrew (ISO-8859-8)",         8),
    ces_iso8859(ISO_8859_9,  "ISO-8859-9",  "Turkish (ISO-8859-9)",        9),
    ces_iso8859(ISO_8859_10, "ISO-8859-10", "Nordic (ISO-8859-10)",        10),
    ces_iso8859(ISO_8859_11, "ISO-8859-11", "Thai (ISO-8859-11, TIS-620)", 11),
    { WC_CES_ISO_8859_12, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    ces_iso8859(ISO_8859_13, "ISO-8859-13", "Baltic Rim (ISO-8859-13)",    13),
    ces_iso8859(ISO_8859_14, "ISO-8859-14", "Celtic (ISO-8859-14)",        14),
    ces_iso8859(ISO_8859_15, "ISO-8859-15", "Latin 9 (ISO-8859-15)",       15),
    ces_iso8859(ISO_8859_16, "ISO-8859-16", "Romanian (ISO-8859-16)",      16),

    ces_iso2022(ISO_2022_JP,   "ISO-2022-JP",   "Japanese (ISO-2022-JP)",   jp),
    ces_iso2022(ISO_2022_JP_2, "ISO-2022-JP-2", "Japanese (ISO-2022-JP-2)", jp2),
    ces_iso2022(ISO_2022_JP_3, "ISO-2022-JP-3", "Japanese (ISO-2022-JP-3)", jp3),
    ces_iso2022(ISO_2022_CN,   "ISO-2022-CN",   "Chinese (ISO-2022-CN)",    cn),
    ces_iso2022(ISO_2022_KR,   "ISO-2022-KR",   "Korean (ISO-2022-KR)",     kr),

    ces_euc(EUC_JP, "EUC-JP", "Japanese (EUC-JP)",        jp),
    ces_euc(EUC_CN, "EUC-CN", "Chinese (EUC-CN, GB2312)", cn),
    ces_euc(EUC_TW, "EUC-TW", "Chinese Taiwan (EUC-TW)",  tw),
    ces_euc(EUC_KR, "EUC-KR", "Korean (EUC-KR)",          kr),

    ces_priv1(CP437,    "CP437",    "Latin (CP437)",         cp437),
    ces_priv1(CP737,    "CP737",    "Greek (CP737)",         cp737),
    ces_priv1(CP775,    "CP775",    "Baltic Rim (CP775)",    cp775),
    ces_priv1(CP850,    "CP850",    "Latin 1 (CP850)",       cp850),
    ces_priv1(CP852,    "CP852",    "Latin 2 (CP852)",       cp852),
    ces_priv1(CP855,    "CP855",    "Cyrillic (CP855)",      cp855),
    ces_priv1(CP856,    "CP856",    "Hebrew (CP856)",        cp856),
    ces_priv1(CP857,    "CP857",    "Turkish (CP857)",       cp857),
    ces_priv1(CP860,    "CP860",    "Portuguese (CP860)",    cp860),
    ces_priv1(CP861,    "CP861",    "Icelandic (CP861)",     cp861),
    ces_priv1(CP862,    "CP862",    "Hebrew (CP862)",        cp862),
    ces_priv1(CP863,    "CP863",    "Canada French (CP863)", cp863),
    ces_priv1(CP864,    "CP864",    "Arabic (CP864)",        cp864),
    ces_priv1(CP865,    "CP865",    "Nordic (CP865)",        cp865),
    ces_priv1(CP866,    "CP866",    "Cyrillic (CP866)",      cp866),
    ces_priv1(CP869,    "CP869",    "Greek 2 (CP869)",       cp869),
    ces_priv1(CP874,    "CP874",    "Thai (CP874)",          cp874),
    ces_priv1(CP1006,   "CP1006",   "Arabic (CP1006)",       cp1006),
    ces_priv1(CP1250,   "CP1250",   "Latin 2 (CP1250)",      cp1250),
    ces_priv1(CP1251,   "CP1251",   "Cyrillic (CP1251)",     cp1251),
    ces_priv1(CP1252,   "CP1252",   "Latin 1 (CP1252)",      cp1252),
    ces_priv1(CP1253,   "CP1253",   "Greek (CP1253)",        cp1253),
    ces_priv1(CP1254,   "CP1254",   "Turkish (CP1254)",      cp1254),
    ces_priv1(CP1255,   "CP1255",   "Hebrew (CP1255)",       cp1255),
    ces_priv1(CP1256,   "CP1256",   "Arabic (CP1256)",       cp1256),
    ces_priv1(CP1257,   "CP1257",   "Baltic Rim (CP1257)",   cp1257),
    ces_priv1(KOI8_R,   "KOI8-R",   "Cyrillic (KOI8-R)",     koi8r),
    ces_priv1(KOI8_U,   "KOI8-U",   "Ukrainian (KOI8-U)",    koi8u),
    ces_priv1(NEXTSTEP, "NeXTSTEP", "NeXTSTEP",              nextstep),

    ces_priv2(RAW, "Raw", "8bit Raw", raw),

    ces_priv2(SHIFT_JIS,  "Shift_JIS",  "Japanese (Shift_JIS, CP932)", sjis),
    ces_priv2(SHIFT_JISX0213, "Shift_JISX0213", "Japanese (Shift_JISX0213)", sjisx0213),
    ces_priv2(GBK,        "GBK",        "Chinese (GBK, CP936)",    gbk),
    ces_priv2(GB18030,    "GB18030",    "Chinese (GB18030)",       gb18030),
    ces_priv2(HZ_GB_2312, "HZ-GB-2312", "Chinese (HZ-GB-2312)",    hz),
    ces_priv2(BIG5,       "Big5",       "Chinese Taiwan (Big5, CP950)", big5),
    ces_priv2(HKSCS,      "HKSCS",      "Chinese Hong Kong (HKSCS)", hkscs),
    ces_priv2(UHC,        "UHC",        "Korean (UHC, CP949)",     uhc),
    ces_priv2(JOHAB,      "Johab",      "Korean (Johab)",          johab),

    ces_priv2(CP1258,     "CP1258",     "Vietnamese (CP1258)",     cp1258),
    ces_priv2(TCVN_5712,  "TCVN-5712",  "Vietnamese (TCVN-5712)",  tcvn5712),
    ces_priv2(VISCII_11,  "VISCII-1.1", "Vietnamese (VISCII 1.1)", viscii11),
    ces_priv2(VPS,        "VPS",        "Vietnamese (VPS)",        vps),

#ifdef USE_UNICODE
    ces_priv2(UTF_8, "UTF-8", "Unicode (UTF-8)", utf8),
    ces_priv2(UTF_7, "UTF-7", "Unicode (UTF-7)", utf7),
#else
    { WC_CES_UTF_8, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { WC_CES_UTF_7, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
#endif
    { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
};
