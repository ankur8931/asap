/* File:      tp_goal.H
** Author(s): Saumya Debray, Kostis Sagonas, Terrance Swift
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
** $Id: tp_goal.H,v 1.8 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



:- export force_tp_goal_loaded/0.
:- export tgoalargs/7, tgetpar/6, tputpar/7.

:- import alloc_reg1/4, release_if_done/6, getreg/2,
	getvarinst/5, putvarinst/5, univarinst/5, bldvarinst/5
   from tp_var.
:- import vprag_type/2, vprag_loc/2, vprag_context/2, vprag_nouse/2,
	vprag_use/2, vprag_lastchunk/2, vprag_argno/2, vprag_level/2,
	vprag_occ/2, vprag_type1/2
   from varproc.
:- import append/3, member/2 from basics.

