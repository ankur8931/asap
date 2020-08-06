## File:      packages/xref/Makefile
## Author(s): warren (adapted from lib/Makefile)
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
## $Id: Makefile,v 1.4 2012-09-27 02:25:58 kifer Exp $
## 
##


OBJEXT=.xwam

ALL = 	xxref${OBJEXT}

Option = [optimize]

#-----------------------------------------------------------------------#
#
# Self-make: Version 1.4.1
#
XSB=../../bin/xsb
Prolog = $(XSB) -i -m 2000 -c 2000
#
#-----------------------------------------------------------------------#

# continue to the next when one error occurs
.IGNORE:

# suffix rules
.SUFFIXES: .P ${OBJEXT} .H
.P${OBJEXT}:
	echo 'mc($*, $(Option)).' >> cmd...

all: $(ALL)
	@touch cmd...
	$(Prolog) < cmd...
	/bin/rm cmd...


.PHONY: clean

clean:
	/bin/rm -f *${OBJEXT} *.O *~ .*~ *.bak


.DONE:
	echo Done
