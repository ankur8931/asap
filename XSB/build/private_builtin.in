/* File:      private_builtin.c
** Author(s): Work of thousands
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: private_builtin.in,v 1.12 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


/* This file is never committed into the repository, so you can put
   experimental code here.
   It is also a place for private builtins that you might be using for
   debugging. 
   Note: even though this is a single builtin, YOU CAN SIMULATE ANY
   NUMBER OF BUILTINS WITH IT.  */

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN_NT
#include <unistd.h> 
#endif
#include <sys/stat.h>

#include "setjmp_xsb.h"
#include "auxlry.h"
#include "context.h" // required for bld_boxedfloat prototype in cell_xsb.h
#include "cell_xsb.h"
#include "error_xsb.h"
#include "cinterf.h"
#include "memory.h"
#include "psc_xsb.h"
#include "heap_xsb.h"
#include "register.h"
#include "flags_xsb.h"

/* The folowing function must be defined. What's inside doesn't matter */
xsbBool private_builtin(void)
{
  /*
  ctop_string(1, "abc");
  */
  return TRUE;
}

/* Here is an example of how to hang multiple pseudo-builtins
   off of private_builtin/11 */

/*
#define PRINT_LS     	    1
#define PRINT_TR     	    2
#define PRINT_HEAP   	    3
#define PRINT_CP     	    4
#define PRINT_REGS          5
#define PRINT_ALL_STACKS    6
#define EXP_HEAP    	    7
#define MARK_HEAP    	    8
#define GC_HEAP    	    9
*/

/* Arg 1 is the OP code. The rest are real arguments to the builtin */
/*
xsbBool private_builtin(void)
{
  switch (ptoc_int(1)) {
  case PRINT_LS: print_ls(1) ; return TRUE ;
  case PRINT_TR: print_tr(1) ; return TRUE ;
  case PRINT_HEAP: print_heap(0,2000,1) ; return TRUE ;
  case PRINT_CP: print_cp(1) ; return TRUE ;
  case PRINT_REGS: print_regs(10,1) ; return TRUE ;
  case PRINT_ALL_STACKS: print_all_stacks() ; return TRUE ;
  case EXP_HEAP: glstack_realloc(glstack.size + 1,0) ; return TRUE ;
  case MARK_HEAP: mark_heap(ptoc_int(2),&tmpval) ; return TRUE ;
  case GC_HEAP: return(gc_heap(0)) ;
  }
}
*/

/* To use these bultins, you must have a .P file with the following
   information: */

/*
print_ls :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
print_tr :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
print_heap :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
print_cp :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
print_regs :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
print_all_stacks :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
exp_heap :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
mark_heap(A) :- private_builtin(1,A,_,_,_,_,_,_,_,_,_).
gc_heap :- private_builtin(1,_,_,_,_,_,_,_,_,_,_).
*/
