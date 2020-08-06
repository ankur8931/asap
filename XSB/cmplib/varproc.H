/* File:      varproc.H
** Author(s): Jiyang Xu, Terrance Swift, Kostis F. Sagonas
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
** $Id: varproc.H,v 1.10 2011-06-25 02:54:47 dwarren Exp $
** 
*/



:- export force_varproc_loaded/0.
:- export varproc/3, vprag_common/2, vprag_argno/2, vprag_level/2,
   vprag_context/2, vprag_loc/2, vprag_locnew/2, vprag_occ/2, vprag_type/2, 
	  vprag_use/2, vprag_nouse/2, vprag_lastchunk/2, vprag_type1/2.
:- export addkey/4, find/3, leaflist/2, updatevs/5.

:- import append/3, member/2, memberchk/2 from basics.
:- import closetail/1, member2/2 from listutil.
:- import sort/2 from setof.

