/* File:      gc_slide.h
** Author(s): Luis Castro, Bart Demoen, Kostis Sagonas
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
** $Id: gc_slide.h,v 1.25 2011-05-19 16:39:06 tswift Exp $
** 
*/

/*=======================================================================*/

/* from here to end of slide_heap is code taken to some extent from
   BinProlog and adapted to XSB - especially what concerns the
   environments
   the BinProlog garbage collector was also written originally by Bart Demoen
*/

#ifdef GC

#define h_set_chained(p)	 heap_marks[(p-heap_bot)] |= CHAIN_BIT
#define h_set_unchained(p)	 heap_marks[(p-heap_bot)] &= ~CHAIN_BIT
#define h_is_chained(p)		 (heap_marks[(p-heap_bot)] & CHAIN_BIT)

#define ls_set_chained(p)        ls_marks[(p-ls_top)] |= CHAIN_BIT 
#define ls_set_unchained(p)      ls_marks[(p-ls_top)] &= ~CHAIN_BIT
#define ls_is_chained(p)         (ls_marks[(p-ls_top)] & CHAIN_BIT)

#define cp_set_chained(p)        cp_marks[(p-cp_top)] |= CHAIN_BIT
#define cp_set_unchained(p)      cp_marks[(p-cp_top)] &= ~CHAIN_BIT
#define cp_is_chained(p)         (cp_marks[(p-cp_top)] & CHAIN_BIT)

#define tr_set_chained(p)        tr_marks[(p-tr_bot)] |= CHAIN_BIT 
#define tr_set_unchained(p)      tr_marks[(p-tr_bot)] &= ~CHAIN_BIT
#define tr_is_chained(p)         (tr_marks[(p-tr_bot)] & CHAIN_BIT)

static void unchain(CTXTdeclc CPtr hptr, CPtr destination)
{
  CPtr start, pointsto ;
  int  whereto, tag ;
  int  continue_after_this = 0 ;

/* hptr is a pointer to the heap and is chained */
/* the whole chain is unchained, i.e.
	the end of the chain is put in the beginning and
	all other chained elements (up to end included) are made
		to point to the destination
	we have to make sure that the tags are ok and that the chain tags
		are switched off
   I have implemented a version which can be optimised, but it shows
   all intermediate steps as the previous chaining steps - except for
   the chain bit of hptr
*/

  h_set_unchained(hptr) ;

  do
    {
      start = (CPtr)(*hptr) ;
      /* start is for sure a pointer - possibly with a tag */
      pointsto = pointer_from_cell(CTXTc (Cell)start,&tag,&whereto) ;
      if (pointsto == NULL) xsb_exit( "pointsto error during unchaining") ;
      switch (whereto)
	{
	  case TO_HEAP :
	    continue_after_this = h_is_chained(pointsto) ;
	    h_set_unchained(pointsto) ;
	    break ;
	  case TO_LS :
	    continue_after_this = ls_is_chained(pointsto) ;
	    ls_set_unchained(pointsto) ;
	    break ;
	  case TO_TR :
	    continue_after_this = tr_is_chained(pointsto) ;
	    tr_set_unchained(pointsto) ;
	    break ;
	  case TO_CP :
	    continue_after_this = cp_is_chained(pointsto) ;
	    cp_set_unchained(pointsto) ;
	    break ;
	  default :
	    xsb_exit( "pointsto wrong space error during unchaining");
	    break;
	}
      *hptr = *pointsto ;
      switch (tag) {
      case XSB_REF: 
      case XSB_REF1:
	*pointsto = (Cell)destination ;
	break ;
      case XSB_STRUCT :
	*pointsto = makecs((Cell)destination) ;
	break ;
      case XSB_LIST :
	*pointsto = makelist((Cell)destination) ;
	break ;
      case XSB_ATTV :
	*pointsto = makeattv((Cell)destination);
	break;
      default :
	xsb_exit( "tag error during unchaining") ;
      }
    }
  while (continue_after_this) ;

} /* unchain */

/*----------------------------------------------------------------------*/

int swt_flag = 0;

inline static void swap_with_tag(CTXTdeclc CPtr p, CPtr q, int tag)
{ /* p points to a cell with contents a tagged pointer
     make *q = p + tag, but maybe shift p
  */
  if (swt_flag)  printf("swap_with_tag(%p,%p,%d)\n",p,q,tag);
   *p = *q ;
   switch (tag) {
   case XSB_REF:
   case XSB_REF1:
     *q = (Cell)p ;
     break ;
   case XSB_STRUCT :
     *q = makecs((Cell)p) ;
     break ;
   case XSB_LIST :
     *q = makelist((Cell)p) ;
     break ;
   case XSB_ATTV :
     *q = makeattv((Cell)p);
     break;
   default : xsb_exit( "error during swap_with_tag") ;
   }
} /* swap_with_tag */

#endif /* GC */

/*----------------------------------------------------------------------*/
/*
	slide_heap: implements a sliding collector for the heap
	see: Algorithm of Morris / ACM paper by Appleby et al.
	num_marked = number of marked heap cells
	the relevant argument registers have been moved to the top
	of the heap prior to marking
*/

#ifdef INDIRECTION_SLIDE

#define mem_swap(a,b) \
{ size_t temp; \
 temp = *a; \
 *a = *b; \
 *b = temp; \
}
#define push_sort_stack(X,Y) \
addr_stack[stack_index] = X;\
size_stack[stack_index] = Y;\
stack_index++
#define pop_sort_stack(X,Y)\
stack_index--; \
X = addr_stack[stack_index]; \
Y = size_stack[stack_index]
#define sort_stack_empty \
(stack_index == 0)

static void randomize_data(size_t *data, size_t size)
{
  size_t i,j;

  for (i=0; i<size; i++) {
    j = (size_t) rand()*(size-1)/RAND_MAX;
    mem_swap((data+i), (data+j));
  }
}

static void sort_buffer(size_t *indata, size_t insize)
{
  size_t *left, *right, *pivot;
  size_t *data, size;
  size_t *addr_stack[4000];
  size_t size_stack[4000];
  int stack_index=0;
  Integer leftsize;
#ifdef GC_PROFILE
  size_t begin_sorting, end_sorting;
#endif
  
  randomize_data(indata,insize);

#ifdef GC_PROFILE
  if (verbose_gc)
    begin_sorting = cpu_time();
#endif
  push_sort_stack(indata,insize);

  while (!sort_stack_empty) {
    
    pop_sort_stack(data,size);

    if (size < 1)
      continue;

    if (size == 1) {
      if (data[0] > data[1])
	mem_swap(data, (data+1));
      continue;
    }
    
    left = data;
    right = &data[size];
    
    pivot = &data[size/2];
    mem_swap(pivot, right);
    
    pivot = right;
    
    while (left < right) {
      while ((*left < *pivot) && (left < right)) 
	left++;
      while ((*right >= *pivot) && (left < right))
	right--;
      if (left < right) { 
	mem_swap(left,right);
	left++;
      }
    }
    if (right == data) {
      mem_swap(right, pivot);
      right++;
    }
    leftsize = right - data;
    if (leftsize >= 1)
      push_sort_stack(data,leftsize);
    if ((size-leftsize) >= 1)
      push_sort_stack(right,(size-leftsize));

  } 
#ifdef GC_PROFILE
  if (verbose_gc) {
    end_sorting = cpu_time();
    fprintf(stddbg,"{GC} Sorting took %f ms.\n", (double)
	    (end_sorting - begin_sorting)*1000/CLOCKS_PER_SEC);
  }
#endif
}

#endif

#ifdef GC

static CPtr slide_heap(CTXTdeclc size_t num_marked)
{
  int  tag = 0;  // TLS: to quiet compiler
  Cell contents;
  CPtr p, q ;

  /* chain external (to heap) pointers */      

    /* chain argument registers */
    /* will be automatic as aregisters were copied to the heap */

    /* chain trail: (TLS) this means to go through the trail, and if a
       trail cell points to the heap, then reverse it so that the heap
       cell points to the trail cell, and set the CHAIN_BIT in the
       corresponding cells of the trail and heap mark areas. */
    /* more precise traversal of trail possible */

    { CPtr endtr ;
      endtr = tr_top ;
      for (p = tr_bot; p <= endtr ; p++ ) 
	{ contents = cell(p) ;
	  /* TLS: why is the top of trail special? */
#ifdef SLG_GC
	if (!tr_marked(p-tr_bot))
	  continue;
	tr_clear_mark(p-tr_bot);
#endif
	  q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	  if (!q) continue ;
	  if (! h_marked(q-heap_bot)) {
	    continue ;
	  }
	  if (h_is_chained(q)) tr_set_chained(p) ;
	  h_set_chained(q) ;
	  swap_with_tag(CTXTc p,q,tag) ;
	}
    }

    /* chain choicepoints */
    /* more precise traversal of choice points possible */

    { CPtr endcp ;
      endcp = cp_top ;
      for (p = cp_bot; p >= endcp ; p--)
	{ contents = cell(p) ;
	  q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	  if (!q) continue ;
	  if (! h_marked(q-heap_bot))
	    { xsb_dbgmsg((LOG_DEBUG, "not marked from cp(%p)",p)); continue ; }
	  if (h_is_chained(q)) cp_set_chained(p) ;
	  h_set_chained(q) ;
	  swap_with_tag(CTXTc p,q,tag) ;
	}
    }


    /* chain local stack */
    /* more precise traversal of local stack possible */

    { CPtr endls ;
      endls = ls_top ;
      for (p = ls_bot; p >= endls ; p-- )
	{
	  if (! ls_marked(p-ls_top)) continue ;
	  ls_clear_mark((p-ls_top)) ; /* chain bit cannot be on yet */
	  contents = cell(p) ;
	  q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	  if (!q) continue ;
	  if (! h_marked(q-heap_bot)) continue ;
	  if (h_is_chained(q)) ls_set_chained(p) ;
	  h_set_chained(q) ;
	  swap_with_tag(CTXTc p,q,tag) ;
	}
    }

    /* if (print_on_gc) print_all_stacks() ; */

  { CPtr destination, hptr ;
    Integer garbage = 0 ;
    Integer index ;

    /* one phase upwards - from top of heap to bottom of heap */

    index = heap_top - heap_bot ;
    destination = heap_bot + num_marked - 1 ;
#ifdef INDIRECTION_SLIDE
    if (slide_buffering) {
      size_t i;
#ifdef GC_PROFILE
      if (verbose_gc) {
	fprintf(stddbg,"{GC} Using Fast-Slide scheme.\n");
      }
#endif
      /* sort the buffer */
      sort_buffer((size_t *)slide_buf, slide_top-1);

      /* upwards phase */
      for (i=slide_top; i > 0; i--) {
	hptr = slide_buf[i-1];

	if (h_is_chained(hptr)) {
	  unchain(CTXTc hptr,destination);
	}
	p = hp_pointer_from_cell(CTXTc *hptr,&tag);
	if (p &&(p<hptr)) {
	  swap_with_tag(CTXTc hptr,p,tag);
	  if (h_is_chained(p))
	    h_set_chained(hptr);
	  else
	    h_set_chained(p);
	}
	destination--;
      }
    } else {
#ifdef GC_PROFILE
      if (verbose_gc && pflags[GARBAGE_COLLECT]==INDIRECTION_SLIDE_GC)
	fprintf(stddbg,"{GC} Giving up Fast-Slide scheme.\n");
#endif
#endif /* INDIRECTION_SLIDE */
      for (hptr = heap_top - 1 ; hptr >= heap_bot ; hptr--) {
	if (h_marked(hptr - heap_bot)) {

	  /* boxing: (TLS) apparently garbage is used to denote how
	     long a segment of garbage is -- its put in the bottom
	     cell of the garbage segment. */

	  if (garbage) {
	    *(hptr+1) = makeint(garbage) ;
	    garbage = 0 ;
	  }
	  if (h_is_chained(hptr)) {
	    unchain(CTXTc hptr,destination) ; 
	  }
	  p = hp_pointer_from_cell(CTXTc *hptr,&tag) ;            
	  if (p && (p < hptr)) {
	    swap_with_tag(CTXTc hptr,p,tag) ;
	    if (h_is_chained(p))
	      h_set_chained(hptr) ;
	    else 
	      h_set_chained(p) ;
	  }
	  destination-- ;
	}
	else 
	  garbage++ ;
	index-- ;
      }
#ifdef INDIRECTION_SLIDE
    }
    if (!slide_buffering)
#endif
    if (garbage)
      /* the first heap cell is not marked */
      *heap_bot = makeint(garbage) ;

    /* one phase downwards - from bottom of heap to top of heap */
    index = 0 ;
    destination = heap_bot ;

#ifdef INDIRECTION_SLIDE
    if (slide_buffering) {
      size_t i;
      for (i=0; i<slide_top; i++) {
	hptr = slide_buf[i];

	if (h_is_chained(hptr)) {
	  unchain(CTXTc hptr,destination);
	}
	if ((Cell)(hptr) == *hptr) /* undef */
	  bld_free(destination);
	else {
	  p = hp_pointer_from_cell(CTXTc *hptr,&tag);
	  *destination = *hptr;
	  if (p && (p > hptr)) {
	    swap_with_tag(CTXTc destination,p,tag);
	    if (h_is_chained(p))
	      h_set_chained(destination);
	    else
	      h_set_chained(p);
	  }
	  h_clear_mark((hptr-heap_bot));
	}
	destination++;
      }	    
    } else {
#endif /* INDIRECTION_SLIDE */
      hptr = heap_bot;
      while (hptr < heap_top) {
	if (h_marked(hptr - heap_bot)) {
	  if (h_is_chained(hptr))
	    { unchain(CTXTc hptr,destination) ; }
	  if ((Cell)(hptr) == *hptr) /* UNDEF */
	    bld_free(destination) ;
	  else {
	    p = hp_pointer_from_cell(CTXTc *hptr,&tag) ;
	    *destination = *hptr ;
	    if (p && (p > hptr)) {
	      swap_with_tag(CTXTc destination,p,tag) ;
	      if (h_is_chained(p))           
		h_set_chained(destination) ;   
	      else 
		h_set_chained(p) ;
	    }
	  }
	  h_clear_mark((hptr-heap_bot)) ;
	  hptr++ ; destination++ ;
	  index++ ;
	} else {
	  garbage = int_val(cell(hptr)) ;
	  index += garbage ;
	  hptr += garbage ;
	}
      }
#ifdef DEBUG_VERBOSE
		if (destination != (heap_bot+num_marked))
		  xsb_dbgmsg((LOG_DEBUG, "bad size %p  %p", destination,heap_bot+num_marked));
#endif
#ifdef INDIRECTION_SLIDE
    }  
#endif
  }

#ifdef PRE_IMAGE_TRAIL

    /* re-tag pre image cells in trail */
    for (p = tr_bot; p <= tr_top ; p++ ) {
      if (tr_pre_marked(p-tr_bot)) {
	*p = *p | PRE_IMAGE_MARK;
	tr_clear_pre_mark(p-tr_bot);
      }
    }
#endif

    return(heap_bot + num_marked) ;

} /* slide_heap */

static void check_zero(char *b, Integer l, char *s)
{ 
#ifdef SAFE_GC
  Integer i = 0 ;
  while (l--)
  {
    if (*b++)
      xsb_dbgmsg((LOG_DEBUG, "%s - left marker - %d - %d - %d", s,*(b-1),i,l));
    i++ ;
  }
#endif
} /* check_zero */

#endif

/*=======================================================================*/
