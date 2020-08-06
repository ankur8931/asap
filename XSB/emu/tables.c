/* File:      tables.c
** Author(s): Ernie Johnson
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
** $Id: tables.c,v 1.107 2013-05-06 21:10:25 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include "debugs/debug_tables.h"
#include "debugs/debug_delay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "heap_xsb.h"
#include "memory_xsb.h"
#include "register.h"
#include "binding.h"
#include "psc_xsb.h"
#include "table_stats.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "error_xsb.h"
#include "flags_xsb.h"
#include "tst_utils.h"
#include "loader_xsb.h" /* for ZOOM_FACTOR, used in stack expansion */
/*#include "subp.h"  for exception_handler, used in stack expansion */
#include "tables.h"
#include "thread_xsb.h"
#include "cinterf.h"

#include "sub_tables_xsb_i.h"
#include "debug_xsb.h"
#include "call_graph_xsb.h" /* for incremental evaluation */
#include "tr_utils.h"  /* for incremental evaluation */
#include "choice.h"

/*=========================================================================
     This file contains the interface functions to the tabling subsystem
  =========================================================================*/


/*=========================================================================*/

/* Engine-Level Tabling Manager Structures
   --------------------------------------- */

/* In the multi-threaded engine, smProdSF and smConsSF will be defined
   as their private analogues */
#ifndef MULTI_THREAD
Structure_Manager smProdSF = SM_InitDecl(subsumptive_producer_sf,
					 SUBGOAL_FRAMES_PER_BLOCK,
					 "Subsumptive Producer Subgoal Frame");

Structure_Manager smConsSF = SM_InitDecl(subsumptive_consumer_sf,
					 SUBGOAL_FRAMES_PER_BLOCK,
					 "Subsumptive Consumer Subgoal Frame");

#define subsumptive_smALN        smALN
#endif

Structure_Manager smVarSF  = SM_InitDecl(variant_subgoal_frame,
					 SUBGOAL_FRAMES_PER_BLOCK,
					 "Variant Subgoal Frame");
Structure_Manager smALN    = SM_InitDecl(AnsListNode, ALNs_PER_BLOCK,
					 "Answer List Node");

/*
 * Allocates and initializes a subgoal frame for a producing subgoal.
 * The TIP field is initialized, fields of the call trie leaf and this
 * subgoal are set to point to one another, while the completion stack
 * frame pointer is set to the next available location (frame) on the
 * stack (but the space is not yet allocated from the stack).  Also,
 * an answer-list node is allocated for pointing to a dummy answer
 * node and inserted into the answer list.  Note that answer sets
 * (answer tries, roots) are lazily created -- not until an answer is
 * generated.  Therefore this field may potentially be NULL, as it is
 * initialized here.  memset() is used so that the remaining fields
 * are initialized to 0/NULL so, in some sense making this macro
 * independent of the number of fields.  

 * In addition, for variant tables, the private/shared field is set.
 * This field is used to determine whether to use shared or private
 * SMs when adding answers.
 */

#if !defined(WIN_NT)
inline 
#endif
VariantSF NewProducerSF(CTXTdeclc BTNptr Leaf,TIFptr TableInfo,unsigned int is_negative_call) {   
    									
  void *pNewSF;							

  if ( IsVariantPredicate(TableInfo) ) {				
#ifdef MULTI_THREAD								
    if (threads_current_sm == PRIVATE_SM) {				
      SM_AllocateStruct(*private_smVarSF,pNewSF);				
      pNewSF = memset(pNewSF,0,sizeof(variant_subgoal_frame));		
      subg_sf_type(pNewSF) = PRIVATE_VARIANT_PRODUCER_SFT;		
    } else {								
      SM_AllocateSharedStruct(smVarSF,pNewSF);				
      pNewSF = memset(pNewSF,0,sizeof(variant_subgoal_frame));		
      subg_sf_type(pNewSF) = SHARED_VARIANT_PRODUCER_SFT;	
    }									
  }
#else
    SM_AllocateStruct(smVarSF,pNewSF);			       
    pNewSF = memset(pNewSF,0,sizeof(variant_subgoal_frame));		
    subg_sf_type(pNewSF) = PRIVATE_VARIANT_PRODUCER_SFT;
  }
#endif
    else {								    
      SM_AllocateStruct(smProdSF,pNewSF);				    
      pNewSF = memset(pNewSF,0,sizeof(subsumptive_producer_sf));		    
      subg_sf_type(pNewSF) = SUBSUMPTIVE_PRODUCER_SFT;			    
    }
/*   subg_deltf_ptr(pNewSF) = NULL; now this is a union type */
   subg_tif_ptr(pNewSF) = TableInfo;					    
   subg_dll_add_sf(pNewSF,TIF_Subgoals(TableInfo),TIF_Subgoals(TableInfo)); 
   subg_leaf_ptr(pNewSF) = Leaf;					    
   CallTrieLeaf_SetSF(Leaf,pNewSF);					    
   subg_ans_list_ptr(pNewSF) = empty_return_handle(pNewSF);		     
   subg_compl_stack_ptr(pNewSF) = openreg - COMPLFRAMESIZE;		    
   INIT_SUBGOAL_CALLSTO_NUMBER(pNewSF);
   ((VariantSF) pNewSF)->visited = 0;
   subg_negative_initial_call(pNewSF) = (unsigned int) is_negative_call;
/* incremental evaluation start */
if((get_incr(TIF_PSC(TableInfo))) &&(IsVariantPredicate(TableInfo))){
  //  sfPrintGoal(stdout,pNewSF,NO);printf(" is marked incr\n");
  subg_callnode_ptr(pNewSF) = makecallnode(pNewSF);                        
 } else{
  //sfPrintGoal(stdout,pNewSF,NO);printf(" is marked NON-incr %s/%d:%d:%u\n",get_name(TIF_PSC(TableInfo)),get_arity(TIF_PSC(TableInfo)),get_incr(TIF_PSC(TableInfo)),TIF_PSC(TableInfo));
  subg_callnode_ptr(pNewSF) = NULL;
 }
/* incremental evaluation end */

   return (VariantSF)pNewSF;						  
}

/*=========================================================================*/

/*
 *			Call Check/Insert Operation
 *			===========================
 */


/*
 * Create an Empty Call Trie, represented by a Basic Trie.  Note that
 * the root of the trie is labelled with the predicate symbol.
 * Assumes that private/shared switch for SMs has been set.
 */

inline static  BTNptr newCallTrie(CTXTdeclc Psc predicate) {

  BTNptr pRoot;

  New_BTN( pRoot, CALL_TRIE_TT, TRIE_ROOT_NT, EncodeTriePSC(predicate),
	   NULL, NULL, NULL );
  return pRoot;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * used in tabletrysinglenoanswers 
 * Note that the call trie of the TIF is not allocated until the first
 * call is entered.  Upon exit, CallLUR_AnsTempl(*results) points to
 * the size of the answer template on the CPS.  See slginsts_xsb_i.h
 * for answer template layout.
 * Assumes that private/shared switch for SMs has been set.
 */

int table_call_search(CTXTdeclc TabledCallInfo *call_info,
 		       CallLookupResults *results) {
  TIFptr tif;

#ifndef MULTI_THREAD
  /* for incremental evaluation begin */ 
  int call_found_flag;
  callnodeptr c;
  VariantSF sf;
  ALNptr pALN;
  BTNptr leaf,Paren;  
  /* for incremental evaluation end */     
#endif

#ifdef CALL_ABSTRACTION
  CallAbsStk_ResetTOS; 
#endif

  tif = CallInfo_TableInfo(*call_info);
  if ( IsNULL(TIF_CallTrie(tif)) )
    TIF_CallTrie(tif) = newCallTrie(CTXTc TIF_PSC(tif));
  if ( IsVariantPredicate(tif) ){
    // Return value for term-depth check.
    if (variant_call_search(CTXTc call_info,results) == XSB_FAILURE) {
      return XSB_FAILURE;
    }
    
#ifndef MULTI_THREAD
    /* incremental evaluation: checking whether falsecount is zero */
    /*
      In call check insert if a call is reinserted and was already
      invalidated (falsecount>0) we mark all the answers deleted and
      make sure this call is treated as a new call (flag=0) 
      
      If falsecount=0 that means the call is not affected then it
      should fetch answers from the earlier answer table as any
      completed call would do.
    */
    call_found_flag=CallLUR_VariantFound(*results);
    Paren=CallLUR_Leaf(*results);

    if(call_found_flag!=0){
      old_call_gl=NULL;
      old_answer_table_gl=NULL;
      
      sf=CallTrieLeaf_GetSF(Paren);
      c=sf->callnode;

      /* TLS: if falsecount == 0 && call_found_flag, we know that call
         is complete.  Otherwise, dfs_outedges would have aborted */

      if(IsNonNULL(c) && (c->falsecount!=0)){
	//	 	  printf("   recomputing (rcomputable = %d) ",c->recomputable);
	//  print_subgoal(stddbg,sf);printf("\n");
	if (c->recomputable == COMPUTE_DEPENDENCIES_FIRST) {
	  lazy_affected = empty_calllist();
	  if ( !dfs_inedges(CTXTc c,  &lazy_affected, CALL_LIST_EVAL) ) {
	    CallLUR_Subsumer(*results) = CallTrieLeaf_GetSF(Paren);
	    return XSB_SPECIAL_RETURN;
	  } /* otherwise if dfs_inedges, treat as falsecount = 0, use completed table */
	}
	else {    /* COMPUTE_DIRECTLY */
	  old_call_gl=c;      
	  call_found_flag=0;  /* treat as a new call */
	  if ( has_answers(sf) ) {
	    pALN = subg_answers(sf);
	    do {
	      leaf=ALN_Answer(pALN);

	      if (!is_conditional_answer(leaf)) {
		BTN_IncrMarking(leaf) = PREVIOUSLY_UNCONDITIONAL;
	      }
		
	      safe_delete_branch(leaf);      
	      pALN = ALN_Next(pALN);
	    } while ( IsNonNULL(pALN) );
	  }
	  c->aln=subg_answers(sf);
	  old_answer_table_gl=sf->ans_root_ptr;
	  //reclaim_incomplete_table_structs(sf);
	  CallTrieLeaf_SetSF(Paren,NULL);            
	}
      }
    }
      CallLUR_Subsumer(*results) = CallTrieLeaf_GetSF(Paren);
      CallLUR_VariantFound(*results) = call_found_flag;
      /* incremental evaluation: end */
#endif /* not MULTI_THREAD (incremental evaluation) */
  }
  else
    subsumptive_call_search(CTXTc call_info,results);
  {
    /*
     * Copy substitution factor from CPS to Heap.  The arrangement of
     * this second subsFact. is similar to that in the CPS: the size
     * of the vector is now at the high end, but the components are
     * still arranged from high mem (first) to low (last) -- see
     * picture at beginning of slginsts_xsb_i.h.  At the return of
     * this function, each cell of the heap s.f. will have the same
     * value as the corresponding cell in the CPS s.g.  This is done
     * so that attvs can be analyzed upon interning answers -- see
     * documentation at the beginning of variant_call_search().
     * 
     * TLS: Copying from the CPS to heap is not good, as 1) the ls may
     * be deallocated more quickly than the heap; 2) it may interfere
     * with gc; and 3) it may interfere with heap / ls expansion.
     * Here, 1) is not a problem because the tabled env will not be
     * deallocated.  2) doesn't seem to be a problem in sliding GC as
     * the heap is compacted but the local stack is not moved; however
     * it does break copying GC.  3) is handled explicitly in our heap
     * expansion routines -- cf glstack_realloc().  Nonetheless, I
     * don't understand the intricies of call subsumption enough to
     * change this around, so we're living with it, and I think we've
     * prevented it from causing a memory problem.
     *
     */
    CPtr tmplt_component, tmplt_var_addr, hrg_addr;
#ifdef CALL_ABSTRACTION
    int size, j,attv_num,abstr_size;  /* call abstraction */
    UNUSED(attv_num);
#else
    int size, j;
#endif

    tmplt_component = CallLUR_AnsTempl(*results);
    size = int_val(*tmplt_component) & 0xffff;
#ifdef CALL_ABSTRACTION
    get_var_and_attv_nums(size, attv_num, abstr_size, int_val(*tmplt_component)); 
    //        printf("done with vcs, size %d attv_num %d abstr_size %d\n",size, attv_num, abstr_size);
    //	   CallLUR_AnsTempl(*results),CallInfo_AnsTempl(*call_info),size);
#endif
    /* expand heap if there's not enough space */
    if ((pb)top_of_localstk < (pb)top_of_heap + size + 	OVERFLOW_MARGIN) {
      /*    if ((pb)top_of_localstk < (pb)top_of_heap + size + 	OVERFLOW_MARGIN + 2*abstr_size) { */
      xsb_abort("{table_call_search} Heap overflow copying answer template");
    }

#ifdef CALL_ABSTRACTION
    //    printf("in TSC %p\n",hreg);                                                                 
    //    print_AbsStack();                                                                            
    // Need to copy abstractions except where we're using a completed table.                         
    if (abstr_size > 0 && (CallLUR_Subsumer(*results) == NULL
                           || !is_completed(CallLUR_Subsumer(*results)))) {
      //      printf("copying abstractions to AT\n");                                                
#ifdef DEBUG_ABSTRACTION
      printf("abstr size %d\n",abstr_size);
      print_AbsStack();
#endif
      copy_abstractions_to_AT(CTXTc hreg,abstr_size);
#ifdef ABSTRACTION_DEBUG
      print_heap_abstraction(hreg,abstr_size);
#endif
      hreg = hreg + 2*abstr_size;
    }
#endif

    /* tmplt_component is CPS address */
    for ( j = size - 1, tmplt_component = tmplt_component + size;
	  j >= 0;
	  j--, tmplt_component-- ) {
      tmplt_var_addr = (CPtr)*tmplt_component;
      //      printf("in TSC, copying AT to heap At[%d]: %p val: %p",
      //		  (size-(j)),tmplt_component,tmplt_var_addr));
      /* Now we are sure that tmplt_var_addr is a var on the heap */
      hrg_addr = hreg+j;
      bld_copy(hrg_addr, (Cell)(tmplt_var_addr));
    }
    hreg += size;
    bld_copy(hreg, cell(CallLUR_AnsTempl(*results)));  // copy size map
    hreg++;
    //    printf("hreg after AT %p\n",hreg);
    /* CallLUR_AnsTemp pointed to the youngest part of AnsTempl, as it
     * was set to SubsFactReg at the end of vcs; reset it to the
     * oldest part of AnsTempl, which will make it =
     * CallInfo_AnsTempl.  Both still point to CPS.  */

    CallLUR_AnsTempl(*results) = CallLUR_AnsTempl(*results) + size + 1;
    //    printf("at end: answer_template %p / %p size %d\n",CallLUR_AnsTempl(*results),
    //   CallInfo_AnsTempl(*call_info),size);
  }
  return XSB_SUCCESS;
}



/* --------------- incremental evaluation --------------- */

int table_call_search_incr(CTXTdeclc TabledCallInfo *call_info,
		       CallLookupResults *results) {

  TIFptr tif;
  BTNptr leaf; callnodeptr cn;

  tif = CallInfo_TableInfo(*call_info);
  if ( IsNULL(TIF_CallTrie(tif)) )
    TIF_CallTrie(tif) = newCallTrie(CTXTc TIF_PSC(tif));

  if ( IsVariantPredicate(tif) ){
    if (variant_call_search(CTXTc call_info,results) == XSB_FAILURE) {
      return XSB_FAILURE;
    }

    leaf=CallLUR_Leaf(*results);
    if (CallLUR_VariantFound(*results)==0){
      /* new call */      
      cn = makecallnode(NULL); 
      BTN_Child(leaf) = (BTNptr)cn;
      callnode_tif_ptr(cn) = tif;
      callnode_leaf_ptr(cn) = leaf;
      initoutedges(CTXTc (callnodeptr)BTN_Child(leaf));
    }
  }
  else
  /* TLS: in principle this should already have been checked, but I'm
     not sure we can count on it */
    xsb_misc_error(CTXTc "Tabled predicate cannot be both incremental and subsumptive",
		get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
  
  {
    /*
     * Copy substitution factor from CPS to Heap.  The arrangement of
     * this second s.f.  is similar to that in the CPS: the
     * size of the vector is now at the high end, but the components
     * are still arranged from high mem (first) to low (last) -- see
     * picture at beginning of slginsts_xsb_i.h.  At the return of
     * this function, each cell of the heap s.f. will have the same
     * value as the corresponding cell in the CPS s.g.  This is done so
     * that attvs can be analyzed upon interning answers -- see
     * documentation at the beginning of variant_call_search().
     */
    /*
|    CPtr tmplt_component, tmplt_var_addr, hrg_addr;
|    int size, j;
|
|    tmplt_component = CallLUR_AnsTempl(*results);
|    size = int_val(*tmplt_component) & 0xffff;
|    xsb_dbgmsg((LOG_TRIE,
|		"done with vcs, answer_template %x\n",tmplt_component));
|
|  
|    if ((pb)top_of_localstk < (pb)top_of_heap + size +
|	OVERFLOW_MARGIN) {
|      xsb_abort("{table_call_search} Heap overflow copying answer template");
|    }
|
|    for ( j = size - 1, tmplt_component = tmplt_component + size;
|	  j >= 0;
|	  j--, tmplt_component-- ) {
|      tmplt_var_addr = (CPtr)*tmplt_component;
|      xsb_dbgmsg((LOG_TRIE,"in TSC, copying AT to heap At[%d]: %x val: %x",
|		  (size-(j)),tmplt_component,tmplt_var_addr));
|  
|      hrg_addr = hreg+j;
|      bld_copy(hrg_addr, (Cell)(tmplt_var_addr));
|    }
|    hreg += size;
|    bld_copy(hreg, cell(CallLUR_AnsTempl(*results)));
|    hreg++;
|    CallLUR_AnsTempl(*results) = CallLUR_AnsTempl(*results) + size + 1;
    */
  }
  return XSB_SUCCESS;
}



/*=========================================================================*/

/*
 *			Answer Check/Insert Operation
 *			=============================
 */


/**********************************************************************

 * Template is a pointer to the first term in the vector, with the
 * elements arranged from high to low memory.
 * Assumes that private/shared switch for SMs has been set.
 * 
 * TLS 12/08.  Now that we may copy delay lists into the table for
 * call subsumption, all untrailing must be done after
 * do_delay_stuff(), rather than within subsumptive_answer_search().
 * Unfortunately, subsumptive_answer_search() uses a different
 * untrailing mechanism than either variant_answer_search() or
 * do_delay_stuff().  As a result, for call subsumption bindings have
 * to be removed through undo_answer_bindings() for do_delay_stuff()
 * and through Trail_Unwind_All (for subsumptive_answer_search()

 ***********************************************************************/

BTNptr table_answer_search(CTXTdeclc VariantSF producer, int size, int attv_num,
			   CPtr templ, xsbBool *is_new) {

  void *answer;
  xsbBool wasFound = TRUE;
  xsbBool hasASI = TRUE;

  if ( IsSubsumptiveProducer(producer) ) {

    //    AnsVarCtr = 0;
    ans_var_pos_reg = hreg++;	/* Leave a cell for functor ret/n (needed in subsumption?) */
    answer =
      subsumptive_answer_search(CTXTc (SubProdSF)producer,size,templ,&wasFound);
    wasFound = !wasFound;
    if ( !wasFound ) {
      ALNptr newALN;
      New_Private_ALN(newALN,answer,NULL);
      SF_AppendNewAnswer(producer,newALN);
    }
    //      fprintf(stddbg, "The answer is new: ");printTriePath(stderr, answer, NO);
    do_delay_stuff(CTXTc (NODEptr)answer, producer, wasFound);

    VarEnumerator_trail_top = (CPtr *)(& VarEnumerator_trail[0]) - 1;
    undo_answer_bindings;
    Trail_Unwind_All;

      *is_new = ! wasFound;
  }
  else {  /* Variant Producer */
    /*
     * We want to save the substitution factor of the answer in the
     * heap, so we have to change variant_answer_search().
     */
    
    ans_var_pos_reg = hreg++;	/* Leave a cell for functor ret/n */

    if (NULL != (answer = variant_answer_search(CTXTc size,attv_num,templ,producer,&wasFound,
						&hasASI))) {
      if (!IsIncrSF(producer)) {
	//	printf("wasFound %d delay %p\n",wasFound,delayreg);
	do_delay_stuff(CTXTc (NODEptr)answer, producer, wasFound);
	*is_new = ! wasFound;
      }
      else {
	//	printf("hasASI %d delay %p\n",hasASI,delayreg);
	do_delay_stuff(CTXTc (NODEptr)answer, producer, (hasASI & wasFound));
	*is_new = ! wasFound;
      }
    }

    *is_new = ! wasFound;
    undo_answer_bindings;

  }
  return (BTNptr)answer;
}

/*=========================================================================*/

/*
 *			    Answer Consumption
 *			    ==================
 */


void table_consume_answer(CTXTdeclc BTNptr answer, int size, int attv_num,
			  CPtr templ, TIFptr predicate) {

  if ( size > 0 ) {
    if ( IsSubsumptivePredicate(predicate) )
      consume_subsumptive_answer(CTXTc answer,size,templ);
    else
      /* this also tracks variables created during unification */
      load_solution_trie(CTXTc size,attv_num,templ,answer);
  }
  else if ( size == 0 ) {
    if ( ! IsEscapeNode(answer) )
      xsb_abort("Size of answer template is 0 but producer contains an "
		"answer\nwith a non-empty substitution!\n");
  }
  else
    xsb_abort("table_consume_answer(): "
	      "Answer template has negative size: %d\n", size);
}

/*=========================================================================*/

/*
 *			   Answer Identification
 *			   =====================
 */


/*
 *  Collects answers from a producer's answer set based on an answer
 *  template and a time stamp value.  Collection is performed to
 *  replenish the answer cache of a properly subsumed subgoal.
 *  Collected answers are placed into the consumer's cache and the time
 *  of this collection is noted.  Returns a pointer to the chain of new
 *  answers found, or NULL if no answers were found.
 */

ALNptr table_identify_relevant_answers(CTXTdeclc SubProdSF prodSF, SubConsSF consSF,
				       CPtr templ) {

  size_t size;
  TimeStamp ts;         /* for selecting answers from subsumer's AnsTbl */

  TSTNptr tstRoot;      /* TS answer trie root of subsumer */
  ALNptr answers;


#ifdef DEBUG_ASSERTIONS
  if ( ((SubProdSF)consSF == prodSF) || (! IsSubsumptiveProducer(prodSF))
       || (! IsProperlySubsumed(consSF)) )
    xsb_abort("Relevant Answer Identification apparently triggered for a "
	      "variant!\nPerhaps SF type is corrupt?");
#endif
  size = int_val(*templ);
  templ--;  /* all templates on the heap, now --lfcastro */
  //  printf("\n"); printAnswerTemplate(stderr,templ,size);
  ts = conssf_timestamp(consSF);
  tstRoot = (TSTNptr)subg_ans_root_ptr(prodSF);

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  NumSubOps_IdentifyRelevantAnswers++;
#endif

  answers = tst_collect_relevant_answers(CTXTc tstRoot,ts,size,templ);
  conssf_timestamp(consSF) = TSTN_TimeStamp(tstRoot);
  if ( IsNonNULL(answers) )
    SF_AppendNewAnswerList(consSF,answers);
  return answers;
}

/*=========================================================================*/

/*
 *                     Table Structure Reclamation
 *                     ===========================
 */

/*
 *  Deallocate all the data structures which become superfluous once
 *  the table has completed.  Currently, this includes the answer list
 *  nodes from the producer (for non-incremental tables), and if
 *  subsumption was used, the TSIs from the answer set along with the
 *  answer list nodes from the subsumed subgoals.  For the producers,
 *  the engine requires that the dummy answer-list node remain, and
 *  that its 'next' field be set to either the constant CON_ANSWERS or
 *  UNCOND_ANSWERS depending on whether there were any conditional
 *  answers in the answer list.  For the subsumed (consumer) subgoals,
 *  the entire answer list, including the dummy, is reclaimed.
 *
 *  For statistical purposes, we check whether the current usage of
 *  these incomplete-table structures are new maximums.  TLS 09/11 --
 *  Now only doing this in NON_OPT_COMPILE
 *
 *  Currently, timestamps from the TSIs are copied back to the TSTNs.
 *  Although not necessary, this method has some advantages.  Foremost,
 *  this field will never contain garbage values, and so we avoid
 *  dangling pointer problems.  Also, maintaining the time stamp values
 *  is beneficial for post-completion analysis of the TSTs and
 *  characteristics of the query evaluation.
 *
 *  Multiple calls to this function is avoided by checking a flag in the
 *  subgoal frame.  See macro reclaim_incomplete_table_structs() which
 *  contains the only reference to this function.
 */

void table_complete_entry(CTXTdeclc VariantSF producerSF) {

  SubConsSF pSF;
  ALNptr pRealAnsList, pALN, tag;
  TSTHTptr ht;

#ifdef NON_OPT_COMPILE  
  TSINptr tsi_entry;
#endif

  dbg_print_subgoal(LOG_STRUCT_MANAGER, stddbg, producerSF);
  xsb_dbgmsg((LOG_STRUCT_MANAGER, " complete... reclaiming structures.\n"));

#ifndef MULTI_THREAD
  if (flags[MAX_USAGE])
    compute_maximum_tablespace_stats(CTXT);
#endif

  /* Reclaim Auxiliary Structures from the TST
     ----------------------------------------- */
  if ( ProducerSubsumesSubgoals(producerSF) &&
       IsNonNULL(subg_ans_root_ptr(producerSF)) )

    for ( ht = TSTRoot_GetHTList(subg_ans_root_ptr(producerSF));
	  IsNonNULL(ht);  ht = TSTHT_InternalLink(ht) ) {

#ifdef NON_OPT_COMPILE
      /*** Put timestamps back into TSTNs ***/
      for ( tsi_entry = TSTHT_IndexHead(ht);  IsNonNULL(tsi_entry);
      	    tsi_entry = TSIN_Next(tsi_entry) )
      	TSTN_TimeStamp(TSIN_TSTNode(tsi_entry)) = TSIN_TimeStamp(tsi_entry);
#endif

      /*** Return TSIN chain to Structure Manager ***/
      if ( IsNULL(TSTHT_IndexTail(ht)) ||
	   IsNonNULL(TSIN_Next(TSTHT_IndexTail(ht))) ||
	   IsNULL(TSTHT_IndexHead(ht)) ||
	   IsNonNULL(TSIN_Prev(TSTHT_IndexHead(ht))) )
	xsb_warn(CTXTc "Malconstructed TSI");

      xsb_dbgmsg((LOG_STRUCT_MANAGER, "  Reclaiming TS Index\n"));
      dbg_smPrint(LOG_STRUCT_MANAGER, smTSIN, "  before chain reclamation");

      /*** Because 'prev' field is first, the tail becomes the list head ***/
      SM_DeallocateStructList(smTSIN,TSTHT_IndexTail(ht),TSTHT_IndexHead(ht));
      TSTHT_IndexHead(ht) = TSTHT_IndexTail(ht) = NULL;

      dbg_smPrint(LOG_STRUCT_MANAGER, smTSIN, "  after chain reclamation");
    }

  subg_visitors(producerSF) = 0;    /* was compl_stack_ptr */
  subg_pos_cons(producerSF) = 0;

  /* incremental  evaluation start */
  /* 
     If a new call is complete we should compare whether its answer
     set is equal to the previous answer set. If no_of_answer is > 0
     we know that not all answers are rederived so the extension has
     changed.  However, if a new answer was added the callnodepointer
     is already marked as changed (see tries.c:
     variant_answer_search). Otherwise, the call is unchanged i.e it
     produced the same set of answers. This information has to be
     propagated to the calls dependent on this call whose falsecount
     needs to be updated (propagate_no_change). Also answers that are
     still marked are deleted as those are not answers of the new
     call.
  */
  if(IsIncrSF(producerSF)){
    callnodeptr pc;  int fewer_true_answers = FALSE;
    producerSF->callnode->recomputable = COMPUTE_DEPENDENCIES_FIRST;
    pc = producerSF->callnode->prev_call;

    if(IsNonNULL(pc)){
      //      printf("has prev call ");print_subgoal(stddbg,producerSF);
      //      printf(" visitors %ld / %p",producerSF->visitors,producerSF);printf("\n");
      
      if ( has_answers(producerSF) ) {
	pALN = pc->aln;
	while(IsNonNULL(pALN)) {
	  if (is_conditional_answer(ALN_Answer(pALN)) 
	      && BTN_IncrMarking(ALN_Answer(pALN)) == PREVIOUSLY_UNCONDITIONAL)
	    fewer_true_answers = TRUE;
	  BTN_IncrMarking(ALN_Answer(pALN)) = 0;
	  if (IsDeletedNode(ALN_Answer(pALN)))
	    delete_branch(CTXTc ALN_Answer(pALN),&subg_ans_root_ptr(producerSF),VARIANT_EVAL_METHOD);
	  pALN = ALN_Next(pALN);
	}
      } else {
	/* There is a memory leak here: I have to delete the answer
	   table here: iterating delete_branch thru the answer trie
	   gives error while deleting the last branch */
	
	producerSF->ans_root_ptr=NULL; 
      }

      /* old calls */
      if (pc->no_of_answers>0 || fewer_true_answers)    // if some previous answers have not been rederived
	pc->changed=1;
      if (pc->changed==0){
	unchanged_call_gl++;
	propagate_no_change(pc); /* defined in call_graph_xsb.c */
      } //else
	//add_callnode(&changed_gl,producerSF->callnode);
    
      producerSF->callnode->prev_call=NULL;
    	
      deallocate_previous_call(pc);
            
    } // else /* newly added calls */
      //      add_callnode(&changed_gl,producerSF->callnode);
    
    if ( has_answers(producerSF) ) {
      pALN = pRealAnsList = subg_answers(producerSF);
      tag = UNCOND_ANSWERS;
      do {
	if ( is_conditional_answer(ALN_Answer(pALN)) ) {
	  tag = COND_ANSWERS;
	}
	if (hasALNtag(ALN_Answer(pALN))) { /* reset to null */
	  Child(ALN_Answer(pALN)) = NULL;
	}
	pALN = ALN_Next(pALN);
      } while ( IsNonNULL(pALN) );
    }
  }    /* incremental  evaluation end */
  else
  /* ------------------------------

     Reclaim Producer's Answer List.  In general, we want to reclaim
     the answer list, but let subg_answers(SF) indicate whether a goal
     has some answer and whether that answer is conditional or not. 

     TLS: July/12.  There was a problem with slg_not using has_answers
     when all answers for a subgoal have been removed via delay
     (surprisingly this case had not been tested).  So we only want an
     XXX_ANSWERS tag if the subg_ans_root_ptr is null; otherwise we
     handle the reclamantion in the else associated with this if...
  */
    if ( has_answers(producerSF) && subg_ans_root_ptr(producerSF) != NULL) {
      pALN = pRealAnsList = subg_answers(producerSF);
    tag = UNCOND_ANSWERS;
    do {
      if ( is_conditional_answer(ALN_Answer(pALN)) ) {
	tag = COND_ANSWERS;
      }
      if (hasALNtag(ALN_Answer(pALN))) { /* reset to null */
	Child(ALN_Answer(pALN)) = NULL;
      }
      pALN = ALN_Next(pALN);
    } while ( IsNonNULL(pALN) );
#ifndef CONC_COMPL
    subg_answers(producerSF) = tag;
#else
    subg_tag(producerSF) = tag;
#endif

      xsb_dbgmsg((LOG_STRUCT_MANAGER, "  Reclaiming ALN chain for subgoal\n"));
      dbg_smPrint(LOG_STRUCT_MANAGER, smALN, "  before chain reclamation");

    if ( IsNULL(subg_ans_list_tail(producerSF)) ||
	 IsNonNULL(ALN_Next(subg_ans_list_tail(producerSF))) )
      xsb_abort("Answer-List exception: Tail pointer incorrectly maintained");
#ifdef MULTI_THREAD
    if (IsSharedSF(producerSF)) {				
#ifndef CONC_COMPL
      /* Can't deallocate answer return list for CONC_COMPL shared tables */
      SM_DeallocateSharedStructList(smALN,pRealAnsList,
			      subg_ans_list_tail(producerSF));
      subg_ans_list_tail(producerSF) = NULL;
#endif
    } else {
      SM_DeallocateStructList(*private_smALN,pRealAnsList,
			      subg_ans_list_tail(producerSF));
      subg_ans_list_tail(producerSF) = NULL;
    }
#else
    SM_DeallocateStructList(smALN,pRealAnsList,
			      subg_ans_list_tail(producerSF));
    subg_ans_list_tail(producerSF) = NULL;
#endif

    dbg_smPrint(LOG_STRUCT_MANAGER, smALN, "  after chain reclamation");
    }
    else {
      if ( has_answers(producerSF) && subg_ans_root_ptr(producerSF) == NULL) {
	SM_DeallocateStructList(smALN,ALN_Next(subg_ans_list_ptr(producerSF)),
				subg_ans_list_tail(producerSF));
	subg_ans_list_tail(producerSF) = NULL;
	subg_answers(producerSF) = NULL;
      }
    }

  /* Process its Consumers
     --------------------- */
  if ( IsSubsumptiveProducer(producerSF) ) {
    pSF = subg_consumers(producerSF);


    if (IsNonNULL(pSF)) {
      xsb_dbgmsg((LOG_STRUCT_MANAGER, 
		 "Reclaiming structures from consumers of "));
      dbg_print_subgoal(LOG_STRUCT_MANAGER, stddbg, producerSF);
      xsb_dbgmsg((LOG_STRUCT_MANAGER, "\n"));
    }

    while ( IsNonNULL(pSF) ) {

      xsb_dbgmsg((LOG_STRUCT_MANAGER, "  Reclaiming ALN chain for consumer\n"));
      dbg_smPrint(LOG_STRUCT_MANAGER, smALN, "  before chain reclamation");

      if ( has_answers(pSF) )    /* real answers exist */
	SM_DeallocateStructList(subsumptive_smALN,subg_ans_list_ptr(pSF),
				subg_ans_list_tail(pSF))
      else
	SM_DeallocateStruct(subsumptive_smALN,subg_ans_list_ptr(pSF))
      dbg_smPrint(LOG_STRUCT_MANAGER, smALN, "  after chain reclamation");

      // This sets deltf to NULL which is what we want.
      subg_ans_list_ptr(pSF) = subg_ans_list_tail(pSF) = NULL;
      pSF = conssf_consumers(pSF);
    }
  }

  xsb_dbgmsg((LOG_STRUCT_MANAGER, "Subgoal structure-reclamation complete!\n"));
}

/*-------------------------------------------------------------------------*/

/* TLS: I made this into a function and moved it from tab_structs.h to
   make the tif_list conditional a little more transparent, and to
   facilitate debugging. */

inline TIFptr New_TIF(CTXTdeclc Psc pPSC) {	   
  TIFptr pTIF;

   pTIF = (TIFptr)mem_alloc(sizeof(TableInfoFrame),TABLE_SPACE);	
   if ( IsNULL(pTIF) )							
     xsb_resource_error_nopred("memory", "Ran out of memory in allocation of TableInfoFrame");	
   TIF_PSC(pTIF) = pPSC;						
   if (get_tabled(pPSC)==T_TABLED) {					
     TIF_EvalMethod(pTIF) = (TabledEvalMethod)pflags[TABLING_METHOD];	
     if (TIF_EvalMethod(pTIF) == VARIANT_EVAL_METHOD)			
       set_tabled(pPSC,T_TABLED_VAR);					
     else set_tabled(pPSC,T_TABLED_SUB);				
   }									
   else if (get_tabled(pPSC)==T_TABLED_VAR) 				
      TIF_EvalMethod(pTIF) = VARIANT_EVAL_METHOD;			
   else if (get_tabled(pPSC)==T_TABLED_SUB) 				
     TIF_EvalMethod(pTIF) = SUBSUMPTIVE_EVAL_METHOD;			
   else {			
     /* incremental evaluation */
     if(get_nonincremental(pPSC))					
       xsb_warn(CTXTc "%s/%d not identified as tabled in .xwam file, Recompile (variant assumed)", \
		get_name(pPSC),get_arity(pPSC));				
      TIF_EvalMethod(pTIF) = VARIANT_EVAL_METHOD;			
      set_tabled(pPSC,T_TABLED_VAR);					
   }									
   TIF_CallTrie(pTIF) = NULL;						
   TIF_Mark(pTIF) = 0;                                                  
   TIF_Visited(pTIF) = 0; 
   TIF_Interning(pTIF) = 0;  /* for now; figure out how to set... DSWDSW */
   TIF_SkipForestLog(pTIF) = 0;
   TIF_DelTF(pTIF) = NULL;						
   TIF_Subgoals(pTIF) = NULL;						
   TIF_NextTIF(pTIF) = NULL;						
   TIF_SubgoalDepth(pTIF) = 0;						
   TIF_AnswerDepth(pTIF) = 0;						
#ifdef MULTI_THREAD
   /* The call trie lock is also initialized for private TIFs,
      just in case they ever change to shared */
   pthread_mutex_init( &TIF_CALL_TRIE_LOCK(pTIF), NULL );
#ifdef SHARED_COMPL_TABLES
   pthread_cond_init( &TIF_ComplCond(pTIF), NULL );
#endif
   if (get_shared(pPSC)) {
     SYS_MUTEX_LOCK( MUTEX_TABLE );				
     if ( IsNonNULL(tif_list.last) )					
       TIF_NextTIF(tif_list.last) = pTIF;				      
     else									
       tif_list.first = pTIF;						
     tif_list.last = pTIF;						
     SYS_MUTEX_UNLOCK( MUTEX_TABLE );				
   } else {
     if ( IsNonNULL(private_tif_list.last) )					
       TIF_NextTIF(private_tif_list.last) = pTIF;			     
     else									
       private_tif_list.first = pTIF;						
     private_tif_list.last = pTIF;					      
   }
#else
   if ( IsNonNULL(tif_list.last) )					
     TIF_NextTIF(tif_list.last) = pTIF;				      
   else									
     tif_list.first = pTIF;						
   tif_list.last = pTIF;						
#endif
   return pTIF;
}

/* Need to add ALN to SF, and pointer to leaf of call trie.
   not yet used (01/09)  */
VariantSF tnotNewSubConsSF(CTXTdeclc BTNptr Leaf,TIFptr TableInfo,VariantSF producer) {	
								
   void *pNewSF;						
								
   SM_AllocateStruct(smConsSF,pNewSF);				
   pNewSF = memset(pNewSF,0,sizeof(subsumptive_consumer_sf));	
   subg_sf_type(pNewSF) = SUBSUMED_CONSUMER_SFT;		
   subg_tif_ptr(pNewSF) = TableInfo;				
   subg_leaf_ptr(pNewSF) = Leaf;				
   CallTrieLeaf_SetSF(Leaf,pNewSF);				
   conssf_producer(pNewSF) = (SubProdSF)producer;		
   //   if ( ! producerSubsumesSubgoals(producer) )			
   //     tstCreateTSIs_handle((TSTNptr)subg_ans_root_ptr(producer));		
   subg_ans_list_ptr(pNewSF) = empty_return_handle(pNewSF);		
   //   conssf_timestamp(pNewSF) = CONSUMER_SF_INITIAL_TS;		
   conssf_timestamp(pNewSF) = 0;		
   conssf_consumers(pNewSF) = subg_consumers(producer);		
   subg_consumers(producer) = (SubConsSF)pNewSF;		
   return pNewSF;					
}

/*=========================================================================*/

