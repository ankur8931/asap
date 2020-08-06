/* File:      tst_aux.h
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
** $Id: tst_aux.h,v 1.21 2013-01-09 20:15:34 dwarren Exp $
** 
*/


#ifndef PRIVATE_TST_DEFS

#define PRIVATE_TST_DEFS

#include "debugs/debug_tables.h"
#include "debugs/debug_tries.h"
#include "dynamic_stack.h"


/*===========================================================================*/

/*
 *                     Auxiliary Data Structures
 *                     =========================
 *
 *  This file contains typedefs of and macros for using some objects
 *  universally needed by the subsumptive components of the engine,
 *  i.e. the operations:
 *
 *  i) Subsumptive Call Check/Insert
 *  ii) Variant TST Answer Check/Insert
 *  iii) Relevant Answer Identification
 *  iv) Answer Consumption
 */

/*---------------------------------------------------------------------------*/
/*
 *  tstTermStack
 *  ------------
 *  For flattening a heap term when seaching for a subsuming call
 */

#ifndef MULTI_THREAD
extern DynamicStack tstTermStack;
#endif

#define TST_TERMSTACK_INITSIZE    25

#define TermStack_Top		((CPtr)DynStk_Top(tstTermStack))
#define TermStack_Base		((CPtr)DynStk_Base(tstTermStack))
#define TermStack_NumTerms	DynStk_NumFrames(tstTermStack)
#define TermStack_ResetTOS	DynStk_ResetTOS(tstTermStack)
#define TermStack_IsEmpty	DynStk_IsEmpty(tstTermStack)

#define TermStack_SetTOS(Index)			\
   DynStk_Top(tstTermStack) = TermStack_Base + Index

#define TermStack_Push(Term) {			\
   CPtr nextFrame;				\
   DynStk_Push(tstTermStack,nextFrame);		\
   *nextFrame = Term;				\
 }

#define TermStack_BlindPush(Term) {		\
   CPtr nextFrame;				\
   DynStk_BlindPush(tstTermStack,nextFrame);	\
   *nextFrame = Term;				\
 }

#define TermStack_Pop(Term) {			\
   CPtr curFrame;				\
   DynStk_BlindPop(tstTermStack,curFrame);	\
   Term = *curFrame;				\
 }

#define TermStack_Peek(Term) {			\
   CPtr curFrame;				\
   DynStk_BlindPeek(tstTermStack,curFrame);	\
   Term = *curFrame;				\
 }

/* Specialty Pushing Macros
   ------------------------ */
#define TermStack_NOOP      /* Nothing to push when constants match */

#define TermStack_PushFunctorArgs(CS_Cell)                   \
   TermStack_PushLowToHighVector( clref_val(CS_Cell) + 1,    \
				  get_arity(get_str_psc(CS_Cell)) )

#define TermStack_PushAbstractFunctorArgs(CS_Cell)                   \
  TermStack_PushAbstractLowToHighVector( (clref_val(CS_Cell) + 1),      \
					 get_arity(get_str_psc(CS_Cell)) )

// Used for subsumptive answer trie      
#define TermStack_PushListArgs(LIST_Cell) {	\
   CPtr pListHeadCell = clref_val(LIST_Cell);	\
   DynStk_ExpandIfOverflow(tstTermStack,2);	\
   TermStack_BlindPush( *(pListHeadCell + 1) );	\
   TermStack_BlindPush( *(pListHeadCell) );	\
 }

// Used for subsumptive call trie                                                                                           
#define TermStack_PushAbstractListArgs(LIST_Cell) {                     \
    CPtr pListHeadCell = clref_val(LIST_Cell);                          \
    DynStk_ExpandIfOverflow(tstTermStack,2);                            \
    if (isattv((*(pListHeadCell + 1))))                                 \
      *(pListHeadCell + 1) = (dec_addr((*(pListHeadCell + 1))) | XSB_REF); \
    TermStack_BlindPush( *(pListHeadCell + 1) );                        \
    if (isattv((*pListHeadCell)))                                       \
      *pListHeadCell = (dec_addr((*pListHeadCell)) | XSB_REF);          \
    TermStack_BlindPush( *(pListHeadCell) );                            \
  }

/*
 * The following macros enable the movement of an argument vector to
 * the TermStack.  Two versions are supplied depending on whether the
 * vector is arranged from high-to-low memory, such as an answer
 * template, or from low-to-high memory, such as the arguments of a
 * compound heap term.  The vector pointer is assumed to reference the
 * first element of the vector.
 */

#define TermStack_PushLowToHighVector(pVectorLow,Magnitude) {	\
   size_t i, numElements;						\
   CPtr pElement;						\
								\
   numElements = Magnitude;					\
   pElement = pVectorLow + numElements;				\
   DynStk_ExpandIfOverflow(tstTermStack,numElements);		\
   for (i = 0; i < numElements; i++) {				\
     pElement--;						\
     TermStack_BlindPush(*pElement);				\
   }								\
 }
   

// Used for subsumptive call trie                                                           
#define TermStack_PushAbstractLowToHighVector(pVectorLow,Magnitude) {   \
    int i, numElements;                                          \
    CPtr pElement;                                               \
                                                                \
    numElements = Magnitude;                                     \
    pElement = pVectorLow + numElements;                         \
    DynStk_ExpandIfOverflow(tstTermStack,numElements);           \
    for (i = 0; i < numElements; i++) {                          \
      pElement--;                                                \
      if (isattv(*pElement))                                     \
	*pElement = (dec_addr(*pElement) | XSB_REF);           \
      TermStack_BlindPush(*pElement);                            \
    }                                                            \
  }

#define TermStack_PushHighToLowVector(pVectorHigh,Magnitude) {	\
   size_t i; size_t numElements;					\
   CPtr pElement;						\
								\
   numElements = Magnitude;					\
   pElement = pVectorHigh - numElements;			\
   DynStk_ExpandIfOverflow(tstTermStack,numElements);		\
   for (i = 0; i < numElements; i++) {				\
     pElement++;						\
     TermStack_BlindPush(*pElement);				\
   }								\
 }

/*
 * This macro copies an array of terms onto the TermStack, checking for
 * overflow only once at the beginning, rather than with each push.  The
 * elements to be pushed are assumed to exist in array elements
 * 0..(NumElements-1).
 */

#define TermStack_PushArray(Array,NumElements) {	\
   counter i;						\
   DynStk_ExpandIfOverflow(tstTermStack,NumElements);	\
   for ( i = 0;  i < NumElements;  i++ )		\
     TermStack_BlindPush(Array[i]);			\
 }

/* ------------------------------------------------------------------------- */

/*
 *  tstTermStackLog
 *  ---------------
 *  For noting the changes made to the tstTermStack during processing
 *  where backtracking is required.  Only the changes necessary to
 *  transform the tstTermStack from its current state to a prior state
 *  are logged.  Therefore, we only need record values popped from the
 *  tstTermStack.
 *
 *  Each frame of the log consists of the index of a tstTermStack
 *  frame and the value that was stored there.  tstTermStack values
 *  are reinstated as a side effect of a tstTermStackLog_Pop.
 */

typedef struct {
  int index;             /* location within the tstTermStack... */
  Cell value;            /* where this value appeared. */
} tstLogFrame;
typedef tstLogFrame *pLogFrame;

#define LogFrame_Index(Frame)	( (Frame)->index )
#define LogFrame_Value(Frame)	( (Frame)->value )


#ifndef MULTI_THREAD
extern DynamicStack tstTermStackLog;
#endif
#define TST_TERMSTACKLOG_INITSIZE    20

#define TermStackLog_Top	((pLogFrame)DynStk_Top(tstTermStackLog))
#define TermStackLog_Base	((pLogFrame)DynStk_Base(tstTermStackLog))
#define TermStackLog_ResetTOS	DynStk_ResetTOS(tstTermStackLog)

/*
 * This doesn't always immediately follow a pop of the TermStack (see
 * tst_retrv.c).  Therefore the subterm is obtained from the TermStack
 * directly before more terms are pushed onto it.
 */
#define TermStackLog_PushFrame {				\
   pLogFrame nextFrame;						\
   DynStk_Push(tstTermStackLog,nextFrame);			\
   LogFrame_Index(nextFrame) = (int)(TermStack_Top - TermStack_Base);	\
   LogFrame_Value(nextFrame) = *(TermStack_Top);		\
 }

#define TermStackLog_PopAndReset {					\
   pLogFrame curFrame;							\
   DynStk_BlindPop(tstTermStackLog,curFrame)				\
   TermStack_Base[LogFrame_Index(curFrame)] = LogFrame_Value(curFrame);	\
 }

/*
 * Reset the TermStack elements down to and including the Index-th
 * entry in the Log.
 */
#define TermStackLog_Unwind(Index) {			\
   pLogFrame unwindBase = TermStackLog_Base + Index;	\
   while ( TermStackLog_Top > unwindBase )		\
     TermStackLog_PopAndReset;				\
 }

/* ------------------------------------------------------------------------- */

/*
 *  tstTrail
 *  ---------
 *  For recording bindings made during processing.  This Trail performs
 *  simple WAM trailing -- it saves address locations only.
 */

#ifndef MULTI_THREAD
extern DynamicStack tstTrail;
#endif
#define TST_TRAIL_INITSIZE    20

#define Trail_Top		((CPtr *)DynStk_Top(tstTrail))
#define Trail_Base		((CPtr *)DynStk_Base(tstTrail))
#define Trail_NumBindings	DynStk_NumFrames(tstTrail)
#define Trail_ResetTOS		DynStk_ResetTOS(tstTrail)

#define tstTrail_Push(Addr) {			\
   CPtr *nextFrame;				\
   DynStk_Push(tstTrail,nextFrame);		\
   *nextFrame = (CPtr)(Addr);			\
 }

#define Trail_PopAndReset {			\
   CPtr *curFrame;				\
   DynStk_BlindPop(tstTrail,curFrame);		\
   bld_free(*curFrame);				\
 }

#define Trail_Unwind_All	Trail_Unwind(0)

/*
 * Untrail down to and including the Index-th element.
 */
#define Trail_Unwind(Index) {			\
   CPtr *unwindBase = Trail_Base + Index;	\
   while ( Trail_Top > unwindBase )		\
     Trail_PopAndReset;				\
 }

/* ------------------------------------------------------------------------- */

/*
 *  tstSymbolStack 
 * --------------- 
 * TLS: I think Ernie Created this for some reason, but I repurposed
 * it for performing simplification for tables w. call subsumption.
 *  See slgdelay.c
 */

#ifndef MULTI_THREAD
extern DynamicStack tstSymbolStack;
#endif
#define TST_SYMBOLSTACK_INITSIZE   25

#define SymbolStack_Top		  ((CPtr)DynStk_Top(tstSymbolStack))
#define SymbolStack_Base	  ((CPtr)DynStk_Base(tstSymbolStack))
#define SymbolStack_NumSymbols	  (SymbolStack_Top - SymbolStack_Base)
#define SymbolStack_ResetTOS	  DynStk_ResetTOS(tstSymbolStack)
#define SymbolStack_IsEmpty	  DynStk_IsEmpty(tstSymbolStack)

#define SymbolStack_Push(Symbol) {		\
   CPtr nextFrame;				\
   DynStk_Push(tstSymbolStack,nextFrame);	\
   *nextFrame = Symbol;				\
 }

#define SymbolStack_Pop(Symbol) {		\
   CPtr curFrame;				\
   DynStk_BlindPop(tstSymbolStack,curFrame);	\
   Symbol = *curFrame;				\
}

#define SymbolStack_Peek(Symbol) {		\
   CPtr curFrame;				\
   DynStk_BlindPeek(tstSymbolStack,curFrame);	\
   Symbol = *curFrame;				\
}

#define SymbolStack_PushPathRoot(Leaf,Root) {	\
   BTNptr btn = (BTNptr)Leaf;			\
   while ( ! IsTrieRoot(btn) ) {		\
     SymbolStack_Push(BTN_Symbol(btn));		\
     btn = BTN_Parent(btn);			\
   }						\
   Root = (void *)btn;				\
 }

#define SymbolStack_PushPath(Leaf) {  		\
   BTNptr root;					\
   SymbolStack_PushPathRoot(Leaf,root);		\
   root = root;  /* to squash warnings */       \
 }

/*=========================================================================*/

/*
 *			Using the Trie Stacks
 *			=====================
 */

/* Messy for now - until I'm sure that attributed variables are working correctly */

#define ProcessNextSubtermFromTrieStacks(Symbol,StdVarNum) {	\
								\
   Cell subterm;						\
								\
   TermStack_Pop(subterm);					\
   XSB_Deref(subterm);						\
   /*   fprintf(stddbg,"ProcessNext ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */ \
   switch ( cell_tag(subterm) ) {				\
   case XSB_REF:						\
   case XSB_REF1:						\
     /*     fprintf(stddbg,"  ProcessNext: variable VE %p\n",VarEnumerator);*/ \
     if ( ! IsStandardizedVariable(subterm) ) {			\
       StandardizeVariable(subterm, StdVarNum);			\
       tstTrail_Push(subterm);					\
       Symbol = EncodeNewTrieVar(StdVarNum);			\
       StdVarNum++;						\
       /*       fprintf(stddbg,"  Var: standardized variable %p %p %d\n",subterm,*(CPtr) subterm,StdVarNum); */	\
     }								\
     else							\
       Symbol = EncodeTrieVar(IndexOfStdVar(subterm));		\
     break;							\
   case XSB_STRING:						\
   case XSB_INT:						\
   case XSB_FLOAT:						\
     /*     printf("found constant ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n");*/ \
     Symbol = EncodeTrieConstant(subterm);			\
     break;							\
   case XSB_STRUCT:						\
     /*     printf("found struct ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */ \
     Symbol = EncodeTrieFunctor(subterm);			\
     TermStack_PushFunctorArgs(subterm);			\
     break;							\
   case XSB_LIST:						\
     /*     printf("found list ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */	\
     Symbol = EncodeTrieList(subterm);				\
     TermStack_PushListArgs(subterm);				\
     break;							\
   case XSB_ATTV:						\
     /*     xsb_table_error(CTXTc					\
	    "Attributed variables not yet implemented in answers for subsumptive tables.");*/ \
     if ( ! IsStandardizedVariable(subterm) ) {			\
       /*       fprintf(stddbg,"  Attv Standardizing %p %p",subterm,clref_val(subterm));printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */ \
       StandardizeVariable(clref_val(subterm), StdVarNum);		\
       /*       fprintf(stddbg,"  Attv Standardized (VE) %p (Num)%d (subterm) %x (@subterm) %p clref %p @clref%x\n",VarEnumerator,StdVarNum,subterm,*(CPtr) subterm,clref_val(subterm),*clref_val(subterm)); */	\
       tstTrail_Push(clref_val(subterm));				\
       Symbol = EncodeNewTrieAttv(StdVarNum);			\
       StdVarNum++;						\
       /*       fprintf(stddbg,"  Attv attribute %p",clref_val(subterm)+1);printterm(stddbg,*(clref_val(subterm)+1),25);fprintf(stddbg,"\n"); */ \
       TermStack_Push(cell(clref_val(subterm)+1));				\
     }								\
     else							\
       Symbol = EncodeTrieVar(IndexOfStdVar(subterm));		\
     break;							\
   default:							\
     Symbol = 0;  /* avoid "uninitialized" compiler warning */	\
     TrieError_UnknownSubtermTag(subterm);			\
   }								\
 }

                       
/*=========================================================================*/

#define ProcessAbstractNextSubtermFromTrieStacks(Symbol,StdVarNum) {    \
                                                                \
    Cell subterm;                                                \
                                                                \
    TermStack_Pop(subterm);                                      \
    XSB_Deref(subterm);                                          \
    /*   fprintf(stddbg,"ProcessNext ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */ \
    switch ( cell_tag(subterm) ) {                               \
    case XSB_ATTV:                                               \
      subterm = (dec_addr(subterm) | XSB_REF);                   \
    case XSB_REF:                                                \
    case XSB_REF1:                                               \
      /*     fprintf(stddbg,"  ProcessNext: variable VE %p\n",VarEnumerator);*/ \
      if ( ! IsStandardizedVariable(subterm) ) {                 \
	StandardizeVariable(subterm, StdVarNum);                 \
	tstTrail_Push(subterm);                                     \
	Symbol = EncodeNewTrieVar(StdVarNum);                    \
	StdVarNum++;                                             \
	/*       fprintf(stddbg,"  Var: standardized variable %p %p %d\n",subterm,*(CPtr) subterm,StdVarNum); */ \
      }                                                          \
      else                                                       \
	Symbol = EncodeTrieVar(IndexOfStdVar(subterm));          \
      break;                                                     \
    case XSB_STRING:                                             \
    case XSB_INT:                                                \
    case XSB_FLOAT:                                              \
      /*     printf("found constant ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n");*/ \
      Symbol = EncodeTrieConstant(subterm);                      \
      break;                                                     \
    case XSB_STRUCT:                                             \
      /*     printf("found struct ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n");*/ \
      Symbol = EncodeTrieFunctor(subterm);                       \
      TermStack_PushAbstractFunctorArgs(subterm);                \
      break;                                                     \
    case XSB_LIST:                                               \
      /*     printf("found list ");printterm(stddbg,subterm,25);fprintf(stddbg,"\n"); */ \
      Symbol = EncodeTrieList(subterm);                          \
      TermStack_PushAbstractListArgs(subterm);                   \
      break;                                                     \
    default:                                                     \
      Symbol = 0;  /* avoid "uninitialized" compiler warning */  \
      TrieError_UnknownSubtermTag(subterm);                      \
    }                                                            \
  }


/*=========================================================================*/


#endif
