dnl aclocal.m4 generated automatically by aclocal 1.4-p6

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

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
# AC_W3M_LANG
# ----------------------------------------------------------------
# Checks for Japanese 
AC_DEFUN([AC_W3M_LANG],
[AC_SUBST(W3M_LANG)
AC_SUBST(DISPLAY_CODE)
AC_SUBST(SYSTEM_CODE)
AC_MSG_CHECKING(if japanese support is enabled)
AC_ARG_ENABLE(japanese,
 [  --enable-japanese=CODE	support Japanese character sets
				CODE=(S|E|j|N|n|m)],,
 [enable_japanese="no"])
AC_MSG_RESULT($enable_japanese)
if test x"$enable_japanese" = xno; then
  w3m_lang="en"
  AC_DEFINE(DISPLAY_CODE, 'x')
  AC_DEFINE(SYSTEM_CODE, 'x')
else
  w3m_lang="ja";
  case x"$enable_japanese" in
  xS) AC_DEFINE_UNQUOTED(DISPLAY_CODE, '$enable_japanese')
      AC_DEFINE(SYSTEM_CODE, 'S');;
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
if test x"$enable_japanese" != xno; then
 AC_MSG_CHECKING(if kanji symbols is used)
 AC_ARG_ENABLE(kanjisymbols,
  [   --disable-kanjisymbols	use kanji symbols (enable japanese only)],,
  [enable_kanjisymbols="yes"])
 test x"$enable_kanjisymbols" = xyes && AC_DEFINE(KANJI_SYMBOLS)
 AC_MSG_RESULT($enable_kanjisymbols)
fi])
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
# AC_W3M_DIGEST_AUTH
# ----------------------------------------------------------------
AC_DEFUN([AC_W3M_DIGEST_AUTH],
[AC_SUBST(USE_DIGEST_AUTH)
 AC_MSG_CHECKING(if digest auth is enabled)
 AC_ARG_ENABLE(digest_auth,
 [  --disable-digest-auth		disable digest auth],,
 [enable_digest_auth="yes"])
 test x"$enable_digest_auth" = xyes && AC_DEFINE(USE_DIGEST_AUTH)
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
 unset ac_cv_lib_gc_GC_version
 AC_CHECK_LIB(gc, GC_version, [LIBS="$LIBS -lgc"])
 if test x"$ac_cv_lib_gc_GC_version" = xno; then
    AC_MSG_CHECKING(GC library location)
    AC_MSG_RESULT($with_gc)
    gclibdir=no
    for dir in $with_gc; do
      ldflags="$LDFLAGS"
      LDFLAGS="$LDFLAGS -L$dir/lib"
      AC_MSG_CHECKING($dir)
      unset ac_cv_lib_gc_GC_version
      AC_CHECK_LIB(gc, GC_version, [gclibdir="$dir/lib"; LIBS="$LIBS -L$dir/lib -lgc"; break])
      LDFLAGS="$ldflags"
    done
    if test x"$gclibdir" = xno; then
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
if test x"$with_ssl" != xno; then
  AC_DEFINE(USE_SSL)
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
  AC_CHECK_LIB(ssl, SSL_new,
	[w3m_ssl="found"; CFLAGS="$CFLAGS $SSL_CFLAGS" W3M_LIBS="$W3M_LIBS $SSL_LIBS -lssl -lcrypto"],
	[w3m_ssl="not found"],
	[$SSL_LIBS -lcrypto])

  if test x"$w3m_ssl" = xfound; then
    AC_MSG_CHECKING(if SSL certificate verify is enabled)
    AC_ARG_ENABLE(sslverify,
      [   --disable-sslverify		verify SSL certificate],,
      [enable_sslverify="yes"])
    test x"$enable_sslverify" = xyes && AC_DEFINE(USE_SSL_VERIFY)
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
 [  --enable-image[=DEVS]		enable inline image handler for DEVS
				 DEVS may be comma separeted: x11,fb,fb+s
				 default: autodetected.
				 'no' means disable inline image],,
 [enable_image="yes"])
 AC_MSG_RESULT($enable_image)
 if test x"$enable_image" != xno; then
  IMGOBJS=w3mimg/w3mimg.o
  if test x"$enable_image" = xyes; then
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
  if test x"$IMLIB_CONFIG" = x; then
    IMLIB_CONFIG=imlib-config
  fi
  if test x"$IMLIB2_CONFIG" = x; then
    IMLIB2_CONFIG=imlib2-config
  fi
  if test x"$GDKPIXBUF_CONFIG" = x; then
    GDKPIXBUF_CONFIG=gdk-pixbuf-config
  fi
  AC_W3M_CHECK_VER([GdkPixbuf],
	[`$GDKPIXBUF_CONFIG --version 2>/dev/null`],
	0, 16, 0,
	[have_gdkpixbuf="yes"],
	[have_gdkpixbuf="no"
  AC_W3M_CHECK_VER([Imlib],
	[`$IMLIB_CONFIG --version 2>/dev/null`],
	1, 9, 8,
	[have_imlib="yes"],
	[have_imlib="no"])
  AC_W3M_CHECK_VER([Imlib2],
	[`$IMLIB2_CONFIG --version 2>/dev/null`],
	1, 0, 5,
	[have_imlib2="yes"],
	[have_imlib2="no"])])
  if test x"$x11" = xyes; then
   if test x"$have_gdkpixbuf" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_GDKPIXBUF)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${GDKPIXBUF_CONFIG} --libs` -lgdk_pixbuf_xlib"
   elif test x"$have_imlib" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_IMLIB)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${IMLIB_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB_CONFIG} --libs`"
   elif test x"$have_imlib2" = xyes; then
     AC_DEFINE(USE_W3MIMG_X11)
     AC_DEFINE(USE_IMLIB2)
     IMGOBJS="$IMGOBJS w3mimg/x11/x11_w3mimg.o"
     IMGX11CFLAGS="`${IMLIB2_CONFIG} --cflags`"
     IMGX11LDFLAGS="`${IMLIB2_CONFIG} --libs`"
   else
     AC_MSG_WARN([unable to build w3mimgdisplay with X11 support])
   fi
  fi
  if test x"$fb" = xyes; then
   if test x"$have_gdkpixbuf" = xyes; then
     AC_DEFINE(USE_W3MIMG_FB)
     AC_DEFINE(USE_GDKPIXBUF)
     IMGOBJS="$IMGOBJS w3mimg/fb/fb_w3mimg.o w3mimg/fb/fb.o w3mimg/fb/fb_img.o"
     IMGFBCFLAGS="`${GDKPIXBUF_CONFIG} --cflags`"
     IMGFBLDFLAGS="`${GDKPIXBUF_CONFIG} --libs`"
   elif test x"$have_imlib2" = xyes; then
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
	[enable_ipv6="yes"; AC_DEFINE(INET6)],
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
		[enable_ipv6="yes"; AC_DEFINE(INET6)
	         use_libinet6="found"; W3M_LIBS="$W3M_LIBS -linet6"; break],
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
 if test x"$ac_cv_type_signal" = xvoid; then
  AC_DEFINE(SIGNAL_RETURN,return)
 else
  AC_DEFINE(SIGNAL_RETURN,return 0)
 fi])

# lib-prefix.m4 serial 3 (gettext-0.12.2)
dnl Copyright (C) 2001-2003 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl From Bruno Haible.

dnl AC_LIB_ARG_WITH is synonymous to AC_ARG_WITH in autoconf-2.13, and
dnl similar to AC_ARG_WITH in autoconf 2.52...2.57 except that is doesn't
dnl require excessive bracketing.
ifdef([AC_HELP_STRING],
[AC_DEFUN([AC_LIB_ARG_WITH], [AC_ARG_WITH([$1],[[$2]],[$3],[$4])])],
[AC_DEFUN([AC_][LIB_ARG_WITH], [AC_ARG_WITH([$1],[$2],[$3],[$4])])])

dnl AC_LIB_PREFIX adds to the CPPFLAGS and LDFLAGS the flags that are needed
dnl to access previously installed libraries. The basic assumption is that
dnl a user will want packages to use other packages he previously installed
dnl with the same --prefix option.
dnl This macro is not needed if only AC_LIB_LINKFLAGS is used to locate
dnl libraries, but is otherwise very convenient.
AC_DEFUN([AC_LIB_PREFIX],
[
  AC_BEFORE([$0], [AC_LIB_LINKFLAGS])
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  dnl By default, look in $includedir and $libdir.
  use_additional=yes
  AC_LIB_WITH_FINAL_PREFIX([
    eval additional_includedir=\"$includedir\"
    eval additional_libdir=\"$libdir\"
  ])
  AC_LIB_ARG_WITH([lib-prefix],
[  --with-lib-prefix[=DIR] search for libraries in DIR/include and DIR/lib
  --without-lib-prefix    don't search for libraries in includedir and libdir],
[
    if test "X$withval" = "Xno"; then
      use_additional=no
    else
      if test "X$withval" = "X"; then
        AC_LIB_WITH_FINAL_PREFIX([
          eval additional_includedir=\"$includedir\"
          eval additional_libdir=\"$libdir\"
        ])
      else
        additional_includedir="$withval/include"
        additional_libdir="$withval/lib"
      fi
    fi
])
  if test $use_additional = yes; then
    dnl Potentially add $additional_includedir to $CPPFLAGS.
    dnl But don't add it
    dnl   1. if it's the standard /usr/include,
    dnl   2. if it's already present in $CPPFLAGS,
    dnl   3. if it's /usr/local/include and we are using GCC on Linux,
    dnl   4. if it doesn't exist as a directory.
    if test "X$additional_includedir" != "X/usr/include"; then
      haveit=
      for x in $CPPFLAGS; do
        AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
        if test "X$x" = "X-I$additional_includedir"; then
          haveit=yes
          break
        fi
      done
      if test -z "$haveit"; then
        if test "X$additional_includedir" = "X/usr/local/include"; then
          if test -n "$GCC"; then
            case $host_os in
              linux*) haveit=yes;;
            esac
          fi
        fi
        if test -z "$haveit"; then
          if test -d "$additional_includedir"; then
            dnl Really add $additional_includedir to $CPPFLAGS.
            CPPFLAGS="${CPPFLAGS}${CPPFLAGS:+ }-I$additional_includedir"
          fi
        fi
      fi
    fi
    dnl Potentially add $additional_libdir to $LDFLAGS.
    dnl But don't add it
    dnl   1. if it's the standard /usr/lib,
    dnl   2. if it's already present in $LDFLAGS,
    dnl   3. if it's /usr/local/lib and we are using GCC on Linux,
    dnl   4. if it doesn't exist as a directory.
    if test "X$additional_libdir" != "X/usr/lib"; then
      haveit=
      for x in $LDFLAGS; do
        AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
        if test "X$x" = "X-L$additional_libdir"; then
          haveit=yes
          break
        fi
      done
      if test -z "$haveit"; then
        if test "X$additional_libdir" = "X/usr/local/lib"; then
          if test -n "$GCC"; then
            case $host_os in
              linux*) haveit=yes;;
            esac
          fi
        fi
        if test -z "$haveit"; then
          if test -d "$additional_libdir"; then
            dnl Really add $additional_libdir to $LDFLAGS.
            LDFLAGS="${LDFLAGS}${LDFLAGS:+ }-L$additional_libdir"
          fi
        fi
      fi
    fi
  fi
])

dnl AC_LIB_PREPARE_PREFIX creates variables acl_final_prefix,
dnl acl_final_exec_prefix, containing the values to which $prefix and
dnl $exec_prefix will expand at the end of the configure script.
AC_DEFUN([AC_LIB_PREPARE_PREFIX],
[
  dnl Unfortunately, prefix and exec_prefix get only finally determined
  dnl at the end of configure.
  if test "X$prefix" = "XNONE"; then
    acl_final_prefix="$ac_default_prefix"
  else
    acl_final_prefix="$prefix"
  fi
  if test "X$exec_prefix" = "XNONE"; then
    acl_final_exec_prefix='${prefix}'
  else
    acl_final_exec_prefix="$exec_prefix"
  fi
  acl_save_prefix="$prefix"
  prefix="$acl_final_prefix"
  eval acl_final_exec_prefix=\"$acl_final_exec_prefix\"
  prefix="$acl_save_prefix"
])

dnl AC_LIB_WITH_FINAL_PREFIX([statement]) evaluates statement, with the
dnl variables prefix and exec_prefix bound to the values they will have
dnl at the end of the configure script.
AC_DEFUN([AC_LIB_WITH_FINAL_PREFIX],
[
  acl_save_prefix="$prefix"
  prefix="$acl_final_prefix"
  acl_save_exec_prefix="$exec_prefix"
  exec_prefix="$acl_final_exec_prefix"
  $1
  exec_prefix="$acl_save_exec_prefix"
  prefix="$acl_save_prefix"
])

# lib-link.m4 serial 4 (gettext-0.12)
dnl Copyright (C) 2001-2003 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl From Bruno Haible.

dnl AC_LIB_LINKFLAGS(name [, dependencies]) searches for libname and
dnl the libraries corresponding to explicit and implicit dependencies.
dnl Sets and AC_SUBSTs the LIB${NAME} and LTLIB${NAME} variables and
dnl augments the CPPFLAGS variable.
AC_DEFUN([AC_LIB_LINKFLAGS],
[
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])
  define([Name],[translit([$1],[./-], [___])])
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./-],
                               [ABCDEFGHIJKLMNOPQRSTUVWXYZ___])])
  AC_CACHE_CHECK([how to link with lib[]$1], [ac_cv_lib[]Name[]_libs], [
    AC_LIB_LINKFLAGS_BODY([$1], [$2])
    ac_cv_lib[]Name[]_libs="$LIB[]NAME"
    ac_cv_lib[]Name[]_ltlibs="$LTLIB[]NAME"
    ac_cv_lib[]Name[]_cppflags="$INC[]NAME"
  ])
  LIB[]NAME="$ac_cv_lib[]Name[]_libs"
  LTLIB[]NAME="$ac_cv_lib[]Name[]_ltlibs"
  INC[]NAME="$ac_cv_lib[]Name[]_cppflags"
  AC_LIB_APPENDTOVAR([CPPFLAGS], [$INC]NAME)
  AC_SUBST([LIB]NAME)
  AC_SUBST([LTLIB]NAME)
  dnl Also set HAVE_LIB[]NAME so that AC_LIB_HAVE_LINKFLAGS can reuse the
  dnl results of this search when this library appears as a dependency.
  HAVE_LIB[]NAME=yes
  undefine([Name])
  undefine([NAME])
])

dnl AC_LIB_HAVE_LINKFLAGS(name, dependencies, includes, testcode)
dnl searches for libname and the libraries corresponding to explicit and
dnl implicit dependencies, together with the specified include files and
dnl the ability to compile and link the specified testcode. If found, it
dnl sets and AC_SUBSTs HAVE_LIB${NAME}=yes and the LIB${NAME} and
dnl LTLIB${NAME} variables and augments the CPPFLAGS variable, and
dnl #defines HAVE_LIB${NAME} to 1. Otherwise, it sets and AC_SUBSTs
dnl HAVE_LIB${NAME}=no and LIB${NAME} and LTLIB${NAME} to empty.
AC_DEFUN([AC_LIB_HAVE_LINKFLAGS],
[
  AC_REQUIRE([AC_LIB_PREPARE_PREFIX])
  AC_REQUIRE([AC_LIB_RPATH])
  define([Name],[translit([$1],[./-], [___])])
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./-],
                               [ABCDEFGHIJKLMNOPQRSTUVWXYZ___])])

  dnl Search for lib[]Name and define LIB[]NAME, LTLIB[]NAME and INC[]NAME
  dnl accordingly.
  AC_LIB_LINKFLAGS_BODY([$1], [$2])

  dnl Add $INC[]NAME to CPPFLAGS before performing the following checks,
  dnl because if the user has installed lib[]Name and not disabled its use
  dnl via --without-lib[]Name-prefix, he wants to use it.
  ac_save_CPPFLAGS="$CPPFLAGS"
  AC_LIB_APPENDTOVAR([CPPFLAGS], [$INC]NAME)

  AC_CACHE_CHECK([for lib[]$1], [ac_cv_lib[]Name], [
    ac_save_LIBS="$LIBS"
    LIBS="$LIBS $LIB[]NAME"
    AC_TRY_LINK([$3], [$4], [ac_cv_lib[]Name=yes], [ac_cv_lib[]Name=no])
    LIBS="$ac_save_LIBS"
  ])
  if test "$ac_cv_lib[]Name" = yes; then
    HAVE_LIB[]NAME=yes
    AC_DEFINE([HAVE_LIB]NAME, 1, [Define if you have the $1 library.])
    AC_MSG_CHECKING([how to link with lib[]$1])
    AC_MSG_RESULT([$LIB[]NAME])
  else
    HAVE_LIB[]NAME=no
    dnl If $LIB[]NAME didn't lead to a usable library, we don't need
    dnl $INC[]NAME either.
    CPPFLAGS="$ac_save_CPPFLAGS"
    LIB[]NAME=
    LTLIB[]NAME=
  fi
  AC_SUBST([HAVE_LIB]NAME)
  AC_SUBST([LIB]NAME)
  AC_SUBST([LTLIB]NAME)
  undefine([Name])
  undefine([NAME])
])

dnl Determine the platform dependent parameters needed to use rpath:
dnl libext, shlibext, hardcode_libdir_flag_spec, hardcode_libdir_separator,
dnl hardcode_direct, hardcode_minus_L.
AC_DEFUN([AC_LIB_RPATH],
[
  AC_REQUIRE([AC_PROG_CC])                dnl we use $CC, $GCC, $LDFLAGS
  AC_REQUIRE([AC_LIB_PROG_LD])            dnl we use $LD, $with_gnu_ld
  AC_REQUIRE([AC_CANONICAL_HOST])         dnl we use $host
  AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT]) dnl we use $ac_aux_dir
  AC_CACHE_CHECK([for shared library run path origin], acl_cv_rpath, [
    CC="$CC" GCC="$GCC" LDFLAGS="$LDFLAGS" LD="$LD" with_gnu_ld="$with_gnu_ld" \
    ${CONFIG_SHELL-/bin/sh} "$ac_aux_dir/config.rpath" "$host" > conftest.sh
    . ./conftest.sh
    rm -f ./conftest.sh
    acl_cv_rpath=done
  ])
  wl="$acl_cv_wl"
  libext="$acl_cv_libext"
  shlibext="$acl_cv_shlibext"
  hardcode_libdir_flag_spec="$acl_cv_hardcode_libdir_flag_spec"
  hardcode_libdir_separator="$acl_cv_hardcode_libdir_separator"
  hardcode_direct="$acl_cv_hardcode_direct"
  hardcode_minus_L="$acl_cv_hardcode_minus_L"
  dnl Determine whether the user wants rpath handling at all.
  AC_ARG_ENABLE(rpath,
    [  --disable-rpath         do not hardcode runtime library paths],
    :, enable_rpath=yes)
])

dnl AC_LIB_LINKFLAGS_BODY(name [, dependencies]) searches for libname and
dnl the libraries corresponding to explicit and implicit dependencies.
dnl Sets the LIB${NAME}, LTLIB${NAME} and INC${NAME} variables.
AC_DEFUN([AC_LIB_LINKFLAGS_BODY],
[
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./-],
                               [ABCDEFGHIJKLMNOPQRSTUVWXYZ___])])
  dnl By default, look in $includedir and $libdir.
  use_additional=yes
  AC_LIB_WITH_FINAL_PREFIX([
    eval additional_includedir=\"$includedir\"
    eval additional_libdir=\"$libdir\"
  ])
  AC_LIB_ARG_WITH([lib$1-prefix],
[  --with-lib$1-prefix[=DIR]  search for lib$1 in DIR/include and DIR/lib
  --without-lib$1-prefix     don't search for lib$1 in includedir and libdir],
[
    if test "X$withval" = "Xno"; then
      use_additional=no
    else
      if test "X$withval" = "X"; then
        AC_LIB_WITH_FINAL_PREFIX([
          eval additional_includedir=\"$includedir\"
          eval additional_libdir=\"$libdir\"
        ])
      else
        additional_includedir="$withval/include"
        additional_libdir="$withval/lib"
      fi
    fi
])
  dnl Search the library and its dependencies in $additional_libdir and
  dnl $LDFLAGS. Using breadth-first-seach.
  LIB[]NAME=
  LTLIB[]NAME=
  INC[]NAME=
  rpathdirs=
  ltrpathdirs=
  names_already_handled=
  names_next_round='$1 $2'
  while test -n "$names_next_round"; do
    names_this_round="$names_next_round"
    names_next_round=
    for name in $names_this_round; do
      already_handled=
      for n in $names_already_handled; do
        if test "$n" = "$name"; then
          already_handled=yes
          break
        fi
      done
      if test -z "$already_handled"; then
        names_already_handled="$names_already_handled $name"
        dnl See if it was already located by an earlier AC_LIB_LINKFLAGS
        dnl or AC_LIB_HAVE_LINKFLAGS call.
        uppername=`echo "$name" | sed -e 'y|abcdefghijklmnopqrstuvwxyz./-|ABCDEFGHIJKLMNOPQRSTUVWXYZ___|'`
        eval value=\"\$HAVE_LIB$uppername\"
        if test -n "$value"; then
          if test "$value" = yes; then
            eval value=\"\$LIB$uppername\"
            test -z "$value" || LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$value"
            eval value=\"\$LTLIB$uppername\"
            test -z "$value" || LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }$value"
          else
            dnl An earlier call to AC_LIB_HAVE_LINKFLAGS has determined
            dnl that this library doesn't exist. So just drop it.
            :
          fi
        else
          dnl Search the library lib$name in $additional_libdir and $LDFLAGS
          dnl and the already constructed $LIBNAME/$LTLIBNAME.
          found_dir=
          found_la=
          found_so=
          found_a=
          if test $use_additional = yes; then
            if test -n "$shlibext" && test -f "$additional_libdir/lib$name.$shlibext"; then
              found_dir="$additional_libdir"
              found_so="$additional_libdir/lib$name.$shlibext"
              if test -f "$additional_libdir/lib$name.la"; then
                found_la="$additional_libdir/lib$name.la"
              fi
            else
              if test -f "$additional_libdir/lib$name.$libext"; then
                found_dir="$additional_libdir"
                found_a="$additional_libdir/lib$name.$libext"
                if test -f "$additional_libdir/lib$name.la"; then
                  found_la="$additional_libdir/lib$name.la"
                fi
              fi
            fi
          fi
          if test "X$found_dir" = "X"; then
            for x in $LDFLAGS $LTLIB[]NAME; do
              AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
              case "$x" in
                -L*)
                  dir=`echo "X$x" | sed -e 's/^X-L//'`
                  if test -n "$shlibext" && test -f "$dir/lib$name.$shlibext"; then
                    found_dir="$dir"
                    found_so="$dir/lib$name.$shlibext"
                    if test -f "$dir/lib$name.la"; then
                      found_la="$dir/lib$name.la"
                    fi
                  else
                    if test -f "$dir/lib$name.$libext"; then
                      found_dir="$dir"
                      found_a="$dir/lib$name.$libext"
                      if test -f "$dir/lib$name.la"; then
                        found_la="$dir/lib$name.la"
                      fi
                    fi
                  fi
                  ;;
              esac
              if test "X$found_dir" != "X"; then
                break
              fi
            done
          fi
          if test "X$found_dir" != "X"; then
            dnl Found the library.
            LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }-L$found_dir -l$name"
            if test "X$found_so" != "X"; then
              dnl Linking with a shared library. We attempt to hardcode its
              dnl directory into the executable's runpath, unless it's the
              dnl standard /usr/lib.
              if test "$enable_rpath" = no || test "X$found_dir" = "X/usr/lib"; then
                dnl No hardcoding is needed.
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
              else
                dnl Use an explicit option to hardcode DIR into the resulting
                dnl binary.
                dnl Potentially add DIR to ltrpathdirs.
                dnl The ltrpathdirs will be appended to $LTLIBNAME at the end.
                haveit=
                for x in $ltrpathdirs; do
                  if test "X$x" = "X$found_dir"; then
                    haveit=yes
                    break
                  fi
                done
                if test -z "$haveit"; then
                  ltrpathdirs="$ltrpathdirs $found_dir"
                fi
                dnl The hardcoding into $LIBNAME is system dependent.
                if test "$hardcode_direct" = yes; then
                  dnl Using DIR/libNAME.so during linking hardcodes DIR into the
                  dnl resulting binary.
                  LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                else
                  if test -n "$hardcode_libdir_flag_spec" && test "$hardcode_minus_L" = no; then
                    dnl Use an explicit option to hardcode DIR into the resulting
                    dnl binary.
                    LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                    dnl Potentially add DIR to rpathdirs.
                    dnl The rpathdirs will be appended to $LIBNAME at the end.
                    haveit=
                    for x in $rpathdirs; do
                      if test "X$x" = "X$found_dir"; then
                        haveit=yes
                        break
                      fi
                    done
                    if test -z "$haveit"; then
                      rpathdirs="$rpathdirs $found_dir"
                    fi
                  else
                    dnl Rely on "-L$found_dir".
                    dnl But don't add it if it's already contained in the LDFLAGS
                    dnl or the already constructed $LIBNAME
                    haveit=
                    for x in $LDFLAGS $LIB[]NAME; do
                      AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
                      if test "X$x" = "X-L$found_dir"; then
                        haveit=yes
                        break
                      fi
                    done
                    if test -z "$haveit"; then
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$found_dir"
                    fi
                    if test "$hardcode_minus_L" != no; then
                      dnl FIXME: Not sure whether we should use
                      dnl "-L$found_dir -l$name" or "-L$found_dir $found_so"
                      dnl here.
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_so"
                    else
                      dnl We cannot use $hardcode_runpath_var and LD_RUN_PATH
                      dnl here, because this doesn't fit in flags passed to the
                      dnl compiler. So give up. No hardcoding. This affects only
                      dnl very old systems.
                      dnl FIXME: Not sure whether we should use
                      dnl "-L$found_dir -l$name" or "-L$found_dir $found_so"
                      dnl here.
                      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-l$name"
                    fi
                  fi
                fi
              fi
            else
              if test "X$found_a" != "X"; then
                dnl Linking with a static library.
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$found_a"
              else
                dnl We shouldn't come here, but anyway it's good to have a
                dnl fallback.
                LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$found_dir -l$name"
              fi
            fi
            dnl Assume the include files are nearby.
            additional_includedir=
            case "$found_dir" in
              */lib | */lib/)
                basedir=`echo "X$found_dir" | sed -e 's,^X,,' -e 's,/lib/*$,,'`
                additional_includedir="$basedir/include"
                ;;
            esac
            if test "X$additional_includedir" != "X"; then
              dnl Potentially add $additional_includedir to $INCNAME.
              dnl But don't add it
              dnl   1. if it's the standard /usr/include,
              dnl   2. if it's /usr/local/include and we are using GCC on Linux,
              dnl   3. if it's already present in $CPPFLAGS or the already
              dnl      constructed $INCNAME,
              dnl   4. if it doesn't exist as a directory.
              if test "X$additional_includedir" != "X/usr/include"; then
                haveit=
                if test "X$additional_includedir" = "X/usr/local/include"; then
                  if test -n "$GCC"; then
                    case $host_os in
                      linux*) haveit=yes;;
                    esac
                  fi
                fi
                if test -z "$haveit"; then
                  for x in $CPPFLAGS $INC[]NAME; do
                    AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
                    if test "X$x" = "X-I$additional_includedir"; then
                      haveit=yes
                      break
                    fi
                  done
                  if test -z "$haveit"; then
                    if test -d "$additional_includedir"; then
                      dnl Really add $additional_includedir to $INCNAME.
                      INC[]NAME="${INC[]NAME}${INC[]NAME:+ }-I$additional_includedir"
                    fi
                  fi
                fi
              fi
            fi
            dnl Look for dependencies.
            if test -n "$found_la"; then
              dnl Read the .la file. It defines the variables
              dnl dlname, library_names, old_library, dependency_libs, current,
              dnl age, revision, installed, dlopen, dlpreopen, libdir.
              save_libdir="$libdir"
              case "$found_la" in
                */* | *\\*) . "$found_la" ;;
                *) . "./$found_la" ;;
              esac
              libdir="$save_libdir"
              dnl We use only dependency_libs.
              for dep in $dependency_libs; do
                case "$dep" in
                  -L*)
                    additional_libdir=`echo "X$dep" | sed -e 's/^X-L//'`
                    dnl Potentially add $additional_libdir to $LIBNAME and $LTLIBNAME.
                    dnl But don't add it
                    dnl   1. if it's the standard /usr/lib,
                    dnl   2. if it's /usr/local/lib and we are using GCC on Linux,
                    dnl   3. if it's already present in $LDFLAGS or the already
                    dnl      constructed $LIBNAME,
                    dnl   4. if it doesn't exist as a directory.
                    if test "X$additional_libdir" != "X/usr/lib"; then
                      haveit=
                      if test "X$additional_libdir" = "X/usr/local/lib"; then
                        if test -n "$GCC"; then
                          case $host_os in
                            linux*) haveit=yes;;
                          esac
                        fi
                      fi
                      if test -z "$haveit"; then
                        haveit=
                        for x in $LDFLAGS $LIB[]NAME; do
                          AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
                          if test "X$x" = "X-L$additional_libdir"; then
                            haveit=yes
                            break
                          fi
                        done
                        if test -z "$haveit"; then
                          if test -d "$additional_libdir"; then
                            dnl Really add $additional_libdir to $LIBNAME.
                            LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-L$additional_libdir"
                          fi
                        fi
                        haveit=
                        for x in $LDFLAGS $LTLIB[]NAME; do
                          AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
                          if test "X$x" = "X-L$additional_libdir"; then
                            haveit=yes
                            break
                          fi
                        done
                        if test -z "$haveit"; then
                          if test -d "$additional_libdir"; then
                            dnl Really add $additional_libdir to $LTLIBNAME.
                            LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }-L$additional_libdir"
                          fi
                        fi
                      fi
                    fi
                    ;;
                  -R*)
                    dir=`echo "X$dep" | sed -e 's/^X-R//'`
                    if test "$enable_rpath" != no; then
                      dnl Potentially add DIR to rpathdirs.
                      dnl The rpathdirs will be appended to $LIBNAME at the end.
                      haveit=
                      for x in $rpathdirs; do
                        if test "X$x" = "X$dir"; then
                          haveit=yes
                          break
                        fi
                      done
                      if test -z "$haveit"; then
                        rpathdirs="$rpathdirs $dir"
                      fi
                      dnl Potentially add DIR to ltrpathdirs.
                      dnl The ltrpathdirs will be appended to $LTLIBNAME at the end.
                      haveit=
                      for x in $ltrpathdirs; do
                        if test "X$x" = "X$dir"; then
                          haveit=yes
                          break
                        fi
                      done
                      if test -z "$haveit"; then
                        ltrpathdirs="$ltrpathdirs $dir"
                      fi
                    fi
                    ;;
                  -l*)
                    dnl Handle this in the next round.
                    names_next_round="$names_next_round "`echo "X$dep" | sed -e 's/^X-l//'`
                    ;;
                  *.la)
                    dnl Handle this in the next round. Throw away the .la's
                    dnl directory; it is already contained in a preceding -L
                    dnl option.
                    names_next_round="$names_next_round "`echo "X$dep" | sed -e 's,^X.*/,,' -e 's,^lib,,' -e 's,\.la$,,'`
                    ;;
                  *)
                    dnl Most likely an immediate library name.
                    LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$dep"
                    LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }$dep"
                    ;;
                esac
              done
            fi
          else
            dnl Didn't find the library; assume it is in the system directories
            dnl known to the linker and runtime loader. (All the system
            dnl directories known to the linker should also be known to the
            dnl runtime loader, otherwise the system is severely misconfigured.)
            LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }-l$name"
            LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }-l$name"
          fi
        fi
      fi
    done
  done
  if test "X$rpathdirs" != "X"; then
    if test -n "$hardcode_libdir_separator"; then
      dnl Weird platform: only the last -rpath option counts, the user must
      dnl pass all path elements in one option. We can arrange that for a
      dnl single library, but not when more than one $LIBNAMEs are used.
      alldirs=
      for found_dir in $rpathdirs; do
        alldirs="${alldirs}${alldirs:+$hardcode_libdir_separator}$found_dir"
      done
      dnl Note: hardcode_libdir_flag_spec uses $libdir and $wl.
      acl_save_libdir="$libdir"
      libdir="$alldirs"
      eval flag=\"$hardcode_libdir_flag_spec\"
      libdir="$acl_save_libdir"
      LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$flag"
    else
      dnl The -rpath options are cumulative.
      for found_dir in $rpathdirs; do
        acl_save_libdir="$libdir"
        libdir="$found_dir"
        eval flag=\"$hardcode_libdir_flag_spec\"
        libdir="$acl_save_libdir"
        LIB[]NAME="${LIB[]NAME}${LIB[]NAME:+ }$flag"
      done
    fi
  fi
  if test "X$ltrpathdirs" != "X"; then
    dnl When using libtool, the option that works for both libraries and
    dnl executables is -R. The -R options are cumulative.
    for found_dir in $ltrpathdirs; do
      LTLIB[]NAME="${LTLIB[]NAME}${LTLIB[]NAME:+ }-R$found_dir"
    done
  fi
])

dnl AC_LIB_APPENDTOVAR(VAR, CONTENTS) appends the elements of CONTENTS to VAR,
dnl unless already present in VAR.
dnl Works only for CPPFLAGS, not for LIB* variables because that sometimes
dnl contains two or three consecutive elements that belong together.
AC_DEFUN([AC_LIB_APPENDTOVAR],
[
  for element in [$2]; do
    haveit=
    for x in $[$1]; do
      AC_LIB_WITH_FINAL_PREFIX([eval x=\"$x\"])
      if test "X$x" = "X$element"; then
        haveit=yes
        break
      fi
    done
    if test -z "$haveit"; then
      [$1]="${[$1]}${[$1]:+ }$element"
    fi
  done
])

# lib-ld.m4 serial 2 (gettext-0.12)
dnl Copyright (C) 1996-2003 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl Subroutines of libtool.m4,
dnl with replacements s/AC_/AC_LIB/ and s/lt_cv/acl_cv/ to avoid collision
dnl with libtool.m4.

dnl From libtool-1.4. Sets the variable with_gnu_ld to yes or no.
AC_DEFUN([AC_LIB_PROG_LD_GNU],
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], acl_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  acl_cv_prog_gnu_ld=yes
else
  acl_cv_prog_gnu_ld=no
fi])
with_gnu_ld=$acl_cv_prog_gnu_ld
])

dnl From libtool-1.4. Sets the variable LD.
AC_DEFUN([AC_LIB_PROG_LD],
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
# Prepare PATH_SEPARATOR.
# The user is always right.
if test "${PATH_SEPARATOR+set}" != set; then
  echo "#! /bin/sh" >conf$$.sh
  echo  "exit 0"   >>conf$$.sh
  chmod +x conf$$.sh
  if (PATH="/nonexistent;."; conf$$.sh) >/dev/null 2>&1; then
    PATH_SEPARATOR=';'
  else
    PATH_SEPARATOR=:
  fi
  rm -f conf$$.sh
fi
ac_prog=ld
if test "$GCC" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  case $host in
  *-*-mingw*)
    # gcc leaves a trailing carriage return which upsets mingw
    ac_prog=`($CC -print-prog-name=ld) 2>&5 | tr -d '\015'` ;;
  *)
    ac_prog=`($CC -print-prog-name=ld) 2>&5` ;;
  esac
  case $ac_prog in
    # Accept absolute paths.
    [[\\/]* | [A-Za-z]:[\\/]*)]
      [re_direlt='/[^/][^/]*/\.\./']
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(acl_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      acl_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$acl_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  acl_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$acl_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_LIB_PROG_LD_GNU
])

