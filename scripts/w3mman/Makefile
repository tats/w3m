
prefix		= /usr/local
bindir		= $(prefix)/bin
libdir		= $(prefix)/lib
distdir		= ./distfiles

W3M_LIBDIR	= $(libdir)/w3m

INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl
W3M		= w3m
# W3M		= w3m -X -o confirm_qq=0
MAN		= man

all: w3mman w3mman2html.cgi

w3mman: w3mman.in Makefile
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@W3M@%$(W3M)%g'	\
	    -e 's%@MAN@%$(MAN)%g'	\
	w3mman.in > w3mman
	chmod +x w3mman
	
w3mman2html.cgi: w3mman2html.cgi.in Makefile
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@MAN@%$(MAN)%g'	\
	w3mman2html.cgi.in > w3mman2html.cgi
	chmod +x w3mman2html.cgi

install: w3mman w3mman2html.cgi
	$(INSTALL_SCRIPT) w3mman $(bindir)
	$(INSTALL_SCRIPT) w3mman2html.cgi $(W3M_LIBDIR)

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

