/* File:      schedrev_xsb_i.h
** Author(s): Juliana Freire
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
** Inc, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: schedrev_xsb_i.h,v 1.30 2012-01-04 23:19:35 tswift Exp $
** 
*/

/*-------------------------------------------------------------------------*/

/*
 * Schedules consumers to which at least some answers haven't been
 * returned.  Does so by linking them through their "previous breg" choice
 * point fields.  The value returned is the scheduling chain of such
 * consumers (NULL if no such consumers of this producer exist).
 * sched_answers() schedules all consumers for a given subgoal in an
 * SCC; it calls ScheduleConsumers which links each single consumer
 * into the scheduling chain.
 */

/* Define COMPL_COND_OFF to turn off completion conditions (in case of emergency :-) */
//define COMPL_COND_OFF

#define ScheduleConsumer(Consumer,First,Last) {		\
   if ( IsNonNULL(Last) )				\
     nlcp_prevbreg(Last) = Consumer;			\
   else	 /* record first consumer to backtrack to */	\
     First = Consumer;					\
   Last = Consumer;					\
 }

#define ANSWER_TEMPLATE nlcp_template(consumer_cpf)

//------------------------------------------------------------------------------------

static CPtr sched_answers(CTXTdeclc VariantSF producer_sf, CPtr *last_consumer)
{
  CPtr first_sched_cons, last_sched_cons, consumer_cpf;

#ifdef PROFILE
  subinst_table[SCHED_ANSWERS][1]++;
#endif	
  first_sched_cons = last_sched_cons = NULL;
  consumer_cpf = subg_pos_cons(producer_sf);
  //  printf(" scheduling answers for ");
  //  print_subgoal(CTXTc stddbg, producer_sf);printf(" first CCP %x\n",consumer_cpf);

    /**** The producer has answers and consuming calls ****/  
  if ( has_answers(producer_sf) && IsNonNULL(consumer_cpf) 
       && !subg_is_reclaimed(producer_sf)) {
    /**** Check each consumer for unresolved answers ****/
    if ( IsSubsumptiveProducer(producer_sf) )
      while ( IsNonNULL(consumer_cpf) ) {
	SubConsSF consumer_sf;
	ALNptr answer_continuation;
	BTNptr next_answer;
        UNUSED(next_answer);

	consumer_sf = (SubConsSF)nlcp_subgoal_ptr(consumer_cpf);

	table_pending_answer( nlcp_trie_return(consumer_cpf),
			      answer_continuation,
			      next_answer,
			      consumer_sf,
			      (SubProdSF)producer_sf,
			      ANSWER_TEMPLATE,
			      switch_envs(consumer_cpf),
			      TPA_NoOp );
	if ( IsNonNULL(answer_continuation) )
	  ScheduleConsumer(consumer_cpf,first_sched_cons,last_sched_cons);
	consumer_cpf = nlcp_prevlookup(consumer_cpf);
      }
#ifndef COMPL_COND_OFF
/*************** with ec optimization **************/

    else {  /* variant case with completion condition checks*/
      CPtr last_perm_cpf = (CPtr) &(subg_pos_cons(producer_sf));
      while ( IsNonNULL(consumer_cpf) ) {
#ifdef CONC_COMPL
	if( int_val(nlcp_tid(consumer_cpf)) != xsb_thread_id )
		;
	else
#endif
	  if ( IsNonNULL(ALN_Next(nlcp_trie_return(consumer_cpf))) ) {
	    VariantSF consumer_sf;
	    consumer_sf = (VariantSF)nlcp_ptcp(consumer_cpf);
	    if (subg_is_completed(consumer_sf) || subg_is_ec_scheduled(consumer_sf)) {
	      //	      printf(" Unchaining ");  print_subgoal(stdout,consumer_sf);   printf(" in ");
	      //	      print_subgoal(stdout,producer_sf); printf("\n");
	      //	      printf("  setting @%p to %p\n",last_perm_cpf,nlcp_prevlookup(consumer_cpf));
	      if (last_perm_cpf == (CPtr) &(subg_pos_cons(producer_sf)))
		last_perm_cpf = nlcp_prevlookup(consumer_cpf);
	      else
		nlcp_prevlookup(last_perm_cpf) = nlcp_prevlookup(consumer_cpf);
	    }
	    else {
	      last_perm_cpf = nlcp_prevlookup(consumer_cpf);
	      //	      printf(" scheduling answer from "); print_subgoal(stdout,producer_sf); printf(" to ");
	      //	      print_subgoal(stdout,consumer_sf); printf("\n");
	      ScheduleConsumer(consumer_cpf,first_sched_cons,last_sched_cons); 
	    }
	  }
	consumer_cpf = nlcp_prevlookup(consumer_cpf);
      }
    }
    if (subg_is_completed(producer_sf))
      subg_pos_cons(producer_sf) = NULL;
//------------------/------------------/------------------/------------------/------------------
#else     
/*************** without ec optimization **************/
    else {
      while ( IsNonNULL(consumer_cpf) ) {
#ifdef CONC_COMPL
	if( int_val(nlcp_tid(consumer_cpf)) != xsb_thread_id )
		;
	else
#endif
	  //	  if (subg_is_complete((VariantSF)nlcp_ptcp(consumer_cpf))) pprintf("checking completed\n");
	  if ( IsNonNULL(ALN_Next(nlcp_trie_return(consumer_cpf))) ) {
	VariantSF consumer_sf;
	consumer_sf = (VariantSF)nlcp_ptcp(consumer_cpf);
	//	printf(" scheduling answer from"); print_subgoal(stdout,producer_sf); printf(" to ");
	//	print_subgoal(stdout,consumer_sf); printf("\n");
	    ScheduleConsumer(consumer_cpf,first_sched_cons,last_sched_cons); 
	  }
	consumer_cpf = nlcp_prevlookup(consumer_cpf);
      }
    }
#endif
//------------------/------------------/------------------/------------------/------------------
    if( last_consumer )
      *last_consumer = last_sched_cons;
    if( last_sched_cons ) {
      /* by default the schain points to the leader */
      nlcp_prevbreg(last_sched_cons) = breg ;
      //      printf("scheduling fixed point base for last %x at %x @breg %x\n",
      //	     last_sched_cons,breg,*tcp_pcreg(breg));
    }
  } /* if any answers and active nodes */

  return first_sched_cons;
}

/*-------------------------------------------------------------------------*/

/* returns 0 if reached fixpoint, otherwise, returns the next breg 
 * for batched.  Essentially this routine performs a sched_answer()
 * for each subgoal in the (A)SCC except the leader.  This
 * sched_answer() has already been performed earlier in the
 * check_complete instruction.
 */

static CPtr find_fixpoint(CTXTdeclc CPtr leader_csf, CPtr producer_cpf) 
{

  VariantSF currSubg;
  CPtr complFrame; /* completion frame for currSubg */
  CPtr last_cons = 0; /* last consumer scheduled */
  CPtr sched_chain = 0, prev_sched = 0, tmp_sched = 0; /* build sched chain */

#ifdef PROFILE
  subinst_table[OUTER_FIXPOINT][1]++;
#endif
  xsb_dbgmsg((LOG_COMPLETION,"starting fp: %x\n",
  	      subg_cp_ptr(compl_subgoal_ptr(leader_csf))));

  complFrame = openreg;
  /* for each subgoal in the ASCC, from youngest to leader there is no
   * need to include the leader.  This is because sched_answers() is
   * done for each subgoal whenever it executes a check_complete
   * operation. Thus, scheduling for the leader has already been done.
   */
#ifdef CONC_COMPL
  InitThreadDepList(&th->TDL);

  while(complFrame <= leader_csf) {
#else
  while(complFrame < leader_csf) {
#endif
#ifdef PROFILE
    subinst_table[ITER_FIXPOINT][1]++;
#endif
    currSubg = compl_subgoal_ptr(complFrame);
    xsb_dbgmsg((LOG_COMPLETION,"iterating: %x\n",subg_cp_ptr(currSubg)));
    /* check if all answers have been resolved for this subgoal */

    /* if there are unresolved answers for currSubg */
#ifdef CONC_COMPL
    if ( IsNonNULL(compl_ext_cons(complFrame)) )
    	last_cons=tmp_sched=sched_external(CTXTc compl_ext_cons(complFrame));
    else
    	tmp_sched = sched_answers(CTXTc currSubg, &last_cons) ;
    if (tmp_sched) {
#else

      /* TLS Sept 08.  Made this change to ensure that a non-leader
	 GCP that has been turned into a CCP will point back to the
	 leader if it is not otherwise scheduled.  It replaces the
	 change made on Nov 05 at the end of check_complete in
	 complete_xsb_i.h.  I have fond hopes that this change is
	 finally correct. */

#ifdef  LOCAL_EVAL
      if (subg_pos_cons(currSubg)) {
	tcp_prevbreg(subg_pos_cons(currSubg)) = breg;
      }
#ifdef COMPL_COND_OFF
      else xsb_warn("Subgoal without any consumer is in non-trivial SCC in Local\n");
#endif
#endif

    if ((tmp_sched = sched_answers(CTXTc currSubg, &last_cons))) {
#endif
      if (prev_sched) { /* if there is a prev subgoal scheduled */
	/* link new node to the previous one */
	nlcp_prevbreg(prev_sched) = tmp_sched;
      }
      else {
	sched_chain = tmp_sched; /* first node in the chain */
      }
      if( last_cons == NULL ) printf("lc NULL");
      prev_sched = last_cons;
    }
    complFrame = prev_compl_frame(complFrame);	
  }  /* while */

  if (prev_sched)  /* if anything has been scheduled */
    /* the first generator should backtrack to leader */
    nlcp_prevbreg(prev_sched) = producer_cpf;  

  xsb_dbgmsg((LOG_COMPLETION,"ending fp: %x\n",
	      subg_cp_ptr(compl_subgoal_ptr(leader_csf))));

  return sched_chain;
}
