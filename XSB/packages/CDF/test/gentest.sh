#! /bin/sh

# $1 is expected to have xsb ececutable + command line options
EMU=$1
FILE=$2

echo "--------------------------------------------------------------------"
echo "Testing ext${FILE}"

$EMU -m 5000 -c 5000 << EOF

[oms_test].

['test${FILE}.P'].

load_test_file(ext${FILE}).

test.

halt.

EOF


