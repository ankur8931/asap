/* File:      modeinf.H
** Author(s): C.R. Ramakrishnan
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
** $Id: modeinf.H,v 1.14 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


:- export modeanalyze/3, force_modeinf_loaded/0.
:- mode modeanalyze(++, ?, ++).

:- export mode_subsumes_chk/2.

:- import
	trie_assert/1, trie_retract/1,
	trie_retract_all/1, trie_dynamic/1,
	abolish_table_pred/1, get_calls/3, get_returns/2
    from tables.

:- import assert/1, clause/2, (dynamic)/1, retractall/1 from assert.

:- import 
	abort/0, atom_codes/2, cputime/1, call/1, file_exists/1, functor/3,
	(index)/3, name/2, nl/1, read/1, repeat/0, statistics/0,
	see/1, seeing/1, seen/0, (table)/1,
	write/1, write/2, writeln/1, writeln/2, writeq/2, 
	('=..')/2, write_canonical/2, open/3, close/1
   from standard.

:- import file_time/2 from file_io.

:- import message/1, time_message/2, warning/1, option/1 from auxlry.

:- import inline_predicate/2 from builtin.

:- import findall/3 from setof.
:- import
	append/3, copy_term/2, ith/3, length/2, member/2, memberchk/2,
	select/3
   from basics.

:- import str_cat/3 from string.
:- import conset/2, conget/2 from gensym.
:- import libpath/1 from loader.

:- import '_$call_user'/1 from usermod.
:- dynamic '_$call_user'/1.
:- dynamic '_$done_before'/2.

:- import bagPO/3, bagPO1/3 from aggregs.
:- import subsumes_chk/2 from subsumes.
:- import get_maximal_calls/1, get_maximal_returns/1 from usermod.
:- import all_calls/1 from usermod.



:- import extract_symlist/3, sym_prop/2, sym_propin/2 from symtab.

%:- export prop_iff/2, prop_iff/1, prop_iff_list/2.
%:- export prop_if_list/2, prop_if/3, prop_if/2.

:- import apply/2, (:-)/2, (:-)/1 from usermod.
