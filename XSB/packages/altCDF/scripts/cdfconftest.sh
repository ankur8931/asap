#! /bin/sh 

# Script to test all configurations of CDF.
# The script is very simple (as you can see) but I'll make it better once 
# we start using it heavily.
# 1) change so that the various defintions are generated off of a list
# 2) take -mswin out of call below.

cp ../cdf_definitions.h temp_definitions.h

cp cdf_def00.h ../cdf_definitins.h

touch ../cdf_init_cdf.P

cd ../../..

XSBDIR=`pwd`

cd ../tests

#############################

echo "Testing non-tabled isa / no XJ"

testsuite.sh -mswin -only "cdf_tests" $XSBDIR

cd ../XSB/packages/altCDF/scripts

cp cdf_def01.h ../cdf_definitins.h

touch ../cdf_init_cdf.P

cd ../../../../tests

#############################

echo "Testing non-tabled isa / XJ"

testsuite.sh -mswin -only "cdf_tests" $XSBDIR

cd ../XSB/packages/altCDF/scripts

cp cdf_def10.h ../cdf_definitins.h

touch ../cdf_init_cdf.P

cd ../../../../tests

#############################

echo "Testing tabled isa / no XJ"

testsuite.sh -mswin -only "cdf_tests" $XSBDIR

cd ../XSB/packages/altCDF/scripts

cp cdf_def11.h ../cdf_definitins.h

touch ../cdf_init_cdf.P

cd ../../../../tests

#############################

echo "Testing tabled isa / XJ"

testsuite.sh -mswin -only "cdf_tests" $XSBDIR

cd ../XSB/packages/altCDF/scripts

cp temp_definitions.h ../cdf_definitions.h 

touch ../cdf_init_cdf.P



