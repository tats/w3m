GCLIBTGZ=gc6.1alpha2.tar.gz

all: XXMakefile
	$(MAKE) -f XXMakefile 

install: XXMakefile
	$(MAKE) -f XXMakefile install

install-core: XXMakefile
	$(MAKE) -f XXMakefile install-core

install-helpfile: XXMakefile
	$(MAKE) -f XXMakefile install-helpfile

install-scripts: XXMakefile
	$(MAKE) -f XXMakefile install-scripts

uninstall: XXMakefile
	$(MAKE) -f XXMakefile uninstall

proto: XXMakefile
	$(MAKE) -f XXMakefile proto

clean: XXMakefile
	$(MAKE) -f XXMakefile clean

sweep: XXMakefile
	$(MAKE) -f XXMakefile sweep

veryclean: clean sweep
	rm XXMakefile
	(cd gc; $(MAKE) clean)
	rm -f config.param
	rm -f */*~ */*.orig */*.rej

prepare:
	rm -rf gc
	gzip -dc ../$(GCLIBTGZ) | tar xvf -
#	cp XMakefile.dist XMakefile

dist: XXMakefile
	$(MAKE) -f XXMakefile dist

bindist: XXMakefile
	$(MAKE) -f XXMakefile bindist

indent:
	-test -f config.h && mv -f config.h config.h-
#	indent -orig -nce -ncdb -i4 -di1 -nbc *.c *.h
	indent -orig -nce -ncdb -i4 -di1 -nbc -l79 -ncs -npcs -nfca -ss \
	   -TAnchor -TAnchorList -TBuffer -TBufferPoint -TBreakpoint \
	   -TDirectory \
	   -TFuncList -TKeyListItem -TKeyList \
	   -TFormList -TFormItemList \
	   -TFormSelectOption -TFormSelectOptionItem \
	   -THist -THistItem -THistList \
	   -THmarkerList -THRequest \
	   -TLine -TLineprop -TLinecolor \
	   -TListItem -TGeneralList -TTextListItem -TTextList \
	   -TMapList -TMatrix \
	   -TMenu -TMenuItem -TMenuList \
	   -TParsedURL \
	   -TRegex \
	   -TStr -TStreamBuffer \
	   -TBaseStream -TFileStream -TStrStream -TSSLStream \
	   -TEncodedStrStream -TInputStream \
	   -TTagAttrInfo \
	   -Ttable_attr \
	   -TTextLine -TTextLineList -TTextLineListItem \
	   -TURLOption -TURLFile \
	   -TVector \
	   *.c *.h w3mimg/*.c w3mimg/*.h w3mimg/*/*.c w3mimg/*/*.h
	-test -f config.h- && mv -f config.h- config.h

XXMakefile: XMakefile config.h
	awk '/^#ifdef makefile_parameter/,/^#else/' config.h | cat - XMakefile > XXMakefile
