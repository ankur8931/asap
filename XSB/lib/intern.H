/* File:      intern.P
** Author(s): Prasad Rao
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** $Id: intern.H,v 1.13 2012-05-22 12:40:00 tswift Exp $
** 
*/

:- export trie_insert/2, trie_delete/2, trie_unify/2, trie_truncate/1, 
   trie_property/2, trie_drop/1, trie_bulk_insert/2, 
   trie_bulk_delete/2, trie_bulk_unify/3.
   
:- export trie_shas_insert/2, trie_shas_delete/2, trie_shas_unify/2.
:- export trie_pras_insert/2, trie_pras_delete/2, trie_pras_unify/2.
:- export trie_prge_insert/2, trie_prge_delete/2, trie_prge_unify/2.

% Low-level or obsolete
:- export new_trie/1, trie_intern/5, trie_intern_deterministic/4, trie_intern/4,
   trie_intern/2,trie_interned/2,
   trie_interned/4, trie_retractall/2, bulk_trie_intern/2,
	  trie_intern/3, trie_interned/3, trie_unintern/2, delete_trie/1,
	  trie_set_last_int/2, trie_create/2,
	  trie_unintern_nr/2, trie_reclaim_uninterned_nr/1, 
   unmark_uninterned_nr/2, incr_unmark_uninterned_nr_inval/2, incr_unmark_uninterned_nr_immed/2,
   incr_unmark_uninterned_nr/2.

:- import integer/1, abort/1 from standard.
:- import call/1, arg/3 from standard.
:- import get_lastnode_cs_retskel/4 from tables.
:- import member/2 from basics.
:- import findall/3 from setof.
:- import dynamic_code_function/5 from machine.
:- import domain_error/5, type_error/4, check_atom/3, check_nonvar/3, check_var/3, 
   permission_error/4, instantiation_error/3, existence_error/4  from error_handler.
   :- import add_aliases_flag_error/4, get_alias/3, remove_alias/3 from file_io.

%%%:- import buff_word/3, buff_set_word/3  from machine.

