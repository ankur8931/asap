/* File:      choice.h
** Author(s): Xu, Swift, Sagonas, Freire, Johnson
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
** $Id: choice.h,v 1.45 2012-02-29 16:20:37 tswift Exp $
** 
*/
#ifndef __CHOICE_H__
#define __CHOICE_H__

/* --- Types of Choice Points ----------------------------------------- */

#define STANDARD_CP_FRAME       0
#define GENERATOR_CP_FRAME	1
#define CONSUMER_CP_FRAME	2
#define COMPL_SUSP_CP_FRAME	3

/* --- type definitions ----------------------------------------------- */

// #define CP_DEBUG 1

typedef struct choice_point {
    byte *next_clause;	/* the entry of next choice */
#ifdef SLG_GC
  CPtr prev_top;        /* previous top of CP stack */
#endif
#ifdef CP_DEBUG
  Psc  psc;             /* PSC of predicate that created this CP */
#endif
    CPtr _ebreg;	/* environment backtrack -- top of env stack */
    CPtr _hreg;		/* current top of heap */
    CPtr *_trreg;	/* current top of trail stack */
    byte *_cpreg;	/* return point of the call to the procedure	*/
    CPtr _ereg;		/* current top of stack */
    CPtr pdreg;		/* value of delay register for the parent subgoal */
    CPtr ptcp;          /* pointer to parent tabled CP (subgoal) */
    CPtr prev;		/* dynamic link */
} *Choice;

#define CP_SIZE	(sizeof(struct choice_point)/sizeof(CPtr))

#define cp_pcreg(b)		((Choice)(b))->next_clause
#define cp_ebreg(b)		((Choice)(b))->_ebreg
#define cp_hreg(b)		((Choice)(b))->_hreg
#define cp_trreg(b)		((Choice)(b))->_trreg
#define cp_cpreg(b)		((Choice)(b))->_cpreg
#define cp_ereg(b)		((Choice)(b))->_ereg
#define cp_prevbreg(b)		((Choice)(b))->prev
#define cp_pdreg(b)		((Choice)(b))->pdreg
#define cp_ptcp(b)              ((Choice)(b))->ptcp


#ifdef SLG_GC
#define cp_prevtop(b)              ((Choice)(b))->prev_top
#endif
#ifdef CP_DEBUG
#define cp_psc(b)               ((Choice)(b))->psc
#define SAVE_PSC(b) cp_psc(b) = pscreg
#else
#define SAVE_PSC(b)
#endif

#define save_choicepoint(t_breg, t_ereg, next_clause, prev) \
    t_breg -= CP_SIZE; \
    cp_ptcp(t_breg) = ptcpreg; \
    cp_pdreg(t_breg) = delayreg; \
    cp_prevbreg(t_breg) = prev; \
    cp_ereg(t_breg) = t_ereg; \
    cp_cpreg(t_breg) = cpreg; \
    cp_trreg(t_breg) = trreg; \
    cp_hreg(t_breg) = hreg; \
    cp_ebreg(t_breg) = ebreg; \
    SAVE_PSC(t_breg); \
    cp_pcreg(t_breg) = next_clause

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/* Table-Producer Choice Point						*/
/*----------------------------------------------------------------------*/

typedef struct tabled_choice_point {
    byte *next_clause;	/* the entry of next choice */
#if defined(SLG_GC)
  CPtr prev_top;
#endif
#ifdef CP_DEBUG
  Psc  psc;             /* PSC of predicate that created this CP */
#endif
    CPtr _ebreg;	/* environment backtrack -- top of env stack */
    CPtr _hreg;		/* current top of heap */
    CPtr *_trreg;	/* current top of trail stack */
    byte *_cpreg;	/* return point of the call to the procedure */
    CPtr _ereg;		/* current top of stack */
    CPtr pdreg;		/* value of delay register for the parent subgoal */
    CPtr ptcp;		/* pointer to parent tabled CP (subgoal) */
    CPtr prev;		/* previous choicepoint */
    CPtr answer_template;
    CPtr subgoal_ptr;	/* pointer to the subgoal frame */
#if defined(LOCAL_EVAL) && !defined(SLG_GC)
  CPtr prev_top;
#endif
/* The following are needed to reclaim frozen space at SCC completion time */
    CPtr _bfreg;
    CPtr _hfreg;
    CPtr *_trfreg;
    CPtr _efreg;
#ifdef LOCAL_EVAL
    ALNptr trie_return;
#endif
#ifdef SHARED_COMPL_TABLES
    byte * reset_pcreg;
#endif
#ifdef CONC_COMPL
    CPtr compl_stack_ptr; 
#endif
} *TChoice;

#define TCP_SIZE	(sizeof(struct tabled_choice_point)/sizeof(CPtr))

#define tcp_pcreg(b)		((TChoice)(b))->next_clause
#define tcp_ebreg(b)		((TChoice)(b))->_ebreg
#define tcp_hreg(b)		((TChoice)(b))->_hreg
#define tcp_trreg(b)		((TChoice)(b))->_trreg
#define tcp_cpreg(b)		((TChoice)(b))->_cpreg
#define tcp_ereg(b)		((TChoice)(b))->_ereg
#define tcp_prevbreg(b)		((TChoice)(b))->prev
#define tcp_pdreg(b)		((TChoice)(b))->pdreg
#define tcp_ptcp(b)		((TChoice)(b))->ptcp
#define tcp_subgoal_ptr(b)	((TChoice)(b))->subgoal_ptr
#ifdef CONC_COMPL
#define tcp_compl_stack_ptr(b) ((TChoice)(b))->compl_stack_ptr
#endif

#ifdef CP_DEBUG
#define tcp_psc(b)              ((TChoice)(b))->psc
#define SAVE_TPSC(b)            tcp_psc(b) = pscreg
#else
#define SAVE_TPSC(b)
#endif

#if defined(SLG_GC) || defined(LOCAL_EVAL)
#define tcp_prevtop(b)          ((TChoice)(b))->prev_top
#endif

#define tcp_template(b)         ((TChoice)(b))->answer_template

#define tcp_bfreg(b)		((TChoice)(b))->_bfreg
#define tcp_hfreg(b)		((TChoice)(b))->_hfreg
#define tcp_trfreg(b)		((TChoice)(b))->_trfreg
#define tcp_efreg(b)		((TChoice)(b))->_efreg

#ifdef LOCAL_EVAL
#define tcp_trie_return(b)	((TChoice)(b))->trie_return
#endif

#ifdef MULTI_THREAD
#define tcp_reset_pcreg(b)	((TChoice)(b))->reset_pcreg
#endif

#define is_generator_choicepoint(b)			\
    ((cp_pcreg(b) == (byte *) &check_complete_inst) ||	\
     (cell_opcode(cp_pcreg(b)) == tabletrust) ||	\
     (cell_opcode(cp_pcreg(b)) == tableretry))

#define _SaveProducerCPF_common(TopCPS, Cont, pSF) {    \
   TopCPS -= TCP_SIZE;					\
   tcp_ptcp(TopCPS) = ptcpreg;				\
   tcp_pdreg(TopCPS) = delayreg;			\
   tcp_ereg(TopCPS) = ereg;				\
   tcp_cpreg(TopCPS) = cpreg;				\
   tcp_trreg(TopCPS) = trreg;				\
   tcp_hreg(TopCPS) = hreg;				\
   tcp_ebreg(TopCPS) = ebreg;				\
   tcp_subgoal_ptr(TopCPS) = (CPtr)pSF;			\
   SAVE_TPSC(TopCPS);                                   \
   tcp_prevbreg(TopCPS) = breg;				\
   tcp_pcreg(TopCPS) = Cont;				\
 }

#define _SaveProducerCPF_slg(TopCPS, Cont, pSF, AT) {	\
   _SaveProducerCPF_common(TopCPS, Cont, pSF);		\
   tcp_bfreg(TopCPS) = bfreg;				\
   tcp_efreg(TopCPS) = efreg;				\
   tcp_trfreg(TopCPS) = trfreg;				\
   tcp_hfreg(TopCPS) = hfreg;				\
   tcp_template(TopCPS) = AT;                           \
 }

#ifdef LOCAL_EVAL
#define SaveProducerCPF(TopCPS, Cont, pSF, Arity, AT) {	\
   _SaveProducerCPF_slg(TopCPS, Cont, pSF, AT);		\
   tcp_trie_return(TopCPS) = NULL;			\
 }
#else
#define SaveProducerCPF(TopCPS, Cont, pSF, Arity, AT)	\
   _SaveProducerCPF_slg(TopCPS, Cont, pSF, AT)
#endif

/*----------------------------------------------------------------------*/
/* Consumer Choice Point						*/
/*----------------------------------------------------------------------*/

typedef struct consumer_choice_point {
    byte *next_clause;	/* the entry of next choice */
#ifdef SLG_GC
  CPtr prev_top;
#endif
#ifdef CP_DEBUG
    Psc  psc;             /* PSC of predicate that created this CP */
#endif    
    CPtr _ebreg;	/* environment backtrack -- top of env stack */
    CPtr _hreg;		/* current top of heap */
    CPtr *_trreg;	/* current top of trail stack */
    byte *_cpreg;	/* return point of the call to the procedure */
    CPtr _ereg;		/* current top of local stack */
    CPtr pdreg;		/* value of delay register for the parent subgoal */
    CPtr ptcp;		/* pointer to parent tabled CP (subgoal) */
    CPtr prev;		/* prev top of choice point stack */
    CPtr answer_template;
    CPtr subgoal_ptr;	/* where the answer list lives */
    CPtr prevlookup;	/* link for chain of consumer CPFs */
    ALNptr trie_return;	/* last answer consumed by this consumer */
#ifdef CONC_COMPL
    Cell tid ;		/* Thread whose stacks the cp resides in */
#endif
}
*NLChoice;
#define NLCP_SIZE	(sizeof(struct consumer_choice_point)/sizeof(CPtr))

#define nlcp_pcreg(b)		((NLChoice)(b))->next_clause
#define nlcp_ebreg(b)		((NLChoice)(b))->_ebreg
#define nlcp_hreg(b)		((NLChoice)(b))->_hreg
#define nlcp_trreg(b)		((NLChoice)(b))->_trreg
#define nlcp_cpreg(b)		((NLChoice)(b))->_cpreg
#define nlcp_ereg(b)		((NLChoice)(b))->_ereg
#define nlcp_subgoal_ptr(b)	((NLChoice)(b))->subgoal_ptr
#define nlcp_pdreg(b)		((NLChoice)(b))->pdreg
#define nlcp_ptcp(b)		((NLChoice)(b))->ptcp
#define nlcp_prevbreg(b)	((NLChoice)(b))->prev
#define nlcp_prevlookup(b)	((NLChoice)(b))->prevlookup
#define nlcp_trie_return(b)	((NLChoice)(b))->trie_return
#define nlcp_template(b)        ((NLChoice)(b))->answer_template
#ifdef SLG_GC
#define nlcp_prevtop(b)         ((NLChoice)(b))->prev_top
#endif
#ifdef CONC_COMPL
#define nlcp_tid(b)        	((NLChoice)(b))->tid
#endif

#define is_consumer_choicepoint(b) \
    (cp_pcreg(b) == (byte *) &answer_return_inst)

#ifdef CP_DEBUG
#define nlcp_psc(b)              ((NLChoice)(b))->psc
#define SAVE_NLPSC(b)            nlcp_psc(b) = pscreg
#else
#define SAVE_NLPSC(b)
#endif

#define _SaveConsumerCPF_common(TopCPS,SF,PrevConsumer) {	\
   TopCPS -= NLCP_SIZE;						\
   nlcp_trie_return(TopCPS) = subg_ans_list_ptr(SF); 		\
   nlcp_subgoal_ptr(TopCPS) = (CPtr)SF;				\
   nlcp_prevlookup(TopCPS) = PrevConsumer;			\
   nlcp_ptcp(TopCPS) = ptcpreg; 				\
   nlcp_pdreg(TopCPS) = delayreg; 				\
   nlcp_prevbreg(TopCPS) = breg; 				\
   nlcp_ereg(TopCPS) = ereg; 					\
   nlcp_cpreg(TopCPS) = cpreg; 					\
   nlcp_trreg(TopCPS) = trreg; 					\
   SAVE_NLPSC(TopCPS);                                          \
   nlcp_hreg(TopCPS) = hreg; 					\
   nlcp_ebreg(TopCPS) = ebreg; 					\
   nlcp_pcreg(TopCPS) = (pb) &answer_return_inst; 		\
 }

#define SaveConsumerCPF(TopCPS,ConsSF,PrevConsumer, AT)	\
   _SaveConsumerCPF_common(TopCPS,ConsSF,PrevConsumer); \
   nlcp_template(TopCPS) = AT

/*----------------------------------------------------------------------*/
/* The following CP is used for a computation that has been suspended	*/
/* on completion of a subgoal.                                          */
/*----------------------------------------------------------------------*/

typedef struct compl_susp_frame {
  byte *next_clause;	/* the completion suspension instruction */
#ifdef SLG_GC
  CPtr prev_top;
#endif
#ifdef CP_DEBUG
  Psc  psc;             /* PSC of predicate that created this CP */
#endif    
  CPtr _ebreg;		/* environment backtrack -- top of env stack */
  CPtr _hreg;		/* current top of heap */
  CPtr *_trreg;	/* current top of trail stack */
  byte *_cpreg;	/* return point of the call to the procedure */
  CPtr _ereg;		/* current top of stack */
  CPtr pdreg;		/* value of delay register for the parent subgoal */
  CPtr ptcp;		/* pointer to parent tabled CP (subgoal) */
  CPtr prevcsf;	/* previous completion suspension frame */
  Cell neg_loop;	/* !0 if the suspension is not LRD stratified */
  CPtr subgoal_ptr;	/* pointer to the call structure */
} *ComplSuspFrame;

#define CSF_SIZE	(sizeof(struct compl_susp_frame)/sizeof(CPtr))

#define csf_pcreg(b)		((ComplSuspFrame)(b))->next_clause
#define csf_ebreg(b)		((ComplSuspFrame)(b))->_ebreg
#define csf_hreg(b)		((ComplSuspFrame)(b))->_hreg
#define csf_trreg(b)		((ComplSuspFrame)(b))->_trreg
#define csf_cpreg(b)		((ComplSuspFrame)(b))->_cpreg
#define csf_ereg(b)		((ComplSuspFrame)(b))->_ereg
#define csf_pdreg(b)		((ComplSuspFrame)(b))->pdreg
#define csf_ptcp(b)		((ComplSuspFrame)(b))->ptcp
#define csf_subgoal_ptr(b)	((ComplSuspFrame)(b))->subgoal_ptr
#define csf_prevcsf(b)		((ComplSuspFrame)(b))->prevcsf
#define csf_neg_loop(b)		((ComplSuspFrame)(b))->neg_loop
#ifdef SLG_GC
#define csf_prevtop(b)          ((ComplSuspFrame)(b))->prev_top
#endif

#ifdef CP_DEBUG
#define csf_psc(b)              ((ComplSuspFrame)(b))->psc
#define SAVE_CSFPSC(b)            csf_psc(b) = pscreg
#else
#define SAVE_CSFPSC(b)
#endif

#define is_compl_susp_frame(b)				\
     ((cp_pcreg(b) == (byte *) &resume_compl_suspension_inst) ||	\
    (cp_pcreg(b) == (byte *) &resume_compl_suspension_inst2))

/*
#define is_compl_susp_frame(b)				\
  ((cp_pcreg(b) == (byte *) &resume_compl_suspension_inst))
*/
      
#define save_compl_susp_frame(t_breg,t_ereg,subg,t_ptcp,CPREG) \
    t_breg -= CSF_SIZE; \
    csf_neg_loop(t_breg) = FALSE; \
    csf_prevcsf(t_breg) = subg_compl_susp_ptr(subg); \
    csf_ptcp(t_breg) = t_ptcp; \
    SAVE_CSFPSC(t_breg); \
    csf_pdreg(t_breg) = delayreg; \
    csf_subgoal_ptr(t_breg) = subg; \
    csf_ereg(t_breg) = t_ereg; \
    csf_cpreg(t_breg) = CPREG; \
    csf_trreg(t_breg) = trreg; \
    csf_hreg(t_breg) = hreg; \
    csf_ebreg(t_breg) = ebreg; \
    csf_pcreg(t_breg) = (pb) &resume_compl_suspension_inst

/*----------------------------------------------------------------------*/
/* The following CP is used to resume a set of computations that have	*/
/* been suspended on completion of a subgoal.  Only SLG-WAM uses this. 	*/
/*----------------------------------------------------------------------*/


typedef struct compl_susp_choice_point {
    byte *next_clause;	/* the completion suspension instruction */
#ifdef SLG_GC
  CPtr prev_top;
#endif
#ifdef CP_DEBUG
  Psc  psc;             /* PSC of predicate that created this CP */
#endif    
    CPtr _ebreg;	/* environment backtrack -- top of env stack */
    CPtr _hreg;		/* current top of heap */
#ifdef SLG_GC
  CPtr *_trreg;	/* current top of trail stack */
  byte *_cpreg;	/* return point of the call to the procedure */
  CPtr _ereg;		/* current top of stack */
  CPtr pdreg;		/* value of delay register for the parent subgoal */
  CPtr ptcp;		/* pointer to parent tabled CP (subgoal) */
#endif
    CPtr prev;		/* lookup: previous choicepoint */
    CPtr compsuspptr;	/* pointer to the completion_suspension_frame */
} *ComplSuspChoice;

#define COMPL_SUSP_CP_SIZE	\
	(sizeof(struct compl_susp_choice_point)/sizeof(CPtr))

#define cs_pcreg(b)		((ComplSuspChoice)(b))->next_clause
#define cs_ebreg(b)		((ComplSuspChoice)(b))->_ebreg
#define cs_hreg(b)		((ComplSuspChoice)(b))->_hreg
#define cs_compsuspptr(b)	((ComplSuspChoice)(b))->compsuspptr
#define cs_prevbreg(b)		((ComplSuspChoice)(b))->prev
#ifdef SLG_GC
#define SAVE_CSCP_EXTRA(t_breg) \
((ComplSuspChoice)(t_breg))->prev_top = t_breg + COMPL_SUSP_CP_SIZE; \
((ComplSuspChoice)(t_breg))->_trreg = trreg; \
((ComplSuspChoice)(t_breg))->_cpreg = cpreg; \
((ComplSuspChoice)(t_breg))->_ereg = ereg; \
((ComplSuspChoice)(t_breg))->pdreg = delayreg; \
((ComplSuspChoice)(t_breg))->ptcp = ptcpreg
#else
#define SAVE_CSCP_EXTRA(t_breg)
#endif
#ifdef CP_DEBUG
#define cscp_psc(b)              ((ComplSuspChoice)(b))->psc
#define SAVE_CSCPPSC(b)            cscp_psc(b) = pscreg
#else
#define SAVE_CSCPPSC(b)
#endif

#define save_compl_susp_cp(t_breg,prev,compsuspptr) \
    t_breg -= COMPL_SUSP_CP_SIZE; \
    cs_prevbreg(t_breg) = prev; \
    SAVE_CSCP_EXTRA(t_breg); \
    SAVE_CSCPPSC(t_breg); \
    cs_compsuspptr(t_breg) = compsuspptr;\
    cs_hreg(t_breg) = hreg; \
    cs_ebreg(t_breg) = ebreg; \
    cs_pcreg(t_breg) = (pb) &resume_compl_suspension_inst2

/* --------------------------------------------------------------------	*/

/*
 *  Push on the CP Stack the arguments in the X reg's.  't_breg' gets the
 *  topmost Cell on the cpstack, 'arity' is the number of args to be pushed,
 *  'ii' is a variable supplied by the caller for macro use, and 'regbase'
 *  is a pointer to the "lowest" reg holding an argument.
 *
 *  On "exit", 't_breg' points to the topmost arg on the cpstack.
 */
#define save_registers(t_breg, arity, regbase) {\
    byte ii;\
    for (ii = 1; ((int)ii) <= arity; ii++) \
        bld_copy(--t_breg, cell(regbase+ii));\
  }

#define restore_registers(t_breg, arity, regbase) {\
    int ii;\
    t_breg += CP_SIZE; \
    for (ii = arity; ii >= 1; ii--) bld_copy(regbase+ii, cell(t_breg++));\
  }

#define table_restore_registers(t_breg, arity, regbase) {\
    int ii;\
    t_breg += TCP_SIZE; \
    for (ii = arity; ii >= 1; ii--) bld_copy(regbase+ii, cell(t_breg++));\
  }


/* Local (Environment) Stack
   ------------------------- */
/*
 *  Structure of an Activation Record (Environment Frame) is
 *
 * low mem
 *   |   Block of Permanent Variables
 *   |   Continuation Pointer  (CPreg value at procedure invocation)
 *   V   Dynamic Link  (ptr to link field of prev frame)
 * high mem
 *
 *  Ereg always points to the Dynamic Link field of an AR, while EBreg and
 *  EFreg always point to the topmost Permanent Variable in the frame.
 */

/*
 *  Set ebreg to the topmost env frame of those pointed to by ereg or efreg.
 */
#define save_find_locx(t_ereg) \
    if (efreg_on_top(t_ereg)) ebreg = efreg;\
    else if (ereg_on_top(t_ereg)) ebreg = t_ereg - *(cpreg-(2*sizeof(Cell)-3))+1;

#define restore_some_wamregs(t_breg, t_ereg) \
    if (hbreg >= hfreg) hreg = hbreg; else hreg = hfreg; \
    cpreg = cp_cpreg(t_breg); \
    t_ereg = cp_ereg(t_breg); \
    trieinstr_unif_stkptr = trieinstr_unif_stk - 1

#endif
