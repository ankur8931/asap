/* File:      util.H	-- general-purpose utilies
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
** $Id: util.H,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- import append/3, length/2, ground/1 from basics.
:- import absmerge/3, absmember/2 from listutil.

:- export abs_intersect/3.
:- export abs_exclude/3.
:- export test_assert/1.
:- export seq_list_term/2, seq_term/3.
:- export same_length/2.
:- export sandwich/4, sandwich/6.
:- export pe_term/2.
:- export test_assert/1.
:- export get_expr_var/2.
