## File:      lib/Makefile
## Author(s): Kostis Sagonas, Jiyang Xu
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
## $Id: Makefile,v 1.63 2012-06-01 16:34:49 tswift Exp $
## 
##

OBJEXT=.xwam

ALL = 	aggregs${OBJEXT} array${OBJEXT} assoc_xsb${OBJEXT} \
	proc_files${OBJEXT} \
	basic_props${OBJEXT} lists${OBJEXT} constraintLib${OBJEXT} \
	directry${OBJEXT} foreign${OBJEXT} format${OBJEXT} \
	intern${OBJEXT} mttestutils${OBJEXT} mutex_xsb${OBJEXT} \
	obsolete${OBJEXT} odbc_call${OBJEXT} \
	ordsets${OBJEXT} ora_call${OBJEXT} \
	packaging${OBJEXT} random${OBJEXT} scrptutl${OBJEXT} \
	socket${OBJEXT} storage${OBJEXT} swi${OBJEXT} \
	ugraphs${OBJEXT} wrapping${OBJEXT} write_term${OBJEXT} \
	xsb_ciao${OBJEXT} xnmr${OBJEXT} justifyLP${OBJEXT} \
	xsb_profiling${OBJEXT} xsb_lint_impexp${OBJEXT} \
	dump_table${OBJEXT} \
	globalvar${OBJEXT} pairlist${OBJEXT} pretty_print${OBJEXT} \
	prolog_db${OBJEXT}

EMU_INCLUDES = ../emu/builtin.h ../emu/celltags_xsb.h ../emu/flag_defs_xsb.h \
	../emu/io_defs_xsb.h ../emu/syscall_xsb.h ../emu/sig_xsb.h \
	../emu/system_xsb.h ../emu/socket_defs_xsb.h ../emu/thread_defs_xsb.h
PROLOG_INCLUDES = ../prolog_includes/char_defs.h ../prolog_includes/standard.h

Option = [sysmod,optimize,quit_on_error]

#-----------------------------------------------------------------------#
#
XSB=../bin/xsb 
Prolog = $(XSB) -e "segfault_handler(warn)."
#
#-----------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = '../build/.xsb_lib_warn.tmp'
STDWARN = 3
HDRMSG = "While compiling XSB/lib:"


# suffix rules
.SUFFIXES: .P ${OBJEXT} .H

all: init $(ALL) finish
	@../build/touch.sh cmd...
	$(Prolog) < cmd...
	/bin/rm -f cmd...
	@cat $(SVFILE)

%${OBJEXT}:   %.P $(PROLOG_INCLUDES) $(EMU_INCLUDES) ../cmplib/builtin.P
	echo "mc('$*', $(Option))." >> cmd...
# The following is done because of brain damage in some make utilities
# (notably Sun's), which don't take %.P %.H in the same dependency list
%.P:   %.H
	../build/touch.sh $@


# These targets are obsolete. They are here just in case some 
# obscure make utility doesn't understand patterns
.H.P:
	../build/touch.sh $@
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
