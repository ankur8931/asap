#!/bin/sh

## File:      test.sh

## This file is called in ../testall.sh.
## $1 is expected to be the XSB executable
## $2 is expected to be the command options passed to XSB

echo "-------------------------------------------------------"
echo "--- Running Leader/test.sh                          ---"
echo "-------------------------------------------------------"

XSB=$1
opts=$2

# gentest.sh "$XSB $opts" FILE-TO-TEST COMMAND

../gentest.sh "$XSB $opts" test "test(2,ae_leader)." ae_out
../gentest.sh "$XSB $opts" test "test(2,one_leader)." one_out
/bin/mv -f test_new test_old
cat ae_out one_out > test_new
/bin/rm -rf ae_out one_out
