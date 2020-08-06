/* File:      function.h
** Author(s): David S. Warren
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
** $Id: function.h,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/

typedef struct {
  int fitype;  // 1 if int, 0 if float
  union {
    Float valflt;
    Integer valint;
  } fival;
} FltInt;

#define set_int_val(fivar,ival) do { 		\
    fivar->fitype = 1;				\
    fivar->fival.valint = ival;			\
  } while (0)
#define set_flt_val(fivar,fval) do { 		\
    fivar->fitype = 0;				\
    fivar->fival.valflt = fval;			\
  } while (0)

#define set_and_return_fail(fivar) do {		\
    set_int_val(fivar,0);			\
    return 0;					\
  } while (0)

#define fiint_val(firec) (Integer)firec.fival.valint

#define fiflt_val(firec) (Float)firec.fival.valflt

#define isfiint(firec) firec.fitype

#define isfiflt(firec) firec.fitype == 0

int xsb_eval(CTXTdeclc Cell exp, FltInt *value);
