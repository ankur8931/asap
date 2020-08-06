#! /bin/sh


cd $1
 
\rm -f subset subset.new subset.prevsort

sort subset.prev > subset.prevsort 
cat *oms.P > subset

sort subset > subset.new

    status=0
    diff subset.new subset.prevsort || status=1
    if test "$status" = 1 ; then 
	echo "subset{$1} differs"
    else 
	echo "subset{$1} tested"
    fi
 
cd ..
