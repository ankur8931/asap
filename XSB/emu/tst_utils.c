/* File:      tst_utils.c
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
** $Id: tst_utils.c,v 1.47 2013-05-06 21:10:25 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "binding.h"
#include "psc_xsb.h"
#include "register.h"
#include "deref.h"
#include "trie_internals.h"
#include "tst_aux.h"
#include "tab_structs.h"
#include "choice.h"
#include "inst_xsb.h"
#include "error_xsb.h"
#include "cell_xsb_i.h"

/* ======================================================================= */

/*
 *            Stacks needed by TST-Subsumptive Operations
 *            ===========================================
 */


#ifndef MULTI_THREAD
DynamicStack  tstTermStack;
DynamicStack  tstTermStackLog;
DynamicStack  tstSymbolStack;
DynamicStack  tstTrail;
#endif

DynamicStack simplGoalStack;
DynamicStack simplAnsStack;

void tstInitDataStructs(CTXTdecl) {

  extern void initCollectRelevantAnswers(CTXTdecl);
  extern void initSubsumptiveLookup(CTXTdecl);

  DynStk_Init(&tstTermStack, 0 /*TST_TERMSTACK_INITSIZE*/, Cell, "TST Term Stack");
  DynStk_Init(&tstTermStackLog, 0 /*TST_TERMSTACKLOG_INITSIZE*/, tstLogFrame,
	      "TST TermStackLog");
  DynStk_Init(&tstSymbolStack, 0 /*TST_SYMBOLSTACK_INITSIZE*/, Cell,
	      "Trie-Symbol Stack");
  DynStk_Init(&tstTrail, 0 /*TST_TRAIL_INITSIZE*/, CPtr, "TST Trail");

  DynStk_Init(&simplGoalStack, 0 /*TST_TRAIL_INITSIZE*/, Cell, "simplGoalStack");
  DynStk_Init(&simplAnsStack, 0 /*TST_TRAIL_INITSIZE*/, Cell, "simplAnsStack");

  initSubsumptiveLookup(CTXT);
  initCollectRelevantAnswers(CTXT);
}


void tstShrinkDynStacks(CTXTdecl) {

  dsShrink(&tstTermStack);
  dsShrink(&tstTermStackLog);
  dsShrink(&tstSymbolStack);
  dsShrink(&tstTrail);
}

/* ======================================================================= */

/*
 *		    D E B U G G I N G   R O U T I N E S
 *	            ===================================
 */


/*-------------------------------------------------------------------------*/

/*
 *			  Printing Trie Nodes
 *			  -------------------
 */

char *stringNodeStatus(byte fieldNodeStatus) {

  if ( fieldNodeStatus == VALID_NODE_STATUS )
    return "Valid";
  else
    return inst_name(fieldNodeStatus);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *TrieTypeStrings[] = {
  "Call Trie", "Basic Answer Trie", "Time-Stamped Answer Trie",
  "Delay Trie", "Asserted Trie", "Interned Trie", "--"
};

char *stringTrieType(byte fieldTrieType) {

  if ( fieldTrieType > 5 )
    return TrieTypeStrings[6];
  else
    return TrieTypeStrings[fieldTrieType];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

char *NodeTypeStrings[] = {
  "Interior", "Indexed Interior", "Leaf", "Indexed Leaf",
  "Index Header", "--", "--", "--", "Root",
};

char *stringNodeType(byte fieldNodeType) {

  if ( fieldNodeType > 8 )
    return NodeTypeStrings[5];
  else
    return NodeTypeStrings[fieldNodeType];
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void printTrieSymbol(FILE *fp, Cell symbol) {

  if ( symbol == ESCAPE_NODE_SYMBOL )
    fprintf(fp, "%lu [ESCAPE_NODE_SYMBOL]", (unsigned long) ESCAPE_NODE_SYMBOL);
  else {
    switch(TrieSymbolType(symbol)) {
    case XSB_INT:
      fprintf(fp, "%"Intfmt, int_val(symbol));
      break;
    case XSB_FLOAT:
      fprintf(fp, "%f", float_val(symbol));
      break;
    case XSB_STRING:
      fprintf(fp, "%s", string_val(symbol));
      break;
    case XSB_TrieVar:
      fprintf(fp, "_V%" Intfmt, DecodeTrieVar(symbol));
      break;
    case XSB_STRUCT:
      {
	Psc psc;
	if (isboxedfloat(symbol))
          {
              fprintf(fp, "%lf", boxedfloat_val(symbol));
              break;              
          }
	psc = DecodeTrieFunctor(symbol);
	fprintf(fp, "%s/%d", get_name(psc), get_arity(psc));
      }
      break;
    case XSB_LIST:
      fprintf(fp, "LIST");
      break;
    default:
      fprintf(fp, "Unknown symbol (tag = %" Intfmt")", cell_tag(symbol));
      break;
    }
  }
}

int sprintTrieSymbol(char * buffer, Cell symbol) {
  int ctr;

  if ( symbol == ESCAPE_NODE_SYMBOL )
    return sprintf(buffer, "%lu [ESCAPE_NODE_SYMBOL]", (unsigned long) ESCAPE_NODE_SYMBOL);
  else {
    switch(TrieSymbolType(symbol)) {
    case XSB_INT:
      return sprintf(buffer, "%"Intfmt, int_val(symbol));
      break;
    case XSB_FLOAT:
      return sprintf(buffer, "%f", float_val(symbol));
      break;
    case XSB_STRING:
      return sprintf(buffer, "%s", string_val(symbol));
      break;
    case XSB_TrieVar:
      return sprintf(buffer, "_V%" Intfmt, DecodeTrieVar(symbol));
      break;
    case XSB_STRUCT:
      {
	Psc psc;
	if (isboxedfloat(symbol))
          {
              return sprintf(buffer, "%lf", boxedfloat_val(symbol));
              break;              
          }
	psc = DecodeTrieFunctor(symbol);
	ctr = sprint_quotedname(buffer, 0, get_name(psc));
	return sprintf(buffer+ctr, "/%d", get_arity(psc));
      }
      break;
    case XSB_LIST:
      return sprintf(buffer, "LIST");
      break;
    default:
      return sprintf(buffer, "Unknown symbol (tag = %" Intfmt")", cell_tag(symbol));
      break;
    }
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void printTrieNode(FILE *fp, BTNptr pTN) {

  fprintf(fp, "Trie Node: Addr(%p)", pTN);
  if ( IsDeletedNode(pTN) )
    fprintf(fp, "  (DELETED)");
  fprintf(fp, "\n\tInstr(%s), Status(%s), NodeType(%s),\n"
	  "\tTrieType(%s), Symbol(",
	  inst_name(TN_Instr(pTN)),
	  stringNodeStatus(TN_Status(pTN)),
	  stringNodeType(TN_NodeType(pTN)),
	  stringTrieType(TN_TrieType(pTN)));
  printTrieSymbol(fp, TN_Symbol(pTN));
  fprintf(fp, ")");
  if ( IsInTimeStampedTrie(pTN) )
    fprintf(fp, ", TimeStamp(%" Intfmt, TSTN_TimeStamp((TSTNptr)pTN));
  fprintf(fp, "\n\tParent(%p), Child(%p), Sibling(%p)\n",
	 TN_Parent(pTN), TN_Child(pTN), TN_Sibling(pTN));
}

/*-----------------------------------------------------------------------*/

/*
 *		Trie-Path Printing Using the SymbolStack
 *              (i.e. tstSymbolStack)
 *		----------------------------------------
 */

static void symstkPrintNextTerm(CTXTdeclc FILE *fp, xsbBool list_recursion) {

  Cell symbol;

  if ( SymbolStack_IsEmpty ) {
    fprintf(fp, "<no subterm>");
    return;
  }
  SymbolStack_Pop(symbol);
  switch ( TrieSymbolType(symbol) ) {
  case XSB_INT:
    if ( list_recursion )
      fprintf(fp, "|%" Intfmt "]", int_val(symbol));
    else
      fprintf(fp, "%"Intfmt, int_val(symbol));
    break;
  case XSB_FLOAT:
    if ( list_recursion )
      fprintf(fp, "|%f]", float_val(symbol));
    else
      fprintf(fp, "%f", float_val(symbol));
    break;
  case XSB_STRING:
    {
      char *string = string_val(symbol);
      if ( list_recursion ) {
	if ( string == nil_string )
	  fprintf(fp, "]");
	else
	  fprintf(fp, "|%s]", string);
      }
      else
	fprintf(fp, "%s", string);
    }
    break;
  case XSB_TrieVar:
    if ( list_recursion )
      fprintf(fp, "|V%" Intfmt "]", DecodeTrieVar(symbol));
    else
      fprintf(fp, "_V%" Intfmt, DecodeTrieVar(symbol));
    break;
  case XSB_STRUCT:
    {
      Psc psc;
      int i;
      if (isboxedfloat(symbol))
      {
        if ( list_recursion )
          fprintf(fp, "|%lf]", boxedfloat_val(symbol));
        else
          fprintf(fp, "%lf", boxedfloat_val(symbol));
        break;         
      }

      if ( list_recursion )
	fprintf(fp, "|");
      psc = DecodeTrieFunctor(symbol);
      fprintf(fp, "%s(", get_name(psc));
      for (i = 1; i < (int)get_arity(psc); i++) {
	symstkPrintNextTerm(CTXTc fp,FALSE);
	fprintf(fp, ",");
      }
      symstkPrintNextTerm(CTXTc fp,FALSE);
      fprintf(fp, ")");
      if ( list_recursion )
	fprintf(fp, "]");
    }
    break;
  case XSB_LIST:
    if ( list_recursion )
      fprintf(fp, ",");
    else
      fprintf(fp, "[");
    symstkPrintNextTerm(CTXTc fp,FALSE);
    symstkPrintNextTerm(CTXTc fp,TRUE);
    break;
  default:
    fprintf(fp, "<unknown symbol>");
    break;
  }
}

extern int sprint_quotedname(char *buffer, int size,char *string);

static int symstkSPrintNextTerm(CTXTdeclc char * buffer, xsbBool list_recursion) {
  int ctr = 0;
  Cell symbol;

  if ( SymbolStack_IsEmpty ) {
    //    fprintf(fp, "<no subterm>");
    return 0;
  }
  SymbolStack_Pop(symbol);
  switch ( TrieSymbolType(symbol) ) {
  case XSB_INT:
    if ( list_recursion )
      ctr = ctr + sprintf(buffer+ctr, "|%" Intfmt "]", int_val(symbol));
    else
      ctr = ctr + sprintf(buffer+ctr, "%"Intfmt, int_val(symbol));
    break;
  case XSB_FLOAT:
    if ( list_recursion )
      ctr = ctr + sprintf(buffer+ctr, "|%f]", float_val(symbol));
    else
      ctr = ctr + sprintf(buffer+ctr, "%f", float_val(symbol));
    break;
  case XSB_STRING:
    {
      char *string = string_val(symbol);
      if ( list_recursion ) {
	if ( string == nil_string )
	  ctr = ctr + sprintf(buffer+ctr, "]");
	else {
	  //	  ctr = ctr + sprintf(buffer+ctr, "|%s]", string);
	  ctr = ctr + sprintf(buffer+ctr, "|");
	  ctr = sprint_quotedname(buffer, ctr, string);
	  ctr = ctr + sprintf(buffer+ctr, "]");
	}
      }
      else
	//	ctr = ctr + sprintf(buffer+ctr, "%s", string);
	ctr = sprint_quotedname(buffer, ctr, string);
    }
    break;
  case XSB_TrieVar:
    if ( list_recursion )
      ctr = ctr + sprintf(buffer+ctr, "|V%" Intfmt "]", DecodeTrieVar(symbol));
    else
      ctr = ctr + sprintf(buffer+ctr, "_V%" Intfmt, DecodeTrieVar(symbol));
    break;
  case XSB_STRUCT:
    {
      Psc psc;
      int i;
      if (isboxedfloat(symbol))
      {
        if ( list_recursion ) 
	  ctr = ctr + sprintf(buffer+ctr, "|%lf]", boxedfloat_val(symbol));
        else
          ctr = ctr + sprintf(buffer+ctr, "%lf", boxedfloat_val(symbol));
        break;         
      }

      if ( list_recursion )
	ctr = ctr + sprintf(buffer+ctr, "|");
      psc = DecodeTrieFunctor(symbol);
      //      ctr = ctr + sprintf(buffer+ctr, "%s(", get_name(psc));
      ctr = sprint_quotedname(buffer, ctr, get_name(psc));
      ctr = ctr + sprintf(buffer+ctr, "(");
      for (i = 1; i < (int)get_arity(psc); i++) {
	ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
	ctr = ctr + sprintf(buffer+ctr, ",");
      }
      ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
      ctr = ctr + sprintf(buffer+ctr, ")");
      if ( list_recursion )
	ctr = ctr + sprintf(buffer+ctr, "]");
    }
    break;
  case XSB_LIST:
    if ( list_recursion )
      ctr = ctr + sprintf(buffer+ctr, ",");
    else
      ctr = ctr + sprintf(buffer+ctr, "[");
    ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
    ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,TRUE);
    break;
  default:
    ctr = ctr + sprintf(buffer+ctr, "<unknown symbol>");
    break;
  }
  return ctr;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void printTriePath(CTXTdeclc FILE *fp, BTNptr pLeaf, xsbBool printLeafAddr) {

  BTNptr pRoot;

  if ( IsNULL(pLeaf) ) {
    fprintf(fp, "NULL");
    return;
  }

  if ( ! IsLeafNode(pLeaf) ) {
    fprintf(fp, "printTriePath() called with non-Leaf node!\n");
    printTrieNode(fp, pLeaf);
    return;
  }

  if ( printLeafAddr )
    fprintf(fp, "Leaf %p: ", pLeaf);

  if  (IsEscapeNode(pLeaf) ) {
    pRoot = BTN_Parent(pLeaf);
    if ( IsNonNULL(pRoot) )
      printTrieSymbol(fp, BTN_Symbol(pRoot));
    else
      fprintf(fp, "ESCAPE node");
    return;
  }

  SymbolStack_ResetTOS;
  SymbolStack_PushPathRoot(pLeaf,pRoot);
  if ( IsTrieFunctor(BTN_Symbol(pRoot)) ) {
    SymbolStack_Push(BTN_Symbol(pRoot));
    symstkPrintNextTerm(CTXTc fp,FALSE);
  }
  else {
    printTrieSymbol(fp,BTN_Symbol(pRoot));
    fprintf(fp, "(");
    symstkPrintNextTerm(CTXTc fp,FALSE);
    while ( ! SymbolStack_IsEmpty ) {
      fprintf(fp, ",");
      symstkPrintNextTerm(CTXTc fp,FALSE);
    }
    fprintf(fp, ")");
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int sprintTriePath(CTXTdeclc char * buffer, BTNptr pLeaf) {

  BTNptr pRoot;
  int ctr = 0;

  if ( IsNULL(pLeaf) ) {
    return sprintf(buffer, "NULL");
  }

  if ( ! IsLeafNode(pLeaf) ) {
    fprintf(stderr, "printTriePath() called with non-Leaf node!\n");
    printTrieNode(stderr, pLeaf);
    return -1;
  }

  if  (IsEscapeNode(pLeaf) ) {
    //    printf("escape\n");
    pRoot = BTN_Parent(pLeaf);
    if ( IsNonNULL(pRoot) ) {
      ctr = ctr + sprintTrieSymbol(buffer+ctr, BTN_Symbol(pRoot));
      return ctr;
    }
    else {
      fprintf(stderr, "ESCAPE node");
      return 0;
    }
  }

  SymbolStack_ResetTOS;
  SymbolStack_PushPathRoot(pLeaf,pRoot);
  if ( IsTrieFunctor(BTN_Symbol(pRoot)) ) {
    //    printf("tf\n");
    SymbolStack_Push(BTN_Symbol(pRoot));
    ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
  }
  else {
    //    printf("nontf\n");
    ctr = ctr + sprintTrieSymbol(buffer+ctr,BTN_Symbol(pRoot));
    ctr = ctr + sprintf(buffer+ctr, "(");
    ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
    while ( ! SymbolStack_IsEmpty ) {
      ctr = ctr + sprintf(buffer+ctr, ",");
      ctr = ctr + symstkSPrintNextTerm(CTXTc buffer+ctr,FALSE);
    }
        ctr = ctr + sprintf(buffer+ctr, ")");
  }
  return ctr;
}

/*-----------------------------------------------------------------------*/

/*
 *		       Printing the Answer Template
 *		       ----------------------------
 *
 *  The answer template is assumed to be stored in high-to-low memory
 *  vector fashion, with the incoming template pointer pointing to the
 *  first term Cell (highest memory Cell of the vector).
 */

void printAnswerTemplate(FILE *fp, CPtr pAnsTmplt, int size) {

  int i;

  fprintf(fp, "Answer Template %p:\n\tret(",pAnsTmplt);
  if (size > 0) {
    for (i = 1; i < size; i++) {
      printf("\t");printterm(fp, *pAnsTmplt--, 10);fprintf(stddbg,"-%p",(void *)*(pAnsTmplt+1));
      fprintf(fp, ",\n");
    }
    printf("\t");printterm(fp, *pAnsTmplt, 10);fprintf(stddbg,"-%p",(void *)*(pAnsTmplt));
  }
  fprintf(fp, "\t\n");
}

/*-------------------------------------------------------------------------*/

/*
 *		Printing Componenets of a Subgoal Frame
 *		---------------------------------------
 *
 * Functions for subgoal frame printing can be found in debug_xsb.c.
 */


/* Printing a SF's associated Call
   ------------------------------- */
void sfPrintGoal(CTXTdeclc FILE *fp, VariantSF pSF, xsbBool printAddr) {

  if ( printAddr )
    fprintf(fp, "SF %p  ", pSF);
  printTriePath(CTXTc fp, subg_leaf_ptr(pSF), NO);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Printing Subsumed Calls
   ----------------------- */
void sfPrintConsGoals(CTXTdeclc FILE *fp, SubProdSF pProd) {

  SubConsSF pCons;

  fprintf(fp, "Producer:\n  ");
  sfPrintGoal(CTXTc fp, (VariantSF)pProd, YES);
  fprintf(fp, "\nConsumers:\n");
  for ( pCons = subg_consumers(pProd);  IsNonNULL(pCons);
        pCons = conssf_consumers(pCons) ) {
    fprintf(fp, "  ");
    sfPrintGoal(CTXTc fp, (VariantSF)pCons, YES);
    fprintf(fp, "\n");
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* Printing Answers in an Answer List
   ---------------------------------- */
void printAnswerList(CTXTdeclc FILE *fp, ALNptr pALN) {

  fprintf(fp, "Answer List %p:\n", pALN);
  while ( IsNonNULL(pALN) ) {
    fprintf(fp, "  ");
    printTriePath(CTXTc fp, ALN_Answer(pALN), YES);
    fprintf(fp, "\n");
    pALN = ALN_Next(pALN);
  }
}
/*-------------------------------------------------------------------------*/

/*
 *			    Miscellaneous
 *			    -------------
 */

void printTabledCall(FILE *fp, TabledCallInfo callInfo) {

  int arity, i;
  Psc pPSC;
  
  pPSC = TIF_PSC(CallInfo_TableInfo(callInfo));
  fprintf(fp, "%s(", get_name(pPSC));
  arity = CallInfo_CallArity(callInfo);
  for (i = 0; i < arity; i++) {
    printterm( fp, (Cell)(CallInfo_Arguments(callInfo)+i), 25 );
    if (i+1 < arity)
      fprintf(fp, ",");
  }
  fprintf(fp, ")");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void printTriePathType(CTXTdeclc FILE *fp, TriePathType type, BTNptr leaf) {

  switch (type) {
  case NO_PATH:
    fprintf(fp,"No path found :-(\n");
    break;
  case VARIANT_PATH:
    fprintf(fp,"Variant path found: ");
    printTriePath(CTXTc fp,leaf,FALSE);
    break;
  case SUBSUMPTIVE_PATH:
    fprintf(fp,"Subsumptive path found: ");
    printTriePath(CTXTc fp,leaf,FALSE);
    break;
  default:
    fprintf(fp,"What kind of path? (%d)\n", type);
    break;
  }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void printSymbolStack(CTXTdeclc FILE *fp, char * name, DynamicStack stack) {
  CPtr curFrame;		
  
  curFrame = DynStk_Top(stack);
  curFrame = (CPtr) ((char *) curFrame - DynStk_FrameSize(stack));
  while ((void *)curFrame >= DynStk_Base(stack)) {
    printf("%s: ",name);printTrieSymbol(fp, *curFrame);fprintf(fp,"\n");
    curFrame = (CPtr) ((char *) curFrame - DynStk_FrameSize(stack));
  }
}
