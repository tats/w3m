/* $Id: keybind.c,v 1.10 2006/05/29 12:17:25 inu Exp $ */
#include "funcname2.h"

unsigned char GlobalKeymap[128] = {
    /*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
#ifdef __EMX__
    pcmap, linbeg, movL, nulcmd, nulcmd, linend, movR, curlno,
#else
    _mark, linbeg, movL, nulcmd, nulcmd, linend, movR, curlno,
#endif
    /*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    ldHist, nextA, followA, cooLst, rdrwSc, followA, movD, nulcmd,
    /*  C-p     C-q     C-r     C-s     C-t     C-u     C-v     C-w      */
    movU, closeT, isrchbak, isrchfor, tabA, prevA, pgFore, wrapToggle,
    /*  C-x     C-y     C-z     C-[     C-\     C-]     C-^     C-_      */
    nulcmd, nulcmd, susp, escmap, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  SPC     !       "       #       $       %       &       '        */
    pgFore, execsh, reMark, pipesh, linend, nulcmd, nulcmd, nulcmd,
    /*  (       )       *       +       ,       -       .       /        */
    undoPos, redoPos, nulcmd, pgFore, col1L, pgBack, col1R, srchfor,
    /*  0       1       2       3       4       5       6       7        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  8       9       :       ;       <       =       >       ?        */
    nulcmd, nulcmd, chkURL, chkWORD, shiftl, pginfo, shiftr, srchbak,
    /*  @       A       B       C       D       E       F       G        */
    readsh, nulcmd, backBf, nulcmd, ldDL, editBf, rFrame, goLineL,
    /*  H       I       J       K       L       M       N       O        */
    ldhelp, followI, lup1, ldown1, linkLst, extbrz, srchprv, nulcmd,
    /*  P       Q       R       S       T       U       V       W        */
    nulcmd, quitfm, reload, svBuf, newT, goURL, ldfile, movLW,
    /*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, ctrCsrH, topA, nulcmd, lastA, linbeg, nulcmd,
    /*  `       a       b       c       d       e       f       g        */
    nulcmd, svA, pgBack, curURL, nulcmd, nulcmd, nulcmd, goLineF,
    /*  h       i       j       k       l       m       n       o        */
    movL, peekIMG, movD, movU, movR, msToggle, srchnxt, ldOpt,
    /*  p       q       r       s       t       u       v       w        */
    nulcmd, qquitfm, dispVer, selMn, nulcmd, peekURL, vwSrc, movRW,
    /*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, ctrCsrV, prevT, pipeBuf, nextT, nulcmd, nulcmd,
};

unsigned char EscKeymap[128] = {
    /*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    nulcmd, prevA, svA, nulcmd, nulcmd, svA, nulcmd, nulcmd,
    /*  C-p     C-q     C-r     C-s     C-t     C-u     C-v     C-w      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  C-x     C-y     C-z     C-[     C-\     C-]     C-^     C-_      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  SPC     !       "       #       $       %       &       '        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  (       )       *       +       ,       -       .       /        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  0       1       2       3       4       5       6       7        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  8       9       :       ;       <       =       >       ?        */
    nulcmd, nulcmd, chkNMID, nulcmd, goLineF, nulcmd, goLineL, nulcmd,
    /*  @       A       B       C       D       E       F       G        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  H       I       J       K       L       M       N       O        */
    nulcmd, svI, nulcmd, nulcmd, nulcmd, linkbrz, nulcmd, escbmap,
    /*  P       Q       R       S       T       U       V       W        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, dictwordat,
    /*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, nulcmd, escbmap, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  `       a       b       c       d       e       f       g        */
    nulcmd, adBmark, ldBmark, execCmd, nulcmd, editScr, nulcmd, goLine,
    /*  h       i       j       k       l       m       n       o        */
    nulcmd, nulcmd, nulcmd, defKey, listMn, movlistMn, nextMk, setOpt,
    /*  p       q       r       s       t       u       v       w        */
    prevMk, nulcmd, nulcmd, svSrc, tabMn, gorURL, pgBack, dictword,
    /*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

unsigned char EscBKeymap[128] = {
    /*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  C-p     C-q     C-r     C-s     C-t     C-u     C-v     C-w      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  C-x     C-y     C-z     C-[     C-\     C-]     C-^     C-_      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  SPC     !       "       #       $       %       &       '        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  (       )       *       +       ,       -       .       /        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  0       1       2       3       4       5       6       7        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  8       9       :       ;       <       =       >       ?        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  @       A       B       C       D       E       F       G        */
    nulcmd, movU, movD, movR, movL, nulcmd, goLineL, pgFore,
    /*  H       I       J       K       L       M       N       O        */
    goLineF, pgBack, nulcmd, nulcmd, nulcmd, mouse, nulcmd, nulcmd,
    /*  P       Q       R       S       T       U       V       W        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, prevA, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  `       a       b       c       d       e       f       g        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  h       i       j       k       l       m       n       o        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  p       q       r       s       t       u       v       w        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

unsigned char EscDKeymap[128] = {
    /*  0       1       INS     3       4       PgUp,   PgDn    7        */
    nulcmd, goLineF, mainMn, nulcmd, goLineL, pgBack, pgFore, nulcmd,
    /*  8       9       10      F1      F2      F3      F4      F5       */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  16      F6      F7      F8      F9      F10     22      23       */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    /*  24      25      26      27      HELP    29      30      31       */
    nulcmd, nulcmd, nulcmd, nulcmd, mainMn, nulcmd, nulcmd, nulcmd,

    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,

    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,

    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

#ifdef __EMX__
unsigned char PcKeymap[256] = {
    //                        Null
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//   0
    //                                                        S-Tab
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, prevA,	//   8
    // A-q    A-w     A-E     A-r     A-t     A-y     A-u     A-i
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  16
    // A-o    A-p     A-[     A-]                     A-a     A-s
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  24
    // A-d    A-f     A-g     A-h     A-j     A-k     A-l     A-;
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  32
    // A-'    A-'             A-\             A-x     A-c     A-v
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  40
    // A-b    A-n     A-m     A-,     A-.     A-/             A-+
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  48
    //                        F1      F2      F3      F4      F5
    nulcmd, nulcmd, nulcmd, ldhelp, nulcmd, qquitfm, nulcmd, nulcmd,	//  56
    // F6     F7      F8      F9      F10                     Home
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, goLineF,	//  64
    // Up     PgUp    A-/     Left    5       Right   C-*     End
    movU, pgBack, nulcmd, movL, nulcmd, movR, nulcmd, goLineL,	//  72
    // Down   PgDn    Ins     Del     S-F1    S-F2    S-F3    S-F4
    movD, pgFore, mainMn, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  80
    // S-F5   S-F6    S-F7    S-F8    S-F9    S-F10   C-F1    C-F2
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  88
    // C-F3   C-F4    C-F5    C-F6    C-F7    C-F8    C-F9    C-F10
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	//  96
    // A-F1   A-F2    A-F3    A-F4    A-F5    A-F6    A-F7    A-F8
    nulcmd, nulcmd, nulcmd, qquitfm, nulcmd, nulcmd, nulcmd, nulcmd,	// 104
    // A-F9   A-F10   PrtSc   C-Left  C-Right C-End   C-PgDn  C-Home
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 112
    // A-1    A-2     A-3     A-4     A-5     A-6     A-7/8   A-9
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 120
    // A-0    A -     A-=             C-PgUp  F11     F12     S-F11
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 128
    // S-F12  C-F11   C-F12   A-F11   A-F12   C-Up    C-/     C-5
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 136
    // S-*    C-Down  C-Ins   C-Del   C-Tab   C -     C-+
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 144
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 152
    //                                A -     A-Tab   A-Enter
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 160
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 168
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 176
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 184
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 192
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 200
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 208
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 216
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 224
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 232
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,	// 240
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd	// 248
};
#endif
