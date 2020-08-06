/* File:      cell_def_xsb.h
** Author(s): David S. Warren, Jiyang Xu, Terrance Swift
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1993-1999
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
** $Id: cell_def_xsb.h,v 1.11 2011-05-18 19:21:40 dwarren Exp $
** 
*/


#ifndef CELL_DEF_XSB_INCL 
#define CELL_DEF_XSB_INCL 

/* CELL and PROLOG_TERM are defined identically.
   However, CELL is used to refer to elements of (slg-)WAM stacks, while
   PROLOG_TERM is used in the interface to point to a cell containing 
   the outer functor of a prolog term. */

#if defined(BITS64) && defined(WIN_NT)
typedef unsigned long long Cell;
#else
typedef unsigned long Cell;
#endif

typedef Cell *CPtr;

#endif
