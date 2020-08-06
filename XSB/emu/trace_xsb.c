/* File:      trace_xsb.c
** Author(s): Jiyang Xu, Terrance Swift, Kostis Sagonas
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
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: trace_xsb.c,v 1.41 2009/11/17 14:59:34 tswift Exp $
** 
*/

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "inst_xsb.h"
#include "memory_xsb.h"
#include "register.h"
#include "psc_xsb.h"
#include "table_stats.h"
#include "trie_internals.h"
#include "tries.h"
#include "tab_structs.h"
#include "choice.h"
#include "flags_xsb.h"
#include "heap_xsb.h"
#include "thread_xsb.h"
#include "trace_xsb.h"
#include "thread_xsb.h"
#include "deadlock.h"
#include "slgdelay.h"
#include "cinterf.h"
#include "system_defs_xsb.h"
#include "subp.h"
#include "error_xsb.h"
#include "tab_structs.h"
#include "tr_utils.h"
#include "loader_xsb.h"
#include "call_graph_xsb.h"
#include "hash_xsb.h"

extern void print_mutex_use(void);
extern void dis(xsbBool);

/*======================================================================*/
/* Process-level information: keep this global */

double time_start_gl;      /* time from which stats started being collected */
double realtime_count_gl;

#ifndef MULTI_THREAD
double cputime_count_gl;
#else 
double time_count = 0;
#endif

static double last_cpu = 0;      /* time from which stats started being collected */
static double last_wall = 0;      /* time from which stats started being collected */

#ifndef MULTI_THREAD
void print_abolish_table_statistics() {
  
  if (total_table_gc_time > 0)
    printf("\n   %.3f seconds spent in table abolishing",total_table_gc_time);
}
#endif

int count_sccs(CTXTdecl) {					
  int ctr = 0;
  int last_scc = 0;
  CPtr csf = openreg;

  //  printf("open %x COMPL %x\n",openreg,COMPLSTACKBOTTOM);
  while (csf < COMPLSTACKBOTTOM) {
    //    printf("comp level %d\n",compl_level(csf));
    if (compl_level(csf) != last_scc) {
      ctr++;
      last_scc = compl_level(csf);
      //      printf("ctr: %d\n",ctr);
    }
    csf = prev_compl_frame(csf);	       
    
  }
  return ctr;
}
    
char *pspace_cat[NUM_CATS_SPACE] =
  {"atom        ","string      ","asserted    ","compiled    ",
   "foreign     ","table       ","findall     ","profile     ",
   "mt-private  ","buffer      ","gc temp     ","hash        ",
   "interprolog ","thread      ","read canon  ","leaking...  ",
   "special     ","other       ","incr table  ","odbc        "};

extern void stat_inusememory(CTXTdeclc double,int);

/*
 * Called through builtin statistics/2.
 */
void statistics_inusememory(CTXTdeclc int type) {
#ifndef MULTI_THREAD
  cputime_count_gl = (cpu_time() - time_start_gl);
#endif
  stat_inusememory(CTXTc real_time()-realtime_count_gl,type);   /* collect */
}
/*======================================================================*/
/*  Memory statistics.					*/
/*======================================================================*/

#ifndef MULTI_THREAD
void stat_inusememory(CTXTdeclc double elapstime, int type) {

  NodeStats
    tbtn,		/* Table Basic Trie Nodes */
    abtn,		/* Asserted Basic Trie Nodes */
    tstn,		/* Time Stamp Trie Nodes */
    aln,		/* Answer List Nodes */
    tsi,		/* Time Stamp Indices (Index Entries/Nodes) */
    varsf,		/* Variant Subgoal Frames */
    prodsf,		/* Subsumptive Producer Subgoal Frames */
    conssf,		/* Subsumptive Consumer Subgoal Frames */
    asi;		/* Answer Subst Info for conditional answers */

  HashStats
    tbtht,		/* Table Basic Trie Hash Tables */
    abtht,		/* Asserted Basic Trie Hash Tables */
    tstht;		/* Time Stamp Trie Hash Tables */
  
  size_t
    total_alloc, total_used,
    tablespace_sm_alloc, tablespace_sm_used,
    trieassert_alloc, trieassert_used,
    gl_avail, pnde_space_alloc, pnde_space_used, pspacetot;

  UInteger dl_space_alloc, dl_space_used,  de_space_alloc, de_space_used, tc_avail;

  size_t
    num_de_blocks, num_dl_blocks, num_pnde_blocks,i;

  tbtn = node_statistics(&smTableBTN);             tbtht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);      prodsf = subgoal_statistics(CTXTc &smProdSF);
  conssf = subgoal_statistics(CTXTc &smConsSF);    aln = node_statistics(&smALN);
  tstn = node_statistics(&smTSTN);                 tstht = hash_statistics(CTXTc &smTSTHT);
  tsi = node_statistics(&smTSIN);                  asi = node_statistics(&smASI);

  tablespace_sm_alloc = CurrentTotalTableSpaceAlloc(tbtn,tbtht,varsf,prodsf,conssf,aln,tstn,tstht,tsi,asi);
  tablespace_sm_used = CurrentTotalTableSpaceUsed(tbtn,tbtht,varsf,prodsf,conssf,aln,tstn,tstht,tsi,asi);

  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  //  de_count = (de_space_used - num_de_blocks * sizeof(Cell)) /	     sizeof(struct delay_element);

  dl_space_alloc = allocated_dl_space(current_dl_block_gl,&num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  //  dl_count = (dl_space_used - num_dl_blocks * sizeof(Cell)) /	     sizeof(struct delay_list);

  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();

  tablespace_sm_alloc = tablespace_sm_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  tablespace_sm_used = tablespace_sm_used + de_space_used + dl_space_used + pnde_space_used;

  abtn = node_statistics(&smAssertBTN);  abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc =    NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used  =    NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);

  gl_avail = (top_of_localstk - top_of_heap - 1) * sizeof(Cell);
  tc_avail = (top_of_cpstack - (CPtr)top_of_trail - 1) * sizeof(Cell);

    switch(type) {

    case TOTALMEMORY: {

  pspacetot = 0;
  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (i != TABLE_SPACE && i != INCR_TABLE_SPACE) pspacetot += pspacesize[i];

  total_alloc =     pspacetot  +  pspacesize[TABLE_SPACE] +    pspacesize[INCR_TABLE_SPACE] +
    (pdl.size + glstack.size + tcpstack.size + complstack.size) * K + de_space_alloc + dl_space_alloc  + pnde_space_alloc;

  total_used  =    pspacetot  +  pspacesize[TABLE_SPACE]-(tablespace_sm_alloc-tablespace_sm_used)
    - (trieassert_alloc - trieassert_used) +    pspacesize[INCR_TABLE_SPACE] +
    (glstack.size * K - gl_avail) + (tcpstack.size * K - tc_avail) +
    de_space_used + dl_space_used;


      ctop_int(CTXTc 4, total_alloc);
      ctop_int(CTXTc 5, total_used);
      break;
    }
    case GLMEMORY: {
      ctop_int(CTXTc 4, glstack.size *K);
      ctop_int(CTXTc 5, (glstack.size * K - gl_avail));
      break;
    }
    case TCMEMORY: {
      ctop_int(CTXTc 4, tcpstack.size * K);
      ctop_int(CTXTc 5, (tcpstack.size * K - tc_avail));
      break;
    }
    case TABLESPACE: {
      ctop_int(CTXTc 4, tablespace_sm_alloc);
      ctop_int(CTXTc 5, tablespace_sm_used);
      break;
    }
    case TRIEASSERTMEM: {
      ctop_int(CTXTc 4, trieassert_alloc);
      ctop_int(CTXTc 5, trieassert_used);
      break;
    }
    case HEAPMEM: {
      ctop_int(CTXTc 4,(Integer)((top_of_heap - (CPtr)glstack.low + 1)* sizeof(Cell)));
      break;
    }
    case CPMEM: {
      ctop_int(CTXTc 4, (Integer)(((CPtr)tcpstack.high - top_of_cpstack) * sizeof(Cell)));
      break;
    }
    case TRAILMEM: {
      ctop_int(CTXTc 4, (Integer)((top_of_trail - (CPtr *)tcpstack.low + 1) * sizeof(CPtr)));
      break;
    }
    case LOCALMEM: {
      ctop_int(CTXTc 4, (Integer)(((CPtr)glstack.high - top_of_localstk) * sizeof(Cell)));
      break;
    }
    case OPENTABLECOUNT: {
      ctop_int(CTXTc 4, ((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk) /   sizeof(struct completion_stack_frame));
      ctop_int(CTXTc 5, count_sccs(CTXT));
      break;
    }
    case ATOMMEM: {
      ctop_int(CTXTc 4, pspacesize[ATOM_SPACE]);
      break;
    }
    }
}


//-----------------------------------------------------------------------------------------------
void total_stat(CTXTdeclc double elapstime) {

  NodeStats
    tbtn,		/* Table Basic Trie Nodes */
    abtn,		/* Asserted Basic Trie Nodes */
    tstn,		/* Time Stamp Trie Nodes */
    aln,		/* Answer List Nodes */
    tsi,		/* Time Stamp Indices (Index Entries/Nodes) */
    varsf,		/* Variant Subgoal Frames */
    prodsf,		/* Subsumptive Producer Subgoal Frames */
    conssf,		/* Subsumptive Consumer Subgoal Frames */
    asi;		/* Answer Subst Info for conditional answers */
  //    tot_Key,            /* Keys used in incremental tabling */
  //    tot_CallNode,     tot_OutEdge,    tot_CallList,    tot_Call2List;

  HashStats
    tbtht,		/* Table Basic Trie Hash Tables */
    abtht,		/* Asserted Basic Trie Hash Tables */
    tstht;		/* Time Stamp Trie Hash Tables */
  
  size_t pnde_space_alloc, pnde_space_used, num_de_blocks, num_dl_blocks, num_pnde_blocks,    i;

  UInteger de_count, dl_count, de_space_alloc, de_space_used, total_alloc, total_used, 
    dl_space_alloc, dl_space_used, tablespace_sm_alloc, tablespace_sm_used, tablespace_sm_free, pspacetot, 
    //incr_tablespace_sm_alloc, incr_tablespace_sm_used,  
    trieassert_alloc, trieassert_used, tc_avail, gl_avail;
  UInteger total_table_space;

  tbtn = node_statistics(&smTableBTN);            tbtht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);     prodsf = subgoal_statistics(CTXTc &smProdSF);
  conssf = subgoal_statistics(CTXTc &smConsSF);   aln = node_statistics(&smALN);
  tstn = node_statistics(&smTSTN);                tstht = hash_statistics(CTXTc &smTSTHT);
  tsi = node_statistics(&smTSIN);                 asi = node_statistics(&smASI);
  tablespace_sm_alloc = CurrentTotalTableSpaceAlloc(tbtn,tbtht,varsf,prodsf,conssf,aln,tstn,tstht,tsi,asi);
  tablespace_sm_used = CurrentTotalTableSpaceUsed(tbtn,tbtht,varsf,prodsf, conssf,aln,tstn,tstht,tsi,asi);

  //  tot_CallNode  = node_statistics(&smCallNode);   tot_OutEdge   = node_statistics(&smOutEdge);
  //  tot_CallList  = node_statistics(&smCallList);   tot_Call2List = node_statistics(&smCall2List);
  //  tot_Key       = node_statistics(&smKey);
  //  incr_tablespace_sm_alloc = CurrentTotalIncrTableSpaceAlloc(tot_CallNode,tot_OutEdge,tot_CallList,tot_Call2List,tot_Key);
  //  incr_tablespace_sm_used = CurrentTotalIncrTableSpaceUsed(tot_CallNode,tot_OutEdge,tot_CallList,tot_Call2List,tot_Key);

  abtn = node_statistics(&smAssertBTN);           abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc =    NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used =    NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);

  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  de_count = (de_space_used - num_de_blocks * sizeof(Cell)) /	     sizeof(struct delay_element);

  dl_space_alloc = allocated_dl_space(current_dl_block_gl,&num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  dl_count = (dl_space_used - num_dl_blocks * sizeof(Cell)) /	     sizeof(struct delay_list);

  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();

  tablespace_sm_alloc = tablespace_sm_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  tablespace_sm_used = tablespace_sm_used + de_space_used + dl_space_used + pnde_space_used;
  tablespace_sm_free = tablespace_sm_alloc - tablespace_sm_used;

  gl_avail = (top_of_localstk - top_of_heap - 1) * sizeof(Cell);
  tc_avail = (top_of_cpstack - (CPtr)top_of_trail - 1) * sizeof(Cell);

  pspacetot = 0;
  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (i != TABLE_SPACE && i != INCR_TABLE_SPACE) pspacetot += pspacesize[i];

  total_table_space = pspacesize[TABLE_SPACE]+(pspacesize[INCR_TABLE_SPACE]-trieassert_alloc)+de_space_alloc+dl_space_alloc+pnde_space_alloc;

  total_alloc = pspacetot + total_table_space + trieassert_alloc +  (pdl.size + glstack.size + tcpstack.size + complstack.size) * K ;
  total_used = pspacetot + total_table_space + trieassert_used - tablespace_sm_free + 
    ((UInteger) ((size_t)(pdlreg+1) - (size_t)pdl.high) + glstack.size*K - gl_avail) + (tcpstack.size * K - tc_avail + complstack.size) ;

  printf("\n");
  printf("Memory (total)    %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 total_alloc, total_used, total_alloc - total_used);
  printf("  permanent space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 pspacetot + trieassert_alloc, pspacetot + trieassert_used, trieassert_alloc - trieassert_used);
  if (trieassert_alloc > 0)
    printf("    trie-asserted                        %15" Intfmt " in use, %15" Intfmt "\n",
	   trieassert_used,trieassert_alloc-trieassert_used);

  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (pspacesize[i] > 0 && i != TABLE_SPACE && i != INCR_TABLE_SPACE)
      printf("    %s                         %15" Intfmt "\n",pspace_cat[i],pspacesize[i]);

  printf("  glob/loc space  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 glstack.size * K, glstack.size * K - gl_avail, gl_avail);
  printf("    global                               %15" Intfmt "\n",
	 (Integer)((top_of_heap - (CPtr)glstack.low + 1) * sizeof(Cell)));
  printf("    local                                %15" Intfmt "\n",
	 (Integer)(((CPtr)glstack.high - top_of_localstk) * sizeof(Cell)));
  printf("  trail/cp space  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 tcpstack.size * K, tcpstack.size * K - tc_avail, tc_avail);
  printf("    trail                                %15" Intfmt "\n",
	 (Integer)((top_of_trail - (CPtr *)tcpstack.low + 1) * sizeof(CPtr)));
  printf("    choice point                         %15" Intfmt "\n",
	 (Integer)(((CPtr)tcpstack.high - top_of_cpstack) * sizeof(Cell)));
  printf("  SLG unific. space    %10" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 (UInteger) pdl.size * K, (UInteger) ((size_t)(pdlreg+1) - (size_t)pdl.high),
	 (UInteger) (pdl.size * K - ((size_t)(pdlreg+1)-(size_t)pdl.high))); 
  printf("  SLG completion  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 (UInteger)complstack.size * K,
	 (UInteger)COMPLSTACKBOTTOM - (UInteger)top_of_complstk,
	 (UInteger)complstack.size * K -
	 (UInteger) ((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk));
  if (((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk) > 0) {
    printf("        (%" Intfmt " incomplete table(s)",
	   (UInteger) (((UInteger)COMPLSTACKBOTTOM - (UInteger)top_of_complstk)/(COMPLFRAMESIZE*WORD_SIZE)));
    printf(" in %d SCCs)",count_sccs(CTXT));
    printf("\n");
  }
  printf("  SLG table space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",	
	 total_table_space, total_table_space - tablespace_sm_free, tablespace_sm_free);

  if (pspacesize[INCR_TABLE_SPACE]) 
    printf("  Incr table space                    %15" Intfmt " in use\n",
	   pspacesize[INCR_TABLE_SPACE]);
  printf("\n");

    if (flags[MAX_USAGE]) {
      /* Report Maximum Usages
         --------------------- */
      update_maximum_tablespace_stats(&tbtn,&tbtht,&varsf,&prodsf,&conssf,
				    &aln,&tstn,&tstht,&tsi,&asi);
    printf("  Maximum table space used:  %" Intfmt " bytes\n",
	   maximum_total_tablespace_usage());
    printf("\n");
  }

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  printf("Tabling Operations\n");
  printf("  %"UIntfmt" subsumptive call check/insert ops: %"UIntfmt" producers, %"UIntfmt" variants,\n"
	 "  %"UIntfmt" properly subsumed (%"UIntfmt" table entries), %"UIntfmt" used completed table.\n"
	 "  %"UIntfmt" relevant answer ident ops.  %"UIntfmt" consumptions via answer list.\n",
	 NumSubOps_CallCheckInsert,		NumSubOps_ProducerCall,
	 NumSubOps_VariantCall,			NumSubOps_SubsumedCall,
	 NumSubOps_SubsumedCallEntry,		NumSubOps_CallToCompletedTable,
	 NumSubOps_IdentifyRelevantAnswers,	NumSubOps_AnswerConsumption);
  {
    UInteger ttl_ops = ans_chk_ins + NumSubOps_AnswerCheckInsert,
	 	 ttl_ins = ans_inserts + NumSubOps_AnswerInsert;

    printf("  %" UIntfmt " variant call check/insert ops: %" UIntfmt " producers, %" UIntfmt" variants.\n"
	   "  %" UIntfmt" answer check/insert ops: %" UIntfmt " unique inserts, %"UIntfmt" redundant.\n",
	   subg_chk_ins, subg_inserts, subg_chk_ins - subg_inserts,
	   ttl_ops, ttl_ins, ttl_ops - ttl_ins);
  }

  if (de_count > 0) {
    printf("  %6" Intfmt " DEs in the tables (space: %5" Intfmt " bytes allocated, %5" Intfmt" in use)\n",
	   de_count, de_space_alloc, de_space_used);
    printf("  %6" Intfmt " DLs in the tables (space: %5" Intfmt " bytes allocated, %5" Intfmt" in use)\n",
	   dl_count, dl_space_alloc, dl_space_used);
    printf("\n");
  }

  if (total_call_node_count_gl) {
    printf("Total number of incremental subgoals created: %d\n",total_call_node_count_gl);
    if (current_call_node_count_gl) {
      printf("Currently %d incremental subgoals, %d dependency edges\n",
	     current_call_node_count_gl,current_call_edge_count_gl);
	}
  }

  if (abol_subg_ctr == 1)
    printf("  1 tabled subgoal explicitly abolished\n");
  else if (abol_subg_ctr > 1) 
    printf("  %" UIntfmt " tabled subgoals explicitly abolished\n",abol_subg_ctr);

  if (abol_pred_ctr == 1) 
    printf("  1 tabled predicate explicitly abolished\n");
  else if (abol_pred_ctr > 1) 
    printf("  %" UIntfmt " tabled predicates explicitly abolished\n",abol_pred_ctr);
  print_abolish_table_statistics();

#endif

  printf("\n");  print_gc_statistics();

  printf("Time: %.3f sec. cputime,  %.3f sec. elapsetime\n",
	 cputime_count_gl, elapstime);

}

/**********************************************************************/
#else /* Below, the MT version */
/**********************************************************************/

void stat_inusememory(CTXTdeclc double elapstime, int type) {

  NodeStats
    tbtn,		/* Table Basic Trie Nodes */
    abtn,		/* Asserted Basic Trie Nodes */
    aln,		/* Answer List Nodes */
    varsf,		/* Variant Subgoal Frames */
    asi,		/* Answer Subst Info for conditional answers */

    pri_tbtn,		/* Table Basic Trie Nodes */
    pri_tstn,		/* Time Stamp Trie Nodes */
    pri_aln,		/* Answer List Nodes */
    pri_asi,		/* Answer Subst Info for conditional answers */
    pri_tsi,		/* Time Stamp Indices (Index Entries/Nodes) */
    pri_varsf,		/* Variant Subgoal Frames */
    pri_prodsf,		/* Subsumptive Producer Subgoal Frames */
    pri_conssf;		/* Subsumptive Consumer Subgoal Frames */

  HashStats
    tbtht,		/* Table Basic Trie Hash Tables */
    abtht,		/* Asserted Basic Trie Hash Tables */

    pri_tbtht,		/* Table Basic Trie Hash Tables */
    pri_tstht;		/* Time Stamp Trie Hash Tables */
  
  size_t
    total_alloc, total_used,
    tablespace_sm_alloc, tablespace_sm_used,
    private_tablespace_sm_alloc, private_tablespace_sm_used,
    shared_tablespace_sm_alloc, shared_tablespace_sm_used,
    trieassert_alloc, trieassert_used,
    gl_avail, tc_avail,
    de_space_alloc, de_space_used,
    dl_space_alloc, dl_space_used,
    pnde_space_alloc, pnde_space_used,
    private_de_space_alloc, private_de_space_used,
    private_dl_space_alloc, private_dl_space_used,
    private_pnde_space_alloc, private_pnde_space_used,
    pspacetot;

  size_t
    num_de_blocks, num_dl_blocks, num_pnde_blocks,
    de_count, dl_count, private_de_count, private_dl_count,
    i;

  tbtn = node_statistics(&smTableBTN);
  tbtht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);
  aln = node_statistics(&smALN);
  asi = node_statistics(&smASI);

  pri_tbtn = node_statistics(&smTableBTN);
  pri_tbtht = hash_statistics(CTXTc &smTableBTHT);
  pri_varsf = subgoal_statistics(CTXTc &smVarSF);
  pri_aln = node_statistics(&smALN);
  pri_asi = node_statistics(&smASI);
  pri_prodsf = subgoal_statistics(CTXTc &smProdSF);
  pri_conssf = subgoal_statistics(CTXTc &smConsSF);
  pri_tstn = node_statistics(&smTSTN);
  pri_tstht = hash_statistics(CTXTc &smTSTHT);
  pri_tsi = node_statistics(&smTSIN);

  private_tablespace_sm_alloc = CurrentPrivateTableSpaceAlloc(pri_tbtn,pri_tbtht,pri_varsf,
							   pri_prodsf,
  		  	  	    pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,pri_asi);
  private_tablespace_sm_used = CurrentPrivateTableSpaceUsed(pri_tbtn,pri_tbtht,pri_varsf,
							   pri_prodsf,
  		  	  	    pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,pri_asi);

  shared_tablespace_sm_alloc = CurrentSharedTableSpaceAlloc(tbtn,tbtht,varsf,aln,asi);
  shared_tablespace_sm_used = CurrentSharedTableSpaceUsed(tbtn,tbtht,varsf,aln,asi);

  tablespace_sm_alloc = shared_tablespace_sm_alloc + private_tablespace_sm_alloc;
  tablespace_sm_used =  shared_tablespace_sm_used + private_tablespace_sm_used;

  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  de_count = (de_space_used - num_de_blocks * sizeof(Cell)) / sizeof(struct delay_element);

  dl_space_alloc = allocated_dl_space(current_dl_block_gl,&num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  dl_count = (dl_space_used - num_dl_blocks * sizeof(Cell)) / sizeof(struct delay_list);

  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();

  private_de_space_alloc = allocated_de_space(private_current_de_block,&num_de_blocks);
  private_de_space_used = private_de_space_alloc - unused_de_space_private(CTXT);
  private_de_count = (private_de_space_used - num_de_blocks * sizeof(Cell)) /
             sizeof(struct delay_element);

  private_dl_space_alloc = allocated_dl_space(private_current_dl_block,&num_dl_blocks);
  private_dl_space_used = private_dl_space_alloc - unused_dl_space_private(CTXT);
  private_dl_count = (private_dl_space_used - num_dl_blocks * sizeof(Cell)) /
             sizeof(struct delay_list);

  private_pnde_space_alloc = allocated_pnde_space(private_current_pnde_block,&num_pnde_blocks);
  private_pnde_space_used = private_pnde_space_alloc - unused_pnde_space_private(CTXT);

  tablespace_sm_alloc = tablespace_sm_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  tablespace_sm_used =  tablespace_sm_used + de_space_used + dl_space_used + pnde_space_alloc;  

  shared_tablespace_sm_alloc = shared_tablespace_sm_alloc + de_space_alloc + dl_space_alloc 
  			   + pnde_space_alloc;
  shared_tablespace_sm_used =  shared_tablespace_sm_used + de_space_used + dl_space_used 
  			    + pnde_space_used;

  private_tablespace_sm_alloc = private_tablespace_sm_alloc + private_de_space_alloc + 
    private_dl_space_alloc + private_pnde_space_alloc;

  private_tablespace_sm_used = private_tablespace_sm_used + private_de_space_used + 
    private_dl_space_used + private_pnde_space_used;

  abtn = node_statistics(&smAssertBTN);
  abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc =
    NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used =
    NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);

  gl_avail = (top_of_localstk - top_of_heap - 1) * sizeof(Cell);
  tc_avail = (top_of_cpstack - (CPtr)top_of_trail - 1) * sizeof(Cell);

  pspacetot = 0;
  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (i != TABLE_SPACE && i != INCR_TABLE_SPACE) pspacetot += pspacesize[i];

  total_alloc =
    pspacetot  +  pspacesize[TABLE_SPACE]  + pspacesize[INCR_TABLE_SPACE] +
    (pdl.size + glstack.size + tcpstack.size + complstack.size) * K +
    de_space_alloc + dl_space_alloc  + pnde_space_alloc;

  total_used  =
    pspacetot  +  pspacesize[TABLE_SPACE]-(tablespace_sm_alloc-tablespace_sm_used)
    - (trieassert_alloc - trieassert_used) +
    pspacesize[INCR_TABLE_SPACE] +
    (glstack.size * K - gl_avail) + (tcpstack.size * K - tc_avail) +
    de_space_used + dl_space_used;

    switch(type) {
	
    case TOTALMEMORY: {
      ctop_int(CTXTc 4, total_alloc);
      ctop_int(CTXTc 5, total_used);
      break;
    }
    case GLMEMORY: {
      ctop_int(CTXTc 4, glstack.size *K);
      ctop_int(CTXTc 5, (glstack.size * K - gl_avail));
      break;
    }
    case TCMEMORY: {
      ctop_int(CTXTc 4, tcpstack.size * K);
      ctop_int(CTXTc 5, (tcpstack.size * K - tc_avail));
      break;
    }
    case TABLESPACE: {
      ctop_int(CTXTc 4, private_tablespace_sm_alloc);
      ctop_int(CTXTc 5, private_tablespace_sm_used);
      break;
    }
    case TRIEASSERTMEM: {
      ctop_int(CTXTc 4, trieassert_alloc);
      ctop_int(CTXTc 5, trieassert_used);
      break;
    }
    case HEAPMEM: {
      ctop_int(CTXTc 4,(Integer)((top_of_heap - (CPtr)glstack.low + 1)* sizeof(Cell)));
      break;
    }
    case CPMEM: {
      ctop_int(CTXTc 4, (Integer)(((CPtr)tcpstack.high - top_of_cpstack) * sizeof(Cell)));
      break;
    }
    case TRAILMEM: {
      ctop_int(CTXTc 4, (Integer)((top_of_trail - (CPtr *)tcpstack.low + 1) * sizeof(CPtr)));
      break;
    }
    case LOCALMEM: {
      ctop_int(CTXTc 4, (Integer)(((CPtr)glstack.high - top_of_localstk) * sizeof(Cell)));
      break;
    }
    case OPENTABLECOUNT: {
      ctop_int(CTXTc 4, ((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk) / 
	       sizeof(struct completion_stack_frame));
      ctop_int(CTXTc 5, count_sccs(CTXT));
      break;
    }
    case SHARED_TABLESPACE: {
      ctop_int(CTXTc 4, shared_tablespace_sm_alloc);
      ctop_int(CTXTc 5, shared_tablespace_sm_used);
      break;
    }
    case ATOMMEM: {
      ctop_int(CTXTc 4, pspacesize[ATOM_SPACE]);
      break;
    }
    }

}

void total_stat(CTXTdeclc double elapstime) {

  NodeStats
    tbtn,		/* Table Basic Trie Nodes */
    abtn,		/* Asserted Basic Trie Nodes */
    aln,		/* Answer List Nodes */
    varsf,		/* Variant Subgoal Frames */
    asi,		/* Answer Substitution Info */

    pri_tbtn,		/* Private Table Basic Trie Nodes */
    pri_tstn,		/* Private Time Stamp Trie Nodes */
    pri_aln,		/* Private Answer List Nodes */
    pri_asi,		/* Private Answer Substitution Info */
    pri_tsi,		/* Private Time Stamp Indices (Index Entries/Nodes) */
    pri_varsf,		/* Private Variant Subgoal Frames */
    pri_prodsf,		/* Private Subsumptive Producer Subgoal Frames */
    pri_conssf;		/* Private Subsumptive Consumer Subgoal Frames */

  HashStats
    abtht,		/* Asserted Basic Trie Hash Tables */
    tbtht,		/* Table Basic Trie Hash Tables */

    pri_tbtht,		/* Table Basic Trie Hash Tables */
    pri_tstht;		/* Time Stamp Trie Hash Tables */
  
  size_t
    total_alloc, total_used,
    tablespace_sm_alloc, tablespace_sm_used,
    shared_tablespace_sm_alloc, shared_tablespace_sm_used,
    private_tablespace_sm_alloc, private_tablespace_sm_used,
    trieassert_alloc, trieassert_used,
    gl_avail, tc_avail,
    de_space_alloc, de_space_used,
    dl_space_alloc, dl_space_used,
    pnde_space_alloc, pnde_space_used,
    private_de_space_alloc, private_de_space_used,
    private_dl_space_alloc, private_dl_space_used,
    private_pnde_space_alloc, private_pnde_space_used,
    pspacetot;

  UInteger de_count;
  size_t
    num_de_blocks,num_dl_blocks,num_pnde_blocks,
    dl_count, private_de_count, private_dl_count, 
    i;

  tbtn = node_statistics(&smTableBTN);
  tbtht = hash_statistics(CTXTc &smTableBTHT);
  varsf = subgoal_statistics(CTXTc &smVarSF);
  aln = node_statistics(&smALN);
  asi = node_statistics(&smASI);

  pri_tbtn = node_statistics(private_smTableBTN);
  pri_tbtht = hash_statistics(CTXTc private_smTableBTHT);
  pri_varsf = subgoal_statistics(CTXTc private_smVarSF);
  pri_aln = node_statistics(private_smALN);
  pri_asi = node_statistics(private_smASI);
  pri_prodsf = subgoal_statistics(CTXTc private_smProdSF);
  pri_conssf = subgoal_statistics(CTXTc private_smConsSF);
  pri_tstn = node_statistics(private_smTSTN);
  pri_tstht = hash_statistics(CTXTc private_smTSTHT);
  pri_tsi = node_statistics(private_smTSIN);

  private_tablespace_sm_alloc = CurrentPrivateTableSpaceAlloc(pri_tbtn,pri_tbtht,pri_varsf,
							   pri_prodsf,
				  pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,pri_asi);
  private_tablespace_sm_used = CurrentPrivateTableSpaceUsed(pri_tbtn,pri_tbtht,pri_varsf,
							 pri_prodsf,
				 pri_conssf,pri_aln,pri_tstn,pri_tstht,pri_tsi,pri_asi);

  shared_tablespace_sm_alloc = CurrentSharedTableSpaceAlloc(tbtn,tbtht,varsf,aln,asi);
  shared_tablespace_sm_used = CurrentSharedTableSpaceUsed(tbtn,tbtht,varsf,aln,asi);

  tablespace_sm_alloc = shared_tablespace_sm_alloc + private_tablespace_sm_alloc;
  tablespace_sm_used =  shared_tablespace_sm_used + private_tablespace_sm_used;

  abtn = node_statistics(&smAssertBTN);
  abtht = hash_statistics(CTXTc &smAssertBTHT);
  trieassert_alloc =
    NodeStats_SizeAllocNodes(abtn) + HashStats_SizeAllocTotal(abtht);
  trieassert_used =
    NodeStats_SizeUsedNodes(abtn) + HashStats_SizeUsedTotal(abtht);

  gl_avail = (top_of_localstk - top_of_heap - 1) * sizeof(Cell);
  tc_avail = (top_of_cpstack - (CPtr)top_of_trail - 1) * sizeof(Cell);
  
  de_space_alloc = allocated_de_space(current_de_block_gl,&num_de_blocks);
  de_space_used = de_space_alloc - unused_de_space();
  de_count = (de_space_used - num_de_blocks * sizeof(Cell)) /
	     sizeof(struct delay_element);

  dl_space_alloc = allocated_dl_space(current_dl_block_gl,&num_dl_blocks);
  dl_space_used = dl_space_alloc - unused_dl_space();
  dl_count = (dl_space_used - num_dl_blocks * sizeof(Cell)) /
	     sizeof(struct delay_list);

  pnde_space_alloc = allocated_pnde_space(current_pnde_block_gl,&num_pnde_blocks);
  pnde_space_used = pnde_space_alloc - unused_pnde_space();

  private_de_space_alloc = allocated_de_space(private_current_de_block,&num_de_blocks);
  private_de_space_used = private_de_space_alloc - unused_de_space_private(CTXT);
  private_de_count = (private_de_space_used - num_de_blocks * sizeof(Cell)) /
	     sizeof(struct delay_element);

  private_dl_space_alloc = allocated_dl_space(private_current_dl_block,&num_dl_blocks);
  private_dl_space_used = private_dl_space_alloc - unused_dl_space_private(CTXT);
  private_dl_count = (private_dl_space_used - num_dl_blocks * sizeof(Cell)) /
	     sizeof(struct delay_list);

  private_pnde_space_alloc = allocated_pnde_space(private_current_pnde_block,&num_pnde_blocks);
  private_pnde_space_used = private_pnde_space_alloc - unused_pnde_space_private(CTXT);

  tablespace_sm_alloc = tablespace_sm_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  tablespace_sm_used =  tablespace_sm_used + de_space_used + dl_space_used + pnde_space_alloc;

  shared_tablespace_sm_alloc = shared_tablespace_sm_alloc + de_space_alloc + dl_space_alloc + pnde_space_alloc;
  shared_tablespace_sm_used =  shared_tablespace_sm_used + de_space_used + dl_space_used + pnde_space_used;

  private_tablespace_sm_alloc = private_tablespace_sm_alloc + private_de_space_alloc + 
    private_dl_space_alloc + private_pnde_space_alloc;

  private_tablespace_sm_used = private_tablespace_sm_used + private_de_space_used + 
    private_dl_space_used + private_pnde_space_used;

  pspacetot = 0;
  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (i != TABLE_SPACE) pspacetot += pspacesize[i];

  total_alloc =
    pspacetot  +  trieassert_alloc  +  pspacesize[TABLE_SPACE] +
    de_space_alloc + dl_space_alloc + pnde_space_alloc; 

  total_used  =
    pspacetot  +  trieassert_used  + 
    pspacesize[TABLE_SPACE]-(tablespace_sm_alloc-tablespace_sm_used) +
    de_space_used + dl_space_used;


  printf("\n");
  printf("Thread-shared memory for process:\n");
  printf("  permanent space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 pspacetot + trieassert_alloc, pspacetot + trieassert_used,
	 trieassert_alloc - trieassert_used);
  if (trieassert_alloc > 0)
    printf("    trie-asserted                     %15" Intfmt "         %15" Intfmt "\n",
	   trieassert_used,trieassert_alloc-trieassert_used);
  for (i=0; i<NUM_CATS_SPACE; i++) 
    if (pspacesize[i] > 0 && i != TABLE_SPACE)
      printf("    %s                      %15" Intfmt "\n",pspace_cat[i],pspacesize[i]);
  printf("  SLG table space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 pspacesize[TABLE_SPACE]-trieassert_alloc,  
	 pspacesize[TABLE_SPACE]-trieassert_alloc-(tablespace_sm_alloc-tablespace_sm_used),
	 tablespace_sm_alloc - tablespace_sm_used);
  printf("  Shared SLG table space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 shared_tablespace_sm_alloc,shared_tablespace_sm_used,
	 shared_tablespace_sm_alloc - shared_tablespace_sm_used);
  printf("Total             %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 total_alloc, total_used, total_alloc - total_used);
  printf("\n");

  printf("Thread-private memory thread %"Intfmt":\n",xsb_thread_id);
  printf("  glob/loc space  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 glstack.size * K, glstack.size * K - gl_avail, gl_avail);
  printf("    global                            %15" Intfmt " bytes\n",
	 (Integer)((top_of_heap - (CPtr)glstack.low + 1) * sizeof(Cell)));
  printf("    local                             %15" Intfmt " bytes\n",
	 (Integer)(((CPtr)glstack.high - top_of_localstk) * sizeof(Cell)));
  printf("  trail/cp space  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 tcpstack.size * K, tcpstack.size * K - tc_avail, tc_avail);
  printf("    trail                             %15" Intfmt " bytes\n",
	 (Integer)((top_of_trail - (CPtr *)tcpstack.low + 1) * sizeof(CPtr)));
  printf("    choice point                      %15" Intfmt " bytes\n",
	 (Integer)(((CPtr)tcpstack.high - top_of_cpstack) * sizeof(Cell)));
  printf("  SLG unific. space %10" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 pdl.size * K, (size_t)(pdlreg+1) - (size_t)pdl.high,
	 pdl.size * K - ((size_t)(pdlreg+1)-(size_t)pdl.high)); 
  printf("  SLG completion  %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 (size_t)complstack.size * K,
	 (size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk,
	 (size_t)complstack.size * K -
	 ((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk));
  if (((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk) > 0) {
    printf("        (%" Intfmt " incomplete table(s)",
	   ((size_t)COMPLSTACKBOTTOM - (size_t)top_of_complstk)/(COMPLFRAMESIZE*WORD_SIZE));
    printf(" in %d SCCs)",count_sccs(CTXT));
  }
  printf("\n");
  printf("  Private SLG table space %15" Intfmt " bytes: %15" Intfmt " in use, %15" Intfmt " free\n",
	 private_tablespace_sm_alloc,private_tablespace_sm_used,
	 private_tablespace_sm_alloc - private_tablespace_sm_used);
  printf("\n");
#ifdef GC
  print_gc_statistics(CTXT);
#endif

/* TLS: Max stack stuff is probably not real useful with multiple
   threads -- to even get it to work correcly you'd have to use locks.
   So omitted below.
*/

#if !defined(MULTI_THREAD) || defined(NON_OPT_COMPILE)
  printf("Tabling Operations (shared and all private tables)\n");
  printf("  %"UIntfmt" subsumptive call check/insert ops: %"UIntfmt" producers, %"UIntfmt" variants,\n"
	 "  %"UIntfmt" properly subsumed (%"UIntfmt" table entries), %"UIntfmt" used completed table.\n"
	 "  %"UIntfmt" relevant answer ident ops.  %"UIntfmt" consumptions via answer list.\n",
	 NumSubOps_CallCheckInsert,		NumSubOps_ProducerCall,
	 NumSubOps_VariantCall,			NumSubOps_SubsumedCall,
	 NumSubOps_SubsumedCallEntry,		NumSubOps_CallToCompletedTable,
	 NumSubOps_IdentifyRelevantAnswers,	NumSubOps_AnswerConsumption);
  {
    size_t ttl_ops = ans_chk_ins + NumSubOps_AnswerCheckInsert,
	  	  ttl_ins = ans_inserts + NumSubOps_AnswerInsert;

    printf("  %"UIntfmt" variant call check/insert ops: %"UIntfmt" producers, %"UIntfmt" variants.\n"
	   "  %"UIntfmt" answer check/insert ops: %"UIntfmt" unique inserts, %"UIntfmt" redundant.\n",
	   subg_chk_ins, subg_inserts, subg_chk_ins - subg_inserts,
	   ttl_ops, ttl_ins, ttl_ops - ttl_ins);
  }

  if (de_count > 0) {
    printf(" %6"UIntfmt" DEs in the tables (space: %5"UIntfmt" bytes allocated, %5"UIntfmt" in use)\n",
	   de_count, de_space_alloc, de_space_used);
    printf(" %6"UIntfmt" DLs in the tables (space: %5"UIntfmt" bytes allocated, %5"UIntfmt" in use)\n",
	   dl_count, dl_space_alloc, dl_space_used);
    printf("\n");
  }

    if (abol_subg_ctr == 1)
      printf("  1 tabled subgoal explicitly abolished\n");
    else if (abol_subg_ctr > 1) 
      printf("  %"UIntfmt" tabled subgoals explicitly abolished\n",abol_subg_ctr);

    if (abol_pred_ctr == 1) 
      printf("  1 tabled predicate explicitly abolished\n");
    else if (abol_pred_ctr > 1) 
      printf("  %"UIntfmt" tabled predicates explicitly abolished\n",abol_pred_ctr);

#endif

#ifdef SHARED_COMPL_TABLES
  printf("%"UIntfmt" thread suspensions have occured\n\n", num_suspends );
  printf("%"UIntfmt" deadlocks have occured\n\n", num_deadlocks );
#endif

  printf("Peak number of active user threads: %"UIntfmt"\n", max_threads_sofar );

  printf("%"UIntfmt" active user thread%s.\n",(UInteger)flags[NUM_THREADS],
	 (flags[NUM_THREADS]>1?"s":""));

  printf("Time: %.3f sec. cputime,  %.3f sec. elapsetime\n",
	 time_count, elapstime);
}
#endif

/*======================================================================*/

/*
 * Called when builtin statistics(0) is invoked.  Resets all operational
 * counts and max memory usage info.
 */

#ifndef MULTI_THREAD
void reset_stat_counters(void)
{
   reset_subsumption_stats();
   //   reset_maximum_tablespace_stats();
   ans_chk_ins = ans_inserts = 0;
   subg_chk_ins = subg_inserts = 0;
   abol_subg_ctr = abol_pred_ctr = abol_all_ctr = 0;   
   time_start_gl = cpu_time();
}
#else
void reset_stat_counters(void)
{
   ans_chk_ins = ans_inserts = 0;
   subg_chk_ins = subg_inserts = 0;
   time_start_gl = cpu_time();
#ifdef SHARED_COMPL_TABLES
   num_suspends = 0;
   num_deadlocks = 0;
#endif
   max_threads_sofar = flags[NUM_THREADS];
}

#endif

/*======================================================================*/

#ifndef MULTI_THREAD
void init_statistics(void)
{
  realtime_count_gl = real_time();
  reset_stat_counters();   /* init statistics. structures */
  cputime_count_gl = 0;
}
#else
void init_statistics(void)
{
  realtime_count_gl = real_time();
  reset_stat_counters();   /* init statistics. structures */
  time_start_gl = 0;
}

#endif

/*======================================================================*/
/*  Print statistics and measurements.					*/
/*======================================================================*/
/*
 * Called through builtins statistics/1 and statistics/0.
 * ( statistics :- statistics(1). )
 */
void print_statistics(CTXTdeclc int choice) {

  switch (choice) {

  case STAT_RESET:		   
#ifndef MULTI_THREAD
    realtime_count_gl = real_time();
    reset_stat_counters();	/* reset op-counts */
    break;
#else
    realtime_count_gl = real_time();
    break;
#endif

  case STAT_DEFAULT:		    /* Default use: Print Stack Usage and CPUtime: */
#ifndef MULTI_THREAD
    cputime_count_gl = (cpu_time() - time_start_gl);
#endif
    total_stat(CTXTc real_time()-realtime_count_gl);   /* print */
    break;

  case STAT_TABLE:		    /* Print Detailed Table Usage */
    print_detailed_tablespace_stats(CTXT);
    break;

  case 3:		    /* Print Detailed Table, Stack, and CPUtime */
#ifndef MULTI_THREAD
    cputime_count_gl += (cpu_time() - time_start_gl);
    total_stat(CTXTc real_time()-realtime_count_gl);
    print_detailed_tablespace_stats(CTXT);
    print_detailed_subsumption_stats();
    break;
#else
    fprintf(stdwarn,"statistics(3) not yet implemented for MT engine\n");
    break;
#endif
  case STAT_MUTEX:                  /* mutex use (if PROFILE_MUTEXES is defined) */
    print_mutex_use();
    print_mem_allocs("stat_mutex");
    break;
  case 5:
    dis(0); 
    break;		/* output memory image - data only; for debugging */
  case 6:
    dis(1); 
    break;		/* output memory image - data + text; for debugging */
#ifdef CP_DEBUG
  case 7:
    print_cp_backtrace();
    break;
#endif
  case STAT_ATOM:              /* print symbol/string statistics */
    symbol_table_stats();
    string_table_stats();
    break;
  }
}

/*======================================================================*/

// dont use register 2 -- used for return in sys_syscall
extern double realtime_count_gl; /* from subp.c */

void  get_statistics(CTXTdecl) {
  int type;
  type = (int)ptoc_int(CTXTc 3);
  switch (type) {
// runtime [since start of Prolog,since previous statistics] 
// CPU time used while executing, excluding time spent
// garbage collecting, stack shifting, or in system calls. 
  case RUNTIME: {
    double tot_cpu, incr_cpu;

    tot_cpu = cpu_time();
    incr_cpu = tot_cpu - last_cpu;
    last_cpu = tot_cpu;

    ctop_float(CTXTc 4, tot_cpu);
    ctop_float(CTXTc 5, incr_cpu);
    break;
  }
  case WALLTIME: {
    double tot_wall,this_wall,incr_wall;

    this_wall = real_time();
    tot_wall = this_wall - realtime_count_gl;

    if (!last_wall) last_wall = realtime_count_gl;
    incr_wall = this_wall - last_wall;
    last_wall = this_wall;

    ctop_float(CTXTc 4, tot_wall);
    ctop_float(CTXTc 5, incr_wall);
    break;
      }
  case SHARED_TABLESPACE: 
    {
#ifdef MULTI_THREAD
	statistics_inusememory(CTXTc type);
#else
	xsb_abort("statistics/2 with parameter shared_tables not supported in this configuration\n");
#endif 
	break;
      }
  case IDG_COUNTS: {
    ctop_int(CTXTc 4,current_call_node_count_gl);
    ctop_int(CTXTc 5,current_call_edge_count_gl);
    break;
  }

  case TABLE_OPS: {
    UInteger ttl_ops = ans_chk_ins + NumSubOps_AnswerCheckInsert,
	 	 ttl_ins = ans_inserts + NumSubOps_AnswerInsert;
    ctop_int(CTXTc 4,NumSubOps_CallCheckInsert);
    ctop_int(CTXTc 5,NumSubOps_ProducerCall);
    ctop_int(CTXTc 6,subg_chk_ins);
    ctop_int(CTXTc 7,subg_inserts);
    ctop_int(CTXTc 8,ttl_ops);
    ctop_int(CTXTc 9,ttl_ins);


  }
  default: {
      statistics_inusememory(CTXTc type);
      break;
    }

  }
}
