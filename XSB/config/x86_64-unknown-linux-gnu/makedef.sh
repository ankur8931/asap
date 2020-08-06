#!/bin/sh
# makedef.sh
# Creates an appropriate xsb.def file, given the object
# files in XSB/config/<configname>/saved.o

SRCDIR=/home/ubuntu/asap/XSB
CONFIGURATION=x86_64-unknown-linux-gnu
CONFIG_TAG=
SRC_CONFIG_PREFIX=/home/ubuntu/asap/XSB/config/x86_64-unknown-linux-gnu
OBJS=$@
DEFFILE=out.def
TMPFILE=out.def.tmp

echo "Creating .def file for ${CONFIGURATION} in ${SRC_CONFIG_PREFIX}/saved.o"
echo "OBJS=${OBJS}" 
echo EXPORTS > ${DEFFILE}
nm ${OBJS} | grep " [TC] " | sed '/ _/s//  /' | \
    awk '{print $3}' | grep @ > ${TMPFILE}
for i in `cat ${TMPFILE}`; do
    echo $i >> ${DEFFILE}
    echo _$i = $i >> ${DEFFILE}
done
rm -f out.def.tmp >/dev/null 2>&1
