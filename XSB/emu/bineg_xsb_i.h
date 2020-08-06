/* File:      bineg_xsb_i.h
** Author(s): Kostis Sagonas, Baoqiu Cui
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
** $Id: bineg_xsb_i.h,v 1.53 2013-04-17 22:02:34 tswift Exp $
** 
*/


/* special debug includes */
#include "debugs/debug_delay.h"

/*----------------------------------------------------------------------
Contains builtin predicates for SLG negation (and tfindall/3) using
call-variance.  Well-founded negation with call-subsumption is handled
by CALL_SUBS_SLG_NOT, curently in tr_utils.c
----------------------------------------------------------------------*/

case SLG_NOT: {

#ifdef DEBUG_ASSERTIONS
  const int Arity = 1;
#endif
  const int regSF = 1;  /* in: variant tabled subgoal frame */
  VariantSF sf;

  sf = ptoc_addr(regSF);
#ifdef DEBUG_ASSERTIONS
  /* Need to change for MT: smVarSF can be private or shared
|  if ( ! smIsValidStructRef(smVarSF,sf) )
|    xsb_abort("Invalid Table Entry Handle\n\t Argument %d of %s/%d",
|	      regSF, BuiltinName(SLG_NOT), Arity);
  */
#endif
  if ( has_no_answers(sf) &&
       (is_completed(sf) || neg_delay == FALSE) )
    return TRUE;
  if ( varsf_has_unconditional_answers(sf) )
    return FALSE;
  else {
    if (flags[CTRACE_CALLS] && !subg_forest_log_off(sf))  {				
      sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF) sf);	      
      sprint_subgoal(CTXTc forest_log_buffer_2,0,(VariantSF)ptcpreg);	 
      fprintf(fview_ptr,"dly(%s,%s,%d).\n",
	      forest_log_buffer_1->fl_buffer,forest_log_buffer_2->fl_buffer,
	      ctrace_ctr++); 
    }
    delay_negatively(sf);
    return TRUE;
  }
}

/*----------------------------------------------------------------------*/

/* This case is obsolescent.  Leaving it around for a while just in
   case there's a problem with well-founded call subsumption, but
   let's hope it can be removed */

case LRD_SUCCESS: {

#ifdef DEBUG_ASSERTIONS
  const int Arity = 2;
#endif
  const int regProducerSF = 1;  /* in: subsumptive producer consumed from */
  const int regNegSubgoal = 2;  /* in: negated subgoal for error reporting */
  SubProdSF sf;

  sf = (SubProdSF)ptoc_addr(regProducerSF);
#ifdef DEBUG_ASSERTIONS
  if ( ! smIsValidStructRef(smProdSF,sf) )
    xsb_abort("Invalid Table Entry Handle\n\t Argument %d of %s/%d",
	      regProducerSF, BuiltinName(SLG_NOT), Arity);
#endif
  if ( is_completed(sf) || neg_delay == FALSE )
    return TRUE;
  else {
    /*
     * Using a static here prevents multiple buffers from being left
     * dangling when executed multiple times, as no opportunity exists
     * to reclaim the string after use in xsb_abort().
     */
    static XSB_StrDefine(vsNegSubgoal);
    XSB_StrSet(&vsNegSubgoal,"");
    print_pterm(CTXTc ptoc_tag(CTXTc regNegSubgoal),1,&vsNegSubgoal);
    xsb_abort("Illegal table operation\n\t Attempted DELAY of negative "
	      "subsumptive literal: %s", vsNegSubgoal.string);
  }
}

/*----------------------------------------------------------------------*/

case IS_INCOMPLETE: {

#ifdef DEBUG_ASSERTIONS
  const int Arity = 2;
#endif
  const int regSubgoalFrame = 1;  /* in: rep of a tabled subgoal */
  const int regRootSubgoal  = 2;  /* in: PTCPreg */

#ifdef SHARED_COMPL_TABLES
	Integer table_tid ;
	th_context *waiting_for_thread ;
	int table_is_shared ;
#endif

  VariantSF producerSF = ptoc_addr(regSubgoalFrame);
  CPtr t_ptcp = ptoc_addr(regRootSubgoal);
#ifdef DEBUG_ASSERTIONS
  /* Need to change for MT: smVarSF can be private or shared
|  if ( ! smIsValidStructRef(smVarSF,producerSF) &&
|       ! smIsValidStructRef(smProdSF,producerSF) )
|    xsb_abort("Invalid Table Entry Handle\n\t Argument %d of %s/%d",
|	      regSubgoalFrame, BuiltinName(IS_INCOMPLETE), Arity);
  */
#endif
  xsb_dbgmsg((LOG_DELAY, "Is incomplete for "));
  dbg_print_subgoal(LOG_DELAY, stddbg, producerSF);
  xsb_dbgmsg((LOG_DELAY, ", (%x)\n", (int)&subg_ans_root_ptr(producerSF)));

#ifdef SHARED_COMPL_TABLES
/* This allows sharing of completed tables.  */
     table_is_shared = IsSharedSF(producerSF);
     if( table_is_shared && !is_completed(producerSF) && 
         subg_tid(producerSF) != (Thread_T) xsb_thread_id )
     {	pthread_mutex_lock(&completing_mut);
     	SYS_MUTEX_INCR( MUTEX_COMPL );
        while( table_is_shared && !is_completed(producerSF) )
        {
	  table_tid = (Integer) subg_tid(producerSF) ;
	  waiting_for_thread = find_context(table_tid) ;
           if( would_deadlock( (Integer) table_tid, xsb_thread_id ) )
	   {	/* code for leader */
                reset_other_threads( th, waiting_for_thread, producerSF );
		reset_leader( th ) ; /* this unlocks the completing_mut */
		return TRUE ;
	   }
	   th->waiting_for_subgoal = producerSF ;
           th->waiting_for_tid = (Integer) table_tid ;
           pthread_cond_wait(&TIF_ComplCond(subg_tif_ptr(producerSF)),
                             &completing_mut) ;
	   SYS_MUTEX_INCR( MUTEX_COMPL );
        }
        /* the thread was reset after being suspended */
        th->waiting_for_tid = -1 ;
        th->waiting_for_subgoal = NULL ;
     	pthread_mutex_unlock(&completing_mut) ;
	return TRUE ;
     }
#endif

  if (is_completed(producerSF)) {
    neg_delay = FALSE;
    ptcpreg = t_ptcp;  /* restore ptcpreg as the compl. suspens. would */
    return TRUE;	/* succeed */
  }
  else {	/* subgoal is not completed; save a completion suspension */
#ifdef SLG_GC
    CPtr old_cptop;
#endif

    xsb_dbgmsg((LOG_DELAY, "... Saving a completion suspension (~"));
    dbg_print_subgoal(LOG_DELAY, stddbg, producerSF);
    xsb_dbgmsg((LOG_DELAY, " in the body of "));
    dbg_print_subgoal(LOG_DELAY, stddbg, (VariantSF)t_ptcp);
    xsb_dbgmsg((LOG_DELAY,"an UNTABLED predicate"));
    xsb_dbgmsg((LOG_DELAY, ")\n"));

    check_tcpstack_overflow;

    adjust_level(subg_compl_stack_ptr(producerSF));
    save_find_locx(ereg);
    efreg = ebreg;
    if (trreg > trfreg) trfreg = trreg;
    if (hfreg < hreg) hfreg = hreg;
    if (bfreg > breg) bfreg = breg;

#ifdef SLG_GC
    old_cptop = bfreg;
#endif
    save_compl_susp_frame(bfreg, ereg, (CPtr)producerSF, t_ptcp, cpreg);
#ifdef SLG_GC
    csf_prevtop(bfreg) = old_cptop;
#endif
    subg_compl_susp_ptr(producerSF) = bfreg;
    return FALSE;
  }
}

/*----------------------------------------------------------------------*/

    case GET_PTCP:
      ctop_int(CTXTc 1, (Integer)ptcpreg);
      break;

/*----------------------------------------------------------------------*/

    case GET_DELAY_LISTS: {
      /*
       * When GET_DELAY_LISTS is called, we can assume that the
       * corresponding tabled subgoal call has been completed and so trie
       * code will be used to return the answer (see
       * trie_get_return()).  After the execution of trie code,
       * trieinstr_vars[] contains the substitution factor of the _answer_ to
       * the call.
       */
      DL dl;
      DE de;
      BTNptr as_leaf;
      CPtr delay_lists;
      Cell dls_head;
	
#ifdef DEBUG_DELAYVAR
      {	int i;
	for (i = 0; i <= global_trieinstr_vars_num; i++) {
	  Cell x;
	  fprintf(stddbg, ">>>> trieinstr_vars[%d] =", i);
	  x = (Cell)trieinstr_vars[i];
	  XSB_Deref(x);
	  printterm(stddbg, x, 25);	  fprintf(stddbg, "\n");
	}
      }
#endif /* DEBUG_DELAYVAR */
      as_leaf = (NODEptr) ptoc_int(CTXTc 1);
      delay_lists = (CPtr)ptoc_tag(CTXTc 2); // should test if var
      if (is_conditional_answer(as_leaf)) {
	int i;	//	int var_addr_accum_arraysz;
	int num_vars_in_anshead = global_trieinstr_vars_num+1;
	{ /*
	   * Initialize var_addr_accum with trieinstr_vars & global_trieinstr_vars_num (after get_returns,
	   * which calls trie_get_return).  (global_trieinstr_vars_num +
	   * 1) is the number of variables left in the answer
	   * (substitution factor of the answer)
	   *
	   * So, var_addr_accum[] starts as the substitution factor of the
	   * answer for the head predicate.  It will accumulate the variables in 
	   * body of the residual rule.
	   */
	  if (var_addr_arraysz < num_vars_in_anshead) 
	    trie_expand_array(CPtr,var_addr,var_addr_arraysz,num_vars_in_anshead,"var_addr");
	  if (var_addr_accum_arraysz < num_vars_in_anshead) 
	    trie_expand_array(CPtr,var_addr_accum,var_addr_accum_arraysz,num_vars_in_anshead,"var_addr_accum");
	  for( i = 0; i < num_vars_in_anshead; i++)
	    var_addr_accum[i] = trieinstr_vars[i];
	  //	  printf("GET_DELAY_LISTS end of setup cova_sz %d num_vars_in_anshead %d\n",
	  //	 var_addr_accum_arraysz,num_vars_in_anshead);
	  //	  var_addr_accum_arraysz = var_addr_arraysz;
	  //	  var_addr_accum = (CPtr *)mem_calloc(var_addr_accum_arraysz, sizeof(CPtr),OTHER_SPACE);
	  
	  /* var_addr_accum[] will accumulate the variables across
	     the delay-elements (i.e., body subgoals in the residual
	     rule.)  var_addr_accum_num will accumulate the
	     max number of vars in var_addr_accum so we can reset
	     them to 0.  The entries in var_addr_accum[], other than
	     for the answer variables, must be 0, since the 1st
	     occurrence mark for variables in the delay list cant be
	     depended on, due to possible deletion of
	     delay-elements. (TES, not sure I understand this last part*/
	  var_addr_accum_num = num_vars_in_anshead;
	}

	for (dl = asi_dl_list((ASI) Delay(as_leaf)); dl != NULL; ) {
	  de = dl_de_list(dl);

	  xsb_dbgmsg((LOG_DELAY, "orig_delayed_term("));
	  dbg_print_subgoal(LOG_DELAY, stddbg, de_subgoal(de)); xsb_dbgmsg((LOG_DELAY, ").\n"));

	  //	  printf("2) accum_nhtv %d num_vars_in_anshead %d va_size %d cova_size %d\n",
	  //	 var_addr_accum_num,num_vars_in_anshead,var_addr_arraysz,var_addr_accum_arraysz);
	  dls_head = build_delay_list(CTXTc de);
	  //	  printf("3) accum_nhtv %d cva_size %d num_vars_in_anshead %d size %d\n",
	  //	 var_addr_accum_num,var_addr_accum_arraysz,num_vars_in_anshead,var_addr_arraysz);

	  /* Answer may have more than one delay list, so must re-init
	     rest of vars used in var_addr_accum back to 0; head vars are unchanged. */

	  if (var_addr_accum) {
	    while (var_addr_accum_num > num_vars_in_anshead 
		   && var_addr_accum_num <= var_addr_accum_arraysz) {
	      //	    printf("cnhtv %d nvia %d\n",var_addr_accum_num,num_vars_in_anshead);
	      var_addr_accum[--var_addr_accum_num] = 0;
	    }
	  }

	  bind_list(delay_lists,hreg);
	  follow(hreg) = dls_head;
	  delay_lists = hreg + 1;
	  hreg += 2;
	  dl = dl_next(dl);
	}
	bind_nil(delay_lists);
	//	mem_dealloc(var_addr_accum,var_addr_accum_arraysz*sizeof(CPtr),OTHER_SPACE);
      } else {
	bind_nil((CPtr)delay_lists);
      }
      break;
    }

/*----------------------------------------------------------------------*/

