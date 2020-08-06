#!/bin/sh

## File:      test.sh

## This file is called in ../testall.sh.
## $1 is expected to be the XSB executable
## $2 is expected to be the command options passed to XSB

echo "-------------------------------------------------------"
echo "--- Running Metalock/test.sh                        ---"
echo "-------------------------------------------------------"

XSB=$1
opts=$2

# gentest.sh "$XSB $opts" FILE-TO-TEST COMMAND

../gentest.sh "$XSB $opts" test "test(2,1,nomutualex(1))." nomutex_out
../gentest.sh "$XSB $opts" test "test(2,1,liveness(1,1))." liveness_out
/bin/mv -f test_new test_old
cat nomutex_out liveness_out > test_new
/bin/rm -f nomutex_out liveness_out
