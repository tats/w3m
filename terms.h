#ifndef TERMS_H
#define TERMS_H

extern int LINES, COLS;

extern char DisplayCode;

#define CODE_ASCII	'\0'
#define CODE_EUC	'E'
#define CODE_SJIS	'S'
#define CODE_JIS_n	'n'
#define CODE_JIS_m	'm'
#define CODE_JIS_N	'N'
#define CODE_JIS_j	'j'
#define CODE_JIS_J	'J'
#define CODE_INNER_EUC	'I'

#define STR_ASCII	"US_ASCII"
#define STR_EUC		"EUC-JP"
#define STR_SJIS	"Shift_JIS"
#define STR_JIS_n	"ISO-2022-JP (JIS X 0208 + US_ASCII)"
#define STR_JIS_m	"ISO-2022-JP (JIS C 6226 + US_ASCII)"
#define STR_JIS_N	"ISO-2022-JP (JIS X 0208 + JIS X 0201)"
#define STR_JIS_j	"ISO-2022-JP (JIS C 6226 + JIS X 0201)"
#define STR_JIS_J	"ISO-2022-JP (JIS C 6226 + '\033(H')"
#define STR_INNER_EUC	"EUC-JP (internal)"

#if defined(__EMX__)&&!defined(JP_CHARSET)
/*
 * Following definitions are valid only for the OS/2 native console
 */
#define CODE_850	'8'	/* code page 850	*/
#define CODE_PC		'P'	/* another code pages	*/
#endif			/* __EMX__ */

#define CODE_JIS(x) ((x)==CODE_JIS_n||(x)==CODE_JIS_m||(x)==CODE_JIS_N||(x)==CODE_JIS_j||(x)==CODE_JIS_J)

#endif				/* not TERMS_H */
