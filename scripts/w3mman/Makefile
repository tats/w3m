
prefix		= /usr/local
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m
DESTDIR		=
distdir		= ./distfiles

TARGETS		= w3mman
LIB_TARGETS	= w3mman2html.cgi

INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl
W3M		= w3m
# W3M		= w3m -X -o confirm_qq=0
MAN		= man

.SUFFIXES: .in

all: $(TARGETS) $(LIB_TARGETS)

.in:
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@W3M@%$(W3M)%g'	\
	    -e 's%@MAN@%$(MAN)%g'	\
	$< > $@
	chmod +x $@

install: $(TARGETS) $(LIB_TARGETS)
	for file in $(TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(BIN_DIR);	\
	done
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);	\
	done

dist: all
	@-rm -fr $(distdir)/w3mman
	mkdir -p $(distdir)/w3mman
	cp Makefile README w3mman w3mman.in \
		w3mman2html.cgi w3mman2html.cgi.in hlink.cgi \
		$(distdir)/w3mman
	( cd $(distdir); \
		 tar -cf - w3mman | GZIP='' gzip ) \
		> $(distdir)/w3mman.tar.gz
	-rm -fr $(distdir)/w3mman

