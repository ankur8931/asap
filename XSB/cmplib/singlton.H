/* File:      singlton.H
** Author(s): Sagonas
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
** $Id: singlton.H,v 1.13 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


:- export singleton_check/2,calculate_singleton/3.

:- import vv/2 from xsb_read.
:- import warning/1 from auxlry.
:- import numbervars_cmplib/3 from auxlry.
:- import option/1 from auxlry.
:- import append/3, copy_term/2, log_ith/3 from basics.
:- import atom_codes/2, functor/3, hilog_functor/3, atomic/1, integer/1,
	  arg/3, '=..'/2
     from standard.
