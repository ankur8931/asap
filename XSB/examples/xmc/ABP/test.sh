#!/bin/sh

## File:      test.sh

## This file is called in ../testall.sh.
## $1 is expected to be the XSB executable
## $2 is expected to be the command options passed to XSB

echo "-------------------------------------------------------"
echo "--- Running ABP/test.sh                             ---"
echo "-------------------------------------------------------"

XSB=$1
opts=$2

# gentest.sh "$XSB $opts" FILE-TO-TEST COMMAND

../gentest.sh "$XSB $opts" test "test('ABP', abp)." abptest_out
../gentest.sh "$XSB $opts" test "test('Buggy ABP', buggyabp)." buggytest_out
/bin/mv -f test_new test_old
/bin/rm -f test_new
cat abptest_out buggytest_out > test_new
/bin/rm -f abptest_out buggytest_out
