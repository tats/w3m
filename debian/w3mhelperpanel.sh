#!/bin/sh
# Copyright (c) 1999 Fumitoshi UKAI <ukai@debian.or.jp>
#

W3MHELPERPANEL=/usr/lib/w3m/w3mhelperpanel-en
eval `locale`
locale=${LC_ALL:-$LANG}
case X"$locale" in
 Xja|Xja_JP|Xja_JP.*)
    [ -x /usr/lib/w3m/w3mhelperpanel-ja ] && W3MHELPERPANEL=/usr/lib/w3m/w3mhelperpanel-ja
    ;;
 *)
    ;;
esac
exec $W3MHELPERPANEL "$@"
