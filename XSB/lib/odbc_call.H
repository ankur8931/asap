/* File:      odbc_call.H
** Author(s): Lily Dong, David S. Warren
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** $Id: odbc_call.H,v 1.30 2012-12-08 18:16:05 dwarren Exp $
**
*/


:- export odbc_open/1, odbc_open/2,
   odbc_open/3, odbc_open/4,
   odbc_close/0, odbc_close/1,
   odbc_shutdown/0,
   odbc_sql/3, odbc_sql/4,
   findall_odbc_sql/3, findall_odbc_sql/4,
   odbc_sql_cnt/4,
   odbc_query/3,
   odbc_deallocate_query/1, odbc_deallocate_query/2,
   odbc_import/2, odbc_import/3, odbc_import/4,
   odbc_transaction/1, odbc_transaction/2,
   odbc_connect_option/5,
   set_connect_option/3, get_connect_option/3,
   odbc_get_schema/2, odbc_get_schema/3,
   odbc_show_schema/1, odbc_show_schema/2,
   odbc_user_tables/1, odbc_user_tables/2,
   odbc_insert/2, odbc_insert/3,
   odbc_delete/2, odbc_delete/3,
   odbc_drivers/2,
   odbc_data_sources/2,
   odbc_connection_exists/1,
   odbc_attach/2, odbc_attach/3,
   odbc_create_table/2, odbc_create_table/3,
   odbc_create_index/3, odbc_create_index/4,
   odbc_delete_table/1, odbc_delete_table/2,
   odbc_delete_view/1, odbc_delete_view/2,
   odbc_delete_index/2, odbc_delete_index/3,
   odbc_get_info/2, odbc_get_info/3,
   odbc_flag/2, set_odbc_flag/2, 
   odbc_flag/3, % obsolescent.
   odbc_current_cursor/1, odbc_deactivate_cursor/1.
  

   % import some necessary predicates from library.

:- import call0/1, xsb_backtrace/1 from machine.
:- import conset/2, conget/2, coninc/1 from gensym.
:- import length/2, append/3, member/2, copy_term/2, memberchk/2 from basics.
:- import domain_error/4 from error_handler.

:- import (dynamic)/1, retractall/1, abolish/1, asserta/1, assert/1,
   retract/1
   from assert.

:- import writeln/1, writeln/2, write/1 , '=..'/2, atomic/1, atom_codes/2,
   atom/1, number_codes/2, name/2, number/1, integer/1, nl/0,
   '\='/2, functor/3, abort/1, structure/1, throw/1
   from standard.

:- import concat_atom/2 from string.

:- import findall/3, setof/3, bagof/3
   from setof.

:- import numbervars/1 from num_vars.

:- import ','/2, (:-)/2, '/'/2 from usermod.
