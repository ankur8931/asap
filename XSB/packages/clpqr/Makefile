################################################################
# SWI-Prolog CLPQR package
# Author:    Jan Wielemaker. jan@swi.psy.uva.nl
# Copyright: LGPL (see COPYING or www.gnu.org
################################################################

.SUFFIXES: .tex .dvi .doc .pl

SHELL=/bin/sh
PLBASE=@PLBASE@
PLARCH=@PLARCH@
srcdir=/Users/macuser/yap-6.2/packages/clpqr
DESTDIR=

ifeq (yap,yap)
PL=YAPSHAREDIR=$(DESTDIR)$(SHAREDIR) YAPLIBDIR=$(DESTDIR)$(YAPLIBDIR) $(DESTDIR)$(BINDIR)/yap $(DESTDIR)$(YAPLIBDIR)/startup.yss
prefix = /usr/local
exec_prefix = ${prefix}
ROOTDIR = $(prefix)
EROOTDIR = ${prefix}
BINDIR = $(EROOTDIR)/bin
LIBDIR=$(EROOTDIR)/lib
YAPLIBDIR=$(EROOTDIR)/lib/Yap
SHAREDIR=$(ROOTDIR)/share/Yap
CLPDIR=$(SHAREDIR)
else
PL=@PL@
XPCEBASE=$(PLBASE)/xpce
PKGDOC=$(PLBASE)/doc/packages
PCEHOME=../../xpce
LIBDIR=$(PLBASE)/library
CLPDIR=$(LIBDIR)/clp
EXDIR=$(PKGDOC)/examples/clpqr
endif	#YAP/SWI

CLPRDIR=$(CLPDIR)/clpr
CLPQDIR=$(CLPDIR)/clpq
CLPQRDIR=$(CLPDIR)/clpqr

INSTALL=/usr/bin/install -c
INSTALL_PROGRAM=${INSTALL}
INSTALL_DATA=${INSTALL} -m 644
LN_INSTALL_DATA=$(shell echo `pwd`/../ln-install)
LN_INSTALL_PROGRAM=$(shell echo `pwd`/../ln-install)

CLPRPRIV=	bb_r.pl bv_r.pl \
		fourmotz_r.pl ineq_r.pl \
		itf_r.pl nf_r.pl \
		store_r.pl
CLPQPRIV=	bb_q.pl bv_q.pl \
		fourmotz_q.pl ineq_q.pl \
		itf_q.pl nf_q.pl \
		store_q.pl
CLPQRPRIV=	class.pl dump.pl \
		geler.pl itf.pl \
		ordering.pl \
		project.pl redund.pl
LIBPL=		$(srcdir)/clpr.pl $(srcdir)/clpq.pl
EXAMPLES=

all::
		@echo "Nothing to be done for this package"

install:	$(LIBPL)
		mkdir -p $(DESTDIR)$(CLPDIR)
		mkdir -p $(DESTDIR)$(CLPRDIR)
		mkdir -p $(DESTDIR)$(CLPQDIR)
		mkdir -p $(DESTDIR)$(CLPQRDIR)
		$(INSTALL_DATA) $(LIBPL) $(DESTDIR)$(CLPDIR)
		for f in $(CLPRPRIV); do $(INSTALL_DATA) $(srcdir)/clpr/$$f $(DESTDIR)$(CLPRDIR); done
		for f in $(CLPQPRIV); do $(INSTALL_DATA) $(srcdir)/clpq/$$f $(DESTDIR)$(CLPQDIR); done
		for f in $(CLPQRPRIV); do $(INSTALL_DATA) $(srcdir)/clpqr/$$f $(DESTDIR)$(CLPQRDIR); done
		$(INSTALL_DATA) $(srcdir)/README $(DESTDIR)$(CLPQRDIR)

ln-install::
		@$(MAKE) INSTALL_DATA=$(LN_INSTALL_DATA) INSTALL_PROGRAM=$(LN_INSTALL_PROGRAM) install

rpm-install:	install

pdf-install:	install-examples

html-install:	install-examples

install-examples::
#		mkdir -p $(DESTDIR)$(EXDIR)
#		(cd Examples && $(INSTALL_DATA) $(EXAMPLES) $(DESTDIR)$(EXDIR))

uninstall:
		(cd $(CLPDIR) && rm -f $(LIBPL))
		rm -rf $(CLPRDIR)
		rm -rf $(CLPQDIR)
		rm -rf $(CLPQRDIR)

check::
#		$(PL) -q -f $(srcdir)/clpr_test.pl -g test,halt -t 'halt(1)'


doc:		html pdf
html::
pdf::

################################################################
# Clean
################################################################

clean:
		rm -f *~ *% config.log

distclean:	clean
		rm -f config.h config.cache config.status Makefile
		rm -rf autom4te.cache

