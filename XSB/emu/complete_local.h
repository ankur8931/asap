/* File:      complete_local.h
** Author(s): Juliana Freire, Kostis Sagonas, Terry Swift, Luis Castro
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-2001
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
** $Id: complete_local.h,v 1.42 2013-04-19 13:46:34 tswift Exp $
**
*/
#ifndef __COMPLETE_LOCAL_H__
#define __COMPLETE_LOCAL_H__

#ifdef LOCAL_EVAL
void makeConsumerFromGenerator(CTXTdeclc VariantSF producer_sf)
{
  xsb_dbgmsg((LOG_COMPLETION,
	      "Transforming %x from a generator to consumer (prev %x)\n",
	      breg,nlcp_prevbreg(breg)));
  nlcp_trie_return(breg) = subg_ans_list_ptr(producer_sf);
  nlcp_pcreg(breg) = (pb) &answer_return_inst;
  nlcp_prevlookup(breg) = subg_pos_cons(producer_sf);
  subg_pos_cons(producer_sf) = breg;
}
#endif /* LOCAL */

#ifdef PROFILE
#define ProfileLeader \
  { \
    int num_subgoals = 0; \
    int num_completed = 0; \
    int num_consumers_in_ascc = 0; \
    int num_compl_susps_in_ascc = 0; \
    int leader_level; \
    CPtr profile_CSF = cs_ptr; \
    VariantSF prof_compl_subg; \
\
    leader_level = compl_level(profile_CSF); \
\
    /* check from leader up to the youngest subgoal */ \
    while (profile_CSF >= openreg) { \
      num_subgoals++; \
      prof_compl_subg = compl_subgoal_ptr(profile_CSF); \
      if (is_completed(prof_compl_subg)) { /* this was early completed */ \
	num_completed++; \
      } \
      else { \
	CPtr nsf; \
	nsf = subg_pos_cons(prof_compl_subg); \
	while (nsf != NULL) { \
	  num_consumers_in_ascc++; \
	  nsf = nlcp_prevlookup(nsf); \
	  } \
	nsf = subg_compl_susp_ptr(prof_compl_subg); \
	while (nsf != NULL) { \
	  num_compl_susps_in_ascc++; \
	  nsf = csf_prevcsf(nsf); \
	  } \
      } \
      profile_CSF = next_compl_frame(profile_CSF); \
    } \
    if (num_subgoals > max_subgoals) { max_subgoals = num_subgoals; } \
    if (num_completed > max_completed) { max_completed = num_completed; } \
    if (num_consumers_in_ascc > max_consumers_in_ascc) { \
      max_consumers_in_ascc = num_consumers_in_ascc; \
    } \
    if (num_compl_susps_in_ascc > max_compl_susps_in_ascc) { \
      max_compl_susps_in_ascc = num_compl_susps_in_ascc; \
    } \
    if (pflags[PROFFLAG] > 2) { \
      fprintf(stdmsg, \
              "p(lev(%d),lead(%d),subg(%d),ec(%d),cons(%d),cs(%d)).\n", \
	      level_num,leader_level,num_subgoals,num_completed, \
	     num_consumers_in_ascc,num_compl_susps_in_ascc); \
    } \
  }
#else
#define ProfileLeader
#endif



/* The following function writes the SDG to stddbg.  The SDG is
written every "std_sample_rate" times the computation determines that
it is checking completion for a leader.  If so, the opentable stack is
traversed along with the various lists hanging off of the subgoal
frames.  For each edge in the SDG a Prolog-readible fact is written

edge(sdg_check_num,Type,Root_subgoal,Selected_tabled_subgoal)

sdg_check_num is the number of the check.  Using sdg_check_num one can
get a somewhat dynamic view of how the SDG changes over the
computation.

Type = 1 if it is a positive link (consumer choice point)
Type = -1 if it is a negative link to a completion suspension choice point
Type = 2 if it is the first call (generator cp is used here).

Right now, there isnt a good way of determining whether the first call
of a tabled predicate is positive or negative.  So its best to table
tnot/1 in tables.P if you want to keep full track of signs */

#ifdef PROFILE
void print_sdg_edge(int check_num,int sign,VariantSF fromsf,VariantSF tosf)
{
  fprintf(stddbg,"edge(%d,%d,",check_num,sign);
  print_subgoal(stddbg,fromsf); fprintf(stddbg,",");
  print_subgoal(stddbg,tosf); fprintf(stddbg,").\n");
}

void SpitOutGraph(CPtr cs_ptr)
{
  CPtr tmp_openreg = cs_ptr;
  CPtr nsf;
  int num_subgoals = 0;
  VariantSF prof_compl_subg;

  sdg_check_num++;
  if (sdg_sample_rate > 0 && (sdg_check_num % sdg_sample_rate == 0)) {
    printf(" /* now checking: %d */ \n",sdg_check_num);
    while (tmp_openreg < COMPLSTACKBOTTOM) {
      num_subgoals++;
      prof_compl_subg = compl_subgoal_ptr(tmp_openreg);

      fprintf(stddbg,"   /* ");
      print_subgoal(stddbg, prof_compl_subg);
      fprintf(stddbg," */ \n");

      if (tcp_ptcp(subg_cp_ptr(prof_compl_subg)) != NULL ) {
      print_sdg_edge(sdg_check_num,2,
		     tcp_ptcp(subg_cp_ptr(prof_compl_subg)),
		     prof_compl_subg);
      } else {
      }

      nsf = subg_pos_cons(prof_compl_subg);
      while (nsf != NULL) {

	print_sdg_edge(sdg_check_num,1,nlcp_ptcp(nsf),prof_compl_subg);

	nsf = nlcp_prevlookup(nsf);
      }

      nsf = subg_compl_susp_ptr(prof_compl_subg);
      while (nsf != NULL) {

	print_sdg_edge(sdg_check_num,-1,csf_ptcp(nsf),prof_compl_subg);

	nsf = csf_prevcsf(nsf);
      }

      tmp_openreg = prev_compl_frame(tmp_openreg);
    }
    /*    printf("doing checking %d %d \n",sdg_check_num,num_subgoals); */
  }
}
#else
#define SpitOutGraph
#endif

#ifdef LOCAL_EVAL
#define DisposeOfComplSusp(subgoal) \
        subg_compl_susp_ptr(subgoal) = NULL
#define DeleteCSF(nsf) \
        csf_prevcsf(nsf)

/* TLS: A pass has been made through the CSF chain to remove those
 * whose root subgoal was early completed.  So what we do now is to
 * reset the hreg and breg of each.  Frankly I don't think that this
 * step should be needed if the freeze registers are not unset until
 * final completion, and the test suite passes if the alternate form
 * (below) is used.  I'll wait until the MT engine stabilizes until I
 * make the change, though.
 *
 * 8/05 Took out lines
 * cs_pcreg(nsftmp) = (pb) &resume_compl_suspension_inst;
 * as these should be unnecessary, and appears to be an artifact from old code
 */
#define ResumeCSFs() \
{ \
  CPtr nsftmp; \
  if (!cur_breg) { \
    cur_breg = cc_tbreg = nsf; \
  } else { \
    csf_prevcsf(cur_breg) = nsf; \
    cur_breg = nsf; \
  } \
  for (nsftmp = cur_breg; csf_prevcsf(nsftmp); nsftmp = csf_prevcsf(nsftmp)) {\
    cs_hreg(nsftmp) = hreg; \
    cs_ebreg(nsftmp) = ebreg; \
  } \
  cs_hreg(nsftmp) = hreg; \
  cs_ebreg(nsftmp) = ebreg; \
  csf_prevcsf(nsftmp) = breg; \
  cur_breg = nsftmp; \
  subg_compl_susp_ptr(compl_subg) = NULL; \
}

/* TLS: alternate form, that does not need to reset the hreg/ebreg.
define ResumeCSFs()				\
{ \
  CPtr nsftmp; \
  if (!cur_breg) { \
    cur_breg = cc_tbreg = nsf; \
  } else { \
    csf_prevcsf(cur_breg) = nsf; \
    cur_breg = nsf; \
  } \
  for (nsftmp = cur_breg; csf_prevcsf(nsftmp); nsftmp = csf_prevcsf(nsftmp)) {\
  } \
  csf_prevcsf(nsftmp) = breg; \
  cur_breg = nsftmp; \
  subg_compl_susp_ptr(compl_subg) = NULL; \
}
*/

static inline CPtr ProcessSuspensionFrames(CTXTdeclc CPtr cc_tbreg_in,
					   CPtr cs_ptr)
{
  CPtr ComplStkFrame = cs_ptr;
  VariantSF compl_subg;
  CPtr cc_tbreg = cc_tbreg_in;
  CPtr cur_breg = NULL; /* tail of chain of nsf's; used in ResumeCSFs */

  /* check from leader up to the youngest subgoal */
  while (ComplStkFrame >= openreg) {
    compl_subg = compl_subgoal_ptr(ComplStkFrame);
    /* TLS: Explanation for the dull-witted (i.e. me).  If compl_subg
     * is early completed, this means it has an unconditional answer.
     * Since a completion suspension is set up only for ground
     * negative calls, this means that the compl suspension will fail,
     * so we just dispose of it.  BTW, this could also be done at
     * early completion (ans-return), though there's no big advantage
     * either way.
     */
    if (is_completed(compl_subg)) { /* this was early completed */
      DisposeOfComplSusp(compl_subg); /* set SGF pointer to null */
    }
    else { /* if not early completed */
      CPtr nsf;
      if ((nsf = subg_compl_susp_ptr(compl_subg)) != NULL) {
	CPtr p, prev_nsf = NULL;
	/* check each suspension frame for appropriate action: if
	 * their root subgoals are completed these completion
	 * suspensions are irrelevant, so skip over them; o/w keep
	 * them on chain, delay them, and let simplification take care
	 * of the rest
	*/
	while (nsf) {
	  if ((p = csf_ptcp(nsf)) != NULL) {
	    if (is_completed(p)) {
	      if (!prev_nsf) { /* deleting the first susp is special */
		nsf = subg_compl_susp_ptr(compl_subg) = DeleteCSF(nsf);
	      }
	      else {
		nsf = csf_prevcsf(prev_nsf) = DeleteCSF(nsf);
	      }
	    }
	    else { /* this completion suspension will be delayed */
	      mark_delayed(ComplStkFrame, subg_compl_stack_ptr(p), nsf);
	      prev_nsf = nsf;
	      nsf = csf_prevcsf(nsf);
	    }
	  }
	} /* while */

	if ((nsf = subg_compl_susp_ptr(compl_subg))) {
	  ResumeCSFs();
	}
      } /* if there are completion suspensions */
    } /* else if not early completed */
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  } /* while - for each subg in compl stack */

  return cc_tbreg;
}

static inline void CompleteSimplifyAndReclaim(CTXTdeclc CPtr cs_ptr)
{
  VariantSF compl_subg;
  SubConsSF pCons;
  CPtr ComplStkFrame = cs_ptr;
  int simplification_required = 0;

  //printf("Child = %p\n",Child(ALN_Answer(subg_ans_list_ptr(compl_subgoal_ptr(ComplStkFrame)))));

  /* mark all SCC as completed and do simplification also, reclaim
     space for all but the leader */

  while (ComplStkFrame >= openreg) {
    if (neg_simplif_possible(compl_subgoal_ptr(ComplStkFrame))) {
      flags[SIMPLIFICATION_DONE] = 1;
      simplification_required = 1;
      break;
    }
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  }
  ComplStkFrame = cs_ptr;

  if (simplification_required) { // set all (non-ec'ed) to needing ac
    while (ComplStkFrame >= openreg) {
      compl_subg = compl_subgoal_ptr(ComplStkFrame);
      if (!subg_is_completed(compl_subg) && subg_ans_root_ptr(compl_subg)) { // not early completed, so
	//	printf("set AC for (%d): ",(Integer)compl_subg); print_subgoal(stddbg,compl_subg); printf("\n");
	subg_needs_answer_completion(compl_subg);
      }
      ComplStkFrame = next_compl_frame(ComplStkFrame);
    }
  ComplStkFrame = cs_ptr;
  }

  while (ComplStkFrame >= openreg) {
    compl_subg = compl_subgoal_ptr(ComplStkFrame);
    mark_as_completed(compl_subg);
    if (flags[CTRACE_CALLS] && !subg_forest_log_off(compl_subg))  { 
      sprint_subgoal(CTXTc forest_log_buffer_1,0,compl_subg);     
      fprintf(fview_ptr,"cmp(%s,%d,%d).\n",forest_log_buffer_1->fl_buffer,
	      compl_level(ComplStkFrame),ctrace_ctr++);
      //      file_print_tables("CompleteSimplifyAndReclaim",ctrace_ctr);
  }

   if (ProducerSubsumesSubgoals(compl_subg)) {
      //      fprintf(stddbg, "Producer:\n  ");
      //      sfPrintGoal(CTXTc stddbg, (VariantSF)compl_subg, YES);
      //      fprintf(stddbg, "\nConsumers:\n");
      for ( pCons = subg_consumers(compl_subg);  IsNonNULL(pCons);
   	    pCons = conssf_consumers(pCons) ) {
	//	fprintf(stddbg, "  ");
	//	sfPrintGoal(CTXTc stddbg, (VariantSF)pCons, YES);
	//	fprintf(stddbg,": fails %d, nde %p\n",subgoal_fails(pCons),subg_nde_list(pCons));
	//	printAnswerList(CTXTc stddbg, subg_ans_list_ptr(pCons));
	//	fprintf(stddbg,"deleted %x, unique %x\n",
	//		IsDeletedNode(ALN_Answer(subg_ans_list_ptr(pCons))),
		  //		ALN_Next(subg_ans_list_ptr(pCons)));
	if (subsumed_neg_simplif_possible(pCons)) {
	  //	  printf("initiating simpl from compl "),print_subgoal(stddbg,pCons);printf("\n");
	  simplify_neg_fails(CTXTc (VariantSF) pCons);
	}
      }
    }
    if (neg_simplif_possible(compl_subg)) {
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      simplify_neg_fails(CTXTc compl_subg);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  } /* while */

  /* TLS: placemarker for development.  This function should happen
   *after* simplification and *before* removal of answer lists, which
   *is useful for traversing dependency graphs. */

  //  /*  remove_unfounded_set(cs_ptr); */

  // TLS: answer_completion was ifdeffed out, so I'm commenting it out here for now.
  //  if (flags[ANSWER_COMPLETION]) {
    // TLS: took out following line as SCC_has_delayed is not otherwise defined.
    // The var is probably part of the heuristics and should be in the thread context
    //	  if ((SCC_has_delayed) || (FALSE) /*  */) {
    //		  answer_completion(CTXTc cs_ptr);
		  //%	  }
    //  }

  /* reclaim all answer lists, but the one for the leader */
  ComplStkFrame = next_compl_frame(cs_ptr);
  while (ComplStkFrame >= openreg) {
    compl_subg = compl_subgoal_ptr(ComplStkFrame);
    reclaim_incomplete_table_structs(compl_subg);
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  } /* while */

#ifdef SHARED_COMPL_TABLES
  /* now wake up all threads suspended on tables just completed
     including leader */
  pthread_mutex_lock( &completing_mut );
  ComplStkFrame = cs_ptr;
  while (ComplStkFrame >= openreg) {
    compl_subg = compl_subgoal_ptr(ComplStkFrame);
    if ( IsSharedSF(compl_subg) )
       pthread_cond_broadcast( &TIF_ComplCond(subg_tif_ptr(compl_subg)) );
    ComplStkFrame = next_compl_frame(ComplStkFrame);
  } /* while */
  pthread_mutex_unlock( &completing_mut );
#endif
}

static inline void SetupReturnFromLeader(CTXTdeclc CPtr orig_breg, CPtr cs_ptr,
					 VariantSF subgoal)
{
  CPtr answer_template;
  int template_size, attv_num;
#ifdef CALL_ABSTRACTION
  int abstr_size;
#endif  
  Integer tmp;

  switch_envs(orig_breg);
  /* check where this brings the stacks, that will determine how
     much can be reclaimed if there are answers to be returned */
  ptcpreg = tcp_ptcp(orig_breg);
  delayreg = tcp_pdreg(orig_breg);
  restore_some_wamregs(orig_breg, ereg);
  /* restore_trail_condition_registers - because success path
   * will be followed
   */
  /* TLS: provisional change for strict_po 9/30/10 */
  //    ebreg = cp_ebreg(tcp_prevbreg(orig_breg));
  //    hbreg = cp_hreg(tcp_prevbreg(orig_breg));
      ebreg = cp_ebreg(orig_breg);
      hbreg = cp_hreg(orig_breg);
  subg_pos_cons(subgoal) = 0;

  /* reclaim stacks, including leader */
  openreg = prev_compl_frame(cs_ptr);
  reclaim_stacks(orig_breg);
  answer_template = tcp_template(breg);
  tmp = int_val(cell(answer_template));
#ifdef CALL_ABSTRACTION
  /* As when tabletry calls a completed table, we simply perform the
     post-unification before branching into the trie. The trie
     instructions will fail if the call doesn't unify -- the
     implementation of call subsumption has already ensured that this
     failure will occur correctly. */
  get_var_and_attv_nums(template_size, attv_num, abstr_size, tmp);
  answer_template = answer_template - template_size;
#ifdef ABSTRACTION_DEBUG
  printf("compl AT %p ABS %p\n",answer_template,answer_template-(2*abstr_size));
  print_heap_abstraction(answer_template-(2*abstr_size),abstr_size);
  printAnswerTemplate(stddbg, answer_template+template_size, template_size);
#endif
  unify_abstractions_from_AT(CTXTc (answer_template-(2*abstr_size)),abstr_size);
#ifdef ABSTRACTION_DEBUG
  printAnswerTemplate(stddbg, answer_template+template_size, template_size);
  print_registers(stddbg,TIF_PSC(subg_tif_ptr(subgoal)),20); 
#endif
#else
  get_var_and_attv_nums(template_size, attv_num, tmp);
  answer_template = answer_template - template_size;
#endif

  /* Now `answer_template' points to the mth term */
  /* Initialize trieinstr_vars[] as the attvs in the call. */
  trieinstr_vars_num = -1;
  if (attv_num > 0) {
    CPtr cptr;
    for (cptr = answer_template + template_size - 1;
	 cptr >= answer_template; cptr--) {
      if (isattv(cell(cptr)))
	trieinstr_vars[++trieinstr_vars_num] = (CPtr) cell(cptr);
    }
    /* now trieinstr_vars_num should be attv_num - 1 */
  }

  trieinstr_unif_stkptr = trieinstr_unif_stk - 1;
  for (tmp = 0; tmp < template_size; tmp++) {
    CPtr cptr = answer_template;
    push_trieinstr_unif_stk(*cptr);
    answer_template++;
  }
  /* backtrack to prev tabled subgoal after returning answers */
  breg = tcp_prevbreg(orig_breg);
  delay_it = 1; /* run delay lists, don't construct them */
}

#endif /* LOCAL_EVAL */
#endif /* __COMPLETE_LOCAL_H__ */
