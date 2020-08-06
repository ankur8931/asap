#!/bin/sh

## File:      test.sh

## This file is called in ../testall.sh.
## $1 is expected to be the XSB executable
## $2 is expected to be the command options passed to XSB

echo "-------------------------------------------------------"
echo "--- Running iproto/test.sh                          ---"
echo "-------------------------------------------------------"

XSB=$1
opts=$2

# gentest.sh "$XSB $opts" FILE-TO-TEST COMMAND

../gentest.sh "$XSB $opts" test "test(1,bug)." bugtest_out
../gentest.sh "$XSB $opts" test "test(1,fix)." fixtest_out
/bin/mv -f test_new test_old
cat bugtest_out fixtest_out > test_new
/bin/rm -f bugtest_out fixtest_out
