/* File:      ti_mod.H
** Author(s): Steve Dawson
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
** $Id: ti_mod.H,v 1.8 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export ti_pred_block/3, force_ti_mod_loaded/0.

:- import option/1 from auxlry.
:- import str_cat/3 from string.
:- import gensym_pred/2, cmp_gensym/2 from preproc.
:- import membercheck/2 from listutil.
:- import sym_name/3, sym_insert/5, sym_pass_thru/1 from symtab.
:- import integer/1, number/1, atom_codes/2, number_codes/2 from standard.

