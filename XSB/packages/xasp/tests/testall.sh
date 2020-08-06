#! /bin/sh 

## File:      testall.sh
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 1996-1999
## 
## XSB is free software; you can redistribute it and/or modify it under the
## terms of the GNU Library General Public License as published by the Free
## Software Foundation; either version 2 of the License, or (at your option)
## any later version.
## 
## XSB is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
## more details.
## 
## You should have received a copy of the GNU Library General Public License
## along with XSB; if not, write to the Free Software Foundation,
## Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##
## $Id: testall.sh,v 1.3 2010-08-19 15:03:39 spyrosh Exp $
## 
##


echo "-------------------------------------------------------"
echo "--- Running testall.sh                              ---"
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

XEMU=$1

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


# float_tests: don't pass. --mk
# regmatch_tests: don't pass on solaris
# what about compiler_tests?
default_testlist="basic_tests"

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
    ./test.sh "$XEMU" "-e segfault_handler(warn). $options"
    cd ..
  fi
done
