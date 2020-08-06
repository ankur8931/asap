/* File:      domain.H
** Author(s): Jiyang Xu, Kostis Sagonas
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
** $Id: domain.H,v 1.1.1.1.8.2 2004/10/18 20:48:19 ruim Exp $
** 
*/


:- export ':'/2, int/3, int/1, intype/1, '/='/2, plus/3, times/3, in/2.
:- export deleteff/3, deleteff_p/3, generate/1, generate_ff/1.

:- import integer/1, arg/3, write/1, nl/0, call/1, op/3 from standard.
:- import dom_range/3, dom_size/2, dom_elem/2, dom_diff/3, dom_enum/2,
	dom_plus/3, dom_times/3, dom_type/2, dom_min/2,
	term_type/2, term_psc/2, term_new/2, psc_arity/2, dom_esize/2
    from machine.

