#! /bin/sh

## File:      copysubdirs.sh -- Copy all subdirectories of $1 to $2 recursively
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
## $Id: copysubdirs.sh,v 1.8 2012-09-27 02:25:57 kifer Exp $
## 
##

    # Paths $1 and $2 are assumed to be relative to the current directory!

if test ! -d "$1" ; then
  echo "\`$1': not a directory"
  exit
fi
if test ! -d "$2" ; then
  echo "\`$2': not a directory"
  exit
fi

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

cur_dir=`pwd`

cd $1
files=`ls`

cd $cur_dir
umask 022

subdir_exclude_list="CVS objfiles.saved"

for f in $files ; do
  if test -d "$1/$f" ; then
    if member "$f" "$subdir_exclude_list" ; then
       continue
    else
       echo "Copying $1/$f to $2"
       cp -rpf $1/$f  $2
       /bin/rm -rf $2/*/CVS $2/*/*/CVS $2/*/.cvsignore $2/*/*/.cvsignore
       /bin/rm -rf $2/*/*/*/CVS $2/*/*/*/.cvsignore
       /bin/rm -rf $2/*/*~ $2/*/*/*~ $2/*/#*# $2/*/*/#*#
       /bin/rm -rf $2/*/*/*/*~ $2/*/*/*/#*#
       /bin/rm -rf $2/*/.#* $2/*/*/.#* $2/*/*/*/.#*
    fi
  fi
done

# This also makes plain files executable. Any easy way to add x to dirs only? 
chmod -R a+rx,u+w $2

