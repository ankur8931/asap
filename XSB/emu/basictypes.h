/* File:      basictypes.h
** Author(s): kifer
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
** $Id: basictypes.h,v 1.33 2011-11-28 01:17:39 tswift Exp $
** 
*/



#ifndef BASIC_TYPES_INCLUDED
#include "xsb_config.h"
#include "cell_def_xsb.h"

/*******************
* Definitions for the basic Integer and Floating point types. 
*   Each type varies depending on whether the BITS64 and FAST_FLOATS flags are set.
*   These types should be used in place of 'float' and 'int'
*******************/
#ifdef BITS64
#ifdef WIN_NT
typedef long long prolog_int ;
typedef long long Integer ;
typedef unsigned long long UInteger ;
#define XSB_MAXINT ((long long)0x7fffffffffffffff)
#else
typedef long prolog_int ;
typedef long Integer ;
typedef unsigned long UInteger ;
#define XSB_MAXINT ((long)0x7fffffffffffffff)
#endif
#else
typedef int prolog_int ;
typedef int Integer ;
typedef unsigned int UInteger ;
#define XSB_MAXINT ((int)0x7fffffff)	/* Modified by Kostis */
#endif
//#define XSB_MININT (-XSB_MAXINT - 1)
// tokenizer cant read the smallest int.
#define XSB_MININT (-XSB_MAXINT)
typedef double prolog_float;

#ifndef FAST_FLOATS
typedef double Float;
#else
typedef float Float;
#endif
/*******************************/

#define MY_MAXINT  XSB_MAXINT
#define MY_MININT  XSB_MININT

typedef int reg_num;

/* CELL and PROLOG_TERM are defined identically.
   However, CELL is used to refer to elements of (slg-)WAM stacks, while
   PROLOG_TERM is used in the C interface to point to a cell containing 
   the outer functor of a prolog term. */
#if defined(BITS64) && defined(WIN_NT)
typedef unsigned long long prolog_term;
#else
typedef unsigned long prolog_term;
#endif

typedef short  xsbBool;

#ifndef __RPCNDR_H__
typedef unsigned char byte;
#endif
typedef UInteger counter;
typedef UInteger word;
typedef byte *pb;
typedef word *pw;
typedef int (*PFI)(void);
// typedef int *int_ptr;


/*******************
* Definitions for converter types. 
*   These types are used as bit manipulation tools, exploiting the shared memory of a union.

* constraints: each member of the union should have equal size, in bytes.
*******************/
typedef union float_conv {
  Cell i;
  float f;
} FloatConv;

typedef union float_to_ints_conv {
   Float float_val;
   struct {
    UInteger high;
    UInteger low;
   }int_vals;
    
} FloatToIntsConv;

typedef union cell_to_bytes_conv {
   Cell cell_val;
   struct {
    byte b1;
    byte b2;
    byte b3;
    byte b4;
   }byte_vals;
    
} CellToBytesConv;


typedef struct CycleTrailFrame {
  CPtr cell_addr;
  Cell value;
  byte arg_num;
  byte arity;
  Cell parent;
} Cycle_Trail_Frame ;

typedef Cycle_Trail_Frame *CTptr;

#define pop_cycle_trail(Term) {						\
    * (CPtr) cycle_trail[cycle_trail_top].cell_addr = cycle_trail[cycle_trail_top].value; \
    Term = cycle_trail[cycle_trail_top].parent;				\
    cycle_trail_top--;							\
  }

#define push_cycle_trail(Term) {					\
    if (  ++cycle_trail_top == cycle_trail_size) {			\
      CTptr old_cycle_trail = cycle_trail;				\
      /*      printf("starting realloc %p %d\n",cycle_trail,cycle_trail_size); */ \
      cycle_trail = mem_realloc(old_cycle_trail,			\
			       cycle_trail_size*sizeof(Cycle_Trail_Frame), \
			       2*cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE); \
      cycle_trail_size = 2*cycle_trail_size;				\
      /*      printf("done w. realloc\n");			*/	\
    }									\
    if (cell_tag(Term) == XSB_STRUCT) {					\
      cycle_trail[cycle_trail_top].arity =  (byte) get_arity(get_str_psc(Term)); \
      cycle_trail[cycle_trail_top].arg_num =  1;				\
      cycle_trail[cycle_trail_top].parent =  Term;			\
    } else {						/* list */	\
      cycle_trail[cycle_trail_top].arity =  1;				\
      cycle_trail[cycle_trail_top].arg_num =  0;			\
      cycle_trail[cycle_trail_top].parent =  Term;			\
    }									\
    cycle_trail[cycle_trail_top].cell_addr = clref_val(Term);		\
    cycle_trail[cycle_trail_top].value =  *clref_val(Term);		\
  }

#define unwind_cycle_trail {			\
    while (cycle_trail_top >= 0) {		\
      pop_cycle_trail(Term);			\
    }						\
}

#define TERM_TRAVERSAL_STACK_INIT 100000

#ifdef MULTI_THREAD
#ifdef WIN_NT
#define Thread_T Integer
#else
#define Thread_T pthread_t
#endif
#endif

#endif /* BASIC_TYPES_INCLUDED */

#define BASIC_TYPES_INCLUDED
