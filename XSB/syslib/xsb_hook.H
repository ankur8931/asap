/* File:      xsb_hook.P -- mechanism for adding hooks to xsb
** Author(s): kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1999
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
** $Id: xsb_hook.H,v 1.4.4.1 2004/09/29 19:44:30 ruim Exp $
** 
*/

:- import
	'=..'/2, atom/1, call/1, call_c/1, callable/1, abort/1, arg/3
	from standard.
:- import retractall/1, assert/1, (dynamic)/1 from assert.
:- import psc_type/2, term_psc/2, term_new/2, psc_arity/2 from machine.

:- export add_xsb_hook/1, remove_xsb_hook/1, call_xsb_hook/2, call_xsb_hook/3.

:- export
	xsb_exit_hook/1,
	xsb_undefined_predicate_hook/1,
	xsb_assert_hook/1,
	xsb_retract_hook/1,
	xsb_before_prompt_hook/1.
