# Generated automatically from Makefile.in by configure.
#
# Makefile
# Generated automatically from Makefile.in by configure.
#
package=w3m
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datadir = ${prefix}/share
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
infodir = ${prefix}/info
libexecdir = ${exec_prefix}/libexec
localstatedir = ${prefix}/var
mandir = ${prefix}/man
oldincludedir = /usr/include
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
srcdir = .
sysconfdir = ${prefix}/etc
top_srcdir = .
DESTDIR = 

CFLAGS = $(OPTS) -g -O2 -I/usr/include/openssl $(DEFS)
CPPFLAGS =  -I/usr/include/gc -I/usr/include/gc
DEFS = -DHAVE_CONFIG_H -I. -I$(top_srcdir) -DAUXBIN_DIR=\"$(libexecdir)\" -DLIB_DIR=\"$(libdir)\" -DHELP_DIR=\"$(datadir)\" -DETC_DIR=\"$(sysconfdir)\"
LDFLAGS = 
LIBS = -L. -lindep -lncurses -lcurses -ltermcap -lm  -lnsl -lgpm -lgc -L/usr/lib -lssl -lcrypto

IMGCFLAGS = -I/usr/include/gdk-pixbuf-1.0 -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include -I/usr/include/gdk-pixbuf-1.0 -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include
IMGLDFLAGS = -L/usr/lib -lgdk_pixbuf -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm -lgdk_pixbuf_xlib -L/usr/lib -lgdk_pixbuf -L/usr/lib -L/usr/X11R6/lib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm

CC = gcc
RANLIB=ranlib
AWK = mawk
PERL = /usr/bin/perl
MKDIR=mkdir -p
RM=rm
AR=ar
INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_SCRIPT=${INSTALL_PROGRAM}
INSTALL_DATA=${INSTALL} -m 644
INSTALL_W3MIMGDISPLAY=${INSTALL_PROGRAM}

HELP_FILE = w3mhelp-w3m_en.html
KEYBIND_SRC = keybind.c
KEYBIND_OBJ = keybind.o
GCLIB=@gclib@

VERSION=0.4.1+cvs-1.781
MODEL=@W3M_TARGET@-@W3M_MODEL@-EN

SRCS=main.c file.c buffer.c display.c etc.c search.c linein.c table.c local.c \
	form.c map.c frame.c rc.c menu.c mailcap.c image.c \
	func.c cookie.c history.c backend.c $(KEYBIND_SRC)
OBJS=main.o file.o buffer.o display.o etc.o search.o linein.o table.o local.o\
	form.o map.o frame.o rc.o menu.o mailcap.o image.o \
	func.o cookie.o history.o backend.o $(KEYBIND_OBJ)
LSRCS=terms.c conv.c url.c anchor.c mimehead.c hash.c parsetagx.c \
	tagtable.c istream.c ftp.c news.c
LOBJS=terms.o conv.o url.o anchor.o mimehead.o hash.o parsetagx.o \
	tagtable.o istream.o ftp.o news.o
LLOBJS=version.o
ALIBOBJS=Str.o indep.o regex.o textlist.o parsetag.o myctype.o entity.o hash.o
ALIB=libindep.a
ALLOBJS=$(OBJS) $(LOBJS) $(LLOBJS)
IMGOBJS=w3mimg/w3mimg.o w3mimg/x11/x11_w3mimg.o w3mimg/fb/fb_w3mimg.o w3mimg/fb/fb.o w3mimg/fb/fb_img.o

EXT=

TARGET=w3m$(EXT)
BOOKMARKER=w3mbookmark$(EXT)
HELPER=w3mhelperpanel$(EXT)
INFLATE=inflate$(EXT)
IMGDISPLAY=w3mimgdisplay$(EXT)
MAN1_TARGET=w3m.1
MAN1=doc/w3m.1
MAN1_JA=doc-jp/w3m.1

LIB_TARGETS=$(BOOKMARKER) $(HELPER)
AUXBIN_TARGETS= w3mimgdisplay$(EXT) inflate$(EXT)
TARGETS=$(TARGET) $(LIB_TARGETS) $(AUXBIN_TARGETS)
HELP_TARGET=w3mhelp.html
HELP_ALLFILES=w3mhelp-w3m_en.html w3mhelp-w3m_ja.html \
	w3mhelp-lynx_en.html w3mhelp-lynx_ja.html

SCRIPTSUBDIRS= scripts scripts/multipart scripts/w3mman
SUBDIRS = $(SCRIPTSUBDIRS)

all: $(TARGETS) all-scripts

$(TARGET): $(ALLOBJS) $(ALIB)
	$(CC) $(CFLAGS) -o $(TARGET) $(ALLOBJS) $(LDFLAGS) $(LIBS)

$(ALIB): $(ALIBOBJS)
	$(AR) rv $(ALIB) $(ALIBOBJS)
	$(RANLIB) $(ALIB)

$(OBJS): fm.h funcname1.h

tagtable.c: html.h tagtable.tab mktable$(EXT)
	./mktable$(EXT) 100 tagtable.tab > tagtable.c

func.o: funcname.c functable.c funcname1.h
keybind.o: funcname2.h
keybind_lynx.o: funcname2.h
parsetagx.o: html.c

funcname.c: funcname.tab
	sort funcname.tab | $(AWK) -f funcname0.awk > funcname.c

funcname1.h: funcname.tab
	sort funcname.tab | $(AWK) -f funcname1.awk > funcname1.h

funcname2.h: funcname.tab
	sort funcname.tab | $(AWK) -f funcname2.awk > funcname2.h

functable.c: funcname.tab mktable$(EXT)
	sort funcname.tab | $(AWK) -f functable.awk > functable.tab
	./mktable$(EXT) 100 functable.tab > functable.c
	-rm -f functable.tab

mktable$(EXT): mktable.o hash.o $(ALIB) $(GCTARGET)
	$(CC) $(CFLAGS) -o mktable mktable.o hash.o $(LDFLAGS) $(LIBS)

$(BOOKMARKER): w3mbookmark.o $(ALIB) $(GCTARGET)
	$(CC) $(CFLAGS) -o $(BOOKMARKER) w3mbookmark.o $(LDFLAGS) $(LIBS)

$(HELPER): w3mhelperpanel.o $(ALIB) $(GCTARGET)
	$(CC) $(CFLAGS) -o $(HELPER) w3mhelperpanel.o $(LDFLAGS) $(LIBS)

$(INFLATE): inflate.o
	$(CC) $(CFLAGS) -o $(INFLATE) inflate.o $(LDFLAGS) $(LIBS) -lz

inflate.o: inflate.c
	$(CC) $(CFLAGS) -o inflate.o -c inflate.c

$(IMGDISPLAY): w3mimgdisplay.o $(IMGOBJS)
	$(CC) $(CFLAGS) -o $(IMGDISPLAY) w3mimgdisplay.o $(IMGOBJS) $(LDFLAGS) $(LIBS) $(IMGLDFLAGS)

w3mimgdisplay.o: w3mimgdisplay.c
	$(CC) $(CFLAGS) $(IMGCFLAGS) -c w3mimgdisplay.c

w3mimg/w3mimg.o: w3mimg/w3mimg.c
	cd w3mimg && $(CC) $(CFLAGS) $(IMGCFLAGS) -I.. -c w3mimg.c

w3mimg/x11/x11_w3mimg.o: w3mimg/x11/x11_w3mimg.c
	cd w3mimg/x11 && $(CC) $(CFLAGS) $(IMGCFLAGS) -I../.. -c x11_w3mimg.c

w3mimg/fb/fb_w3mimg.o: w3mimg/fb/fb_w3mimg.c
	cd w3mimg/fb && $(CC) $(CFLAGS) $(IMGCFLAGS) -I../.. -c fb_w3mimg.c

w3mimg/fb/fb.o: w3mimg/fb/fb.c
	cd w3mimg/fb && $(CC) $(CFLAGS) $(IMGCFLAGS) -I../.. -c fb.c

w3mimg/fb/fb_img.o: w3mimg/fb/fb_img.c w3mimg/fb/fb_gdkpixbuf.c w3mimg/fb/fb_imlib2.c
	cd w3mimg/fb && $(CC) $(CFLAGS) $(IMGCFLAGS) -I../.. -c fb_img.c

install: install-core install-scripts

install-core: $(TARGETS)
	-$(MKDIR) $(DESTDIR)$(bindir)
	-$(MKDIR) $(DESTDIR)$(libdir)
	-$(MKDIR) $(DESTDIR)$(libexecdir)
	-$(MKDIR) $(DESTDIR)$(datadir)
	-$(MKDIR) $(DESTDIR)$(mandir)/man1
	-$(MKDIR) $(DESTDIR)$(mandir)/ja/man1
	$(INSTALL_PROGRAM) $(TARGET) $(DESTDIR)$(bindir)/$(TARGET)
	$(INSTALL_DATA) $(HELP_FILE) $(DESTDIR)$(datadir)/$(HELP_TARGET)
	$(INSTALL_DATA) $(MAN1) $(DESTDIR)$(mandir)/man1/$(MAN1_TARGET)
	$(INSTALL_DATA) $(MAN1_JA) $(DESTDIR)$(mandir)/ja/man1/$(MAN1_TARGET)
	targets="$(AUXBIN_TARGETS)"; for file in $$targets; \
	do \
		case $$file in \
		$(IMGDISPLAY)) $(INSTALL_W3MIMGDISPLAY) $$file $(DESTDIR)$(libexecdir)/$$file;; \
		*) $(INSTALL_PROGRAM) $$file $(DESTDIR)$(libexecdir)/$$file;; \
		esac; \
	done
	for file in $(LIB_TARGETS); \
	do \
		$(INSTALL_PROGRAM) $$file $(DESTDIR)$(libdir)/$$file; \
	done

install-helpfile:
	-$(MKDIR) $(DESTDIR)$(datadir)
	for file in $(HELP_ALLFILES); \
	do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(datadir)/$$file; \
	done

all-scripts:
	for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir; $(MAKE) PERL='$(PERL)' BIN_DIR='$(bindir)' AUXBIN_DIR='$(libexecdir)' LIB_DIR='$(libdir)' HELP_DIR='$(datadir)' RC_DIR='$(sysconfdir)' KEYBIND_SRC='$(KEYBIND_SRC)');	\
	done

install-scripts: all-scripts
	topdir=`pwd`; \
	for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir; $(MAKE) PERL='$(PERL)' MKDIR='$(MKDIR)' BIN_DIR='$(bindir)' AUXBIN_DIR='$(libexecdir)' LIB_DIR='$(libdir)' HELP_DIR='$(datadir)' MAN_DIR='$(mandir)' DESTDIR='$(DESTDIR)' INSTALL="$(INSTALL)" install); \
	done

uninstall:
	-$(RM) $(bindir)/$(TARGET)
	-for file in $(AUXBIN_TARGETS); \
	do \
		$(RM) -f $(libexecdir)/$$file; \
	done
	-for file in $(LIB_TARGETS); \
	do \
		$(RM) -f $(libdir)/$$file; \
	done
	-for file in $(HELP_ALLFILES); \
	do \
		$(RM) -f $(datadir)/$$file; \
	done
	-$(RM) -f $(datadir)/$(HELP_TARGET)
	-$(RM) -f $(mandir)/man1/$(MAN1_TARGET)
	-$(RM) -f $(mandir)/ja/man1/$(MAN1_TARGET)
	-for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir; $(MAKE) BIN_DIR='$(bindir)' LIB_DIR='$(libdir)' HELP_DIR='$(datadir)' MAN_DIR='$(mandir)' uninstall); \
	done

clean: sweep
	-$(RM) -f *.o *.a $(TARGETS) mktable$(EXT)
	-$(RM) -f funcname.c funcname1.h funcname2.h tagtable.c functable.c
	-$(RM) -f w3mimg/*.o w3mimg/*/*.o 
	-for dir in $(SCRIPTSUBDIRS);	\
	do	\
		(cd $$dir; $(MAKE) clean); \
	done


sweep:
	-$(RM) -f core *~ *.bak *.orig *.rej

depend: 
	makedepend $(CFLAGS) *.c

dist:
	cd ..; tar cvfz w3m-$(VERSION).tar.gz w3m

bindist:
	cd ..; tar cvfz w3m-$(VERSION)-$(MODEL).tar.gz w3m/w3m* w3m/doc* w3m/Bonus* w3m/README w3m/scripts 

