/* File:      trie_search.c
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
** $Id: trie_search.c,v 1.19 2010-12-07 21:08:02 tswift Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include "debugs/debug_tries.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "deref.h"
#include "psc_xsb.h"
#include "trie_internals.h"
#include "tst_aux.h"
#include "debug_xsb.h"
#include "flags_xsb.h"

/****************************************************************************
*
*   In XSB parlance, "search" routines look for something in an entity
*   and insert it if it doesn't already exist.
*
*   The following routines search different flavors of tries for sets
*   of terms using either variant or subsumptive match criteria.
*
*     TSTNptr subsumptive_tst_search(TSTNptr,int,CPtr,xsbBool,xsbBool *)
*  (Not used)   BTNptr  subsumptive_bt_search(BTNptr,int,CPtr,xsbBool *)
*  (Not used)   TSTNptr variant_tst_search(TSTNptr,int,CPtr,xsbBool,xsbBool *)
*  (Not used)   BTNptr  variant_bt_search(BTNptr,int,CPtr,xsbBool *)
*
*   They assume they are given a non-NULL trie pointer and accept a
*   term set as an integer count and an array of terms.
*
*   As of 11/05, the only one of these used is subsumptive_tst_search()
*
****************************************************************************/


/*=========================================================================*/

/*
 *			Searching for an Escape Node
 *			============================
 *
 * Conditionally creates and returns the Escape node in the trie, given
 * as a non-NULL root pointer.  Indicates to the caller whether this
 * node was preexisting through the second argument.  Aborts the
 * computation if something other than an escape node is found in the
 * trie.
 *
 * These functions are intended for internal use by other search
 * routines.
 */


/*-------------------------------------------------------------------------*/

BTNptr bt_escape_search(CTXTdeclc BTNptr btRoot, xsbBool *isNew) {

  BTNptr btn;

  btn = BTN_Child(btRoot);
  if ( IsNULL(btn) ) {
    BT_InsertEscapeNode(btRoot,btn);
    *isNew = TRUE;
  }
  else if ( IsEscapeNode(btn) )
    *isNew = FALSE;
  else
    TrieError_AbsentEscapeNode(btRoot);
  return btn;
}

/*-------------------------------------------------------------------------*/

inline static TSTNptr tst_escape_search(CTXTdeclc TSTNptr tstRoot, xsbBool *isNew) {

  TSTNptr tstn;

  tstn = TSTN_Child(tstRoot);
  if ( IsNULL(tstn) ) {
    TST_InsertEscapeNode(tstRoot,tstn);
    *isNew = TRUE;
  }
  else if ( IsEscapeNode(tstn) )
    *isNew = FALSE;
  else
    TrieError_AbsentEscapeNode(tstRoot);
  return tstn;
}

/*=========================================================================*/

/*
 *		  Subsumptive Search for a Set of Terms
 *		  =====================================
 *
 * Given a trie (a non-NULL root pointer) and a (possibly empty) set of
 * terms, searches the trie for a subsuming term set.  If none is found,
 * inserts the given terms into the trie.  Returns a pointer to the leaf
 * node representing the given set of terms (if they were inserted) or
 * the discovered subsuming term set (otherwise).  A flag indicates to
 * the caller what the returned leaf represents.
 * 
 * This does not appear to be used (11/05)
 */


/*-------------------------------------------------------------------------*/

/* TLS: commenting out until needed.
| BTNptr subsumptive_bt_search(CTXTdeclc BTNptr btRoot, int nTerms, CPtr termVector,
| 			     xsbBool *isNew) {
| 
|   BTNptr btn;
|   TriePathType path_type;
| 
|   
| #ifdef DEBUG_ASSERTIONS
|   if ( IsNULL(btRoot) || (nTerms < 0) )
|     TrieError_InterfaceInvariant("subsumptive_bt_search()");
| #endif
| 
|   if ( nTerms > 0 ) {
|     Trail_ResetTOS;
|     TermStack_ResetTOS;
|     TermStack_PushHighToLowVector(termVector,nTerms);
|     if ( IsEmptyTrie(btRoot) ) {
|       btn = bt_insert(CTXTc btRoot,btRoot,NO_INSERT_SYMBOL);
|       *isNew = TRUE;
|     }
|     else {
|       TermStackLog_ResetTOS;
|       btn = iter_sub_trie_lookup(CTXTc btRoot,&path_type);
|       if ( path_type == NO_PATH ) {
| 	Trail_Unwind_All;
| 	btn = bt_insert(CTXTc btRoot,stl_restore_variant_cont(CTXT),
| 			NO_INSERT_SYMBOL);
| 	*isNew = TRUE;
|       }
|       else
| 	*isNew = FALSE;
|     }
|     Trail_Unwind_All;
|   }
|   else
|     btn = bt_escape_search(CTXTc btRoot,isNew);
|   return btn;
| }
*/

/*-------------------------------------------------------------------------*/

/*
 * An additional flag is supplied to this routine indicating whether
 * TSIs are being maintained for the given Time-Stamped Trie.  This
 * information is needed when the term set is inserted.
 */

TSTNptr subsumptive_tst_search(CTXTdeclc TSTNptr tstRoot, int nTerms, CPtr termVector,
			       xsbBool maintainTSI, xsbBool *isNew) {

  TSTNptr tstn;
  TriePathType path_type;

  
#ifdef DEBUG_ASSERTIONS
  if ( IsNULL(tstRoot) || (nTerms < 0) )
    TrieError_InterfaceInvariant("subsumptive_tst_search()");
#endif

#ifdef DEBUG_VERBOSE
 {
    int i;
    fprintf(stddbg,"------------------------------------------------------------\n");
    fprintf(stddbg,"Entered subsumptive_tst_search() with the following terms:");
    for (i = 0; i < nTerms; i++) {
      fprintf(stddbg,"\t");
      printterm(stddbg, (Cell)(termVector - i),25);
      fprintf(stddbg,"\n");
    }
  }
#endif

  if ( nTerms > 0 ) {
    Trail_ResetTOS;
    TermStack_ResetTOS;
    TermStack_PushHighToLowVector(termVector,nTerms);
    //    printSymbolStack(CTXTc stddbg, "TermStack [sts]", tstTermStack);
    if ( IsEmptyTrie(tstRoot) ) {
      //      printf("empty trie %d\n", AnsVarCtr);
      tstn = tst_insert(CTXTc tstRoot,tstRoot,NO_INSERT_SYMBOL,maintainTSI);
      *isNew = TRUE;
    }
    else {
      TermStackLog_ResetTOS;
      tstn = iter_sub_trie_lookup(CTXTc tstRoot,&path_type);
      if ( path_type == NO_PATH ) {
	Trail_Unwind_All;
	tstn = tst_insert(CTXTc tstRoot, stl_restore_variant_cont(CTXT),
			  NO_INSERT_SYMBOL, maintainTSI);
	*isNew = TRUE;
      }
      else
	*isNew = FALSE;
    }

#ifdef DEBUG_VERBOSE
 {
    int i;
    fprintf(stddbg,"after insert:");
    for (i = 0; i < nTerms; i++) {
      fprintf(stddbg,"\t");
      printterm(stddbg, (Cell)(termVector - i),25);
      fprintf(stddbg,"\n");
    }
  }
#endif

 /* TLS: needs comment */
    //    Trail_Unwind_All;
  }
  else
    tstn = tst_escape_search(CTXTc tstRoot,isNew);
  return tstn;
}

/*=========================================================================*/

/*
 *		    Variant Search for a Set of Terms
 *		    =================================
 *
 * Searches the given trie (a non-NULL root pointer) for the given
 * (possibly empty) term set.  The terms are inserted if they are not
 * already present.  Returns a pointer to the leaf node representing
 * these terms, and indicates in a flag whether the terms were inserted.
 */


/*-------------------------------------------------------------------------*/

/* TLS: commenting out until used.
| BTNptr variant_bt_search(CTXTdeclc BTNptr btRoot, int nTerms, CPtr termVector,
| 			 xsbBool *isNew) {
| 
|   BTNptr btn;
|   xsbBool wasFound;
|   Cell symbol;
| 
| 
| #ifdef DEBUG_ASSERTIONS
|   if ( IsNULL(btRoot) || (nTerms < 0) )
|     TrieError_InterfaceInvariant("variant_bt_search()");
| #endif
| 
|  if ( nTerms > 0 ) {
|    Trail_ResetTOS;
|    TermStack_ResetTOS;
|    TermStack_PushLowToHighVector(termVector,nTerms);
|    if ( IsEmptyTrie(btRoot) ) {
|      btn = bt_insert(CTXTc btRoot,btRoot,NO_INSERT_SYMBOL);
|      *isNew = TRUE;
|    }
|    else {
|      btn = var_trie_lookup(CTXTc btRoot,&wasFound,&symbol);
|      if ( ! wasFound )
|	btn = bt_insert(CTXTc btRoot,btn,symbol);
|      *isNew = ( ! wasFound );
|    }
|    Trail_Unwind_All;
|  }
|  else
|    btn = bt_escape_search(CTXTc btRoot,isNew);
|  return btn;
|}
*/


/*-------------------------------------------------------------------------*/

/*
 * An additional flag is supplied to this routine indicating whether
 * TSIs are being maintained for the given Time-Stamped Trie.  This
 * information is needed when the term set is inserted.
 */

/*
| TSTNptr variant_tst_search(CTXTdeclc TSTNptr tstRoot, int nTerms, CPtr termVector,
| 			   xsbBool maintainTSI, xsbBool *isNew) {
| 
|   TSTNptr tstn;
|   xsbBool wasFound;
|   Cell symbol;
| 
| 
| #ifdef DEBUG_ASSERTIONS
|   if ( IsNULL(tstRoot) || (nTerms < 0) )
|     TrieError_InterfaceInvariant("variant_tst_search()");
| #endif
| 
|   if ( nTerms > 0 ) {
|     Trail_ResetTOS;
|     TermStack_ResetTOS;
|     TermStack_PushHighToLowVector(termVector,nTerms);
|     if ( IsEmptyTrie(tstRoot) ) {
|       tstn = tst_insert(CTXTc tstRoot,tstRoot,NO_INSERT_SYMBOL,maintainTSI);
|       *isNew = TRUE;
|     }
|     else {
|       tstn = var_trie_lookup(CTXTc tstRoot,&wasFound,&symbol);
|       if ( ! wasFound )
| 	tstn = tst_insert(CTXTc tstRoot,tstn,symbol,maintainTSI);
|       *isNew = ( ! wasFound );
|     }
|     Trail_Unwind_All;
|   }
|   else
|     tstn = tst_escape_search(CTXTc tstRoot,isNew);
|   return tstn;
| }
*/

/*=========================================================================*/
