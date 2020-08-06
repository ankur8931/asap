/* File:      string.H
** Author(s): Xu, Swift, Sagonas
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
** $Id: string.H,v 1.19 2005/01/14 18:32:04 ruim Exp $
** 
*/


:- export concat_atom/2, concat_atom/3.
:- export
        str_cat/3, str_sub/2, str_sub/3, str_length/2, substring/4,
	str_match/5, string_substitute/4.
:- export term_to_codes/2, term_to_atom/2, codes_to_term/2, atom_to_term/2.
:- export term_to_codes/3, term_to_atom/3.
:- export
        read_atom_to_term_print_on_error/3,
        read_atom_to_term/3,
        read_atom_to_term/2.

:- import append/3, memberchk/2, copy_term/2, length/2 from basics.
:- import str_len/2 from machine.
:- import
        atom/1, atom_codes/2, number/1, integer/1, structure/1,
        '=..'/2,
        is_list/1, number_codes/2, float/1,
        error_writeln/1,
        warning/1
   from standard.
:- import file_open/3, file_close/1 from file_io.
:- import
        file_read/3,
        file_read_foe/3
   from xsb_read.
:- import domain_error/4, check_nonvar_list/3, type_error/4 from error_handler.

/* ---------------------- end of file string.H ------------------------ */
