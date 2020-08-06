/* File:      sub_tables_xsb_i.h
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
** $Id: sub_tables_xsb_i.h,v 1.28 2011-10-16 19:20:34 tswift Exp $
** 
*/


#include "tst_aux.h"
#include "deref.h"

extern void print_TermStack();
extern void print_AbsStack();

/*=========================================================================*/

/*
 *		  Subsumptive Call Check/Insert Operation
 *		  =======================================
 */

/*-------------------------------------------------------------------------*/

/*
 * Answer Template Creation
 * ------------------------
 * There are three ways that the answer template may be created during a
 * Subsumptive Call Check/Insert Operation:
 * (1) If a producing table entry is found during the lookup of a call,
 *     then the template appears in the first several entries of the
 *     TrieVarBindings[] array.
 * (2) If a subsuming, but non-producing, entry is found during call
 *     lookup, then the template for the call must be reconstructed
 *     based upon the producer associated with the found entry.
 * (3) If no subsuming entry is found, then the call is inserted into
 *     the trie, and the template exists on the (trie-specific) Trail.
 * See the file slginsts_xsb_i.h for the layout of the answer template.
 */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Used when this call finds a subsuming call that is a producer */

inline static  CPtr extract_template_from_lookup(CTXTdeclc CPtr ans_tmplt) {

  int i;

  i = 0;
  while ( TrieVarBindings[i] != (Cell) (& TrieVarBindings[i]) )
    *ans_tmplt-- = TrieVarBindings[i++];
  //  *ans_tmplt = makeint(i);
#ifdef CALL_ABSTRACTION
   *ans_tmplt = encode_ansTempl_ctrs(0,callAbsStk_index,i);   // no attvars   
#else
   *ans_tmplt = encode_ansTempl_ctrs(0,i);   // no attvars
#endif
  return ans_tmplt;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Used when this call finds a subsuming call that is itself consumed */

inline static
CPtr reconstruct_template_for_producer(CTXTdeclc TabledCallInfo *call_info,
				       SubProdSF subsumer, CPtr ans_tmplt) {

  int sizeAnsTmplt;
  Cell subterm, symbol;

  /*
   * Store the symbols along the path of the more general call.
   */
  SymbolStack_ResetTOS;
  SymbolStack_PushPath(subg_leaf_ptr(subsumer));

  /*
   * Push the arguments of the subsumed call.
   */
  TermStack_ResetTOS;
  TermStack_PushLowToHighVector(CallInfo_Arguments(*call_info),
			        CallInfo_CallArity(*call_info));

  /*
   * Create the answer template while we process.  Since we know we have a
   * more general subsuming call, we can greatly simplify the "matching"
   * process: we know we either have exact matches of non-variable symbols
   * or a variable paired with some subterm of the current call.
   */
  sizeAnsTmplt = 0;
  while ( ! TermStack_IsEmpty ) {
    TermStack_Pop(subterm);
    XSB_Deref(subterm);
    SymbolStack_Pop(symbol);
    if ( IsTrieVar(symbol) && IsNewTrieVar(symbol) ) {
      *ans_tmplt-- = subterm;
      sizeAnsTmplt++;
    }
    else if ( IsTrieFunctor(symbol) )
      TermStack_PushFunctorArgs(subterm)
    else if ( IsTrieList(symbol) )
      TermStack_PushListArgs(subterm)
  }
#ifdef CALL_ABSTRACTION
  //  *ans_tmplt = makeint(sizeAnsTmplt);
  *ans_tmplt = encode_ansTempl_ctrs(0,callAbsStk_index,sizeAnsTmplt);  // no attvars    
#else
  *ans_tmplt = encode_ansTempl_ctrs(0,sizeAnsTmplt);  // no attvars
#endif
  return ans_tmplt;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Call is new -- no subsumer. */

inline static  CPtr extract_template_from_insertion(CTXTdeclc CPtr ans_tmplt) {

  int i;

  i = 0;
  while ( i < (int) Trail_NumBindings )
    *ans_tmplt-- = (Cell)Trail_Base[i++];
  //  *ans_tmplt = makeint(i);
#ifdef CALL_ABSTRACTION
  *ans_tmplt = encode_ansTempl_ctrs(0,callAbsStk_index,i);   // no attvars      
#else
  *ans_tmplt = encode_ansTempl_ctrs(0,i);   // no attvars
#endif
  return ans_tmplt;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
inline static void  init_termstack_for_subsumptive_check_ins(CTXTdeclc CPtr pVectorLow,int Magnitude) {

  int i, numElements;
  CPtr pElement;
  Cell newElement;

  //   printf("initting termstack %d\n",Magnitude);                                                                        
  numElements = Magnitude;
  pElement = pVectorLow + numElements;
  DynStk_ExpandIfOverflow(tstTermStack,numElements);
  for (i = 0; i < numElements; i++) {
    pElement--;
    //     printf("*pElement %x\n",*pElement);                                                                             
    XSB_Deref(*pElement);
    if (isattv(*pElement)) {
      /* Abstract out attributed variables */
      newElement = (Cell) hreg;new_heap_free(hreg);
      //       printf("newElement %p @newElement %x\n",newElement,*(CPtr)newElement);                                      
      push_AbsStk(*pElement,newElement);

      *pElement = newElement;
      TermStack_BlindPush(newElement);
      //       *pElement = newElement;                                                                                     
    }
    else {TermStack_BlindPush(*pElement);
    }
  }
  //   print_TermStack();                                                                                                  
  //   print_AbsStack();                                                                                                   
  //   printf("out of init_termstack_for_subsumptive_ci\n");                                                               
}

/*-------------------------------------------------------------------------*/

/*
 * Subsumptive Call Check/Insert
 * -----------------------------
 * Look for a subsuming call in the table and conditionally create an
 * entry for the given one.  An entry for the call is created when
 * either
 *   1) no subsuming call exists in the table, or
 *   2) a more general (not variant) call exists, but its table is
 *      incomplete.
 * Therefore, no entry is created when either
 *  1) a variant call already exists in the table, or
 *  2) a more general (not variant) call is found and its table is
 *     complete.
 * Subsumptive call check/insert statistics are also updated.
 *
 * This routine relies on a lower-level lookup function rather than the
 * interface function subsumptive_trie_lookup() because this latter
 * function unwinds the trail, destroying the answer template in certain
 * cases.  The supplied template location is assumed to be the Choice
 * Point Stack, and in particular, a pointer to the topmost used Cell.
 *
 * It is assumed that the Call Trie exists (that there is at least a
 * root node).
 */

inline static  void subsumptive_call_search(CTXTdeclc TabledCallInfo *callStruct,
					    CallLookupResults *results) {

  BTNptr btRoot, btn;
  CPtr answer_template;    /* Initially, the location to create the answer
			      template, then a pointer to the template
			      itself: INT-encoded size followed by a vector
			      of subterms. */
  SubProdSF sf_with_ans_set;  /* Pointer to a producer; the subgoal from
				 which the call will consume */
  TriePathType path_type;    /* VARIANT/SUBSUMPTIVE/NONE */


#ifdef DEBUG_CALL_CHK_INS
  char *targetGN = "";   /* allows you to spy on a particular predicate */
  char *goal_name;

  goal_name = get_name(TIF_PSC(CallInfo_TableInfo(*callStruct)));
  if ( strcmp(targetGN,goal_name) == 0 ) {
    fprintf(stddbg,"\nCall Check/Insert (#%d overall) on:\n  ",
	       NumSubOps_CallCheckInsert + 1);
    printTabledCall(stddbg,*callStruct);
    fprintf(stddbg,"\n");
  }
#endif

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  NumSubOps_CallCheckInsert++;
#endif

  btRoot = TIF_CallTrie(CallInfo_TableInfo(*callStruct));
  answer_template = CallInfo_AnsTempl(*callStruct) - 1;

  /* Handle 0-ary Predicates
     ----------------------- */
  if (CallInfo_CallArity(*callStruct) == 0) {
    xsbBool isNew;

    btn = bt_escape_search(CTXTc btRoot, &isNew);

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
    if ( isNew )
      NumSubOps_ProducerCall++;
    else {
      if ( is_completed(CallTrieLeaf_GetSF(btn)) )
	NumSubOps_CallToCompletedTable++;
      else
	NumSubOps_VariantCall++;
    }
#endif

    CallLUR_VariantFound(*results) = ( ! isNew );
    CallLUR_Leaf(*results) = btn;
    CallLUR_Subsumer(*results) = CallTrieLeaf_GetSF(btn);
    *answer_template = makeint(0);
    CallLUR_AnsTempl(*results) = answer_template;
    return;
  }

  /* Handle N-ary Predicates, N > 0
     ------------------------------ */
  TermStack_ResetTOS;
  TermStackLog_ResetTOS;
  Trail_ResetTOS;

#ifdef CALL_ABSTRACTION_FOR_SUBSUMPTION
  CallAbsStk_ResetTOS;

  init_termstack_for_subsumptive_check_ins(CallInfo_Arguments(*callStruct),
					   CallInfo_CallArity(*callStruct));
#else
  TermStack_PushLowToHighVector(CallInfo_Arguments(*callStruct),
				CallInfo_CallArity(*callStruct));
#endif

  btn = iter_sub_trie_lookup(CTXTc btRoot, &path_type);

  /*
   * If this subsuming call maintains its own answer set, then this call
   * can consume from it.  Otherwise, this subsuming call is itself
   * subsumed and is consuming from some producer.  The new call will
   * then consume from this producer, too.  However, the computed answer
   * template was for the found subsuming call, not the one from which
   * consumption will occur.  Therefore, the template must be
   * recomputed.  In either case, if no variant was found AND the
   * subsuming call is incomplete, an entry is created in the Call Trie.
   */

  if ( path_type == NO_PATH ) {    /* new producer */
#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
    NumSubOps_ProducerCall++;
#endif
    Trail_Unwind_All;
    CallLUR_Subsumer(*results) = NULL;  /* no SF found, so no subsumer */
    CallLUR_VariantFound(*results) = NO;
    /* Insertoin of new branch is done here! */
    CallLUR_Leaf(*results) =
      bt_insert(CTXTc btRoot,stl_restore_variant_cont(CTXT),NO_INSERT_SYMBOL);
    CallLUR_AnsTempl(*results) =
      extract_template_from_insertion(CTXTc answer_template);
    Trail_Unwind_All;
#ifdef DEBUG_CALL_CHK_INS
    if ( strcmp(targetGN,goal_name) == 0 ) {
      fprintf(stddbg,"New Producer Goal: ");
      printTriePath(stddbg,CallLUR_Leaf(*results),NO);
      fprintf(stddbg,"\n");
    }
#endif
    return;
  }
  else {                           /* consume from producer */
  /* Set Correct Answer Template
     --------------------------- */
    sf_with_ans_set = (SubProdSF)CallTrieLeaf_GetSF(btn);
    if ( IsSubsumptiveProducer(sf_with_ans_set) ) {
#ifdef DEBUG_CALL_CHK_INS
      if ( strcmp(targetGN,goal_name) == 0 ) {
	fprintf(stddbg,"Found producer:\n  ");
	sfPrintGoal(stddbg,sf_with_ans_set,YES);
	fprintf(stddbg,"\nWith ");   /* continued below */
      }
#endif
#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
      if ( is_completed(sf_with_ans_set) )
	NumSubOps_CallToCompletedTable++;
      else {
	if ( path_type == VARIANT_PATH )
	  NumSubOps_VariantCall++;
	else
	  NumSubOps_SubsumedCall++;
      }
#endif
      answer_template = extract_template_from_lookup(CTXTc answer_template);
      Trail_Unwind_All;
    }
    else {                         /* consume from subsumed call */
#ifdef DEBUG_CALL_CHK_INS
      if ( strcmp(targetGN,goal_name) == 0 ) {
	fprintf(stddbg,"Found entry without own answer table:\n  ");
	sfPrintGoal(stddbg,sf_with_ans_set,YES);
	fprintf(stddbg,"\nRecomputing template for:\n  ");
	sfPrintGoal(stddbg,conssf_producer(sf_with_ans_set),YES);
	fprintf(stddbg,"\n");   /* continue with A.T. print, below */
      }
#endif
      sf_with_ans_set = conssf_producer(sf_with_ans_set);
#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
      if ( is_completed(sf_with_ans_set) )
	NumSubOps_CallToCompletedTable++;
      else
	NumSubOps_SubsumedCall++;
#endif
      Trail_Unwind_All;
      answer_template =
	reconstruct_template_for_producer(CTXTc callStruct, sf_with_ans_set,
					  answer_template);
    }

#ifdef DEBUG_CALL_CHK_INS
    if ( strcmp(targetGN,goal_name) == 0 )
      printAnswerTemplate(stddbg,
			  answer_template + int_val(*answer_template),
			  int_val(*answer_template));
#endif
    CallLUR_Subsumer(*results) = (VariantSF)sf_with_ans_set;
    CallLUR_Leaf(*results) = btn;
    CallLUR_VariantFound(*results) = (path_type == VARIANT_PATH);
    CallLUR_AnsTempl(*results) = answer_template;

    /* Conditionally Create Call Entry
       ------------------------------- */
    if ( (path_type != VARIANT_PATH) && (! is_completed(sf_with_ans_set)) ) {
#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
      NumSubOps_SubsumedCallEntry++;
#endif
      CallLUR_Leaf(*results) =
	bt_insert(CTXTc btRoot,stl_restore_variant_cont(CTXT),NO_INSERT_SYMBOL);
      Trail_Unwind_All;
#ifdef DEBUG_CALL_CHK_INS
      if ( strcmp(targetGN,goal_name) == 0 ) {
	fprintf(stddbg,"Constructed new Call entry:\n  ");
	printTriePath(stddbg,CallLUR_Leaf(*results),NO);
	fprintf(stddbg,"\n");
      }
#endif
    }
  }
}

/*=========================================================================*/

/*
 *		 Subsumptive Answer Check/Insert Operation
 *		 =========================================
 */


/*
 * Create an Empty Answer Set, represented as a Time-Stamped Trie.
 * Note that the root of the TST is labelled with a ret/n symbol,
 * where `n' is the number of terms in an answer.
 */

inline static  void *newAnswerSet(CTXTdeclc int n, TSTNptr Parent) {

  TSTNptr root;
  Cell symbol;

  if ( n > 0 )
    symbol = EncodeTriePSC(get_ret_psc(n));
  else
    symbol = EncodeTrieConstant(makestring(get_ret_string()));
  New_TSTN(root, TS_ANSWER_TRIE_TT, TRIE_ROOT_NT, symbol, Parent, NULL );
  TSTN_TimeStamp(root) = EMPTY_TST_TIMESTAMP;
  return root;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * Update statistical info, allocate an Answer Set object to house the
 * answer if the set was originally empty, conditionally insert the
 * answer into the set, and indicate to the caller whether it is new.
 */

inline static
TSTNptr subsumptive_answer_search(CTXTdeclc SubProdSF sf, int nTerms,
				  CPtr answerVector, xsbBool *isNew) {

  TSTNptr root, tstn;

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  NumSubOps_AnswerCheckInsert++;
#endif

  AnsVarCtr = 0;
  if ( IsNULL(subg_ans_root_ptr(sf)) )
    subg_ans_root_ptr(sf) = newAnswerSet(CTXTc nTerms, (TSTNptr) sf);
  root = (TSTNptr)subg_ans_root_ptr(sf);
  tstn = subsumptive_tst_search( CTXTc root, nTerms, answerVector, 
				 (xsbBool)ProducerSubsumesSubgoals(sf), isNew );

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  if ( *isNew )  NumSubOps_AnswerInsert++;
#endif

  return tstn;
}

/*=========================================================================*/
