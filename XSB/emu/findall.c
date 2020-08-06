/* File:      findall.c
** Author(s): Bart Demoen
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
** $Id: findall.c,v 1.60 2013-05-06 21:10:24 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"  	     /* cell_xsb.h must be included before deref.h */
#include "deref.h"
#include "register.h"
#include "memory_xsb.h"
#include "psc_xsb.h"
#include "error_xsb.h"
#include "heap_xsb.h"
#include "binding.h"
#include "subp.h"
#include "flags_xsb.h"
#include "loader_xsb.h"
#include "cinterf.h"
#include "findall.h"
#include "thread_xsb.h"
#include "tries.h"
#include "debug_xsb.h"
#include "flag_defs_xsb.h"
#include "struct_intern.h"
#include "cell_xsb_i.h"

#ifndef MULTI_THREAD
findall_solution_list *findall_solutions = NULL;
findall_solution_list *current_findall;

static int nextfree ; /* nextfree index in findall array */

CPtr gl_bot, gl_top ;

static f_tr_chunk *cur_tr_chunk ;
static CPtr *cur_tr_top ;
static CPtr *cur_tr_limit ;
#endif

extern void findall_free(CTXTdeclc int);
extern int get_more_chunk(CTXTdecl);
extern void findall_copy_to_heap(CTXTdeclc Cell, CPtr, CPtr *);
extern int findall_init_c(CTXTdecl);
static void findall_untrail(CTXTdecl);

#define MAX_FINDALLS  250
/* make MAX_FINDALLS larger if you want */

#define on_glstack(p) ((gl_bot <= p) && (p < gl_top))

// seems to sometimes give wrong answers... not currently used
int in_findall_buffers(CTXTdeclc Cell term) {
  CPtr tptr = (CPtr)dec_addr(term);
  CPtr prev_buff = NULL;
  CPtr curr_buff = current_findall->first_chunk;
  do {
    if (tptr >= curr_buff && tptr <= curr_buff+FINDALL_CHUNCK_SIZE) {
      if (curr_buff != current_findall->first_chunk) {
	*prev_buff = *curr_buff;
	*curr_buff = *(current_findall->first_chunk);
	current_findall->first_chunk = curr_buff;
      }
      return TRUE;
    }
    prev_buff = curr_buff;
    curr_buff = *(CPtr *)curr_buff;
  } while (curr_buff);
  return FALSE;
}


#include "ptoc_tag_xsb_i.h"

/* This attempt to recover by freeing allocated chunks doesn't help
        much, since memory is fragmented, so essentially stacks can no
        longer be expanded, at least on the mallocs I've tested.  */
void release_and_throw_error(CTXTdeclc char *mem_call) {
  int i = 0;
  findall_untrail(CTXT);
  while (i < MAX_FINDALLS && (findall_solutions+i) != current_findall) i++;
  if (i < MAX_FINDALLS) {
    findall_free(CTXTc i);
    (findall_solutions + i)->size = i+1 ;
    if ((findall_solutions+i)->first_chunk != NULL) {
      mem_dealloc((findall_solutions+i)->first_chunk,
		  FINDALL_CHUNCK_SIZE * sizeof(Cell),FINDALL_SPACE);
      (findall_solutions+i)->first_chunk = NULL;
    }
  }
  xsb_throw_memory_error(encode_memory_error(FINDALL_SPACE,SYSTEM_MEMORY_LIMIT)); 
}

/* malloc a new chunck and link it in in the current findall */
int get_more_chunk(CTXTdecl)
{ CPtr newchunk ;

  /* calloc so string gc marking doesn't look at uninitted cells */
  if (!(newchunk = (CPtr)mem_calloc_nocheck(FINDALL_CHUNCK_SIZE, sizeof(Cell),FINDALL_SPACE))) {
    release_and_throw_error(CTXTc "mem_calloc()");
  }

  *newchunk = 0 ;
  *(current_findall->current_chunk) = (Cell)newchunk ;
  current_findall->current_chunk = newchunk ;
  current_findall->top_of_chunk = newchunk + 1 ;

  return TRUE;

} /*get_more_chunk*/

/* $$findall_init/2

   to be called with 2 free variables
   the first variable is bound to an index in the findall table
   the second remains free - it will be set to 666 by findall_get_solutions
   without trailing, so that later add's will not occur
*/

int findall_init_c(CTXTdecl)
{
  CPtr w ;
  findall_solution_list *p ;
  int thisfree;
  
  if (findall_solutions == 0)
	{ int i ;
	  p = findall_solutions = (findall_solution_list *)
			mem_alloc(MAX_FINDALLS*sizeof(findall_solution_list),FINDALL_SPACE) ;
	  if (findall_solutions == 0)
	    xsb_exit("init of findall failed") ;
	  for (i = 0 ; i++ < MAX_FINDALLS ; p++)
                { p->first_chunk = NULL; /* no first chunk */
		  p->size = i ;
		  p->tail = 0 ;
		}
          (--p)->size = -1 ;
          nextfree = 0 ;
	}

  if (nextfree < 0) /* could realloc here - too lazy to do it */
	xsb_abort("[FINDALL] Maximum number of active findalls reached");
  thisfree = nextfree;
	/* no checking - no trailing - just use findall_init correctly :-) */
  p = findall_solutions + thisfree ;

  if (p->first_chunk == NULL) {
    w = (CPtr)mem_calloc(FINDALL_CHUNCK_SIZE,sizeof(Cell),FINDALL_SPACE);
  } else w = p->first_chunk;  /* already a first chunk, so use it */
  *w = 0 ;
  p->first_chunk = p->current_chunk = w ;
  w++ ; 
  bld_free(w) ; p->termptr = p->tail = w ; /* create an undef as init of tail */
  w++ ; p->top_of_chunk = w ;
  nextfree = p->size ;
  p->size = 1 ;
  return(thisfree) ;
} /* findall_init_c */

int findall_init(CTXTdecl)
{
  Cell arg1 ;
  int ichunk;
 
  arg1 = ptoc_tag(CTXTc 1); 
  ichunk = findall_init_c(CTXT); 
  *(CPtr)arg1 = makeint(ichunk) ;
  return TRUE;
} /* findall_init */

/* findall_free is called to desactive an entry in the solution_list
   at the end of findall_get_solutions, and from findall_clean
*/

void findall_free(CTXTdeclc int i)
{ CPtr to_free,p ;
  findall_solution_list *this_solution = findall_solutions + i ;

  /* Leave first chunk, so no need to realloc later */
  p = (CPtr) *(this_solution->first_chunk) ;
  while (p != NULL)
    { to_free = p ; p = (CPtr)(*p) ; mem_dealloc(to_free,FINDALL_CHUNCK_SIZE * sizeof(Cell),FINDALL_SPACE) ; }
  this_solution->tail = 0 ;
  this_solution->size = nextfree ;
  nextfree = i ;
} /*findall_free*/

/* findall_clean should be called after interrupts or jumps out of the
   interpreter - or just before jumping back into it.
   It frees the first chunk of each chain of chunks as well.
*/

void findall_clean(CTXTdecl)
{ findall_solution_list *p ;
  int i ;
	p = findall_solutions ;
	if (! p) return ;
	for (i = 0 ; i < MAX_FINDALLS ; i++) {
	  if (p->tail != 0) findall_free(CTXTc i) ;
	  (findall_solutions + i)->size = i+1 ;
	  if ((findall_solutions+i)->first_chunk != NULL) {
	    /* and free every first block as well */
	    mem_dealloc((findall_solutions+i)->first_chunk,
			FINDALL_CHUNCK_SIZE * sizeof(Cell),FINDALL_SPACE);
	    (findall_solutions+i)->first_chunk = NULL;
	  }
	}
	(findall_solutions + i - 1)->size = -1 ;
	nextfree = 0 ;
} /* findall_clean */

/* clean up ALL memory used by findall */
void findall_clean_all(CTXTdecl) {
  return;
  findall_clean(CTXT);
  mem_dealloc(findall_solutions,MAX_FINDALLS*sizeof(findall_solution_list),FINDALL_SPACE);
  findall_solutions = NULL;
  return;
}



/* findall_copy_to_heap does not need overflow checking - heap space is
   ensured; variables in the findall solution list can be altered without
   problem, because they are not needed afterwards anymore, so no trailing
*/

void findall_copy_to_heap(CTXTdeclc Cell from, CPtr to, CPtr *h)
{
copy_again : /* for tail recursion optimisation */

  switch ( cell_tag( from ) )
    {
    case XSB_INT :
    case XSB_STRING :
      *to = from;
      return;
    case XSB_FLOAT :
#ifndef FAST_FLOATS
      {
	Float tempFloat = getfloatval(from);
	bld_functor((*h),box_psc);
	bld_int(((*h)+1),((ID_BOXED_FLOAT << BOX_ID_OFFSET ) | FLOAT_HIGH_16_BITS(tempFloat) ));
	bld_int(((*h)+2),FLOAT_MIDDLE_24_BITS(tempFloat));
	bld_int(((*h)+3),FLOAT_LOW_24_BITS(tempFloat));
	cell(to) = makecs((*h));
	(*h) += 4;
      }
#else
      *to = from;
#endif
      return;
    case XSB_REF :
    case XSB_REF1 :
      XSB_Deref(from);
      if (! isref(from)) goto copy_again; /* it could be a XSB_LIST */
      if (on_glstack((CPtr)(from)))
	*to = from;
      else
	{
	  *(CPtr)from = (Cell)to;
	  *to = (Cell)to;
	}
      return;

  case XSB_LIST :
    {
      CPtr pfirstel;
      Cell q ;

      if (isinternstr_really(from)) {
	*to = from;
	return;
      }

      /* first test whether from - which is an L - is actually the left over
	 of a previously copied first list element
      */
      pfirstel = clref_val(from) ;
      if (on_glstack(pfirstel))
	{
	  /* pick up the old value and copy it */
	  *to = *pfirstel;
	  return;
	}

      q = *pfirstel;

      if (islist(q))
	{
	  CPtr p;
	  
	  p = clref_val(q);
	  if (on_glstack(p))  /* meaning it is a shared list */
	    {
	      *to = q;
	      return;
	    }
	}

      /* this list cell has not been copied before */
      /* now be careful: if the first element of the list to be copied
           is an undef (a ref to an undef is not special !)
           we have to copy this undef now, before we do the general
           thing for lists
      */

      {
	Cell tr1;
        UNUSED(tr1);

	tr1 = *to = makelist(*h) ;
	to = (*h) ;
	(*h) += 2 ;
	if (q == (Cell)pfirstel) /* it is an UNDEF - special care needed */
	  {
	    /* it is an undef in the part we are copying from */
	    *to = (Cell)to ;
	    *pfirstel = makelist((CPtr)to);
	  }
	else
	  {
	    *pfirstel = makelist((CPtr)to);
	    findall_copy_to_heap(CTXTc q,to,h);
	  }
	from = *(pfirstel+1) ; to++ ;
	goto copy_again ;
      }
    }
	      
    case XSB_STRUCT :
      {
	CPtr pfirstel;
	Cell newpsc;
	int ar;
    
	if (isinternstr(from) && isinternstr_really(from)) {
	  *to = from;
	  return;
	}
	pfirstel = (CPtr)cs_val(from);
	if ( cell_tag((*pfirstel)) == XSB_STRUCT )
	  {
	    /* this struct was copied before - it must be shared */
            *to = *pfirstel;
            return;
	  }

	/* first time we visit this struct */
      
	ar = get_arity((Psc)(*pfirstel)) ;
	
	newpsc = *to = makecs((Cell)(*h)) ;
	to = *h ;
	*to = *pfirstel ; /* the functor */
	*pfirstel = newpsc; /* no need for trailing */
	
	*h += ar + 1 ;
	if (ar>0) {
	  while ( --ar )
	    {
	      from = *(++pfirstel) ; to++ ;
	      findall_copy_to_heap(CTXTc from,to,h) ;
	    }
	  from = *(++pfirstel) ; to++ ;
	  goto copy_again ;
	} else return;
      }
  
  case XSB_ATTV: {
    CPtr var;
    
    /*
     * The following XSB_Deref() is necessary, because, in copying
     * f(X,X), after the first occurrence of X is copied, the VAR
     * part of X has been pointed to the new copy on the heap.  When
     * we see this X again, we should dereference it to find that X
     * is already copied, but this deref is not done (see the code
     * in `case XSB_STRUCT:' -- deref's are gone).
     */
    XSB_Deref(from);
    var = clref_val(from);  /* the VAR part of the attv  */
    if (on_glstack(var))    /* is a new attv in the `to area' */
      bld_attv(to, var);
    else {		  /* has not been copied before */
      from = cell(var + 1); /* from -> the ATTR part of the attv */
      XSB_Deref(from);
      *to = makeattv(*h);
      to = (*h);
      (*h) += 2;		  /* skip two cells */
      /*
       * Trail and bind the VAR part of the attv to the new attv
       * just created in the `to area', so that attributed variables
       * are shared in the `to area'.
       */
      bld_attv(var, to);
      cell(to) = (Cell) to;
      to++;
      goto copy_again;
    }
  } /* case XSB_ATTV */
  }
 
} /*findall_copy_to_heap*/


/* trailing variables during copying a template: a linked list of arrays is used */

static void findall_untrail(CTXTdecl)
{
  CPtr *p, *begin_trail ;
  f_tr_chunk *tr_chunk, *tmp ;
  
  if( !(tr_chunk = cur_tr_chunk) ) return ; /* protection */
  begin_trail = tr_chunk->tr ;
  
  for (p = cur_tr_top ; p-- > begin_trail ; )
    {
      *((CPtr)(*p)) = (Cell)(*(p-1));
      p--;
    }
  
  tmp = tr_chunk ; tr_chunk = tr_chunk->previous ; mem_dealloc(tmp,sizeof(f_tr_chunk),FINDALL_SPACE) ;
  while (tr_chunk != 0)
    {
      begin_trail = tr_chunk->tr ;
      for (p = tr_chunk->tr + F_TR_NUM; p-- > begin_trail ; )
	{
	  *((CPtr)(*p)) = (Cell)(*(p-1));
	  p--;
	}
      tmp = tr_chunk ; tr_chunk = tr_chunk->previous ; mem_dealloc(tmp,sizeof(f_tr_chunk),FINDALL_SPACE) ;
    }
} /* findall_untrail */

/* if tr2 == 0, then we need to trail only the first two */

static int findall_trail(CTXTdeclc CPtr p, Cell val)
{ 
  f_tr_chunk *new_tr_chunk ;
  Integer trail_left = cur_tr_limit - cur_tr_top;
  
  if (trail_left == 0)
    {
      if (!(new_tr_chunk = (f_tr_chunk *)mem_alloc_nocheck(sizeof(f_tr_chunk),FINDALL_SPACE)))
	release_and_throw_error(CTXTc "mem_alloc()");
      cur_tr_top = new_tr_chunk->tr ;
      cur_tr_limit = new_tr_chunk->tr+F_TR_NUM ;
      new_tr_chunk->previous = cur_tr_chunk ;
      cur_tr_chunk = new_tr_chunk ;
    }

  *(cur_tr_top++) = (CPtr)val;
  *(cur_tr_top++) = (CPtr)p;
  return TRUE;
} /* findall_trail */

static int init_findall_trail(CTXTdecl)
{
  if (!(cur_tr_chunk = (f_tr_chunk *)mem_alloc(sizeof(f_tr_chunk),FINDALL_SPACE)))
    xsb_exit("init_findall_trail failed");
  cur_tr_top = cur_tr_chunk->tr ;
  cur_tr_limit = cur_tr_chunk->tr+F_TR_NUM ;
  cur_tr_chunk->previous = 0 ;
  return TRUE;
} /* init_findall_trail */

/* findall_copy_template_to_chunk
   must do: overflow checking
   trailing of variables
   returns size of copied term
   if it "fails", returns a negative number
*/

static int findall_copy_template_to_chunk(CTXTdeclc Cell from, CPtr to, CPtr *h)
{
  int size = 0 ;
  int s ;

  copy_again : /* for tail recursion optimisation */

    switch ( cell_tag( from ) )
      {
      case XSB_INT :
      case XSB_STRING :
	*to = from ;
	return(size) ;
    
      case XSB_FLOAT :
	*to = from ;
#ifndef FAST_FLOATS
	return(size+4);  /* will be copied out as boxed so need its space */
#else
	return(size);
#endif
      case XSB_REF :
      case XSB_REF1 :
	if (on_glstack((CPtr)(from)))
	  {
	    findall_trail(CTXTc (CPtr)from,from) ;
	    *(CPtr)from = (Cell)to ;
	    *to = (Cell)to ;
	  } else *to = from ;
	return(size) ;

      case XSB_LIST :
	{
	  CPtr pfirstel ;
	  Cell q ;

	  /* first test whether from - which is an L - is actually the left over
	     of a previously copied first list element
	  */
	  if (isinternstr_really(from)) {
	    *to = from;
	    return(size);
	  }
	  pfirstel = clref_val(from) ;
	  if (! on_glstack(pfirstel) && !isinternstr_really(*pfirstel))  // copied already
	    {
	      /* pick up the old value and copy it */
	      *to = *pfirstel;
	      return(size);
	    }
	  
	  q = *pfirstel;
	  if (islist(q) && !isinternstr_really(q))
	    {
	      CPtr p;
	      
	      p = clref_val(q);
	      if (! on_glstack(p))  // copied already or interned
		{
		  *to = q;
		  return(size);
		}
	    }

	  if (*h > (current_findall->current_chunk + FINDALL_CHUNCK_SIZE - 3))
	    {
	      if (! get_more_chunk(CTXT)) return(-1) ;
	      *h = current_findall->top_of_chunk ;
	    }

	  {
	    Cell tr1;
            UNUSED(tr1);

	    tr1 = *to = makelist(*h) ;
	    to = (*h) ;
	    (*h) += 2 ;
	    if (q == (Cell)pfirstel) /* it is an UNDEF - special care needed */
	      {
		/* it is an undef in the part we are copying from */
		findall_trail(CTXTc pfirstel,(Cell)pfirstel);
		*to = (Cell)to ;
		*pfirstel = makelist((CPtr)to);
		size += 2;
	      }
	    else
	      {
		findall_trail(CTXTc pfirstel,q);
		*pfirstel = makelist((CPtr)to);
		XSB_Deref(q);
		s = findall_copy_template_to_chunk(CTXTc q,to,h);
		if (s < 0) return(-1) ;
		size += s + 2 ;
	      }
	  }
	  from = *(pfirstel+1) ; XSB_Deref(from) ; to++ ;
	  goto copy_again ;
	}
	
      case XSB_STRUCT :
	{
	  CPtr pfirstel ;
	  Cell newpsc;
	  int ar ;
    
	  if (isinternstr(from) && isinternstr_really(from)) {
	    *to = from;
	    return(size);
	  }

	  pfirstel = (CPtr)cs_val(from) ;
	  if ( cell_tag((*pfirstel)) == XSB_STRUCT )
	    {
	      /* this struct was copied before - it must be shared */
	      *to = *pfirstel;
	      return(size);
	    }

	  /* first time we visit this struct */

	  findall_trail(CTXTc pfirstel,*pfirstel);

	  ar = get_arity((Psc)(*pfirstel)) ;
	  /* make sure there is enough space in the chunks */
	  if (*h > (current_findall->current_chunk + FINDALL_CHUNCK_SIZE - 1 - ar))
	    {
	      if (! get_more_chunk(CTXT)) return(-1) ;
	      *h = current_findall->top_of_chunk ;
	    }

	  newpsc = *to = makecs((Cell)(*h)) ;
	  to = *h ;
	  *to = *pfirstel ; /* the functor */
	  *pfirstel = newpsc; /* was trailed already */

	  *h += ar + 1 ;
	  size += ar + 1 ;
	  if (ar>0) {
	    while ( --ar )
	      {
		from = *(++pfirstel) ; XSB_Deref(from) ; to++ ;
		s = findall_copy_template_to_chunk(CTXTc from,to,h) ;
		if (s < 0) return(-1) ;
		size += s ;
	      }
	    from = *(++pfirstel) ; XSB_Deref(from) ; to++ ;
	    goto copy_again ;
	  } else return(size);
	}
  
  case XSB_ATTV: {
    CPtr var;
    
    var = clref_val(from);  /* the VAR part of the attv  */
    if (on_glstack(var)) {  /* has not been copied before */
      from = cell(var + 1); /* from -> the ATTR part of the attv */
      XSB_Deref(from);
      if (*h > (current_findall->current_chunk + FINDALL_CHUNCK_SIZE - 3)) {
	if (! get_more_chunk(CTXT)) return(-1) ;
	*h = current_findall->top_of_chunk ;
      }
      *to = makeattv(*h);
      to = (*h);
      (*h) += 2;		  /* skip two cells */
      size += 2;
      /*
       * Trail and bind the VAR part of the attv to the new attv
       * just created in the `to area', so that attributed variables
       * are shared in the `to area'.
       */
      findall_trail(CTXTc var,(Cell)var);
      bld_attv(var, to);
      cell(to) = (Cell) to;
      to++;
      goto copy_again;
    }
    else {		  /* is a new attv in the `to area' */
      bld_attv(to, var);
      return(size);
    }
  } /* case XSB_ATTV */
  } /* switch */
 
 return(-1) ; /* to keep compiler happy */
 
} /*findall_copy_template_to_chunk */

/*
  $$findall_add/3
  arg1 : any
  arg2 : findall index - where to add it
  arg3 : if not var, then return with fail immediately
  at arg2, the term arg1 is added to the solution list
  if findall_add/2 fails, the associated term remains unchanged
*/

int findall_add(CTXTdecl)
{
  Cell arg1, arg2, arg3 ;
  CPtr to, h ;
  int size ;
  
  arg3 = ptoc_tag(CTXTc 3);
  {
    int t = cell_tag(arg3) ;
    if ((t != XSB_REF) && (t != XSB_REF1)) return(0) ;
  }
  
  arg1 = ptoc_tag(CTXTc 1);
  arg2 = ptoc_tag(CTXTc 2);
  
  current_findall = findall_solutions + int_val(arg2) ;
  if (current_findall->tail == 0)
    xsb_exit("internal error 1 in findall") ;
  
  to = current_findall->top_of_chunk ;
  if ((to+2) > (current_findall->current_chunk + FINDALL_CHUNCK_SIZE -1)) {
    if (! get_more_chunk(CTXT)) return(0) ;
    to = current_findall->top_of_chunk ;
  }
  
  h = to + 2 ;
  gl_bot = (CPtr)glstack.low ; gl_top = (CPtr)glstack.high ;
  
  if (init_findall_trail(CTXT) &&
      (0 <= (size = findall_copy_template_to_chunk(CTXTc arg1,to,&h)))) {
    findall_untrail(CTXT) ;
    current_findall->top_of_chunk = h ;
    /* 2 because of ./2 of solution list */
    current_findall->size += size + 2 ;
    bld_free((to+1)) ;
    /* link in new template now */
    *(CPtr)(*(current_findall->tail)) = makelist(to);
    current_findall->tail = to+1 ; /* fill in new tail */
    return TRUE;
  }
  
  findall_untrail(CTXT) ;
  return FALSE;
} /* findall_add */

/* $$findall_get_solutions/4
   arg1 : out : solution list of findall
   arg2 : out : tail of the list
   arg3 : in : integer = findall index
   arg4 : a variable which is now destructively set to 666
   
   the list at arg3 is copied to the heap and then this copy is unified with
   arg1-arg2 
*/

int findall_get_solutions(CTXTdecl)
{
  Cell arg1, arg2, arg3, arg4, from ;
  int cur_f ;
  findall_solution_list *p ;
  
  arg4 = ptoc_tag(CTXTc 4);
  {
    int t = cell_tag(arg4) ;
    if ((t == XSB_REF) || (t == XSB_REF1)) *(CPtr)arg4 = makeint(666) ;
  }
  
  arg3 = ptoc_tag(CTXTc 3); 
  cur_f = (int)int_val(arg3) ;
  
  p = findall_solutions + cur_f ;
  
  //  check_glstack_overflow(4, pcreg, p->size*sizeof(Cell)) ;
  check_glstack_overflow(4, pcreg, p->size*sizeof(Cell)) ;

  arg1 = ptoc_tag(CTXTc 1);  /* only after enough space is ensured */
  arg2 = ptoc_tag(CTXTc 2);  /* only after enough space is ensured */
  
  gl_bot = (CPtr)glstack.low ; gl_top = (CPtr)glstack.high ;
  
  //  from = *(p->first_chunk+1) ; /* XSB_Deref not necessary */
  from = *(p->termptr);
  //  printf("e1 fcth\n");  print_term(stdout,from,1,(long)flags[WRITE_DEPTH]);printf("\n");
  findall_copy_to_heap(CTXTc from,(CPtr)arg1,&hreg) ; /* this can't fail */
  //  print_term(stdout,arg1,1,(long)flags[WRITE_DEPTH]);printf("\n\n");
  *(CPtr)arg2 = *(p->tail) ; /* no checking, no trailing */
  findall_free(CTXTc cur_f) ;
  return TRUE;
} /* findall_get_solutions */

/* adapted from findall_copy_template_to_chunck */
/* returns the number of cells needed for the construction of term */
Integer term_size(CTXTdeclc Cell term)
{
  Integer size = 0 ;
 recur:
  switch(cell_tag(term)) {
  case XSB_FREE:
  case XSB_REF1:
  case XSB_INT:
  case XSB_STRING:
  case XSB_FLOAT:
    return size ;
  case XSB_LIST: {
    CPtr pfirstel ;
    
    pfirstel = clref_val(term) ;
    term = *pfirstel ; 
    XSB_Deref(term) ;
    size += 2 + term_size(CTXTc term) ;
    term = *(pfirstel+1) ; XSB_Deref(term) ;
    goto recur;
  }
  case XSB_STRUCT: {
    int a ;
    CPtr pfirstel ;
    
    pfirstel = (CPtr)cs_val(term) ;
    a = get_arity((Psc)(*pfirstel)) ;
    size += a + 1 ;
    if (a) {  
      while( --a ) {
	term = *++pfirstel ; 
	XSB_Deref(term) ;
	size += term_size( CTXTc term ) ;
      }
      term = *++pfirstel ; XSB_Deref(term) ;
      goto recur;
    } else return size;
  }
  case XSB_ATTV: {
    CPtr pfirstel;

    pfirstel = clref_val(term);
    if (pfirstel < hreg) {
      /*
       * This is the first occurrence of an attributed variable.  Its
       * first cell (the VAR part) will be changed to an XSB_ATTV cell which
       * points to hreg, and the cell of hreg will be set to a free
       * variable.  So the later occurrence of this attributed variable is
       * dereferenced and seen as an XSB_ATTV pointing to hreg, and we can
       * tell it has been counted before.
       */
      size += 2;
      findall_trail(CTXTc pfirstel,(Cell)pfirstel);
      bld_attv(pfirstel, hreg); /* bind VAR part to a cell out of hreg */
      bld_free(hreg);
      term = cell(clref_val(term) + 1);
      goto recur;
    }
    else /* this XSB_ATTV has been counted before */
      return size;
  }
  }
  return FALSE;
}

/* rewritten */
/* recursively copies a term to a area of memory */
/* used by copy_term to build a variant in the heap */

static void do_copy_term(CTXTdeclc Cell from, CPtr to, CPtr *h)
{
copy_again : /* for tail recursion optimisation */

  switch ( cell_tag( from ) )
    {
    case XSB_INT :
    case XSB_FLOAT :
    case XSB_STRING :
      *to = from ;
      return ;
    
    case XSB_REF :
    case XSB_REF1 :
#ifndef MULTI_THREAD
      if ((CPtr)from < hreg)  /* meaning: a not yet copied undef */
#else
      if ( ((CPtr)from < hreg) || ((CPtr)from >= (CPtr)glstack.high) )
#endif
	{
	  findall_trail(CTXTc (CPtr)from,from) ;
	  *(CPtr)from = (Cell)to ;
	  *to = (Cell)to ;
	}
      else *to = from ;
      return ;

    case XSB_LIST :
      {
	/*
	 *  before copying:
	 *  
	 *  +----+        +----+----+
	 *  | x L|    (x) | a  | b  |    empty trail
	 *  +----+        +----+----+
	 *  
	 *  
	 *  after copying:
	 *  
	 *  
	 *  +----+        +----+----+
	 *  | x L|    (x) | x'L| b  |
	 *  +----+        +----+----+
	 *  
	 *  
	 *  trail:
	 *  
	 *  +----+----+
	 *  | a  | x  |
	 *  +----+----+
	 *  
	 *  
	 *  the copy is:
	 *  
	 *  +----+         +----+----+
	 *  | x'L|    (x') | a' | b' |
	 *  +----+         +----+----+
	 *  
	 *  careful if a is undef !
	 */

	CPtr pfirstel;
	Cell q ;

	/* first test whether from - which is an L - is actually the left over
	   of a previously copied first list element
	*/
	if (isinternstr(from)) {*to = from; return;} // DSWDSW
	pfirstel = clref_val(from) ;
#ifndef MULTI_THREAD
	if (pfirstel >= hreg)
#else
	if ( ((CPtr)pfirstel >= hreg) && ((CPtr)pfirstel < (CPtr)glstack.high) )
#endif
	  {
	    /* pick up the old value and copy it */
	    *to = *pfirstel;
	    return;
	  }

	q = *pfirstel;
	if (islist(q))
	  {
	    CPtr p;

	    p = clref_val(q);
#ifndef MULTI_THREAD
	    if (p >= hreg)  /* meaning it is a shared list */
#else
            if ( (p >= hreg) && (p < (CPtr)glstack.high) )
#endif
	      {
		*to = q;
		return;
	      }
	  }

	/* this list cell has not been copied before */
	/* now be careful: if the first element of the list to be copied
	   is an undef (a ref to an undef is not special !)
	   we have to copy this undef now, before we do the general
	   thing for lists
	*/

	{
	  Cell tr1;
          UNUSED(tr1);

	  tr1 = *to = makelist(*h) ;
	  to = (*h) ;
	  (*h) += 2 ;
	  if (q == (Cell)pfirstel) /* it is an UNDEF - special care needed */
	    {
	      /* it is an undef in the part we are copying from */
	      findall_trail(CTXTc pfirstel,(Cell)pfirstel);
	      *to = (Cell)to ;
	      *pfirstel = makelist((CPtr)to);
	    }
	  else
	    {
	      findall_trail(CTXTc pfirstel,q);
	      *pfirstel = makelist((CPtr)to);
	      XSB_Deref(q);
	      do_copy_term(CTXTc q,to,h);
	    }

	  from = *(pfirstel+1) ; XSB_Deref(from) ; to++ ;
	  goto copy_again ;
	}
      }

    case XSB_STRUCT : {
      /*
       before copying:
       
           +--------+     +-----------------------------------+      +--------+
       (b) |a STRUCT| (a) | Functor | arg1 | arg2 | ... | argn|  (f) |a STRUCT|
           +--------+     +-----------------------------------+      +--------+
       
       trail stack empty
       
       after copying the first (at b)
       
       
           +--------+     +------------------------------------+     +--------+
       (b) |a STRUCT| (a) | d STRUCT | arg1 | arg2 | ... | argn| (f) |a STRUCT|
           +--------+     +------------------------------------+     +--------+
        	       
           +--------+      +-----------------------------------+ 
       (c) |d STRUCT|  (d) | Functor | arg1 | arg2 | ... | argn| 
           +--------+      +-----------------------------------+ 
       
              +-------------+
       trail: | Functor | a |
              +-------------+
        
       c and d are addresses of the copied things
       
       so when we come at the STRUCT pointer at f, we hit the |d STRUCT| cell
       at a, which means that it was copied before
        
       this relies on a Functor cell not having a STRUCT tag
        
       the situation for lists is more complicated
      */
      
	CPtr pfirstel ;
	Cell newpsc;
	int ar ;

	if (isinternstr(from)) {*to = from; return;} // DSWDSW

	pfirstel = (CPtr)cs_val(from) ;
	if ( cell_tag((*pfirstel)) == XSB_STRUCT )
	  {
	    /* this struct was copied before - it must be shared */
	    *to = *pfirstel;
	    return;
	  }

	/* first time we visit this struct */

	findall_trail(CTXTc pfirstel,*pfirstel);

	ar = get_arity((Psc)(*pfirstel)) ;
	
	newpsc = *to = makecs((Cell)(*h)) ;
	to = *h ;
	*to = *pfirstel ; /* the functor */
	*pfirstel = newpsc; /* was trailed already */
	
	*h += ar + 1 ;
	if (ar > 0) {
	  while ( --ar )
	    {
	      from = *(++pfirstel) ; XSB_Deref(from) ; to++ ;
	      do_copy_term(CTXTc from,to,h) ;
	    }
	  from = *(++pfirstel) ; XSB_Deref(from) ; to++ ;
	  goto copy_again ;
	} else return;
      }

    case XSB_ATTV:
      {
	/*
	 *  before copying: (A means XSB_ATTV tag)
	 *  
	 *  +----+        +----+----+
	 *  | x A|    (x) | a  | b  |    empty trail
	 *  +----+        +----+----+
	 *  
	 *  because of deref, a is always an undef, meaning that actually
	 *  a == x
	 *  
	 *  
	 *  after copying:
	 *  
	 *  
	 *  +----+        +----+----+
	 *  | x A|    (x) | x'A| b  |
	 *  +----+        +----+----+
	 *  
	 *  
	 *  trail:
	 *  
	 *  +----+----+
	 *  | x  | x  |
	 *  +----+----+
	 *  
	 *  the copy is:
	 *  
	 *  +----+         +----+----+
	 *  | x'A|    (x') | a' | b' |
	 *  +----+         +----+----+
	 */

	CPtr var;
    
	var = clref_val(from);	/* the VAR part of the attv  */
	if (var < hreg) {	/* has not been copied before */
	  from = cell(var + 1);	/* from -> the ATTR part of the attv */
	  XSB_Deref(from);
	  *to = makeattv(*h);
	  to = (*h);
	  (*h) += 2;		/* skip two cells */
	  /*
	   * Trail and bind the VAR part of the attv to the new attv just
	   * created in the `to area', so that attributed variables are
	   * shared in the `to area'.
	   */
	  findall_trail(CTXTc var,(Cell)var);
	  bld_attv(var, to);
	  cell(to) = (Cell) to;
	  to++;
	  goto copy_again;
	} else			/* is a new attv in the `to area' */
	  bld_attv(to, var);
      } /* case XSB_ATTV */
    } /* switch */
} /* do_copy_term */



static void do_copy_term_3(CTXTdeclc Cell from, CPtr to, CPtr *h)
{
copy_again_3 : /* for tail recursion optimisation */

  switch ( cell_tag( from ) )
    {
    case XSB_INT :
    case XSB_FLOAT :
    case XSB_STRING :
      *to = from ;
      return ;
    
    case XSB_REF :
    case XSB_REF1 :
#ifndef MULTI_THREAD
      if ((CPtr)from < hreg)  /* meaning: a not yet copied undef */
#else
      if ( ((CPtr)from < hreg) || ((CPtr)from >= (CPtr)glstack.high) )
#endif
	{
	  findall_trail(CTXTc (CPtr)from,from) ;
	  *(CPtr)from = (Cell)to ;
	  *to = (Cell)to ;
	}
      else *to = from ;
      return ;

    case XSB_LIST :
      {
	/*
	 *  before copying:
	 *  
	 *  +----+        +----+----+
	 *  | x L|    (x) | a  | b  |    empty trail
	 *  +----+        +----+----+
	 *  
	 *  
	 *  after copying:
	 *  
	 *  
	 *  +----+        +----+----+
	 *  | x L|    (x) | x'L| b  |
	 *  +----+        +----+----+
	 *  
	 *  
	 *  trail:
	 *  
	 *  +----+----+
	 *  | a  | x  |
	 *  +----+----+
	 *  
	 *  
	 *  the copy is:
	 *  
	 *  +----+         +----+----+
	 *  | x'L|    (x') | a' | b' |
	 *  +----+         +----+----+
	 *  
	 *  careful if a is undef !
	 */

	CPtr pfirstel;
	Cell q ;

	/* first test whether from - which is an L - is actually the left over
	   of a previously copied first list element
	*/
	if (isinternstr(from)) {*to = from; return;} // DSWDSW
	pfirstel = clref_val(from) ;
#ifndef MULTI_THREAD
	if (pfirstel >= hreg)
#else
	if ( ((CPtr)pfirstel >= hreg) && ((CPtr)pfirstel < (CPtr)glstack.high) )
#endif
	  {
	    /* pick up the old value and copy it */
	    *to = *pfirstel;
	    return;
	  }

	q = *pfirstel;
	if (islist(q))
	  {
	    CPtr p;

	    p = clref_val(q);
#ifndef MULTI_THREAD
	    if (p >= hreg)  /* meaning it is a shared list */
#else
            if ( (p >= hreg) && (p < (CPtr)glstack.high) )
#endif
	      {
		*to = q;
		return;
	      }
	  }

	/* this list cell has not been copied before */
	/* now be careful: if the first element of the list to be copied
	   is an undef (a ref to an undef is not special !)
	   we have to copy this undef now, before we do the general
	   thing for lists
	*/

	{
	  *to = makelist(*h) ;
	  to = (*h) ;
	  (*h) += 2 ;
	  if (q == (Cell)pfirstel) /* it is an UNDEF - special care needed */
	    {
	      /* it is an undef in the part we are copying from */
	      findall_trail(CTXTc pfirstel,(Cell)pfirstel);
	      *to = (Cell)to ;
	      *pfirstel = makelist((CPtr)to);
	    }
	  else
	    {
	      findall_trail(CTXTc pfirstel,q);
	      *pfirstel = makelist((CPtr)to);
	      XSB_Deref(q);
	      do_copy_term_3(CTXTc q,to,h);
	    }

	  from = *(pfirstel+1) ; XSB_Deref(from) ; to++ ;
	  goto copy_again_3 ;
	}
      }

    case XSB_STRUCT : {
      /*
       before copying:
       
           +--------+     +-----------------------------------+      +--------+
       (b) |a STRUCT| (a) | Functor | arg1 | arg2 | ... | argn|  (f) |a STRUCT|
           +--------+     +-----------------------------------+      +--------+
       
       trail stack empty
       
       after copying the first (at b)
       
       
           +--------+     +------------------------------------+     +--------+
       (b) |a STRUCT| (a) | d STRUCT | arg1 | arg2 | ... | argn| (f) |a STRUCT|
           +--------+     +------------------------------------+     +--------+
        	       
           +--------+      +-----------------------------------+ 
       (c) |d STRUCT|  (d) | Functor | arg1 | arg2 | ... | argn| 
           +--------+      +-----------------------------------+ 
       
              +-------------+
       trail: | Functor | a |
              +-------------+
        
       c and d are addresses of the copied things
       
       so when we come at the STRUCT pointer at f, we hit the |d STRUCT| cell
       at a, which means that it was copied before
        
       this relies on a Functor cell not having a STRUCT tag
        
       the situation for lists is more complicated
      */
      
	CPtr pfirstel ;
	Cell newpsc;
	int ar ;

	if (isinternstr(from)) {*to = from; return;} // DSWDSW

	pfirstel = (CPtr)cs_val(from) ;
	if ( cell_tag((*pfirstel)) == XSB_STRUCT )
	  {
	    /* this struct was copied before - it must be shared */
	    *to = *pfirstel;
	    return;
	  }

	/* first time we visit this struct */

	findall_trail(CTXTc pfirstel,*pfirstel);

	ar = get_arity((Psc)(*pfirstel)) ;
	
	newpsc = *to = makecs((Cell)(*h)) ;
	to = *h ;
	*to = *pfirstel ; /* the functor */
	*pfirstel = newpsc; /* was trailed already */
	
	*h += ar + 1 ;
	if (ar > 0) {
	  while ( --ar )
	    {
	      //	      printf("copying arg %d\n",ar+1);
	      from = *(++pfirstel) ; 
	      //	      printf("pfirstel %p @%p @@%p\n",pfirstel,from,cell((CPtr) dec_addr(from)));
	      XSB_Deref(from) ; to++ ;
	      do_copy_term_3(CTXTc from,to,h) ;
	    }
	  from = *(++pfirstel) ; 
	  //	      printf("pfirstel %p @%p @@%p\n",pfirstel,from,cell((CPtr) dec_addr(from)));
	  XSB_Deref(from) ; to++ ;
	  goto copy_again_3 ;
	} else return;
      }

    case XSB_ATTV: {
	CPtr var;
    
	var = clref_val(from);	/* the VAR part of the attv  */
#ifndef MULTI_THREAD
      if ((CPtr)var < hreg)  /* meaning: a not yet copied undef */
#else
      if ( ((CPtr)var < hreg) || ((CPtr)var >= (CPtr)glstack.high) )
#endif
	{
	  //	  printf("before trail %p @%p @@%p\n",var,cell(var),cell((CPtr) dec_addr(cell(var))));
	  findall_trail(CTXTc var,cell(var)) ;
	  //	  printf("after trail %p @%p @@%p\n",var,cell(var),cell((CPtr) dec_addr(cell(var))));
	  *(CPtr)clref_val(from) = (Cell)var ;
	  *to = (Cell)var ;
	}
      else *to = from ;
      return ;
    }
    }
}


      //      {
      //	/*
      //	 *  before copying: (A means XSB_ATTV tag)
      //	 *  
      //	 *  +----+        +----+----+
      //	 *  | x A|    (x) | a  | b  |    empty trail
      //	 *  +----+        +----+----+
      //	 *  
	      // *  because of deref, a is always an undef, meaning that actually
      //	 *  a == x
      //	 *  
      //	 *  
      //	 *  after copying:
      //	 *  
      //	 *  
      //	 *  +----+        +----+----+
      //	 *  | x A|    (x) | x'A| b  |
      //	 *  +----+        +----+----+
      //	 *  
      //      //	 *  
	      // *  trail:
      //	 *  
      //      //	 *  +----+----+
      //	 *  | x  | x  |
      //	 *  +----+----+
      //	 *  
      //	 *  the copy is:
      //	 *  
      //	 *  +----+         +----+----+
      //	 *  | x'A|    (x') | a' | b' |
      //	 *  +----+         +----+----+
      //	 */
      //
      //	CPtr var;
      //    
      //	var = clref_val(from);	/* the VAR part of the attv  */
      //	if (var < hreg) {	/* has not been copied before */
      //	  from = cell(var + 1);	/* from -> the ATTR part of the attv */
      //	  XSB_Deref(from);
      //	  *to = makeattv(*h);
      //	  to = (*h);
      //      //	  (*h) += 2;		/* skip two cells */
      //	  /*
      //	   * Trail and bind the VAR part of the attv to the new attv just
      //	   * created in the `to area', so that attributed variables are
      //	   * shared in the `to area'.
      //	   */
      //	  findall_trail(CTXTc var,(Cell)var);
      //      //	  bld_attv(var, to);
      //	  cell(to) = (Cell) to;
      //	  to++;
      //      //	  goto copy_again_3;
      //	} else			/* is a new attv in the `to area' */
      //	  bld_attv(to, var);
      //      } /* case XSB_ATTV */
      //    } /* switch */
      //} /* do_copy_term_3 */


/* Creates a new variant of a term in the heap
   arg1 - old term
   arg2 - new term; copy of old term unifies with new term
*/

int copy_term(CTXTdecl)
{
  size_t size ;
  Cell arg1, arg2, to ;
  CPtr hptr ;

  arg1 = ptoc_tag(CTXTc 1);
  
  if( isref(arg1) ) return 1;

  init_findall_trail(CTXT) ;
  size = term_size(CTXTc arg1) ;
  findall_untrail(CTXT) ;

  check_glstack_overflow( 2, pcreg, size*sizeof(Cell)) ;
  
  /* again because stack might have been reallocated */
  arg1 = ptoc_tag(CTXTc 1);
  arg2 = ptoc_tag(CTXTc 2);
  
  hptr = hreg ;
  
  gl_bot = (CPtr)glstack.low ; gl_top = (CPtr)glstack.high ;
  init_findall_trail(CTXT) ;
  do_copy_term( CTXTc arg1, &to, &hptr ) ;
  findall_untrail(CTXT) ;
  
  {
    size_t size2 = hptr - hreg;
    /* fprintf(stderr,"copied size = %d\n",size2); */
    if (size2 > size)
      fprintf(stderr,"panic after copy_term\n");
  }

  hreg = hptr;

  return(unify(CTXTc arg2, to));
} /* copy_term */

int copy_term_3(CTXTdecl)
{
  size_t size ;
  Cell arg1, arg2, to ;
  CPtr hptr ;

  arg1 = ptoc_tag(CTXTc 1);
  
  if( isref(arg1) ) return 1;

  init_findall_trail(CTXT) ;
  size = term_size(CTXTc arg1) ;
  findall_untrail(CTXT) ;

  check_glstack_overflow( 2, pcreg, size*sizeof(Cell)) ;
  
  /* again because stack might have been reallocated */
  arg1 = ptoc_tag(CTXTc 1);
  arg2 = ptoc_tag(CTXTc 2);
  
  hptr = hreg ;
  
  gl_bot = (CPtr)glstack.low ; gl_top = (CPtr)glstack.high ;
  init_findall_trail(CTXT) ;
  do_copy_term_3( CTXTc arg1, &to, &hptr ) ;
  findall_untrail(CTXT) ;
  
  {
    size_t size2 = hptr - hreg;
    /* fprintf(stderr,"copied size = %d\n",size2); */
    if (size2 > size)
      fprintf(stderr,"panic after copy_term\n");
  }

  hreg = hptr;

  return(unify(CTXTc arg2, to));
} /* copy_term_3 */

void mark_findall_strings(CTXTdecl) {
  int i;
  CPtr chunk;
  CPtr cell;

  if (findall_solutions == 0) return;
  for (i = 0; i < MAX_FINDALLS; i++) {
    chunk = (findall_solutions+i)->first_chunk;
    if ((findall_solutions+i)->tail != 0) {
      while (chunk != (findall_solutions+i)->current_chunk) {
	for (cell=chunk+1; cell<(chunk+FINDALL_CHUNCK_SIZE); cell++) {
	  mark_if_string(*cell,"findall");
	}
	chunk = *(CPtr *)chunk;
      }
      for (cell=chunk+1; cell<(findall_solutions+i)->top_of_chunk; cell++) {
	mark_if_string(*cell,"findall");
      }
    }
  }
}

/* Copies a term from another thread's stack
 * NOTE THAT ALL REFERENCES to th are to the newly created thread, rather than the callng thread.
 */
#ifdef MULTI_THREAD
Cell copy_term_from_thread( th_context *th, th_context *from, Cell arg1 )
{
  Integer size ;
  CPtr hptr ;
  Cell to ;

  init_findall_trail(th) ;
  size = term_size(from, arg1) ;
  //  printf("size %d from %x to %x\n",size,from,th);
  findall_untrail(th) ;
  //  thread_check_glstack_overflow(th, 1, size*sizeof(Cell)) ;
  check_glstack_overflow(1,pcreg, size*sizeof(Cell)) ;
  
  hptr = hreg ;

  init_findall_trail(th) ;
  do_copy_term( th, arg1, &to, &hptr ) ;
  findall_untrail(th) ;

  hreg = hptr ;

  return to ;
}
#endif

