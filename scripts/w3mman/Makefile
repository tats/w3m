
prefix		= /usr/local
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m
MAN_DIR		= $(prefix)/man
DESTDIR		=
distdir		= ./distfiles

TARGETS		= w3mman
LIB_TARGETS	= w3mman2html.cgi
MAN_TARGETS	= w3mman.1

MKDIR		= mkdir -p
INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl
W3M		= w3m
# W3M		= w3m -X -o confirm_qq=0
MAN		= man

.SUFFIXES: .in

all: $(TARGETS) $(LIB_TARGETS) $(MAN_TARGETS)

.in:
	sed -e 's%@PERL@%$(PERL)%g'	\
	    -e 's%@W3M@%$(W3M)%g'	\
	    -e 's%@MAN@%$(MAN)%g'	\
	    -e 's%@LIB_DIR@%$(LIB_DIR)%g' \
	$< > $@
	chmod +x $@

install: $(TARGETS) $(LIB_TARGETS) $(MAN_TARGETS)
	-$(MKDIR) $(DESTDIR)$(BIN_DIR)
	-$(MKDIR) $(DESTDIR)$(LIB_DIR)
	for file in $(TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(BIN_DIR);	\
	done
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);	\
	done
	for file in $(MAN_TARGETS); \
	do	\
		$(INSTALL) -m 644 $$file $(DESTDIR)$(MAN_DIR); \
	done

uninstall:
	-for file in $(TARGETS);     \
	do      \
		rm -f $(BIN_DIR)/$$file;	\
	done
	-for file in $(LIB_TARGETS);     \
	do      \
		rm -f $(LIB_DIR)/$$file;	\
	done
	-for file in $(MAN_TARGETS); \
	do \
		rm -f $(MAN_DIR)/$$file; \
	done

clean:
	-rm -f $(TARGETS) $(LIB_TARGETS) $(MAN_TARGETS)

dist: all
	@-rm -fr $(distdir)/w3mman
	-$(MKDIR) $(distdir)/w3mman
	cp Makefile README w3mman.in w3mman2html.cgi.in hlink.cgi w3mman.1.in $(distdir)/w3mman
	( cd $(distdir); tar -cf - w3mman | GZIP='' gzip ) \
		> $(distdir)/w3mman.tar.gz
	-rm -fr $(distdir)/w3mman

