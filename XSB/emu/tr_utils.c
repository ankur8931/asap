/* File:      tr_utils.c
** Author(s): Prasad Rao, Juliana Freire, Kostis Sagonas
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
** $Id: tr_utils.c,v 1.193 2010/06/18 17:07:05 tswift Expxtox $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Special debug includes */
#include "debugs/debug_tries.h"


#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "cinterf.h"
#include "binding.h"
#include "psc_xsb.h"
#include "heap_xsb.h"
#include "memory_xsb.h"
#include "register.h"
#include "deref.h"
#include "flags_xsb.h"
#include "trie_internals.h"
#include "tst_aux.h"
#include "tab_structs.h"
#include "sw_envs.h"
#include "choice.h"
#include "cut_xsb.h"
#include "inst_xsb.h"
#include "error_xsb.h"
#include "io_builtins_xsb.h"
#include "tr_utils.h"
#include "tst_utils.h"
#include "subp.h"
#include "rw_lock.h"
#include "debug_xsb.h"
#include "thread_xsb.h"
#include "storage_xsb.h"
#include "hash_xsb.h"
#include "tables.h"
#include "builtin.h"
#include "trie_defs.h"
#include "trassert.h"
#include "biassert_defs.h"
#include "call_graph_xsb.h" /* incremental evaluation */
#include "table_inspection_defs.h"
#include "heap_xsb.h"
#include "residual.h"
#include "basictypes.h"
#include "hashtable.h"
#include "hashtable_itr.h"

counter abol_subg_ctr,abol_pred_ctr,abol_all_ctr; /* statistics */

/*----------------------------------------------------------------------*/

#include "ptoc_tag_xsb_i.h"
#include "term_psc_xsb_i.h"

void  maybe_detach_incremental_call_single(CTXTdeclc VariantSF, int);
void  abolish_incremental_call_single_nocheck_deltf(CTXTdeclc VariantSF, int, int);
void  abolish_incremental_call_single_nocheck_no_nothin(CTXTdeclc VariantSF subgoal, int);
void  abolish_subsumptive_call_single_nocheck_deltf(CTXTdeclc SubProdSF, int);

extern int incr_table_update_safe_gl;

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
double total_table_gc_time = 0;

#define declare_timer double timer;

#define start_table_gc_time(MARK) {					\
    MARK = cpu_time();							\
}

#define end_table_gc_time(MARK) {					\
    total_table_gc_time =  total_table_gc_time + (cpu_time() - MARK);	\
  }
#else

#define declare_timer
#define start_table_gc_time(MARK) 
#define end_table_gc_time(MARK) 

#endif

//#define ABOLISH_DBG2 1
//#define DEBUG_ABOLISH 1
#ifdef DEBUG_ABOLISH
#define abolish_dbg(X) printf X
#define abolish_print_subgoal(X) print_subgoal(stdout,X)
#else
#define abolish_dbg(X) 
#define abolish_print_subgoal(X)
#endif

/*----------------------------------------------------------------------*/
/* various utility predicates and macros */
/*----------------------------------------------------------------------*/

int is_ancestor_sf(VariantSF consumer_sf, VariantSF producer_sf) {
  while (consumer_sf != NULL && consumer_sf != producer_sf) {
    consumer_sf = (VariantSF)(nlcp_ptcp(subg_cp_ptr(consumer_sf)));
  }
  if (consumer_sf == NULL) return FALSE; else return TRUE;
}

xsbBool varsf_has_unconditional_answers(VariantSF subg)
{
  ALNptr node_ptr = subg_answers(subg);
 
  /* Either subgoal has no answers or it is completed */
  /* and its answer list has already been reclaimed. */
  /* In either case, the result is immediately obtained. */
 
#ifndef CONC_COMPL
  if (node_ptr <= COND_ANSWERS) return (node_ptr == UNCOND_ANSWERS);
#else
  if (subg_tag(subg) <= COND_ANSWERS) return (subg_tag(subg) == UNCOND_ANSWERS);
#endif
 
  /* If the subgoal has not been completed, or is early completed but its */
  /* answer list has not been reclaimed yet, check each of its nodes. */
 
  while (node_ptr) {
    if (is_unconditional_answer(ALN_Answer(node_ptr))) return TRUE;
    node_ptr = ALN_Next(node_ptr);
  }
  return FALSE;
}

xsbBool varsf_has_conditional_answer(VariantSF subg)
{

  ALNptr node_ptr = subg_answers(subg);
 
  if (subg_is_completed(subg)) 
#ifndef CONC_COMPL
    return (node_ptr == COND_ANSWERS);
#else
    return (subg_tag(subg) == COND_ANSWERS);
#endif
 
  /* If the subgoal has not been completed, or is early completed but its */
  /* answer list has not been reclaimed yet, check each of its nodes. */
  else { 
    while (node_ptr) {
      if (is_conditional_answer(ALN_Answer(node_ptr))) return TRUE;
      node_ptr = ALN_Next(node_ptr);
    }
    return FALSE;
  }
}

/* This is needed to find an actual trie node from a CP -- hash-handle must be special-cased */
BTNptr TrieNodeFromCP(CPtr pCP) {							
    prolog_int i;	
    BTNptr pBTN;						
    if (*(byte *)*pCP == hash_handle) {					
      pBTN = (BTNptr) string_val(*(pCP+CP_SIZE+1));			
      for (i = 0 ; i < (prolog_int)BTHT_NumBuckets((BTHTptr) pBTN); i++) {		
	if (BTHT_BucketArray((BTHTptr) pBTN)[i] != 0) {			
	  return BTHT_BucketArray((BTHTptr) pBTN)[i];			
	}									
      }
      return NULL;
    }
    else return (BTNptr) *pCP;						
}


/*----------------------------------------------------------------------*/

/* get_call() and supporting code. */

/*----------------------------------------------------------------------*/

/*
 * Given a subgoal of a variant predicate, returns its subgoal frame
 * if it has a table entry; returns NULL otherwise.  If requested, the
 * answer template is constructed on the heap as a ret/n term and
 * passed back via the last argument.
 */

VariantSF get_variant_sf(CTXTdeclc Cell callTerm, TIFptr pTIF, Cell *retTerm) {

  int arity;
  BTNptr root, leaf;
  Cell callVars[MAX_VAR_SIZE + 1];

  if (TIF_Interning(pTIF)) {
    xsb_abort("[get_variant_sf] not supported for tables with interned ground terms: %s/%d\n",
	   get_name(TIF_PSC(pTIF)),get_arity(TIF_PSC(pTIF)));
  }

  root = TIF_CallTrie(pTIF);
  if ( IsNULL(root) )
    return NULL;

  arity = get_arity(TIF_PSC(pTIF));
  leaf = variant_trie_lookup(CTXTc root, arity, clref_val(callTerm) + 1, callVars);
  if ( IsNULL(leaf) )
    return NULL;
  if ( IsNonNULL(retTerm) )
    *retTerm = build_ret_term(CTXTc (int)callVars[0], &callVars[1]);
  return ( CallTrieLeaf_GetSF(leaf) );
}

/*----------------------------------------------------------------------*/

/*
 * Given a subgoal of a subsumptive predicate, returns the subgoal
 * frame of some producing table entry which subsumes it; returns NULL
 * otherwise.  The answer template with respect to this producer entry
 * is constructed on the heap as a ret/n term and passed back via the
 * last argument.
 * 
 * Note that unlike get_variant_sf, the answer template is derived
 * from the subsuming tabled call and the call itself (via
 * construct_answer_template), before building the ret_term.
 */

SubProdSF get_subsumer_sf(CTXTdeclc Cell callTerm, TIFptr pTIF, Cell *retTerm) {

  BTNptr root, leaf;
  int arity;
  TriePathType path_type;
  SubProdSF sf;
  Cell ansTmplt[MAX_VAR_SIZE + 1];

  root = TIF_CallTrie(pTIF);
  if ( IsNULL(root) )
    return NULL;

  arity = get_arity(TIF_PSC(pTIF));
  leaf = subsumptive_trie_lookup(CTXTc root, arity, clref_val(callTerm) + 1,
				 &path_type, ansTmplt);
  if ( IsNULL(leaf) )
    return NULL;
  sf = (SubProdSF)CallTrieLeaf_GetSF(leaf);
  if ( IsProperlySubsumed(sf) ) {
    sf = conssf_producer(sf);
    construct_answer_template(CTXTc callTerm, sf, ansTmplt);
  }
  if ( IsNonNULL(retTerm) )
    *retTerm = build_ret_term(CTXTc (int)ansTmplt[0], &ansTmplt[1]);
  return ( sf );
}
  
/*----------------------------------------------------------------------*/

BTNptr get_trie_root(BTNptr node) {

  while ( IsNonNULL(node) ) {
    if ( IsTrieRoot(node) )
      return node;
    node = BTN_Parent(node);
  }
  /*
   * If the trie is constructed correctly, processing will not reach
   * here, other than if 'node' was originally NULL.
   */
  return NULL;
}

/*----------------------------------------------------------------------*/

/*
 * Given a vector of terms and their number, N, builds a ret/N structure
 * on the heap containing those terms.  Returns this constructed term.
 */

Cell build_ret_term(CTXTdeclc int arity, Cell termVector[]) {

  Psc sym_psc;
  CPtr ret_term;
  int  i;

  if ( arity == 0 )
    return makestring(get_ret_string());  /* return as a term */
  else {
    ret_term = hreg;  /* pointer to where ret(..) will be built */
    sym_psc = get_ret_psc((byte)arity);
    new_heap_functor(hreg, sym_psc);
    for ( i = 0; i < arity; i++ )
      nbldval_safe(termVector[i]);
    return makecs(ret_term);  /* return as a term */
  }
}

Cell build_ret_term_reverse(CTXTdeclc int arity, Cell termVector[]) {

  Psc sym_psc;
  CPtr ret_term;
  int  i;

  if ( arity == 0 )
    return makestring(get_ret_string());  /* return as a term */
  else {
    ret_term = hreg;  /* pointer to where ret(..) will be built */
    sym_psc = get_ret_psc((byte)arity);
    new_heap_functor(hreg, sym_psc);
    for ( i = arity-1; i >= 0; i-- )
      nbldval_safe(termVector[i]);
    return makecs(ret_term);  /* return as a term */
  }
}

/*----------------------------------------------------------------------*/

/*
 * Create the answer template for a subsumed call with the given producer.
 * The template is stored in an array supplied by the caller.
 */

void construct_answer_template(CTXTdeclc Cell callTerm, SubProdSF producer,
			       Cell templ[]) {

  Cell subterm, symbol;
  int  sizeAnsTmplt;

  /*
   * Store the symbols along the path of the more general call.
   */
  SymbolStack_ResetTOS;
  SymbolStack_PushPath(subg_leaf_ptr(producer));

  /*
   * Push the arguments of the subsumed call.
   */
  TermStack_ResetTOS;
  TermStack_PushFunctorArgs(callTerm);

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
    if ( IsTrieVar(symbol) && IsNewTrieVar(symbol) )
      templ[++sizeAnsTmplt] = subterm;
    else if ( IsTrieFunctor(symbol) )
      TermStack_PushFunctorArgs(subterm)
    else if ( IsTrieList(symbol) )
      TermStack_PushListArgs(subterm)
  }
  templ[0] = sizeAnsTmplt;
}


/*----------------------------------------------------------------------*/
/*
 * Given a term representing a tabled call, determine whether it is
 * recorded in the Call Table.  If it is, then return a pointer to its
 * subgoal frame and construct on the heap the answer template required
 * to retrieve answers for this call.  Place a reference to this term in
 * the location pointed to by the second argument.
 */

// If passed a NULL retTerm, dont bother to build it
VariantSF get_call(CTXTdeclc Cell callTerm, Cell *retTerm) {

  Psc  psc;
  TIFptr tif;
  int arity;
  BTNptr root, leaf;
  VariantSF sf;
  Cell callVars[MAX_VAR_SIZE + 1];

  //  printf("get_call ");printterm(stddbg,callTerm,25);fprintf(stddbg,"\n");
  psc = term_psc(callTerm);
  if ( IsNULL(psc) ) {
    xsb_type_error(CTXTc "callable",callTerm,"get_call/3",1);
    return NULL;
  }

  tif = get_tip(CTXTc psc);
  if ( IsNULL(tif) )
    xsb_permission_error(CTXTc "table access","non-tabled predicate",callTerm,
			   "get_call",3);
  root = TIF_CallTrie(tif);
  if ( IsNULL(root) )
    return NULL;

  arity = get_arity(psc);
  leaf = variant_trie_lookup(CTXTc root, arity, clref_val(callTerm) + 1, callVars);
  if ( IsNULL(leaf) )
    return NULL;
  else {

    sf = CallTrieLeaf_GetSF(leaf);

    /* incremental evaluation: check introduced as because of fact
       predicates  */
    
    if(IsNonNULL(sf) && get_incr(psc) && IsNULL(sf->callnode)) 
      return NULL;
    
    /* incremental evaluation end */

    if ( IsProperlySubsumed(sf) )
      construct_answer_template(CTXTc callTerm, conssf_producer(sf), callVars);

    if (retTerm) 
      *retTerm = build_ret_term(CTXTc (int)callVars[0],&callVars[1]);

    return sf;
  }
}

/*======================================================================*/
/*
 *                     D E L E T I N G   T R I E S
 *                     ===========================
 */

/*----------------------------------------------------------------------*/
/* delete_predicate_table(), reclaim_deleted_predicate_table() 
 * and supporting code.
 * 
 * Used to delete/reclaim a predicate-level call and answer trie, works for
 * both call-variance and call subsumption. */
/*----------------------------------------------------------------------*/

/* Stack for top-down traversing and freeing components of a trie
   -------------------------------------------------------------- */

#define freeing_stack_increment 1000

#define push_node(node) {\
  if (node_stk_top >= freeing_stack_size) {\
    size_t old_freeing_stack_size = freeing_stack_size; \
    freeing_stack_size = freeing_stack_size + freeing_stack_increment;\
    freeing_stack = (BTNptr *)mem_realloc(freeing_stack,old_freeing_stack_size*sizeof(BTNptr),\
					  freeing_stack_size*sizeof(BTNptr),TABLE_SPACE);\
  }\
  freeing_stack[node_stk_top] = node;\
  node_stk_top++;\
  }

#define pop_node(node) {\
  node_stk_top--;\
  node = freeing_stack[node_stk_top];\
}

/* TLS: since this deallocates from smBTHT, make sure
   trie_allocation_type is set to private/shared before using this
   function. */
static void free_trie_ht(CTXTdeclc BTHTptr ht) {
#ifdef MULTI_THREAD
  if (BTHT_NumBuckets(ht) == TrieHT_INIT_SIZE 
      && threads_current_sm != SHARED_SM) {
    SM_DeallocateStruct(*private_smTableBTHTArray,BTHT_BucketArray(ht)); 
  }
  else
#endif
    mem_dealloc(BTHT_BucketArray(ht),BTHT_NumBuckets(ht)*sizeof(void *),
		TABLE_SPACE);
#ifdef MULTI_THREAD
  if( threads_current_sm == SHARED_SM )
	SM_Lock(*smBTHT);
#endif
  TrieHT_RemoveFromAllocList(*smBTHT,ht);
#ifdef MULTI_THREAD
  if( threads_current_sm == SHARED_SM )
	SM_Unlock(*smBTHT);
#endif
  SM_DeallocatePossSharedStruct(*smBTHT,ht); 
}

/* -------------------------------------------------------------- 
 * Space reclamation for Conditional Answers 
 * All routines assume that trie allocation type has been set (cf. struct_manager.h)
 * ------------------------------------------------------------ */

#ifdef MULTI_THREAD
#define GLOBAL_TABLE    (threads_current_sm == SHARED_SM)		
#endif

void release_any_pndes(CTXTdeclc PNDE firstPNDE) {
  PNDE lastPNDE;

#ifdef MULTI_THREAD  
  if (GLOBAL_TABLE)
#endif
    while (firstPNDE) {
      lastPNDE = firstPNDE;
      firstPNDE = pnde_next(firstPNDE);
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      release_entry(lastPNDE, released_pndes_gl, pnde_next);	
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
#ifdef MULTI_THREAD  
  else
    while (firstPNDE) {
      lastPNDE = firstPNDE;
      firstPNDE = pnde_next(firstPNDE);
      release_entry(lastPNDE, private_released_pndes, pnde_next);	
    }
#endif
}

/* TLS: a delay "trie" should be a simple chain, as I understand it --
   hence the error messages below. 
---------------------------------------------------------------------*/
void delete_delay_trie(CTXTdeclc BTNptr root) {
  int node_stk_top = 0;
  BTNptr rnod;
  //  BTNptr *Bkp; 
  //  BTHTptr ht;
  //  printf(" in delete delay trie\n");
  BTNptr *freeing_stack = NULL;
  int freeing_stack_size = 0;

  if ( IsNonNULL(root) ) {
    push_node(root);
    while (node_stk_top != 0) {
      pop_node(rnod);
      if ( IsHashHeader(rnod) ) {
	xsb_abort("encountered hashtable in delay trie?!?\n");

	//	ht = (BTHTptr) rnod;
	//	for (Bkp = BTHT_BucketArray(ht);
	//	     Bkp < BTHT_BucketArray(ht) + BTHT_NumBuckets(ht);
	//	     Bkp++) {
	//	  if ( IsNonNULL(*Bkp) )
	//	    push_node(*Bkp);
	//	}
	//	free_trie_ht(CTXTc ht);
      }
      else {
	if (BTN_Sibling(rnod)) 
	  xsb_abort("encountered sibling in delay trie?!?\n");
	//	  push_node(BTN_Sibling(rnod));
	if ( BTN_Child(rnod))
	  push_node(BTN_Child(rnod));
	SM_DeallocatePossSharedStruct(*smBTN,rnod);
      }
    }
  } /* free answer trie */
  mem_dealloc(freeing_stack,freeing_stack_size*sizeof(BTNptr),TABLE_SPACE);
  //  printf("leaving delete delay trie\n");
}

/* TLS: unlike release_all_dls() in slgdelay.c which is used for
   simplification, I am not releasing pndes of the subgoal or answer
   associated with the delay element.  Rather, a linked set of tables
   should be abolished in one fell swoop. 
---------------------------------------------------------------------*/
   
void abolish_release_all_dls_shared(CTXTdeclc ASI asi)
{
  //  ASI de_asi;
  DE de, tmp_de;
  DL dl, tmp_dl;

  dl = asi_dl_list(asi);
  SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
  while (dl) {
    tmp_dl = dl_next(dl);
    de = dl_de_list(dl);
    while (de) {
      tmp_de = de_next(de);
      release_shared_de_entry(de);
      de = tmp_de; /* next DE */
    } /* while (de) */
    release_entry(dl, released_dls_gl, dl_next);
    dl = tmp_dl; /* next DL */
  }
  SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
}

#ifdef MULTI_THREAD
void abolish_release_all_dls_private(CTXTdeclc ASI asi)
{
  DE de, tmp_de;
  DL dl, tmp_dl;

  dl = asi_dl_list(asi);
  while (dl) {
    tmp_dl = dl_next(dl);
    de = dl_de_list(dl);
    while (de) {
      tmp_de = de_next(de);
      release_private_de_entry(de);
      de = tmp_de; /* next DE */
    } /* while (de) */
    release_entry(dl, private_released_dls, dl_next);
    dl = tmp_dl; /* next DL */
  }
}
#endif

void abolish_release_all_dls(CTXTdeclc ASI asi)
{

#ifdef MULTI_THREAD  
  if (GLOBAL_TABLE)
#endif
    abolish_release_all_dls_shared(CTXTc asi);
#ifdef MULTI_THREAD
  else
    abolish_release_all_dls_private(CTXTc asi);
#endif
}

void release_conditional_answer_info(CTXTdeclc BTNptr node) {
  ASI asiptr;
  if ((asiptr = (ASI) BTN_Child(node))) {
    abolish_release_all_dls(CTXTc asiptr);		    
    release_any_pndes(CTXTc asi_pdes(asiptr)); // handles shared/private
#ifdef MULTI_THREAD  
  if (GLOBAL_TABLE)
#endif
    SM_DeallocateSharedStruct(smASI,asiptr)
#ifdef MULTI_THREAD  
  else
    SM_DeallocateStruct(*private_smASI,asiptr);
#endif
  }
}

/* 
 * delete_variant_call deletes and reclaims space for
 *  answers and their subgoal frame in a variant table, and is used by
 *  abolish_table_call (which does not work on subsumptive table).  It
 *  copies code from delete_variant_predicate_tablex, but uses its own stack.
 *  (Not easy to integrate due to macro usage.) 
 * 
 * TLS: since this deallocates from SMs, make sure
 * trie_allocation_type is set before using.
 */
void delete_variant_call(CTXTdeclc VariantSF pSF, xsbBool should_warn) {
  int node_stk_top = 0;
  BTNptr rnod, *Bkp; 
  BTHTptr ht;
  
  BTNptr *freeing_stack = NULL;
  int freeing_stack_size = 0;

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  abol_subg_ctr++;
#endif
  
#ifdef DEBUG_ABOLISH
  abolish_dbg(("delete_variant_call %p ",pSF));print_subgoal(stddbg,pSF);printf("\n");
#endif

  TRIE_W_LOCK();
  /* TLS: this checks whether any answer for this subgoal has a delay
     list: may overstate problems but will warn for any possible
     corruption. */
#ifndef CONC_COMPL
  if ( subg_answers(pSF) == COND_ANSWERS && should_warn) {
#else
    if ( subg_tag(pSF) == COND_ANSWERS && should_warn) {
#endif
      xsb_warn(CTXTc "abolish_table_call/1 is deleting a table entry for %s/%d with conditional"
                      " answers: delay dependencies may be corrupted.\n",	    
	       get_name(TIF_PSC(subg_tif_ptr(pSF))),get_arity(TIF_PSC(subg_tif_ptr(pSF))));
    }

  if ( IsNonNULL(subg_ans_root_ptr(pSF)) ) {
    push_node((BTNptr)subg_ans_root_ptr(pSF));
    while (node_stk_top != 0) {
      pop_node(rnod);
      if ( IsHashHeader(rnod) ) {
	ht = (BTHTptr) rnod;
	for (Bkp = BTHT_BucketArray(ht);
	     Bkp < BTHT_BucketArray(ht) + BTHT_NumBuckets(ht);
	     Bkp++) {
	  if ( IsNonNULL(*Bkp) )
	    push_node(*Bkp);
	}
	free_trie_ht(CTXTc ht);
      }
      /* It can be that a table is in an inconsistent state when we
      abolish it, if we are doing so as exception handling for some
      sort of term depth limit.  Hence, the isNonNulls below.    */
      else { 
	if (BTN_Sibling(rnod) && IsNonNULL(BTN_Sibling(rnod)))
	  { push_node(BTN_Sibling(rnod)); }
	if ( !IsLeafNode(rnod) && IsNonNULL(BTN_Child(rnod))) {
	  push_node(BTN_Child(rnod)) } 
	else { /* leaf node */
	  if (!hasALNtag(rnod)) release_conditional_answer_info(CTXTc rnod);
	  }
 	SM_DeallocatePossSharedStruct(*smBTN,rnod);
      }
    }
  } /* free answer trie */
  free_answer_list(pSF);
  FreeProducerSF(pSF);
  TRIE_W_UNLOCK();
  mem_dealloc(freeing_stack,freeing_stack_size*sizeof(BTNptr),TABLE_SPACE);
  //  printf("leaving delete_variant\n");
  }

//---------------------------------------------------------------------------

#include "tr_code_xsb_i.h"

ALNptr traverse_variant_answer_trie(CTXTdeclc VariantSF subgoal, CPtr rootptr, CPtr leafptr) {
  int node_stk_top = 0; Integer hash_offset;
  BTNptr rnod, hash_bucket;   
  BTHTptr hash_hdr, *hash_base;
  BTNptr *freeing_stack = NULL;  int freeing_stack_size = 0;
  ALNptr lastALN = (ALNptr) NULL; ALNptr thisALN;
  BTNptr tempstk[1280]; int tempstk_top = 0;

  // printf("starting leafptr to %p  %x\n",leafptr,BTN_Instr((BTNptr) *leafptr));

  while (leafptr != rootptr) {
    if (BTN_Instr((BTNptr) *leafptr) == 0x7a) {
      tempstk[tempstk_top++] = (BTNptr) string_val(cell(breg+CP_SIZE+1));
      tempstk[tempstk_top++] = (BTNptr) int_val(cell(breg+CP_SIZE));   // Number of bucket / flag
    }
    else {
      tempstk[tempstk_top++] =  *(BTNptr *)leafptr;
    }
    //    printf("   setting leafptr to %p  %x\n",cp_prevbreg(leafptr),BTN_Instr((BTNptr) *cp_prevbreg(leafptr)));
    leafptr = (CPtr)  cp_prevbreg(leafptr);
  }
  if (tempstk_top >= 1280) 
      xsb_abort("Ran out of reversal stack space while logging for incremental tabling update\n");
  while(tempstk_top > 0) {
    push_node(tempstk[--tempstk_top]);
  }
  while (node_stk_top != 0) {
      pop_node(rnod);
      if ( IsHashHeader(rnod)) {
	pop_node(hash_bucket);
	hash_offset = (Integer) hash_bucket;
	//	printf("hash header %p offset %p\n",rnod,hash_bucket);
	hash_hdr = (BTHTptr) rnod;
	hash_base = (BTHTptr *) BTHT_BucketArray(hash_hdr);
	find_next_nonempty_bucket(hash_hdr,hash_base,hash_offset);
	if (hash_offset != NO_MORE_IN_HASH) {
	  push_node((BTNptr) hash_offset);
	  push_node((BTNptr) hash_hdr);

	  push_node(*(BTNptr *)(hash_base + hash_offset));
	  //	  printf("pushed %p\n",hash_base + hash_offset);
	}
      }
      /* Non nulls from abolish-- but keeping for now */
      else { 
	//	printf("not hash header\n");
	if (BTN_Sibling(rnod) && IsNonNULL(BTN_Sibling(rnod)))
	  { push_node(BTN_Sibling(rnod)); }
	if ( !IsLeafNode(rnod) && IsNonNULL(BTN_Child(rnod))) { 
	  push_node(BTN_Child(rnod)) } 
	else { /* leaf node */
	  //	  printf("Trie traversal Leaf Node %p\n",rnod);
	  New_ALN(subgoal,thisALN, rnod, lastALN);
	  lastALN = thisALN;
	}
      }
  }
  return lastALN;
} 

//---------------------------------------------------------------------------

/* Code to abolish tables for a variant predicate */
/* Incremental tabling is not yet implemented for predicates */

static void delete_variant_predicate_table(CTXTdeclc BTNptr x, xsbBool should_warn) {

   //   printf("in delete variant table\n");

  int node_stk_top = 0, call_nodes_top = 0;
  BTNptr node, rnod, *Bkp; 
  BTHTptr ht;

  BTNptr *freeing_stack = NULL;
  int freeing_stack_size = 0;

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  abol_pred_ctr++;
#endif
  
  if ( IsNULL(x) )
    return;

  TRIE_W_LOCK();
  push_node(x);
  while (node_stk_top > 0) {
    pop_node(node);
    if ( IsHashHeader(node) ) {
      ht = (BTHTptr) node;
      for (Bkp = BTHT_BucketArray(ht);
	   Bkp < BTHT_BucketArray(ht) + BTHT_NumBuckets(ht);
	   Bkp++) {
	if ( IsNonNULL(*Bkp) )
	  push_node(*Bkp);
      }
      free_trie_ht(CTXTc ht);
    }
    else {
      if ( IsNonNULL(BTN_Sibling(node)) )
	push_node(BTN_Sibling(node));
      if ( IsNonNULL(BTN_Child(node)) ) {
	if ( IsLeafNode(node) ) {
	  /**
	   * Remove the subgoal frame and its dependent structures
	   */
	  VariantSF pSF = CallTrieLeaf_GetSF(node);

	  /* TLS: this checks whether any answer for this subgoal has
	  a delay list: may overstate problems but will warn for any
	  possible corruption. */
#ifndef CONC_COMPL
	  if ( subg_answers(pSF) == COND_ANSWERS && should_warn) {
#else
	  if ( subg_tag(pSF) == COND_ANSWERS && should_warn) {
#endif
	    /* until add parameter...
	    xsb_warn(CTXTc "abolish_table_pred/1 is deleting a table entry for %s/%d with conditional \
answers: delay dependencies may be corrupted flags %d.\n",	    
		     get_name(TIF_PSC(subg_tif_ptr(pSF))),get_arity(TIF_PSC(subg_tif_ptr(pSF))),flags[TABLE_GC_ACTION]);
	    */
	    /*
	    xsb_warn(CTXTc "abolish_table_pred/1 is deleting a table entry for %s/%d with conditional\
                      answers: delay dependencies may be corrupted.\n",	    
		     get_name(TIF_PSC(subg_tif_ptr(pSF))),get_arity(TIF_PSC(subg_tif_ptr(pSF))));
	    */
	    should_warn = FALSE;
	  }

	  if ( IsNonNULL(subg_ans_root_ptr(pSF)) ) {
	    call_nodes_top = node_stk_top;
	    push_node((BTNptr)subg_ans_root_ptr(pSF));
	    while (node_stk_top != call_nodes_top) {
	      pop_node(rnod);
	      if ( IsHashHeader(rnod) ) {
		ht = (BTHTptr) rnod;
		for (Bkp = BTHT_BucketArray(ht);
		     Bkp < BTHT_BucketArray(ht) + BTHT_NumBuckets(ht);
		     Bkp++) {
		  if ( IsNonNULL(*Bkp) )
		    push_node(*Bkp);
		}
		free_trie_ht(CTXTc ht);
	      }
	      else {
		if (BTN_Sibling(rnod)) 
		  push_node(BTN_Sibling(rnod));
		if ( ! IsLeafNode(rnod) )
		  push_node(BTN_Child(rnod))
		else { /* leaf node */
		  if (!hasALNtag(rnod)) release_conditional_answer_info(CTXTc rnod);
		}
		SM_DeallocatePossSharedStruct(*smBTN,rnod);
	      }
	    }
	  } /* free answer trie */
	  /* Dont destroy hashtable -- done elsewhere.... */
	  //	  if (incr) hashtable1_destroy(pSF->callnode->outedges->hasht,0);
	  free_answer_list(pSF);
	  FreeProducerSF(pSF);
	} /* callTrie node is leaf */
	else 
	  push_node(BTN_Child(node));
      } /* there is a child of "node" */
      SM_DeallocatePossSharedStruct(*smBTN,node);
    }
  }
  TRIE_W_UNLOCK();

  mem_dealloc(freeing_stack,freeing_stack_size*sizeof(BTNptr),TABLE_SPACE);

}

/* Low-level abolish, called by abolish_table_predwhen it is now known
   whether a predicate is variant or subsumptive.  Assumes incremental
   check has already been made.*/
  void delete_predicate_table(CTXTdeclc TIFptr tif, xsbBool warn) {
  if ( TIF_CallTrie(tif) != NULL ) {
    SET_TRIE_ALLOCATION_TYPE_TIP(tif);
    if ( IsVariantPredicate(tif) ) {
      if (!isIncrementalTif(tif) ){
	delete_variant_predicate_table(CTXTc TIF_CallTrie(tif),warn);
      } else {
	char message[ERRMSGLEN/2];
	snprintf(message,ERRMSGLEN/2,"incremental tabled predicate %s/%d",get_name(TIF_PSC(tif)),
		 get_arity(TIF_PSC(tif)));
	xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
      }
    }
    else
      delete_subsumptive_predicate_table(CTXTc tif);
    TIF_CallTrie(tif) = NULL;
    TIF_Subgoals(tif) = NULL;
  }
}

void transitive_delete_predicate_table(CTXTdeclc TIFptr tif, xsbBool should_warn) {

  SET_TRIE_ALLOCATION_TYPE_TIP(tif);
  delete_variant_predicate_table(CTXTc TIF_CallTrie(tif),should_warn);
  TIF_CallTrie(tif) = NULL;
  TIF_Subgoals(tif) = NULL;
}

/* - - - - - */

/* Just like delete_predicate_table, but called from gc sweeps with deltf_ptr.
   In addition, does not reset TIFs.*/
void reclaim_deleted_predicate_table(CTXTdeclc DelTFptr deltf_ptr) {
  TIFptr tif = subg_tif_ptr(DTF_Subgoals(deltf_ptr));

  SET_TRIE_ALLOCATION_TYPE_TIP(tif);
  if ( IsVariantPredicate(tif) ) {
    delete_variant_predicate_table(CTXTc DTF_CallTrie(deltf_ptr), DTF_Warn(deltf_ptr));
  } else reclaim_deleted_subsumptive_table(CTXTc deltf_ptr);
}

/*----------------------------------------------------------------------*/
/* delete_branch(), safe_delete_branch(), undelete_branch() and
 * supporting code. */
/*----------------------------------------------------------------------*/

 /* 
 * Used for call tries (abolish_table_call/1), answer tries (within
 * delay handling routines), and by trie_retract.
 */

static int is_hash(BTNptr x) 
{
  if( x == NULL)
    return(0);
  else
    return( IsHashHeader(x) );
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * Set values for "parent" -- the parent node of "current" -- and
 * "cur_hook" -- an address containing a pointer into "current"'s level
 * in the trie.  If there is no parent node, use the value of
 * "root_hook" to find the level.  If the hook is actually contained in
 * the parent of current (as its child field), then we've ascended as
 * far as we need to go.  Set parent to NULL to indicate this.
 */

static void set_parent_and_node_hook(BTNptr current, BTNptr *root_hook,
				     BTNptr *parent, BTNptr **cur_hook) {

  BTNptr par;

  if ( IsTrieRoot(current) )  /* defend against root having a set parent field */
    par = NULL;
  else {
    par = BTN_Parent(current);
    if ( IsNonNULL(par) && (root_hook == &BTN_Child(par)) )
      par = NULL;    /* stop ascent when hooking node is reached */
  }
  if ( IsNULL(par) )
    *cur_hook = root_hook;
  else
    *cur_hook = &BTN_Child(par);
  *parent = par;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * Given some non-root node which is *not the first* (or only) sibling,
 * find the node which precedes it in the chain.  Should ONLY be used
 * when deleting trie components.  If a hash table is encountered, then
 * its number of contents is decremented.
 */
static BTNptr get_prev_sibl(BTNptr node)
{
  BTNptr sibling_chain;

  sibling_chain = BTN_Child(BTN_Parent(node));
  if ( IsHashHeader(sibling_chain) ) {
    BTHTptr ht = (BTHTptr)sibling_chain;
    BTHT_NumContents(ht)--;
    sibling_chain = *CalculateBucketForSymbol(ht,BTN_Symbol(node));
  }
  while(sibling_chain != NULL){
    if (BTN_Sibling(sibling_chain) == node)
      return(sibling_chain);
    sibling_chain = BTN_Sibling(sibling_chain);
  }  
  xsb_abort("Error in get_previous_sibling");
  return(NULL);
}

/*---------------------------------------------------------*/

/*
 * Delete a branch in the trie down from node `lowest_node_in_branch'
 * up to the level pointed to by the hook location, as pointed to by
 * `hook'.  Under normal use, the "hook" is either for the root of the
 * trie, or for the first level of the trie (is a pointer to the child
 * field of the root).  */

/* 
 * TLS: since this deallocates from SMs, make sure
 * trie_allocation_type is set before using.
 *
 * In addition, it now works for call subsumption, so need to set the
 * structure manager to use (smBTN vs smTSTN)
 */
void delete_branch(CTXTdeclc BTNptr lowest_node_in_branch, BTNptr *hook,int eval_method) {

  Integer num_left_in_hash;
  BTNptr prev, parent_ptr, *y1, *z;
  Structure_Manager *smNODEptr;

  //  printf("delete branch %p %p\n",lowest_node_in_branch,*hook);

  if (eval_method == VARIANT_EVAL_METHOD)
    smNODEptr = smBTN;
  else 
    smNODEptr = &smTSTN;
  while ( IsNonNULL(lowest_node_in_branch) && 
	  ( Contains_NOCP_Instr(lowest_node_in_branch) ||
	    IsTrieRoot(lowest_node_in_branch) ) ) {
    /*
     *  Walk up a path with no branches, i.e., the nodes along this path
     *  have no siblings.  We know this because the instruction in the
     *  node is of the no_cp variety.
     */
    set_parent_and_node_hook(lowest_node_in_branch,hook,&parent_ptr,&y1);
    if (is_hash(*y1)) {
      //      printf("found hash\n");
      z = CalculateBucketForSymbol((BTHTptr)(*y1),
				   BTN_Symbol(lowest_node_in_branch));
#ifdef DEBUG_VERBOSE
      if ( *z != lowest_node_in_branch )
		  xsb_dbgmsg((LOG_DEBUG,"DELETE_BRANCH: trie node not found in hash table"));
#endif
      *z = NULL;
      num_left_in_hash = --BTHT_NumContents((BTHTptr)*y1);
      if (num_left_in_hash  > 0) {
	/*
	 * `lowest_node_in_branch' has siblings, even though they are not in
	 * the same chain.  Therefore we cannot delete the parent, and so
	 * we're done.
	 */
	if ( *(((prolog_int *)lowest_node_in_branch)+1) ==  FREE_TRIE_NODE_MARK) 
	  ;//printf("double deallocation in delete_branch %p\n",lowest_node_in_branch);
	else{
	  SM_DeallocateStruct(*smNODEptr,lowest_node_in_branch);
	}
	return;
      }
      else
	free_trie_ht(CTXTc (BTHTptr)(*y1));
    }
    /*
     *  Remove this node and continue.
     */
    if ( *(((prolog_int *)lowest_node_in_branch)+1) ==  FREE_TRIE_NODE_MARK)  
      return;//printf("double deallocation in delete_branch %p\n",lowest_node_in_branch);
    else{
      //      printf("deleting-1 %p ",lowest_node_in_branch);
      //      print_trie_atom(lowest_node_in_branch->symbol);printf("\n");
    
      SM_DeallocateStruct(*smNODEptr,lowest_node_in_branch);
    }
    lowest_node_in_branch = parent_ptr;
  }

  if (lowest_node_in_branch == NULL)
    *hook = 0;
  else {
    if (Contains_TRY_Instr(lowest_node_in_branch)) {
      /* Alter sibling's instruction:  trust -> no_cp  retry -> try */
      BTN_Instr(BTN_Sibling(lowest_node_in_branch)) =
	BTN_Instr(BTN_Sibling(lowest_node_in_branch)) -1;
      y1 = &BTN_Child(BTN_Parent(lowest_node_in_branch));
      if (is_hash(*y1)) {
	z = CalculateBucketForSymbol((BTHTptr)(*y1),
				     BTN_Symbol(lowest_node_in_branch));
//	num_left_in_hash = --BTHT_NumContents((BTHTptr)*y1);
	--BTHT_NumContents((BTHTptr)*y1);
      }
      else
	z = y1;
      *z = BTN_Sibling(lowest_node_in_branch);      
    }
    else { /* not the first in the sibling chain */
      prev = get_prev_sibl(lowest_node_in_branch);      
      BTN_Sibling(prev) = BTN_Sibling(lowest_node_in_branch);
      if (Contains_TRUST_Instr(lowest_node_in_branch))
	BTN_Instr(prev) -= 2; /* retry -> trust ; try -> nocp */
    }
    //    printf("deleting %x\n",lowest_node_in_branch->symbol);
    if ( *(((prolog_int *)lowest_node_in_branch)+1) ==  FREE_TRIE_NODE_MARK) 
      ;//      printf("double deallocation in delete_branch %p\n",lowest_node_in_branch);
    else{
      //      printf("deleting-2 %p ",lowest_node_in_branch);
      //      print_trie_atom(lowest_node_in_branch->symbol);printf("\n");
      SM_DeallocateStruct(*smNODEptr,lowest_node_in_branch);
    }
  }
}

/*------------------------------*/
void safe_delete_branch(BTNptr lowest_node_in_branch) {

  byte instruction;

  MakeStatusDeleted(lowest_node_in_branch);
  instruction = BTN_Instr(lowest_node_in_branch);
  instruction = (instruction & 0x3) | trie_no_cp_fail;
  BTN_Instr(lowest_node_in_branch) = instruction;
}

void undelete_branch(BTNptr lowest_node_in_branch) {

   byte choicepttype; 
   byte typeofinstr;

   if( IsDeletedNode(lowest_node_in_branch) ){
     choicepttype = 0x3 &  BTN_Instr(lowest_node_in_branch);
     /* Status contains the original instruction that was in that trie node.
	here we extract the original instruction and the next statement
	makes it into the instruction associated with that node. */
     typeofinstr = (~0x3) & BTN_Status(lowest_node_in_branch);

     BTN_Instr(lowest_node_in_branch) = choicepttype | typeofinstr;
     /* This only sets the status field. It is also necessary to set the
	instruction field correctly, which is done above. */
     MakeStatusValid(lowest_node_in_branch);
   }
   else
     /* This is permitted, because we might bt_delete, then insert
	(non-backtrackably) and then backtrack.
     */
     xsb_dbgmsg((LOG_INTERN, "Undeleting a node that is not deleted"));
}


/*----------------------------------------------------------------------*/
/* delete_trie() and supporting code.  
 * 
 * Code to support deletion of asserted or interned tries.
 * delete_trie() is used by gen_retractall (i.e. abolish or retractall
 * with an open atomic formula) to delete an entire asserted trie.
 * Its also called via the builtin DELETE_TRIE --
 * (delete_interned_trie()) to delete an interned trie or storage trie */
/*----------------------------------------------------------------------*/

#define DELETE_TRIE_STACK_INIT 100
#define DT_NODE 0
#define DT_DS 1
#define DT_HT 2

#define push_delete_trie_node(node,op) {\
  trie_op_top++;\
  if (trie_op_top >= trie_op_size) {\
    trie_op_size = 2*trie_op_size;\
    delete_trie_op = (char *)mem_realloc(delete_trie_op,(trie_op_size/2)*sizeof(char),trie_op_size*sizeof(char),TABLE_SPACE);\
    if (!delete_trie_op) xsb_exit( "out of space for deleting trie");\
    /*xsb_dbgmsg((LOG_DEBUG,"realloc delete_trie_op to %d",trie_op_size));*/\
  }\
  delete_trie_op[trie_op_top] = op;\
  trie_node_top++;\
  if (trie_node_top >= trie_node_size) {\
    trie_node_size = 2*trie_node_size;\
    delete_trie_node = (BTNptr *)mem_realloc(delete_trie_node,(trie_node_size/2)*sizeof(BTNptr),trie_node_size*sizeof(BTNptr),TABLE_SPACE);\
    if (!delete_trie_node) xsb_exit( "out of space for deleting trie");\
    /*xsb_dbgmsg((LOG_DEBUG,"realloc delete_trie_node to %d",trie_node_size));*/\
  }\
  delete_trie_node[trie_node_top] = node;\
}  
#define push_delete_trie_hh(hh) {\
  trie_op_top++;\
  if (trie_op_top >= trie_op_size) {\
    trie_op_size = 2*trie_op_size;\
    delete_trie_op = (char *)mem_realloc(delete_trie_op,(trie_op_size/2)*sizeof(char),trie_op_size*sizeof(char),TABLE_SPACE);\
    if (!delete_trie_op) xsb_exit( "out of space for deleting trie");\
    /*xsb_dbgmsg((LOG_DEBUG,"realloc delete_trie_op to %d",trie_op_size));*/\
  }\
  delete_trie_op[trie_op_top] = DT_HT;\
  trie_hh_top++;\
  if (trie_hh_top >= trie_hh_size) {\
    trie_hh_size = 2*trie_hh_size;\
    delete_trie_hh = (BTHTptr *)mem_realloc(delete_trie_hh,(trie_hh_size/2)*sizeof(BTHTptr),trie_hh_size*sizeof(BTHTptr),TABLE_SPACE);\
    if (!delete_trie_hh) xsb_exit( "out of space for deleting trie");\
    /*xsb_dbgmsg((LOG_DEBUG,"realloc delete_trie_hh to %d",trie_hh_size));*/\
  }\
  delete_trie_hh[trie_hh_top] = hh;\
}  

/*************************************************************************/
/* TLS: assumed for the purpose of MT storage managers, that
   delete_trie() is being called only to delete asserted tries --
   otherwise, need to set smBTN and smBTHT to private/shared */

void delete_trie(CTXTdeclc BTNptr iroot) {
  BTNptr root, sib, chil;  
  int trie_op_top = 0;
  int trie_node_top = 0;
  int trie_hh_top = -1;

  char *delete_trie_op = NULL;
  BTNptr *delete_trie_node = NULL;
  BTHTptr *delete_trie_hh = NULL;
  int trie_op_size, trie_node_size, trie_hh_size;

  if (!delete_trie_op) {
    delete_trie_op = (char *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(char),TABLE_SPACE);
    delete_trie_node = (BTNptr *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(BTNptr),TABLE_SPACE);
    delete_trie_hh = (BTHTptr *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(BTHTptr),TABLE_SPACE);
    trie_op_size = trie_node_size = trie_hh_size = DELETE_TRIE_STACK_INIT;
  }

  delete_trie_op[0] = 0;
  delete_trie_node[0] = iroot;
  while (trie_op_top >= 0) {
    /*    xsb_dbgmsg((LOG_DEBUG,"top %d %d %d %p",trie_op_top,trie_hh_top,
	  delete_trie_op[trie_op_top],delete_trie_node[trie_node_top])); */
    switch (delete_trie_op[trie_op_top--]) {
    case DT_DS:
      root = delete_trie_node[trie_node_top--];
      SM_DeallocatePossSharedStruct(*smBTN,root);
      break;
    case DT_HT:
      free_trie_ht(CTXTc delete_trie_hh[trie_hh_top--]);
      break;
    case DT_NODE:
      root = delete_trie_node[trie_node_top--];
      if ( IsNonNULL(root) ) {
	if ( IsHashHeader(root) ) {
	  BTHTptr hhdr;
	  BTNptr *base, *cur;
	  hhdr = (BTHTptr)root;
	  base = BTHT_BucketArray(hhdr);
	  push_delete_trie_hh(hhdr);
	  for ( cur = base; cur < base + BTHT_NumBuckets(hhdr); cur++ ) {
	    if (IsNonNULL(*cur)) {
	      push_delete_trie_node(*cur,DT_NODE);
	    }
	  }
	}
	else {
	  sib  = BTN_Sibling(root);
	  chil = BTN_Child(root);      
	  /* Child nodes == NULL is not the correct test*/
	  if (IsLeafNode(root)) {
	    if (IsNonNULL(chil))
	      xsb_exit( "Anomaly in delete_trie !");
	    push_delete_trie_node(root,DT_DS);
	    if (IsNonNULL(sib)) {
	      push_delete_trie_node(sib,DT_NODE);
	    }
	  }
	  else {
	    push_delete_trie_node(root,DT_DS);
	    if (IsNonNULL(sib)) {
	      push_delete_trie_node(sib,DT_NODE);
	    }
	    if (IsNonNULL(chil)) {
	      push_delete_trie_node(chil,DT_NODE);
	    }
	  }
	}
      } else
	fprintf(stdwarn,"null node encountered in delete_trie\n");
      break;
    }
  }
  mem_dealloc(delete_trie_op,trie_op_size*sizeof(char),TABLE_SPACE); delete_trie_op = NULL;
  mem_dealloc(delete_trie_node,trie_node_size*sizeof(BTNptr),TABLE_SPACE); delete_trie_node = NULL;
  mem_dealloc(delete_trie_hh,trie_hh_size*sizeof(BTHTptr),TABLE_SPACE); delete_trie_hh = NULL;
}

/*======================================================================*/

/*
 *                  A N S W E R   O P E R A T I O N S
 *                  =================================
 */

/*----------------------------------------------------------------------*/

/*
 * This does not reclaim space for deleted nodes, only marks
 * the node as deleted and changes the try instruction to fail.
 * The deleted node is then linked into the del_nodes_list
 * in the completion stack.
 * 
 * TLS: I have a question about the simplification done at the end of
 * this predicate.  It should only be performed if the trie is completed.
 * 
 * TLS: put in some protection for simplification operations using
 * MUTEX_DELAY.  But I'm not sure that other parts of this function
 * are thread-safe.
 */
void delete_return(CTXTdeclc BTNptr leaf, VariantSF sg_frame,int eval_method) 
{
  ALNptr a, n, next;
  NLChoice c;
  int groundcall = FALSE;
#ifdef LOCAL_EVAL
  TChoice  tc;
#endif

  //  printf("DELETE_NODE: %d - Par: %d\n", leaf, BTN_Parent(leaf));
  //    ans_deletes++;

    // already simplified this return away
    if (!has_answer_code(sg_frame)) return;

    /* deleting an answer makes it false, so we have to deal with 
       delay lists */
    SET_TRIE_ALLOCATION_TYPE_SF(sg_frame);
    if (is_conditional_answer(leaf)) {
      ASI asi = Delay(leaf);
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      release_all_dls(CTXTc asi);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
      /* TLS 12/00 changed following line from 
	 (leaf == subg_ans_root_ptr(sg_frame) && ..
	 so that negation failure simplification is properly performed */
      if (leaf == BTN_Child(subg_ans_root_ptr(sg_frame)) &&
	  IsEscapeNode(leaf))
	groundcall=TRUE; /* do it here, when leaf is still valid */
    }

  if (is_completed(sg_frame)) {
    safe_delete_branch(leaf);
  } else {
    delete_branch(CTXTc leaf,&subg_ans_root_ptr(sg_frame),eval_method);
    if (hasALNtag(leaf)) {
      n = untag_as_ALNptr(leaf);
    } else {
      n = subg_ans_list_ptr(sg_frame);
      /* Find previous sibling -pvr */
      while (ALN_Answer(ALN_Next(n)) != leaf) {
	n = ALN_Next(n);/* if a is not in that list a core dump will result */
      }
#ifdef NON_OPT_COMPILE
      if (n == NULL) {
	xsb_exit( "Error in delete_return()");
      }
#endif
    }
    if (ALN_Next(n) && ALN_Next(ALN_Next(n)) &&
	hasALNtag(ALN_Answer(ALN_Next(ALN_Next(n))))) {
	Child(ALN_Answer(ALN_Next(ALN_Next(n)))) = Child(leaf);
      }
    a               = ALN_Next(n);
    next            = ALN_Next(a);
    ALN_Answer(a)   = NULL; /* since we eagerly release trie nodes, this is
			       necessary to keep garbage collection sane */
    ALN_Next(a) = compl_del_ret_list(subg_compl_stack_ptr(sg_frame));
    compl_del_ret_list(subg_compl_stack_ptr(sg_frame)) = a;    

    ALN_Next(n) = next;
    
    /* Make consumed answer field of consumers point to
       previous sibling if they point to a deleted answer */
    c = (NLChoice) subg_pos_cons(sg_frame);
    while(c != NULL){
      if(nlcp_trie_return(c) == a){
	nlcp_trie_return(c) = n;
      }
      c = (NLChoice)nlcp_prevlookup(c);
    }

#if (defined(LOCAL_EVAL))
      /* if gen-cons points to deleted answer, make it
       * point to previous sibling */
      tc = (TChoice)subg_cp_ptr(sg_frame);
      if (tcp_trie_return(tc) == a) {
	tcp_trie_return(tc) = n;
      }
#endif
   
    if(next == NULL){ /* last answer */
      subg_ans_list_tail(sg_frame) = n;
    }      
  }
  if (is_conditional_answer(leaf)) {
    SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
    simplify_pos_unsupported(CTXTc leaf);
    if (groundcall) {
      mark_subgoal_failed(sg_frame);
      simplify_neg_fails(CTXTc sg_frame);
    }
    SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
  }
}

/*----------------------------------------------------------------------*/
/* Given a tabled subgoal, go through its list of deleted nodes (in the
 * completion stack), and reclaim the leaves and corresponding branches
 *----------------------------------------------------------------------*/

//int myctr = 0;

void  reclaim_del_ret_list(CTXTdeclc VariantSF sg_frame) {
  ALNptr x,y;

  x = compl_del_ret_list(subg_compl_stack_ptr(sg_frame));

  /*
  myctr++;

  if (myctr >= 50000 && myctr%20000 == 0) {
    ALNptr temp = 0x103cfa2e3;
    printf("%d: oreg %p complstk %p bot %p -- %p\n",myctr,openreg,complstack,COMPLSTACKBOTTOM,*temp);
  }
  
  if (x != NULL) {
    printf("openreg %p bot %p\n",openreg,COMPLSTACKBOTTOM );
    printf("%d: reclaim del ret list %p (%p @%p) for ",myctr,subg_compl_stack_ptr(sg_frame),
	   &compl_del_ret_list(subg_compl_stack_ptr(sg_frame)),x); 
    print_subgoal(stddbg,sg_frame); printf("\n");
  }
  */
  while (x != NULL) {
    y = x;
    x = ALN_Next(x);
    //    printf("x %p y %p\n",x,y);
/*      delete_branch(ALN_Answer(y), &subg_ans_root_ptr(sg_frame)); */
#ifndef MULTI_THREAD
    SM_DeallocateStruct(smALN,y);
#else
   if (IsSharedSF(sg_frame)) {			
     SM_DeallocateSharedStruct(smALN,y);
   } else {
     SM_DeallocateStruct(*private_smALN,y);
   }
#endif
  }
  compl_del_ret_list(subg_compl_stack_ptr(sg_frame)) = NULL;
}
 
/*----------------------------------------------------------------------*/

/*
**   Used in aggregs.P to implement aggregates.v
**   Takes:   breg (the place where choice point is saved) and arity.  
**   Returns: subgoal skeleton (i.e., ret(X,Y,Z), where X,Y,Z are all the 
**    	      	                distinct variables in the subgoal);
**   	      Pointer to the subgoal.
*/

void breg_retskel(CTXTdecl)
{
    Psc    psc;
    Cell    term;
    VariantSF sg_frame;
    CPtr    tcp, cptr, where;
    Integer breg_offset;
    int     i, template_size,attv_num,abstr_size;
    UNUSED(attv_num);
    UNUSED(abstr_size);

    breg_offset = ptoc_int(CTXTc 1);
    tcp = (CPtr)((Integer)(tcpstack.high) - breg_offset);
    sg_frame = (VariantSF)(tcp_subgoal_ptr(tcp));
    where = tcp_template(tcp);
    //    Nvars = int_val(cell(where)) & 0xffff;
    //    cptr = where - Nvars - 1;
#ifdef CALL_ABSTRACTION
    get_var_and_attv_nums(template_size, attv_num, abstr_size,int_val(cell(where)));
#else
    get_var_and_attv_nums(template_size, attv_num, int_val(cell(where)));
#endif
    //    cptr = where - (template_size + 2*abstr_size) - 1;
    cptr = where - (template_size) - 1;
    if (template_size == 0) {
      ctop_string(CTXTc 3, get_ret_string());
    } else {
      bind_cs((CPtr)ptoc_tag(CTXTc 3), hreg);
      psc = get_ret_psc((byte)template_size);
      new_heap_functor(hreg, psc);
      for (i = template_size; i > 0; i--) {
	term = (Cell)(*(CPtr)(cptr+i));
        nbldval_safe(term);
      }
    }
    ctop_int(CTXTc 4, (Integer)sg_frame);
}


/*======================================================================*/

/*
 *                    I N T E R N E D   T R I E S
 *                    ===========================
 */

/* Allocate an array of handles to interned tries, and initialize
   global variables. 

   In MT-engine default is private tries -- thus make private trie
   code and single-threaded trie code identical, and require shared
   tries only for MT engine.*/

#define ISTRIENULL(Field) ((Field) == -1)
#define TRIENULL -1

#ifndef MULTI_THREAD
int itrie_array_first_free;
//int itrie_array_last_free;
int itrie_array_first_trie  = TRIENULL;
struct interned_trie_t* itrie_array;
#else
int shared_itrie_array_first_free;
struct shared_interned_trie_t* shared_itrie_array;
#endif

/* Need to expand trie array on demand -- this is high up on list -- TLS.*/

int max_interned_tries_glc;

void init_private_trie_table(CTXTdecl) {
  int i ;
  
  itrie_array = 
    mem_calloc(max_interned_tries_glc+1, sizeof(struct interned_trie_t), TABLE_SPACE);

  for( i = 0; i < max_interned_tries_glc; i++ ) {
    //    itrie_array[i].valid = FALSE;
    itrie_array[i].next_entry = i+1;
    itrie_array[i].root = NULL;
  }
  itrie_array[max_interned_tries_glc].next_entry = -1;

  /* Set to 1 to avoid problems with storage_xsb.c which ues this (it
     returns a new trie with a 0 value) */
  itrie_array_first_free = 1;
  itrie_array_first_trie = -1;
}

#ifdef MULTI_THREAD
void init_shared_trie_table() {
  int i ;
  
  shared_itrie_array = 
    mem_calloc(max_interned_tries_glc+1, sizeof(struct shared_interned_trie_t), TABLE_SPACE);

  for( i = 0; i < max_interned_tries_glc; i++ ) {
      shared_itrie_array[i].next_entry = i+1;
      pthread_mutex_init(&shared_itrie_array[i].trie_mutex, &attr_rec_gl ) ;
  }
    shared_itrie_array[max_interned_tries_glc].next_entry = -1;

    // Set to 1 to avoid problems with storage_xsb.c which ues this (it
    //  returns a new trie with a 0 value) 
  shared_itrie_array_first_free = 1;
}
#endif

/*----------------------------------------------------------------------*/
// get_trie_type is directly in dynamic_code_function

Integer new_private_trie(CTXTdeclc int props) {
  Integer result = 0;
  int index;
  int type = TRIE_TYPE(props);

  if (itrie_array_first_free < 0) {
    xsb_resource_error(CTXTc "interned tries","newtrie",1);
    return 0;
  }
  else {
    itrie_array[itrie_array_first_free].valid = TRUE;
    itrie_array[itrie_array_first_free].root = NULL;
    itrie_array[itrie_array_first_free].type = type;  // needed for trie_property
    SET_TRIE_ID(itrie_array_first_free,type,result);
    index = itrie_array_first_free;

    itrie_array_first_free = itrie_array[itrie_array_first_free].next_entry;
    itrie_array[index].prev_entry = TRIENULL;
    if (!ISTRIENULL(itrie_array_first_trie)) {
      itrie_array[index].next_entry = itrie_array_first_trie;
      itrie_array[itrie_array_first_trie].prev_entry = index;
    }
    else  itrie_array[index].next_entry = TRIENULL;
    itrie_array_first_trie = index;

    if IS_INCREMENTAL_TRIE(props) {
	itrie_array[index].callnode = makecallnode(NULL);
	(itrie_array[index].callnode)->is_incremental_trie = 1;
	//	printf("callnode %p\n",itrie_array[index].callnode);
	initoutedges(CTXTc (callnodeptr)itrie_array[index].callnode);
	itrie_array[index].incremental = 1;
	//	printf("incremental\n");
      }

    return result;
  }
}

#ifdef MULTI_THREAD
Integer new_shared_trie(CTXTdeclc int type) {
  Integer result;

  if (shared_itrie_array_first_free < 0) {
    xsb_resource_error(CTXTc "shared interned tries","newtrie",1);
    return 0;
  }
  else {
    shared_itrie_array[itrie_array_first_free].valid = TRUE;
    shared_itrie_array[itrie_array_first_free].root = NULL;
    shared_itrie_array[itrie_array_first_free].type = type;
    SET_TRIE_ID(shared_itrie_array_first_free,SHAS_TRIE,result);
    shared_itrie_array_first_free = shared_itrie_array[shared_itrie_array_first_free].next_entry;
    return result;
  }
}
#endif

/*----------------------------------------------------------------------*/

/* The type argument to newtrie() uses bit-mapping to indicate whether
   the trie to be created 1) is thread-private or thread-shared; 2)
   can backtrack in a general manner or whether the trie must use a
   key (term) as in an associateive array; 3) is incremental or not.
   See trie_defs.h for declarations of the bit-maps. */

#ifdef MULTI_THREAD
Integer newtrie(CTXTdeclc int type) {
  if (PRIVATE_TRIE(type)) 
    return new_private_trie(CTXTc type);
  else return (int) new_shared_trie(CTXTc type);
}
#else
Integer newtrie(CTXTdeclc int type) {
  return new_private_trie(CTXTc type);
}
#endif

/*----------------------------------------------------------------------*/
/* i_trie_intern(_Term,_Root,_Leaf,_Flag,_Check_CPS,_Expand_or_not)     
* 
* If called from trie_intern(), we'll need to check to see whether its
* safe to expand -- hence check_cps_flag; if called from
* bulk_trie_intern() we don't need to check, and expand_flag will be
* set to tell us whether we can expand or not.
*/

int private_trie_intern(CTXTdecl) {
  prolog_term term;
  int Trie_id,index,type;
  int flag, check_cps_flag, expand_flag;
  BTNptr Leaf;

  Trie_id = (int)iso_ptoc_int(CTXTc 1,"trie_intern/2");
  term = ptoc_tag(CTXTc 2);
  check_cps_flag = (int)ptoc_int(CTXTc 5);
  expand_flag = (int)ptoc_int(CTXTc 6);

  switch_to_trie_assert;
  SPLIT_TRIE_ID(Trie_id,index,type);
  if (itrie_array[index].root != NULL && BTN_Instr(itrie_array[index].root) == trie_no_cp_fail) {
    printf("Inserting into trie with trie_no_cp_fail root\n");
  }

  Leaf = trie_intern_chk_ins(CTXTc term,&(itrie_array[index].root),
				 &flag,check_cps_flag,expand_flag);
  switch_from_trie_assert;

  SQUASH_LINUX_COMPILER_WARN(type);
  //  printf("root %p\n",itrie_array[index].root);
  if (Leaf) {
    ctop_int(CTXTc 3,(Integer)Leaf);
    ctop_int(CTXTc 4,flag);
    return(TRUE);
  }
  else return(FALSE);
}

#ifdef MULTI_THREAD
int shas_trie_intern(CTXTdecl) {
  prolog_term term;
  int Trie_id,index,type;
  BTNptr Leaf;
  int flag;

  Trie_id = iso_ptoc_int(CTXTc 2,"trie_intern/2");
  term = ptoc_tag(CTXTc 3);
  //  check_cps_flag = ptoc_int(CTXTc 6);
  //  expand_flag = ptoc_int(CTXTc 7);
  SPLIT_TRIE_ID(Trie_id,index,type);
  if (shared_itrie_array[index].root != NULL 
      && BTN_Instr(shared_itrie_array[index].root) == trie_no_cp_fail) {
    printf("Inserting into trie with trie_no_cp_fail root\n");
  }

  switch_to_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));
  Leaf = trie_intern_chk_ins(CTXTc term,&(shared_itrie_array[index].root),
			    &flag,NO_CPS_CHECK,EXPAND_HASHES);
  switch_from_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));
  
  if (Leaf) {
    ctop_int(CTXTc 4,(Integer)Leaf);
    ctop_int(CTXTc 5,flag);
    return(TRUE);
  }
  else return(FALSE);

}
#endif

/*----------------------------------------------------------------------*/
// TLS: check to see if attv safe
int private_trie_interned(CTXTdecl) {
  int ret_val = FALSE;
  Cell Leafterm, trie_term;
  int Trie_id,index,type;
  BTNptr *trie_root_addr;

  Trie_id = (int)iso_ptoc_int(CTXTc 1,"private_trie_interned/3");
  trie_term =  ptoc_tag(CTXTc 2);
 Leafterm = ptoc_tag(CTXTc 3);

  SPLIT_TRIE_ID(Trie_id,index,type);
  trie_root_addr = &(itrie_array[index].root);

  if(IsNonNULL(ptcpreg) && itrie_array[index].incremental) {
    VariantSF sf;
    callnodeptr thiscallnode;

    sf=(VariantSF)ptcpreg;
    if(IsIncrSF(sf)){
      thiscallnode = itrie_array[index].callnode;
      if(IsNonNULL(thiscallnode)){
	addcalledge(thiscallnode,sf->callnode);  
      }
    }
  }
  SQUASH_LINUX_COMPILER_WARN(type);
  if ((*trie_root_addr != NULL) && (!((Integer) *trie_root_addr & 0x3))) {
    XSB_Deref(trie_term);
    XSB_Deref(Leafterm);
    if ( isref(Leafterm) ) {  
      trieinstr_unif_stkptr = trieinstr_unif_stk -1;
      trieinstr_vars_num = -1;
      push_trieinstr_unif_stk(trie_term); 
      pcreg = (byte *)*trie_root_addr;
      ret_val =  TRUE;
    }
    else{
      xsb_instantiation_error(CTXTc "trie_interned/[2,4]",3);
    }
  }
  return(ret_val);
}

#ifdef MULTI_THREAD  
/* TLS: ensuring that LEAF is uninstantiated */
int shas_trie_interned(CTXTdecl) {
  int ret_val = FALSE;
  Cell Leafterm, trie_term;
  int Trie_id,index,type;
  BTNptr *trie_root_addr;

  Trie_id = iso_ptoc_int(CTXTc 2,"shas_trie_interned/[3]");
  trie_term =  ptoc_tag(CTXTc 3);
  Leafterm = ptoc_tag(CTXTc 4);

  SPLIT_TRIE_ID(Trie_id,index,type);
  trie_root_addr = &(shared_itrie_array[index].root);

  if ((*trie_root_addr != NULL) && (!((UInteger) *trie_root_addr & (UInteger) 0x3))) {
    XSB_Deref(trie_term);
    XSB_Deref(Leafterm);
    //    if ( isref(Leafterm) ) {  
      trieinstr_unif_stkptr = trieinstr_unif_stk -1;
      trieinstr_vars_num = -1;
      push_trieinstr_unif_stk(trie_term); 
      pcreg = (byte *)*trie_root_addr;
      ret_val =  TRUE;
      //    }
      //    else xsb_instantiation_error(CTXTc "trie_interned/[2,4]",3);
  }
  return(ret_val);
}
#endif

/*----------------------------------------------------------------------*/

/*
 * This is builtin #162: TRIE_UNINTERN(+ROOT, +LEAF), to dispose a branch
 * of the trie rooted at Set_ArrayPtr[ROOT].
 * 
 * If called within a trie_retractall(), the CPS check will already
 * have been done: if it isn't safe to reclaim trie_dispose_nr() would
 * have been called; otherwise its safe and we don't need to check
 * here.  If called from within a trie_unintern() (maybe we should get
 * the names straight) we do need to check.
 */

void private_trie_unintern(CTXTdecl)
{
  BTNptr Leaf;
  int disposalType;
  int Trie_id,index,type;
  BTNptr *trie_root_addr;

  Trie_id = (int)iso_ptoc_int(CTXTc 1,"trie_unintern/2");
  Leaf = (BTNptr)iso_ptoc_int(CTXTc 2,"trie_unintern/2");
  disposalType = (int)ptoc_int(CTXTc 3);
 
  SPLIT_TRIE_ID(Trie_id,index,type);
  trie_root_addr = &(itrie_array[index].root);
  switch_to_trie_assert;

  if (disposalType == NO_CPS_CHECK)     
    delete_branch(CTXTc Leaf, trie_root_addr,VARIANT_EVAL_METHOD);
  else {
    if (!interned_trie_cps_check(CTXTc *trie_root_addr)) {
      //          printf(" really deleting branch \n");
      delete_branch(CTXTc Leaf, trie_root_addr,VARIANT_EVAL_METHOD);
    }
    else {
      //           printf(" safely deleting branch\n");
      safe_delete_branch(Leaf);
    }
  }
  SQUASH_LINUX_COMPILER_WARN(type);
  switch_from_trie_assert;
}

#ifdef MULTI_THREAD
// For shared, disposalType is always NO_CPS_CHECK
void shas_trie_unintern(CTXTdecl)
{
  BTNptr Leaf;
  int Trie_id,index,type;

  Trie_id = iso_ptoc_int(CTXTc 2,"trie_unintern/2");
  Leaf = (BTNptr)iso_ptoc_int(CTXTc 3,"trie_unintern/2");
  //  disposalType = ptoc_int(CTXTc 3);
 
  SPLIT_TRIE_ID(Trie_id,index,type);
  switch_to_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));;

  delete_branch(CTXTc Leaf, &(shared_itrie_array[index].root),VARIANT_EVAL_METHOD);
  switch_from_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));
}
#endif

/*----------------------------------------------------------------------*/
/* 
 * For interned tries, I'm checking whether there is a trie choice
 * point with the same root as the the trie that we want to do
 * something with.  
 */

#define CAN_RECLAIM 0
#define CANT_RECLAIM 1

int interned_trie_cps_check(CTXTdeclc BTNptr root) 
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  BTNptr pLeaf, trieNode;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      // Below we want interned_trie_tt
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInInternTrie(trieNode)) {
	pLeaf = trieNode;
	while ( IsNonNULL(pLeaf) && (! IsTrieRoot(pLeaf))) {
		//&& 		((int) TN_Instr(pLeaf) != trie_fail_unlock) ) {
	  pLeaf = BTN_Parent(pLeaf);
	}
	if (pLeaf == root) {
	  return CANT_RECLAIM;
	}
      }
    }
    cp_top1 = cp_prevtop(cp_top1);
  }
  return CAN_RECLAIM;
}


/*----------------------------------------------------------------------*/

/* Here, we check to see that the trie is non empty (via its root) and
   also valid (so that the free list may be rearranged.  If it is a
   pras trie, we do not need a CPS check.

   Actually, if the trie is not valid we just succeed -- arguably an
   error might be thrown.
 */

void private_trie_truncate(CTXTdeclc struct interned_trie_t* trie_root_addr, 
			   int index, int cps_check) {
  if ((trie_root_addr->root != NULL) &&
      (!((Integer) trie_root_addr->root & 0x3))) {
    if (cps_check == CPS_CHECK) {
      if (!interned_trie_cps_check(CTXTc trie_root_addr->root)) {
	switch_to_trie_assert;
	delete_trie(CTXTc trie_root_addr->root);
	switch_from_trie_assert;
      }
      else xsb_abort("Backtracking through trie to be abolished (in trie_drop/1, trie_truncate/1 or retractall/1 for trie-asserted code).");
    } else delete_trie(CTXTc trie_root_addr->root);
    trie_root_addr->root = NULL;
  }
}

#ifdef MULTI_THREAD
void shas_trie_truncate(CTXTdeclc struct shared_interned_trie_t* trie_root_addr, int index) {
  if ((trie_root_addr->root != NULL) &&
      (!((Integer) trie_root_addr->root & 0x3))) {
    switch_to_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));;
    delete_trie(CTXTc trie_root_addr->root);
    switch_from_shared_trie_assert(&(shared_itrie_array[index].trie_mutex));
    trie_root_addr->root = NULL;
  }
}
#endif

void trie_truncate(CTXTdeclc Integer Trie_id) {
  int index,type;
  
  SPLIT_TRIE_ID(Trie_id,index,type);
#ifdef MULTI_THREAD  
  if (type == PRGE_TRIE) {
    private_trie_truncate(CTXTc &(itrie_array[index]),index,CPS_CHECK);
  } else if (type == PRAS_TRIE) {
    private_trie_truncate(CTXTc &(itrie_array[index]),index,NO_CPS_CHECK);
  } else 
      shas_trie_truncate(CTXTc &(shared_itrie_array[index]),index);
#else
  if (type == PRGE_TRIE) {
    private_trie_truncate(CTXTc &(itrie_array[index]),index,CPS_CHECK);
  } else if (type == PRAS_TRIE) {
    private_trie_truncate(CTXTc &(itrie_array[index]),index,NO_CPS_CHECK);
  }
#endif
}

static void private_trie_drop(CTXTdeclc int i ) {
  int j;
	/* delete from trie list */
  if( !ISTRIENULL(itrie_array[i].prev_entry) ) {
    j = itrie_array[i].prev_entry;
    itrie_array[j].next_entry = itrie_array[i].next_entry ;
  }
  if( !ISTRIENULL(itrie_array[i].next_entry)) {
    j = itrie_array[i].next_entry;
    itrie_array[j].prev_entry = itrie_array[i].prev_entry ;
  }
  if( itrie_array_first_trie == i )
    itrie_array_first_trie = itrie_array[i].next_entry ;

  /* add to free list */
  if (ISTRIENULL(itrie_array_first_free)) {
    itrie_array_first_free = i;
    itrie_array[i].next_entry = TRIENULL;
  }
  else {	
    itrie_array[i].next_entry = itrie_array_first_free;
    itrie_array_first_free = i;
  }
  itrie_array[i].valid = FALSE;
}

#ifdef MULTI_THREAD
// Chaining not yet implemented for shared tries
static void shared_trie_drop(CTXTdeclc int i ) {
  shared_itrie_array[i].valid = FALSE;
}
#endif

/* truncate trie and remove properties */
void trie_drop(CTXTdecl) {
  int index,type;

  int Trie_id = (int)iso_ptoc_int(CTXTc 2,"trie_drop/1");
  trie_truncate(CTXTc Trie_id);
  SPLIT_TRIE_ID(Trie_id,index,type);
  if (PRIVATE_TRIE(type)) 
    private_trie_drop(CTXTc index);
#ifdef MULTI_THREAD
  else shared_trie_drop(CTXTc index);
#endif
}

/* * * * * * * * * * * * * * * * * * */

void first_trie_property(CTXTdecl) {
  int index,type;
  Integer Trie_id;

  if (!ISTRIENULL(itrie_array_first_trie)) {
    index = (int) (itrie_array_first_trie);

    if( !itrie_array[index].valid ) 
      xsb_existence_error(CTXTc "trie", reg[2], "trie_property", 1,1);

    type = itrie_array[index].type;
    ctop_int(CTXTc 3, type);
    SET_TRIE_ID(index,type,Trie_id);
    ctop_int(CTXTc 2, Trie_id);
    ctop_int(CTXTc 4, itrie_array[index].next_entry);
  }
  else ctop_int(CTXTc 2, TRIENULL);
}

void next_trie_property(CTXTdecl) {
  int index = (int)ptoc_int(CTXTc 2);
  ctop_int(CTXTc 3, itrie_array[index].type);
  ctop_int(CTXTc 4,itrie_array[index].next_entry);
}

/*----------------------------------------------------------------------*/
/*

TLS -- 4/07 I'm keeping these in for now, but they may be taken out
when/if the interned tries evolve.

 Changes made by Prasad Rao. Jun 20th 2000

 The solution for reclaiming the garbage nodes resulting
 from trie dispose is as follows.
 Maintain a datastructure as follows
 1)  IGRhead -> Root1 -> Root2 -> Root3 -> null
                 |        |        |
                 |        |        |
                 v        v        v
                Leaf11   Leaf21   Leaf31
	         |        |        |  
                 |        |        |
                 V        v        v
                Leaf12    null    Leaf32        
                 |                 |
                 v                 |
                null               v
                                 Leaf33
                                   |
                                   v
                                  null
To reclaim all the garbage associated with a particular root
 a) remove the root from the root list
 b) remove all the garbage branches assoc with the root 
    by calling delete_branch(leaf,....)
   Done!!

*/

#ifndef MULTI_THREAD
static IGRptr IGRhead = NULL;
#endif

static IGRptr newIGR(Integer root)
{
  IGRptr igr;
  
  igr = (IGRptr) mem_alloc(sizeof(InternGarbageRoot),TABLE_SPACE);
  igr -> root   = root;
  igr -> leaves = NULL;
  igr -> next   = NULL;
  return igr;
}

static IGLptr newIGL(BTNptr leafn)
{
  IGLptr igl;
  
  igl = (IGLptr) mem_alloc(sizeof(InternGarbageLeaf),TABLE_SPACE);
  igl -> leaf = leafn;
  igl -> next = NULL;
  return igl;
}

static IGRptr getIGRnode(CTXTdeclc Integer rootn)
{
  IGRptr p = IGRhead;  

  while(p != NULL){
    if(p -> root == rootn)
      return p;
    else
      p = p -> next;
  }  
  if(p != NULL)
    xsb_warn(CTXTc "Invariant p == NULL violated");

  p = newIGR(rootn);
  p -> next = IGRhead;
  IGRhead = p;    
  return p;
}

static IGRptr getAndRemoveIGRnode(CTXTdeclc Integer rootn)
{
  IGRptr p = IGRhead;  

  if(p == NULL)
    return NULL;
  else if(p -> root == rootn){
    IGRhead = p -> next;
    return p;
  }
  else{
    IGRptr q = p;
    p = p -> next;
    while(p != NULL){
      if(p -> root == rootn){
	q -> next = p -> next;
	return p;
      } else{
	q = p;
	p = p -> next;
      }
    }  
  }
  xsb_dbgmsg((LOG_INTERN, "Root node not found in Garbage List"));
  return NULL;
}

/*
 *  Insert "leafn" into the garbage list, "r".
 *  This is done when leafn is deleted so that we could undelete it or later
 *  garbage-collect it.
 */
static void insertLeaf(IGRptr r, BTNptr leafn)
{
  /* Just make sure that the leaf is not already there */
  IGLptr p;

  if(r == NULL)
    return;
#ifdef UNDEFINED
  p = r -> leaves;
  while(p != NULL){
    if(p -> leaf == leafn){
      /* The following should be permitted, because we should be able to
	 backtrackably delete backtrackably deleted nodes (which should have no
	 effect)
      */
      if (IsDeletedNode(leafn))
	xsb_dbgmsg((LOG_INTERN,
		   "The leaf node being deleted has already been deleted"));
      return;
    }
    p = p -> next;
  }
#endif
  p = newIGL(leafn);
  p -> next = r -> leaves;
  r -> leaves = p;
}

/*
  This feature does not yet support shared tries or call subsumption.
 */
void reclaim_uninterned_nr(CTXTdeclc Integer rootidx)
{
  IGRptr r = getAndRemoveIGRnode(CTXTc rootidx);
  IGLptr l, p;
  BTNptr leaf;

  if (r!=NULL)
    l = r-> leaves;
  else
    return;

  mem_dealloc(r,sizeof(InternGarbageRoot),TABLE_SPACE);

  while(l != NULL){
    /* printf("Loop b %p\n", l); */
    leaf = l -> leaf;
    p = l -> next;
    mem_dealloc(l,sizeof(InternGarbageLeaf),TABLE_SPACE);
    switch_to_trie_assert;
    if(IsDeletedNode(leaf)) {
      //      SYS_MUTEX_LOCK(MUTEX_TRIE);
      delete_branch(CTXTc leaf, &(itrie_array[rootidx].root),VARIANT_EVAL_METHOD);
      //      SYS_MUTEX_UNLOCK(MUTEX_TRIE);
    } else {
      /* This is allowed:
	 If we backtrack over a delete, the node that was marked for deletion
	 and placed in the garbage list is unmarked, but isn't removed from
	 the garbage list. So it is a non-deleted node on the garbage list.
	 It is removed from there only when we reclaim space.
      */
      xsb_dbgmsg((LOG_INTERN,"Non deleted interned node in garbage list - ok"));
    }

    switch_from_trie_assert;
    l = p;
  }
}

/*----------------------------------------------------------------------*/
/*
 * This is builtin : TRIE_DISPOSE_NR(+ROOT, +LEAF), to
 * mark for disposal (safe delete) a branch
 * of the trie rooted at Set_ArrayPtr[ROOT].
 * This function does not yet support shared tries.
 */
void trie_dispose_nr(CTXTdecl)
{
  BTNptr Leaf;
  Integer Rootidx;

  Rootidx = iso_ptoc_int(CTXTc 1,"trie_unintern_nr/2");
  Leaf = (BTNptr)iso_ptoc_int(CTXTc 2,"trie_unintern_nr/2");
  switch_to_trie_assert;
  //  SYS_MUTEX_LOCK(MUTEX_TRIE);
  insertLeaf(getIGRnode(CTXTc Rootidx), Leaf);
  //  SYS_MUTEX_UNLOCK(MUTEX_TRIE);
  safe_delete_branch(Leaf);
  switch_from_trie_assert;
}

/*----------------------------------------------------------------------*/
/*
 * This is builtin : TRIE_UNDISPOSE_NR(+ROOT, +LEAF), to
 * unmark a safely deleted branch.
 * This function does not yet support shared tries.
 */

void trie_undispose(CTXTdeclc Integer rootIdx, BTNptr leafn)
{
  IGRptr r = getIGRnode(CTXTc rootIdx);
  IGLptr p = r -> leaves;
  if(p == NULL){
    xsb_dbgmsg((LOG_INTERN,
   "In trie_undispose: The node being undisposed has been previously deleted"));
  } else{
    if(p -> leaf == leafn){
      r -> leaves = p -> next;
      mem_dealloc(p,sizeof(InternGarbageLeaf),TABLE_SPACE);
      if(r -> leaves == NULL){
	/* Do not want roots with no leaves hanging around */
	getAndRemoveIGRnode(CTXTc rootIdx);
      }
    }
    undelete_branch(leafn);
  }
}

/*======================================================================*/

/*
 *              TABLE ABOLISHING AND GARBAGE COLLECTING 
 *                    ===========================
 */

/*----------------------------------------------------------------------*/
/* 

 * When a table T is abolished, various checks must be made before its
 * space can be reclaimed.  T must be completed, and it must be
 * ensured that there are not any trie choice points for T in the
 * choice point stack.  In addition, the heap delay list must be
 * checked for pointers to T.  And finally, a check must be made that
 * there is a single active thread in order to reclaim shared tables.
 *
 * In the case of abolish_all_tables, if there are any incomplete
 * tables, or if there are trie nodes for completed tables on the
 * choice point stack, an error is thrown.  In the case of
 * abolish_table_pred(P) (and other abolishes), if P is not completed
 * an error is thrown; while if trie choice points for P are on the
 * stack, P is "abolished" (pointers in the TIF are reset) but its
 * space is not yet reclaimed.  Rather, a deleted table frame (DelTF)
 * is set up so that P can later be reclaimed upon a call to
 * gc_tables/1.  The same action is also taken if P is shared and
 * there is more than one active thread.  Note that if we have to
 * create a DelTF for them, shared tables will not be gc'd until we're
 * down to a single thread, so its best to call these abolishes when
 * we dont have any more backtracking points.
 *
 * Later, on a call to gc_tables/1 (which affects shared tables only
 * if there is a single active thread), the choice point stacks may be
 * traversed to mark those DelTF frames corresponding to tables with
 * trie CPs in the CP stack.  Once this is done, the chain of DelTF
 * frames is traversed to reclaim tables for those unmarked DelTF
 * frames (and free the frames) as well as to unmark the marked DelTF
 * frames.
 * 
 * Note that all of these require SLG_GC to be defined as we need to
 * properly traverse the CPS.  So, probably we should take out SLG_GC.
 */

/*------------------------------------------------------------------*/
/* Utility Code */
/*------------------------------------------------------------------*/

/* used by mt engine for shared tables */
DelTFptr deltf_chain_begin = (DelTFptr) NULL;

/* - - - - - */

xsbBool is_completed_table(TIFptr tif) {
  VariantSF sf;

  for ( sf = TIF_Subgoals(tif);  IsNonNULL(sf);  
	sf = (VariantSF)subg_next_subgoal(sf) )
    if ( ! is_completed(sf) )
      return FALSE;
  return TRUE;
}

/* - - - - - */


int trie_warning_message_array[9][6];

/* below used for table gc; Need different access method if pointing
   to the trie_fail instruction (used in incremental tabling).  See
   tc_insts_xsb_i.h which tells how to access the tif from the related
   cp.*/

Psc get_psc_for_trie_cp(CTXTdeclc CPtr cp_ptr, BTNptr trieNode) {
  BTNptr  LocNodePtr;   TIFptr tif_ptr;
  if (IsInAnswerTrie((trieNode))) {
    while ( IsNonNULL(trieNode) && (! IsTrieRoot(trieNode)) & ((int) TN_Instr(trieNode) != trie_fail) ) {
      trieNode = BTN_Parent(trieNode);
    }
    if (TN_Parent(trieNode) && TN_Instr(trieNode) != trie_fail) { 
      tif_ptr = subg_tif_ptr(TN_Parent(trieNode));
      //    printf("Predicate is %s/%d\n",get_name(TIF_PSC(tif_ptr)),
      //    get_arity(TIF_PSC(tif_ptr)));
      return TIF_PSC(tif_ptr);
    } 
    else { 
      if (trie_warning_message_array[TN_NodeType(trieNode)][TN_TrieType(trieNode)] == 0) {
	fprintf(stderr,"Null parent ptr for TN Root Node type: %s Trie type %s\n",
		trie_node_type_table[TN_NodeType(trieNode)], trie_trie_type_table[TN_TrieType(trieNode)]);
	trie_warning_message_array[TN_NodeType(trieNode)][TN_TrieType(trieNode)] = 1;
      }
    return NULL;
    }
  } else if ((int) TN_Instr(trieNode) == trie_fail) { 
    LocNodePtr    = (BTNptr) *(cp_ptr + CP_SIZE);
    return TIF_PSC(subg_tif_ptr(BTN_Parent(LocNodePtr)));
  } else {
      if (trie_warning_message_array[TN_NodeType(trieNode)][TN_TrieType(trieNode)] == 0) {
	fprintf(stderr,"Non-answer trie instr in cp stack: TN Root Node type: %s Trie type %s\n",
		trie_node_type_table[TN_NodeType(trieNode)], trie_trie_type_table[TN_TrieType(trieNode)]);
	trie_warning_message_array[TN_NodeType(trieNode)][TN_TrieType(trieNode)] = 1;
      }
    return NULL;
  }
}

/* - - - - - */

// -- old -- VariantSF get_subgoal_frame_for_answer_trie_cp(CTXTdeclc BTNptr pLeaf) 
// {
// 
//   while ( IsNonNULL(pLeaf) && (! IsTrieRoot(pLeaf)) && ((int) TN_Instr(pLeaf) != trie_fail) ) {
//     pLeaf = BTN_Parent(pLeaf);
//   }
// 
//   if (TN_Parent(pLeaf)) { /* workaround till all roots pointing to subg's */
//     return (VariantSF) TN_Parent(pLeaf);
//   } else {
//     fprintf(stderr,"Null parent ptr for TN Root Node type: %d Trie type %d\n",
// 	    TN_TrieType(pLeaf), TN_NodeType(pLeaf));
//     return NULL;
//   }
//}

VariantSF get_subgoal_frame_for_answer_trie_cp(CTXTdeclc BTNptr pLeaf, CPtr tbreg)
{
  while ( IsNonNULL(pLeaf) && (! IsTrieRoot(pLeaf)) && ((int) TN_Instr(pLeaf) != trie_fail) ) {
    //    printf("Getting ptn_parent %p\n",pLeaf);                                                                                            
    pLeaf = BTN_Parent(pLeaf);
  }
  if (TN_Instr(pLeaf) == trie_fail) {
    //    printf("returning for trie_fail\n");
    return  (VariantSF) BTN_Parent((BTNptr) *(tbreg + CP_SIZE));
  }
  if (TN_Parent(pLeaf)) { /* workaround till all roots pointing to subg's */
    return (VariantSF) TN_Parent(pLeaf);
  } else {
    fprintf(stderr,"Null parent ptr for TN Root Node type: %d Trie type %d\n",
            TN_TrieType(pLeaf), TN_NodeType(pLeaf));
    return NULL;
  }
}

/* - - - - - */

/* TLS: this routine is called from mark_cp_tabled_preds() and similar
   functions, which traverse the CP stack.  They were core-dumping if
   they found a trie_fail on the CP stack -- hence the special case.
 */

TIFptr get_tif_for_answer_trie_cp(CTXTdeclc BTNptr pLeaf,CPtr tbreg)
{
  while ( IsNonNULL(pLeaf) && (! IsTrieRoot(pLeaf)) && ((int) TN_Instr(pLeaf) != trie_fail) )  {
    // &&      ((int) TN_Instr(pLeaf) != trie_fail_unlock) ) {
    pLeaf = BTN_Parent(pLeaf);
  }
  if (TN_Instr(pLeaf) == trie_fail) {
    //      printf("returning for trie_fail\n");
      return subg_tif_ptr(BTN_Parent((BTNptr) *(tbreg + CP_SIZE)));
      //          return NULL;
  }
  else return subg_tif_ptr(TN_Parent(pLeaf));
  
}

/* - - - - - */

BTNptr get_call_trie_from_subgoal_frame(CTXTdeclc VariantSF subgoal)
{
  
  BTNptr pLeaf = subg_leaf_ptr(subgoal);

  while ( IsNonNULL(BTN_Parent(pLeaf)) ) {
    pLeaf = BTN_Parent(pLeaf);
  }
  return pLeaf;
}

/* - - - - - */

/* Creating two doubly-linked chains -- one for all DelTf, the other
   for Deltfs for this predicate.  Depending on the value of
   *chain_begin this can be used for either private or shared
   predicates */
static inline DelTFptr New_DelTF_Pred(CTXTdeclc TIFptr pTIF, DelTFptr *chain_begin, xsbBool Warn) {		      
  DelTFptr pDTF;
  
  pDTF = (DelTFptr)mem_alloc(sizeof(DeletedTableFrame),TABLE_SPACE);	
  if ( IsNULL(pDTF) )							
    xsb_resource_error_nopred("memory" "Ran out of memory in allocation of DeletedTableFrame","");  
   DTF_CallTrie(pDTF) = TIF_CallTrie(pTIF);				
   DTF_Subgoals(pDTF) = TIF_Subgoals(pTIF);				
   DTF_Type(pDTF) = DELETED_PREDICATE;					
   DTF_Warn(pDTF) = (byte) Warn;					
   DTF_Mark(pDTF) = 0;                                                  
   DTF_PrevDTF(pDTF) = 0;						
   DTF_PrevPredDTF(pDTF) = 0;						
   DTF_NextDTF(pDTF) = *chain_begin;				
   DTF_NextPredDTF(pDTF) = TIF_DelTF(pTIF);				
   if (*chain_begin) DTF_PrevDTF(*chain_begin) = pDTF;	
   if (TIF_DelTF(pTIF))  DTF_PrevPredDTF(TIF_DelTF(pTIF)) = pDTF;	
   *chain_begin = pDTF;                                            
   TIF_DelTF(pTIF) = pDTF;                                              
   return pDTF;
}

static inline DelTFptr New_DelTF_Subgoal(CTXTdeclc TIFptr pTIF, VariantSF pSubgoal,
			      DelTFptr *chain_begin, xsbBool Warn) { 
  DelTFptr pDTF;

  pDTF = (DelTFptr)mem_alloc(sizeof(DeletedTableFrame),TABLE_SPACE);	
   if ( IsNULL(pDTF) )			
     xsb_resource_error_nopred("memory", "Ran out of memory in allocation of DeletedTableFrame");
   DTF_CallTrie(pDTF) = NULL;						
   DTF_Subgoal(pDTF) = pSubgoal;					
   DTF_Type(pDTF) = DELETED_SUBGOAL;					
   DTF_Mark(pDTF) = 0;                                                  
   DTF_Warn(pDTF) = (byte) Warn;					
   DTF_PrevDTF(pDTF) = 0;						
   DTF_PrevPredDTF(pDTF) = 0;						
   DTF_NextDTF(pDTF) = *chain_begin;			
   DTF_NextPredDTF(pDTF) = TIF_DelTF(pTIF);				
   if (*chain_begin) DTF_PrevDTF(*chain_begin) = pDTF;			
   if (TIF_DelTF(pTIF))  DTF_PrevPredDTF(TIF_DelTF(pTIF)) = pDTF;	
   *chain_begin = pDTF;					
   TIF_DelTF(pTIF) = pDTF;                                              
   return pDTF;
  }

/* If there is a deltf with same subgoals and arity (can this be) dont
   add; otherwise if there is a subgoal for this pred, delete the
   deltf (it must be for this generation of the table)
*/
void check_insert_global_deltf_pred(CTXTdeclc TIFptr tif, xsbBool Warning) { 
  DelTFptr dtf = TIF_DelTF(tif), next_dtf; 
  BTNptr call_trie = TIF_CallTrie(tif); 
  VariantSF subgoals = TIF_Subgoals(tif); 
  int found = 0;

  SYS_MUTEX_LOCK(MUTEX_TABLE);
  while ( dtf != 0 ) {
    next_dtf = DTF_NextPredDTF(dtf);
    if (DTF_Type(dtf) == DELETED_PREDICATE && 
	DTF_CallTrie(dtf) == call_trie && DTF_Subgoals(dtf) == subgoals)
      found = 1;
    if (DTF_Type(dtf) == DELETED_SUBGOAL) {
      //      fprintf(stderr,"Predicate over-riding subgoal for %s/%d\n",
      //      get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
      Free_Global_DelTF_Subgoal(dtf,tif);
    }
    dtf = next_dtf;
  }
  if (!found) {
    New_DelTF_Pred(CTXTc tif,&deltf_chain_begin,Warning);
  }
  TIF_CallTrie(tif) = NULL;
  TIF_Subgoals(tif) = NULL;
  SYS_MUTEX_UNLOCK(MUTEX_TABLE);
}

void check_insert_global_deltf_subgoal(CTXTdeclc VariantSF subgoal,xsbBool Warning) {
  DelTFptr dtf;
  TIFptr tif;

  if (!DELETED_SUBGOAL_FRAME(subgoal)) {
      SYS_MUTEX_LOCK(MUTEX_TABLE);

      DELETE_SUBGOAL_FRAME(subgoal); /* set DELETED bit */

      tif = subg_tif_ptr(subgoal);
      //      printf("check_insert_subgoal s %p t %p\n",subgoal,tif);

      dtf = New_DelTF_Subgoal(CTXTc tif,subgoal,&deltf_chain_begin,Warning);

      if (subg_prev_subgoal(subgoal) != 0) 
	subg_prev_subgoal(subgoal) = subg_next_subgoal(subgoal);

      if (subg_next_subgoal(subgoal) != 0) 
	subg_next_subgoal(subgoal) = subg_prev_subgoal(subgoal);

      subg_deltf_ptr(subgoal) = dtf;

      SYS_MUTEX_UNLOCK(MUTEX_TABLE);
    }
}

#ifdef MULTI_THREAD

// extern void printTIF(TIFptr);

void check_insert_private_deltf_pred(CTXTdeclc TIFptr tif, xsbBool Warning) {
  DelTFptr dtf = TIF_DelTF(tif), next_dtf;
  BTNptr call_trie = TIF_CallTrie(tif);
  VariantSF subgoals = TIF_Subgoals(tif);	
  int found = 0;

  while ( dtf != 0 ) {
    next_dtf = DTF_NextPredDTF(dtf);
    if (DTF_Type(dtf) == DELETED_PREDICATE && 
	DTF_CallTrie(dtf) == call_trie && DTF_Subgoals(dtf) == subgoals)
      found = 1;
    if (DTF_Type(dtf) == DELETED_SUBGOAL) {
      //            fprintf(stderr,"Predicate over-riding subgoal for %s/%d\n",
      //            get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
      Free_Private_DelTF_Subgoal(dtf,tif);
    }
    dtf = next_dtf;
  }
  if (!found) {
    //    New_Private_DelTF_Pred(CTXTc dtf,tif,Warning);
    New_DelTF_Pred(CTXTc tif,&private_deltf_chain_begin,Warning);
  }
  TIF_CallTrie(tif) = NULL;
  TIF_Subgoals(tif) = NULL;
}

#define check_insert_shared_deltf_pred(context, tif, warning)	\
  check_insert_global_deltf_pred(context, tif, warning)	 

/* * * * * * * */

void check_insert_private_deltf_subgoal(CTXTdeclc VariantSF subgoal,xsbBool Warning)
{
  DelTFptr dtf;
  TIFptr tif;

  if (!DELETED_SUBGOAL_FRAME(subgoal)) {

    DELETE_SUBGOAL_FRAME(subgoal); /* set DELETED bit */

    tif = subg_tif_ptr(subgoal);
    dtf = New_DelTF_Subgoal(CTXTc tif,subgoal,&private_deltf_chain_begin,Warning);

    if (subg_prev_subgoal(subgoal) != 0) 
      subg_prev_subgoal(subgoal) = subg_next_subgoal(subgoal);

    if (subg_next_subgoal(subgoal) != 0) 
      subg_next_subgoal(subgoal) = subg_prev_subgoal(subgoal);
    
    subg_deltf_ptr(subgoal) = dtf;
  }

}

#define check_insert_shared_deltf_subgoal(context, subgoal,Warning)	\
  check_insert_global_deltf_subgoal(context, subgoal,Warning)	 

#else /* not MULTI_THREAD */

#define check_insert_private_deltf_pred(tif,warning)	\
  check_insert_global_deltf_pred(tif,warning)	 

#define check_insert_private_deltf_subgoal(subgoal,Warning)	\
  check_insert_global_deltf_subgoal(subgoal,Warning)	 

#endif

/* - - - - - - - - - - */

/* used for transitive abolishes */
static inline void mark_delaylist_tabled_preds(CTXTdeclc CPtr dlist) {
  Cell tmp_cell;
  VariantSF subgoal;

  //  if (dlist != NULL) {
  //    printf("checking list ");print_delay_list(CTXTc stddbg, dlist);
    while (islist(dlist)) {
      dlist = clref_val(dlist);
      // printf("\n checking element "); print_delay_element(CTXTc stddbg, cell(dlist));
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      //      printf(" mark subgoal ");print_subgoal(stddbg, subgoal);
      gc_mark_tif(subg_tif_ptr(subgoal));
      dlist = (CPtr)cell(dlist+1);
    }
  }

static inline void unmark_delaylist_tabled_preds(CTXTdeclc CPtr dlist) {
  Cell tmp_cell;
  VariantSF subgoal;

  while (islist(dlist)) {
    dlist = clref_val(dlist);
    tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
    subgoal = (VariantSF) addr_val(tmp_cell);
    gc_unmark_tif(subg_tif_ptr(subgoal));
    dlist = (CPtr)cell(dlist+1);
  }
}

/* Used for predicate-level abolishes */
void mark_cp_tabled_preds(CTXTdecl)
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  TIFptr tif;
  BTNptr trieNode;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	tif = get_tif_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	if (tif) gc_mark_tif(tif);
      }
    }
    mark_delaylist_tabled_preds(CTXTc cp_pdreg(cp_top1));
    cp_top1 = cp_prevtop(cp_top1);
  }
}

void unmark_cp_tabled_preds(CTXTdecl)
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  TIFptr tif;
  BTNptr trieNode;
  
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	tif = get_tif_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	if (tif) gc_unmark_tif(tif);
      }
    }
    unmark_delaylist_tabled_preds(CTXTc cp_pdreg(cp_top1));
    cp_top1 = cp_prevtop(cp_top1);
  }
}

/*------------------------------------------------------------------*/
/* abolish_table_call() and supporting code */
/*------------------------------------------------------------------*/

/* Used when deleting a single subgoal
   Recurse through CP stack looking for trie nodes that match PSC.
*/
int abolish_table_call_cps_check(CTXTdeclc VariantSF subgoal) {
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  BTNptr trieNode;
  CPtr dlist;
  Cell tmp_cell;

  if (IsIncrSF(subgoal)) {
    if (subg_visitors(subgoal)) return CANT_RECLAIM;
    else return CAN_RECLAIM;
  }
				  
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      // Below we want basic_answer_trie_tt, ts_answer_trie_tt
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	if (subgoal == get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1))
	  return CANT_RECLAIM;
      }
    }
    /* now check the delay list for the call */
    dlist = cp_pdreg(cp_top1);
    while (islist(dlist)) {
      dlist = clref_val(dlist);
      // printf("\n checking element "); print_delay_element(CTXTc stddbg, cell(dlist));
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      if (subgoal == (VariantSF) addr_val(tmp_cell)) 
	return CANT_RECLAIM;
      dlist = (CPtr)cell(dlist+1);
      }
    cp_top1 = cp_prevtop(cp_top1);
  }
  return CAN_RECLAIM;
}

/* used for transitive abolishes */
static inline void mark_delaylist_tabled_subgoals(CTXTdeclc CPtr dlist) {
  Cell tmp_cell;
  VariantSF subgoal;

  //  if (dlist != NULL) {
  //    printf("checking list ");print_delay_list(CTXTc stddbg, dlist);
    while (islist(dlist)) {
      dlist = clref_val(dlist);
      // printf("\n checking element "); print_delay_element(CTXTc stddbg, cell(dlist));
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      //      printf(" mark subgoal ");print_subgoal(stddbg, subgoal);
      GC_MARK_SUBGOAL(subgoal);
      dlist = (CPtr)cell(dlist+1);
    }
  }

static inline void unmark_delaylist_tabled_subgoals(CTXTdeclc CPtr dlist) {
  Cell tmp_cell;
  VariantSF subgoal;

  while (islist(dlist)) {
    dlist = clref_val(dlist);
    tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
    subgoal = (VariantSF) addr_val(tmp_cell);
    //    printf(" unmark subgoal ");print_subgoal(stddbg, subgoal);
    GC_UNMARK_SUBGOAL(subgoal);
    dlist = (CPtr)cell(dlist+1);
  }
}

/* Mark and unmark are used when deleting a set of depending subgoals */
void mark_cp_tabled_subgoals(CTXTdecl) {
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  VariantSF subgoal;
  BTNptr trieNode;
  
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      //  printf("found trie instruction %x %d\n",cp_inst,
      //                 TSC_TrieType(((BTNptr)string_val(*(cp_top1+CP_SIZE+1)))));
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	//	printf("is in answer trie\n");
	subgoal = get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	//       	printf("Marking ");print_subgoal(CTXTc stddbg, subgoal);printf("\n");
	GC_MARK_SUBGOAL(subgoal);
      }
    }
    mark_delaylist_tabled_subgoals(CTXTc cp_pdreg(cp_top1));
    cp_top1 = cp_prevtop(cp_top1);
  }
}

void unmark_cp_tabled_subgoals(CTXTdecl)
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  VariantSF subgoal;
  BTNptr trieNode;
  
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	subgoal = get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	GC_UNMARK_SUBGOAL(subgoal);
      }
    }
    unmark_delaylist_tabled_subgoals(CTXTc cp_pdreg(cp_top1));
    cp_top1 = cp_prevtop(cp_top1);
  }
}

/*------------------------------------------------------------------

In the presence of conditional answers, its not enough to simply
delete (or invalidate) a table T for a subgoal S -- in most
circumstances we should delete all other tables that depend on T along
with T.  To do this we need to find all conditional answers for T, and
check their back pointers transitively, and eventually obtain the set
of subgoals that depend on T.

The way the algorithm works is that pointers to all conditional
answers to T are put the an answer_stack, and S is marked as visited.
Back-pointers of these answes are traversed -- and if the newly
visited answers belong to an unvisited subgoal S', S' is marked as
visited and a pointer to S's subgoal frame is put on the done stack.
This continues until all answers and subgoals have been traversed.  Of
course, back-pointers from the visitied subgoals themselves are also
traversed.

So, at the end of find_conditional_answers_for_subgoal(S) the answer_stack is
incremented to include all unconditional answers for S.  At the end of
find_subgoal_cross_dependencies(), the done_stack contains all
visited subgoals, while the answer_stack contains all conditional
answers for all visited subgoals.

-----------------------------------------------------------------*/
// Answer stack utilities -------------------------------------

#ifndef MULTI_THREAD
int trans_abol_answer_stack_top = 0;
BTNptr * trans_abol_answer_stack = NULL;
int trans_abol_answer_stack_size = 0;

int trans_abol_answer_array_top = 0;
BTNptr * trans_abol_answer_array = NULL;
int trans_abol_answer_array_size = 0;
#endif

#define trans_abol_answer_increment 1000

void reset_ta_answer_stack(CTXTdecl) {
  trans_abol_answer_stack_top = 0; 
}

void reset_ta_answer_array(CTXTdecl) {
  trans_abol_answer_array_top = 0; 
}

#define push_node_ta_answer_stack(as_leaf) {				                  \
    if (trans_abol_answer_stack_top >= trans_abol_answer_stack_size) {			          \
      size_t old_trans_abol_answer_stack_size = trans_abol_answer_stack_size;		          \
      trans_abol_answer_stack_size = trans_abol_answer_stack_size + trans_abol_answer_increment; \
      trans_abol_answer_stack = (BTNptr *) mem_realloc(trans_abol_answer_stack,			  \
					  old_trans_abol_answer_stack_size*sizeof(BTNptr *), \
					  trans_abol_answer_stack_size*sizeof(BTNptr *),     \
					  TABLE_SPACE);			          \
    }									          \
    trans_abol_answer_stack[trans_abol_answer_stack_top] =     as_leaf;		                  \
    trans_abol_answer_stack_top++;							          \
  }

#define push_node_ta_answer_array(as_leaf) {				\
    if (trans_abol_answer_array_top >= trans_abol_answer_array_size) {			          \
      size_t old_trans_abol_answer_array_size = trans_abol_answer_array_size;		          \
      trans_abol_answer_array_size = trans_abol_answer_array_size + trans_abol_answer_increment;	\
      trans_abol_answer_array = (BTNptr *) mem_realloc(trans_abol_answer_array,			  \
					  old_trans_abol_answer_array_size*sizeof(BTNptr *), \
					  trans_abol_answer_array_size*sizeof(BTNptr *),     \
					  TABLE_SPACE);			          \
    }									          \
    trans_abol_answer_array[trans_abol_answer_array_top] =     as_leaf;		                  \
    trans_abol_answer_array_top++;							          \
  }

#define pop_ta_answer_node(answer_ptr) {			\
  if (trans_abol_answer_stack_top > 0)				\
      answer_ptr = trans_abol_answer_stack[--trans_abol_answer_stack_top];	\
  else answer_ptr = NULL;   }

#define pop_ta_answer_array(answer_ptr) {			\
  if (trans_abol_answer_array_top >= 0)				\
      answer_ptr = trans_abol_answer_array[--trans_abol_answer_array_top];	\
  else answer_ptr = NULL;   }

void print_ta_answer_stack(CTXTdecl) {
  int frame = 0;
  while (frame < trans_abol_answer_stack_top) {
    printf("answer_stack_frame %d answer %p ",frame, trans_abol_answer_stack[frame]);
    print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(trans_abol_answer_stack[frame]))); printf("\n");
    frame++;
  }
  printf("Answer stack TOP = %d\n", trans_abol_answer_stack_top);
}

void print_ta_answer_array(CTXTdecl) {
  int frame = 0;
  while (frame < trans_abol_answer_array_top) {
    printf("answer_array_frame %d answer %p ",frame, trans_abol_answer_array[frame]);
    print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(trans_abol_answer_array[frame]))); printf("\n");
    frame++;
  }
  printf("Answer array TOP = %d\n", trans_abol_answer_array_top);
}

// End of answer stack utilities -------------------------------------

/* Trie traversal was copied from one of the delete routines. 
   There's probably a cleaner way to do this.
*/
int find_conditional_answers_for_subgoal(CTXTdeclc VariantSF subgoal) {

  BTNptr root, sib, chil;  
  int trie_op_top = 0;
  int trie_node_top = 0;
  int trie_hh_top = -1;
  int  num_leaves = 0;

  char *delete_trie_op = NULL;
  BTNptr *delete_trie_node = NULL;
  BTHTptr *delete_trie_hh = NULL;
  int trie_op_size, trie_node_size, trie_hh_size;
  //  printf("finding answers for ");print_subgoal(stddbg,subgoal);printf("\n");
  if (!delete_trie_op) {
    delete_trie_op = (char *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(char),TABLE_SPACE);
    delete_trie_node = (BTNptr *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(BTNptr),TABLE_SPACE);
    delete_trie_hh = (BTHTptr *)mem_alloc(DELETE_TRIE_STACK_INIT*sizeof(BTHTptr),TABLE_SPACE);
    trie_op_size = trie_node_size = trie_hh_size = DELETE_TRIE_STACK_INIT;
  }

  delete_trie_op[0] = 0;
  if (subg_ans_root_ptr(subgoal)) {
    delete_trie_node[0] = subg_ans_root_ptr(subgoal);
    while (trie_op_top >= 0) {
      switch (delete_trie_op[trie_op_top--]) {
      case DT_DS:
//	root = delete_trie_node[trie_node_top--];   // Don't use root in this case do we?
		trie_node_top--;
	break;
      case DT_HT:
	trie_hh_top--;
	break;
      case DT_NODE:
	root = delete_trie_node[trie_node_top--];
	if ( IsNonNULL(root) ) {
	  if ( IsHashHeader(root) ) {
	    BTHTptr hhdr;
	    BTNptr *base, *cur;
	    hhdr = (BTHTptr)root;
	    base = BTHT_BucketArray(hhdr);
	    push_delete_trie_hh(hhdr);
	    for ( cur = base; cur < base + BTHT_NumBuckets(hhdr); cur++ ) {
	      if (IsNonNULL(*cur)) {
		push_delete_trie_node(*cur,DT_NODE);
	      }
	    }
	  }
	  else {
	    sib  = BTN_Sibling(root);
	    chil = BTN_Child(root);      
	    if (IsLeafNode(root)) {
            // 12-05-07 
            // put one more condition for an answer leaf to be added: it must not be visited before
            // It might be visited before in find_single_backward_dependencies
            // minhdt - Do we need this change???
            // if ((is_conditional_answer(root)) && (!VISITED_ANSWER(root))){
	      if (is_conditional_answer(root)){
		push_node_ta_answer_stack(root);
		push_node_ta_answer_array(root);
		num_leaves++;
	      }
	      push_delete_trie_node(root,DT_DS);
	      if (IsNonNULL(sib)) {
		push_delete_trie_node(sib,DT_NODE);
	      }
	    }
	    else {
	      push_delete_trie_node(root,DT_DS);
	      if (IsNonNULL(sib)) {
		push_delete_trie_node(sib,DT_NODE);
	      }
	      if (IsNonNULL(chil)) {
		push_delete_trie_node(chil,DT_NODE);
	      }
	    }
	  }
	} else {
	  fprintf(stdwarn,"null node encountered in find_conditional_answers_for_subgoal ");
	  print_subgoal(CTXTc stdwarn,subgoal); fprintf(stdwarn,"\n");
	  break;
	}
      }
    }
  }
  mem_dealloc(delete_trie_op,trie_op_size*sizeof(char),TABLE_SPACE); delete_trie_op = NULL;
  mem_dealloc(delete_trie_node,trie_node_size*sizeof(BTNptr),TABLE_SPACE); delete_trie_node = NULL;
  mem_dealloc(delete_trie_hh,trie_hh_size*sizeof(BTHTptr),TABLE_SPACE); delete_trie_hh = NULL;
  //  print_ta_answer_stack(CTXT);
  return num_leaves;
}

// Done stack utilities -------------------------------------

#ifndef MULTI_THREAD
int ta_done_subgoal_stack_top = 0;
VariantSF *ta_done_subgoal_stack = NULL;
int ta_done_subgoal_stack_size = 0;
#endif

#define ta_done_subgoal_stack_increment 1000

static inline void push_ta_done_subgoal_node(CTXTdeclc VariantSF Subgoal) {				
    if (ta_done_subgoal_stack_top >= ta_done_subgoal_stack_size) {		
      size_t old_ta_done_subgoal_stack_size = ta_done_subgoal_stack_size; 
      ta_done_subgoal_stack_size = ta_done_subgoal_stack_size + ta_done_subgoal_stack_increment; 
      ta_done_subgoal_stack = (VariantSF *) mem_realloc(ta_done_subgoal_stack,	
						    old_ta_done_subgoal_stack_size*sizeof(VariantSF *), 
						    ta_done_subgoal_stack_size*sizeof(VariantSF *), 
						    TABLE_SPACE);	
    }									
    ta_done_subgoal_stack[ta_done_subgoal_stack_top] = Subgoal;	
    ta_done_subgoal_stack_top++;						
  }

void print_ta_done_subgoal_stack(CTXTdecl) {
  int frame = 0;
  while (frame < ta_done_subgoal_stack_top) {
    printf("ta_done_subgoal_frame %d ",frame);
    print_subgoal(CTXTc stddbg, ta_done_subgoal_stack[frame]);
    printf("\n");
    frame++;
  }
}

void reset_ta_done_subgoal_stack(CTXTdecl) {
  ta_done_subgoal_stack_top = 0;
}

void traverse_conditional_affects_edges(CTXTdeclc VariantSF);

static inline void traverse_subgoal_ndes(CTXTdeclc VariantSF subgoal)	{				
    DL nde_delayList;							
    BTNptr nde_as;							
    PNDE ndeElement;							
    VariantSF new_subgoal;						

    ndeElement = subg_nde_list(subgoal);				
    while (ndeElement) {						
      nde_delayList = pnde_dl(ndeElement);				
      while (nde_delayList) {
	nde_as = dl_asl(nde_delayList);				
	new_subgoal = asi_subgoal((ASI) Child(nde_as));		
	if (!subg_deltf_ptr(new_subgoal) && !VISITED_SUBGOAL(new_subgoal)) { 
	  traverse_conditional_affects_edges(CTXTc new_subgoal);
	}			
	nde_delayList = dl_next(nde_delayList);
      }						
      ndeElement = pnde_next(ndeElement);				
    }									
}
		      
static inline void traverse_answer_pdes(CTXTdeclc int current_answer_pos) {
  BTNptr as_from, as_to;
  PNDE pdeElement;
  DL delayList;   
  VariantSF subgoal;

  while (current_answer_pos < trans_abol_answer_stack_top) {
    pop_ta_answer_node(as_from);
    pdeElement = asi_pdes((ASI) Child(as_from));
    while (pdeElement) {
#ifdef ABOLISH_DBG2
      { VariantSF subgoal;
	printf(" in subgoal ");print_subgoal(stddbg,asi_subgoal((ASI) Child(as_from)));printf("  pde start %p",pdeElement);
	print_pdes(pdeElement);}
#endif
      delayList = pnde_dl(pdeElement);
      while (delayList) {
	as_to = dl_asl(delayList);
	subgoal = asi_subgoal((ASI) Child(as_to));
#ifdef ABOLISH_DBG2
	printf("considering ");print_subgoal(stddbg,subgoal);printf("\n");
	printf("deltf %d visited %d\n",subg_deltf_ptr(subgoal),VISITED_SUBGOAL(subgoal));
#endif
	if (!subg_deltf_ptr(subgoal) && !VISITED_SUBGOAL(subgoal)) {
	  traverse_conditional_affects_edges(CTXTc subgoal);	  }
	delayList = dl_next(delayList);
      }
      pdeElement = pnde_next(pdeElement);
    }
  }
}
      /* The commented code shows how to find forward dependencies */
      //      delayList = asi_dl_list((ASI) Child(as_leaf));
      //      while (delayList) {
      //	current = dl_de_list(delayList);
      //	printf("----- new dl -----\n");
      //	while (current) {
      //	  print_subgoal(stddbg,de_subgoal(current));printf(" | ");
      //  if (!VISITED_SUBGOAL(de_subgoal(current))) {
      //    MARK_VISITED_SUBGOAL(de_subgoal(current));
      //    push_ta_done_subgoal_node(CTXTc de_subgoal(current));
      //    find_conditional_answers_for_subgoal(CTXTc de_subgoal(current));
      //  }
      //  current = de_next(current);
      //}
      //	printf("\n");
      //	delayList = dl_next(delayList);
      //    }

void traverse_conditional_affects_edges(CTXTdeclc VariantSF subgoal) {
  int starting_answer_pos;
    MARK_VISITED_SUBGOAL(subgoal);
    traverse_subgoal_ndes(CTXTc subgoal);
    starting_answer_pos = trans_abol_answer_stack_top;
    find_conditional_answers_for_subgoal(CTXTc subgoal);
    traverse_answer_pdes(CTXTc starting_answer_pos);
    push_ta_done_subgoal_node(CTXTc subgoal); 
}

void clean_up_backpointers_to_abolished_subgoals(CTXTdecl) {
  BTNptr as_leaf;
  DL delayList;     DE current_de; PNDE current_pnde;

  while (trans_abol_answer_array_top > 0) {
    pop_ta_answer_array(as_leaf);
#ifdef ABOLISH_DBG2
    printf("checking answer in ");print_subgoal(stddbg,asi_subgoal((ASI) Child(as_leaf)));printf("\n");
#endif
    delayList = asi_dl_list((ASI) Child(as_leaf));
    while (delayList) {
      current_de = dl_de_list(delayList);
#ifdef ABOLISH_DBG2
      printf("  ----- new dl ");
#endif
      while (current_de) {
#ifdef ABOLISH_DBG2
	print_subgoal(stddbg,de_subgoal(current_de));printf(" | ");
#endif
	if (!VISITED_SUBGOAL(de_subgoal(current_de))) {
	  if (de_ans_subst(current_de)) {
	      current_pnde = asi_pdes((ASI) Child(de_ans_subst(current_de)));
	    } else {
	      current_pnde = subg_nde_list(de_subgoal(current_de));
	    }
	  while (current_pnde && pnde_de(current_pnde) != current_de) {
#ifdef ABOLISH_DBG2
	    printf("current pnde from %p to %p\n",current_pnde,pnde_next(current_pnde));
#endif
	    current_pnde = pnde_next(current_pnde);
	      }
	  if (current_pnde) {
#ifdef ABOLISH_DBG2
	    printf("removing %p (next %p) from chain`\n",current_pnde,pnde_next(current_pnde));
#endif
	    if (de_ans_subst(current_de)) {
	      remove_pnde(asi_pdes((ASI) Child(de_ans_subst(current_de))), current_pnde,released_pndes_gl);
	    } else {
	      remove_pnde(subg_nde_list(de_subgoal(current_de)), current_pnde,released_pndes_gl);
	    }
	  } else {
	    xsb_warn(CTXTc "(*&(*& couldn't find pnde\n");
	  }
	}
#ifdef ABOLISH_DBG2
	printf("\n");
#endif
	current_de = de_next(current_de);
      }
      delayList = dl_next(delayList);
    }
  }
}

/* Start by marking input subgoal as visited (and put it on the done
   stack).  Then find conditional answers for subgoal, and put these
   answers on the answer stack.  Finally, traverse its ndes and set
   last_subgoal to subgoal.

   Thereafter, go through each answer on the answer stack. If the
   subgoal is different from the last subgoal, traverse the subgoal's
   ndes (this works because all answers for any subgoal will form a
   contiguous segment of the answer stack).  Also traverse the answers
   backpointers (pdes) as well.

   Traversing either nde's or pde's marks new subgoals as they are
   visited, and puts all of their conditional answes onto the answer
   stack.

   Thus, we examine pde's for each answer during the search exactly
   once, and examine the nde's for each subgoal exactly once.  At the
   end, the answer stack contains all answers that we've traversed,
   and the done subgoal stack contains all subgoals that we've
   traversed.

   As of 5/07, fbsd() is the only system code that uses answer_stack.
   For reasons of concurrency, it doesn't deallocate the answer stack
   when its done -- I'm assuming that if an appliction gets abolishes
   tables with conditional answers once, its likely to do so again --
   if this is a bad assumption, a deallocate can be put in the end.
*/


int find_subgoal_cross_dependencies(CTXTdeclc VariantSF subgoal) {

    reset_ta_done_subgoal_stack(CTXT);
    reset_ta_answer_stack(CTXT);
    reset_ta_answer_array(CTXT);
    traverse_conditional_affects_edges(CTXTc subgoal);
#ifdef ABOLISH_DBG2
    printf("answer stack before clean_up_backpointers\n");
    print_ta_answer_stack(CTXT);
    print_ta_answer_array(CTXT);
    print_ta_done_subgoal_stack(CTXT);
#endif
    clean_up_backpointers_to_abolished_subgoals(CTXT);
    reset_ta_answer_stack(CTXT);
    reset_ta_answer_array(CTXT);
    return ta_done_subgoal_stack_top;
}

#ifdef UNDEFINED
//TLS unused
static int find_subgoal_forward_dependencies(CTXTdeclc VariantSF subgoal) {
    BTNptr as_leaf;
    DL delayList;
    DE current;
    VariantSF last_subgoal;
    int answer_stack_current_pos = 0;

    reset_ta_answer_stack(CTXT);
    reset_ta_answer_array(CTXT);
    find_conditional_answers_for_subgoal(CTXTc subgoal);
    //    print_ta_answer_stack(CTXT);
    last_subgoal = subgoal;
    //    printf("--------\n");

    while (answer_stack_current_pos < trans_abol_answer_stack_top) {
      as_leaf = answer_stack[answer_stack_current_pos];
      //      if (asi_subgoal((ASI) Child(as_leaf)) != last_subgoal) {
	last_subgoal = asi_subgoal((ASI) Child(as_leaf));
      }
      delayList = asi_dl_list((ASI) Child(as_leaf));
      while (delayList) {
	current = dl_de_list(delayList);
	//	printf("----- new dl -----\n");
	while (current) {
	  //	  print_subgoal(stddbg,de_subgoal(current));printf(" | ");
	  if (!VISITED_SUBGOAL(de_subgoal(current))) {
	    MARK_VISITED_SUBGOAL(de_subgoal(current));
	    push_ta_done_subgoal_node(CTXTc de_subgoal(current));
	    find_conditional_answers_for_subgoal(CTXTc de_subgoal(current));
	  }
	  current = de_next(current);
	}
	//	printf("\n");
	delayList = dl_next(delayList);
      }
      answer_stack_current_pos++;
    }
    reset_ta_answer_stack(CTXT);
    reset_ta_answer_array(CTXT);
    return ta_done_subgoal_stack_top;
}
#endif
// End of done stack utilities -------------------------------------

/* To be used in nocheck_deltf functions as well as gc sweeps and
   reclaim_incomplete_tables.  For this reason it rechecks for
   incremental or subsumptive.  Assumes that branches have already
   been deleted. */

void abolish_table_call_single_nocheck_no_nothin(CTXTdeclc VariantSF subgoal,int cond_warn_flag) {
  //  TIFptr tif = subg_tif_ptr(subgoal);

  //  delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
  if (!IsIncrSF(subgoal)){
    if (IsVariantSF(subgoal)) {
      delete_variant_call(CTXTc subgoal, cond_warn_flag);
    }
    else {
      delete_subsumptive_call(CTXTc (SubProdSF) subgoal);
    }
  }
  else abolish_incremental_call_single_nocheck_no_nothin(CTXTc subgoal, cond_warn_flag);
}


void abolish_nonincremental_call_single_nocheck_deltf(CTXTdeclc VariantSF subgoal, int action) {
    TIFptr tif;
    Psc psc;

    tif = subg_tif_ptr(subgoal);
    psc = TIF_PSC(tif);

    if (flags[CTRACE_CALLS])  { 
      sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)subgoal);     
      fprintf(fview_ptr,"abol_tab_call_sing(subg(%s),%d).\n",forest_log_buffer_1->fl_buffer,ctrace_ctr++);
    }

    SET_TRIE_ALLOCATION_TYPE_SF(subgoal); // set smBTN to private/shared
    if (action == CAN_RECLAIM && !GC_MARKED_SUBGOAL(subgoal)) {
      delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
      abolish_table_call_single_nocheck_no_nothin(CTXTc subgoal,SHOULD_COND_WARN); /* DSW, until par added */
    }
    else {  /* CANT_RECLAIM */
      //	printf("Mark %x GC %x\n",subgoal->visited,GC_MARKED_SUBGOAL(subgoal));
      if (!get_shared(psc)) {
	delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
	check_insert_private_deltf_subgoal(CTXTc subgoal,FALSE);
      }
#ifdef MULTI_THREAD
      else {
	safe_delete_branch(subgoal->leaf_ptr); 
	check_insert_shared_deltf_subgoal(CTXT, subgoal,FALSE);
      }
#endif
    }
}

/* For use when abolishing nonincremental tabled subgoals that do not have conditional answers */
void abolish_table_call_single(CTXTdeclc VariantSF subgoal) {
    TIFptr tif;
    Psc psc;
    int action;

    tif = subg_tif_ptr(subgoal);
    psc = TIF_PSC(tif);

    if (flags[NUM_THREADS] == 1 || !get_shared(psc)) {
      action = abolish_table_call_cps_check(CTXTc subgoal);
    } else action = CANT_RECLAIM;


  //  delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
  if (!IsIncrSF(subgoal)){
    abolish_dbg(("abolishing nonincremental table %p\n",subgoal));abolish_print_subgoal(subgoal);abolish_dbg(("\n"));
    if (IsVariantSF(subgoal)) {
      abolish_nonincremental_call_single_nocheck_deltf(CTXTc subgoal,action);
    }
    else {
      abolish_subsumptive_call_single_nocheck_deltf(CTXTc (SubProdSF) subgoal,action);
    }
  }
  else abolish_incremental_call_single_nocheck_deltf(CTXTc subgoal, action, DO_INVALIDATE);
  // No unmarking needed
}

void abolish_subsumptive_call_single_nocheck_deltf(CTXTdeclc SubProdSF subgoal,int action) {
  delete_subsumptive_call(CTXTc  subgoal);
}

//------------------- end abolish_table_call_single

/* Assuming no intermixing of shared and private tables */
void abolish_table_call_transitive_nocheck_deltf(CTXTdeclc VariantSF subgoal, int action) {

    TIFptr tif;
    Psc psc;

    //       printf("in abolish_table_call_transitive\n");
    tif = subg_tif_ptr(subgoal);
    psc = TIF_PSC(tif);

    find_subgoal_cross_dependencies(CTXTc subgoal);  // also forward deps

    SET_TRIE_ALLOCATION_TYPE_SF(subgoal); // set smBTN to private/shared
    while (ta_done_subgoal_stack_top) {
      ta_done_subgoal_stack_top--;
      subgoal = ta_done_subgoal_stack[ta_done_subgoal_stack_top];
      tif = subg_tif_ptr(subgoal);
      if (flags[CTRACE_CALLS])  { 
	sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)subgoal);     
	fprintf(fview_ptr,"ta(subg(%s),%d).\n",forest_log_buffer_1->fl_buffer,
		ctrace_ctr++);
      }

      maybe_detach_incremental_call_single(CTXTc subgoal,DO_INVALIDATE);  // detatch from IDG
      if (action == CAN_RECLAIM && !GC_MARKED_SUBGOAL(subgoal) ) {
	delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
	abolish_table_call_single_nocheck_no_nothin(CTXTc subgoal,SHOULDNT_COND_WARN);
       }
      else {
	//	printf(" onto gc list "); print_subgoal(stddbg,subgoal);printf("\n");
	//	printf("Mark %x GC %x\n",subgoal->visited,GC_MARKED_SUBGOAL(subgoal));
	if (!get_shared(psc)) {
	  if (IsIncrSF(subgoal)) invalidate_call(CTXTc subg_callnode_ptr(subgoal),ABOLISHING);
	  delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
	  check_insert_private_deltf_subgoal(CTXTc subgoal,FALSE);
	}
#ifdef MULTI_THREAD
	else {
	  safe_delete_branch(subgoal->leaf_ptr); 
	  check_insert_shared_deltf_subgoal(CTXT, subgoal,FALSE);
	}
#endif
      }
    }
    reset_ta_done_subgoal_stack(CTXT);
}

/* Assuming no intermixing of shared and private tables */
void abolish_table_call_transitive(CTXTdeclc VariantSF subgoal) {
  TIFptr tif;
  Psc psc;
  int action;

  tif = subg_tif_ptr(subgoal);
  psc = TIF_PSC(tif);

  if (flags[NUM_THREADS] == 1 || !get_shared(psc)) {
    mark_cp_tabled_subgoals(CTXT);     // also marks tables in delay list
    action = CAN_RECLAIM;
  } else action = CANT_RECLAIM;

  abolish_table_call_transitive_nocheck_deltf(CTXTc subgoal,action);

  unmark_cp_tabled_subgoals(CTXT);
}

/* Took check for incomplete out -- its been done in tables.P.
However, we now need to check the default setting (settable in
xsb_flag) as well as an option set by the options list, if any. 
*/

void abolish_table_call(CTXTdeclc VariantSF subgoal, int invocation_flag) {
  declare_timer

  start_table_gc_time(timer);

#ifdef DEBUG_ABOLISH
  printf("abolish_table_call called (%p): ",subgoal); print_subgoal(CTXTc stddbg, subgoal); printf("\n");
#endif

  if (IsVariantSF(subgoal) 
      &&  (varsf_has_conditional_answer(subgoal) 
             && (invocation_flag == ABOLISH_TABLES_TRANSITIVELY 
   	          || !(invocation_flag == ABOLISH_TABLES_DEFAULT 
		       && flags[TABLE_GC_ACTION] == ABOLISH_TABLES_SINGLY)))) {
    abolish_dbg(("calling atc_t\n"));
    abolish_table_call_transitive(CTXTc subgoal);
    }
  else {
    abolish_dbg(("calling atc_s\n"));
    abolish_table_call_single(CTXTc subgoal);
  }
  abolish_dbg(("atc done %p \n",subgoal));
  end_table_gc_time(timer);

}

/*------------------------------------------------------------------*/
/* abolish_table_pred() and supporting code */
/*------------------------------------------------------------------*/

// done_tif_stack stack utilities -------------------------------------

#ifndef MULTI_THREAD
int done_tif_stack_top = 0;
TIFptr * done_tif_stack = NULL;
int done_tif_stack_size = 0;
#endif

#define done_tif_stack_increment 1000

void reset_done_tif_stack(CTXTdecl) {
  done_tif_stack_top = 0;
}

void unvisit_done_tifs(CTXTdecl) {
  int i;
  for (i = 0; i < done_tif_stack_top; i++) {
    TIF_Visited(done_tif_stack[i]) = 0;
  }
}

static inline void push_done_tif_node(CTXTdeclc TIFptr node) {					\
    if (done_tif_stack_top >= done_tif_stack_size) {			\
      size_t old_done_tif_stack_size = done_tif_stack_size;	\
      done_tif_stack_size = done_tif_stack_size + done_tif_stack_increment;	\
      done_tif_stack = (TIFptr *) mem_realloc(done_tif_stack,			  \
					      old_done_tif_stack_size*sizeof(TIFptr *), \
					      done_tif_stack_size*sizeof(TIFptr *), \
					      TABLE_SPACE);		\
    }									\
    done_tif_stack[done_tif_stack_top] =     node;			\
    done_tif_stack_top++;						\
  }

void print_done_tif_stack(CTXTdecl) {
  int frame = 0;
  while (frame < done_tif_stack_top) {
    printf("done_tif_frame %d tif %p (%s/%d)\n",frame, done_tif_stack[frame],
	   get_name(TIF_PSC(done_tif_stack[frame])),get_arity(TIF_PSC(done_tif_stack[frame])));
    frame++;
  }
}

//----------------------------------------------------------------------

/* TLS: this doesn't yet work correctly for WF call subsumption */
static void find_subgoals_and_answers_for_pred(CTXTdeclc TIFptr tif) {

  VariantSF pSF;
  TRIE_W_LOCK();
  pSF = TIF_Subgoals(tif);
  if ( IsNULL(pSF) )   return;

  while (pSF) {
    if (varsf_has_conditional_answer(pSF)) {
      push_ta_done_subgoal_node(CTXTc pSF);
      find_conditional_answers_for_subgoal(CTXTc pSF);
      traverse_subgoal_ndes(CTXTc pSF);
	} 
    pSF = subg_next_subgoal(pSF);
  } /* there is a child of "node" */
  TRIE_W_UNLOCK();
}

int find_pred_backward_dependencies(CTXTdeclc TIFptr tif) {
    BTNptr as_leaf;
    PNDE pdeElement, ndeElement;
    DL delayList, nde_delayList; DE current;
    BTNptr as_prev, nde_as_prev;
    VariantSF subgoal;
    int answer_stack_current_pos = 0;
    int ta_done_subgoal_stack_current_pos = 0;

    reset_ta_answer_stack(CTXT);
    reset_ta_done_subgoal_stack(CTXT);
    reset_done_tif_stack(CTXT);

    if (!TIF_Visited(tif)) {
      TIF_Visited(tif) = 1;
      push_done_tif_node(CTXTc tif); 
      find_subgoals_and_answers_for_pred(CTXTc tif);

      //      print_ta_answer_stack(CTXT);
    while (answer_stack_current_pos < trans_abol_answer_stack_top 
	   || ta_done_subgoal_stack_current_pos < ta_done_subgoal_stack_top) {
      while (answer_stack_current_pos < trans_abol_answer_stack_top) {
	as_leaf = trans_abol_answer_stack[answer_stack_current_pos];
	pdeElement = asi_pdes((ASI) Child(as_leaf));
	while (pdeElement) {
	  delayList = pnde_dl(pdeElement);
	  as_prev = dl_asl(delayList);
	  if (as_prev) tif = subg_tif_ptr(asi_subgoal((ASI) Child(as_prev)));
	  if (/*!tif_deltf_ptr(tif) &&*/ !TIF_Visited(tif)) {
	    TIF_Visited(tif) = 1;
	    push_done_tif_node(CTXTc tif);
	    find_subgoals_and_answers_for_pred(CTXTc tif);
	  }
	  pdeElement = pnde_next(pdeElement);
	}
      delayList = asi_dl_list((ASI) Child(as_leaf));
      while (delayList) {
	current = dl_de_list(delayList);
	//	printf("----- new dl -----\n");
	while (current) {
	  tif = subg_tif_ptr(de_subgoal(current));
	  if (!TIF_Visited(tif)) {
	    TIF_Visited(tif) = 1;
	    push_done_tif_node(CTXTc tif);
	    find_subgoals_and_answers_for_pred(CTXTc tif);
	  }
	  current = de_next(current);
	}
	//	printf("\n");
	delayList = dl_next(delayList);
      }
	answer_stack_current_pos++;
      }
      while (ta_done_subgoal_stack_current_pos < ta_done_subgoal_stack_top) {
	subgoal = ta_done_subgoal_stack[ta_done_subgoal_stack_current_pos];
	ndeElement = subg_nde_list(subgoal);				
	while (ndeElement) {						
	  nde_delayList = pnde_dl(ndeElement);				
	  nde_as_prev = dl_asl(nde_delayList);				
	  if (nde_as_prev) tif = subg_tif_ptr(asi_subgoal((ASI) Child(nde_as_prev)));
	  if (/*!tif_deltf_ptr(tif) &&*/ !TIF_Visited(tif)) {
	    TIF_Visited(tif) = 1;
	    push_done_tif_node(CTXTc tif);
	    find_subgoals_and_answers_for_pred(CTXTc tif);
	  }								
	  ndeElement = pnde_next(ndeElement);				
	}								
	ta_done_subgoal_stack_current_pos++;
      }
    }
    }

    unvisit_done_tifs(CTXT);
    reset_ta_answer_stack(CTXT);
    reset_ta_done_subgoal_stack(CTXT);
    return done_tif_stack_top;
  }

/* 
   Recurse through CP stack looking for trie nodes that match PSC.
   Also look through delay list.
   Returns 1 if found a psc match, 0 if safe to delete now
*/
int abolish_table_pred_cps_check(CTXTdeclc Psc psc) 
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  BTNptr trieNode;
  CPtr dlist;
  Cell tmp_cell;
  VariantSF subgoal;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  /* First check the CP stack */
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      // Below we want basic_answer_trie_tt, ts_answer_trie_tt
      trieNode = TrieNodeFromCP(cp_top1);
      if (psc == get_psc_for_trie_cp(CTXTc cp_top1, trieNode)) {
	  return CANT_RECLAIM;
      }
    }
    /* Now check delaylist */
    dlist = cp_pdreg(cp_top1);
    while (islist(dlist) ) {
      dlist = clref_val(dlist);
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      if (psc == TIF_PSC(subg_tif_ptr(subgoal))) 
	return CANT_RECLAIM;
      dlist = (CPtr)cell(dlist+1);
    }
    cp_top1 = cp_prevtop(cp_top1);
  }
  return CAN_RECLAIM;
}

/* Delays spece reclamation if the cps check does not pass OR if
   shared and more than 1 thread is active.

  abolish_table_predicate does not reclaim space for previously
 "abolished" tables in deltf frames.  Need to do gc tables for
  that. 

  Don't need a warning flag for this predicate -- it must always warn
*/
static inline void abolish_table_pred_single(CTXTdeclc TIFptr tif, int cps_check_flag,int incomplete_action_flag) {
  int action;

  if (IsVariantPredicate(tif) && IsNULL(TIF_CallTrie(tif))) {
    return ;
  }

  if ( !is_completed_table(tif) ) {
    if (incomplete_action_flag == ERROR_ON_INCOMPLETE) {
      char message[ERRMSGLEN/2];
      snprintf(message,ERRMSGLEN/2,"incomplete tabled predicate %s/%d",get_name(TIF_PSC(tif)),
	       get_arity(TIF_PSC(tif)));
      xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
    }
    //      xsb_abort("[abolish_table_pred] Cannot abolish incomplete table"
    //	" of predicate %s/%d\n", get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)));
    else 
      return;
  }

 if (flags[CTRACE_CALLS])  { 
    fprintf(fview_ptr,"ta(pred('/'(%s,%d)),%d).\n",get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)),ctrace_ctr++);
  }

  if (flags[NUM_THREADS] == 1 || !get_shared(TIF_PSC(tif))) {
    if (cps_check_flag) action = abolish_table_pred_cps_check(CTXTc TIF_PSC(tif));
    else action = TIF_Mark(tif);  /* 1 = CANT_RECLAIM; 0 = CAN_RECLAIM */
  }
  else action = CANT_RECLAIM;
  if (action == CAN_RECLAIM) {
    delete_predicate_table(CTXTc tif,TRUE);
  }
  else if (TIF_Subgoals(tif)) {
    //        fprintf(stderr,"Delaying abolish of table in use: %s/%d\n",
    //        get_name(psc),get_arity(psc));
#ifndef MULTI_THREAD
    check_insert_private_deltf_pred(CTXTc tif,TRUE);
#else
    if (!get_shared(TIF_PSC(tif)))
      check_insert_private_deltf_pred(CTXTc tif,TRUE);
    else
      check_insert_shared_deltf_pred(CTXT, tif,TRUE);
#endif
  }
}  

static inline void abolish_table_pred_transitive(CTXTdeclc TIFptr tif, int cps_check_flag) {
  int action;

  find_pred_backward_dependencies(CTXTc tif);

  if (flags[NUM_THREADS] == 1 || !get_shared(TIF_PSC(tif))) {
    if (cps_check_flag) mark_cp_tabled_preds(CTXT);
    action = CAN_RECLAIM;
  } else action = CANT_RECLAIM;

  while (done_tif_stack_top) {
    done_tif_stack_top--;
    tif = done_tif_stack[done_tif_stack_top];

    if(get_incr(TIF_PSC(tif))) {     /* incremental */
      char message[ERRMSGLEN/2];
      snprintf(message,ERRMSGLEN/2,"incremental tabled predicate %s/%d",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
      xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
      //      xsb_abort("[abolish_table_predicate] Cannot abolish incremental tables for "
      //		  "predicate  %s/%d.  Either abolish table calls or abolish_all_tables.",
      //       get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)));
      //      free_incr_hashtables(tif);
    }

    if ( ! is_completed_table(tif) ) {
      char message[ERRMSGLEN/2];
      snprintf(message,ERRMSGLEN/2,"incomplete tabled predicate %s/%d",get_name(TIF_PSC(tif)),
	       get_arity(TIF_PSC(tif)));
      xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
    }
 if (flags[CTRACE_CALLS])  { 
    fprintf(fview_ptr,"ta(pred('/'(%s,%d)),%d).\n",get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)),ctrace_ctr++);
  }

    //        printf(" abolishing %s/%d\n",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
    if (action == CAN_RECLAIM && !TIF_Mark(tif) ) {
      abolish_dbg(("   abolish predicate transitive (reclaim) %s/%d\n",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif))));
      transitive_delete_predicate_table(CTXTc tif,FALSE);
    }
    /* This check is needed to avoid makding a deltf frame multiple times for the same predicate, 
       when it is encountered more tha once by find_pred_backwared_dependencies() */
    else if (TIF_Subgoals(tif)) {
      abolish_dbg(("   abolish predicate transitive (put on gc list) %s/%d\n",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif))));
      //        fprintf(stderr,"Delaying abolish of table in use: %s/%d\n",
      //        get_name(psc),get_arity(psc));
#ifndef MULTI_THREAD
      //      print_deltf_chain(CTXT);
      //      printTIF(tif);
      check_insert_private_deltf_pred(CTXTc tif,FALSE);
#else
      if (!get_shared(TIF_PSC(tif)))
	check_insert_private_deltf_pred(CTXTc tif,FALSE);
      else
	check_insert_shared_deltf_pred(CTXT, tif,FALSE);
#endif
      }
  }
  if (cps_check_flag) unmark_cp_tabled_preds(CTXT);
}  

inline void abolish_table_predicate_switch(CTXTdeclc TIFptr tif, Psc psc, int invocation_flag, 
					   int cps_check_flag, int incomplete_action_flag) {

  abolish_dbg(("abolish_table_pred called: %s/%d\n",get_name(psc),get_arity(psc)));

  if (get_variant_tabled(psc)
      && (invocation_flag == ABOLISH_TABLES_TRANSITIVELY 
	  || (invocation_flag == ABOLISH_TABLES_DEFAULT 
	      && flags[TABLE_GC_ACTION] == ABOLISH_TABLES_TRANSITIVELY))) {
    abolish_dbg(("atp_trans called\n"));
    abolish_table_pred_transitive(CTXTc tif, cps_check_flag);
  }
    else {
      abolish_dbg(("...atp_single called\n"));
      abolish_table_pred_single(CTXTc tif, cps_check_flag,incomplete_action_flag);
    }
}

/* When calling a_t_p_switch, cps_check_flag is set to true, to ensure a check. 
 invocation_flag (default, transitive) has been set on Prolog side. */

inline void abolish_table_predicate(CTXTdeclc Psc psc, int invocation_flag) {
  TIFptr tif;
  declare_timer

  tif = get_tip(CTXTc psc);

  start_table_gc_time(timer);

  gc_tabled_preds(CTXT);
  if ( IsNULL(tif) ) {
    xsb_abort("[abolish_table_pred] Attempt to delete non-tabled predicate (%s/%d)\n",
	      get_name(psc), get_arity(psc));
  }

  if(get_incr(TIF_PSC(tif))) {  /* incremental */
      char message[ERRMSGLEN/2];
      snprintf(message,ERRMSGLEN/2,"incremental tabled predicate %s/%d",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
      xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
      //      xsb_abort("[abolish_table_predicate] Cannot abolish incremental tables for "
      //		  "predicate  %s/%d.  Either abolish table calls or abolish_all_tables.",
      //  	          get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)));
      //    free_incr_hashtables(tif);  TLS: took out 201408 - this makes no sense to me.
  }

  /* Check CPS stack = TRUE, ERROR if table is incomplete */
  abolish_table_predicate_switch(CTXTc tif, psc, invocation_flag, TRUE,ERROR_ON_INCOMPLETE);

  end_table_gc_time(timer);

}

/*------------------------------------------------------------------*/
/* Table gc and supporting code */
/*------------------------------------------------------------------*/

/* Go through and mark DelTfs to ensure that on sweep we dont abolish
  "active" predicates we're backtracking through.  Note that only the
  first DelTF in the pred-specific chain may be active in this sense.
  And its active only if calltrie and subgoals for tif are 0 -- if
  they are 0, the table has been abolished (even though we're
  backtracking through it).  If they aren't 0, we're backtracking
  through a different table altogether, and we needn't mark.
*/

static inline void mark_deltfs(CTXTdeclc TIFptr tif, VariantSF subgoal) {
  BTNptr call_trie;
  DelTFptr dtf;

  call_trie = get_call_trie_from_subgoal_frame(CTXTc subgoal);
  //	printf("subgoal %p call_trie %p\n",subgoal,call_trie);
	
  dtf = TIF_DelTF(tif);
  // Cycle through all deltfs for this pred
  while (dtf) {
    if (DTF_CallTrie(dtf) == call_trie) {
      DTF_Mark(dtf) = 1;
      dtf = NULL;
    } else dtf = DTF_NextPredDTF(dtf);
  }

  //	if (TIF_CallTrie(tif) == NULL && TIF_Subgoals(tif) == NULL) {
  //	 //   && !get_shared(TIF_PSC(tif))) { 
  //	  dtf = TIF_DelTF(tif);
  //	  DTF_Mark(dtf) = 1;
  //	}
	
  /* Now check for subgoal DelTFs */
  if (is_completed(subgoal)) {
    if (subg_deltf_ptr(subgoal) != NULL) {
      DTF_Mark((DelTFptr) subg_deltf_ptr(subgoal)) = 1;
    }
  }
}

static inline void gc_mark_delaylist_tabled_preds(CTXTdeclc CPtr dlist) {
  Cell tmp_cell;
  VariantSF subgoal;

  //  if (dlist != NULL) {
  //    printf("checking list ");print_delay_list(CTXTc stddbg, dlist);
    while (islist(dlist)) {
      dlist = clref_val(dlist);
      // printf("\n checking element "); print_delay_element(CTXTc stddbg, cell(dlist));
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      mark_deltfs(CTXTc subg_tif_ptr(subgoal), subgoal);
      dlist = (CPtr)cell(dlist+1);
    }
  }

void mark_tabled_preds(CTXTdecl) { 
  CPtr cp_top1,cp_bot1 ; byte cp_inst;
  TIFptr tif;
  VariantSF subgoal;
  BTNptr trieNode;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but need to distinguish asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	/* Check for predicate DelTFs */
	tif = get_tif_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	subgoal = get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	//	printf("marking tif? %p \n",tif);
	if (tif) mark_deltfs(CTXTc tif, subgoal);
      }
    }

    /* Now check delaylist */
    gc_mark_delaylist_tabled_preds(CTXTc cp_pdreg(cp_top1));

    cp_top1 = cp_prevtop(cp_top1);
  }
}

/* Mark only private tables -- ignore shared tables. Used by mt system
   when gc-ing with more than 1 active thread -- and used in lieu
   of mark_tabled_preds() */
#ifdef MULTI_THREAD
void mark_private_tabled_preds(CTXTdecl) { 
  CPtr cp_top1,cp_bot1 ; byte cp_inst;
  TIFptr tif;
  VariantSF subgoal;
  BTNptr trieNode;
  
  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;

  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but need to distinguish asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	//      if (IsInAnswerTrie(trieNode)) {
	/* Check for predicate DelTFs */
	tif = get_tif_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	if (tif && !get_shared(TIF_PSC(tif))) {
	  subgoal = get_subgoal_frame_for_answer_trie_cp(CTXTc trieNode,cp_top1);
	  mark_deltfs(CTXTc tif, subgoal);
	}
      }
    }
    /* Now check delaylist */
    gc_mark_delaylist_tabled_preds(CTXTc cp_pdreg(cp_top1));

    cp_top1 = cp_prevtop(cp_top1);
  }
}

int sweep_private_tabled_preds(CTXTdecl) {
  DelTFptr deltf_ptr, next_deltf_ptr;
  int dtf_cnt = 0;
  TIFptr tif_ptr;

  deltf_ptr = private_deltf_chain_begin;
  SET_TRIE_ALLOCATION_TYPE_PRIVATE();
  while (deltf_ptr) {
    next_deltf_ptr = DTF_NextDTF(deltf_ptr);
    if (DTF_Mark(deltf_ptr)) {
      //      tif_ptr = subg_tif_ptr(DTF_Subgoals(deltf_ptr));
      //           fprintf(stderr,"Skipping: %s/%d\n",
      //   get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
      DTF_Mark(deltf_ptr) = 0;
      dtf_cnt++;
    }
    else {
      if (DTF_Type(deltf_ptr) == DELETED_PREDICATE) {
	tif_ptr = subg_tif_ptr(DTF_Subgoals(deltf_ptr));
	//	fprintf(stderr,"Garbage Collecting Predicate: %s/%d\n",
	//		get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
	reclaim_deleted_predicate_table(CTXTc deltf_ptr);
	Free_Private_DelTF_Pred(deltf_ptr,tif_ptr);
      } else 
	if (DTF_Type(deltf_ptr) == DELETED_SUBGOAL) {
	  tif_ptr = subg_tif_ptr(DTF_Subgoal(deltf_ptr));
	  //	 	  fprintf(stderr,"Garbage Collecting Subgoal: %s/%d\n",
	  //			  get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
	  //	  delete_variant_call(CTXTc DTF_Subgoal(deltf_ptr),DTF_Warn(deltf_ptr)); 
	  abolish_table_call_single_nocheck_no_nothin(CTXTc DTF_Subgoal(deltf_ptr),SHOULDNT_COND_WARN);
	  Free_Private_DelTF_Subgoal(deltf_ptr,tif_ptr);
	}
    }
    deltf_ptr = next_deltf_ptr;
  }
  return dtf_cnt;
}
#endif

/* No mutex on this predicate, as global portions can only be called
   with one active thread.  This actually reclaims both abolished
   predicates and subgoals */

int sweep_tabled_preds(CTXTdecl) {
  DelTFptr deltf_ptr, next_deltf_ptr;
  int dtf_cnt = 0;
  TIFptr tif_ptr;

  /* Free global deltfs */
  deltf_ptr = deltf_chain_begin;
  SET_TRIE_ALLOCATION_TYPE_SHARED();
  while (deltf_ptr) {
    next_deltf_ptr = DTF_NextDTF(deltf_ptr);
    if (DTF_Mark(deltf_ptr)) {
      //      tif_ptr = subg_tif_ptr(DTF_Subgoals(deltf_ptr));
      //      fprintf(stderr,"Skipping: %s/%d\n",
      //      get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
      DTF_Mark(deltf_ptr) = 0;
      dtf_cnt++;
    }
    else {
      if (DTF_Type(deltf_ptr) == DELETED_PREDICATE) {
	tif_ptr = subg_tif_ptr(DTF_Subgoals(deltf_ptr));
	//	  printf("fia\n");printTIF((TIFptr) 0x54c620);
	//	fprintf(stderr,"Garbage Collecting Predicate: %s/%d\n",
	//	get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
	reclaim_deleted_predicate_table(CTXTc deltf_ptr);
	Free_Global_DelTF_Pred(deltf_ptr,tif_ptr);
      } else 
	if (DTF_Type(deltf_ptr) == DELETED_SUBGOAL) {
	  tif_ptr = subg_tif_ptr(DTF_Subgoal(deltf_ptr));
	  //		    fprintf(stderr,"Garbage Collecting Subgoal: %s/%d\n",
	  //	   get_name(TIF_PSC(tif_ptr)),get_arity(TIF_PSC(tif_ptr)));
	  abolish_table_call_single_nocheck_no_nothin(CTXTc DTF_Subgoal(deltf_ptr),SHOULDNT_COND_WARN);
	  Free_Global_DelTF_Subgoal(deltf_ptr,tif_ptr);
	}
    }
    deltf_ptr = next_deltf_ptr;
  }

#ifdef MULTI_THREAD
  dtf_cnt = dtf_cnt + sweep_private_tabled_preds(CTXT);
#endif

  return dtf_cnt;
}

/* * * * * * * * * * * * * * * 
 * In MT engine gcs does not gc shared tables if there is more than
 * one thread. 
 */

#ifndef MULTI_THREAD
int gc_tabled_preds(CTXTdecl) {
  //    printf("gc-ing tabled preds\n");
  //    print_deltf_chain(CTXT);
  mark_tabled_preds(CTXT);
  return sweep_tabled_preds(CTXT);
}
#else
int gc_tabled_preds(CTXTdecl) 
{

  if (flags[NUM_THREADS] == 1) {
    mark_tabled_preds(CTXT);
    return sweep_tabled_preds(CTXT);
  } else {
    mark_private_tabled_preds(CTXT);
    return sweep_private_tabled_preds(CTXT);
  } 
}
#endif

/*----------------------------------------------------------------------*/
/* abolish_module_tables() and supporting code */
/*------------------------------------------------------------------*/

int abolish_usermod_tables(CTXTdecl)
{
  size_t i;
  Pair pair;
  Psc psc;
  TIFptr tif;
  declare_timer

  start_table_gc_time(timer);

  mark_cp_tabled_preds(CTXT);

  for (i=0; i<symbol_table.size; i++) {
    if ((pair = (Pair) *(symbol_table.table + i))) {
      byte type;
      
      psc = pair_psc(pair);
      type = get_type(psc);
      if (type == T_DYNA || type == T_PRED) 
	if (!get_data(psc) || isstring(get_data(psc)) ||
	    !strcmp(get_name(get_data(psc)),"usermod") ||
	    !strcmp(get_name(get_data(psc)),"global")) 
	  if (get_tabled(psc)) {
	    tif = get_tip(CTXTc psc);
	    /* Check CPS stack = TRUE */
	    abolish_table_predicate_switch(CTXTc tif, psc, ABOLISH_TABLES_SINGLY, FALSE,ERROR_ON_INCOMPLETE);
	  }
    }
  }

  unmark_cp_tabled_preds(CTXT);

  end_table_gc_time(timer);

  return TRUE;
}

/* - - - - - - - - - - */

int abolish_module_tables(CTXTdeclc const char *module_name)
{
  Pair modpair, pair;
  byte type;
  Psc psc, module;
  TIFptr tif;
  declare_timer

  start_table_gc_time(timer);
  
  mark_cp_tabled_preds(CTXT);
  modpair = (Pair) flags[MOD_LIST];
  
  while (modpair && 
	 strcmp(module_name,get_name(pair_psc(modpair))))
    modpair = pair_next(modpair);

  if (!modpair) {
    xsb_warn(CTXTc "[abolish_module_tables] Module %s not found.\n",
		module_name);
    return FALSE;
  }

  module = pair_psc(modpair);
  pair = (Pair) get_data(module);

  while (pair) {
    psc = pair_psc(pair);
    type = get_type(psc);
    if (type == T_DYNA || type == T_PRED) 
      if (get_tabled(psc)) {
	tif = get_tip(CTXTc psc);
	abolish_table_predicate_switch(CTXTc tif, psc, ABOLISH_TABLES_SINGLY, FALSE,ERROR_ON_INCOMPLETE);
      }
    pair = pair_next(pair);
  }
  unmark_cp_tabled_preds(CTXT);

  end_table_gc_time(timer);
  return TRUE;
}

/*----------------------------------------------------------------------*/
/* abolish_nonincremental_tables() */
/*------------------------------------------------------------------*/

int abolish_nonincremental_tables(CTXTdeclc int incomplete_action)
{
  size_t i;
  Pair modpair, pair;
  byte type;
  Psc psc, module;
  TIFptr tif;
  VariantSF sf, this_next; VariantSF * last_next;
  int action;
  declare_timer

  start_table_gc_time(timer);
  
  mark_cp_tabled_preds(CTXT);
  mark_cp_tabled_subgoals(CTXT);

  abolish_dbg(("abolishing non-incremental tables\n"));
  if (flags[NUM_THREADS] == 1) {
    action = CAN_RECLAIM;
  } else action = CANT_RECLAIM;

  /********  first abolish tables in usermod *********/
  
  for (i=0; i<symbol_table.size; i++) {
    if ((pair = (Pair) *(symbol_table.table + i))) {
      byte type;
      
      while (pair) {
	psc = pair_psc(pair);
	type = get_type(psc);
	if (type == T_DYNA || type == T_PRED) {
	  if (!get_data(psc) || isstring(get_data(psc)) ||!strcmp(get_name(get_data(psc)),"usermod") ||  !strcmp(get_name(get_data(psc)),"global"))
	    if (get_tabled(psc) && !get_incr(psc)) {
	      tif = get_tip(CTXTc psc);
	      if (tif) {
		if (IsVariantPredicate(tif)) {
		  sf = TIF_Subgoals(tif);  last_next = &TIF_Subgoals(tif); 
		  while (IsNonNULL(sf)) {
		    this_next = subg_next_subgoal(sf);
		    if ( is_completed(sf) ) {
		      abolish_dbg(("... abolishing non-incremental tables-1 (usermod) %p\n",sf));
		      //		      delete_branch(CTXTc sf->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
		      *last_next = this_next;
		      abolish_nonincremental_call_single_nocheck_deltf(CTXTc sf, action);
		      //		printf("deleting ");print_subgoal(stddbg,sf);printf("\n");
		    }
		    else {
		      //		printf("skipping ");print_subgoal(stddbg,sf);printf("\n");
		      if (incomplete_action == ERROR_ON_INCOMPLETE) {
			char message[ERRMSGLEN/2];
			snprintf(message,ERRMSGLEN/2,"incomplete tabled predicate %s/%d",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
			xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
		      }
		    }
		    sf = this_next;
		  }
		}
		else delete_predicate_table(CTXTc tif, TRUE);   // Delete whole subsumptive table (should delve into calls)
	      }
	    }
	}
      pair = pair_next(pair);
      }
    }
  }
  /********  next, abolish tables not in usermod *********/
  modpair = (Pair) flags[MOD_LIST];
  
  while (modpair && strcmp("usermod",get_name(pair_psc(modpair))) && strcmp("global",get_name(pair_psc(modpair)))) {
    //    printf("abolishing %s\n",get_name(pair_psc(modpair)));
    module = pair_psc(modpair);
    pair = (Pair) get_data(module);

    while (pair) {
      psc = pair_psc(pair);
      type = get_type(psc);
      if (type == T_DYNA || type == T_PRED) {
	if (!get_data(psc) || isstring(get_data(psc)) ||!strcmp(get_name(get_data(psc)),"usermod") ||  !strcmp(get_name(get_data(psc)),"global"))
	  if (get_tabled(psc) && !get_incr(psc)) {
	    tif = get_tip(CTXTc psc);
	    if (IsVariantPredicate(tif)) {
	      sf = TIF_Subgoals(tif);  last_next = &TIF_Subgoals(tif); 
	      while (IsNonNULL(sf)) {
		//	      printf("TIF_Subgoals %p\n",TIF_Subgoals(tif));
		this_next = subg_next_subgoal(sf);
		if ( is_completed(sf) ) {
		  abolish_dbg(("... abolishing non-incremental tables-1 (non-usermod) %p\n",sf));
		  //		  delete_branch(CTXTc sf->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
		  *last_next = this_next;
		  abolish_nonincremental_call_single_nocheck_deltf(CTXTc sf, action);
		  //		printf("deleting ");print_subgoal(stddbg,sf);printf("\n");
		  //		  delete_variant_call(CTXTc sf, SHOULD_COND_WARN) ;
		}
		else {
		  //		printf("skipping ");print_subgoal(stddbg,sf);printf("\n");
		  if (incomplete_action == ERROR_ON_INCOMPLETE) {
		    char message[ERRMSGLEN/2];
		    snprintf(message,ERRMSGLEN/2,"incomplete tabled predicate %s/%d",get_name(TIF_PSC(tif)),get_arity(TIF_PSC(tif)));
		    xsb_permission_error(CTXTc "abolish",message,0,"abolish_table_pred",1);
		  }
		}
		sf = this_next;
	      }
	    }
	    else delete_predicate_table(CTXTc tif, TRUE);   // Delete whole subsumptive table (should delve into calls)
	  }
      }
      pair = pair_next(pair);
    }
    modpair = pair_next(modpair);
  }

  unmark_cp_tabled_preds(CTXT);
  unmark_cp_tabled_subgoals(CTXT);

  end_table_gc_time(timer);
  return TRUE;
}

/*----------------------------------------------------------------------*/
/* abolish_private/shared_tables() and supporting code */
/*------------------------------------------------------------------*/

/* Freeing deltfs is necessary in abolish_all_*_tables so that we
   don't later try to reclaim a subgoal or predicate that had been
   reclaimed by abolish_all_tables */

static inline void free_global_deltfs(CTXTdecl) {

  DelTFptr next_deltf;
  DelTFptr deltf = deltf_chain_begin;
  deltf_chain_begin = NULL;

  while (deltf) {
    next_deltf = DTF_NextDTF(deltf);
    mem_dealloc(deltf,sizeof(DeletedTableFrame),TABLE_SPACE);		
    deltf = next_deltf;
  }
}

#ifdef MULTI_THREAD
static inline void thread_free_private_deltfs(CTXTdecl) {

  DelTFptr next_deltf;
  DelTFptr deltf = private_deltf_chain_begin;

  while (deltf) {
    next_deltf = DTF_NextDTF(deltf);
    mem_dealloc(deltf,sizeof(DeletedTableFrame),TABLE_SPACE);		
    deltf = next_deltf;
  }
}
#endif

/* Factored into a function because of large buffer */
void check_for_incomplete_tables_error(CTXTdeclc VariantSF subgoal,char * predname) {
  
  sprint_subgoal(CTXTc forest_log_buffer_1,0, subgoal);
  xsb_table_error_vargs(CTXTc predname,"[%s] Illegal table operation:  Cannot abolish incomplete tables."
			"\n\t First incomplete subgoal encountered: %s\n",predname,forest_log_buffer_1->fl_buffer);		
  
}

#define check_for_incomplete_tables(PredName)			\
  {								\
    CPtr csf;							\
    for ( csf = top_of_complstk;  csf != COMPLSTACKBOTTOM;	\
	  csf = csf + COMPLFRAMESIZE )				\
      if ( ! is_completed(compl_subgoal_ptr(csf)) ) {			\
	check_for_incomplete_tables_error(CTXTc compl_subgoal_ptr(csf),PredName);			\
      }									\
  }

#ifdef MULTI_THREAD

/* Checks to see whether there are any private tables in choice point
   stack */
int abolish_mt_tables_cps_check(CTXTdecl,xsbBool isPrivate) 
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  BTNptr trieNode;
  CPtr dlist;
  Cell tmp_cell;
  VariantSF subgoal;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;
  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    // Want trie insts, but will need to distinguish from
    // asserted and interned tries
    if ( is_trie_instruction(cp_inst) ) {
      trieNode = TrieNodeFromCP(cp_top1);
      // Below we want basic_answer_trie_tt, ts_answer_trie_tt
      if (get_private(get_psc_for_trie_cp(CTXTc cp_top1, trieNode))  == isPrivate) {
	return CANT_RECLAIM;
      }
    }
    /* Now check delaylist */
    dlist = cp_pdreg(cp_top1);
    while (islist(dlist) ) {
      dlist = clref_val(dlist);
      tmp_cell = cell((CPtr)cs_val(cell(dlist)) + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      if (get_private(TIF_PSC(subg_tif_ptr(subgoal))) == isPrivate) 
	return CANT_RECLAIM;
      dlist = (CPtr)cell(dlist+1);
    }
    cp_top1 = cp_prevtop(cp_top1);
  }
  return CAN_RECLAIM;
}

/* will not reclaim space if more than one thread */
void abolish_shared_tables(CTXTdecl) {
  TIFptr abol_tif;
  declare_timer

  start_table_gc_time(timer);
  mark_cp_tabled_preds(CTXT);

  for (abol_tif = tif_list.first ; abol_tif != NULL
	 ; abol_tif = TIF_NextTIF(abol_tif) ) {
    abolish_table_predicate_switch(CTXTc abol_tif, TIF_PSC(abol_tif), ABOLISH_TABLES_SINGLY, FALSE,ERROR_ON_INCOMPLETE);
  }

  unmark_cp_tabled_preds(CTXT);
  end_table_gc_time(timer);

}

/* will not reclaim space if more than one thread (via fast_atp) */
void abolish_all_shared_tables(CTXTdecl) {
  TIFptr pTIF;
  declare_timer

  start_table_gc_time(timer);

  // FALSE means we found a shared table
  if (flags[NUM_THREADS] != 1) {
    xsb_table_error(CTXTc 
		    "abolish_all_shared_tables/1 called with more than one active thread.");
  } else {
    
    check_for_incomplete_tables("abolish_all_shared_tables/0");
    if ( abolish_mt_tables_cps_check(CTXTc FALSE) ) 
      xsb_permission_error(CTXTc "abolish","backtracking through tables to be abolished",0,"abolish_all_shared_tables",0);
    //      xsb_abort("[abolish_all_shared_tables/0] Illegal table operation"
    //		"\n\t Backtracking through tables to be abolished.");
    else {
      for ( pTIF = tif_list.first; IsNonNULL(pTIF) ; pTIF = TIF_NextTIF(pTIF) ) {
	  TIF_CallTrie(pTIF) = NULL;
    	  TIF_Subgoals(pTIF) = NULL;
      }

      free_global_deltfs(CTXT);
      SM_ReleaseResources(smTableBTN);
      TrieHT_FreeAllocatedBuckets(smTableBTHT);
      SM_ReleaseResources(smTableBTHT);
      SM_ReleaseResources(smALN);
      SM_ReleaseResources(smVarSF);
      SM_ReleaseResources(smASI);
      
      abolish_wfs_space(CTXT);
    }
  }
  end_table_gc_time(timer);

}

void abolish_private_tables(CTXTdecl) {
  TIFptr abol_tif;
  declare_timer

  start_table_gc_time(timer);

  mark_cp_tabled_preds(CTXT);

  for (abol_tif = private_tif_list.first ; abol_tif != NULL
	 ; abol_tif = TIF_NextTIF(abol_tif) ) {
    //    printf("calling %s\n",get_name(TIF_PSC(abol_tif)));
    abolish_table_predicate_switch(CTXTc abol_tif, TIF_PSC(abol_tif), ABOLISH_TABLES_SINGLY, FALSE,ERROR_ON_INCOMPLETE);
  }

  unmark_cp_tabled_preds(CTXT);

  end_table_gc_time(timer);
}

void abolish_all_private_tables(CTXTdecl) {
  TIFptr pTIF;
  declare_timer

  start_table_gc_time(timer);

  check_for_incomplete_tables("abolish_all_private_tables/0");

  // TRUE means we found a private table
  if ( abolish_mt_tables_cps_check(CTXTc TRUE) ) 
    xsb_permission_error(CTXTc "abolish","backtracking through tables to be abolished",0,"abolish_all_private_tables",0);
  //    xsb_abort("[abolish_all_private_tables/0] Illegal table operation"
  //		  "\n\t Backtracking through tables to be abolished.");
  else {
    for ( pTIF = private_tif_list.first; IsNonNULL(pTIF)
	  			       ; pTIF = TIF_NextTIF(pTIF) ) {
	  TIF_CallTrie(pTIF) = NULL;
    	  TIF_Subgoals(pTIF) = NULL;
    }

    thread_free_private_deltfs(CTXT);
    SM_ReleaseResources(*private_smTableBTN);
    TrieHT_FreeAllocatedBuckets(*private_smTableBTHT);
    SM_ReleaseResources(*private_smTableBTHT);
    SM_ReleaseResources(*private_smTSTN);
    TrieHT_FreeAllocatedBuckets(*private_smTSTHT);
    SM_ReleaseResources(*private_smTSTHT);
    SM_ReleaseResources(*private_smTSIN);
    SM_ReleaseResources(*private_smALN);
    SM_ReleaseResources(*private_smVarSF);
    SM_ReleaseResources(*private_smProdSF);
    SM_ReleaseResources(*private_smConsSF);
    SM_ReleaseResources(*private_smASI);

    abolish_private_wfs_space(CTXT);

  }
  end_table_gc_time(timer);
}

extern struct TDispBlkHdr_t tdispblkhdr; // defined in loader

/* This function handles the case when one thread creates a private
   tif, exits, its xsb_thread_id is reused, and the new thread creates
   a private tif for the same table.  Mutex may not be needed here, as
   we're freeing private resources (nobody except the current thread
   will access &(tdispblk->Thread0))[xsb_thread_entry]) */

void thread_free_private_tifs(CTXTdecl) {
  struct TDispBlk_t *tdispblk;
  TIFptr tip;

  SYS_MUTEX_LOCK( MUTEX_TABLE );
  for (tdispblk=tdispblkhdr.firstDB 
	 ; tdispblk != NULL ; tdispblk=tdispblk->NextDB) {
    if (xsb_thread_entry <= tdispblk->MaxThread) {
      tip = (&(tdispblk->Thread0))[xsb_thread_entry];
      if (tip) {
	(&(tdispblk->Thread0))[xsb_thread_entry] = (TIFptr) NULL; {
	  Free_Private_TIF(tip);
	}
      }
    }
  }
  SYS_MUTEX_UNLOCK( MUTEX_TABLE );
}

void release_private_tabling_resources(CTXTdecl) {

  thread_free_private_deltfs(CTXT);
  thread_free_private_tifs(CTXT);
  SM_ReleaseResources(*private_smTableBTN);
  TrieHT_FreeAllocatedBuckets(*private_smTableBTHT);
  SM_ReleaseResources(*private_smTableBTHT);
  SM_ReleaseResources(*private_smTSTN);
  TrieHT_FreeAllocatedBuckets(*private_smTSTHT);
  SM_ReleaseResources(*private_smTSTHT);
  SM_ReleaseResources(*private_smTSIN);
  SM_ReleaseResources(*private_smALN);
  SM_ReleaseResources(*private_smVarSF);
  SM_ReleaseResources(*private_smProdSF);
  SM_ReleaseResources(*private_smConsSF);
  SM_ReleaseResources(*private_smASI);
  mem_dealloc(itrie_array,((max_interned_tries_glc+1) * sizeof(struct interned_trie_t)), TABLE_SPACE);
  //    mem_calloc(max_interned_tries_glc+1, sizeof(struct interned_trie_t), TABLE_SPACE);
}

#endif

/*----------------------------------------------------------------------*/
/* abolish_all_tables() and supporting code */
/*------------------------------------------------------------------*/

void reinitialize_incremental_tries(CTXTdecl) {

int index =  itrie_array_first_trie;

 if (index >= 0) {
   do {
     if (itrie_array[index].incremental == 1) {
       itrie_array[index].callnode = makecallnode(NULL);
       //       printf("reinitizlizing incremental trie %d %p\n",index,itrie_array[index].callnode);
       initoutedges(CTXTc (callnodeptr)itrie_array[index].callnode);
     }
     //     printf("next %d\n",itrie_array[index].next_entry);
     index = itrie_array[index].next_entry;
   }
   while (index >= 0);
 }
}


/*
 * Frees all the tabling space resources (with a hammer)
 * WFS stuff released elsewhere -- including smASI.
 */

void release_all_tabling_resources(CTXTdecl) {
  free_global_deltfs(CTXT);
  SM_ReleaseResources(smTableBTN);
  TrieHT_FreeAllocatedBuckets(smTableBTHT);
  SM_ReleaseResources(smTableBTHT);
  SM_ReleaseResources(smTSTN);
  //    SM_ReleaseResources(*private_smTSTN);
  TrieHT_FreeAllocatedBuckets(smTSTHT);
  SM_ReleaseResources(smTSTHT);
  SM_ReleaseResources(smTSIN);
  SM_ReleaseResources(smALN);
  SM_ReleaseResources(smVarSF);
  SM_ReleaseResources(smProdSF);
  SM_ReleaseResources(smConsSF);
  SM_ReleaseResources(smASI);

  /* Release incremental structure managers */
#ifndef MULTI_THREAD
  SM_ReleaseResources(smCallNode);
  SM_ReleaseResources(smCallList);
  SM_ReleaseResources(smCall2List);
  SM_ReleaseResources(smOutEdge);
  SM_ReleaseResources(smKey);
#endif

  /* In mt engine, also release private resources */
#ifdef MULTI_THREAD
    thread_free_private_deltfs(CTXT);
    SM_ReleaseResources(*private_smTableBTN);
    TrieHT_FreeAllocatedBuckets(*private_smTableBTHT);
    SM_ReleaseResources(*private_smTableBTHT);
    //    SM_ReleaseResources(*private_smTSTN);
    TrieHT_FreeAllocatedBuckets(*private_smTSTHT);
    SM_ReleaseResources(*private_smTSTHT);
    //    SM_ReleaseResources(*private_smTSIN);
    SM_ReleaseResources(*private_smALN);
    SM_ReleaseResources(*private_smVarSF);
    //    SM_ReleaseResources(*private_smProdSF);
    //    SM_ReleaseResources(*private_smConsSF);
    SM_ReleaseResources(*private_smASI);
#endif

}

/* TLS: Unlike the other abolishes, "all" aborts if it detects the
   presence of CPs for completed tables (incomplete tables are caught
   as before, by examining the completion stack).  It also aborts if
   called with more than one thread.
*/

void abolish_all_tables_cps_check(CTXTdecl) 
{
  CPtr cp_top1,cp_bot1 ;
  byte cp_inst;
  BTNptr trieNode;

  cp_bot1 = (CPtr)(tcpstack.high) - CP_SIZE;
  cp_top1 = breg ;				 
  while ( cp_top1 < cp_bot1 ) {
    cp_inst = *(byte *)*cp_top1;
    /* Check for trie instructions */
    if ( is_trie_instruction(cp_inst)) {
      trieNode = TrieNodeFromCP(cp_top1);
      /* Here, we want call_trie_tt,basic_answer_trie_tt,ts_answer_trie_tt"*/
      if (IsInAnswerTrie(trieNode) || cp_inst == trie_fail) {
	abolish_dbg(("[abolish_all_tables/0] Illegal table operation"
		       "\n\t Backtracking through tables to be abolished.\n"));
	xsb_permission_error(CTXTc "abolish","backtracking through tables to be abolished",0,"abolish_all_tables",0);
	//	xsb_abort("[abolish_all_tables/0] Illegal table operation"
	//		  "\n\t Backtracking through tables to be abolished.");
      }
    }
    /* Now check delaylist */
    if ( cp_pdreg(cp_top1) != (CPtr) NULL)  {
      //      int ctr = 0;
      memset(forest_log_buffer_2,0,MAXTERMBUFSIZE);
      sprint_delay_list(CTXTc forest_log_buffer_1, cp_pdreg(cp_top1));
      sprintf(forest_log_buffer_2->fl_buffer,
	      "[abolish_all_tables/0] Illegal table operation"
	      "\n\t tables to be abolished are in delay list, e.g.: %s",
	      forest_log_buffer_1->fl_buffer);
      xsb_abort(forest_log_buffer_2->fl_buffer);
    }
    cp_top1 = cp_prevtop(cp_top1);
  }
}

#if !defined(WIN_NT) || defined(CYGWIN) 
inline 
#endif
void abolish_all_tables(CTXTdeclc int action) {
  declare_timer
  TIFptr pTIF;

  abolish_dbg(("abolish all tables called\n"));

  start_table_gc_time(timer);

  /* a_a_t() may be called with ABOLISH_INCOMPLETES for memory errors; otherwsie called with ERROR_ON_INCOMPLETES */
  if (action == ERROR_ON_INCOMPLETES) {
    check_for_incomplete_tables("abolish_all_shared_tables/0");

    if (flags[NUM_THREADS] == 1) {
      abolish_all_tables_cps_check(CTXT) ;
    } else {
      xsb_table_error(CTXTc 
		      "abolish_all_tables/1 called with more than one active thread.");
    }
  }

  for ( pTIF = tif_list.first; IsNonNULL(pTIF); pTIF = TIF_NextTIF(pTIF) ) {
    TIF_CallTrie(pTIF) = NULL;
    TIF_Subgoals(pTIF) = NULL;
  }

#ifdef MULTI_THREAD
    for ( pTIF = private_tif_list.first; IsNonNULL(pTIF)
  	  ; pTIF = TIF_NextTIF(pTIF) ) {
      TIF_CallTrie(pTIF) = NULL;
      TIF_Subgoals(pTIF) = NULL;
    }
#endif

 if (flags[CTRACE_CALLS])  { 
    fprintf(fview_ptr,"ta(all,%d).\n",ctrace_ctr++);
  }
  reset_freeze_registers;
  openreg = COMPLSTACKBOTTOM;
  hashtable1_destroy_all(0);             /* free all incr hashtables in use */
  release_all_tabling_resources(CTXT);
  current_call_node_count_gl = 0; current_call_edge_count_gl = 0;
  abolish_wfs_space(CTXT);               // free wfs stuff that does not use structure managers
  //  affected_gl = empty_calllist();
  reinitialize_incremental_tries(CTXT);  
  //  changed_gl = empty_calllist();
  incr_table_update_safe_gl = TRUE;
  lazy_affected = empty_calllist();
  end_table_gc_time(timer);
}


//--------------------------------------------------------------------------------------------

/* Checks are either done in abolish_call or in throw
   Do not delete branch here, as this may be called by gc. */
void abolish_incremental_call_single_nocheck_no_nothin(CTXTdeclc VariantSF goal, int cond_warn_flag) {
  #ifdef DEBUG_ABOLISH
  abolish_dbg(("abolish incr call single: %p ",goal)); print_subgoal(CTXTc stddbg,goal); 
  //  printf("(id %d flag %d)\n",callnode->id,invalidate_flag);
  //  print_call_list(CTXTc affected_gl," affected_gl ");
  #endif
  SET_TRIE_ALLOCATION_TYPE_SF(goal); // set smBTN to private/shared
  //  tif = subg_tif_ptr(goal);
  //  delete_branch(CTXTc goal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
  delete_variant_call(CTXTc goal,cond_warn_flag); // delete answers + subgoal
  abolish_dbg(("end aics\n"));
}

void maybe_detach_incremental_call_single(CTXTdeclc VariantSF goal, int invalidate_flag) {
  callnodeptr callnode;
  if (!IsIncrSF(goal)) return;

  callnode = subg_callnode_ptr(goal);
  #ifdef DEBUG_ABOLISH
  abolish_dbg(("detachinng incr call for gc: %p ",goal)); print_subgoal(CTXTc stddbg,goal); 
  printf("(id %d flag %d)\n",callnode->id,invalidate_flag);
  //  print_call_list(CTXTc affected_gl," affected_gl ");
  #endif
  if (invalidate_flag) {
    invalidate_call(CTXTc callnode,ABOLISHING);
  }
  deleteoutedges(CTXTc callnode);
  deleteinedges(CTXTc callnode);
  deletecallnode(callnode);
  //  SET_TRIE_ALLOCATION_TYPE_SF(goal); // set smBTN to private/shared
  //  tif = subg_tif_ptr(goal);
  //  delete_branch(CTXTc goal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
  //  delete_variant_call(CTXTc goal,cond_warn_flag); // delete answers + subgoal
  abolish_dbg(("end detach incremental call\n"));
  //  if (affected_gl->item != NULL || changed_gl != NULL) incr_table_update_safe_gl = FALSE;
}

void abolish_incremental_call_single_nocheck_deltf(CTXTdeclc VariantSF subgoal, int action, int invalidate_flag) {
    TIFptr tif;

    tif = subg_tif_ptr(subgoal);

    if (flags[CTRACE_CALLS])  { 
      sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)subgoal);     
      fprintf(fview_ptr,"abol_incr_call_sing(subg(%s),%d).\n",forest_log_buffer_1->fl_buffer,ctrace_ctr++);
    }

    SET_TRIE_ALLOCATION_TYPE_SF(subgoal); // set smBTN to private/shared
    /* Invalidate Flag -- don't self-invalidate when abolishing, leads to core dump */
    maybe_detach_incremental_call_single(CTXTc subgoal,invalidate_flag);
    delete_branch(CTXTc subgoal->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
    if (action == CAN_RECLAIM && !GC_MARKED_SUBGOAL(subgoal)) {
      abolish_incremental_call_single_nocheck_no_nothin(CTXTc subgoal,SHOULD_COND_WARN);
    }
    else {  /* CANT_RECLAIM */
      //	printf("Mark %x GC %x\n",subgoal->visited,GC_MARKED_SUBGOAL(subgoal));
      check_insert_private_deltf_subgoal(CTXTc subgoal,FALSE);
    }
}


/*----------------------------------------------------------------------*/

/* TLS: this should be used only for subsumptive tables.  And even so,
   I think it could probably be replaced by some other deletion
   routine in tr_utils.c (?) */

static void remove_calls_and_returns(CTXTdeclc VariantSF CallStrPtr)
{
  ALNptr pALN;

  /* Delete the call entry
     --------------------- */
  SET_TRIE_ALLOCATION_TYPE_SF(CallStrPtr);
  delete_branch(CTXTc subg_leaf_ptr(CallStrPtr),
		&TIF_CallTrie(subg_tif_ptr(CallStrPtr)),VARIANT_EVAL_METHOD);

  /* Delete its answers
     ------------------ */
  for ( pALN = subg_answers(CallStrPtr);  IsNonNULL(pALN); pALN = ALN_Next(pALN) )
    delete_branch(CTXTc ALN_Answer(pALN), &subg_ans_root_ptr(CallStrPtr),SUBSUMPTIVE_EVAL_METHOD);

  /* Delete the table entry
     ---------------------- */
  free_answer_list(CallStrPtr);
  FreeProducerSF(CallStrPtr);
}

void remove_incomplete_tries(CTXTdeclc CPtr bottom_parameter)
{
  xsbBool warned = FALSE;
  VariantSF CallStrPtr;
  TIFptr tif;
  while (openreg < bottom_parameter) {
    CallStrPtr = (VariantSF)compl_subgoal_ptr(openreg);
    if (!is_completed(CallStrPtr)) {
      if (warned == FALSE) {
	xsb_mesg("Removing incomplete tables...");
	//	check_table_cut = FALSE;  /* permit cuts over tables */
	warned = TRUE;
      }
  if (flags[CTRACE_CALLS] && !subg_forest_log_off(CallStrPtr))  {		\
    sprint_subgoal(CTXTc forest_log_buffer_1,0,(VariantSF)CallStrPtr);	\
    fprintf(fview_ptr,"abort(%s,%d).\n",forest_log_buffer_1->fl_buffer,ctrace_ctr++);  
  }
  /* TLS: will need to change when incr combined with subsumptive */
  //     else if (IsVariantSF(CallStrPtr)) {
  if (IsVariantSF(CallStrPtr)) {
    SET_TRIE_ALLOCATION_TYPE_SF(CallStrPtr); // set smBTN to private/shared
    tif = subg_tif_ptr(CallStrPtr);
    delete_branch(CTXTc CallStrPtr->leaf_ptr, &tif->call_trie,VARIANT_EVAL_METHOD); /* delete call */
    //       delete_variant_call(CTXTc CallStrPtr,FALSE); // delete answers + subgoal
    maybe_detach_incremental_call_single(CTXTc CallStrPtr,DONT_INVALIDATE);
    abolish_table_call_single_nocheck_no_nothin(CTXTc CallStrPtr,SHOULDNT_COND_WARN);
  } else remove_calls_and_returns(CTXTc CallStrPtr);  // not sure why this is used, and not delete_subsumptive_predicate_table
    }
    openreg += COMPLFRAMESIZE;
  }
  level_num = compl_level(openreg);
}


#define is_encoded_addr(term)	(isointeger(term))
#define decode_addr(term)	(void *)oint_val(term)
#define ctop_addr(regnum, val)    ctop_int(CTXTc regnum, (prolog_int)val)

#ifndef MULTI_THREAD
extern int vcs_tnot_call;
#endif
    /*
     * Given a tabled goal, report on the following attributes:
     * 1) Predicate Type: Variant, Subsumptive, or Untabled
     * 2) Goal Type: Producer, Properly Subsumed Consumer, Has No
     *      Call Table Entry, or Undefined
     * 3) Answer Set Status: Complete, Incomplete, Undefined or Inremental-needs-reeval
     *
     * Valid combinations reported by this routine:
     * When the predicate is an untabled functor, then only one sequence
     *   is generated:  Untabled,Undefined,Undefined
     * Otherwise the following combinations are possible:
     *
     * GoalType    AnsSetStatus   Meaning
     * --------    ------------   -------
     * producer    complete       call exists; it is a completed producer.
     *             incomplete     call exists; it is an incomplete producer.
     *
     * subsumed    complete       call exists; it's properly subsumed by a
     *                              completed producer.
     *             incomplete     call exists; it's properly subsumed by an
     *                              incomplete producer.
     *
     * no_entry    undefined      is a completely new call, not subsumed by
     *                              any other -> if this were to be called
     *                              right now, it would be a producing call.
     *             complete       there is no entry for this call, but if it
     *                              were to be called right now, it would
     *                              consume from a completed producer.
     *                              (The call is properly subsumed.)
     *             incomplete     same as previous, except the subsuming
     *                              producer is incomplete.
     *
     * Notice that not only can these combinations describe the
     * characteristics of a subgoal in the table, but they are also
     * equipped to predict how a new goal would have been treated had it
     * really been called.
     */

int table_status(CTXTdeclc Cell goalTerm, TableStatusFrame* ResultsFrame) {
  int pred_type, goal_type, answer_set_status;
  VariantSF goalSF = NULL, subsumerSF;

  if ( is_encoded_addr(goalTerm) ) {
    //    printf("encoded addr %p\n",goalTerm);
    goalSF = (VariantSF)decode_addr(goalTerm);
  if ( IsProperlySubsumed(goalSF) )
    subsumerSF = (VariantSF)conssf_producer(goalSF);
  else
    subsumerSF = goalSF;
  pred_type = TIF_EvalMethod(subg_tif_ptr(subsumerSF));
  }
  else {   /* Regular term: Not SF ptr */
    Psc psc;   TIFptr tif;
    
    psc = term_psc(goalTerm);
    if ( IsNULL(psc) ) {
      xsb_type_error(CTXTc "callable",goalTerm,"table_status/4",1);
      return 0;
      }
    tif = get_tip(CTXTc psc);
    if ( IsNULL(tif) ) {
      TableStatusFrame_pred_type(*ResultsFrame) = UNTABLED_PREDICATE;
      TableStatusFrame_goal_type(*ResultsFrame) = UNDEFINED_CALL;
      TableStatusFrame_answer_set_status(*ResultsFrame) = UNDEFINED_ANSWER_SET;
      TableStatusFrame_subgoal(*ResultsFrame) = goalSF;
      return TRUE;
    }
    pred_type = TIF_EvalMethod(tif);
    if ( IsVariantPredicate(tif) )
      goalSF = subsumerSF = get_variant_sf(CTXTc goalTerm, tif, NULL);
      else {
	BTNptr root, leaf;
	TriePathType path_type;

	root = TIF_CallTrie(tif);
	if ( IsNonNULL(root) )
	  leaf = subsumptive_trie_lookup(CTXTc root, get_arity(psc),
					 clref_val(goalTerm) + 1,
					 &path_type, NULL);
	else {
	  leaf = NULL;
	  path_type = NO_PATH;
	}
	if ( path_type == NO_PATH )
	  goalSF = subsumerSF = NULL;
	else if ( path_type == VARIANT_PATH ) {
	  goalSF = CallTrieLeaf_GetSF(leaf);
	  if ( IsProperlySubsumed(goalSF) )
	    subsumerSF = (VariantSF)conssf_producer(goalSF);
	  else
	    subsumerSF = goalSF;
	}
	else {
	  goalSF = NULL;
	  subsumerSF = CallTrieLeaf_GetSF(leaf);
	  if ( IsProperlySubsumed(subsumerSF) )
	    subsumerSF = (VariantSF)conssf_producer(subsumerSF);
	}
      }
    }
    /*
     * Now both goalSF and subsumerSF should be set for all cases.
     * Determine status values based on these pointers.
     */
#ifndef SHARED_COMPL_TABLES
    if ( IsNonNULL(goalSF) ) {
#else
    if ( IsNonNULL(goalSF) && !subg_grabbed(goalSF)) {
#endif
      if ( goalSF == subsumerSF )
	goal_type = PRODUCER_CALL;
      else
	goal_type = SUBSUMED_CALL;
    }
    else
      goal_type = NO_CALL_ENTRY;

#ifndef SHARED_COMPL_TABLES
    if ( IsNonNULL(subsumerSF) ) {
#else
    if ( IsNonNULL(subsumerSF) && !subg_grabbed(subsumerSF)) {
#endif
      if ( is_completed(subsumerSF) ) {
	if (subg_callnode_ptr(subsumerSF) && subg_callnode_ptr(subsumerSF)->falsecount!=0)
	  answer_set_status = INCR_NEEDS_REEVAL;
	else answer_set_status = COMPLETED_ANSWER_SET;
      }
      else
	answer_set_status = INCOMPLETE_ANSWER_SET;
    }
    else
      answer_set_status = UNDEFINED_ANSWER_SET;

    TableStatusFrame_pred_type(*ResultsFrame) = pred_type;
    TableStatusFrame_goal_type(*ResultsFrame) = goal_type;
    TableStatusFrame_answer_set_status(*ResultsFrame) = answer_set_status;
    TableStatusFrame_subgoal(*ResultsFrame) = goalSF;
    return TRUE;
}

//----------------------------------------------------------------------
// Code from here to end of file is under development -- TLS

// Code for marking ASI scratchpad

#ifdef BITS64
#define VISITED_MASK 0xf000000000000000
#else
#define VISITED_MASK 0xf0000000
#endif

#ifdef BITS64
#define STACK_MASK 0xffffffffffffff
#else
#define STACK_MASK 0xffffff
#endif

#define VISITED_ANSWER(as_leaf)  (asi_scratchpad((ASI) Child(as_leaf)) & VISITED_MASK)
#define STACK_INDEX(as_leaf)  (asi_scratchpad((ASI) Child(as_leaf)) & STACK_MASK)
#define MARK_VISITED_ANSWER(as_leaf) {asi_scratchpad((ASI) Child(as_leaf)) = asi_scratchpad((ASI) Child(as_leaf)) | VISITED_MASK;}
#define MARK_STACK_INDEX(as_leaf,index) {			\
    asi_scratchpad((ASI) Child(as_leaf)) = ( VISITED_MASK | index );}

#define POP_MARK_STACK_INDEX(as_leaf,index) {			\
    asi_scratchpad((ASI) Child(as_leaf)) = ( POPPED_MASK | VISITED_MASK | index );}

static int dfn = 0;

//----------------------------------------------------------------------
//  Component stack is used for SCC detection and DFS

struct answer_dfn {
  BTNptr  answer;
  int     dfn;
  int     min_link;
} ;
typedef struct answer_dfn *answerDFN;

#define component_stack_increment 1000

int component_stack_top = 0;
answerDFN component_stack = NULL;
int component_stack_size = 0;

#ifdef BITS64
#define POPPED_MASK 0x0f00000000000000
#else
#define POPPED_MASK 0x0f000000
#endif

#define POPPED_ANSWER(as_leaf)  (asi_scratchpad((ASI) Child(as_leaf)) & POPPED_MASK)
#define	MARK_POPPED(answer_idx) ( \
(asi_scratchpad((ASI) Child(component_stack[answer_idx].answer))) \
= (asi_scratchpad((ASI) Child(component_stack[answer_idx].answer)) | POPPED_MASK))


#define push_comp_node(as_leaf,index) {				\
  if (component_stack_top >= component_stack_size) {\
    size_t old_component_stack_size = component_stack_size; \
    component_stack_size = component_stack_size + component_stack_increment;\
    component_stack = (answerDFN)mem_realloc(component_stack,		\
					     old_component_stack_size*sizeof(struct answer_dfn), \
					     component_stack_size*sizeof(struct answer_dfn), \
					     TABLE_SPACE);		\
  }\
  component_stack[component_stack_top].dfn = dfn;		\
  component_stack[component_stack_top].min_link = dfn++;	\
  component_stack[component_stack_top].answer = as_leaf;	\
  index = component_stack_top;					\
  MARK_STACK_INDEX(as_leaf,index);				\
  component_stack_top++;					\
  }

/* for use when you don't need the index returned */
#define push_comp_node_1(as_leaf) {			\
    int index;						\
    MARK_VISITED_ANSWER(as_leaf);				\
    push_comp_node(as_leaf,index);			\
  }

#define pop_comp_node(node) {				\
    component_stack_top--;				\
    node = component_stack[component_stack_top];	\
  }

#define copy_pop_comp_node(node) {				\
    push_done_node((component_stack_top-1),			\
		   (component_stack[component_stack_top-1].dfn));	\
    component_stack_top--;					\
    node = component_stack[component_stack_top].answer;		\
  }

void print_comp_stack(CTXTdecl) {
  int frame = 0;
  while (frame < component_stack_top) {
    printf("comp_frame %d answer %p dfn %d min_link %d  ",frame,component_stack[frame].answer,
	   component_stack[frame].dfn,component_stack[frame].min_link);
    print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(component_stack[frame].answer)));
    printf("\n");
    frame++;
  }
}

#define deallocate_comp_stack	{					\
  mem_dealloc(component_stack,component_stack_size*sizeof(struct answer_dfn), \
		TABLE_SPACE);						\
  component_stack_size = 0;						\
  component_stack = 0;							\
  }

//----------------------------------------------------------------------

struct done_answer_dfn {
  BTNptr  answer;
  int     scc;
  int     checked;
} ;

typedef struct done_answer_dfn *doneAnswerDFN;

int done_answer_stack_top = 0;
doneAnswerDFN done_answer_stack = NULL;
int done_answer_stack_size = 0;
#define done_answer_stack_increment 1000

#define push_done_node(index,dfn_num) {					\
    if (done_answer_stack_top >= done_answer_stack_size) {				\
      size_t old_done_answer_stack_size = done_answer_stack_size;		\
      done_answer_stack_size = done_answer_stack_size + done_answer_stack_increment; \
      done_answer_stack = (doneAnswerDFN) mem_realloc(done_answer_stack,		\
					       old_done_answer_stack_size*sizeof(struct done_answer_dfn), \
					  done_answer_stack_size*sizeof(struct done_answer_dfn), \
					  TABLE_SPACE);			\
    }									\
    done_answer_stack[done_answer_stack_top].scc = dfn_num;				\
    done_answer_stack[done_answer_stack_top].checked = 0;				\
    done_answer_stack[done_answer_stack_top].answer = component_stack[index].answer;	\
    POP_MARK_STACK_INDEX(component_stack[index].answer,done_answer_stack_top);	\
    done_answer_stack_top++;							\
  }

void print_done_answer_stack(CTXTdecl) {
  int frame = 0;
  while (frame < done_answer_stack_top) {
    if (done_answer_stack[frame].answer) {
      printf("done_frame %d answer %p scc %d checked %d ",frame, 
	     done_answer_stack[frame].answer,done_answer_stack[frame].scc,done_answer_stack[frame].checked);
      print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(done_answer_stack[frame].answer)));
      printf("\n");
      frame++;
    }
  }
}

void alt_print_done_answer_stack(CTXTdecl) {
  int frame = 0;
  int old_scc = -1; int first_scc = 1;
  while (frame < done_answer_stack_top) {
    if (done_answer_stack[frame].answer) {
      if (old_scc != done_answer_stack[frame].scc) {
	if (!first_scc) printf("}\n");
	first_scc = 0; old_scc = done_answer_stack[frame].scc;
	printf("SCC %d { ",done_answer_stack[frame].scc);       
      }
      else printf(", ");
	print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(done_answer_stack[frame].answer)));
    }
    frame++;
  }
  printf("}\n");
}

// reset the scratchpad for each answer in done stack
void reset_done_answer_stack() {
  int frame = 0;
  while (frame < done_answer_stack_top) {
    asi_scratchpad((ASI) Child(done_answer_stack[frame].answer)) = 0;
    frame++;
  }
}

//----------------------------------------------------------------------
/* Returns -1 when no answer found (not 0, as 0 can be an index */
int visited_answer(BTNptr as_leaf) {		
  if (!VISITED_ANSWER(as_leaf)) return -1;
  else return (int)STACK_INDEX(as_leaf);
}

/* Negative DEs dont have answer pointer -- so need to obtain it from subgoal */
BTNptr traverse_subgoal(VariantSF pSF) {
  BTNptr cur_node = 0;

  if ( subg_answers(pSF) == COND_ANSWERS && IsNonNULL(subg_ans_root_ptr(pSF))) {
    cur_node = subg_ans_root_ptr(pSF);
    while (!IsLeafNode(cur_node)) {
      cur_node = BTN_Child(cur_node);
    } 
    return cur_node;
  }
  return 0;
}

#define  update_minlink_minlink(from_answer,to_answer) { \
    if (component_stack[to_answer].min_link < component_stack[from_answer].min_link)	 \
      component_stack[from_answer].min_link = component_stack[to_answer].min_link;	 \
  }							 \

#define  update_minlink_dfn(from_answer,to_answer) { \
    if (component_stack[to_answer].dfn < component_stack[from_answer].min_link) { \
      printf(" to_answer dfn %d from answer minlink %d\n",component_stack[to_answer].dfn,component_stack[from_answer].min_link); \
      component_stack[from_answer].min_link = component_stack[to_answer].dfn;	} \
  }							 \

int table_component_check(CTXTdeclc NODEptr from_answer) {
  DL delayList;
  DE delayElement;
  BTNptr to_answer;
  int to_answer_idx, from_answer_idx, component_num;

  //  if (is_conditional_answer(from_answer)) {
    push_comp_node(from_answer,from_answer_idx);
    //    printf("starting: %d %d ; ",VISITED_ANSWER(from_answer),STACK_INDEX(from_answer)); 
    printf("tcc: ");print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(from_answer)));printf("\n"); 
    printf("*");    print_comp_stack(CTXT); printf("\n");
      
    //    print_comp_stack(CTXT);
    delayList = asi_dl_list((ASI) Child(from_answer));
    while (delayList) {
      delayElement = dl_de_list(delayList);
      while (delayElement) {
	if (de_ans_subst(delayElement)) to_answer = de_ans_subst(delayElement);
	else to_answer = traverse_subgoal(de_subgoal(delayElement));
	if  (0 >  (to_answer_idx = visited_answer(to_answer))) {
	  printf("doing table component check  ");
	  printf("*");    print_comp_stack(CTXT); printf("\n");
	  to_answer_idx = table_component_check(CTXTc to_answer);
	  update_minlink_minlink(from_answer_idx,to_answer_idx);
	} else {
	  if (!POPPED_ANSWER(to_answer)) {
	    //	    printf("updating from dfn to_answer %p %x\n",to_answer,asi_scratchpad((ASI) Child(to_answer)));
	    update_minlink_dfn(from_answer_idx,to_answer_idx);
	  }
	}
	delayElement = de_next(delayElement);
      }
      delayList = de_next(delayList);
    }
      printf("checking: "); 
      print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(from_answer)));printf("\n");

    if (component_stack[from_answer_idx].dfn 
	== component_stack[from_answer_idx].min_link) {
      component_num = component_stack[from_answer_idx].dfn;
      while (from_answer_idx < component_stack_top) {
	MARK_POPPED(from_answer_idx);
	push_done_node(component_stack_top-1,component_num);
	component_stack_top--;
      }
    }
    print_comp_stack(CTXT); printf("\n");
    return from_answer_idx;
    // }
}

void unfounded_component(CTXTdecl) {
  int founded = 0;
  int index;
  int starting_index = 0;
//	int starting_scc = done_answer_stack[starting_index].scc;  This gets done 9 lines from now anyway....
  int starting_scc;
  DL delayList;
  DE delayElement;
  BTNptr cur_answer = 0;  // TLS: compiler (rightly) complained about it being uninit.

  while (starting_index < done_answer_stack_top) {
    founded = 0; 
    index = starting_index;
    starting_scc = done_answer_stack[index].scc;
    while (!founded && done_answer_stack[index].scc == starting_scc 
	   && index < done_answer_stack_top) {
      cur_answer = done_answer_stack[index].answer;
      delayList = asi_dl_list((ASI) Child(cur_answer));
      print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(cur_answer)));printf("\n");
      while (!founded && delayList) {
	delayElement = dl_de_list(delayList);
	while (!founded && delayElement) {
	  if (!de_ans_subst(delayElement)) 
	    founded = 1;
	  else {
	    if (done_answer_stack[STACK_INDEX(de_ans_subst(delayElement))].checked == 1)
	      founded = 1;
	  }
	  delayElement = de_next(delayElement);
	}
	delayList = de_next(delayList);
      }
      index++;
    }
    if (!founded) {
      for ( ; done_answer_stack[starting_index].scc == starting_scc && starting_index < done_answer_stack_top ; starting_index++) {

      printf("SCC %d is unfounded!\n",starting_scc);
      // start simplification operations

      /* This function is not used much, and does not handle call subsumption */ 
      //    SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      //      subgoal = asi_subgoal(Delay(cur_answer));
      release_all_dls(CTXTc Delay(cur_answer));
      //    SET_TRIE_ALLOCATION_TYPE_SF(subgoal);
      handle_unsupported_answer_subst(CTXTc cur_answer);
      //    SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;

      done_answer_stack[starting_index].answer = 0;
      } 
    }
    else {
      printf("SCC %d is founded\n",starting_scc);
      for ( ; done_answer_stack[starting_index].scc == starting_scc && starting_index < done_answer_stack_top ; starting_index++) 
	done_answer_stack[starting_index].checked = 1;
    }
  }
}

#ifdef UNDEFINED

#include "system_xsb.h"

xsbBool checkSupportedAnswer(BTNptr answer_leaf) {
  int index, answer_supported, delaylist_supported;
  DL DelayList;
  DE DelayElement;  

  if (VISITED_ANSWER(answer_leaf)) {
    if (FALSE) /* answer_leaf is in completion stack */
      return FALSE;
    else return TRUE;
  }
  else { /* not visited */
    MARK_VISITED_ANSWER(answer_leaf);
    push_comp_node(answer_leaf,index);
    DelayList = asi_dl_list(Delay(answer_leaf));
    answer_supported = UNKNOWN;
    while (DelayList && answer_supported != TRUE) {
      DelayElement = dl_de_list(DelayList);
      delaylist_supported = TRUE; /* UNKNOWN? */
      while (DelayElement && delaylist_supported != FALSE) {
	if (de_positive(DelayElement) /* && Is in SCC */) {
	  if (!checkSupportedAnswer((BTNptr) de_ans_subst(DelayElement)))
	    delaylist_supported = FALSE;
	  DelayElement = de_next(DelayElement);
	}
        if (delaylist_supported == FALSE) { /* does this propagate */
	  if (!remove_dl_from_dl_list(CTXTc DelayList, Delay(answer_leaf)))
	    simplify_pos_unsupported(CTXTc (NODEptr) answer_leaf);
	}
	else answer_supported = TRUE;
      }
      DelayList = dl_next(DelayList);
    }
    if (IsValidNode(answer_leaf)) return TRUE;
    else return FALSE;
  }
}


void resetStack() {
}

// this code is obsolete (dsw 8/2/15)
void answer_completion(CTXTdeclc CPtr cs_ptr) {
  VariantSF compl_subg;
  CPtr ComplStkFrame = cs_ptr; 
  ALNptr answerPtr;
  BTNptr answer_leaf;

  printf("calling answer completion\n");

  /* For each subgoal S in SCC */
  while (ComplStkFrame >= openreg) {
    compl_subg = compl_subgoal_ptr(ComplStkFrame);
    answerPtr = subg_ans_list_ptr(compl_subg);
    while (answerPtr ) {
      answer_leaf = ALN_Answer(answerPtr);
      checkSupportedAnswer(answer_leaf);
      resetStack();
      answerPtr = ALN_Next(answerPtr);
    }

    ComplStkFrame = next_compl_frame(ComplStkFrame);
  }

}
#endif

/*****************************************************************************/
static unsigned int hashid(void *ky)
{
  return (unsigned int)(UInteger)ky;
}

static int equalkeys(void *k1, void *k2)
{
    return (0 == memcmp(k1,k2,sizeof(KEY)));
}

static Cell cell_array1[500];

int return_ans_depends_scc_list(CTXTdeclc SCCNode * nodes, Integer num_nodes){
 
  VariantSF subgoal;
  TIFptr tif;
  BTNptr ansLeaf;
  int cur_node = 0,arity, j;
  Psc psc;
  CPtr oldhreg = NULL;

  reg[4] = makelist(hreg);
  new_heap_free(hreg);  new_heap_free(hreg);
  do {
    subgoal = asi_subgoal((ASI)Child((BTNptr) nodes[cur_node].node));
    ansLeaf = (BTNptr) nodes[cur_node].node;
    tif = (TIFptr) subgoal->tif_ptr;
    psc = TIF_PSC(tif);
    arity = get_arity(psc);
    //    printf("subgoal %p, %s/%d\n",subgoal,get_name(psc),arity);
    check_glstack_overflow(4,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 
    //    check_glstack_overflow(4,pcreg,2+arity*200); // don't know how much for build_subgoal_args..
    oldhreg=hreg-2;                          // ptr to car
    sreg = hreg;
    follow(oldhreg++) = makecs(sreg);      
    new_heap_functor(sreg,get_ret_psc(3)); //  car pts to ret/2  psc
    hreg += 4;                             //  hreg pts past ret/2
    sreg = hreg;
    follow(hreg-1) = makeint(nodes[cur_node].component);  // arg 3 of ret/2 pts to component
    follow(hreg-2) = makeint(ansLeaf);  // arg 2 of ret/2 pts to answer leaf
    if(arity>0){
      follow(hreg-3) = makecs(sreg);         
      new_heap_functor(sreg, psc);           //  arg 1 of ret/2 pts to goal psc
      hreg += arity + 1;
      for (j = 1; j <= arity; j++) {
	new_heap_free(sreg);
	cell_array1[arity-j] = cell(sreg-1);
      }
      load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
      //      build_subgoal_args(arity,cell_array1,subgoal);		
    }
    else {
      follow(hreg-3) = makestring(get_name(psc));
      //      follow(hreg-2) = makeint(0);
    }
    follow(oldhreg) = makelist(hreg);        // cdr points to next car
    new_heap_free(hreg); new_heap_free(hreg);
    cur_node++;
  } while (cur_node  < num_nodes);
  follow(oldhreg) = makenil;                // cdr points to next car
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
}

extern void *  search_some(struct hashtable*, void *);
extern int insert_some(struct hashtable*, void *, void *);

void xsb_compute_ans_depends_scc(SCCNode * nodes,int * dfn_stack,int node_from, 
				 int * dfn_top,struct hashtable* hasht,int * dfn,
				 int * component ) {
  prolog_int node_to; int j;
  ASI asi; BTNptr ansLeaf;
  DL current_dl;     DE current_de;

  //  printf("xsb_compute_scc for %d %p", node_from,nodes[node_from].node);
  //  printf(" %s/%d dfn %d dfn_top %d \n",
  //	 get_name(TIF_PSC(subg_tif_ptr(asi_subgoal((ASI)Child((BTNptr)(nodes[node_from].node)))))),
  //	 get_arity(TIF_PSC(subg_tif_ptr(asi_subgoal((ASI)Child((BTNptr)(nodes[node_from].node)))))),
  //	 *dfn,*dfn_top);
  nodes[node_from].low = nodes[node_from].dfn = (*dfn)++;
  dfn_stack[*dfn_top] = node_from;
  nodes[node_from].stack = *dfn_top;
  (*dfn_top)++;
  asi = (ASI) Child( (BTNptr) nodes[node_from].node);
  current_dl = asi_dl_list(asi);
  while (current_dl != NULL) {
    //    printf("DL: %p\n",current_dl);
    current_de = dl_de_list(current_dl);
    while (current_de != NULL) {
      if (de_ans_subst(current_de)) {
	//	printf("  DE: %p SF %p Ans %p %ld\n",current_de,de_subgoal(current_de),
	//               de_ans_subst(current_de),(long) de_ans_subst(current_de));
	ansLeaf = de_ans_subst(current_de);
      }
      else {
	//	printf("  DE: %p SF %p Ans %p %ld\n",current_de,de_subgoal(current_de),
	//	       Child(subg_ans_root_ptr(de_subgoal(current_de))),
	//     (long) Child(subg_ans_root_ptr(de_subgoal(current_de))));
	ansLeaf = Child(subg_ans_root_ptr(de_subgoal(current_de)));
      }
      node_to = (prolog_int) search_some(hasht, (void *)ansLeaf);
      //      printf("edge from %p to %p (%d)\n",(void *)nodes[node_from].node,sf,node_to);
      if (nodes[node_to].dfn == 0) {
	xsb_compute_ans_depends_scc(nodes,dfn_stack,(int)node_to, dfn_top,hasht,dfn,component );
	if (nodes[node_to].low < nodes[node_from].low) 
	  nodes[node_from].low = nodes[node_to].low;
	}	  
	else if (nodes[node_to].dfn < nodes[node_from].dfn  && nodes[node_to].component == 0) {
	  if (nodes[node_to].low < nodes[node_from].low) { nodes[node_from].low = nodes[node_to].low; }
	}
      current_de = de_next(current_de);
    }
    //    printf("nodes[%d] low %d dfn %d\n",node_from,nodes[node_from].low, nodes[node_from].dfn);
    if (nodes[node_from].low == nodes[node_from].dfn) {
      for (j = (*dfn_top)-1 ; j >= nodes[node_from].stack ; j--) {
	//	printf(" pop %d and assign %d\n",j,*component);
	nodes[dfn_stack[j]].component = *component;
      }
      (*component)++;       *dfn_top = j+1;
    }
    current_dl = dl_next(current_dl);
  }
}

int  get_residual_sccs(CTXTdeclc Cell listterm) {
  Cell orig_listterm, node, intterm;
    UInteger node_num=0;
    int i = 0, dfn, component = 1;     int * dfn_stack; int dfn_top = 0, ret;
    SCCNode * nodes;
    struct hashtable* hasht; 

    XSB_Deref(listterm);
    orig_listterm = listterm;
    hasht = create_hashtable1(HASH_TABLE_SIZE, hashid, equalkeys);
    //    printf("listptr %p @%p\n",listptr,(CPtr) int_val(*listptr));
    intterm = get_list_head(listterm);
    XSB_Deref(intterm);
    insert_some(hasht,(void *) oint_val(intterm),(void *) node_num);
    node_num++; 

    listterm = get_list_tail(listterm);
    XSB_Deref(listterm);
    while (!isnil(listterm)) {
      intterm = get_list_head(listterm);
      XSB_Deref(intterm);
      node = oint_val(intterm);
      if (NULL == search_some(hasht, (void *)node)) {
	insert_some(hasht,(void *)node,(void *)node_num);
	node_num++;
      }
      listterm = get_list_tail(listterm);
      XSB_Deref(listterm);
    }
    nodes = (SCCNode *) mem_calloc(node_num, sizeof(SCCNode),OTHER_SPACE); 
    dfn_stack = (int *) mem_alloc(node_num*sizeof(int),OTHER_SPACE); 
    listterm = orig_listterm;; 
    //    printf("listptr %p @%p\n",listptr,(void *)int_val(*(listptr)));
    intterm = get_list_head(listterm);
    XSB_Deref(intterm);
    nodes[0].node = (CPtr) oint_val(intterm);
    listterm = get_list_tail(listterm);
    XSB_Deref(listterm);
    i = 1;
    while (!isnil(listterm)) {
      intterm = get_list_head(listterm);
      XSB_Deref(intterm);
      node = oint_val(intterm);
      nodes[i].node = (CPtr) node;
      listterm = get_list_tail(listterm);
      XSB_Deref(listterm);
      i++;
    }
    //     struct hashtable_itr *itr = hashtable1_iterator(hasht);       
    //       do {
	 //         printf("k %p val %p\n",hashtable1_iterator_key(itr),
         //     hashtable1_iterator_value(itr));
    //        } while (hashtable1_iterator_advance(itr));

    listterm = orig_listterm;
    //       printf("2: k %p v %p\n",(void *) int_val(*listptr),
    //       search_some(hasht,(void *) int_val(*listptr)));
        while (!isnil(listterm)) {
	  intterm = get_list_head(listterm);
	  node = oint_val(intterm);
	 //         printf("2: k %p v %p\n",(CPtr) node,search_some(hasht,(void *) node));
	  listterm = get_list_tail(listterm);
	  XSB_Deref(listterm);
       }
    dfn = 1;
    for (i = 0; i < (Integer)node_num; i++) {
      if (nodes[i].dfn == 0) 
	xsb_compute_ans_depends_scc(nodes,dfn_stack,i,&dfn_top,hasht,&dfn,&component);
      //      printf("++component for node %d is %d (high %d)\n",i,nodes[i].component,component);
    }
    ret = return_ans_depends_scc_list(CTXTc  nodes, node_num);
    hashtable1_destroy(hasht,0);
    mem_dealloc(nodes,node_num*sizeof(SCCNode),OTHER_SPACE); 
    mem_dealloc(dfn_stack,node_num*sizeof(int),OTHER_SPACE); 
    return ret;
}

 int pred_type, goal_type, answer_set_status;
 VariantSF goalSF = NULL, subsumerSF;
 Cell goalTerm;

FILE * fview_ptr = NULL;

//----------------------------------------------------------------------
extern CPtr trie_asserted_clref(CPtr);
extern PrRef get_prref(CTXTdeclc Psc psc);

#ifndef MULTI_THREAD
forest_log_buffer_struct fl_buffer_1;
forest_log_buffer_struct fl_buffer_2;
forest_log_buffer_struct fl_buffer_3;

forestLogBuffer forest_log_buffer_1;
forestLogBuffer forest_log_buffer_2;
forestLogBuffer forest_log_buffer_3;
#endif

Cell list_of_answers_from_answer_list(CTXTdeclc VariantSF sf,int as_length,int attv_length,ALNptr ALNlist) {
  //  BTNptr leaf;
  Cell listHead;   CPtr argvec1,oldhreg=NULL;
  int i, isNew;   Psc ans_desig;
  ALNptr ansPtr;
  VariantSF undef_sf;
    Pair undefPair;				      

  //  print_subgoal(stdout, sf); printf("\n"); 

    //  leaf = subg_leaf_ptr(sf);
  ans_desig = get_ret_psc(2);
  ansPtr = ALNlist;
  undefPair = insert("brat_undefined", 0, pair_psc(insert_module(0,"xsbbrat")), &isNew); 
  //  printf("tip %p\n",get_tip(CTXTc pair_psc(undefPair)));
  undef_sf = TIF_Subgoals(get_tip(CTXTc pair_psc(undefPair)));

  listHead = makelist(hreg);
  while (ansPtr != 0) {
    new_heap_free(hreg);  new_heap_free(hreg);
    oldhreg=hreg-2;                          // ptr to car
    new_heap_functor(hreg,ans_desig); 
    follow(oldhreg) = makecs(oldhreg+2);
    follow(hreg) = makecs(hreg+2);
    hreg++; 
    //    new_heap_free(hreg);
    if (is_unconditional_answer(ALN_Answer(ansPtr))) {
      //      printf("is unconditional\n");
      new_heap_int(hreg,TRUE);
    }
    else {
      //      printf("conditional answer\n");
      new_heap_int(hreg,undef_sf);
    }
    new_heap_functor(hreg, get_ret_psc(as_length));
    argvec1 = hreg;
    for (i=0; i<as_length; i++)
      bld_free(hreg+i);
    hreg = hreg + as_length;
    //    printf("ALN_answer %p\n",ALN_Answer(ansPtr));
    load_solution_trie_notrail(CTXTc as_length, attv_length, argvec1, ALN_Answer(ansPtr));  // Need to handle attvs
    follow((oldhreg+1)) = makelist(hreg);

    ansPtr = ALN_Next(ansPtr);
  }

  follow(oldhreg+1) = makenil;
  //  printf("---listhead--- ");  printterm(stddbg,listHead,80);printf("\n");
  return listHead;
}

int hello_world(void) {
  printf("hello world!!!\n");
  return TRUE;
}

//----------------------------------------------------------------------

//----------------------------------------------------------------------

int table_inspection_function( CTXTdecl ) {
  switch (ptoc_int(CTXTc 1)) {

#ifdef UNDEFINED
  case FIND_COMPONENTS: {
//    int new;    Psc sccpsc;
    Cell temp;

    dfn = 0;
    component_stack_top = 0;
    done_answer_stack_top = 0;
    table_component_check(CTXTc (NODEptr) ptoc_int(CTXTc 2));
    print_done_answer_stack(CTXT);
    alt_print_done_answer_stack(CTXT);
    unfounded_component(CTXT);
    printf("done w. unfounded component\n");
    //    print_done_answer_stack(CTXT);

    bind_list(&temp,hreg);                 //[ 
    bld_list(hreg,hreg+2);    hreg++;      // [
    new_heap_nil(hreg);                    //]
    bld_int(hreg,0);  hreg++;              //   0
    bld_list(hreg,hreg+2); hreg++;         //    ,
    bld_list(hreg,hreg+2);    hreg++;      //     [
    bld_list(hreg,hreg+2);    hreg++;      //  
    new_heap_nil(hreg);                    // ]
    bld_int(hreg,1);  hreg++;              //      1
    bld_list(hreg,hreg+2);    hreg++;      //       ,
    bld_list(hreg,hreg+2);    hreg++;      //         [
    bld_int(hreg,2);  hreg++;              //          2
    bld_list(hreg,hreg+2);    hreg++;
    bld_list(hreg,hreg+2);    hreg++;
    bld_int(hreg,3);  hreg++;              //          2
    bld_list(hreg,hreg+2);    hreg++;
    bld_list(hreg,hreg+2);    hreg++;
    bld_int(hreg,4);  hreg++;              //          2
    new_heap_nil(hreg);
    //    ctop_tag(CTXTc 3, temp);

    break;
  }
#endif

  case FIND_FORWARD_DEPENDENCIES: {
  DL delayList;
  DE delayElement;
  BTNptr as_leaf, new_answer;
  struct answer_dfn stack_node;

  printf("find foward dependencies \n");
  done_answer_stack_top = 0; dfn = 0;
  as_leaf = (NODEptr) ptoc_int(CTXTc 2);
  if (is_conditional_answer(as_leaf)) {
    push_comp_node_1(as_leaf);
    while (component_stack_top != 0) {
      //      print_comp_stack(CTXT);
      pop_comp_node(stack_node);
      as_leaf = stack_node.answer;
      push_done_node((component_stack_top),component_stack[component_stack_top].dfn);
      // print_subgoal(CTXTc stddbg, asi_subgoal((ASI) Child(as_leaf)));printf("\n");
      delayList = asi_dl_list((ASI) Child(as_leaf));
      while (delayList) {
	delayElement = dl_de_list(delayList);
	while (delayElement) {
	  if (de_ans_subst(delayElement)) {
	    if (!VISITED_ANSWER(de_ans_subst(delayElement)))
	      push_comp_node_1(de_ans_subst(delayElement));
	  } else {
	    new_answer = traverse_subgoal(de_subgoal(delayElement));
	    if (!VISITED_ANSWER(new_answer)) 
	      push_comp_node_1(new_answer);
	    }
	    delayElement = de_next(delayElement);
	  }
	  delayList = de_next(delayList);
	}
      }
    printf("component stack %p\n",component_stack);
    deallocate_comp_stack;
    print_done_answer_stack(CTXT);
    // Don't deallocte done stack until done with its information.
    reset_done_answer_stack();
    return 0;
  }
  else  printf("found unconditional answer %p\n",as_leaf);
    break;
  }

  case GET_CALLSTO_NUMBER: {
    VariantSF subgoal_frame;
    subgoal_frame = (VariantSF) ptoc_int(CTXTc 2);
    ctop_int(CTXTc 3, subg_callsto_number(subgoal_frame));
    break;
  }

  case GET_ANSWER_NUMBER: {
    VariantSF subgoal_frame;
    subgoal_frame = (VariantSF) ptoc_int(CTXTc 2);
    ctop_int(CTXTc 3, subg_ans_ctr(subgoal_frame));
    break;
  }

    /* TLS: for answer subsumption, can simply use SF */
  case EARLY_COMPLETE_ON_NTH: {
    VariantSF subgoal;
    Integer breg_offset;
    CPtr tcp;

    breg_offset = ptoc_int(CTXTc 2);
    tcp = (CPtr)((Integer)(tcpstack.high) - breg_offset);
    subgoal = (VariantSF)(tcp_subgoal_ptr(tcp));
    //    subgoal = (VariantSF) ptoc_int(CTXTc 2);
    //    printf("san %d\n",get_subgoal_answer_number(subgoal));
    if ( subg_ans_ctr(subgoal) +1 >= ptoc_int(CTXTc 3) &&
	 !subg_is_completed(subgoal)) {
       schedule_ec(subgoal); 
       //       printf("scheduling ec\n");
    }
    break;
  }

    /* TLS: for answer subsumption, can simply use SF */
  case EARLY_COMPLETE: {
    VariantSF subgoal;
    Integer breg_offset;
    CPtr tcp;

    //    printf("uncond ec\n");
    breg_offset = ptoc_int(CTXTc 2);
    tcp = (CPtr)((Integer)(tcpstack.high) - breg_offset);
    subgoal = (VariantSF)(tcp_subgoal_ptr(tcp));
    if (!subg_is_completed(subgoal)) {
      schedule_ec(subgoal); 
       //       printf("scheduling ec\n");
    }
    break;
  }

  case FIND_ANSWERS: {
    return find_pred_backward_dependencies(CTXTc get_tip(CTXTc term_psc(ptoc_tag(CTXTc 2))));
  }

/********************************************************************

The following builtin serves as an analog to SLG_NOT for call
subsumption when we the negated subgoal Subgoal is not a producer.
Because there is no direct connection between a consumer for Subgoal
(which is ground) and its answer, we either start here with the answer
leaf pointer as produced by get_returns/3, or with a null leaf pointer
(e.g. in case the function is invoked by delaying after
is_incomplete).  Obtaining the leaf pointer should probably be moved
into this function, but for now we start with an indication of whether
a return has been found, and handle our cases separately.  

A separate case is that we may not have a consumer subgoal frame if
Subgoal is subsumed by Producer and Producer is completed (the call
subsumption algorithm does not create a subgoal frame in this case).
So we have to handle that situation also.  For now, if we dont have a
consumer SF, I add tnot(Producer) to the delay list -- although
creating a new consumer SF so that we can add tnot(Consumer) might be
better.
*********************************************************************/

case CALL_SUBS_SLG_NOT: {

  VariantSF producerSF, consumerSF;
  BTNptr answerLeaf;
  int hasReturn;

  producerSF = (VariantSF) ptoc_int(CTXTc 2);
  hasReturn =  (int)ptoc_int(CTXTc 4);
  consumerSF = (VariantSF) ptoc_int(CTXTc 5);

  if (hasReturn) {
    answerLeaf = (BTNptr) ptoc_int(CTXTc 3);
    /* Return with unconditional answer: fail */
    if ( is_unconditional_answer(answerLeaf) ) {
      //      fprintf(stddbg,"failing : ");print_subgoal(stddbg,consumerSF),printf("\n");
      return FALSE;
    }
    else {
    /* Return with conditional answer - propagate delay */
      if (IsNonNULL(consumerSF)) delay_negatively(consumerSF)
      else delay_negatively(producerSF);
      return TRUE;
    }
  } else { /* has no return: we havent resumed to delay */
    if   (is_completed(producerSF) || neg_delay == FALSE) {
      //      fprintf(stddbg,"succeeding: ");print_subgoal(stddbg,consumerSF),printf("\n");
      return TRUE;
    }
    else { /* has no return: but function was invoked for delaying */
      //     fprintf(stddbg,"delaying (wo): ");print_subgoal(stddbg,consumerSF),printf("\n");
      if (IsNonNULL(consumerSF)) delay_negatively(consumerSF)
	else delay_negatively(producerSF);
      return TRUE;
    }
  }
  break;
    }

  case GET_PRED_CALLTRIE_PTR: {
    
    Psc psc = (Psc)ptoc_int(CTXTc 2);
    if (get_tip(CTXTc psc))
      ctop_int(CTXTc 3,(Integer)TIF_CallTrie(get_tip(CTXTc psc)));
    else 
      ctop_int(CTXTc 3,0);
    break;
  }
 
  case START_FOREST_VIEW: {
    char *filename = ptoc_string(CTXTc 2);
    if (!(strcmp(filename,"userout")))
      fview_ptr = stdout; 
    else fview_ptr = fopen(filename,"w");
    break;
  }

  case STOP_FOREST_VIEW: {
    if (fview_ptr != stdout && fview_ptr != NULL) {
      fflush(fview_ptr);
      fclose(fview_ptr);
    }
    fview_ptr = NULL;
    break;
  }
    
  case FLUSH_FOREST_VIEW: {
    if (fview_ptr != stdout && fview_ptr != NULL) {
      fflush(fview_ptr);
    }
    break;
  }

  case TNOT_SETUP: {
    Cell goalTerm;
    TableStatusFrame TSF;

    goalTerm = ptoc_tag(CTXTc 2);
    if ( isref(goalTerm) ) {
      xsb_instantiation_error(CTXTc "table_status/4",1);
      break;
    }

    table_status(CTXTc goalTerm, &TSF);

    if (TableStatusFrame_pred_type(TSF) < 0) {
      sprintCyclicTerm(CTXTc forest_log_buffer_1, ptoc_tag(CTXTc 2));
      xsb_abort("Illegal (non-tabled?) subgoal in tnot/1: %s\n", forest_log_buffer_1->fl_buffer);   
    }	

    if (TableStatusFrame_pred_type(TSF) == VARIANT_EVAL_METHOD 
	&& TableStatusFrame_answer_set_status(TSF) < 0) {
      vcs_tnot_call = 1;
    }

    ctop_int(CTXTc 3,TableStatusFrame_pred_type(TSF));
    ctop_int(CTXTc 4,TableStatusFrame_goal_type(TSF));
    ctop_int(CTXTc 5,TableStatusFrame_answer_set_status(TSF));
    ctop_addr(6, TableStatusFrame_subgoal(TSF));
    ctop_addr(7, ptcpreg);
    break;
  } 

  case GET_CURRENT_SCC: {
    VariantSF subgoal_frame;
    subgoal_frame = (VariantSF) ptoc_int(CTXTc 2);
    if (subg_is_completed(subgoal_frame) || subg_is_ec_scheduled(subgoal_frame)) return FALSE;
    ctop_int(CTXTc 3, compl_level(subg_compl_stack_ptr(subgoal_frame)));
    break;
  }

  case PRINT_COMPLETION_STACK: {
    int stream;

    if (0 > (stream = (int)ptoc_int(CTXTc 2)))
      print_completion_stack(CTXTc open_files[(int) pflags[CURRENT_OUTPUT]].file_ptr);
    else 
      print_completion_stack(CTXTc open_files[stream].file_ptr);
    break;
  }

  case PRINT_CYCLIC_TERM: {
    printCyclicTerm(CTXTc ptoc_tag(CTXTc 2));
    break;		    
  }

  case MARK_TERM_CYCLIC_1: {
    mark_cyclic(CTXTc ptoc_tag(CTXTc 2));
    return TRUE;
  }

  case GET_VISITORS_NUMBER: {
    VariantSF subgoal_frame;
    subgoal_frame = (VariantSF) ptoc_int(CTXTc 2);
    ctop_int(CTXTc 3, subg_visitors(subgoal_frame));
    break;
  }

  case GET_SCC_DUMPFILE: {
  Cell addr = cell(reg+2);
  XSB_Deref(addr);
  if (!isref(addr))  xsb_instantiation_error(CTXTc "get_scc_dumpfile/1",1);
  else {
    if (!abort_file_gl[0]) return FALSE; 
    ctop_string(CTXTc 2,abort_file_gl);
  }
    break;
  }


  case CHECK_VARIANT: {
    //    CPtr addr, val;
    Psc psc;
    Cell term;
    int dont_cares;
    PrRef prref;
    BTNptr Paren, leaf;
    term = ptoc_tag(CTXTc 2);
    dont_cares = (int)ptoc_int(CTXTc 3);
    if (isconstr(term)) {
      psc = get_str_psc(term);
    }
    else {
      xsb_type_error(CTXTc "callable structure",term,"check_variant/[1,2]",1);
      return 0; /* purely to quiet compiler */
    }
    prref = get_prref(CTXTc psc);
    // prref points to a clref which *then* points to the root of the trie.
    Paren = (BTNptr)*(trie_asserted_clref((CPtr) prref) + 3);
    //    printf("new psc %p, prref %p parent %p\n",psc,get_prref(CTXTc psc),Paren);
    //    val = (CPtr) *(reg+2);
    //    XSB_Deref_with_address(val,addr);
    leaf = variant_trie_lookup(CTXTc Paren, get_arity(psc)-dont_cares, 
				      clref_val(term)+1, NULL);
    return (leaf != NULL);
  }

  case IS_CONDITIONAL_ANSWER: {
    NODEptr as_leaf;
    as_leaf = (NODEptr) ptoc_int(CTXTc 2);
    if (is_conditional_answer(as_leaf))
      ctop_int(CTXTc 3,TRUE);
    else ctop_int(CTXTc 3,FALSE);
    break;
  }

  case SET_TIF_PROPERTY: { 
      /* reg 2=psc, reg 3 = property to set, reg 4 = val */
    Psc psc;
    TIFptr tip;

    Cell term = ptoc_tag(CTXTc 2);
    int property = (int)ptoc_int(CTXTc 3);
    int val = (int)ptoc_int(CTXTc 4);

    if ( isref(term) ) {
      xsb_instantiation_error(CTXTc "set_tif_property/3",1);
      break;
    }
    psc = term_psc(term);
    if ( IsNULL(psc) ) {
      xsb_type_error(CTXTc "predicate_indicator",term,"set_tabled_eval/2",1);
      break;
    }
    if ((tip = get_tip(CTXTc psc)) ) {
      if (property == SUBGOAL_DEPTH) 
	TIF_SubgoalDepth(tip) = val;
      else if (property == ANSWER_DEPTH) 
	TIF_AnswerDepth(tip) = val;
      else if (property == INTERNING_GROUND)
	TIF_Interning(tip) = val;
    }
    else  /* cant find tip */
      xsb_permission_error(CTXTc "set property","tif",term,
			   "set_tif_property",3);
    break;
  }

  case GET_TIF_PROPERTY: { 
      /* reg 2=psc, reg 3 = property to get, reg 4 = val */
    Psc psc;
    TIFptr tip;

    Cell term = ptoc_tag(CTXTc 2);
    int property = (int)ptoc_int(CTXTc 3);

    if ( isref(term) ) {
      xsb_instantiation_error(CTXTc "get_tif_property/3",1);
      break;
    }
    psc = term_psc(term);
    if ( IsNULL(psc) ) {
      xsb_type_error(CTXTc "predicate_indicator",term,"set_tabled_eval/2",1);
      break;
    }
    if ((tip = get_tip(CTXTc psc)) ) {
      if (property == SUBGOAL_DEPTH) 
	ctop_int(CTXTc 4,TIF_SubgoalDepth(tip));
      else if (property == ANSWER_DEPTH) 
	ctop_int(CTXTc 4,TIF_AnswerDepth(tip));
      else if (property == INTERNING_GROUND)
	ctop_int(CTXTc 4,TIF_Interning(tip));
    }
    else  /* cant find tip */
      return FALSE;

    break;
  }

  case IMMED_ANS_DEPENDS_PTRLIST: {
    ASI asi;
    DL current_dl = NULL;
    DE current_de;
    CPtr oldhreg = NULL;
    int  count = 0;

    reg[4] = makelist(hreg);
    new_heap_free(hreg);  new_heap_free(hreg);
    asi = (ASI) Child( (BTNptr) ptoc_int(CTXTc 2));
    if (asi) current_dl = asi_dl_list(asi);
    while (current_dl != NULL) {
      //      printf("DL: %p\n",current_dl);
      current_de = dl_de_list(current_dl);
      while (current_de != NULL) {
	//	print_subgoal(stddbg, de_subgoal(current_de));
	count++;
	check_glstack_overflow(4,pcreg,2); 
	oldhreg = hreg-2;
	sreg = hreg;
	follow(oldhreg++) = makecs(sreg);      
	new_heap_functor(sreg,get_ret_psc(2)); //  car pts to ret/2  psc
	hreg += 3;
	if (de_ans_subst(current_de)) {
	  //	  printf("  DE: %p SF %p Ans %p %ld\n",current_de,de_subgoal(current_de),
	  //   de_ans_subst(current_de),(long) de_ans_subst(current_de));
	  follow(hreg-2) = makeint(de_ans_subst(current_de));
	  follow(hreg-1) = makeint(IS_ASI);
	}
	else {
	  //	  printf("  DE: %p SF %p Ans %p %ld\n",current_de,de_subgoal(current_de),
	  //		 Child(subg_ans_root_ptr(de_subgoal(current_de))),
	  // (long) Child(subg_ans_root_ptr(de_subgoal(current_de))));
	  follow(hreg-2) = makeint(Child(subg_ans_root_ptr(de_subgoal(current_de))));
	  follow(hreg-1) = makeint(IS_SUBGOAL_FRAME);
	}
	follow(oldhreg) = makelist(hreg);
	new_heap_free(hreg);	new_heap_free(hreg);
	current_de = de_next(current_de);
      }
      current_dl = dl_next(current_dl);
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[3] = makenil;
  return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
  break;
  }
  
  case GET_RESIDUAL_SCCS: {

    return get_residual_sccs(CTXTc ptoc_tag(CTXTc 2));
  }

  case SET_FOREST_LOGGING_FOR_PRED: {
    //    Psc psc =  term_psc((Cell)(ptoc_tag(CTXTc 2)));
    Psc psc =  (Psc)(ptoc_int(CTXTc 2));
    if (get_tabled(psc)) {
      TIF_SkipForestLog(get_tip(CTXTc psc)) = (ptoc_int(CTXTc 3) & 1);
    } else
      xsb_permission_error(CTXTc "set forest logging","non-tabled predicate",ptoc_tag(CTXTc 2),"set_forest_logging_for_pred",1);
    return TRUE;
  } 
  case ABOLISH_NONINCREMENTAL_TABLES: {

    return abolish_nonincremental_tables(CTXTc (int)ptoc_int(CTXTc 2));
  }

  case INCOMPLETE_SUBGOAL_INFO: {
    CPtr newcsf;
    CPtr csf = (CPtr) ptoc_int(CTXTc 2);
    if (openreg == COMPLSTACKBOTTOM) { 
      ctop_int(CTXTc 5,(Integer)NULL);
      return TRUE;
    }
    if (csf == NULL) csf = openreg;
    newcsf = prev_compl_frame(csf);
    if (newcsf >= COMPLSTACKBOTTOM) newcsf = NULL;
    ctop_int(CTXTc 3,(Integer)newcsf);
    if (csf != NULL) {
      ctop_int(CTXTc 4,compl_level(csf));
      ctop_int(CTXTc 5,(Integer)compl_subgoal_ptr(csf));
      ctop_int(CTXTc 6,subg_callsto_number(compl_subgoal_ptr(csf)));
      ctop_int(CTXTc 7,subg_ans_ctr(compl_subgoal_ptr(csf)));
      ctop_int(CTXTc 8,subg_negative_initial_call(compl_subgoal_ptr(csf)));
      ctop_int(CTXTc 9,(Integer)tcp_ptcp(subg_cp_ptr(compl_subgoal_ptr(csf))));
    }
    return TRUE;
	     
  }
  case GET_CALL_FROM_SF:  {
    TIFptr tif; Psc psc; int arity,j;
    VariantSF subgoal = (VariantSF) ptoc_int(CTXTc 2);
    CPtr  call_return = (CPtr) ptoc_tag(CTXTc 3);

    tif = (TIFptr) subgoal->tif_ptr;
    psc = TIF_PSC(tif);
    arity = get_arity(psc);
    check_glstack_overflow(4,pcreg,2+(sizeof(Cell)*trie_path_heap_size(CTXTc subg_leaf_ptr(subgoal)))); 
    if(arity>0){
      sreg = hreg;
      follow(call_return) = makecs(sreg);
      hreg += arity + 1;
      new_heap_functor(sreg, psc);
      for (j = 1; j <= arity; j++) {
	new_heap_free(sreg);
	cell_array1[arity-j] = cell(sreg-1);
      }
      load_solution_trie_no_heapcheck(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(subgoal));
	    //    build_subgoal_args(arity,cell_array1,subgoal);		
    } else{
         follow(call_return) = makestring(get_name(psc));
    }
    return TRUE;
  }

  case GET_POS_AFFECTS: {
  CPtr oldhreg = NULL;
  int count = 0;
    VariantSF subgoal = (VariantSF) ptoc_int(CTXTc 2);
    //    printf("sf %p: ",subgoal);
    CPtr consumer_cpf = subg_pos_cons(subgoal);
    reg[4] = makelist(hreg);
    new_heap_free(hreg);
    new_heap_free(hreg);
    //    printf("initial negative call %d: %p\n",subg_negative_initial_call(subgoal),tcp_ptcp(subg_cp_ptr(subgoal)));
 
    while (consumer_cpf != NULL) {
      count++;
      check_glstack_overflow(4,pcreg,2); 
      oldhreg=hreg-2;
      follow(oldhreg++) = makeint(nlcp_ptcp(consumer_cpf));
      follow(oldhreg) = makelist(hreg);
      new_heap_free(hreg);
      new_heap_free(hreg);
      consumer_cpf = nlcp_prevlookup(consumer_cpf);
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
    return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
  }
  case GET_NEG_AFFECTS: {
  CPtr oldhreg = NULL;
  int count = 0;
    VariantSF subgoal = (VariantSF) ptoc_int(CTXTc 2);
    CPtr consumer_cpf = subg_compl_susp_ptr(subgoal);
    reg[4] = makelist(hreg);
    new_heap_free(hreg);
    new_heap_free(hreg);
  
    while (consumer_cpf != NULL) {
      count++;
      check_glstack_overflow(4,pcreg,2); 
      oldhreg=hreg-2;
      follow(oldhreg++) = makeint(csf_ptcp(consumer_cpf));
      follow(oldhreg) = makelist(hreg);
      new_heap_free(hreg);
      new_heap_free(hreg);
      consumer_cpf = csf_prevcsf(consumer_cpf);
    }
    if (count>0)
      follow(oldhreg) = makenil;
    else
      reg[4] = makenil;
    return unify(CTXTc reg_term(CTXTc 3),reg_term(CTXTc 4));
  }
  case PSC_IMMUTABLE: {
    Psc psc = (Psc)ptoc_int(CTXTc 2);
    ctop_int(CTXTc 3, (Integer)get_immutable(psc));
    return TRUE;
  }
  case PRINT_LS: print_ls(CTXTc 1) ; return TRUE ;
  case PRINT_TR: print_tr(CTXTc 1) ; return TRUE ;
  case PRINT_HEAP: print_heap(CTXTc 0,2000,1) ; return TRUE ;
  case PRINT_CP: alt_print_cp(CTXTc ptoc_string(CTXTc 1)) ; return TRUE ;
  case PRINT_REGS: print_regs(CTXTc 10,1) ; return TRUE ;
  case PRINT_ALL_STACKS: print_all_stacks(CTXTc 10) ; return TRUE ;

  case TEMP_FUNCTION: {
    //int (*pfunc)() = hello_world;
    //(void)(*pfunc)(void);
    //insert_cpred("hello_world",0,pfunc );
    return TRUE;
  }
  } /* switch */
  return TRUE;
}

//---------------------------------------------------------------------------------------------------
    //  case TEMP_FUNCTION: {
    //    /* Input : Incall ; Output : Outcall */
    //    VariantSF sf;
    //    BTNptr leaf, root;
    //    Cell callTermIn,ret, callTermOut; 
    //    Psc psc; int i, arity;
    //    CPtr argvec1;
    //    ALNptr ansPtr;
    //
    //    callTermIn = ptoc_tag(CTXTc 2);
    //    sf = get_call(CTXTc callTermIn, &ret); 
    //    if ( IsNonNULL(sf) ) {
    //      root = TIF_CallTrie(subg_tif_ptr(sf));
    //      psc = term_psc(callTermIn);
    //      arity = get_arity(psc);
    //      callTermOut = makecs(hreg); 
    //      bld_functor(hreg++, psc);
    //      argvec1 = hreg;
    //      for (i=0; i<arity; i++)
    //	bld_free(hreg+i);
    //      hreg = hreg + arity;
    //
    //      leaf = subg_leaf_ptr(sf);
    //      load_subgoal_trie(arity, 0, argvec1, leaf);  // Need to handle attvs
    //      ansPtr = ALN_Next(subg_ans_list_ptr(sf));
    //      load_subgoal_trie(2, 0, (clref_val(ret) +1), ALN_Answer(ansPtr));  // Need to handle attvs
    //
    //      //      listHead = list_of_answers_from_answer_list(sf);
    //
      //      printterm(stddbg,(Cell) clref_val(listHead),8);printf("\n");	
      //      ctop_tag(CTXTc 3, callTermOut);
      //      ctop_tag(CTXTc 4, ret);
      //      ctop_tag(CTXTc 3, listHead);
      //      printf("lh %x clr %p *clr %x\n",listHead,clref_val(listHead),clref_val(*clref_val(listHead)));
      //      printterm(stddbg,clref_val(listHead)+3,8);printf("\n");
      //      for (i=1; i <= arity; i++) {
	//	printf("lh %x clr %p *clr %x\n",listHead,clref_val(listHead),((CPtr) *clref_val(listHead)) +i);      
      //	printterm(stddbg,(Cell) (clref_val(*clref_val(listHead))+i),8);printf("\n");
      //	ctop_tag(CTXTc 3+i, clref_val(*clref_val(listHead))+i);
      //      }
    //  ]

    //    return TRUE; 

    //  }

    //  case TEMP_FUNCTION_2: {
    //    /* Input : Incall ; Output : Outcall */
    //    VariantSF sf;
    //    Cell callTermIn;
    //
    //    callTermIn = ptoc_tag(CTXTc 2);
    //    sf = get_call(CTXTc callTermIn,NULL); 
    //    if ( IsNonNULL(sf) ) {
    //      find_the_visitors(sf);
    //    }
    //  }


/* incremental */
//int abolish_table_call_incr(CTXTdeclc VariantSF subgoal) {
//  declare_timer
//
//#ifdef DEBUG_ABOLISH
//  printf("abolish_table_call_incr: "); print_subgoal(stddbg, subgoal); printf("\n");
//#endif
//
//  start_table_gc_time(timer);
//  
//  if(IsIncrSF(subgoal))
//    abolish_incr_call(CTXTc subgoal->callnode);
//  
//  end_table_gc_time(timer);
//    
//  return TRUE;
//}

/*
* void abolish_if_tabled(CTXTdeclc Psc psc)
* {
*   CPtr ep;
* 
*   ep = (CPtr) get_ep(psc);
*   switch (*(pb)ep) {
*   case tabletry:
*   case tabletrysingle:
*     abolish_table_predicate(CTXTc psc);
*     break;
*   case test_heap:
*     if (*(pb)(ep+2) == tabletry || *(pb)(ep+2) == tabletrysingle)
*       abolish_table_predicate(CTXTc psc);
*     break;
*   case switchon3bound:
*   case switchonbound:
*   case switchonterm:
*     if (*(pb)(ep+3) == tabletry || *(pb)(ep+3) == tabletrysingle)
*       abolish_table_predicate(CTXTc psc);
*     break;
*   }
* }
*/
