/* File:      inprog.H
** Author(s): Saumya Debray
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
** $Id: inprog.H,v 1.10 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


:- export force_inprog_loaded/0, inprog/3.

:- import gennum/1 from gensym.
:- import sym_name/3, sym_type/2 from symtab.
:- import varproc/3, vprag_common/2, vprag_argno/2, vprag_level/2,
	  vprag_context/2
     from varproc.
:- import error/1 from auxlry.
:- import abort/0 from standard.
:- import find/3, addkey/4 from varproc.
