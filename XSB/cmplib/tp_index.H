/* File:      tp_index.H
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
** $Id: tp_index.H,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export force_tp_index_loaded/0, makeindex/6, index_split/2.

:- import sym_name/3 from symtab.
:- import gennum/1 from gensym.
:- import member/2 from basics.
:- import length/2, nthmember/3 from listutil.
:- import warning/1 from auxlry.
