/* $Id: entity.c,v 1.7 2003/09/24 18:48:59 ukai Exp $ */
#ifdef DUMMY
#include "Str.h"
#define NBSP " "
#define UseAltEntity 1
#undef USE_M17N
#else				/* DUMMY */
#include "fm.h"
#ifdef USE_M17N
#ifdef USE_UNICODE
#include "ucs.h"
#include "utf8.h"
#endif
#endif
#endif				/* DUMMY */

/* *INDENT-OFF* */
static char *alt_latin1[ 96 ] = {
    NBSP,  "!",   "-c-", "-L-", "CUR", "=Y=",  "|",  "S:",
    "\"",  "(C)", "-a",  "<<",  "NOT", "-",   "(R)", "-",
    "DEG", "+-",  "^2",  "^3",   "'",  "u",   "P:",  ".",
    ",",   "^1",  "-o",  ">>",  "1/4", "1/2", "3/4", "?", 
    "A`",  "A'",  "A^",  "A~",  "A:",  "AA",  "AE",  "C,",
    "E`",  "E'",  "E^",  "E:",  "I`",  "I'",  "I^",  "I:",
    "D-",  "N~",  "O`",  "O'",  "O^",  "O~",  "O:",  "x",
    "O/",  "U`",  "U'",  "U^",  "U:",  "Y'",  "TH",  "ss",
    "a`",  "a'",  "a^",  "a~",  "a:",  "aa",  "ae",  "c,", 
    "e`",  "e'",  "e^",  "e:",  "i`",  "i'",  "i^",  "i:",
    "d-",  "n~",  "o`",  "o'",  "o^",  "o~",  "o:",  "-:",
    "o/",  "u`",  "u'",  "u^",  "u:",  "y'",  "th",  "y:"
};
/* *INDENT-ON* */

char *
conv_entity(unsigned int c)
{
    char b = c & 0xff;

    if (c < 0x20)		/* C0 */
	return " ";
    if (c < 0x7f)		/* ASCII */
	return Strnew_charp_n(&b, 1)->ptr;
    if (c < 0xa0)		/* DEL, C1 */
	return " ";
    if (c == 0xa0)
	return NBSP;
    if (c < 0x100) {		/* Latin1 (ISO 8859-1) */
	if (UseAltEntity)
	    return alt_latin1[c - 0xa0];
#ifdef USE_M17N
	return wc_conv_n(&b, 1, WC_CES_ISO_8859_1, InnerCharset)->ptr;
#else
	return Strnew_charp_n(&b, 1)->ptr;
#endif
    }
#ifdef USE_M17N
#ifdef USE_UNICODE
    if (c <= WC_C_UCS4_END) {	/* Unicode */
	wc_uchar utf8[7];
	wc_ucs_to_utf8(c, utf8);
	return wc_conv((char *)utf8, WC_CES_UTF_8, InnerCharset)->ptr;
    }
#endif
#endif
    return "?";
}
