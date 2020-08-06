/* File:      preproc.H 
** Author(s): Saumya Debray, Terrance Swift, Kostis Sagonas
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
** $Id: preproc.H,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export force_preprocess_loaded/0. 
:- export preprocess/3, cmp_gensym/2, gensym_pred/2, have_hardcut/2.

:- import option/1 from auxlry.
:- import compl_cond/5 from pre_cond.
:- import sym_prop/2, sym_insert/5, sym_name/3 from symtab.
:- import str_cat/3 from machine.
:- import conget/2, coninc/2 from gensym.
:- import append/3 from basics.
:- import inline_jumpcof_cond/3 from builtin.
:- import number_codes/2, atom_codes/2 from standard.
