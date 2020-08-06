/* File:      xsb_writ.H
** Author(s): Kostis Sagonas
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
** $Id: xsb_writ.H,v 1.21 2008/04/03 19:38:05 tswift Exp $
** 
*/


:- export file_write/2, file_write0/2, file_write_prolog/2, 
   file_write_quoted/2, file_write_canonical/2, file_write_canonical_lettervar/2,
	file_nl/1.
:- export write_int/1, write_float/1, write_int/2, write_float/2.
:- export write_double/1, write_double/2.
:- export write_canonical_lettervar/1.
:- export set_file_write_depth/1, get_file_write_depth/1.
:- export l_write/3.
	
:- import '_$op'/3 from file_op.
:- import file_put/2 from file_io.
:- import file_function/3 from machine.
:- import integer/1 from standard.
:- import attribute_portray_hook/3,
   term_psc/2, psc_arity/2, psc_name/2, term_arg/3, 
   stat_flag/2,stat_set_flag/2,get_attributes/2,
   term_type/2, file_puttoken/3, file_writequoted/2
     from machine.
:- import call/1 from standard.

%:- import xsb_sys_mutex_lock/1, xsb_sys_mutex_unlock/1 from thread.
:- import xsb_stream_lock/1, xsb_stream_unlock/1 from file_io.
/* ------------------- end of file file_write.H ----------------------- */
