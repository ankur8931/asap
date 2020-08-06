/* File:      complete_xsb_i.h
** Author(s): Juliana Freire, Kostis Sagonas, Terry Swift
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
** $Id: complete_xsb_i.h,v 1.51 2012-02-12 22:49:12 tswift Exp $
** 
*/

/* special debug includes */
#include "debugs/debug_delay.h"

#define FailIfAnswersFound(func) \
{ \
  CPtr tmp_breg; \
  if ((tmp_breg = func)) { \
    breg = tmp_breg; \
    Fail1; \
    XSB_Next_Instr(); \
  } \
}

#define check_fixpoint(sg, b)    find_fixpoint(CTXTc sg, b)

#define find_leader(cs_ptr)						\
  while (!(is_leader(cs_ptr))) {					\
    cs_ptr = prev_compl_frame(cs_ptr);					\
      }

#define setup_to_return_completed_answers(SGF,tsize)			\
  do {									\
    if (!subg_is_answer_completed(SGF) && flags[ANSWER_COMPLETION]) {	\
      flags[ANSWER_COMPLETION] = 0;					\
      reg[1] = makeint(SGF);						\
      reg[2] = build_ret_term_reverse(CTXTc tsize, trieinstr_unif_stk);	\
      /*      lpcreg = (byte *) subg_ans_root_ptr(SGF);	*/		\
      lpcreg = (pb)get_ep(answer_completion_psc);			\
    } else {								\
      lpcreg = (byte *) subg_ans_root_ptr(SGF);				\
    }									\
  } while(0)

/*----------------------------------------------------------------------*/

XSB_Start_Instr(check_complete,_check_complete)
  CPtr    cs_ptr;
#ifdef CONC_COMPL 
  CPtr	new_leader;
  CPtr	tmp_breg ;
  int busy ;
  switch_envs(breg);    
  ptcpreg = tcp_ptcp(breg);
  delayreg = tcp_pdreg(breg);

  cs_ptr = tcp_compl_stack_ptr(breg);
  if (!is_leader(cs_ptr)) 
    breg = tcp_prevbreg(breg); 
  else
  {     pthread_mutex_lock(&completing_mut);
        SYS_MUTEX_INCR( MUTEX_COMPL );
  	for(;;)
  	{
		if( (tmp_breg = check_fixpoint(cs_ptr,breg)) )
		{
			th->last_ans++;
			breg = tmp_breg ;
			break ;
		}
		new_leader = cs_ptr ;

		UpdateDeps( th, &busy, &new_leader ) ;

		if( new_leader > cs_ptr )
		{
			adjust_level(new_leader) ;
			breg = tcp_prevbreg(breg); 
			break ;
		}

		if( EmptyThreadDepList(&th->TDL) )
		{	CPtr orig_breg = breg;	
    			batched_compute_wfs(CTXTc cs_ptr, breg, 
					(VariantSF)tcp_subgoal_ptr(breg));
    			if (openreg == prev_compl_frame(cs_ptr))
			{	if (breg == orig_breg)
				{	reclaim_stacks(orig_breg);
					breg = tcp_prevbreg(breg);
      			}	}
			break ;
		}

		if( !busy )
		{	

			if( MayHaveAnswers(th) )
				;
			else if( CheckForSCC(th) )
			{
				CompleteOtherThreads(th);
				CompleteTop(th, cs_ptr);
				break;
			}
			else
				WakeOtherThreads(th) ;
		}

		th->completing = TRUE ;
		th->completed = FALSE ;
		th->cc_leader = cs_ptr ;
		pthread_cond_wait(&th->cond_var, &completing_mut) ;
        	SYS_MUTEX_INCR( MUTEX_COMPL );
		th->completing = FALSE ;
		if( th->completed )
			break ;
  	}
        pthread_mutex_unlock(&completing_mut);
  }
#else /* not CONC_COMPL */
  CPtr    orig_breg = breg;
  xsbBool leader = FALSE;
  VariantSF subgoal;

  /* this CP has exhausted program resolution -- backtracking occurs */
  switch_envs(breg);    /* in CHAT: undo_bindings() */
  ptcpreg = tcp_ptcp(breg);
  delayreg = tcp_pdreg(breg);

  subgoal = (VariantSF) tcp_subgoal_ptr(breg);	/* subgoal that is checked */

//gccp(CTXT);

//  printf("Check complete for ");print_subgoal(stderr, subgoal);printf("\n");

  cs_ptr = subg_compl_stack_ptr(subgoal);

  if ( is_leader(cs_ptr)) {
    leader = 1;
  }
  
/* 
 * This code is done regardless of whether breg is a leader.  The
 * thought was that as long as we're here, why not schedule answers
 * for this subgoal.  Its purely a heuristic -- perhaps we should test
 * to see whether its inclusion makes any difference 
 */
  FailIfAnswersFound(sched_answers(CTXTc subgoal, NULL));
  if (leader) {
    
    //    printf("leader scheduling answers breg %x\n",breg);
    /* The following code is only done in profile mode; it keeps track
     * of characteristics of SCCs */

     ProfileLeader; 
    /* SpitOutGraph(cs_ptr); */
    /* check if fixpoint has been reached, otherwise schedule any
     * unresolved answers */
    FailIfAnswersFound(check_fixpoint(cs_ptr,breg));

#ifdef LOCAL_EVAL
    {
      CPtr cc_tbreg = orig_breg;
      
      breg = orig_breg; /* mark topmost SCC as completed */
      
      /* schedule completion suspensions if any */
      cc_tbreg = ProcessSuspensionFrames(CTXTc cc_tbreg, cs_ptr);
      FailIfAnswersFound((cc_tbreg == orig_breg ? 0 : cc_tbreg));
      
      CompleteSimplifyAndReclaim(CTXTc cs_ptr);
/*
#ifdef SHARED_COMPL_TABLES
    pthread_mutex_lock(&completing_mut);
    pthread_cond_broadcast(&completing_cond);
    pthread_mutex_unlock(&completing_mut);
#endif
*/
    /* TLS: not sure about condition: how could subg_answers be true
       and has_answer_code be false? */
      /* leader has non-returned answers? */
      if (has_answer_code(subgoal) && (subg_answers(subgoal) > COND_ANSWERS)) {
	int tsize;
	reclaim_incomplete_table_structs(subgoal);
	/* schedule return of answers from trie code */
	SetupReturnFromLeader(CTXTc orig_breg, cs_ptr, subgoal);
	get_template_size(tsize,int_val(*tcp_template(orig_breg)));
	setup_to_return_completed_answers(subgoal,tsize);
	//	printf("finished completion");
	XSB_Next_Instr();
      } else {  /* There are no answers to return */
	reclaim_incomplete_table_structs(subgoal);
	/* there is nothing else to be done for this SCC */
	cc_tbreg = tcp_prevbreg(orig_breg); /* go to prev SCC */
	openreg = prev_compl_frame(cs_ptr); 
	reclaim_stacks(orig_breg);
      } 
      breg = cc_tbreg; /* either orig, prev_cp or compl_susp */
    }
    
#else /* NOT LOCAL:  FOR BATCHED SCHEDULING */

    batched_compute_wfs(CTXTc cs_ptr, orig_breg, subgoal);

    /* do all possible stack reclamation */
    if (openreg == prev_compl_frame(cs_ptr)) {
      if (breg == orig_breg) {	
	reclaim_stacks(orig_breg); /* lfcastro */
	breg = tcp_prevbreg(breg);
      }
    }

#endif /* ifdef LOCAL_EVAL */

  }
  else {    /* if not leader */
    //    CPtr tmp_breg = breg;    CPtr tmp_cs_ptr = cs_ptr;

#ifdef LOCAL_EVAL
    makeConsumerFromGenerator(CTXTc subgoal);
/*     subg_cp_ptr(subgoal) = NULL; */
#endif
    breg = tcp_prevbreg(breg); 
    
    /* TLS: Nov 05; backed out Sept 08.  in the case where we
     * backtrack to a check_complete instruction for a subtoal that is
     * not the leader, we need to set its prevbreg value to the
     * leader.  Otherwise, we could end up backtracking into the same
     * choice point multiple times -- incorrectly if we have a cut. */

    //    find_leader(tmp_cs_ptr); 
    //    tcp_prevbreg(tmp_breg) = subg_cp_ptr(compl_subgoal_ptr(tmp_cs_ptr));

    xsb_dbgmsg((LOG_SCHED,"backtracking from check complete to: %x @breg %x\n",
		  breg,*tcp_pcreg(breg)));

    /* since the trail condition registers are not reset in
     * tabletrust for local, they should be restored here
     */
#ifdef LOCAL_EVAL
    hbreg = tcp_hreg(breg);
    ebreg = tcp_ebreg(breg);
#endif
  }
#endif /* CONC_COMPL */
  Fail1;
XSB_End_Instr()
/* end of check_complete */
