#!/bin/sh 

    # Create an XSB tarball sans testsuite; untar in the XSB install directory

    # RUN this in ./admin/ directory!

files="./XSB/LICENSE ./XSB/INSTALL ./XSB/INSTALL_PROBLEMS \
        ./XSB/INSTALL_WINDOWS ./XSB/README ./XSB/FAQ ./XSB/Makefile \
	./XSB/admin \
	./XSB/bin/xsb.bat \
        ./XSB/build/ac* ./XSB/build/*.in ./XSB/build/config.guess \
     	./XSB/build/config.sub ./XSB/build/*sh ./XSB/build/*.msg \
    	./XSB/build/*.bat ./XSB/build/*.sed \
    	./XSB/build/configure ./XSB/build/README \
        ./XSB/emu ./XSB/syslib ./XSB/cmplib ./XSB/lib \
	./XSB/gpp \
	./XSB/prolog_includes \
        ./XSB/config/x86-pc-windows \
        ./XSB/etc \
        ./XSB/packages \
        ./XSB/docs \
        ./XSB/installer \
        ./XSB/InstallXSB.jar \
        ./XSB/examples "

    cd ../..

    tar cvf XSB/XSB-NT.tar $files

    gzip -f XSB/XSB-NT.tar
