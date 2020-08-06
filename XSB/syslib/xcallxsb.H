/* File:      xcallxsb.H
** Author(s): Ernie Johnson
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
** $Id: xcallxsb.H,v 1.13 2007/10/09 16:40:18 dwarren Exp $
** 
*/

:- export c_callloop/0.  % to force it to be a module!

:- import c_callloop_query_loop/1 from ccallxsb.

:- import set_inthandler/2 from loader.

:- import catch/3, repeat/0, call/1, (\=)/2, abort/0 from standard.

:- import file_open/3, file_close/1
     from file_io.

:- import file_read/2 from xsb_read.

:- import xsb_flag/2 from curr_sym.

 :- import ccall_error_handler/1 from error_handler.
