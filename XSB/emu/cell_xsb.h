/* File:      cell_xsb.h
** Author(s): David S. Warren, Jiyang Xu, Terrance Swift
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-19989
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
** $Id: cell_xsb.h,v 1.49 2013-05-06 21:10:24 dwarren Exp $
** 
*/

#ifndef __CELL_XSB_H__
#define __CELL_XSB_H__

#ifndef CONFIG_INCLUDED
#error "File xsb_config.h must be included before this file"
#endif

/*======================================================================*/
/* CELL: an element in the local stack or global stack (heap).		*/
/*									*/
/* Corresponds to the basic word type of the physical machine.		*/
/* This module also defines the basic INTEGER and REAL types to use	*/
/* by the Prolog interpreter						*/
/*									*/
/* Interface routines							*/
/*	They are put in the different files indicated below, according  */
/*	to the tagging scheme you used.					*/
/* The functions (macros, indeed) include:				*/
/*	cell_tag(cell):		give any cell, return its tag		*/
/*	isnonvar(dcell):	check if the cell is a ref or a var.	*/
/*				(an index in VARASINDEX)		*/
/*				when used with derefed cell, check if	*/
/*				the cell is instanciated 		*/
/*	int_val(dcell):		assume derefed integer cell, return its */
/*				value in machine format			*/
/*	clref_val(dcell):	assume derefed cs or list cell, return  */
/*				a CPtr to the address it points to.	*/
/*	cs_val(dcell):		assume derefed cs cell, return Pair	*/
/*	bld_int(addr, val):	build a new integer cell		*/
/*	bld_float(addr, val):	build a new float cell			*/
/*	bld_ref(addr, val):	build a new reference cell		*/
/*	bld_cs(addr, str):	build a new cs cell			*/
/*	bld_string(addr, str):	build a new string cell			*/
/*	bld_list(addr, list):	build a new list cell			*/
/*	bld_functor(addr, psc):	build a functor cell for a new structure*/
/*	bld_free(addr):		build a new free variable 		*/
/*	bld_copy(dest, source): build a copy of the given cell.		*/
/*			if the source is a free var, the copy is indeed */
/*			a ref cell. Need special checking when free var */
/*			is not a pointer to itself but a special tag.	*/
/*	bld_copy0(dest, src):   the same as bld_copy except assume      */
/*                    non-var, or where semantics is to resume/set.	*/
/*                    (in set CP and resume CP)				*/
/*                    For variable as selfpointer, no differnce.    	*/
/*      bld_boxedint		builds an integer with no precision     */
/*			loss on the heap (hreg)                         */
/*      bld_boxedfloat          builds a double float with no precision */
/*                      loss on the heap (hreg)                         */
/*======================================================================*/
#include "xsb_config.h"
#include "cell_def_xsb.h"
#include "basictypes.h"
#include "box_defines.h"

/* ==== types of cells =================================================*/

#include "celltags_xsb.h"

/*======================================================================*/
/* CELL: an element in the local stack or global stack (heap).		*/
/*======================================================================*/

#define cell(cptr) *(cptr)
#define follow(cell) (*(CPtr)(cell))

/*======================================================================*/
/* floating point conversions                                           */
/*    The below 3 methods are to be used when floats and Cells are the  */
/*    same size, in bytes, to convert between the two.                  */
/*======================================================================*/

/* lose some precision in conversions from 32 bit formats */
#ifdef BITS64
#define FLOAT_MASK 0xfffffffffffffff8
#else
#define FLOAT_MASK 0xfffffff8
#endif
//extern inline float getfloatval(Cell);
//extern inline Cell makefloat(float);
extern int sign(Float);

#define isref(cell)  (!((word)(cell)&0x3))
#define isnonvar(cell) ((word)(cell)&0x3)		/* dcell -> xsbBool */

#define cell_tag(cell) ((word)(cell)&0x7)

/*======================================================================*/
/*======================================================================*/

#ifndef GENERAL_TAGGING
#if ((defined(HP700) || defined(IBM) || defined(MIPS_BASED) || defined(SOLARIS_x86) || defined(LINUX)) && !defined(BITS64))
#define GENERAL_TAGGING
#endif
#endif

#if BITS64

/* 64 bit, take bits 0, 61-63 */
/* Encoded integers/addresses */
#define enc_int(val) ((Integer)(val) << 3)
#define dec_int(dcell) ((Integer)(dcell) >> 3)
/* Fewer bit representation of pointers */
#define enc_addr(addr) ((Cell)(addr))
#define dec_addr(dcell) (((Cell)(dcell)) & 0xfffffffffffffff8)

#elif defined(GENERAL_TAGGING)

/* General Tagging for systems that return addresses in higher half
   (2-4GB) of virtual memory.  This builds a table for the high-order
   nibble of addresses and maps them to 0-7, to free up the 1st bit to
   allow it to be stolen. */

extern unsigned long enc[], dec[];

#define enc_int(val)  (((Integer)(val) << 3))
#define dec_int(val)  ((Integer)(val) >> 3)

#define enc_addr(addr) ((Cell)((enc[((unsigned long)(addr))>>28] | ((unsigned long)(addr) & 0x0ffffffc)) << 1))
#define dec_addr(dcell) ((Cell)(dec[(unsigned long)(dcell)>>29] | (((unsigned long)(dcell) >> 1) & 0x0ffffffc)))

#else
/* take bits 0-1, 31 */
/* Steals bit 31 (high order).  If system allocates memory (malloc) in
   the upper 2 GB of virtual memory, then use GENERAL_TAGGING. */

#define enc_int(val)  (((Integer)(val) << 3))
#define dec_int(val)  ((Integer)(val) >> 3)

#define enc_addr(addr) ((Cell)(addr) << 1)
#define dec_addr(dcell) (((Cell)(dcell) >> 1) & 0x7ffffffc)

#endif

/*======================================================================*/

/* integer manipulation */
#define int_val(dcell) (Integer)dec_int(dcell)
#define makeint(val) (Cell)((enc_int(val)) | XSB_INT)

/* string manipulation */
#define string_val(dcell) (char *)dec_addr(dcell)
#define makestring(str) (Cell)(enc_addr(str) | XSB_STRING)

/* special-case strings [] */
#define	makenil		makestring(nil_string)

/* pointer manipulation */
#define cs_val(dcell) (Pair)dec_addr(dcell)
#define makecs(str) (Cell)(enc_addr(str) | XSB_STRUCT)
#define clref_val(dcell) ((CPtr)dec_addr(dcell))
#define makelist(list) (Cell)(enc_addr(list) | XSB_LIST)
#define makeattv(attv) (Cell)(enc_addr(attv) | XSB_ATTV)
#define trievar_val(dcell) (Integer)dec_int(dcell)
#define maketrievar(val) (Cell)(enc_int(val) | XSB_TrieVar)

/**#define get_str_psc(dcell) ((cs_val(dcell))->psc_ptr)**/
#define get_str_psc(dcell) (*((Psc *)dec_addr(dcell)))
#define get_str_arg(dcell, argind) (cell(clref_val(dcell)+(argind)))
#define get_list_head(dcell) (cell(clref_val(dcell)))
#define get_list_tail(dcell) (cell(clref_val(dcell)+1))

#define addr_val(dcell) string_val(dcell)
#define makeaddr(val) makestring(val)

/* #define addr_val(dcell) int_val(dcell) */
/* #define makeaddr(val) makeint(val) */

/* common representations */
#define vptr(dcell) (CPtr)(dcell)
#define float_val(dcell) getfloatval(dcell)
#define ref_val(dcell) (CPtr)(dcell)

#define bld_nil(addr) cell(addr) = makenil
#define bld_string(addr, str) cell(addr) = makestring(str)
#define bld_int_tagged(addr, val) cell(addr) = val
#define bld_int(addr, val) cell(addr) = makeint(val)
#define bld_float_tagged(addr, val) cell(addr) = val
//Note: See section 'Miscellaneous' for implementation of bld_boxedfloat & bld_boxedint.
#define bld_float(addr, val) cell(addr) = makefloat(val)
#define bld_ref(addr, val) cell(addr) = (Cell)(val)
#define bld_cs(addr, str) cell(addr) = makecs(str)
#define bld_list(addr, list) cell(addr) = makelist(list)
#define bld_attv(addr, attv) cell(addr) = makeattv(attv)
#define bld_functor(addr, psc) cell(addr) = (word)psc
#define bld_copy0(addr, val) cell(addr) = val
#define bld_copy(addr, val) cell(addr) = val
/* tls -- this bld_copy wont work for VARASINDEX */
#define bld_free(addr) cell(addr) = (Cell)(addr) /* CPtr => XSB_FREE cell */

#define isref(cell)  (!((word)(cell)&0x3))    // NOTE not sat. by attvar.
#define isnonvar(cell) ((word)(cell)&0x3)		/* dcell -> xsbBool */
#define isinteger(dcell) (cell_tag(dcell)==XSB_INT)	/* dcell -> xsbBool */
//Note: See section 'Miscellaneous' for implementation of isboxedfloat.
#define isfloat(dcell) (cell_tag(dcell)==XSB_FLOAT)	/* dcell -> xsbBool */
#define isconstr(dcell) (cell_tag(dcell)==XSB_STRUCT)	/* dcell -> xsbBool */
#define islist(dcell) (cell_tag(dcell)==XSB_LIST)	/* dcell -> xsbBool */
#define isstr(dcell) (isconstr(dcell) || islist(dcell))
/**#define isinternstr0(dcell)						\
   (isstr(dcell) && (clref_val(dcell)<(CPtr)glstack.low || clref_val(dcell)>(CPtr)glstack.high)) **/
/* perhaps this added test of hash bucket link as addr causes faster failure, and most calls fail */
#define isinternstr0(dcell)						\
  (isstr(dcell) && !(*(clref_val(dcell)-1)&3) && (clref_val(dcell)<(CPtr)glstack.low || clref_val(dcell)>(CPtr)glstack.high))
/* multi-threaded engine has pointers from one heap to another at times; so more care needed */
#ifndef MULTI_THREAD
//#define isinternstr(dcell) isinternstr0(dcell)
#define isinternstr(dcell) (isinternstr0(dcell) && isinternstr_really((prolog_term)(dcell)))
#else
#define isinternstr(dcell) (isinternstr0(dcell) && isinternstr_really((prolog_term)(dcell)))
#endif

#define isattv(dcell) (cell_tag(dcell)==XSB_ATTV)	/* dcell -> xsbBool */

#define is_attv_or_ref(cell) (isref(cell) || isattv(cell))

#define iscallable(op2) ((isconstr(op2) && !isboxed(op2)) || isstring(op2) || islist(op2))

/* TLS: this can be made mre efficient, but I want to get the problem with the cut_psc fixed */
#define is_directly_callable(op2) ((isconstr(op2) && !isboxed(op2)) 	\
				   && get_str_psc(op2) != comma_psc	\
				   && get_str_psc(op2) != colon_psc && get_str_psc(op2) != cut_psc \
				    && get_str_psc(op2) != cond_psc )

//  Saving, in the unlikely possibility that there is a problem with the new call
//				   && op2 != (Cell) pflags[MYSIG_UNDEF+INT_HANDLERS_FLAGS_START])

#define isstring(dcell) (cell_tag(dcell)==XSB_STRING)
#define numequal(num1, num2) num1 == num2

#define xsb_isnumber(dcell)	((isinteger(dcell)) || (isfloat(dcell)))
#define isconstant(dcell)  ( isstring(dcell) || xsb_isnumber(dcell) )
#define isatom(dcell)	(isstring(dcell))
#define isatomic(dcell)	((isstring(dcell)) || (xsb_isnumber(dcell)))

#define isnil(dcell) (isstring(dcell) && (char *)string_val(dcell) == nil_string)
#define isboxed(term) (isconstr(term) && get_str_psc(term) == box_psc )
#define box_has_id(dcell, box_identifier) (int_val(get_str_arg(dcell,1))>>16 == box_identifier)

#define isboxedTrieSym(term) ((Psc)cs_val(term) == box_psc)
/*======================================================================*/
/* Miscellaneous							*/
/*======================================================================*/

#define MAX_ARITY	255
#define CELL_DEFS_INCLUDED

#define arity_integer(dcell) \
   (isinteger(dcell) && int_val(dcell) >= 0 \
	  && int_val(dcell) <= MAX_ARITY)
            
#define isboxedinteger(dcell) (isboxed(dcell) && box_has_id(dcell, ID_BOXED_INT))
#define isointeger(dcell) (isinteger(dcell) || isboxedinteger(dcell))

#ifndef FAST_FLOATS
#define isboxedfloat(dcell) (isboxed(dcell) && box_has_id(dcell, ID_BOXED_FLOAT))

#else
//If FAST_FLOATS is defined, there should be no boxed floats (only Cell floats). 
//  isboxedfloat is therefore expanded to FALSE (0), in hopes that the optimizer will remove.
//  any code exclusively dependent on it.

#define isboxedfloat(dcell) 0
#endif

#define boxedint_val(dcell) \
       ((Integer)((((UInteger)int_val(get_str_arg(dcell,2))<<24)   \
		   | int_val(get_str_arg(dcell,3))))) 
       
#ifndef FAST_FLOATS
//make_float_from_ints, defined in emuloop.c, is used by EXTRACT_FLOAT_FROM_16_24_24 to combine
//    two UInteger values into a Float value that is twice the size the UInteger, in bytes. 

// It assumes that the double type is twice the size of the integer type on the target machine.
//    It is an inline function that joins the two ints in a struct, unions this struct with a 
//    Float variable, and returns this Float.

#ifdef BITS64
//extern inline Float make_float_from_ints(UInteger);

// EXTRACT_FLOAT_FROM_16_24_24 works by first merging the three ints together into 
//    two Integers (assuming high-order 16 bits are in the first, middle-24 bits in the 
//    second, and low-24 bits in the third).

#define EXTRACT_FLOAT_FROM_16_24_24(highInt, middleInt, lowInt)	\
  (make_float_from_ints(  (((((UInteger) highInt) & LOW_16_BITS_MASK) <<48) \
			   | (((UInteger) middleInt & LOW_24_BITS_MASK) << 24)) \
			      | ((((UInteger) lowInt & LOW_24_BITS_MASK)))))

#else

//extern inline Float make_float_from_ints(UInteger, UInteger);

#define EXTRACT_FLOAT_FROM_16_24_24(highInt, middleInt, lowInt)	\
      (make_float_from_ints(  (((((UInteger) highInt) & LOW_16_BITS_MASK) <<16) | (((UInteger) middleInt) >> 8)), \
                              ((((UInteger) middleInt)<<24) | (UInteger)lowInt)) )

#endif   /* BITS64 */

#define boxedfloat_val(dcell)                           \
    (                                                   \
     (Float)(    EXTRACT_FLOAT_FROM_16_24_24(           \
		   (int_val(get_str_arg(dcell,1))),     \
                   (int_val(get_str_arg(dcell,2))),	\
                   (int_val(get_str_arg(dcell,3)))	\
                 )                                      \
            )                                           \
    )
#else   /* FAST_FLOATS */
//if FAST_FLOATS is defined, then boxedfloat_val should never get called. To satisfy the 
//  compiler, boxedfloat_val has been turned into an alias for float_val, since everything that 
//  would have been a boxedfloat should now be a float.
#define boxedfloat_val(dcell) float_val(dcell)
#endif

#define oint_val(dcell)      \
    (isinteger(dcell)        \
     ?int_val(dcell)         \
     :(isboxedinteger(dcell) \
       ?boxedint_val(dcell)  \
       :(Integer)0x80000000))


#ifndef FAST_FLOATS
/* below returns the error-code 12345.6789 if the macro is called on a non-float related cell. 
   One should be careful with this (possibly change?), since it ordinarily can return any double
   in the full floating point domain, including 12345.6789.*/
#define ofloat_val(dcell)    \
    (isfloat(dcell)          \
     ?float_val(dcell)         \
     :(isboxedfloat(dcell) \
       ?boxedfloat_val(dcell)  \
       :12345.6789))
#else
//when FAST_FLOATS is defined, ofloat_val exists for the same reason that boxedfloat_val 
//  does in the FAST_FLOATS defined case. See boxedfloat_val for comments.
#define ofloat_val(dcell) float_val(dcell)
#endif


#ifndef FAST_FLOATS
#define isofloat(val)  ( (isboxedfloat(val)) || (isfloat(val)) )
#else
#define isofloat(val) isfloat(val)
#endif

#define int_overflow(value)                 \
   (int_val(makeint(value)) != (value))

#define bld_boxedint(addr, value)				                    \
     {Cell binttemp = makecs(hreg);	                                \
      new_heap_functor(hreg,box_psc);				                \
      bld_int(hreg,((ID_BOXED_INT << BOX_ID_OFFSET ) )); hreg++;	\
      bld_int(hreg,INT_LOW_24_BITS(value)); hreg++;	                \
      bld_int(hreg,((value) & LOW_24_BITS_MASK)); hreg++;		    \
      cell(addr) = binttemp;}

      
#define bld_oint(addr, value)					    \
    if (int_overflow(((Integer)value))) {			\
      bld_boxedint(addr, ((Integer)value));			\
    } else {bld_int(addr,((Integer)value));}

    
//if FAST_FLOATS is defined, then bld_boxedfloat becomes an alias for bld_float. If it is
//defined, bld_boxedfloat is a function declaired here and implemented in emuloop.c.
#ifndef FAST_FLOATS
//Note: anything that includes cell_xsb.h in the multithreaded must includes
//context.h as well, since bld_boxedfloat references hreg.
//extern inline void bld_boxedfloat(CTXTdeclc CPtr, Float);
#endif /*FAST_FLOATS*/
   
#endif /* __CELL_XSB_H__ */
