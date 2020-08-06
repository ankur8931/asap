/* File:      call_xsb_i.h
** Author(s): Xu, Warren, Sagonas, Swift
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
** $Id: call_xsb_i.h,v 1.18 2012-03-19 15:19:59 dwarren Exp $
** 
*/

#include "xsb_config.h"
#include <stdio.h>

#include "context.h"
#include "register.h"
#include "flags_xsb.h"
#include "auxlry.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "psc_xsb.h"

extern void intercept(CTXTdeclc Psc);

/* load argument registers with fields of a term, and set pcreg to
   term's entry point, to effect an execute */
#if !defined(WIN_NT) || defined(CYGWIN)
//inline
#endif
int prolog_call0(CTXTdeclc Cell term)
{
    Psc  psc;
    if (isconstr(term)) {
      int  disp;
      char *addr;
      psc = get_str_psc(term);
      addr = (char *)(clref_val(term));
      for (disp = 1; disp <= (int)get_arity(psc); ++disp) {
	bld_copy(reg+disp, cell((CPtr)(addr)+disp));
      }
    } else if (isstring(term)) {
      int  value;
      Pair sym;
      if (string_val(term) == true_string) return TRUE; /* short-circuit if calling "true" */
      sym = insert(string_val(term),0,(Psc)flags[CURRENT_MODULE],&value);
      psc = pair_psc(sym);
    } else {
      if (isnonvar(term))
	xsb_type_error(CTXTc "callable",term,"call/1",1);
      else xsb_instantiation_error(CTXTc "call/1",1);
      return FALSE;
    }
#ifdef CP_DEBUG
    pscreg = psc;
#endif
    pcreg = get_ep(psc);
    if (asynint_val) intercept(CTXTc psc);
    return TRUE;
}

/* fill argument registers with subfields of a term, to prepare for an
   execute */
#if !defined(WIN_NT) || defined(CYGWIN)
//inline
#endif
int prolog_code_call(CTXTdeclc Cell term, int value)
{
  Psc  psc;
  if (isconstr(term)) {
    int  disp;
    char *addr;
    psc = get_str_psc(term);
    addr = (char *)(clref_val(term));
    for (disp = 1; disp <= (int)get_arity(psc); ++disp) {
      bld_copy(reg+disp, cell((CPtr)(addr)+disp));
    }
    bld_int(reg+get_arity(psc)+1, value);
  } else bld_int(reg+1, value);
  return TRUE;
}
