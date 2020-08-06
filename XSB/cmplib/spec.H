/* File:      spec.H
** Author(s): Kostis Sagonas
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
** $Id: spec.H,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export spec/2.

:- import name/2, atom_codes/2, ttywritenl/2 from standard.
:- import conget/2, conset/2 from gensym.
:- import memberchk/2, append/3 from basics.
:- import gensym_pred/2, have_hardcut/2 from preproc.
:- import warning/1, message/1, error/1, option/1 from auxlry.
:- import sym_name/3, sym_insert/5, sym_prop/2, sym_propin/2,
	  sym_type/2, add_new_clause/2, clause_listify/2
     from symtab.

/*----------------------- end of file spec.H ---------------------------*/
