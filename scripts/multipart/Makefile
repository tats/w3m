
prefix		= /usr/local
bindir		= $(prefix)/bin
libdir		= $(prefix)/lib
distdir		= ./distfiles

W3M_LIBDIR	= $(libdir)/w3m

INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl
NKF		= /usr/local/bin/nkf

all: multipart.cgi

multipart.cgi: multipart.cgi.in Makefile
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@NKF@%$(NKF)%g'	\
	multipart.cgi.in > multipart.cgi
	chmod +x multipart.cgi

install: multipart.cgi
	$(INSTALL_SCRIPT) multipart.cgi $(W3M_LIBDIR)

dist: all
	@-rm -fr $(distdir)/multipart
	mkdir -p $(distdir)/multipart
	cp Makefile README multipart.cgi multipart.cgi.in \
		$(distdir)/multipart
	( cd $(distdir); \
		 tar -cf - multipart | GZIP='' gzip ) \
		> $(distdir)/multipart.tar.gz
	-rm -fr $(distdir)/multipart

