dnl w3m autoconf
dnl Process this file with autoconf to produce a configure script.
AC_INIT(w3m, 0.5.3, satodai@w3m.jp)
PACKAGE=w3m
VERSION=0.5.3
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_SUBST(W3M)
W3M=w3m
AC_DEFINE(W3M, w3m)

AC_SUBST(W3M_LANG)
W3M_LANG=${LC_ALL:-$LANG}
dnl AM_INIT_AUTOMAKE(w3m, 0.4)
AC_CONFIG_HEADER(config.h)
dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PATH_PROGS(PERL, perl, /usr/local/bin/perl)
AC_PATH_PROGS(NKF, nkf, /usr/local/bin/nkf)
AC_PATH_PROGS(MAN, man, /usr/bin/man)
AC_CANONICAL_HOST

AC_EXEEXT
AC_SUBST(EXEEXT)

AUXBIN_TARGETS=

AC_SUBST(W3M_TARGET)
AC_SUBST(W3M_LIBS)
W3M_TARGET=$target
AC_DEFINE_UNQUOTED(W3M_TARGET, $W3M_TARGET)

AC_W3M_M17N
if test x$enable_messagel10n = xno; then
 AM_GNU_GETTEXT([external])
fi

NLSTARGET=""
if test x"$USE_NLS" = xyes; then
  NLSTARGET="po"
fi
AC_SUBST(INTLTARGET)
AC_SUBST(NLSTARGET)

AC_W3M_COLOR
AC_W3M_ANSI_COLOR
AC_W3M_BG_COLOR

AC_W3M_IMAGE
if test x"$enable_image" != xno; then
 AUXBIN_TARGETS="$AUXBIN_TARGETS w3mimgdisplay\$(EXT)"
 AC_W3M_XFACE
fi

AC_W3M_KEYMAP
AC_W3M_MENU
AC_W3M_MOUSE
AC_W3M_HISTORY
AC_W3M_ALARM

AC_W3M_COOKIE
AC_W3M_NNTP
AC_W3M_GOPHER

AC_W3M_DICT
AC_W3M_HELP_CGI
AC_W3M_EXTERNAL_URI_LOADER
AC_W3M_W3MMAILER
AC_W3M_MIGEMO
AC_W3M_EDITOR
AC_W3M_MAILER
AC_W3M_EXT_BROWSER

dnl Checks for ext libs
for lib in bsd BSD 44bsd socket nsl dld dl
do
 AC_W3M_EXTLIBS($lib)
done

AC_W3M_IPv6
AC_W3M_SSL_DIGEST_AUTH

dnl Checks for libraries.
AC_CHECK_FUNC(sqrt,,[AC_CHECK_LIB(m, sqrt)])
if test x"$enable_mouse" = xyes; then
 AC_SUBST(USE_W3M)
 AC_CHECK_LIB(gpm, Gpm_Open, [AC_DEFINE(USE_GPM)
   		W3M_LIBS="$W3M_LIBS -lgpm"])
 AC_SUBST(USE_SYSMOUSE)
 AC_TRY_CPP([#include <sys/consio.h>
   MOUSE_GETINFO], [AC_DEFINE(USE_SYSMOUSE)],
   [AC_TRY_CPP([#include <machine/console.h>
      MOUSE_GETINFO], [AC_DEFINE(USE_SYSMOUSE)])])
fi
AC_SUBST(AUXBIN_TARGETS)
AC_W3M_TERMLIB
AC_W3M_GC
AC_CHECK_LIB(z, zError, [AUXBIN_TARGETS="$AUXBIN_TARGETS inflate\$(EXT)"])

AC_CHECK_TYPE([socklen_t],
	 [AC_DEFINE(HAVE_SOCKLEN_T)], ,
	 [#include <sys/types.h>
	  #include <sys/socket.h>])

AC_DEFINE(AUXBIN_TARGETS, "$AUXBIN_TARGETS")

AC_SUBST(USE_BINMODE_STREAM)
case "$host_os" in
  *cygwin*|*os2-emx*|*mingw*)
    AC_DEFINE(USE_BINMODE_STREAM)
    ;;
esac

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
dnl AC_CHECK_HEADERS(fcntl.h malloc.h sgtty.h strings.h sys/ioctl.h sys/time.h termio.h unistd.h setjmp.h)
AC_CHECK_HEADER(termios.h, [AC_DEFINE(HAVE_TERMIOS_H)],
	 [AC_CHECK_HEADER(termio.h, [AC_DEFINE(HAVE_TERMIO_H)],
	  [AC_CHECK_HEADER(sgtty.h, [AC_DEFINE(HAVE_SGTTY_H)])])])
AC_CHECK_HEADER([float.h], [AC_DEFINE(HAVE_FLOAT_H)])
AC_CHECK_HEADER([sys/select.h], [AC_DEFINE(HAVE_SYS_SELECT_H)])
AC_CHECK_HEADER([locale.h], [AC_DEFINE(HAVE_LOCALE_H)])
AC_CHECK_HEADER([stdint.h], [AC_DEFINE(HAVE_STDINT_H)])
AC_CHECK_HEADER([inttypes.h], [AC_DEFINE(HAVE_INTTYPES_H)])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_CONST
dnl AC_C_INLINE
dnl AC_TYPE_PID_T
dnl AC_TYPE_SIZE_T
dnl AC_HEADER_TIME
dnl AC_STRUCT_TM
AC_CHECK_SIZEOF(long long, 0)

dnl Checks for library functions.
dnl AC_FUNC_ALLOCA
dnl AC_PROG_GCC_TRADITIONAL
dnl AC_FUNC_MEMCMP
dnl AC_FUNC_MMAP
dnl AC_FUNC_SETVBUF_REVERSED
dnl AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strcasecmp strcasestr strchr memcpy strerror bcopy setpgrp chdir getcwd getwd readlink setenv putenv strtoll stroq atoll atoq symlink readlink lstat srand48 srandom getpassphrase waitpid setlocale)
AC_FUNC_STRFTIME
AC_FUNC_WAIT3
AC_FUNC_SETPGRP
case "$host_os" in
  *cygwin*)
    ;;
  *)
    AM_LANGINFO_CODESET
    ;;
esac
AC_W3M_SYS_ERRLIST
AC_W3M_SIGSETJMP
AC_W3M_SIGNAL

AC_SUBST(HELP_DIR)
HELP_DIR="$datadir/$PACKAGE"
AC_DEFINE(HELP_DIR, "$HELP_DIR")
AC_SUBST(RC_DIR)
RC_DIR="~/.$PACKAGE"
AC_DEFINE(RC_DIR, "$RC_DIR")
AC_SUBST(DOCDIRS)
DOCDIRS="doc:en_English doc-jp:ja_Japanese"
AC_DEFINE_UNQUOTED(DOCDIRS, "$DOCDIRS")

AC_W3M_VERSION
AC_OUTPUT([Makefile posubst po/Makefile.in \
scripts/Makefile scripts/dirlist.cgi \
scripts/w3mhelp.cgi \
scripts/w3mmail.cgi scripts/xface2xpm \
scripts/multipart/Makefile scripts/multipart/multipart.cgi \
scripts/w3mman/Makefile scripts/w3mman/w3mman scripts/w3mman/w3mman.1 \
scripts/w3mman/w3mman2html.cgi \
libwc/Makefile \
w3mimg/Makefile w3mimg/fb/Makefile w3mimg/x11/Makefile w3mimg/win/Makefile \
w3mhelp-w3m_en.html w3mhelp-w3m_ja.html \
w3mhelp-lynx_en.html w3mhelp-lynx_ja.html])

chmod +x posubst
