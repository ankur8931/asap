/* File:      tp_var.H
** Author(s): Saumya Debray, Jiyang Xu, Terrance Swift
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
** $Id: tp_var.H,v 1.10 2011-06-25 02:54:47 dwarren Exp $
** 
*/



:- export force_tp_var_loaded/0.
:- export putvarinst/5, getvarinst/5, bldvarinst/5, univarinst/5,
	  release/3, alloc_reg/3, release_if_done/6, hold/3, getreg/2,
	  reserve/3, alloc_reg1/4, release_if_done0/5.

:- import abort/0 from standard.
:- import append/3, memberchk/2 from basics.
:- import member2/2 from listutil.
:- import sort/2 from setof.
:- import error/1, warning/1 from auxlry.
:- import vprag_occ/2, vprag_loc/2, vprag_locnew/2, vprag_use/2, vprag_type/2,
	  vprag_nouse/2, vprag_type1/2
     from varproc.
