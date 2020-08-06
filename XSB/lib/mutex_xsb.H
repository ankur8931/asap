/* File:      xsb_mutex.H
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
** 
*/

:- export mutex_create/1, mutex_create/2, mutex_lock/1, mutex_unlock/1,
          mutex_trylock/1, mutex_destroy/1, with_mutex/2.
:- export mutex_unlock_all/0, mutex_property/2.

:- export
	message_queue_create/1, message_queue_create/2, message_queue_destroy/1,
	thread_get_message/1, thread_get_message/2,
	thread_send_message/1, thread_send_message/2,
	thread_peek_message/1, thread_peek_message/2,
	print_message_queue/1.
:- export thread_peek_signal/1, thread_get_signal/1.

:- import xsb_flag/2 from curr_sym.
:- import code_to_buff/2 from assert.
:- import check_atom/3,
	existence_error/4, permission_error/4, domain_error/4, domain_error/5, misc_error/1,
	type_error/4, instantiation_error/3, check_var/3, check_nonvar/3
	from error_handler.
   :- import get_alias/3, add_aliases/4, add_alias/4, remove_alias/3 from file_io.
:- import thread_request/9 from machine.
:- import catch/3, repeat/0, throw/1, (\=)/2 from standard.
%:- import thread_self/1 from thread.
