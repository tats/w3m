GCLIBTGZ=gc5.0alpha3.tar.gz

all: XXMakefile
	$(MAKE) -f XXMakefile 

install: XXMakefile
	$(MAKE) -f XXMakefile install

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
	cp XMakefile.dist XMakefile

dist: XXMakefile
	$(MAKE) -f XXMakefile dist

bindist: XXMakefile
	$(MAKE) -f XXMakefile bindist

indent:
	indent -orig -nce -ncdb -i4 -di1 -nbc *.c *.h

XXMakefile: XMakefile config.h
	awk '/^#ifdef makefile_parameter/,/^#else/' config.h | cat - XMakefile > XXMakefile
