/* File:      subsumes.P
** Author(s): Kostis F. Sagonas
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
** $Id: subsumes.H,v 1.8 2007/01/24 22:24:28 dwarren Exp $
** 
*/


:- export subsumes_chk/2, subsumes_term/2, variant/2, subsumes/2.

%%:- import numbervars/1 from num_vars.
%%:- import conset/2, conget/2 from gensym.
:- import term_psc/2, psc_arity/2, term_arg/3, term_type/2
     from machine.
:- import get_attributes/2,attv_unify/2 from machine.
%%:- import functor/3, arg/3 from standard.

:- import excess_vars/4 from setof.
:- import is_most_general_term/1 from standard.
