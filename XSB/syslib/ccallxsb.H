/* File:      ccallxsb.H
** Author(s): David S. Warren, Ernie Johnson
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1997-1998
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
** $Id: ccallxsb.H,v 1.4 2008/11/12 21:56:52 dwarren Exp $
** 
*/

:- export c_callloop_query_loop/1.

:- import
   close_open_tables/0, stat_flag/2
   from machine.

:- import file_open/3, file_close/1
     from file_io.

:- import (dynamic)/1, assert/1, retract/1, retractall/1 from assert.
:- import ccall_error_handler/1 from error_handler.
:- import set_inthandler/2 from loader.
:- import excess_vars/4 from setof.
:- import call/1, call_c/1, call_expose/1, (index)/2, catch/3,
   call_goal_cut_trans/3, handle_interrupts/1, repeat/0, (=..)/2  from standard.
:- import [] / 0 from usermod.	       % just to make usermod typed "module"
:- import xsb_configuration/2 from xsb_configuration.
:- import file_read/3 from xsb_read.
:- import file_write/2, file_nl/1 from xsb_writ.
