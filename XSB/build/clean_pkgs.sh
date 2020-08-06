#! /bin/sh

## File:      clean_pkgs.sh -- Clean package directory out of OBJ and .so files
## Author(s): kifer
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 1998
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
## $Id: clean_pkgs.sh,v 1.6 2010-08-19 15:03:35 spyrosh Exp $
## 
##

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

# assumes we are in the directory packages/

cur_dir=`pwd`
files=`ls`

subdir_exclude_list="CVS objfiles.saved"

# clean the packages/ dir itself
make clean

for f in $files ; do
  if test -d "$f" ; then
    if member "$f" "$subdir_exclude_list" ; then
       continue
    else
       echo "Cleaning up $cur_dir/$f"
       cd $f
       make clean
       cd ..
    fi
  fi
done
