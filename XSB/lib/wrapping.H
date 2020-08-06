/* File:      wrapping.H
** Author(s): Luis Castro
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
** $Id: wrapping.H,v 1.7 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

:- export wrap/2, write_header/0, process_from/3.

:- import append/3, member/2, select/3, length/2 from basics.
:- import write/1, nl/0, atom_codes/2 from standard.
:- import arg/3, functor/3, told/0, seen/0, read/1 from standard.
:- import repeat/0, tell/1, see/1 from standard.
:- import fmt_write/2 from file_io.
:- import xsb_configuration/2 from xsb_configuration.
:- import throw/1, (index)/2 from standard.
:- import syntax_error/1 from error_handler.
:- import (:-)/1 from usermod.

