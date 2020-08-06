/* File:      tabdef.H
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
** $Id: tabdef.H,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export perform_table_all/1, perform_auto_table/1,
	  call_themselves/3, table_it/1.

:- import ll_arg/3 from auxlry.
:- import message/1, error/1 from auxlry.
:- import sym_prop/2, sym_propin/2, sym_gen/2, sym_name/3 from symtab.

/* ----------------------- end of file tabdef.H ----------------------- */
