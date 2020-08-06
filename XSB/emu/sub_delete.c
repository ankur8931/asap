/* File:      sub_delete.c
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
** $Id: sub_delete.c,v 1.27 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "error_xsb.h"
#include "thread_xsb.h"
#include "memory_xsb.h"
#include "tr_utils.h"

extern BTHTptr hhadded;

/* TLS: this file is used to delete subsumptive tables.  In the
   current MT engine, all structure managers used for the mt engine
   are private, and this code relies on that fact. I've made these
   dependencies explicit in the various function names. */

/* Freeing Individual Structures
   ----------------------------- */

static void free_private_tstn(CTXTdeclc TSTNptr tstn) {
  SM_DeallocateStruct(smTSTN,tstn);
}

static void free_private_tstht(CTXTdeclc TSTHTptr tstht) {
  TrieHT_RemoveFromAllocList(smTSTHT,tstht);
  SM_DeallocateStruct(smTSTHT,tstht);
}

static void free_private_tsi(CTXTdeclc TSTHTptr tstht) {
  if ( IsNonNULL(TSTHT_IndexHead(tstht)) )
    SM_DeallocateStructList(smTSIN,TSTHT_IndexTail(tstht),
			    TSTHT_IndexHead(tstht));
}


/*
 * Answer List of a Consumer may already be completely deallocated,
 * even the dummy node.  free_answer_list will handle either shared or
 * private SMs.
 */
static void free_private_al(CTXTdeclc VariantSF sf) {
  if ( IsNonNULL(subg_ans_list_ptr(sf)) )
    free_answer_list(sf);
}


/* Deleting Structures with Substructures for Call Subsumption
   -------------------------------------- */

static void delete_private_btht(CTXTdeclc BTHTptr btht) {
#ifdef MULTI_THREAD
  if (BTHT_NumBuckets(btht) == TrieHT_INIT_SIZE) {
    SM_DeallocateStruct(*private_smTableBTHTArray,BTHT_BucketArray(btht)); 
  }
  else
#endif
  mem_dealloc(BTHT_BucketArray(btht),BTHT_NumBuckets(btht)*sizeof(void *),TABLE_SPACE);
  TrieHT_RemoveFromAllocList(subsumptive_smBTHT,btht);
  SM_DeallocateStruct(subsumptive_smBTHT,btht);
}

static void delete_private_tstht(CTXTdeclc TSTHTptr tstht) {
#ifdef MULTI_THREAD
  if (BTHT_NumBuckets(tstht) == TrieHT_INIT_SIZE) {
    SM_DeallocateStruct(*private_smTableBTHTArray,BTHT_BucketArray(tstht)); 
  }
  else
#endif
  mem_dealloc(BTHT_BucketArray(tstht),BTHT_NumBuckets(tstht)*sizeof(void *),TABLE_SPACE);
  free_private_tsi(CTXTc tstht);
  free_private_tstht(CTXTc tstht);
}

static void delete_private_sf(CTXTdeclc VariantSF sf) {
  free_private_al(CTXTc sf);
  if ( IsProducingSubgoal(sf) )
    FreeProducerSF(sf)
  else
    SM_DeallocateStruct(smConsSF,sf);
}

/*-----------------------------------------------------------------------*/

/*
 *  Delete the given TST Answer Set node and recursively all of
 *  its children.
 */

extern void release_conditional_answer_info(CTXTdeclc BTNptr);

static void delete_tst_answer_set(CTXTdeclc TSTNptr root) {

  TSTNptr current, sibling;
  TSTHTptr hash_hdr;
  unsigned int i;


  if ( IsNULL(root) )
    return;

  /* I inserted the check for TSTN_Child(root) below to avoid
     a segmentation fault. It seems to be working fine with it, 
     and there doesn't seem to be any memory leak (wrt abolishing
     subsumptive tables), but as I don't know the code, I'd feel
     better if somebody who did looked at it.       -- lfcastro */

  if ( TSTN_Child(root) && IsHashHeader(TSTN_Child(root)) ) {
    hash_hdr = TSTN_GetHashHdr(root);
    for ( i = 0;  i < TSTHT_NumBuckets(hash_hdr);  i++ )
      for ( current = TSTHT_BucketArray(hash_hdr)[i];
	    IsNonNULL(current);  current = sibling ) {
	sibling = TSTN_Sibling(current);
	delete_tst_answer_set(CTXTc current);
      }
    delete_private_tstht(CTXTc hash_hdr);
  }

  else if ( IsLeafNode(root) )	{	  
    if (!hasALNtag(root))   
      release_conditional_answer_info(CTXTc (BTNptr) root);
  }

  else if ( ! IsLeafNode(root) )
    for ( current = TSTN_Child(root);  IsNonNULL(current);
	  current = sibling ) {
      sibling = TSTN_Sibling(current);
      delete_tst_answer_set(CTXTc current);
    }
  free_private_tstn(CTXTc root);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 *  Delete the given Call Table node and recursively all of its
 * children.  
 * 
 * TLS: subsumptive tables apparently are using BTNs, so make sure
 * that you delete from the correct SM.
 */

void delete_call_index(CTXTdeclc BTNptr root) {

  BTNptr current, sibling;
  BTHTptr hash_hdr;
  unsigned int i;

  if ( IsNULL(root) )
    return;

  if ( ! IsLeafNode(root) ) {
    if ( IsHashHeader(BTN_Child(root)) ) {

      hash_hdr = BTN_GetHashHdr(root);
      for ( i = 0;  i < BTHT_NumBuckets(hash_hdr);  i++ )
	for ( current = BTHT_BucketArray(hash_hdr)[i];
	      IsNonNULL(current);  current = sibling ) {
	  sibling = BTN_Sibling(current);
	  delete_call_index(CTXTc current);
	}
      delete_private_btht(CTXTc hash_hdr);
    }
    else 
      for ( current = BTN_Child(root);  IsNonNULL(current);
	    current = sibling ) {
	sibling = BTN_Sibling(current);
	delete_call_index(CTXTc current);
      }
  }
  SM_DeallocateStruct(subsumptive_smBTN,root);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void delete_subsumptive_predicate_table(CTXTdeclc TIFptr tif) {

  SubProdSF cur_prod, next_prod;
  SubConsSF cur_cons, next_cons;

  for ( cur_prod = (SubProdSF)TIF_Subgoals(tif);
	IsNonNULL(cur_prod);  cur_prod = next_prod ) {
    for ( cur_cons = subg_consumers(cur_prod);
	  IsNonNULL(cur_cons);  cur_cons = next_cons ) {
      next_cons = conssf_consumers(cur_cons);
      delete_private_sf(CTXTc (VariantSF)cur_cons);
    }
    next_prod = (SubProdSF)subg_next_subgoal(cur_prod);
    delete_tst_answer_set(CTXTc (TSTNptr)subg_ans_root_ptr(cur_prod));
    delete_private_sf(CTXTc (VariantSF)cur_prod);
  }
  delete_call_index(CTXTc TIF_CallTrie(tif));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void delete_subsumptive_call(CTXTdeclc SubProdSF cur_prod) {
  SubConsSF cur_cons, next_cons;
  
  //  if (IsSubConsSF(cur_prod)) {
  //    printf("abolishing producer via consume\n");
  //    cur_prod = conssf_producer(cur_prod);
  //  }

  for ( cur_cons = subg_consumers(cur_prod);
	IsNonNULL(cur_cons);  cur_cons = next_cons ) {
    next_cons = conssf_consumers(cur_cons);
    delete_private_sf(CTXTc (VariantSF)cur_cons);
    }
  delete_tst_answer_set(CTXTc (TSTNptr)subg_ans_root_ptr(cur_prod));
  delete_private_sf(CTXTc (VariantSF)cur_prod);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* TLS: use this when table has been deleted (via an abolish), but
   space for it has not been reclaimed.  In this case, we get the
   access points from the DelTF rather than the TIF -- this is
   necessary because we need an *old* set of subsumed calls rather
   than the new set we'd get from the TIF.  For now, called only when
   1 active thread.

   Input types reflect those of the TIFs */

void reclaim_deleted_subsumptive_table(CTXTdeclc DelTFptr deltf_ptr) {

  SubProdSF cur_prod, next_prod;
  SubConsSF cur_cons, next_cons;

  for ( cur_prod = (SubProdSF) DTF_Subgoals(deltf_ptr);
	IsNonNULL(cur_prod);  cur_prod = next_prod ) {
    for ( cur_cons = subg_consumers(cur_prod);
	  IsNonNULL(cur_cons);  cur_cons = next_cons ) {
      next_cons = conssf_consumers(cur_cons);
      delete_private_sf(CTXTc (VariantSF)cur_cons);
    }
    next_prod = (SubProdSF)subg_next_subgoal(cur_prod);
    delete_tst_answer_set(CTXTc (TSTNptr)subg_ans_root_ptr(cur_prod));
    delete_private_sf(CTXTc (VariantSF)cur_prod);
  }
  delete_call_index(CTXTc DTF_CallTrie(deltf_ptr));
}

