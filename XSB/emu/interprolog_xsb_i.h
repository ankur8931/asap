/* File:      xsb_intrprolog_i.h
** Author(s): Swift, to integrate InterProlog System of Calejo
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
** $Id: interprolog_xsb_i.h,v 1.8 2010-08-19 15:03:36 spyrosh Exp $
** 
*/


/* TLS: this follows vaguely the model of the ODBC interface.
   However, if there turns out to be more than one builtin needed, we
   need only add another switch statement */

#ifdef XSB_INTERPROLOG

  case INTERPROLOG_CALLBACK: 
          return interprolog_callback(CTXT); 

#endif



