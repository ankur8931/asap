/* File:      tprog.H
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
** $Id: tprog.H,v 1.11 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export compile_clause/5.
:- export force_tprog_loaded/0.

:- import option/1 from auxlry.
:- import inprog/3 from inprog.
:- import merge/3 from listutil.
:- import flatten/2 from flatten.
:- import gennum/1 from gensym.
:- import geninline/9 from tpinline.
:- import inline_builtin/3 from builtin.
:- import release/3, reserve/3 from tp_var.
:- import tcond/9, tvar_hold/3 from tp_cond.
:- import sym_name/3, sym_type/2 from symtab.
:- import tgetpar/6, tgoalargs/7 from tp_goal.

