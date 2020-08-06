#! /bin/sh

cd $1
 
\rm -f subset subset.new subset.1

cat *omsext.P > subset

sort subset > subset.new
sort subset.prev > subset.1

    status=0
    diff subset.new subset.1 || status=1
    if test "$status" = 1 ; then 
	echo "subset{$1} differs"
    else 
	echo "subset{$1} tested"
    fi
 
cd ..
