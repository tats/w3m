GCLIBTGZ=gc5.0alpha3.tar.gz

all: XXMakefile
	make -f XXMakefile 

install: XXMakefile
	make -f XXMakefile install

uninstall: XXMakefile
	make -f XXMakefile uninstall

proto: XXMakefile
	make -f XXMakefile proto

clean: XXMakefile
	make -f XXMakefile clean

sweep: XXMakefile
	make -f XXMakefile sweep

veryclean: clean sweep
	rm XXMakefile
	(cd gc; make clean)
	rm -f config.param
	rm -f */*~ */*.orig */*.rej

prepare:
	rm -rf gc
	gzip -dc ../$(GCLIBTGZ) | tar xvf -
	cp XMakefile.dist XMakefile

dist: XXMakefile
	make -f XXMakefile dist

bindist: XXMakefile
	make -f XXMakefile bindist

indent:
	indent -orig -nce -ncdb -i4 -di1 -nbc *.c *.h

XXMakefile: XMakefile config.h
	awk '/^#ifdef makefile_parameter/,/^#else/' config.h | cat - XMakefile > XXMakefile
