/* File:      file_op.H 
** Author(s): Kostis Sagonas, David S. Warren
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
** $Id: file_op.H,v 1.13 2007/02/09 18:08:56 dwarren Exp $
** 
*/


:- export '_$op'/3, add_ops/3, op_priority/1, op_specifier/1, op_type/2.
:- export add_index/3, retract_index/1, add_trie_index/1, retract_trie_index/1, 
  '_$index'/3 , '_$trie_asserted'/1.
:- export add_table/1, retract_table/1.   %%, '_$table'/1.
:- export add_incr_table/1, add_opaque/1. %% incremental evaluation 
:- export mpa_to_skel/2.
:- export force_file_op_loaded/0.

:- import default_assert_size/1 from config.
:- import integer/1, functor/3, atom/1 from standard.
:- import /*incremental evaluation */ assert/1, assertai/2, retract/1, retractall/1, (dynamic)/1 from assert.

:- import conpsc/2, psc_type/2, term_new/2, psc_insert/4 from machine.
:- import instantiation_error/2 from error_handler.

:- index '_$op1'/3-3.

/*
:- export fx/0, fy/0, yf/0, xf/0, xfx/0, xfy/0, yfx/0, yfy/0.
*/

/* --------------------- end of file file_op.H ------------------------ */
