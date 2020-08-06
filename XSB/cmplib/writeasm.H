/* File:      writeasm.H
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
** $Id: writeasm.H,v 1.10 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export write_asmheader/4, write_asm_instructions/2.

:- import member/2 from basics.
:- import asm_inst/4 from asm_inst.
:- import nl/1, write/2, writeln/2, writeq/2,
          arg/3, functor/3 from standard.
:- import file_put/2, fmt_write/3 from file_io.
:- import sym_name/3, sym_env/2, sym_category/2, sort_by_index/2,
          sym_prop/2 from symtab.
:- import option/1 from auxlry.
