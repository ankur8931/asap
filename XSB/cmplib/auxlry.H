/* File:      auxlry.H
** Author(s): Saumya Debray, Kostis F. Sagonas
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
** $Id: auxlry.H,v 1.21 2010-10-07 01:21:40 kifer Exp $
** 
*/


:- export message/1, warning/1, error/1, time_message/2, 
	  setoptions/1, setoptions1/1, subsumes/2, dict_lookup/3, option/1,
	  verbo_time_message/2, subsumed_member/2, ll_arg/3,
	  freeze/1, melt/2, writetop/2, set_global_compiler_options/1,
              check_file_compiler_options/1,
	  numbervars_cmplib/3.
:- export current_xsb_compiler_option/1.

:- import current_compiled_module/1 from compile.
:- import load_dyn_module/1 from consult.
:- import conget/2, conset/2 from gensym.
:- import member/2 from basics.
:- import  '=..'/2, arg/3, atomic/1, cputime/1, functor/3, nl/1, op/3,
	  ttywritenl/2, write/2
     from standard.
:- import assert/1, retractall/1, (dynamic)/1 from assert.
%%:- import sys_exit/1 from shell.
:- import term_psc/2, term_type/2, psc_arity/2, term_arg/3
        from machine.
:- import misc_error/1 from error_handler.

:- import
	unset_preprocessing_done/0,
	enqueue_preprocessor/1
   from parse.

/* ------------------------ end of file auxlry.H ---------------------- */

