/* File:      demand.h
** Author(s): Luis Castro
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1999
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
** $Id: demand.h,v 1.9 2011-05-18 19:21:40 dwarren Exp $
** 
*/

#include "ptoc_tag_xsb_i.h"
#include "celltags_xsb.h"
#include "cut_xsb.h"

/* debug-related macros */
#ifdef DEBUG_VERBOSE
#ifdef CP_DEBUG
#define INSPECT_CP(Type,CP) \
    fprintf(stddbg,\
            "choicepoint(type(%s),address(%p),pred(%s/%d),ptcp(%p)).\n", \
            Type, CP, ((Choice)CP)->psc->nameptr, ((Choice)CP)->psc->arity,\
            ((Choice)CP)->ptcp)
#else /* CP_DEBUG */
#define INSPECT_CP(Type,CP) \
    fprintf(stddbg,"choicepoint(type(%s),address(%p),ptcp(%p)).\n",\
            Type,CP,((Choice)CP)->ptcp)
#endif /*CP_DEBUG */
#else /* DEBUG */
#define INSPECT_CP(Type,CP)
#endif /* DEBUG */
	    

/* static code to be inlined */

static inline void perform_once(void)
{
  unsigned int arg1;
  int has_generator;
  CPtr cutpoint,cutto;
  Choice b;

  /* extracting and validating argument */
  arg1 = ptoc_int(1);
  cutpoint = (CPtr) ((size_t) tcpstack.high - arg1);
  if (cutpoint < breg) {
    xsb_abort("demand_once/1 called with Breg greater than current.");
  }

  /* traversing and analysing stack section */
  b = (Choice) breg;
  has_generator=0;
  while (b < cutpoint) {
    if (is_generator_choicepoint(b)) {
      INSPECT_CP("generator",b);
/*       has_generator=1; */
      delete_table(b);
    } else if (is_consumer_choicepoint(b)) {
      INSPECT_CP("consumer",b);
    } else if (is_compl_susp_frame(b)) {
      INSPECT_CP("compl_susp_frame",b);
    } else {
      INSPECT_CP("prolog",b);
    }
    /* invert chain */
    b = b->prev_top;
  }
  /* if there are only prolog & consumer CP's, we can throw them away */
  if (!has_generator) {
    /* perform cut */
    ebreg = cp_ebreg(cutpoint);
    hbreg = cp_hreg(cutpoint);
    if (breg != cutpoint) {
      CPtr xtemp1, xtemp2;
      unwind_trail(breg,xtemp1,xtemp2);
      breg = cutpoint;
    }
  }
}

static inline void delete_table(TChoice choicepoint)
{
  VariantSF subgoal;
  struct completion_stack_frame *csf;
  TIFptr tif;

  subgoal = (VariantSF) choicepoint->subgoal_ptr;
  csf = subgoal->compl_stack_ptr;
  tif = subgoal->tif_ptr;

  reclaim_incomplete_table_structs(subgoal);
  delete_predicate_table(tif);
  
}
