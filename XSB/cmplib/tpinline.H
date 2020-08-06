/* File:      tpinline.H
** Author(s): Saumya Debray, Kostis Sagonas, Terrance Swift
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
** $Id: tpinline.H,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



:- export force_tp_geninline_loaded/0. 
:- export geninline/9, geninl_load_lhs/7.

%%:- import sym_name/3 from symtab.
%%:- import error/1, message/1 from auxlry.
:- import append/3 from basics.
:- import tgetpar/6, tputpar/7 from tp_goal.
:- import term_comp_op/1, term_comp_tab/4 from tp_comp.
:- import vprag_occ/2, vprag_loc/2, vprag_type/2 from varproc.
:- import arithrelop/1, /*eval_exp/8,*/ eval_relop/10 from tp_eval.
:- import release/3, alloc_reg/3, release_if_done/6, hold/3, getreg/2,
	  putvarinst/5
     from tp_var.
:- import geninline_isindeed/8 from tp_eval.
:- import inline_jumpcof_cond/3 from builtin.
:- import memberchk/2 from basics.
