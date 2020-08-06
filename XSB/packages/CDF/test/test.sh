
#! /bin/sh

# $1 is expected to have xsb ececutable + command line options
EMU=$1

sh gentest.sh $EMU 1
sh gentest.sh $EMU 2
sh gentest.sh $EMU 3
sh gentest.sh $EMU 4
sh gentest.sh $EMU 5
sh gentest.sh $EMU 6
#sh gentest.sh $EMU 7
sh gentest.sh $EMU 8
