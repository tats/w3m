
prefix		= /usr/local
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m
DESTDIR		=
distdir		= ./distfiles

LIB_TARGETS	= multipart.cgi

MKDIR		= mkdir -p
INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl
NKF		= nkf

.SUFFIXES: .in

all: $(LIB_TARGETS)

.in:
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@NKF@%$(NKF)%g'	\
	$< > $@
	chmod +x $@

install: $(LIB_TARGETS)
	-$(MKDIR) $(DESTDIR)$(LIB_DIR)
	for file in $(LIB_TARGETS);	\
	do	\
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);	\
	done

uninstall:
	-for file in $(LIB_TARGETS);	\
	do	\
		rm -f $(LIB_DIR)/$$file;	\
	done

clean:
	-rm -f $(LIB_TARGETS)

dist: all
	@-rm -fr $(distdir)/multipart
	$(MKDIR) $(distdir)/multipart
	cp Makefile README multipart.cgi.in $(distdir)/multipart
	( cd $(distdir); tar -cf - multipart | GZIP='' gzip ) \
		> $(distdir)/multipart.tar.gz
	-rm -fr $(distdir)/multipart

