#! /bin/sh
#
# oldconfig.sh: convert a config.param file and execute configure
#

# functions
opt_enable_set () {
  val=""
  if test x"$1" = xy; then
    val="--enable-$2"
  elif test x"$1" = xn; then
    val="--disable-$2"
  fi
  if test x"$val" != x; then
    OPT="${OPT} $val"
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
    OPT="${OPT} $val"
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

OPT="--bindir='$bindir' --libexecdir='$auxbindir' --datadir='$helpdir' --sysconfdir='$sysconfdir' --libdir='$libdir' --mandir='$mandir'"

#case "$dmodel" in
#  1) val=baby;;
#  2) val=little;;
#  3) val=mouse;;
#  4) val=cookie;;
#  5) val=monster;;
#  *) echo "ERROR: Illegal model type (model=$dmodel)."
#     exit 1;;
#esac
#OPT="${OPT} --enable-model=$val"

case "$lang" in
  JA)
    if test x$display_code != x; then
      OPT="${OPT} --enable-japanese='$display_code'"
    else
      OPT="${OPT} --enable-japanese"
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
    OPT="${OPT} --with-migemo='$def_migemo_command'"
  fi
elif test x"$use_migemo" = xn; then
  OPT="${OPT} --without-migemo"
fi
opt_enable_set "$use_mouse" mouse
opt_enable_set "$use_menu" menu
opt_enable_set "$use_cookie" cookie
opt_enable_set "$use_dict" dict
opt_enable_set "$use_history" history
opt_enable_set "$use_digest_auth" digest-auth
opt_enable_set "$use_nntp" nntp
opt_enable_set "$use_gopher" gopher
opt_enable_set "$use_lynx_key" keymap=lynx
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
    OPT="${OPT} --enable-image"
  else
    OPT="${OPT} --enable-image='$val'"
  fi

  opt_enable_set "$use_xface" xface
elif test x"$use_image" = xn; then
  OPT="${OPT} --disable-image"
fi
if test x"$use_ssl" = xy; then
  OPT="${OPT} --with-ssl"

  opt_enable_set "$use_ssl_verify" sslverify
elif test x"$use_ssl" = xn; then
  OPT="${OPT} --without-ssl"
fi
opt_enable_set "$use_ipv6" ipv6

env_set CC "$dcc"
env_set CFLAGS "$dcflags"
env_set LDFLAGS "$dldflags"

echo "( cd '$topdir'; sh configure ${OPT} )"
if test "${echo_only+set}" != set; then
  echo "( cd '$topdir'; sh configure ${OPT} )" | sh
fi
