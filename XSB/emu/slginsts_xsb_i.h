/* File:      slginsts_xsb_i.h
** Author(s): Swift, Rao, Sagonas, Freire, Cui, Johnson
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
** $Id: slginsts_xsb_i.h,v 1.92 2010/07/29 19:57:47 tswift Exp $
** 
*/


/* special debug includes */
#include "debugs/debug_delay.h"

#define ARITY	op1	/* register Cell */
#define Yn	op2	/* register Cell */
#define LABEL	op3	/* CPtr */

/*-------------------------------------------------------------------------*/

/*
 *  Organization of Tabling Choice Points:
 *
 *             +-------------+
 *             |             |   LOW MEMORY
 *             |    Trail    |
 *             |             |
 *             |      |      |
 *             |      V      |
 *             |             |
 *             |             |
 *             |      ^      |
 *             |      |      |
 *             |             |
 *             |  CP Stack   |
 *             |             |
 *             |             |
 *             |=============|
 *             | Rest of CPF |--- Different for Generator and Consumer
 *             |-   -   -   -|_
 *             |   INT: m    | \
 *             |   Term-m    |  |
 *             |      .      |  |- Answer Template (Note that this is overritten by
 *             |      .      |  |  Generator or consumer CPS after the A.T. is copied 
 *             |      .      |  |  to the heap).
 *             |   Term-1    |_/
 *             |=============|
 *             |      .      |
 *             |      .      |
 *             |      .      |    HIGH MEMORY
 *             +-------------+
 *
 *
 *  Answer Templates are stored in the Heap:
 *
 *             +-------------+
 *             |      .      |   LOW MEMORY
 *             |      .      |
 *             |      .      |
 *             |-------------|_
 *             |   Term-m    | \
 *             |      .      |  |
 *             |      .      |  |- Answer Template
 *             |      .      |  |
 *             |   Term-1    |  |
 *             |   INT: m    |_/
 *             |-------------|
 *             |             |
 *             |    Heap     |
 *             |             |
 *             |      |      |
 *             |      V      |
 *             |             |
 *             |             |
 *             |      ^      |
 *             |      |      |
 *             |             |
 *             |    Local    |
 *             |             |    HIGH MEMORY
 *             +-------------+
 */

/*-------------------------------------------------------------------------*/

#ifdef MULTI_THREAD

#define TABLE_IS_SHARED()	(threads_current_sm == SHARED_SM)

#define LOCK_CALL_TRIE()						\
{	if( TABLE_IS_SHARED() )						\
        {	pthread_mutex_lock( &TIF_CALL_TRIE_LOCK(tip) );		\
		SYS_MUTEX_INCR( MUTEX_CALL_TRIE )			\
}	}

#define UNLOCK_CALL_TRIE()						\
{	if( TABLE_IS_SHARED() )						\
		pthread_mutex_unlock( &TIF_CALL_TRIE_LOCK(tip) );	\
}

#else
#define LOCK_CALL_TRIE()
#define UNLOCK_CALL_TRIE()
#endif

#define  LOG_TABLE_CALL(state)						\
  /*  printf("ctrace %d forest_log_off %d\n",flags[CTRACE_CALLS],!subg_forest_log_off(producer_sf)); */	\
  if (flags[CTRACE_CALLS] && !subg_forest_log_off(producer_sf))  {		\
    sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)producer_sf);	\
    if (ptcpreg) {							\
      sprint_subgoal(CTXTc forest_log_buffer_2,0,(VariantSF)ptcpreg);	\
    }									\
    else sprintf(forest_log_buffer_2->fl_buffer,"null");		\
    if (is_neg_call)							\
      fprintf(fview_ptr,"nc(%s,%s,%s,%d).\n",forest_log_buffer_1->fl_buffer, \
	      forest_log_buffer_2->fl_buffer,state,ctrace_ctr++);	\
    else {								\
      fprintf(fview_ptr,"tc(%s,%s,%s,%d).\n",forest_log_buffer_1->fl_buffer, 	\
	      forest_log_buffer_2->fl_buffer,state,ctrace_ctr++); }	\
  }

#define LOG_ANSWER_RETURN(answer,template_ptr)				\
  if (flags[CTRACE_CALLS] > 1 && !subg_forest_log_off(consumer_sf))  {			\
    sprintAnswerTemplate(CTXTc forest_log_buffer_1,template_ptr, template_size); \
    sprint_subgoal(CTXTc forest_log_buffer_2,0,(VariantSF)consumer_sf);	\
    sprint_subgoal(CTXTc forest_log_buffer_3,0,(VariantSF)ptcpreg); \
    if (is_conditional_answer(answer))					\
      fprintf(fview_ptr,"dar(%s,%s,%s,%d).\n",forest_log_buffer_1->fl_buffer, \
	      forest_log_buffer_2->fl_buffer,forest_log_buffer_3->fl_buffer, \
	      ctrace_ctr++);						\
    else								\
      fprintf(fview_ptr,"ar(%s,%s,%s,%d).\n",forest_log_buffer_1->fl_buffer, \
	      forest_log_buffer_2->fl_buffer,forest_log_buffer_3->fl_buffer, \
	      ctrace_ctr++);						\
  }

/* need to test if ground (or better, but harder, if goals are
   identical) since p(X) :- p(X) is different from p(X) :- p(Y), yet
   they have the same goals. */
#define fail_if_direct_recursion(SUBGOAL)				\
  do {									\
    if (SUBGOAL == (VariantSF)ptcpreg					\
	&& is_ground_subgoal(SUBGOAL) ) {				\
      Fail1;								\
      XSB_Next_Instr();							\
    }									\
    } while (0)

#include "complete_xsb_i.h"

/*
 *  Instruction format:
 *    1st word: opcode X X pred_arity
 *    2nd word: pred_first_clause_label
 *    3rd word: preds_TableInfo_record
 */

XSB_Start_Instr_Chained(tabletry,_tabletry)
XSB_Start_Instr(tabletrysingle,_tabletrysingle) 
  DefOps13
  /*
   *  Retrieve instruction arguments and test the system stacks for
   *  overflow.  The local PCreg, "lpcreg", is incremented to point to
   *  the instruction to be executed should this one fail.
   */
  byte this_instr = *lpcreg;
  byte *continuation;
  TabledCallInfo callInfo;
  CallLookupResults lookupResults;
  VariantSF producer_sf, consumer_sf;
  CPtr answer_template_cps, answer_template_heap;
  int template_size, attv_num, is_neg_call; Integer tmp;
  TIFptr tip;
  int ret;
  int parent_table_is_incr = 0; /* for incremental evaluation */
  VariantSF parent_table_sf=NULL; /* used for creating call graph */
#ifdef CALL_ABSTRACTION
  int abstr_size;
  UNUSED(abstr_size);
#endif

  UNUSED(op1);

  //  printf("starting breg is %x %x\n",breg,((pb)tcpstack.high - (pb)breg));

#ifdef SHARED_COMPL_TABLES
  byte *inst_addr = lpcreg;
  Integer table_tid ;
  int grabbed = FALSE;
  th_context * waiting_for_thread;
#endif
#ifdef MULTI_THREAD_RWL
  CPtr tbreg;
#ifdef SLG_GC
  CPtr old_cptop;
#endif
#endif

  old_call_gl=NULL;   /* incremental evaluation */

  xwammode = 1;
  CallInfo_Arguments(callInfo) = reg + 1;
  CallInfo_CallArity(callInfo) = get_xxa; 
  LABEL = (CPtr)((byte *) get_xxxl);  
  Op1(get_xxxxl);
  tip =  (TIFptr) get_xxxxl;
  SET_TRIE_ALLOCATION_TYPE_TIP(tip); /* No-op in seq engine */
#ifdef MULTI_THREAD
  handle_dispatch_block(tip);
#endif
  CallInfo_TableInfo(callInfo) = tip;
  ADVANCE_PC(size_xxxXX);

#ifdef DEBUG_ABSTRACTION
  printf("tabletry -- ");
  print_registers(stddbg,TIF_PSC(tip),20); 
  printf("\n");
#endif

  check_tcpstack_overflow;
  CallInfo_AnsTempl(callInfo) = top_of_cpstack;

  if ( this_instr == tabletry ) {
    /* lpcreg was left pointing to the next clause, e.g. tableretry */
    continuation = lpcreg;
  }
  else 
    continuation = (pb) &check_complete_inst;

  check_glstack_overflow(CallInfo_CallArity(callInfo),lpcreg,OVERFLOW_MARGIN);

  LOCK_CALL_TRIE();
  /*
   *  Perform a call-check/insert operation on the current call.  The
   *  subterms of this call which form the answer template are
   *  computed and pushed on top of the CP stack, along with its size
   *  (encoded as a Prolog INT).  A pointer to this size, followed by
   *  the answer template (as depicted above), is returned in
   *  CallLUR_AnsTempl(lookupResults).  The template is also copied               
   *  from the CPS to the heap, where (heap - 1) points to the A.T.               
   *  Meanwhile, CallLUR_AnsTempl(lookupResults) and                              
   *  CallInfo_AnsTempl(callInfo) both point to the A.T. on the CPS.              
   *                                                                  
   *  As of 2010, table_call_search() can return XSB_FAILURE(=1) or
   *  abort if the term depth of a call is greater than a specified
   *  amount.
   */

is_neg_call = vcs_tnot_call;

if ((ret = table_call_search(CTXTc &callInfo,&lookupResults))) {
     if (ret == XSB_FAILURE) {
       Fail1;
       XSB_Next_Instr();
     }
     else { /* ret == XSB_SPECIAL_RETURN: needs incr reeval */
       Psc psc = (Psc) pflags[LAZY_REEVAL_INTERRUPT+INT_HANDLERS_FLAGS_START];
       //       printf("setting up lazy reeval for ");
       //       print_subgoal(stddbg,CallLUR_Subsumer(lookupResults));printf("\n");
       bld_cs(reg+1, build_call(CTXTc TIF_PSC(tip)));
       bld_int(reg+2, 0);
       lpcreg = get_ep(psc);
       XSB_Next_Instr();
     }
   }

  producer_sf = CallLUR_Subsumer(lookupResults);
  answer_template_cps = CallLUR_AnsTempl(lookupResults);
#ifdef MULTI_THREAD
  if( !IsNULL(producer_sf) ) UNLOCK_CALL_TRIE();
#endif

/* --------- for incremental evaluation  --------- */

/* Get parent table subgoalframe from ptcpreg */
  if(IsNonNULL(ptcpreg)){
    parent_table_sf=(VariantSF)ptcpreg;
    if(IsIncrSF(parent_table_sf)) {
      parent_table_is_incr = 1;
      /* adding called-by graph edge */
      if(IsNonNULL(producer_sf)){
	if(IsIncrSF(producer_sf)){
	  addcalledge(producer_sf->callnode,parent_table_sf->callnode);  
	} 
	else{
	  if(!get_opaque(TIF_PSC(CallInfo_TableInfo(callInfo))))
	    xsb_abort("Predicate %s:%s/%d not declared incr_table: cannot create dependency edge\n", 
		      get_mod_name(TIF_PSC(CallInfo_TableInfo(callInfo))),
		      get_name(TIF_PSC(CallInfo_TableInfo(callInfo))),
		      get_arity(TIF_PSC(CallInfo_TableInfo(callInfo))));       
	}
      }
    }
  }
 
/* --------- end incremental evaluation  --------- */

//printf("After variant call search AT: %x\n",answer_template_cps);
//printf("After variant call search producer_sf: %x\n",producer_sf);

#ifdef SHARED_COMPL_TABLES
#include "usurp.h"
#else  /* !SHARED_COMPL_TABLES */
  if ( IsNULL(producer_sf) ) {

    /* New Producer
       ------------ */
    CPtr producer_cpf;
    producer_sf = NewProducerSF(CTXTc CallLUR_Leaf(lookupResults),CallInfo_TableInfo(callInfo),is_neg_call);
    //    printf("new producer sf %p\n",producer_sf);

#endif /* !SHARED_COMPL_TABLES */
#ifdef CONC_COMPL
    subg_tid(producer_sf) = xsb_thread_id;
    subg_tag(producer_sf) = INCOMP_ANSWERS;
    UNLOCK_CALL_TRIE() ;
#endif
    LOG_TABLE_CALL("new");

/* --------- for new producer incremental evaluation  --------- */
    /* table_call_search tried to find the affected call, so if it has
       found the answer table of the new call it is made same as the
       answer table of the old call - so that we can check whether the
       answers that will be generated for the new call are same as that for
       the old call */

    if(IsNonNULL(old_call_gl)){
      producer_sf->callnode->prev_call=old_call_gl;
      producer_sf->callnode->outedges=old_call_gl->outedges;
      producer_sf->callnode->outcount=old_call_gl->outcount;
      producer_sf->callnode->outedges->callnode=producer_sf->callnode;
      producer_sf->ans_root_ptr=old_answer_table_gl;
      old_call_gl->falsecount=0; /* this will stop propagation of unmarking */
      old_call_gl->deleted=1; 
      old_call_gl=NULL;
      old_answer_table_gl=NULL;
    } 
    else {
      if(IsIncrSF(producer_sf))
	initoutedges(CTXTc producer_sf->callnode);
    }

    if(parent_table_is_incr){  
      if(IsIncrSF(producer_sf)){
	addcalledge(producer_sf->callnode,parent_table_sf->callnode);  
      }else{
	if(!get_opaque(TIF_PSC(CallInfo_TableInfo(callInfo))))
	  xsb_abort("Predicate %s:%s/%d not declared incr_table\n",
		    get_mod_name(TIF_PSC(CallInfo_TableInfo(callInfo))),
		    get_name(TIF_PSC(CallInfo_TableInfo(callInfo))),
		    get_arity(TIF_PSC(CallInfo_TableInfo(callInfo))));       
      }
    }

/* --------- end new producer incremental evaluation  --------- */

#ifdef CALL_ABSTRACTION
    answer_template_heap = hreg - 1;
    tmp = int_val(cell(answer_template_heap));
    get_var_and_attv_nums(template_size, attv_num, abstr_size,tmp);
#ifdef DEBUG_ABSTRACTION
    printf("AT (new) %p\n",answer_template_heap-1);
    printterm(stddbg,reg[1],8);printf("\n");	
    printAnswerTemplate(stddbg,answer_template_heap-1,template_size);
    printf("about to create new producer \n");                              
    print_AbsStack();                                                       
    print_callRegs(CallInfo_CallArity(callInfo));                           
#endif
/*    obsolete
  if (abstr_size > 0) copy_abstractions_to_cps(CTXTc answer_template_cps,abstr_size);              
*/
#endif
    producer_cpf = answer_template_cps;
    save_find_locx(ereg);
    save_registers(producer_cpf, CallInfo_CallArity(callInfo), rreg);
    SaveProducerCPF(producer_cpf, continuation, producer_sf,
		    CallInfo_CallArity(callInfo), (hreg - 1));
#ifdef SHARED_COMPL_TABLES
    tcp_reset_pcreg(producer_cpf) = inst_addr ;
#endif
#ifdef SLG_GC
    tcp_prevtop(producer_cpf) = answer_template_cps; 
   /* answer_template_cps points to the previous cps, since the
      A.T. proper has been copied to the heap.  In fact,
      SaveProducerCPF has already over-written the CPS A.T. */
#endif
    push_completion_frame(producer_sf);
#ifdef CONC_COMPL
    compl_ext_cons(openreg) = NULL ;
    tcp_compl_stack_ptr(producer_cpf) = openreg ;
#endif
    ptcpreg = (CPtr)producer_sf;
    subg_cp_ptr(producer_sf) = breg = producer_cpf;
    //    printf("new breg is %x %x\n",breg,((pb)tcpstack.high - (pb)breg));
    xsb_dbgmsg((LOG_COMPLETION,"just created tabled cp %x\n",breg));
    delayreg = NULL;
    if (root_address == 0)
      root_address = breg;
    hbreg = hreg;
    lpcreg = (byte *) LABEL;	/* branch to program clause */
    XSB_Next_Instr();
  } /* end new producer case */

  else if ( is_completed(producer_sf) ) {

    LOG_TABLE_CALL("cmp");
    //    printf("completed table "); print_n_registers(stddbg, 6 , 8);printf("\n");

    /* Unify Call with Answer Trie
       --------------------------- */
    SUBG_INCREMENT_CALLSTO_SUBGOAL(producer_sf);
    if (has_answer_code(producer_sf)) {
      int i;
      //      printf("++Returning answers from COMPLETED table: ");
      //      print_subgoal(stddbg, producer_sf);printf("\n");
      answer_template_heap = hreg - 1; 

      tmp = int_val(cell(answer_template_heap));
      trieinstr_vars_num = -1;

#ifdef CALL_ABSTRACTION
      get_var_and_attv_nums(template_size, attv_num, abstr_size,tmp);
#ifdef DEBUG_ABSTRACTION
      print_AbsStack();                                                   
#endif
      /* As in SetupReturnfromLeader we simply perform the
	 post-unification before branching into the trie. The trie
	 instructions will fail if the call doesn't unify -- the
	 implementation of call subsumption has already ensured that
	 this failure will occur correctly. */
      unify_abstractions_from_absStk(CTXT);
#ifdef DEBUG_ABSTRACTION
      print_callRegs(CallInfo_CallArity(callInfo));                       
#endif
#else
      get_var_and_attv_nums(template_size, attv_num, tmp);
#endif

      /* Initialize trieinstr_vars[] as the attvs in the call.  This is
	 needed by the trie_xxx_val instructions, and the symbols of
	 the trie nodes have been set up to account for this in
	 variant_answer_search() -- see the documentation there.  */
      if (attv_num > 0) {
	CPtr cptr;
	for (cptr = answer_template_heap - 1;
	     cptr >= answer_template_heap - template_size; cptr--) {
	  // tls changed from 10/05 cptr >= answer_template_heap + template_size; cptr++) 
	  if (isattv(cell(cptr))) {
	    trieinstr_vars[++trieinstr_vars_num] = (CPtr) cell(cptr);
	    xsb_dbgmsg((LOG_TRIE_INSTR, "setting trieinstr_vars for attv %d \n",
			trieinstr_vars_num));
	  }
	}
	/* now trieinstr_vars_num should be attv_num - 1 */
      }
      //      printf("nvivrs %d\n",trieinstr_vars_num);
      trieinstr_unif_stkptr = trieinstr_unif_stk-1;
      for (i = 0; i < template_size; i++) {
	push_trieinstr_unif_stk(cell(answer_template_heap-template_size+i));
      }
      //      printf("unif stk size is %d\n",i);
      delay_it = 1;

      setup_to_return_completed_answers(producer_sf,template_size);

      //#ifdef MULTI_THREAD_RWL
      ///* save choice point for trie_unlock instruction */
      //      save_find_locx(ereg);
      //      tbreg = top_of_cpstack;
      //#ifdef SLG_GC
      //      old_cptop = tbreg;
      //#endif
      //      save_choicepoint(tbreg,ereg,(byte *)&trie_fail_unlock_inst,breg);
      //#Ifdef SLG_GC
      //      cp_prevtop(tbreg) = old_cptop;
      //#endif
      //      breg = tbreg;
      //      hbreg = hreg;
      //#endif
      XSB_Next_Instr();
    }
    else {
      Fail1;
      XSB_Next_Instr();
    }
  }

  else if ( CallLUR_VariantFound(lookupResults) ) {

    LOG_TABLE_CALL("incmp");

    /* Previously Seen Subsumed Call
       ----------------------------- */
    consumer_sf = CallTrieLeaf_GetSF(CallLUR_Leaf(lookupResults)); 
    SUBG_INCREMENT_CALLSTO_SUBGOAL(producer_sf);
  }
  else {

    // Buglet -- tc instead of nc for subsumed calls.
    LOG_TABLE_CALL("incmp");
    /* New Properly Subsumed Call
       -------------------------- */
    NewSubConsSF( consumer_sf, CallLUR_Leaf(lookupResults),
		   CallInfo_TableInfo(callInfo), producer_sf );
    SUBG_INCREMENT_CALLSTO_SUBGOAL(producer_sf);

}

  
  /*
   * The call, represented by "consumer_sf", will consume from an
   * incomplete producer, represented by "producer_sf".
   */
  {
    CPtr consumer_cpf;
#ifdef SLG_GC
    CPtr prev_cptop;
#endif
    ALNptr answer_continuation;
    BTNptr first_answer;


    /* Create Consumer Choice Point
       ---------------------------- */
#ifdef CONC_COMPL
    CPtr producer_cpf = NULL;

    /* with variant tabling and thus in CONC_COMPL shared tables, 
       producer_sf == consumer_sf */
    if( subg_tid(producer_sf) == xsb_thread_id )
    {
#endif
    adjust_level(subg_compl_stack_ptr(producer_sf));
#ifdef CONC_COMPL
        consumer_cpf = answer_template_cps;
    }
    else if( openreg < COMPLSTACKBOTTOM )
        consumer_cpf = answer_template_cps;
    else
    {
	producer_cpf = answer_template_cps;
    	SaveProducerCPF(producer_cpf, (pb)&check_complete_inst, producer_sf,
                    	CallInfo_CallArity(callInfo), (hreg - 1));
	consumer_cpf = breg = producer_cpf;
    }
#else
    consumer_cpf = answer_template_cps;
#endif
    save_find_locx(ereg);

#ifdef SLG_GC
    prev_cptop = consumer_cpf;
#endif

    answer_template_heap = hreg-1;
#ifdef CALL_ABSTRACTION
    //    printf("answer_template_heap %x\n",answer_template_heap);               
    tmp = int_val(cell(answer_template_heap));
    get_var_and_attv_nums(template_size, attv_num, abstr_size,tmp);
    //    printf("++there are %d abstactions %d vars %d attvs\n",abstr_size,templ    ate_size,attv_num);   
#ifdef DEBUG_ABSTRACTION
    printf("AT (cons) %p @%p\n",answer_template_heap-1,*(answer_template_heap-1));
    printterm(stddbg,reg[1],8);printf("-- AT cons\n");	
    printAnswerTemplate(stddbg,answer_template_heap-1,template_size);
    print_heap_abstraction(answer_template_heap-(template_size+2*abstr_size),abstr_size);
#endif
#endif
    efreg = ebreg;
    if (trreg > trfreg) trfreg = trreg;
    if (hfreg < hreg) hfreg = hreg;
    SaveConsumerCPF( consumer_cpf, consumer_sf,
		     subg_pos_cons(producer_sf), 
		     answer_template_heap);

#ifdef SLG_GC
    nlcp_prevtop(consumer_cpf) = prev_cptop;
#endif

#ifdef CONC_COMPL
    nlcp_tid(consumer_cpf) = makeint(xsb_thread_id);

    if( IsSharedSF(producer_sf) )
	SYS_MUTEX_LOCK( MUTEX_CONS_LIST ) ;
    /* this is done again to allow to hold the lock for a shorter period */
    nlcp_prevlookup(consumer_cpf) = subg_pos_cons(producer_sf) ;
#endif
    subg_pos_cons(producer_sf) = consumer_cpf;
#ifdef CONC_COMPL
    if( IsSharedSF(producer_sf) )
	SYS_MUTEX_UNLOCK( MUTEX_CONS_LIST ) ;
#endif
    breg = bfreg = consumer_cpf;

    xsb_dbgmsg((LOG_COMPLETION,"created ccp at %x with prevbreg as %x\n",
		breg,nlcp_prevbreg(breg)));

#ifdef CONC_COMPL
    if( subg_tid(producer_sf) != xsb_thread_id )
    {
	push_completion_frame(producer_sf);
	compl_ext_cons(openreg) = consumer_cpf;
	if( openreg == (COMPLSTACKBOTTOM - COMPLFRAMESIZE) )
    		tcp_compl_stack_ptr(producer_cpf) = openreg ;
    }
#endif

    /* Consume First Answer or Suspend
       ------------------------------- */
    table_pending_answer( subg_ans_list_ptr(consumer_sf),
			  answer_continuation,
			  first_answer,
			  (SubConsSF)consumer_sf,
			  (SubProdSF)producer_sf,
			  answer_template_heap,
			  TPA_NoOp,
			  TPA_NoOp );

    if ( IsNonNULL(answer_continuation) ) {
#ifdef CALL_ABSTRACTION
      get_var_and_attv_nums(template_size, attv_num, abstr_size,
                            int_val(cell(answer_template_heap)));
#else
      Integer tmp;
      tmp = int_val(cell(answer_template_heap));
      get_var_and_attv_nums(template_size, attv_num, tmp);
#endif
      nlcp_trie_return(consumer_cpf) = answer_continuation; 
      hbreg = hreg;

      answer_template_heap--;
      //      printf("answer_template_heap %p\n",answer_template_heap);
      /* TLS 060913: need to initialize here, as it doesnt get
	 initialized in all paths of table_consume_answer */
      num_heap_term_vars = 0;

      //      printf("consuming answer attv_num %d\n",attv_num);

      table_consume_answer(CTXTc first_answer,template_size,attv_num,answer_template_heap,
			   CallInfo_TableInfo(callInfo));

#ifdef CALL_ABSTRACTION
#ifdef DEBUG_ABSTRACTION
      printf("about to post-unify\n");
      printAnswerTemplate(stddbg,answer_template_heap,template_size);
      print_heap_abstraction(answer_template_heap+1-(template_size+2*abstr_size),abstr_size);
#endif
      if (abstr_size > 0) {
	if (!unify_abstractions_from_AT(CTXTc (answer_template_heap+1-(template_size+2*abstr_size)), 
				      abstr_size)) {
          printf("failing at new subgoal CCP\n");
	  breg = nlcp_prevbreg(consumer_cpf);
	  Fail1;
	}
	//	printf("unify_abstractions (cons) succeeding\n");
    }
#endif

      LOG_ANSWER_RETURN(first_answer,answer_template_heap);

      if (is_conditional_answer(first_answer)) {
	xsb_dbgmsg((LOG_DELAY,
		"! POSITIVELY DELAYING in lay active (delayreg = %p)\n",
		delayreg));
	xsb_dbgmsg((LOG_DELAY, "\n>>>> delay_positively in lay_down_active\n"));
	xsb_dbgmsg((LOG_DELAY, ">>>> subgoal = "));
	dbg_print_subgoal(LOG_DELAY, stddbg, producer_sf);
	xsb_dbgmsg((LOG_DELAY, "\n"));
	{
	  /*
	   * Similar to delay_positively() in retry_active, we also
	   * need to put the substitution factor of the answer,
	   * var_addr[], into a term ret/n and pass it to
	   * delay_positively().
	   */
	  if (num_heap_term_vars == 0) {
	    fail_if_direct_recursion(producer_sf);
	    delay_positively(producer_sf, first_answer,
			     makestring(get_ret_string()));
	  }
	  else {
#ifndef IGNORE_DELAYVAR
	    int i;
	    CPtr temp_hreg = hreg;
	    new_heap_functor(hreg, get_ret_psc(num_heap_term_vars));
	    if (var_addr == NULL) printf("var_addr NULL 3\n");
	    for (i = 0; i < num_heap_term_vars; i++)
	      cell(hreg+i) = (Cell) var_addr[i];
	    hreg += num_heap_term_vars;
	    delay_positively(producer_sf, first_answer, makecs(temp_hreg));
#else
	    delay_positively(producer_sf, first_answer,
			     makestring(get_ret_string()));
#endif /* IGNORE_DELAYVAR */
	  }
	}
      }
      lpcreg = cpreg;
    }
    else {
      breg = nlcp_prevbreg(consumer_cpf);
      Fail1;
    }
  }
XSB_End_Instr()


/* incremental evaluation */

XSB_Start_Instr(tabletrysinglenoanswers,_tabletrysinglenoanswers) 
    DefOps13
  /*
   *  Retrieve instruction arguments and test the system stacks for
   *  overflow.  The local PCreg, "lpcreg", is incremented to point to
   *  the instruction to be executed should this one fail.
   *  Parameters are the same as tabletrysingle
   */
  
  TabledCallInfo callInfo;
  CallLookupResults lookupResults;
  VariantSF  sf;
  TIFptr tip;
  callnodeptr cn;

    UNUSED(op1);
    UNUSED(op3);

    //#ifdef MULTI_THREAD
    //  xsb_abort("Incremental Maintenance of tables is not available for multithreaded engine.\n");
    //#endif  
  
  //  printf("tabletrysinglenoanswers\n");

  xwammode = 1;
   
  CallInfo_CallArity(callInfo) = get_xxa; 
  LABEL = (CPtr)((byte *) get_xxxl);  
  Op1(get_xxxxl);
  tip =  (TIFptr) get_xxxxl;

			       //  printf("Subgoal Depth %d\n",TIF_SubgoalDepth(tip));
  if ( TIF_SubgoalDepth(tip) == 65535) {
    int i;
    //    new_heap_functor(hreg, TIF_PSC(tip)); /* set str psc ptr */
    CallInfo_Arguments(callInfo) = hreg;
    for (i=1; i <= (int)get_arity(TIF_PSC(tip)); i++) {
      new_heap_free(hreg);		   
    }
  }
  else CallInfo_Arguments(callInfo) = reg + 1;

  if (get_incr(TIF_PSC(tip))) {
  
    SET_TRIE_ALLOCATION_TYPE_TIP(tip); 

    CallInfo_TableInfo(callInfo) = tip;
    
    check_tcpstack_overflow;
    CallInfo_AnsTempl(callInfo) = top_of_cpstack;

  /* 
   *  As of 2010, table_call_search() can return XSB_FAILURE(=1) or
   *  abort if the term depth of a call is greater than a specified
   *  amount.
   */
    if ( table_call_search_incr(CTXTc &callInfo,&lookupResults)) {
      Fail1;
      XSB_Next_Instr();
    }
  
    if(IsNonNULL(ptcpreg)) {
      sf=(VariantSF)ptcpreg;
      if(IsIncrSF(sf)){
	cn=(callnodeptr)BTN_Child(CallLUR_Leaf(lookupResults));
	if(IsNonNULL(cn)) {
	  addcalledge(cn,sf->callnode);  
	}
      }
    }
    //    printf("creating cn for: "); print_callnode(stddbg, cn); printf("\n");
  }
#ifdef NON_OPT_COMPILE
  else  /* not incremental */
    if (!get_opaque(TIF_PSC(tip))) {
      sf=(VariantSF)ptcpreg;
      xsb_abort("Parent predicate %s:%s/%d not declared incr_table\n", 
		get_mod_name(TIF_PSC(subg_tif_ptr(sf))),
		get_name(TIF_PSC(subg_tif_ptr(sf))),
		get_arity(TIF_PSC(subg_tif_ptr(sf)))
		); 
    }
#endif
  ADVANCE_PC(size_xxx);
  lpcreg = *(pb *)lpcreg;

XSB_End_Instr()







/*-------------------------------------------------------------------------*/

/*
 *  Instruction format:
 *    1st word: opcode X X X
 *
 *  Description:
 *    Returns to a consumer an answer if one is available, otherwise it
 *    suspends.  Answer consumption is effected by unifying the consumer's
 *    answer template with an answer.  This instruction is encountered only
 *    by backtracking into a consumer choice point frame, either as a
 *    result of WAM- style backtracking or having been resumed via a
 *    check-complete instruction.  The CPF field "nlcp-trie-return" points
 *    to the last answer consumed.  If none have yet been consumed, then it
 *    points to the dummy answer.
 */

XSB_Start_Instr(answer_return,_answer_return) 
  VariantSF consumer_sf;
  ALNptr answer_continuation;
  BTNptr next_answer;
  CPtr answer_template;
  int template_size, attv_num;
#ifdef CALL_ABSTRACTION
  int abstr_size;
#endif

  /* locate relevant answers
     ----------------------- */
  answer_continuation = ALN_Next(nlcp_trie_return(breg)); /* step to next answer */
  consumer_sf = (VariantSF)nlcp_subgoal_ptr(breg);
  answer_template = nlcp_template(breg);

  //    fprintf(stddbg,"Starting answer return %x (%x) (prev %x) aln %x\n",
  //  	  breg,*lpcreg,nlcp_prevbreg(breg),answer_continuation); 

  /* TLS: put this in so that timed_call will exit on this sort of loop */
  if (asynint_val & TIMER_MARK) {					
	synint_proc(CTXTc TIF_PSC(subg_tif_ptr(consumer_sf)), TIMER_INTERRUPT);	   
        lpcreg = pcreg;							
	asynint_val = asynint_val & ~TIMER_MARK;	
        asynint_code = 0;						
    }
  else {
  table_pending_answer( nlcp_trie_return(breg),
			answer_continuation,
			next_answer,
			(SubConsSF)consumer_sf,
			conssf_producer(consumer_sf),
			answer_template,
			switch_envs(breg),
			TPA_NoOp );

  if ( IsNonNULL(answer_continuation)) {
    Integer tmp;

    /* Restore Consumer's state
       ------------------------ */
    switch_envs(breg);
    ptcpreg = nlcp_ptcp(breg);
    delayreg = nlcp_pdreg(breg);
    restore_some_wamregs(breg, ereg);

    /* Consume the next answer
       ----------------------- */
    nlcp_trie_return(breg) = answer_continuation;   /* update */
    tmp = int_val(cell(answer_template));
#ifdef CALL_ABSTRACTION
    get_var_and_attv_nums(template_size, attv_num, abstr_size,tmp);
#else
    get_var_and_attv_nums(template_size, attv_num, tmp);
#endif
    answer_template--;

    //    printf("answer_template %x size %d\n",answer_template,template_size);
    //    sfPrintGoal(CTXTdeclc stddbg, consumer_sf, FALSE);


    /* TLS 060927: need to initialize here, as it doesnt get
       initialized in all paths of table_consume_answer */
      num_heap_term_vars = 0;

table_consume_answer(CTXTc next_answer,template_size,attv_num,answer_template,
			 subg_tif_ptr(consumer_sf));

#ifdef CALL_ABSTRACTION
// printf("ansret abstrsize %d\n",abstr_size);
 if (abstr_size > 0) {
   if (!unify_abstractions_from_AT(CTXTc (answer_template+1-(template_size+2*abstr_size)), 
				   abstr_size)) {
     printf("failing in answer return\n");
     //      breg = nlcp_prevbreg(consumer_cpf);                             
     Fail1;
     XSB_Next_Instr();
   }
   //   printf("succeeding\n");
 }
#endif

 LOG_ANSWER_RETURN(next_answer,answer_template);

    if (is_conditional_answer(next_answer)) {
      /*
       * After load_solution_trie(), the substitution factor of the
       * answer is left in array var_addr[], and its arity is in
       * num_heap_term_vars.  We have to put it into a term ret/n (on 
       * the heap) and pass it to delay_positively().
       */
      if (num_heap_term_vars == 0) {
	delay_positively(consumer_sf, next_answer,
			 makestring(get_ret_string()));
      }
      else {
#ifndef IGNORE_DELAYVAR
	int i;
	CPtr temp_hreg = hreg;
	new_heap_functor(hreg, get_ret_psc(num_heap_term_vars));
	if (var_addr == NULL) printf("var_addr NULL 4\n");
	for (i = 0; i < num_heap_term_vars; i++) {
	  cell(hreg+i) = (Cell) var_addr[i];
	}
	hreg += num_heap_term_vars;
	delay_positively(consumer_sf, next_answer, makecs(temp_hreg));
#else
	delay_positively(consumer_sf, next_answer,
			 makestring(get_ret_string()));
#endif /* IGNORE_DELAYVAR */

      }
    }
    lpcreg = cpreg;
    }

  else {

    /* Suspend this Consumer
       --------------------- */
    xsb_dbgmsg((LOG_DEBUG,"Failing from answer return %x to %x (inst %x)\n",
		breg,nlcp_prevbreg(breg),*tcp_pcreg(nlcp_prevbreg(breg))));
    breg = nlcp_prevbreg(breg); /* in semi-naive this execs next active */
    restore_trail_condition_registers(breg);
    if (hbreg >= hfreg) hreg = hbreg; else hreg = hfreg;
    Fail1;
  }
  }
XSB_End_Instr()

/*-------------------------------------------------------------------------*/

/*
 *  Instruction format:
 *    1st word: opcode X pred_arity perm_var_index
 *
 *  Description:
 *    Last instruction in each clause of a tabled predicate.  The instruction
 *    (1) saves the answer substitution in the answer trie and (2)
 *    adds a cell to the end of the answer list pointing to the root
 *    of the new subtstitution. All the
 *    information necessary to perform this Answer Check/Insert operation
 *    is saved in the producer's choice point frame.  This CP is 
 *    reached through the subgoal frame, which is noted in the first
 *    environment variable of the tabled clause.
 * 
 *    In the case where we have added an unconditional ground answer
 *    we perform early completion for the subgoal.
 *
 *    Next, if we are executing Local Evaluation, we fail after adding
 *    the answer (and perhaps performing ec)  This is not always the
 *    optimal way, as we need fail only if the subgoal is potentially
 *    a leader.
 * 
 *    If we are not executing local evaluation, we take the forward
 *    condinuation (i.e. we'll proceed).  In his case a delay element
 *    must be added to the delay list or the root subgoal of the
 *    current subgoal before proceeding.
 */


#define check_new_answer_interrupt {					\
    if ( !(asynint_val) ) {						\
      Fail1;								\
    } else {								\
      if (asynint_val & THREADINT_MARK) {				\
	/*	printf("Entered thread cancel: proceed\n");	*/	\
        synint_proc(CTXTc true_psc, THREADSIG_CANCEL);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
      } else if (asynint_val & KEYINT_MARK) {				\
		printf("Entered keyb handle: new_answer_dealloc\n");  \
	synint_proc(CTXTc true_psc, MYSIG_KEYB);			\
	lpcreg = pcreg;							\
        asynint_val = asynint_val & ~KEYINT_MARK;			\
        asynint_code = 0;						\
      } else if (asynint_val & TIMER_MARK) {				\
	/*	printf("Entered timer handle: new_answer_dealloc\n"); */ \
	synint_proc(CTXTc true_psc, TIMER_INTERRUPT);			\
        lpcreg = pcreg;							\
        asynint_val = 0;						\
        asynint_code = 0;						\
      } else {								\
	lpcreg = pcreg;							\
        asynint_code = 0;						\
      }									\
    }									\
  }								


XSB_Start_Instr(new_answer_dealloc,_new_answer_dealloc) 
  Def2ops
  CPtr producer_cpf, producer_csf, answer_template;
  int template_size, attv_num; Integer tmp;
  VariantSF producer_sf;
  xsbBool isNewAnswer = FALSE;
  BTNptr answer_leaf;
#ifdef CALL_ABSTRACTION
  int abstr_size;
  UNUSED(abstr_size);
#endif

  UNUSED(op1);
  UNUSED(producer_csf);

  ARITY = get_xax;
  Yn = get_xxa; /* we want the # of the register, not a pointer to it */

  ADVANCE_PC(size_xxx);

  xsb_dbgmsg((LOG_COMPLETION,"starting new_answer breg %x\n",breg));
  producer_sf = (VariantSF)cell(ereg-Yn);
  producer_cpf = subg_cp_ptr(producer_sf);

#ifdef DEBUG_DELAYVAR
  printf(">>>> New answer for %s subgoal: ",
	 (is_completed(producer_sf) ? "completed" : "incomplete"));
  fprintf(stddbg, ">>>> ");
  dbg_print_subgoal(LOG_DEBUG, stddbg, producer_sf);
  xsb_dbgmsg((LOG_DEBUG,">>>> has delayreg = %p", delayreg));
#endif

  producer_csf = subg_compl_stack_ptr(producer_sf);

  /* if the subgoal has been early completed and its space reclaimed
   * from the stacks, access to its relevant information (e.g. to its
   * substitution factor) in the stacks is not safe, so better not
   * try to add this answer; it is a redundant one anyway...
   *
   * TLS: change to simply check whether the subgoal was complete
   * which means that AR for EC'd subgoals will not get past here.
   */
  //  if ((subgoal_space_has_been_reclaimed(producer_sf,producer_csf)) ||
  if ((subg_is_completed(producer_sf)) ||
	(IsNonNULL(delayreg) && answer_is_unsupported(CTXTc delayreg))) {
    Fail1;
    XSB_Next_Instr();
  }

  /* answer template is now in the heap for generators */
  answer_template = tcp_template(subg_cp_ptr(producer_sf));
  tmp = int_val(cell(answer_template));
#ifdef CALL_ABSTRACTION
  get_var_and_attv_nums(template_size, attv_num, abstr_size,tmp);
//  printf("na template %d attv %d abstr %d\n",template_size, attv_num, abstr_size);
#else
  get_var_and_attv_nums(template_size, attv_num, tmp);
#endif
  answer_template--;

  xsb_dbgmsg((LOG_DELAY, "\t--> This answer for "));
  dbg_print_subgoal(LOG_DELAY, stddbg, producer_sf);
#ifdef DEBUG_VERBOSE
    if (delayreg != NULL) {
      fprintf(stddbg, " has delay list = ");
      print_delay_list(stddbg, delayreg);
    } else {
      fprintf(stddbg, " has no delay list\n");
    }
#endif

#ifdef DEBUG_DELAYVAR
  fprintf(stddbg, "\n>>>> (before variant_answer_search) template_size = %d\n",
	  (int)template_size);
  {
    int i;
    for (i = 0; i < template_size; i++) {
      fprintf(stddbg, ">>>> answer_template[%d] = ", i);
      printterm(stddbg, (Cell)(answer_template - i), 25);
      fprintf(stddbg, "\n");
    }
  }
#endif

  SET_TRIE_ALLOCATION_TYPE_SF(producer_sf); /* No-op in seq engine */
  answer_leaf = table_answer_search( CTXTc producer_sf, template_size, attv_num,
				     answer_template, &isNewAnswer );

  if ( isNewAnswer ) {   /* go ahead -- look for more answers */

  if (flags[CTRACE_CALLS] && !subg_forest_log_off(producer_sf))  { 
    memset(forest_log_buffer_1->fl_buffer,0,MAXTERMBUFSIZE);
    memset(forest_log_buffer_2->fl_buffer,0,MAXTERMBUFSIZE);
    memset(forest_log_buffer_3->fl_buffer,0,MAXTERMBUFSIZE);
    //    sprint_registers(CTXTc  buffera,TIF_PSC(subg_tif_ptr(producer_sf)),flags[MAX_TABLE_SUBGOAL_SIZE]);
    //    printAnswerTemplate(stddbg,answer_template ,(int) template_size);
    sprintAnswerTemplate(CTXTc forest_log_buffer_1, 
			 answer_template, template_size);
    if (ptcpreg)
      sprint_subgoal(CTXTc forest_log_buffer_2,0,(VariantSF)producer_sf);     
    else sprintf(forest_log_buffer_2->fl_buffer,"null");
    if (delayreg) {
      sprint_delay_list(CTXTc forest_log_buffer_3, delayreg);
      fprintf(fview_ptr,"nda(%s,%s,%s,%d).\n",forest_log_buffer_1->fl_buffer,
	      forest_log_buffer_2->fl_buffer,forest_log_buffer_3->fl_buffer,
	      ctrace_ctr++);
    }
    else 
      fprintf(fview_ptr,"na(%s,%s,%d).\n",forest_log_buffer_1->fl_buffer,
	      forest_log_buffer_2->fl_buffer,ctrace_ctr++);
  }
#ifdef DEBUG_ABSTRACTION
  printf("AT (na) %p size %d\n",answer_template,template_size);
  print_heap_abstraction(answer_template-2*(abstr_size)-(template_size-1),abstr_size);
  printAnswerTemplate(stddbg,answer_template,template_size);
  print_registers(stddbg, TIF_PSC(subg_tif_ptr(producer_sf)),flags[MAX_TABLE_SUBGOAL_SIZE]);
#endif

    SUBG_INCREMENT_ANSWER_CTR(producer_sf);
    /* incremental evaluation */
    if(IsIncrSF(producer_sf))
      subg_callnode_ptr(producer_sf)->no_of_answers++;
    
    
    delayreg = tcp_pdreg(producer_cpf);      /* restore delayreg of parent */
    if (is_conditional_answer(answer_leaf)) {	/* positive delay */
#ifndef LOCAL_EVAL
#ifdef DEBUG_DELAYVAR
      fprintf(stddbg, ">>>> delay_positively in new_answer_dealloc\n");
#endif
      /*
       * The new answer for this call is a conditional one, so add it
       * into the delay list for its root subgoal.  Notice that
       * delayreg has already been restored to the delayreg of parent.
       *
       * This is the new version of delay_positively().  Here,
       * ans_var_pos_reg is passed from variant_answer_search().  It is a
       * pointer to the heap where the substitution factor of the
       * answer was saved as a term ret/n (in variant_answer_search()).
       */
#ifndef IGNORE_DELAYVAR
      fail_if_direct_recursion(producer_sf);
      if (isinteger(cell(ans_var_pos_reg))) {
	delay_positively(producer_sf, answer_leaf,
			 makestring(get_ret_string()));
      }
      else 
	delay_positively(producer_sf, answer_leaf, makecs(ans_var_pos_reg));
#else
	delay_positively(producer_sf, answer_leaf,
			 makestring(get_ret_string()));
#endif /* IGNORE_DELAYVAR */
#endif /* ! LOCAL_EVAL */
    }
    else {
      if (template_size == 0 || subg_is_ec_scheduled(producer_sf) ) {
	/*
	 *  The table is for a ground call which we just proved true.
	 *  (We entered an ESCAPE Node, above, to note this fact in the
	 *  table.)  As we only need to do this once, we perform "early
	 *  completion" by ignoring the other clauses of the predicate
	 *  and setting the failure continuation (next_clause) field of
	 *  the CPF to a check_complete instr.
	 *
	 */
	/*		
		printf("performing early completion for: (%d)",subg_is_complete(producer_sf));
		print_subgoal(CTXTc stddbg, producer_sf);
		printf("(breg: %x pcpf %x\n",breg,producer_cpf);alt_print_cp(CTXTc"ec");
	 */
	perform_early_completion(CTXTc producer_sf, producer_cpf);
#if defined(LOCAL_EVAL)
	flags[PIL_TRACE] = 1;
	/* The following prunes deeper choicepoints at early completion if possible */
	if (tcp_pcreg(producer_cpf) != (byte *) &answer_return_inst) {
          CPtr b = breg;
	  //          printf("+++ ec breg: %p, producer %p\n",b,producer_cpf);
          while (b < producer_cpf) {
	    //            printf("+++ unwind %p\n",b);
            CHECK_TRIE_ROOT(CTXTc b);
            CHECK_CALL_CLEANUP(CTXTc b);
            b = cp_prevbreg(b);
          }
          breg = producer_cpf;
        }
#endif
      }
    }
#ifdef LOCAL_EVAL
   check_new_answer_interrupt;
    //    Fail1;	/* and do not return answer to the generator */
    xsb_dbgmsg((LOG_DEBUG,"Failing from new answer %x to %x (inst %x)\n",
		breg,tcp_pcreg(breg),*tcp_pcreg(breg)));

#else
    ptcpreg = tcp_ptcp(producer_cpf);
    cpreg = *((byte **)ereg-1);
    ereg = *(CPtr *)ereg;
    lpcreg = cpreg; 
#endif
  }
  else     /* repeat answer -- ignore */
     Fail1;
XSB_End_Instr()

/*-------------------------------------------------------------------------*/

/*
 *  Instruction format:
 *    1st word: opcode X X pred_arity
 *    2nd word: pred_next_clause_label
 *
 *  Description:
 *    Store the predicate's arity in "op1", update the failure continuation
 *    to the instruction following this one, and set the program counter to
 *    the predicate's next code subblock to be executed, as pointed to by
 *    the second argument to this instruction.  Finally, restore the state
 *    at the point of choice and continue execution using the predicate's
 *    next code subblock.
 */

XSB_Start_Instr(tableretry,_tableretry)
  Def1op
  Op1(get_xxa);
  tcp_pcreg(breg) = lpcreg+sizeof(Cell)*2;
  lpcreg = *(pb *)(lpcreg+sizeof(Cell));
  restore_type = 0;
  TABLE_RESTORE_SUB
XSB_End_Instr()

/*-------------------------------------------------------------------------*/

/*
 *  Instruction format:
 *    1st word: opcode X X pred_arity
 *
 *  Description:
 *    Store the predicate's arity in "op1", update the failure continuation
 *    to a check_complete instruction, and set the program counter to the
 *    predicate's last code subblock to be executed, as pointed to by the
 *    second argument to this instruction.  Finally, restore the state at
 *    the point of choice and continue execution with this last code
 *    subblock.
 */

XSB_Start_Instr(tabletrust,_tabletrust)
  Def1op
  Op1(get_xxa);
  ADVANCE_PC(size_xxx);
  tcp_pcreg(breg) = (byte *) &check_complete_inst;
  lpcreg = *(pb *)lpcreg;
#if defined(LOCAL_EVAL)
  /* trail cond. registers should not be restored here for Local */
  restore_type = 0;
#else
  restore_type = 1;
#endif
  TABLE_RESTORE_SUB
XSB_End_Instr()
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

XSB_Start_Instr(resume_compl_suspension,_resume_compl_suspension)

  //       printf(">>>> resume_compl_suspension is called %d\n",infcounter);
  //      print_local_stack_nonintr(CTXTc "resume_cs");
  //       print_instr = 1;
  //   alt_dis();
{
  if (csf_pcreg(breg) == (pb)(&resume_compl_suspension_inst)) {
    CPtr csf = breg;
    
    //     printf(">>>> csf\n");

    /* Switches the environment to a frame of a subgoal that was	*/
    /* suspended on completion, and sets the continuation pointer.	*/
    check_glstack_overflow(0,lpcreg,OVERFLOW_MARGIN);
    freeze_and_switch_envs(csf, CSF_SIZE);
    ptcpreg = csf_ptcp(csf);
    neg_delay = (csf_neg_loop(csf) != FALSE);
    delayreg = csf_pdreg(csf);
    cpreg = csf_cpreg(csf); 
    ereg = csf_ereg(csf);
    ebreg = csf_ebreg(csf);
    hbreg = csf_hreg(csf);
    save_find_locx(ereg);
    hbreg = hreg;
    breg = csf_prevcsf(csf);
    lpcreg = cpreg;
  } else {
    //     printf(">>>> csp\n");
    CPtr csf = cs_compsuspptr(breg);
    /* Switches the environment to a frame of a subgoal that was	*/
    /* suspended on completion, and sets the continuation pointer.	*/
    check_glstack_overflow(0,lpcreg,OVERFLOW_MARGIN);
    freeze_and_switch_envs(csf, COMPL_SUSP_CP_SIZE);
    ptcpreg = csf_ptcp(csf);
    neg_delay = (csf_neg_loop(csf) != FALSE);
    delayreg = csf_pdreg(csf);
    cpreg = csf_cpreg(csf); 
    ereg = csf_ereg(csf);
    ebreg = csf_ebreg(csf);
    hbreg = csf_hreg(csf);
    save_find_locx(ereg);
    hbreg = hreg;
    if (csf_prevcsf(csf) != NULL) {
      cs_compsuspptr(breg) = csf_prevcsf(csf);
    } else {
      breg = cs_prevbreg(breg);
    }
    lpcreg = cpreg;
  }
  //  print_local_stack_nonintr(CTXTc "resume_cs");

}
XSB_End_Instr()

/*----------------------------------------------------------------------*/

