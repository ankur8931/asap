#! /bin/sh

cat <<EOF  > .makedepend.tmp
# DO NOT DELETE THIS LINE -- make  depend  depends  on it.

EOF

cd ../emu

DEPEND=`which makedepend 2> /dev/null`

TODOS=`which todos 2> /dev/null`
U2D=`which unix2dos 2> /dev/null`
if test -z "$DEPEND" ; then
    echo "*** Warning: The command 'makedepend' is not installed."
    echo "***          Install it to speed up compilation of XSB."
fi
if test -n "$TODOS" ; then
    UNIX2DOS_CMD=$TODOS
elif test -n "$U2D" ; then
    UNIX2DOS_CMD=$U2D
else
    echo "*** Warning: The commands 'todos' and 'unix2dos' are not installed."
    echo "***          Install one of these to speed up compilation of XSB in Windows"
    UNIX2DOS_CMD=
fi
if test -z "$DEPEND" -o -z "$UNIX2DOS_CMD" ; then
    echo "***          Due to this, always run 'makexsb clean' before every 'makexsb'"
fi

MSVCDEP=../build/MSVC.dep
chmod 644 $MSVCDEP

# Convert Unix Makefile dependencies to NMAKE format, add ^M at the end
if test -n "$DEPEND" -a -n "$UNIX2DOS_CMD" ; then

    # -w2000 tells makedepend to create long line, 1 dependency per source file:
    # don't know if MSVC's NMAKE accepts multiple dependencies per source file
    # -Y tells not to bother with system include dirs:
    # they aren't important as dependencies
    # Grep is here so that the warnings about system include files
    # won't be displayed.
    makedepend -w2000 -f ../build/.makedepend.tmp -o.obj -p@@@ -Y -- -I../build/windows -- *c 2>&1 \
	| grep -v "cannot find include" | grep -v "not in"

    cat ../build/.makedepend.tmp | sed  -f ../build/MSVC.sed | $UNIX2DOS_CMD > $MSVCDEP
else
    rm -f ../build/MSVC.dep
    ../build/touch.sh  $MSVCDEP
fi


