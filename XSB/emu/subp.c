/* File:      subp.c
** Author(s): Warren, Swift, Xu, Sagonas, Johnson
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
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public Livallcense for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: subp.c,v 1.117 2009/11/17 14:59:34 tswift Exp $
** 
*/


/* xsb_config.h must be the first #include.  Pls don't move it! */
#include "xsb_config.h"
#include "xsb_debug.h"


#include "debugs/debug_attv.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>

#ifdef WIN_NT
#include <windows.h>
#include <process.h>	/* _beginthread, _endthread */
#include <winbase.h>
#include <stddef.h>
#include <stdlib.h>
#include <winsock.h>
#include <io.h>
#include <string.h>
#else
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#endif

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "tries.h"
#include "debug_xsb.h"
#include "error_xsb.h"

#include "memory_xsb.h"
#include "register.h"
#include "heap_xsb.h"
#include "deref.h"
#include "flags_xsb.h"
#include "binding.h"
#include "trie_internals.h"
#include "trassert.h"
#include "choice.h"
#include "token_xsb.h"
#include "sig_xsb.h"
#include "inst_xsb.h"
#include "tab_structs.h"
#include "table_stats.h"
#include "unify_xsb.h"
#include "subp.h"
#include "thread_xsb.h"
#include "debug_xsb.h"
#include "hash_xsb.h"
#include "trace_xsb.h"
#include "tr_utils.h"
#include "system_defs_xsb.h"
#include "builtin.h"
#include "struct_intern.h"
#include "cell_xsb_i.h"

extern void debug_call(CTXTdeclc Psc);

/*======================================================================*/
extern xsbBool quotes_are_needed(char *string);

/*======================================================================*/

#ifndef MULTI_THREAD
extern int asynint_val;	/* 0 - no interrupt (or being processed) */
extern int asynint_code;	/* 0 means keyboard interrupt */
#endif

#ifdef LINUX
static struct sigaction int_act, int_oact;
static struct sigaction abrt_act, abrt_oact;
#endif

void (*xsb_default_segfault_handler)(int); /* where the previous value of the
					     SIGSEGV/SIGBUS handler is saved */

/*
 * Put an attv interrupt into the interrupt chain. op1 is the related
 * attv, and op2 is the value (see verify_attributes/2).
 */

void add_interrupt(CTXTdeclc Cell op1, Cell op2) {

  Cell head, tail, temp;
  CPtr addr_head, addr_tail;

  addr_head = (CPtr)glstack.low;
  head = cell(addr_head); // addr of interrupt list
  addr_tail = (CPtr)glstack.low+1;
  tail = cell(addr_tail); // addr of last cons cell of interrupt list

  // Build the new list cons pair and the new op-pair cons
  // This record is 4 words long and so INT_REC_SIZE=4
  bld_list(&temp,hreg);  // temp -> new cons pair 1
  bld_list(hreg,hreg+2); // 1.car -> 2nd new cons pair 2
  bld_free(hreg+1);        // 1.cdr is free var
  bld_copy(hreg+2,op1);    // 2.car is op1
  bld_copy(hreg+3,op2);    // 2.cdr is op2
  hreg += 4;

  if (isnonvar(head)) { // nonempty
    CPtr addr_cdr;
    addr_cdr = clref_val(tail)+1;
    bind_copy(addr_cdr,temp);
    push_pre_image_trail(addr_tail,temp);
    bld_copy(addr_tail,temp);
  } else { // first
    bind_copy(addr_head,temp);
    bind_copy(addr_tail,temp);
  }
}

Cell build_interrupt_chain(CTXTdecl) {
  Cell head, tail;
  CPtr addr_head, addr_tail;

  addr_tail = (CPtr)glstack.low+1;
  tail = cell(addr_tail); // addr of last cons cell of interrupt list
  bind_nil(clref_val(tail)+1);  // close the tail

  addr_head = (CPtr)glstack.low;
  head = cell(addr_head);  

  // set intlist back to empty;
  push_pre_image_trail(addr_head,addr_head);
  bld_free(addr_head);
  push_pre_image_trail(addr_tail,addr_tail);
  bld_free(addr_tail);

  return(head); // addr of interrupt list
}







/*======================================================================*/
/*  Unification routines.						*/
/*======================================================================*/

/* DSW (11/7/2011): This is a unify routine (copied from the
   unify_xsb(_) macro and modified) to unify cyclic terms.  It uses
   pointer changing and preimage trailing.  The third argument,
   op1loc, is a pointer to the memory location where the op1 pointer
   was loaded from.  It uses this to change that address to point to
   op2 if they are potentially matching structures.  But op1loc may be
   a small integer (under 100 and > 0), in which case the pointer
   swizzling is not done and recursive calls reduce it by one.  So
   calling it with op1loc = 10, e.g., causes it not to do the pointer
   swizzling until it gets 10 levels deep in the recursion.  This
   allows it to avoid somewhat expensive pre-image trailing for most
   acyclic unifications, and yet catch them all, albeit later. */

xsbBool unify_rat(CTXTdeclc Cell rop1, Cell rop2, CPtr op1loc) {
  register Cell op1, op2;
  op1 = rop1; op2 = rop2;

 tail_recursion:
  XSB_Deref2(op1, goto label_op1_free);
  XSB_Deref2(op2, goto label_op2_free);

  if (isattv(op1)) goto label_op1_attv;
  if (isattv(op2)) goto label_op2_attv;

  if (isfloat(op2) && isboxedfloat(op1) ) {
    if ( float_val(op2) == (float)boxedfloat_val(op1))
      {return 1;}
    else
      {return 0;}
  }
  if (isfloat(op1) && isboxedfloat(op2) ) {
    if ( float_val(op1) == (float)boxedfloat_val(op2))
      {return 1;}
    else
      {return 0;}
  }

  if (cell_tag(op1) != cell_tag(op2))
    {return 0;}

  if (isconstr(op1)) goto label_both_struct;
  if (islist(op1)) goto label_both_list;
  /* now they are both atomic */
  if (op1 == op2) {return 1;}
  return 0;

 label_op1_free:
  XSB_Deref2(op2, goto label_both_free);
  if (flags[UNIFY_WITH_OCCURS_CHECK_FLAG] &&
      (isconstr(op2) || islist(op2)) &&
      (COND1) && !not_occurs_in(op1,op2)) {
    return 0;
  } else {
    bind_copy((CPtr)(op1), op2);
    return 1;
  }

 label_op2_free:
  if (flags[UNIFY_WITH_OCCURS_CHECK_FLAG] &&
      (isconstr(op1) || islist(op1)) &&
      (COND2) && !not_occurs_in(op2,op1)) {
    return 0;
  } else {
    bind_copy((CPtr)(op2), op1);
    return 1;
  }

 label_both_free:
  if ( (CPtr)(op1) == (CPtr)(op2) ) {return 1;}
  if ( (CPtr)(op1) < (CPtr)(op2) )
    {
      if (COND1)
	/* op1 not in local stack */
	{ bind_ref((CPtr)(op2), (CPtr)(op1)); }
      else  /* op1 points to op2 */
	{ bind_ref((CPtr)(op1), (CPtr)(op2)); }
      }
  else
    { /* op1 > op2 */
      if (COND2)
	{ bind_ref((CPtr)(op1), (CPtr)(op2)); }
      else
	{ bind_ref((CPtr)(op2), (CPtr)(op1)); }
    }
  return 1;

 label_both_list:
  if (op1 == op2) {return 1;}

  if ((char *)op1loc > (char *)100) {
    //    printf("ppil\n");
    push_pre_image_trail(op1loc,op2);
    cell(op1loc) = op2;
  }

  op1 = (Cell)(clref_val(op1));
  op2 = (Cell)(clref_val(op2));
  if ( !unify_rat(CTXTc cell((CPtr)op1), cell((CPtr)op2),
		  (((op1loc==0||((char *)op1loc>(char *)100)))?((CPtr)op1):((CPtr)((char *)op1loc-1)))))
    { return 0; }
  if ((op1loc==0||((char *)op1loc>(char *)100))) op1loc = ((CPtr)op1+1);
  else op1loc = ((CPtr)((char *)op1loc-1));
  op1 = cell((CPtr)op1+1);
  op2 = cell((CPtr)op2+1);
  goto tail_recursion;

 label_both_struct:
  if (op1 == op2) {return 1;}

  /* a != b */
  op1 = (Cell)(clref_val(op1));
  op2 = (Cell)(clref_val(op2));
  if (((Pair)(CPtr)op1)->psc_ptr!=((Pair)(CPtr)op2)->psc_ptr)
    {
      /* 0(a) != 0(b) */
      return 0;
    }
  {
    int arity = get_arity(((Pair)(CPtr)op1)->psc_ptr);
    if (arity == 0) {return 1;}

    if ((char *)op1loc>(char *)100) {
      //      printf("ppif\n");
      push_pre_image_trail(op1loc,makecs(op2));
      cell(op1loc) = makecs(op2);
    }
    while (--arity)
      {
	op1 = (Cell)((CPtr)op1+1); op2 = (Cell)((CPtr)op2+1);
	if (!unify_rat(CTXTc cell((CPtr)op1), cell((CPtr)op2),
		       (((op1loc==0||((char *)op1loc>(char *)100)))?((CPtr)op1):((CPtr)((char *)op1loc-1)))))
	  {
	    return 0;
	  }
      }
    if ((op1loc==0||((char *)op1loc>(char *)100))) op1loc = ((CPtr)op1+1);
    else op1loc = ((CPtr)((char *)op1loc-1));
    op1 = cell((CPtr)op1+1); 				
    op2 = cell((CPtr)op2+1);			
    goto tail_recursion;
  }

  /* if the order of the arguments in add_interrupt */
  /* is not important, the following three can actually */
  /* be collapsed into one; loosing some meaningful */
  /* attv_dbgmsg - they have been lost partially */
  /* already */

 label_op1_attv:
  if (isattv(op2)) goto label_both_attv;
  attv_dbgmsg(">>>> ATTV = something, interrupt needed\n");
  add_interrupt(CTXTc cell(((CPtr)dec_addr(op1) + 1)),op2);
  bind_copy((CPtr)dec_addr(op1), op2);
  return 1;

 label_op2_attv:
  attv_dbgmsg(">>>> something = ATTV, interrupt needed\n");
  add_interrupt(CTXTc cell(((CPtr)dec_addr(op2) + 1)),op1);
  bind_copy((CPtr)dec_addr(op2), op1);
  return 1;

 label_both_attv:
  if (op1 != op2)
    {
      attv_dbgmsg(">>>> ATTV = ???, interrupt needed\n");
      add_interrupt(CTXTc cell(((CPtr)dec_addr(op1) + 1)),op2);
      bind_copy((CPtr)dec_addr(op1), op2);
    }
  return 1;
}

/* the follow redefinitions change how the unify_xsb() macro below works! */
#undef IFTHEN_FAILED
#define IFTHEN_FAILED	return 0
#undef IFTHEN_SUCCEED
#define IFTHEN_SUCCEED	return 1

xsbBool unify(CTXTdeclc Cell rop1, Cell rop2)
{ /* begin unify */
  register Cell op1, op2;

  op1 = rop1; op2 = rop2;

/*----------------------------------------*/
  unify_xsb(unify);
  /* unify_xsb.h already ends with this statement
     IFTHEN_SUCCEED;
  */
/*----------------------------------------*/

}  /* end of unify */

/*======================================================================*/
/*  Determining whether two terms are identical.			*/
/*  (Used mostly by subsumptive trie lookup routines)                   */
/*======================================================================*/

xsbBool are_identical_terms(Cell term1, Cell term2) {

 begin_are_identical_terms:
  XSB_Deref(term1);
  XSB_Deref(term2);
  
  if ( term1 == term2 )
    return TRUE;

  if ( cell_tag(term1) != cell_tag(term2) )
    return FALSE;

  if ( cell_tag(term1) == XSB_STRUCT ) {
    CPtr cptr1 = clref_val(term1);
    CPtr cptr2 = clref_val(term2);
    Psc psc1 = (Psc)*cptr1;
    int i;

    if ( psc1 != (Psc)*cptr2 )
      return FALSE;

    for ( cptr1++, cptr2++, i = 0;  i < (int)get_arity(psc1)-1;  cptr1++, cptr2++, i++ )
      if ( ! are_identical_terms(*cptr1,*cptr2) ) 
	return FALSE;
    term1 = *cptr1; 
    term2 = *cptr2;
    goto begin_are_identical_terms;
  }
  else if ( cell_tag(term1) == XSB_LIST ) {
    CPtr cptr1 = clref_val(term1);
    CPtr cptr2 = clref_val(term2);

    if ( are_identical_terms(*cptr1, *cptr2) ) {
      term1 = *(cptr1 + 1); 
      term2 = *(cptr2 + 1);
      goto begin_are_identical_terms;
    } else return FALSE;
  }
  else return FALSE;
}

/*======================================================================*/
/*======================================================================*/

static void default_inthandler(CTXTdeclc int intcode)
{
  char message[80];

  switch (intcode) {
  case MYSIG_UNDEF:
    xsb_exit( "Undefined predicate; exiting by the default handler.");
    break;
  case MYSIG_KEYB:
    xsb_exit( "Keyboard interrupt; exiting by the default handler.");
    break;
  case MYSIG_PSC:
    break;
  default:
    sprintf(message,
	    "Unknown interrupt (%d) occured; exiting by the default handler", 
	    intcode);
    xsb_exit( message);
    break;
  }
}

/*======================================================================*/
/* builds the current call onto the heap and returns a pointer to it.	*/
/*======================================================================*/

Pair build_call(CTXTdeclc Psc psc)
{
  register Cell arg;
  register Pair callstr;
  register int i;

  callstr = (Pair)hreg; /* save addr of new structure rec */
  new_heap_functor(hreg, psc); /* set str psc ptr */
  for (i=1; i <= (int)get_arity(psc); i++) {
    arg = cell(reg+i);
    nbldval_safe(arg);
  }
  return callstr;
}

/*======================================================================*/
/* set interrupt code in reg 2 and return ep of interrupt handler.	*/
/* the returned value is normally assigned to pcreg, so this is like	*/
/* raising a trap.							*/
/* Note that the interrupt handlers referred to by flags array values   */
/* are set up on the Prolog side via set_inthandler/2                   */
/* PSC value is value of continuation                                   */
/*======================================================================*/

Psc synint_proc(CTXTdeclc Psc psc, int intcode)
{
  if (pflags[intcode+INT_HANDLERS_FLAGS_START]==(Cell)0) {
    /* default hard handler */
    default_inthandler(CTXTc intcode);
    psc = 0;
  } else {				/* call Prolog handler */
    switch (intcode) {
    case MYSIG_UNDEF:		/*  0 */
      SYS_MUTEX_LOCK( MUTEX_LOAD_UNDEF ) ;
    case MYSIG_KEYB:		/*  1 */
    case MYSIG_SPY:		/*  3 */
    case MYSIG_TRACE:		/*  4 */
    case TIMER_INTERRUPT:		/* d */
    case THREADSIG_CANCEL:		/* f */
    case MYSIG_CLAUSE:		/* 16 */
      if (psc) bld_cs(reg+1, build_call(CTXTc psc));
      psc = (Psc)pflags[intcode+INT_HANDLERS_FLAGS_START];
      bld_int(reg+2, asynint_code);
      pcreg = get_ep(psc);
      break;
    case MYSIG_ATTV:		/*  8 */
      /* the old call must be built first */
      if (psc)
	bld_cs(reg+2, build_call(CTXTc psc));
      psc = (Psc)pflags[intcode+INT_HANDLERS_FLAGS_START];
      /*
       * Pass the interrupt chain to reg 1.  The interrupt chain
       * will be reset to 0 in build_interrupt_chain()).
       */
      bld_copy(reg + 1, build_interrupt_chain(CTXT));
      /* bld_int(reg + 3, intcode); */	/* Not really needed */
      pcreg = get_ep(psc);
      break;
    default:
      xsb_abort("Unknown intcode in synint_proc()");
    } /* switch */
  }
  return psc;
}

void init_interrupt(void);

/* TLS: 2/02 removed "inline static" modifiers so that this function
   can be called from interprolog_callback.c */
void keyint_proc(int sig)
{
#ifdef MULTI_THREAD
  th_context *th = find_context(xsb_thread_self());

  if (th->cond_var_ptr != NULL)
	pthread_cond_broadcast( th->cond_var_ptr ) ;
#endif
#ifndef LINUX
  init_interrupt();  /* reset interrupt, if using signal */
#endif
  if (asynint_val & KEYINT_MARK) {
    xsb_abort("unhandled keyboard interrupt");
  } else {
    asynint_val |= KEYINT_MARK;
    asynint_code = 0;
  }
}

void cancel_proc(int sig)
{
#ifdef MULTI_THREAD
  th_context *th = find_context(xsb_thread_self());

  if (th->cond_var_ptr != NULL)
	pthread_cond_broadcast( th->cond_var_ptr ) ;
#endif
#ifndef LINUX
  init_interrupt();  /* reset interrupt, if using signal */
#endif
  //    asynint_val |= THREADINT_MARK;
    asynint_code = 0;
}

/* Called during XSB initialization -- could be in init_xsb.c, apart
   from funky use in keyint_proc() */

void init_interrupt(void)
{
#if (defined(LINUX))
  int_act.sa_handler = keyint_proc;
  sigemptyset(&int_act.sa_mask); 
  int_act.sa_flags = 0;
  sigaction(SIGINT, &int_act, &int_oact);

  abrt_act.sa_handler = cancel_proc;
  sigemptyset(&abrt_act.sa_mask); 
  abrt_act.sa_flags = 0;
  sigaction(SIGABRT, &abrt_act, &abrt_oact);
#else
  signal(SIGINT, keyint_proc); 
  signal(SIGABRT, cancel_proc); 
#endif

#if (defined(DEBUG_VERBOSE) || defined(DEBUG_VM) || defined(DEBUG_ASSERTIONS) || defined(DEBUG))
  /* Don't handle SIGSEGV/SIGBUS if configured with DEBUG */
  xsb_default_segfault_handler = SIG_DFL;
#else 
  xsb_default_segfault_handler = xsb_segfault_quitter;
#endif

#ifdef SIGBUS
  signal(SIGBUS, xsb_default_segfault_handler);
#endif
  signal(SIGSEGV, xsb_default_segfault_handler);
}


/*
 * Maintains max stack usage when "-s" option is given at startup.
 */
void intercept(CTXTdeclc Psc psc) {

  if (pflags[CLAUSE_INT])
    synint_proc(CTXTc psc, MYSIG_CLAUSE);
  else if (flags[DEBUG_ON] && !flags[HIDE_STATE]) {
    if (get_spy(psc)) { /* spy'ed pred, interrupted */
      synint_proc(CTXTc psc, MYSIG_SPY);
      flags[HIDE_STATE]++; /* hide interrupt handler */
    }
    else if (flags[TRACE]) {
      synint_proc(CTXTc psc, MYSIG_TRACE);
      flags[HIDE_STATE]++; /* hide interrupt handler */
    }
  }
  if (flags[HITRACE])
    debug_call(CTXTc psc);

}

/*======================================================================*/
/* floating point conversions                                           */
/*    The below 3 methods are to be used when floats and Cells are the  */
/*    same size, in bytes, to convert between the two.                  */
/*======================================================================*/

/* lose some precision in conversions from 32 bit formats */

#ifdef BITS64
#define FLOAT_MASK 0xfffffffffffffff8
#else
#define FLOAT_MASK 0xfffffff8
#endif

int sign(Float num)
{
  if (num==0.0) return 0;
  else if (num>0.0) return 1;
  else return -1;
}

/*======================================================================*/
/* compare(V1, V2)							*/
/*	compares two terms; returns zero if V1=V2, a positive value	*/
/*	if V1>V2 and a negative value if V1<V2.  Term comparison is	*/
/*	done according to the ISO standard total order of Prolog	*/
/*	terms which is as follows:					*/
/*									*/
/*	    variables < floats < integers < atoms < compound terms	*/
/*									*/
/*	A list is compared as an ordinary compound term with arity	*/
/*	2 and functor '.'.						*/
/*									*/
/*	This function was rewritten from scratch by Kostis so that	*/
/*	it is independent of the relative order of tag encoding.	*/
/*	However, it should ONLY be used to compare terms that appear	*/
/*	in the above ordering list.					*/
/*======================================================================*/
#define sign_of(exp) (((exp)>0)?1:((exp)<0)?-1:0)

int compare(CTXTdeclc const void * v1, const void * v2)
{
  int comp;
  Integer compexp;
  CPtr cptr1, cptr2;
  Cell val1 = (Cell) v1 ;
  Cell val2 = (Cell) v2 ;

  XSB_Deref(val2);		/* val2 is not in register! */
  XSB_Deref(val1);		/* val1 is not in register! */
  if (val1 == val2) return 0;
  switch(cell_tag(val1)) {
  case XSB_FREE:
  case XSB_REF1:
    if (isattv(val2)) {
      compexp = (vptr(val1) - (CPtr)dec_addr(val2));
      return sign_of(compexp);
    }
    else if (isnonvar(val2)) return -1;
    else { /* in case there exist local stack variables in the	  */
	   /* comparison, globalize them to guarantee that their  */
	   /* order is retained as long as nobody "touches" them  */
	   /* in the future -- without copying garbage collection */
      if ((top_of_localstk <= vptr(val1)) &&
	  (vptr(val1) <= (CPtr)glstack.high-1)) {
	bld_free(hreg);
	bind_ref(vptr(val1), hreg);
	hreg++;
	val1 = follow(val1);	/* deref again */
      }
      if ((top_of_localstk <= vptr(val2)) &&
	  (vptr(val2) <= (CPtr)glstack.high-1)) {
	bld_free(hreg);
	bind_ref(vptr(val2), hreg);
	hreg++;
	val2 = follow(val2);	/* deref again */
      }
      compexp = (vptr(val1) - vptr(val2));
      return sign_of(compexp);
    }
  case XSB_FLOAT:
    if (isref(val2) || isattv(val2)) return 1;
    else if (isofloat(val2)) {
      return sign(float_val(val1) - ofloat_val(val2));
    } else return -1;
  case XSB_INT:
    if (isref(val2) || isofloat(val2) || isattv(val2)) return 1;
    else if (isointeger(val2)) {
      compexp = (int_val(val1) - oint_val(val2));
      return sign_of(compexp);
    } else return -1;
  case XSB_STRING:
    if (isref(val2) || isofloat(val2) || isointeger(val2) || isattv(val2)) 
      return 1;
    else if (isstring(val2)) {
      return strcmp(string_val(val1), string_val(val2));
    }
    else return -1;
  case XSB_STRUCT:
    // below, first 2 if-checks test to see if this struct is actually a number representation,
    // (boxed float or boxed int) and if so, does what the number case would do, only with boxed_val
    // macros.
    if (isboxedinteger(val1)) {
      if (isref(val2) || isofloat(val2) || isattv(val2)) return 1;
      else if (isointeger(val2)) {
	compexp = (boxedint_val(val1) - oint_val(val2));
	return sign_of(compexp);
      } else return -1;
    } else if (isboxedfloat(val1)) {
        if (isref(val2) || isattv(val2)) return 1;
        else if (isofloat(val2)) 
          return sign(boxedfloat_val(val1) - ofloat_val(val2));
        else return -1; 
    } else if ((cell_tag(val2) != XSB_STRUCT && cell_tag(val2) != XSB_LIST)
	       || isboxedinteger(val2) || isboxedfloat(val2))  return 1;
    else {
      int arity1, arity2;
      Psc ptr1 = get_str_psc(val1);
      Psc ptr2 = get_str_psc(val2);

      arity1 = get_arity(ptr1);
      if (islist(val2)) arity2 = 2; 
      else arity2 = get_arity(ptr2);
      if (arity1 != arity2) return arity1-arity2;
      if (islist(val2)) comp = strcmp(get_name(ptr1), ".");
      else comp = strcmp(get_name(ptr1), get_name(ptr2));
      if (comp || (arity1 == 0)) return comp;
      cptr1 = clref_val(val1);
      cptr2 = clref_val(val2);
      for (arity2 = 1; arity2 <= arity1; arity2++) {
	if (islist(val2))
	  comp = compare(CTXTc (void*)cell(cptr1+arity2), (void*)cell(cptr2+arity2-1));  
	else
	  comp = compare(CTXTc (void*)cell(cptr1+arity2), (void*)cell(cptr2+arity2));
	if (comp) break;
      }
      return comp;
    }
    break;
  case XSB_LIST:
    if (cell_tag(val2) != XSB_STRUCT && cell_tag(val2) != XSB_LIST) return 1;
    else if (isconstr(val2)) return -(compare(CTXTc (void*)val2, (void*)val1));
    else {	/* Here we are comparing two list structures. */
      cptr1 = clref_val(val1);
      cptr2 = clref_val(val2);
      comp = compare(CTXTc (void*)cell(cptr1), (void*)cell(cptr2));
      if (comp) return comp;
      return compare(CTXTc (void*)cell(cptr1+1), (void*)cell(cptr2+1));
    }
    break;
  case XSB_ATTV:
    if (isattv(val2)) {
      compexp = (CPtr)dec_addr(val1) - (CPtr)dec_addr(val2);
      return sign_of(compexp);
    } else if (isref(val2)) {
      compexp = (CPtr)dec_addr(val1) - vptr(val2);
      return sign_of(compexp);
    } else
      return -1;
  default:
    xsb_abort("Compare (unknown tag %ld); returning 0", cell_tag(val1));
    return 0;
  }
}

/*======================================================================*/
/* key_compare(V1, V2)							*/
/*	compares the keys of two terms of the form Key-Value; returns	*/
/*	zero if Key1=Key2, a positive value if Key1>Key2 and a negative */
/*	value if Key1<Key2.  Term comparison is done according to the	*/
/*	standard total order of Prolog terms (see compare()).		*/
/*======================================================================*/

int key_compare(CTXTdeclc const void * t1, const void * t2)
{
  Cell term1 = (Cell) t1 ;
  Cell term2 = (Cell) t2 ;

  XSB_Deref(term1);		/* term1 is not in register! */
  XSB_Deref(term2);		/* term2 is not in register! */
  return compare(CTXTc (void*)get_str_arg(term1,1), (void*)get_str_arg(term2,1));
}

/*======================================================================*/
/* print an atom as quoted.						*/
/* This, and the next few functions are used in file_puttoken.  I have  */
/* no idea why we keep them here.                                       */
/*======================================================================*/

void print_aqatom(FILE *file, int charset, char *string) {
  int loc = 0;

  if (charset == UTF_8) {
    fprintf(file,"'");
    while (string[loc] != '\0') {
      if (string[loc] == '\'') fprintf(file,"'");
      fprintf(file,"%c",string[loc++]);
    }
    fprintf(file,"'");
  } else {
    write_string_code(file,charset,(byte *)"'");
    while (*string != '\0') {
      if (*string == '\'') write_string_code(file,charset,(byte *)"'");
      PutCode(utf8_char_to_codepoint((byte **)&string),charset,file);
    }
    write_string_code(file,charset,(byte *)"'");
  }
}

/*======================================================================*/
/* print an atom, quote it if necessary.				*/
/*======================================================================*/

void print_qatom(FILE *file, int charset, char *string)
{
  if (quotes_are_needed(string)) print_aqatom(file, charset, string);
  else write_string_code(file,charset,(byte *)string);
}

/*======================================================================*/
/* print a double quoted string, doubling internal double quotes 	*/
/* if necessary.							*/
/*======================================================================*/

void print_dqatom(FILE *file, int charset, char *string) {
  if (charset == UTF_8) {
    int loc = 0;
    fprintf(file,"\"");
    while (string[loc] != '\0') {
      if (string[loc] == '"') fprintf(file,"\"");
      fprintf(file,"%c",string[loc++]);
    }
    fprintf(file,"\"");
  } else {
    write_string_code(file,charset,(byte *)"\"");
    while (*string != '\0') {
      if (*string == '"') write_string_code(file,charset,(byte *)"\"");
      PutCode(utf8_char_to_codepoint((byte **)&string),charset,file);
    }
    write_string_code(file,charset,(byte *)"\"");
  }
}

/*======================================================================*/
/* print an operator.							*/
/*======================================================================*/

void print_op(FILE *file, int charset, char *string, int pos)
{

  if (*(string+1) == '\0' && (*string == ',' || *string == ';')) {
    write_string_code(file,CURRENT_CHARSET,(byte *)string);
  } else {
    switch (pos) {
    case 1: print_qatom(file, charset, string); putc(' ', file); break;
    case 2: putc(' ', file);
      print_qatom(file, charset, string); putc(' ', file); break;
    case 3: putc(' ', file); print_qatom(file, charset, string); break;
    }
  }
}

/* ----- The following is also called from the Prolog level ----------- */

void remove_incomplete_tables_reset_freezes(CTXTdeclc int memory_flag)
{
  if (memory_flag == MEMORY_ERROR) {
    //    printf("memory\n");
    abolish_all_tables(CTXTc ABOLISH_INCOMPLETES);
  }
  else {
    //    printf("non_memory\n");
    remove_incomplete_tables();
  }
    reset_freeze_registers;
}

/* ----- C level exception handlers ----------------------------------- */

/* SIGSEGV/SIGBUS handler that catches segfaults; used unless 
   configured with DEBUG */ 
void xsb_segfault_catcher(int err)
{
  char *tmp_message = xsb_segfault_message;
#ifdef MULTI_THREAD
  xsb_exit( tmp_message);
#else
  xsb_segfault_message = xsb_default_segfault_msg; /* restore default */
  printf("segfault!!\n");
  xsb_basic_abort(tmp_message);
#endif
}

void xsb_segfault_quitter(int err)
{
#ifdef MULTI_THREAD
  th_context *th = find_context(xsb_thread_self());
#endif
  print_xsb_backtrace(CTXT);
  xsb_exit( xsb_segfault_message);
}

#ifdef WIN_NT
/* Our separate thread */
void checkJavaInterrupt(void *info)
{
  char ch;
  SOCKET intSocket = (SOCKET)info;
  xsb_dbgmsg((LOG_DEBUG, "Thread started on socket %ld",(int)intSocket));
  while(1){
    if (1!=recv(intSocket,&ch,1,0)) {
      warn_xsb("Problem handling interrupt from Java");
    }
    else 
      xsb_mesg("--- Java interrupt detected");
    /* Avoid those annoying lags? */
    fflush(stdout);
    fflush(stderr);
    fflush(stdmsg);
    fflush(stdwarn);
    fflush(stddbg);
    keyint_proc(SIGINT); /* Do XSB's "interrupt" thing */
  }
}

xsbBool startInterruptThread(SOCKET intSocket)
{
  xsb_mesg("Beginning interrupt thread on socket %ld",(int)intSocket);
#ifdef _MT
  _beginthread( checkJavaInterrupt, 0, (void*)intSocket );
#endif
  return 1;
}
#endif


extern Integer if_profiling;
extern Integer prof_gc_count;
extern Integer prof_int_count;
extern int garbage_collecting;

void setProfileBit(void *place_holder) {
#ifdef MULTI_THREAD
  th_context *th = find_context(xsb_thread_self());
#endif
  while (TRUE) {
    if (if_profiling) {
      if (garbage_collecting) {
	prof_gc_count++;
      } else {
	prof_int_count++;
	asynint_val |= PROFINT_MARK;
      }
    }
#ifdef WIN_NT
    Sleep(10);
#else
    usleep(10000);
#endif
  }
}

xsbBool startProfileThread()
{
#ifdef WIN_NT
  HANDLE Thread;
  if (!if_profiling) {
    Thread = (HANDLE)_beginthread(setProfileBit,0,NULL);
    SetThreadPriority(Thread,THREAD_PRIORITY_HIGHEST/*_ABOVE_NORMAL*/);
  }
#elif defined(SOLARIS)
  printf("Profiling not supported\n");
#else
  pthread_t         a_thread;
  struct sched_param param;

  if (!if_profiling) {
    pthread_create(&a_thread, NULL, (void*)&setProfileBit, (void*)NULL);
    param.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_setschedparam(a_thread, SCHED_OTHER, &param);

    if_profiling = 1;
  }
#endif
  return TRUE;
}

#ifndef MULTI_THREAD

int sleep_interval;
#ifdef WIN_NT
HANDLE executing_sleeper_thread = NULL;
#else
pthread_t executing_sleeper_thread = (pthread_t) NULL;
#endif

// TLS: For some embarassing reason, I don't seem to be able to pass a
// parameter to the thread function executeSleeperThread() (?!?) So
// I'm using a global.
void
#ifdef WIN_NT
_stdcall  // since no parameters, does not matter it seems.
#endif
executeSleeperThread(void * interval) {
  //  long *i1;
  //  i1 = (long *) interval;
  int i;
  i = sleep_interval;
  //  printf("found sleeper thread %p %d\n",i1, *(long *)i1);
  //  printf("found sleeper thread %d\n",i);
#ifdef WIN_NT
  Sleep(i);
#else
  usleep(1000*i);  // want milliseconds
#endif
  asynint_val |= TIMER_MARK;
  if (executing_sleeper_thread) {
#ifdef WIN_NT
    CloseHandle(executing_sleeper_thread);
    executing_sleeper_thread = (HANDLE) NULL;
#else
    executing_sleeper_thread = (pthread_t) NULL;
#endif
  }
}

xsbBool cancelSleeperThread(void) {
#ifdef WIN_NT
  /*
  */
  if (executing_sleeper_thread){
    if (!TerminateThread(executing_sleeper_thread,0)) {
      xsb_warn(CTXTc "could not kill sleeper thread\n");
    }
    CloseHandle(executing_sleeper_thread);
    executing_sleeper_thread = (HANDLE) NULL;
  }
  //return FALSE;  // should be defined??
#else
  if (executing_sleeper_thread) { // previous sleeper
    int killrc;
    if ((killrc = pthread_cancel(executing_sleeper_thread))) {
	xsb_warn(CTXTc "could not kill sleeper thread: error %d\n",killrc);
    }
    executing_sleeper_thread = (pthread_t) NULL;
  }
#endif
  asynint_val = asynint_val & ~TIMER_MARK;	
  return TRUE;
}

// TLS, copied thread start for windows from startProfileThread()
xsbBool startSleeperThread(int interval) {
  //  struct sched_param param;

#ifdef WIN_NT
  int killrc;
  HANDLE sleeper_thread;
  sleep_interval = interval;
  if (executing_sleeper_thread) { // previous sleeper still running, now obsolete, so kill it.
    killrc = TerminateThread(executing_sleeper_thread,0);
    CloseHandle(executing_sleeper_thread);
    executing_sleeper_thread = NULL;
  }
  //    sleeper_thread = (HANDLE)_beginthread(executeSleeperThread,0,NULL);
    // Miguel's change
  sleeper_thread = (HANDLE)_beginthreadex(NULL,0,(unsigned (__stdcall *)(void *))executeSleeperThread,NULL,0,NULL);
  executing_sleeper_thread = sleeper_thread;
  SetThreadPriority(sleeper_thread,THREAD_PRIORITY_HIGHEST/*_ABOVE_NORMAL*/);
  Sleep(1);  // race condition, need to get into sleeper, otw may never get control? (priority?)
#else
  pthread_t         sleeper_thread;
  struct sched_param param;
  sleep_interval = interval;
  int killrc;

  if (executing_sleeper_thread) { // previous sleeper, now obsolete
    if ((killrc = pthread_cancel(executing_sleeper_thread))) {
	xsb_warn(CTXTc "could not kill sleeper thread: error %d\n",killrc);
    }
    executing_sleeper_thread = (pthread_t) NULL;
  }
  pthread_create(&sleeper_thread, NULL, (void*)&executeSleeperThread,(void *) &sleep_interval);
  param.sched_priority = sched_get_priority_max(SCHED_OTHER);
  pthread_setschedparam(sleeper_thread, SCHED_OTHER, &param);
  pthread_detach(sleeper_thread);
  executing_sleeper_thread = sleeper_thread;
#endif
  //  pthread_create(&a_thread, NULL, (void*)&executeSleeperThread,NULL);
  //  printf("i %p %d\n",&i,*&i);
  //  param.sched_priority = sched_get_priority_max(SCHED_OTHER);
  //  pthread_setschedparam(a_thread, SCHED_OTHER, &param);

    return TRUE;
  
}
#endif
