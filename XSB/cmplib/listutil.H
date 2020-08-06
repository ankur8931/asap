/* File:      listutil.H
** Author(s): Kostis F. Sagonas, Saumya Debray
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
** $Id: listutil.H,v 1.8 2011-09-06 17:33:46 dwarren Exp $
** 
*/


:- export force_listutil_loaded/0.
:- export attach/2, merge/3, absmember/2, member2/2, length/2,
	  closetail/1, insert/2, membercheck/2, nthmember/3,
	  absmerge/3, delete/3, delete_ith/4, reverse/2.
:- export build_bt/2,find_in_bt/2.

:- import member/2 from basics.

:- import sort/2 from setof.

