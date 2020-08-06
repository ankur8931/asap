/* File:      asm.H
** Author(s): Saumya Debray, Kostis Sagonas, Terrance Swift
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
** $Id: asm.H,v 1.10 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


:- export asm_genheader/4, asm_PIL/5, asm_magic/2, force_asm_loaded/0.

:- import insert/2 from listutil.
:- import asm_opt/5 from asm_opt.
:- import asm_inst/4 from asm_inst.
:- import term_hash/3, str_len/2 from machine.
:- import write_int/2 from xsb_writ.
:- import log_ith/3 from basics.
:- import write/2 from standard.
:- import file_put/2 from file_io.
:- import asm_gendata/2, asm_genindex/3, asm_gentext/3, asm_putlength/2
     from asmpass2.
:- import sym_name/3, sym_count/2, sym_propin/2, sort_by_index/2 from symtab.

