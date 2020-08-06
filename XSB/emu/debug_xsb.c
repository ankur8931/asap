/* File:      debug_xsb.c
** Author(s): Xu, Swift, Sagonas, Johnson, Freire
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
** $Id: debug_xsb.c,v 1.107 2013-05-06 21:10:24 dwarren Exp $
**
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "binding.h"
#include "psc_xsb.h"
#include "memory_xsb.h"
#include "flags_xsb.h"
#include "register.h"
#include "deref.h"
#include "trie_internals.h"
#include "choice.h"
#include "tab_structs.h"
#include "inst_xsb.h"
#include "debug_xsb.h"
#include "varstring_xsb.h"
#include "cinterf.h"
#include "io_defs_xsb.h"
#include "io_builtins_xsb.h"
#include "thread_defs_xsb.h"
#include "thread_xsb.h"
#include "tr_utils.h"
#include "tst_utils.h"
#include "cell_xsb_i.h"

#if (defined(DEBUG_VERBOSE) || defined(DEBUG_VM))
#include "subp.h"
#endif

void print_subgoal(CTXTdeclc FILE *, VariantSF);
#define virtual_buffer (fl_buf->fl_buffer)

/*=============================================================================*/
/*  The first section of predicates are used for tracing as well as by XSB     */
/*  developers during debugging.  They should always be defined		       */
/*=============================================================================*/

#define CAR		1
#define CDR		0

#define MAXFLOATLEN 24
#define MAXINTLEN 21

void print_term(FILE *fp, Cell term, byte car, long level)
{
  unsigned short i, arity;
  Psc psc;
  CPtr cptr;

  level--;
  if (level < 0) {
    fprintf(fp, "'...'");
    return;
  }
  printderef(term);
  switch (cell_tag(term)) {
  case XSB_FREE:
  case XSB_REF1:
    fprintf(fp, "_%p", vptr(term));
    return;
  case XSB_ATTV:
    fprintf(fp, "_%p {'...'}", (CPtr)dec_addr(term));
    return;
  case XSB_STRUCT:
      //NOTE: Below is a check for boxed numbers. If such is the case, then
      //  the behavior is the same as XSB_INT or XSB_FLOAT, but with boxed[X]_val macro
      //  instead of [X]_val macro
    if (isboxedfloat(term)) {
        fprintf(fp, "%f", boxedfloat_val(term));
        return;
    }
    else if (isboxedinteger(term)) {
        fprintf(fp, "%" Intfmt, (Integer)boxedint_val(term));
        return;
    }
    psc = get_str_psc(term);
    if (quotes_are_needed(get_name(psc))) {
      fprintf(fp, "'%s'", get_name(psc));
    } else {
      fprintf(fp, "%s", get_name(psc));
    }
    arity = get_arity(psc);
    if ( arity == 0 )   /* constant */
      return;
    /* structure */
    fprintf(fp, "(");
    cptr = clref_val(term);
    for ( i = 1; i <= arity; i++ ) {
      print_term(fp, cell(cptr+i), CAR, level);
      if ( i < arity )
	fprintf(fp, ",");
    }
    fprintf(fp, ")");
    return;
  case XSB_STRING:
    if (quotes_are_needed(string_val(term))) {
      fprintf(fp, "'%s'", string_val(term));
    } else {
      fprintf(fp, "%s", string_val(term));
    }
    break;
  case XSB_INT:
    fprintf(fp, "%" Intfmt, (Integer)int_val(term));
    return;
  case XSB_FLOAT:
    fprintf(fp, "%f", float_val(term));
    fprintf(fp, "%f", ofloat_val(term));
    return;
  case XSB_LIST:
    cptr = clref_val(term);
    if ( car )
      fprintf(fp, "[");
    print_term(fp, cell(cptr), CAR, level);
    term = cell(cptr+1);
    XSB_Deref(term);
    switch (cell_tag(term)) {
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      goto vertbar;
    case XSB_LIST:
      fprintf(fp, ",");
      print_term(fp, term, CDR, level);
      return;
    case XSB_STRING:
      if (string_val(term) != nil_string)
	goto vertbar;
      else {
	fprintf(fp, "]");
	return;
      }
    case XSB_STRUCT:
    case XSB_INT:
    case XSB_FLOAT:
    vertbar:
    fprintf(fp, "|");
    print_term(fp, term, CAR, level);
    fprintf(fp, "]");
    return;
    }
  }
}

int sprint_quotedname(char *buffer, int size,char *string)
{
  int dlength = 0;

  if (*string == '\0')  {
   sprintf(buffer+size,"''");
   return size + 2;
  }
  else {
    if (!quotes_are_needed(string)) {
      return size + snprintf(buffer+size,MAXTERMBUFSIZE-size-1, "%s", string);
      //      return size + sprintf(buffer+size,"%s", string);
    }
    else {
      if (MAXTERMBUFSIZE - (size + 2*strlen(string) + 2) > 0) {
	sprintf(buffer+size,"\'");
	dlength = double_quotes(string,buffer+size+1);
	sprintf(buffer+size+ dlength + 1,"\'");
	return size + dlength + 2;
      }
      else {
	//	printf(" quoted name too long!\n");
	return size;
      }
    }
  }
}

static inline int get_int_print_width(Integer num) {
  if (abs((int)num) < 100) return 2;
  else if (abs((int)num) < 10000) return 4;
  else return MAXINTLEN;
}

#ifndef MULTI_THREAD
extern CTptr cycle_trail;
extern int cycle_trail_size;
extern  int cycle_trail_top;
#endif

void print_cell_tag(Cell Term) {
  printf("Cell tag for %p is: ",(CPtr) Term);
  switch (cell_tag(Term)) {
  case XSB_STRUCT: printf("structure (%p %s/%d) \n",clref_val(Term),get_name(get_str_psc(Term)),
			  get_arity(get_str_psc(Term))); break;
  case XSB_INT: printf("integer (%d)\n",(int) int_val(Term));break;
  case XSB_LIST: printf("list (%p)\n",clref_val(Term));break;
  case XSB_STRING: printf("atom (%p/%s)\n",string_val(Term),string_val(Term));break;
  case XSB_FLOAT: printf("atom (%f)\n",float_val(Term));break;
  case XSB_ATTV: printf("attvar (%p)\n",clref_val(Term));break;
  default: printf("variable\n");
  }
}

typedef struct CycleTrailFrame_2 {
  CPtr cell_addr;
  Cell value;
} Cycle_Trail_Frame_2 ;

typedef Cycle_Trail_Frame_2 *CTptr_2;

int cycle_trail_size_2 = 10;
int cycle_trail_top_2 = -1;
CTptr_2 cycle_trail_2;

/* We want to untrail everything except '<cyclic>', which will be untrailed when we backtrack */
#define pop_cycle_trail_keep_mark(Term) {				\
    /*    printf("ctt %d %p @%p\n",cycle_trail_top,cycle_trail[cycle_trail_top].cell_addr,*/ \
    /*	   *(CPtr) cycle_trail[cycle_trail_top].cell_addr);		*/ \
    while (cycle_trail_top >= 0 \
	   && * (CPtr) cycle_trail[cycle_trail_top].cell_addr == (Cell) makestring(cyclic_string)) { \
      cycle_trail_top--;						\
    }									\
    if (cycle_trail_top >= 0) {					\
      /*    printf("     plain pop addr %p: %p set to %p\n",cycle_trail[cycle_trail_top].cell_addr,*/ \
      /*           *cycle_trail[cycle_trail_top].cell_addr, cycle_trail[cycle_trail_top].value); */ \
      * (CPtr) cycle_trail[cycle_trail_top].cell_addr = cycle_trail[cycle_trail_top].value; \
      Term = cycle_trail[cycle_trail_top].parent;			\
      cycle_trail_top--;						\
    }									\
  }

#define push_cycle_trail_2(Addr,Val) {		\
    ++cycle_trail_top_2;			\
    printf("trailing value of %p as  %p\n",Addr,Val);	\
    cycle_trail_2[cycle_trail_top_2].cell_addr = Addr;	\
    cycle_trail_2[cycle_trail_top_2].value = Val;	 \
}

#define unwind_cycle_trail_2 {						\
    while (cycle_trail_top_2 >= 0) {					\
      if (clref_val(*cycle_trail_2[cycle_trail_top_2].cell_addr) != (Cell) cyclic_string) { \
	printf("untrailing value %p of %p to %p\n",*cycle_trail_2[cycle_trail_top_2].cell_addr, \
	       cycle_trail_2[cycle_trail_top_2].cell_addr,cycle_trail_2[cycle_trail_top_2].value); \
	* (CPtr) cycle_trail_2[cycle_trail_top_2].cell_addr = cycle_trail_2[cycle_trail_top_2].value; \
      }									\
      cycle_trail_top_2--;						\
    }									\
  }

/* mark_cyclic() traverses a term, eewriting all cyclic elements using
   the string '<cyclic>' and otherwise not affecting the term.  The
   destructive assignment is pre-image trailed, so you get the cyclic
   term back upon backtracing.

   The use of visited_string or visited_psc can probably be
   simplified, I kept the variants in for debugging - visited psc has
   the same type as the PSC of a non-list structure.

*/

#define mark_as_visited(Term) {					   \
    if (cell_tag(Term) == XSB_STRUCT)				   \
      *clref_val(Term) = (Cell) visited_psc;			   \
    else *clref_val(Term) = (Cell) visited_string;		   \
  }

#define has_been_visited(Term)	(*clref_val(Term) == (Cell) visited_string \
				 || *clref_val(Term) == (Cell) visited_psc)

#define is_marked_cyclic(Term) (*(CPtr)(Term) == makestring(cyclic_string))
#define is_marked_cyclic_clref(Term) (*clref_val(Term) == makestring(cyclic_string))

#define mark_with_cyclic_string(preTerm) {				\
  if (cell_tag(preTerm) == XSB_LIST || cell_tag(preTerm) == XSB_STRUCT) \
    *clref_val(preTerm) = makestring(cyclic_string);			\
  else									\
    *(CPtr)preTerm = makestring(cyclic_string);				\
  }

void mark_cyclic(CTXTdeclc Cell Term) {
  Cell preTerm, visited_string;

  cycle_trail_top = -1;

  XSB_Deref(Term);
  if (cell_tag(Term) != XSB_LIST && cell_tag(Term) != XSB_STRUCT) return;

  if (cycle_trail == (CTptr) 0){
    cycle_trail_size = TERM_TRAVERSAL_STACK_INIT;
    cycle_trail = (CTptr) mem_alloc(cycle_trail_size*sizeof(Cycle_Trail_Frame),OTHER_SPACE);
    }

  //  printf("visited %p %s\n",visited_psc,get_name(visited_psc));
  //  printf("cyclic_string %p\n",cyclic_string);
  visited_string = makestring(string_find("_$visited",1));

  push_cycle_trail(Term);
  //  printf("Term starting %p\n",Term);print_cell_tag(Term);
  mark_as_visited(Term);
  //  printf("Term starting set to %p\n",Term);print_cell_tag(Term);

  while (cycle_trail_top >= 0) {
    //    printf("--------\n");
    if (cycle_trail[cycle_trail_top].arg_num > cycle_trail[cycle_trail_top].arity) {
      pop_cycle_trail_keep_mark(Term);
      continue;
    }
    else {
      if (cycle_trail[cycle_trail_top].arg_num == 0) {
	preTerm = (Cell) cycle_trail[cycle_trail_top].cell_addr;
	Term = cycle_trail[cycle_trail_top].value;
	//	printf("getting car %p @%p\n",cycle_trail[cycle_trail_top].cell_addr,
	//	       cycle_trail[cycle_trail_top].value);
	if (!(cell_tag(Term) == XSB_LIST || cell_tag(Term) == XSB_STRUCT)) {
	  cycle_trail[cycle_trail_top].arg_num++;
	  continue;
	}
      }
      else {
	//	printf("examining list/struct %p %d\n",
	//	       clref_val(cycle_trail[cycle_trail_top].parent),
	//	       cycle_trail[cycle_trail_top].arg_num);
	preTerm = Term = (Cell) (clref_val(cycle_trail[cycle_trail_top].parent)
				 + cycle_trail[cycle_trail_top].arg_num);
      }
    }
    cycle_trail[cycle_trail_top].arg_num++;
    //    printf("Term/preTerm before %p\n",Term);
    if (!is_marked_cyclic(preTerm)) {
      XSB_Deref(Term);
      //      printf("Term after %p\n",Term);print_cell_tag(Term);
      if (cell_tag(Term) == XSB_LIST || cell_tag(Term) == XSB_STRUCT) {
	if (is_marked_cyclic_clref(Term) && !is_marked_cyclic(preTerm)) {
	    push_pre_image_trail0((CPtr) preTerm, makestring(cyclic_string));
	    *clref_val(preTerm) = makestring(cyclic_string);
	    //	    printf("marking %p @%p as cyclic %p\n",preTerm,*(CPtr)preTerm,
	    //		   makestring(cyclic_string));
	}
	else if (!is_marked_cyclic_clref(Term)) {
	  if (!has_been_visited(Term)) {
	    //	    printf("setting visited string for struct\n");
	    push_cycle_trail(Term);
	    mark_as_visited(Term);
	    //	    printf("marking %p as visited\n",clref_val(Term));
	  }
	  else {
	    push_pre_image_trail0((CPtr) preTerm, makestring(cyclic_string));
	    mark_with_cyclic_string(preTerm);
	    //	    printf("marking %p @%p as cyclic %p\n",preTerm,*(CPtr)preTerm,
	    //   makestring(cyclic_string));
	  }
	}
	else printf("term is cyclic\n");
      }
    }
      else printf("preterm is cyclic\n");
  }
  return;
  }

static int sprint_term(forestLogBuffer fl_buf, int insize, Cell term, byte car, long level)
{
  unsigned short i, arity;
  Psc psc;
  CPtr cptr;
  int size = insize;

  //  if (size > MAXTERMBUFSIZE) return size;
  maybe_realloc_buffers(fl_buf,size);
  //  char * buffer = fl_buf->fl_buffer;

  level--;
  if (level < 0) {
    if (car == CAR)
      return size + sprintf(virtual_buffer+size, "'...'");
    else       return size + sprintf(virtual_buffer+size, "'...']");
  }
  printderef(term);
  switch (cell_tag(term)) {
  case XSB_FREE:
  case XSB_REF1:
    return size+sprintf(virtual_buffer+size, "_v%p",vptr(term));
  case XSB_ATTV:
    if (flags[WRITE_ATTRIBUTES] == WA_DOTS) {
      return size + sprintf(virtual_buffer+size, "_attv%p {'...'} ", (CPtr)dec_addr(term));
      //      return size+15+2*sizeof(CPtr);
    }
    else {
      return size + sprintf(virtual_buffer+size, "_attv%p ", (CPtr)dec_addr(term));
      //      return size+9+2*sizeof(CPtr);
    }
  case XSB_STRUCT:
    /* NOTE: Below is a check for boxed numbers. If such is the case,
       then the behavior is the same as XSB_INT or XSB_FLOAT, but with
       boxed[X]_val macro instead of [X]_val macro.  Also note, that
       in order to attain a predictable width for the sprintf I
       converted things to decimal.  Perhaps there's some way of
       printing out the integer part of float in a fixed width, but I
       don't know how.
    */
    if (isboxedfloat(term)) {
      Float val = boxedfloat_val(term);
      //      int width = get_int_print_width((Integer)floor(val));
      return size + sprintf(virtual_buffer+size, "%" Intfmt ".%4d",(Integer)floor(val),(int)((val-floor(val))*10000));
    }
    else if (isboxedinteger(term)) {
      Integer val = boxedint_val(term);
      int width = get_int_print_width(val);
      return size + sprintf(virtual_buffer+size, "%*" Intfmt, width, (Integer)boxedint_val(term));
      //      return size+width;
    }
    psc = get_str_psc(term);
    size = sprint_quotedname(virtual_buffer, size, get_name(psc));
    arity = get_arity(psc);
    if ( arity == 0 )   /* constant */
      return size;
    /* structure */
    sprintf(virtual_buffer+size, "(");size++;
    cptr = clref_val(term);
    for ( i = 1; i <= arity; i++ ) {
      size = sprint_term(fl_buf, size,cell(cptr+i), CAR, level);
      if ( i < arity ) {
	sprintf(virtual_buffer+size, ",");size++;
      }
    }
    sprintf(virtual_buffer+size, ")");size++;
    return size;
  case XSB_STRING:
    return sprint_quotedname(virtual_buffer, size, string_val(term));
  case XSB_INT: {
    //    int width = get_int_print_width((Integer)int_val(term));
    //    return size + sprintf(virtual_buffer+size, "%*" Intfmt, width, (Integer)int_val(term));
    return size + sprintf(virtual_buffer+size, "%" Intfmt, (Integer)int_val(term));
  }
  case XSB_FLOAT:
    printf("here\n");
    size = size + sprintf(virtual_buffer+size, "%f", float_val(term));
    return size + sprintf(virtual_buffer+size, "%f", ofloat_val(term));
    //    return size+MAXFLOATLEN;
  case XSB_LIST:
    cptr = clref_val(term);
    if ( car ) {
      sprintf(virtual_buffer+size, "[");size++;
    }
    size = sprint_term(fl_buf, size,cell(cptr), CAR, level);
    term = cell(cptr+1);
    XSB_Deref(term);
    switch (cell_tag(term)) {
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      goto vertbar;
    case XSB_LIST:
      sprintf(virtual_buffer+size, ",");size++;
      return sprint_term(fl_buf, size,term, CDR, level);
    case XSB_STRING:
      if (string_val(term) != nil_string)
	goto vertbar;
      else {
	sprintf(virtual_buffer+size, "]");size++;
	return size;
      }
    case XSB_STRUCT:
    case XSB_INT:
    case XSB_FLOAT:
    vertbar:
      sprintf(virtual_buffer+size, "|");size++;
      size = sprint_term(fl_buf, size,term, CAR, level);
      sprintf(virtual_buffer+size, "]");size++;
      return size;
    }
  }
  return size;  // to quiet compiler.
}

static int sprint_termsize(Cell term, byte car, int level)
{
  unsigned short i, arity;
  Psc psc;
  CPtr cptr;
  int size = 0;

  level--;
  if (level < 0) {
    return size+3;
  }
  printderef(term);
  switch (cell_tag(term)) {
  case XSB_FREE:
  case XSB_REF1:
    return size+10;
  case XSB_ATTV:
    return size+15;
  case XSB_STRUCT:
      //NOTE: Below is a check for boxed numbers. If such is the case, then
      //  the behavior is the same as XSB_INT or XSB_FLOAT, but with boxed[X]_val macro
      //  instead of [X]_val macro
    if (isboxedfloat(term)) {
        return size+10;
    }
    else if (isboxedinteger(term)) {
        return size+10;
    }
    psc = get_str_psc(term);
    arity = get_arity(psc);
    size = size + (int)strlen(get_name(psc)) + (arity-1) +2;
    if ( arity == 0 )   /* constant */
      return size;
    /* structure */
    cptr = clref_val(term);
    for ( i = 1; i <= arity; i++ ) {
      size = size + sprint_termsize(cell(cptr+i), CAR, level);
    }
    return size;
  case XSB_STRING:
    size = size+(int)strlen(string_val(term));
    break;
  case XSB_INT:
    return size+10;
  case XSB_FLOAT:
    return size+10;
  case XSB_LIST:
    cptr = clref_val(term);
    size = size + sprint_termsize(cell(cptr), CAR, level) + 2;
    term = cell(cptr+1);
    XSB_Deref(term);
    switch (cell_tag(term)) {
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      goto vertbar;
    case XSB_LIST:
      return size+sprint_termsize( term, CDR, level);;
    case XSB_STRING:
      if (string_val(term) != nil_string)
	goto vertbar;
      else {
	return size;
      }
    case XSB_STRUCT:
    case XSB_INT:
    case XSB_FLOAT:
    vertbar:
    return size + sprint_termsize( term, CAR, level);
    }
  }
  return size;  // to quiet compiler.
}

void printterm(FILE *fp, Cell term, long depth) {

  print_term(fp, term, CAR, depth);
  fflush(fp);
}

void printCyclicTerm(CTXTdeclc Cell term) {
  mark_cyclic(CTXTc term);
  printterm(stddbg, term, (long)flags[WRITE_DEPTH]);
}

void sprintCyclicTerm(CTXTdeclc forestLogBuffer fl_buf, Cell Term) {
  mark_cyclic(CTXTc Term);
  sprint_term(fl_buf, 0, Term, CAR, (long)flags[WRITE_DEPTH]);
  unwind_cycle_trail;
}

int sprintTerm(forestLogBuffer fl_buf, Cell Term) {
  return sprint_term(fl_buf, 0, Term, CAR, (long)flags[WRITE_DEPTH]);
}

static int sprint_cyclic_term_nonvoid(CTXTdeclc forestLogBuffer fl_buf, int size, Cell Term,long depth) {
  int newsize;
  mark_cyclic(CTXTc Term);
  newsize = sprint_term(fl_buf, size, Term, CAR, depth);
  unwind_cycle_trail;
  return newsize;
}

/*------------------------------------------------------------------*/
/* Used to print out call using WAM registers -- print registers*/

void sprint_callsize(CTXTdeclc Psc psc,int depth) {
  int i, arity,size;

  size = (int)strlen(get_name(psc));
  arity = (int)get_arity(psc);
  size = size + (arity-1) + 2;

  for (i=1; i <= arity; i++) {
    size = size+sprint_termsize(cell(reg+i), CAR, depth);
  }
  printf("size %d\n",size);

}

/* cf. tst_utils.c */
/* uses MAXBUFSIZE */
void sprint_answer_template(CTXTdeclc forestLogBuffer fl_buf, CPtr pAnsTmplt, int template_size,long depth) {

  int i,size;
  char * buffer = fl_buf->fl_buffer;

  sprintf(buffer,"["); size = 1;
  if (template_size > 0) {
    for (i = 1; i < template_size; i++) {
      //      sprint_term(fp, *pAnsTmplt--, CAR,depth);
      size = sprint_term(fl_buf, size, *pAnsTmplt--, CAR,depth);
      sprintf(buffer+size, ",");size++;
    }
    size = sprint_term(fl_buf, size, *pAnsTmplt, CAR,depth);
  }
  sprintf(buffer+size,"]");
}

void sprintAnswerTemplate(CTXTdeclc forestLogBuffer fl_buf, CPtr pAnsTmplt, int template_size) {
  sprint_answer_template(CTXTc  fl_buf, pAnsTmplt, template_size,(long)flags[WRITE_DEPTH]);
}

//-----------
extern xsbBool is_cyclic(CTXTdeclc Cell);

xsbBool cyclic_registers(CTXTdeclc Psc psc) {
  int i, arity;

  arity = (int)get_arity(psc);
  if (arity == 0) return FALSE;
  for (i = 1; i <= arity; i++) {
    if (is_cyclic(CTXTc cell(reg+i))) return TRUE;
  }
  return FALSE;
 }

//-----------
/* Should usually be set to MAXTERMBUFSIZE */

static void sprint_registers(CTXTdeclc forestLogBuffer fl_buf,Psc psc,long depth) {
  int i, arity,size;
  char * buffer = fl_buf->fl_buffer;

  arity = (int)get_arity(psc);

  size = sprint_quotedname(buffer, 0, get_name(psc));
  if (arity != 0) sprintf(buffer+size, "(");size++;
  for (i=1; i <= arity; i++) {
    size = sprint_term(fl_buf, size, cell(reg+i), CAR, depth-1);
    if (i < arity) {sprintf(buffer+size, ",");size++;}
  }
  //  printf("grand return %d %s\n",size,buffer);
  if (arity != 0) sprintf(buffer+size, ")");
}

void sprintNonCyclicRegisters(CTXTdeclc forestLogBuffer fl_buf,Psc psc) {
  sprint_registers(CTXTc fl_buf,psc,(long)flags[WRITE_DEPTH]);
}


/* Should usually be set to MAXTERMBUFSIZE */
static void sprint_cyclic_registers(CTXTdeclc forestLogBuffer fl_buf,Psc psc,long depth) {
  int i, arity,size;
  char * buffer = fl_buf->fl_buffer;
  arity = (int)get_arity(psc);

  size = sprint_quotedname(buffer, 0, get_name(psc));
  if (arity != 0) sprintf(buffer+size, "(");size++;
  for (i=1; i <= arity; i++) {
    size = sprint_cyclic_term_nonvoid(CTXTc fl_buf, size,  cell(reg+i), depth-1);
    if (i < arity) {sprintf(buffer+size, ",");size++;}
  }
  //  printf("grand return %d %s\n",size,buffer);
  if (arity != 0) sprintf(buffer+size, ")");
}

void sprintCyclicRegisters(CTXTdeclc forestLogBuffer fl_buf,Psc psc) {
  sprint_cyclic_registers(CTXTc fl_buf,psc,(long) flags[WRITE_DEPTH]);
}

void print_registers(CTXTdeclc FILE *fp, Psc psc,long depth) {
  int i, arity;

  arity = (int)get_arity(psc);
  fprintf(fp, "%s", get_name(psc));
  if (arity != 0) fprintf(fp, "(");
  for (i=1; i <= arity; i++) {
    printterm(fp, cell(reg+i), depth);
    //    fflush(fp);
    if (i < arity) fprintf(fp, ",");
  }
  //  if (arity != 0) fprintf(fp, ")\n"); else fprintf(fp, "\n");
  if (arity != 0) fprintf(fp, ")");
  //  fflush(fp);
}

void print_n_registers(CTXTdeclc FILE *fp, int arity , long depth) {
  int i;

  if (arity != 0) fprintf(fp, "(");
  for (i=1; i <= arity; i++) {
    printterm(fp, cell(reg+i), depth);
    //    fflush(fp);
    if (i < arity) fprintf(fp, ",");
  }
  //  if (arity != 0) fprintf(fp, ")\n"); else fprintf(fp, "\n");
  if (arity != 0) fprintf(fp, ")");
  //  fflush(fp);
}

/*------------------------------------------------------------------*/
/* These variables are global, so in principle, you could run the
   instruction debugger with multiple active threads.  It hasn't been
   tested out, however. */

#ifdef DEBUG_VM
static void debug_interact(CTXTdecl);
#endif

int call_step_gl = 0;
int hitrace_suspend_gl = 0;

void debug_call(CTXTdeclc Psc psc)
{
  if (call_step_gl || get_spy(psc)) {
    fprintf(stddbg, "(w1) call: "); print_registers(CTXTc stddbg,psc,3);
    fprintf(stddbg,"\n");
#ifdef DEBUG_VM
    debug_interact(CTXT);
#endif
  } else if (!hitrace_suspend_gl) {
    fprintf(stddbg, "(w1) call: ");    print_registers(CTXTc stddbg,psc,3);
    fprintf(stddbg,"\n");
  }
}

/*=============================================================================*/
/*  The second section of predicates I (TLS) use when debugging with gdb.      */
/*  Please ensure they stay defined whenever we compile with -dbg option.      */
/*=============================================================================*/

/*----------------------------------------------------------------------*/
/* This set of routines prints out the CP stack with stuff I happen to
   want.  It has different information than print_cp() in gc_print.h, so
   please keep it around. */

#ifdef CP_DEBUG
void print_cpf_pred(CPtr cpf)
{
  Psc psc;

  psc = cp_psc(cpf);
  if (psc) {
    switch(get_type(psc)) {
    case T_PRED:
      fprintf(stddbg,"choicepoint(address(%p),pred(%s/%d)).\n",
	      cpf, get_name(psc), get_arity(psc));
      break;
    case T_DYNA:
      fprintf(stddbg,"choicepoint(address(%p),dyna_pred(%s/%d)).\n",
	      cpf, get_name(psc), get_arity(psc));
      break;
    case T_ORDI:
      fprintf(stddbg,"choicepoint(address(%p),t_ordi).\n",
	      cpf);
      break;
    case T_UDEF:
      fprintf(stddbg,"choicepoint(address(%p),unloaded(%s/%d)).\n",
	      cpf, get_name(psc), get_arity(psc));
      break;
    default:
      fprintf(stddbg,"choicepoint(address(%p),unknown_pred).\n", cpf);
      break;
    }
  } else
    fprintf(stddbg,"choicepoint(address(%p),unknown_psc).\n", cpf);

}

void print_cp_backtrace()
{
  CPtr mycp;
  mycp = (CPtr) breg;
  while (mycp <= tcpstack.high - CP_SIZE -1 && mycp != (CPtr) cp_prevbreg(mycp)) {
    print_cpf_pred(mycp);
    mycp = cp_prevbreg(mycp);
  }
}

void alt_print_cpf_pred(CPtr cpf,FILE* where)
{
  Psc psc;

  psc = * (Psc *)cpf;
  if (psc) {
    switch(get_type(psc)) {
    case T_PRED:
      fprintf(where,"   CP stack %p\t Static Predicate: \t%s/%d\n",
	      cpf, get_name(psc), get_arity(psc));
      break;
    case T_DYNA:
      fprintf(where,"   CP stack %p\t Dyna Predicate: \t%s/%d\n",
	      cpf, get_name(psc), get_arity(psc));
      break;
    case T_ORDI:
      fprintf(where,"CP stack %p\t ORDI Predicate: \t\n",
	      cpf);
      break;
    case T_UDEF:
      fprintf(where,"CP stack %p\t UNDEF Predicate: \t\n",
	      cpf);
      break;
    default:
      fprintf(where,"choicepoint(address(%p),unknown_pred).\n", cpf);
      break;
    }
  } else
    fprintf(where,"choicepoint(address(%p),unknown_psc).\n", cpf);
}

#endif /* CP_DEBUG */

/*-------------------------------------------*/

static void print_common_cpf_part(CPtr cpf_addr, FILE* where) {

  fprintf(where,"   CP stack %p:\tptr to next clause:\t%p\n",
	     &(cp_pcreg(cpf_addr)), cp_pcreg(cpf_addr));
  fprintf(where,"   CP stack %p:\tprev top:\t%p\n",
	     &(cp_prevtop(cpf_addr)), cp_prevtop(cpf_addr));
#ifdef CP_DEBUG
  if ( (int) cp_psc(cpf_addr) != 0)
    alt_print_cpf_pred((CPtr) &(cp_psc(cpf_addr)),where);
#endif
  fprintf(where,"   CP stack %p:\tprev env cap (ebreg):\t%p\n",
	     &(cp_ebreg(cpf_addr)), cp_ebreg(cpf_addr));
  fprintf(where,"   CP stack %p:\ttop of heap:\t\t%p\n",
	     &(cp_hreg(cpf_addr)), cp_hreg(cpf_addr));
  fprintf(where,"   CP stack %p:\ttop of trail:\t\t%p\n",
	     &(cp_trreg(cpf_addr)), cp_trreg(cpf_addr));
  fprintf(where,"   CP stack %p:\tcontinuation pointer:\t%p\n",
	     &(cp_cpreg(cpf_addr)), cp_cpreg(cpf_addr));
  fprintf(where,"   CP stack %p:\tereg:\t%p\n",
	     &(cp_ereg(cpf_addr)), cp_ereg(cpf_addr));
  fprintf(where,"   CP stack %p:\tparent subgoal dreg:\t%p\n",
	     &(cp_pdreg(cpf_addr)), cp_pdreg(cpf_addr));
  fprintf(where,"   CP stack %p:\troot subgoal:\t%p\n",
	     &(cp_ptcp(cpf_addr)), cp_ptcp(cpf_addr));
  fprintf(where,"   CP stack %p:\tscheduling link:\t\t%p\n",
	     &(cp_prevbreg(cpf_addr)), cp_prevbreg(cpf_addr));
 }

static void print_cpf(CTXTdeclc CPtr cpf_addr, FILE* where) {

  CPtr arg;
  int i, num_of_args, cp_type = 0;
  byte inst;

  inst = * (byte *) * cpf_addr;

  /* tableretry, tabletrust, check_complete */
  if (inst == 0xc3 || inst == 0xc4 || inst == 0xc4)
    cp_type = GENERATOR_CP_FRAME;
  /* retryme, trustme, retry, trust, dynretry, dyntrust, retrymeor, trustmeor */
  else if (inst == 0xa1 || inst == 0xa2 || inst == 0xa4
	   || inst == 0xa5 || inst == 0xba || inst == 0xbb || inst == 0xb8 || inst == 0xb9)
    cp_type = STANDARD_CP_FRAME;
  else if (inst >= 0x5c && inst <= 0x77)  // tries
    cp_type = STANDARD_CP_FRAME;
  else if (inst == 0xc5)
    cp_type = CONSUMER_CP_FRAME;
  else if (inst == 0xc6)
    cp_type = COMPL_SUSP_CP_FRAME;

  switch (cp_type) {
  case STANDARD_CP_FRAME:
    fprintf(where,"Standard Choice Point Frame: (%s)\n",(char *)inst_table[inst][0]);

    print_common_cpf_part(cpf_addr,where);

    num_of_args = (int)((cp_prevtop(cpf_addr) - cpf_addr) - CP_SIZE);
    for (i = 1, arg = cpf_addr + CP_SIZE; i <= num_of_args; i++, arg++)
      fprintf(where,"   CP stack %p:\tpredicate arg #%d:\t0x%p\n",
		 arg, i, ref_val(*arg));
    break;
  case GENERATOR_CP_FRAME:
    fprintf(where,"Generator Choice Point Frame: ");
    print_subgoal(CTXTc where, (VariantSF) tcp_subgoal_ptr(cpf_addr));fprintf(where,"\n");
    print_common_cpf_part(cpf_addr,where);
    fprintf(where,"   CP stack %p:\ttemplate:\t0x%p\n",
	       &(tcp_template(cpf_addr)), tcp_template(cpf_addr));
    fprintf(where,"   CP stack %p:\tsubgoal frame ptr:\t0x%p\n",
	       &(tcp_subgoal_ptr(cpf_addr)), tcp_subgoal_ptr(cpf_addr));
    fprintf(where,"   CP stack %p:\tCh P  freeze register:\t0x%p\n",
	       &(tcp_bfreg(cpf_addr)), tcp_bfreg(cpf_addr));
    fprintf(where,"   CP stack %p:\tHeap  freeze register:\t0x%p\n",
	       &(tcp_hfreg(cpf_addr)), tcp_hfreg(cpf_addr));
    fprintf(where,"   CP stack %p:\tTrail freeze register:\t0x%p\n",
	       &(tcp_trfreg(cpf_addr)), tcp_trfreg(cpf_addr));
    fprintf(where,"   CP stack %p:\tLo St freeze register:\t0x%p\n",
	       &(tcp_efreg(cpf_addr)), tcp_efreg(cpf_addr));
#ifdef LOCAL_EVAL
    fprintf(where,"   CP stack %p:\tlocal eval trie_return:\t0x%p\n",
	       &(tcp_trie_return(cpf_addr)), tcp_trie_return(cpf_addr));
#endif
    num_of_args = (int)((cp_prevtop(cpf_addr) - cpf_addr) - TCP_SIZE);
    for (i = 1, arg = cpf_addr + TCP_SIZE; i <= num_of_args; i++, arg++)
      fprintf(where,"   CP stack %p:\tpredicate arg #%d:\t0x%p\n",
	      arg, i, ref_val(*arg));
    break;
  case CONSUMER_CP_FRAME:
    fprintf(where,"Consumer Choice Point Frame: ");
    print_subgoal(CTXTc where, (VariantSF) nlcp_subgoal_ptr(cpf_addr));fprintf(where,"\n");
    print_common_cpf_part(cpf_addr,where);
    fprintf(where,"   CP stack %p:\ttemplate:\t0x%p\n",
	       &(nlcp_template(cpf_addr)), nlcp_template(cpf_addr));
    fprintf(where,"   CP stack %p:\tsubgoal frame ptr:\t0x%p\n",
	       &(nlcp_subgoal_ptr(cpf_addr)), nlcp_subgoal_ptr(cpf_addr));
    fprintf(where,"   CP stack %p:\tPrevlookup:\t0x%p\n",
	       &(nlcp_prevlookup(cpf_addr)), nlcp_prevlookup(cpf_addr));
#ifdef LOCAL_EVAL
    fprintf(where,"   CP stack %p:\tlocal eval trie_return:\t0x%p\n",
	       &(nlcp_trie_return(cpf_addr)), nlcp_trie_return(cpf_addr));
#endif
    num_of_args = (int)((cp_prevtop(cpf_addr) - cpf_addr) - NLCP_SIZE);
    for (i = 1, arg = cpf_addr + NLCP_SIZE; i <= num_of_args; i++, arg++)
      fprintf(where,"   CP stack %p:\tpredicate arg #%d:\t0x%p\n",
		arg, i, ref_val(*arg));
    break;
    case COMPL_SUSP_CP_FRAME:
    fprintf(where,"Completion Choice Point Frame:\n");
    print_common_cpf_part(cpf_addr,where);
    fprintf(where,"   CP stack %p:\tsubgoal frame ptr:\t0x%p\n",
	       &(csf_subgoal_ptr(cpf_addr)), csf_subgoal_ptr(cpf_addr));
    fprintf(where,"   CP stack %p:\tPrevCSF:\t0x%p\n",
	       &(csf_prevcsf(cpf_addr)), csf_prevcsf(cpf_addr));
    fprintf(where,"   CP stack %p:\tNeg Loop:\t%d\n",
	    &(csf_neg_loop(cpf_addr)), (int) csf_neg_loop(cpf_addr));
    num_of_args = (int)((cp_prevtop(cpf_addr) - cpf_addr) - CSF_SIZE);
    for (i = 1, arg = cpf_addr + CSF_SIZE; i <= num_of_args; i++, arg++)
      fprintf(where,"   CP stack %p:\tpredicate arg #%d:\t0x%p\n",
		arg, i, ref_val(*arg));
    break;
  default:
    xsb_error("CP Type %d not handled yet...", cp_type);
    break;
  }
}

int alt_printnum = 0 ;

void alt_print_cp(CTXTdeclc char * title)
{
  CPtr startp, endp ;
  char buf[100] ;
  int  start ;
  FILE *where ;

  sprintf(buf,"ACP%d",alt_printnum) ;
  alt_printnum++ ;
  where = fopen(buf,"w") ;
  if (! where)
    { xsb_dbgmsg((LOG_GC, "could not open CP%d", alt_printnum));
      return;
    }

  start = 0 ;
  startp = (CPtr)tcpstack.high - 1 ;
  endp = top_of_cpstack ;

  fprintf(where,"%s breg: %p bfreg %p\n",title,breg,bfreg);
  while ( startp > endp )
  { fflush(where);
    start++ ;
    print_cpf(CTXTc endp, where );

    endp = cp_prevtop(endp);
  }

  fclose(where) ;
} /* print_cp */

extern void dis_data(FILE *);
extern void dis_text(FILE *);

void alt_dis(CTXTdecl)
{
  FILE *where ;

  alt_printnum++ ;
  where = fopen("ALTDIS","w") ;
  if (! where)
    { xsb_dbgmsg((LOG_GC, "could not open ALTDIS"));
      return;
    }

  dis_data(where);
  dis_text(where);

  fclose(where) ;
} /* print_cp */

/*----- For table debugging --------------------------------------------

  Among other functionality, the following routines allow subgoals to
  be printed when given a pointer to a variant subgoal frame.  For
  boxed integers and floats, the boxed terms, which are entered into
  the trie as structures, must be interpreted in a special manner to
  be printed out, as the information is kept in a cell array rather
  than in the heap..  Special macros are used for this --
  trie_boxed_int() and trie_boxed_float().

-----------------------------------------------------------------------*/




#ifndef MULTI_THREAD
static Cell cell_array[MAXTERMBUFSIZE];
#endif

#define trie_boxedfloat_val(i)				\
    (                                                   \
     (Float)(    EXTRACT_FLOAT_FROM_16_24_24(           \
					     (int_val(cell_array[(*i)-1])), \
					     (int_val(cell_array[(*i)-2])), \
					     (int_val(cell_array[(*i)-3])) \
							)		\
		 )							\
							)

#define trie_boxedint_val(i) \
  ((Integer) ((UInteger)int_val(cell_array[(*i)-2])<<24 | int_val(cell_array[(*i)-3])))

static void print_term_of_subgoal(CTXTdeclc FILE *fp, byte car, int *i)
{
  Cell term;
  int  j, args;

  term = cell_array[*i];
  switch (cell_tag(term)) {
  case XSB_TrieVar:
    fprintf(fp, "_v%d", ((int) int_val(term) & 0xffff));
    break;
  case XSB_STRUCT:
    if (isboxedTrieSym(term) && ((int_val(cell_array[(*i)-1]) >> 16) == ID_BOXED_FLOAT)) {
      Float val = trie_boxedfloat_val(i);
      fprintf(fp, "%d.%4d",(int)floor(val),(int)((val-floor(val))*10000));
      *i = (*i) -3;
      break;
    }
    else if (isboxedTrieSym(term) && ((int_val(cell_array[(*i)-1]) >> 16) == ID_BOXED_INT)) {
      fprintf(fp, "%" Intfmt, (Integer)trie_boxedint_val(i));
      *i = (*i) -3;
      break;
    }
    args = get_arity((Psc)cs_val(term));
    write_quotedname(fp, CURRENT_CHARSET, get_name((Psc)cs_val(term)));
    /*    fprintf(fp, "%s", get_name((Psc)cs_val(term))); */
    if (args > 0) fprintf(fp, "(");
    for (j = args; j > 0; j--) {
      (*i)--;
      print_term_of_subgoal(CTXTc fp, car,i);
      if (j > 1) fprintf(fp, ",");
    }
    if (args > 0) fprintf(fp, ")");
    break;
  case XSB_LIST:
    if ( car ) { fprintf(fp, "["); }
    (*i)--;
    print_term_of_subgoal(CTXTc fp, CAR, i);
    (*i)--;
    term = cell_array[*i];
  switch (cell_tag(term)) {
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      goto vertbar;
    case XSB_LIST:
      fprintf(fp, ",");
      print_term_of_subgoal(CTXTc fp, CDR, i);
      break;
    case XSB_STRING:
      if (string_val(term) != nil_string)
	goto vertbar;
      else {
	fprintf(fp, "]");
	break;
      }
    case XSB_STRUCT:
    case XSB_INT:
    case XSB_FLOAT:
    vertbar:
      fprintf(fp, "|");
      print_term_of_subgoal(CTXTc fp, CDR, i);
      fprintf(fp, "]");
      break;
  }
  break;
  case XSB_STRING:
    write_quotedname(fp,CURRENT_CHARSET,string_val(term));
  /*    fprintf(fp, "%s", string_val(term));*/
    break;
  case XSB_INT:
    fprintf(fp, "%d", (int) int_val(term));
    break;
  case XSB_FLOAT:
    fprintf(fp, "%.5g", float_val(term));
    break;
  default:
    xsb_error("Term with unknown tag (%d) in print_subgoal()",
	      (int)cell_tag(term));
    break;
  }
}

static int sprint_term_of_subgoal(CTXTdeclc forestLogBuffer fl_buf, int size,byte car, int *i)
{
  Cell term;
  int  j, args;

  //  printf("sprint_term %d\n",size);

  maybe_realloc_buffers(fl_buf,size);
  //  char * buffer = fl_buf->fl_buffer;

  term = cell_array[*i];
  switch (cell_tag(term)) {
  case XSB_TrieVar:
    sprintf(virtual_buffer+size, "_v%d", ((int) int_val(term) & 0xffff));size=size+3;
    break;
  case XSB_STRUCT:
    if (isboxedTrieSym(term) && ((int_val(cell_array[(*i)-1]) >> 16) == ID_BOXED_FLOAT)) {
      Float val = trie_boxedfloat_val(i);
      int width = get_int_print_width((Integer)floor(val));
      sprintf(virtual_buffer+size, "%*d.%4d", width,(int)floor(val),(int)((val-floor(val))*10000));
      *i = (*i) -3;
      return size+width+5;
    }
    else if (isboxedTrieSym(term) && ((int_val(cell_array[(*i)-1]) >> 16) == ID_BOXED_INT)) {
      Integer val = boxedint_val(term);
      int width = get_int_print_width(val);
      sprintf(virtual_buffer+size, "%*" Intfmt, width, (Integer)trie_boxedint_val(i));
      *i = (*i) -3;
      return size+width;
    }
    args = get_arity((Psc)cs_val(term));
    size = sprint_quotedname(virtual_buffer, size, get_name((Psc)cs_val(term)));
    if (args > 0) {sprintf(virtual_buffer+size, "(");size++;}
    for (j = args; j > 0; j--) {
      (*i)--;
      size = sprint_term_of_subgoal(CTXTc fl_buf,size, CAR, i);
      if (j > 1) {sprintf(virtual_buffer+size, ",");size++;}
    }
    if (args > 0) {sprintf(virtual_buffer+size, ")");size++;}
    break;
  case XSB_LIST:
    if ( car ) { sprintf(virtual_buffer+size, "["); size++;}
    (*i)--;
    size = sprint_term_of_subgoal(CTXTc fl_buf,size, CAR, i);
    (*i)--;
    term = cell_array[*i];
  switch (cell_tag(term)) {
    case XSB_FREE:
    case XSB_REF1:
    case XSB_ATTV:
      goto vertbar;
    case XSB_LIST:
      sprintf(virtual_buffer+size, ",");size++;
      size = sprint_term_of_subgoal(CTXTc fl_buf, size, CDR, i);
      return size;
    case XSB_STRING:
      if (string_val(term) != nil_string)
	goto vertbar;
      else {
	sprintf(virtual_buffer+size, "]");
	return size+1;
      }
    case XSB_STRUCT:
    case XSB_INT:
    case XSB_FLOAT:
    vertbar:
      sprintf(virtual_buffer+size, "|");size++;
      size = sprint_term_of_subgoal(CTXTc fl_buf,size, CDR, i);
      sprintf(virtual_buffer+size, "]");size++;
    return size;
  }
  case XSB_STRING:
    size = sprint_quotedname(virtual_buffer,size,string_val(term));
    break;
  case XSB_INT: {
    //    int length = get_int_print_width((Integer)int_val(term));
    //    size = size + sprintf(virtual_buffer+size, "%*" Intfmt, length, (Integer)int_val(term));
    //    return size+length;
    size = size + sprintf(virtual_buffer+size, "%" Intfmt, (Integer)int_val(term));
    return size;
    break;
  }
  case XSB_FLOAT:
    size = size + sprintf(virtual_buffer+size, "%10f", float_val(term));
    break;
  default:
    xsb_error("Term with unknown tag (%d) in print_subgoal()",
	      (int)cell_tag(term));
    break;
  }
  return size;
}

/*----------------------------------------------------------------------*/

void print_subgoal_callnode_leaf(CTXTdeclc FILE *fp, callnodeptr cn)
{
  BTNptr leaf;
  int  i = 0;
  Psc  psc;

  fprintf(fp,"Incr dyn: ");
  if (!callnode_tif_ptr(cn)) {
    fprintf(fp,"NULL TIF");
    return;
  }
  psc = TIF_PSC((TIFptr) callnode_tif_ptr(cn));

  for (leaf = callnode_leaf_ptr(cn); leaf != NULL; leaf = Parent(leaf)) {
    cell_array[i++] = BTN_Symbol(leaf);
  }
  write_quotedname(fp,CURRENT_CHARSET,get_name(psc));
  /*  fprintf(fp, "%s", get_name(psc)); */
  if (get_arity(psc) > 0) {
    fprintf(fp, "(");
    for (i = i-2; i >= 0 ; i--) {
      print_term_of_subgoal(CTXTc fp, CAR, &i);
      if (i > 0) fprintf(fp, ", ");
    }
    fprintf(fp, ")");
  }
}

void print_callnode(CTXTdeclc FILE *fp, callnodeptr cn) {
  if (cn -> goal)  print_subgoal(CTXTc fp,cn->goal);
  else if (!cn->is_incremental_trie) print_subgoal_callnode_leaf(CTXTc fp, cn);
  else fprintf(stdout,"incremental trie");
}

void print_subgoal(CTXTdeclc FILE *fp, VariantSF subg)
{
  BTNptr leaf;
  int  i = 0;
  Psc  psc = TIF_PSC(subg_tif_ptr(subg));

  for (leaf = subg_leaf_ptr(subg); leaf != NULL; leaf = Parent(leaf)) {
    cell_array[i++] = BTN_Symbol(leaf);
  }
  write_quotedname(fp,CURRENT_CHARSET,get_name(psc));
  /*  fprintf(fp, "%s", get_name(psc)); */
  if (get_arity(psc) > 0) {
    fprintf(fp, "(");
    for (i = i-2; i >= 0 ; i--) {
      print_term_of_subgoal(CTXTc fp, CAR, &i);
      if (i > 0) fprintf(fp, ", ");
    }
    fprintf(fp, ")");
  }
}


int sprint_subgoal(CTXTdeclc forestLogBuffer fl_buf,  int ctr, VariantSF subg)
{
  BTNptr leaf;
  int  i = 0;
  int size = ctr;

  Psc  psc = TIF_PSC(subg_tif_ptr(subg));

  for (leaf = subg_leaf_ptr(subg);
       leaf != NULL && (int) i < MAXTERMBUFSIZE;
       leaf = Parent(leaf) ) {
    cell_array[i++] = BTN_Symbol(leaf);
  }

  if (i >= MAXTERMBUFSIZE) {
    //    printf("i %d\n",i);
    size = sprintf(fl_buf->fl_buffer,"'!!! term too lorge'");
    return size;
  }

  size = sprint_quotedname(virtual_buffer, size, get_name(psc));
  if (get_arity(psc) > 0) {
    sprintf(virtual_buffer+size, "(");size++;
    for (i = i-2; i >= 0 //&& size < MAXTERMBUFSIZE - 100
	   ; i--) {
      size = sprint_term_of_subgoal(CTXTc fl_buf, size, CAR,&i);
      if (i > 0) {sprintf(virtual_buffer+size, ",");size++;}
    }
    sprintf(virtual_buffer+size,")");size++;
  }
  maybe_realloc_buffers(fl_buf,(size));
  return size;
}

/*----------------------------------------------------------------------*/

void print_completion_stack(CTXTdeclc FILE *fptr)
{
  int SCCnum = 1; int lastSCCnum;
  VariantSF subg;
  CPtr temp = COMPLSTACKBOTTOM-COMPLFRAMESIZE;
  printf("----------- scc -----------\n");
  lastSCCnum = compl_level(temp);

 while (temp >= openreg) {
   if (compl_level(temp) != lastSCCnum) {
     SCCnum++; lastSCCnum = compl_level(temp);
   }
   subg = (VariantSF) *temp;
   print_subgoal(CTXTc fptr,subg);
   fprintf(fptr," - scc(%d,%d).\n",SCCnum,compl_level(temp));
   temp = next_compl_frame(temp);
  }
}

/*----------------------------------------------------------------------*/

void print_delay_element(CTXTdeclc FILE *fp, Cell del_elem)
{
  Psc  psc = 0;
  CPtr cptr;
  int arity, i;
  Cell tmp_cell;
  char *name;

  if ((psc = get_str_psc(del_elem)) == delay_psc) {

    fprintf(fp, "%s(", get_name(psc));
    cptr = (CPtr)cs_val(del_elem);
    tmp_cell = cell(cptr + 1);
    print_subgoal(CTXTc fp, (VariantSF) addr_val(tmp_cell)); fprintf(fp, ",");
    tmp_cell = cell(cptr + 2);
    fprintf(fp, "%p", (BTNptr) addr_val(tmp_cell)); fprintf(fp, ",");
    tmp_cell = cell(cptr + 3);
    if (isointeger(tmp_cell)) {
      fprintf(fp, "NEG");
    }
    else {
      if (isstring(tmp_cell)) {
	arity = 0;
	name = string_val(tmp_cell);
      }
      else {
	psc = get_str_psc(cell(cptr + 3));
	arity = get_arity(psc);
	name = get_name(psc);
      }
      fprintf(fp, "%s/%d(", name, arity);
      if (arity > 0) {
	cptr = (CPtr) cs_val(cell(cptr + 3));
	for (i = 0; i < arity; i++)
	  printterm(fp, cell(cptr + 1 + i), 25);
      }
    }
    fprintf(fp, ")");
  }
  else {
    xsb_abort("Unknown delay list element in print_delay_element()");
  }
}

/* This does a more thorough job of printing out delay elements than
   print_delay_element(), which I haven't updated */
int sprint_delay_element(CTXTdeclc forestLogBuffer fl_buf, int ctr ,Cell del_elem) {
  Psc  psc = 0;
  CPtr cptr;
  Cell tmp_cell;
  //  int arity, i;  char *name;

  //  print_delay_element(CTXTc stdout, del_elem);printf("\n");

  if ((psc = get_str_psc(del_elem)) == delay_psc) {
    cptr = (CPtr)cs_val(del_elem);
    /* If NDE, only worry about first cell (cf. delay_negatively) */
    if (cell(cptr + 3) == makeaddr(0)) {
      ctr = ctr + sprintf(virtual_buffer+ctr,"tnot(");
      tmp_cell = cell(cptr + 1);
      ctr = sprint_subgoal(CTXTc fl_buf,ctr, (VariantSF) addr_val(tmp_cell));
      ctr = ctr + sprintf(virtual_buffer+ctr,")");
      //      printf("2 %s | %s\n",buffer,fl_buf->fl_buffer);
    }
    else {     /* If PDE */
      ctr =  ctr + sprintf(virtual_buffer+ctr,"de(");
      tmp_cell = cell(cptr + 1);
      ctr = sprint_subgoal(CTXTc fl_buf,ctr, (VariantSF) addr_val(tmp_cell));
      tmp_cell = cell(cptr + 2);
      ctr = ctr + sprintf(virtual_buffer+ctr, ",");
      ctr = ctr + sprintTriePath(CTXTc virtual_buffer+ctr, (BTNptr) addr_val(tmp_cell));
      //      ctr = sprintTriePath(CTXTc virtual_buffer+ctr, (BTNptr) addr_val(tmp_cell));
      ctr = ctr + sprintf(virtual_buffer+ctr, ")");

      //      ctr = ctr + sprintf(virtual_buffer+ctr, "%p", (BTNptr) addr_val(tmp_cell));
      //      ctr = ctr + sprintf(virtual_buffer+ctr, ",");
      //      tmp_cell = cell(cptr + 3);
      //      printf("tmp_cell %x\n",tmp_cell);
      //      if (isointeger(tmp_cell)) {
      //	ctr = ctr + sprintf(virtual_buffer+ctr, "NEG");
      //      }
      //      else {
      //	if (isstring(tmp_cell)) {
      //  arity = 0;
      //  name = string_val(tmp_cell);
      //}
      //	else {
      //  psc = get_str_psc(cell(cptr + 3));
      //  arity = get_arity(psc);
      //  name = get_name(psc);
      //}
      //ctr = ctr + sprintf(virtual_buffer+ctr, "%s", name);
      //if (arity > 0) {
      //  ctr = ctr + sprintf(virtual_buffer+ctr, "(");
      //  cptr = (CPtr) cs_val(cell(cptr + 3));
      //  for (i = 0; i < arity; i++)
      //    ctr = ctr + sprintTerm(virtual_buffer+ctr, cell(cptr + 1 + i));
      //  ctr = ctr + sprintf(virtual_buffer+ctr, ")");
      //}
    }
    return ctr;
  }
  else {
    xsb_abort("Unknown delay list element in print_delay_element()");
    return 0; // quiet compiler.
  }
}

/*----------------------------------------------------------------------*/

void print_delay_list(CTXTdeclc FILE *fp, CPtr dlist)
{
  CPtr cptr;

  if (dlist == NULL) {
    fprintf(fp, "[]"); fflush(fp);
  } else {
    if (islist(dlist) || isnil(dlist)) {
      fprintf(fp, "["); cptr = dlist;
      while (islist(cptr)) {
	cptr = clref_val(cptr);
	print_delay_element(CTXTc fp, cell(cptr));
	cptr = (CPtr)cell(cptr+1);
	if (islist(cptr)) fprintf(fp, ", ");
      }
      if (isnil(cptr)) {
	fprintf(fp, "]"); fflush(fp);
      } else {
	xsb_abort("Delay list with unknown tail type in print_delay_list()");
      }
    } else {
      xsb_abort("Delay list with unknown type in print_delay_list()");
    }
  }
}

int sprint_delay_list(CTXTdeclc forestLogBuffer fl_buf, CPtr dlist)
{
  CPtr cptr;
  int ctr = 0;

  if (dlist == NULL) {
    return sprintf(virtual_buffer, "[]");
  } else {
    if (islist(dlist) || isnil(dlist)) {
      ctr = ctr + sprintf(virtual_buffer, "[");
      cptr = dlist;
      while (islist(cptr)) {
	cptr = clref_val(cptr);
	ctr = sprint_delay_element(CTXTc fl_buf, ctr, cell(cptr));
	cptr = (CPtr)cell(cptr+1);
	if (islist(cptr)) {
	  ctr = ctr + sprintf(virtual_buffer+ctr, ", ");
	}
      }
      if (isnil(cptr)) {
	return ctr + sprintf(virtual_buffer+ctr, "]");
      } else {
	xsb_abort("Delay list with unknown tail type in print_delay_list()");
      }
    } else {
      xsb_abort("Delay list with unknown type in print_delay_list()");
    }
  }
  return 0; // quiet compiler.
}

/*-------------------------------------------------------------------------*/

/*
 *			TableInfoFrame Printing
 *			-----------------------
 */

/*
 * Run the doubly-linked list of Subgoal Frames to the end, then back to
 * the beginning -- which may lie beyond `dll' -- counting the number of
 * frames encountered in each direction.
 */

void subg_dll_length(VariantSF dll, counter *forward, counter *back) {

  VariantSF cur, prev;
  counter f, b;

  /* Count the number of frames on the chain from `dll' forward. */
  f = 0;
  for ( prev = NULL, cur = dll;
	IsNonNULL(cur);
	prev = cur, cur = subg_next_subgoal(cur) )
    f++;

  /* Count the number of frames on the chain from the end to the beginning */
  b = 0;
  for ( cur = prev;  IsNonNULL(cur);  cur = subg_prev_subgoal(cur) )
    b++;

  *forward = f;
  *back = b;
}

void printTIF(FILE * where,TIFptr tif) {

  counter forward, back;

  fprintf(where,"TableInfoFrame  %p\n"
	 "{ psc_ptr = %p  (%s/%d)\n"
	 "  call_trie = %p\n"
	 "  del_tf_ptr = %p\n"
	 "  subgoals = %p  }",
	 tif,
	 TIF_PSC(tif), get_name(TIF_PSC(tif)), get_arity(TIF_PSC(tif)),
	 //	 stringTabledEvalMethod(TIF_EvalMethod(tif)),
	 TIF_CallTrie(tif),
	 TIF_DelTF(tif),
	 TIF_Subgoals(tif));
  subg_dll_length(TIF_Subgoals(tif),&forward,&back);
  if ( forward == back )
    fprintf(where,"(%d total)", (int) forward);
  else
    fprintf(where,"(chain length mismatch: %d forward, %d back)", (int) forward, (int) back);
  fprintf(where,"\n  next_tif = %p }\n", TIF_NextTIF(tif));
}

void printDelTF(DelTFptr dtf) {
  printf("DelTF %p\n"
	 "{ call_trie = %p\n"
	 "  subgoals = %p\n"
	 "  type = %d\n"
	 "  mark = %d\n"
	 "  next_delTF = %p\n"
	 "  prev_delTF = %p\n"
	 "  next_pred_delTF = %p\n"
	 "  prev_pred_delTF = %p\n",
	 dtf,DTF_CallTrie(dtf),DTF_Subgoals(dtf),DTF_Type(dtf),DTF_Mark(dtf),
	 DTF_NextDTF(dtf),DTF_PrevDTF(dtf),DTF_NextPredDTF(dtf),DTF_PrevPredDTF(dtf));
}

#ifdef MULTI_THREAD
void print_private_deltfs(CTXTdecl) {
  DelTFptr dtf =   private_deltf_chain_begin;
  printf("========================== private deltf chain\n");
  while (dtf != NULL) {
    printDelTF(dtf);
    printf("\n");
    dtf = DTF_NextDTF(dtf);
  }
  printf("==========================\n");
}
#endif

extern DelTFptr deltf_chain_begin;

void print_deltf_chain(CTXTdecl) {
  DelTFptr dtf =   deltf_chain_begin;
  printf("========================== deltf chain\n");
  while (dtf != NULL) {
    printDelTF(dtf);
    printf("\n");
    dtf = DTF_NextDTF(dtf);
  }
  printf("==========================\n");
}

/*----- For table debugging --------------------------------------------*/

/** not used, but in commented-out code..
static char *compl_stk_frame_field[] = {
  "subgoal_ptr",   "del_ret_list", "level_num","visited",
#ifndef LOCAL_EVAL
"DG_edges", "DGT_edges"
#endif
};
***/

void print_subg_header(CTXTdeclc FILE * where,VariantSF SUBG) {
    fprintf(where, "=== Frame for "); print_subgoal(CTXTc where, SUBG);
    if (is_completed(SUBG)) fprintf(where, " (completed) ===\n");
    else fprintf(where, " (incomplete) ===\n"); }

/* Not using array of field names because int fields are bit-mapped in a single 64-bit word */
void debug_print_completion_stack(CTXTdecl)
{
  int i = 0;
  VariantSF subg;
  CPtr temp = openreg;
  fprintf(stddbg,"openreg -> ");
  while (temp < COMPLSTACKBOTTOM) {
    if ((i % COMPLFRAMESIZE) == 0) {
      fprintf(stddbg,EOFR);	/* end of frame */
      subg = (VariantSF) *temp;
      print_subg_header(CTXTc stddbg, subg);
    }
    if ((i % COMPLFRAMESIZE) == 0) { fprintf(stddbg,"Completion Stack %p: %lx\t(subgoal_ptr)",temp, (unsigned long) *temp);}
    if ((i % COMPLFRAMESIZE) == 1) { fprintf(stddbg,"Completion Stack %p: %lx\t(del_ret_list)",temp, (unsigned long) *temp);}
    if ((i % COMPLFRAMESIZE) == 2) { fprintf(stddbg,"Completion Stack %p: %lu/%lu\t(level_num/visited)",temp, (unsigned long) (*temp)&0xffffffff,(unsigned long) (*temp)>>16);}
#ifndef LOCAL_EVAL
    EPtr eptr;
    if ((i % COMPLFRAMESIZE) >= COMPLFRAMESIZE-2) {
      for (eptr = (EPtr)*temp; eptr != NULL; eptr = next_edge(eptr)) {
	fprintf(stddbg," --> %p", edge_to_node(eptr));
      }
    }
#endif
    fprintf(stddbg,"\n");
    temp++; i++;
  }
  fprintf(stddbg,"--------------------BOTTOM_OF_STACK (%p) --------------------\n",COMPLSTACKBOTTOM);
}


static int count_producer_subgoals(CTXTdecl)
{
  int i;
  TIFptr tif;
  VariantSF temp_ptr;

  i = 0;

  SYS_MUTEX_LOCK( MUTEX_TABLE );
  for ( tif = tif_list.first;  IsNonNULL(tif);  tif = TIF_NextTIF(tif) )
    for ( temp_ptr = TIF_Subgoals(tif);  IsNonNULL(temp_ptr);
	  temp_ptr = (VariantSF)subg_next_subgoal(temp_ptr) )
      i ++;
  SYS_MUTEX_UNLOCK( MUTEX_TABLE );
  return(i);
}

/*----------------------------------------------------------------------*/

/*
 *			Subgoal Frame Printing
 *			----------------------
 */

char *stringSubgoalFrameType(byte type) {

  switch(type) {
  case VARIANT_PRODUCER_SFT:
    return("variant");
    break;
  case SUBSUMPTIVE_PRODUCER_SFT:
    return("subsumptive producer");
    break;
  case SUBSUMED_CONSUMER_SFT:
    return("subsumptive consumer");
    break;
  default:
    return("unknown");
    break;
  }
}

/*
 * Tries to make the interface more robust by cleaning-up any extra user
 * input supplied to a prompt.  Place a call to this function after any
 * input scan which doesn't take the whole input line (ie. which isn't a
 * `scanf("%s", &array);').
 */
static void skip_to_nl(void)
{
  char c;

  do {
    c = getchar();
  } while (c != '\n');
}

void print_tables(CTXTdecl)
{
  int i = 0;
  char ans = 'y';
  TIFptr tif;
  VariantSF subg;
  SubConsSF cons;

  i = count_producer_subgoals(CTXT);
  xsb_dbgmsg((LOG_DEBUG,"\t There are %d producer subgoal structures...", i));

  i = 0;
  SYS_MUTEX_LOCK( MUTEX_TABLE );
  for ( tif = tif_list.first;  IsNonNULL(tif) && (ans == 'y');
	tif = TIF_NextTIF(tif) ) {
    fprintf(stddbg,EOSUBG);
    printTIF(stddbg,tif);
    subg = TIF_Subgoals(tif);
    while ( IsNonNULL(subg) && (ans == 'y') ) {
      i++;
      print_subg_header(CTXTc stddbg,subg);
      fprintf(stddbg, "%p:\n", subg);
      fprintf(stddbg,"  sf_type = %s,  is_complete = %s,  is_reclaimed = %s,",
		 stringSubgoalFrameType(subg_sf_type(subg)),
		 (subg_is_completed(subg) ? "YES" : "NO"),
		 (subg_is_reclaimed(subg) ? "YES" : "NO"));
      fprintf(stddbg,"  tif_ptr = %p,  leaf_ptr = %p,  ans_root_ptr = %p,\n"
		 "  ans_list_ptr = %p,   ans_list_tail = %p,\n"
		 "  next_subgoal = %p,  prev_subgoal = %p,  cp_ptr = %p",
		 subg_tif_ptr(subg), subg_leaf_ptr(subg),
		 subg_ans_root_ptr(subg),
		 subg_ans_list_ptr(subg), subg_ans_list_tail(subg),
		 subg_next_subgoal(subg), subg_prev_subgoal(subg),
		 subg_cp_ptr(subg));
      fprintf(stddbg,"  pos_cons = %p,", subg_pos_cons(subg));
      fprintf(stddbg,"  compl_stk_ptr = %p,  compl_susp_ptr = %p,"
		 "  nde_list = %p",
		 subg_compl_stack_ptr(subg), subg_compl_susp_ptr(subg),
		 subg_nde_list(subg));
      if ( IsSubProdSF(subg) ) {
	fprintf(stddbg,"  consumers = %p", subg_consumers(subg));
	for ( cons = subg_consumers(subg);  IsNonNULL(cons);
	      cons = conssf_consumers(cons) )
	  fprintf(stddbg,"Consumer  %p\n"
		     "  sf_type = %11s,  tif_ptr = %p,         leaf_ptr = %p\n"
		     "  producer = %10p,  ans_list_ptr = %p,"
		     "  ans_list_tail = %p\n"
		     "  ts = %ul,  consumers = %p",
		  cons, stringSubgoalFrameType(subg_sf_type(cons)), subg_tif_ptr(cons),
		  subg_leaf_ptr(cons), conssf_producer(cons),
		  subg_ans_list_ptr(cons), subg_ans_list_tail(cons),
		  (int) conssf_timestamp(cons), conssf_consumers(cons));
      }
      subg = subg_next_subgoal(subg);
      if (subg != NULL)
	fprintf(stddbg, EOSUBG);
      if (i == 10) {
	int retcode; /* to squash warnings */
	fprintf(stddbg, "more (y/n)?  ");
	retcode = scanf("%c", &ans);
	SQUASH_LINUX_COMPILER_WARN(retcode) ;
	skip_to_nl();
	i = 0;
      }
    }
  SYS_MUTEX_UNLOCK( MUTEX_TABLE );
  }
  fprintf(stddbg, EOS);
}

static int table_printnum = 0 ;

void file_print_tables(CTXTdeclc char * message,int number)
{
  int i = 0;
  TIFptr tif;
  VariantSF subg;
  SubConsSF cons;

  char buf[100] ;
  FILE *where ;

  sprintf(buf,"TABLES%d",table_printnum) ;
  table_printnum++ ;
  where = fopen(buf,"w") ;

  fprintf(where,"%s: %d\n",message,number);
  i = count_producer_subgoals(CTXT);
  xsb_dbgmsg((LOG_DEBUG,"\t There are %d producer subgoal structures...", i));

  i = 0;
  SYS_MUTEX_LOCK( MUTEX_TABLE );
  for ( tif = tif_list.first;  IsNonNULL(tif);
	tif = TIF_NextTIF(tif) ) {
    fprintf(where,EOSUBG);
    printTIF(where,tif);
    subg = TIF_Subgoals(tif);
    while ( IsNonNULL(subg)  ) {
      i++;
      print_subg_header(CTXTc where,subg);
      fprintf(where, "%p:\n", subg);
      fprintf(where,"  sf_type = %s,  is_complete = %s,  is_reclaimed = %s,",
		 stringSubgoalFrameType(subg_sf_type(subg)),
		 (subg_is_completed(subg) ? "YES" : "NO"),
		 (subg_is_reclaimed(subg) ? "YES" : "NO"));
      fprintf(where,"  tif_ptr = %p,  leaf_ptr = %p,  ans_root_ptr = %p,\n"
		 "  ans_list_ptr = %p,   ans_list_tail = %p,\n"
		 "  next_subgoal = %p,  prev_subgoal = %p,  cp_ptr = %p",
		 subg_tif_ptr(subg), subg_leaf_ptr(subg),
		 subg_ans_root_ptr(subg),
		 subg_ans_list_ptr(subg), subg_ans_list_tail(subg),
		 subg_next_subgoal(subg), subg_prev_subgoal(subg),
		 subg_cp_ptr(subg));
      fprintf(where,"  pos_cons = %p,", subg_pos_cons(subg));
      fprintf(where,"  compl_stk_ptr = %p,  compl_susp_ptr = %p,"
		 "  nde_list = %p",
		 subg_compl_stack_ptr(subg), subg_compl_susp_ptr(subg),
		 subg_nde_list(subg));
      if ( IsSubProdSF(subg) ) {
	fprintf(where,"  consumers = %p", subg_consumers(subg));
	for ( cons = subg_consumers(subg);  IsNonNULL(cons);
	      cons = conssf_consumers(cons) )
	  fprintf(where,"Consumer  %p\n"
		     "  sf_type = %11s,  tif_ptr = %p,         leaf_ptr = %p\n"
		     "  producer = %10p,  ans_list_ptr = %p,"
		     "  ans_list_tail = %p\n"
		     "  ts = %ul,  consumers = %p",
		  cons, stringSubgoalFrameType(subg_sf_type(cons)), subg_tif_ptr(cons),
		  subg_leaf_ptr(cons), conssf_producer(cons),
		  subg_ans_list_ptr(cons), subg_ans_list_tail(cons),
		  (int) conssf_timestamp(cons), conssf_consumers(cons));
      }
      subg = subg_next_subgoal(subg);
      if (subg != NULL)
	fprintf(where, EOSUBG);
      /*
      if (i == 10) {
	int dummy;
	fprintf(where, "more (y/n)?  ");
	dummy = scanf("%c", &ans);
	skip_to_nl();
	i = 0;
	} */
    }
  SYS_MUTEX_UNLOCK( MUTEX_TABLE );
  }
  fprintf(where, EOS);
  fclose(where) ;
} /* file_print_tables() */

/*----------------------------------------------------------------------*/
extern DelCFptr delcf_chain_begin;

void print_deleted_clause_frame(DelCFptr delcf) {
  printf("deleted %s frame for %s/%d: %p ref %p mark %d next_pred %p next %p\n",
	 ((delcf->type)== 0 ? "pred":"clause"),get_name(delcf->psc),get_arity(delcf->psc),
	 ((delcf->type)== 0 ? (ClRef) delcf->prref:delcf->clref),
	 delcf,delcf->mark,delcf->next_pred_delCF,delcf->next_delCF);
}

void print_delcf_chain() {

  DelCFptr delcf = delcf_chain_begin;
  printf("--------------------------------------------------------\n");
  while(delcf) {
    print_deleted_clause_frame(delcf);
    delcf = delcf->next_delCF;
  }
  printf("--------------------------------------------------------\n");
}
static void print_cell(char *addrtype, CPtr addr, Cell term, char *more_info)
{
  switch (cell_tag(term)) {
  case XSB_REF:
  case XSB_REF1:
    printf( "%s %p: XSB_REF (tag=%" Intfmt "), value=0x%p",
	    addrtype, addr, cell_tag(term), ref_val(term));
    break;
  case XSB_ATTV:
    printf( "%s %p: XSB_ATTV (tag=%" Intfmt"), value=0x%p",
	    addrtype, (CPtr)dec_addr(cell(addr)),
	    cell_tag(term), ref_val(term));
    break;
  case XSB_STRUCT:
    if (addr == (CPtr)dec_addr(term) || (CPtr)dec_addr(term) == NULL) {
      printf( "Possible source of core dump\n");
      printf( "%s %p: XSB_STRUCT, value=0x%p, hexval=0x%p",
	      addrtype, addr, cs_val(term), ref_val(term));
    }	else {
      printf( "%s %p: XSB_STRUCT, value=0x%p, hexval=0x%p (%s/%d)",
	      addrtype, addr, cs_val(term), ref_val(term),
	      get_name((struct psc_rec *) follow(cs_val(term))),
	      get_arity((struct psc_rec *) follow(cs_val(term))));
    }
    break;
  case XSB_INT:
    printf( "%s %p: XSB_INT, value=%" Intfmt  "hexval=0x%p",
	    addrtype, addr, int_val(term), ref_val(term));
    break;
  case XSB_STRING:
    if (isnil(term))
      printf( "%s %p: XSB_STRING, hexval=0x%p\t ([])",
	      addrtype, addr, ref_val(term));
    else
      printf( "%s %p: XSB_STRING, hexval=0x%p\t (%s)",
	      addrtype, addr, ref_val(term), string_val(term));
    break;
  case XSB_FLOAT:
    printf( "%s %p: XSB_FLOAT, value=%f, hexval=0x%lx",
	    addrtype, addr, float_val(term), (unsigned long) dec_addr(term));
    break;
  case XSB_LIST:
    printf( "%s %p: XSB_LIST, clref=%p, hex=%p",
	    addrtype, addr, clref_val(term), ref_val(term));
    break;
  default:
    printf( "%s %p: tag=%" Intfmt ", hex=0x%p, cval=%" Intfmt,
	    addrtype, addr, cell_tag(term), ref_val(term), int_val(term));
    break;
  }

  if (more_info != NULL)
    printf( ",\t(%s)\n", more_info);
  else printf( "\n");
}

//---------------------------------------------------------------

void check_trail_ok(CTXTdecl) /* trail grows up */
{
  CPtr *i;

  for (i = (top_of_trail); (i > (CPtr *)tcpstack.low + 1); i = i-3)     {
    if (!in_heap(cell(CPtr)(i-2)) && !in_localstack(cell((CPtr)(i-2)))) {
      printf("!!! PROBLEM ");
      print_cell("          Trail variab ", (CPtr)(i-2), cell((CPtr)(i-2)), NULL);
    }
  }
}

void check_backtrack_regs(CTXTdecl) {
  if (ebreg != cp_ereg(breg)) {
    printf("!!! PROBLEM EBREG %p cp_ereg %p",ebreg,cp_ereg(breg));
  }
  if (hbreg != cp_ereg(breg)) {
    printf("!!! PROBLEM HBREG %p cp_ereg %p",hbreg,cp_hreg(breg));
  }
}

void check_stack_invariants(CTXTdecl)
{
  check_backtrack_regs(CTXT);
  check_trail_ok(CTXT);
}

//---------------------------------------------------------------

void quick_print_trail(CTXTdecl)	/* trail grows up */
{

  CPtr *i;

  for (i = (top_of_trail); (i > (CPtr *)tcpstack.low + 1); i = i-3)     {
    printf("-------------------------------------------------------------------\n");
    if ( i == trreg ) printf("trreg");if ( i == trfreg ) printf("trfreg ");
    if ((CPtr *)cell(CPtr)(i) != i-3) printf("*");
    print_cell("          Trail parent ", (CPtr)(i), cell((CPtr)(i)), NULL);
    print_cell("          Trail value  ", (CPtr)(i-1), cell((CPtr)(i-1)), NULL);
    if (!in_heap(cell(CPtr)(i-2)) && !in_localstack(cell((CPtr)(i-2))))
      printf("!!! PROBLEM ");
    print_cell("          Trail variab ", (CPtr)(i-2), cell((CPtr)(i-2)), NULL);

  }
  printf("trail bottom \n");
}

static void fprint_cp_cell(FILE * output,char *addrtype, CPtr addr, Cell term)
{
  if ((ref_val(term) != NULL) && (cell_tag(term) == term)) {
    fprintf(output, "NULL cell in %s %p: tag=%"UIntfmt", value=0x%p\n",
	    addrtype, addr, (UInteger) cell_tag(term), ref_val(term));
  } else {
    switch (cell_tag(term)) {
    case XSB_REF:
    case XSB_REF1:
      fprintf(output, "%s %p: XSB_REF (tag=%"UIntfmt", value=0x%p\n",
	      addrtype, addr, cell_tag(term), ref_val(term));
      break;
    case XSB_ATTV:
      fprintf(output, "%s %p: XSB_ATTV (tag=%"UIntfmt", value=0x%p\n",
	      addrtype, (CPtr)dec_addr(cell(addr)),cell_tag(term), ref_val(term));
      break;
    case XSB_STRUCT:
            fprintf(output, "%s %p: XSB_STRUCT, value=0x%p, hexval=0x%p (%s/%d)\n",
	      addrtype, addr, cs_val(term), ref_val(term),
	      get_name((struct psc_rec *) follow(cs_val(term))),
	      get_arity((struct psc_rec *) follow(cs_val(term))));

	    /*      fprintf(output, "%s %p: XSB_STRUCT, value=0x%p, hexval=0x%p \n",
		    addrtype, addr, cs_val(term), ref_val(term));*/
      break;
    case XSB_INT:
      fprintf(output, "%s %p: XSB_INT, value=%" Intfmt ", hexval=0x%p\n",
	      addrtype, addr, int_val(term), ref_val(term));
      break;
    case XSB_STRING:
      fprintf(output, "%s %p: XSB_STRING, hexval=0x%p (%s)\n",
	      addrtype, addr, ref_val(term), string_val(term));
      break;
    case XSB_FLOAT:
      fprintf(output, "%s %p: XSB_FLOAT, value=%f, hexval=0x%lx\n",
	      addrtype, addr, float_val(term), (unsigned long) dec_addr(term));
      break;
    case XSB_LIST:
      fprintf(output, "%s %p: XSB_LIST, value=%p\n",
	      addrtype, addr, ref_val(term));
      break;
    default:
      fprintf(output, "%s %p: tag=%"UIntfmt", value=0x%p\n",
	      addrtype, addr, cell_tag(term), ref_val(term));
      break;
    }
  }
}

void print_cp_cell(char *addrtype, CPtr addr, Cell term)
{
  fprint_cp_cell(stddbg,addrtype, addr, term);
}


/*
 * Local Stack grows from high to low memory.
 */
void print_local_stack_nonintr(CTXTdeclc char * string)
{
  int i;
  CPtr cell_ptr,
    local_stack_bottom = (CPtr) glstack.high;
  char buf[100] ;
  FILE * where;

  sprintf(buf,"LS%d",alt_printnum) ;
  where = fopen(buf,"w") ;

  fprintf(where,"%s\n",string);
  if (ereg_on_top(ereg)) {
    cell_ptr = ereg;
    fprintf(where, "ereg on top\n");
  }
  else {
    cell_ptr = ebreg;
    fprintf(where, "ebreg on top\n");
  }
  for (i = -20; i < 0; i++) {
    if ( cell_ptr+i == efreg ) fprintf(where, "efreg\n");
    fprint_cp_cell(where,"Local Stack", cell_ptr+i, cell(cell_ptr+i));
  }
  fprintf(where, "top\n");

  for (i=0; (cell_ptr < local_stack_bottom); i++) {
    if (cell_ptr == ebreg) fprintf(where, "ebreg\n");
    if (cell_ptr == ereg)  fprintf(where, "ereg\n");
    if (cell_ptr == efreg) fprintf(where, "efreg\n");
    fprint_cp_cell(where,"Local Stack", cell_ptr, cell(cell_ptr));
    cell_ptr++;
    }
    fprintf(where, EOS);
    fclose(where) ;
}

/*----------------------------------------------------------------------*/

void terry_print_heap(CTXTdeclc char * string)	/* Heap grows up */
{
  CPtr topofheap;
  CPtr cell_ptr;
  CPtr heap_bottom = (CPtr)(glstack.low);
  char buf[100] ;
  FILE * where;

  sprintf(buf,"HEAP%d",alt_printnum) ;
  where = fopen(buf,"w") ;

  if (hfreg > hreg) topofheap = hfreg; else topofheap = hreg;
  if (hbreg > topofheap) topofheap = hbreg;

  for (cell_ptr = topofheap; cell_ptr >= heap_bottom; cell_ptr--) {
    if (cell_ptr == hreg) fprintf(where,"hreg\n");
    if (cell_ptr == hfreg) fprintf(where,"hbreg\n");
    if (cell_ptr == hbreg) fprintf(where,"hfreg\n");
    fprint_cp_cell(where,"Heap", cell_ptr, cell(cell_ptr));
  }
  fprintf(where, EOS);
  fclose(where) ;
}

void terry_print_stacks(CTXTdeclc char * string) {
  terry_print_heap(CTXTc string);
  print_local_stack_nonintr(CTXTc string);
  alt_printnum++;
}

/*======================================================================*/
/*  The final set of routines should be useful with the instruction-    */
/*  level debugger.  This can be useful if you're adding a bunch of new */
/*  instructions, but it hasnt been used for years, so it would need    */
/*  some work to get it back into shape.  These routines should be      */
/*  defined only with DEBUG_VM                                          */
/*======================================================================*/

#ifdef DEBUG_VM
extern int  xctr;

int pil_step = 1;
int compl_step = 0;
int debug_ctr = 0;
int print_hide = 0;
int memory_watch_flag = 0;
int register_watch_flag = 0;
#endif

#ifdef DEBUG_VM
CPtr decode_ptr(Cell cell) {
  return ( clref_val(cell) );
}

int decode_int(Cell cell) {
  return ( int_val(cell) );
}

int decode_tag(Cell cell) {
  return ( cell_tag(cell) );
}

/*----------------------------------------------------------------------*/

void print_help(void)
{
      fprintf(stddbg, "\n      a r/v/d/a <addr>: inspect the content of the address");
      fprintf(stddbg, "\n      b <module> <name> <arity>: spy the predicate");
      fprintf(stddbg, "\n      B <num>: print detailed Prolog choice points from the top");
      fprintf(stddbg, "\n\tof the choice point stack with <num>-Cell overlap");
      fprintf(stddbg, "\n      c <num>: print top of choice point stack with <num> overlap");
      fprintf(stddbg, "\n      C <num>: print choice point stack (around bfreg) with <num> overlap");
      fprintf(stddbg, "\n      d: print disassembled code for module");
      fprintf(stddbg, "\n      D: print current value of delay list (pointed by delayreg)");
      fprintf(stddbg, "\n      e <size>: expand trail/cp stack to <size> K-byte blocks");
      fprintf(stddbg, "\n      E <num>: print top of environment (local) stack with <num> overlap");
      fprintf(stddbg, "\n      g: leap to the next check_complete instruction");
      fprintf(stddbg, "\n      G: same as 'g', but does not print intermediate info");
      fprintf(stddbg, "\n      h: help");
      fprintf(stddbg, "\n      H <num>: print top of heap with <num> overlap");
      fprintf(stddbg, "\n      k <int>: print and skip <int> instructions");
      fprintf(stddbg, "\n      K <int>: skip <int> instructions");
      fprintf(stddbg, "\n      l: leap to the next spy point");
      fprintf(stddbg, "\n      L: same as 'l', but does not print intermediate info");
      fprintf(stddbg, "\n      M: print statistics");
      fprintf(stddbg, "\n      n: leap to the next call");
      fprintf(stddbg, "\n      N: nodebugging, continue to the end");
      fprintf(stddbg, "\n      o: print completion stack");
      fprintf(stddbg, "\n      P: print PDLSTK");
      fprintf(stddbg, "\n      q: quit XSB");
      fprintf(stddbg, "\n      r <num>: print register <num> as term");
      fprintf(stddbg, "\n      R <num>: print register <num> as ptr");
      fprintf(stddbg, "\n      S: print status registers");
      fprintf(stddbg, "\n      T <num>: print top of trail with <num> overlap");
      fprintf(stddbg, "\n      u <name> <arity>: unspy the predicate");
      fprintf(stddbg, "\n      w <stack> <val>: watch <stack> register for <val>");
      fprintf(stddbg, "\n      W <stack> <val>: watch memory area of <stack> for <val>");
      fprintf(stddbg, "\n      1: print top of (persistent) subgoal stack");
      fprintf(stddbg, "\n      2 <num>: print val of table pointer");
      fprintf(stddbg, "\n      ?: help");
      fprintf(stddbg, "\n");
}

/*--------------------------------------------------------------------------*/

/*----- For table debugging --------------------------------------------*/

/*** already defined above ???
static char *compl_stk_frame_field[] = {
  "subgoal_ptr", "level_num",
  "del_ret_list", "visited",
#ifndef LOCAL_EVAL
"DG_edges", "DGT_edges"
#endif
};

void print_completion_stack(CTXTdecl)
{
  int i = 0;
  EPtr eptr;
  VariantSF subg;
  CPtr temp = openreg;

  fprintf(stddbg,"openreg -> ");
  while (temp < COMPLSTACKBOTTOM) {
    if ((i % COMPLFRAMESIZE) == 0) {
      fprintf(stddbg,EOFR);
      subg = (VariantSF) *temp;
      print_subg_header(stddbg,subg);
    }
    fprintf(stddbg,"Completion Stack %p: %lx\t(%s)",
	    temp, *temp, compl_stk_frame_field[(i % COMPLFRAMESIZE)]);
    if ((i % COMPLFRAMESIZE) >= COMPLFRAMESIZE-2) {
      for (eptr = (EPtr)*temp; eptr != NULL; eptr = next_edge(eptr)) {
	fprintf(stddbg," --> %p", edge_to_node(eptr));
      }
    }
    fprintf(stddbg,"\n");
    temp++; i++;
  }
  fprintf(stddbg, EOS);
}
*********************/
/*----------------------------------------------------------------------*/

static void print_pdlstack(CTXTdecl)
{
  CPtr temp = pdlreg;

  while (temp <= (CPtr)(pdl.high) - 1) {
    xsb_dbgmsg((LOG_DEBUG,"pdlstk %p: %lx", temp, *temp));
    temp++;
  }
}

/*----------------------------------------------------------------------*/

extern void dis(xsbBool);
extern byte *print_inst(FILE *, byte *);

struct watch_struct {
  Integer  heap_flag;
  CPtr heap_val;
  Integer  stack_flag;
  CPtr stack_val;
  Integer  choice_flag;
  CPtr choice_val;
  Integer  trail_flag;
  CPtr trail_val;
} reg_watch, mem_watch;

/*======================================================================*/
static void set_register_watch(int num1, CPtr num2)
{
  register_watch_flag = 1;
  switch (num1) {
  case 1:
    reg_watch.heap_flag = 1;
    reg_watch.heap_val = num2;
    break;
  case 2:
    reg_watch.stack_flag = 1;
    reg_watch.stack_val = num2;
    break;
  case 3:
    reg_watch.choice_flag = 1;
    reg_watch.choice_val = num2;
    break;
  case 4:
    reg_watch.trail_flag = 1;
    reg_watch.trail_val = num2;
    break;
  }
}

/*----------------------------------------------------------------------*/

static void set_memory_watch(Integer num1, int num2)
{
  memory_watch_flag = 1;
  switch (num1) {
  case 1:
    mem_watch.heap_flag = num2;
    mem_watch.heap_val = *(CPtr *) num2;
    break;
  case 2:
    mem_watch.stack_flag = num2;
    mem_watch.stack_val = *(CPtr *) num2;
    break;
  case 3:
    mem_watch.choice_flag = num2;
    mem_watch.choice_val = *(CPtr *) num2;
    break;
  case 4:
    mem_watch.trail_flag = num2;
    mem_watch.trail_val = *(CPtr *) num2;
    break;
  }
}

/*----------------------------------------------------------------------*/

static void monitor_register_watch(CTXTdecl)
{
  if (reg_watch.heap_flag)
    if (reg_watch.heap_val == hreg)
      xsb_dbgmsg((LOG_DEBUG,"!!! hreg == %p, %d", hreg, xctr));
  if (reg_watch.stack_flag)
    if (reg_watch.stack_val == ereg)
      xsb_dbgmsg((LOG_DEBUG,"!!! ereg == %p, %d", ereg, xctr));
  if (reg_watch.choice_flag)
    if (reg_watch.choice_val == breg)
      xsb_dbgmsg((LOG_DEBUG,"!!! breg == %p, %d", breg, xctr));
  if (reg_watch.trail_flag)
    if ((CPtr *) reg_watch.trail_val == trreg)
      xsb_dbgmsg((LOG_DEBUG,"!!! trreg == %p, %d", trreg, xctr));
}

/*----------------------------------------------------------------------*/

static void monitor_memory_watch(void)
{
  if (mem_watch.heap_flag)
    if (*(CPtr *) mem_watch.heap_flag != mem_watch.heap_val) {
      xsb_dbgmsg((LOG_DEBUG,"Heap watch val %x was %p is now %lx, xctr %d",
		 mem_watch.heap_flag, mem_watch.heap_val,
		 *(CPtr) mem_watch.heap_flag, xctr));
      debug_ctr = 0;
      mem_watch.heap_val = *(CPtr *) mem_watch.heap_flag;
    }
  if (mem_watch.stack_flag)
    if (*(CPtr *) mem_watch.stack_flag != mem_watch.stack_val) {
      xsb_dbgmsg((LOG_DEBUG,"Stack watch val %x was %p is now %lx, xctr %d",
		 mem_watch.stack_flag,mem_watch.stack_val,
		 *(CPtr) mem_watch.stack_flag,xctr));
      debug_ctr = 0;
      mem_watch.stack_val = *(CPtr *) mem_watch.stack_flag;
    }
  if (mem_watch.choice_flag)
    if (*(CPtr *) mem_watch.choice_flag != mem_watch.choice_val) {
      xsb_dbgmsg((LOG_DEBUG,"Choice watch val %x was %p is now %lx, xctr %d",
		 mem_watch.choice_flag,mem_watch.choice_val,
		 *(CPtr) mem_watch.choice_flag,xctr));
      debug_ctr = 0;
      mem_watch.choice_val = *(CPtr *) mem_watch.choice_flag;
    }
  if (mem_watch.trail_flag)
    if (*(CPtr *) mem_watch.trail_flag != mem_watch.trail_val) {
      xsb_dbgmsg((LOG_DEBUG,"Trail watch val %x was %p is now %lx, xctr %d",
		 mem_watch.trail_flag,mem_watch.trail_val,
		 *(CPtr) mem_watch.trail_flag,xctr));
      debug_ctr = 0;
      mem_watch.trail_val = *(CPtr *) mem_watch.trail_flag;
    }
}
void debug_inst(CTXTdeclc byte *lpcreg, CPtr le_reg)
{
  if (!print_hide) {
    fprintf(stddbg, "\nxctr %d ",xctr);
    printf(" (intFlg:%x)",asynint_val);
    print_inst(stddbg, lpcreg);
  }
  if (register_watch_flag) monitor_register_watch(CTXT);
  if (memory_watch_flag) monitor_memory_watch();
  if (pil_step && debug_ctr == 0) {
    print_hide = 0;
    pcreg = lpcreg; ereg = le_reg;
    debug_interact(CTXT);
  } else {
    if (debug_ctr > 0) debug_ctr--;
    else
      if (call_step_gl == 1 && *lpcreg == call) {
	pil_step = 1; debug_interact(CTXT);
      }
    if (compl_step == 1 && *lpcreg == check_complete) {
      pil_step = 1; debug_interact(CTXT);
    }
  }
}

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

static void print_trail(CTXTdeclc int overlap)	/* trail grows up */
{
  int  i, offset=0;
  char ans = 'y';
  CPtr *temp;

  if (trfreg > trreg) temp = trfreg;  else temp = trreg;
  for (i = overlap; (i > 0); i--)
    {
      if ( (temp + i) == trreg ) xsb_dbgmsg((LOG_DEBUG,"trreg"));
      if ( (temp + i) == trfreg ) xsb_dbgmsg((LOG_DEBUG,"trfreg"));
      print_cell("Trail", (CPtr)(temp+i), cell((CPtr)(temp+i)), NULL);
    }
  while (ans == 'y' && temp-offset >= (CPtr *) tcpstack.low) {
    for (i = 0
	   ; (i <= STRIDESIZE && temp-(offset+i) >= (CPtr *)tcpstack.low)
	   ; i++ )      {
      if ( (temp - (offset+i)) == trreg ) xsb_dbgmsg((LOG_DEBUG,"trreg"));
      if ( (temp - (offset+i)) == trfreg ) xsb_dbgmsg((LOG_DEBUG,"trfreg"));
      print_cell("Trail", (CPtr)(temp-(offset+i)),
		 cell((CPtr)(temp-(offset+i))), NULL);
      if ( (temp-(offset+i)) == (CPtr *) tcpstack.low )
	xsb_dbgmsg((LOG_DEBUG,"bottom"));
    }
    offset += STRIDESIZE;
    fprintf(stddbg, "more (y/n)?  ");
    scanf("%c", &ans);
    skip_to_nl();
  }
}

/*----------------------------------------------------------------------*/

static void print_freeze_choice_points(CTXTdeclc int overlap)	/* CPs grow down */
{
  int i,last = 0;
  char ans = 'y';

  for (i = -overlap; i < 0 ; i++) {
    print_cp_cell("CP stack", bfreg+i, cell(bfreg+i));
    if ( (bfreg + i) == breg ) xsb_dbgmsg((LOG_DEBUG,"breg"));
  }
  xsb_dbgmsg((LOG_DEBUG, "bfreg"));
  for (i = 0; (i <= STRIDESIZE && bfreg+i<=(CPtr)tcpstack.high); i++){
    if ( (bfreg + i) == breg ) xsb_dbgmsg((LOG_DEBUG, "breg"));
    print_cp_cell("CP stack", bfreg+i, cell(bfreg+i));
    if ( (bfreg + i) == (CPtr) tcpstack.high ) fprintf(stddbg, EOS);
  }
  fprintf(stddbg, "more (y/n)?  ");
  scanf("%c", &ans);
  skip_to_nl();
  while (ans == 'y' && bfreg+last < (CPtr) tcpstack.high ) {
    last = last+STRIDESIZE;
    for ( i = last
	    ; (i <= last+STRIDESIZE && bfreg+i <= (CPtr) tcpstack.high)
	    ; i++ ) {
      if ( (bfreg + i) == breg ) xsb_dbgmsg((LOG_DEBUG, "breg"));
      print_cp_cell("CP stack", bfreg+i, cell(bfreg+i));
      if ( (bfreg + i) == (CPtr) tcpstack.high ) fprintf(stddbg, EOS);
    }
    fprintf(stddbg, "more (y/n)?  ");
    scanf("%c", &ans);
    skip_to_nl();
  }
}

/*----------------------------------------------------------------------*/

/*
 * Choice point stack grows from high to low memory.
 */

static void print_cpfs(CTXTdeclc int overlap)
{
  int i, frames = 2;
  char ans = 'y';
  CPtr  cpf,            /* ptr to choice point frame */
    cp_stack_bottom = (CPtr) tcpstack.high;
  int  length,          /* needed to increment the stack pointer, hence
                           analyze_cpf must supply it */
    type;

  for (i = -overlap ; (i < 0) ; i++) {
    if ((breg+i) == bfreg) xsb_dbgmsg((LOG_DEBUG,"bfreg"));
    print_cp_cell("   CP stack", breg+i, cell(breg+i));
  }
  xsb_dbgmsg((LOG_DEBUG,"breg"));
  cpf = breg;
  do {
    for (i = 0; (i < frames) && (cpf < cp_stack_bottom); i++) {
      if ( cpf == bfreg )
	xsb_dbgmsg((LOG_DEBUG,"bfreg"));
      //      analyze_cpf(cpf, &length, &type);  // not defined...
      //      print_cpf(CTXTc cpf, length, type);  // wrong num of args, figure out later
      cpf = cpf + length;
    }
    if (cpf < cp_stack_bottom) {
      fprintf(stddbg, "more (y/n)?  ");
      scanf("%c", &ans);
      skip_to_nl();
    }
    else {
      fprintf(stddbg, EOS);
      ans = 'n';
    }
  } while (ans == 'y');
}

/*----------------------------------------------------------------------*/

static void print_choice_points(CTXTdeclc int overlap)
{
  int i, last = 0;
  char ans = 'y';
  CPtr cp_stack_bottom = (CPtr)tcpstack.high;

  for (i = -overlap ; (i < 0) ; i++) {
    if ((breg+i) == bfreg) xsb_dbgmsg((LOG_DEBUG,"bfreg"));
    print_cp_cell("CP stack", breg+i, cell(breg+i));
  }
  xsb_dbgmsg((LOG_DEBUG,"breg"));
  do {
    for (i = last;
	 (i <= last + STRIDESIZE) && (breg+i <= cp_stack_bottom);
	 i++) {
      if ( (breg + i) == bfreg ) xsb_dbgmsg((LOG_DEBUG,"bfreg"));
      print_cp_cell("CP stack", breg+i, cell(breg+i));
      if ( (breg + i) == cp_stack_bottom ) fprintf(stddbg, EOS);
    }
    if (breg+i < cp_stack_bottom) {
      last = last + STRIDESIZE;
      fprintf(stddbg, "more (y/n)?  ");
      scanf("%c", &ans);
      skip_to_nl();
    }
    else
      ans = 'n';
  } while (ans == 'y');
}

/*----------------------------------------------------------------------*/

/* Needs to change when new xwam stacks are introduced.  */
static void print_heap(int overlap)	/* Heap grows up */
{
  int i, offset = 0;
  char ans = 'y';

  for (i = overlap; (i > 0); i--) {
    print_cell("Heap", hreg+i, cell(hreg+i), NULL);
  }
  xsb_dbgmsg((LOG_DEBUG,"hreg"));
  while (ans == 'y' && hreg-i > (CPtr) glstack.low) {
    for (i = 0
	   ;(i <= STRIDESIZE && hreg-(offset+i) >= (CPtr) glstack.low)
	   ; i++) {
      if ( (hreg - (offset+i)) == hfreg ) xsb_dbgmsg((LOG_DEBUG,"hfreg"));
      if ( (hreg - (offset+i)) == hbreg ) xsb_dbgmsg((LOG_DEBUG,"hbreg"));
      print_cell("Heap", hreg-(offset+i), cell(hreg-(offset+i)), NULL);
      if ( (hreg-(offset+i)) == (CPtr) glstack.low )
	xsb_dbgmsg((LOG_DEBUG,"bottom"));
    }
    if ( (hreg-(offset+i)) != (CPtr) glstack.low ) {
      offset += STRIDESIZE;
      fprintf(stddbg, "more (y/n)?  ");
      scanf("%c", &ans);
      skip_to_nl();
    }
  }
}

static void print_status(CTXTdecl)
{
  xsb_dbgmsg((LOG_DEBUG,"     ereg: 0x%p", ereg));
  xsb_dbgmsg((LOG_DEBUG,"    ebreg: 0x%p", ebreg));
  xsb_dbgmsg((LOG_DEBUG,"     breg: 0x%p", breg));
  xsb_dbgmsg((LOG_DEBUG,"     hreg: 0x%p", hreg));
  xsb_dbgmsg((LOG_DEBUG,"    hbreg: 0x%p", hbreg));
  xsb_dbgmsg((LOG_DEBUG,"    trreg: 0x%p", trreg));
  xsb_dbgmsg((LOG_DEBUG,"    cpreg: 0x%p", cpreg));
  xsb_dbgmsg((LOG_DEBUG,"    pcreg: 0x%p", pcreg));

  xsb_dbgmsg((LOG_DEBUG,"    efreg: 0x%p", efreg));
  xsb_dbgmsg((LOG_DEBUG,"    bfreg: 0x%p", bfreg));
  xsb_dbgmsg((LOG_DEBUG,"    hfreg: 0x%p", hfreg));
  xsb_dbgmsg((LOG_DEBUG,"   trfreg: 0x%p", trfreg));
  xsb_dbgmsg((LOG_DEBUG,"   pdlreg: 0x%p", pdlreg));
  xsb_dbgmsg((LOG_DEBUG,"  ptcpreg: 0x%p", ptcpreg));
  xsb_dbgmsg((LOG_DEBUG," delayreg: 0x%p", delayreg));
  xsb_dbgmsg((LOG_DEBUG,"neg_delay: %s", (neg_delay == FALSE) ? "False" : "True"));
  xsb_dbgmsg((LOG_DEBUG,"   level#: %d", level_num));

  xsb_dbgmsg((LOG_DEBUG,"\nPDL"));
  xsb_dbgmsg((LOG_DEBUG,"\tlow:       %p", pdl.low));
  xsb_dbgmsg((LOG_DEBUG,"\thigh:      %p", pdl.high));
  xsb_dbgmsg((LOG_DEBUG,"\tsize:      %" Intfmt, pdl.size)); /* JF: long */
  xsb_dbgmsg((LOG_DEBUG,"\tinit size: %" Intfmt, pdl.init_size)); /* JF: long */

  xsb_dbgmsg((LOG_DEBUG,"\nGlobal / Local Stack"));
  xsb_dbgmsg((LOG_DEBUG,"\tlow:       %p", glstack.low));
  xsb_dbgmsg((LOG_DEBUG,"\thigh:      %p", glstack.high));
  xsb_dbgmsg((LOG_DEBUG,"\tsize:      %" Intfmt, glstack.size)); /* JF: long */
  xsb_dbgmsg((LOG_DEBUG,"\tinit size: %" Intfmt, glstack.init_size)); /* JF: long */

  xsb_dbgmsg((LOG_DEBUG,"\nTrail / Choice Point Stack"));
  xsb_dbgmsg((LOG_DEBUG,"\tlow:       %p", tcpstack.low));
  xsb_dbgmsg((LOG_DEBUG,"\thigh:      %p", tcpstack.high));
  xsb_dbgmsg((LOG_DEBUG,"\tsize:      %" Intfmt, tcpstack.size)); /* JF: long */
  xsb_dbgmsg((LOG_DEBUG,"\tinit size: %" Intfmt, tcpstack.init_size)); /* JF: long */

  xsb_dbgmsg((LOG_DEBUG,"\nCompletion Stack"));
  xsb_dbgmsg((LOG_DEBUG,"\tlow:       %p", complstack.low));
  xsb_dbgmsg((LOG_DEBUG,"\thigh:      %p", complstack.high));
  xsb_dbgmsg((LOG_DEBUG,"\tsize:      %" Intfmt, complstack.size)); /* JF: long */
  xsb_dbgmsg((LOG_DEBUG,"\tinit size: %" Intfmt, complstack.init_size)); /* JF: long */
}

static void debug_interact(CTXTdecl)
{
  char command, mod[32], name[32];
  Integer num, num1;
  Pair sym;

 again:
  fprintf(stddbg, "\n > ");
  fflush(stddbg);
  scanf("%c", &command);
  switch (command) {
  case 'a':
    scanf("%s %x", name, &num);
    skip_to_nl();
    switch (name[0]) {
    case 'a':
      xsb_dbgmsg((LOG_DEBUG,"0x%x: 0x%x", num, *(Integer *)num));
      break;
    case 'r':
      print_cell("Reg", (CPtr)num, cell(reg+num), NULL);
      break;
    case 'v':
      print_cell("Var", (CPtr)num, cell(ereg-num), NULL);
      break;
    case 'd':
      print_cell("Addr", (CPtr)num, cell((CPtr)(num)), NULL);
      break;
    }
    goto again;
  case 'b':
    scanf("%s %s %d", mod, name, &num);
    skip_to_nl();
    sym = insert_module(0, mod);
    sym = insert(name, num, sym->psc_ptr, &num);
    set_spy(sym->psc_ptr, 0x80);
    goto again;
  case 'B':
    scanf("%d", &num);
    skip_to_nl();
    print_cpfs(CTXTc num);
    goto again;
  case 'c':
    scanf("%d", &num);
    skip_to_nl();
    print_choice_points(CTXTc num);
    goto again;
  case 'C':
    scanf("%d", &num);
    skip_to_nl();
    print_freeze_choice_points(CTXTc num);
    skip_to_nl();
    goto again;
  case 'd':
    skip_to_nl();
    dis(1);
    goto again;
  case 'D':
    skip_to_nl();
    fprintf(stddbg, "Delay List = ");
    print_delay_list(stddbg, delayreg);
    fprintf(stddbg, "\n");
    goto again;
  case 'e':
    scanf("%d", &num);
    skip_to_nl();
    tcpstack_realloc(CTXTc num);
    goto again;
  case 'E':
    scanf("%d", &num);
    skip_to_nl();
    print_local_stack(CTXTc num);
    goto again;
  case 'g':
    skip_to_nl();
    pil_step = hitrace_suspend_gl = call_step_gl = 0;
    compl_step = 1;
    break;
  case 'G':
    skip_to_nl();
    print_hide = hitrace_suspend_gl = compl_step = 1;
    pil_step = call_step_gl = 0;
    break;
  case 'h':
  case '?':
    skip_to_nl();
    print_help();
    goto again;
  case 'H':
    scanf("%d", &num);
    skip_to_nl();
    terry_print_heap(CTXTc num);
    goto again;
  case 'k':
    scanf("%d", &num);
    skip_to_nl();
    debug_ctr = num;
    flags[PIL_TRACE] = 1;
    break;
  case 'K':
    scanf("%d", &num);
    skip_to_nl();
    debug_ctr = num;
    print_hide = flags[PIL_TRACE] = 1;
    break;
  case 'l':
    skip_to_nl();
    pil_step = call_step_gl = hitrace_suspend_gl = 0;
    break;
  case 'L':
    skip_to_nl();
    pil_step = flags[PIL_TRACE] = call_step_gl = 0;
    print_hide = hitrace_suspend_gl = 1;
    break;
  case 'M':
    skip_to_nl();
    print_statistics(CTXTc 1);
    goto again;
  case 'n':
    skip_to_nl();
    pil_step = hitrace_suspend_gl = 0;
    call_step_gl = 1;
    break;
  case 'N':
    skip_to_nl();
    pil_step = flags[PIL_TRACE] = flags[HITRACE] = call_step_gl = 0;
    print_hide = 1;
    break;
  case 'o':
    skip_to_nl();
    print_completion_stack(CTXT);
    goto again;
  case 'P':
    skip_to_nl();
    print_pdlstack(CTXT);
    goto again;
  case 'q':
  case 'x':
    xsb_exit("Debugging aborted by the user");
    break;
  case 'r':
    scanf("%d", &num);
    skip_to_nl();
    fprintf(stddbg, "Reg[%d] = ", num);
    printterm(stddbg, cell(reg+num), 8);
    fprintf(stddbg, "\n");
    fprintf(stddbg, "%lx\n",*(reg+num));
    goto again;
  case 'R':
    scanf("%d", &num);
    skip_to_nl();
    fprintf(stddbg, "Reg[%d] = %lx\n",num,*(reg+num));
    goto again;
  case 's':
    skip_to_nl();
    pil_step = 1;
    flags[PIL_TRACE] = 1;
    hitrace_suspend_gl = 0;
    break;
  case 'S':
    skip_to_nl();
    print_status(CTXT);
    goto again;
  case 'T':
    scanf("%d", &num);
    skip_to_nl();
    print_trail(CTXTc num);
    goto again;
  case 'u':
    scanf("%s %s %d", mod, name, &num);
    skip_to_nl();
    sym = insert_module(0, mod);
    sym = insert(name, num, sym->psc_ptr, &num);
    set_spy(sym->psc_ptr, 0x00);
    goto again;
  case 'v':
    scanf("%d", &num);
    skip_to_nl();
    fprintf(stddbg, "Var[%d] = ", num);
    printterm(stddbg, cell(ereg-num), 8);
    fprintf(stddbg, "\n");
    goto again;
  case 'w':
    scanf("%d %x", &num1, &num);
    skip_to_nl();
    set_register_watch(num1, (CPtr)num);
    goto again;
  case 'W':
    scanf("%x %x", &num1, &num);
    skip_to_nl();
    set_memory_watch(num1, num);
    goto again;
  case '1':
    skip_to_nl();
    print_tables();
    goto again;
  case '2':
    scanf("%d",&num);
    skip_to_nl();
    fprintf(stddbg, "tabptr: 0x%p tabptrval: 0x%lx\n",
	    ((CPtr) (pdl.low)) + num,
	    *(((CPtr) (pdl.low)) + num));
    goto again;
  case '\n':
    break;
  default:
    skip_to_nl();
    fprintf(stddbg, "Unknown command\n");
    goto again;
  }
  return;
}

#endif /* DEBUG_VM */
