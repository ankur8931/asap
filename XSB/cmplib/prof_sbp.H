/* File:      prof_sbp.H 
** Author(s): Kostis F, Sagonas, Jiyang Xu 
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
** $Id: prof_sbp.H,v 1.10 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export prepare/0, name_append/3.
:- export gensym/2, gennum/1, myclose/1, write_int/1, write_float/1.
:- export copyterm/2, conlength/2.
:- export conset/2, conget/2, coninc/1.
:- export newcounter/1, inccounter/1, getcounter/1.
:- export ll_arg/3.

:- import dict_lookup/3 from auxlry.
:- import append/3 from basics.
:- import psc_set_prop/2, conpsc/2, conname/2,psc_prop/2, str_len/2,
 	  stat_flag/2, file_puttoken/3, psc_set_type/2
     from machine.
:- import name/2, arg/3, functor/3, atomic/1, 
	  integer/1, atom_codes/2
     from standard.
:- import obsolete/2 from obsolete.
