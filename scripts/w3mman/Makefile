
prefix		= /usr/local
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m/cgi-bin
MAN_DIR		= $(prefix)/man
MAN1_DIR	= $(MAN_DIR)/man1
DESTDIR		=
distdir		= ./distfiles

TARGETS		= w3mman
LIB_TARGETS	= w3mman2html.cgi
MAN1_TARGETS	= w3mman.1
MAN_TARGETS	= $(MAN1_TARGETS)

MKDIR		= mkdir -p
INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755
INSTALL_MAN	= $(INSTALL) -m 644

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
	-$(MKDIR) $(DESTDIR)$(MAN1_DIR)
	for file in $(TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(BIN_DIR);	\
	done
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);	\
	done
	for file in $(MAN1_TARGETS); \
	do	\
		$(INSTALL_MAN) $$file $(DESTDIR)$(MAN1_DIR); \
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
	-for file in $(MAN1_TARGETS); \
	do \
		rm -f $(MAN1_DIR)/$$file; \
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

