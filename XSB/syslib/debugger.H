/* File:      debugger.H
** Author(s): David S. Warren
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
** $Id: debugger.H,v 1.21 2007/12/05 18:16:02 dwarren Exp $
** 
*/


/* Trace and Debug package */

:- export debug/0, nodebug/0, dump/0,
	  (spy)/1, (nospy)/1,
	  trace/0, trace/1, trace/2,
	  notrace/0, piltrace/0, hitrace/0,
	  debug_logging_queriable_trace/1,
	  set_debug_logging_queriable_trace/1,
	  make_trace_interactive/0,
	  dump_trace_to_file/1,
	  nosimtrace/0, debugging/0, debug_ctl/2.
:- export '_$deb_trace'/1, '_$deb_spy'/1, '_b_SPY_condition'/1,
	trace_goalstack/2,
	show/1, leash/1.

:- import set_inthandler/2, '_$load_undef'/1, load_pred/1 from loader.
:- import file_write/2, file_write_quoted/2 from xsb_writ.

:- import =.. /2, functor/3, atom/1, is_list/1, call/1, cputime/1,
          call/1, see/1, seeing/1, seen/0, repeat/0, read/1, warning/1,
	  abort/0, abort/1, (import)/1, callable/1
     from standard.

:- import assert/1 from assert.
:- import (dynamic)/1, assert0/1, retract/1, retractall/1 from assert.

:- import break/0 from x_interp.
:- import term_psc/2, psc_insert/4, stat_flag/2, stat_set_flag/2, 
	  xwam_state/2,
	  psc_name/2, psc_type/2, psc_ep/2, psc_spy/2, psc_set_spy/2,
	  psc_arity/2, conname/2, code_call/3, term_arg/3, term_type/2,
	  term_new/2, prompt/3
     from machine.
:- import standard_symbol/3 from std_xsb.
:- import predicate_property/2, current_predicate/1 from curr_sym.
:- import findall/3, setof/3 from setof.
:- import type_error/4 from error_handler.
% := hide debug/0.

:- import hide_this_show/2, hide_this_hide/2, show_this_hide/2,
	trace_this_print/2, trace_this_noprint/2,
	is_xpar_call/2,
	trace_standard_predicate/2,
	trace_this_enable_print_on_redo/2,
	debug_display_call_hook/1, 
	debug_tracelog_predicate/1, 
	'_$_deb_user_hidden'/2,
	'_$_deb_old_show'/1, '_$_debugger_data_loaded'/0,
	'_$_deb_old_leash'/1, '_$_deb_cputime_stack'/1
   from usermod.
:- import xsb_configuration/2 from xsb_configuration.

:- import fmt_write_string/3, file_flush/2 from file_io.
:- import file_get/2, file_put/2, file_flush/2 from file_io.
:- import file_reopen/4, file_clone/3 from file_io.

:- dynamic 
	debug_display_call_hook(_),
	debug_tracelog_predicate(_),
	'_$_debugger_data_loaded'/0.
:- dynamic '_$_deb_old_leash'/1, '_$_deb_old_show'/1, '_$_deb_cputime_stack'/1.
:- dynamic '_$_deb_user_hidden'/2.
:- dynamic hide_this_hide(_,_), hide_this_show(_,_), show_this_hide(_,_).
:- dynamic trace_this_noprint(_,_), trace_this_print(_,_), 
	trace_standard_predicate(_,_), is_xpar_call(_,_),
	trace_this_enable_print_on_redo(_,_).

:- import
	gensym/2,
	conset/2,
	conget/2
   from gensym.

:- import (:-)/2 from usermod.

/* --------------------- end of file debugger.H ----------------------- */
