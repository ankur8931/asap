/* File:      tr_utils.h
** Author(s): Prasad Rao, Kostis Sagonas
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
** $Id: tr_utils.h,v 1.77 2012-11-28 16:39:29 tswift Exp $
** 
*/

#ifndef __TR_UTILS_H__
#define __TR_UTILS_H__

#include "context.h"

struct interned_trie_t {
  BTNptr  root;
  byte    valid;
  byte    type;
  byte	  incremental;
  callnodeptr   callnode;	/* for incremental eval */
  int     prev_entry;
  int     next_entry;
} ;
typedef struct interned_trie_t *TrieTabPtr;

/* Also includes trie stuff */
#ifndef MULTI_THREAD
extern TrieTabPtr itrie_array;
#endif

/* Shared interned tries are only used in the MT engine, but define
   the types in the MT engine */
extern void init_shared_trie_table();
struct shared_interned_trie_t {
  BTNptr  root;
  byte    valid;
  byte    type;
  int     prev_entry;
  int     next_entry;
#ifdef MULTI_THREAD
  pthread_mutex_t      trie_mutex;
#endif
} ;

typedef struct shared_interned_trie_t *ShrTrieTabPtr;


#define CAN_RECLAIM 0
#define CANT_RECLAIM 1

#define TRIE_ID_TYPE_MASK               0xfff00000
#define TRIE_ID_ID_MASK                 0x000fffff
#define TRIE_ID_SHIFT                   20
#define SET_TRIE_ID(IND,TYPE,TID)       ((TID) = (((TYPE) << TRIE_ID_SHIFT)| IND))
#define SPLIT_TRIE_ID(TID,IND,TYPE)     {				\
    ((TYPE) = ((TID) & TRIE_ID_TYPE_MASK) >> 20);				\
    ((IND) = ((TID) &TRIE_ID_ID_MASK)); }

extern VariantSF get_subgoal_frame_for_answer_trie_cp(CTXTdeclc BTNptr,CPtr);
extern VariantSF get_variant_sf(CTXTdeclc Cell, TIFptr, Cell *);
extern SubProdSF get_subsumer_sf(CTXTdeclc Cell, TIFptr, Cell *);
extern BTNptr get_trie_root(BTNptr);
extern VariantSF get_call(CTXTdeclc Cell, Cell *);
extern Cell build_ret_term(CTXTdeclc int, Cell[]);
extern Cell build_ret_term_reverse(CTXTdeclc int, Cell[]);
extern void construct_answer_template(CTXTdeclc Cell, SubProdSF, Cell[]);
extern void breg_retskel(CTXTdecl);
extern void delete_predicate_table(CTXTdeclc TIFptr,xsbBool);
extern void reclaim_del_ret_list(CTXTdeclc VariantSF);
extern void delete_return(CTXTdeclc BTNptr, VariantSF,int);
extern void init_private_trie_table(CTXTdecl);
extern void delete_branch(CTXTdeclc BTNptr, BTNptr *,int);
extern void safe_delete_branch(BTNptr);
extern void undelete_branch(BTNptr);
extern void reclaim_uninterned_nr(CTXTdeclc Integer rootidx);
extern void delete_trie(CTXTdeclc BTNptr);
extern xsbBool is_completed_table(TIFptr);

extern xsbBool varsf_has_unconditional_answers(VariantSF);
extern void    first_trie_property(CTXTdecl);
extern void    next_trie_property(CTXTdecl);

extern Integer newtrie(CTXTdeclc int);
extern void trie_drop(CTXTdecl);
extern int private_trie_intern(CTXTdecl);
extern int shas_trie_intern(CTXTdecl);
extern int  private_trie_interned(CTXTdecl);
extern int  shas_trie_interned(CTXTdecl);
extern void private_trie_unintern(CTXTdecl);
extern void shas_trie_unintern(CTXTdecl);
extern void trie_dispose_nr(CTXTdecl);
extern void trie_truncate(CTXTdeclc Integer);
extern void trie_undispose(CTXTdeclc Integer, BTNptr);
extern int interned_trie_cps_check(CTXTdeclc BTNptr);

// extern xsbBool check_table_cut;

extern void abolish_table_predicate(CTXTdeclc Psc, int);
extern void abolish_table_predicate_switch(CTXTdeclc TIFptr, Psc, int, int,int);
extern void abolish_table_call(CTXTdeclc VariantSF, int);
extern void abolish_private_tables(CTXTdecl);
extern void abolish_shared_tables(CTXTdecl);
extern void abolish_all_tables(CTXTdeclc int);
extern int abolish_usermod_tables(CTXTdecl);
extern int abolish_module_tables(CTXTdeclc const char *module_name);
extern int abolish_table_call_incr(CTXTdeclc VariantSF); /* incremental evaluation */
extern int gc_tabled_preds(CTXTdecl);
extern int abolish_table_call_cps_check(CTXTdeclc VariantSF);
extern void check_insert_global_deltf_subgoal(CTXTdeclc VariantSF,xsbBool);
extern double total_table_gc_time;

//TLS: should probably be static
//extern void delete_variant_sf_and_answers(CTXTdeclc VariantSF pSF, xsbBool warn);

extern void release_any_pndes(CTXTdeclc PNDE firstPNDE);
extern void delete_delay_trie(CTXTdeclc BTNptr root);
extern void release_all_tabling_resources(CTXTdecl);

// Perhaps this should be in hashtable.h?
extern void hashtable1_destroy_all(int);

// from sub_delete.c
extern void delete_subsumptive_call(CTXTdeclc SubProdSF);
extern void reclaim_deleted_subsumptive_table(CTXTdeclc DelTFptr);

extern void	remove_incomplete_tries(CTXTdeclc CPtr);

extern counter abol_subg_ctr,abol_pred_ctr,abol_all_ctr; /* statistics */
extern FILE * fview_ptr;

extern int is_ancestor_sf(VariantSF consumer_sf, VariantSF producer_sf);

#define MAX_INTERNED_TRIES 2003

typedef struct Table_Status_Frame {
  int pred_type;
  int goal_type;
  int answer_set_status;
  VariantSF subgoal;
} TableStatusFrame;

typedef struct scc_node {
  CPtr node;
  int dfn;
  int component;
  int low;
  int stack;
} SCCNode;

#define TableStatusFrame_pred_type(TSF) ( (TSF).pred_type )
#define TableStatusFrame_goal_type(TSF) ( (TSF).goal_type )
#define TableStatusFrame_answer_set_status(TSF) ( (TSF).answer_set_status )
#define TableStatusFrame_subgoal(TSF) ( (TSF).subgoal )

extern int table_status(CTXTdeclc Cell, TableStatusFrame*);

extern void alt_print_cp(CTXTdeclc char *);

#define MAX_VAR_SIZE	200

/* #define build_subgoal_args(SUBG)					\
	load_solution_trie(CTXTc arity, 0, &cell_array1[arity-1], subg_leaf_ptr(SUBG))
*/

#define build_subgoal_args(ARITY,ARRAY,SUBG)				\
	load_solution_trie(CTXTc ARITY, 0, &ARRAY[arity-1], subg_leaf_ptr(SUBG))


#define is_trie_instruction(cp_inst) \
 ((int) cp_inst >= 0x5c && (int) cp_inst < 0x80) \
	   || ((int) cp_inst >= 0x90 && (int) cp_inst < 0x95) 


#endif /* __TR_UTILS_H__ */


