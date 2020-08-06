/* File:      modes.H
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
** $Id: modes.H,v 1.16 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


:- export ma/2.
:- mode ma(++,++).

:- import
	trie_assert/1, trie_retract/1,
	trie_retract_all/1, trie_dynamic/1,
	abolish_table_pred/1, get_calls/3, get_returns/2
    from tables.

:- import assert/1, clause/2, (dynamic)/1, retractall/1 from assert.

:- import 
	abort/0, atom_codes/2, cputime/1, call/1, 
	expand_term/2, file_exists/1, functor/3,
	(index)/3, name/2, nl/0, (not)/1, op/3, read/1, repeat/0,
	statistics/0, see/1, seeing/1, seen/0, tell/1,
	telling/1, told/0, write/1, writeln/1, write_canonical/1,
	writeq/1, ('=..')/2
   from standard.
:- import ground/1 from basics.

:- import message/1, time_message/2 from auxlry.
:- import findall/3 from setof.
:- import append/3, length/2, member/2 from basics.
:- import str_cat/3 from string.
:- import conset/2, conget/2 from gensym.
:- import libpath/1 from loader.

:- import '_$call_user'/1 from usermod.
:- dynamic '_$call_user'/1.

:- import bagPO/3, bagPO1/3 from aggregs.
:- import subsumes_chk/2 from subsumes.
:- import get_maximal_calls/1 from usermod.

:- import '_$local_calls'/1, apply/2, (:-)/2, (:-)/1 from usermod.

:- import inline_predicate/2 from builtin.
