/* File:      cp_opt.H
** Author(s): Terrance Swift
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
** $Id: cp_opt.H,v 1.15 2012-02-01 20:06:42 dwarren Exp $
** 
*/


:- export cp_opt/2.
:- export perform_tabling_transforms/2.
:- export extract_aggr_spec/4.
:- export get_return_w_tv/4.
:- export call_join_op/6.
:- export collect_orders_from_indexes/4.

:- import nthmember/3 from listutil.
:- import have_hardcut/2 from preproc.
:- import index_split/2 from tp_index.
:- import member/2, append/3, length/2, copy_term/2 from basics.
:- import functor/3, arg/3, atom_codes/2, '=..'/2, call/1 from standard.
:- import sym_type/2, sym_name/3, sym_index/2, sym_insert/5, sym_propin/2, sym_prop/2 from symtab.
:- import warning/1 from auxlry.
:- import syntax_error/1 from error_handler.
:- import excess_vars/4 from setof.
:- import concat_atom/2 from string.
:- import trans_head/5, trans_goals/5 from parse.
:- import memberchk/2 from basics.
:- import '\='/2 from standard.
:- import xwam_state/2 from machine.
:- import parsort/4 from machine.
:- import findall/3, sort/2 from setof.
:- import concat_atom/3 from string.
:- import ith/3, member/2 from basics.
:- import ','/2 from standard.
