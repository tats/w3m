dnl w3m autoconf macros
#
# ----------------------------------------------------------------
# AC_W3M_VERSION
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_VERSION],
[AC_SUBST(CURRENT_VERSION)
 cvsver=`$AWK '\$[1] ~ /Id:/ { print \$[3]}' ChangeLog`
 sed -e 's/define CURRENT_VERSION "\(.*\)+cvs/define CURRENT_VERSION "\1+cvs-'$cvsver'/' version.c.in > version.c
 CURRENT_VERSION=`sed -n 's/.*define CURRENT_VERSION *"w3m\/\(.*\)".*$/\1/p' version.c`])
#
# ----------------------------------------------------------------
# AC_W3M_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_COLOR],
[AC_SUBST(USE_COLOR)
AC_MSG_CHECKING(if color escape sequence for kterm/pxvt is enabled)
AC_ARG_ENABLE(color,
 [  --disable-color		disable color escape sequence for kterm/pxvt],,
 [enable_color="yes"])
test x$enable_color = xyes && AC_DEFINE(USE_COLOR)
AC_MSG_RESULT($enable_color)])
#
# ----------------------------------------------------------------
# AC_W3M_ANSI_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_ANSI_COLOR],
[AC_SUBST(USE_ANSI_COLOR)
AC_MSG_CHECKING(if ansi color escape sequence support is enabled)
AC_ARG_ENABLE(ansi_color,
 [  --disable-ansi-color		disable ansi color escape sequence],,
 [enable_ansi_color="yes"])
 test x$enable_ansi_color = xyes && AC_DEFINE(USE_ANSI_COLOR)
 AC_MSG_RESULT($enable_ansi_color)])
#
# ----------------------------------------------------------------
# AC_W3M_BG_COLOR
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_BG_COLOR],
[AC_SUBST(USE_BG_COLOR)
AC_MSG_CHECKING(if background color support is enabled)
AC_ARG_ENABLE(bgcolor,
 [  --disable-bgcolor		disable to set background color],,
 [enable_bgcolor="yes"])
 test x$enable_bgcolor = xyes && AC_DEFINE(USE_BG_COLOR)
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
 test x$enable_menu = xyes && AC_DEFINE(USE_MENU)
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
test x$enable_mouse = xyes && AC_DEFINE(USE_MOUSE)
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
test x$enable_cookie = xyes && AC_DEFINE(USE_COOKIE)
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
 test x$enable_dict = xyes && AC_DEFINE(USE_DICT)
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
 test x$enable_history = xyes && AC_DEFINE(USE_HISTORY)
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
 test x$enable_nntp = xyes && AC_DEFINE(USE_NNTP)
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
 test x$enable_gopher = xyes &&  AC_DEFINE(USE_GOPHER)
 AC_MSG_RESULT($enable_gopher)])
#
# ----------------------------------------------------------------
# AC_W3M_LANG
# ----------------------------------------------------------------
# Checks for Japanese 
AC_DEFUN([AC_W3M_LANG],
[AC_SUBST(W3M_LANG)
AC_SUBST(DISPLAY_CODE)
AC_SUBST(SYSTEM_CODE)
AC_MSG_CHECKING(if japanese support is enabled)
AC_ARG_ENABLE(japanese,
 [  --enable-japanese=CODE	support Japanese character sets, CODE=(S|E|j|N|n|m)],,
 [enable_japanese="no"])
AC_MSG_RESULT($enable_japanese)
if test x$enable_japanese = xno; then
  w3m_lang="en"
  AC_DEFINE(DISPLAY_CODE, 'x')
  AC_DEFINE(SYSTEM_CODE, 'x')
else
  w3m_lang="ja";
  case x$enable_japanese in
  xS) AC_DEFINE_UNQUOTED(DISPLAY_CODE, '$enable_japanese')
      AC_DEFINE(DISPLAY_CODE, 'S');;
  xE|xj|xN|xn|xm) 
      AC_DEFINE_UNQUOTED(DISPLAY_CODE, '$enable_japanese')
      AC_DEFINE(SYSTEM_CODE, 'E');;
  *) AC_DEFINE(DISPLAY_CODE, 'E')
     AC_DEFINE(SYSTEM_CODE, 'E');;
  esac;
fi
W3M_LANGDEF=`echo $w3m_lang | tr 'a-z' 'A-Z'`
W3M_LANG=$W3M_LANGDEF
AC_DEFINE_UNQUOTED(W3M_LANG,$W3M_LANG)])
#
# ----------------------------------------------------------------
# AC_W3M_KANJI_SYMBOLS
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_KANJI_SYMBOLS],
[AC_SUBST(KANJI_SYMBOLS)
if test x$enable_japanese != xno; then
 AC_MSG_CHECKING(if kanji symbols is used)
 AC_ARG_ENABLE(kanjisymbols,
  [   --enable-kanjisymbols	use kanji symbols (enable japanese only)],,
  [enable_kanjisymbols="yes"])
 test x$enable_kanjisymbols = xyes && AC_DEFINE(KANJI_SYMBOLS)
 AC_MSG_RESULT($enable_kanjisymbols)
fi])
#
# ----------------------------------------------------------------
# AC_W3M_KEYMAP
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_KEYMAP],
[AC_SUBST(KEYMAP_FILE)
 w3m_keybind="w3m"
 AC_MSG_CHECKING(lynx style keybind is used)
 AC_ARG_ENABLE(lynx,
  [  --enable-lynx			lynx style keybind],,
  [enable_lynx="no"])
 AC_MSG_RESULT($enable_lynx)
 if test x$enable_lynx = xyes; then
  w3m_keybind="lynx"
  KEYMAP_FILE="keybind_lynx"
 else
  w3m_keybind="w3m"
  KEYMAP_FILE="keybind"
 fi
 AC_SUBST(HELP_FILE)
 HELP_FILE=w3mhelp-${w3m_keybind}_$w3m_lang.html
 AC_DEFINE_UNQUOTED(HELP_FILE, "$HELP_FILE")
 AC_SUBST(KEYBIND)
 AC_DEFINE_UNQUOTED(KEYBIND, $w3m_keybind)])
#
# ----------------------------------------------------------------
# AC_W3M_DIGEST_AUTH
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_DIGEST_AUTH],
[AC_SUBST(USE_DIGEST_AUTH)
 AC_MSG_CHECKING(if digest auth is enabled)
 AC_ARG_ENABLE(digest_auth,
 [  --disable-digest-auth		disable digest auth],,
 [enable_digest_auth="yes"])
 test x$enable_digest_auth = xyes && AC_DEFINE(USE_DIGEST_AUTH)
 AC_MSG_RESULT($enable_digest_auth)])
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
  [test x$with_migemo = xyes || migemo_command="$with_migemo"])
 if test "${with_migemo+set}" = set; then
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
 [w3m_editor=$with_editor])
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
 [w3m_mailer=$with_mailer])
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
 [w3m_browser=$with_browser])
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
 test x$enable_help_cgi = xyes && AC_DEFINE(USE_HELP_CGI)
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
 test x$enable_external_uri_loader = xyes && AC_DEFINE(USE_EXTERNAL_URI_LOADER)
 AC_MSG_RESULT($enable_external_uri_loader)])
# 
# ----------------------------------------------------------------
# AC_W3M_W3MMAILER
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_W3MMAILER],
[AC_SUBST(USE_W3MMAILER)
 AC_MSG_CHECKING(if w3mmail is used)
 AC_ARG_ENABLE(w3mmailer,
 [  --disable-w3mmailer		disable w3mmailer],,
 [enable_w3mmailer="$enable_external_uri_loader"])
 test x$enable_external_uri_loader = xno && enable_w3mmailer=no
 test x$enable_w3mmailer = xyes && AC_DEFINE(USE_W3MMAILER)
 AC_MSG_RESULT($enable_w3mmailer)])
#
# ----------------------------------------------------------------
# AC_W3M_EXTLIBS(libs)
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_EXTLIBS],
[lib=$1
 AC_MSG_CHECKING(for -l$lib)
 extlib="not found"
 for dir in /lib /usr/lib /usr/local/lib /usr/ucblib /usr/ccslib /usr/ccs/lib
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
# AC_W3M_GC
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_GC],
[AC_MSG_CHECKING(GC library exists)
AC_ARG_WITH(gc,
 [  --with-gc[=PREFIX]	  	libgc PREFIX],
 [test x$with_gc = xno && AC_MSG_ERROR([You can not build w3m without gc])],
 [with_gc=yes])
 AC_MSG_RESULT($with_gc)
 unset ac_cv_header_gc_h
 AC_CHECK_HEADER(gc.h)
 if test x$ac_cv_header_gc_h = xno; then
   AC_MSG_CHECKING(GC header location)
   AC_MSG_RESULT()
   gc_includedir="$with_gc/include"
   test x"$with_gc" = xyes && gc_includedir="/usr/include /usr/include/gc /usr/local/include /usr/local/include/gc ${HOME}/include"
   gcincludedir=no
   for dir in $gc_includedir; do
     cppflags="$CPPFLAGS"
     CPPFLAGS="$CPPFLAGS -I$dir"
     AC_MSG_CHECKING($dir)
     unset ac_cv_header_gc_h
     AC_CHECK_HEADER(gc.h, [gcincludedir=$dir; CPPFLAGS="$CPPFLAGS -I$dir"; CFLAGS="$CFLAGS -I$dir"; break])
     CPPFLAGS="$cppflags"
   done
   if test x$gcincludedir = xno; then
     AC_MSG_ERROR([gc.h not found])
   fi
 fi
 unset ac_cv_lib_gc_GC_version
 AC_CHECK_LIB(gc, GC_version, [LIBS="$LIBS -lgc"])
 if test x$ac_cv_lib_gc_GC_version = xno; then
    AC_MSG_CHECKING(GC library location)
    AC_MSG_RESULT()
    gc_libdir="$with_gc/lib"
    test x"$gc_libdir" = xyes && gc_libdir="/lib /usr/lib /usr/local/lib /usr/ucblib /usr/ccslib /usr/ccs/lib ${HOME}/lib"
    gclibdir=no
    for dir in $gc_libdir; do
      ldflags="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$dir"
      AC_MSG_CHECKING($dir)
      unset ac_cv_gc_GC_version
      AC_CHECK_LIB(gc, GC_version, [gclibdir=$dir; LIBS="$LIBS -L$dir -lgc"; break])
      LDFLAGS="$ldflags"
    done
    if test x$gclibdir = xno; then
      AC_MSG_ERROR([libgc not found])
    fi
 fi])
#
# ----------------------------------------------------------------
# AC_W3M_SSL
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_SSL],
[AC_SUBST(USE_SSL)
AC_SUBST(USE_SSL_VERIFY)
AC_MSG_CHECKING(if SSL is suported)
AC_ARG_WITH(ssl,
 [  --with-ssl[=PREFIX]		support https protocol],,
 [with_ssl="yes"])
AC_MSG_RESULT($with_ssl)
if test x$with_ssl != xno; then
  AC_DEFINE(USE_SSL)
  AC_MSG_CHECKING(for SSL library/header)
  test x"$with_ssl" = xyes || with_ssl="/usr/openssl /usr/ssl /usr /usr/local/openssl /usr/local/ssl /usr/local"
  for dir in $with_ssl
  do
     if test -f "$dir/include/openssl/ssl.h"; then
        CFLAGS="$CFLAGS -I$dir/include/openssl"
     elif test -f "$dir/include/ssl.h"; then
        CFLAGS="$CFLAGS -I$dir/include"
     fi
     if test -f "$dir/lib/libssl.a"; then
	LIBS="$LIBS -L$dir/lib"
     fi
  done
  AC_CHECK_LIB(ssl, SSL_new,
	[w3m_ssl="found"; LIBS="$LIBS -lssl -lcrypto"],
	[w3m_ssl="not found"],
	[-lcrypto])

  if test x$w3m_ssl = xfound; then
    AC_MSG_CHECKING(if SSL certificate verify is enabled)
    AC_ARG_ENABLE(sslverify,
      [   --disable-sslverify		vefify SSL certificate],,
      [enable_sslverify="yes"])
    test x$enable_sslverify = xyes && AC_DEFINE(USE_SSL_VERIFY)
    AC_MSG_RESULT($enable_sslverify)
  fi
fi])
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
 if test x$enable_alarm = xyes; then
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
[version=$2
 if test x$version != x; then
   AC_MSG_CHECKING($1 version)
   AC_MSG_RESULT($version)
   set -- `echo "$version" | sed 's/[[^0-9]]/ /g'`
   if test "$[1]" -ne "$3" -o "$[2]" -lt "$4" -o "$[3]" -lt "$5"; then
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
 AC_SUBST(W3MIMGDISPLAY_SETUID)
 AC_SUBST(INSTALL_W3MIMGDISPLAY)
 INSTALL_W3MIMGDISPLAY='${INSTALL_PROGRAM}'
 AC_DEFINE(INSTALL_W3MIMGDISPLAY, $INSTALL_W3MIMGDISPLAY)
 AC_SUBST(USE_GDKPIXBUF)
 AC_SUBST(USE_IMLIB)
 AC_SUBST(USE_IMLIB2)
 AC_SUBST(IMGOBJS)
 AC_SUBST(IMGX11CFLAGS)
 AC_SUBST(IMGX11LDFLAGS)
 AC_SUBST(IMGFBCFLAGS)
 AC_SUBST(IMGFBLDFLAGS)
 AC_MSG_CHECKING(if image is enabled)
 AC_ARG_ENABLE(image,
 [  --disable-image=x11,fb,fb+s	disable inline image],,
 [enable_image="x11,fb"])
 AC_MSG_RESULT($enable_image)
 if test x$enable_image != xno; then
  IMGOBJS=w3mimg/w3mimg.o
  if test x$enable_image = xyes; then
    enable_image=x11
    case "`uname -s`" in
    Linux|linux|LINUX) 
	if test -c /dev/fb0; then
	  enable_image=x11,fb
        fi;;
    esac
  fi   
  save_ifs="$IFS"; IFS=",";
  for img in $enable_image; do
    case $img in
      x11) x11=yes;;
      fb)  fb=yes;;
      fb+s) fb=yes
           AC_DEFINE(W3MIMGDISPLAY_SETUID)
           INSTALL_W3MIMGDISPLAY='${INSTALL} -o root -m 4755 -s'
           AC_DEFINE(INSTALL_W3MIMGDISPLAY, $INSTALL_W3MIMGDISPLAY);;
    esac
  done
  IFS="$save_ifs"
  enable_image=yes
  AC_DEFINE(USE_IMAGE)
  if test x$IMLIB_CONFIG = x; then
    IMLIB_CONFIG=imlib-config
  fi
  if test x$IMLIB2_CONFIG = x; then
    IMLIB2_CONFIG=imlib2-config
  fi
  if test x$GDKPIXBUF_CONFIG = x; then
    GDKPIXBUF_CONFIG=gdk-pixbuf-config
  fi
  AC_W3M_CHECK_VER([GdkPixbuf],
	[`$GDKPIXBUF_CONFIG --version 2>/dev/null`],
	0, 16, 0,
	[have_gdkpixbuf=yes],
	[have_gdkpixbuf=no
  AC_W3M_CHECK_VER([Imlib],
	[`$IMLIB_CONFIG --version 2>/dev/null`],
	1, 9, 8,
	[have_imlib=yes],
	[have_imlib=no])
  AC_W3M_CHECK_VER([Imlib2],
	[`$IMLIB2_CONFIG --version 2>/dev/null`],
	1, 0, 5,
	[have_imlib2=yes],
	[have_imlib2=no])])
  if test x$x11 = xyes; then
   if test x$have_gdkpixbuf = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_GDKPIXBUF)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${GDKPIXBUF_CONFIG} --libs` -lgdk_pixbuf_xlib"
   elif test x$have_imlib = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_IMLIB)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${IMLIB_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB_CONFIG} --libs`"
   elif test x$have_imlib2 = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_IMLIB2)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${IMLIB2_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB2_CONFIG} --libs`"
   else
     AC_MSG_WARN([unable to build w3mimgdisplay with X11 support])
   fi
  fi
  if test x$fb = xyes; then
   if test x$have_gdkpixbuf = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     AC_DEFINE(USE_GDKPIXBUF)
     IMGOBJS="$IMGOBJS w3mimg/fb/fb_w3mimg.o w3mimg/fb/fb.o w3mimg/fb/fb_img.o"
     IMGFBCFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGFBLDFLAGS="`${GDKPIXBUF_CONFIG} --libs`"
   elif test x$have_imlib2 = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     AC_DEFINE(USE_IMLIB2)
     IMGOBJS="$IMGOBJS w3mimg/fb/fb_w3mimg.o w3mimg/fb/fb.o w3mimg/fb/fb_img.o"
     IMGFBCFLAGS="`${IMLIB2_CONFIG} --cflags`"
     IMGFBLDFLAGS="`${IMLIB2_CONFIG} --libs`"
   else
     AC_MSG_WARN([unable to build w3mimgdisplay with FB support])
   fi
  fi
  AC_DEFINE(IMGOBJS, "$IMGOBJS")
  AC_DEFINE(IMGX11CFLAGS, "$IMGX11CFLAGS")
  AC_DEFINE(IMGX11LDFLAGS, "$IMGX11LDFLAGS")
  AC_DEFINE(IMGFBCFLAGS, "$IMGFBCFLAGS")
  AC_DEFINE(IMGFBLDFLAGS, "$IMGLDFLAGS")
 fi])
# ----------------------------------------------------------------
# AC_W3M_XFACE
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_XFACE],
[AC_SUBST(USE_XFACE)
 AC_MSG_CHECKING(if xface is enabled)
 AC_ARG_ENABLE(xface,
  [   --enable-xface		enable xface support],,
  [enable_xface="$enable_image"])
 test x$enable_xface = xyes && AC_DEFINE(USE_XFACE)
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

if test x$enable_ipv6 = xyes; then
 AC_MSG_CHECKING(if IPv6 API available)
 AC_SUBST(INET6)
 AC_CHECK_FUNC(getaddrinfo, 
	[enable_ipv6=yes; AC_DEFINE(INET6)],
	[enable_ipv6=no])
 if test x$enable_ipv6 = xno; then
    AC_MSG_CHECKING(for libinet6)
    for dir in /usr/local/v6/lib /usr/local/lib /usr/lib
    do
	if test -f $dir/libinet6.a; then
	  if test $dir != "/usr/lib"; then
		LIBS="$LIBS -L$dir"
	  fi
	  AC_CHECK_LIB(inet6, getaddrinfo,
		[enable_ipv6=yes; AC_DEFINE(INET6)
	         use_libinet6="found"; LIBS="$LIBS -linet6"; break],
		[use_libinet6="not found"])
	fi
    done
    AC_MSG_RESULT($use_libinet6)
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
 if test x$ac_cv_type_signal = xvoid; then
  AC_DEFINE(SIGNAL_RETURN,return)
 else
  AC_DEFINE(SIGNAL_RETURN,return 0)
 fi])
