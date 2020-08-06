/* File:      dbclause.H
** Author(s): Kostis Sagonas, David S. Warren
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
** $Id: dbclause.H,v 1.17 2006/04/17 20:51:30 tswift Exp $
** 
*/

	  
:- export db_retract/4, db_retract_nr/4, db_reclaim_space/2.
:- export db_clause/4, db_retractall/3, db_abolish/1, '_$clause_int'/1.

:- import abort/1, atom/1, call_c/1, repeat/0, is_most_general_term/1
     from standard.
:- import set_inthandler/2, '_$load_undef'/1 from loader.
:- import log_ith/3 from basics.
:- import stat_set_flag/2, code_call/3, conname/2,
          gen_retract_all/2,dynamic_code_function/5,
	  db_get_clause/11, db_retract0/3, db_abolish0/2, db_reclaim0/2,
	  globalvar/1, term_set_arg/4, psc_ep/2,
	  trie_assert_hdr_info/3
     from machine.

