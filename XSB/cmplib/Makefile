## File:      cmplib/Makefile
## Author(s): Kostis Sagonas, Jiyang Xu, Terrance Swift
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
## $Id: Makefile,v 1.26 2011-03-28 12:47:32 dwarren Exp $
## 
##

OBJEXT=.xwam

ALL =	asm${OBJEXT} asm_inst${OBJEXT} asm_opt${OBJEXT} asmpass2${OBJEXT} \
	auxlry${OBJEXT} builtin${OBJEXT} \
	ciao_directives${OBJEXT} \
	compile${OBJEXT} config${OBJEXT} cp_opt${OBJEXT} cutcheck${OBJEXT} \
	flatten${OBJEXT} inprog${OBJEXT} \
	listutil${OBJEXT} makemode${OBJEXT} modeinf${OBJEXT} modes${OBJEXT} \
	parse${OBJEXT} peephole${OBJEXT} \
	pre_cond${OBJEXT} preproc${OBJEXT} printres${OBJEXT} \
	prof_sbp${OBJEXT} sanity${OBJEXT} \
	singlton${OBJEXT} spec${OBJEXT} suppltab${OBJEXT} symtab${OBJEXT} \
	tabdef${OBJEXT} ti_mod${OBJEXT} \
	tp_comp${OBJEXT} tp_cond${OBJEXT} tp_eval${OBJEXT} tp_goal${OBJEXT} \
	tp_index${OBJEXT} tp_var${OBJEXT} \
	tpinline${OBJEXT} tprog${OBJEXT} useinfer${OBJEXT} varproc${OBJEXT} \
	writeasm${OBJEXT}

EMU_INCLUDES = ../emu/builtin.h ../emu/celltags_xsb.h ../emu/flag_defs_xsb.h \
	../emu/io_defs_xsb.h ../emu/syscall_xsb.h ../emu/sig_xsb.h \
	../emu/system_defs_xsb.h ../emu/socket_xsb.h 
PROLOG_INCLUDES = ../prolog_includes/char_defs.h ../prolog_includes/standard.h

Option = [sysmod,optimize,spec_repr,quit_on_error]

#-------------------------------------------------------------------#
#
XSB=../bin/xsb
Prolog = $(XSB) -e "segfault_handler(warn)."
#
#-------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = '../build/.xsb_cmplib_warn.tmp'
STDWARN = 3
HDRMSG = "While compiling XSB/cmplib:"
CMDFILE = 'cmd...'

OBJFILESAVED = objfiles.saved


# suffix rules
.SUFFIXES: .P ${OBJEXT} .H

all: init  $(ALL) finish
	@../build/touch.sh $(CMDFILE)
	$(Prolog) < $(CMDFILE)
	/bin/rm $(CMDFILE)
	@cat $(SVFILE)

%${OBJEXT}:   %.P  $(PROLOG_INCLUDES) $(EMU_INCLUDES)
	@cp -f $*${OBJEXT} $(OBJFILESAVED) || echo ""
	echo 'mc($*, $(Option)).' >> $(CMDFILE)
# The following is done because of brain damage in some make utilities
# (notably Sun's), which don't take %.P %.H in the same dependency list
%.P:   %.H
	../build/touch.sh $@

# These targets are obsolete. They are here just in case some 
# obscure make utility doesn't understand patterns
.H.P:
	../build/touch.sh $@
.P${OBJEXT}:
	@cp -f $*${OBJEXT} $(OBJFILESAVED) || echo ""
	echo 'mc($*, $(Option)).' >> $(CMDFILE)

# Redirect warnings into a file so we would be able to display it later.
# Assert the emu/prolog_includes into xpp_include_dir, to make sure that
# the right include directories are used if we compile cmplib using an
# older version of XSB
init:
	rm -f $(CMDFILE)
	rm -f $(SVFILE)
	@test -d $(OBJFILESAVED) || mkdir $(OBJFILESAVED)
	@echo "import file_reopen/4, file_flush/2 from file_io." >> $(CMDFILE)
	@echo "file_reopen($(SVFILE),w,$(STDWARN),_)." >> $(CMDFILE)
	@echo "file_write($(STDWARN),'----------------- Warnings ------------------------'), file_nl($(STDWARN))." >> $(CMDFILE)
	@echo "file_write($(STDWARN),'$(HDRMSG)'), file_nl($(STDWARN))." >> $(CMDFILE)
	@echo "file_flush($(STDWARN),_)." >> $(CMDFILE)
	@echo "import xpp_include_dir/1 from parse." >> $(CMDFILE)
	@echo "assert(xpp_include_dir('../emu'))." >> $(CMDFILE)
	@echo "assert(xpp_include_dir('../prolog_includes'))." >> $(CMDFILE)

finish:
	@echo "file_write($(STDWARN),'-------------------- End --------------------------'), file_nl($(STDWARN))." >> $(CMDFILE)

restore:
	mv -f $(OBJFILESAVED)/*${OBJEXT} .

clean:
	rm -f core *~ *.bak .#*
	rm -f objfiles.saved/*

.DONE:
	echo Done
