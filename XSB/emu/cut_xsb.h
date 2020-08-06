/* File:      cut_xsb.h
** Author(s): Kostis Sagonas
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: cut_xsb.h,v 1.32 2012-02-19 19:17:55 tswift Exp $
** 
*/

#ifndef __CUT_XSB_H__
#define __CUT_XSB_H__

/*----------------------------------------------------------------------*/
/* This file contains code for cuts.  It was written in December 1997	*/
/* by Kostis Sagonas based on the code written by Terry Swift and Rui	*/
/* Marques for unwinding the trail.  The input of Bart Demoen is also	*/
/* acknowledged.							*/
/* 									*/
/* The main problem is to find which part of the trail to clean up.	*/
/* Cleaning the trail means deleting trail frames for bindings that are	*/
/* no longer conditional, i.e. unbinding these variables.  In general,	*/
/* one needs to tidy/unwind the part of the trail that lies between the	*/
/* current choice point (breg) and the choice point that is immediately	*/
/* _after_ the choice point that we are really cut back to (cut_breg).	*/
/* Of course, if we are cutting to the current choice point which often	*/
/* happens, or if there is nothing in between, there is no need to	*/
/* check whether there is trail to unwind.				*/
/* This is the general scheme and the code that pretty much implements  */
/* this, is as follows:                                         	*/
/*									*/
/*	#define cut_code(OP1)						*/
/*	    XSB_Deref(OP1);						*/
/*	    cut_breg = (CPtr)(tcpstack.high - oint_val(OP1));		*/
/*	    cut_restore_trail_condition_registers(cut_breg);		*/
/*	    if (breg != cut_breg) {					*/
/*		while (cp_prevbreg(breg) != cut_breg)			*/
/*		    breg = cp_prevbreg(breg);				*/
/*		unwind_trail(breg,xtemp1,xtemp2);			*/
/*		breg = cut_breg;					*/
/*	    } goto contcase;						*/
/*									*/
/*----------------------------------------------------------------------*/

#define IS_TABLE_INSTRUC(instruc)		\
   (instruc == check_complete || 		\
    instruc == resume_compl_suspension ||	\
    instruc == answer_return ||			\
    instruc == tabletrust ||			\
    instruc == tableretry )	       

#define IS_GENERATOR_CP(instruc)		\
   (instruc == check_complete || 		\
    instruc == tabletrust ||			\
    instruc == tableretry )	       

#define IS_INCR_TRIE_ROOT(instruc)		\
  (instruc == trie_root)

/*
#define CHECK_TABLE_CUT(instruc)       \
  if (IS_TABLE_INSTRUC(instruc) && !is_completed(tcp_subgoal_ptr(breg)))  {\
          char msg[MAXBUFSIZE];  \
          Psc psc = TIF_PSC(subg_tif_ptr(tcp_subgoal_ptr(breg)));\
          Psc call_psc = *(*((Psc **)ereg-1)-1);  \
	  sprintf(msg,"Illegal cut over incomplete tabled predicate: %s/%d, from within a call to %s/%d", \
		    get_name(psc), get_arity(psc),          \
		    get_name(call_psc), get_arity(call_psc));          \
	  xsb_table_error(CTXTc msg);  \
  }
*/

#define CHECK_TABLE_CUT(instruc)       \
  if (IS_TABLE_INSTRUC(instruc) && !is_completed(tcp_subgoal_ptr(breg)))  {\
  Psc calling_psc = *(*((Psc **)ereg-1)-1);				\
  sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF) tcp_subgoal_ptr(breg));	\
  printf("Illegal cut over incomplete tabled subgoal: %s, from within a call to %s/%d", \
	 forest_log_buffer_1->fl_buffer,get_name(calling_psc), get_arity(calling_psc));	\
  xsb_table_error_vargs(CTXTc "NULL", \
				"Illegal cut over incomplete tabled subgoal: %s, from within a call to %s/%d", \
				forest_log_buffer_1->fl_buffer,get_name(calling_psc), get_arity(calling_psc));\
  }

extern CPtr call_cleanup_gl;
extern void add_interrupt(CTXTdeclc Cell, Cell);
#include "deref.h"
extern void CHECK_CALL_CLEANUP(CTXTdeclc CPtr);
extern void CHECK_TRIE_ROOT(CTXTdeclc CPtr);

/* TLS: this creates a root choice point when backtracking through a
   completed answer trie.  The choice point increments/decrements the
   "visitors" to the trie, and this "visitors" field is used to
   determine whether the table can be incrementally updated.

   Undefining this prevents the choice point from being created, but
   means that we may core dump when updating incremental tables that
   have choice points pointing into them.
*/

#define COUNT_TRIE_VISITORS 1

#ifdef BITS64
#define PAD64	4
#else
#define PAD64	0
#endif

#define cut_code(OP1)	                                        \
   { CPtr cut_breg;					        \
     byte inst_cut_over;                                        \
     CPtr xtemp1, xtemp2;                                       \
								\
     XSB_Deref(OP1);						\
     cut_breg = (CPtr)(tcpstack.high - oint_val(OP1));		\
     cut_restore_trail_condition_registers(cut_breg);		\
     if (breg != cut_breg) { /* not cutting back to the current CP */\
	while (cp_prevbreg(breg) != cut_breg) {			\
	  CHECK_CALL_CLEANUP(CTXTc breg);				\
	  CHECK_TRIE_ROOT(CTXTc breg);					\
           inst_cut_over = *cp_pcreg(breg);                     \
           CHECK_TABLE_CUT(inst_cut_over) ;                     \
	   breg = cp_prevbreg(breg);				\
	}							\
        inst_cut_over = *cp_pcreg(breg);                        \
        CHECK_TABLE_CUT(inst_cut_over) ;                        \
	CHECK_TRIE_ROOT(CTXTc breg);				\
	CHECK_CALL_CLEANUP(CTXTc breg);				\
        unwind_trail(breg,xtemp1,xtemp2);			\
	breg = cut_breg;					\
	if (attv_pending_interrupts) {				\
	  int reserved_regs = *(lpcreg-2-PAD64);			\
	  int arsize = *(lpcreg-1-PAD64);				\
	  allocate_env_and_call_check_ints(reserved_regs,arsize);	\
	}								\
     }								\
     /*     check_table_cut = TRUE;	*/			\
   }

/*----------------------------------------------------------------------*/
/* Takes a pointer to the choice point frame we are cutting back to.	*/
/*----------------------------------------------------------------------*/

#define cut_restore_trail_condition_registers(CUTB) \
  restore_trail_condition_registers(CUTB)	    

/*
#define cut_restore_trail_condition_registers(CUTB) \
    if ((CPtr)  *CUTB >= (CPtr) pdl.low || \
		*CUTB == (Cell) &answer_return_inst || \
                *CUTB == (Cell) &resume_compl_suspension_inst ||\
                *CUTB == (Cell) &resume_compl_suspension_inst2) { \
	ebreg = cp_ebreg(CUTB); \
	hbreg = cp_hreg(CUTB); \
    }
*/

/*
#define cut_restore_trail_condition_registers(CUTB) \
    if ((CPtr)  *CUTB >= (CPtr) pdl.low || \
		*CUTB == (Cell) &answer_return_inst || \
 	        *CUTB == (Cell) &resume_compl_suspension_inst) {	\
	ebreg = cp_ebreg(CUTB); \
	hbreg = cp_hreg(CUTB); \
    }
*/

/*----------------------------------------------------------------------*/
/* Deletes all trail frames that are no longer conditional.		*/
/* Most probably, it does *NOT* work for cuts over tables!!		*/
/*----------------------------------------------------------------------*/

#define trail_parent(t)         ((CPtr *)*(t))
#define trail_value(t)          ((CPtr *)*((t)-1))
#define trail_variable(t)       ((CPtr *)*((t)-2))
#define good_trail_register(t)	(conditional(((CPtr) *((t)-2))))

#define unwind_trail(tbreg, t1, t2) {	\
    while (trreg > trfreg &&						\
           !good_trail_register(trreg) &&				\
	   trreg > cp_trreg(tbreg))					\
      trreg = trail_parent(trreg);					\
    (t2) = (CPtr) trail_parent((t1) = (CPtr)trreg);			\
    while ((t2) > (CPtr) cp_trreg(tbreg) && (t2) > (CPtr) trfreg) {	\
      if (!good_trail_register(t2)) {					\
	(t2) = (CPtr) trail_parent(t2);					\
	*(t1) = (Cell) (t2);						\
      } else {								\
	(t1) = (t2);							\
	(t2) = (CPtr) trail_parent(t2);					\
      }									\
    }									\
  }


#endif /* __CUT_XSB_H__ */
