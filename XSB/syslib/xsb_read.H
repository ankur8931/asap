/* File:      xsb_read.H
** Author(s): Kostis F. Sagonas, Jiyang Xu
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
** $Id: xsb_read.H,v 1.24 2008/01/28 18:14:36 dwarren Exp $
** 
*/


:- export file_read/2, file_read/3, vv/2.
:- export file_read_foe/3, file_read_fapoe/3.
:- export vv/2.  %% this is a functor symbol, not a predicate
%%:- export iso_file_read/3.

:- import '_$op'/3 from file_op.
:- import file_put/2 from file_io.
:- import file_write0/2, file_nl/1 from xsb_writ.
:- import atom/1, atom_codes/2, repeat/0, '=..'/2 from standard.
:- import syntax_error/1 from error_handler.
:- import '_$hilog_symbol'/1 from hilogsym.
:- import term_new/2, term_set_arg/4, 
	  file_gettoken/5, psc_insert/4, conname/2, 
	  conpsc/2, term_psc/2, psc_set_prop/2, psc_prop/2
     from machine.
:- import banner_control/1 from banner.
:- import concat_atom/2 from string.
:- import length/2 from basics.
:- import xsb_stream_lock/1, xsb_stream_unlock/1 from file_io.
%%:- import xsb_thread_self/1 from thread.  %% needed for mt?
:- import current_prolog_flag/2 from curr_sym.
:- import file_put/2,file_pos/2 from file_io.
:- import length/2, ith/3 from basics.

/* -------------------- end of file file_read.H ----------------------- */
