/* File:      heap_xsb.c
** Author(s): Bart Demoen, Kostis Sagonas
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1998
** Copyright (C) K.U. Leuven, 1998-1999
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
** $Id: heap_xsb.c,v 1.90 2013-05-06 21:10:24 dwarren Exp $
** 
*/

/*************************************************************************
 * This module provides:

	reallocation of the heap/environment area
	-----------------------------------------
	Function glstack_realloc(new_size,arity)
		originally written by E. Johnson in file
		memory_xsb.c, but completely redone by Bart Demoen

	heap garbage collection
	-----------------------
	Function gc_heap(arity,IfStringGC) - 
	   To understand the usefulness logic, see paper:
		B. Demoen and K. Sagonas.
		Memory Management for Prolog with Tabling.
		in Proceedings of the 1998 ACM SIGPLAN International
		Symposium on Memory Management, Vancouver, B.C., Canada,
		Oct. 1998. ACM Press. p. 97-106

	   To understand the implementation and for additional information see:
		B. Demoen and K. Sagonas.
		Heap Garbage Collection in XSB: Practice and Experience.
		CW report 272, September 1998; finished February 1999.


	Function slide_heap() implements a sliding collector a la Morris
	   It was mostly written by Bart Demoen
	       for a Prolog specific one see paper:
	       K. Appleby, M. Carlsson, S. Haridi, and D. Sahlin.
	       Garbage Collection for Prolog Based on WAM.
	       Communications of the ACM, 31(6):719--741, June 1988.


	Function copy_heap() implements a copying collector a la Cheney
	   It was mostly written by Kostis Sagonas
               for a Prolog specific one see paper:
	       J. Bevemyr and T. Lindgren.
	       A Simple and Efficient Copying Garbage Collector for Prolog.
	       In M. Hermenegildo and J. Penjam, editors,
	       Proceedings of the Sixth International Symposium on
	       Programming Language Implementation and Logic Programming,
	       number 844 in LNCS, pages 88--101, Madrid, Spain, Sept.  1994.
	       Springer-Verlag.


	printing routines for some areas
		print_heap
		print_ls
		print_cp
		print_regs
		print_tr
		print_all_stacks: does all of the above
	some - maybe all - of these were somewhere in the system already
		but weren't entirely what we needed


Todo:
        adapt the garbage collectors to SLG-WAM
	provide a decent user interface to the garbage collector
	integrate with compiler

****************************************************************************/
/****************************************************************************
String table garbage collection, by mark and collect.  

First mark all strings in use.  A string Cell is a tagged pointer to a
sequence of chars, word aligned.  The previous word is the link
pointer in the hash bucket chain.  Use the lowest bit in that pointer
as the mark bit.  When marking what looks like a string, be sure it is
a indeed a string by comparing it to the result of calling
string_find.

Mark: 

a) Piggyback on marking of heap gc to mark all strings accessible
from stacks and trail.

b) Mark tries by running trie-node blocks rooted at smTableBTN,
smTSTN, and smAssertBTN.  Changed trie-node free to set 2nd word in
trie node to distinctive pattern (-1), so know to skip those nodes.
(NOT DONE: At the same time could free blocks all of whose nodes are free.  
Mark 2nd work in freed nodes in to-be-freed blocks (-2?), and run free
chain to remove them.  Then free the blocks.)

c) Mark all strings in code by running atom-table to get entry points,
including through private dispatch tables, and then scanning the code
for instructions containing strings.

d) Mark all strings in findall buffers by running them.

e) Mark strings that are used as filenames for open files.

f) Mark strings used as hash-keys in hashtables.

g) NOT DONE: Consider ways to deal with string pointers given to C programs in
ptoc_string.

Collect: run through the string table, freeing unmarked strings and
unmarking marked strings.

****************************************************************************/

/* xsb_config.h must be the first #include. Pls don't move it. */
#include "xsb_config.h"
#include "xsb_debug.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Take care of the time.h business */
/* #include "xsb_time.h" */
/* But I need time.h, not sys/time.h here! -lfcastro */
#include <time.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "memory_xsb.h"
#include "inst_xsb.h"
#include "cinterf_defs.h"

/* For Reallocation Routines
   ------------------------- */
#include <stdio.h>         /* for printf and friends */

#include "register.h"       /* breg, trreg */
#include "psc_xsb.h"        /* needed by "tries.h" and "tab_structs.h" */
#include "tries.h"          /* needed by "choice.h" */
#include "choice.h"         /* choice point structures and macros */
#include "error_xsb.h"      /* xsb_exit() and friends */
#include "tab_structs.h"   /* Completion Stack and Subgoal Frame def's */
#include "realloc.h"       /* Heap - ls reallocation macros */
#include "flags_xsb.h"     /* for checking whether functionality is enabled */
#include "heap_xsb.h"
#include "io_builtins_xsb.h"
#include "binding.h"       /* for PRE_IMAGE_TRAIL */
#include "thread_xsb.h"	   /* for mutex definitions */
#include "debug_xsb.h"
#include "loader_xsb.h" /* for ZOOM_FACTOR, used in stack expansion */
#include "struct_manager.h"
#include "hash_xsb.h"
#include "struct_intern.h"
#include "cell_xsb_i.h"
/*=========================================================================*/

/* this might belong somewhere else (or should be accessible to init.c),
   but in the meantime, this will do */
#ifdef GC
static float mark_threshold = 0.9F;
#endif

#ifdef DEBUG_VM
#define GC_PROFILE
#endif

#ifdef GC_PROFILE

static char count_chains=0, examine_data=0, verbose_gc=0;
size_t chains[64];
size_t tag_examined[9];
size_t deep_mark;
size_t current_mark;
size_t old_gens;
size_t current_gen;
CPtr start_hbreg;
size_t functor;
size_t chain_from_ls;
size_t active_cps, frozen_cps;
void print_cpf_pred(CPtr cpf);

#endif /* GC_PROFILE */

extern void extend_enc_dec_as_nec(void *,void *);
extern void free_unused_strings();
extern void mark_nonheap_strings(CTXTdecl);

/*=========================================================================*/

/* to choose between copying or sliding collector:
   its value is determined based on the the value of pflags[GARBAGE_COLLECT] */
static xsbBool slide;

#ifdef GC
/* measuring fragmentation without collection - it also sets slide = 0 */
static const int fragmentation_only = 0;
#endif
		      
/* choose to do early reset or not.  TLS: I doubt that EARLY_RESET
   will work with the pre-image trail needed for attributed
   variables */
/* #define EARLY_RESET 1 */


/* expresses how often early reset of a trailed heap cell occurs */
static int heap_early_reset;

/* expresses how often early reset of a trailed local stack cell occurs */
static int ls_early_reset;


/* ways to count gc and control the output during a gc */
static int printnum = 0 ;

#ifdef DEBUG_VERBOSE
static int print_at = 0 ; /* at the print_at-th gc, the stacks are printed */
static int print_after = 0 ; /* if non zero, print all after this numgc */
static int print_anyway = 0 ;

#define print_on_gc \
        ((print_at == num_gc) \
	 || ((print_after > 0) && (print_after <= num_gc)) \
	 || print_anyway)
#else
#define print_on_gc 0
#endif

/* Whether to garbage collect strings on this heap gc or not. */
int gc_strings = FALSE;

int force_string_gc = FALSE; /* flag set when string space has expanded a lot */
size_t last_string_space_size = 100000;
size_t last_assert_space_size = 10000;
#define AUTO_STRING_GC_NTH 10

/******* When to GC string space? *************/
int should_gc_strings() {
  static int till_forced_string_gc = 1;  /* string collect first time */

  /* every AUTO_STRING_GC_NTH time that heap gc is done, regardless */
  if (!(--till_forced_string_gc)) {
    till_forced_string_gc = AUTO_STRING_GC_NTH;
    //    printf("should_gc_strings: cycle\n");
    return TRUE;
  }
  /* if already requested by someone else, do it. */
  if (gc_strings) {
    till_forced_string_gc = AUTO_STRING_GC_NTH;
    //    printf("should_gc_strings: requested\n");
    return TRUE;
  }
  /* if string_space has doubled, but assert space hasn't, since last string gc */
  if ((pspacesize[STRING_SPACE] > 2*last_string_space_size) &&
      (pspacesize[ASSERT_SPACE] < 2*last_assert_space_size)) {
    till_forced_string_gc = AUTO_STRING_GC_NTH;
    //    printf("should_gc_strings: strings grew\n");
    return TRUE;
  }
  /* if assert space has shrunk alot */
  if (pspacesize[ASSERT_SPACE] < last_assert_space_size/4 ||
      (last_assert_space_size - pspacesize[ASSERT_SPACE]) > 1000000) {
    till_forced_string_gc = AUTO_STRING_GC_NTH;
    //    printf("should_gc_strings: assert shrunk\n");
    return TRUE;
  }
  return FALSE;
}


/* if SAFE_GC is defined, some more checks are made after gargage collection */
/* #define SAFE_GC */
/* #define DEBUG_ASSERTIONS */

/* if VERBOSE_GC is defined, garbage collection prints its statistics */
/* #define VERBOSE_GC */


/*---------------------------------------------------------------------------*/
/* global variables for top and bottom of some areas + macro to compute them */
/*---------------------------------------------------------------------------*/

#ifndef MULTI_THREAD
static CPtr heap_bot,heap_top,
  ls_bot,ls_top,
  tr_bot,tr_top,
  cp_bot,cp_top,
  compl_top,compl_bot;
static size_t heap_marks_size;
#endif


#define stack_boundaries \
  heap_top = hreg; \
  ls_top = top_of_localstk - 256;  /* extra space for environment above top */ \
  if (ls_top < heap_top) {						\
    if ((ls_top + 256) < heap_top)				    \
      xsb_exit( "Heap and local stack are clobbered"); \
    else xsb_exit("Not enough extra space to expand heap/local stacks (lst=%p,ht=%p)",ls_top,heap_top); \
  } \
  heap_bot = (CPtr)glstack.low ; \
  ls_bot = (CPtr)glstack.high - 1 ; \
  tr_top = (CPtr)(top_of_trail) /*- 1*/ ; \
  tr_bot = (CPtr)tcpstack.low ; \
  cp_bot = (CPtr)tcpstack.high - 1 ; \
  cp_top = top_of_cpstack ; \
  compl_top = (CPtr)complstack.low ; /* NOT top_of_complstk !!! */\
  compl_bot = (CPtr)complstack.high ;

#define points_into_heap(p)  ((heap_bot <= p) && (p < heap_top))
#define points_into_ls(p)    ((ls_top <= p) && (p <= ls_bot))
#define points_into_cp(p)    ((cp_top <= p) && (p <= cp_bot))
#define points_into_tr(p)    ((tr_bot <= p) && (p <= tr_top))
#define points_into_compl(p) ((compl_top <= p) && (p <= compl_bot))

/*======================================================================*/
/* global variables used for statistics.                                */
/*======================================================================*/

#ifndef MULTI_THREAD
static double total_time_gc = 0 ;
static UInteger total_collected = 0 ;
static int num_gc = 0 ;
#endif

static int num_sgc = 0 ;

/*----------------------------------------------------------------------*/
/* marker bits in different areas.                                      */
/*----------------------------------------------------------------------*/

#ifndef MULTI_THREAD
static char *heap_marks  = NULL ;
static char *ls_marks    = NULL ;
static char *tr_marks    = NULL ;
static char *cp_marks    = NULL ;
#endif

#define INDIRECTION_SLIDE
#ifndef MULTI_THREAD
#ifdef INDIRECTION_SLIDE
static CPtr *slide_buf= NULL;
static size_t slide_top = 0;
static int slide_buffering = 0;
static size_t slide_buf_size = 0;
#endif
#endif

#define MARKED    1
#define TRAIL_PRE 2
#define CHAIN_BIT 4                            

/* in the absence of serious bugs, the test is an invariant of the WAM */
#ifdef DEBUG_ASSERTIONS
#define testreturnit(retp)   if (points_into_heap(retp)) return(retp)
#else
#define testreturnit(retp)   return(retp)
#endif

/*=========================================================================*/
/* GC-specific includes */
#include "gc_profile.h"
#include "gc_mark.h"
#include "gc_print.h"
#include "gc_slide.h"
#include "gc_copy.h"
/*=========================================================================*/


/*==========================================================================
        new_size = new size of heap + environmentstack
        arity = number of argument registers in use at moment of call

        assumption: the argument registers in use are
                        reg+1 up to reg+arity included

        if you call glstack_realloc with new_size == the current size,
                you will get a reallocated area !

        Re-allocate the space for the Global and Local Stacks' data area
        to "new_size" K-byte blocks.


        Optimizations:
                if the heap hasn't been moved, then there is no need to change:
                        o pointers INTO the heap;
                        o pointers IN the heap (because there shouldn't be
                                any pointing into the local stack).
*/
/*----------------------------------------------------------------------*/

// increment is in k-byte
#define MIN_GL_STACK_INCREMENT(CURRENT_SIZE) (CURRENT_SIZE/4)

/* Dont initialize glstack.high: cf. stack_boundaries  */
void initialize_glstack(CPtr from, CPtr to)
{
  CPtr p = (CPtr) from;
  while (p <= (CPtr) to) {
    *p = 0;
    p++;
  }
}

#define GL_HANDLE_USER_MEMORY_LIMIT_OVERFLOW(CATEGORY,STRING) {		\
    /*    printf("Checking memory %s\n",STRING);		*/	\
    if (flags[MAX_MEMORY_ACTION] == XSB_ERROR) {			\
      flags[MAX_MEMORY] = (int) (flags[MAX_MEMORY]*1.2);		\
      xsb_throw_memory_error(encode_memory_error(CATEGORY,USER_MEMORY_LIMIT)); \
    }									\
    else {								\
      tripwire_interrupt(CTXTc "max_memory_handler");			\
      new_size = glstack.size + MIN_GL_STACK_INCREMENT(glstack.size);	\
      flags[MAX_MEMORY] = (int) (flags[MAX_MEMORY]*1.2 + MIN_GL_STACK_INCREMENT(glstack.size)); \
    }									\
  }

xsbBool glstack_realloc(CTXTdeclc size_t new_size, int arity)
{
  CPtr   new_heap_bot=NULL ;       /* bottom of new Global Stack area */
  CPtr   new_ls_bot ;         /* bottom of new Local Stack area */

  size_t heap_offset ;        /* offsets between the old and new */
  size_t local_offset ;       /* stack bottoms, measured in Cells */

  CPtr   *cell_ptr ;
  Cell   cell_val ;
  size_t i, rnum_in_trieinstr_unif_stk = (trieinstr_unif_stkptr-trieinstr_unif_stk)+1;

  size_t  new_size_in_bytes, new_size_in_cells ; /* what a mess ! */
  double   expandtime ;

#ifdef NON_OPT_COMPILE
  printf("Reallocating the Heap and Local Stack data area from %ld to %ld\n", glstack.size, new_size);
#endif

  if (pflags[STACK_REALLOC] == FALSE) xsb_basic_abort(local_global_exception);

  if (new_size <= glstack.size) { // asked to shrink
    // new_size is space needed + half of init_size, rounded to K
    new_size = (((glstack.high - (byte *)top_of_localstk) +
		 ((byte *)hreg - glstack.low)) + glstack.init_size*K/2 + (K-1)) / K;
    // but not smaller than init_size
    if (new_size < glstack.init_size) new_size = glstack.init_size;
    if (new_size >= glstack.size) return 0;  // computed new_size won't shrink
    //    printf("shrinking glstack from %dK to %dK\n",glstack.size,new_size);
  }

#ifdef DEBUG_VERBOSE
  if (LOG_REALLOC <= cur_log_level) {
    if (glstack.size == glstack.init_size) {
      xsb_dbgmsg((LOG_REALLOC,"\tBottom:\t\t%p\t\tInitial Size: %" Intfmt "K",
		 glstack.low, glstack.size));
      xsb_dbgmsg((LOG_REALLOC,"\tTop:\t\t%p", glstack.high));
    }
  }
#endif
  expandtime = cpu_time();

  new_size_in_bytes = new_size*K ;
  new_size_in_cells = new_size_in_bytes/sizeof(Cell) ;
  		/* and let's hope K stays divisible by sizeof(Cell) */

  stack_boundaries ;

  /* Expand the data area and push the Local Stack to the high end. */

  if (new_size < glstack.size) { //shrinking
    // move local stack down
    memmove(glstack.low + new_size_in_bytes-(glstack.high-(byte *)ls_top),  // to
	    ls_top,  // from
	    glstack.high - (byte *)ls_top  // size
	    );
    new_heap_bot = (CPtr)realloc(heap_bot, new_size_in_bytes);
    heap_offset = new_heap_bot - heap_bot ;
    new_ls_bot = new_heap_bot + new_size_in_cells - 1 ;
    local_offset = new_ls_bot - ls_bot ;
  } else { // expanding
      while (STACK_USER_MEMORY_LIMIT_OVERFLOW(glstack.size, new_size) 
	   && (new_size-glstack.size >= MIN_GL_STACK_INCREMENT(glstack.size)) ) {
      new_size = glstack.size + (new_size-glstack.size)/2;
      //      printf("ulimit problem for gl; trying new_size %ld (user limit = %ld)\n",new_size,flags[MAX_MEMORY]);
    }
    if (new_size - glstack.size < MIN_GL_STACK_INCREMENT(glstack.size)) {
      //      return 1;  /* return an error output -- will be picked up later */
      GL_HANDLE_USER_MEMORY_LIMIT_OVERFLOW(GL_SPACE,"heap_expansion");
    }
    new_heap_bot = (CPtr)realloc(heap_bot, new_size_in_bytes);
    if (new_heap_bot == NULL) {
      //      if (2*glstack.size == new_size) { /* if trying to double, try backing off, may not help */
	size_t increment = new_size;
	while (new_heap_bot == NULL && increment > MIN_GL_STACK_INCREMENT(glstack.size)) {
	  increment = increment/2;
	  new_size = glstack.size + increment;
	  new_size_in_bytes = new_size*K ;
	  new_size_in_cells = new_size_in_bytes/sizeof(Cell) ;
	  //  if (!USER_MEMORY_LIMIT_EXHAUSTED(new_size))
	    new_heap_bot = (CPtr)realloc(heap_bot, new_size_in_bytes);
	}
	if (new_heap_bot == NULL) {
	  //	  xsb_error("Not enough core to resize the Heap/Local Stack! (current: %"Intfmt"; resize %"Intfmt")",
	  //   glstack.size*K,new_size_in_bytes);
	  //return 1; /* return an error output -- will be picked up later */
	  xsb_throw_memory_error(encode_memory_error(GL_SPACE,SYSTEM_MEMORY_LIMIT));
	}
    } 
    //    printf("realloced heap %d -> %d\n",glstack.size,new_size);
    heap_offset = new_heap_bot - heap_bot ;
    new_ls_bot = new_heap_bot + new_size_in_cells - 1 ;
    local_offset = new_ls_bot - ls_bot ;

#if defined(GENERAL_TAGGING)
    //  printf("glstack expand %p %p\n",(void *)new_heap_bot,(void *)new_ls_bot+1);
    extend_enc_dec_as_nec(new_heap_bot,new_ls_bot+1);
#endif

    memmove(ls_top + local_offset,             /* move to */
	    ls_top + heap_offset,              /* move from */
	    (ls_bot - ls_top + 1)*sizeof(Cell) );      /* number of bytes */
}

  initialize_glstack(heap_top + heap_offset,ls_top+local_offset);

  /* TLS: below, the condition should not need to be commented out.
     If the heap expands, there should be no pointers from heap into
     the local stack, so we shouldnt need to traverse the heap.
     However, call subumption code actually copies the substitution
     factor from the CPS to heap (I dont know why, but see the comment
     after the call to subsumptive_call_search() in slginsts_xsb_i.h),
     so that substitution factor pointers may point from the heap to
     local stack.  Therefore the pointer update causes the heap-ls
     pointers to be harmless at glstack expansion.
  */

  /* Update the Heap links */
  //  if (heap_offset != 0) {
    for (cell_ptr = (CPtr *)(heap_top + heap_offset);
	 cell_ptr-- > (CPtr *)new_heap_bot;
	 )
      { reallocate_heap_or_ls_pointer(cell_ptr) ; }
    //  }

  /* Update the pointers in the Local Stack */
  for (cell_ptr = (CPtr *)(ls_top + local_offset);
       cell_ptr <= (CPtr *)new_ls_bot;
       cell_ptr++)
  { reallocate_heap_or_ls_pointer(cell_ptr) ; }

  /* Update the trailed variable pointers */
  for (cell_ptr = (CPtr *)top_of_trail - 1;
       cell_ptr > (CPtr *)tcpstack.low;
       cell_ptr = cell_ptr - 2)
  { /* first the value */
    reallocate_heap_or_ls_pointer(cell_ptr);
    /* now the address */
    cell_ptr-- ;
    cell_val = (Cell)*cell_ptr ;
#ifdef PRE_IMAGE_TRAIL
    if ((size_t) cell_val & PRE_IMAGE_MARK) {
      /* remove tag */
      cell_val = (Cell) ((Cell) cell_val & ~PRE_IMAGE_MARK);
      /* realloc and tag */
      realloc_ref_pre_image(cell_ptr,(CPtr)cell_val) ;
      cell_ptr--;
      /* realoc pre-image */
      reallocate_heap_or_ls_pointer(cell_ptr);
    } else
#endif
      realloc_ref(cell_ptr,(CPtr)cell_val) ;
  }

  /* Update the CP Stack pointers */
  for (cell_ptr = (CPtr *)top_of_cpstack;
       cell_ptr < (CPtr *)tcpstack.high;
       cell_ptr++)
  { reallocate_heap_or_ls_pointer(cell_ptr) ; }

  /* Update the argument registers */
  while (arity)
  { cell_ptr = (CPtr *)(reg+arity) ;
    reallocate_heap_or_ls_pointer(cell_ptr) ;
    arity-- ;  
  }

  i = 0;
  while (i < rnum_in_trieinstr_unif_stk) {
    cell_ptr = (CPtr *)(trieinstr_unif_stk+i);
    //    printf(" reallocate trieinstr_unif_stk[%d]=%p\n",i,cell_ptr);
    reallocate_heap_or_ls_pointer(cell_ptr) ;
    i++;
  }

  /* Update the system variables */
  glstack.low = (byte *)new_heap_bot ;
  glstack.high = (byte *)(new_ls_bot + 1) ;
  pspace_tot_gl = pspace_tot_gl + (new_size - glstack.size)*K;
  glstack.size = new_size ;

  hreg = (CPtr)hreg + heap_offset ;
  hbreg = (CPtr)hbreg + heap_offset ;
  hfreg = (CPtr)hfreg + heap_offset ;
  ereg = (CPtr)ereg + local_offset ;
  ebreg = (CPtr)ebreg + local_offset ;
  efreg = (CPtr)efreg + local_offset ;

  if (islist(delayreg))
    delayreg = (CPtr)makelist(clref_val(delayreg) + heap_offset);

  expandtime = cpu_time() - expandtime;

  xsb_dbgmsg((LOG_REALLOC,"\tNew Bottom:\t%p\t\tNew Size: %" Intfmt "K",
	     glstack.low, glstack.size));
  xsb_dbgmsg((LOG_REALLOC,"\tNew Top:\t%p", glstack.high));
  xsb_dbgmsg((LOG_REALLOC,
	     "Heap/Local Stack data area expansion - finished in %lf secs\n",
	     expandtime));
  return 0;
} /* glstack_realloc */


/*======================================================================*/
/* The main routine that performs garbage collection.                   */
/*======================================================================*/
int garbage_collecting = 0;

int gc_heap(CTXTdeclc int arity, int ifStringGC)
{
#ifdef GC
  CPtr p;
  double  begin_marktime, end_marktime,
    end_slidetime, end_copy_time,
    begin_stringtime, end_stringtime;
  size_t  marked = 0, marked_dregs = 0, i;
  int ii;
  size_t  start_heap_size;
  size_t  rnum_in_trieinstr_unif_stk = (trieinstr_unif_stkptr-trieinstr_unif_stk)+1;
  DECL_GC_PROFILE;
  garbage_collecting = 1;  // flag for profiling that we are gc-ing

  //  printf("start gc(%ld): e:%p,h:%p,hf:%p\n",(long)(cpu_time()*1000),ereg,hreg,hfreg);
  INIT_GC_PROFILE;
  if (pflags[GARBAGE_COLLECT] != NO_GC) {
    num_gc++ ;
    GC_PROFILE_PRE_REPORT;
    slide = (pflags[GARBAGE_COLLECT] == SLIDING_GC) | 
      (pflags[GARBAGE_COLLECT] == INDIRECTION_SLIDE_GC);
    
    if (fragmentation_only) 
      slide = FALSE;
    heap_early_reset = ls_early_reset = 0;
    
    GC_PROFILE_START_SUMMARY;
    
    begin_marktime = cpu_time();
    start_heap_size = hreg+1-(CPtr)glstack.low;
    
    /* make sure the top choice point heap pointer 
       that might not point into heap, does */
    if (hreg == cp_hreg(breg)) {
      *hreg = makeint(666) ;
      hreg++;
    }
#ifdef SLG_GC
    /* same for the freeze heap pointer */
    if (hfreg == hreg && hreg == cp_hreg(bfreg)) {
      *hreg = makeint(66600);
      hreg++;
    }
#endif
    
    /* copy the aregs to the top of the heap - only if sliding */
    /* just hope there is enough space */
    /* this happens best before the stack_boundaries are computed */
    if (slide) {
      if (delayreg != NULL) {
	arity++;
	reg[arity] = (Cell)delayreg;
      }
      for (ii = 1; ii <= arity; ii++) {
	//	printf("reg[%d] to heap: %lx\n",ii,(size_t)reg[i]);
	*hreg = reg[ii];
	hreg++;
      }
      arity += (int)rnum_in_trieinstr_unif_stk;
      for (i = 0; i < rnum_in_trieinstr_unif_stk; i++) {
	//	printf("trieinstr_unif_stk[%d] to heap: %lx\n",i,(size_t)trieinstr_unif_stk[i]);
	*hreg = trieinstr_unif_stk[i];
	hreg++;
      }
      //      printf("extended heap: hreg=%p, arity=%d, rnum_in=%d\n",hreg,arity, rnum_in_trieinstr_unif_stk);
#ifdef SLG_GC
      /* in SLGWAM, copy hfreg to the heap */
      //      printf("hfreg to heap is %p at %p, rnum_in_trieinstr_unif_stk=%d,arity=%d,delay=%p\n",hfreg,hreg,rnum_in_trieinstr_unif_stk,arity,delayreg);
      *(hreg++) = (Cell) hfreg;
#endif
    }

    if (top_of_localstk < hreg) {
      fprintf(stderr,"stack clobbered: no space for gc_heap\n"); 
      xsb_exit( "stack clobbered");
    }

    gc_strings = ifStringGC; /* default */
    gc_strings = should_gc_strings();  // collect strings for any reason?
    marked = mark_heap(CTXTc arity, &marked_dregs);
    
    end_marktime = cpu_time();
    
    if (fragmentation_only) {
      /* fragmentation is expressed as ratio not-marked/total heap in use
	 this is internal fragmentation only.  we print marked and total,
	 so that postprocessing can do what it wants with this info. */
      xsb_dbgmsg((LOG_GC, "marked_used_missed(%d,%d,%d,%d).",
		 marked,hreg+1-(CPtr)glstack.low,
		 heap_early_reset,ls_early_reset));

    free_marks:

#ifdef PRE_IMAGE_TRAIL
      /* re-tag pre image cells in trail */
      for (p = tr_bot; p <= tr_top ; p++ ) {
	if (tr_pre_marked(p-tr_bot)) {
	  *p = *p | PRE_IMAGE_MARK;
	  tr_clear_pre_mark(p-tr_bot);
	}
      }
#endif

      /* get rid of the marking areas - if they exist */
      if (heap_marks)  { mem_dealloc((heap_marks-1),heap_marks_size,GC_SPACE); heap_marks = NULL; }
      if (tr_marks)    { mem_dealloc(tr_marks,tr_top-tr_bot+1,GC_SPACE); tr_marks = NULL; }
      if (ls_marks)    { mem_dealloc(ls_marks,ls_bot - ls_top + 1,GC_SPACE); ls_marks = NULL; }
      if (cp_marks)    { mem_dealloc(cp_marks,cp_bot - cp_top + 1,GC_SPACE); cp_marks = NULL; }
      if (slide_buf)   { mem_dealloc(slide_buf,(slide_buf_size+1)*sizeof(CPtr),GC_SPACE); slide_buf = NULL; }
      goto end;
    }
    
    GC_PROFILE_MARK_SUMMARY;
    
    /* An attempt to add some gc/expansion policy;
       ideally this should be user-controlled */
#if (! defined(GC_TEST))
    if (marked > ((hreg+1-(CPtr)glstack.low)*mark_threshold))
      {
	GC_PROFILE_QUIT_MSG;
        if (slide)
          hreg -= arity;
	total_time_gc += (double) 
	  (end_marktime-begin_marktime);
        goto free_marks; /* clean-up temp areas and get out of here... */
      }
#endif
    
    total_collected += (start_heap_size - marked);

    if (slide)
      {
	GC_PROFILE_SLIDE_START_TIME;
	hreg = slide_heap(CTXTc marked) ;

#ifdef DEBUG_VERBOSE
	if (hreg != (heap_bot+marked))
	  xsb_dbgmsg((LOG_GC, "heap sliding gc - inconsistent hreg"));
#endif

#ifdef SLG_GC
	/* copy hfreg back from the heap */
	hreg--;
	hfreg = (CPtr) *hreg;
#endif

	/* copy the aregs from the top of the heap back */
	hreg -= arity;
	hbreg = cp_hreg(breg);
	
	p = hreg;
	
	arity -= (int)rnum_in_trieinstr_unif_stk;
	for (ii = 1; ii <= arity; ii++) {
	  reg[ii] = *p++;
	  //	  printf("heap to reg[%d]: %lx\n",ii,(size_t)reg[i]);
	}
	if (delayreg != NULL)
	  delayreg = (CPtr)reg[arity--];
	for (i = 0; i < rnum_in_trieinstr_unif_stk; i++) {
	  trieinstr_unif_stk[i] = *p++;
	  //	  printf("heap to trieinstr_unif_stk[%d]: %lx\n",i,(size_t)trieinstr_unif_stk[i]);
	}

	end_slidetime = cpu_time();
	
	total_time_gc += (double) 
	  (end_slidetime - begin_marktime);
	
	GC_PROFILE_SLIDE_FINAL_SUMMARY;
      }
    else
      { /* else we call the copying collector a la Cheney */
	CPtr begin_new_heap, end_new_heap;
	
	GC_PROFILE_COPY_START_TIME;
	
	begin_new_heap = (CPtr)mem_alloc(marked*sizeof(Cell),GC_SPACE);
	if (begin_new_heap == NULL)
	  xsb_exit( "copying garbage collection could not allocate new heap");
	end_new_heap = begin_new_heap+marked;

	hreg = copy_heap(CTXTc marked,begin_new_heap,end_new_heap,arity);

	mem_dealloc(begin_new_heap,marked*sizeof(Cell),GC_SPACE);
	adapt_hfreg_from_choicepoints(CTXTc hreg);
	hbreg = cp_hreg(breg);

#ifdef SLG_GC
	hfreg = hreg;
#endif
	end_copy_time = cpu_time();
	
	total_time_gc += (double) 
	  (end_copy_time - begin_marktime);
	
	GC_PROFILE_COPY_FINAL_SUMMARY;
      }
    
    if (print_on_gc) print_all_stacks(CTXTc arity);
    
    /* get rid of the marking areas - if they exist */
    if (heap_marks)  { 
      check_zero(heap_marks,(heap_top - heap_bot),"heap") ;
      mem_dealloc((heap_marks-1),heap_marks_size,GC_SPACE) ; /* see its calloc */
      heap_marks = NULL ;
    }
    if (tr_marks)    { 
      check_zero(tr_marks,(tr_top - tr_bot + 1),"tr") ;
      mem_dealloc(tr_marks,tr_top-tr_bot+1,GC_SPACE) ;
      tr_marks = NULL ;
    }
    if (ls_marks)    { 
      check_zero(ls_marks,(ls_bot - ls_top + 1),"ls") ;
      mem_dealloc(ls_marks,ls_bot - ls_top + 1,GC_SPACE) ;
      ls_marks = NULL ;
    }
    if (cp_marks)    {  
      check_zero(cp_marks,(cp_bot - cp_top + 1),"cp") ;
      mem_dealloc(cp_marks,cp_bot - cp_top + 1,GC_SPACE) ;
      cp_marks = NULL ;
    }
    if (slide_buf)   { 
      mem_dealloc(slide_buf,(slide_buf_size+1)*sizeof(CPtr),GC_SPACE); 
      slide_buf = NULL; 
    }
#ifdef SAFE_GC
    p = hreg;
    while (p < heap_top)
      *p++ = 0;
#endif
    
  } /* if (pflags[GARBAGE_COLLECT]) */
#else
  /* for no-GC, there is no gc, but stack expansion can be done */
#endif
  
#ifdef GC
 end:
  
  /*************** GC STRING-TABLE (already marked from heap) *******************/
#ifndef NO_STRING_GC
#ifdef MULTI_THREAD
  if (flags[NUM_THREADS] == 1) {
#endif
    if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
      num_sgc++;
      begin_stringtime = cpu_time();
      mark_nonheap_strings(CTXT);
      free_unused_strings();
      //      printf("String GC reclaimed: %d bytes\n",beg_string_space_size - pspacesize[STRING_SPACE]);
      gc_strings = FALSE;
      end_stringtime = cpu_time();
      total_time_gc += end_stringtime - begin_stringtime;
    }
    /* update these even if no GC, to avoid too many calls just to gc strings */
    last_string_space_size = pspacesize[STRING_SPACE];
    last_assert_space_size = pspacesize[ASSERT_SPACE];
    force_string_gc = FALSE;
#ifdef MULTI_THREAD
  }
#endif
#endif /* ndef NO_STRING_GC */

  GC_PROFILE_POST_REPORT;
  garbage_collecting = 0;
  
#endif /* GC */
  //  printf("   end gc(%ld), hf:%p,h:%p, space=%d\n",(long)(cpu_time()*1000),hfreg,hreg,(pb)top_of_localstk - (pb)top_of_heap);

  return(TRUE);

} /* gc_heap */

/*--------------------------------------------------------------------------*/

void glstack_ensure_space(CTXTdeclc size_t extra, int arity) {
  if ((pb)top_of_localstk < (pb)top_of_heap+(256*ZOOM_FACTOR)) {
    xsb_basic_abort("\nFatal ERROR:  -- "
		    "Local Stack clobbered Heap --\n");
  } else {
    if (pflags[GARBAGE_COLLECT] != NO_GC && arity < 255) {
      gc_heap(CTXTc arity,FALSE);
    }
    //    if (pflags[STACK_REALLOC] == FALSE) xsb_basic_abort(local_global_exception);
    if ((pb)top_of_localstk < (pb)top_of_heap + OVERFLOW_MARGIN + extra) {
      glstack_realloc(CTXTc resize_stack(glstack.size,extra+OVERFLOW_MARGIN),arity);
    }
  }
}

/*--------------------------------------------------------------------------*/


