/* File:      thread.H
** Author(s): R. Marques
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
** $Id: thread.H,v 1.26 2008/04/27 15:25:57 tswift Exp $
** 
*/

:- export
	thread_cancel/1,
	thread_create/1, thread_create/2,thread_create/3,
   thread_detach/1,thread_disable_cancel/0, thread_enable_cancel/0, 
	thread_exit/1, internal_thread_exit/1,
	thread_join/2, thread_join/1,
	thread_property/2,
	thread_self/1,set_xsb_ready/0,
   thread_signal/2,
	thread_yield/0, usleep/1, thread_sleep/1,
	threads/0,
	thread_exit_handler/1.

:- export abolish_private_tables/0,abolish_shared_tables/0.
:- export abolish_all_private_tables/0,abolish_all_shared_tables/0.

/* All obsolete -- try not to use */
:- export xsb_thread_cancel/1,
          xsb_thread_create/2,xsb_thread_create/3, 
          xsb_thread_detach/1,
	  xsb_thread_join/2, 
          xsb_thread_join/1,  % obsolescent
   xsb_thread_property/2,
          xsb_thread_self/1,
          xsb_thread_yield/0.

:- export xsb_sys_mutex_lock/1, xsb_sys_mutex_unlock/1.
:- export xsb_show_mutexes/0.

:- import retractall/1,asserta/1 from assert. 
 
:- import check_nonvar/3, check_atom/3,default_error_handler/1,domain_error/5,
          instantiation_error/3, misc_error/1, permission_error/4, type_error/4
          from error_handler.

:- import thread_request/9 from machine.
:- import stat_set_flag/2, term_psc/2 from machine.

   :- import call/1, catch/3, integer/1,throw/1, (;)/2 /*,call_cleanup/2*/ from standard.

:- import '_$thread_exit_ball'/2 from usermod.

:- import add_aliases_flag_error/4, remove_alias/3, get_alias/3 from file_io.

/* Higher level */
:- export par_execute/1, n_par_execute/2.



