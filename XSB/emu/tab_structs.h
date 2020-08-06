/* File:      tab_structs.h
** Author(s): Swift, Sagonas, Rao, Freire, Johnson
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
** $Id: tab_structs.h,v 1.29 2013-05-06 21:10:25 dwarren Exp $
** 
*/

#ifndef __MACRO_XSB_H__
#define __MACRO_XSB_H__

#include "debug_xsb.h"

#include "basictypes.h"

/*----------------------------------------------------------------------*/

/* typedef struct subgoal_frame *VariantSF; */

/*===========================================================================*/

/*             
 * 		     Deleted Table Frame Records
 *                   ===========================
 *
 *    These records are used to hold pointers to abolished call and
 *    answer tries for table garbage collection.  These are effectively
 *    a union type, as they can hold deleted subgoals as well as
 *    deleted predicates.
 */     

typedef struct Deleted_Table_Frame *DelTFptr;
typedef struct Deleted_Table_Frame {
  BTNptr call_trie;		/* pointer to the root of the call trie */
  VariantSF subgoals;		/* chain of predicate's subgoals */
  byte     type;                /* subgoal or predicate */
  byte     mark;                /* Marked by scan of CP stack */
  byte     warn;                /* Issue warning message? */
  DelTFptr next_delTF;		/* pointer to next table info frame */
  DelTFptr next_pred_delTF;	/* pointer to next table info frame same pred */
  DelTFptr prev_delTF;		/* pointer to prev table info frame */
  DelTFptr prev_pred_delTF;	/* pointer to prev table info frame same pred */
} DeletedTableFrame;

#define DELETED_PREDICATE 0 
#define DELETED_SUBGOAL 1

#define DTF_Mark(pDTF)	           ( (pDTF)->mark )
#define DTF_Type(pDTF)	           ( (pDTF)->type )
#define DTF_Warn(pDTF)	           ( (pDTF)->warn )
#define DTF_CallTrie(pDTF)	   ( (pDTF)->call_trie )
#define DTF_Subgoals(pDTF)	   ( (pDTF)->subgoals )
#define DTF_Subgoal(pDTF)	   ( (pDTF)->subgoals )
#define DTF_NextDTF(pDTF)	   ( (pDTF)->next_delTF )
#define DTF_NextPredDTF(pDTF)	   ( (pDTF)->next_pred_delTF )
#define DTF_PrevDTF(pDTF)	   ( (pDTF)->prev_delTF )
#define DTF_PrevPredDTF(pDTF)	   ( (pDTF)->prev_pred_delTF )


/* Functions to create deltfs are inlined, and in tr_utils.c */

/* In macro below, need to reset DTF chain, and Pred-level DTF chain.
 * No mutexes, because it is called only during gc, w. only 1 active
 * thread. */
#define Free_Global_DelTF_Pred(pDTF,pTIF) {				\
  if (DTF_PrevDTF(pDTF) == 0) {						\
    deltf_chain_begin = DTF_NextDTF(pDTF);				\
  }									\
  else {								\
    DTF_NextDTF(DTF_PrevDTF(pDTF)) = DTF_NextDTF(pDTF);			\
  }									\
  if (DTF_NextDTF(pDTF) != 0) {						\
    DTF_PrevDTF(DTF_NextDTF(pDTF)) = DTF_PrevDTF(pDTF);			\
  }									\
  if (DTF_PrevPredDTF(pDTF) == 0) {					\
    TIF_DelTF(pTIF) = DTF_NextDTF(pDTF);				\
  }									\
  else {								\
    DTF_NextPredDTF(DTF_PrevPredDTF(pDTF)) = DTF_NextPredDTF(pDTF);	\
  }									\
  if (DTF_NextPredDTF(pDTF) != 0) {					\
    DTF_PrevPredDTF(DTF_NextPredDTF(pDTF)) = DTF_PrevPredDTF(pDTF);	\
  }									\
  mem_dealloc(pDTF,sizeof(DeletedTableFrame),TABLE_SPACE);		\
}

#define Free_Global_DelTF_Subgoal(pDTF,pTIF) {				\
  if (DTF_PrevDTF(pDTF) == 0) {						\
    deltf_chain_begin = DTF_NextDTF(pDTF);				\
  }									\
  else {								\
    DTF_NextDTF(DTF_PrevDTF(pDTF)) = DTF_NextDTF(pDTF);			\
  }									\
  if (DTF_NextDTF(pDTF) != 0) {						\
    DTF_PrevDTF(DTF_NextDTF(pDTF)) = DTF_PrevDTF(pDTF);			\
  }									\
  if (DTF_PrevPredDTF(pDTF) == 0) {					\
    TIF_DelTF(pTIF) = DTF_NextDTF(pDTF);				\
  }									\
  if (DTF_PrevPredDTF(pDTF) != 0) {					\
    DTF_NextPredDTF(DTF_PrevPredDTF(pDTF)) = DTF_NextPredDTF(pDTF);	\
  }									\
  if (DTF_NextPredDTF(pDTF) != 0) {					\
    DTF_PrevPredDTF(DTF_NextPredDTF(pDTF)) = DTF_PrevPredDTF(pDTF);	\
  }									\
  mem_dealloc(pDTF,sizeof(DeletedTableFrame),TABLE_SPACE);		\
}

#ifdef MULTI_THREAD
/* In macro below, need to reset DTF chain, and Pred-level DTF chain.
 * No mutexes, because it is called only during gc, w. only 1 active
 * thread. */
#define Free_Private_DelTF_Pred(pDTF,pTIF) {				\
  if (DTF_PrevDTF(pDTF) == 0) {						\
    private_deltf_chain_begin = DTF_NextDTF(pDTF);			\
  }									\
  else {								\
    DTF_NextDTF(DTF_PrevDTF(pDTF)) = DTF_NextDTF(pDTF);			\
  }									\
  if (DTF_NextDTF(pDTF) != 0) {						\
    DTF_PrevDTF(DTF_NextDTF(pDTF)) = DTF_PrevDTF(pDTF);			\
  }									\
  if (DTF_PrevPredDTF(pDTF) == 0) {					\
    TIF_DelTF(pTIF) = DTF_NextDTF(pDTF);				\
  }									\
  else {								\
    DTF_NextPredDTF(DTF_PrevPredDTF(pDTF)) = DTF_NextPredDTF(pDTF);	\
  }									\
  if (DTF_NextPredDTF(pDTF) != 0) {					\
    DTF_PrevPredDTF(DTF_NextPredDTF(pDTF)) = DTF_PrevPredDTF(pDTF);	\
  }									\
  mem_dealloc(pDTF,sizeof(DeletedTableFrame),TABLE_SPACE);		\
}

#define Free_Private_DelTF_Subgoal(pDTF,pTIF) {				\
  if (DTF_PrevDTF(pDTF) == 0) {						\
    private_deltf_chain_begin = DTF_NextDTF(pDTF);			\
  }									\
  else {								\
    DTF_NextDTF(DTF_PrevDTF(pDTF)) = DTF_NextDTF(pDTF);			\
  }									\
  if (DTF_NextDTF(pDTF) != 0) {						\
    DTF_PrevDTF(DTF_NextDTF(pDTF)) = DTF_PrevDTF(pDTF);			\
  }									\
  if (DTF_PrevPredDTF(pDTF) == 0) {					\
    TIF_DelTF(pTIF) = DTF_NextDTF(pDTF);				\
  }									\
  if (DTF_PrevPredDTF(pDTF) != 0) {					\
    DTF_NextPredDTF(DTF_PrevPredDTF(pDTF)) = DTF_NextPredDTF(pDTF);	\
  }									\
  if (DTF_NextPredDTF(pDTF) != 0) {					\
    DTF_PrevPredDTF(DTF_NextPredDTF(pDTF)) = DTF_PrevPredDTF(pDTF);	\
  }									\
  mem_dealloc(pDTF,sizeof(DeletedTableFrame),TABLE_SPACE);		\
}

#endif

/*===========================================================================*/

/*             
 * 		     Predicate Reference Records
 *                   ===========================
 *
 *    These records are used to hold some predicate-level information
 *    for dynamic code (non-trie asserted).  Prrefs are pointed to by
 *    the ep field of a PSC record and in turn point to ClRefs of each
 *    asserted clause and back to the PSC record itself.  Prrefs also
 *    contain fields for various GC information for the dynamic
 *    predicate and its clauses.
 */

typedef struct Deleted_Clause_Frame *DelCFptr;
typedef struct {
  Cell	Instr ;
  struct ClRefHdr *FirstClRef ;
  struct ClRefHdr *LastClRef ;
  Psc psc;                          // pointer to PSC
  int mark;                         // mark (for gc)
  //  int generation; 
  DelCFptr delcf;                      // delcf pointer
}	*PrRef, PrRefData ;

#define PrRef_Instr(PRREF)          ( (PRREF)->Instr )
#define PrRef_FirstClRef(PRREF)     ( (PRREF)->FirstClRef )
#define PrRef_LastClRef(PRREF)      ( (PRREF)->LastClRef )
//#define PrRef_Generation(PRREF)     ( (PRREF)->generation )
#define PrRef_Psc(PRREF)            ( (PRREF)->psc )
#define PrRef_Mark(PRREF)           ( (PRREF)->mark )
#define PrRef_DelCF(PRREF)          ( (PRREF)->delcf )

/* Can't use CTXTdeclc here because its included early in context.h */
#ifdef MULTI_THREAD
extern xsbBool assert_buff_to_clref_p(struct th_context *, prolog_term, byte, PrRef, int, prolog_term, int, ClRef *);
extern void c_assert_code_to_buff(struct th_context *, prolog_term);

#else
extern xsbBool assert_buff_to_clref_p(prolog_term, byte, PrRef, int, prolog_term, int, ClRef *);

extern void c_assert_code_to_buff(prolog_term);
#endif


/*===========================================================================*/

/*             
 * 		     Deleted Clause Frames
 *                   ===========================
 *
 *    These records are used to hold pointers to abolished dynamic
 *    predicates and clauses for dynamic clause garbage collection.
 *    These are effectively a union type, as they can hold deleted
 *    clauses as well as information about retractall-ed or abolished
 *    predicates.
 */     

typedef struct Deleted_Clause_Frame {
  PrRef prref;		        /* ptr to prref whose clauses are to be deleted*/
  ClRef clref;		        /* ptr to first CLref in chain */
  //  int generation;	        /* generation of retractalled prref*/
  Psc psc;		        /* pointer to psc of prref (necess?) */
  byte      type;               /* Prref or Clref */
  byte      mark;               /* Marked by scan of CP stack */
  DelCFptr next_delCF;		/* pointer to next DelCl frame */
  DelCFptr next_pred_delCF;	/* pointer to next DelCl frame same pred */
  DelCFptr prev_delCF;		/* pointer to prev DelCl frame */
  DelCFptr prev_pred_delCF;	/* pointer to prev DelCl frame same pred */
} DeletedClauseFrame;

#define DELETED_PRREF 0 
#define DELETED_CLREF 1

#define DCF_Mark(pDCF)	           ( (pDCF)->mark )
#define DCF_Type(pDCF)	           ( (pDCF)->type )
#define DCF_PrRef(pDCF)	           ( (pDCF)->prref )
#define DCF_ClRef(pDCF)	           ( (pDCF)->clref )
//#define DCF_Generation(pDCF)	   ( (pDCF)->generation )
#define DCF_PSC(pDCF)	           ( (pDCF)->psc )
#define DCF_NextDCF(pDCF)	   ( (pDCF)->next_delCF )
#define DCF_PrevDCF(pDCF)	   ( (pDCF)->prev_delCF )
#define DCF_NextPredDCF(pDCF)	   ( (pDCF)->next_pred_delCF )
#define DCF_PrevPredDCF(pDCF)	   ( (pDCF)->prev_pred_delCF )

#define DELCFs_PER_BLOCK  2048

/*===========================================================================*/

/*
 *                         Table Information Frame
 *                         =======================
 *
 *  Table Information Frames are created for each tabled predicate,
 *  allowing access to its calls and their associated answers.
 */

#include "table_status_defs.h"

/*
 *typedef enum Tabled_Evaluation_Method {
 *  VARIANT_TEM      = VARIANT_EVAL_METHOD,
 *  SUBSUMPTIVE_TEM  = SUBSUMPTIVE_EVAL_METHOD,
 * DISPATCH_BLOCK    = 3
 *} TabledEvalMethod;
 */

#define isSharedTIF(pTIF)   (TIF_EvalMethod(pTIF) != DISPATCH_BLOCK)
#define isPrivateTIF(pTIF)  (TIF_EvalMethod(pTIF) == DISPATCH_BLOCK)
#define isIncrementalTif(pTIF) (get_incr(TIF_PSC(pTIF)))

typedef byte TabledEvalMethod;

typedef struct Table_Info_Frame *TIFptr;
typedef struct Table_Info_Frame {
  Psc  psc_ptr;			/* pointer to the PSC record of the subgoal */
  byte method;	                /* eval pred using variant or subsumption? */
  byte mark;                    /* (bit) to indicate tif marked for gc */
  unsigned int visited:1;       /* (bit) to indicate tif visited during cascading deletes */
  unsigned int skip_forest_log:1;
  unsigned int unused:6;
  byte intern_strs;		/* (bit) to indicate if variant table interns ground terms */
  DelTFptr del_tf_ptr;          /* pointer to first deletion frame for pred */
  BTNptr call_trie;		/* pointer to the root of the call trie */
  VariantSF subgoals;		/* chain of predicate's subgoals */
  TIFptr next_tif;		/* pointer to next table info frame */ 
  unsigned int  subgoal_depth:16;
  unsigned int  answer_depth:16;
#ifdef MULTI_THREAD
  pthread_mutex_t call_trie_lock;
#endif
#ifdef SHARED_COMPL_TABLES
  pthread_cond_t compl_cond;
#endif
} TableInfoFrame;

#define TIF_PSC(pTIF)		   ( (pTIF)->psc_ptr )
#define TIF_DelTF(pTIF)	           ( (pTIF)->del_tf_ptr )
#define TIF_EvalMethod(pTIF)	   ( (pTIF)->method )
#define TIF_Mark(pTIF)	           ( (pTIF)->mark )
#define TIF_Visited(pTIF)          ( (pTIF)->visited )
#define TIF_SkipForestLog(pTIF)    ( (pTIF)->skip_forest_log )

#define TIF_Interning(pTIF)         ( (pTIF)->intern_strs )
#define TIF_CallTrie(pTIF)	   ( (pTIF)->call_trie )
#define TIF_Subgoals(pTIF)	   ( (pTIF)->subgoals )
#define TIF_NextTIF(pTIF)	   ( (pTIF)->next_tif )
#ifdef MULTI_THREAD
#define TIF_CALL_TRIE_LOCK(pTIF)   ( (pTIF)->call_trie_lock )
#endif
#ifdef SHARED_COMPL_TABLES
#define TIF_ComplCond(pTIF)   	   ( (pTIF)->compl_cond )
#endif
#define	TIF_SubgoalDepth(pTIF)      ( (pTIF)->subgoal_depth )
#define TIF_AnswerDepth(pTIF)       ( (pTIF)->answer_depth )


#define	gc_mark_tif(pTIF)   TIF_Mark(pTIF) = 0x1
#define	gc_unmark_tif(pTIF)   TIF_Mark(pTIF) = 0x0

/*
 * #define IsVariantPredicate(pTIF)		\
 *   ( TIF_EvalMethod(pTIF) == VARIANT_TEM )
 *
 * #define IsSubsumptivePredicate(pTIF)		\
 *   ( TIF_EvalMethod(pTIF) == SUBSUMPTIVE_TEM )
 */

#define IsVariantPredicate(pTIF)		\
   ( TIF_EvalMethod(pTIF) == VARIANT_EVAL_METHOD )

#define IsSubsumptivePredicate(pTIF)		\
 ( TIF_EvalMethod(pTIF) == SUBSUMPTIVE_EVAL_METHOD )

struct tif_list {
  TIFptr first;
  TIFptr last;
};
extern struct tif_list  tif_list;

/* TLS: New_TIF is now a function in tables.c */

#ifdef MULTI_THREAD
extern TIFptr New_TIF(struct th_context *,Psc);
#else
extern TIFptr New_TIF(Psc);
#endif

/* TLS: as of 8/05 tifs are freed only when abolishing a dynamic
   tabled predicate, (or when exiting a thread to abolish
   thread-private tables).  Otherwise, keep the TIF around. */

/* shared tifs use the global structure tif_list.  Thus, the
   sequential engine uses Free_Shared_Tif rather than
   Free_Private_TIF */

#ifdef MULTI_THREAD
#define free_call_trie_mutex(pTIF) 					\
{	pthread_mutex_destroy(&TIF_CALL_TRIE_LOCK(pTIF) );	}
#else
#define free_call_trie_mutex(pTIF)
#endif

#ifdef SHARED_COMPL_TABLES
#define free_tif_cond_var(pTIF) 					\
{	pthread_cond_destroy(&TIF_ComplCond(pTIF) );	}
#else
#define free_tif_cond_var(pTIF)
#endif
   
#define Free_Shared_TIF(pTIF) {						\
    TIFptr tTIF;							\
    SYS_MUTEX_LOCK( MUTEX_TABLE );					\
    tTIF = tif_list.first;						\
    if (tTIF ==  (pTIF)) {						\
      tif_list.first = TIF_NextTIF((pTIF));				\
      if  (tif_list.last == (pTIF)) tif_list.last = NULL;		\
    }									\
    else {								\
      while  (tTIF != NULL && TIF_NextTIF(tTIF) != (pTIF))		\
	tTIF =  TIF_NextTIF(tTIF);					\
      if (!tTIF) xsb_exit("Trying to free nonexistent TIF");	\
      if ((pTIF) == tif_list.last) tif_list.last = tTIF;		\
      TIF_NextTIF(tTIF) = TIF_NextTIF((pTIF));				\
    }									\
    SYS_MUTEX_UNLOCK( MUTEX_TABLE );					\
    delete_predicate_table(CTXTc pTIF,TRUE);				\
    free_call_trie_mutex(pTIF);						\
    free_tif_cond_var(pTIF);						\
    mem_dealloc((pTIF),sizeof(TableInfoFrame),TABLE_SPACE);		\
  }

#define Free_Private_TIF(pTIF) {					\
    TIFptr tTIF = private_tif_list.first;				\
    if (tTIF ==  (pTIF)) {						\
      private_tif_list.first = TIF_NextTIF((pTIF));			\
      if  (private_tif_list.last == (pTIF)) private_tif_list.last = NULL; \
    }									\
    else {								\
      while  (tTIF != NULL && TIF_NextTIF(tTIF) != (pTIF))		\
	tTIF =  TIF_NextTIF(tTIF);					\
      if (!tTIF) xsb_exit("Trying to free nonexistent TIF");	\
      if ((pTIF) == private_tif_list.last) private_tif_list.last = tTIF; \
      TIF_NextTIF(tTIF) = TIF_NextTIF((pTIF));				\
    }									\
    SET_TRIE_ALLOCATION_TYPE_PRIVATE();					\
    delete_predicate_table(CTXTc pTIF,TRUE);				\
    free_call_trie_mutex(pTIF);						\
    free_tif_cond_var(pTIF);						\
    mem_dealloc((pTIF),sizeof(TableInfoFrame),TABLE_SPACE);		\
  }

/*===========================================================================*/

/*
 *                         Table Dispatch Blocks
 *                         =======================
 *
 *  Table Information Frames are created in the multi-threaded engine
 *  to allow a single predicate to have multiple TIFs (and by
 *  extension tries), one for each thread.  A Table Dispatch Block
 *  stands between the outer TIF (to which tabletry(single)
 *  instructions point, and the per-predicate TIFs that are used to
 *  manage thread-private tables.
 */

struct TDispBlk_t { /* first two fields must be same as Table_Info_Frame for coercion! */
  Psc psc_ptr;
  byte method; /* == DISPATCH_BLOCK for disp block, VARIANT/SUB for TIF */
  byte mark;	                /* eval pred using variant or subsumption? */
  struct TDispBlk_t *PrevDB;
  struct TDispBlk_t *NextDB;
  int MaxThread;
  TIFptr Thread0;       /* should probably call this tifarray */
};
typedef struct TDispBlk_t *TDBptr;
 
#define TIF_DispatchBlock(pTIF)	   	((TDBptr) (pTIF)->psc_ptr )
#define TDB_MaxThread(pTDB)	   	( (pTDB)->MaxThread )
#define TDB_TIFArray(pTDB)         	( (&(pTDB)->Thread0) )
#define TDB_PrivateTIF(pTDB,tid_pos)    ( TDB_TIFArray(pTDB)[(tid_pos)] )

struct TDispBlkHdr_t {
  struct TDispBlk_t *firstDB;
  struct TDispBlk_t *lastDB;
};

/* If private predicate in MT engine, find the thread's private TIF,
   otherwise leave unchanged.

   TLS: took out check of MaxThread -- MaxThread is always set to max_threads_glc  */
#define  handle_dispatch_block(tip)					\
  if ( isPrivateTIF(tip) ) {						\
    TDBptr tdispblk;							\
    tdispblk = (TDBptr) tip;						\
    tip = TDB_PrivateTIF(tdispblk,xsb_thread_entry);			\
    if (!tip) {								\
      /* this may not be possible, as it may always be initted in get_tip? */\
      tip = New_TIF(CTXTc tdispblk->psc_ptr);			\
      TDB_PrivateTIF(tdispblk,xsb_thread_entry) = tip;			\
    }									\
  }

/*===========================================================================*/

typedef struct ascc_edge *EPtr;
typedef struct completion_stack_frame *ComplStackFrame;

/*----------------------------------------------------------------------*/
/*  Approximate Strongly Connected Component Edge Structure.		*/
/*----------------------------------------------------------------------*/

struct ascc_edge {
  ComplStackFrame ascc_node_ptr;
  EPtr next;
};

#define ASCC_EDGE_SIZE		(sizeof(struct ascc_edge)/sizeof(CPtr))

#define edge_to_node(e)		((EPtr)(e))->ascc_node_ptr
#define next_edge(e)		((EPtr)(e))->next

/*----------------------------------------------------------------------*/
/*  Completion Stack Structure (ASCC node structure).			*/
/*									*/
/*  NOTE: Please make sure that fields "DG_edges" and "DGT_edges" are	*/
/*	  the last fields of the structure, and each time you modify	*/
/*	  this structure you also update the definition of the		*/
/*	  "compl_stk_frame_field" array defined in file debug.c		*/
/*----------------------------------------------------------------------*/

#define DELAYED		-1

struct completion_stack_frame {
  VariantSF subgoal_ptr;
  ALNptr  del_ret_list;   /* to reclaim deleted returns */
  int     _level_num;
  int     visited;
#ifndef LOCAL_EVAL
  EPtr    DG_edges;
  EPtr    DGT_edges;
#endif
#ifdef CONC_COMPL
  CPtr	  ext_cons;
#endif
} ;

#define COMPLFRAMESIZE	(sizeof(struct completion_stack_frame)/sizeof(CPtr))
#define COMPLSTACKSIZE (COMPLSTACKBOTTOM - openreg)/COMPLFRAMESIZE

#define compl_subgoal_ptr(b)	((ComplStackFrame)(b))->subgoal_ptr
#define compl_level(b)		((ComplStackFrame)(b))->_level_num
#define compl_del_ret_list(b)	((ComplStackFrame)(b))->del_ret_list
#define compl_visited(b)	((ComplStackFrame)(b))->visited
#ifndef LOCAL_EVAL
#define compl_DG_edges(b)	((ComplStackFrame)(b))->DG_edges
#define compl_DGT_edges(b)	((ComplStackFrame)(b))->DGT_edges
#endif
#ifdef CONC_COMPL
#define compl_ext_cons(b)	((ComplStackFrame)(b))->ext_cons
#endif

#define prev_compl_frame(b)	(((CPtr)(b))+COMPLFRAMESIZE)
#define next_compl_frame(b)	(((CPtr)(b))-COMPLFRAMESIZE)

#define check_scc_subgoals_tripwire {					\
    if (subgoals_in_scc > flags[MAX_SCC_SUBGOALS]) { /* account for 0-index */ \
      if (flags[MAX_SCC_SUBGOALS_ACTION] == XSB_ERROR)			\
	xsb_abort("Tripwire max_scc_subgoals hit.  The user-set limit of %d incomplete subgoals within a single SCC" \
		  " has been exceeded\n",				\
		  flags[MAX_SCC_SUBGOALS]);				\
      else { /* flags[MAX_SCC_SUBGOALS_ACTION] == XSB_SUSPEND */	\
	/* printf("Debug: suspending on max_scc_subgoals\n"); */        \
	tripwire_interrupt(CTXTc "max_scc_subgoals_handler");		\
      }									\
    }									\
  }

#define adjust_level(CS_FRAME) {					\
    int new_level = compl_level(CS_FRAME);				\
    UInteger subgoals_in_scc = 0;					\
    if ( new_level < compl_level(openreg) ) {				\
      CPtr csf = CS_FRAME;						\
      while ( (csf >= openreg) && (compl_level(csf) >= new_level) ) {	\
	compl_level(csf) = new_level;					\
	csf = next_compl_frame(csf);					\
      }									\
      /* if flag is turned on, and complstacksize is "big" need to find leader */\
      if (COMPLSTACKSIZE > flags[MAX_SCC_SUBGOALS]) {   \
	csf = CS_FRAME;							\
	while (csf < COMPLSTACKBOTTOM && compl_level(csf) == new_level) { \
	  csf = prev_compl_frame(csf);					\
	} /* finding frame right before leader */			\
	subgoals_in_scc = (csf-openreg)/COMPLFRAMESIZE;			\
	/*	printf("subgoals in SCC: %d\n",subgoals_in_scc);*/	\
	check_scc_subgoals_tripwire;					\
      }									\
    }									\
  }									

/*
 *  The overflow test MUST be placed after the initialization of the
 *  ComplStackFrame in the current implementation.  This is so that the
 *  corresponding subgoal which points to this frame can be found and its
 *  link can be updated if an expansion is required.  This was the simplest
 *  solution to not leaving any dangling pointers to the old area.
 */

#define check_incomplete_subgoals_tripwire {	    \
    /*    printf("Level num %d active goals %d flag %d\n",level_num,COMPLSTACKSIZE,flags[MAX_INCOMPLETE_SUBGOALS]);*/ \
    /*    if ((UInteger)level_num > flags[MAX_INCOMPLETE_SUBGOALS] -1) { */ /* account for 0-index */ \
    if (COMPLSTACKSIZE > flags[MAX_INCOMPLETE_SUBGOALS]) { \
      if (flags[MAX_INCOMPLETE_SUBGOALS_ACTION] == XSB_ERROR)		\
	xsb_abort("Tripwire max_incomplete_subgoals hit.  The user-set limit of %d incomplete subgoals within a derivation" \
		  " has been exceeded\n",				\
		  flags[MAX_INCOMPLETE_SUBGOALS]);			\
      else { /* flags[MAX_INCOMPLETE_SUBGOALS_ACTION] == XSB_SUSPEND */	\
	/* printf("Debug: suspending on max_incomplete_subgoals\n"); */	\
	tripwire_interrupt(CTXTc "max_incomplete_subgoals_handler");	\
      }									\
    }									\
  }




#define push_completion_frame_common(subgoal) \
  check_incomplete_subgoals_tripwire; \
  level_num++; \
  openreg -= COMPLFRAMESIZE; \
  compl_subgoal_ptr(openreg) = subgoal; \
  compl_level(openreg) = level_num; \
  compl_del_ret_list(openreg) = NULL; \
  compl_visited(openreg) = FALSE

#define push_completion_frame_batched(subgoal) \
  compl_DG_edges(openreg) = compl_DGT_edges(openreg) = NULL

#ifdef LOCAL_EVAL
#define	push_completion_frame(subgoal)	\
  push_completion_frame_common(subgoal); \
  check_completion_stack_overflow
#else
#define	push_completion_frame(subgoal)	\
  push_completion_frame_common(subgoal); \
  push_completion_frame_batched(subgoal); \
  check_completion_stack_overflow
#endif

#if (!defined(LOCAL_EVAL))
#define compact_completion_frame(cp_frame,cs_frame,subgoal)	\
  compl_subgoal_ptr(cp_frame) = subgoal;			\
  compl_level(cp_frame) = compl_level(cs_frame);		\
  compl_visited(cp_frame) = FALSE;				\
  compl_DG_edges(cp_frame) = compl_DGT_edges(cp_frame) = NULL;  \
  cp_frame = next_compl_frame(cp_frame)
#endif

/*----------------------------------------------------------------------*/
/*  Subgoal (Call) Structure.						*/
/*----------------------------------------------------------------------*/

#include "slgdelay.h"

enum SubgoalFrameType {
  SHARED_VARIANT_PRODUCER_SFT        = 0x06,   /* binary 0110 */
  SHARED_SUBSUMPTIVE_PRODUCER_SFT    = 0x05,   /* binary 0101 */
  SHARED_SUBSUMED_CONSUMER_SFT       = 0x04,   /* binary 0100 */
  PRIVATE_VARIANT_PRODUCER_SFT       = 0x02,   /* binary 0010 */
  PRIVATE_SUBSUMPTIVE_PRODUCER_SFT   = 0x01,   /* binary 0001 */
  PRIVATE_SUBSUMED_CONSUMER_SFT      = 0x00    /* binary 0000 */
};

/* Private is default */
#define VARIANT_PRODUCER_SFT   0x02
#define SUBSUMPTIVE_PRODUCER_SFT   0x01
#define SUBSUMED_CONSUMER_SFT   0x00

#define VARIANT_SUBSUMPTION_MASK  0x03

#define SHARED_PRIVATE_MASK 0x04
#define SHARED_SFT 0x04
#define PRIVATE_SFT 0x00

/* --------------------------------
   Variant (Producer) Subgoal Frame

   Note that the cp_ptr, which is not needed until a table is
   complete, is reused as the pointer to the DelTF -- but the DelTF
   usage will occur only after the table has been completed, so its
   safe.

   Fields marked pre-compl are not needed after completion; post-compl
   Are not needed before completion.
   -------------------------------- */

typedef struct subgoal_frame {
  byte sf_type;		  /* The type of subgoal frame */
  byte is_complete;	  /* If producer, whether its answer set is complete */
  byte is_reclaimed:1;	  /* Whether structs for supporting answer res from an
			     incomplete table have been reclaimed */
  byte negative_initial_call:1;
  byte unused:6;
  byte visited;
  TIFptr tif_ptr;	  /* Table of which this call is a part */
  BTNptr leaf_ptr;	  /* Handle for call in the CallTrie */
  BTNptr ans_root_ptr;	  /* Root of the return trie */
  ALNptr ans_list_ptr;	  /* Pointer to the list of returns in the ret trie */
  ALNptr ans_list_tail;   /* pointer to the tail of the answer list */
  PNDE nde_list;	  /* pointer to a list of negative DEs */
  void *next_subgoal;	  
  void *prev_subgoal;
  CPtr  cp_ptr;		  /* Pointer to the Generator CP (cannot be used as union w. post-compl)*/
  union{ 
    CPtr pos_cons;	  /* Pointer to list of (CP) active subgoal frames (pre-compl) */
    DelTFptr deltf_ptr;     /* pointer to deltf (post-compl) */
  };
  union{
    CPtr compl_stack_ptr;	  /* Pointer to subgoal's completion stack frame (pre-compl) */
    long visitors;};
  CPtr compl_suspens_ptr; /* SLGWAM: CP stack ptr (pre-compl)  */
#ifdef MULTI_THREAD
  Thread_T tid;	  /* Thread id of the generator thread for this sg */
#endif
#ifdef CONC_COMPL
  ALNptr tag;		  /* Tag can't be stored in answer list in conc compl */
#endif
#ifdef SHARED_COMPL_TABLES
  byte grabbed; 	  /* Subgoal is marked to be computed for leader in
			     deadlock detection */
#endif
 /* The following field is added for incremental evaluation: */
  callnodeptr callnode;
#ifdef BITS64
  unsigned int callsto_number:32;
  unsigned int ans_ctr:32;
#else
  Integer callsto_number;    /* if 64 bits double-use call_ans_ctr */
  UInteger ans_ctr; 
#endif


} variant_subgoal_frame;

#define subg_sf_type(b)		( ((VariantSF)(b))->sf_type )
#define subg_is_complete(b)	( ((VariantSF)(b))->is_complete )
#define subg_is_reclaimed(b)	( ((VariantSF)(b))->is_reclaimed )
#define subg_prev_subgoal(b)	( ((VariantSF)(b))->prev_subgoal )
#define subg_next_subgoal(b)	( ((VariantSF)(b))->next_subgoal )
#define subg_tif_ptr(b)		( ((VariantSF)(b))->tif_ptr )
#define subg_leaf_ptr(b)	( ((VariantSF)(b))->leaf_ptr )
#define subg_ans_root_ptr(b)	( ((VariantSF)(b))->ans_root_ptr )
#define subg_ans_list_ptr(b)	( ((VariantSF)(b))->ans_list_ptr )
#define subg_ans_list_tail(b)	( ((VariantSF)(b))->ans_list_tail )
#define subg_cp_ptr(b)		( ((VariantSF)(b))->cp_ptr )
#define subg_deltf_ptr(b)     	( ((VariantSF)(b))->deltf_ptr )
/*#define subg_deltf_ptr(b)     	( (DelTFptr)((VariantSF)(b))->ans_list_tail )*/
#define subg_forest_log_off(b)       (TIF_SkipForestLog(subg_tif_ptr(b)))

#define subg_pos_cons(b)	( ((VariantSF)(b))->pos_cons )
#define subg_callnode_ptr(b)    ( ((VariantSF)(b))->callnode ) /* incremental evaluation */

/* use this for mark as completed == 0 */
#define subg_compl_stack_ptr(b)	( ((VariantSF)(b))->compl_stack_ptr )
#define subg_compl_susp_ptr(b)	( ((VariantSF)(b))->compl_suspens_ptr )
#define subg_nde_list(b)	( ((VariantSF)(b))->nde_list )
#define subg_call_ans_ctr(b)	( ((VariantSF)(b))->call_ans_ctr )

#define subg_tid(b)		( ((VariantSF)(b))->tid )
#define subg_tag(b)		( ((VariantSF)(b))->tag )
#define subg_grabbed(b)		( ((VariantSF)(b))->grabbed )
#define subg_callsto_number(b)		( ((VariantSF)(b))->callsto_number )
#define subg_ans_ctr(b)		( ((VariantSF)(b))->ans_ctr )
#define subg_visitors(b)	( ((VariantSF)(b))->visitors )
#define subg_negative_initial_call(b) 	( ((VariantSF)(b))->negative_initial_call )
/* The subgoal visited field can be used for both marking during GC
   (the GC_MARK mask) and during table traversal to transitively remove
   depending subgoals (the VISITED mask).  */
#define VISITED_SUBGOAL_MASK  1
#define VISITED_SUBGOAL_NEG_MASK 10
#define DELETED_SUBGOAL_MASK  2
#define GC_SUBGOAL_MASK  8
#define GC_SUBGOAL_NEG_MASK  3

#define VISITED_SUBGOAL(subgoal)  ((subgoal->visited) & VISITED_SUBGOAL_MASK)
#define MARK_VISITED_SUBGOAL(subgoal) {(subgoal->visited) = VISITED_SUBGOAL_MASK | (subgoal->visited);}
//Unmarked are not yet used
#define UNMARK_VISITED_SUBGOAL(subgoal) {(subgoal->visited) = (subgoal->visited) & (VISITED_SUBGOAL_NEG_MASK);}
#define GC_MARKED_SUBGOAL(subgoal)  ((subgoal->visited) & GC_SUBGOAL_MASK)
#define GC_MARK_SUBGOAL(subgoal) {(subgoal->visited) = GC_SUBGOAL_MASK | (subgoal->visited);}
#define GC_UNMARK_SUBGOAL(subgoal) {(subgoal->visited) =  (subgoal->visited) & GC_SUBGOAL_NEG_MASK;}

#define DELETED_SUBGOAL_FRAME(subgoal)  ((subgoal->visited) & DELETED_SUBGOAL_MASK)
#define DELETE_SUBGOAL_FRAME(subgoal) {(subgoal->visited) = DELETED_SUBGOAL_MASK | (subgoal->visited);}

#define is_completed(SUBG_PTR)		(subg_is_complete(SUBG_PTR) & 1)
#define subg_is_completed(SUBG_PTR)		(subg_is_complete(SUBG_PTR) & 1)
#define complete_subg(SUBG_PTR)    subg_is_complete((SUBG_PTR)) |= 1

#define subg_is_answer_completed(SUBG_PTR) !(subg_is_complete(SUBG_PTR) & 4)
#define answer_complete_subg(SUBG_PTR) subg_is_complete((SUBG_PTR)) &= ~4
#define subg_needs_answer_completion(SUBG_PTR) subg_is_complete((SUBG_PTR)) |= 4

#define subg_is_ec_scheduled(SUBG_PTR)		(subg_is_complete(SUBG_PTR) & 2)
#define schedule_ec(SUBG_PTR)                   subg_is_complete(SUBG_PTR) |= 2

#define SUBG_INCREMENT_CALLSTO_SUBGOAL(subgoal)  (subgoal -> callsto_number)++
#define INIT_SUBGOAL_CALLSTO_NUMBER(subgoal) subg_callsto_number(subgoal)  = 1

#define SUBG_INCREMENT_ANSWER_CTR(subgoal) {	\
    if (++subg_ans_ctr(subgoal) > (unsigned) flags[MAX_ANSWERS_FOR_SUBGOAL]) { \
      if (flags[MAX_ANSWERS_FOR_SUBGOAL_ACTION] == XSB_ERROR) {		\
	sprint_subgoal(CTXTc forest_log_buffer_1,0, subgoal);		\
xsb_abort("Tripwire max_answers_for_subgoal hit. The user-set limit on the number of %d answers for a single subgoaol has been exceeded for %s\n",flags[MAX_ANSWERS_FOR_SUBGOAL],forest_log_buffer_1->fl_buffer); \
      }									\
      else { /* flags[MAX_SCC_SUBGOALS_ACTION] == XSB_SUSPEND */	\
	tripwire_interrupt(CTXTc "max_answers_for_subgoal_handler");	\
      }									\
    }									\
  }




/* Subsumptive Producer Subgoal Frame
   ---------------------------------- */
typedef struct SubsumedConsumerSubgoalFrame *SubConsSF;
typedef struct SubsumptiveProducerSubgoalFrame *SubProdSF;
typedef struct SubsumptiveProducerSubgoalFrame {
  variant_subgoal_frame  var_sf;
  SubConsSF  consumers;		/* List of properly subsumed subgoals which
				   consume from a producer's answer set */
} subsumptive_producer_sf;

#define subg_consumers(SF)	((SubProdSF)(SF))->consumers


/* Subsumed Consumer Subgoal Frame
 * -------------------------------
 *  Position of shared fields MUST correspond to that of variant_subgoal_frame.
 */
typedef struct SubsumedConsumerSubgoalFrame {
  byte sf_type;		   /* The type of subgoal frame */
  byte junk[2];
  TIFptr tif_ptr;	   /* Table of which this call is a part */
  BTNptr leaf_ptr;	   /* Handle for call in the CallTrie */
  SubProdSF producer;	   /* The subgoal frame from whose answer set answers
			      are collected into the answer list */
  ALNptr ans_list_ptr;	   /* Pointer to the list of returns in the ret trie */
  ALNptr ans_list_tail;	   /* Pointer to the tail of the answer list */
  PNDE nde_list;	  /* pointer to a list of negative DEs */
  TimeStamp ts;		   /* Time stamp to use during next answer ident */
  SubConsSF consumers;	   /* Chain link for properly subsumed subgoals */
} subsumptive_consumer_sf;

#define conssf_producer(SF)	((SubConsSF)(SF))->producer
#define conssf_timestamp(SF)	((SubConsSF)(SF))->ts
#define conssf_consumers(SF)	((SubConsSF)(SF))->consumers


/* beginning of REAL answers in the answer list */
#define subg_answers(subg)	ALN_Next(subg_ans_list_ptr(subg))

#define IsVariantSF(pSF) \
  ((subg_sf_type(pSF) & VARIANT_SUBSUMPTION_MASK) == VARIANT_PRODUCER_SFT)
#define IsSubProdSF(pSF) \
  ((subg_sf_type(pSF) & VARIANT_SUBSUMPTION_MASK) == SUBSUMPTIVE_PRODUCER_SFT)
#define IsSubConsSF(pSF) \
  ((subg_sf_type(pSF) & VARIANT_SUBSUMPTION_MASK) == SUBSUMED_CONSUMER_SFT)

/* incremental evaluation */
#define IsIncrSF(pSF) \
  (IsNonNULL(subg_callnode_ptr(pSF)))

#define IsNonIncrSF(pSF) \
  (IsNULL(subg_callnode_ptr(pSF)))


#define IsPrivateSF(pSF) \
  ((subg_sf_type(pSF) & SHARED_PRIVATE_MASK) == PRIVATE_SFT)
#define IsSharedSF(pSF) \
  ((subg_sf_type(pSF) & SHARED_PRIVATE_MASK) == SHARED_SFT)

#define IsVariantProducer(pSF)		IsVariantSF(pSF)
#define IsSubsumptiveProducer(pSF)	IsSubProdSF(pSF)
#define IsProperlySubsumed(pSF)		IsSubConsSF(pSF)

#define IsProducingSubgoal(pSF)		\
   ( IsVariantProducer(pSF) || IsSubsumptiveProducer(pSF) )

#define ProducerSubsumesSubgoals(pSF)	\
   ( IsSubsumptiveProducer(pSF) && IsNonNULL(subg_consumers(pSF)) )


/* Doubly-linked lists of Producer Subgoal Frames
 * ----------------------------------------------
 * Manipulating a doubly-linked list maintained through fields
 * `prev' and `next'.
 */

#define subg_dll_add_sf(pSF,Chain,NewChain) {	\
   subg_prev_subgoal(pSF) = NULL;		\
   subg_next_subgoal(pSF) = Chain;		\
   if ( IsNonNULL(Chain) )			\
     subg_prev_subgoal(Chain) = pSF;		\
   NewChain = (VariantSF)pSF;				\
 }

/* Need to be careful here.  In general we want to unchain this
   subgoal frame from the chain of subgoal frames -- however, if this
   subgoal frame is from an old generation of the table, we dont want
   to update TIF_Subgoals (chain,newchain) to point to the
   invalidated, "next" frame for this subgoal */
#define subg_dll_remove_sf(pSF,Chain,NewChain) {			 \
   if ( IsNonNULL(subg_prev_subgoal(pSF)) ) {				 \
     subg_next_subgoal(subg_prev_subgoal(pSF)) = subg_next_subgoal(pSF); \
     NewChain = Chain;							 \
   }									 \
   else	{								\
     if (pSF == TIF_Subgoals(subg_tif_ptr(pSF)))			\
       NewChain = (VariantSF)subg_next_subgoal(pSF);			\
   }									\
   if ( IsNonNULL(subg_next_subgoal(pSF)) )				 \
     subg_prev_subgoal(subg_next_subgoal(pSF)) = subg_prev_subgoal(pSF); \
   subg_prev_subgoal(pSF) = subg_next_subgoal(pSF) = NULL;		 \
 }

#ifndef MULTI_THREAD
extern ALNptr empty_return(VariantSF);
#define empty_return_handle(SF) empty_return(SF)
#else
extern ALNptr empty_return(struct th_context *,VariantSF);
#define empty_return_handle(SF) empty_return(th,SF)
#endif

/* tags for ALP pointers in leaf nodes, for faster delete */
#define HasALNPtr 0x2
#define untag_as_ALNptr(Node) ((ALNptr)((word)(Child(Node)) & ~HasALNPtr))
#define hasALNtag(Node) ((word)(Child(Node)) & HasALNPtr)

/* Appending to the Answer List of a SF
   ------------------------------------ */
#define SF_AppendNewAnswerList(pSF,pAnsList) {	\
						\
   ALNptr pLast;				\
						\
   pLast = pAnsList;				\
   while ( IsNonNULL(ALN_Next(pLast)) )		\
     pLast = ALN_Next(pLast);			\
   SF_AppendToAnswerList(pSF,pAnsList,pLast);	\
 }

#define SF_AppendNewAnswer(pSF,pAns)	SF_AppendToAnswerList(pSF,pAns,pAns)

#define SF_AppendToAnswerList(pSF,pHead,pTail) {			\
   if ( has_answers(pSF) ) {						\
     /*
      *  Insert new answer at the end of the answer list.
      */								\
     ALN_Next(subg_ans_list_tail(pSF)) = pHead; 			\
     Child(ALN_Answer(pHead)) = (NODEptr) ((word)(subg_ans_list_tail(pSF)) | HasALNPtr); \
   } else {								\
     /*
      * The dummy answer list node is the only node currently in the list.
      * It's pointed to by the head ptr, but the tail ptr is NULL.
      */								\
     ALN_Next(subg_ans_list_ptr(pSF)) = pHead;				\
   }									\
   subg_ans_list_tail(pSF) = pTail;					\
 }


/* Global Structure Management
   --------------------------- */
#define SUBGOAL_FRAMES_PER_BLOCK    128

extern struct Structure_Manager smVarSF;
extern struct Structure_Manager smProdSF;
extern struct Structure_Manager smConsSF;


/* Subgoal Frame (De)Allocation
   ---------------------------- */

/* NewProducerSF() is now a function, in tables.c */
#define FreeProducerSF(SF) {					\
    release_any_pndes(CTXTc subg_nde_list(SF));			\
    subg_dll_remove_sf(SF,TIF_Subgoals(subg_tif_ptr(SF)),	\
			 TIF_Subgoals(subg_tif_ptr(SF)));	\
    if ( IsVariantSF(SF) ) {					\
      if (IsSharedSF(SF)) {					\
	SM_DeallocateSharedStruct(smVarSF,SF);			\
      } else {							\
	SM_DeallocateStruct(smVarSF,SF);			\
      }								\
    }								\
    else							\
      SM_DeallocateStruct(smProdSF,SF);				\
  }


/*
 *  Allocates and initializes a subgoal frame for a consuming subgoal: a
 *  properly subsumed call consuming from an incomplete producer.
 *  Consuming subgoals are NOT inserted into the subgoal chain
 *  maintained by the TIF, but instead are maintained by the producer in
 *  a private linked list.  Many fields of a consumer SF are left blank
 *  since it won't be used in the same way as those for producers.  Its
 *  main purpose is to maintain the answer list and the call form.  Just
 *  as for the producer, an answer-list node is allocated for pointing
 *  to a dummy answer node and inserted into the answer list.
 *
 *  Finally, some housekeeping is needed to support lazy creation of the
 *  auxiliary structures in the producer's answer TST.  If this is the
 *  first consumer for this producer, then create these auxiliary
 *  structures.
 */

#ifndef MULTI_THREAD
void tstCreateTSIs(TSTNptr);
#define     tstCreateTSIs_handle(Producer) tstCreateTSIs(Producer) 
#else
void tstCreateTSIs(struct th_context *,TSTNptr);
#define     tstCreateTSIs_handle(Producer) tstCreateTSIs(th,Producer) 
#endif

/* Private structure manager for subsumptive tables: no shared
   allocation needed */
#define NewSubConsSF(SF,Leaf,TableInfo,Producer) {		\
								\
   void *pNewSF;						\
								\
   SM_AllocateStruct(smConsSF,pNewSF);				\
   pNewSF = memset(pNewSF,0,sizeof(subsumptive_consumer_sf));	\
   subg_sf_type(pNewSF) = SUBSUMED_CONSUMER_SFT;		\
   subg_tif_ptr(pNewSF) = TableInfo;				\
   subg_leaf_ptr(pNewSF) = Leaf;				\
   CallTrieLeaf_SetSF(Leaf,pNewSF);				\
   conssf_producer(pNewSF) = (SubProdSF)Producer;		\
   if ( ! ProducerSubsumesSubgoals(Producer) )			\
     tstCreateTSIs_handle((TSTNptr)subg_ans_root_ptr(Producer));	\
   subg_ans_list_ptr(pNewSF) = empty_return_handle(pNewSF);		\
   conssf_timestamp(pNewSF) = CONSUMER_SF_INITIAL_TS;		\
   conssf_consumers(pNewSF) = subg_consumers(Producer);		\
   subg_consumers(Producer) = (SubConsSF)pNewSF;		\
   SF = (VariantSF)pNewSF;					\
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#define set_min(a,b,c)	if (b < c) a = b; else a = c

#define tab_level(SUBG_PTR)     \
        compl_level((subg_compl_stack_ptr(SUBG_PTR)))
#define next_tab_level(CSF_PTR) \
        compl_level(prev_compl_frame(CSF_PTR))

#define is_leader(CSF_PTR)				\
  (prev_compl_frame(CSF_PTR) >= COMPLSTACKBOTTOM	\
   || next_tab_level(CSF_PTR) < compl_level(CSF_PTR))

/*----------------------------------------------------------------------*/
/* Codes for completed subgoals (assigned to subg_answers)              */
/*----------------------------------------------------------------------*/

#define NO_ANSWERS	(ALNptr)0
#define UNCOND_ANSWERS	(ALNptr)1
#define COND_ANSWERS	(ALNptr)2
#define INCOMP_ANSWERS	(ALNptr)3

/*----------------------------------------------------------------------*/
/* The following 2 macros are to be used for incomplete subgoals.	*/
/*----------------------------------------------------------------------*/

#define has_answers(SUBG_PTR)	    IsNonNULL(subg_answers(SUBG_PTR))
#define has_no_answers(SUBG_PTR)    IsNULL(subg_answers(SUBG_PTR))

/*----------------------------------------------------------------------*/
/* The following 5 macros should be used only for completed subgoals.	*/
/*----------------------------------------------------------------------*/

/*
 * These defs depend on when the root of an answer trie is created.
 * Currently, this is when the first answer is added to the set.  They
 * are also dependent upon representation of the truth of ground goals.
 * Currently, true subgoals have an ESCAPE node placed below the root,
 * while false goals have no root nor leaves (since no answer was ever
 * inserted, there was no opportunity to create a root).
 */
#define has_answer_code(SUBG_PTR)				\
	( IsNonNULL(subg_ans_root_ptr(SUBG_PTR)) &&		\
	  IsNonNULL(BTN_Child(subg_ans_root_ptr(SUBG_PTR))) )

#define subgoal_fails(SUBG_PTR)			\
	( ! has_answer_code(SUBG_PTR) )

/* should only be used on ground subgoals (is for escape node inspection) */
#define subgoal_unconditionally_succeeds(SUBG_PTR)			    \
        ( has_answer_code(SUBG_PTR) &&					    \
	  is_unconditional_answer(BTN_Child(subg_ans_root_ptr(SUBG_PTR))) )

#define mark_subgoal_failed(SUBG_PTR)	\
	(subg_ans_root_ptr(SUBG_PTR) = NULL)

#define neg_simplif_possible(SUBG_PTR)	\
	((subgoal_fails(SUBG_PTR)) && (subg_nde_list(SUBG_PTR) != NULL))

#define subsumed_subgoal_fails(SUBG_PTR) \
  (IsSubConsSF(SUBG_PTR) && IsDeletedNode(ALN_Answer(subg_ans_list_ptr(SUBG_PTR))) && \
   !ALN_Next(subg_ans_list_ptr(SUBG_PTR)))

#define subsumed_neg_simplif_possible(SUBG_PTR) \
  ((subsumed_subgoal_fails(SUBG_PTR)) && (subg_nde_list(SUBG_PTR) != NULL))

/*----------------------------------------------------------------------*/

#ifndef MULTI_THREAD   
#define mark_as_completed(SUBG_PTR) {		\
    if (  !subg_is_completed(SUBG_PTR) ) { \
      complete_subg(SUBG_PTR);			\
      reclaim_del_ret_list(SUBG_PTR);		\
    }						\
  }
#else
#define mark_as_completed(SUBG_PTR) {		\
    if (  !subg_is_completed(SUBG_PTR)) {	\
      complete_subg(SUBG_PTR);			\
      reclaim_del_ret_list(th, SUBG_PTR);	\
    }						\
  }
#endif

#define subgoal_space_has_been_reclaimed(SUBG_PTR,CS_FRAME) \
        (SUBG_PTR != compl_subgoal_ptr(CS_FRAME))

/* TLS: the "visited" fields are only used by batched, not local.
   Mainly, the neg_loop field is set to true to indicate that the
   subgoal will be delayed upon resuming */
#define mark_delayed(csf1, csf2, susp) { \
	  compl_visited(csf1) = DELAYED; \
	  compl_visited(csf2) = DELAYED; \
  /* do not put TRUE but some !0 value that GC can recognize as int tagged */\
	  csf_neg_loop(susp) = XSB_INT; \
        }

/*----------------------------------------------------------------------*/
/* The following macro might be called more than once for some subgoal. */
/* So, please make sure that functions/macros that it calls are robust  */
/* under repeated uses.                                 - Kostis.       */
/* A new Subgoal Frame flag prevents multiple calls.	- Ernie		*/
/*----------------------------------------------------------------------*/

#ifndef MULTI_THREAD
#define reclaim_incomplete_table_structs(SUBG_PTR) {	\
   if ( ! subg_is_reclaimed(SUBG_PTR) ) {		\
     table_complete_entry(SUBG_PTR);			\
     subg_is_reclaimed(SUBG_PTR) = TRUE;		\
   }							\
 }
#else
#define reclaim_incomplete_table_structs(SUBG_PTR) {	\
   if ( ! subg_is_reclaimed(SUBG_PTR) ) {		\
     table_complete_entry(th, SUBG_PTR);		\
     subg_is_reclaimed(SUBG_PTR) = TRUE;		\
   }							\
 }
#endif

/*----------------------------------------------------------------------*/
#ifdef DEMAND
#define Reset_Demand_Freeze_Registers \
    bdfreg = bfreg; \
    trdfreg = trfreg; \
    hdfreg = hfreg; \
    edfreg = efreg
#else
#define Reset_Demand_Freeze_Registers 
#endif


/* Use this when backtracking out of the completed leader of an SCC.  
   This doesn't automatically reset hfreg to the bottom of the stack to protect the list constructed during 
   List creation in ISO incremental tabling */
#define reset_freeze_registers_backtrack(tcp)		\
  /*  printf("reset freeze registers\n");	*/	\
    bfreg = (CPtr)(tcpstack.high) - CP_SIZE; \
    trfreg = (CPtr *)(tcpstack.low); \
    /* hfreg = (CPtr)(glstack.low);			*/	 \
    if (hfreg > tcp_hfreg(tcp)) { hfreg = tcp_hfreg(tcp); }	 \
    /*    if (hfreg != (CPtr)(glstack.low)) printf("... hfreg not fully reset\n");*/ \
    efreg = (CPtr)(glstack.high) - 1; \
    level_num = xwammode = 0; \
    root_address = ptcpreg = NULL; \
    Reset_Demand_Freeze_Registers

#define reset_freeze_registers		\
    bfreg = (CPtr)(tcpstack.high) - CP_SIZE; \
    trfreg = (CPtr *)(tcpstack.low); \
    hfreg = (CPtr)(glstack.low);      \
    efreg = (CPtr)(glstack.high) - 1; \
    level_num = xwammode = 0; \
    root_address = ptcpreg = NULL; \
    Reset_Demand_Freeze_Registers

#define adjust_freeze_registers(tcp) \
    if (bfreg < tcp_bfreg(tcp)) { bfreg = tcp_bfreg(tcp); }	 \
    if (trfreg > tcp_trfreg(tcp)) { trfreg = tcp_trfreg(tcp); }\
    if (hfreg > tcp_hfreg(tcp)) { hfreg = tcp_hfreg(tcp); }	 \
    if (efreg < tcp_efreg(tcp)) { efreg = tcp_efreg(tcp); }

#define reclaim_stacks(tcp) \
  if (tcp == root_address) { \
    reset_freeze_registers_backtrack(tcp);	     \
    /* xsb_dbgmsg("reset registers...."); */ \
  } \
  else { \
    adjust_freeze_registers(tcp); \
    /* xsb_dbgmsg(adjust registers...."); */ \
  }

/*----------------------------------------------------------------------*/

#define pdlpush(cell)	*(pdlreg) = cell;				\
                        if (pdlreg-- < (CPtr)pdl.low)	       		\
			  xsb_abort("PANIC: pdl overflow; large or cyclic structure?")

#define pdlpop		*(++pdlreg)

#define pdlempty	(pdlreg == (CPtr)(pdl.high) - 1)

#define resetpdl \
   if (pdlreg < (CPtr) pdl.low) \
     xsb_exit("pdlreg grew too much"); \
   else (pdlreg = (CPtr)(pdl.high) - 1)

#define pdlprint  {				\
    CPtr temp_pdlreg = pdlreg;				\
    while (!(temp_pdlreg == (CPtr)(pdl.high) - 1)) {			\
      print_cp_cell("pdl", temp_pdlreg, *(temp_pdlreg+1));		\
      temp_pdlreg++;							\
    }						  \
  }						 

#define remove_incomplete_tables_loop(Endpoint) remove_incomplete_tries(Endpoint)

#define remove_incomplete_tables() remove_incomplete_tries(CTXTc COMPLSTACKBOTTOM)

/*----------------------------------------------------------------------*/

#ifdef CALL_ABSTRACTION
#define get_var_and_attv_nums(var_num, attv_num, abstr_size, tmp_int)   \
  var_num = (int)tmp_int & 0xffff;					\
  abstr_size = ((int)tmp_int & 0x3f0000) >>16;				\
  attv_num = (int)tmp_int >> 27

#define get_template_size(var_num,tmp_int)   var_num = tmp_int & 0xffff

#define encode_ansTempl_ctrs(Attvars,AbstractSize,Ctr)   \
  makeint((Attvars << 27) | (AbstractSize << 16) | Ctr)
#else
#define get_var_and_attv_nums(var_num, attv_num, tmp_int)	\
  var_num = (int) (tmp_int & 0xffff);				\
  attv_num = (int)(tmp_int >> 16)

#define encode_ansTempl_ctrs(Attvars,Ctr)   makeint(Attvars << 16 | Ctr)
#endif



/*----------------------------------------------------------------------*/

#endif /* __MACRO_XSB_H__ */
