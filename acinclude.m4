dnl w3m autoconf macros
#
# ----------------------------------------------------------------
# AC_W3M_VERSION
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_VERSION],
[AC_SUBST(CURRENT_VERSION)
 cvsver=`$AWK '\$[1] ~ /Id:/ { print \$[3]}' $srcdir/ChangeLog`
 sed -e 's/define CURRENT_VERSION "\(.*\)+cvs/define CURRENT_VERSION "\1+cvs-'$cvsver'/' $srcdir/version.c.in > version.c
 CURRENT_VERSION=`sed -n 's/.*define CURRENT_VERSION *"w3m\/\(.*\)".*$/\1/p' version.c`])
#
# ----------------------------------------------------------------
# AC_W3M_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_COLOR],
[AC_SUBST(USE_COLOR)
AC_MSG_CHECKING(if color escape sequence for kterm/pxvt is enabled)
AC_ARG_ENABLE(color,
 [  --disable-color		disable color for vt100 terminal],,
 [enable_color="yes"])
test x"$enable_color" = xyes && AC_DEFINE(USE_COLOR)
AC_MSG_RESULT($enable_color)])
#
# ----------------------------------------------------------------
# AC_W3M_ANSI_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_ANSI_COLOR],
[AC_SUBST(USE_ANSI_COLOR)
AC_MSG_CHECKING(if ansi color escape sequence support is enabled)
AC_ARG_ENABLE(ansi_color,
 [   --disable-ansi-color		disable ansi color escape sequence],,
 [enable_ansi_color="$enable_color"])
 test x"$enable_ansi_color" = xyes && AC_DEFINE(USE_ANSI_COLOR)
 AC_MSG_RESULT($enable_ansi_color)])
#
# ----------------------------------------------------------------
# AC_W3M_BG_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_BG_COLOR],
[AC_SUBST(USE_BG_COLOR)
AC_MSG_CHECKING(if background color support is enabled)
AC_ARG_ENABLE(bgcolor,
 [   --disable-bgcolor		disable to set background color],,
 [enable_bgcolor="$enable_color"])
 test x"$enable_bgcolor" = xyes && AC_DEFINE(USE_BG_COLOR)
AC_MSG_RESULT($enable_bgcolor)])
#
# ----------------------------------------------------------------
# AC_W3M_MENU
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_MENU],
[AC_SUBST(USE_MENU)
AC_MSG_CHECKING(if popup menu is enabled)
AC_ARG_ENABLE(menu,
 [  --disable-menu		disable popup menu],,
 [enable_menu="yes"])
 test x"$enable_menu" = xyes && AC_DEFINE(USE_MENU)
 AC_MSG_RESULT($enable_menu)])
#
# ----------------------------------------------------------------
# AC_W3M_MOUSE
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_MOUSE],
[AC_SUBST(USE_MOUSE)
AC_MSG_CHECKING(if mouse operation enabled)
AC_ARG_ENABLE(mouse,
 [  --disable-mouse		disable mouse operation],,
 [enable_mouse="yes"])
test x"$enable_mouse" = xyes && AC_DEFINE(USE_MOUSE)
AC_MSG_RESULT($enable_mouse)])
#
# ----------------------------------------------------------------
# AC_W3M_COOKIE
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_COOKIE],
[AC_SUBST(USE_COOKIE)
AC_MSG_CHECKING(if cookie is enabled)
AC_ARG_ENABLE(cookie,
 [  --disable-cookie		disable cookie],,
 [enable_cookie="yes"])
test x"$enable_cookie" = xyes && AC_DEFINE(USE_COOKIE)
AC_MSG_RESULT($enable_cookie)])
#
# ----------------------------------------------------------------
# AC_W3M_DICT
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_DICT],
[AC_SUBST(USE_DICT)
AC_MSG_CHECKING(if dictionary lookup is enabled)
AC_ARG_ENABLE(dict,
 [  --disable-dict		disable dictionary lookup (see README.dict)],,
 [enable_dict="yes"])
 test x"$enable_dict" = xyes && AC_DEFINE(USE_DICT)
 AC_MSG_RESULT($enable_dict)])
#
# ----------------------------------------------------------------
# AC_W3M_HISTORY
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_HISTORY],
[AC_SUBST(USE_HISTORY)
AC_MSG_CHECKING(if URL history is enabled)
AC_ARG_ENABLE(history,
 [  --disable-history		disable URL history],,
 [enable_history="yes"])
 test x"$enable_history" = xyes && AC_DEFINE(USE_HISTORY)
 AC_MSG_RESULT($enable_history)])
#
# ----------------------------------------------------------------
# AC_W3M_NNTP
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_NNTP],
[AC_SUBST(USE_NNTP)
 AC_MSG_CHECKING(if nntp is enabled)
 AC_ARG_ENABLE(nntp,
  [  --disable-nntp		disable NNTP],,
  [enable_nntp="yes"])
 test x"$enable_nntp" = xyes && AC_DEFINE(USE_NNTP)
 AC_MSG_RESULT($enable_nntp)])
# 
# ----------------------------------------------------------------
# AC_W3M_GOPHER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_GOPHER],
[AC_SUBST(USE_GOPHER)
 AC_MSG_CHECKING(if gopher is enabled)
 AC_ARG_ENABLE(gopher,
  [  --enable-gopher		enable GOPHER],,
  [enable_gopher="no"])
 test x"$enable_gopher" = xyes &&  AC_DEFINE(USE_GOPHER)
 AC_MSG_RESULT($enable_gopher)])
#
# ----------------------------------------------------------------
# AC_W3M_M17N
# ----------------------------------------------------------------
# m17n enable?
AC_DEFUN([AC_W3M_M17N],
[AC_SUBST(USE_M17N)
AC_SUBST(USE_UNICODE)
AC_SUBST(WCTARGET)
AC_SUBST(WCCFLAGS)
AC_SUBST(DISPLAY_CHARSET)
AC_SUBST(SYSTEM_CHARSET)
AC_SUBST(DOCUMENT_CHARSET)
AC_SUBST(POSUBST)
AC_SUBST(POLANG)
WCTARGET=""; WCCFLAGS=""; wcinclude=""; wclib=""
AC_MSG_CHECKING(if m17n support is enabled)
AC_ARG_ENABLE(m17n,
 [  --disable-m17n		do not use m17n],,
 [enable_m17n="yes"])
AC_MSG_RESULT($enable_m17n)
if test x"$enable_m17n" = xno; then
  w3m_lang="en"
  WCTARGET=""
  WCCFLAGS=""
  wcinclude=""
  wclib=""
  display_charset='WC_CES_US_ASCII'
  system_charset='WC_CES_US_ASCII'
  document_charset='WC_CES_US_ASCII'
else
 AC_DEFINE(USE_M17N)
 WCTARGET="libwc/libwc.a"
 WCCFLAGS='-I$(srcdir) -I$(srcdir)/..'
 wcinclude='-I$(srcdir)/libwc'
 wclib="-L./libwc -lwc"
 AC_MSG_CHECKING(if unicode support is enabled)
 AC_ARG_ENABLE(unicode,
  [   --disable-unicode		do not use unicode],,
  [enable_unicode="yes"])
 AC_MSG_RESULT($enable_unicode)
 if test x"$enable_m17n" = xyes; then
  charset=US-ASCII
 else
  charset=$enable_m17n
 fi
 if test x"$enable_unicode" = xyes; then
    WCCFLAGS="-DUSE_UNICODE $WCCFLAGS"
    if test x"$charset" = xUS-ASCII; then
     charset=UTF-8
    fi
    AC_DEFINE(USE_UNICODE)
 fi
 AC_MSG_CHECKING(if message l10n)
 AC_ARG_ENABLE(messagel10n,
   [   --enable-messagel10n=LL	message l10n instead of NLS],,
   [enable_messagel10n="no"])
 if test x$enable_messagel10n = xyes; then
  enable_messagel10n="ja";
 fi
 AC_MSG_RESULT($enable_messagel10n)
 if test x$enable_messagel10n = xno; then
    :
 else
    POSUBST="\$(top_srcdir)/posubst"
    POLANG="$enable_messagel10n"
 fi
 AC_MSG_CHECKING(if japanese support is enabled)
 AC_ARG_ENABLE(japanese,
   [   --enable-japanese=CODE	support Japanese CODE=(S|E|J|U)],,
   [enable_japanese="no"])
 AC_MSG_RESULT($enable_japanese)
 if test x"$enable_japanese" = xno; then
   w3m_lang="en"
 else
   w3m_lang="ja"
   case "$enable_japanese" in
   E*) charset=EUC-JP;;
   S*) charset=Shift_JIS;;
   J*) charset=ISO-2022-JP;;
   U*) charset=UTF-8;;
   esac 
 fi
 display_charset=$charset
 AC_MSG_CHECKING(which charset is used for display)
 AC_ARG_WITH(charset,
  [  --with-charset=CHARSET],
  [test x"with_charset" = xyes || display_charset="$with_charset"])
 AC_MSG_RESULT($display_charset)
 display_charset=`awk '$[1] == "'$display_charset'" {print $[2]}' $srcdir/charset-list`
 case "$display_charset" in
   WC_CES_ISO_2022_JP*)
     system_charset=WC_CES_EUC_JP
     document_charset=WC_CES_EUC_JP
     ;;
   WC_CES_SHIFT_JIS)
     system_charset=$display_charset
     # for auto-detect
     document_charset=WC_CES_EUC_JP
     ;;
   WC_CES_ISO_2022_CN|WC_CES_HZ_GB_2312)
     system_charset=WC_CES_EUC_CN
     document_charset=WC_CES_EUC_CN
     ;;
   WC_CES_BIG5)
     system_charset=$display_charset
     # for auto-detect
     document_charset=WC_CES_EUC_TW
     ;;
  WC_CES_ISO_2022_KR)
     system_charset=WC_CES_EUC_KR
     document_charset=WC_CES_EUC_KR
     ;;
  *)
     system_charset=$display_charset
     document_charset=$display_charset
     ;;
 esac
fi
W3M_LANGDEF=`echo $w3m_lang | tr 'a-z' 'A-Z'`
W3M_LANG=$W3M_LANGDEF
AC_DEFINE_UNQUOTED(W3M_LANG, $W3M_LANG)
AC_DEFINE_UNQUOTED(WCTARGET, "$WCTARGET")
AC_DEFINE_UNQUOTED(WCCFLAGS, "$WCCFLAGS")
CFLAGS="$CFLAGS $wcinclude"
W3M_LIBS="$W3M_LIBS $wclib"
AC_DEFINE_UNQUOTED(DISPLAY_CHARSET, $display_charset)
AC_DEFINE_UNQUOTED(SYSTEM_CHARSET, $system_charset)
AC_DEFINE_UNQUOTED(DOCUMENT_CHARSET, $document_charset)])
#
# ----------------------------------------------------------------
# AC_W3M_KEYMAP
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_KEYMAP],
[AC_SUBST(KEYMAP_FILE)
 AC_MSG_CHECKING(default keymap)
 AC_ARG_ENABLE(keymap,
  [  --enable-keymap[=w3m|lynx]	default keybind style(w3m or lynx)],,
  [enable_keymap="w3m"])
 AC_MSG_RESULT($enable_keymap)
 case x"$enable_keymap" in
 xw3m)
  KEYMAP_FILE="keybind";;
 xlynx)
  KEYMAP_FILE="keybind_lynx";;
 *)
  AC_MSG_ERROR([keymap should be either w3m or lynx.]);;
 esac
 AC_SUBST(HELP_FILE)
 HELP_FILE=w3mhelp-${enable_keymap}_$w3m_lang.html
 AC_DEFINE_UNQUOTED(HELP_FILE, "$HELP_FILE")
 AC_SUBST(KEYBIND)
 AC_DEFINE_UNQUOTED(KEYBIND, $enable_keymap)])
#
# ----------------------------------------------------------------
# AC_W3M_MIGEMO
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_MIGEMO],
[AC_SUBST(USE_MIGEMO)
 AC_SUBST(DEF_MIGEMO_COMMAND)
 migemo_command="migemo -t egrep /usr/local/share/migemo/migemo-dict"
 AC_MSG_CHECKING(if migemo is supported with)
 AC_ARG_WITH(migemo,
  [  --with-migemo=MIGEMO_COMMAND	migemo command],
  [test x"$with_migemo" = xyes || migemo_command="$with_migemo"])
 if test "${with_migemo+set}" = set -a "$with_migemo" != "no"; then
   AC_DEFINE(USE_MIGEMO)
 fi
 AC_MSG_RESULT($migemo_command)
 AC_DEFINE_UNQUOTED(DEF_MIGEMO_COMMAND, "$migemo_command")])
#
# ----------------------------------------------------------------
# AC_W3M_EDITOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_EDITOR],
[AC_SUBST(DEF_EDITOR)
w3m_editor="/usr/bin/vi"
AC_MSG_CHECKING(which editor is used by default)
AC_ARG_WITH(editor,
 [  --with-editor=EDITOR		default editor (/usr/bin/vi)],
 [w3m_editor="$with_editor"])
AC_MSG_RESULT($w3m_editor)
AC_DEFINE_UNQUOTED(DEF_EDITOR, "$w3m_editor")])
#
# ----------------------------------------------------------------
# AC_W3M_MAILER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_MAILER],
[AC_SUBST(DEF_MAILER)
w3m_mailer="/usr/bin/mail"
AC_MSG_CHECKING(which mailer is used by default)
AC_ARG_WITH(mailer,
 [  --with-mailer=MAILER		default mailer (/usr/bin/mail)],
 [w3m_mailer="$with_mailer"])
AC_MSG_RESULT($w3m_mailer)
AC_DEFINE_UNQUOTED(DEF_MAILER, "$w3m_mailer")])
#
# ----------------------------------------------------------------
# AC_W3M_EXT_BROWSER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_EXT_BROWSER],
[AC_SUBST(DEF_EXT_BROWSER)
w3m_browser="/usr/bin/mozilla"
AC_MSG_CHECKING(which external browser is used by default)
AC_ARG_WITH(browser,
 [  --with-browser=BROWSER	default browser (/usr/bin/mozilla)],
 [w3m_browser="$with_browser"])
AC_MSG_RESULT($w3m_browser)
AC_DEFINE_UNQUOTED(DEF_EXT_BROWSER, "$w3m_browser")])
#
# ----------------------------------------------------------------
# AC_W3M_HELP_CGI
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_HELP_CGI],
[AC_SUBST(USE_HELP_CGI)
 AC_MSG_CHECKING(if help cgi is enabled)
 AC_ARG_ENABLE(help_cgi,
  [  --disable-help-cgi		disable help cgi],,
  [enable_help_cgi="yes"])
 test x"$enable_help_cgi" = xyes && AC_DEFINE(USE_HELP_CGI)
 AC_MSG_RESULT($enable_help_cgi)])
# 
# ----------------------------------------------------------------
# AC_W3M_EXTERNAL_URI_LOADER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_EXTERNAL_URI_LOADER],
[AC_SUBST(USE_EXTERNAL_URI_LOADER)
 AC_MSG_CHECKING(if external URI loader is enabled)
 AC_ARG_ENABLE(external_uri_loader,
 [  --disable-external-uri-loader	disable external URI loader],,
 [enable_external_uri_loader="yes"])
 test x"$enable_external_uri_loader" = xyes && AC_DEFINE(USE_EXTERNAL_URI_LOADER)
 AC_MSG_RESULT($enable_external_uri_loader)])
# 
# ----------------------------------------------------------------
# AC_W3M_W3MMAILER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_W3MMAILER],
[AC_SUBST(USE_W3MMAILER)
 AC_MSG_CHECKING(if w3mmail is used)
 AC_ARG_ENABLE(w3mmailer,
 [   --disable-w3mmailer		disable w3mmailer],,
 [enable_w3mmailer="$enable_external_uri_loader"])
 test x"$enable_external_uri_loader" = xno && enable_w3mmailer=no
 test x"$enable_w3mmailer" = xyes && AC_DEFINE(USE_W3MMAILER)
 AC_MSG_RESULT($enable_w3mmailer)])
#
# ----------------------------------------------------------------
# AC_W3M_EXTLIBS(libs)
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_EXTLIBS],
[lib=$1
 AC_MSG_CHECKING(for -l$lib)
 extlib="not found"
 for dir in /lib /usr/lib /usr/local/lib /usr/ucblib /usr/ccslib /usr/ccs/lib /lib64 /usr/lib64
 do
   if test -f $dir/lib$lib.a -o -f $dir/lib$lib.so ; then 
    LIBS="$LIBS -l$lib"
    extlib="found at $dir"
    break
   fi
 done
 AC_MSG_RESULT($extlib)])
#
# ----------------------------------------------------------------
# AC_W3M_TERMLIB
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_TERMLIB],
[AC_MSG_CHECKING(terminal library)
AC_ARG_WITH(termlib,
 [  --with-termlib[=LIBS]		terminal library
				LIBS is space separated list of:
				  terminfo mytinfo termcap ncurses curses],,
 [with_termlib="yes"])
 AC_MSG_RESULT($with_termlib)
 test x"$with_termlib" = xyes && with_termlib="terminfo mytinfo termlib termcap ncurses curses"
 for lib in $with_termlib; do
   AC_CHECK_LIB($lib, tgetent, [W3M_LIBS="$W3M_LIBS -l$lib"; break])
 done
])
#
# ----------------------------------------------------------------
# AC_W3M_GC
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_GC],
[AC_MSG_CHECKING(GC library exists)
AC_SUBST(LIBGC)
AC_ARG_WITH(gc,
 [  --with-gc[=PREFIX]	  	libgc PREFIX],
 [test x"$with_gc" = xno && AC_MSG_ERROR([You can not build w3m without gc])],
 [with_gc="yes"])
 AC_MSG_RESULT($with_gc)
 test x"$with_gc" = xyes && with_gc="/usr /usr/local ${HOME}"
 unset ac_cv_header_gc_h
 AC_CHECK_HEADER(gc.h)
 if test x"$ac_cv_header_gc_h" = xno; then
   AC_MSG_CHECKING(GC header location)
   AC_MSG_RESULT($with_gc)
   gcincludedir=no
   for dir in $with_gc; do
     for inc in include include/gc; do
       cppflags="$CPPFLAGS"
       CPPFLAGS="$CPPFLAGS -I$dir/$inc"
       AC_MSG_CHECKING($dir/$inc)
       unset ac_cv_header_gc_h
       AC_CHECK_HEADER(gc.h, [gcincludedir="$dir/$inc"; CFLAGS="$CFLAGS -I$dir/$inc"; break])
       CPPFLAGS="$cppflags"
     done
     if test x"$gcincludedir" != xno; then
       break;
     fi
   done
   if test x"$gcincludedir" = xno; then
     AC_MSG_ERROR([gc.h not found])
   fi
 fi
 unset ac_cv_lib_gc_GC_init
 AC_CHECK_LIB(gc, GC_init, [LIBGC="-lgc"])
 if test x"$ac_cv_lib_gc_GC_init" = xno; then
    AC_MSG_CHECKING(GC library location)
    AC_MSG_RESULT($with_gc)
    gclibdir=no
    for dir in $with_gc; do
      ldflags="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$dir/lib"
      AC_MSG_CHECKING($dir)
      unset ac_cv_lib_gc_GC_init
      AC_CHECK_LIB(gc, GC_init, [gclibdir="$dir/lib"; LIBGC="-L$dir/lib -lgc"; break])
      LDFLAGS="$ldflags"
    done
    if test x"$gclibdir" = xno; then
      AC_MSG_ERROR([libgc not found])
    fi
 fi])
#
# ----------------------------------------------------------------
# AC_W3M_SSL_DIGEST_AUTH
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_SSL_DIGEST_AUTH],
[AC_SUBST(USE_SSL)
AC_SUBST(USE_SSL_VERIFY)
AC_MSG_CHECKING(if SSL is suported)
AC_ARG_WITH(ssl,
 [  --with-ssl[=PREFIX]		support https protocol],,
 [with_ssl="yes"])
AC_MSG_RESULT($with_ssl)
if test x"$with_ssl" != xno; then
  PKG_CHECK_MODULES(SSL, openssl,,[
    AC_MSG_CHECKING(for SSL library/header)
    test x"$with_ssl" = xyes && with_ssl="/usr/openssl /usr/ssl /usr /usr/local/openssl /usr/local/ssl /usr/local"
    AC_MSG_RESULT($with_ssl)
    for dir in $with_ssl
    do
       if test -f "$dir/include/openssl/ssl.h"; then
          SSL_CFLAGS="$SSL_CFLAGS -I$dir/include/openssl"
          if test "$dir" != "/usr"; then
             SSL_CFLAGS="$SSL_CFLAGS -I$dir/include"
          fi
       elif test "$dir" != "/usr" -a -f "$dir/include/ssl.h"; then
          SSL_CFLAGS="$SSL_CFLAGS -I$dir/include"
       fi
       if test "$dir" != "/usr" -a -f "$dir/lib/libssl.a"; then
	  SSL_LIBS="$SSL_LIBS -L$dir/lib"
       fi
    done
  ])
  AC_CHECK_LIB(ssl, SSL_new,
	[w3m_ssl="found"; CFLAGS="$CFLAGS $SSL_CFLAGS" W3M_LIBS="$W3M_LIBS $SSL_LIBS -lssl -lcrypto"],
	[w3m_ssl="not found"],
	[$SSL_LIBS -lcrypto])

  if test x"$w3m_ssl" = xfound; then
    AC_DEFINE(USE_SSL)
    AC_MSG_CHECKING(if SSL certificate verify is enabled)
    AC_ARG_ENABLE(sslverify,
      [   --disable-sslverify		verify SSL certificate],,
      [enable_sslverify="yes"])
    test x"$enable_sslverify" = xyes && AC_DEFINE(USE_SSL_VERIFY)
    AC_MSG_RESULT($enable_sslverify)
  fi
fi
AC_SUBST(USE_DIGEST_AUTH)
AC_MSG_CHECKING(if digest auth is enabled)
AC_ARG_ENABLE(digest_auth,
 [  --disable-digest-auth		disable digest auth],,
 [enable_digest_auth="yes"])
if test x"$enable_digest_auth" = xyes -a x"$w3m_ssl" = xfound; then
  AC_DEFINE(USE_DIGEST_AUTH)
else
  enable_digest_auth="no"
fi
AC_MSG_RESULT($enable_digest_auth)
])
#
# ----------------------------------------------------------------
# AC_W3M_ALARM
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_ALARM],
[AC_SUBST(USE_ALARM)
 AC_MSG_CHECKING(if alarm is enabled)
 AC_ARG_ENABLE(alarm,
 [  --disable-alarm		disable alarm],,
 [enable_alarm="yes"])
 AC_MSG_RESULT($enable_alarm)
 if test x"$enable_alarm" = xyes; then
   AC_TRY_COMPILE(
    [#include <unistd.h>
#include <signal.h>],
    [int sa = SIGALRM;
     void (*a) = alarm;],
   [AC_DEFINE(USE_ALARM)])
 fi])
#
# ----------------------------------------------------------------
# AC_W3M_CHECK_VER(name, version, major, minor, micro, 
#		action-if-ok, message-if-badver, action-if-nover)
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_CHECK_VER],
[version="$2"
 if test x"$version" != x; then
   AC_MSG_CHECKING($1 version)
   AC_MSG_RESULT($version)
   set -- `echo "$version" | sed 's/[[^0-9]]/ /g'`
   if test "$[1]" -ne "$3" -o "$[2]" -lt "$4" || test "$[2]" -eq "$4" -a "$[3]" -lt "$5"; then
     AC_MSG_WARN([$1 is too old. Install $1 (version >= $3.$4.$5)])
     $7
   else
     $6
   fi
 else
   AC_MSG_WARN([$1 is not installed.  Install $1 (version >= $3.$4.$5)])
   $7
 fi])
#
# ----------------------------------------------------------------
# AC_W3M_IMAGE
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_IMAGE],
[AC_SUBST(USE_IMAGE)
 AC_SUBST(USE_W3MIMG_X11)
 AC_SUBST(USE_W3MIMG_FB)
 AC_SUBST(USE_W3MIMG_WIN)
 AC_SUBST(IMGLINK)
 AC_SUBST(W3MIMGDISPLAY_SETUID)
 AC_SUBST(INSTALL_W3MIMGDISPLAY)
 INSTALL_W3MIMGDISPLAY='${INSTALL_PROGRAM}'
 AC_DEFINE(INSTALL_W3MIMGDISPLAY, $INSTALL_W3MIMGDISPLAY)
 AC_SUBST(USE_GDKPIXBUF)
 AC_SUBST(USE_GTK2)
 AC_SUBST(USE_IMLIB)
 AC_SUBST(USE_IMLIB2)
 AC_SUBST(IMGTARGETS)
 AC_SUBST(IMGOBJS)
 AC_SUBST(IMGX11CFLAGS)
 AC_SUBST(IMGX11LDFLAGS)
 AC_SUBST(IMGFBCFLAGS)
 AC_SUBST(IMGFBLDFLAGS)
 AC_SUBST(IMGWINCFLAGS)
 AC_SUBST(IMGWINLDFLAGS)
 AC_MSG_CHECKING(if image is enabled)
 AC_ARG_ENABLE(image,
 [  --enable-image[=DEVS]		enable inline image handler for DEVS
				 DEVS may be comma separeted: x11,fb,fb+s,win
				 default: autodetected.
				 'no' means disable inline image],,
 [enable_image="yes"])
 AC_MSG_RESULT($enable_image)
 if test x"$enable_image" != xno; then
  IMGOBJS=w3mimg.o
  if test x"$enable_image" = xyes; then
    enable_image=x11
    case "`uname -s`" in
    Linux|linux|LINUX) 
	if test -c /dev/fb0; then
	  enable_image=x11,fb
        fi;;
    CYGWIN*)
	enable_image=x11,win;;
    esac
  fi   
  save_ifs="$IFS"; IFS=",";
  set x $enable_image; shift
  IFS="$save_ifs"
  for img in "$[]@"; do
    case $img in
      x11) x11=yes;;
      fb)  fb=yes;;
      fb+s) fb=yes
           AC_DEFINE(W3MIMGDISPLAY_SETUID)
           INSTALL_W3MIMGDISPLAY='${INSTALL} -o root -m 4755 -s'
           AC_DEFINE(INSTALL_W3MIMGDISPLAY, $INSTALL_W3MIMGDISPLAY);;
      win) win=yes;;
    esac
  done
  enable_image=yes
  AC_DEFINE(USE_IMAGE)
  AC_MSG_CHECKING(image library)
  AC_ARG_WITH(imagelib,
   [  --with-imagelib=IMAGELIBS		image library
				 IMAGELIBS may be space separeted list of: 
				    gtk2 gdk-pixbuf imlib imlib2],,

   [with_imagelib="yes"])
  if test x"$with_imagelib" = xyes; then
    with_imagelib="gtk2 gdk-pixbuf imlib imlib2"
  fi
  AC_MSG_RESULT($with_imagelib)
  with_imlib=no
  with_imlib2=no
  with_gdkpixbuf=no
  with_gtk2=no
  for imagelib in $with_imagelib
  do
   case "$imagelib" in
   imlib)
     with_imlib="yes"
     if test x"$IMLIB_CONFIG" = x; then
       IMLIB_CONFIG=imlib-config
     fi;;
   imlib2)
     with_imlib2="yes"
     if test x"$IMLIB2_CONFIG" = x; then
       IMLIB2_CONFIG=imlib2-config
     fi;;
   gdk-pixbuf)
     with_gdkpixbuf="yes"
     if test x"$GDKPIXBUF_CONFIG" = x; then
       GDKPIXBUF_CONFIG=gdk-pixbuf-config
     fi;;
   gtk2)
     with_gtk2="yes"
     if test x"$PKG_CONFIG" = x; then
       PKG_CONFIG=pkg-config
     else
       PKG_CONFIG=:
     fi;;
   esac
  done
  IMGTARGETS=""
  IMGLINK='$(CC)'
  if test x"$with_gtk2" = xyes; then
   AC_W3M_CHECK_VER([GdkPixbuf],
	[`$PKG_CONFIG --modversion gdk-pixbuf-2.0 2>/dev/null`],
	2, 0, 0,
	[have_gdkpixbuf="yes"; have_gtk2="yes"],
	[have_gdkpixbuf="no"; have_gtk2="no"])
  fi
  if test x"$with_gdkpixbuf" = xyes; then
   if test x"$have_gdkpixbuf" != xyes; then
    AC_W3M_CHECK_VER([GdkPixbuf],
	[`$GDKPIXBUF_CONFIG --version 2>/dev/null`],
	0, 16, 0,
	[have_gdkpixbuf="yes"],
	[have_gdkpixbuf="no"])
   fi
  fi
  if test x"$with_imlib" = xyes; then
   AC_W3M_CHECK_VER([Imlib],
	[`$IMLIB_CONFIG --version 2>/dev/null`],
	1, 9, 8,
	[have_imlib="yes"],
	[have_imlib="no"])
  fi
  if test x"$with_imlib2" = xyes; then
   AC_W3M_CHECK_VER([Imlib2],
	[`$IMLIB2_CONFIG --version 2>/dev/null`],
	1, 0, 5,
	[have_imlib2="yes"],
	[have_imlib2="no"])
  fi
  if test x"$x11" = xyes; then
   if test x"$have_gtk2" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     IMGOBJS="$IMGOBJS x11/x11_w3mimg.o"
     IMGTARGETS="x11"    
     AC_DEFINE(USE_GDKPIXBUF)
     AC_DEFINE(USE_GTK2)
     IMGX11CFLAGS="`${PKG_CONFIG} --cflags gdk-pixbuf-2.0 gdk-pixbuf-xlib-2.0 gtk+-2.0`"
     IMGX11LDFLAGS="`${PKG_CONFIG} --libs gdk-pixbuf-2.0 gdk-pixbuf-xlib-2.0 gtk+-2.0`"
   elif test x"$have_gdkpixbuf" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     IMGOBJS="$IMGOBJS x11/x11_w3mimg.o"
     IMGTARGETS="x11"    
     AC_DEFINE(USE_GDKPIXBUF)
     IMGX11CFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${GDKPIXBUF_CONFIG} --libs` -lgdk_pixbuf_xlib"
   elif test x"$have_imlib" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     IMGOBJS="$IMGOBJS x11/x11_w3mimg.o"
     IMGTARGETS="x11"    
     AC_DEFINE(USE_IMLIB)
     IMGX11CFLAGS="`${IMLIB_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB_CONFIG} --libs`"
     IMGTARGETS="x11"    
   elif test x"$have_imlib2" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     IMGOBJS="$IMGOBJS x11/x11_w3mimg.o"
     IMGTARGETS="x11"    
     AC_DEFINE(USE_IMLIB2)
     IMGX11CFLAGS="`${IMLIB2_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB2_CONFIG} --libs`"
   else
     AC_MSG_WARN([unable to build w3mimgdisplay with X11 support])
   fi
  fi
  if test x"$fb" = xyes; then
   if test x"$have_gtk2" = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     IMGOBJS="$IMGOBJS fb/fb_w3mimg.o fb/fb.o fb/fb_img.o"
     IMGTARGETS="${IMGTARGETS} fb"
     AC_DEFINE(USE_GDKPIXBUF)
     AC_DEFINE(USE_GTK2)
     IMGFBCFLAGS="`${PKG_CONFIG} --cflags gdk-pixbuf-2.0 gtk+-2.0`"
     IMGFBLDFLAGS="`${PKG_CONFIG} --libs gdk-pixbuf-2.0 gtk+-2.0`"
   elif test x"$have_gdkpixbuf" = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     IMGOBJS="$IMGOBJS fb/fb_w3mimg.o fb/fb.o fb/fb_img.o"
     IMGTARGETS="${IMGTARGETS} fb"
     AC_DEFINE(USE_GDKPIXBUF)
     IMGFBCFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGFBLDFLAGS="`${GDKPIXBUF_CONFIG} --libs`"
   elif test x"$have_imlib2" = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     IMGOBJS="$IMGOBJS fb/fb_w3mimg.o fb/fb.o fb/fb_img.o"
     IMGTARGETS="${IMGTARGETS} fb"
     AC_DEFINE(USE_IMLIB2)
     IMGOBJS="$IMGOBJS fb/fb_w3mimg.o fb/fb.o fb/fb_img.o"
     IMGFBCFLAGS="`${IMLIB2_CONFIG} --cflags`"
     IMGFBLDFLAGS="`${IMLIB2_CONFIG} --libs`"
   else
     AC_MSG_WARN([unable to build w3mimgdisplay with FB support])
   fi
  fi
  if test x"$win" = xyes; then
    AC_DEFINE(USE_W3MIMG_WIN)
    IMGOBJS="$IMGOBJS win/win_w3mimg.o"
    IMGTARGETS="${IMGTARGETS} win"
    IMGWINCFLAGS="-I/usr/include/w32api"
    IMGWINLDFLAGS="-lgdiplus -lgdi32 -luser32"
    IMGLINK='$(CXX)'
  fi
  AC_DEFINE(IMGTARGETS, "$IMGTARGETS")
  AC_DEFINE(IMGOBJS, "$IMGOBJS")
  AC_DEFINE(IMGX11CFLAGS, "$IMGX11CFLAGS")
  AC_DEFINE(IMGX11LDFLAGS, "$IMGX11LDFLAGS")
  AC_DEFINE(IMGFBCFLAGS, "$IMGFBCFLAGS")
  AC_DEFINE(IMGFBLDFLAGS, "$IMGFBLDFLAGS")
  AC_DEFINE(IMGLINK, "$IMGLINK")
  AC_DEFINE(IMGWINCFLAGS, "$IMGWINCFLAGS")
  AC_DEFINE(IMGWINLDFLAGS, "$IMGWINLDFLAGS")
 fi])
# ----------------------------------------------------------------
# AC_W3M_XFACE
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_XFACE],
[AC_SUBST(USE_XFACE)
 AC_MSG_CHECKING(if xface is enabled)
 AC_ARG_ENABLE(xface,
  [   --disable-xface		disable xface support],,
  [enable_xface="$enable_image"])
 test x"$enable_xface" = xyes && AC_DEFINE(USE_XFACE)
 AC_MSG_RESULT($enable_xface)
 AC_CHECK_PROG(uncompface, uncompface, "yes", "no")
 test "$uncompface" = "no" && AC_MSG_WARN([uncompface is not installed.])
])
#
# ----------------------------------------------------------------
# AC_W3M_IPv6
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_IPv6],
[AC_MSG_CHECKING(if IPv6 support is enabled)
AC_ARG_ENABLE(ipv6,
 [  --disable-ipv6		disable IPv6],,
 [enable_ipv6="yes"])
AC_MSG_RESULT($enable_ipv6)

if test x"$enable_ipv6" = xyes; then
 AC_MSG_CHECKING(if IPv6 API available)
 AC_SUBST(INET6)
 AC_CHECK_FUNC(getaddrinfo, 
	[enable_ipv6="yes"],
	[enable_ipv6="no"])
 if test x"$enable_ipv6" = xno; then
    AC_MSG_CHECKING(for libinet6)
    for dir in /usr/local/v6/lib /usr/local/lib /usr/lib
    do
	if test -f $dir/libinet6.a; then
	  if test $dir != "/usr/lib"; then
		W3M_LIBS="$W3M_LIBS -L$dir"
	  fi
	  AC_CHECK_LIB(inet6, getaddrinfo,
		[enable_ipv6="yes"
	         use_libinet6="found"; W3M_LIBS="$W3M_LIBS -linet6"; break],
		[use_libinet6="not found"])
	fi
    done
    AC_MSG_RESULT($use_libinet6)
 fi
 if test x"$enable_ipv6" = xyes; then
    AC_SUBST(HAVE_OLD_SS_FAMILY)
    AC_MSG_CHECKING(if struct sockaddr_storage has an ss_family member)
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
      ], [
	struct sockaddr_storage ss;
	int i = ss.ss_family;
      ],
      [AC_MSG_RESULT(yes)],
      [AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
	],
	[
	struct sockaddr_storage ss;
	int i = ss.__ss_family;
	],
	[AC_MSG_RESULT(no, but __ss_family exists)
	 AC_DEFINE(HAVE_OLD_SS_FAMILY)],
	[AC_MSG_RESULT(no)
	 AC_MSG_WARN(IPv6 support is disabled)
	 enable_ipv6="no"])
      ])
 fi
 if test x"$enable_ipv6" = xyes; then
    AC_DEFINE(INET6)
 fi
fi])
#
# ----------------------------------------------------------------
# AC_W3M_SYS_ERRLIST
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_SYS_ERRLIST],
[AC_SUBST(HAVE_SYS_ERRLIST)
AC_MSG_CHECKING(for sys_errlist)
AC_TRY_COMPILE(
changequote(<<,>>)dnl
<<extern char *sys_errlist[];>>,
<<printf(sys_errlist[0]);>>,
changequote([,])dnl
[have_sys_errlist="yes"; AC_DEFINE(HAVE_SYS_ERRLIST)],
[have_sys_errlist="no"])
AC_MSG_RESULT($have_sys_errlist)])
#
# ----------------------------------------------------------------
# AC_W3M_SIGSETJMP
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_SIGSETJMP],
[AC_SUBST(HAVE_SIGSETJMP)
AC_MSG_CHECKING(for sigsetjmp)
AC_TRY_COMPILE(
[#include <setjmp.h>],
[ jmp_buf env;
   if (sigsetjmp(env, 1) != 0) { exit(0); } siglongjmp(env, 1);],
[have_sigsetjmp="yes"; AC_DEFINE(HAVE_SIGSETJMP)],
[have_sigsetjmp="no"])
AC_MSG_RESULT($have_sigsetjmp)])
#
# ----------------------------------------------------------------
# AC_W3M_SIGNAL_RETURN
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_SIGNAL],
[AC_TYPE_SIGNAL
 AC_SUBST(RETSIGTYPE)
 AC_SUBST(SIGNAL_RETURN)
 if test x"$ac_cv_type_signal" = xvoid; then
  AC_DEFINE(SIGNAL_RETURN,return)
 else
  AC_DEFINE(SIGNAL_RETURN,return 0)
 fi])
