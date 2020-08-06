/* File:      hilog_symbol.H
** Author(s): Kostis F. Sagonas
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
** $Id: hilogsym.H,v 1.2 1998/11/15 06:09:15 kifer Exp $
** 
*/


:- export '_$hilog_symbol'/1, add_hilog_symbol/1. 


:- import assertai/2, (dynamic)/1 from assert.
:- import atom/1, abort/1 from standard.


/* ----------------- end of file hilog_symbol.H ----------------------- */

