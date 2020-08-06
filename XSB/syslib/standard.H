/* File:      standard.H
** Author(s): Sagonas, Warren, Kifer, Cui, Demoen, Swift and others
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
** $Id: standard.H,v 1.107 2010/03/16 22:55:07 kifer Exp $
**
*/


/* ------------------------------ */
/*  exported built-in predicates  */
/* ------------------------------ */


:- export 
	arg/3,		arg0/3,		atom/1,		
	atom_concat/3,   atomic/1,	atom_length/2,
	goal_arg/3,	atom_chars/2,   atom_codes/2,
	call/1, call/2, call/3, call/4, call/5, call/6, call/7, call/8, call/9, call/10, call/11,
   call_tv/2,
	call_expose/1,	callable/1, char_code/2,
	check_arity_integer/3,  call_cleanup/2, cleanup_handler/2,
	close/1,             close/2,	compare/3,	
	compound/1,	cputime/1,	
	directly_callable/1,
   display/1, displayln/1, now/1, datime/1, local_datime/1,
	get_date/6,
	get_utcdate/6, get_localdate/6,
	console_write/1, console_writeln/1,
	error_write/1, error_writeln/1,
	expand_term/2,	file_exists/1,	fail/0,	false/0,	fail_if/1,
	fileerrors/0,	float/1,	forall/2,	functor/3,
	gc_dynamic/1,   gc_tables/1,    gc_atoms/0,
	get/1,		get0/1,		
	get_char/1,	get_char/2,     get_code/1,	get_code/2,
	get_byte/1,	get_byte/2,     put_byte/1, put_byte/2,
	halt/0,	halt/1,	
	(hilog)/1,	hilog_arg/3,	hilog_functor/3,
	(index)/3,	(index)/2,      (table)/1,
	(use_incremental_tabling)/1,    (use_incremental_dynamic)/1, 
   use_incremental_dynamic_internal/1,
	(use_opaque_tabling)/1, /* incremental */
	integer/1,	is/2,		is_attv/1,	is_list/1,
	is_most_general_term/1,		is_number_atom/1,
	listing/0,	listing/1,	name/2,		nl/0, nl/1,
	nofileerrors/0, nonvar/1,	(not)/1,	number/1,
	number_chars/2, number_codes/2, number_digits/2,
	once/1,		op/3,		open/3,		open/4,
%	print/1,	
	otherwise/0,  proper_hilog/1,	
	put/1,          put_char/1,     put_char/2,     put_code/1,
	put_code/2,
	read/1,		read/2,             read_term/3,    read_term/2,
	real/1,		repeat/0,
	see/1,		seeing/1,	seen/0,
	set_input/1,    set_output/1,  
   demand_once/1, check_table_options/3,
	set_tmp_file/3,
	skip/1,		statistics/1,	statistics/0,	structure/1,
	tab/1,		tell/1,		telling/1,	time/1,	told/0,true/0,
	ttywrite/1,	ttywritenl/1,   ttywrite/2,	ttywritenl/2, 
        use_module/2,   var/1,		walltime/1,	
	write/1,	write/2,	write_prolog/1,	write_prolog/2,
	writeln/1,	writeln/2,	writeq/1,	writeq/2,
	write_canonical/1,  write_canonical/2,
	('\+')/1,	'=:='/2,	'=\='/2,	'<'/2,
	'>'/2,		'=<'/2,		'>='/2,		'='/2,
	'\='/2,		'=..'/2,	'^=..'/2,	'=='/2, 	'@='/2,
	'\=='/2,	'@<'/2,		'@>'/2,		'@=<'/2,
        '@>='/2,	';'/2,		','/2,		'->'/2,         '?='/2,
	':'/2,		'_$cutto'/1,	'_$savecp'/1.	
:- export handle_interrupts/1.
%:- export timed_call/3.
%:- export timed_call/4.
:- export get_timed_call_index/1.
:- export timer_interrupt_trap/2.
:- export timed_call/2,timed_call_modify/1.
:- export current_timed_call/2.
:- export remove_timed_call_state/1.
:- export timed_call_cancel/0.

:- export memory_bounded_call/3.

   :- export expand_atom/2.

/* --------------------------------------------------------------- */
/*  exported predicates used in other modules (but not "standard")  */
/* --------------------------------------------------------------- */

:- export
        call_c/1,	
        catch/3,                throw/1,        '$$exception_ball'/2,
        goal_cut_trans/3,	call_goal_cut_trans/3, goal_cut_untrans/2,
	tcall/1,	arity_integer/1,	prolog_arglist/4,
   message/1,      messageln/1, message/2, messageln/2,
   warning/1,      warning/2,
   warningln/1,  % obsolete
   abort/1,	abort/0, abort_all/1, abort_level/1,
	set_timer/1,
   (import)/1,	(multifile)/1, '_$multifile'/1.
:- export cleanup_handler/2.
:- export statistics/2.
:- export use_module/3.
:- export  '_$timed_call_rep_handler'/3.
:- export  '_$timed_call_max_handler'/5.
:- export  '_$timed_call_rep_max_handler'/7.
:- export bounded_call/4.
:- export bounded_call/3.
:- export get_interpreter_index/1.

:- import (dynamic)/1, asserta/1, assert0/4, retract/1 from assert.
:- import member/2,append/3,comma_member/2,memberchk/2 from basics.
:- import consult/1, reconsult/1 from consult.
:- import predicate_property/2, current_op/3 from curr_sym.
:- import dcg/2 from dcg.
%%:- import eval/2 from eval.
:- import file_open/3, file_open/4, file_close/2, 
	  file_get_code/2, file_get_byte/2, file_get_char/2, 
	  file_put/2, file_put_code/2, file_put_char/2,
  check_stream_or_alias/4, check_open_stream_or_alias/5, add_aliases/4, remove_alias/3,
  fd2iostream/3      from file_io.
:- import fmt_write/2 from file_io.
:- import file_function/3, xsb_stream_lock/1,xsb_stream_unlock/1 from file_io.

:- import add_ops/3, add_index/3, '_$index'/3,
   add_trie_index/1  from file_op.

:- import add_table/1, add_incr_table/2 from tables.

:- import '_$hilog_symbol'/1, add_hilog_symbol/1 from hilogsym.
:- import clause/2, retractall/1 from assert.
   :- import domain_error/4, domain_error/5, check_atom/3, check_nonvar/3,check_var/3, check_callable/3,
	  check_ground/3,resource_error/2, representation_error/3,
   type_error/4, permission_error/4, 
   instantiation_error/3, instantiation_error/2, check_nonneg_integer/3, check_pos_integer/3,
	  misc_error/1,misc_error/2, table_error/1
     from error_handler.
:- import  check_nonvar_list/3 from error_handler.
:- import '_$load_undef'/1 from loader.
:- import call0/1, conname/2,
          dynamic_code_function/5,
          expand_filename/2, gc_stuff/2,
          machine_file_exists/1, 
	  psc_arity/2, psc_import/3, psc_insert/4,
	  psc_name/2, psc_type/2, psc_set_shared/2,
	  stat_set_flag/2, stat_flag/2, term_compare/3,
	  stat_sta/1, stat_cputime/1, stat_walltime/1,
          %%str_len/2,
          term_psc/2, term_new/2, term_new_mod/3, term_type/2,
          cancel_sleeper_thread/0,
          verify_attribute_handler/4, conget/2
	  from machine.
:- import cancel_sleeper_thread/0 from machine.
:- import calln/3,calln/4,calln/5,calln/6,calln/7,calln/8,calln/9,calln/10,calln/11,calln/12 from machine.
:- import sys_syscall/5,sys_syscall/9 from machine.

:- import '$$set_scope_marker'/0, '$$unwind_stack'/0, 
	  '$$clean_up_block'/1, xwam_state/2
     from machine. 

%:- import numbervars/1 from num_vars.
:- import numbervars/4 from num_vars.
:- import calculate_singleton/3 from singlton.
:- import concat_atom/2  from string.
:- import file_read/2, file_read/3 from xsb_read.
:- import file_write0/2, file_write/2, file_write_prolog/2, l_write/3,
	  file_write_quoted/2, file_write_canonical/2, file_nl/1
     from xsb_writ.
:- import xsb_thread_self/1 from thread.
:- import (thread_shared)/1, (thread_private)/1 from consult.
:- import findall/3 from setof.
:- import (use_variant_tabling)/1, (use_subsumptive_tabling)/1 from tables.

:- import (:-)/2 from usermod.
:- import load_page/5 from curl.
:- import start_sleeper_thread/1 from machine.
:- import assert/1 from assert.

:- import sys_exit/1 from shell.

/* --------------------- end of file standard.H ----------------------- */
