/* File:      gc_mark.h
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
** $Id: gc_mark.h,v 1.43 2013-05-06 21:10:24 dwarren Exp $
** 
*/
//#define NO_STRING_GC

#ifndef MULTI_THREAD
extern Structure_Manager smTSTN;
#endif

extern Structure_Manager smTableBTN;
extern Structure_Manager smAssertBTN;
extern PrRef dynpredep_to_prref(CTXTdeclc void *pred_ep);
extern ClRef db_get_clause_code_space(PrRef, ClRef, CPtr *, CPtr *);
extern void mark_findall_strings(CTXTdecl);
extern void mark_open_filenames();
extern void mark_hash_table_strings(CTXTdecl);
extern int is_interned_rec(Cell);

#ifdef INDIRECTION_SLIDE
#define TO_BUFFER(ptr) \
{ \
  if (slide_buffering) { \
    slide_buf[slide_top] = ptr; \
    slide_top++; \
    slide_buffering = slide_top <= slide_buf_size; \
  } \
}
#else
#define TO_BUFFER(ptr)
#endif

#ifdef GC_PROFILE
#define h_mark(i) \
do { \
  CPtr cell_ptr; int place;\
  place = i;\
  cell_ptr = (CPtr) heap_bot + place;\
  inspect_ptr(cell_ptr);  \
  heap_marks[place] |= MARKED;\
} while (0)
#else
#define h_mark(i)          heap_marks[i] |= MARKED
#endif

#define h_marked(i)        (heap_marks[i])
#define h_clear_mark(i)	   heap_marks[i] &= ~MARKED

#define ls_marked(i)       (ls_marks[i])
#ifdef GC_PROFILE
#define ls_mark(i)         \
do { \
  int tag, place; \
  CPtr ptr; \
  place = i; \
  ptr = (CPtr) ls_top + place; \
  tag = cell_tag(*ptr); \
  inspect_chain(ptr); \
  ls_marks[place] |= MARKED; \
} while (0)
#else
#define ls_mark(i)         ls_marks[i] |= MARKED
#endif
#define ls_clear_mark(i)   ls_marks[i] = 0

#define tr_marked(i)         (tr_marks[i])
#define tr_mark(i)           tr_marks[i] |= MARKED
#define tr_clear_mark(i)     tr_marks[i] &= ~MARKED
#define tr_mark_pre(i)       tr_marks[i] |= TRAIL_PRE
#define tr_clear_pre_mark(i) tr_marks[i] &= ~TRAIL_PRE
#define tr_pre_marked(i)     (tr_marks[i] & TRAIL_PRE)

#define cp_marked(i)       (cp_marks[i])
#define cp_mark(i)         cp_marks[i] |= MARKED
#define cp_clear_mark(i)   cp_marks[i] &= ~MARKED

void mark_interned_term(CTXTdeclc Cell interned_term) {
  int celltag, i, arity;
  Cell subterm;

  if (!isinternstr(interned_term)) {printf("ERROR: calling mark_interned_term\n"); return;}

 begin_mark_interned_term:
  celltag = cell_tag(interned_term);
  if (celltag == XSB_LIST) {
    subterm = get_list_head(interned_term);
    if (isinternstr(subterm)) mark_interned_term(CTXTc subterm);
    else if (isstring(subterm)) mark_string(string_val(subterm),"intern");
    subterm = get_list_tail(interned_term);
    if (isinternstr(subterm)) {
      interned_term = subterm;
      goto begin_mark_interned_term;
    } else if (isstring(subterm)) mark_string(string_val(subterm),"intern");
  } else if (celltag == XSB_STRUCT) {
    arity = get_arity(get_str_psc(interned_term));
    for (i = 1; i < arity; i++) {
      subterm = get_str_arg(interned_term,i);
      if (isinternstr(subterm)) mark_interned_term(CTXTc subterm);
      else if (isstring(subterm)) mark_string(string_val(subterm),"intern");
    }
    subterm = get_str_arg(interned_term,arity);
    if (isinternstr(subterm)) {
      interned_term = subterm;
      goto begin_mark_interned_term;
    } else if (isstring(subterm)) mark_string(string_val(subterm),"intern");
  } else printf("GC ERROR; in mark_interned_term\n");
}

/*=========================================================================*/
/*
hp_pointer_from_cell() returns the address,tag pair if a cell points
to part of the heap.  The trail needs a special function for this
because of pre-image trailing used for handling attributed variables.
The pre-image mark is handled correctly by the garbage collectors --
information about this is set in the array gc_marks in the marking
phase so that subsequent phases deal with clean addresses.  However,
the pre-image value may be a list (or structure) that is in the
attv-interrupt vector rather than in the heap itself.  The new
function thus double-checks to see whether structures or lists are
actually in the heap.
*/

#ifdef GC
inline static CPtr hp_pointer_from_cell(CTXTdeclc Cell cell, int *tag)
{
  int t;
  CPtr retp;

  // unlikely to be right....  don't know what to do here...
  if (isinternstr(cell)) {
    if (!is_interned_rec(cell)) printf("GC: interned str not found %p\n", (void *)cell);
    return NULL;
  }
  t = cell_tag(cell) ;

  /* the use of if-tests rather than a switch is for efficiency ! */
  /* as this function is very heavily used - do not modify */
  if (t == XSB_LIST)
    {
      *tag = XSB_LIST;
      retp = clref_val(cell);
      testreturnit(retp);
    }
  if (t == XSB_STRUCT)
    {
      *tag = XSB_STRUCT;
      retp = (CPtr)(cs_val(cell));
      testreturnit(retp);
    }
  if ((t == XSB_REF) || (t == XSB_REF1))
    {
      *tag = t;
      retp = (CPtr)cell ;
      if (points_into_heap(retp)) return(retp);
    }
  if (t == XSB_ATTV)
    {
      *tag = XSB_ATTV;
      retp = clref_val(cell);
      testreturnit(retp);
    }

  return NULL;
} /* hp_pointer_from_cell */
#endif

static CPtr pointer_from_cell(CTXTdeclc Cell cell, int *tag, int *whereto)
{ int t ;
 CPtr retp ;

 *tag = t = cell_tag(cell) ;
 switch (t)
   {
   case XSB_REF:
   case XSB_REF1: 
     retp = (CPtr)cell ;
     break ;
   case XSB_LIST: 
   case XSB_ATTV:
     retp = clref_val(cell) ;
     break ;
   case XSB_STRUCT:
     //if (isinternstr(cell)) ?? cant do default...
     retp = ((CPtr)(cs_val(cell))) ;
     break ;
   default:
     *whereto = TO_NOWHERE ;
     return((CPtr)cell) ;
   }

 if (points_into_heap(retp)) *whereto = TO_HEAP ;
 else
   if (points_into_tr(retp)) *whereto = TO_TR ;
   else
     if (points_into_ls(retp)) *whereto = TO_LS ;
     else
       if (points_into_cp(retp)) *whereto = TO_CP ;
       else
	 if (points_into_compl(retp)) *whereto = TO_COMPL ;
	 else *whereto = TO_NOWHERE ;
 return(retp) ;

} /* pointer_from_cell */

/*-------------------------------------------------------------------------*/

inline static char * pr_h_marked(CTXTdeclc CPtr cell_ptr)
{ size_t i ;
 i = cell_ptr - heap_bot ;
 if (heap_marks == NULL) return("not_m") ;
 if (h_marked(i) == MARKED) return("marked") ;
 if (h_marked(i) == CHAIN_BIT) return("chained") ;
 if (h_marked(i) == (CHAIN_BIT | MARKED)) return("chained+marked") ;
 return("not_m") ;
} /* pr_h_marked */

inline static char * pr_ls_marked(CTXTdeclc CPtr cell_ptr) 
{ size_t i ; 
 i = cell_ptr - ls_top ;
 if (ls_marks == NULL) return("not_m") ;
 if (ls_marked(i) == MARKED) return("marked") ;
 if (ls_marked(i) == CHAIN_BIT) return("chained") ;
 if (ls_marked(i) == (CHAIN_BIT | MARKED)) return("chained+marked") ;
 return("not_m") ; 
} /* pr_ls_marked */ 

inline static char * pr_cp_marked(CTXTdeclc CPtr cell_ptr) 
{ size_t i ; 
 i = cell_ptr - cp_top ;
 if (cp_marks == NULL) return("not_m") ;
 if (cp_marked(i) == MARKED) return("marked") ;
 if (cp_marked(i) == CHAIN_BIT) return("chained") ;
 if (cp_marked(i) == (CHAIN_BIT | MARKED)) return("chained+marked") ;
 return("not_m") ; 
} /* pr_cp_marked */ 

inline static char * pr_tr_marked(CTXTdeclc CPtr cell_ptr) 
{ size_t i ; 
 i = cell_ptr - tr_bot ;
 if (tr_marks == NULL) return("not_m") ;
 if (tr_marked(i) == MARKED) return("marked") ;
 if (tr_marked(i) == CHAIN_BIT) return("chained") ;
 if (tr_marked(i) == (CHAIN_BIT | MARKED)) return("chained+marked") ;
 if (tr_marked(i) == (CHAIN_BIT | MARKED | TRAIL_PRE)) 
   return("chained+marked+pre");
 return("not_m") ; 
} /* pr_tr_marked */ 

/*-------------------------------------------------------------------------*/

/* Function mark_cell() keeps an explicit stack to perform marking.
   Marking without using such a stack, as in SICStus, should not be
   considered.  It is nice, but slower and more prone to errors.
   Recursive marking is the only alternative in my opinion, but one can
   construct too easily examples that overflow the C-stack - Bart Demoen.
*/

#define MAXS 3700
#define push_to_mark(p) mark_stack[mark_top++] = p
#define mark_overflow   (mark_top >= MAXS)
static int mark_cell(CTXTdeclc CPtr cell_ptr)
{
  CPtr p ;
  Cell cell_val ;
  Integer i;
  int  m, arity, tag ;
  int  mark_top = 0 ;
  CPtr mark_stack[MAXS+MAX_ARITY+1] ;

  m = 0 ;
 mark_more:
  if (!points_into_heap(cell_ptr)) /* defensive marking */
    goto pop_more ;
 safe_mark_more:
  i = cell_ptr - heap_bot ;
  if (h_marked(i)) goto pop_more ;
  TO_BUFFER(cell_ptr);
  h_mark(i) ;
  m++ ;

  cell_val = *cell_ptr;
  tag = cell_tag(cell_val);


  if (tag == XSB_LIST || tag == XSB_ATTV) {
    if (isinternstr(cell_val)) {
      if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
	if (!is_interned_rec(cell_val)) printf("GC: interned str not found %p\n", (void *)cell_val);
	mark_interned_term(CTXTc cell_val);
      }
    } else {
      cell_ptr = clref_val(cell_val) ;
      if (mark_overflow)
	{ m += mark_cell(CTXTc cell_ptr+1) ; }
      else push_to_mark(cell_ptr+1) ;
      goto safe_mark_more ;
    }
  }

  if (tag == XSB_STRUCT) {
    if (isinternstr(cell_val)) {
      if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
	if (!is_interned_rec(cell_val)) printf("GC: interned str not found %p\n", (void *)cell_val);
	mark_interned_term(CTXTc cell_val);
      }
    } else {
      p = (CPtr)cell_val ;
      cell_ptr = ((CPtr)(cs_val(cell_val))) ;
      i = cell_ptr - heap_bot ;
      if (h_marked(i)) goto pop_more ;
      TO_BUFFER(cell_ptr);
      h_mark(i) ; m++ ;
      cell_val = *cell_ptr;
      arity = get_arity((Psc)(cell_val)) ;
      p = ++cell_ptr ;
      if (arity > 0) {
	if (mark_overflow )
	  { while (--arity)
	      { m += mark_cell(CTXTc ++p) ; }
	  }
	else while (--arity) push_to_mark(++p) ;
      }
      goto mark_more ;
    }
  }

  if ((tag == XSB_REF) || (tag == XSB_REF1))
    { p = (CPtr)cell_val ;
      if (p == cell_ptr) goto pop_more ;
    cell_ptr = p ;
    goto mark_more ;
    }

#ifndef NO_STRING_GC
#ifdef MULTI_THREAD
  if (flags[NUM_THREADS] == 1)
#endif
    if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
      if (tag == XSB_STRING) {
	char *astr = string_val(cell_val);
	if (astr && (string_find_safe(astr) == astr))
	  mark_string_safe(astr,"mark_cell");
      }
    }
#endif /* ndef NO_STRING_GC */

 pop_more:
  if (mark_top--)
    { cell_ptr = mark_stack[mark_top] ; goto mark_more ; }
  return(m) ;

} /* mark_cell */

/*----------------------------------------------------------------------*/

/* TLS: Overall, attvs are treated analogously to lists in the GC:
   thus when an attv is encountered, its attribute list is also
   traversed.  Note that when one attv is replaced by another in an
   interrupt handler, it should be through a put_attr().  When this
   happens a chain of attvs is created, so that starting out from some
   cell or another, the old attvs (which may be needed in
   backtracking) are marked since they are proximate to attvs.  Thus,
   you dont need to set the attvs in the pre-image trail (as I
   mistakenly thought), at least not for GC. */

static int mark_root(CTXTdeclc Cell cell_val)
{
  Integer i;
  int m, arity ;
  CPtr cell_ptr;
  int tag, whereto ;
  Cell v ;

  /* this is one of the places to be defensive while marking: an uninitialised
     cell in the ls can point to a Psc; the danger is not in following the Psc
     and mark something outside of the heap: mark_cell takes care of that; the
     dangerous thing is to mark the cell with the Psc on the heap without
     marking all its arguments */

  if (cell_val == 0) return(0) ;
  switch (cell_tag(cell_val))
    {
    case XSB_REF:
    case XSB_REF1:
      v = *(CPtr)cell_val ;
      pointer_from_cell(CTXTc v,&tag,&whereto) ;
      switch (tag)
    	{ case XSB_REF: case XSB_REF1:
	  if (whereto != TO_HEAP) return(0) ;
	  break ;
    	}
      return(mark_cell(CTXTc (CPtr)cell_val)) ;

    case XSB_STRUCT : 
      if (isinternstr(cell_val)) {  // interned term ignore for now
	if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
	  if (!is_interned_rec(cell_val)) printf("GC: interned str not found %p\n", (void *)cell_val);
	  mark_interned_term(CTXTc cell_val);
	}
	return 0;
      } else {
	cell_ptr = ((CPtr)(cs_val(cell_val))) ;
	if (!points_into_heap(cell_ptr)) return(0) ;
	i = cell_ptr - heap_bot ; 
	if (h_marked(i)) return(0) ; 
	/* now check that at i, there is a Psc */
	v = *cell_ptr ;
	pointer_from_cell(CTXTc v,&tag,&whereto) ;
	/* v must be a PSC - the following tries to test this */
	switch (tag) {
	case XSB_REF: 
	case XSB_REF1 :
	  if (whereto != TO_NOWHERE) return(0) ;
	  break ;
	default: xsb_warn(CTXTc "Encountered bad STR pointer in GC marking; ignored\n");
	  return(0);
	}
	TO_BUFFER(cell_ptr);
	h_mark(i) ; m = 1 ; 
	cell_val = *cell_ptr;
	arity = get_arity((Psc)(cell_val)) ;
	while (arity--) m += mark_cell(CTXTc ++cell_ptr) ;
	return(m) ;
      }
    case XSB_LIST: 
      if (isinternstr(cell_val)) {  // interned list ignore for now
	if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
	  if (!is_interned_rec(cell_val)) printf("GC: interned str not found %p\n", (void *)cell_val);
	  mark_interned_term(CTXTc cell_val);
	}
	return 0;
      } // fall through for regular lists!
    case XSB_ATTV:
      /* the 2 cells will be marked iff neither of them is a Psc */
      cell_ptr = clref_val(cell_val) ;
      if (!points_into_heap(cell_ptr)) return(0) ;
      v = *cell_ptr ;
#ifndef NO_STRING_GC
      if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1 && flags[NUM_THREADS] == 1)) 
	mark_if_string(v,"attv 1");
#endif
      pointer_from_cell(CTXTc v,&tag,&whereto) ;
      switch (tag) {
      case XSB_REF:
      case XSB_REF1:
	if (whereto != TO_HEAP) return(0) ;
	break ;
      }
      v = *(++cell_ptr) ;
#ifndef NO_STRING_GC
      if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1 && flags[NUM_THREADS] == 1))
	mark_if_string(v,"attv 2");
#endif
      pointer_from_cell(CTXTc v,&tag,&whereto) ;
      switch (tag) {
      case XSB_REF:
      case XSB_REF1:
	if (whereto != TO_HEAP) return(0) ;
	break ;
      }
      m = mark_cell(CTXTc cell_ptr) ;
      cell_ptr-- ; 
      m += mark_cell(CTXTc cell_ptr) ;
      return(m) ;

#ifndef NO_STRING_GC
    case XSB_STRING:
#ifdef MULTI_THREAD
  if (flags[NUM_THREADS] == 1)
#endif
    if (gc_strings && (flags[STRING_GARBAGE_COLLECT] == 1)) {
      char *sstr = string_val(cell_val);
      if (sstr && (string_find_safe(sstr) == sstr))
	mark_string_safe(sstr,"mark_root");
      return(0);
    }
#endif /* ndef NO_STRING_GC */

    default : return(0) ;
    }

} /* mark_root */

/*----------------------------------------------------------------------*/

inline static int mark_region(CTXTdeclc CPtr beginp, CPtr endp)
{
  int marked = 0 ;

  while (beginp <= endp) {
    marked += mark_root(CTXTc *(beginp++)) ;
  }

  return(marked) ;
} /* mark_region */

/*----------------------------------------------------------------------*/

/* TLS: This function uses the PRE_IMAGE_MARK for the GC marking.  In
    order to run through the trail, it is useful to know whether we
    are dealing with a 3- or 4- cell frame.  If it is a 3-cell frame,
    there won't be a PRE_IMAGE_MARK in the 3-rd cell; otherwise there
    will be. */
 
inline static size_t mark_trail_section(CTXTdeclc CPtr begintr, CPtr endtr)
{
  CPtr a = begintr;
  CPtr trailed_cell;
  size_t i=0, marked=0;
#ifdef PRE_IMAGE_TRAIL
  CPtr pre_value = NULL;
#endif
  
  while (a > (CPtr)endtr)
    { 
      tr_mark(a - tr_bot); /* mark trail cell as visited */
      /* lfcastro -- needed for copying */
      tr_mark((a-tr_bot)-1);
      tr_mark((a-tr_bot)-2);

      trailed_cell = (CPtr) *(a-2);
#ifdef PRE_IMAGE_TRAIL
      if (((UInteger)trailed_cell) & PRE_IMAGE_MARK) {
	trailed_cell = (CPtr) ((Cell) trailed_cell & ~PRE_IMAGE_MARK);
	pre_value = (CPtr) *(a-3);
	tr_mark_pre((a-tr_bot)-2); /* mark somewhere else */
	*(a-2) = ((Cell)trailed_cell & ~PRE_IMAGE_MARK); /* and delete mark */
      /* lfcastro -- needed for copying */
	tr_mark((a-tr_bot)-3);
      }
#endif
      
      if (points_into_heap(trailed_cell))
	{ i = trailed_cell - heap_bot ;
	if (! h_marked(i))
	  {

	    /* TLS: if early reset is not done (and I'm not sure it
 	       still works) the tr_marks array will have one mark
 	       indicating that it needs a mark bit, but the address
 	       will be clean.  Thus the chaining/copying phase does
 	       not need to look at this bit, and it will be reset at
 	       the end of the pass. */

#if (EARLY_RESET == 1)
| 	    {
| 	      /* instead of marking the word in the heap, 
| 		 we make the trail cell point to itself */
| 	      TO_BUFFER(trailed_cell);
| 	      h_mark(i) ;
| 	      marked++ ;
|	      } 
|#ifdef PRE_IMAGE_TRAIL
|	      if (pre_value) 
|		*trailed_cell = (Cell) pre_value;
|	      else
|#endif
|		bld_free(trailed_cell); /* early reset */
|	      
|	      /* could do trail compaction now or later */
|	      heap_early_reset++;
|	    }
#else
	    {
	      marked += mark_root(CTXTc (Cell)trailed_cell);
	    }
#endif
	    
	  }
	}
      else
	/* it must be a ls pointer, but for safety
	   we take into account between_h_ls */
	if (points_into_ls(trailed_cell))
	  { i = trailed_cell - ls_top ;
	  if (! ls_marked(i))
	    {
#if (EARLY_RESET == 1)
	      {
		/* don't ls_mark(i) because we early reset
		   so, it is not a heap pointer
		   but marking would be correct */
#ifdef PRE_IMAGE_TRAIL
		if (pre_value)
		  *trailed_cell = (Cell) pre_value;
		else
#endif
		  bld_free(trailed_cell) ; /* early reset */
		
		/* could do trail compaction now or later */
		ls_early_reset++;
	      }
#else
	      { ls_mark(i) ;
	        marked += mark_root(CTXTc *trailed_cell);
	      }
#endif
	    }
	  }
      
      /* mark the forward value */
      marked += mark_root(CTXTc (Cell) *(a-1));
      
#ifdef PRE_IMAGE_TRAIL
      if (pre_value) { 
	marked += mark_root(CTXTc (Cell) pre_value);
	pre_value = NULL;
      }
#endif

      /* stop if we're not going anywhere */
      if ((UInteger) a == (UInteger) *a)
	break;

      /* jump to previous cell */
      a = (CPtr) *a;
    }
  return marked;
}
/*----------------------------------------------------------------------*/
/* TLS: as I understand it, this should mark all WAM-register regions
 * of choice points, as well as the substitution factors of tabled
 * choice points.  It traverses the CP stack via the special cell
 * prev-top which has noting to do with the previous breg.  The part
 * of this code that checks first time and resets the register values
 * seems a little weird -- I don't see why just takeing the youngest
 * of breg/bfreg and traversing from there wouldn't amount to the same
 * thing. */

static size_t mark_query(CTXTdecl)
{
  Integer i;
  int yvar; size_t total_marked = 0 ;
  CPtr b,e,*tr,a,d;
  byte *cp;
  int first_time;

  b = breg ;
  e = ereg ;
  tr = trreg ;
  cp = cpreg ;
  first_time = 1;

restart:
  while (1)
    {
      while ((e < ls_bot) && (cp != NULL))
	{
	  if (ls_marked(e - ls_top)) break ;
	  ls_mark(e - ls_top) ;
	  /* TLS: get number of perm. vars from cpreg */
	  yvar = *(cp-2*sizeof(Cell)+3) - 1 ;  
	  //	  printf("ereg=%p, npvar=%d\n",e,yvar);
	  total_marked += mark_region(CTXTc e-yvar,e-2) ;
	  i = (e-2) - ls_top ;
	  while (yvar-- > 1) { ls_mark(i--); }
	  cp = (byte *)e[-1] ;
	  e = (CPtr)e[0] ;
	}
      if (b >= (cp_bot-CP_SIZE)) {
	return(total_marked) ;
      }
      a = (CPtr)tr ;
      tr = cp_trreg(b) ;

      /* the answer template is part of the forward computation for 
	 consumers, so it should be marked before the trail in order
	 to allow for early reset                      --lfcastro */
      
      if (is_generator_choicepoint(b)) {
	CPtr region;
	int at_size;
	region = (CPtr) tcp_template(b);
	at_size = (int_val(cell(region)) & 0xffff) + 1;
	while (at_size--)
	  total_marked += mark_cell(CTXTc region--);
      } else if (is_consumer_choicepoint(b)) {
	CPtr region;
	int at_size;
	region = (CPtr) nlcp_template(b);
	at_size = (int_val(cell(region))&0xffff)+1;
	while (at_size--)
	  total_marked += mark_cell(CTXTc region--);
      }

      /* mark the delay list field of all choice points in CP stack too */
      if ((d = cp_pdreg(b)) != NULL) {
	total_marked += mark_root(CTXTc (Cell)d);
      }
      total_marked += mark_trail_section(CTXTc a,(CPtr) tr);

      /* mark the arguments in the choicepoint */
      /* the choicepoint can be a consumer, a generator or ... */

      /* the code for non-tabled choice points is ok */
      /* for all other cps - check that
	 (1) the saved arguments are marked
	 (2) the substitution factor is marked
      */

      if (is_generator_choicepoint(b))
	{ /* mark the arguments */
	  total_marked += mark_region(CTXTc b+TCP_SIZE, tcp_prevtop(b)-1);
	}
      else if (is_consumer_choicepoint(b))
	{ /* mark substitution factor -- skip the number of SF vars */
	  /* substitution factor is in the choicepoint for consumers */
#ifdef SLG_GC
	  if (nlcp_prevtop(b) != b+NLCP_SIZE) {
	    /* this was a producer that was backtracked over --lfcastro */
	    /* mark the arguments, since chaining & copying consider them */
	    CPtr ptr;
	    for (ptr = b+NLCP_SIZE; ptr < nlcp_prevtop(b); ptr++)
	      *ptr = makeint(6660666);
/* 	    total_marked += mark_region(b+NLCP_SIZE, nlcp_prevtop(b)-1); */
	  } 

#endif
	}
      else if (is_compl_susp_frame(b)) 
	/* there is nothing to do in this case */ ;
      else { 
	CPtr endregion, beginregion;
	endregion = cp_prevtop(b)-1;
	beginregion = b+CP_SIZE;
	total_marked += mark_region(CTXTc beginregion,endregion) ;

      }

      e = cp_ereg(b) ;
      cp = cp_cpreg(b) ;
#if defined(GC_PROFILE) && defined(CP_DEBUG)
      if (examine_data) {
	print_cpf_pred(b);
	active_cps++;
      }
#endif
      if (first_time) {
	first_time = 0;
	if (bfreg < breg) {
	  b = bfreg;
	  e = cp_ereg(b);
	  cp = cp_cpreg(b);
	  tr = cp_trreg(b);
	  goto restart;
	}
      }

      b = cp_prevtop(b);
    }

} /* mark_query */

/*----------------------------------------------------------------------*/

static int mark_hreg_from_choicepoints(CTXTdecl)
{
  CPtr b, bprev, h;
  Integer i;
  int  m;

  /* this has to happen after all other marking ! */
  /* actually there is no need to do this for a copying collector */

  b = (bfreg < breg ? bfreg : breg);

  bprev = 0;
  UNUSED(bprev);

  m = 0;
    while(1)
     {
      h = cp_hreg(b) ;
      i = h - heap_bot ;
      if (! h_marked(i)) /* h from choicepoint should point to something that
			    is marked; if not, mark it now and set it
			    to something reasonable - int(666) is ok
			    although a bit scary :-)
			 */
	{
	  cell(h) = makeint(666) ;
	  TO_BUFFER(h);
	  h_mark(i) ;
	  m++ ;
	}
#ifdef SLG_GC
      /* should mark hfreg for generators, too --lfcastro */
      if (is_generator_choicepoint(b)) {
	h = tcp_hfreg(b);
	i = h - heap_bot;
	if (! h_marked(i)) {
	  cell(h) = makeint(6660);
	  TO_BUFFER(h);
	  h_mark(i);
	  m++;
	}
      }
#endif
      bprev = b; 
    b = cp_prevtop(b);
    if (b >= (cp_bot-CP_SIZE))
      break;
  }
  return m;
} /* mark_hreg_from_choicepoints */

/*-------------------------------------------------------------------------*/


size_t mark_heap(CTXTdeclc int arity, size_t *marked_dregs)
{
  size_t avail_dreg_marks = 0, marked = 0, rnum_in_trieinstr_unif_stk = (trieinstr_unif_stkptr-trieinstr_unif_stk)+1;

  /* the following seems unnecessary, but it is not !
     mark_heap() may be called directly and not only through gc_heap() */
  slide = (pflags[GARBAGE_COLLECT] == SLIDING_GC) |
    (pflags[GARBAGE_COLLECT] == INDIRECTION_SLIDE_GC);
  
  stack_boundaries ;
  
  if (print_on_gc) print_all_stacks(CTXTc arity);
  
  if (slide) {
#ifdef INDIRECTION_SLIDE
    /* space for keeping pointers to live data */
    slide_buf_size = (UInteger) ((hreg+1-(CPtr)glstack.low)*0.2);
    slide_buf = (CPtr *) mem_calloc_nocheck(slide_buf_size+1, sizeof(CPtr),GC_SPACE);
    if (!slide_buf)
      xsb_exit( "Not enough space to allocate slide_buf");
    slide_top=0;
    if (pflags[GARBAGE_COLLECT] == INDIRECTION_SLIDE_GC)
      slide_buffering=1;
    else
      slide_buffering=0;
#endif
  }
  
#ifdef INDIRECTION_SLIDE
  else
    slide_buffering=0;
#endif
  
#ifdef SLG_GC
  cp_marks = (char *)mem_calloc_nocheck(cp_bot - cp_top + 1,1,GC_SPACE);
  tr_marks = (char *)mem_calloc_nocheck(tr_top - tr_bot + 1,1,GC_SPACE);
  if ((! cp_marks) || (! tr_marks))
    xsb_exit( "Not enough core to perform garbage collection chaining phase");
#endif  
  heap_marks_size = heap_top - heap_bot + 2 + avail_dreg_marks;
  heap_marks = (char * )mem_calloc_nocheck(heap_marks_size,1,GC_SPACE);
  ls_marks   = (char * )mem_calloc_nocheck(ls_bot - ls_top + 1,1,GC_SPACE);
  if ((! heap_marks) || (! ls_marks))
    xsb_exit("Not enough core to perform garbage collection marking phase");
  
  heap_marks += 1; /* see its free; also note that heap_marks[-1] = 0 is
		      needed for copying garbage collection see copy_block() */
  
  /* start marking phase */
  marked = mark_region(CTXTc reg+1,reg+(slide?(arity-rnum_in_trieinstr_unif_stk):arity));
  if (delayreg != NULL) {
    marked += mark_root(CTXTc (Cell)delayreg);
  }
  if (rnum_in_trieinstr_unif_stk>0) {
    //    printf("marking trieinstr_unif_stk(%d): %p to %p\n",rnum_in_trieinstr_unif_stk,trieinstr_unif_stk,trieinstr_unif_stkptr);
    marked += mark_region(CTXTc trieinstr_unif_stk,trieinstr_unif_stkptr);
  }
  /* Heap[0] is a global variable */
  marked += mark_region(CTXTc (CPtr)glstack.low, (CPtr)glstack.low+2);
  
  if (slide)
    { 
      int put_on_heap;
      put_on_heap = arity;
      marked += put_on_heap;
      while (put_on_heap > 0) {
#ifdef SLG_GC
	TO_BUFFER((heap_top-put_on_heap-1));
	h_mark((heap_top - 1 - put_on_heap)-heap_bot);
	put_on_heap--;
#else
	TO_BUFFER((heap_top-put_on_heap));
	h_mark((heap_top - put_on_heap--)-heap_bot);
#endif
      }
    }

#ifdef SLG_GC
  /* hfreg's also kept in the heap so that it's automatically adjusted */
  /* only for sliding GC */
  if (slide) { 
    CPtr hfreg_in_heap;
    /* mark from hfreg */
    marked += mark_root(CTXTc (Cell)hfreg);
  
    hfreg_in_heap = heap_top - 1;
    TO_BUFFER(hfreg_in_heap);
    if (!h_marked(hfreg_in_heap - heap_bot)) {
      h_mark(hfreg_in_heap - heap_bot);
      marked++;
    }
  }
#endif

  marked += mark_query(CTXT);

  if (slide)
    marked += mark_hreg_from_choicepoints(CTXT);

  if (print_on_gc) print_all_stacks(CTXTc arity);

  return marked ;
} /* mark_heap */

#ifndef NO_STRING_GC
#define mark_trie_strings_for(SM,BType,pStruct,apStruct)	\
  pBlock = SM_CurBlock(SM);					\
  if (pBlock != NULL) {						\
    int isusedblock, anyunusedblock = FALSE;			\
    for (pStruct = (BType)SMBlk_FirstStruct(pBlock); 		\
	 pStruct < (BType)SM_NextStruct(SM); 			\
	 pStruct = (BType)SMBlk_NextStruct(pStruct,SM_StructSize(SM))) { \
      if (isstring(BTN_Symbol(pStruct)) &&				\
	  (*(((Integer *)pStruct)+1) != FREE_TRIE_NODE_MARK)) {		\
        mark_string(string_val(BTN_Symbol(pStruct)),"trie 1");  \
      }								\
    }								\
    pBlock = SMBlk_NextBlock(pBlock);				\
    while (pBlock != NULL) {					\
      isusedblock = FALSE;					\
      for (pStruct = (BType)SMBlk_FirstStruct(pBlock);	 	\
	   pStruct <= (BType)SMBlk_LastStruct(pBlock,SM_StructSize(SM),SM_StructsPerBlock(SM)); \
	   pStruct = (BType)SMBlk_NextStruct(pStruct,SM_StructSize(SM))) { \
        if (*(((Integer *)pStruct)+1) != FREE_TRIE_NODE_MARK) { \
	  isusedblock = TRUE;					\
	  if (isstring(BTN_Symbol(pStruct)))			\
	    mark_string(string_val(BTN_Symbol(pStruct)),"trie 2");\
	}							\
      }								\
      anyunusedblock |= !isusedblock;				\
      if (!isusedblock) {					\
        /*printf("%p in %p is an unused block\n",pBlock,SM);*/	\
        for (pStruct = (BType)SMBlk_FirstStruct(pBlock);	\
	     pStruct <= (BType)SMBlk_LastStruct(pBlock,SM_StructSize(SM),SM_StructsPerBlock(SM)); \
	     pStruct = (BType)SMBlk_NextStruct(pStruct,SM_StructSize(SM))) { \
          *(((Integer *)pStruct)+1) = FREE_TRIE_BLOCK_MARK;	\
	}							\
      }								\
      pBlock = SMBlk_NextBlock(pBlock);				\
    }								\
    if (anyunusedblock) {					\
      apStruct = (BType *)&SM_FreeList(SM);	       		\
      while (*apStruct != NULL) {				\
	if (*((Integer *)(*apStruct)+1) == FREE_TRIE_BLOCK_MARK)\
	  *apStruct = *((void **)(*apStruct));			\
	else apStruct = *(void **)apStruct;			\
      }								\
      apBlock = &(SM_CurBlock(SM));				\
      while (*apBlock != NULL) {				\
	if (*((Integer *)(*apBlock)+2) == FREE_TRIE_BLOCK_MARK) {\
	  pBlock = *apBlock;					\
	  *apBlock = *((void **)(*apBlock));			\
	  mem_dealloc(pBlock,SM_NewBlockSize(SM),TABLE_SPACE);	\
	} else apBlock = *((void **)(apBlock));			\
      }								\
    }								\
  }


void mark_trie_strings(CTXTdecl) {

  BTNptr pBTNStruct, *apBTNStruct;
  TSTNptr pTSTNStruct, *apTSTNStruct;
  void *pBlock, **apBlock;

#ifdef MULTI_THREAD
  //  printf("marking private trie strings\n");
  mark_trie_strings_for(*private_smTableBTN,BTNptr,pBTNStruct,apBTNStruct);
  mark_trie_strings_for(*private_smAssertBTN,BTNptr,pBTNStruct,apBTNStruct);
  //  printf("marked private trie strings\n");
#endif  
  mark_trie_strings_for(smTableBTN,BTNptr,pBTNStruct,apBTNStruct);
  mark_trie_strings_for(smAssertBTN,BTNptr,pBTNStruct,apBTNStruct);
  mark_trie_strings_for(smTSTN,TSTNptr,pTSTNStruct,apTSTNStruct);
}

void mark_code_strings(CTXTdeclc int pflag, CPtr inst_addr, CPtr end_addr) {
  int  current_opcode, oprand;

  //  printf("buffer\n");
  while (inst_addr<end_addr) {
    current_opcode = cell_opcode(inst_addr);
    //    printf("opcode: %x\n",current_opcode);
    inst_addr ++;
    for (oprand=1; oprand<=4; oprand++) {
      switch (inst_table[current_opcode][oprand]) {
      case A: case V: case R: case P: case PP: case PPP:
      case PPR: case PRR: case RRR: case X:
	break;
      case S: case L: case N: case B: case F:
      case I: case T:	  
	inst_addr ++;
	break;
      case G:
	printf("gc optype G\n");
      case C:
	if (pflag) printf("    %s\n",*(char **)inst_addr);
	mark_string(*(char **)inst_addr,"code");
	inst_addr ++;
	break;
      case H: // internstr
	mark_interned_term(CTXTc *(CPtr)inst_addr);
	inst_addr ++;
	break;
      default:
	break;
      }  /* switch */
    } /* for */
  }
}

void mark_atom_and_code_strings(CTXTdecl) {
  size_t i;
  Pair pair_ptr, mod_pair_ptr;
  PrRef prref;
  ClRef clref;
  CPtr code_beg, code_end;

  //  printf("marking atom and code strings\n");
  //  printf("marking atoms in usermod\n");
  for (i = 0; i < symbol_table.size; i++) {
    if (symbol_table.table[i] != NULL) {
      for (pair_ptr = (Pair)symbol_table.table[i]; pair_ptr != NULL;
	   pair_ptr = pair_next(pair_ptr)) {
	char *string = get_name(pair_psc(pair_ptr));
	mark_string(string,"usermod atom");
	if (get_type(pair_psc(pair_ptr)) == T_PRED && isstring(get_data(pair_psc(pair_ptr)))) {
	  string = string_val((get_data((pair_psc(pair_ptr)))));
	  mark_string(string,"filename");
	}
	if (get_type(pair_psc(pair_ptr)) == T_DYNA) {
	  //	  printf("mark dc for usermod:%s/%d\n",string,get_arity(pair_psc(pair_ptr)));
	  prref = dynpredep_to_prref(CTXTc get_ep(pair_psc(pair_ptr))); // fix for multi-threading to handle dispatch for privates 
	  if (prref) {
	    clref = db_get_clause_code_space(prref,(ClRef)NULL,&code_beg,&code_end);
	    while (clref) {
	      mark_code_strings(CTXTc 0,code_beg,code_end);
	      clref = db_get_clause_code_space(prref,clref,&code_beg,&code_end);
	    }
	  }
	}
      }
    }
  }
  for (mod_pair_ptr = (Pair)flags[MOD_LIST]; 
       mod_pair_ptr != NULL; 
       mod_pair_ptr = pair_next(mod_pair_ptr)) {
    mark_string(get_name(pair_psc(mod_pair_ptr)),"mod");
    pair_ptr = (Pair) get_data(pair_psc(mod_pair_ptr));
    if ((Integer)pair_ptr != 1) { // not global mod 
      while (pair_ptr != NULL) {
	char *string = get_name(pair_psc(pair_ptr));
	mark_string(string,"mod atom");
	if (get_type(pair_psc(pair_ptr)) == T_DYNA) {
	  //	  if (strcmp(get_name(pair_psc(pair_ptr)),"ipObjectSpec_T")==0) printf("mark dc for %s:%s/%d\n",get_name(pair_psc(mod_pair_ptr)),string,get_arity(pair_psc(pair_ptr)));
	  prref = dynpredep_to_prref(CTXTc get_ep(pair_psc(pair_ptr)));
	  clref = db_get_clause_code_space(prref,(ClRef)NULL,&code_beg,&code_end);
	  while (clref) {
	    //	    printf("  mark code from %s/%d(%s), %p\n", string, get_arity(pair_psc(pair_ptr)), get_name(pair_psc(mod_pair_ptr)), clref);
	    mark_code_strings(CTXTc 0,code_beg,code_end);
	    clref = db_get_clause_code_space(prref,clref,&code_beg,&code_end);
	  }
	}
	pair_ptr = pair_next(pair_ptr);
      }
    }
  }
  //  printf("marked atom and code strings\n");
}

void mark_nonheap_strings(CTXTdecl) {
  char *empty;

  mark_string((char *)(ret_psc[0]),"ret"); /* "ret" necessary */
  mark_string(nil_string,"[]"); /* necessary */
  empty = string_find_safe("");
  if (!empty) mark_string(empty,"empty");

  mark_trie_strings(CTXT);
  mark_atom_and_code_strings(CTXT);
  mark_findall_strings(CTXT);
  mark_open_filenames();
  mark_hash_table_strings(CTXT);

}
#endif /* ndef NO_STRING_GC */

