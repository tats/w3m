#!/bin/sh
# Copyright (c) 1999 Fumitoshi UKAI <ukai@debian.or.jp>
#

W3MBOOKMARK=/usr/lib/w3m/w3mbookmark-en
eval `locale`
locale=${LC_ALL:-$LANG}
case X"$locale" in
 Xja|Xja_JP|Xja_JP.*)
    [ -x /usr/lib/w3m/w3mbookmark-ja ] && W3MBOOKMARK=/usr/lib/w3m/w3mbookmark-ja
    ;;
 *)
    ;;
esac
exec $W3MBOOKMARK "$@"
