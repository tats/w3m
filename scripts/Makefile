
prefix		= /usr/local
DESTDIR		=
BIN_DIR		= $(prefix)/bin
AUXBIN_DIR	= $(prefix)/lib/w3m
LIB_DIR		= $(prefix)/lib/w3m/cgi-bin
HELP_DIR	= $(prefix)/share/w3m
RC_DIR		= ~/.w3m

AUXBIN_TARGETS	= xface2xpm
LIB_TARGETS	= dirlist.cgi w3mhelp.cgi w3mmail.cgi
HELP_TARGETS	= w3mhelp-funcname.pl w3mhelp-funcdesc-stamp

MKDIR		= mkdir -p
INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755
INSTALL_DATA	= $(INSTALL) -m 644

PERL		= /usr/local/bin/perl

DOCDIRS = doc:en_English doc-jp:ja_Japanese

.SUFFIXES: .in

all: $(LIB_TARGETS) $(AUXBIN_TARGETS) $(HELP_TARGETS)

.in:
	@echo "generating $@..."
	@sed -e 's%@PERL@%$(PERL)%' -e 's%@HELP_DIR@%$(HELP_DIR)%' \
	  -e 's%@DOCDIRS@%$(DOCDIRS)%' \
	  -e 's%@RC_DIR@%$(RC_DIR)%' $< > $@
	@chmod +x $@
	@echo done

w3mhelp-funcname.pl: w3mhelp-funcname.pl.in ../funcname.tab ../doc/keymap.default ../doc/keymap.lynx
	@echo "generating w3mhelp-funcname.pl..."
	@echo '%funcname = (' > w3mhelp-funcname.pl
	@sed -ne "/^[a-zA-Z@]/s/\([a-zA-Z@][a-zA-Z0-9@_]*\)[ 	][ 	]*\([^ 	]*\)/'\1', '\2',/p" < ../funcname.tab >> w3mhelp-funcname.pl
	@echo ');' >> w3mhelp-funcname.pl
	@echo '%keyfunc = (' >> w3mhelp-funcname.pl
	@case "$(KEYBIND_SRC)" in *lynx*) keymap=keymap.lynx;; *) keymap=keymap.default;; esac; \
	sed -ne "/^keymap/s/^keymap[ 	][ 	]*\([^ 	]*\)[ 	][ 	]*\([a-zA-Z@][a-zA-Z0-9@_]*\)/'\1', '\2', /p" < ../doc/$$keymap >> w3mhelp-funcname.pl
	@echo ');' >> w3mhelp-funcname.pl
	@cat w3mhelp-funcname.pl.in >> w3mhelp-funcname.pl
	@echo "done"

w3mhelp-funcdesc-stamp: ../doc/README.func ../doc-jp/README.func w3mhelp-funcdesc.en.pl.in w3mhelp-funcdesc.ja.pl.in
	@echo "generating w3mhelp-funcdesc*.pl..."
	@for dirlang in $(DOCDIRS); do \
	  dir=`expr "$$dirlang" : "\(.*\):.*"`; \
	  lang=`expr "$$dirlang" : ".*:\(.*\)_.*"`; \
	  echo '%funcdesc = (' > w3mhelp-funcdesc.$$lang.pl; \
	  sed -ne "/^[a-zA-Z@]/s/\([a-zA-Z@][a-zA-Z0-9@_]*\)[ 	][ 	]*\(.*\)/'\1', '\2',/p" < ../$$dir/README.func >> w3mhelp-funcdesc.$$lang.pl; \
	  echo ');' >> w3mhelp-funcdesc.$$lang.pl; \
	  cat w3mhelp-funcdesc.$$lang.pl.in >> w3mhelp-funcdesc.$$lang.pl; \
	done
	@echo done
	@touch w3mhelp-funcdesc-stamp

install: $(LIB_TARGETS) $(HELP_TARGETS)
	-$(MKDIR) $(DESTDIR)$(AUXBIN_DIR)
	-$(MKDIR) $(DESTDIR)$(LIB_DIR)
	-$(MKDIR) $(DESTDIR)$(HELP_DIR)
	for file in $(AUXBIN_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(AUXBIN_DIR);  \
	done
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);  \
	done
	for file in w3mhelp-*.pl; \
	do \
		$(INSTALL_DATA) $$file $(DESTDIR)$(HELP_DIR); \
	done

uninstall:
	-for file in $(AUXBIN_TARGETS); \
	do      \
		rm -f $(AUXBIN_DIR)/$$file; \
	done
	-for file in $(LIB_TARGETS); \
	do      \
		rm -f $(LIB_DIR)/$$file; \
	done
	-for file in w3mhelp-*.pl; \
	do \
		rm -f $(HELP_DIR)/$$file; \
	done

clean:
	-rm -f $(LIB_TARGETS) $(AUXBIN_TARGETS) $(HELP_TARGETS) w3mhelp-*.pl
