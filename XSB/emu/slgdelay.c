/* File:      slgdelay.c
** Author(s): Kostis Sagonas, Baoqiu Cui
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
** $Id: slgdelay.c,v 1.67 2010/02/20 21:48:13 evansbj Exp $
**
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* special debug includes */
#include "debugs/debug_delay.h"

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "register.h"
#include "trie_internals.h"
#include "memory_xsb.h"
#include "choice.h"
#include "tab_structs.h"
#include "tr_utils.h"
#include "inst_xsb.h"
#include "error_xsb.h"
#include "io_builtins_xsb.h"
#include "thread_xsb.h"
#include "tr_utils.h"
#include "debug_xsb.h"
#include "dynamic_stack.h"
#include "tst_aux.h"
#include "tries.h"
#include "tables.h"
#include "flags_xsb.h"
#include "tst_utils.h"
#include "tables_i.h"
#include "slgdelay.h"

static void simplify_neg_succeeds(CTXTdeclc VariantSF);
extern void simplify_pos_unsupported(CTXTdeclc NODEptr);
void simplify_pos_unconditional(CTXTdeclc NODEptr);
void print_pdes(PNDE);
//void simplify_neg_succeeds_for_subsumed_subgoals(NODEptr);

Structure_Manager smASI      = SM_InitDecl(ASI_Node, ASIs_PER_BLOCK,
					    "Answer Substitution Info Node");

/*
 * Global variables for shared/sequential tables.
 */

/* Constants */
/* If you change the size of the blocks, please update info in
   trie_internals.h also */

static size_t de_block_size_glc = 2048 * sizeof(struct delay_element);
static size_t dl_block_size_glc = 2048 * sizeof(struct delay_list);
static size_t pnde_block_size_glc = 2048 *sizeof(struct pos_neg_de_list);

/* Rest of globals protected by MUTEX_DELAY (I hope) */
char *current_de_block_gl = NULL;
char *current_dl_block_gl = NULL;
char *current_pnde_block_gl = NULL;

DE released_des_gl = NULL;	/* the list of released DEs */
DL released_dls_gl = NULL;	/* the list of released DLs */
PNDE released_pndes_gl = NULL;	/* the list of released PNDEs */

static DE next_free_de_gl = NULL;	/* next available free DE space */
static DL next_free_dl_gl = NULL;	/* next available free DL space */
static PNDE next_free_pnde_gl = NULL;	/* next available free PNDE space */

static DE current_de_block_top_gl = NULL;      /* the top of current DE block */
static DL current_dl_block_top_gl = NULL;      /* the top of current DL block */
static PNDE current_pnde_block_top_gl = NULL; /* the top of current PNDE block*/


/*
 * A macro definition for allocating a new entry of DE, DL, or PNDE.  To
 * release such an entry, use definition release_entry (see below).
 *
 * new_entry(NEW_ENTRY,            -- pointer to the new entry
 * 	     RELEASED,             -- pointer to the released entries
 * 	     NEXT_FREE,            -- pointer to the next free entry
 * 	     CURRENT_BLOCK,        -- pointer to the current block
 * 	     CURRENT_BLOCK_TOP,    -- pointer to the current block top
 * 	     NEXT_FUNCTION,        -- next function (eg. de_next)
 * 	     ENTRY_TYPE,           -- type of the entry (eg. DE)
 * 	     BLOCK_SIZE,           -- block size (for malloc a new block)
 * 	     ABORT_MESG)           -- xsb_abort mesg when no enough memory
 *
 * No mutexes needed at this level, as shared structures will be
 * protected by MUTEX_DELAY.
 */

#define new_entry(NEW_ENTRY,						\
		  RELEASED,						\
		  NEXT_FREE,						\
		  CURRENT_BLOCK,					\
		  CURRENT_BLOCK_TOP,					\
		  NEXT_FUNCTION,					\
		  ENTRY_TYPE,						\
		  BLOCK_SIZE,						\
		  ABORT_MESG)						\
  if (RELEASED) {							\
    NEW_ENTRY = RELEASED;						\
    RELEASED = NEXT_FUNCTION(RELEASED);					\
  }									\
  else if (NEXT_FREE < CURRENT_BLOCK_TOP)				\
    NEW_ENTRY = NEXT_FREE++;						\
  else {								\
    char *new_block;							\
    if ((new_block = (char *) mem_alloc(BLOCK_SIZE + sizeof(Cell),TABLE_SPACE)) == NULL)\
      xsb_abort(ABORT_MESG);						\
    *(char **) new_block = CURRENT_BLOCK;				\
    CURRENT_BLOCK = new_block;						\
    NEXT_FREE = (ENTRY_TYPE)(new_block + sizeof(Cell));			\
    CURRENT_BLOCK_TOP = (ENTRY_TYPE)(new_block + sizeof(Cell) + BLOCK_SIZE);\
    NEW_ENTRY = NEXT_FREE++;						\
  }

/* * * * */

/* * * * */

/*
 * TLS: AS info had been using malloc directly, so I changed it to use
 * the structure managers that are more common to the rest of the
 * system, rather than the new_entry/release_entry methods.
 *
 * Allocate shared structure is not needed, as shared structures will be
 * protected by the lock in do_delay_stuff to MUTEX_DELAY
 */

#define create_asi_info(ST_MAN,ANS, SUBG)			\
  {								\
    SM_AllocateStruct(ST_MAN,( asi));				\
    Child(ANS) = (NODEptr) asi;					\
    asi_pdes(asi) = NULL;					\
    asi_subgoal(asi) = SUBG;					\
    asi_dl_list(asi) = NULL;					\
    asi_scratchpad(asi) = 0;					\
  }

/*
 * The following functions are used for statistics.  If changing their
 * usage pattern, make sure you adjust the mutexes.  (MUTEX_DELAY is
 * non-recursive.)   Use of NOERROR mutexes is ok here.
 */


size_t allocated_de_space(char * current_de_block,size_t * num_blocks)
{
  size_t size = 0;
  char *t = current_de_block;

  *num_blocks = 0;
  while (t) {
    (*num_blocks)++;
    size += (de_block_size_glc + sizeof(Cell));
    t = *(char **)t;
  }
  return size;
}

static int released_de_num(DE released_des)
{
  int i = 0;
  DE p;

  p = released_des;
  while (p != NULL) {
    i++;
    p = de_next(p);
  }
  return(i);
}

size_t unused_de_space(void)
{
  return (current_de_block_top_gl
	  - next_free_de_gl
	  + released_de_num(released_des_gl)) * sizeof(struct delay_element);
}

#ifdef MULTI_THREAD
size_t unused_de_space_private(CTXTdecl)
{
  return (private_current_de_block_top
	  - private_next_free_de
	  + released_de_num(private_released_des)) * sizeof(struct delay_element);
}
#endif

/* * * * * */

size_t allocated_dl_space(char * current_dl_block,size_t * num_blocks)
{
  size_t size = 0;
  char *t = current_dl_block;

  *num_blocks = 0;
  while (t) {
    (*num_blocks)++;
    size += (dl_block_size_glc + sizeof(Cell));
    t = *(char **)t;
  }
  return size;
}

static int released_dl_num(DL released_dls)
{
  int i = 0;
  DL p;

  p = released_dls;
  while (p != NULL) {
    i++;
    p = dl_next(p);
  }
  return(i);
}

size_t unused_dl_space(void)
{
  return (current_dl_block_top_gl
	  - next_free_dl_gl
	  + released_dl_num(released_dls_gl)) * sizeof(struct delay_list);
}

#ifdef MULTI_THREAD
size_t unused_dl_space_private(CTXTdecl)
{
  return (private_current_dl_block_top
	  - private_next_free_dl
	  + released_dl_num(private_released_dls)) * sizeof(struct delay_list);
}
#endif

/* * * * * */

size_t allocated_pnde_space(char * current_pnde_block, size_t * num_blocks)
{
  size_t size = 0;
  char *t = current_pnde_block;

  *num_blocks = 0;
  while (t) {
    (*num_blocks)++;
    size += (pnde_block_size_glc + sizeof(Cell));
    t = *(char **)t;
  }
  return size;
}

static int released_pnde_num(PNDE released_pndes)
{
  int i = 0;
  PNDE p;

  p = released_pndes;
  while (p != NULL) {
    i++;
    p = dl_next(p);
  }
  return(i);
}

size_t unused_pnde_space(void)
{
  return (current_pnde_block_top_gl
	  - next_free_pnde_gl
	  + released_pnde_num(released_pndes_gl)) * sizeof(struct pos_neg_de_list);
}

#ifdef MULTI_THREAD
size_t unused_pnde_space_private(CTXTdecl)
{
  return (private_current_pnde_block_top
	  - private_next_free_pnde
	  + released_pnde_num(private_released_pndes)) * sizeof(struct pos_neg_de_list);
}
#endif

/* * * * * * * * * * * * * * * * * * * * ** * * * * * * * * * * * * *
 * Proto-simplification routines for new answer:

 * when a new answer is about to be added, for each element DE in its
 * delay list, a check is made that DE is neither successful nor
 * failed in the current interpretation.  The checks thus perform an
 * analog of simplification.
 *
 * For positive DEs we have a pointer to an answer, which is either
 * deleted, unconditional or conditional.  For negative DEs we have a
 * pointer to a subgoal frame.  Determining whether the subgoal is
 * succeeded or failed is simple for variant SFs and producer SFs (for
 * call subsumption), but for consumer SFs in call subsumption we need
 * to check whether the answer is in the trie for the subsuming
 * subgoal -- an operation performed by get_answer_for_subgoal()

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* simplified trie-lookup using variance for use in
   get_answer_for_subgoal() and simpl_variant_trie_lookup(). */
void *simpl_var_trie_lookup(CTXTdeclc void *branchRoot, xsbBool *wasFound,
		      Cell *failedSymbol) {

  BTNptr parent;	/* Last node containing a matched symbol */

  Cell symbol = 0;	/* Trie representation of current heap symbol,
			   used for matching/inserting into a TSTN */

  int std_var_num;	/* Next available TrieVar index; for
  			   standardizing variables when interned.
  			   This is needed for handling non-linear
  			   variables in construct_ground_term() */

  parent = branchRoot;
  std_var_num = (int)Trail_NumBindings;
  SQUASH_LINUX_COMPILER_WARN(std_var_num) ; 
  while ( ! TermStack_IsEmpty ) {

    TermStack_Pop(symbol);
    {
      BTNptr chain;
      int chain_length;
      if ( IsHashHeader(BTN_Child(parent)) ) {
    	  BTHTptr ht = BTN_GetHashHdr(parent);
    	  chain = *CalculateBucketForSymbol(ht,symbol);
      }
      else
    	  chain = BTN_Child(parent);
      SearchChainForSymbol(chain,symbol,chain_length);
      if ( IsNonNULL(chain) )
    	  parent = chain;
      else {
    	  *wasFound = FALSE;
    	  *failedSymbol = symbol;
    	  return NULL;
      }
    }
  }
  *wasFound = TRUE;
  return parent;
}

#define SimplStack_Pop(Stack,Symbol) {		\
   CPtr curFrame;				\
   DynStk_Pop(Stack,curFrame);		\
   if (curFrame != NULL) Symbol = *curFrame;	\
}

#define SimplStack_Push(Stack,Symbol) {		\
   CPtr nextFrame;				\
   DynStk_Push(Stack,nextFrame);		\
   *nextFrame = Symbol;				\
 }

#define SimplStack_PushPathRoot(Stack,Leaf,Root) { 	\
    BTNptr btn = Leaf;					\
    while ( ! IsTrieRoot(btn) ) {			\
      /*      printTrieNode(stddbg,btn);	*/	\
      SimplStack_Push(Stack,BTN_Symbol(btn));		\
    btn = BTN_Parent(btn);			\
   }						\
   Root = (void *)btn;				\
 }

NODEptr get_answer_for_subgoal(CTXTdeclc SubConsSF consumerSF) {
  BTNptr trieNode = NULL;
  xsbBool wasFound;
  Cell symbol;

  TermStack_ResetTOS;
  SimplStack_PushPathRoot(tstTermStack,
			  subg_leaf_ptr(consumerSF),TIF_CallTrie(subg_tif_ptr(consumerSF)));

  trieNode = simpl_var_trie_lookup(CTXTc TIF_CallTrie(subg_tif_ptr(consumerSF)),
				   &wasFound,&symbol);
  return trieNode;
}

/* * * * * * * * * *
 * Checks whether a delay element that is about to be interned was
 * simplifiable (simplifications were already initiated for this DE).
 * More specifically, negative delay elements were simplifiable if their
 * subgoal failed (was completed without any answers), while positive
 * delay elements are simplifiable if their answer substitution became
 * unconditional.
 * * * * * * * * * */

xsbBool was_simplifiable(CTXTdeclc VariantSF subgoal, NODEptr ANS) {
  NODEptr AnsLeaf;

  if (ANS == NULL) { /* negative delay element */
    if (IsSubConsSF(subgoal)) {
      AnsLeaf = get_answer_for_subgoal(CTXTc (SubConsSF) subgoal);
      return (is_completed(conssf_producer(subgoal))
	      && (IsNULL(AnsLeaf) || IsDeletedNode(AnsLeaf)));
    } else /* TLS: not 100% SURE subgoal_fails handles deleted nodes */
      return (is_completed(subgoal) && subgoal_fails(subgoal));
  }
  else return is_unconditional_answer(ANS);
}

xsbBool is_failing_delay_element(CTXTdeclc VariantSF subgoal, NODEptr ANS) {
  NODEptr AnsLeaf;

  if (ANS == NULL) {  /* negative delay element */
    if (IsSubConsSF(subgoal)) {
      AnsLeaf = get_answer_for_subgoal(CTXTc (SubConsSF) subgoal);
      return (IsNonNULL(AnsLeaf) && is_unconditional_answer(AnsLeaf));
    } else
      return (is_completed(subgoal) && has_answer_code(subgoal)
	      && subgoal_unconditionally_succeeds(subgoal));
  }
  else /* positive delay element: no difference between subsumption and variance */
    return IsDeletedNode(ANS);
}

/* * * * * * * * * * * * * * * * * * * * ** * * * * * * * * * * * * *
 * Assign one entry for delay_elem in the current DE (Delay Element)
 * block.  A new block will be allocated if necessary.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static DE intern_delay_element(CTXTdeclc Cell delay_elem, VariantSF par_subgoal)
{
  DE de;
  CPtr cptr = (CPtr) cs_val(delay_elem);
  /*
   * All the following information about delay_elem is set in
   * delay_negatively() or delay_positively().  Note that cell(cptr) is
   * the delay_psc ('DL').
   */
  VariantSF subgoal;
  NODEptr ans_subst;
  CPtr ret_n = 0;
  int arity;
  Cell tmp_cell;

  tmp_cell = cell(cptr + 1);
  subgoal = (VariantSF) addr_val(tmp_cell);
  tmp_cell = cell(cptr + 2);
  ans_subst = (NODEptr) addr_val(tmp_cell);
  tmp_cell = cell(cptr + 3);

  /*
   * cell(cptr + 3) can be one of the following:
   *   1. integer 0 (NEG_DELAY), for a negative DE;
   *   2. string "ret", for a positive DE with arity 0;
   *   3. constr ret/n, for a positive DE with arity >=1.
   */
  if (isinteger(tmp_cell) || isstring(tmp_cell))
    arity = 0;
  else {
    ret_n = (CPtr) cs_val(tmp_cell);
    arity = get_arity((Psc) get_str_psc(cell(cptr + 3)));
  }

  if (!was_simplifiable(CTXTc subgoal, ans_subst)) {
    new_entry(de,
	      released_des_gl,
	      next_free_de_gl,
	      current_de_block_gl,
	      current_de_block_top_gl,
	      de_next,
	      DE,
	      de_block_size_glc,
	      "Not enough memory to expand DE space");
    de_subgoal(de) = subgoal;
    de_ans_subst(de) = ans_subst; /* Leaf of the answer (substitution) trie */

#ifndef IGNORE_DELAYVAR
    de_subs_fact(de) = NULL;
    if (arity != 0) {
      de_subs_fact_leaf(de) = delay_chk_insert(CTXTc arity, ret_n + 1,
					       (CPtr *) &de_subs_fact(de));
    }
#endif /* IGNORE_DELAYVAR */
    return de;
  }
  else
    return NULL;
}

/* * * * * *
 * Private version of above predicate (took out ignore/debug
 * delayvars)
 * * * * * */

#ifdef MULTI_THREAD
static DE intern_delay_element_private(CTXTdeclc Cell delay_elem, VariantSF par_subgoal)
{
  DE de;
  CPtr cptr = (CPtr) cs_val(delay_elem);
  /*
   * All the following information about delay_elem is set in
   * delay_negatively() or delay_positively().  Note that cell(cptr) is
   * the delay_psc ('DL').
   */
  VariantSF subgoal;
  NODEptr ans_subst;
  CPtr ret_n = 0;
  int arity;
  Cell tmp_cell;

  tmp_cell = cell(cptr + 1);
  subgoal = (VariantSF) addr_val(tmp_cell);
  tmp_cell = cell(cptr + 2);
  ans_subst = (NODEptr) addr_val(tmp_cell);
  tmp_cell = cell(cptr + 3);

  /*
   * cell(cptr + 3) can be one of the following:
   *   1. integer 0 (NEG_DELAY), for a negative DE;
   *   2. string "ret", for a positive DE with arity 0;
   *   3. constr ret/n, for a positive DE with arity >=1.
   */
  if (isinteger(tmp_cell) || isstring(tmp_cell))
    arity = 0;
  else {
    ret_n = (CPtr) cs_val(tmp_cell);
    arity = get_arity((Psc) get_str_psc(cell(cptr + 3)));
  }

  if (!was_simplifiable(CTXTc subgoal, ans_subst)) {
    new_entry(de,
	      private_released_des,
	      private_next_free_de,
	      private_current_de_block,
	      private_current_de_block_top,
	      de_next,
	      DE,
	      de_block_size_glc,
	      "Not enough memory to expand DE space");
    de_subgoal(de) = subgoal;
    de_ans_subst(de) = ans_subst; /* Leaf of the answer (substitution) trie */

    de_subs_fact(de) = NULL;
    if (arity != 0) {
      de_subs_fact_leaf(de) = delay_chk_insert(CTXTc arity, ret_n + 1,
					       (CPtr *) &de_subs_fact(de));
    }
    return de;
  }
  else
    return NULL;
}
#endif // (MULTI_THREAD)

/*
 * Construct a delay list according to dlist.  Assign an entry in the
 * current DL block for it.  A new DL block will be allocated if
 * necessary.
 */

static DL intern_delay_list(CTXTdeclc CPtr dlist, VariantSF subgoal) /* assumes that dlist != NULL */
{
  DE head = NULL, de;
  DL dl = NULL;

  while (islist(dlist)) {
    dlist = clref_val(dlist);
    if ((de = intern_delay_element(CTXTc cell(dlist),subgoal)) != NULL) {
      de_next(de) = head;
      head = de;
    }
    dlist = (CPtr) cell(dlist+1);
  }
  if (head) {
    new_entry(dl,
	      released_dls_gl,
	      next_free_dl_gl,
	      current_dl_block_gl,
	      current_dl_block_top_gl,
	      dl_next,
	      DL,
	      dl_block_size_glc,
	      "Not enough memory to expand DL space");
    dl_de_list(dl) = head;
    dl_asl(dl) = NULL;
    return dl;
  }
  else return NULL;
}

#ifdef MULTI_THREAD
static DL intern_delay_list_private(CTXTdeclc CPtr dlist, VariantSF subgoal) /* assumes that dlist != NULL	*/
{
  DE head = NULL, de;
  DL dl = NULL;

  while (islist(dlist)) {
    dlist = clref_val(dlist);
    if ((de = intern_delay_element_private(CTXTc cell(dlist),subgoal)) != NULL) {
      de_next(de) = head;
      head = de;
    }
    dlist = (CPtr) cell(dlist+1);
  }
  if (head) {
    new_entry(dl,
	      private_released_dls,
	      private_next_free_dl,
	      private_current_dl_block,
	      private_current_dl_block_top,
	      dl_next,
	      DL,
	      dl_block_size_glc,
	      "Not enough memory to expand DL space");
    dl_de_list(dl) = head;
    dl_asl(dl) = NULL;
    return dl;
  }
  else return NULL;
}
#endif

/*
 * For each delay element de in delay list dl, do one of the following
 * things:
 *
 * 1) (If de is a negative DE) Add a NDE `pnde' to the nde_list of de's
 *    subgoal frame.  Point de_pnde(de) to `pnde', and point
 *    pnde_dl(pnde) to dl, pnde_de(pnde) to de.
 *
 * 2) (If de is a positive DE) Add a PDE `pnde' to the pdes of the
 *    Delay Info node of de's answer substitution leaf.  Point
 *    de_pnde(de) to `pnde', and point pnde_dl(pnde) to dl, pnde_de(pnde)
 *    to de.
 */

static void record_de_usage(DL dl)
{
  DE de;
  PNDE pnde, current_first;
  NODEptr as_leaf;
#ifdef DEBUG_DELAYVAR
  PNDE tmp;
#endif

  de = dl_de_list(dl);
  while (de) {
    new_entry(pnde,
	      released_pndes_gl,
	      next_free_pnde_gl,
	      current_pnde_block_gl,
	      current_pnde_block_top_gl,
	      pnde_next,
	      PNDE,
	      pnde_block_size_glc,
	      "Not enough memory to expand PNDE space");
    pnde_dl(pnde) = dl;
    pnde_de(pnde) = de;
    pnde_prev(pnde) = NULL;
    if ((as_leaf = de_ans_subst(de)) == NULL) {	/* a negative DE */
      current_first = subg_nde_list(de_subgoal(de));
      pnde_next(pnde) = current_first;
      if (current_first)
	pnde_prev(current_first) = pnde;
      subg_nde_list(de_subgoal(de)) = pnde;
    }
    else {					/* a positive DE */
      current_first = asi_pdes(Delay(as_leaf));
      pnde_next(pnde) = current_first;
      if (current_first)
	pnde_prev(current_first) = pnde;
      asi_pdes(Delay(as_leaf)) = pnde;
    }
    de_pnde(de) = pnde;	/* record */
    de = de_next(de);
  }
}

/* * * * * *
 * Private version of above predicate (took out ignore/debug
 * delayvars)
 * * * * * */
#ifdef MULTI_THREAD
static void record_de_usage_private(CTXTdeclc DL dl)
{
  DE de;
  PNDE pnde, current_first;
  NODEptr as_leaf;

  de = dl_de_list(dl);
  while (de) {
    new_entry(pnde,private_released_pndes,
	      private_next_free_pnde,private_current_pnde_block,
	      private_current_pnde_block_top,pnde_next,
	      PNDE,pnde_block_size_glc,
	      "Not enough memory to expand PNDE space");
    pnde_dl(pnde) = dl;
    pnde_de(pnde) = de;
    pnde_prev(pnde) = NULL;
    if ((as_leaf = de_ans_subst(de)) == NULL) {	/* a negative DE */
      current_first = subg_nde_list(de_subgoal(de));
      pnde_next(pnde) = current_first;
      if (current_first)
	pnde_prev(current_first) = pnde;
      subg_nde_list(de_subgoal(de)) = pnde;
    }
    else {					/* a positive DE */
      current_first = asi_pdes(Delay(as_leaf));
      pnde_next(pnde) = current_first;
      if (current_first)
	pnde_prev(current_first) = pnde;
      asi_pdes(Delay(as_leaf)) = pnde;
    }
    de_pnde(de) = pnde;	/* record */
    de = de_next(de);
  }
}
#endif

/*
 * do_delay_stuff() is called in table_answer_search(), immediately
 * after <variant/subsumptive>_answer_search(), regardless of whether
 * the answer is new.  Here, `as_leaf' is the leaf node of the answer
 * trie (the return value of variant_answer_search), `subgoal' is the
 * subgoal frame of the current call, and `sf_exists' tells whether
 * the non-conditional part of this answer is new or not.
 *
 * At call time, `delayreg' is the delay register of the _current_
 * execution state.  If delayreg is not NULL, then it means this
 * answer has some delay elements and is conditional.  (If non-NULL,
 * delayreg points to a delay list of delay elements on the heap).
 * This function assumes that conditional answers have previously been
 * checked to ensure none of their delay elements is non-satisfiable
 * (done in new_answer_dealloc).  At the same time, do_delay_stuff()
 * will recheck the delay elements to remove any that have become
 * unconditional.  Thus, two traversals of the delay list are
 * required.
 *
 * Function intern_delay_list() will be called to save the delay list
 * information in the Delay Info node of current call's answer leaf.  It
 * will call intern_delay_element(), which will call delay_chk_insert().
 * A delay trie is created by delay_chk_insert() for the corresponding
 * delay element.
 *
 * When the delay trie has been created, and a pointer in the delay
 * element (saved in the answer trie) has been set, we can say the
 * conditional answer is tabled.
 *
 * TLS: moved mutexes into conditionals.  This avoids locking the
 * delay mutex when adding an answer for a LRD stratified program.
 * For non-LRD programs the placement of mutexes may mean that we lock
 * the mutex more than once per answer, but such cases will be
 * uncommon for the most part (as two of the cases engender
 * simplification).
 */

void do_delay_stuff_shared(CTXTdeclc NODEptr as_leaf, VariantSF subgoal, xsbBool sf_exists)
{
    ASI	asi;
    DL dl = NULL;

    if (delayreg) {
      //      print_delay_list(CTXTc stddbg,delayreg);fprintf(stddbg,"\n");
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      if (!sf_exists || is_conditional_answer(as_leaf)) {
        if ((dl = intern_delay_list(CTXTc delayreg, subgoal)) != NULL) {
	  mark_conditional_answer(as_leaf, subgoal, dl, smASI);
	  //	  printf("marked conditional ans as_leaf %p asi %p\n",as_leaf,asi);
	  record_de_usage(dl);
        }
      }
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
    /*
     * Check for the derivation of an unconditional answer.
     */
    //    printf("sf %d is_cond %d delayreg %d\n",sf_exists,is_conditional_answer(as_leaf),delayreg);

    if (sf_exists && is_conditional_answer(as_leaf) &&
	(!delayreg || !dl)) {
      /*
       * Initiate positive simplification in places where this answer
       * substitution has already been returned.
       */
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      simplify_pos_unconditional(CTXTc as_leaf);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
    if (is_unconditional_answer(as_leaf) && subg_nde_list(subgoal)) {
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      simplify_neg_succeeds(CTXTc subgoal);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
}

#ifdef MULTI_THREAD
void do_delay_stuff_private(CTXTdeclc NODEptr as_leaf, VariantSF subgoal, xsbBool sf_exists)
{
    ASI	asi;
    DL dl = NULL;

    if (delayreg && (!sf_exists || is_conditional_answer(as_leaf))) {
      if ((dl = intern_delay_list_private(CTXTc delayreg, subgoal)) != NULL) {
	mark_conditional_answer(as_leaf, subgoal, dl, *private_smASI);
	record_de_usage_private(CTXTc dl);
      }
    }
    /*
     * Check for the derivation of an unconditional answer.
     */
    if (sf_exists && is_conditional_answer(as_leaf) &&
	(!delayreg || !dl)) {
      /*
       * Initiate positive simplification in places where this answer
       * substitution has already been returned.
       */
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      simplify_pos_unconditional(CTXTc as_leaf);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
    if (is_unconditional_answer(as_leaf) && subg_nde_list(subgoal)) {
      SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
      simplify_neg_succeeds(CTXTc subgoal);
      SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
    }
}
#endif

void do_delay_stuff(CTXTdeclc NODEptr as_leaf, VariantSF subgoal, xsbBool sf_exists)
{

  //  fprintf(stddbg,"in do delay stuff (past answer is junk)\n");
#ifdef MULTI_THREAD
  if (threads_current_sm == PRIVATE_SM) {
    do_delay_stuff_private(CTXTc as_leaf, subgoal, sf_exists);
  } else {
    do_delay_stuff_shared(CTXTc as_leaf, subgoal, sf_exists);
    }
#else
  do_delay_stuff_shared(CTXTc as_leaf, subgoal, sf_exists);
#endif

}

/*----------------------------------------------------------------------*/
/* test whether a subgoal call is ground; used for testing direct
   positive recursion in residual program.  Maybe a better by looking
   at template size in heap? See test in early completion. */

int is_ground_subgoal(VariantSF subg) {
  BTNptr leaf;
  for (leaf = subg_leaf_ptr(subg); leaf != NULL; leaf = Parent(leaf)) {
    if (IsTrieVar(BTN_Symbol(leaf))) return FALSE;
  }
  return TRUE;
}

xsbBool answer_is_unsupported(CTXTdeclc CPtr dlist)  /* assumes that dlist != NULL */
{
    CPtr    cptr;
    VariantSF subgoal;
    NODEptr ans_subst;
    Cell tmp_cell;

    while (islist(dlist)) {
      dlist = clref_val(dlist);
      cptr = (CPtr) cs_val(cell(dlist));
      tmp_cell = cell(cptr + 1);
      subgoal = (VariantSF) addr_val(tmp_cell);
      tmp_cell = cell(cptr + 2);
      ans_subst = (NODEptr) addr_val(tmp_cell);
      if (is_failing_delay_element(CTXTc subgoal,ans_subst)) {
	return TRUE;
      }
      dlist = (CPtr) cell(dlist+1);
    }
    return FALSE;
}

/*************************************************************
 * Function remove_de_from_dl(de, dl) removes de from dl when de is
 * positive and succeeds, or negative and fails.  It is used in
 * simplify_pos_unconditional() and simplify_neg_fails().
 *************************************************************/

static xsbBool remove_de_from_dl(CTXTdeclc DE de, DL dl)
{
  DE current = dl_de_list(dl);
  DE prev_de = NULL;

  while (current != de) {
    prev_de = current;
    current = de_next(current);
  }
  if (prev_de == NULL)		/* to remove the first DE */
    dl_de_list(dl) = de_next(current);
  else
    de_next(prev_de) = de_next(current);
#ifdef MULTI_THREAD
  if (IsPrivateSF(asi_subgoal((ASI) Child(dl_asl(dl)))))
    release_private_de_entry(current)
  else
#endif
    release_shared_de_entry(current)
  return (NULL != dl_de_list(dl));
}

/******************************************************************
 * Function remove_dl_from_dl_list(dl, asi) removes dl from the DL list
 * which is pointed by asi.  Called when a DE in dl is negative and
 * succeeds, or positive and unsupported (in functions
 * simplify_neg_succeeds() and simplify_pos_unsupported()).
 ******************************************************************/

xsbBool remove_dl_from_dl_list(CTXTdeclc DL dl, ASI asi)
{
  DL current = asi_dl_list(asi);
  DL prev_dl = NULL;

  // amp: what if 'current' is already NULL ?? It can happen, you know? Maybe it shouldn't happen, but it does.
  if (!current)
	  return FALSE;

  while (current != dl) {
    prev_dl = current;
    current = dl_next(current);
  }
  if (prev_dl == NULL)		/* to remove the first DL */
    asi_dl_list(asi) = dl_next(current);
  else
    dl_next(prev_dl) = dl_next(current);

#ifdef MULTI_THREAD
  if (IsPrivateSF(asi_subgoal(asi)))
    release_entry(current, private_released_dls, dl_next)
  else
#endif
    release_entry(current, released_dls_gl, dl_next);
  return (NULL != asi_dl_list(asi));
}

/*******************************************************************

 * Simplification and call subsumption.  When simplifying for
 * call-subsumption a special approach is made to account for
 * simplifying negative subgoals that depend on positive answers.  The
 * issue is that there is not a PDE from an answer to consuming
 * subgoal frames -- only to producing.  Accordingly, if the
 * simplified answer is ground and belongs to a predicate that is
 * tabled with call subsumption, we have to go to the call trie and
 * determine whether there is a consuming subgoal frame associated
 * with that answer.  In order to do this we have to bind the answer
 * to the substitution factor of the call in construct_ground_term().
 *
 * This check is done both in handle_empty_dl_creation() and in
 * handle_unsupported_answer_subst()

 *******************************************************************/

extern DynamicStack simplGoalStack;
extern DynamicStack simplAnsStack;

/* differs from get_answer_for_subgoal in that its pushing a vector, rather than a trie path */
void *simpl_variant_trie_lookup(CTXTdeclc void *trieRoot, int nTerms, CPtr termVector,
			  Cell varArray[]) {

  BTNptr trieNode = NULL;
  xsbBool wasFound;
  Cell symbol;

  TermStack_ResetTOS;
  TermStack_PushLowToHighVector(termVector,DynStk_NumFrames(tstSymbolStack));
  //  printSymbolStack(CTXTc stddbg, "tstTermStack", tstTermStack);
  trieNode = simpl_var_trie_lookup(CTXTc trieRoot,&wasFound,&symbol);

  return trieNode;
}

/* assume term copied is not a var */
void answerStack_copyTerm(CTXTdecl) {
  Cell symbol = (Cell) NULL;
  int i;

  SimplStack_Pop(simplAnsStack,symbol);
  //  printf("working on AS: ");printTrieSymbol(stddbg, symbol);fprintf(stddbg,"\n");
  if (symbol != (Cell) NULL) {
    SymbolStack_Push(symbol);
    if (IsTrieFunctor(symbol))
      for (i = 0 ; i < get_arity(DecodeTrieFunctor(symbol)) ; i++) {
	answerStack_copyTerm(CTXT);
      }
    else if (IsTrieList(symbol)) {
      answerStack_copyTerm(CTXT);
      answerStack_copyTerm(CTXT);
    }
  }
}

/* For aliased variables; assume term copied is not a var */
void answerStack_copyTermPtr(CTXTdeclc CPtr symbolPtr) {
  Cell symbol;
  int i;

  //  printf("answerStack_copyTermPtr: %p ",symbolPtr);
  symbol = *symbolPtr;
  //  printf("working on AS (Ptr): ");printTrieSymbol(stddbg, symbol);fprintf(stddbg,"\n");
  SymbolStack_Push(symbol);
  if (IsTrieFunctor(symbol))
    for (i = 0 ; i < get_arity(DecodeTrieFunctor(symbol)) ; i++) {
      symbolPtr--;
      answerStack_copyTermPtr(CTXTc symbolPtr);
    }
  else if (IsTrieList(symbol)) {
      symbolPtr--;
      answerStack_copyTermPtr(CTXTc symbolPtr);
      symbolPtr--;
      answerStack_copyTermPtr(CTXTc symbolPtr);
    }
}

void construct_ground_term(CTXTdeclc BTNptr as_leaf,VariantSF subgoal) {
  Cell symbol = (Cell) NULL;
  int maxvar = -1;
  CPtr aliasArray[DEFAULT_NUM_TRIEVARS];
  int i;
  for(i = 0; i < DEFAULT_NUM_TRIEVARS; i++) aliasArray[i] = NULL;

  DynStk_ResetTOS(simplAnsStack);
  SimplStack_PushPathRoot(simplAnsStack,as_leaf,subg_ans_root_ptr(subgoal));
  //  printSymbolStack(CTXTc stddbg, "simplAnsStack", simplAnsStack);

  DynStk_ResetTOS(simplGoalStack);
  SimplStack_PushPathRoot(simplGoalStack,
			  subg_leaf_ptr(subgoal),TIF_CallTrie(subg_tif_ptr(subgoal)));

  //  printSymbolStack(CTXTc stddbg, "simplGoalStack", simplGoalStack);

  SymbolStack_ResetTOS;
  while (!DynStk_IsEmpty(simplGoalStack)) {
    // TLS: should probably check for null sumbol after pop?
    SimplStack_Pop(simplGoalStack,symbol);
    //    printf("working on GS: ");printTrieSymbol(stddbg, symbol);fprintf(stddbg,"\n");
    if (IsTrieVar(symbol)) {
      if (DecodeTrieVar(symbol) <= maxvar) {
	answerStack_copyTermPtr(CTXTc aliasArray[DecodeTrieVar(symbol)]);
      }
      else {
	maxvar++;
	aliasArray[maxvar] = DynStk_PrevFrame(simplAnsStack);
	  //DynStk_Top(simplAnsStack) - DynStk_FrameSize(simplAnsStack);
	answerStack_copyTerm(CTXT);
      }
    }
    else SymbolStack_Push(symbol);
  }
  //  printSymbolStack(CTXTc stddbg, "tstSymbolStack",tstSymbolStack);
}

int is_ground_answer(NODEptr as_leaf) {
  while (! IsTrieRoot(as_leaf)) {
    //    printTrieNode(stddbg,as_leaf);
    if ( IsTrieVar(BTN_Symbol(as_leaf)) )
      return 0;
    else as_leaf = TSTN_Parent(as_leaf);
  }
  return 1;
}

/*******************************************************************
 * When a DL becomes empty (after remove_de_from_dl()), the answer
 * substitution which uses this DL becomes unconditional.  Further
 * simplification operations go on ...
 * Should be called with MUTEX_DELAY already locked -- e.g. by do_delay_stuff()
 *******************************************************************/

static void handle_empty_dl_creation(CTXTdeclc DL dl)
{
  NODEptr as_leaf = dl_asl(dl);
  ASI asi = Delay(as_leaf);
  VariantSF subgoal;

  //  fprintf(stderr,"in handle_empty_dl_creation\n");

  /*
   * Only when `as_leaf' is still a conditional answer can we do
   * remove_dl_from_dl_list(), simplify_pos_unconditional(), and
   * simplify_neg_succeeds() here.
   *
   * If `as_leaf' is already marked UNCONDITIONAL (by
   * unmark_conditional_answer(as_leaf) in simplify_pos_unconditional()),
   * that means this is the second time when `as_leaf' becomes
   * unconditional. So we don't need do anything.  All the DLs have been
   * released in the first time.
   */
  if (is_conditional_answer(as_leaf)) {	/* if it is still conditional */
    remove_dl_from_dl_list(CTXTc dl, asi);
    subgoal = asi_subgoal(Delay(as_leaf));

    // fprintf(stddbg,"in handle empty dl ");
    // fprintf(stddbg, ">>>> the subgoal is: ");
    //  print_subgoal(stddbg, subgoal); fprintf(stddbg,"\n");	
    //  fprintf(stddbg, ">>>> the answer is: ");
    //  printTriePath(CTXTc stddbg, as_leaf, FALSE); fprintf(stddbg,"\n");	
    //  printf("dl list %p\n",asi_dl_list(Delay(as_leaf)));

    /** simplify_pos_unconditional(as_leaf) will release all other DLs for
     * as_leaf, and mark as_leaf as UNCONDITIONAL.*/

    simplify_pos_unconditional(CTXTc as_leaf);

    /* Perform simplify_neg_succeeds() for consumer sfs (producers and
       variants done below) */
    if (IsSubProdSF(subgoal) && is_ground_answer(as_leaf)) {
      BTNptr leaf;
      Cell callVars[DEFAULT_NUM_TRIEVARS];
      construct_ground_term(CTXTc as_leaf,subgoal);
      leaf = simpl_variant_trie_lookup(CTXTc TIF_CallTrie(subg_tif_ptr(subgoal)),
				 get_arity(TIF_PSC(subg_tif_ptr(subgoal))),
				 (CPtr) DynStk_Base(tstSymbolStack), callVars);
      if ( IsNonNULL(leaf) ) {
	complete_subg( (VariantSF) Child(leaf));
	// if calling simplify_neg_succeeds, must be unconditional, so
	//	subg_asf_list_ptr( (VariantSF) Child(leaf)) = NULL;
	//	fprintf(stderr,"hedc B\n");
	simplify_neg_succeeds(CTXTc (VariantSF) Child(leaf));
	//	fprintf(stderr,"hedc D\n");
      }
    }
    /*-- perform early completion if necessary; please preserve invariants --*/
    if (!is_completed(subgoal) && most_general_answer(as_leaf)) {
      perform_early_completion(CTXTc subgoal, subg_cp_ptr(subgoal));
      subg_compl_susp_ptr(subgoal) = NULL;
    }
    simplify_neg_succeeds(CTXTc subgoal);
  }
}

/*******************************************************************
 * Run further simplifications when an answer substitution leaf,
 * as_leaf, has no supported conditional answers.  This happens when
 * all DLs of as_leaf are removed (by remove_dl_from_dl_list)
 *******************************************************************/

void handle_unsupported_answer_subst(CTXTdeclc NODEptr as_leaf)
{

  ASI unsup_asi = Delay(as_leaf);
  VariantSF unsup_subgoal = asi_subgoal(unsup_asi);
  VariantSF subsumed_subgoal = NULL;
  BTNptr subgoal_leaf = NULL;

  //  fprintf(stddbg, "handle unsupported answer subst; subgoal: "); print_subgoal(stddbg, unsup_subgoal); fprintf(stddbg, " >>> the answer is: ");
  //  printTriePath(CTXTc stddbg, as_leaf, FALSE); fprintf(stddbg,"\n");
  //  printf("dl list %p\n",asi_dl_list(Delay(as_leaf)));

  /* For call subsumption, we need to split the lookup (which must be
     done before delete_branch) from the simplification itself (which
     must be done after delete_branch). */
  if (IsSubProdSF(unsup_subgoal) && is_ground_answer(as_leaf)) {
    Cell callVars[DEFAULT_NUM_TRIEVARS];
    construct_ground_term(CTXTc as_leaf,unsup_subgoal);

    subgoal_leaf = simpl_variant_trie_lookup(CTXTc TIF_CallTrie(subg_tif_ptr(unsup_subgoal)),
				     get_arity(TIF_PSC(subg_tif_ptr(unsup_subgoal))),
				     (CPtr) DynStk_Base(tstSymbolStack), callVars);

    if (IsNonNULL(subgoal_leaf) )
	subsumed_subgoal = (VariantSF) Child(subgoal_leaf);
  }

  /* must do delete branch before simplifying -- otherwise checks will be thrown off */
  if (IsSubProdSF(unsup_subgoal))
    delete_branch(CTXTc as_leaf, &subg_ans_root_ptr(unsup_subgoal),SUBSUMPTIVE_EVAL_METHOD);
  else { /* VariantSF -- set to private or shared.  */
    SET_TRIE_ALLOCATION_TYPE_SF(unsup_subgoal);
    delete_branch(CTXTc as_leaf, &subg_ans_root_ptr(unsup_subgoal),VARIANT_EVAL_METHOD);
  }

  simplify_pos_unsupported(CTXTc as_leaf);
  if (is_completed(unsup_subgoal)) {
    if (subgoal_fails(unsup_subgoal)) {
      simplify_neg_fails(CTXTc unsup_subgoal); // may be recursive
    }

    /* Perform simplify_neg_fails() for consumer sfs */
    if ( IsSubProdSF(unsup_subgoal) && IsNonNULL(subgoal_leaf)
	 && subsumed_subgoal != unsup_subgoal) {
      simplify_neg_fails(CTXTc subsumed_subgoal); // IS recursive
    }
  }
#ifdef MULTI_THREAD
  if (IsPrivateSF(asi_subgoal(unsup_asi)))
    SM_DeallocateStruct(*private_smASI,unsup_asi)
  else
#endif
    SM_DeallocateSharedStruct(smASI,unsup_asi);
}

/**********************************************************************
 * To release all the DLs (and their DEs) from the DelayInfo node `asi'.
 * Also releases the PNDEs associated with the DEs (and the delay tries)
 **********************************************************************/

void release_all_dls(CTXTdeclc ASI asi)
{
  ASI de_asi;
  DE de, tmp_de;
  DL dl, tmp_dl;
#ifdef MULTI_THREAD
  xsbBool isPrivate;
#endif

  dl = asi_dl_list(asi);
  while (dl) {
#ifdef MULTI_THREAD
    isPrivate = IsPrivateSF(asi_subgoal(Delay(dl_asl(dl))));
#endif
    tmp_dl = dl_next(dl);
    de = dl_de_list(dl);
    while (de) {
      tmp_de = de_next(de);
      if (de_ans_subst(de) == NULL) { /* is NDE */
    	  if (subg_nde_list(de_subgoal(de))) {
#ifdef MULTI_THREAD
	  if (IsPrivateSF(subg_nde_list(de_subgoal(de))))
	    remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), private_released_pndes)
	  else
#endif
	    remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), released_pndes_gl);
    	  }
      }
      else {
    	  de_asi = Delay(de_ans_subst(de));
#ifdef MULTI_THREAD
	if (IsPrivateSF(asi_subgoal(de_asi)))
	  remove_pnde(asi_pdes(de_asi), de_pnde(de),private_released_pndes)
	else
#endif
		remove_pnde(asi_pdes(de_asi), de_pnde(de),released_pndes_gl);
      }
#ifdef MULTI_THREAD
	  /* Now remove the de itself (allocated for the same table as the DL and anawer) */
	if (isPrivate) release_private_de_entry(de)
	  else
#endif
	    release_shared_de_entry(de);
	  de = tmp_de; /* next DE */
    } /* while (de) */
#ifdef MULTI_THREAD
	  /* Now remove the de itself (allocated for the same table as the DL and anawer) */
      if (isPrivate) release_entry(dl, private_released_dls, dl_next)
      else
#endif
	release_entry(dl, released_dls_gl, dl_next);
      dl = tmp_dl; /* next DL */
  }
}

/*******************************************************************
 * Pos unconditional simplification
 *
 * When the answers substitution gets an unconditional answer, remove
 * the positive delay literals of this answer substitution from the
 * delay lists that contain them.
 *******************************************************************/

void simplify_pos_unconditional(CTXTdeclc NODEptr as_leaf)
{
  ASI asi = Delay(as_leaf);
  PNDE pde;
  DE de;
  DL dl;
  VariantSF subgoal;
#ifdef MULTI_THREAD
  xsbBool isPrivate = IsPrivateSF(asi_subgoal(asi));
#endif
  subgoal = asi_subgoal(asi);
  release_all_dls(CTXTc asi);
  unmark_conditional_answer(as_leaf);

  //  print_pdes(asi_pdes(asi));
  while ((pde = asi_pdes(asi))) {

    if (flags[CTRACE_CALLS] && !subg_forest_log_off(subgoal))  {				
      char buffera[MAXTERMBUFSIZE];			
      char bufferc[MAXTERMBUFSIZE];			
      memset(buffera,0,MAXTERMBUFSIZE);
      memset(bufferc,0,MAXTERMBUFSIZE);
      //      memset(bufferb,0,MAXTERMBUFSIZE);
      //      memset(bufferd,0,MAXTERMBUFSIZE);
      sprintTriePath(CTXTc buffera, as_leaf);
      sprint_subgoal(CTXTc forest_log_buffer_1,0, subgoal);
      sprintTriePath(CTXTc bufferc, dl_asl(pnde_dl(pde)));
      sprint_subgoal(CTXTc forest_log_buffer_2,0, 
		     asi_subgoal(Delay(dl_asl(pnde_dl(pde)))));
      //      print_subgoal(stdout, asi_subgoal(Delay(dl_asl(pnde_dl(pde)))));printf("\n");
      fprintf(fview_ptr,"puc_smpl(%s,%s,%s,%s,%d).\n",buffera,
	      forest_log_buffer_1->fl_buffer,bufferc,
	      forest_log_buffer_2->fl_buffer,ctrace_ctr++); 
    }

    de = pnde_de(pde);
    dl = pnde_dl(pde);
#ifdef MULTI_THREAD
    if (isPrivate) remove_pnde(asi_pdes(asi), pde, private_released_pndes)
    else
#endif
      remove_pnde(asi_pdes(asi), pde, released_pndes_gl);
    if (!remove_de_from_dl(CTXTc de, dl))
      handle_empty_dl_creation(CTXTc dl);
  }
  /*
   * Now this DelayInfo `asi' does not contain any useful info, so we can
   * free it and mark `as_leaf' as an unconditional answer.
   */
  Child(as_leaf) = NULL;
  if (subg_answers(subgoal) == COND_ANSWERS) {
    subg_answers(subgoal) = UNCOND_ANSWERS;
    answer_complete_subg(subgoal);
  }
#ifdef MULTI_THREAD
  if (isPrivate)
    SM_DeallocateStruct(*private_smASI,asi)
  else
#endif
  SM_DeallocateSharedStruct(smASI,asi);
}

/*******************************************************************
 * When the subgoal fails (is completed without any answers), remove
 * the negative delay literals of this subgoal from the delay lists
 * that contain them.
 ******************************************************************/

#ifndef MULTI_THREAD
#define MAX_SIMPLIFY_NEG_FAILS_STACK 10
  VariantSF simplify_neg_fails_stack[MAX_SIMPLIFY_NEG_FAILS_STACK];
  Integer simplify_neg_fails_stack_top;
  int in_simplify_neg_fails;
#endif

VariantSF * dyn_simplify_neg_fails_stack;
int dyn_simplify_neg_fails_stack_index = 0;
int dyn_simplify_neg_fails_stack_size   = 0;

#define push_neg_simpl(X) {\
    if (dyn_simplify_neg_fails_stack_index+1 >= dyn_simplify_neg_fails_stack_size) {\
      trie_expand_array(VariantSF, dyn_simplify_neg_fails_stack,	\
			dyn_simplify_neg_fails_stack_size,0,"dyn_simplify_neg_fails_stack"); \
    }									\
    /*    printf("pushing %p\n",subgoal);	*/			\
    dyn_simplify_neg_fails_stack[dyn_simplify_neg_fails_stack_index++] = ((VariantSF) X);\
}

#define pop_neg_simpl dyn_simplify_neg_fails_stack[--dyn_simplify_neg_fails_stack_index]

  //  if (simplify_neg_fails_stack_top < MAX_SIMPLIFY_NEG_FAILS_STACK - 1) {
  //    simplify_neg_fails_stack[simplify_neg_fails_stack_top++] = subgoal;
  //  } else xsb_abort("Overflow simplify_neg_fails stack");

void simplify_neg_fails(CTXTdeclc VariantSF subgoal)
{
  PNDE nde;
  DE de;
  DL dl;

  push_neg_simpl(subgoal);

  if (in_simplify_neg_fails) {
    return;
  }
  in_simplify_neg_fails = 1;

  subg_answers(subgoal) = NO_ANSWERS; // mark as having no answers now. dsw
  answer_complete_subg(subgoal);

  while (dyn_simplify_neg_fails_stack_index > 0) {
    subgoal = pop_neg_simpl;
    /*    printf("popped %p\n",subgoal);*/ 
    while ((nde = subg_nde_list(subgoal))) {
      de = pnde_de(nde);
      dl = pnde_dl(nde);

      if (flags[CTRACE_CALLS] && !subg_forest_log_off(subgoal))  {				
      //      memset(bufferb,0,MAXTERMBUFSIZE);
      //      memset(buffera,0,MAXTERMBUFSIZE);
      sprint_subgoal(CTXTc forest_log_buffer_1,0, subgoal);
      sprint_subgoal(CTXTc forest_log_buffer_2,0,
		     asi_subgoal(Delay(dl_asl(dl))));
      fprintf(fview_ptr,"nf_smpl(tnot(%s),%s,%d).\n",
	      forest_log_buffer_1->fl_buffer,
	      forest_log_buffer_2->fl_buffer,ctrace_ctr++); 
    }

#ifdef MULTI_THREAD
      if (IsPrivateSF(subgoal)) remove_pnde(subg_nde_list(subgoal), nde, private_released_pndes)
      else
#endif
      remove_pnde(subg_nde_list(subgoal), nde,released_pndes_gl);
      if (!remove_de_from_dl(CTXTc de, dl)) {
	//	fprintf(stderr,"snf_while B\n");
	handle_empty_dl_creation(CTXTc dl);
	//	fprintf(stderr,"snf_while C\n");
      }
    }
  }
  in_simplify_neg_fails = 0;
}

/********************************************************************
 * On occasion that the subgoal succeeds (gets an unconditional
 * answer that is identical to the subgoal), it deletes all delay
 * lists that contain a negative delay element with that subgoal.
 *
 * Before remove_dl_from_dl_list(), all the DEs in the DL to be
 * removed have to be released, and each P(N)DE which points to a DE
 * in the DL has also to be released.
 ********************************************************************/

static void simplify_neg_succeeds(CTXTdeclc VariantSF subgoal)
{
  PNDE nde;
  DE de, tmp_de;
  ASI used_asi, de_asi;
  NODEptr used_as_leaf;
  DL dl, tmp_dl;
  UNUSED(tmp_dl);

  //  printf("in simplify neg succeeds: ");print_subgoal(stddbg,subgoal);printf("\n");
  answer_complete_subg(subgoal);

  while ((nde = subg_nde_list(subgoal))) {

    if (flags[CTRACE_CALLS] && !subg_forest_log_off(subgoal))  {
      //      memset(bufferb,0,MAXTERMBUFSIZE);
      //      memset(buffera,0,MAXTERMBUFSIZE);
      sprint_subgoal(CTXTc forest_log_buffer_1,0, subgoal);
      sprint_subgoal(CTXTc forest_log_buffer_2,0, 
		     asi_subgoal(Delay(dl_asl(pnde_dl(nde)))));
      fprintf(fview_ptr,"ns_smpl(tnot(%s),%s,%d).\n",
	      forest_log_buffer_1->fl_buffer,
	      forest_log_buffer_2->fl_buffer,ctrace_ctr++); 
    }

    dl = pnde_dl(nde); /* dl: to be removed */
    used_as_leaf = dl_asl(dl);
    //    printf("checking ");printTriePath(CTXTc stddbg, used_as_leaf, FALSE); fprintf(stddbg,"\n");
    tmp_dl = dl;
    if (IsValidNode(used_as_leaf) && (used_asi = Delay(used_as_leaf)) != NULL) {
      de = dl_de_list(dl); /* to release all DEs in dl */
      while (de) {
    	  tmp_de = de_next(de);
    	  if (de_ans_subst(de) == NULL) { /* is NDE */
    		  /* Remove pndes from the subgoal to which the de refers */
#ifdef MULTI_THREAD
	  if (IsPrivateSF(subg_nde_list(de_subgoal(de))))
	    remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), private_released_pndes)
	  else
#endif
			  remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), released_pndes_gl);
    	  }
    	  else { 	  /* Remove pndes from the answer to which the de refers */
    		  de_asi = Delay(de_ans_subst(de));
#ifdef MULTI_THREAD
	  if (IsPrivateSF(asi_subgoal(de_asi)))
	    remove_pnde(asi_pdes(de_asi), de_pnde(de),private_released_pndes)
	  else
#endif
			  remove_pnde(asi_pdes(de_asi), de_pnde(de),released_pndes_gl);
    	  }
#ifdef MULTI_THREAD
	  /* Now remove the de itself (allocated for the same table as the DL and anawer) */
	if (IsPrivateSF(asi_subgoal(Delay(dl_asl(dl)))))
	    release_private_de_entry(de)
	  else
#endif
	    release_shared_de_entry(de);
		de = tmp_de; /* next DE */
	} /* while */
      if (!remove_dl_from_dl_list(CTXTc dl, used_asi)) {
	//	fprintf(stderr,"isns E\n");
    	  handle_unsupported_answer_subst(CTXTc used_as_leaf);
	  //	  fprintf(stderr,"isns F\n");
      }
    } /* if */
  } /* while */
}

/*********************************************************************
 * On occasion that an AnswerSubstitution at `as_leaf' looses all its
 * conditional answers (all its DLs have been removed),
 * simplify_pos_unsupported() deletes all delay lists (of other
 * predicates' conditional answers) that contain a positive delay element
 * pointing to that AnswerSubstitution.
 **********************************************************************/

void simplify_pos_unsupported(CTXTdeclc NODEptr as_leaf)
{
  ASI asi = Delay(as_leaf);
  PNDE pde;
  DL dl;
  DE de, tmp_de;
  ASI used_asi, de_asi;
  NODEptr used_as_leaf;

  //  printf("in simplify pos unsupported: ");print_subgoal(stddbg,asi_subgoal(Delay(as_leaf)));printf("\n");

  while ((pde = asi_pdes(asi))) {

    // TLS: seems to be a problem with printing out as_leaf in this case.
    if (flags[CTRACE_CALLS] && !subg_forest_log_off(asi_subgoal(asi)))  {
      char bufferb[MAXTERMBUFSIZE];			
      memset(bufferb,0,MAXTERMBUFSIZE);
      sprint_subgoal(CTXTc forest_log_buffer_1,0,asi_subgoal(Delay(as_leaf)));
      sprintTriePath(CTXTc bufferb, dl_asl(pnde_dl(pde)));
      sprint_subgoal(CTXTc forest_log_buffer_3,0, 
		     asi_subgoal(Delay(dl_asl(pnde_dl(pde)))));
      fprintf(fview_ptr,"pus_smpl(%s,%s,%s,%d).\n",
	      forest_log_buffer_1->fl_buffer,bufferb,
	      forest_log_buffer_3->fl_buffer,ctrace_ctr++); 
    }

    dl = pnde_dl(pde); /* dl: to be removed */

    if (!dl) return; // amp: necessary check for Answer Completion

    used_as_leaf = dl_asl(dl);
    if (IsValidNode(used_as_leaf) && (used_asi = Delay(used_as_leaf)) != NULL) {
      de = dl_de_list(dl); /* to release all DEs in dl */
      while (de) {
    	  tmp_de = de_next(de);
    	  if (de_ans_subst(de) == NULL) { /* is NDE */
#ifdef MULTI_THREAD
	  if (IsPrivateSF(subg_nde_list(de_subgoal(de))))
	    remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), private_released_pndes)
	  else
#endif
			  remove_pnde(subg_nde_list(de_subgoal(de)), de_pnde(de), released_pndes_gl);
    	  }
    	  else {			  /* is PDE */
    		  de_asi = Delay(de_ans_subst(de));
#ifdef MULTI_THREAD
	  /* TLS: changed this */
	  //	  de_asi = Delay(de_ans_subst(de));
	  if (IsPrivateSF(asi_subgoal(de_asi)))
	    remove_pnde(asi_pdes(de_asi), de_pnde(de),private_released_pndes)
	  else
#endif
			  remove_pnde(asi_pdes(de_asi), de_pnde(de), released_pndes_gl);
    	  }
#ifdef MULTI_THREAD
	  /* Now remove the de itself (allocated for the same table as the DL and anawer) */
	if (IsPrivateSF(asi_subgoal(Delay(dl_asl(dl)))))
	  release_private_de_entry(de)
	else
#endif
	  release_shared_de_entry(de);
	de = tmp_de; /* next DE */
      } /* while */
    if (!remove_dl_from_dl_list(CTXTc dl, used_asi)) {
	handle_unsupported_answer_subst(CTXTc used_as_leaf);
      }
    } /* if */
  } /* while */
}

/* Can currently be called with only one active thread: otherwise put
   mutexes back in. */
void abolish_wfs_space(CTXTdecl)
{
  char *last_block;

#ifndef LOCAL_EVAL
    extern void abolish_edge_space();
#endif

  /* clear DE blocks */
  while (current_de_block_gl) {
    last_block = *(char **) current_de_block_gl;
    mem_dealloc(current_de_block_gl,de_block_size_glc + sizeof(Cell),TABLE_SPACE);
    current_de_block_gl = last_block;
  }

  /* clear DL blocks */
  while (current_dl_block_gl) {
    last_block = *(char **) current_dl_block_gl;
    mem_dealloc(current_dl_block_gl,dl_block_size_glc + sizeof(Cell),TABLE_SPACE);
    current_dl_block_gl = last_block;
  }

  /* clear PNDE blocks */
  while (current_pnde_block_gl) {
    last_block = *(char **) current_pnde_block_gl;
    mem_dealloc(current_pnde_block_gl,pnde_block_size_glc + sizeof(Cell),TABLE_SPACE);
    current_pnde_block_gl = last_block;
  }

  /* reset some pointers */

  released_des_gl = NULL;
  released_dls_gl = NULL;
  released_pndes_gl = NULL;

  next_free_de_gl = NULL;
  next_free_dl_gl = NULL;
  next_free_pnde_gl = NULL;

  current_de_block_top_gl = NULL;
  current_dl_block_top_gl = NULL;
  current_pnde_block_top_gl = NULL;

  SM_ReleaseResources(smASI);

#ifndef LOCAL_EVAL
    abolish_edge_space();
#endif
}

#ifdef MULTI_THREAD
void abolish_private_wfs_space(CTXTdecl)
{
  char *last_block;

// #ifndef LOCAL_EVAL
//    extern void abolish_edge_space();
// #endif

  /* clear DE blocks */
  while (private_current_de_block) {
    last_block = *(char **) private_current_de_block;
    mem_dealloc(private_current_de_block,de_block_size_glc + sizeof(Cell),TABLE_SPACE);
    private_current_de_block = last_block;
  }

  /* clear DL blocks */
  while (private_current_dl_block) {
    last_block = *(char **) private_current_dl_block;
    mem_dealloc(private_current_dl_block,dl_block_size_glc + sizeof(Cell),TABLE_SPACE);
    private_current_dl_block = last_block;
  }

  /* clear PNDE blocks */
  while (private_current_pnde_block) {
    last_block = *(char **) private_current_pnde_block;
    mem_dealloc(private_current_pnde_block,pnde_block_size_glc + sizeof(Cell),TABLE_SPACE);
    private_current_pnde_block = last_block;
  }

  SM_ReleaseResources(*private_smASI);

  /* reset some pointers */
  private_released_des = NULL;
  private_released_dls = NULL;
  private_released_pndes = NULL;

  private_next_free_de = NULL;
  private_next_free_dl = NULL;
  private_next_free_pnde = NULL;

  private_current_de_block_top = NULL;
  private_current_dl_block_top = NULL;
  private_current_pnde_block_top = NULL;

  // #ifndef LOCAL_EVAL
  //    abolish_edge_space();
  //#endif
}

#endif

/*
 * Two functions added for builtin force_truth_value/2.
 */

void force_answer_true(CTXTdeclc NODEptr as_leaf)
{
  VariantSF subgoal;

  if (is_conditional_answer(as_leaf)) {
    SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
    subgoal = asi_subgoal(Delay(as_leaf));
    simplify_pos_unconditional(CTXTc as_leaf);
    simplify_neg_succeeds(CTXTc subgoal);
    SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
  }
}

void force_answer_false(CTXTdeclc NODEptr as_leaf)
{
  ASI asi = Delay(as_leaf);
  VariantSF subgoal;

  if (is_conditional_answer(as_leaf)) {
    SYS_MUTEX_LOCK( MUTEX_DELAY ) ;
    subgoal = asi_subgoal(asi);
    release_all_dls(CTXTc asi);
    SET_TRIE_ALLOCATION_TYPE_SF(subgoal);
    delete_branch(CTXTc as_leaf, &subg_ans_root_ptr(subgoal),VARIANT_EVAL_METHOD);
    simplify_pos_unsupported(CTXTc as_leaf);
    mark_subgoal_failed(subgoal);
    simplify_neg_fails(CTXTc subgoal);
    SYS_MUTEX_UNLOCK( MUTEX_DELAY ) ;
  }
}

/* Prints backpointers of PNDEs (tested w. answers, not yet w. subgoals) */
#ifndef MULTI_THREAD
void print_pdes(PNDE firstPNDE) {
  //  DE de;
  DL dl;

  printf("PNDE for subgoal : ");print_subgoal(stddbg,de_subgoal(pnde_de(firstPNDE))); printf("\n");
  while (firstPNDE) {
    //    de = pnde_de(firstPNDE);
    //      while (de) {
    //	fprintf(stddbg,"  PDE subgoal ");
    //	print_subgoal(stddbg, de_subgoal(de)); fprintf(stddbg,"\n");
    //	de = de_next(de);
    //      }
    printf("considering pnde %p\n",firstPNDE);
    dl = pnde_dl(firstPNDE);
    while (dl) {
      fprintf(stddbg,"  backpoints to subgoal: ");
      print_subgoal(stddbg,asi_subgoal((ASI) Child(dl_asl(dl))));fprintf(stddbg,"\n");
      dl = dl_next(dl);
    }
    firstPNDE = pnde_next(firstPNDE);
  }
}
#endif

/*---------------------- end of file slgdelay.c ------------------------*/

