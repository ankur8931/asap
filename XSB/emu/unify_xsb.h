/* File:      unify_xsb.h
** Author(s): Bart Demoen (maintained & checked by Kostis Sagonas)
** Contact:   xsb-contact@cs.sunysb.edu
** Note: This file is an adaptation of unify_xsb_i.h, made by Luis Castro, 
** in order to define the unification code as a C macro.
** 
** Copyright (C) K.U. Leuven 1999-2000
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
** $Id: unify_xsb.h,v 1.16 2013-01-04 14:56:22 dwarren Exp $
** 
*/

#define IFTHEN_SUCCEED  XSB_Next_Instr()
#define IFTHEN_FAILED	Fail1 ; XSB_Next_Instr();


#define COND1      (CPtr)(op1) < hreg ||  (CPtr)(op1) < hfreg 
#define COND2      (CPtr)(op2) < hreg ||  (CPtr)(op2) < hfreg 

#define attv_dbgmsg(String) xsb_dbgmsg((LOG_ATTV,String))

/* Assumes that first arg is a derefed var */
static inline xsbBool not_occurs_in(Cell Var, Cell Term) {
 rec_not_occurs_in:
  XSB_Deref(Term);

  switch (cell_tag(Term)) {
  case XSB_ATTV: 
  case XSB_REF: 
  case XSB_REF1: 
    if (Var == Term) return FALSE; else return TRUE;
  case XSB_INT:
  case XSB_STRING:
  case XSB_FLOAT:
    return TRUE;
  case XSB_LIST: {
    if (not_occurs_in(Var,(Cell) clref_val(Term))) {
      Term = (Cell)(clref_val(Term) + 1);
      goto rec_not_occurs_in;
    } else return FALSE;
  }
  case XSB_STRUCT: {
    int i, arity;
    arity = get_arity(get_str_psc(Term));
    if (arity == 0) return TRUE;
    for (i = 1; i < arity; i++) {
      if (!not_occurs_in(Var,(Cell) (clref_val(Term) +i))) return FALSE;
    }
    Term = (Cell)(clref_val(Term)+arity);
    goto rec_not_occurs_in;
  }
  }
  return TRUE;  /* hush, little compiler */
}
  
/* DSW (11/7/2011): changed unify_xsb to call unify_rat to handle
   cyclic terms. unify_rat is defined in subp.c and changes pointers
   (and uses pre-image trailing) to avoid inifinite loops.  The calls
   here pass a third argument to unify_rat, which is a (small) number
   indicating how many levels to recurse before using pointer changing
   and pre-image trailing.  It is set to 20 here after simple
   experiments: If it is 0, running the entire testsuite causes ~1.2M
   preimage trailings; if it is 20, it causes ~930.  So this should
   have smaller effect on performance, and yet still catch infinite
   loops (of course after deeper, unnecessary, recursion in cyclic
   terms.)  My thought is that since cyclic terms are unusual, this
   should be a reasonable tradeoff. */
#define DEPTH_TILL_CYCLE_CHECK 20

#define unify_xsb(loc)                                       \
  XSB_Deref2(op1, goto loc##_label_op1_free);                \
  XSB_Deref2(op2, goto loc##_label_op2_free);                \
                                                             \
  if (isattv(op1)) goto loc##_label_op1_attv;                \
  if (isattv(op2)) goto loc##_label_op2_attv;                \
                                                             \
  if (isfloat(op2) && isboxedfloat(op1) ) {                  \
    if ( float_val(op2) == (float)boxedfloat_val(op1))       \
      {IFTHEN_SUCCEED;}                                      \
    else                                                     \
      {IFTHEN_FAILED;}                                       \
  }                                                          \
  if (isfloat(op1) && isboxedfloat(op2) ) {                  \
    if ( float_val(op1) == (float)boxedfloat_val(op2))       \
      {IFTHEN_SUCCEED;}                                      \
    else                                                     \
      {IFTHEN_FAILED;}                                       \
  }                                                          \
                                                             \
  if (cell_tag(op1) != cell_tag(op2))                        \
    {IFTHEN_FAILED;}                                         \
                                                             \
  if (isconstr(op1)) goto loc##_label_both_struct;           \
  if (islist(op1)) goto loc##_label_both_list;               \
  /* now they are both atomic */                             \
  if (op1 == op2) {IFTHEN_SUCCEED;}                          \
  IFTHEN_FAILED;                                             \
                                                             \
 loc##_label_op1_free:                                       \
  XSB_Deref2(op2, goto loc##_label_both_free);               \
  if (flags[UNIFY_WITH_OCCURS_CHECK_FLAG] &&                      \
      (isconstr(op2) || islist(op2)) &&                      \
      (COND1) && !not_occurs_in(op1,op2)) {		     \
    IFTHEN_FAILED;                                           \
  } else {                                                   \
    bind_copy((CPtr)(op1), op2);                             \
    IFTHEN_SUCCEED;                                          \
  }                                                          \
                                                             \
 loc##_label_op2_free:                                       \
  if (flags[UNIFY_WITH_OCCURS_CHECK_FLAG] &&                      \
      (isconstr(op1) || islist(op1)) &&                      \
      (COND2) && !not_occurs_in(op2,op1)) {		     \
    IFTHEN_FAILED;                                           \
  } else {                                                   \
    bind_copy((CPtr)(op2), op1);                             \
    IFTHEN_SUCCEED;                                          \
  }                                                          \
                                                             \
 loc##_label_both_free:                                      \
  if ( (CPtr)(op1) == (CPtr)(op2) ) {IFTHEN_SUCCEED;}        \
  if ( (CPtr)(op1) < (CPtr)(op2) )                           \
    {                                                        \
      if (COND1)                                             \
	/* op1 not in local stack */                         \
	{ bind_ref((CPtr)(op2), (CPtr)(op1)); }              \
      else  /* op1 points to op2 */                          \
	{ bind_ref((CPtr)(op1), (CPtr)(op2)); }              \
      }                                                      \
  else                                                       \
    { /* op1 > op2 */                                        \
      if (COND2)                                             \
	{ bind_ref((CPtr)(op1), (CPtr)(op2)); }              \
      else                                                   \
	{ bind_ref((CPtr)(op2), (CPtr)(op1)); }              \
    }                                                        \
  IFTHEN_SUCCEED;                                            \
                                                             \
 loc##_label_both_list:                                      \
  if (op1 == op2) {IFTHEN_SUCCEED;}                          \
  if (isinternstr(op1) && isinternstr(op2) && (op1 != op2))  \
    { IFTHEN_FAILED; }					     \
                                                             \
  op1 = (Cell)(clref_val(op1));                              \
  op2 = (Cell)(clref_val(op2));                              \
  if ( !unify_rat(CTXTc cell((CPtr)op1), cell((CPtr)op2),    \
		  (CPtr)DEPTH_TILL_CYCLE_CHECK))	     \
    { IFTHEN_FAILED; }                                       \
  op1 = (Cell)((CPtr)op1+1);                                 \
  op2 = (Cell)((CPtr)op2+1);                                 \
  if ( unify_rat(CTXTc cell((CPtr)op1), cell((CPtr)op2),     \
		 (CPtr)DEPTH_TILL_CYCLE_CHECK))		     \
    {IFTHEN_SUCCEED;} else { IFTHEN_FAILED; }		     \
                                                             \
 loc##_label_both_struct:                                    \
  if (op1 == op2) {IFTHEN_SUCCEED;}                          \
  if (isinternstr(op1) && isinternstr(op2) && (op1 != op2))  \
    { IFTHEN_FAILED; }					     \
                                                             \
  /* a != b */                                               \
  op1 = (Cell)(clref_val(op1));                              \
  op2 = (Cell)(clref_val(op2));                              \
  if (((Pair)(CPtr)op1)->psc_ptr!=((Pair)(CPtr)op2)->psc_ptr)\
    {                                                        \
      /* 0(a) != 0(b) */                                     \
      IFTHEN_FAILED;                                         \
    }                                                        \
  {                                                          \
    int arity = get_arity(((Pair)(CPtr)op1)->psc_ptr);       \
    if (arity == 0) {IFTHEN_SUCCEED;}			     \
    while (arity--)                                          \
      {                                                      \
	op1 = (Cell)((CPtr)op1+1); op2 = (Cell)((CPtr)op2+1);\
	if (!unify_rat(CTXTc cell((CPtr)op1), cell((CPtr)op2), \
                       (CPtr)DEPTH_TILL_CYCLE_CHECK))	     \
	  {                                                  \
	    IFTHEN_FAILED;                                   \
	  }                                                  \
      }                                                      \
    IFTHEN_SUCCEED;                                          \
  }                                                          \
                                                             \
  /* if the order of the arguments in add_interrupt */       \
  /* is not important, the following three can actually */   \
  /* be collapsed into one; loosing some meaningful */       \
  /* attv_dbgmsg - they have been lost partially */          \
  /* already */                                              \
                                                             \
 loc##_label_op1_attv:                                       \
  if (isattv(op2)) goto loc##_label_both_attv;               \
  attv_dbgmsg(">>>> ATTV = something, interrupt needed\n");  \
  add_interrupt(CTXTc cell(((CPtr)dec_addr(op1) + 1)),op2);  \
  bind_copy((CPtr)dec_addr(op1), op2);                       \
  IFTHEN_SUCCEED;                                            \
                                                             \
 loc##_label_op2_attv:                                       \
  attv_dbgmsg(">>>> something = ATTV, interrupt needed\n");  \
  add_interrupt(CTXTc cell(((CPtr)dec_addr(op2) + 1)),op1);  \
  bind_copy((CPtr)dec_addr(op2), op1);                       \
  IFTHEN_SUCCEED;                                            \
                                                             \
 loc##_label_both_attv:                                      \
  if (op1 != op2)                                            \
    {                                                        \
      attv_dbgmsg(">>>> ATTV = ???, interrupt needed\n");    \
      add_interrupt(CTXTc cell(((CPtr)dec_addr(op1) + 1)),op2);    \
      bind_copy((CPtr)dec_addr(op1), op2);                   \
    }                                                        \
  IFTHEN_SUCCEED



