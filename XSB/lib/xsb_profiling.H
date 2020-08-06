/* File:      xsb_profiling.H
** Author(s): David S. Warren
**
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1998
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
** 
*/

:- export profile_call/1, start_profiling/0, stop_profiling/0.

:- export profile_mode_init/0, profile_mode_call/1, profile_mode_dump/0.

:- export profile_unindexed_calls/1.

:- import parsort/4 from machine.
:- import psc_name/2 from machine. 
:- import psc_arity/2 from machine.
:- import term_psc/2, term_new/2, psc_arity/2, stat_set_flag/2, 
   term_new_mod/3 from machine.

:- import call/1 from standard.
:- import cputime/1 from standard.
:- import nl/0 from standard.
:- import tab/1 from standard.
:- import tell/1 from standard.
:- import telling/1 from standard.
:- import write/1 from standard.
:- import writeln/1 from standard.
:- import catch/3 from standard.
:- import throw/1 from standard.
:- import ground_term/1 from standard.
:- import call_c/1, (index)/2, arg/3, writeln/2, write/2 from standard.
:- import functor/3 from standard.
:- import warning/1 from standard.

:- import write_term/2 from write_term.

:- import findall/3 from setof.
:- import sort/2 from setof.

:- import gensym/2, conset/2, conget/2, coninc/1 from gensym.

:- import (dynamic)/1, retractall/1, assert/1 from assert.

:- import member/2 from basics.

