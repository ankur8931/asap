/* File:      gensym.H 
** Author(s): Kostis F, Sagonas, Jiyang Xu 
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1986, 1993-1999
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
** $Id: gensym.H,v 1.8 2004/02/25 21:24:07 tswift Exp $
** 
*/


:- export gensym/2, gennum/1, gennum/2, prepare/0, prepare/1.

:- export conlength/2, conset/2, conget/2, coninc/1, coninc/2.
:- export newcounter/1, inccounter/1, getcounter/1.

:- import psc_set_prop/2, 
	%% conpsc/2,
	conname/2,psc_prop/2, 
	str_len/2, str_cat/3, psc_set_type/2, term_psc/2
     from machine.
:- import number_codes/2, atom_codes/2, arg/3,
	integer/1, atom/1, abort/1
     from standard.
