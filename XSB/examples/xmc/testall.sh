#! /bin/sh 

## File:      testall.sh
## Author:    Baoqiu Cui (Changed from XSB's testsuites)
##
## $Id: testall.sh,v 1.4 2010-08-19 15:03:38 spyrosh Exp $


echo "-------------------------------------------------------"
echo "--- Running XMC's testall.sh                        ---"
echo "-------------------------------------------------------"

while test 1=1
do
    case "$1" in
     *-opt*)
	    shift
	    options=$1
	    shift
	    ;;
     *-exclud*)
	    shift
	    excluded_tests=$1
	    shift
	    ;;
     *-add*)
	    shift
	    added_tests=$1
	    shift
	    ;;
     *-only*)
	    shift
	    only_tests=$1
	    shift
	    ;;
      *)
	    break
	    ;;
    esac
done

if test -z "$1" -o $# -gt 1; then
  echo "Usage: testall.sh [-opts opts] [-exclude \"excl_list\"] [-add \"added_tests\"] [-only \"test-list\"] executable"
  echo "where: opts       -- options to pass to XSB executable"
  echo "       excl_list  -- quoted, space-separated list of tests to NOT run"
  echo "       add_list   -- list of additional tests to run"
  echo "       only_list  -- run only this list of tests"
  echo "       executable -- full path name of the XSB executable"
  exit
fi

XSB=$1

# Test if element is a member of exclude list
# $1 - element
# $2 - exclude list
member ()
{
    for elt in $2 ; do
	if test "$1" = "$elt" ; then
	    return 0
	fi
    done
    return 1
}

default_testlist="ABP Leader Sieve Rether Metalock Iproto"

if test -z "$only_tests"; then
    testlist="$default_testlist $added_tests"
else
    testlist=$only_tests
fi
    echo $testlist

# Run each test in $testlist except for the tests in $excluded_tests
for tst in $testlist ; do
  if member "$tst" "$excluded_tests" ; then
    continue
  else
    cd $tst
    if test -f core ; then
	rm -f core
    fi
    ./test.sh "$XSB" "$options"
    cd ..
  fi
done
