/* $Id: ctrlcode.h,v 1.3 2003/09/22 21:02:17 ukai Exp $ */
/* control characters */

#define CTRL_A          1
#define CTRL_B          2
#define CTRL_C          3
#define CTRL_D          4
#define CTRL_E          5
#define CTRL_F          6
#define CTRL_G          7
#define CTRL_H          8
#define CTRL_I          9
#define CTRL_J          10
#define CTRL_K          11
#define CTRL_L          12
#define CTRL_M          13
#define CTRL_N          14
#define CTRL_O          15
#define CTRL_P          16
#define CTRL_Q          17
#define CTRL_R          18
#define CTRL_S		19
#define CTRL_T		20
#define CTRL_U		21
#define CTRL_V		22
#define CTRL_W		23
#define CTRL_X		24
#define CTRL_Y		25
#define CTRL_Z		26
#define	ESC_CODE	27
#define	DEL_CODE	127

/* ISO-8859-1 alphabet characters */

#define NBSP_CODE	160
#define IEXCL_CODE	161
#define CENT_CODE	162
#define POUND_CODE	163
#define CURREN_CODE	164
#define YEN_CODE	165
#define BRVBAR_CODE	166
#define SECT_CODE	167
#define UML_CODE	168
#define COPY_CODE	169
#define ORDF_CODE	170
#define LAQUO_CODE	171
#define NOT_CODE	172
#define SHY_CODE	173
#define REG_CODE	174
#define MACR_CODE	175
#define DEG_CODE	176
#define PLUSMN_CODE	177
#define SUP2_CODE	178
#define SUP3_CODE	179
#define ACUTE_CODE	180
#define MICRO_CODE	181
#define PARA_CODE	182
#define MIDDOT_CODE	183
#define CEDIL_CODE	184
#define SUP1_CODE	185
#define ORDM_CODE	186
#define RAQUO_CODE	187
#define FRAC14_CODE	188
#define FRAC12_CODE	189
#define FRAC34_CODE	190
#define IQUEST_CODE	191
#define AGRAVE_CODE	192
#define AACUTE_CODE	193
#define ACIRC_CODE	194
#define ATILDE_CODE	195
#define AUML_CODE	196
#define ARING_CODE	197
#define AELIG_CODE	198
#define CCEDIL_CODE	199
#define EGRAVE_CODE	200
#define EACUTE_CODE	201
#define ECIRC_CODE	202
#define EUML_CODE	203
#define IGRAVE_CODE	204
#define IACUTE_CODE	205
#define ICIRC_CODE	206
#define IUML_CODE	207
#define ETH_CODE	208
#define NTILDE_CODE	209
#define OGRAVE_CODE	210
#define OACUTE_CODE	211
#define OCIRC_CODE	212
#define OTILDE_CODE	213
#define OUML_CODE	214
#define TIMES_CODE	215
#define OSLASH_CODE	216
#define UGRAVE_CODE	217
#define UACUTE_CODE	218
#define UCIRC_CODE	219
#define UUML_CODE	220
#define YACUTE_CODE	221
#define THORN_CODE	222
#define SZLIG_CODE	223
#define aGRAVE_CODE	224
#define aACUTE_CODE	225
#define aCIRC_CODE	226
#define aTILDE_CODE	227
#define aUML_CODE	228
#define aRING_CODE	229
#define aELIG_CODE	230
#define cCEDIL_CODE	231
#define eGRAVE_CODE	232
#define eACUTE_CODE	233
#define eCIRC_CODE	234
#define eUML_CODE	235
#define iGRAVE_CODE	236
#define iACUTE_CODE	237
#define iCIRC_CODE	238
#define iUML_CODE	239
#define eth_CODE	240
#define nTILDE_CODE	241
#define oGRAVE_CODE	242
#define oACUTE_CODE	243
#define oCIRC_CODE	244
#define oTILDE_CODE	245
#define oUML_CODE	246
#define DIVIDE_CODE	247
#define oSLASH_CODE	248
#define uGRAVE_CODE	249
#define uACUTE_CODE	250
#define uCIRC_CODE	251
#define uUML_CODE	252
#define yACUTE_CODE	253
#define thorn_CODE	254
#define yUML_CODE	255

/* internally used characters  */
#define ANSP_CODE	0x9e	/* use for empty anchor */
#define IMSP_CODE	0x9f	/* blank around image */

#define NBSP		"\xa0"
#define ANSP		"\x9e"
#define IMSP		"\x9f"

#include "myctype.h"

/* Local Variables:    */
/* c-basic-offset: 4   */
/* tab-width: 8        */
/* End:                */
