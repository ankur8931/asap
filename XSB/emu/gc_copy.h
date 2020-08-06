/* File:      gc_copy.h
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
** $Id: gc_copy.h,v 1.18 2011-05-18 19:21:40 dwarren Exp $
** 
*/

/*=======================================================================*/
/*  Heap collection by copying                                           */
/*  Marking is finished and new heap allocated                           */
/*  Idea is to copy a la Cheney and copy immediatly back to original     */
/*          heap - this allows release of the new heap                   */
/*          Cheney is slightly adapted to make it possible to have       */
/*          the copy back as a mem copy; so heap pointers during Cheney  */
/*          will already point to the final destination                  */
/*                                                                       */
/*  It is very similar to the BinProlog garbage collector.               */
/*=======================================================================*/

#ifdef GC

/* the following variables are set by copy_heap() and used as */
/* globals in the two functions below.                        */

#ifndef MULTI_THREAD
static Integer gc_offset;
static CPtr gc_scan, gc_next;
#endif


#ifdef DEBUG_ASSERTIONS
static void CHECK(CPtr p)
{ CPtr q;
  q = (CPtr)(*p);
  if (((heap_bot - gc_offset) <= q) && (q < gc_next)) return;
  xsb_dbgmsg((LOG_GC, "really bad thing discovered"));
} /* CHECK */
#define GCDBG(mes,val) /*if (num_gc == 61)*/ xsb_dbgmsg((LOG_GC,mes,val))
#else
#define CHECK(p)
#define GCDBG(mes,val)
#endif

#define adapt_external_heap_pointer(P,Q,TAG) \
    CHECK(Q);\
    GCDBG("Adapting %p ", P); GCDBG("with %p ", Q);\
    Q = (CPtr)((CPtr)(cell(Q))+gc_offset); \
    if (TAG == XSB_REF || TAG == XSB_REF1) {\
      bld_ref(P, Q); \
    } else {\
      cell(P) = (Cell)(enc_addr(Q) | TAG); \
    } \
    GCDBG("to %lx\n", cell(P))

#define copy_block(HP,NEXT) /* make sure this macro does not modify HP ! */\
    i = HP-heap_bot; \
    while (h_marked(--i)) ; /* assumes that h_marked[-1] = 0 !!! */\
    /* while (--i >= 0 && h_marked(i)) ; otherwise */\
    p = heap_bot+i+1;\
    for (i = p-heap_bot; h_marked(i); p++, i++) { \
      h_clear_mark(i); \
      cell(NEXT) = cell(p); \
      cell(p) = (Cell)(NEXT); /* leave a forwarding pointer */\
      NEXT++; \
    }

static void find_and_copy_block(CTXTdeclc CPtr hp)
{
    Integer  i, tag;
    CPtr p, q, addr;

    /* copy the block into the new heap area */
    copy_block(hp,gc_next);

    /* perform a Cheney gc_scan: pointer "gc_scan" chases the "gc_next" pointer  */
    /* note that "gc_next" is modified inside the for loop by copy_block() */
    for ( ; gc_scan < gc_next; gc_scan++) {
      q = (CPtr)cell(gc_scan);
      tag = cell_tag(q);
      switch (tag) {
      case XSB_REF: 
      case XSB_REF1:
	if (points_into_heap(q)) {
	  GCDBG("Reference to heap with tag %d\n", tag);

	  xsb_dbgmsg((LOG_GC, "In adapting case for %p with %p (%lx)...",
		     gc_scan, q, cell(q)));

	  if (h_marked(q-heap_bot)) {
	    copy_block(q,gc_next);
	  }
	  q = (CPtr)((CPtr)(cell(q))+gc_offset);
	  GCDBG(" to be adapted to %p\n", q);
	  bld_ref(gc_scan, q);
	}
	break;
      case XSB_STRUCT :
	addr = (CPtr)cs_val(q);
	GCDBG("Structure pointing to %p found...\n", addr);
	if (h_marked(addr-heap_bot)) { /* if structure not already copied */
	  copy_block(addr,gc_next); /* this modifies *addr */
	}
	CHECK(addr);
	GCDBG("*p = %lx ", cell(addr));
	addr = (CPtr)((CPtr)(cell(addr))+gc_offset);
	GCDBG("q = %p ", addr);
	bld_cs(gc_scan, addr);
	GCDBG("made to point to %lx\n", cell(gc_scan));
	break;
      case XSB_LIST :
	addr = clref_val(q);
	GCDBG("List %p found... \n", addr);
	if (h_marked(addr-heap_bot)) { /* if list head not already copied */
	  copy_block(addr,gc_next); /* this modifies *addr */
	}
	CHECK(addr);
	addr = (CPtr)((CPtr)(cell(addr))+gc_offset);
	bld_list(gc_scan, addr);
	break;
      case XSB_ATTV:
	addr = clref_val(q);
	GCDBG("Attv %p found... \n", addr);
	if (h_marked(addr-heap_bot)) {
	  copy_block(addr,gc_next);
	}
	CHECK(addr);
	addr = (CPtr)((CPtr)(cell(addr))+gc_offset);
	bld_attv(gc_scan, addr);
	break;
      default :
	break;
      }
    }
} /* find_and_copy_block */

#endif /* GC */

/*-------------------------------------------------------------------------*/
#ifdef GC

inline static void adapt_hreg_from_choicepoints(CTXTdeclc CPtr h)
{
  CPtr b, bprev;

  /* only after copying collection */
  b = breg;
  b = top_of_cpstack;
  bprev = 0;
  while (b != bprev) {
    cp_hreg(b) = h;
    bprev = b; 
    b = cp_prevbreg(b);
  }
} /* adapt_hreg_from_choicepoints */

#endif

#ifdef SLG_GC

inline static void adapt_hfreg_from_choicepoints(CTXTdeclc CPtr h)
{
  CPtr b, bprev;
  b = (bfreg < breg ? bfreg : breg);
  bprev = 0;
  UNUSED(bprev);
  while (1) {
    if (is_generator_choicepoint(b))
      tcp_hfreg(b) = h;
    cp_hreg(b) = h;
    b = cp_prevtop(b);
    if (b >= (cp_bot - CP_SIZE))
      break;
  }
}

#endif

/*=======================================================================*/

#ifdef GC

static CPtr copy_heap(CTXTdeclc size_t marked, CPtr begin_new_h, CPtr end_new_h, int arity)
{
    CPtr p, q;
    int  tag; 
    Cell contents;

    printf("garbage collection: copying\n");
    gc_offset = heap_bot-begin_new_h;
    gc_scan = gc_next = begin_new_h; 

    xsb_dbgmsg((LOG_GC, 
	       "New heap space between %p and %p", begin_new_h,end_new_h));

  /* the order in which stuff is copied might be important                 */
  /* but like the price of a ticket from Seattle to New York: nobody knows */

  /* trail */
  /* more precise traversal of trail possible */

    { CPtr endtr ;
      endtr = tr_top ;
      for (p = tr_bot; p <= endtr; p++) {
	contents = cell(p);

#ifdef SLG_GC
	if (!tr_marked(p-tr_bot))
	  continue;
	tr_clear_mark(p-tr_bot);
#endif
	q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	if (!q) continue ;
	if (h_marked(q-heap_bot)) 
	  find_and_copy_block(CTXTc q); 
	adapt_external_heap_pointer(p,q,tag);
	}
#ifdef PRE_IMAGE_TRAIL
      /* re-tag pre image cells in trail */
      if (tr_pre_marked(p-tr_bot)) {
	*p = *p | PRE_IMAGE_MARK;
	tr_clear_pre_mark(p-tr_bot);
      }
#endif
    }

  /* choicepoints */
  /* a more precise traversal of choicepoints is possible */

    { CPtr endcp ;
      endcp = cp_top ;
      for (p = cp_bot; p >= endcp ; p--)
	{ contents = cell(p) ;
	  q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	  if (!q) continue ;
	  if (h_marked(q-heap_bot)) { find_and_copy_block(CTXTc q); }
	  adapt_external_heap_pointer(p,q,tag);
	}
    }

  /* local stack */
  /* a more precise traversal of local stack is possible */

    { CPtr endls;
      endls = ls_top ;
      for (p = ls_bot; p >= endls ; p-- )
	{ if (! ls_marked(p-ls_top)) continue ;
          ls_clear_mark((p-ls_top)) ;
	  contents = cell(p) ;
	  q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	  if (!q) continue ;
	  if (h_marked(q-heap_bot)) { find_and_copy_block(CTXTc q); }
	  adapt_external_heap_pointer(p,q,tag);
	}
    }

    /* now do the argument registers */

    { CPtr p;
      for (p = reg+1; arity-- > 0; p++)
        { contents = cell(p) ;
          q = hp_pointer_from_cell(CTXTc contents,&tag) ;
          if (!q) continue ;
          if (h_marked(q-heap_bot)) { find_and_copy_block(CTXTc q); }
          adapt_external_heap_pointer(p,q,tag);
        }
    }

    /* now do the delay register */

    { CPtr p;

      if (delayreg != NULL)
	{ 
	  p = (CPtr)(&delayreg);
	  contents = cell(p) ;
          q = hp_pointer_from_cell(CTXTc contents,&tag) ;
          if (!q)
	    xsb_dbgmsg((LOG_GC, "non null delayreg points not in heap"));
          else
	    {
	      if (h_marked(q-heap_bot)) { find_and_copy_block(CTXTc q); }
	      adapt_external_heap_pointer(p,q,tag);
	    }
        }
    }

    /* now do the trieinstr_unif_stk registers, if nec */

    if (trieinstr_unif_stk && (trieinstr_unif_stkptr >= trieinstr_unif_stk))
      { CPtr p;
	printf("gc_copy: trieinstr_unif_stk adjust\n");
	for (p = trieinstr_unif_stk; p <= trieinstr_unif_stkptr; p++)
	  { contents = cell(p) ;
	    q = hp_pointer_from_cell(CTXTc contents,&tag) ;
	    if (!q) continue ;
	    if (h_marked(q-heap_bot)) { find_and_copy_block(CTXTc q); }
	    adapt_external_heap_pointer(p,q,tag);
	  }
      }

    if (gc_next != end_new_h) { 
      xsb_dbgmsg((LOG_GC, "heap copy gc - inconsistent hreg: %d cells not copied. (num_gc=%d)\n",
		 (end_new_h-gc_next),num_gc));
    }

    memcpy((void *)heap_bot, (void *)begin_new_h, marked*sizeof(Cell));

    return(heap_bot+marked);
} /* copy_heap */

#endif /* GC */

