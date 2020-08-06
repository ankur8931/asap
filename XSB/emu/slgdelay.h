/* File:      slgdelay.h
** Author(s): Kostis Sagonas, Juliana Freire, Baoqiu Cui
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
** $Id: slgdelay.h,v 1.43 2013-05-06 21:10:25 dwarren Exp $
** 
*/

#ifndef __SLGDELAY_H__
#define __SLGDELAY_H__

/* special debug includes */
#include "debugs/debug_delay.h"

#define NEG_DELAY	0

/*
 * Definitions of delay operations to be used while clause or answer
 * resolution is performed.
 */

#define delay_negatively(SUBGOAL) {					\
  Cell new_delay_cons_cell;						\
									\
  new_delay_cons_cell = makelist(hreg);					\
  follow(hreg) = makecs(hreg+2);					\
  follow(hreg+1) = (delayreg == NULL) ? makenil : (Cell) delayreg;	\
  hreg += 2; 								\
  bld_functor(hreg, delay_psc);						\
  cell(hreg+1) = makeaddr(SUBGOAL);					\
  cell(hreg+2) = makeaddr(NEG_DELAY); 					\
  cell(hreg+3) = makeaddr(NEG_DELAY); /* NOT STRINGS */			\
  hreg += 4;								\
  delayreg = (CPtr) new_delay_cons_cell;				\
}
    
/*
 * When delay_positively() is called, SUBGOAL is the subgoal frame of the
 * delayed subgoal, ANSWER is the answer node in the trie, and MAKE_SUBSF
 * is the pointer to the substitution factor (the ret/n functor built on
 * the heap, or a string ret_psc[0]) of the answer of the delayed subgoal
 * call.
 *
 * A delay element will be built on the heap according to the value in
 * MAKE_SUBSF, and it is inserted at the head of delay list of the parent
 * predicate (pointed by delayreg).
 */

#define delay_positively(SUBGOAL, ANSWER, MAKE_SUBSF) {			\
  Cell new_delay_cons_cell;						\
									\
  new_delay_cons_cell = makelist(hreg);					\
  follow(hreg) = makecs(hreg+2);					\
  follow(hreg+1) = (delayreg == NULL) ? makenil : (Cell) delayreg;	\
  hreg += 2; \
  bld_functor(hreg, delay_psc);					\
  cell(hreg+1) = makeaddr(SUBGOAL);					\
  cell(hreg+2) = makeaddr(ANSWER);					\
  cell(hreg+3) = MAKE_SUBSF;						\
  hreg += 4; \
  delayreg = (CPtr) new_delay_cons_cell;				\
}

/*--------------------------------------------------------------------*/

typedef struct delay_element	*DE;
typedef struct delay_list	*DL;
typedef struct pos_neg_de_list	*PNDE;

/*--------------------------------------------------------------------*/

/* hangs off of answer escape node, which is main access to it, along
   with PDES; function is vaguely analogous to subgoal frame. */

typedef struct AS_info {
  PNDE	  pdes;		/* pos DEs that refer to this answer substitution */
  VariantSF subgoal;	/* subgoal to which this answer substitution belongs */
  DL	  dl_list;	/* delay lists that this answer substitution has */
  UInteger  scratchpad;  /* to be used for answer completion and
				other answer-oriented post-processing  */ 
} *ASI;
typedef struct AS_info ASI_Node;

#define asi_pdes(X)	(X) -> pdes
#define asi_subgoal(X)	(X) -> subgoal
#define asi_dl_list(X)	(X) -> dl_list
#define asi_scratchpad(X)	(X) -> scratchpad

#define ASIs_PER_BLOCK  256

/*--------------------------------------------------------------------*/

/* TLS: delay_elements may have some unnecessary information.
   Abolishing uses subs_fact, but not subs_fact_leaf, while other
   routines such as get_residual() use subs_fact_leaf, but not
   subs_fact. */

struct delay_element {
  VariantSF subgoal;	/* pointer to the subgoal frame of this DE */
  NODEptr ans_subst;	/* pointer to an answer substitution leaf */
  DE	  next;		/* pointer to the next DE in the same DL */
  PNDE	  pnde;		/* pointer to the element in PDE list or NDE
			 * list, depending on what DE it is (positive or
			 * negative).  Will be set in record_de_usage()
			 */
  NODEptr subs_fact;	/* root of the delay trie for this DE */
  NODEptr subs_fact_leaf;
} ;

#define de_subgoal(X)	 (X) -> subgoal
#define de_ans_subst(X)  (X) -> ans_subst
#define de_next(X)	 (X) -> next
#define de_pnde(X)	 (X) ->	pnde
#define de_subs_fact(X)	     (X) -> subs_fact
#define de_subs_fact_leaf(X) (X) -> subs_fact_leaf

#define de_positive(X)   ((X) -> ans_subst != NULL)
/*--------------------------------------------------------------------*/

struct delay_list {
  DE      de_list;	
  NODEptr asl;		/* answer substitution leaf */
  DL      next;		/* next DL for the same AS */
} ;

#define dl_de_list(X)	(X) -> de_list
#define dl_next(X)	(X) -> next
#define dl_asl(X)	(X) -> asl

/*--------------------------------------------------------------------*/

struct pos_neg_de_list {
  DL	dl;
  DE	de;
  PNDE	prev, next;
} ;

#define pnde_dl(X)	(X) -> dl
#define pnde_de(X)	(X) -> de
#define pnde_prev(X)	(X) -> prev
#define pnde_next(X)	(X) -> next


/*
 * Handling of conditional answers.	     			      
 */

#define UNCONDITIONAL_MARK 0x1

#define Delay(X) (ASI) ((word) (TN_Child(X)) & ~(UNCONDITIONAL_MARK | HasALNPtr))

#define is_conditional_answer(ANS) \
  (Child(ANS) && !hasALNtag(ANS) && !((word) (Child(ANS)) & UNCONDITIONAL_MARK))

#define is_unconditional_answer(ANS) \
  !is_conditional_answer(ANS)
  /*  (!Child(ANS) || ((word) (Child(ANS)) & UNCONDITIONAL_MARK)) */

/*
Checking for these two conditionis was made more difficult when WFS
for call subsumption was introduced.  Accordingly they both were moved
to functions in slgdelay.c

#define was_simplifiable(SUBG, ANS)					\
    ((ANS == NULL) ? (is_completed(SUBG) && subgoal_fails(SUBG))	\
    		   : (is_unconditional_answer(ANS)))

#define is_failing_delay_element(SUBG, ANS)				\
    ((ANS == NULL) ? (is_completed(SUBG) && has_answer_code(SUBG) &&	\
		      subgoal_unconditionally_succeeds(SUBG))		\
		   : (IsDeletedNode(ANS)))
*/

/*
 * mark_conditional_answer(ANS, SUBG, NEW_DL) will add a new delay list,
 * NEW_DL, into the list of DLs for answer ANS, which is the answer
 * substitution leaf in answer trie.  If ANS does not have a Delay Info
 * node, then a Delay Info node, `asi', has to be created first (that's
 * why we call this macro definition mark_conditional_answer).  `asi' has
 * a pointer to the list of DLs for ANS.
 */

#define mark_conditional_answer(ANS, SUBG, NEW_DL,STRUCT_MGR)		\
  if (Child(ANS) == NULL || hasALNtag(ANS)) {				\
    create_asi_info(STRUCT_MGR,ANS, SUBG);				\
  }									\
  else {								\
    asi = Delay(ANS);							\
  }									\
  dl_next(NEW_DL) = asi_dl_list(asi);					\
  asi_dl_list(asi) = NEW_DL;						\
  dl_asl(NEW_DL) = ANS

#define unmark_conditional_answer(ANS) /*-- NEEDS CHANGE --*/		\
    Child(ANS) = (NODEptr) ((word) (Child(ANS)) | UNCONDITIONAL_MARK)

#define most_general_answer(ANS) IsEscapeNode(ANS)

#define release_entry(ENTRY_TO_BE_RELEASED,				\
		      RELEASED,						\
		      NEXT_FUNCTION) {					\
  NEXT_FUNCTION(ENTRY_TO_BE_RELEASED) = RELEASED;			\
  RELEASED = ENTRY_TO_BE_RELEASED;					\
}

#define release_shared_de_entry(DE) {	       \
    delete_delay_trie(CTXTc de_subs_fact(de)); \
    release_entry(de, released_des_gl, de_next);	\
  }

#ifdef MULTI_THREAD
#define release_private_de_entry(DE) {	       \
    delete_delay_trie(CTXTc de_subs_fact(de)); \
    release_entry(de, private_released_des, de_next);	\
  }
#endif

/*
 * Variables used in other parts of the system.
 */

extern xsbBool neg_delay;


/*
 * Procedures used in other parts of the system.
 */

/* TLS: because of include dependencies (context -> tab_structs.h ->
   slgdelay), context.h cannot be included until the code is
   refactored.  Therefore, the CTXT-style declarations cannot yet be
   used. */

#ifndef MULTI_THREAD
extern xsbBool answer_is_unsupported(CPtr);
extern void abolish_wfs_space(void);
extern void simplify_neg_fails(VariantSF);
extern void simplify_pos_unconditional(NODEptr);
extern void do_delay_stuff(NODEptr, VariantSF, xsbBool);
extern void handle_unsupported_answer_subst(NODEptr);
#else
struct th_context;
extern xsbBool answer_is_unsupported(struct th_context *,CPtr);
extern void abolish_wfs_space(struct th_context *);
extern void abolish_private_wfs_space(struct th_context *);
extern void simplify_neg_fails(struct th_context *, VariantSF);
extern void simplify_pos_unconditional(struct th_context *, NODEptr);
extern void do_delay_stuff(struct th_context *, NODEptr, VariantSF, xsbBool);
extern void handle_unsupported_answer_subst(struct th_context *, NODEptr);
#endif
extern size_t unused_de_space(void);
extern size_t unused_dl_space(void);
extern size_t unused_pnde_space(void);
extern size_t allocated_de_space(char *,size_t * num_blocks);
extern size_t allocated_dl_space(char *,size_t * num_blocks);
extern size_t allocated_pnde_space(char *,size_t * num_blocks);
#ifndef MULTI_THREAD
extern void simplify_pos_unsupported(NODEptr);
extern void release_all_dls(ASI);
#else
extern size_t unused_de_space_private(struct th_context *);
extern size_t unused_dl_space_private(struct th_context *);
extern size_t unused_pnde_space_private(struct th_context *);
extern void simplify_pos_unsupported(struct th_context *, NODEptr);
extern void release_all_dls(struct th_context *, ASI);
#endif

extern char *current_pnde_block_gl;
extern char *current_de_block_gl;
extern char *current_dl_block_gl;
extern PNDE released_pndes_gl;	/* the list of released PNDEs */
extern DE released_des_gl;	/* the list of released DEs */
extern DL released_dls_gl;	/* the list of released DLs */

/*
 * remove_pnde(PNDE_HEAD, PNDE_ITEM, PNDE_FREELIST) removes PNDE_ITEM
 * from the corresponding doubly-linked PNDE list, and adds it to
 * PNDE_FREELIST. If PNDE_ITEM is the first one in the list, resets
 * PNDE_HEAD to point to the next one.
 *
 * One principle: Whenever we remove a DE, its PDE (or NDE) must be
 * removed from the PNDE list *first* using remove_pnde().
 */

#define remove_pnde(PNDE_HEAD, PNDE_ITEM, PNDE_FREELIST) {	\
    PNDE *pnde_head_ptr;					\
    PNDE next;							\
    pnde_head_ptr = &(PNDE_HEAD);				\
    next = pnde_next(PNDE_ITEM);				\
    if (*pnde_head_ptr == PNDE_ITEM) {				\
      *pnde_head_ptr = next;					\
    } else {							\
      if (pnde_prev(PNDE_ITEM)) {				\
	pnde_next(pnde_prev(PNDE_ITEM)) = next;			\
      }								\
      if (next) {						\
	pnde_prev(next) = pnde_prev(PNDE_ITEM);			\
      }								\
    }								\
    release_entry(PNDE_ITEM, PNDE_FREELIST, pnde_next);		\
  }


/*---------------------- end of file slgdelay.h ------------------------*/

#endif /* __SLGDELAY_H__ */
