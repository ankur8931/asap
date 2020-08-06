## File:      modelchecker/Makefile
## Author(s): Yifei Dong
## Contact:   lmc@cs.sunysb.edu
##
## $Id: Makefile,v 1.12 2012-09-27 02:25:58 kifer Exp $


OBJEXT=.xwam

# Note: xlparse must NOT be on the list below
ALL = comp${OBJEXT} count${OBJEXT} formula${OBJEXT} \
	mucalculus${OBJEXT} quad${OBJEXT}  \
	typecheck${OBJEXT} util${OBJEXT} xmc${OBJEXT} \
	justify${OBJEXT} navigate${OBJEXT} gui${OBJEXT}

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
SVFILE = './.xmc.tmp'
OWRITE = w
STDWARN = 3
STDERR = 2
HDRMSG = "While compiling XSB/packages/xmc:"


# continue to the next when one error occurs
.IGNORE:

# suffix rules
.SUFFIXES: .P $(OBJEXT) .H

%$(OBJEXT): %.P %.H
	echo 'mc($*, $(Option)),' >> cmd...

all:  init $(ALL) finish parse libxlp.a xlparse.so
	@$(Prolog) < cmd... 2> /dev/null
	@rm cmd...
	@cat $(SVFILE)
	@rm -f $(SVFILE)
	@chmod 755 xmc-gui

init:
	@cp /dev/null cmd...
	@/bin/rm -f $(SVFILE)
	@echo "file_reopen($(SVFILE),$(OWRITE),$(STDWARN),_)," >> cmd...
	@echo "file_clone($(STDWARN), $(STDERR),_)," >> cmd...
	@echo 'fmt_write($(STDWARN),"-------------- Warnings/Errors --------------------\n",_),' >> cmd...
	@echo 'fmt_write($(STDWARN),"%s\n",arg($(HDRMSG))),' >> cmd...
	@echo "file_flush($(STDWARN), _)," >> cmd...

finish:
	@echo 'fmt_write($(STDWARN),"---------------------------------------------------\n",_).' >> cmd...


parse:	driver.o main.o xl.tab.o lex.yy.o
	$(CC) -o parse main.o driver.o xl.tab.o lex.yy.o
	@strip parse

libxlp.a: driver.o xl.tab.o lex.yy.o
	$(AR) r libxlp.a driver.o xl.tab.o lex.yy.o

xlparse.so: xlparse.H xlparse.c libxlp.a
	@$(XSB) -e 'compile(xlparse). halt.'

xl.tab.h xl.tab.c: xl.y
	bison -d xl.y

xl.tab.o: xl.tab.c attr.h

lex.yy.o: lex.yy.c xl.tab.h attr.h

lex.yy.c: xl.l
	flex  xl.l

.PHONY : clean

clean:
	@echo "Removing all C object files..."
	-rm -f parse *.o
	@echo "Removing all Prolog bytecode files..."
	-rm -f *$(OBJEXT) *.O
	@echo "Removing all libraries!"
	-rm -f libxlp.a xlparse.so
	@echo "Removing intermediate C files..."
	-rm -f xl.tab.c xl.tab.h lex.yy.c
	@echo "Removing junk..."
	-rm -f *~ .*~ *.so
.DONE:
	@echo Done
