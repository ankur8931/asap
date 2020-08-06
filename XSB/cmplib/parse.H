/* File:      parse.H
** Author(s): Saumya Debray, Kostis F. Sagonas
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
** $Id: parse.H,v 1.54 2012-10-10 22:29:02 tswift Exp $
** 
*/


:- export
	register_xsb_preprocessor/2,
	unregister_xsb_preprocessor/1,
	unset_preprocessing_done/0,
	xppdump_is_on/0,
	xpp_is_off/0,
	set_xpp_off/0,
	check_xpp_process_status/2, check_xpp_process_status/3,
	make_gpp_command/4,
        parse/4, get_symtab/2, trans_goals/5, trans_head/5.
:- export get_p_mod/3, %%change_multifile_directive/3, 
	xpp_process_file/3, xpp_process_file/4, gpp_process_file/3,
	open_or_xpp_file/1,
	open_or_xpp_file/2, xpp_include_dir/1, gpp_include_dir/1,
	xpp_options/1,
	gpp_options/1,
	enqueue_preprocessor/1,
	dequeue_preprocessor/1,
	'_$multifile'/1, '_$multifile1'/1,
	'_$multifile2'/3, '_$apply_arity'/1,
	'_$multifile_retract'/3.
:- export parsing_file_name/1.

:- import
	retractall/1, retract/1,
	asserta/1, assertz/1,
	(dynamic)/1,
	check_dynamic_options/2
   from assert.

:- import check_file_compiler_options/1, 
	error/1, message/1, option/1, setoptions1/1, warning/1
   from auxlry.

:- import append/3, memberchk/2, log_ith/3, member/2, copy_term/2 from basics.
:- import ith/3 from basics.

:- import inline_predicate/2 from builtin.

:- import included_ciao_directive/1 from ciao_directives.

:- import search_module/6 from consult.

:- import
	xsb_flag/2,
	current_input_port/1
   from curr_sym.

:- import
	preprocessing_error/1,
	existence_error/4, misc_error/1, syntax_error/1, check_ground/3
   from error_handler.

:- import
	file_size/2,
	fmt_write_string/3
   from file_io.

:- import mpa_to_skel/2 from file_op.

:- import conset/2, conget/2, gennum/1, prepare/1 from gensym.

:- import add_hilog_symbol/1 from hilogsym.

:- import closetail/1 from listutil.

:- import stat_flag/2, stat_set_flag/2, slash/1 from machine.
:- import file_read_canonical/3 from machine.
:- import conpsc/2, term_new/2 from machine.

:- import findall/3 from setof.

:- import spawn_process/5, process_control/2 from shell.

:- import singleton_check/2 from singlton.

:- import functor/3, seen/0, file_exists/1, '=..'/2, atom/1, integer/1,
          op/3, (index)/3, (index)/2, real/1, see/1, expand_term/2, (=..)/2,
	  '\='/2, check_table_options/3,
	  arg/3,
	  open/3,
	  close/1,
	  call/1, compound/1,
	  is_most_general_term/1, number/1, atom_codes/2
     from standard.
:- import throw/1 from standard.

:- import file_read/3 from xsb_read.

:- import standard_symbol/3 from std_xsb.
:- import standard_metapredicate/3 from std_xsb.

:- import concat_atom/2 from string.

:- import sym_insert/5, sym_propin/2, sym_complete/1, sym_prop/2,
	  add_new_clause/3, clause_listify/2
     from symtab.

:- import xsb_configuration/2 from xsb_configuration.

:- import sort/2 from setof.
