/* File:      scrptutl.H
** Author(s): Swift, Warren
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1999
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
** $Id: scrptutl.H,v 1.12 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

:- export  
	   ca/2,ca_null/2,
	   date/1,
	   simple_tokenize/2,
	   file_to_list/2,
	   %% Obsolete
	   fget_line/3, file_to_list/3,
	   sysin/2.

:- import retractall/1, assert/1, (dynamic)/1 from assert.
:- import file_read_line_list/2 from file_io.

:- import concat_atom/3 from string.
:- import name/2, see/1, get0/1, atom/1, arg/3, functor/3,
	repeat/0, seeing/1
	from standard.

:- import open/3, close/1 from standard.

:- import reverse/2 from basics.
:- import obsolete/2 from obsolete.
:- import datime/1 from standard.
:- import xsb_configuration/2 from xsb_configuration.

:- import unix/1 from unix.

% :- import spawn_process/5 from shell.
