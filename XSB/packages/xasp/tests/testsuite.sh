#! /bin/sh

## File:      testsuite.sh
## Author(s): Juliana Freire (kifer responsible for all recent bugs)
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
## $Id: testsuite.sh,v 1.3 2010-08-19 15:03:39 spyrosh Exp $
## 
##

#==================================================================
# This is supposed to automate the testsuite by checking the
# log for possible errors.
#==================================================================

#Usage: testsuite.sh [-opts opts] [-tag tag] [-exclude exclude_list] \
#    	     	     [-add add_list] [-only test_list] [-mswin] path
# where: opts         -- options to pass to XSB
#        tag          -- the configuration tag to use
#        exclude_list -- the list of tests (in quotes) to NOT run
#        add_list     -- ilist of test directories to adds
#    	     	     	 (which are normally not tested)
#        test_list    -- the list of tests TO run; replaces default,
#    	     	     	 both -exclude and -only can be specified at once
#        path         -- full path name of the XSB installation directory
#    The -mswin option says to run tests under MS Windows
#    If tag specified, this is the tag to use to 
#    locate the XSB executable (e.g., dbg, chat, etc.). 
#    It is usually specified using the --config-tag option of 'configure'
#    The XSB executable is either in
#    	 $1/config/architecture/bin/xsb
#    or in
#    	 $1/config/architecture-tag/bin/xsb
#    depending on whether the configuration tag was given on command line.

echo ==========================================================================

export config_tag

if test -n "$USER"; then
   USER=`whoami`
   export USER
fi


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
	    
     *-tag*)
	    shift
	    config_tag=$1
	    shift
	    ;;

      *-mswin*)
	    shift
	    windows=true
	    echo "Running tests under Windows"
	    ;;

      *)
	    break
	    ;;
    esac
done

installdir=$1
testdir=`pwd`
if test -z "$installdir" ; then
   echo "***XSB root directory hasn't been specified."
   installdir=$testdir/../XSB
   echo "***Assuming $installdir"
fi
if test ! -d "$installdir" ; then
   echo "$installdir: XSB root directory doesn't exist or has wrong permissions"
   exit 1
fi

# install dir argument
if test $# -gt 1; then
    echo "Usage: testsuite.sh [-opts opts] [-tag tag] [-exclude \"excl_list\"] [-add \"add_list\"] [-only \"test_list\"] [-mswin] path"
    echo "where: opts      -- options to pass to XSB"
    echo "       tag       -- the configuration tag to use"
    echo "       excl_list -- the list of tests to NOT run"
    echo "       add_list  -- the list of additional tests to run"
    echo "       test_list -- run only these tests"
    echo "       path      -- full path name of the XSB installation directory"
    exit
fi


if test -n "$config_tag" ; then
    config_tag="-$config_tag"
fi


# get canonical configuration name
if test -z "$windows"; then
config=`$installdir/build/config.guess`
canonical=`$installdir/build/config.sub $config`
else
config=x86-pc-windows
canonical=x86-pc-windows
fi

XEMU=$installdir/config/$canonical$config_tag/bin/xsb

GREP="grep -i"
MSG_FILE=/tmp/xsb_test_msg.$USER
LOG_FILE=/tmp/xsb_test_log.$USER
RES_FILE=/tmp/xsb_test_res.$USER

if test ! -x "$XEMU"; then
    echo "Can't execute $XEMU"
    echo "aborting..."
    echo "Can't execute $XEMU" >$MSG_FILE
    HOSTNAME=`hostname`
    echo "Aborted testsuite on $HOSTNAME..." >> $MSG_FILE
    # Mail does not work on the HP
    #	Mail -s "Testsuite aborted" $USER@cs.sunysb.edu < $MSG_FILE
    mail $USER < $MSG_FILE
    rm -f $MSG_FILE
    exit
fi

lockfile=lock.test
testdir=`pwd`

trap 'rm -f $testdir/$lockfile; exit 1' 1 2 15

if test -f $testdir/$lockfile; then
    cat <<EOF

************************************************************
The lock file ./$lockfile exists.
Probably testsuite is already running...
If not, remove
        ./$lockfile
and continue
************************************************************

EOF
   exit
else
   echo $$ > $lockfile
fi


if test -f "$RES_FILE"; then
  echo "There was an old $RES_FILE"
  echo "removing..."
  rm -f $RES_FILE
fi


if test -f "$LOG_FILE"; then
  echo "There was an old $LOG_FILE"
  echo "removing..."
  rm -f $LOG_FILE
fi



# should parameterize: create a script that given an input file
# greps for errors and prints the results to an output file
# and then this script can also be used in buildtest

echo "Testing $XEMU"
echo "The log will be left in  $LOG_FILE"

echo "Log for  $XEMU $options" > $LOG_FILE
(echo "Date-Time: `date +"%y%m%d-%H%M"`" >> $LOG_FILE) || status=failed
if test -n "$status"; then
	echo "Date-Time: no date for NeXT..." >> $LOG_FILE
	NeXT_DATE=1
else
	NeXT_DATE=0
fi

./testall.sh -opts "$options" -exclude "$excluded_tests" -add "$added_tests" -only "$only_tests" \
		    $XEMU  >> $LOG_FILE 2>&1


touch $RES_FILE
coredumps=`find . -name core -print`

if test -n "$coredumps" ; then
  echo "The following coredumps occurred during this test run:" >> $RES_FILE
  ls -1 $coredumps >> $RES_FILE
  echo "End of the core dumps list" >> $RES_FILE
fi
# check for seg fault, but not for segfault_handler
$GREP "fault" $LOG_FILE | grep -v "segfault_handler" >> $RES_FILE
# core dumped
$GREP "dumped" $LOG_FILE >> $RES_FILE
# when no output file is generated
$GREP "no match" $LOG_FILE >> $RES_FILE
# for bus error
$GREP "bus" $LOG_FILE >> $RES_FILE
# for really bad outcomes
$GREP "missing" $LOG_FILE >> $RES_FILE
# for wrong results
$GREP "differ!" $LOG_FILE >> $RES_FILE
$GREP "different!" $LOG_FILE >> $RES_FILE
# when xsb aborts... it writes something like ! Aborting...
$GREP "abort" $LOG_FILE >> $RES_FILE
# for overflows (check for Overflow & overflow)
$GREP "overflow" $LOG_FILE >> $RES_FILE
# for ... missing command...
$GREP "not found" $LOG_FILE >> $RES_FILE
$GREP "abnorm" $LOG_FILE >> $RES_FILE
$GREP "denied" $LOG_FILE >> $RES_FILE
$GREP "no such file" $LOG_FILE >> $RES_FILE
$GREP "illegal" $LOG_FILE >> $RES_FILE
# sometimes after overflow the diff fails and a message with Missing
# is displayed
$GREP "missing" $LOG_FILE >> $RES_FILE
# 
$GREP "fatal" $LOG_FILE >> $RES_FILE
# some other problems that should highlight bugs in the test suite
$GREP "syntax error" $LOG_FILE >> $RES_FILE
$GREP "cannot find" $LOG_FILE >> $RES_FILE
$GREP "predicate undefined" $LOG_FILE >> $RES_FILE

echo "-----------------------------------------"

if test "$NeXT_DATE" = 1; then
	NEW_LOG=$LOG_FILE.$NeXT_DATE
	cp $LOG_FILE $NEW_LOG
else
	NEW_LOG=$LOG_FILE-`date +"%y.%m.%d-%H:%M:%S"`
	cp $LOG_FILE $NEW_LOG
fi

HOSTNAME=`hostname`

# -s tests if size > 0
if test -s $RES_FILE; then
	cat $RES_FILE
	echo "-----------------------------------------"
	echo "***FAILED testsuite for $XEMU on $HOSTNAME"
        echo "***FAILED testsuite for $XEMU on $HOSTNAME" > $MSG_FILE
	echo "Check the log file $NEW_LOG" >> $MSG_FILE
	echo "" >> $MSG_FILE
	echo "    Summary of the problems:" >> $MSG_FILE
	echo "" >> $MSG_FILE
	cat $RES_FILE >> $MSG_FILE
	mail $USER < $MSG_FILE
	rm -f $MSG_FILE
else
	echo "PASSED testsuite for $XEMU on $HOSTNAME"
	rm -f $NEW_LOG
fi

rm -f $RES_FILE
rm -f $lockfile

echo "Done"
echo ==============================================================
