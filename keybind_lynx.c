
/* 
 * Lynx-like key binding.
 *
 * modified from original keybind.c by Keisuke Hashimoto
 * <hasimoto@shimada.nuee.nagoya-u.ac.jp>
 * http://www.shimada.nuee.nagoya-u.ac.jp/~hasimoto/
 *
 * further modification by Akinori Ito
 *
 * Date: Tue, 23 Feb 1999 13:14:44 +0900
 */

#include "funcname2.h"

char GlobalKeymap[128] =
{
/*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
    _mark,  goLineF,backBf, nulcmd, nulcmd, goLineL,followA,curlno,
/*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    ldHist, nextA, followA, cooLst, rdrwSc, followA, nextA, nulcmd,
/*  C-p     C-q     C-r     C-s     C-t     C-u     C-v     C-w      */
    prevA, nulcmd, reload, srchfor, nulcmd, nulcmd, pgFore, rdrwSc,
/*  C-x     C-y     C-z     C-[     C-\     C-]     C-^     C-_      */
    nulcmd, nulcmd, susp,   escmap, nulcmd, nulcmd, nulcmd, nulcmd,
/*  SPC     !       "       #       $       %       &       '        */
    pgFore, execsh, reMark, pipesh, linend, nulcmd, nulcmd, nulcmd,
/*  (       )       *       +       ,       -       .       /        */
    nulcmd, nulcmd, nulcmd, pgFore, nulcmd, pgBack, nulcmd, srchfor,
/*  0       1       2       3       4       5       6       7        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  8       9       :       ;       <       =       >       ?        */
    nulcmd, nulcmd, chkURL, nulcmd, shiftl, pginfo, shiftr, ldhelp,
/*  @       A       B       C       D       E       F       G        */
    readsh, nulcmd, backBf, nulcmd, nulcmd, editBf, rFrame, goLine,
/*  H       I       J       K       L       M       N       O        */
    ldhelp, followI,lup1,   ldown1, nulcmd, extbrz, nextMk, nulcmd,
/*  P       Q       R       S       T       U       V       W        */
    prevMk, quitfm, reload, svBuf,  nulcmd, goURL, ldfile,  nulcmd,
/*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, ctrCsrH, nulcmd, vwSrc, nulcmd, linbeg, nulcmd,
/*  `       a       b       c       d       e       f       g        */
    nulcmd, adBmark,pgBack, curURL, svA,    nulcmd, nulcmd, goURL,
/*  h       i       j       k       l       m       n       o        */
    movL,   peekIMG,movD,   movU,   movR,   nulcmd, srchnxt,ldOpt,
/*  p       q       r       s       t       u       v       w        */
    svBuf,  qquitfm,nulcmd, selMn,  nulcmd, peekURL,ldBmark,wrapToggle,
/*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, ctrCsrV,nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

char EscKeymap[128] =
{
/*  C-@     C-a     C-b     C-c     C-d     C-e     C-f     C-g      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  C-h     C-i     C-j     C-k     C-l     C-m     C-n     C-o      */
    nulcmd, prevA,  svA,    nulcmd, nulcmd, svA,    nulcmd, nulcmd,
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
    nulcmd, nulcmd, chkNMID, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  @       A       B       C       D       E       F       G        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  H       I       J       K       L       M       N       O        */
    nulcmd, svI,    nulcmd, nulcmd, nulcmd, linkbrz,nulcmd, escbmap,
/*  P       Q       R       S       T       U       V       W        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, nulcmd, escbmap,nulcmd, nulcmd, nulcmd, nulcmd,
/*  `       a       b       c       d       e       f       g        */
    nulcmd, adBmark,ldBmark,nulcmd, nulcmd, editScr,nulcmd, nulcmd,
/*  h       i       j       k       l       m       n       o        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  p       q       r       s       t       u       v       w        */
    nulcmd, nulcmd, nulcmd, svSrc,  nulcmd, nulcmd, pgBack, nulcmd,
/*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

char EscBKeymap[128] =
{
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
    nulcmd, prevA, nextA, followA, backBf, nulcmd, nulcmd, nulcmd,
/*  H       I       J       K       L       M       N       O        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, mouse, nulcmd, nulcmd,
/*  P       Q       R       S       T       U       V       W        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  X       Y       Z       [       \       ]       ^       _        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  `       a       b       c       d       e       f       g        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  h       i       j       k       l       m       n       o        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  p       q       r       s       t       u       v       w        */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
/*  x       y       z       {       |       }       ~       DEL      */
    nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd, nulcmd,
};

char EscDKeymap[128] =
{
/*  0       1       INS     3       4       PgUp,   PgDn    7        */
    nulcmd, goLineF,mainMn, nulcmd, goLineL,pgBack, pgFore, nulcmd,
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
