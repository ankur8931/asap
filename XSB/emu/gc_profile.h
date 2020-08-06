/* File:      gc_profile.h
** Author(s): Luis Castro
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
** $Id: gc_profile.h,v 1.18 2012-10-03 19:15:21 tswift Exp $
** 
*/

#ifdef GC_PROFILE
#define GC_PROFILE_PRE_REPORT \
do { \
  /* first, run the heap & collect info */ \
  if (examine_data) { \
    CPtr cell_ptr; \
    size_t heap_traversed=0; \
    int i,tag; \
    functor = 0; \
    for (i=0; i<9; i++)  \
      tag_examined[i] = 0; \
    \
    stack_boundaries; \
    \
    for (cell_ptr = (CPtr) glstack.low; cell_ptr < hreg; cell_ptr++) { \
      heap_traversed++; \
      tag = cell_tag(*cell_ptr); \
      if (tag == XSB_REF || tag == XSB_REF1) { \
	if (points_into_heap((CPtr) *cell_ptr)) { \
	  if (cell_ptr == (CPtr) *cell_ptr) { \
	    ++tag_examined[8]; \
	  } else { \
	    tag_examined[tag]++; \
	  } \
	} else if (points_into_heap((CPtr) cell_ptr)) { \
	  functor++; \
	} else { \
	  fprintf(stddbg,"+++ outside pointer %p in %p.\n", \
		  *(CPtr*)cell_ptr, (CPtr)cell_ptr); \
	} \
      } else { \
	++tag_examined[tag]; \
      } \
    } \
    \
    fprintf(stddbg,"\n\n\n{GC} Prior to GC:\n"); \
    fprintf(stddbg,"{GC} Cells visited:\n"); \
    fprintf(stddbg, "{GC}    %ld variables.\n", tag_examined[8]); \
    fprintf(stddbg, "{GC}    %ld reference cells.\n",  \
	    tag_examined[XSB_REF] + tag_examined[XSB_REF1]); \
    fprintf(stddbg, "{GC}    %ld references from the local stack.\n", \
	    chain_from_ls); \
    fprintf(stddbg, "{GC}    %ld atom cells.\n", tag_examined[XSB_STRING]); \
    fprintf(stddbg, "{GC}    %ld integer cells.\n", tag_examined[XSB_INT]); \
    fprintf(stddbg, "{GC}    %ld float cells.\n", tag_examined[XSB_FLOAT]); \
    fprintf(stddbg, "{GC}    %ld list cells.\n", tag_examined[XSB_LIST]); \
    fprintf(stddbg, "{GC}    %ld structure cells.\n",  \
	    tag_examined[XSB_STRUCT]); \
    fprintf(stddbg, "{GC}    %ld functor cells.\n",functor); \
    fprintf(stddbg, "{GC}    %ld attributed variable cells.\n",  \
	    tag_examined[XSB_ATTV]); \
    fprintf(stddbg, "{GC}    %ld heap cells traversed.\n", \
	    heap_traversed); \
    \
  } \
  \
  if (count_chains) { \
    int i; \
    for (i=0; i<64; i++) \
      chains[i]=0; \
  } \
  \
  if (examine_data){  \
    int i; \
    for (i=0; i<9; i++) { \
      tag_examined[i]=0; \
      /*        tag_examined_current[i]=0; */ \
    }\
    chain_from_ls = functor = 0; \
    current_mark = deep_mark=0; \
    start_hbreg = cp_hreg(breg); \
    old_gens = ((size_t) start_hbreg - (size_t) glstack.low) /  \
      sizeof(CPtr); \
    current_gen = ((size_t) hreg - (size_t) start_hbreg) /  \
      sizeof(CPtr); \
    active_cps = 0; \
    frozen_cps = 0; \
  }\
} while(0)

#define INIT_GC_PROFILE \
  verbose_gc=pflags[VERBOSE_GC]; \
  examine_data=pflags[EXAMINE_DATA]; \
  count_chains=pflags[COUNT_CHAINS]

#define DECL_GC_PROFILE \
  double begin_slidetime, begin_copy_time

#define GC_PROFILE_START_SUMMARY \
do { \
    if (verbose_gc) { \
      fprintf(stddbg,"\n{GC} Heap gc - used = %d - left = %d - #gc = %d\n", \
	      hreg+1-(CPtr)glstack.low,ereg-hreg,num_gc);		\
    } \
} while(0)

/* Luis'
  fprintf(stddbg,"{GC} Heap gc - arity = %d - used = %d - left = %d - #gc = %d\n", 
	 arity,hreg+1-(CPtr)glstack.low,ereg-hreg,num_gc); 
*/

#define GC_PROFILE_MARK_SUMMARY \
do { \
    if (verbose_gc) { \
      fprintf(stddbg,"{GC} Heap gc - marking finished - #marked = %d - start compact\n", \
		 marked); \
    }  \
} while (0)

#define GC_PROFILE_QUIT_MSG \
do { \
	if (verbose_gc) { \
	  fprintf(stddbg,"{GC} Heap gc - marked too much - quitting gc\n"); \
	} \
} while (0)

#define GC_PROFILE_SLIDE_START_TIME begin_slidetime = end_marktime

#define GC_PROFILE_SLIDE_FINAL_SUMMARY \
do { \
	if (verbose_gc) { \
	  fprintf(stddbg,"{GC} Heap gc end - mark time = %f; slide time = %f; total = %f\n", \
	  (double)(end_marktime - begin_marktime), \
	  (double)(end_slidetime - begin_slidetime), \
	  total_time_gc) ; \
	} \
} while(0)

#define GC_PROFILE_COPY_START_TIME begin_copy_time = end_marktime

#define GC_PROFILE_COPY_FINAL_SUMMARY \
do { \
	if (verbose_gc) { \
	  fprintf(stddbg,"{GC} Heap gc end - mark time = %f; copy_time = %f; total = %f\n", \
	   (double)(end_marktime - begin_marktime)*1000/CLOCKS_PER_SEC, \
	   (double)(end_copy_time - begin_copy_time)*1000/CLOCKS_PER_SEC, \
	   total_time_gc) ; \
	} \
} while(0)

#define GC_PROFILE_POST_REPORT \
do { \
  if (count_chains|examine_data) { \
    fprintf(stddbg, "\n{GC} Heap Garbage Collection #%d\n",num_gc); \
    fprintf(stddbg, "{GC} Heap early reset reclaimed %d cells.\n", \
	    heap_early_reset); \
    fprintf(stddbg, "{GC} Local early reset reclaimed %d cells.\n", \
	    ls_early_reset); \
  } \
\
  if (count_chains) { \
    int i; \
    fprintf(stddbg,"{GC} Reference Chains: \n"); \
    for (i=0; i<64; i++) \
      if (chains[i]) \
	fprintf(stddbg, "{GC}  chain[%d]=%ld\n",i,chains[i]); \
  } \
\
  if (examine_data) { \
    fprintf(stddbg,"{GC} Active Choice-points: %ld\n", active_cps); \
    fprintf(stddbg,"{GC} Frozen Choice-points: %ld\n",frozen_cps); \
    fprintf(stddbg,"{GC} Local stack size: %d\n", ls_bot - ls_top); \
    fprintf(stddbg,"{GC} CP stack size: %d\n", cp_bot - cp_top); \
    fprintf(stddbg,"{GC} Trail stack size: %d\n", tr_top - tr_bot); \
    fprintf(stddbg,"{GC} Cells visited:\n"); \
    fprintf(stddbg, "{GC}    %ld variables.\n", tag_examined[8]); \
    fprintf(stddbg, "{GC}    %ld reference cells.\n",  \
	    tag_examined[XSB_REF] + tag_examined[XSB_REF1]); \
    fprintf(stddbg, "{GC}    %ld references from the local stack.\n", \
	    chain_from_ls); \
    fprintf(stddbg, "{GC}    %ld atom cells.\n", tag_examined[XSB_STRING]); \
    fprintf(stddbg, "{GC}    %ld integer cells.\n", tag_examined[XSB_INT]); \
    fprintf(stddbg, "{GC}    %ld float cells.\n", tag_examined[XSB_FLOAT]); \
    fprintf(stddbg, "{GC}    %ld list cells.\n", tag_examined[XSB_LIST]); \
    fprintf(stddbg, "{GC}    %ld structure cells.\n",  \
	    tag_examined[XSB_STRUCT]); \
    fprintf(stddbg, "{GC}    %ld functor cells.\n",functor); \
    fprintf(stddbg, "{GC}    %ld attributed variable cells.\n",  \
	    tag_examined[XSB_ATTV]); \
\
    if (current_gen > 0) \
      fprintf(stddbg, "{GC} Cells marked on current generation: %ld / %ld = %ld / 100\n", \
	      current_mark, current_gen, (current_mark*100/current_gen)); \
    if (old_gens > 0) \
      fprintf(stddbg, "{GC} Cells marked on deep generations: %ld / %ld = %ld / 100\n", \
	      deep_mark, old_gens, (deep_mark*100/old_gens)); \
    if (current_gen + old_gens > 0) \
      fprintf(stddbg, "{GC} Total cells marked: %ld / %ld = %ld / 100\n", \
	      deep_mark + current_mark, current_gen + old_gens, \
	      ((deep_mark+current_mark)*100/(current_gen+old_gens))); \
  } \
} while (0)
#else
#define GC_PROFILE_PRE_REPORT
#define INIT_GC_PROFILE
#define DECL_GC_PROFILE
#define GC_PROFILE_START_SUMMARY
#define GC_PROFILE_MARK_SUMMARY 
#define GC_PROFILE_QUIT_MSG
#define GC_PROFILE_SLIDE_START_TIME
#define GC_PROFILE_SLIDE_FINAL_SUMMARY
#define GC_PROFILE_COPY_START_TIME
#define GC_PROFILE_COPY_FINAL_SUMMARY
#define GC_PROFILE_POST_REPORT
#endif


#ifdef GC_PROFILE
inline static void inspect_chain(CPtr cell_ptr)
{
  int tag;
  tag = cell_tag(*cell_ptr);

  if (count_chains) {
    if ((tag == XSB_REF || tag == XSB_REF1) &&
	points_into_heap((CPtr)*cell_ptr)) {
      int temp=0;
      CPtr ptr = (CPtr) *cell_ptr;
      if (points_into_ls(cell_ptr)) {
	temp++;
	ptr = (CPtr) follow(ptr);
	chain_from_ls++;
      }
      while (isref(ptr) && points_into_heap(ptr) && 
	     ptr != (CPtr) follow(ptr)) {
	temp++;
	ptr = (CPtr) follow(ptr);
      }
      if (temp > 64)
	xsb_abort("Chain too long when inspecting cell in %p.\n", cell_ptr);
      ++chains[temp];
    }
  }
}
    
inline static void inspect_ptr(CPtr cell_ptr)
{
  int tag;
  tag = cell_tag(*cell_ptr);
  
  inspect_chain(cell_ptr);

  if (examine_data) {
    if (tag == XSB_REF || tag == XSB_REF1) {
      if (points_into_heap((CPtr) *cell_ptr)) {
	if (cell_ptr == (CPtr) *cell_ptr)
	  ++tag_examined[8];
	else
	  tag_examined[tag]++;
      }
      else if (points_into_heap((CPtr) cell_ptr))
	functor++;
      else
	fprintf(stddbg,"+++ outside pointer %p in %p.\n",
		*(CPtr*)cell_ptr, (CPtr)cell_ptr);
    } else
      ++tag_examined[tag];
    if (cell_ptr < start_hbreg)
      ++deep_mark;
    else
      ++current_mark;
  }
}  

#endif

/* the following cannot be "static inline" since it's used in
   emu/trace_xsb.c and also uses global variables in heap_xsb.c
   --lfcastro */
void print_gc_statistics(CTXTdecl)
{
  char *which = (slide) ? "sliding" : "copying" ;

  printf("%4d heap (%3d string) garbage collections by %s: collected %" UIntfmt " cells in %lf secs\n\n",
	 num_gc, num_sgc, which, total_collected, total_time_gc);
}


