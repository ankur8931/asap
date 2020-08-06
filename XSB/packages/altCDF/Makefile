## File:      packages/altCDF/Makefile
## Author(s): David S. Warren (from Kostis Sagonas, Jiyang Xu)
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
## Copyright (C) ECRC, Germany, 1990
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
## $Id: Makefile,v 1.8 2010-08-19 15:03:38 spyrosh Exp $
## 
##

OBJEXT=.xwam

ALL = 	cdf_checks${OBJEXT} cdf_comps_share${OBJEXT} cdf_concurrency${OBJEXT} \
	cdf_config${OBJEXT} \
	cdf_exceptions${OBJEXT} cdf_init_cdf${OBJEXT} \
	cdf_io${OBJEXT} cdf_oblivion${OBJEXT} cdf_utilities${OBJEXT}

EMU_INCLUDES = 
PROLOG_INCLUDES = 

Option = [optimize,quit_on_error]

#-----------------------------------------------------------------------#
#
XSB=../../bin/xsb
#XSB=.../../../XSBENV/XSB/bin/xsb
Prolog = $(XSB) -e "segfault_handler(warn)."
#
#-----------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = '../../build/.xsb_lib_warn.tmp'
STDWARN = 3
HDRMSG = "While compiling XSB/packages/altCDF:"


# suffix rules
.SUFFIXES: .P ${OBJEXT} .H

all: init $(ALL) finish
	@../../build/touch.sh cmd...
	$(Prolog) < cmd...
	/bin/rm -f cmd...
	@cat $(SVFILE)

%${OBJEXT}:   %.P $(PROLOG_INCLUDES) $(EMU_INCLUDES) ../cmplib/builtin.P
	echo "mc('$*', $(Option))." >> cmd...
# The following is done because of brain damage in some make utilities
# (notably Sun's), which don't take %.P %.H in the same dependency list
%.P:   %.H
	../../build/touch.sh $@


# These targets are obsolete. They are here just in case some 
# obscure make utility doesn't understand patterns
.H.P:
	../../build/touch.sh $@
.P${OBJEXT}:
	echo 'mc($*, $(Option)).' >> cmd...

init:
	/bin/rm -f $(SVFILE)
	@echo "import file_reopen/4, file_flush/2 from file_io." >> cmd...
	@echo "file_reopen($(SVFILE),w,$(STDWARN),_)." >> cmd...
	@echo "file_write($(STDWARN),'----------------- Warnings ------------------------'), file_nl($(STDWARN))." >> cmd...
	@echo "file_write($(STDWARN),'$(HDRMSG)'), file_nl($(STDWARN))." >> cmd...
	@echo "file_flush($(STDWARN),_)." >> cmd...

finish:
	@echo "file_write($(STDWARN),'-------------------- End --------------------------'), file_nl($(STDWARN))." >> cmd...


clean:
	/bin/rm -f *${OBJEXT} *.O core *~ *.bak .#*


.DONE:
	echo Done
