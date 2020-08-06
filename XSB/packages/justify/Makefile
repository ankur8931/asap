## File:      modelchecker/Makefile
## Author(s): Yifei Dong
## Contact:   lmc@cs.sunysb.edu
##
## $Id: Makefile,v 1.4 2012-09-27 02:25:58 kifer Exp $


OBJEXT=.xwam

# Note: xlparse must NOT be on the list below
ALL = jxm${OBJEXT}

Option = [optimize]

#-----------------------------------------------------------------------#
#
# Self-make: Version 1.4.1
#
XSB=../../bin/xsb
Prolog = $(XSB) -i -m 2000 -c 2000

CC = gcc
CFLAGS = -Wall
AR = ar

#
#-----------------------------------------------------------------------#

# Make sure warnings are saved in a file.
SVFILE = './.justify.tmp'
OWRITE = w
STDWARN = 3
STDERR = 2
HDRMSG = "While compiling XSB/packages/justify:"


# continue to the next when one error occurs
.IGNORE:

# suffix rules
.SUFFIXES: .P $(OBJEXT) .H

%$(OBJEXT): %.P
	echo 'mc($*, $(Option)).' >> cmd...

all:  init $(ALL)
	@$(Prolog) < cmd... 2> /dev/null
	@rm cmd...
	@cat $(SVFILE)
	@rm -f $(SVFILE)

init:
	@cp /dev/null cmd...
	@/bin/rm -f $(SVFILE)
	@echo "file_reopen($(SVFILE),$(OWRITE),$(STDWARN),_)," >> cmd...
	@echo "file_clone($(STDWARN), $(STDERR),_)," >> cmd...
	@echo 'fmt_write($(STDWARN),"-------------- Warnings/Errors --------------------\n",_),' >> cmd...
	@echo 'fmt_write($(STDWARN),"%s\n",arg($(HDRMSG))),' >> cmd...
	@echo "file_flush($(STDWARN), _)." >> cmd...

finish:
	@echo 'fmt_write($(STDWARN),"---------------------------------------------------\n",_).' >> cmd...


.PHONY : clean

clean:
	@echo "Removing all C object files..."
	-rm -f parse *.o
	@echo "Removing all Prolog bytecode files..."
	-rm -f *$(OBJEXT) *.O
	-rm -f *~ .*~ *.so
.DONE:
	@echo Done
