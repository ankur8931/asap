/* File:      storage_xsb.h  -- support for the storage.P module
** Author(s): Michael Kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2001
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
** $Id: storage_xsb_defs.h,v 1.8 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#define GET_STORAGE_HANDLE     	   1
#define INCREMENT_STORAGE_SNAPSHOT 2
#define MARK_STORAGE_CHANGED       3
#define DESTROY_STORAGE_HANDLE     4
#define SHOW_TABLE_STATE           5
