## File:      Makefile - recompiles all object files in all directories
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
## $Id: Makefile,v 1.14 2012-09-27 02:25:57 kifer Exp $
## 
##


SUBDIR = cmplib syslib lib
XSB=./bin/xsb

all: $(SUBDIR)

force: touch all

etags:
	etags -l c `find . -name \*.[ch] | grep -v Base` -l prolog `find . -name \*.[PH] | grep -v Base`

touch::
	touch cmplib/*.P lib/*.P syslib/*.P packages/*.P packages/*/*.P packages/*/*/*.P

syslib::
	@echo ""
	cd syslib; $(MAKE)

lib::
	@echo ""
	cd lib; $(MAKE)

cmplib::
	@echo ""
	cd cmplib; $(MAKE)

packages::
	@echo ""
	cd packages; $(MAKE)
