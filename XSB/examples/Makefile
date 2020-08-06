#
#	File		: XSB/examples/Makefile
#	Authors		: Kostis Sagonas, Jiyang Xu
#	Last updated	: April 5, 1993
#

OBJEXT=.O

ALL =	fib$(OBJEXT) farmer$(OBJEXT) ham$(OBJEXT) houses$(OBJEXT) \
	qsort$(OBJEXT) queens$(OBJEXT) mandel$(OBJEXT) map$(OBJEXT) \
	nrev$(OBJEXT) lips_mega$(OBJEXT)\
	first$(OBJEXT) ptq$(OBJEXT) ptq_utils$(OBJEXT) ptqlex$(OBJEXT) \
	parser$(OBJEXT) ptq_examples$(OBJEXT) \
	shortest_path$(OBJEXT) wfs$(OBJEXT)

Option = []
Prolog = ../emu/xsb -i -m 600 -c 400

# continue to the next when one error occurs
.IGNORE:

# suffix rules
.SUFFIXES: .P $(OBJEXT) .H
.P$(OBJEXT):
	echo 'mc($*, $(Option)).' >> cmd...

all: $(ALL)
	echo 'statistics.' >> cmd...
	$(Prolog) < cmd...
	/bin/rm cmd...
# Unfortunately, this needs to be commented on the NeXT
# %$(OBJEXT):	%.H

.DONE:
	echo Done

