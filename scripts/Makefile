
prefix		= /usr/local
DESTDIR		=
BIN_DIR		= $(prefix)/bin
LIB_DIR		= $(prefix)/lib/w3m
HELP_DIR	= $(prefix)/lib/w3m

LIB_TARGETS	= dirlist.cgi w3mhelp.cgi

INSTALL		= install -c
INSTALL_SCRIPT	= $(INSTALL) -m 755

PERL		= /usr/local/bin/perl

DOCDIRS = doc:en_English doc-jp:ja_Japanese

.SUFFIXES: .in

all: $(LIB_TARGETS) helplibs

.in:
	@echo "generating $@..."
	@sed -e 's%@PERL@%$(PERL)%' -e 's%@HELP_DIR@%$(HELP_DIR)%' \
	  -e 's%@DOCDIRS@%$(DOCDIRS)%' $< > $@
	@chmod +x $@
	@echo done

helplibs:
	@echo "generating w3mhelp-funcname.pl..."
	@echo '%funcname = (' > w3mhelp-funcname.pl
	@while read fname fid; do \
	  case "$$fname" in [a-zA-Z@]*) echo "'$$fname', '$$fid',";; esac; \
	done < ../funcname.tab >> w3mhelp-funcname.pl
	@echo ');' >> w3mhelp-funcname.pl
	@echo '%keyfunc = (' >> w3mhelp-funcname.pl
	@case "$(KEYBIND_SRC)" in *lynx*) keymap=keymap.lynx;; *) keymap=keymap.default;; esac; \
	while read keyword keys func rest; do \
	 if [ "X$$keyword" = Xkeymap ]; then \
	   keys=`echo "$$keys" | sed -e 's/\\\\/\\\\&/g'`; \
	   echo "'$$keys', '$$func',"; \
	 fi; \
	done < ../doc/$$keymap >> w3mhelp-funcname.pl
	@echo ');' >> w3mhelp-funcname.pl
	@cat w3mhelp-funcname.pl.in >> w3mhelp-funcname.pl
	@echo "done"

	@echo "generating w3mhelp-funcdesc*.pl..."
	@for dirlang in $(DOCDIRS); do \
	  dir=`expr "$$dirlang" : "\(.*\):.*"`; \
	  lang=`expr "$$dirlang" : ".*:\(.*\)_.*"`; \
	  echo '%funcdesc = (' > w3mhelp-funcdesc.$$lang.pl; \
	  while read func desc; do \
	   case "$$func" in [a-zA-Z@]*) echo "'$$func', '$$desc', ";; esac; \
	  done < ../$$dir/README.func >> w3mhelp-funcdesc.$$lang.pl; \
	  echo ');' >> w3mhelp-funcdesc.$$lang.pl; \
	  cat w3mhelp-funcdesc.$$lang.pl.in >> w3mhelp-funcdesc.$$lang.pl; \
	done
	@echo done

install: $(LIB_TARGETS)
	for file in $(LIB_TARGETS);     \
	do      \
		$(INSTALL_SCRIPT) $$file $(DESTDIR)$(LIB_DIR);  \
	done
	for file in w3mhelp-*.pl; \
	do \
		$(INSTALL) $$file $(DESTDIR)$(HELP_DIR); \
	done

clean:
	rm -f $(LIB_TARGETS) w3mhelp-*.pl
