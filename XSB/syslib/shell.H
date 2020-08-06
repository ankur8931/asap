/* File:      shell.H
** Author(s): Kostis F. Sagonas, Jiyang Xu
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
** $Id: shell.H,v 1.20 2009/02/26 16:09:13 tswift Exp $
** 
*/


:- export cd/1, ls/0, rename/2, rm/1, rmdir/1, rmdir_rec/1, edit/1,
	expand_filename_no_prepend/2,
	cwd/1, create_lockfile/1, sys_cwd/2, sys_pid/1,
	shell/5, shell/2, shell/1, shell_to_list/3, sleep/1,
	copy/2, append/2,
	is_plain_file/1, is_directory/1,
	spawn_process/5, get_process_table/1, process_status/2, 
	process_control/2,
	exec/1, list_directory/2.
:- export sys_chdir/2, sys_access/3, sys_stat/3, sys_link/3,
	sys_mkdir/2, sys_rmdir/2,
	sys_unlink/2, sys_exit/1, sys_rename/3,
   sys_filecopy/3, sys_fileappend/3, sys_main_memory/1.
:- export platform/1.

:- import file_to_list/2 from scrptutl.
:- import append/3, member/2 from basics.
:- import findall/3 from setof.
:- import path_sysop/2 from file_io.
:- import check_nonvar/3 from error_handler.
:- import sys_system/7, expand_filename/2, sys_syscall/4,
  tilde_expand_filename/2, slash/1
   from machine.
:- import atom/1, abort/1, is_list/1 from standard.
:- import xsb_configuration/2 from xsb_configuration.

