
prefix		= /usr/local
DESTDIR		=
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m

LIB_TARGETS	= dirlist.cgi

INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl

.SUFFIXES: .in

all: $(LIB_TARGETS)

.in:
	sed 's%@PERL@%$(PERL)%' $< > $@

install: $(LIB_TARGETS)
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DISTDIR)$(LIB_DIR);  \
	done
