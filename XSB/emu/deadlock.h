/* File:      deadlock.h
** Author(s): Marques
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
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public Livallcense for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
*/

#ifdef SHARED_COMPL_TABLES

extern counter num_deadlocks;
extern counter num_suspends;
                                                                                
int would_deadlock( Integer tid1, Integer tid2 ) ;
void reset_leader( th_context *th ) ;
void reset_other_threads( th_context *th, th_context *ctxt, VariantSF sgf ) ;

#endif

