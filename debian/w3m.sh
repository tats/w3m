#!/bin/bash
# Copyright (c) 1999 Fumitoshi UKAI <ukai@debian.or.jp>
# Copyright (c) 1999 Jacobo Tarrio Barreiro <jtarrio@iname.com>
# This program is covered by the GNU General Public License version 2
#
unset params

while [ $# -gt 0 ]
do
	case "$1" in
		-t | -l | -T | -bookmark | -cols | -ppc | -o | -config)
			params[${#params[@]}]="$1"
			params[${#params[@]}]="$2"
			shift ;;
		-* | +* | *://*)
			params[${#params[@]}]="$1"
			;;
		*)
			if [ -f "$1" -o -d "$1" ]
			then
				params[${#params[@]}]="$1"
			else
				params[${#params[@]}]="http://$1"
			fi ;;
	esac
	shift
done

## for I18N variants, not yet: try w3mmee
#W3M=${W3M:-/usr/bin/w3m-ssl-i18n}
#test -x $W3M && exec $W3M "${params[@]}"
#
#for W3M in /usr/bin/w3m-ssl-i18n /usr/bin/w3m-ssl
#do
# test -x $W3M && exec $W3M "${params[@]}"
#done

W3M=/usr/bin/w3m-en
if [ -x /usr/bin/locale ]; then
  eval `locale`
fi
locale=${LC_ALL:-$LANG}
case X"$locale" in
 Xja|Xja_JP|Xja_JP.*)
    [ -x /usr/bin/w3m-ja ] && W3M=/usr/bin/w3m-ja
    [ -x /usr/bin/w3m-ssl-ja ] && W3M=/usr/bin/w3m-ssl-ja
    ;;
 *)
    [ -x /usr/bin/w3m-ssl-en ] && W3M=/usr/bin/w3m-ssl-en
    ;;
esac
exec $W3M "${params[@]}"
