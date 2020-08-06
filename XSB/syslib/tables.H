/* File:      tables.H
** Author(s): Terry Swift, Prasad Rao, Kostis Sagonas
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
** $Id: tables.H,v 1.41 2009/08/28 17:43:09 tswift Exp $
** 
*/

:- export
   abolish_all_tables/0,
   abolish_table_subgoals/1,       abolish_table_subgoals/2, 
   abolish_table_subgoal/1,       abolish_table_subgoal/2, 
   abolish_table_pred/1,      abolish_table_pred/2,
   abolish_module_tables/1,
   abolish_nonincremental_tables/1,
   add_incr_table/2, add_table/1,
   analyze_an_scc/2, analyze_an_scc/3,
   check_variant/1,  check_variant/2,
   delete_return/2, delete_return/3,
   force_truth_value/2, force_answer_true/1, force_answer_false/1,
   forest_log_overview/0,
   get_answer_number/2,       /* table_dump */
   get_call/3,
   get_calls/3,
   get_calls_for_table/2,     /* table_dump and others */
   get_callsto_number/2,      /* table_dump */
   get_current_scc/2,         /* table_dump */
   get_lastnode_cs_retskel/4, /* because it is used in intern.P */
   get_producer_call/3,
   get_ptcp/1,
   get_residual/2,
   get_returns/2,   get_returns/3,
   get_returns_and_dls/3, 
   get_returns_and_tvs/3,     /* table_dump */
   get_returns_for_call/2,
   get_sdg_info/1,
   get_unifiable_returns/3,
   get_visitors_number/2,
   invalidate_tables_for/2,
   is_incomplete/2,
   load_forest_log/1,
   log_all_forest_start/0, log_all_forest_start/1,
   log_all_forest_end/0, 
   log_all_forest_flush/0, 
   log_forest/1, log_forest/2,
   morph_dep_graph/3,
   print_incomplete_tables/0,print_incomplete_tables/1,
print_ls/0, print_tr/0, print_heap/2, print_cp/1, print_regs/0,	print_all_stacks/0, 
           retract_table/1,          /* abolish/1 */
   sdg_scc_info/2, 
   sk_not/1,
%      table_state/2,
   table_state/4,
   tfindall/3,
   three_valued_sccs/1,
   (tnot)/1,
   't not'/1,
   (trie_dynamic)/1,
   trie_assert/1,   trie_assert/2,
   trie_retract/1,   trie_retract_nr/1,
   trie_retract_all/1,
   u_not/1,
   undefined/0, 
   (use_subsumptive_tabling)/1,
   (use_variant_tabling)/1,
   variant_get_residual/2.
:- export not_exists(Goal).
:- export floundered_undefined/0.
:- export answer_completion/2.
:- export dump_table_counts/0, table_counts/5.
:- export print_sdg_info/0,print_sdg_info_modes/0, print_sdg_info_functor/0, print_sdg_info/1, 
          print_sdg_subgoal_info/0.
:- export get_sdg_subgoal_info/1.
:- export get_residual_sccs/3.
:- export get_residual_sccs/5.

% Obsolete
:- export    abolish_table_call/1,       abolish_table_call/2.

/****** Experimental ********/
:- export find_components/1,
	find_forward_dependencies/1,
	%%%find_backward_dependencies/1,
	find_answers/1.
:- export early_complete_on_nth/2.
:- export early_complete_on_delta/4.
:- export get_scc_dumpfile/1.
:- export set_forest_logging_for_pred/2.

/***** Other ******/
:- export
        start_forest_view/1,
        flush_forest_view/0,
        stop_forest_view/0.
:- export '_$$get_returns'/3.
:- export '_$$get_returns_and_tvs'/4.
:- export predicate_has_tables/1.

:- import
	(dynamic)/1,
	retract/1, retract_nr/1, retractall/1,
	system_retract_fact/1,
	t_assert/2,
	t_assert_2/2
   from assert.

:- import clause/2 from assert.

:- import list_to_assoc/2,get_assoc/3 from assoc_xsb.

:- import reverse/2 from basics.
:- import copy_term/2,flatten/2,length/2, member/2, memberchk/2
     from basics.

:- import set_prolog_flag/2 from curr_sym.
:- import current_prolog_flag/2 from curr_sym.
:- import predicate_property/2, current_predicate/1 from curr_sym.
:- import load_dync/2 from consult.

:- import
   domain_error/4, domain_error/5, check_nonvar/3, type_error/4, table_error/1, 
   check_atom/3,check_var/3,check_ground/3,
   permission_error/4, check_cyclic/3, misc_error/1
   from error_handler.

:- import check_open_stream_or_alias/5 from file_io.
:- import add_trie_index/1, mpa_to_skel/2
     from file_op.
:- import format/2 from format.

:- import abolish_table_info/0, delete_attributes/1, 
   ground_and_acyclic/1,
   psc_set_incr/2, psc_set_tabled/2, psc_tabled/2, psc_type/2,
   table_inspection_function/5, table_inspection_function/7,table_inspection_function/8,table_inspection_function/9,
   term_psc/2, term_new/2, stat_flag/2
   from machine.
:- import term_new_mod/3 from machine.
:- import term_arg/3, parsort/4 from machine.
:- import stat_set_flag/2 from machine.

:- import numbervars/4, unnumbervars/2 from num_vars.

:- import bagof/3, findall/3, setof/3, sort/2, excess_vars/4 from setof.

:- import
   (=..)/2, abort/1, call/1,arg/3,
	functor/3, is_attv/1, tcall/1,
	(index)/2, (==)/2
   from standard.
:- import call_c/1,nl/0,write/1, tell/1, told/0, '\='/2, arg/3 from standard.
:- import writeln/2, write/2, writeq/2 from standard.
:- import is_most_general_term/1 from standard.
:- import call/3 from standard.
:- import writeq/1 from standard.
:- import warning/1 from standard.

:- import variant/2 from subsumes.
:- import invalidate_table_for/2 from usermod.

:- import trie_interned/2,trie_intern/2,new_trie/1,delete_trie/1 from intern.
   :- import gensym/2, conset/2, conget/2, coninc/1 from gensym.

/* ------------------- end of file tables.H --------------------------- */
