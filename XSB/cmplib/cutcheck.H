/* File:      cutcheck.H
** Author(s): K. Dvortsova
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** $Id: cutcheck.H,v 1.6 2010-08-19 15:03:35 spyrosh Exp $
** 
*/


:- export build_graph/1. 

:- import error/1 from auxlry.
:- import select/3 from basics.
:- import standard_symbol/3 from std_xsb.
:- import sym_name/3, sym_prop/2, sym_propin/2 from symtab. 

/* -------------------- end of file cutcheck.H ------------------------ */

