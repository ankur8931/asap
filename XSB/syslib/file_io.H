/* File:      file_io.H
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: file_io.H,v 1.39 2008/03/22 19:23:16 tswift Exp $
** 
*/

:- import (dynamic)/1,asserta/1, retractall/1 from assert.

:- import stat_flag/2, file_function/7, file_function/3,
	url_encode_decode/3, slash/1,
	parse_filename/4, is_absolute_filename/1,
	expand_filename/2, getenv/2,
	sys_system/7, formatted_io/5 
      from machine.
%%%:- import conpsc/2, psc_set_type/2, psc_set_prop/2 from machine.

:- import for/3 from basics.

:- import  abort/1, atom_codes/2, repeat/0, integer/1,
               file_exists/1, atom/1
      from standard.

%%%:- import gensym/2 from gensym.

:- import is_plain_file/1, is_directory/1, rename/2, rm/1, rmdir_rec/1, cwd/1,
	sys_filecopy/3, sys_fileappend/3,
	cd/1, sys_link/3, sys_mkdir/2, sys_rmdir/2, sys_access/3
      from shell.

:- import file_write/2 from xsb_writ.

:- import domain_error/4, existence_error/4, instantiation_error/2,
		  permission_error/4, type_error/4 from error_handler.

:- import concat_atom/2 from string.

:- export fmt_write_string/3, fmt_write/2, fmt_read/3,
	  fmt_write/3, fmt_read/4,
	  print_file_contents/1,
	  print_file_contents/2,
	  path_sysop/2, path_sysop/3,
	  copyIOport/2,
  file_seek/4, file_truncate/3,
  file_pos/2, file_size/2, file_time/2,
  file_open/3, file_open/4, file_close/1, file_close/2,
	  file_get/2,file_get_code/2,file_get_byte/2, file_get_char/2, 
		  file_put/2, file_put_code/2,file_put_char/2, 
	  file_getbuf/4,
	  file_getbuf_atom/4, file_getbuf_atom/3,
	  file_getbuf_list/4, file_getbuf_list/3,
	  file_putbuf/5, file_putbuf/4,
	  file_write_line/2, file_write_line/3,
      	  file_read_line/2,
	  file_read_line_atom/1, file_read_line_atom/2,
	  file_read_line_list/1, file_read_line_list/2,
	  file_reopen/4, file_clone/3,
	  pipe_open/2, fd2ioport/2, fd2iostream/3, 
	  file_clearerr/1, tmpfile_open/1,
   print_openfiles/0, check_enumerate_stream_or_alias/4, 
   check_stream_or_alias/4, check_open_stream_or_alias/5, 
   add_aliases/4, add_aliases_flag_error/4,
   get_alias/3, remove_alias/3, add_alias/4,
   is_valid_stream/2, peek_code/1, peek_code/2, peek_char/1, peek_char/2,peek_byte/1, peek_byte/2,
   at_end_of_stream/1, at_end_of_stream/0, set_stream_position/2,
   url_encode/2, url_decode/2,
   xsb_stream_lock/1,xsb_stream_unlock/1.   

% ISO predicates (sometimes rewrites of earlier predicates w. more checking).
:- export stream_property/2, flush_output/1, flush_output/0.

% Obsolescent?
:- export  file_flush/2.
