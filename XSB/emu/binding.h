/* File:      binding.h
** Author(s): Jiyang Xu, Terrance Swift, Kostis Sagonas, Baoqiu Cui
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
** $Id: binding.h,v 1.26 2011-11-11 18:21:39 dwarren Exp $
** 
*/

/*
 * Structures Of Different Trail Frames:
 *
 * In SLG-WAM, trreg and trfreg always point to the trail_parent field of
 * a trail frame, and the trail stack may contain both 3-word trail frames 
 * and 4-word ones.  After untrailing a trail frame, trreg can be reset by 
 * following the trail_parent pointer.
 *
 * 1) Regular (forward) trail frame in SLG-WAM (3-word trail frame)
 * ----------------------------------------------------------------
 *    low memory
 *     +-----+
 *     |     | trail_variable : Address of the trailed variable
 *     +-----+
 *     |     | trail_value    : New value to which the trailed variable is
 *     +-----+                  to be bound
 *     |     | trail_parent   : Pointer to the trail_parent cell of the
 *     +-----+                  previous trail frame)
 *    high memory
 *
 * 2) Pre-image trail frame in SLG-WAM (4-word trail frame)
 * --------------------------------------------------------
 *    low memory
 *     +-----+
 *     |     | trail_pre_image: Old value of the trailed variable (cell)
 *     +---+-+
 *     |   |1| trail_variable : Address of the trailed variable
 *     +---+-+                  (with PRE_IMAGE_MARK)
 *     |     | trail_value    : New value to which the trailed variable is
 *     +-----+                  to be bound
 *     |     | trail_parent   : Pointer to the trail_parent cell of the
 *     +-----+                  previous trail frame)
 *    high memory
 */

#include "xsb_config.h"
#include "basictypes.h"
#include "box_defines.h"
#include "memory_xsb.h"

#define PRE_IMAGE_TRAIL

#define PRE_IMAGE_MARK   1

#define TRAIL_FRAME_SIZE  4

#define pushtrail0(addr,val)  \
   if (trfreg > trreg) {\
     if ((char *)trfreg > \
	 ((char *)(top_of_cpstack) - (TRAIL_FRAME_SIZE*sizeof(CPtr)))) {\
       handle_tcpstack_overflow(CTXT);\
     }\
     *(trfreg+3) = (CPtr) trreg;\
     trreg = trfreg + 3;\
     *(trreg-1) = (CPtr) val;\
     *(trreg-2) = addr;\
   }\
   else {\
     if ((char *)trreg > \
	 ((char *)(top_of_cpstack) - (TRAIL_FRAME_SIZE*sizeof(CPtr)))) {\
       handle_tcpstack_overflow(CTXT);\
     }\
     trreg = trreg+3;\
     *trreg = (CPtr) trreg-3;\
     *(trreg-1) = (CPtr) val;\
     *(trreg-2) = addr;\
   }

#ifdef PRE_IMAGE_TRAIL
#define push_pre_image_trail0(addr, new_value)			\
  if (trfreg > trreg) {						\
    if ((char *)trfreg > ((char *)(top_of_cpstack) -		\
                          TRAIL_FRAME_SIZE*sizeof(CPtr))) {	\
      handle_tcpstack_overflow(CTXT);				\
    }								\
    *(trfreg + 4) = (CPtr) trreg;				\
    trreg = trfreg + 4;						\
    *(trreg - 1) = (CPtr) (new_value);				\
    *(trreg - 2) = (CPtr) ((Cell) (addr) | PRE_IMAGE_MARK);	\
    *(trreg - 3) = (CPtr) (cell(addr));				\
  }								\
  else {							\
    if ((char *)trreg > ((char *)(top_of_cpstack) -		\
                          TRAIL_FRAME_SIZE*sizeof(CPtr))) {	\
      handle_tcpstack_overflow(CTXT);				\
    }								\
    trreg = trreg + 4;						\
    *trreg = (CPtr) trreg - 4;					\
    *(trreg - 1) = (CPtr) (new_value);				\
    *(trreg - 2) = (CPtr) ((Cell) (addr) | PRE_IMAGE_MARK);	\
    *(trreg - 3) = (CPtr) (cell(addr));				\
  }
#endif /* PRE_IMAGE_TRAIL */

#define conditional(a)	( ((a) >= ebreg || (a) >= efreg) || \
			  ((a) < hbreg  || (a) < hfreg) )

#define pushtrail(a,v)	if (conditional(a)) { pushtrail0(a,v); }
#define dpushtrail(a,v) pushtrail0(a,v)

#ifdef PRE_IMAGE_TRAIL
#define push_pre_image_trail(a, new_v)			\
  if (conditional(a)) {push_pre_image_trail0(a, new_v)}
#endif /* PRE_IMAGE_TRAIL */

/* --- binding -------------------------------------------------------- */

#define bind_int_tagged(addr, val)	pushtrail(addr, (Cell) val); \
   				        bld_int_tagged(addr, val)

#define bind_float_tagged(addr, val) pushtrail(addr, (Cell) val);\
                        bld_float_tagged(addr, val)
                        
#define bind_int(addr, val)	pushtrail(addr, makeint(val));\
				bld_int(addr, val)

#define bind_float(addr, val) pushtrail(addr, makefloat(val)); bld_float(addr, val)
//below is old boxedint code. Retained in case new boxedint is faulty
/*#define bind_boxedint(addr, val)				 \
     {Cell temp = makecs(hreg);					 \
      new_heap_functor(hreg,box_psc);				 \
      bld_int(hreg,1); hreg++;					 \
      bld_int(hreg,(((Integer)(val)) >> 24)); hreg++;	 \
      bld_int(hreg,((val) & 0xffffff)); hreg++;			 \
      pushtrail(addr, temp);					 \
      cell(addr) = temp;} */

#define bind_boxedint(addr, value)				                    \
     {Cell binttemp = makecs(hreg);	    \
      new_heap_functor(hreg,box_psc);				                \
      bld_int(hreg,((ID_BOXED_INT << BOX_ID_OFFSET ) )); hreg++;	\
      bld_int(hreg,INT_LOW_24_BITS(value)); hreg++;	                \
      bld_int(hreg,((value) & LOW_24_BITS_MASK)); hreg++;		    \
      pushtrail(addr, binttemp);		                            \
      cell(addr) = binttemp;}


#ifndef FAST_FLOATS
//Below, bind_boxedfloat builds a boxedfloat, from Float 'value', on the heap.
//It also creates a Cell pointing to it, and inserts it at addr. 
//Pre-condition: value must be a 64-bit floating point value
/*#define bind_boxedfloat(addr, value)				            \
     {Float tempFloat = value;                          \
      Cell bfloattemp = makecs(hreg);				        \
      new_heap_functor(hreg,box_psc);				        \
      bld_int(hreg,(ID_BOXED_FLOAT << BOX_ID_OFFSET ) | FLOAT_HIGH_16_BITS(tempFloat) ); \
      hreg++;                                               \
      bld_int(hreg,FLOAT_MIDDLE_24_BITS(tempFloat)); hreg++;	\
      bld_int(hreg,FLOAT_LOW_24_BITS(tempFloat)); hreg++;		\
      pushtrail(addr, bfloattemp);				            \
      cell(addr) = bfloattemp;}
      
      */
      
#define bind_boxedfloat(addr, value)   \
    {   pushtrail(addr, (Cell)makecs(hreg)); \
        bld_boxedfloat(CTXTc addr, value); }                                   
#else
#define bind_boxedfloat(addr, value) bind_float(addr, value)
#endif


#define bind_oint(addr, val)                \
     if (int_overflow(val)) {               \
	bind_boxedint(addr, val);				\
      } else {bind_int(addr, val);}


#define bind_ref(addr, val)	pushtrail(addr, val);\
				bld_ref(addr, val)

#define dbind_ref(addr, val)	dpushtrail(addr, val);\
				bld_ref(addr, val)

#define bind_cs(addr, str)	pushtrail(addr, makecs(str));\
				bld_cs(addr, str)

#define bind_string(addr, str)	pushtrail(addr, makestring(str));\
				bld_string(addr,str)

#define bind_nil(addr)		pushtrail(addr, makenil);\
				bld_nil(addr)

#define bind_list(addr, list)	pushtrail(addr, makelist(list));\
				bld_list(addr, list)

#define bind_attv(addr, attv)	pushtrail(addr, makeattv(attv));\
				bld_attv(addr, attv)

#define bind_copy(addr, val)	pushtrail(addr, val); *(addr) = val

/* value trail MUST be used because first CP cell has cp_trreg = 0 !!!! */

#define untrail(addr) bld_free(addr)

#ifdef PRE_IMAGE_TRAIL	/* untrail2 is for pre_image trail. */

#define untrail2(trail_ptr, addr)		\
  if ((addr) & PRE_IMAGE_MARK) {		\
    bld_copy0((CPtr)((addr) - PRE_IMAGE_MARK),	\
	      cell((CPtr)trail_ptr - 3));	\
  }						\
  else						\
    bld_free((CPtr)(addr))

#endif /* PRE_IMAGE_TRAIL */


/* --- testing location of variable ----------------------------------- */

#define ereg_on_top(t_ereg)	t_ereg < ebreg

#define efreg_on_top(t_ereg) efreg < ebreg  && efreg < t_ereg


/* --- for building vals on the heap ---------------------------------- */

#define nbldval(OP1) {							  \
   XSB_Deref(OP1);							  \
   if ( isnonvar(OP1) ||						  \
	( /* (CPtr)(OP1) >= glstack.low && */				  \
	  (CPtr)(OP1) <= top_of_heap ) ) {			 	  \
     if (flags[UNIFY_WITH_OCCURS_CHECK_FLAG] &&                    	\
         (isconstr(OP1) || islist(OP1)) &&				\
         occurs_in_list_or_struc((Cell)hreg,OP1)) Fail1;		\
     else new_heap_node(hreg, OP1);					\
   }									  \
   else {  /* local stack vars point to heap vars and not vice-versa! */  \
     bind_ref((CPtr)(OP1), hreg);					  \
     new_heap_free(hreg);						  \
   }									  \
}

#define nbldval_safe(OP1) {						  \
   XSB_Deref(OP1);							  \
   if ( isnonvar(OP1) ||						  \
	(CPtr)(OP1) <= top_of_heap ) {					  \
      new_heap_node(hreg, OP1);						  \
   }									  \
   else {  /* local stack vars point to heap vars and not vice-versa! */  \
     bind_ref((CPtr)(OP1), hreg);					  \
     new_heap_free(hreg);						  \
   }									  \
}
