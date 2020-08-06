## File:      proc.mk
## Author(s): The XSB Group
## Contact:   xsb-contact@cs.sunysb.edu
## 
## Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
## $Id: proc.mk,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
## 
##


include $(ORACLE_HOME)/precomp/env_precomp.mk

.SUFFIXES: .pc .c .o

CC= gcc
LDSTRING=
PRODUCT_LIBHOME=
MAKEFILE=proc.mk
PROCPLSFLAGS= sqlcheck=full userid=$(USERID) dbms=v6_char
PROCPPFLAGS= code=cpp include=/usr/include
USERID=scott/tiger
INCLUDE=$(I_SYM). $(PRECOMPPUBLIC)

SAMPLES=sample1 sample2 sample3 sample4 sample6 sample7 sample8 \
	sample9 sample10 sample11 sample12 oraca sqlvcp cv_demo orastuff
CPPSAMPLES=cppdemo1 cppdemo2 cppdemo3

# Rule to compile any program (specify EXE= and OBJS= on command line)

build: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) -L$(LIBHOME) $(PROLDLIBS)

cppbuild:
	$(PROC) $(PROCPPFLAGS) iname=$(EXE)
	CC -c $(INCLUDE) $(EXE).c
	CC -o $(EXE) $(OBJS) -L$(LIBHOME) $(PROLDLIBS)

samples: $(SAMPLES)
cppsamples: $(CPPSAMPLES)

$(SAMPLES):
	$(MAKE) -f $(MAKEFILE) build OBJS=$@.o EXE=$@

$(CPPSAMPLES):
	$(MAKE) -f $(MAKEFILE) cppbuild OBJS=$@.o EXE=$@

sample5:
	@echo 'sample5 is a user-exit demo; use a forms makefile to build it.'

.pc.c:
	$(PROC) $(PROCFLAGS) iname=$*.pc lines=yes code=ansi_c

.pc.o:
	$(PROC) $(PROCFLAGS) iname=$*.pc
	$(CC) $(CFLAGS) -c $*.c

.c.o:
	$(CC) $(CFLAGS) -c $*.c

sample6.o: sample6.pc
	$(PROC) dbms=v6_char iname=$*.pc
	$(CC) $(CFLAGS) $(PRECOMPPUBLIC) -c $*.c

sample9.o: sample9.pc calldemo-sql
	$(PROC) $(PROCPLSFLAGS) iname=$*.pc
	$(CC) $(CFLAGS) $(PRECOMPPUBLIC) -c $*.c

cv_demo.o: cv_demo.pc cv_demo-sql
	$(PROC) $(PROCPLSFLAGS) iname=$*.pc
	$(CC) $(CFLAGS) $(PRECOMPPUBLIC) -c $*.c

sample11.o: sample11.pc sample11-sql
	$(PROC) $(PROCPLSFLAGS) iname=$*.pc dbms=v6
	$(CC) $(CFLAGS) $(PRECOMPPUBLIC) -c $*.c

calldemo-sql:
	sqlplus scott/tiger @../sql/calldemo </dev/null

sample11-sql:
	sqlplus scott/tiger @../sql/sample11 </dev/null

cv_demo-sql:
	sqlplus scott/tiger @../sql/cv_demo </dev/null
