/* File:      proc_files.H
** Author(s): David S. Warren
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, , 1993-1998
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
** $Id: proc_files.H,v 1.5 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

:- compiler_options([ciao_directives]).

:- export
        load_csv/3,
        load_csv/2,
        load_psv/2.
:- export load_dsv/3.
:- export save_dsv/3.
:- export copy_dsv/4.
:- export load_with_header/2.
:- export file_header/4.
:- export read_dsv/3.

:- import append/3 from basics.
:- import length/2 from basics.
:- import member/2 from basics.
:- import warning/1, throw/1 from standard.
:- import conset/2, coninc/2, conget/2, coninc/1 from gensym.
:- import file_read_line_list/2, file_put/2 from file_io.
:- import writeln/1, atom_codes/2, repeat/0, '=..'/2,
	functor/3, arg/3, number_codes/2, call/1, 
	nl/1, writeq/2, write/2, writeln/2, cputime/1,
	open/3, close/1
	from standard.
:- import retractall/1, assertz/1, asserta/1, (dynamic)/1 from assert.
:- import str_match/5 from machine.
:- import time_message/2 from auxlry.
:- import atom_to_term/2 from string.
