/* File:      printres.H
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
** $Id: printres.H,v 1.5 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export print_result/1.

:- import option/1 from auxlry.
:- import get_symtab/2 from parse.
:- import extract_symlist/3, sym_name/3 from symtab.
:- import write/1, nl/0, ttywritenl/1 from standard.
