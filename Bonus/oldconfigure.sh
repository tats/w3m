#! /bin/sh
#
# oldconfig.sh: convert a config.param file and execute configure
#

# functions
opt_push () {
#  OPT="${OPT} $1"
  OPT="${OPT}	\\
	$1"
}

opt_enable_set () {
  val=""
  if test x"$1" = xy; then
    val="--enable-$2"
  elif test x"$1" = xn; then
    val="--disable-$2"
  fi
  if test x"$val" != x; then
    opt_push "$val"
  fi
}

opt_with_set () {
  val=""
  if test x"$1" != x; then
    val="--with-$2='$1'"
  else
    val="--without-$2"
  fi
  if test x"$val" != x; then
    opt_push "$val"
  fi
}

env_set () {
  # no overwrite
  if test x"$1" != x && eval "test -z \"\$$1\"" > /dev/null; then
    echo "$1='$2'; export $1"
    eval "$1='$2'; export $1"
  fi
}

# main
topdir="`dirname $0`/.."

if test x"$1" = x-v; then
  echo_only=yes
  shift
fi
if test x"$1" = x; then
  echo "USAGE: $0 [-v] <config.param file>"
  echo "    option: -v ... echo only"
  exit 1
fi

if expr "$1" : '.*/' > /dev/null; then
  conffile="$1"
else
  conffile=./"$1"
fi
. "$conffile" # read config.param

OPT=""
libdir=`echo $libdir | sed 's@/w3m[^/]*/cgi-bin@@'`
libexecdir=`echo $auxbindir | sed 's@/w3m[^/]*@@'`
datadir=`echo $helpdir | sed 's@/w3m[^/]*@@'`
sysconfdir=`echo $sysconfdir | sed 's@/w3m[^/]*@@'`
opt_push "--bindir='$bindir'"
opt_push "--libexecdir='$libexecdir'"
opt_push "--datadir='$datadir'"
opt_push "--sysconfdir='$sysconfdir'"
opt_push "--libdir='$libdir'"
opt_push "--mandir='$mandir'"

#case "$dmodel" in
#  1) val=baby;;
#  2) val=little;;
#  3) val=mouse;;
#  4) val=cookie;;
#  5) val=monster;;
#  *) echo "ERROR: Illegal model type (model=$dmodel)."
#     exit 1;;
#esac
#opt_push "--enable-model=$val"

case "$lang" in
  JA)
    if test x$display_code != x; then
      opt_push "--enable-japanese='$display_code'"
    else
      opt_push "--enable-japanese"
    fi
    opt_enable_set "$kanji_symbols" kanjisymbols
    ;;
  *)
    ;;
esac

opt_enable_set "$use_color" color
opt_enable_set "$use_ansi_color" ansi-color
opt_enable_set "$use_bg_color" bgcolor
if test x"$use_migemo" = xy; then
  if test x"$def_migemo_command" != x; then
    opt_push "--with-migemo='$def_migemo_command'"
  fi
elif test x"$use_migemo" = xn; then
  opt_push "--without-migemo"
fi
opt_enable_set "$use_mouse" mouse
opt_enable_set "$use_menu" menu
opt_enable_set "$use_cookie" cookie
opt_enable_set "$use_dict" dict
opt_enable_set "$use_history" history
opt_enable_set "$use_digest_auth" digest-auth
opt_enable_set "$use_nntp" nntp
opt_enable_set "$use_gopher" gopher
if test x"$use_lynx_key" = xy; then
  opt_push "--enable-keymap=lynx"
else
  opt_push "--enable-keymap=w3m"
fi
opt_with_set "$ded" editor
opt_with_set "$dmail" mailer
opt_with_set "$dbrowser" browser
opt_enable_set "$use_help_cgi" help-cgi
opt_enable_set "$use_external_uri_loader" external-uri-loader
opt_enable_set "$use_w3mmailer" w3mmailer
opt_enable_set "$use_alarm" alarm
if test x"$use_image" = xy; then
  val_x11=""
  val_fb=""
  if test x"$use_w3mimg_x11" = xy; then
    val_x11="x11"
  fi
  if test x"$use_w3mimg_fb" = xy; then
    if test x"$w3mimgdisplay_setuid" = xy; then
      val_fb="fb+s"
    else
      val_fb="fb"
    fi
  fi
  if test x"$val_x11" != x; then
    if test x"$val_fb" != x; then
      val="$val_x11,$val_fb"
    else
      val="$val_x11"
    fi
  elif test x"$val_fb" != x; then
    val="$val_fb"
  fi

  if test x"$val" = x; then
    opt_push "--enable-image"
  else
    opt_push "--enable-image='$val'"
  fi

  opt_enable_set "$use_xface" xface
elif test x"$use_image" = xn; then
  opt_push "--disable-image"
fi
if test x"$dtermlib" != x; then
  dtermlib=`echo "$dtermlib"|sed 's/^-l//'`
  opt_with_set "$dtermlib" termlib
fi
if test x"$use_ssl" = xy; then
  opt_push "--with-ssl"
  opt_enable_set "$use_ssl_verify" sslverify
elif test x"$use_ssl" = xn; then
  opt_push "--without-ssl"
fi
opt_enable_set "$use_ipv6" ipv6

env_set CC "$dcc"
env_set CFLAGS "$dcflags"
env_set LDFLAGS "$dldflags"

echo "( cd '$topdir' && sh configure ${OPT} )"
if test "${echo_only+set}" != set; then
  echo "( cd '$topdir' && sh configure ${OPT} )" | sh
fi
