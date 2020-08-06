/* File:      ora_call.H
** Author(s): Hasan Davulcu
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
** $Id: ora_call.H,v 1.11 2010-08-19 15:03:38 spyrosh Exp $
** 
*/


:- export db_open/1, db_close/0, db_flag/3, db_SQLCA/2, db_query/2.
:- export db_import/2, db_import_test/2, flush/0, db_sql/1,db_attach/2.
:- export db_transaction/1, db_sql_select/2, db_delete/2, db_insert/2.
:- export db_show_schema/1, db_get_schema/2, user_tables/1, db_get_schema/2.
:- export db_record/2, db_create_table/2, db_create_index/3, db_delete_table/1.
:- export db_delete_view/1, db_delete_index/1, db_add_record/2, delete_record/2.
:- export db_error_message/3, connect/0.
:- export '$ora_err$'/1, '$show_query$'/1, '$cursor_table$'/1.

:- import table_arity/2, ','/2, (:-)/2 from usermod.
:- import str_cat/3, oracle_query/7 from machine.
:- import conset/2, conget/2 from gensym.
:- import (dynamic)/1, assert/1, asserta/1, retract/1,
   retractall/1 from assert.
:- import write/1, writeln/1, (=..)/2, atom_codes/2 from standard.
:- import name/2, atom/1, integer/1, (\=)/2, atomic/1 from standard.
:- import nl/0, functor/3, number/1, call/1, (not)/1 from standard.
:- import findall/3, bagof/3 from setof.
:- import copy_term/2, append/3, member/2 from basics.
