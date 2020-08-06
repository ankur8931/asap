/* File:      subp.h
** Author(s): Kostis Sagonas
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
** Copyright (C) ECRC, Germany, 1990
** 
** XSB is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XSB is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: subp.h,v 1.25 2011-11-09 02:28:19 dwarren Exp $
** 
*/

#include "context.h" 

extern void intercept(CTXTdeclc Psc);
extern void init_interrupt(void);
extern void print_qatom(FILE *, int, char *);
extern void print_aqatom(FILE *, int, char *);
extern void print_dqatom(FILE *, int, char *);
extern void print_op(FILE *, int, char *, int);
extern void remove_incomplete_tables_reset_freezes(CTXTdeclc int);

extern xsbBool unify(CTXTdeclc Cell, Cell);
extern xsbBool unify_rat(CTXTdeclc Cell, Cell, CPtr);
extern xsbBool are_identical_terms(Cell, Cell);
extern xsbBool startSleeperThread(int);
extern xsbBool cancelSleeperThread(void);

/* don't use Cell declarations here, to avoid gcc compiler warnings;
   However, this causes warnings under Windows */
extern int compare( CTXTdeclc const void *, const void * );
extern int key_compare( CTXTdeclc const void *, const void * );

extern byte *exception_handler(CTXTdeclc char *);

extern Psc synint_proc(CTXTdeclc Psc, int);
extern void add_interrupt(CTXTdeclc Cell, Cell);
extern Cell build_interrupt_chain(CTXTdecl);

#define INT_REC_SIZE 4   /* number of words in heap interrupt record */
