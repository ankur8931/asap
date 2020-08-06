/* File:      ptoc_tag_xsb_i.h
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
** $Id: ptoc_tag_xsb_i.h,v 1.9 2010-08-19 15:03:37 spyrosh Exp $
** 
*/

#ifndef __PTOC_TAG_XSB_I_H__
#define __PTOC_TAG_XSB_I_H__

#include "binding.h"
#include "error_xsb.h"

/*
 *  Returns the still-tagged value (a Cell) at the end of the deref chain
 *  leading from `regnum'.
 */
static inline Cell ptoc_tag(CTXTdeclc int regnum)
{
  /* reg is global array in register.h */
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  return addr;
}

extern char *canonical_term(CTXTdeclc Cell, int);

/*
 *  Bind the variable pointed to by the "regnum"th argument register to the
 *  term at address "term".  Make an entry in the trail for this binding.
 */
// TLS: added static for clang
static inline  void ctop_tag(CTXTdeclc int regnum, Cell term)
{
  register Cell addr = cell(reg+regnum);

  XSB_Deref(addr);
  if (isref(addr)) {
    bind_copy(vptr(addr), term);
  }
  else
    xsb_abort("[CTOP_TAG] Argument %d of illegal type: %s",regnum,canonical_term(CTXTc addr, 0));
}

#endif
