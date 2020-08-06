/* File:      emudef.h
** Author(s): Warren, Swift, Xu, Sagonas
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
** $Id: emudef.h,v 1.93 2013-05-06 21:10:24 dwarren Exp $
** 
*/

#include "debugs/debug_attv.h"

#define attv_pending_interrupts (isnonvar(cell((CPtr)glstack.low)))

#ifndef MULTI_THREAD
/* Argument Registers
   ------------------ */
Cell reg[MAX_REGS];

//#define CP_DEBUG 1

/* Special Registers
   ----------------- */
CPtr ereg;		/* last activation record       */
CPtr breg;		/* last choice point            */
CPtr hreg;		/* top of heap                  */
CPtr *trreg;		/* top of trail stack           */
CPtr hbreg;		/* heap back track point        */
CPtr sreg;		/* current build or unify field */
byte *cpreg;		/* return point register        */
byte *pcreg;		/* program counter              */
CPtr ebreg;		/* breg into environment stack	*/
#ifdef CP_DEBUG
Psc pscreg;
#endif

CPtr efreg;
CPtr bfreg;
CPtr hfreg;
CPtr *trfreg;

CPtr pdlreg;
CPtr openreg;

/* TLS 08/05: Root address is the address of the first tabled choice
   point on the thread's stack.  It is used to reclaim freeze
   registers, but could be removed, I think. */
CPtr root_address;      

CPtr ptcpreg = NULL;
CPtr delayreg;

#ifdef DEMAND
/* demand-freeze registers */
CPtr edfreg;
CPtr bdfreg;
CPtr hdfreg;
CPtr *trdfreg;
#endif

VarString *tsgLBuff1;
VarString *tsgLBuff2;
VarString *tsgSBuff1;
VarString *tsgSBuff2;

/*
 * interrupt_reg points to interrupt_counter, which stores the number of
 * interrupts in the interrupt chain for attributed variables.
 */
Cell interrupt_counter;
CPtr interrupt_reg = &interrupt_counter;

byte *current_inst;

#endif /* MULTI_THREAD */

/*
 * Ptr to the beginning of instr. array
 */ 
byte *inst_begin_gl;

char *nil_string, *true_string, *cut_string, *cyclic_string;

Pair list_pscPair;

Psc list_psc, comma_psc, true_psc, if_psc, colon_psc, caret_psc, ccall_mod_psc, c_callloop_psc, dollar_var_psc;
Psc tnot_psc, delay_psc, cond_psc, cut_psc, load_undef_psc, setof_psc, bagof_psc;
Psc box_psc, visited_psc, answer_completion_psc;
/*
 * Ret PSC's are used to store substitution factors for subgoal calls
 * or answers.  A psc with a new arity will be created when needed,
 * except that ret_psc[0] stores the pointer to STRING "ret" and is
 * initialized when the system is started.
 */
Psc ret_psc[MAX_ARITY];

/* TLS: changed name to accord with new global conventions. */
char *list_dot_string;

#ifndef MULTI_THREAD
int asynint_code = 0;
int asynint_val = 0;
#endif

#ifdef BITS64
#define _FULL_ULONG_BITS 0xffffffffffffffff
#else
#define _FULL_ULONG_BITS 0xffffffff
#endif

Integer next_free_code = 0;

#if defined(GENERAL_TAGGING)
unsigned long enc[16] = {_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,
			 _FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,
			 _FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,
			 _FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS};
unsigned long dec[8] = {_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,
			_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS,_FULL_ULONG_BITS};
#endif

/* Replacements for labelled code in emusubs.i */

#define nunify_with_nil(op)						\
  XSB_Deref(op);       							\
  if (isref(op)) {							\
    /* op is FREE */							\
    bind_nil((CPtr)(op));						\
  }									\
  else if (isnil(op)) {XSB_Next_Instr();} /* op == [] */		\
  else if (isattv(op)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_nil, interrupt needed\n"));	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(op) + 1)),makenil);   	\
    bind_copy((CPtr)dec_addr(op), makenil);                  		\
  }									\
  else Fail1;	/* op is LIST, INT, or FLOAT */

/*======================================================================*/

#define nunify_with_con(OP1,OP2)					\
  XSB_Deref(OP1);      							\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_string((CPtr)(OP1), (char *)OP2);				\
  }									\
  else if (isstring(OP1)) {						\
    if (string_val(OP1) == (char *)OP2) {XSB_Next_Instr();} else Fail1;	\
  }									\
  else if (isattv(OP1)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_con, interrupt needed\n"));	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(OP1) + 1)),makestring((char *)OP2));   	\
    bind_string((CPtr)dec_addr(OP1),(char *)OP2);		     	\
  }									\
  else Fail1;


/*======================================================================*/

#define nunify_with_internstr(OP1,OP2)					\
  XSB_Deref(OP1);      							\
  if (isref(OP1)) {							\
    if (islist(OP2)) {							\
      bind_list((CPtr)(OP1), (CPtr)OP2);				\
    } else {								\
      bind_cs((CPtr)(OP1), (CPtr)OP2);					\
    }									\
  } else if (isinternstr(OP1)) {					\
    if (OP1 == OP2) {XSB_Next_Instr();} else Fail1;			\
  } else {								\
    if (unify(CTXTc OP1,OP2)) {						\
      XSB_Next_Instr();							\
    } else Fail1;							\
  }

/*======================================================================*/

#define nunify_with_num(OP1,OP2)					\
  /* op1 is general, op2 has number (untagged) */			\
  XSB_Deref(OP1);      							\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_oint((CPtr)(OP1), (Integer)OP2);                 		\
  }									\
  else if (isointeger(OP1)) {						\
    if (oint_val(OP1) == (Integer)OP2) {XSB_Next_Instr();} else Fail1;	\
  }									\
  else if (isattv(OP1)) {						\
    /*    printf(">>>> ATTV nunify_with_num, interrupt needed\n");*/	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(OP1) + 1)),makeint(OP2));  \
    bind_oint((CPtr)dec_addr(OP1), (Integer)OP2);                 	\
  }									\
  else Fail1;	/* op1 is STRING, FLOAT, STRUCT, or LIST */

/*======================================================================*/

#define nunify_with_float(OP1,OP2)					\
  XSB_Deref(OP1);      							\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_float_tagged(vptr(OP1), makefloat(OP2));			\
  }									\
  else if (isofloat(OP1)) {						\
    if ( (float)ofloat_val(OP1) == OP2) {				\
      XSB_Next_Instr();							\
    }									\
    else Fail1;								\
  }									\
  else if (isattv(OP1)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_float, interrupt needed\n"));	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(OP1) + 1)),makefloat(OP2)); \
    bind_float_tagged((CPtr)dec_addr(OP1), makefloat(OP2));		\
  }									\
  else Fail1;	/* op1 is INT, STRING, STRUCT, or LIST */ 

/*======================================================================*/

#define nunify_with_float_get(OP1,OP2)					\
  XSB_Deref(OP1);      							\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_boxedfloat(vptr(OP1), (Float)OP2);				\
  }									\
  else if (isofloat(OP1)) {						\
    if (ofloat_val(OP1) == (Float)OP2) {				\
      XSB_Next_Instr();							\
    }									\
    else Fail1;								\
  }									\
  else if (isattv(OP1)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_float, interrupt needed\n"));	\
    bind_boxedfloat((CPtr)dec_addr(OP1), (Float)OP2);			\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(OP1) + 1)),cell((CPtr)dec_addr(OP1))); \
  }									\
  else Fail1;	/* op1 is INT, STRING, STRUCT, or LIST */ 

/*======================================================================*/

#define nunify_with_str(OP1,OP2)					\
  /* struct psc_rec *str_ptr; using op2 */				\
  XSB_Deref(OP1);					       		\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_cs((CPtr)(OP1), (Pair)hreg);					\
    new_heap_functor(hreg, (Psc)OP2);					\
    flag = WRITE;							\
  }									\
  else if (isconstr(OP1)) {						\
    OP1 = (Cell)(cs_val(OP1));						\
    if (*((Psc *)OP1) == (Psc)OP2) {					\
      flag = READFLAG;							\
      sreg = (CPtr)OP1 + 1;						\
    }									\
    else Fail1;								\
  }									\
  else if ((Psc)OP2 == box_psc) {					\
    Cell ignore_addr;							\
    if (isfloat(OP1)) 							\
      bld_boxedfloat(CTXTc &ignore_addr, float_val(OP1));		\
    else if (isointeger(OP1)) 						\
      {bld_oint(&ignore_addr, oint_val(OP1));}				\
    else Fail1;								\
    flag = READFLAG;							\
    sreg = hreg - 3;							\
  } else if (isattv(OP1)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_str, interrupt needed\n"));    \
    /* add_interrupt(OP1, makecs(hreg)); */				\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(OP1) + 1)),makecs(hreg+INT_REC_SIZE));	\
    bind_copy((CPtr)dec_addr(OP1), makecs(hreg));                       \
    new_heap_functor(hreg, (Psc)OP2);					\
    flag = WRITE;							\
  }									\
  else Fail1;

/*======================================================================*/

#define nunify_with_list_sym(OP1)					\
  XSB_Deref(OP1);	       						\
  if (isref(OP1)) {							\
    /* op1 is FREE */							\
    bind_list((CPtr)(OP1), hreg);					\
    flag = WRITE;							\
  }									\
  else if (islist(OP1)) {						\
    sreg = clref_val(OP1);						\
    flag = READFLAG;							\
  }									\
  else if (isattv(OP1)) {						\
    xsb_dbgmsg((LOG_ATTV,">>>> ATTV nunify_with_list_sym, interrupt needed\n"));	\
    add_interrupt(CTXTc cell(((CPtr)dec_addr(op1) + 1)),makelist(hreg+INT_REC_SIZE));\
    bind_copy((CPtr)dec_addr(op1), makelist(hreg));                     \
    flag = WRITE;							\
  }									\
  else Fail1;

/*======================================================================*/

/*
 * In getattv, the flag will always be WRITE.  The unification will be
 * done here...
 * This operation is used in the getattv instruction, emitted for
 * asserted code with attributed variables.
 *
 * The way to do it:
 * 
 * href      ->  Op1   
 * href + 1  ->  _
 *
 * Put [reference to href + 1|X] in the interrupt queue.
 *
 * Set the WRITE flag to have the next instructions put the attribute
 * at href + 1.
 *
 * The interrupt should not be handled before the attribute is created.
 */
#define nunify_with_attv(OP1) {					\
  XSB_Deref(OP1);	       					\
  if (isref(OP1)) {						\
    bind_attv((CPtr)(OP1), hreg);				\
    new_heap_free(hreg);	/* the VAR part of the attv */	\
  }								\
  else {							\
    xsb_dbgmsg((LOG_ATTV,">>>> nunify_with_attv, interrupt needed\n"));	\
    add_interrupt(CTXTc (Integer)(hreg+(INT_REC_SIZE+1)), OP1); \
    *hreg = OP1; hreg++;			 \
  }								\
  flag = WRITE;							\
}

/*======================================================================*/

/* TLS: refactored to support Thread Cancellation */

#define call_sub(PSC) {							\
  if ( !(asynint_val) & !attv_pending_interrupts ) {   	        \
    lpcreg = (pb)get_ep(PSC);						\
  } else {								\
    if (attv_pending_interrupts) {					\
      synint_proc(CTXTc PSC, MYSIG_ATTV);		                \
      lpcreg = pcreg;							\
    }									\
    else {								\
      if (asynint_val) {						\
      if (asynint_val & KEYINT_MARK) {					\
        synint_proc(CTXTc PSC, MYSIG_KEYB);	                      	\
        lpcreg = pcreg;							\
        asynint_val = asynint_val & ~KEYINT_MARK;			\
        asynint_code = 0;						\
      } else if (asynint_val & PROFINT_MARK) {				\
        asynint_val &= ~PROFINT_MARK;					\
        log_prog_ctr(lpcreg);						\
        lpcreg = (byte *)get_ep(PSC);					\
      } else if (asynint_val & MSGINT_MARK) {                           \
        pcreg = (byte *)get_ep(PSC);					\
        intercept(CTXTc PSC);						\
        lpcreg = pcreg;							\
      } else if (asynint_val & THREADINT_MARK) {			\
        synint_proc(CTXTc PSC, THREADSIG_CANCEL);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
      } else if (asynint_val & TIMER_MARK) {				\
	/*	printf("Entered timer handle: call_sub\n");	*/	\
	synint_proc(CTXTc PSC, TIMER_INTERRUPT);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
      } else {								\
        lpcreg = (byte *)get_ep(PSC);					\
        asynint_val = 0;		         			\
      }									\
    }									\
  }									\
  }									\
}

#define proceed_sub {							\
  if ( !(asynint_val) & !attv_pending_interrupts ) {		\
     lpcreg = cpreg;							\
  } else {								\
    if (asynint_val) {							\
     if (asynint_val & KEYINT_MARK) {					\
        synint_proc(CTXTc true_psc, MYSIG_KEYB);			\
        lpcreg = pcreg;							\
        asynint_val = asynint_val & ~KEYINT_MARK;			\
        asynint_code = 0;						\
     } else if (asynint_val & MSGINT_MARK) {				\
       lpcreg = cpreg;  /* ignore MSGINT in proceed */			\
     } else if (asynint_val & PROFINT_MARK) {				\
       asynint_val &= ~PROFINT_MARK;					\
       log_prog_ctr(lpcreg);						\
       lpcreg = cpreg;							\
       asynint_code = 0;						\
     } else if (asynint_val & THREADINT_MARK) {				\
       /*printf("Entered thread cancel: proceed\n");*/			\
        synint_proc(CTXTc true_psc, THREADSIG_CANCEL);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
      } else if (asynint_val & TIMER_MARK) {				\
	/*	printf("Entered timer handle: call_sub\n");	*/	\
	synint_proc(CTXTc true_psc, TIMER_INTERRUPT);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
     } else {								\
        lpcreg = cpreg;							\
        asynint_code = 0;						\
     }									\
    } else if (attv_pending_interrupts) {				\
        synint_proc(CTXTc true_psc, MYSIG_ATTV);			\
        lpcreg = pcreg;							\
    }									\
  }									\
}

#define allocate_env_and_call_check_ints(numregs,envsize) 	\
do {								\
  int new11;							\
  Cell op11;							\
  CPtr term11; Psc psc11;					\
  if (efreg_on_top(ereg)) {					\
    op11 = (Cell)(efreg-1);					\
  } else {							\
    if (ereg_on_top(ereg)) op11 = (Cell)(ereg - envsize);	\
    else op11 = (Cell)(ebreg-1);				\
  }								\
  *(CPtr *)op11 = ereg;						\
  ereg = (CPtr)op11; 						\
  *((byte **)((CPtr)ereg-1)) = cpreg;				\
  *((byte **)((CPtr)ereg-2)) = lpcreg;	/* return here */	\
  psc11 = pair_psc(insert("ret",(byte)numregs,(Psc)flags[CURRENT_MODULE],&new11)); \
  term11 = (CPtr)build_call(CTXTc psc11);			\
  bld_cs((ereg-3),((Cell)term11));				\
  lpcreg = check_interrupts_restore_insts_addr;			\
} while(0)

