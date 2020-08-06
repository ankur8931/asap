/* File:      assert.H
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
** $Id: assert.H,v 1.31 2010/02/10 19:13:00 dwarren Exp $
** 
*/


:- export convert_to_dyna/3,/* incremental evaluation */ assert0/1, assert0/4, assert/1, asserta/1, assertz/1, 
	  asserti/2, assertai/2, assertzi/2, assert/3.
:- export retract/1, retractall/1, abolish/1, abolish/2. 
:- export retract0/1.	% used in standard.P (for multifile)
:- export retract_nr/1, reclaim_space/1.
:- export retract_last_fact/1.
:- export clause0/2, clause/2.
:- export (dynamic)/1.
:- export t_assert/2.
:- export code_to_buff/2. % for message queues
:- export exchange_definitions/2.
:- export predicate_defined/1.
:- export update/4, get_fact_clref/2, update_clref/4.  %% hackery...
:- export check_dynamic_options/2.
:- export retract_fact/1.
:- export system_retractall/1.
:- export system_retract_fact/1.
:- export t_assert_2/2.

:- import append/3, memberchk/2,comma_member/2 from basics.

:- import default_assert_size/1 from config.

:- import (thread_shared)/1, (thread_private)/1 from consult.

:- import db_retract/4, db_retractall/3, db_abolish/1, db_clause/4,
  db_retract_nr/4, db_reclaim_space/2
     from dbclause.

:- import instantiation_error/3 , permission_error/4, misc_error/1, 
	  type_error/4 from error_handler.
:- import check_nonvar/3 from error_handler.
:- import check_nonvar_comma_list/3 from error_handler.
:- import domain_error/5,domain_error/4,check_callable/3 from error_handler.
:- import check_nonvar/3 from error_handler.
:- import check_nonvar_comma_list/3 from error_handler.
:- import check_atom/3 from error_handler.
:- import table_error/1 from error_handler.

:- import '_$index'/3, '_$trie_asserted'/1, retract_trie_index/1,
	  retract_index/1, mpa_to_skel/2  from file_op.

:- import psc_ep/2, psc_type/2, psc_set_type/2, psc_arity/2,
	  conpsc/2, assert_code_to_buff/1, assert_buff_to_clref/6,
	  psc_set_ep/2, unload_seg/1, code_call/3, db_retract0/3, term_set_arg/4, term_new_mod/3,
	  psc_init_info/1, intern_term/2, psc_data/2, psc_immutable/2
     from machine.
:- import db_get_last_clause/4 from machine.
:- import dynamic_code_function/5 from machine.
:- import psc_set_incr/2,psc_set_intern/2  from machine.
:- import psc_get_intern/2, psc_get_incr/2 from machine.
:- import buff_set_word/3, term_psc/2 from machine.
:- import get_attributes/2 from machine.
:- import db_retract_fact/3 from dbclause.
:- import dynamic_code_function/6 from machine.

:- import obsolete/2 from obsolete.

:- import get_p_mod/3 from parse.

:- import (table)/1 from standard.
:- import '_$multifile'/1 from standard.
:- import call/1, call_c/1 from standard.
:- import (=..)/2, functor/3, integer/1, callable/1, arg/3,
         goal_cut_trans/3, goal_cut_untrans/2, abort/1, check_arity_integer/3,
         is_most_general_term/1, atomic/1, warning/1
     from standard.

:- import standard_symbol/3, standard_dynamic_symbol/2 from std_xsb.

:- import (use_variant_tabling)/1 from tables.
:- import retract_table/1 from tables.

:- import call_xsb_hook/3, xsb_assert_hook/1, xsb_retract_hook/1 from xsb_hook.
 
