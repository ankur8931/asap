/* File:      sanity.H
** Author(s): Kostis F. Sagonas
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
** $Id: sanity.H,v 1.7 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



:- export dump_module/2, write_preds/2.

:- import sym_name/3, sym_prop/2, sym_gen/2 from symtab.
:- import nl/1, write/2, writeq/2, integer/1, open/3, close/1
     from standard.
:- import file_put/2 from file_io.

/* --------------------- end of file sanity.H ------------------------- */

