/* File:      struct_intern.c
** Author(s): Warren
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
** $Id: struct_intern.c,v 1.3 2013-05-06 21:10:25 dwarren Exp $
** 
*/

/* to do: in no particular order
1. automatic expansion of hashtables
2. C program to do interning of full term
3. Garbage collection of interned terms
4. Fix tabling to take advantage of interned terms.

*/


#include "xsb_config.h"
#include "xsb_debug.h"

/* Special debug includes */
#include "debugs/debug_biassert.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "setjmp_xsb.h"
#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "error_xsb.h"
#include "cinterf.h"
#include "memory_xsb.h"
#include "deref.h"
#include "register.h"
#include "heap_xsb.h"
#include "flags_xsb.h"
#include "struct_intern.h"

extern int is_cyclic(CTXTdeclc Cell);
extern int ground(Cell);
extern void printterm(FILE *, Cell, long);
extern int compare(CTXTdeclc const void *, const void *);

/* term must have been dereferenced */
Integer intern_term_size(CTXTdeclc Cell term)
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
    if (isinternstr(term)) {return size;}
    else {
      CPtr pfirstel ;
      pfirstel = clref_val(term) ;
      term = *pfirstel ; 
      XSB_Deref(term) ;
      size += 2 + intern_term_size(CTXTc term) ;
      term = *(pfirstel+1) ; XSB_Deref(term) ;
      goto recur;
    }
  }
  case XSB_STRUCT: {
    if (isinternstr(term)) return size;
    else {
      int a ;
      CPtr pfirstel ;
      pfirstel = (CPtr)cs_val(term) ;
      a = get_arity((Psc)(*pfirstel)) ;
      size += a + 1 ;
      if (a) {  
	while( --a ) {
	  term = *++pfirstel ; 
	  XSB_Deref(term) ;
	  size += intern_term_size( CTXTc term ) ;
	}
      }
      term = *++pfirstel ; XSB_Deref(term) ;
      goto recur;
    }
  }
  case XSB_ATTV:
    return size;
  }
  return FALSE;
}



/* block headers, 1 for each arity; 256 for lists */
#define LIST_INDEX 256
struct hc_block_rec hc_block[257] = { {0, 0, 0} };

UInteger it_hash(Integer ht_size, int reclen, CPtr termrec) {
  UInteger hsh = 0;
  int i;
  // termrec is untagged address!
  for (i = 0; i<reclen; i++) {
    hsh = (hsh << 2*(i % 16)) + cell(termrec+i); 
  }
  //  return (hsh>=0 ? hsh : -hsh) % ht_size;
  return hsh % ht_size;
}

/* must be called with interned term (isinternstr(term)is true) */
int is_interned_rec(Cell term) {
  int areaindex, reclen;
  struct intterm_rec *recptr;
  CPtr term_rec;
  UInteger hashindex; 

  if (islist(term)) {areaindex = LIST_INDEX; reclen = 2; }
  else {areaindex = get_arity(get_str_psc(term)); reclen = areaindex + 1; }
  if (!hc_block[areaindex].base) return FALSE;
  term_rec = (CPtr)cs_val(term);

  hashindex = it_hash(hc_block[areaindex].hashtab_size,reclen,term_rec);
  recptr = hc_block[areaindex].hashtab[hashindex];
  while (recptr) {
    if (term_rec == &(recptr->intterm_psc)) {return TRUE;}
    recptr = recptr->next;
  }
  return FALSE;
}


#define it_hashtab_size 1048573
CPtr insert_interned_rec(int reclen, int areaindex, CPtr termrec) {
  struct intterm_rec *recptr, *prev;
  Integer hashindex; 
  int i, found;
  CPtr hc_term;

  if (!hc_block[areaindex].base) { /* allocate first block */
    hc_block[areaindex].base = 
      mem_calloc(sizeof(Cell),(1+hc_num_in_block*(1+reclen)),OTHER_SPACE); /* for now, make own space*/
    if (!hc_block[areaindex].base) {
      xsb_error("No memory for interned terms\n");
    }
    hc_block[areaindex].hashtab = 0;
    hc_block[areaindex].hashtab_size = 0;
    hc_block[areaindex].freechain = 0;
    hc_block[areaindex].freedisp = &(hc_block[areaindex].base->recs);
  }
  if (!hc_block[areaindex].hashtab) {
    hc_block[areaindex].hashtab = mem_calloc(sizeof(Cell),it_hashtab_size,OTHER_SPACE);
    if (!hc_block[areaindex].hashtab) xsb_abort("No memory for interned terms\n");
    hc_block[areaindex].hashtab_size = it_hashtab_size;
  }

  hashindex = it_hash(hc_block[areaindex].hashtab_size,reclen,termrec);
  prev = recptr = hc_block[areaindex].hashtab[hashindex];
  while (recptr) {
    found = 1;
    hc_term = &(recptr->intterm_psc);
    for (i=0; i<reclen; i++) {
      if (cell(hc_term+i) != cell(termrec+i)) {
	found = 0; break;
      }
    }
    if (found) {/*printf("old %p\n",hc_term);*/ return hc_term;}
    prev = recptr;
    recptr = recptr->next;
  }
  recptr = hc_block[areaindex].freedisp;
  if ((CPtr)recptr >= (CPtr)(&(hc_block[areaindex].base->recs)) + (hc_num_in_block*(1+reclen))) { 
    struct intterm_block *newblock;
    //    printf("oflow: %p\n",recptr);
    newblock = mem_calloc(sizeof(Cell),(1+hc_num_in_block*(1+reclen)),OTHER_SPACE);
    if (!newblock) {
      xsb_error("No memory for interned terms\n");
    }
    newblock->nextblock =  hc_block[areaindex].base;
    hc_block[areaindex].base = newblock;
    hc_block[areaindex].freedisp = &(newblock->recs);
    recptr = &(newblock->recs);
    //    printf("Alloc new block for interned structs: %d, %p-%p\n",reclen,newblock,((char *)newblock)+((1+hc_num_in_block*(1+reclen))*sizeof(Cell)));
  }
  hc_block[areaindex].freedisp = (struct intterm_rec *)((CPtr)(hc_block[areaindex].freedisp) + reclen+1);
  if (prev) prev->next = recptr; else hc_block[areaindex].hashtab[hashindex] = recptr;
  recptr->next = 0;
  hc_term = &(recptr->intterm_psc);
  for (i=0; i<reclen; i++) {
    cell(hc_term+i) = cell(termrec+i);
  }
  /*printf("new %p\n",hc_term);*/
  return hc_term;
}

/* should be passed a term which is dereffed for which isinternstr is true! */
int isinternstr_really(prolog_term term) {
  int areaindex, reclen, i;
  CPtr termrec;
  CPtr hc_term;
  struct intterm_rec *recptr;
  Integer hashindex; 
  int found;
  
  XSB_Deref(term);
  if (isconstr(term)) {
    areaindex = get_arity(get_str_psc(term)); 
    reclen = areaindex + 1;
  } else if (islist(term)) {
    areaindex = LIST_INDEX; 
    reclen = 2;
  } else return FALSE;
  if (!hc_block[areaindex].hashtab) return FALSE;
  termrec = (CPtr)dec_addr(term);
  hashindex = it_hash(hc_block[areaindex].hashtab_size,reclen,termrec);
  recptr = hc_block[areaindex].hashtab[hashindex];
  while (recptr) {
    found = 1;
    hc_term = &(recptr->intterm_psc);
    for (i=0; i<reclen; i++) {
      if (cell(hc_term+i) != cell(termrec+i)) {
	found = 0; break;
      }
    }
    //    if (found && (hc_term == termrec)) printf("found interned term\n");
    if (found) return (hc_term == termrec);
    recptr = recptr->next;
  }
  return FALSE;

 
}

/* intern_rec takes a reference to struct record with all subfields
   pointing either to atoms, integers, or other interned term records,
   and returns a str-tagged pointer to the interned version of that
   struct record.  It returns 0 if the struct record isn't of this
   form */

prolog_term intern_rec(CTXTdeclc prolog_term term) {

  int areaindex, reclen, i, j;
  CPtr hc_term;
  Cell dterm[255];
  Cell arg;

  //  printf("intern_rec\n");
  // create term-record with all fields dereffed in dterm
  XSB_Deref(term);
  if (isinternstr(term)) {printf("old\n"); return term;}
  if (isconstr(term)) {
    areaindex = get_arity(get_str_psc(term)); 
    reclen = areaindex + 1;
    cell(dterm) = (Cell)get_str_psc(term); // copy psc ptr
    j=1;
  } else if (islist(term)) {
    areaindex = LIST_INDEX; 
    reclen = 2;
    j=0;
  } else return 0;
  for (i=j; i<reclen; i++) {
    arg = get_str_arg(term,i);  // works for lists and strs
    XSB_Deref(arg);
    if (isref(arg) || (isstr(arg) && !isinternstr(arg)) || isattv(arg)) {
      return 0;
    }
    cell(dterm+i) = arg;
  }
  hc_term = insert_interned_rec(reclen, areaindex, dterm);
  if (islist(term)) return makelist(hc_term); else return makecs(hc_term);
}


struct term_subterm *ts_array = 0;
Integer ts_array_len = 0;
// make bigger???
#define init_ts_array_len 5000  
#define check_ts_array_overflow \
  if (ti >= ts_array_len) {						\
    ts_array = mem_realloc(ts_array,ts_array_len*sizeof(*ts_array),	\
			   ts_array_len*2*sizeof(*ts_array),OTHER_SPACE); \
    ts_array_len = ts_array_len*2;					\
    /*    printf("expanded ts_array: %p, %d\n",ts_array,ts_array_len);*/ \
  }


/* caller must ensure enough heap space (term_size(term)*sizeof(Cell)) */
prolog_term intern_term(CTXTdeclc prolog_term term) {
  Integer ti = 0;
  Cell arg, newterm, interned_term, orig_term;
  int subterm_index;

  XSB_Deref(term);
  if (!(islist(term) || isconstr(term))) {return term;}
  if (isinternstr(term)) {return term;}
  if (is_cyclic(CTXTc term)) {xsb_abort("Cannot intern a cyclic term\n");}
  //  if (!ground(term)) {return term;}

  orig_term = term;
  //  printf("iti: ");printterm(stdout,orig_term,100);printf("\n");

  if (!ts_array) {
    ts_array = mem_alloc(init_ts_array_len*sizeof(*ts_array),OTHER_SPACE);
    if (!ts_array) xsb_abort("No space for interning term\n");
    ts_array_len = init_ts_array_len;
  }
  
  ts_array[0].term = term;
  if (islist(term)) {
    ts_array[0].subterm_index = 0;
    ts_array[0].newterm = makelist(hreg);
    hreg += 2;
  }
  else {
    //    if (isboxedinteger(term)) printf("interning boxed int\n");
    //    else if (isboxedfloat(term)) printf("interning boxed float %f\n",boxedfloat_val(term));
    ts_array[0].subterm_index = 1;
    ts_array[0].newterm = makecs(hreg);
    new_heap_functor(hreg, get_str_psc(term));
    hreg += get_arity(get_str_psc(term));
  }
  ts_array[ti].ground = 1;

  while (ti >= 0) {
    term = ts_array[ti].term;
    newterm = ts_array[ti].newterm;
    subterm_index = ts_array[ti].subterm_index;
    if ((islist(term) && subterm_index >= 2) ||
	(isconstr(term) && subterm_index > get_arity(get_str_psc(term)))) {
      if (ts_array[ti].ground) {
	interned_term = intern_rec(CTXTc newterm);
	if (!interned_term) xsb_abort("error term should have been interned\n");
	hreg = clref_val(newterm);  // reclaim used stack space
	if (!ti) {
	  if (compare(CTXTc (void*)orig_term,(void*)interned_term) != 0) printf("NOT SAME\n");
	  //printf("itg: ");printterm(stdout,interned_term,100);printf("\n"); 
	  return interned_term;
	}
	ti--;
	get_str_arg(ts_array[ti].newterm,ts_array[ti].subterm_index-1) = interned_term;
      } else {
	//printf("hreg = %p, ti=%d\n",hreg,ti);
	if (!ti) {
	  if (compare(CTXTc (void*)orig_term,(void*)newterm) != 0) printf("NOT SAME\n");
	  //printf("ito: ");printterm(stdout,newterm,100);printf("\n"); 
	  return newterm;
	}
	ti--;
	get_str_arg(ts_array[ti].newterm,ts_array[ti].subterm_index-1) = newterm;
	ts_array[ti].ground = 0;
      }
    } else {
      arg = get_str_arg(term, (ts_array[ti].subterm_index)++);
      XSB_Deref(arg);
      switch (cell_tag(arg)) {
      case XSB_FREE:
      case XSB_REF1:
      case XSB_ATTV:
	ts_array[ti].ground = 0;
	get_str_arg(newterm,subterm_index) = arg;
	break;
      case XSB_STRING:
	if (string_find_safe(string_val(arg)) != string_val(arg)) printf("uninterned string?\n");
      case XSB_INT:
      case XSB_FLOAT:
	get_str_arg(newterm,subterm_index) = arg;
	break;
      case XSB_LIST:
	if (isinternstr(arg)) get_str_arg(newterm,subterm_index) = arg;
	else {
	  ti++;
	  check_ts_array_overflow;
	  ts_array[ti].term = arg;
	  ts_array[ti].subterm_index = 0;
	  ts_array[ti].ground = 1;
	  ts_array[ti].newterm = makelist(hreg);
	  hreg += 2;
	}
	break;
      case XSB_STRUCT:
	if (isinternstr(arg)) get_str_arg(newterm,subterm_index) = arg;
	else {
	  //	  if (isboxedinteger(arg)) printf("interning boxed int\n");
	  //	  else if (isboxedfloat(arg)) printf("interning boxed float %f\n",boxedfloat_val(arg));
	  ti++;
	  check_ts_array_overflow;
	  ts_array[ti].term = arg;
	  ts_array[ti].subterm_index = 1;
	  ts_array[ti].ground = 1;
	  ts_array[ti].newterm = makecs(hreg);
	  new_heap_functor(hreg,get_str_psc(arg));
	  hreg += get_arity(get_str_psc(arg));
	}
      }
    }
  }
  printf("intern_term: shouldn't happen\n");
  return 0;
}
