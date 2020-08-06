/* File:      conget_xsb_i.h
** Author(s): ??
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
** $Id: conget_xsb_i.h,v 1.10 2011-03-05 17:26:23 tswift Exp $
** 
*/

static inline Integer conget(Cell string)
{
  int value;
  Psc psc, str_psc;
  Pair sym;

  if (!isstring(string))
    xsb_abort("[CONGET] Non-string in the first argument");

  psc = (Psc)flags[CURRENT_MODULE];
  sym = insert(string_val(string), 0, psc, &value);
  str_psc = pair_psc(sym);

  if (get_type(str_psc) == T_PRED || get_type(str_psc) == T_DYNA)
    xsb_abort("[conget] Cannot get data from callable predicate %s/%d.\n",
	      get_name(str_psc),get_arity(str_psc));

  return (Integer)get_data(str_psc);
}


static inline xsbBool conset(Cell string, Integer newval)
{
  int value;
  Psc psc, str_psc;
  Pair sym;

  if (!isstring(string))
    xsb_abort("[CONSET] Non-string in the first argument");
  psc = (Psc)flags[CURRENT_MODULE];
  sym = insert(string_val(string), 0, psc, &value);
  str_psc = pair_psc(sym);

  if (get_type(str_psc) == T_PRED || get_type(str_psc) == T_DYNA)
    xsb_abort("[conget] Cannot set data of callable predicate %s/%d.\n",
	      get_name(str_psc),get_arity(str_psc));

  set_data(str_psc, (Psc) newval);
  return TRUE;
}
