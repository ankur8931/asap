/* File:      celltags_xsb.h
** Author(s): David S. Warren, Jiyang Xu, Terrance Swift
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
** $Id: celltags_xsb.h,v 1.9 2010-08-19 15:03:36 spyrosh Exp $
** 
*/



/* ==== types of cells =================================================*/

#define XSB_FREE	0	/* Free variable */
#define XSB_REF		0	/* Reference */
#define XSB_STRUCT	1	/* Structure */
#define XSB_INT     	2	/* integer */
#define XSB_LIST	3	/* List */
#define XSB_REF1	4	/* XSB_REF */
#define XSB_STRING  	5	/* Non-Numeric Constant (Atom) */
#define XSB_FLOAT	6	/* Floating point number */
#define XSB_ATTV	7	/* Attributed variable */

#define XSB_CELL_TAG_NBITS	3

#define XSB_TrieVar XSB_FREE    /* Cell tag of a standardized var in tries */
