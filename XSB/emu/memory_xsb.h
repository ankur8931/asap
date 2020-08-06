/* File:      memory_xsb.h
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
** $Id: memory_xsb.h,v 1.54 2012-12-13 02:52:57 dwarren Exp $
** 
*/


/*===========================================================================*/

/*
 *			SLG-WAM Stack Management
 *			========================
 *
 * Information on each independent data area needed by the slg-wam is kept
 * in one of these structures.  "low" and "high" point into memory to
 * delineate the bounds of the data area: "low" gets the address returned
 * by an allocation routine, while "high" gets "low" + "size" * K.  Note:
 * this means that (1) "size" represents the number of K-byte blocks
 * allocated for this stack, and (2) "high" points off the end of the
 * allocated area.
 *
 * The logical, data structure specific details of how a region is used,
 * e.g., which pointer represents the bottom of a stack, is represented in
 * the code and the documentation.  "init_size" is used for storing the size
 * the stack was initialized to, for purposes of restoring it to this size
 * after each query via the trimcore/0 predicate.
 */


#ifndef _MEMORY_XSB_H_
#define _MEMORY_XSB_H_

#include "memory_defs.h"
#include "export.h"

#define encode_memory_error(category,limit_type) (category << 2 | limit_type)

/* Info Structure for the Data Regions
   ----------------------------------- */
typedef struct stack_info {
   byte *low;
   byte *high;
   UInteger size;
   UInteger init_size;
} System_Stack;


/* The SLG-WAM System Data Regions
   ------------------------------- */
extern System_Stack pdl,            /* PDL                        */
                    glstack,        /* Global + Local Stacks      */
                    tcpstack,       /* Trail + Choice Point Stack */
                    complstack;     /* Completion Stack           */


/*
 *  Finding the tops of stacks.
 *  ---------------------------
 *    In this form, the result can be used immediately for deref, recast,
 *    etc., as well as for assignment.  ALL macros return a pointer to the
 *    topmost USED cell on their respective stack.
 */
#define top_of_heap      (hreg - 1)
#define top_of_localstk  ( ((efreg < ebreg) && (efreg < ereg)) \
                           ? efreg  \
			   : ( (ereg < ebreg) \
			       ? ereg - *(cpreg - (2*sizeof(Cell)-3)) + 1 \
			       : ebreg ) )
#define top_of_trail     ((trreg > trfreg) ? trreg : trfreg)
#define top_of_cpstack   ((breg < bfreg) ? breg : bfreg)

#define top_of_complstk  openreg

#define in_localstack(Addr) ((CPtr)Addr >= top_of_localstk && (CPtr)Addr <= (CPtr)glstack.high)
#define in_heap(Addr) ((CPtr)Addr <= top_of_heap && (CPtr)Addr >=  ((CPtr)glstack.low + 1))

/* Testing pointer addresses
   ------------------------- */
#define IsInHeap(Ptr)	( ( (CPtr)(Ptr) <= top_of_heap ) &&	\
    			  ( (CPtr)(Ptr) >= (CPtr)glstack.low ) )

#define IsInEnv(Ptr)	( ( (CPtr)(Ptr) < (CPtr)glstack.high ) &&	\
			  ( (CPtr)(Ptr) >= top_of_localstk) )

#define IsInTrail(Ptr)	( ( (CPtr)(Ptr) <= (CPtr)top_of_trail ) &&	\
    			  ( (CPtr)(Ptr) >= (CPtr)tcpstack.low ) )

#define IsInCPS(Ptr)	( ( (CPtr)(Ptr) < (CPtr)tcpstack.high ) &&	\
			  ( (CPtr)(Ptr) >= top_of_cpstack) )


#define COMPLSTACKBOTTOM ((CPtr) complstack.high)


/*
 *  Size of margin between facing stacks before reallocating a larger area.
 */
//#define OVERFLOW_MARGIN	(2048 * ZOOM_FACTOR)
//#define OVERFLOW_MARGIN	(8192 * ZOOM_FACTOR)
#define OVERFLOW_MARGIN	((Integer)flags[HEAP_GC_MARGIN])


/* Calculate New Stack Size
   ------------------------ */
#define resize_stack(stack_size,min_exp) /*"stack_size" is in K-byte blocks*/\
  ((stack_size) < (unsigned int)(min_exp)/K ? (stack_size) + (min_exp)/K : 2 * (stack_size))


/* Program and Symbol Tables Space (in Bytes)
   ------------------------------------------ */
extern UInteger pspacesize[NUM_CATS_SPACE];
extern UInteger pspace_tot_gl;

/* Memory Function Prototypes
   -------------------------- */
DllExport extern void* call_conv mem_alloc(size_t, int);
extern void *mem_alloc_nocheck(size_t, int);
extern void *mem_calloc(size_t, size_t, int);
extern void *mem_calloc_nocheck(size_t, size_t, int);
DllExport extern void* call_conv mem_realloc(void *, size_t, size_t, int);
extern void *mem_realloc_nocheck(void *, size_t, size_t, int);
extern void mem_dealloc(void *, size_t, int);
extern void print_mem_allocs(char *);
#ifndef MULTI_THREAD
extern void tcpstack_realloc(size_t);
extern void complstack_realloc(size_t);
extern void handle_tcpstack_overflow(void);
#else
struct th_context ;
extern void tcpstack_realloc(struct th_context *, size_t);
extern void complstack_realloc(struct th_context *, size_t);
extern void handle_tcpstack_overflow(struct th_context *);
#endif


/* Instruction Externs
   ------------------- */
extern byte *inst_begin_gl;       /* ptr to beginning of instruction array. */

#ifndef MULTI_THREAD
extern byte *current_inst;
#endif

extern Cell answer_return_inst, hash_handle_inst,
	    resume_compl_suspension_inst, fail_inst, dynfail_inst, 
  halt_inst, proceed_inst, trie_fail_inst,
  resume_compl_suspension_inst2,completed_trie_member_inst,
            reset_inst, trie_fail_unlock_inst;
extern CPtr check_complete_inst;
extern byte *check_interrupts_restore_insts_addr;


/* Stack Overflow Checkers
   ----------------------- */
#define local_global_exception "! Local/Global Stack Overflow Exception\n"

#define complstack_exception "! Completion Stack Overflow Exception\n"

#define trail_cp_exception "! Trail/CP Stack Overflow Exception\n"

#define check_tcpstack_overflow {					\
									\
   CPtr cps_top = top_of_cpstack;					\
									\
   if ((pb)cps_top < (pb)top_of_trail + OVERFLOW_MARGIN) {		\
     if ((pb)cps_top < (pb)top_of_trail) {				\
       xsb_basic_abort("\nFatal ERROR:  -- Trail "			\
				  "clobbered Choice Point Stack --\n");	\
     }									\
     else {								\
       if (pflags[STACK_REALLOC])					\
         tcpstack_realloc(CTXTc resize_stack(tcpstack.size,0));		\
       else {								\
         xsb_basic_abort(trail_cp_exception);				\
       }								\
     }									\
   }									\
 }

#define heap_local_overflow(Margin)					\
  ((unsigned)((top_of_localstk)-hreg)<(unsigned)(Margin))
  //  ((ereg<ebreg)?((ereg-hreg)<(Margin)):((ebreg-hreg)<(Margin)))

#define glstack_overflow(EXTRA)						\
  ((pb)top_of_localstk < (pb)top_of_heap + (OVERFLOW_MARGIN + EXTRA))	\

/* Bytes, not calls */
#define check_glstack_overflow(arity,PCREG,EXTRA)			      \
  if ((pb)top_of_localstk < (pb)top_of_heap + (OVERFLOW_MARGIN + EXTRA))      \
     glstack_ensure_space(CTXTc EXTRA,arity)


#define check_completion_stack_overflow				\
   if ( (pb)openreg < (pb)complstack.low + OVERFLOW_MARGIN ) {	\
     if (pflags[STACK_REALLOC])					\
       complstack_realloc(CTXTc resize_stack(complstack.size,0));\
     else {							\
       xsb_basic_abort(complstack_exception);		        \
     }								\
   }

#define STACK_USER_MEMORY_LIMIT_OVERFLOW(oldSize, newSize)			\
  (flags[MAX_MEMORY] && (pspace_tot_gl/1024 - oldSize + newSize > flags[MAX_MEMORY]))

#endif /* _MEMORY_XSB_H_ */
