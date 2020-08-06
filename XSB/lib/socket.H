/* File:      socket.H
** Author(s): The XSB Group
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
** $Id: socket.H,v 1.9 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

%% Calls that don't return the error code
:- export socket/1, socket_bind/2, socket_listen/2, socket_accept/2, 
     socket_connect/3, socket_flush/1, socket_close/1, socket_recv/2,
     socket_send/2, socket_put/2, socket_set_option/3.

%% Calls that do return the error code
:- export socket/2, socket_bind/3, socket_listen/3, socket_accept/3, 
     socket_connect/4, socket_close/2, socket_recv/3,
     socket_send/3, socket_get0/3, socket_put/3.

:- export socket_set_select/4, socket_select/6, socket_select_destroy/1.

:- import socket_request/7, file_function/7 from machine.

:- import obsolete/2 from obsolete.
