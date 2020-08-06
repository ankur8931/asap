/* File:      dcg.H
** Author(s): Kostis F. Sagonas
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
** $Id: dcg.H,v 1.14 2004/03/11 18:11:44 dwarren Exp $
** 
*/


:- export dcg/2, 'C'/3, phrase/2, phrase/3, set_dcg_style/1.
:- export tphrase/2, tphrase/3, tphrase/1, reset_dcg_mode/0.
:- export tphrase_set_string/1, tphrase_set_string_keeping_tables/1, 
  tphrase_string_length/1, tphrase_add_word/3, tphrase_print_words/0.
:- export tphrase_set_string_auto_abolish/1, set_dcg_supported_table/1.
:- export set_dcg_terminal_functor/2.


:- import abort/1, arg/3, call/1, is_list/1, functor/3,
   write/1, writeln/1, warning/1, (index)/3, '=..'/2
   from standard.
:- import xsb_flag/2 from curr_sym.
:- import stat_set_flag/2, term_new_mod/3 from machine.
:- import assert/1, retractall/1, (dynamic)/1, clause/2 from assert.
:- import abolish_all_tables/0, invalidate_tables_for/2, table_state/4
	from tables.
:- import (:-)/2 from usermod.
:- import invalidate_table_for/2 from usermod.

/* ----------------------- end of file dcg.H -------------------------- */

