/* File:      table_stats.c
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
** $Id: table_stats.c,v 1.42 2012-07-04 15:10:46 tswift Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "table_stats.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "error_xsb.h"
#include "flags_xsb.h"
#include "debug_xsb.h"
#include "thread_xsb.h"
#include "slgdelay.h"
#include "memory_xsb.h"
#include "call_graph_xsb.h"
/*==========================================================================*/

/*
 *		  T A B L I N G   S T A T I S T I C S
 *		  ===================================
 */

/*
 *                      Recording Current Usage
 *                      -----------------------
 */

/* In order to use with both private and shared tables, this function
   does not use mutexes -- so its the responsability of the caller */

NodeStats node_statistics(Structure_Manager *sm) {

  NodeStats stats;

  SM_CurrentCapacity(*sm, NodeStats_NumBlocks(stats),
		     NodeStats_NumAllocNodes(stats));
  SM_CountFreeStructs(*sm, NodeStats_NumFreeNodes(stats));
  NodeStats_NodeSize(stats) = SM_StructSize(*sm);

  return stats;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* In order to use with both private and shared tables, this function
   does not use mutexes -- so its the responsability of the caller */

HashStats hash_statistics(CTXTdeclc Structure_Manager *sm) {

  HashStats ht_stats;
  counter num_used_hdrs;
  BTHTptr pBTHT;
  BTNptr *ppBTN;


  ht_stats.hdr = node_statistics(sm);

  num_used_hdrs = 0;
  HashStats_NumBuckets(ht_stats) = 0;
  HashStats_TotalOccupancy(ht_stats) = 0;
  HashStats_NonEmptyBuckets(ht_stats) = 0;
  HashStats_BucketSize(ht_stats) = sizeof(void *);
  pBTHT = (BTHTptr)SM_AllocList(*sm);
  while ( IsNonNULL(pBTHT) ) {
#ifdef DEBUG_ASSERTIONS
    /* Counter for contents of current hash table
       ------------------------------------------ */
v    counter num_contents = 0;
#endif
    num_used_hdrs++;
    HashStats_NumBuckets(ht_stats) += BTHT_NumBuckets(pBTHT);
    HashStats_TotalOccupancy(ht_stats) += BTHT_NumContents(pBTHT);
    for ( ppBTN = BTHT_BucketArray(pBTHT);
	  ppBTN < BTHT_BucketArray(pBTHT) + BTHT_NumBuckets(pBTHT);
	  ppBTN++ )
      if ( IsNonNULL(*ppBTN) ) {
#ifdef DEBUG_ASSERTIONS
	/* Count the objects in each bucket
	   -------------------------------- */
	BTNptr pBTN = *ppBTN;
	do {
	  num_contents++;
	  pBTN = BTN_Sibling(pBTN);
	} while ( IsNonNULL(pBTN) );
#endif
	HashStats_NonEmptyBuckets(ht_stats)++;
      }
#ifdef DEBUG_ASSERTIONS
    /* Compare counter and header values
       --------------------------------- */
    if ( num_contents != BTHT_NumContents(pBTHT) )
      xsb_warn(CTXTc "Inconsistent %s Usage Calculations:\n"
	       "\tHash table occupancy mismatch.", SM_StructName(*sm));
#endif
    pBTHT = BTHT_NextBTHT(pBTHT);
  }
  if ( HashStats_NumAllocHeaders(ht_stats) !=
       (num_used_hdrs + HashStats_NumFreeHeaders(ht_stats)) )
    xsb_warn(CTXTc "Inconsistent %s Usage Calculations:\n"
	     "\tHeader count mismatch:  Alloc: %d  Used: %d  Free: %d",
	     SM_StructName(*sm), HashStats_NumAllocHeaders(ht_stats),
	     num_used_hdrs,  HashStats_NumFreeHeaders(ht_stats));

  return ht_stats;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* In order to use with both private and shared tables, this function
   does not use mutexes -- so its the responsability of the caller */

NodeStats subgoal_statistics(CTXTdeclc Structure_Manager *sm) {

  NodeStats sg_stats;
  TIFptr tif;
  int nSubgoals;
  VariantSF pProdSF;
  SubConsSF pConsSF;

  sg_stats = node_statistics(sm);
  nSubgoals = 0;
  SYS_MUTEX_LOCK( MUTEX_TABLE );				
  if ( sm == &smVarSF ) {
    for ( tif = tif_list.first;  IsNonNULL(tif);  tif = TIF_NextTIF(tif) )
      if ( IsVariantPredicate(tif) )
	for ( pProdSF = TIF_Subgoals(tif);  IsNonNULL(pProdSF);
	      pProdSF = (VariantSF)subg_next_subgoal(pProdSF) )
	  nSubgoals++;
  }
  /* No shared smProdSF or smConsSF in MT engine */
  else if ( sm == &smProdSF ) {
    for ( tif = tif_list.first;  IsNonNULL(tif);  tif = TIF_NextTIF(tif) )
      if ( IsSubsumptivePredicate(tif) )
	for ( pProdSF = TIF_Subgoals(tif);  IsNonNULL(pProdSF);
	      pProdSF = (VariantSF)subg_next_subgoal(pProdSF) )
	  nSubgoals++;
  }
  else if ( sm == &smConsSF ) {
    for ( tif = tif_list.first;  IsNonNULL(tif);  tif = TIF_NextTIF(tif) )
      if ( IsSubsumptivePredicate(tif) )
	for ( pProdSF = TIF_Subgoals(tif);  IsNonNULL(pProdSF);
	      pProdSF = (VariantSF)subg_next_subgoal(pProdSF) )
	  for ( pConsSF = subg_consumers(pProdSF);  IsNonNULL(pConsSF); 
		pConsSF = conssf_consumers(pConsSF) )
	    nSubgoals++;
  }
  else {
    SYS_MUTEX_UNLOCK( MUTEX_TABLE );				
    xsb_dbgmsg((LOG_DEBUG, "Incorrect use of subgoal_statistics()\n"
	       "SM does not contain subgoal frames"));
    return sg_stats;
  }

  SYS_MUTEX_UNLOCK( MUTEX_TABLE );				
  if ( NodeStats_NumUsedNodes(sg_stats) != (counter) nSubgoals )
    xsb_warn(CTXTc "Inconsistent Subgoal Frame Usage Calculations:\n"
	     "\tSubgoal Frame count mismatch");

  return sg_stats;
}

/*-------------------------------------------------------------------------*/

/*
 *                      Displaying Current Usage
 *                      ------------------------
 */

void print_NodeStats(NodeStats node,char * nodeName) {
      printf("    %s (%"UIntfmt" blocks)\n"
	     "      Allocated:   %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "      Used:        %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "      Free:        %10"UIntfmt"  (%8"UIntfmt" bytes)\n",
	     nodeName,NodeStats_NumBlocks(node),
	     NodeStats_NumAllocNodes(node),  NodeStats_SizeAllocNodes(node),
	     NodeStats_NumUsedNodes(node),  NodeStats_SizeUsedNodes(node),
	     NodeStats_NumFreeNodes(node),  NodeStats_SizeFreeNodes(node));
    }

void print_HashStats(HashStats hashTab,char * tabName) {
      printf("    %s (%"UIntfmt" blocks)\n"
	     "      Headers:     %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "        Used:      %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "        Free:      %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "      Buckets:     %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	     "        Used:      %10"UIntfmt"\n"
	     "        Empty:     %10"UIntfmt"\n"
	     "      Occupancy:   %10"UIntfmt" Elements\n",
	     tabName,HashStats_NumBlocks(hashTab),
	     HashStats_NumAllocHeaders(hashTab),  HashStats_SizeAllocHeaders(hashTab),
	     HashStats_NumUsedHeaders(hashTab),  HashStats_SizeUsedHeaders(hashTab),
	     HashStats_NumFreeHeaders(hashTab),  HashStats_SizeFreeHeaders(hashTab),
	     HashStats_NumBuckets(hashTab),  HashStats_SizeAllocBuckets(hashTab),
	     HashStats_NonEmptyBuckets(hashTab),  HashStats_EmptyBuckets(hashTab),
	     HashStats_TotalOccupancy(hashTab));
}

void print_wfs_usage(Integer space_alloc,Integer space_used, size_t num_blocks,int frames_per_block,
		int size_per_block,char * elt_name) {
    printf("    %s (%"UIntfmt" blocks)\n"
	   "      Allocated:   %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	   "      Used:        %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	   "      Free:        %10"UIntfmt"  (%8"UIntfmt" bytes)\n",
	   elt_name,(UInteger) num_blocks, 
	   (UInteger) num_blocks*frames_per_block, (UInteger) space_alloc, 
	   (UInteger) (space_used/size_per_block),  (UInteger) space_used, 
	   (UInteger) (num_blocks*frames_per_block - (space_used/size_per_block)), 
	   (UInteger)  (space_alloc - space_used));
}

#ifndef MULTI_THREAD
void print_detailed_tablespace_stats(CTXTdecl) {

  NodeStats
    abtn,		/* Asserted Basic Trie Nodes */
    btn,		/* Basic Trie Nodes */
    tstn,		/* Time Stamp Trie Nodes */
    aln,		/* Answer List Nodes */
    tsi,		/* Time Stamp Indices (Index Entries/Nodes) */
    varsf,		/* Variant Subgoal Frames */
    prodsf,		/* Subsumptive Producer Subgoal Frames */
    conssf,		/* Subsumptive Consumer Subgoal Frames */ 
    asi;                /* Answer Subst. Info (for conditional answers) */

  HashStats
    abtht,		/* Asserted Basic Trie Hash Tables */
    btht,		/* Basic Trie Hash Tables */
    tstht;		/* Time Stamp Trie Hash Tables */
  
  size_t
    trieassert_alloc, trieassert_used,
    tablespace_alloc, tablespace_used,
    de_space_alloc, de_space_used,
    dl_space_alloc, dl_space_used,
    pnde_space_alloc, pnde_space_used;

  size_t num_de_blocks, num_dl_blocks, num_pnde_blocks;


  btn = node_statistics(&smTableBTN);
  btht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);
  prodsf = subgoal_statistics(CTXTc &smProdSF);
  conssf = subgoal_statistics(CTXTc &smConsSF);
  aln = node_statistics(&smALN);
  tstn = node_statistics(&smTSTN);
  tstht = hash_statistics(CTXTc &smTSTHT);
  tsi = node_statistics(&smTSIN);
  tsi = node_statistics(&smTSIN);
  asi = node_statistics(&smASI);

  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  dl_space_alloc = allocated_dl_space(current_dl_block_gl,& num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();

  tablespace_alloc = CurrentTotalTableSpaceAlloc(btn,btht,varsf,prodsf,conssf,aln,
						 tstn,tstht,tsi,asi);
  tablespace_used =  CurrentTotalTableSpaceUsed(btn,btht,varsf,prodsf,conssf,aln,
						tstn,tstht,tsi,asi);

  tablespace_alloc = tablespace_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;

  tablespace_used = tablespace_used + de_space_used + dl_space_used + pnde_space_used;

  abtn = node_statistics(&smAssertBTN);
  abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc =
    NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used =
    NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);
  SQUASH_LINUX_COMPILER_WARN(trieassert_used) ; 

  printf("\n"
	 "Table Space Usage (excluding asserted and interned tries) \n");
  printf("  Current Total Allocation:   %12" UIntfmt"  bytes\n"
  	 "  Current Total Usage:        %12" UIntfmt" bytes\n",
	 (UInteger) (pspacesize[TABLE_SPACE]-trieassert_alloc),  
	 (UInteger) (pspacesize[TABLE_SPACE]-trieassert_alloc-(tablespace_alloc-tablespace_used)));

  // Basic Trie Stuff
  if ( NodeStats_NumBlocks(btn) > 0 || HashStats_NumBlocks(btht) > 0 || NodeStats_NumBlocks(aln) > 0) {
    printf("\n"
	   "  Basic Tries\n");
    if ( NodeStats_NumBlocks(btn) > 0) 
      print_NodeStats(btn,"Basic Trie Nodes");
    if (HashStats_NumBlocks(btht) > 0)
      print_HashStats(btht,"Basic Trie Hash Tables");
    if ( NodeStats_NumBlocks(aln) > 0) 
      print_NodeStats(aln,"Answer List Nodes (for Incomplete Variant Tables)");
  }

  // Subgoal Frames
  if (NodeStats_NumBlocks(varsf) > 0 || NodeStats_NumBlocks(prodsf) > 0 
      || NodeStats_NumBlocks(conssf) > 0) {
    printf("\n"
	   "  Subgoal Frames\n");
    if (NodeStats_NumBlocks(varsf) > 0)
      print_NodeStats(varsf,"Variant Subgoal Frames");
    if (NodeStats_NumBlocks(prodsf) > 0)
      print_NodeStats(prodsf,"Producer Subgoal Frames");
    if (NodeStats_NumBlocks(conssf) > 0)
      print_NodeStats(conssf,"Consumer Subgoal Frames");
  }

  // Subsumptive Tables
  if (NodeStats_NumBlocks(tstn) > 0 || NodeStats_NumBlocks(tsi) > 0 || HashStats_NumBlocks(tstht) > 0) {
    printf("\n"
	   "  Time Stamp Tries (for Subsumptive Tables) \n");
    if (NodeStats_NumBlocks(tstn) > 0) 
      print_NodeStats(tstn,"Time Stamp Trie Nodes");
    if (HashStats_NumBlocks(tstht) > 0) 
      print_HashStats(tstht,"Time Stamp Trie Hash Tables");
    if (NodeStats_NumBlocks(tsi) > 0) 
      print_NodeStats(tsi,"Time Stamp Trie Index Nodes");
  }

  // Conditional Answers
  if (dl_space_alloc > 0 || de_space_alloc > 0 || pnde_space_alloc > 0 
      || NodeStats_NumBlocks(asi) > 0) {
    printf("\n"
	   "  Information for Conditional Answers in Variant Tables \n");
    if (dl_space_alloc > 0) 
      print_wfs_usage(dl_space_alloc,dl_space_used,num_dl_blocks,DLS_PER_BLOCK,
		      sizeof(struct delay_list),"Delay Lists");
    if (de_space_alloc > 0) 
      print_wfs_usage(de_space_alloc,de_space_used,num_de_blocks,DES_PER_BLOCK,
		      sizeof(struct delay_element),"Delay Elements");
    if (pnde_space_alloc > 0) 
      print_wfs_usage(pnde_space_alloc,pnde_space_used,num_pnde_blocks,PNDES_PER_BLOCK,
		      sizeof(struct pos_neg_de_list),"Back-pointer Lists");
    if ( NodeStats_NumBlocks(asi) > 0)
      print_NodeStats(asi,"Answer Substitution Frames");
  }

  if (total_call_node_count_gl) {
    printf("\nTotal number of incremental subgoals created: %d\n",total_call_node_count_gl);
    printf("   Current number of incremental call nodesd: %d\n",current_call_node_count_gl);
    printf("   Total number of incremental call edges created: %d\n",current_call_edge_count_gl);
  }

  // Private trie assert space
  if ( NodeStats_NumBlocks(abtn) > 0 || HashStats_NumBlocks(abtht) > 0) {
  printf("\n ------------------ Asserted and Interned Tries  ----------------\n");
    if ( NodeStats_NumBlocks(abtn) > 0) 
      print_NodeStats(abtn,"Basic Trie Nodes (Assert)");
    if (HashStats_NumBlocks(abtht) > 0)
      print_HashStats(abtht,"Basic Trie Hash Tables (Assert)");
  }

  if (flags[MAX_USAGE]) {
    /* Report Maximum Usages
       --------------------- */
    update_maximum_tablespace_stats(&btn,&btht,&varsf,&prodsf,&conssf,
				    &aln,&tstn,&tstht,&tsi,&asi);
    printf("\n"
	   "Maximum Total Usage:        %12"UIntfmt" bytes\n",
	   maximum_total_tablespace_usage());
    printf("Maximum Structure Usage:\n"
	   "  ALNs:            %10"UIntfmt"  (%8"UIntfmt" bytes)\n"
	   "  TSINs:           %10"UIntfmt"  (%8"UIntfmt" bytes)\n",
	   maximum_answer_list_nodes(),
	   maximum_answer_list_nodes() * NodeStats_NodeSize(aln),
	   maximum_timestamp_index_nodes(),
	   maximum_timestamp_index_nodes() * NodeStats_NodeSize(tsi));
  }
  printf("\n");
}

// ====================================================================================
#else // (MULTI_THREAD)
void print_detailed_tablespace_stats(CTXTdecl) {

  NodeStats
    abtn,		/* Asserted Basic Trie Nodes */
    btn,		/* Basic Trie Nodes */
    aln,		/* Answer List Nodes */
    varsf,		/* Variant Subgoal Frames */
    asi;		/* Answer Subgoal Information */

  HashStats
    abtht,		/* Asserted Basic Trie Hash Tables */
    btht;		/* Basic Trie Hash Tables */

  NodeStats  
    pri_tstn,		/* Private Time Stamp Trie Nodes */
    pri_tsi,		/* Private Time Stamp Indices (Index Entries/Nodes) */
    pri_prodsf,		/* Private Subsumptive Producer Subgoal Frames */
    pri_conssf,		/* Private Subsumptive Consumer Subgoal Frames */
    pri_btn,		/* Private Basic Trie Nodes (Tables) */
    pri_assert_btn,	/* Private Basic Trie Nodes (Asserts) */
    pri_aln,		/* Private Answer List Nodes */
    pri_varsf,		/* Private Variant Subgoal Frames */
    pri_asi;		/* Private Answer Subgoal Information */
  
  HashStats
    pri_btht,		/* Private Basic Trie Hash Tables (Tables) */
    pri_assert_btht,	/* Private Basic Trie Hash Tables (Asserts) */
    pri_tstht;		/* Private Time Stamp Trie Hash Tables */

  size_t
    tablespace_alloc, tablespace_used,
    pri_tablespace_alloc, pri_tablespace_used,
    shared_tablespace_alloc, shared_tablespace_used,
    trieassert_alloc, trieassert_used,
    de_space_alloc, de_space_used,
    dl_space_alloc, dl_space_used,
    pnde_space_alloc, pnde_space_used,
    pri_de_space_alloc, pri_de_space_used,
    pri_dl_space_alloc, pri_dl_space_used,
    pri_pnde_space_alloc, pri_pnde_space_used;

  size_t num_de_blocks, num_dl_blocks, num_pnde_blocks;
  size_t pri_num_de_blocks, pri_num_dl_blocks, pri_num_pnde_blocks;

  SM_Lock(smTableBTN);
  btn = node_statistics(&smTableBTN);
  SM_Unlock(smTableBTN);
  SM_Lock(smTableBTHT);
  btht = hash_statistics(CTXTc &smTableBTHT);
  SM_Unlock(smTableBTHT);
  SM_Lock(smVarSF);
  varsf = subgoal_statistics(CTXTc &smVarSF);
  SM_Unlock(smVarSF);
  SM_Lock(smALN);
  aln = node_statistics(&smALN);
  SM_Unlock(smALN);
  SM_Lock(smASI);
  asi = node_statistics(&smASI);
  SM_Unlock(smASI);

  SYS_MUTEX_LOCK( MUTEX_DELAY );			
  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  dl_space_alloc = allocated_dl_space(current_dl_block_gl,&num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();
  SYS_MUTEX_UNLOCK( MUTEX_DELAY );			

  pri_btn = node_statistics(private_smTableBTN);
  pri_btht = hash_statistics(CTXTc private_smTableBTHT);
  pri_assert_btn = node_statistics(private_smAssertBTN);
  pri_assert_btht = hash_statistics(CTXTc private_smAssertBTHT);
  pri_aln = node_statistics(private_smALN);
  pri_asi = node_statistics(private_smASI);
  pri_varsf = subgoal_statistics(CTXTc private_smVarSF);
  pri_prodsf = subgoal_statistics(CTXTc private_smProdSF);
  pri_conssf = subgoal_statistics(CTXTc private_smConsSF);
  pri_tstn = node_statistics(private_smTSTN);
  pri_tsi = node_statistics(private_smTSIN);
  pri_btht = hash_statistics(CTXTc private_smTableBTHT);
  pri_tstht = hash_statistics(CTXTc private_smTSTHT);

  pri_de_space_alloc = allocated_de_space(private_current_de_block,&pri_num_de_blocks);
  pri_de_space_used = pri_de_space_alloc - unused_de_space_private(CTXT);
  pri_dl_space_alloc = allocated_dl_space(private_current_dl_block,&pri_num_dl_blocks);
  pri_dl_space_used = pri_dl_space_alloc - unused_dl_space_private(CTXT);
  pri_pnde_space_alloc = allocated_pnde_space(private_current_pnde_block,&pri_num_pnde_blocks);
  pri_pnde_space_used = pri_pnde_space_alloc - unused_pnde_space_private(CTXT);

  pri_tablespace_alloc = CurrentPrivateTableSpaceAlloc(pri_btn,pri_btht,pri_varsf,pri_prodsf,
						       pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,
						       pri_asi);
  pri_tablespace_used = CurrentPrivateTableSpaceUsed(pri_btn,pri_btht,pri_varsf,pri_prodsf,
						     pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,
						     pri_asi);

  pri_tablespace_alloc = pri_tablespace_alloc + pri_de_space_alloc + 
    pri_dl_space_alloc + pri_pnde_space_alloc;
  pri_tablespace_used = pri_tablespace_used + pri_de_space_used + pri_dl_space_used + pri_pnde_space_used;

  shared_tablespace_alloc = CurrentSharedTableSpaceAlloc(btn,btht,varsf,aln,asi);
  shared_tablespace_used = CurrentSharedTableSpaceUsed(btn,btht,varsf,aln,asi);

  shared_tablespace_alloc = shared_tablespace_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  shared_tablespace_used = shared_tablespace_used + de_space_used + dl_space_used + pnde_space_used;

  tablespace_alloc = shared_tablespace_alloc + pri_tablespace_alloc;
  tablespace_used =  shared_tablespace_used + pri_tablespace_used;

  abtn = node_statistics(&smAssertBTN);
  abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc = NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used = NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);
  SQUASH_LINUX_COMPILER_WARN(trieassert_used) ; 

  printf("  Current Total Allocation:   %12"UIntfmt" bytes\n"
  	 "  Current Total Usage:        %12"UIntfmt" bytes\n",
	 pspacesize[TABLE_SPACE]-trieassert_alloc,  
	 pspacesize[TABLE_SPACE]-trieassert_alloc-(tablespace_alloc-tablespace_used));

  //   printf("\n --------------------- Shared tables ---------------------\n");

  printf("\n"
	 "Shared Table Space Usage (exc"UIntfmt"ding asserted and interned tries) \n");
  printf("  Current Total Allocation:   %12"UIntfmt" bytes\n"
  	 "  Current Total Usage:        %12"UIntfmt" bytes\n",
	 shared_tablespace_alloc,shared_tablespace_used);

  // Basic Trie Stuff
  if ( NodeStats_NumBlocks(btn) > 0 || HashStats_NumBlocks(btht) > 0 || NodeStats_NumBlocks(aln) > 0) {
    printf("\n"
	   "  Basic Tries\n");
    if ( NodeStats_NumBlocks(btn) > 0) 
      print_NodeStats(btn,"Basic Trie Nodes");
    if (HashStats_NumBlocks(btht) > 0)
      print_HashStats(btht,"Basic Trie Hash Tables");
    if ( NodeStats_NumBlocks(aln) > 0) 
      print_NodeStats(aln,"Answer List Nodes (for Incomplete Variant Tables)");
  }

  // Subgoal Frames
  if (NodeStats_NumBlocks(varsf) > 0) {
    printf("\n"
	   "  Subgoal Frames\n");
    if (NodeStats_NumBlocks(varsf) > 0)
      print_NodeStats(varsf,"Variant Subgoal Frames");
  }

  // Conditional Answers
  if (dl_space_alloc > 0 || de_space_alloc > 0 || pnde_space_alloc > 0 
      || NodeStats_NumBlocks(asi) > 0) {
    printf("\n"
	   "  Information for Conditional Answers in Variant Tables \n");
    if (dl_space_alloc > 0) 
      print_wfs_usage(dl_space_alloc,dl_space_used,num_dl_blocks,DLS_PER_BLOCK,
		      sizeof(struct delay_list),"Delay Lists");
    if (de_space_alloc > 0) 
      print_wfs_usage(de_space_alloc,de_space_used,num_de_blocks,DES_PER_BLOCK,
		      sizeof(struct delay_element),"Delay Elements");
    if (pnde_space_alloc > 0) 
      print_wfs_usage(pnde_space_alloc,pnde_space_used,num_pnde_blocks,PNDES_PER_BLOCK,
		      sizeof(struct pos_neg_de_list),"Back-pointer Lists");
    if ( NodeStats_NumBlocks(asi) > 0)
      print_NodeStats(asi,"Answer Substitution Frames");
  }

  // Private trie assert space
  if ( NodeStats_NumBlocks(abtn) > 0 || HashStats_NumBlocks(abtht) > 0) {
  printf("\n ---------------- Shared Asserted and Interned Tries  ----------------\n");
    if ( NodeStats_NumBlocks(abtn) > 0) 
      print_NodeStats(abtn,"Basic Trie Nodes (Assert)");
    if (HashStats_NumBlocks(abtht) > 0)
      print_HashStats(abtht,"Basic Trie Hash Tables (Assert)");
  }

  printf("\n --------------------- Private tables ---------------------\n");
  printf("\n"
	 "Private Table Space Usage for Thread %"Intfmt" (exc"UIntfmt"ding asserted and interned tries) \n"
	 "  Current Total Allocation:   %12"UIntfmt" bytes\n"
  	 "  Current Total Usage:        %12"UIntfmt" bytes\n",
	 xsb_thread_id,pri_tablespace_alloc,pri_tablespace_used);

  // Basic Trie Stuff
  if ( NodeStats_NumBlocks(pri_btn) > 0 || HashStats_NumBlocks(pri_btht) > 0 
       || NodeStats_NumBlocks(pri_aln) > 0) {
    printf("\n"
	   "  Basic Tries\n");
    if ( NodeStats_NumBlocks(pri_btn) > 0) 
      print_NodeStats(pri_btn,"Basic Trie Nodes");
    if (HashStats_NumBlocks(pri_btht) > 0)
      print_HashStats(pri_btht,"Basic Trie Hash Tables");
    if ( NodeStats_NumBlocks(pri_aln) > 0) 
      print_NodeStats(pri_aln,"Answer List Nodes (for Incomplete Variant Tables)");
  }

  // Subgoal Frames
  if (NodeStats_NumBlocks(pri_varsf) > 0 || NodeStats_NumBlocks(pri_prodsf) > 0 ||
      NodeStats_NumBlocks(pri_conssf) > 0) {
    printf("\n"
	   "  Subgoal Frames\n");
    if (NodeStats_NumBlocks(pri_varsf) > 0)
      print_NodeStats(pri_varsf,"Variant Subgoal Frames");
    if (NodeStats_NumBlocks(pri_prodsf) > 0)
      print_NodeStats(pri_prodsf,"Producer Subgoal Frames");
    if (NodeStats_NumBlocks(pri_conssf) > 0)
      print_NodeStats(pri_conssf,"Consumer Subgoal Frames");
  }

  // Subsumptive Tables
  if ( NodeStats_NumBlocks(pri_tstn) > 0 || HashStats_NumBlocks(pri_tstht) > 0
       || NodeStats_NumBlocks(pri_tsi) > 0) {
    printf("\n"
	   "  Time Stamp Tries (for Subsumptive Tables) \n");
    if (NodeStats_NumBlocks(pri_tstn) > 0) 
      print_NodeStats(pri_tstn,"Time Stamp Trie Nodes");
    if (HashStats_NumBlocks(pri_tstht) > 0) 
      print_HashStats(pri_tstht,"Time Stamp Trie Hash Tables");
    if (NodeStats_NumBlocks(pri_tsi) > 0) 
      print_NodeStats(pri_tsi,"Time Stamp Trie Index Nodes");
  }

  // Conditional Answers
  if (pri_dl_space_alloc > 0 || pri_de_space_alloc > 0 || pri_pnde_space_alloc > 0 
      || NodeStats_NumBlocks(pri_asi) > 0) {
    printf("\n"
	   "  Information for Conditional Answers in Variant Tables \n");
    if (pri_dl_space_alloc > 0) 
      print_wfs_usage(pri_dl_space_alloc,pri_dl_space_used,pri_num_dl_blocks,DLS_PER_BLOCK,
		      sizeof(struct delay_list),"Delay Lists");
    if (pri_de_space_alloc > 0) 
      print_wfs_usage(pri_de_space_alloc,pri_de_space_used,pri_num_de_blocks,DES_PER_BLOCK,
		      sizeof(struct delay_element),"Delay Elements");
    if (pri_pnde_space_alloc > 0) 
      print_wfs_usage(pri_pnde_space_alloc,pri_pnde_space_used,pri_num_pnde_blocks,PNDES_PER_BLOCK,
		      sizeof(struct pos_neg_de_list),"Back-pointer Lists");
    if ( NodeStats_NumBlocks(pri_asi) > 0)
      print_NodeStats(pri_asi,"Answer Substitution Frames");
  }

  // Private trie assert space
  if ( NodeStats_NumBlocks(pri_assert_btn) > 0 || HashStats_NumBlocks(pri_assert_btht) > 0) {
  printf("\n ---------------- Private Asserted and Interned Tries  ----------------\n");
    if ( NodeStats_NumBlocks(pri_assert_btn) > 0) 
      print_NodeStats(pri_assert_btn,"Basic Trie Nodes (Assert)");
    if (HashStats_NumBlocks(pri_assert_btht) > 0)
      print_HashStats(pri_assert_btht,"Basic Trie Hash Tables (Assert)");
  }

  printf("\n");
}
#endif

/*-------------------------------------------------------------------------*/

/*
 *                       Recording Maximum Usage
 *                       -----------------------
 */

/*
 * Most of the data structures used in tabling are persistent: table
 * info frames, subgoal frames, and the trie nodes and hash tables
 * (basic and time-stamped).  Therefore, the maximum usage of each of
 * these structures at any particular time during an evaluation is the
 * same as their current usage.  However, there are some structures
 * whose usage fluctuates during an evaluation.  In particular,
 * TimeStamp Indices and answer list nodes are reclaimed upon
 * completion.  Therefore, we explicitly track their maximum usage at
 * those times just prior to reclamation.  The following data
 * structure maintains the maximum usage for each of these structures.
 * And as the (ultimate) maximum usage of a particular structure is
 * independent of that of other structures, we also explicitly track
 * the maximum utilization of table space.
 */

struct {
  counter tsi;                   /* TS Index Nodes */
  counter alns;                  /* Answer List Nodes */
  size_t  total_bytes;    /* total tablespace in bytes */
} maxTableSpaceUsage = {0,0,0};


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//void reset_maximum_tablespace_stats() {
//
//  maxTableSpaceUsage.tsi = maxTableSpaceUsage.alns = 0;
//  maxTableSpaceUsage.total_bytes = 0;
//}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * To be used to facilitate statistical recordings, but in situations
 * where one is not currently inspecting results, e.g., when reclaiming
 * table space at completion.  This procedure queries the current state
 * of usage of all table components, and tests whether this is a new
 * maximum.
 */

#ifndef MULTI_THREAD
void compute_maximum_tablespace_stats(CTXTdecl) {

  NodeStats tstn, btn, aln, tsi;
  NodeStats varsf, prodsf, conssf, asi;
  HashStats tstht, btht;

  btn = node_statistics(&smTableBTN);
  btht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);
  prodsf = subgoal_statistics(CTXTc &smProdSF);
  conssf = subgoal_statistics(CTXTc &smConsSF);
  tstn = node_statistics(&smTSTN);
  tstht = hash_statistics(CTXTc &smTSTHT);
  tsi = node_statistics(&smTSIN);
  aln = node_statistics(&smALN);
  asi = node_statistics(&smASI);

  update_maximum_tablespace_stats(&btn,&btht,&varsf,&prodsf,&conssf,
				  &aln,&tstn,&tstht,&tsi,&asi);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
 * To be used to facilitate statistical reporting at the moment -- when
 * one is currently inspecting results.  Presumably one would want to
 * independently query the current state of usage of all table
 * components for such reporting, so we use these results rather than
 * recompute them.
 */

void update_maximum_tablespace_stats(NodeStats *btn, HashStats *btht,
				     NodeStats *varsf, NodeStats *prodsf,
				     NodeStats *conssf, NodeStats *aln,
				     NodeStats *tstn, HashStats *tstht,
				     NodeStats *tsi,NodeStats *asi) {
   size_t  byte_size;

   byte_size = CurrentTotalTableSpaceUsed(*btn,*btht,*varsf,*prodsf,*conssf,
					  *aln,*tstn,*tstht,*tsi,*asi);
   if ( byte_size > maxTableSpaceUsage.total_bytes )
     maxTableSpaceUsage.total_bytes = byte_size;
   if ( NodeStats_NumUsedNodes(*aln) > maxTableSpaceUsage.alns )
     maxTableSpaceUsage.alns = NodeStats_NumUsedNodes(*aln);
   if ( NodeStats_NumUsedNodes(*tsi) > maxTableSpaceUsage.tsi )
     maxTableSpaceUsage.tsi = NodeStats_NumUsedNodes(*tsi);
}
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

counter maximum_timestamp_index_nodes() {

  return (maxTableSpaceUsage.tsi);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

counter maximum_answer_list_nodes() {

  return (maxTableSpaceUsage.alns);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

UInteger  maximum_total_tablespace_usage() {

  return (maxTableSpaceUsage.total_bytes);
}

/*-------------------------------------------------------------------------*/

/*
 *               Recording and Displaying Operational Counts
 *               -------------------------------------------
 */

NumSubOps numSubOps = INIT_NUMSUBOPS;


void reset_subsumption_stats() {

  NumSubOps initRecord = INIT_NUMSUBOPS;

  numSubOps = initRecord;
}


void print_detailed_subsumption_stats() {

  printf("Subsumptive Operations\n"
	 "  Subsumptive call check/insert ops: %8"UIntfmt"\n"
	 "  * Calls to nonexistent or incomplete tables\n"
	 "    - Producers:                   %6"UIntfmt"\n"
	 "    - Variants of producers:       %6"UIntfmt"\n"
	 "    - Properly subsumed:           %6"UIntfmt"\n"
	 "        Resulted in call table entry: %"UIntfmt"\n"
	 "  * Calls to completed tables:     %6"UIntfmt"\n",
	 NumSubOps_CallCheckInsert, NumSubOps_ProducerCall,
	 NumSubOps_VariantCall, NumSubOps_SubsumedCall,
	 NumSubOps_SubsumedCallEntry, NumSubOps_CallToCompletedTable);
  printf("  Answer check/insert operations:    %8"UIntfmt"\n"
	 "  * Actual answer inserts:         %6"UIntfmt"\n"
	 "  * Derivation ratio (New/Total):    %4.2f\n",
	 NumSubOps_AnswerCheckInsert, NumSubOps_AnswerInsert,
	 ( (NumSubOps_AnswerCheckInsert != 0)
	   ? (float)NumSubOps_AnswerInsert / (float)NumSubOps_AnswerCheckInsert
	   : 0 ));
  printf("  Relevant-answer identify ops:      %8"UIntfmt"\n"
	 "  Answer-list consumption ops:       %8"UIntfmt"\n",
	 NumSubOps_IdentifyRelevantAnswers, NumSubOps_AnswerConsumption);
}

/*==========================================================================*/
