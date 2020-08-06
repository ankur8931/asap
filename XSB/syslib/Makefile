## File:      syslib/Makefile
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
## $Id: Makefile,v 1.50 2007/10/09 16:40:17 dwarren Exp $
## 
##

OBJEXT=.xwam

ALL =	assert${OBJEXT} basics${OBJEXT} consult${OBJEXT} \
	banner${OBJEXT} \
	curr_sym${OBJEXT} dbclause${OBJEXT} \
	dcg${OBJEXT} debugger${OBJEXT} domain${OBJEXT}\
	error_handler${OBJEXT}  eval${OBJEXT}  \
	file_io${OBJEXT} \
	file_op${OBJEXT} gensym${OBJEXT} hilogsym${OBJEXT} loader${OBJEXT}  \
	machine${OBJEXT} num_vars${OBJEXT} \
	push_io${OBJEXT} \
	setof${OBJEXT} shell${OBJEXT} standard${OBJEXT} std_xsb${OBJEXT} \
	string${OBJEXT} subsumes${OBJEXT} sysinitrc${OBJEXT} tables${OBJEXT}  \
	term_exp${OBJEXT} thread${OBJEXT} sub_atom${OBJEXT} \
	unix${OBJEXT} x_interp${OBJEXT} xsb_hook${OBJEXT} xsb_read${OBJEXT}  \
	xsb_writ${OBJEXT} xcallxsb${OBJEXT} ccallxsb${OBJEXT} increval${OBJEXT}

EMU_INCLUDES = ../emu/builtin.h ../emu/celltags_xsb.h ../emu/flag_defs_xsb.h \
	../emu/io_defs_xsb.h ../emu/syscall_xsb.h ../emu/sig_xsb.h \
	../emu/system_defs_xsb.h ../emu/socket_defs_xsb.h \
	../emu/thread_defs_xsb.h ../emu/psc_defs.h
PROLOG_INCLUDES = ../prolog_includes/char_defs.h ../prolog_includes/standard.h


Option = [sysmod,optimize,quit_on_error]

#-------------------------------------------------------------------#
#
XSB=../bin/xsb
Prolog = $(XSB) -e "segfault_handler(warn)."
#
#-------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = '../build/.xsb_syslib_warn.tmp'
STDWARN = 3
HDRMSG = "While compiling XSB/syslib:"

OBJFILESAVED = objfiles.saved


# suffix rules
.SUFFIXES: .P .H ${OBJEXT}

all: init $(ALL) finish
	./CompileChangedFiles.sh $(Prolog)
	@cat $(SVFILE)

%${OBJEXT}:   %.P  $(PROLOG_INCLUDES) $(EMU_INCLUDES)
	@cp -f $*${OBJEXT} $(OBJFILESAVED) || echo ""
	echo 'mc($*, $(Option)).' >> cmd...

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
	echo 'mc($*, $(Option)).' >> cmd...

# Redirect warnings into a file so we would be able to display it later.
# The header cmd...hdr is prepended before each split cmd...xx file
# so that the output will be redirected into the appropriate cmd...?? file.
# Also: assert the emu/prolog_includes into xpp_include_dir, to make sure that
# the right include directories are used if we compile cmplib using an
# older version of XSB
init:
	rm -f $(SVFILE)
	@test -d $(OBJFILESAVED) || mkdir $(OBJFILESAVED)
	@echo "file_write($(STDWARN),'----------------- Warnings ------------------------'), file_nl($(STDWARN))." >> cmd...
	@echo "file_write($(STDWARN),'$(HDRMSG)'), file_nl($(STDWARN))." >> cmd...
	@echo "file_flush($(STDWARN),_)." >> cmd...
	@echo "import file_reopen/4, file_flush/2 from file_io." >> cmd...hdr
	@echo "file_reopen($(SVFILE),a,$(STDWARN),_)." >> cmd...hdr
	@echo "import xpp_include_dir/1 from parse." >> cmd...hdr
	@echo "assert(xpp_include_dir('../emu'))." >> cmd...hdr
	@echo "assert(xpp_include_dir('../prolog_includes'))." >> cmd...hdr

finish:
	@echo "file_write($(STDWARN),'-------------------- End --------------------------'), file_nl($(STDWARN)), file_nl($(STDWARN))." >> cmd...

restore:
	mv -f $(OBJFILESAVED)/*${OBJEXT} .

clean:
	rm -f core *~ *.bak .#*
	rm -f objfiles.saved/*

.DONE:
	echo Done
