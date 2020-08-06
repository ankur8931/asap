/* File:      foreign.H
** Author(s): Jiyang Xu, Kostis Sagonas
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
** $Id: foreign.H,v 1.25 2010-10-07 01:21:40 kifer Exp $
** 
*/


:- export
	compile_cH/6,
	compile_so/3,
	runtime_loader_flag/3,
	foreign_module_library_extension/2.

:- import asm_magic/2 from asm.
:- import asm_putlength/2 from asmpass2.
:- import assert/1, retractall/1 from assert.
:- import message/1, warning/1, time_message/2 from auxlry.
:- import memberchk/2 from basics.
:- import current_compiled_module/1 from compile.
:- import engine_mode_magic_num/3 from consult.
:- import conset/2, conget/2 from gensym.
:- import
	file_close/1,
	fmt_write_string/3, file_put/2
   from file_io.
:- import attach/2, length/2 from listutil.
:- import
	stat_flag/2,
	str_len/2, str_cat/3
   from machine.
:- import substring/4 from machine.
:- import parse_filename/4 from machine.
:- import
	check_xpp_process_status/3,
	open_or_xpp_file/2
   from parse.
:- import shell/1 from shell.
:- import read/1, see/1, seen/0, open/3, close/1, tell/1, told/0, repeat/0,
	atom_codes/2,
	write/2, atom/1, file_exists/1, cputime/1, abort/1, seeing/1, telling/1
     from standard.
:- import str_sub/2 from string.
:- import concat_atom/2 from string.
:- import write_header/0, process_from/3 from wrapping.
:- import write_int/2 from xsb_writ.
:- import xsb_configuration/2 from xsb_configuration.
