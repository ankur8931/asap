/* File:      tst_utils.h
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
** $Id: tst_utils.h,v 1.14 2012-06-07 19:37:42 tswift Exp $
** 
*/

#include "dynamic_stack.h"

/* Debugging Routines
   ------------------ */
extern char *stringNodeType(byte);
extern char *stringTrieType(byte);
extern void printTrieSymbol(FILE *, Cell);
extern void printTrieNode(FILE *, BTNptr);
extern void printTriePath(FILE *, BTNptr, xsbBool);
extern int sprintTriePath(CTXTdeclc char *, BTNptr);

extern void printAnswerTemplate(FILE *, CPtr, int);

extern void sfPrintGoal(FILE *, VariantSF, xsbBool);
extern void sfPrintConsGoals(FILE *, VariantSF);
extern void printAnswerList(FILE *, ALNptr);

extern void printTabledCall(FILE *, TabledCallInfo);
extern void printTriePathType(FILE *, TriePathType, BTNptr);
extern void printSymbolStack(FILE *, char* , DynamicStack);
