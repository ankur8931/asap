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
** $Id: storage_xsb.h,v 1.13 2010-08-19 15:03:37 spyrosh Exp $
** 
*/


#include "storage_xsb_defs.h"

/* data structure for the storage handle
   NAME is the name of the storage
   HANDLE is the pointer to the root of the trie or the index of the trie root
   SNAPSHOT_NUMBER is the snapshot number of the storage
   CHANGED is a flag that tell if the trie has been changed by a backtrackable
	   update since the last commit.
*/


typedef struct storage_handle STORAGE_HANDLE;
struct storage_handle {
  Cell             name;
  STORAGE_HANDLE  *next;
  Integer          handle;
  Integer          snapshot_number;
  xsbBool          changed;
};


extern STORAGE_HANDLE *storage_builtin(CTXTdeclc int builtin_number, Cell storage_name, prolog_term trie_type);

/* 127 is a prime that is close to 2^7 */
#define STORAGE_TBL_SIZE  127

#ifndef MULTI_THREAD
extern xsbHashTable bt_storage_hash_table;
#endif
