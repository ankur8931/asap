/* File:      comp.H
** Author(s): Yifei Dong
** Contact:   lmc@cs.sunysb.edu
** 
** Copyright (C) SUNY at Stony Brook, 1998-2000
** 
** XMC is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XMC is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XMC; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: comp.H,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


:- export xlcomp/0.

:- import
	append/3, member/2 from basics.

:- import
	sk_not/1 from tables.

:- import 
	absmerge/3, 
	closetail/1
	from listutil.
:- import
	abs_intersect/3, abs_exclude/3,
	test_assert/1,
	seq_list_term/2,
	seq_term/3,
	sandwich/4,
	sandwich/6,
	same_length/2,
	get_expr_var/2
	from util.
:- import
	rec_quad/6
	from quad.
