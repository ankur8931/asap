#!/bin/sh 

# Create an gzipped tarball for a new release on Unix

# RUN this in ./admin/ directory!

files="./XSB/LICENSE ./XSB/INSTALL ./XSB/INSTALL_PROBLEMS \
        ./XSB/INSTALL_WINDOWS ./XSB/README ./XSB/FAQ ./XSB/Makefile ./XSB/admin/*.sh \
        ./XSB/build/ac* ./XSB/build/*.in ./XSB/build/config.guess \
        ./XSB/build/config.sub ./XSB/build/*sh ./XSB/build/*.msg \
        ./XSB/build/configure ./XSB/build/README \
        ./XSB/build/makexsb*.bat ./XSB/build/MSVC.* ./XSB/build/windows* \
        ./XSB/emu ./XSB/syslib ./XSB/cmplib  ./XSB/lib \
	./XSB/gpp \
	./XSB/bin \
	./XSB/prolog_includes \
	./XSB/prolog-commons \
        ./XSB/etc \
        ./XSB/packages \
        ./XSB/docs/userman/manual?.pdf \
        ./XSB/docs/userman/xsb.1 \
        ./XSB/installer \
        ./XSB/InstallXSB.jar \
        ./XSB/examples "

    cd ../..

#    (cd XSB/build; chmod u+rwx configure; autoconf)
    (cd XSB/build; chmod u+rwx configure)
#    (cd XSB; make)
    (cd XSB/docs/userman; make)

    tar cvf XSB/XSB.tar $files

    gzip -f XSB/XSB.tar
