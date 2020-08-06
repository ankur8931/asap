#!/bin/sh 

    # Create an XSB tarball; to be untarr'ed in the XSB installation directory
    # No docs or examples
    # Run this in ./admin/ directory!

files="./XSB/LICENSE ./XSB/INSTALL ./XSB/INSTALL_PROBLEMS ./XSB/INSTALL_WINDOWS \
	./XSB/README ./XSB/FAQ ./XSB/Makefile \
	./XSB/admin \
        ./XSB/build/ac* ./XSB/build/*.in ./XSB/build/config.guess \
        ./XSB/build/config.sub ./XSB/build/*sh ./XSB/build/*.msg \
        ./XSB/build/configure ./XSB/build/README \
        ./XSB/emu ./XSB/syslib ./XSB/cmplib ./XSB/lib \
	./XSB/gpp \
	./XSB/prolog_includes \
        ./XSB/etc \
        ./XSB/packages "

    cd ../..

    tar cvhf XSB/XSB.tar $files

    gzip -f XSB/XSB.tar
    chmod 644 XSB/XSB.tar.gz
