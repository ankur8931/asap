/* File:      x_interp.H
** Author(s): David Warren, Terrance Swift, Jiyang Xu, Kostis Sagonas
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
** $Id: x_interp.H,v 1.44 2008/04/07 00:25:47 tswift Exp $
** 
*/


:- export break/0.
:- export handle_timed_calls/0, reinvoke_timed_calls/0.

:- import [] / 0 from usermod.	% just to make usermod typed "module"
:- import assert/1, retract/1 from assert.
:- import xsb_flag/2 from curr_sym.
:- import numbervars/1 from num_vars.
:- import set_inthandler/2 from loader.
:- import file_write/2, file_nl/1 from xsb_writ.
:- import xsb_configuration/2 from xsb_configuration.
:- import file_read/3, vv/2 from xsb_read.
:- import handle_goal_interrupt/1 from mutex_xsb.
%% These are explicitly imported because they are used early on, 
%% before XSB is fully bootstrapped.
   :- import abort/0,call/1, call_c/1, call_expose/1, catch/3, call_goal_cut_trans/3,
   atom_codes/2, seeing/1, telling/1, see/1, tell/1, seen/0, told/0,
   gc_tables/1,gc_dynamic/1,
   handle_interrupts/1
     from standard.
:- import '_$timed_call_rep_handler'/3 from standard.
:- import '_$timed_call_max_handler'/5 from standard.
:- import '_$timed_call_rep_max_handler'/7 from standard.
:- import get_interpreter_index/1 from standard.
:- import '$$exception_ball'/2 from standard.
:- import close_open_tables/1,
          is_xwammode/1, 
	  stat_flag/2, stat_set_flag/2,
	  slash/1, trimcore/0,
	  xwam_state/2,
	  prompt/2, prompt/3
     from machine.
:- import cancel_sleeper_thread/0,start_sleeper_thread/1 from machine.
:- import file_open/3, file_close/1, file_get/2, fmt_write_string/3,
          file_flush/2, /*ioport2iostream/2,*/
%	  fmt_write/3, 
          file_getbuf_atom/4
    from file_io.
:- import push_stdin/2, pop_stdin/1 from push_io.
:- import  retractall/1 from assert.
:- import call_xsb_hook/2, xsb_exit_hook/1 from xsb_hook.
%:- import verify_attributes/2 from usermod.

:- import conset/2, conget/2 from gensym.

:- import banner_control/1 from banner.

:- import xsb_thread_self/1 from thread.

