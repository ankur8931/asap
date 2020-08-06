/* File:      x_interp.H
** Author(s): David Warren
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
** $Id: xnmr.H,v 1.5 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

:- export wfs/0.

:- export set_answerset_mode/0, set_nmr_mode/0.
:- export (-)/1.

:- import [] / 0 from usermod.	% just to make usermod typed "module"
:- import numbervars/1 from num_vars.
:- import file_write/2, file_nl/1 from xsb_writ.
:- import file_read/3, vv/2 from xsb_read.
:- import xsb_configuration/2 from xsb_configuration.
:- import call_expose/1, goal_cut_trans/3,
	  catch/3,
	  is_most_general_term/1, (table)/1, repeat/0,
	  (import)/1 
     from standard.
:- import conpsc/2, psc_set_type/2, psc_prop/2, psc_set_prop/2,
          term_psc/2, conname/2, stat_flag/2, stat_set_flag/2, 
	  trimcore/0, intern_string/2, slash/1
   from machine.
:- import set_inthandler/2 from loader.
:- import file_get/2, file_flush/2, fmt_write_string/3, print_file_contents/2
   from file_io.
:- import get_calls/3, get_returns/3, abolish_all_tables/0,
	  table_state/2, (use_variant_tabling)/1
    from tables.

:- import consult/1 from consult.
:- consult(xasppkg).

:- import coninc/1, conget/2 from gensym.

:- import init_smodels/1, a_stable_model/0, print_current_stable_model/0,
	in_all_stable_models/2, set_query_true/1
        from xnmr_int.

:- import retractall/1 from assert.
:- import '$$exception_ball'/1 from usermod.