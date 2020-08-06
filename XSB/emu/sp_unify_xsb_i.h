/* File:      sp_unify_xsb_i.h
** Author(s): Kostis Sagonas
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1998
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
** $Id: sp_unify_xsb_i.h,v 1.10 2011-06-06 20:20:29 dwarren Exp $
** 
*/


/* to be used when cell op1 is known to contain an int */
static inline xsbBool int_unify(CTXTdeclc Cell op1, Cell op2)
{	/* No checking for interrupts is needed!       */
  XSB_Deref(op2);
  if (isref(op2)) {
    /* op2 is XSB_FREE:                       num ... free */
    bind_copy((CPtr)(op2), op1);
    return TRUE;
  }
  if (isointeger(op2)) {
    return (oint_val(op1) == oint_val(op2));
  } else return FALSE;
}


/* to be used when cell op1 is known to contain an atom */
static inline xsbBool atom_unify(CTXTdeclc Cell op1, Cell op2)
{	/* No checking for interrupts is needed!	*/
  XSB_Deref(op2);
  if (isref(op2)) {
    /* op2 is XSB_FREE                      string ... free */
    bind_copy((CPtr)(op2), op1);
    return TRUE;
  }
  return (op1 == op2);
}
