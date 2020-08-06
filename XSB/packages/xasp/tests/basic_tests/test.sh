#! /bin/sh

echo "-------------------------------------------------------"
echo "--- Running basic XASP tests                        ---"
echo "-------------------------------------------------------"

XEMU=$1
options=$2

../gentest.sh "$XEMU $options" cook_ex "cookex_all."
../gentest.sh "$XEMU $options" cooked_choice "test."
#../gentest.sh "$XEMU $options" lowlevel_ex "test."
../gentest.sh "$XEMU $options" rawex "test."
../gentest.sh "$XEMU $options" xnmr_int1 "test."
