/* File:      machine.H
** Author(s): Kostis Sagonas, Jiyang Xu David S. Warren, Terrance Swift
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
** $Id: machine.H,v 1.93 2009/10/19 16:56:42 dwarren Exp $
** 
*/


/*======================================================================*/
/* This file contains definitions for predicates implemented by the	*/
/* underline machine (hard builtins).					*/
/* Please keep this format for easier diffing.				*/
/*======================================================================*/

:- export
    float_op/10,
    float_op/5,
	psc_name/2, 
	psc_arity/2, 
	psc_type/2, 
	psc_prop/2, 
	psc_mod/2, 
	psc_set_type/2, 
	psc_set_prop/2,
	psc_set_spy/2,
	psc_ep/2,
	psc_set_ep/2,
	psc_data/2,
	psc_init_info/1,
	mark_term_cyclic/1,
	term_new_mod/3,
	term_psc/2, 
	term_type/2, 
	term_compare/3, 
	term_new/2,
	term_arg/3, 
	term_set_arg/4, 
	term_hash/3,
	crypto_hash/3,
	conget/2,
	conset/2,
	stat_flag/2, 
	stat_set_flag/2, 
	stat_sta/1,
	xwam_state/2,
	buff_alloc/2, 
	buff_word/3, 
	buff_set_word/3,
	buff_dealloc/3, 
	buff_cell/3, 
	buff_set_cell/4, 
	buff_set_var/4,
	buff_byte/3, 
	buff_set_byte/3, 
	file_puttoken/3,
	code_call/3, 
	trie_assert_hdr_info/3,
	call0/1,
        calln/2,calln/3,calln/4,calln/5,calln/6,calln/7,calln/8,calln/9,calln/10,calln/11,calln/12,
	code_load/3, 
	file_gettoken/5,
	stat_cputime/1,
	stat_walltime/1,
	str_match/5,
        dirname_canonic/2,
	psc_insert/4, 
	psc_import/3, 
	psc_import_as/2, 
	psc_insertmod/3,
	substring/4,
	str_len/2, 
	str_cat/3, 
	str_cmp/3, 
	string_substitute/4,
	unload_seg/1, 
	load_obj/4, 
	expand_filename/2,
        tilde_expand_filename/2,
        is_absolute_filename/1,
        parse_filename/4,
	almost_search_module/5,
	existing_file_extension/2,
	getenv/2, 
	putenv/1, 
	sys_syscall/4,	sys_syscall/5,sys_syscall/9,
	sys_system/7, 
	sys_gethost/2, 
	sys_errno/1, 
	file_stat/3,
	file_writequoted/2,
        intern_string/2,
 	psc_set_shared/2, 
 	psc_shared/2, 
 	psc_set_private/2, 
 	psc_env/2, 
	psc_spy/2, 
	psc_tabled/2, 
	psc_set_tabled/2, 
	dereference_the_bucket/2,
	pair_psc/2, 
	pair_next/2, 
	next_bucket/2,
	is_xwammode/1, 
	close_open_tables/1, 
	abolish_table_info/0,
	zero_out_profile/0, 
	write_out_profile/0,
	assert_code_to_buff/1,
	assert_buff_to_clref/6,
        formatted_io/5,
	file_read_canonical/3,
	db_get_last_clause/4,
	gen_retract_all/2,
	db_get_clause/11,
	db_retract0/3,
	db_build_prref/3,
	db_get_prref/2,
	db_abolish0/2,
	db_reclaim0/2,
   dynamic_code_function/5,
   table_inspection_function/5,table_inspection_function/7,table_inspection_function/8,table_inspection_function/9,
	url_encode_decode/3,
	oracle_query/7,
	odbc_exec_query/6,
        parsort/4,
   unify_with_occurs_check/2,
	private_builtin/11,
	segfault_handler/1,
	is_charlist/2,
	is_charlist/1,
	is_cyclic/1,is_acyclic/1,acyclic_term/1,
	trimcore/0,
	ground_or_cyclic/1,
	ground_and_acyclic/1,
   term_depth/2,
   term_size/2,
   term_size_limit/2,
        '$$set_scope_marker'/0,
        '$$unwind_stack'/0,
        '$$clean_up_block'/1,
%        '$$clean_up_block'/0,
        thread_request/9,  mt_random_request/3,
        gc_clauses/0,
	expand_heap/0, mark_heap/1, gc_heap/0,
	gc_stuff/2,
	'$$findall_init'/2,
	'$$findall_add'/3,
	'$$findall_get_solutions'/4,
	socket_request/7,
	set_arg/3,
	put_attributes/2,
	get_attributes/2,
	delete_attributes/1,
        attv_unify/2,
        put_attr/3,
        get_attr/3,
        del_attr/2,
        install_verify_attribute_handler/4,
        install_verify_attribute_handler/5,
        install_attribute_portray_hook/3,
	attribute_portray_hook/3,
        install_constraint_portray_hook/4,
        verify_attribute_handler/4,
	globalvar/1,
        ccall_store_error/2,
   get_breg/1,
   psc_set_incr/2,psc_get_incr/2,psc_get_intern/2,psc_set_intern/2,
	xsb_backtrace/0, xsb_backtrace/1.

:- import assert/1, retract/1, (dynamic)/1 from assert.
:- import write/1, writeln/1, is_attv/1, warning/1 from standard.
   :- import incr_eval_builtin/3 from increval.

:- export 
	file_function/7, file_function/3, conname/2, conpsc/2, 
	machine_file_exists/1, slash/1.

:- export set_pil_on/0, set_pil_off/0, set_prof_on/1, set_prof_off/0.

:- export prompt/2, prompt/3.

:- export breg_retskel/4.

:- export intern_term/2.

:- export module_of_term/2.

/* --------------------- end of file machine.H ------------------------ */

