/* File:      push_io.H
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2000
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
** $Id: push_io.H,v 1.1.4.2 2004/10/18 20:48:22 ruim Exp $
** 
*/



:- import abort/1, atom/1 from standard.
:- import see/1, seeing/1, seen/0 from standard.
:- import assert/1, retractall/1, (dynamic)/1 from assert.

:- export push_stdin/2, pop_stdin/1.

