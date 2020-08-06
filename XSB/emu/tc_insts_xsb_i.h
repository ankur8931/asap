/* File:      tc_insts_xsb_i.h
** Author(s): Prasad Rao, Kostis Sagonas
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
** $Id: tc_insts_xsb_i.h,v 1.47 2012-08-12 14:52:36 tswift Exp $
** 
*/

#include "debugs/debug_tries.h"

//#define DEBUG_TRIE_INSTRS 1
#ifdef DEBUG_TRIE_INSTRS
#define trie_instr_print(X) printf X
#else
#define trie_instr_print(X) 
#endif

/*----------------------------------------------------------------------*/
/* The following is the list of Trie-Code instructions.			*/
/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_str,_trie_no_cp_str)
	TRIE_R_LOCK();
	trie_instr_print(("trie_no_cp_str\n"));
	//	printf("stk %x\n",trieinstr_unif_stkptr-trieinstr_unif_stk);
	//	printf("stack bottom %x @ %x\n",trieinstr_unif_stk[0],*(CPtr)trieinstr_unif_stk[0]);
	NodePtr = (BTNptr) lpcreg;
	unify_with_trie_str;
	non_ftag_lpcreg;
XSB_End_Instr()

/* TLS: opfail below is actually the sibling trie node */
XSB_Start_Instr(trie_try_str,_trie_try_str) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
	trie_instr_print(("trie_try_str\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);                        
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);  
#ifdef SLG_GC                                              
        cp_prevtop(tbreg) = old_cptop;
#endif                                                     
	breg = tbreg;
	hbreg = hreg;
	unify_with_trie_str;
	non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_str,_trie_retry_str) 
        CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_retry_str\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) =  (byte *)opfail;
	unify_with_trie_str;
	non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_str,_trie_trust_str) 
        CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_trust_str\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);	/* Remove this CP */
	restore_trail_condition_registers(breg);
	unify_with_trie_str;
	non_ftag_lpcreg;
XSB_End_Instr()

/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_numcon,_trie_no_cp_numcon)
	TRIE_R_LOCK();
       	trie_instr_print(("trie_no_cp_numcon\n"));
	//	printf("stk %x\n",trieinstr_unif_stkptr-trieinstr_unif_stk);
	NodePtr = (BTNptr) lpcreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
	non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_try_numcon,_trie_try_numcon) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
	trie_instr_print(("trie_try_numcon\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
	breg = tbreg;
	hbreg = hreg;
	non_ftag_lpcreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_numcon,_trie_retry_numcon) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_retry_numcon\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) = (byte *) opfail;
	non_ftag_lpcreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_numcon,_trie_trust_numcon) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_trust_numcon\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);
	restore_trail_condition_registers(breg);
	non_ftag_lpcreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
XSB_End_Instr()

/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_numcon_succ,_trie_no_cp_numcon_succ)
	TRIE_R_LOCK();
	trie_instr_print(("trie_no_cp_numcon_succ\n"));
	NodePtr = (BTNptr) lpcreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
	proceed_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_try_numcon_succ,_trie_try_numcon_succ) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
	trie_instr_print(("trie_try_numcon_succ\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
	breg = tbreg;
	hbreg = hreg;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
	proceed_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_numcon_succ,_trie_retry_numcon_succ) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(( "trie_retry_numcon_succ"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) = (byte *) opfail;
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
	proceed_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_numcon_succ,_trie_trust_numcon_succ) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(( "trie_trust_numcon_succ\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);
	restore_trail_condition_registers(breg);
	unify_with_trie_numcon;
	trieinstr_unif_stkptr--;
	proceed_lpcreg;
XSB_End_Instr()

/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_var,_trie_no_cp_var)
	TRIE_R_LOCK();
        trie_instr_print(("trie_no_cp_var\n"));
	NodePtr = (BTNptr) lpcreg;
	trieinstr_vars_num = DecodeTrieVar(opatom);
	trieinstr_vars[trieinstr_vars_num] = (CPtr) *trieinstr_unif_stkptr;
#ifdef DEBUG_ASSERTIONS
        { int i = trieinstr_vars_num;
	  if ((isref(trieinstr_vars[i])) &&
            ((trieinstr_vars[i] < (CPtr)glstack.low) || (trieinstr_vars[i] >= hreg)) &&
	    ((trieinstr_vars[i] < top_of_localstk) || (trieinstr_vars[i] >= (CPtr) glstack.high))) {
	    xsb_dbgmsg((LOG_DEBUG, "tc_insts_xsb_i.h (no_cp): var reg assigned bad 0x%p %d 0x%p",
		       hreg, i, trieinstr_vars[i])); }
	} 
#endif
	trieinstr_unif_stkptr--;
	next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_try_var,_trie_try_var) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
        trie_instr_print(("trie_try_var\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
	/*	save_find_locx(ereg);*/
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
	breg = tbreg;
	hbreg = hreg;
	trieinstr_vars_num = DecodeTrieVar(opatom);
	trieinstr_vars[trieinstr_vars_num] = (CPtr) *trieinstr_unif_stkptr;
#ifdef DEBUG_ASSERTIONS
        { int i = trieinstr_vars_num;
	  if ((isref(trieinstr_vars[i])) &&
            ((trieinstr_vars[i] < (CPtr)glstack.low) || (trieinstr_vars[i] >= hreg)) &&
	    ((trieinstr_vars[i] < top_of_localstk) || (trieinstr_vars[i] >= (CPtr) glstack.high))) {
	    xsb_dbgmsg((LOG_DEBUG, "tc_insts_xsb_i.h (try): var reg assigned bad 0x%p %d 0x%p",
		       hreg, i, trieinstr_vars[i]));
	  }
	} 
#endif
	trieinstr_unif_stkptr--;
	next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_var,_trie_retry_var) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(( "trie_retry_var"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) = (byte *) opfail;
	trieinstr_vars_num = DecodeTrieVar(opatom);
	trieinstr_vars[trieinstr_vars_num] = (CPtr) *trieinstr_unif_stkptr;
#ifdef DEBUG_ASSERTIONS
        { int i = trieinstr_vars_num;
	  if ((isref(trieinstr_vars[i])) &&
            ((trieinstr_vars[i] < (CPtr)glstack.low) || (trieinstr_vars[i] >= hreg)) &&
	    ((trieinstr_vars[i] < top_of_localstk) || (trieinstr_vars[i] >= (CPtr) glstack.high))) {
	    xsb_dbgmsg((LOG_DEBUG, "tc_insts_xsb_i.h (retry): var reg assigned bad 0x%p %d 0x%p",
		       hreg, i, trieinstr_vars[i]);
	  }
	} 
#endif
	trieinstr_unif_stkptr--;
	next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_var,_trie_trust_var)  
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(( "trie_trust_var"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);	/* Remove this CP */
	restore_trail_condition_registers(breg);
	trieinstr_vars_num = DecodeTrieVar(opatom);
	trieinstr_vars[trieinstr_vars_num] = (CPtr) *trieinstr_unif_stkptr;
#ifdef DEBUG_ASSERTIONS
        { int i = trieinstr_vars_num;
	  if ((isref(trieinstr_vars[i])) &&
            ((trieinstr_vars[i] < (CPtr)glstack.low) || (trieinstr_vars[i] >= hreg)) &&
	    ((trieinstr_vars[i] < top_of_localstk) || (trieinstr_vars[i] >= (CPtr) glstack.high))) {
	     xsb_dbgmsg((LOG_DEBUG, "tc_insts_xsb_i.h (trust): var reg assigned bad 0x%p %d 0x%p",
			hreg, i, trieinstr_vars[i]));
	  }
	} 
#endif
	trieinstr_unif_stkptr--;
	next_lpcreg;
XSB_End_Instr()

/*----------------------------------------------------------------------*/

/* TLS: Trie val instructions were split up in Jan 11 to fix a bug in
   the handling of attributed variables.  A test case
   /attv_tests/attv_val_test.P was also added.

   Asserted, Interned, and (future) subsumptive tables use
   trie_xxx_val instructions, while vsriant tables may safely use
   variant_trie_xxx_val.

   In the general case, suppose we have a goal:

        when(nonvar(X),X \= 3),when(nonvar(Y),Y \= 1),q(X,Y)

   to an asserted trie with fact q(X,X).  In this case, we can
   represent the trie_xxx_val instruction as unifying call position 2
   with position 1.  Whenever call position 2 is an attv, and
   @position 2 is not already equal to @position1, we need to trigger
   an interrupt after unifying the values.

   (documenation I wrote a while ago -- I don't know if I believe it,
   but it argues for why trie_xxx_variant_val) does not need the
   interrupt mentioned above.

   In the case of variant tabels, the routine variant_answer_search()
   distinguishes between usage 1) an attv of the call that is
   unchanged by the answer; usage 2) an attv of the call that is
   changed to another attv in the answer; usage 3) a variable in the
   call that is bound to an attv; usage 4) a variable in the call that
   is bound to another variable in the call by a binding in the
   answer.  The routine variant_answer_search() generates trie_xxx_val
   instructions only usages 1 and 4 above.  In usages 2 and 3 a
   trie_xxx_attv instruction is generated.  Thus in variant tabling
   *trieinstr_unif_stkptr will dereference only to an attv iff
   trie_xxx_val dereferences to that same attv (case b.2 in the code
   below corresponding to usage 1 ); and trieinstr_unif_stkptr will
   dereference to a ref vanilla variable iff the associated symbol is
   a ref (case a corresponding to usage 4)
*/

XSB_Start_Instr(trie_no_cp_val,_trie_no_cp_val)
  Def2ops
  TRIE_R_LOCK();
 trie_instr_print(("trie_no_cp_val\n"));
  NodePtr = (BTNptr) lpcreg;
  unify_with_trie_val; 
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_try_val,_trie_try_val) 
  Def2ops
  CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
  TRIE_R_LOCK();
  trie_instr_print(("trie_try_val\n"));
  NodePtr = (BTNptr) lpcreg;
  save_find_locx(ereg);
  tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
  save_trie_registers(tbreg);
  save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
  breg = tbreg;
  hbreg = hreg;
  unify_with_trie_val;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_val,_trie_retry_val) 
  Def2ops
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(( "trie_retry_val\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  cp_pcreg(breg) = (byte *) opfail;
  unify_with_trie_val;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_val,_trie_trust_val) 
  Def2ops
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(( "trie_trust_val\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  breg = cp_prevbreg(breg);	/* Remove this CP */
  restore_trail_condition_registers(breg);
  unify_with_trie_val;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(variant_trie_no_cp_val,_variant_trie_no_cp_val)
  Def2ops
  TRIE_R_LOCK();
 trie_instr_print(("variant_trie_no_cp_val\n"));
//	printf("stk %x\n",trieinstr_unif_stkptr-trieinstr_unif_stk);
  NodePtr = (BTNptr) lpcreg;
  unify_with_variant_trie_val; 
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(variant_trie_try_val,_variant_trie_try_val) 
  Def2ops
  CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
  TRIE_R_LOCK();
  trie_instr_print(("vriant_trie_try_val\n"));
  NodePtr = (BTNptr) lpcreg;
  save_find_locx(ereg);
  tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
  save_trie_registers(tbreg);
  save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
  breg = tbreg;
  hbreg = hreg;
  unify_with_variant_trie_val;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(variant_trie_retry_val,_variant_trie_retry_val) 
  Def2ops
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(("trie_retry_val\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  cp_pcreg(breg) = (byte *) opfail;
  unify_with_variant_trie_val;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(variant_trie_trust_val,_variant_trie_trust_val) 
  Def2ops
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(("trie_trust_val\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  breg = cp_prevbreg(breg);	/* Remove this CP */
  restore_trail_condition_registers(breg);
  unify_with_variant_trie_val;
  next_lpcreg;
XSB_End_Instr()

/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_list,_trie_no_cp_list)
  TRIE_R_LOCK();
 trie_instr_print(("trie_no_cp_list\n"));
  NodePtr = (BTNptr) lpcreg;
  unify_with_trie_list;
  non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_try_list,_trie_try_list) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
	trie_instr_print(("trie_try_list\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
	breg = tbreg;
	hbreg = hreg;
	unify_with_trie_list;
	non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_list,_trie_retry_list) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_retry_list\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) = (byte *) opfail;
	unify_with_trie_list;
	non_ftag_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_list,_trie_trust_list) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_trust_list\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);	/* Remove this CP */
	restore_trail_condition_registers(breg);
	unify_with_trie_list;
	non_ftag_lpcreg;
XSB_End_Instr()

/*----------------------------------------------------------------------*/
/* fail insts for deleted nodes - reclaim deleted returns at completion */
/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_no_cp_fail,_trie_no_cp_fail)
  trie_instr_print(("trie_no_cp_fail\n"));
	lpcreg = (byte *) & fail_inst;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_fail,_trie_trust_fail) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_trust_fail\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	breg = cp_prevbreg(breg);	/* Remove this CP */
	restore_trail_condition_registers(breg);
	lpcreg = (byte *) & fail_inst;
XSB_End_Instr()

XSB_Start_Instr(trie_try_fail,_trie_try_fail) 
	CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	TRIE_R_LOCK();
	trie_instr_print(("trie_try_fail\n"));
	NodePtr = (BTNptr) lpcreg;
	save_find_locx(ereg);
	tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
	save_trie_registers(tbreg);
	save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
	breg = tbreg;
	hbreg = hreg;
	lpcreg = (byte *) & fail_inst;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_fail,_trie_retry_fail) 
	CPtr tbreg;
	TRIE_R_LOCK();
	trie_instr_print(("trie_retry_fail\n"));
	NodePtr = (BTNptr) lpcreg;
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE);
	cp_pcreg(breg) = (byte *) opfail;
	lpcreg = (byte *) & fail_inst;
XSB_End_Instr()

/*----------------------------------------------------------------------*/
/* The following code, that handles hashing in coded tries, has been	*/
/* modified for garbage collection.  Choice points for hashes in tries,	*/
/* besides the usual trie argument registers (see file tr_code.h), also	*/
/* contain 3 fields with certain information about the hash bucket.	*/
/* The first and third of these fields are predefined integers and are	*/
/* now encoded as such.  The second field contains a malloc-ed address	*/
/* which is encoded as a STRING (that's how addresses are encoded in	*/
/* XSB -- see file cell_xsb.h) to prevent garbage collection from       */
/* treating it as a reference either to a WAM stack or to a CHAT area.	*/
/*----------------------------------------------------------------------*/

/* Structure of the CPF created by hash_opcode:

             +-------------+
             |             |   LOW MEMORY
             |    Trail    |
             |             |
             |      |      |
             |      V      |
             |             |
             |             |
             |      ^      |
             |      |      |
             |             |
             |  CP Stack   |
             |             |
             |             |
             |=============|
             | Rest of CPF |--- Basic CPF (no argument registers)
             |-------------|
             | HASH index  | - last bucket explored
             |  ht header  | - ptr to HashTable Header structure
             | HASH_IS flag| - var/nonvar status of topmost term
             |-------------|    (the next to be unified with the trie)
             |     n+1     |_
             |trieinstr_unif_stk[n] | \
             |      .      |  |
             |      .      |  |- Subterms to be unified with trie
             |      .      |  |
             |trieinstr_unif_stk[0] |_/
             |-------------|
             |      m      |_
             | trieinstr_vars[m] | \
             |      .      |  |
             |      .      |  |- Variables encountered so far along trie path
             |      .      |  |   (m is -1 if no variables were encountered)
             | trieinstr_vars[0] |_/
             |=============|
             |      .      |
             |      .      |
             |      .      |    HIGH MEMORY
             +-------------+
*/

XSB_Start_Instr(hash_opcode,_hash_opcode) 
	CPtr tbreg, temp_ptr_for_hash;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
	BTHTptr hash_header;
	BTHTptr *hash_base;
	size_t hashed_hash_offset;

	trie_instr_print(("hash_opcode\n"));
   /*
    *  Under new trie structure, NodePtr is actually pointing at a
    *  Hash Table Header struct.
    */
    hash_header = (BTHTptr) lpcreg;
    hash_base = (BTHTptr *) BTHT_BucketArray(hash_header);

	temp_ptr_for_hash = (CPtr)*trieinstr_unif_stkptr;
        XSB_CptrDeref(temp_ptr_for_hash);
        if (!is_attv_or_ref(temp_ptr_for_hash)
	    && (*hash_base == NULL)){ 
            /* Ground call and no variables in hash table */
	    hash_nonvar_subterm(temp_ptr_for_hash,hash_header,
				hashed_hash_offset);
	    if(*(hash_base + hashed_hash_offset) == NULL)
	      /* fail to previous CPF */
	      lpcreg = (byte *) cp_pcreg(breg);
	    else
	      /* execute code of tries in this bucket */
	      lpcreg = (byte *) *(hash_base + hashed_hash_offset);
	}
        else {  
	  save_find_locx(ereg);
	  tbreg = top_of_cpstack;
#ifdef SLG_GC
	  old_cptop = tbreg;
#endif
	  save_trie_registers(tbreg);
	  if (is_attv_or_ref(temp_ptr_for_hash))
	    cell(--tbreg) = makeint(HASH_IS_FREE);
	  else 
	    cell(--tbreg) = makeint(HASH_IS_NOT_FREE);
    /*
     *  For normal trie nodes, this next CP value was given as the beginning
     *  of the hash table (bucket array).  With the new trie structure, I
     *  instead pass in the header, allowing access to all needed fields,
     *  including the bucket array.
     */
	  //	  printf("makeaddr: tc_insts_xsb_i.h: %p\n",hash_header); //dsw!!!
	  cell(--tbreg) = makeaddr(hash_header); /* BUT NOT A STRING */
	  cell(--tbreg) = makeint(FIRST_HASH_NODE);
	  save_choicepoint(tbreg,ereg,(byte *)&hash_handle_inst,breg);
#ifdef SLG_GC
	  cp_prevtop(tbreg) = old_cptop;
#endif
	  breg = tbreg;
	  hbreg = hreg;
	  lpcreg = (byte *) &hash_handle_inst;
	} 
XSB_End_Instr()

/*
 *  Since this instruction is called immediately after 'hash_opcode' and
 *  is also backtracked to while exploring a bucket chain, a mechanism is
 *  needed to distinguish between the two cases.  Hence the use of the
 *  FIRST_HASH_NODE flag in the CPS.
 */
XSB_Start_Instr(hash_handle,_hash_handle)
    CPtr    tbreg;
    BTHTptr hash_hdr, *hash_base;
    Integer     hash_offset, hashed_hash_offset;

    trie_instr_print(("hash_handle\n"));
    hash_offset = int_val(cell(breg+CP_SIZE));   // This may be number of bucket. */
    hash_hdr = (BTHTptr) string_val(cell(breg+CP_SIZE+1));
    hash_base = (BTHTptr *) BTHT_BucketArray(hash_hdr);
if ( int_val(cell(breg + CP_SIZE + 2)) == HASH_IS_NOT_FREE ) {
      /* Unify with nonvar */
      if ( (hash_offset != FIRST_HASH_NODE) &&
	   (hash_offset != NO_MORE_IN_HASH) ) {
	tbreg = breg;
	restore_regs_and_vars(tbreg, CP_SIZE+3);
      }
      XSB_Deref(*trieinstr_unif_stkptr);
#ifdef NON_OPT_COMPILE
      if (isref(*trieinstr_unif_stkptr))   /* sanity check */
	xsb_exit("error_condition in hash_handle\n");
#endif
      hash_nonvar_subterm(*trieinstr_unif_stkptr,hash_hdr,hashed_hash_offset);
      if (hash_offset == FIRST_HASH_NODE) {
	if (*hash_base == NULL) { /* No Variables in hash table */
	  breg = cp_prevbreg(breg);   /* dealloc this CPF */
	  if(*(hash_base + hashed_hash_offset) == NULL)
	    /* fail to previous CPF */
	    lpcreg = (byte *) cp_pcreg(breg);
	  else
	    /* execute code of tries in this bucket */
	    lpcreg = (byte *) *(hash_base + hashed_hash_offset);
	}
	else {   /* save hashed-to bucket, explore bucket 0 */
	  if ( (*(hash_base + hashed_hash_offset) == NULL) ||
	       (hashed_hash_offset == 0) )
	    breg = cp_prevbreg(breg);   /* dealloc this CPF */
	  else
	    cell(breg + CP_SIZE) = makeint(hashed_hash_offset);  /* Thus backtrack into matching con/sym after backtracking thru vars */
	  lpcreg = (byte *) *hash_base;
	}
      }
      else if (hash_offset == hashed_hash_offset) {   /* TLS: probably could opt this "if" away */
	/* explore hashed-to bucket */
	lpcreg = (byte *)*(hash_base + hash_offset);
	breg = cp_prevbreg(breg);
      }
      else {
	xsb_error("Hash Offset %d, HHO %d",
		  hash_offset, hashed_hash_offset);
	xsb_exit( "error_condition in hash_handle\n");
      }
 }
 else {  /* unification of trie with variable term */
   find_next_nonempty_bucket(hash_hdr,hash_base,hash_offset);
   if (hash_offset == NO_MORE_IN_HASH) {
     breg = cp_prevbreg(breg);
     lpcreg = (byte *) cp_pcreg(breg);
   }
   else {
     if ( int_val(cell(breg+CP_SIZE)) != FIRST_HASH_NODE ) {
       tbreg = breg;
       restore_regs_and_vars(tbreg, CP_SIZE+3);
     }
     lpcreg = (byte *) *(hash_base + hash_offset);
     cell(breg+CP_SIZE) = makeint(hash_offset);
   }
 }
XSB_End_Instr()

/*----------------------------------------------------------------------*/

XSB_Start_Instr(trie_proceed,_trie_proceed)	/* This is essentially a "proceed" */
  trie_instr_print(("trie_proceed\n"));
	NodePtr = (BTNptr) lpcreg;
	proceed_lpcreg;
XSB_End_Instr()

/* For non-incremental answer tables a no-op; begin processing with
   child.  For incremental tables, create a choice point that will
   increment and decrement the visitors number.  Keep this number on
   the choice point tagged as an int so that GC is not affected. */

XSB_Start_Instr(trie_root,_trie_root)      
  CPtr    tbreg;
#ifdef SLG_GC
 CPtr old_cptop;
#endif
 TRIE_R_LOCK()
 NodePtr = (BTNptr) lpcreg;
 lpcreg = (byte *) BTN_Child(NodePtr);
 trie_instr_print(("trie_root %d %p\n",trieinstr_vars_num,NodePtr));
 // printf("stack bottom %x @ %x\n",trieinstr_unif_stk[0],*(CPtr)trieinstr_unif_stk[0]);
 // if (BTN_Parent(NodePtr)) { 
 //   printf("trie root NP %p for %p visitors %d",NodePtr,BTN_Parent(NodePtr),subg_visitors(BTN_Parent(NodePtr))+1);print_subgoal(stddbg,(VariantSF) BTN_Parent(NodePtr));
 //  printf("\n");
 // else printf("trie root NP %p called with 0 nodeptr\n",NodePtr);
#ifdef COUNT_TRIE_VISITORS
 if (BTN_Parent(NodePtr)  && IsIncrSF(BTN_Parent(NodePtr)) ) {  
   trie_instr_print(("trie_root %d %p parent%p\n",trieinstr_vars_num,NodePtr,BTN_Parent(NodePtr)));
   subg_visitors(BTN_Parent(NodePtr)) =    subg_visitors(BTN_Parent(NodePtr)) + 1;
   save_find_locx(ereg);
   tbreg = top_of_cpstack;
#ifdef SLG_GC
   old_cptop = tbreg;	
#endif
   //   save_trie_registers(tbreg);         // the following macros are a slight change from save_trie_registers, 
   save_trieinstr_vars(tbreg);              // to handle case of uninitialized trieinstr_unif_stk.
   if (trieinstr_unif_stk) 
     save_trieinstr_unif_stk(tbreg);
   //   *(--tbreg) = (Cell) makeint(NodePtr);
   *(--tbreg) = (Cell) NodePtr;
   save_choicepoint(tbreg,ereg,(byte *)&trie_fail_inst,breg);  
#ifdef SLG_GC
   cp_prevtop(tbreg) = old_cptop;
#endif
   breg = tbreg;
   hbreg = hreg;
 }
#endif
XSB_End_Instr()

  /* If the way info on the CP stack changes, make sure to change  get_psc_for_trie_fail() in tr_utils.c */
XSB_Start_Instr(trie_fail,_trie_fail)
  CPtr tbreg;
 TRIE_R_UNLOCK()
 tbreg = breg;
 // NodePtr = (BTNptr) int_val((*(tbreg + CP_SIZE)));
 NodePtr = (BTNptr) *(tbreg + CP_SIZE);
 trie_instr_print(("trie_fail %d %p\n",NodePtr));
 //    restore_regs_and_vars(tbreg, CP_SIZE);
 breg = cp_prevbreg(breg);       /* Remove this CP */
 //    restore_trail_condition_registers(breg);
 subg_visitors(BTN_Parent(NodePtr)) =    subg_visitors(BTN_Parent(NodePtr)) - 1;
 // if (BTN_Parent(NodePtr)) { 
 //   printf("trie fail for %p ",BTN_Parent(NodePtr) );print_subgoal(stddbg,(VariantSF) BTN_Parent(NodePtr));printf(" visitors %d\n", subg_visitors(BTN_Parent(NodePtr)));}
 lpcreg = (byte *) & fail_inst;
XSB_End_Instr()

/*
 * This is the embedded-trie instruction which is placed in the root of
 * asserted tries.  It looks a lot like both "return_table_code", which
 * prepares the engine to walk an answer trie, and "get_calls", which
 * prepares the engine to walk a call trie.  Maybe there's a way to
 * "unify" these operations now that all tries contain root nodes.
 */
XSB_Start_Instr(trie_assert_inst,_trie_assert_inst)
  Psc psc_ptr;
  int i;
#ifdef MULTI_THREAD_RWL
  CPtr tbreg;
#ifdef SLG_GC
  CPtr old_cptop;
#endif
#endif

  trie_instr_print(("trie_assert\n"));
  NodePtr = (BTNptr) lpcreg;
  if (Child(NodePtr) != NULL) {
    TRIE_R_LOCK()
    psc_ptr = DecodeTrieFunctor(BTN_Symbol(NodePtr));
    trieinstr_unif_stkptr = trieinstr_unif_stk -1;
    trieinstr_vars_num = -1;
    for (i = get_arity(psc_ptr); i >= 1; i--) { push_trieinstr_unif_stk(*(rreg+i)); }
    lpcreg = (byte *) Child(NodePtr);
#ifdef MULTI_THREAD_RWL
/* save choice point for trie_unlock instruction */
    save_find_locx(ereg);
    tbreg = top_of_cpstack;
#ifdef SLG_GC
    old_cptop = tbreg;
#endif
    save_choicepoint(tbreg,ereg,(byte *)&trie_fail_unlock_inst,breg);
#ifdef SLG_GC
    cp_prevtop(tbreg) = old_cptop;
#endif
    breg = tbreg;
    hbreg = hreg;
#endif
  }
  else
    lpcreg = (byte *) &fail_inst;
XSB_End_Instr()

XSB_Start_Instr(trie_no_cp_attv,_trie_no_cp_attv)
  TRIE_R_LOCK();
 trie_instr_print(("trie_no_cp_attv\n"));
  NodePtr = (BTNptr) lpcreg;
  unify_with_trie_attv;
  next_lpcreg
XSB_End_Instr()

XSB_Start_Instr(trie_try_attv,_trie_try_attv)
  CPtr tbreg;
#ifdef SLG_GC
	CPtr old_cptop;
#endif
  TRIE_R_LOCK();
  trie_instr_print(("trie_try_attv\n"));
  NodePtr = (BTNptr) lpcreg;
  save_find_locx(ereg);
  tbreg = top_of_cpstack;
#ifdef SLG_GC
	old_cptop = tbreg;
#endif
  save_trie_registers(tbreg);
  save_choicepoint(tbreg,ereg,(byte *)opfail,breg);
#ifdef SLG_GC
	cp_prevtop(tbreg) = old_cptop;
#endif
  breg = tbreg;
  hbreg = hreg;
  unify_with_trie_attv;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_retry_attv,_trie_retry_attv) 
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(("trie_retry_attv\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  cp_pcreg(breg) = (byte *) opfail;
  unify_with_trie_attv;
  next_lpcreg;
XSB_End_Instr()

XSB_Start_Instr(trie_trust_attv,_trie_trust_attv) 
  CPtr tbreg;
  TRIE_R_LOCK();
  trie_instr_print(("trie_trust_attv\n"));
  NodePtr = (BTNptr) lpcreg;
  tbreg = breg;
  restore_regs_and_vars(tbreg, CP_SIZE);
  breg = cp_prevbreg(breg);	/* Remove this CP */
  restore_trail_condition_registers(breg);
  unify_with_trie_attv;
  next_lpcreg;
XSB_End_Instr()

 /* A completed_trie_member instruction has, on top of the choice
    point 1) the ans_sf_length; 2) the ans_sf itself; 3) the pointer to
    the list of answer substitutions in the heap (listHead). 

    The list through which backtracking is performed contains elements of
    the form: ret(ret(<ans_subst>),unconditional_flag) where
    unconditional_flag is 1 if the answer is unconditional and contains a
    pointer to the subgoal frame of brat_undefined otherwise. */

XSB_Start_Instr(completed_trie_member,_completed_trie_member) 
  Cell listHead, unconditional_flag; int i, ans_sf_length; CPtr this_ret; CPtr ans_sf; CPtr xtemp1;

// printf("\nin completed_trie_member hreg %p hfreg %p\n",hreg,hfreg);  //print_n_registers(stddbg, 6 , 8);printf("\n");
  ans_sf_length = (int)int_val(cell(breg + CP_SIZE));
  //  printf("ans_sf_length %d\n",ans_sf_length);
  listHead = * (breg + CP_SIZE + ans_sf_length+1);
  this_ret = (CPtr) follow(clref_val(listHead));      // pointer to beginning of ret(ret/n,uncond_flag).
  unconditional_flag = cell(clref_val(this_ret)+2);
  this_ret = (CPtr) follow((clref_val(this_ret)+1));  // pointer to beginning of ret/n term
  //  printterm(stddbg,listHead,8);printf("\n");	
  //  printf("...");printterm(stddbg,(Cell) this_ret,8);printf("\n");
  undo_bindings(breg);		                      // will not undo the list itself.
  delayreg = cp_pdreg(breg);                
  restore_some_wamregs(breg, ereg);	       
  //  printf("begin ");print_n_registers(stddbg, 1 , 8);printf("\n");
  if (int_val(unconditional_flag) != (Integer) TRUE) {
    //    printf("conditional %x\n",int_val(unconditional_flag)); 
    delay_negatively(int_val(unconditional_flag));
  }
  ans_sf = breg + CP_SIZE + ans_sf_length + 1;  // points just below index starting with 1.
  for (i=1; i <= ans_sf_length; i++) {
    //      printf("lh %x clr %p *clr %x\n",listHead,clref_val(listHead),((CPtr) *clref_val(listHead)) +i);      
    //    printf("   ");printterm(stddbg,(Cell) (clref_val(this_ret)+i),8);printf("\n");
    xtemp1 = (ans_sf - i);
    XSB_CptrDeref(xtemp1);
    if (isattv(xtemp1)) {
      printf(" found attv in ISO incremental tabling\n");
      xtemp1 = (CPtr)dec_addr(xtemp1);
      XSB_CptrDeref(xtemp1);
      add_interrupt(CTXTc cell(((CPtr)dec_addr(xtemp1) + 1)),(Cell) (clref_val(this_ret)+i));	
      dbind_ref(xtemp1,  (clref_val(this_ret)+i));			
      //            unify((Cell) xtemp1,(Cell) (clref_val(this_ret)+i));
      //      unify((Cell) (ans_sf-i),(Cell) (clref_val(this_ret)+i));
      //      printf("......");printterm(stddbg,(Cell) (xtemp1),8);printf("\n");
    }
    else
      dbind_ref(xtemp1, clref_val(this_ret)+i);
    //      ctop_tag(CTXTc 2+i, clref_val(*clref_val(listHead))+i);
  }
  //  printf("hreg %x hfreg %x\n",hreg,hfreg);
  if (isnil(*(clref_val(listHead)+1))) {
    hfreg = (CPtr) * (breg + CP_SIZE + 2+ ans_sf_length);     // Need to reset, as find_the_visitors increased hfreg to protect list.
    breg = cp_prevbreg(breg);	/* Remove this CP */
  }
  else {  /* Point to CDR */
    * (breg + CP_SIZE + ans_sf_length +1) = *(clref_val(listHead) + 1);
    //    printf("reset : ");    printterm(stddbg,(Cell) (clref_val(listHead) + 1),8); printf("\n");
    flags[PIL_TRACE] = 1;
  }
  //  printf("end ");print_n_registers(stddbg, 6 , 8);printf("\n");
  lpcreg = cpreg;			       
  if (attv_pending_interrupts) {					
    printf("pending interrupts  \n");        // TLS: Fix -- find out the right number of registers
    allocate_env_and_call_check_ints(7,100);	
    //    printf("finished interrupts\n");
  }								
XSB_End_Instr()

/*----------------------------------------------------------------------*/
