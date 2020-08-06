/* File:      tp_eval.H
** Author(s): Saumya Debray, Kostis Sagonas
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
** $Id: tp_eval.H,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export force_tp_eval_loaded/0.
:- export eval_relop/10, eval_exp/8, arithrelop/1.
:- export geninline_isindeed/8.

:- import sym_name/3 from symtab.
:- import error/1, message/1 from auxlry.
:- import builtin_function/3 from builtin.
:- import vprag_type/2, vprag_loc/2, vprag_occ/2 from varproc.
:- import release/3, hold/3, release_if_done0/5, getreg/2, alloc_reg/3,
   putvarinst/5, release_if_done/6
     from tp_var.
:- import tgetpar/6 from tp_goal.
:- import eval/2 from eval.

