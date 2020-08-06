## File:      packages/wildmatch/Makefile
## Author(s): kifer
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 1999
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
## $Id: Makefile,v 1.15 2012-09-27 02:25:58 kifer Exp $
## 
##

OBJEXT=.xwam

XSBDIR=../..

ALL = 	wldmtchconfig${OBJEXT}

Option = [optimize]

#-----------------------------------------------------------------------#
#
# Self-make: Version 1.4.1
#
XSB=$(XSBDIR)/bin/xsb
Prolog = $(XSB) -i -m 2000 -c 2000
#
#-----------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = '$(XSBDIR)/build/.wildmatch_warn.tmp'
OWRITE = w
STDWARN = 3
STDERR = 2
HDRMSG = "While compiling XSB/packages/wildmatch:"


# continue to the next when one error occurs
.IGNORE:

# suffix rules
.SUFFIXES: .P ${OBJEXT} .H

all: init $(ALL) finish
	@touch cmd...
	$(Prolog) < cmd...
	@rm cmd...
	@cat $(SVFILE)

.P${OBJEXT}:
	echo 'mc($*, $(Option)).' >> cmd...

init:
	/bin/rm -f $(SVFILE)
	@echo "file_reopen($(SVFILE),$(OWRITE),$(STDWARN),_)." >> cmd...
	@echo "file_clone($(STDWARN), $(STDERR),_)." >> cmd...
	@echo "file_write($(STDWARN),'-------------- Warnings/Errors --------------------'), file_nl($(STDWARN))." >> cmd...
	@echo "file_write($(STDWARN),'$(HDRMSG)'), file_nl($(STDWARN))." >> cmd...
	@echo "file_flush($(STDWARN), _)." >> cmd...

finish:
	@echo "file_write($(STDWARN),'-------------------- End --------------------------'), file_nl($(STDWARN))." >> cmd...

clean:
	/bin/rm -f *${OBJEXT} *.O *~ .*~ *.bak
	/bin/rm -f cc/*${OBJEXT} cc/*.[Oo] cc/*.so cc/*~ cc/.*~  cc/*.bak


.DONE:
	echo Done
