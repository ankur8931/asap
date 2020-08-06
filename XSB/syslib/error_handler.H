/* File:      error_handler.H
** Author(s): Swift
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
** $Id: error_handler.H,v 1.29 2008/03/27 19:53:33 tswift Exp $
** 
*/

:- export abort_on_thread_cancel/0, domain_error/4,domain_error/5,
   evaluation_error/3, existence_error/4,
   instantiation_error/2,  % obsolescent
   instantiation_error/3,
   misc_error/1, misc_error/2,
   permission_error/4,    preprocessing_error/1,
   representation_error/3, resource_error/2,
   syntax_error/1, system_error/3,
   table_error/1,    table_error/4, type_error/4, 
   check_atom/3, check_atom_list/3, check_callable/3, check_cyclic/3,
   check_listable/3, check_ground/3, 
   check_integer/3, check_nonneg_integer/3, check_pos_integer/3,
   check_nonvar_list/3, check_nonvar_comma_list/3, check_nonvar/3, 
   check_number/3, check_one_thread/3,
   check_var/3, 
   default_error_handler/1,
   default_sys_error_handler/1,
   default_sys_error_handler_nonmain/4,
   permission_warning/5,
   ccall_error_handler/1,
              get_sys_error_description/2,
              get_sys_error_description/3,
              print_backtrace/1, print_backtrace/2,
	      get_backtrace/2,
	      get_backtrace_list/2.

:- export psc_modname_filename/3.
:- export flush_all_open_streams/1.
:- export xsb_error_get_tag/2,xsb_error_get_message/2,xsb_error_get_backtrace/2,xsb_error_get_goal/2,xsb_error_get_tid/2,
          xsb_error_get_goalatom/2.

:- import (dynamic)/1,asserta/1 from assert.
%%:- import reset_undef_pred_handlers/0 from loader.
:- import close_open_tables/1, psc_data/2, psc_name/2, psc_arity/2, ccall_store_error/2,
   stat_flag/2, xsb_backtrace/1, psc_ep/2,conget/2, conset/2 from machine.

:- import ','/2, (=..)/2 from standard.
   :- import atom/1, callable/1, messageln/2, number/1, warningln/1, message/2,
          throw/1,integer/1 from standard.
:- import concat_atom/2, term_to_atom/2 from string.
:- import xsb_flag/2 from curr_sym.
:- import internal_thread_exit/1, thread_property/2 , xsb_thread_self/1 from thread.
:- import flush_output/1,stream_property/2 from file_io.
