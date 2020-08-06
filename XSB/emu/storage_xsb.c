/* File:      storage_xsb.c  -- support for the storage.P module
** Author(s): Michael Kifer
** Contact:   xsb-contact@cs.sunysb.edu
**
** Copyright (C) The Research Foundation of SUNY, 2001,2002
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
** $Id: storage_xsb.c,v 1.22 2011-05-18 19:21:40 dwarren Exp $
**
*/

#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "cinterf.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "error_xsb.h"
#include "tr_utils.h"
#include "storage_xsb.h"

#include "debug_xsb.h"
#include "flags_xsb.h"

/* this func would insert handle into hashtable, if it isn't there */
#define find_or_insert_storage_handle(name)  \
    	    (STORAGE_HANDLE *)search_bucket(CTXTc name,&bt_storage_hash_table,hashtable_insert)
#define destroy_storage_handle(name) \
    	    search_bucket(CTXTc name,&bt_storage_hash_table,hashtable_delete)
#define show_table_state()    show_table_state(&bt_storage_hash_table)

static STORAGE_HANDLE        *increment_storage_snapshot(CTXTdeclc Cell name);
static STORAGE_HANDLE        *mark_storage_changed(CTXTdeclc Cell name);

#ifndef MULTI_THREAD
xsbHashTable bt_storage_hash_table =
  {STORAGE_TBL_SIZE,sizeof(STORAGE_HANDLE),FALSE,NULL};
#endif

static inline STORAGE_HANDLE *get_storage_handle(CTXTdeclc Cell name, prolog_term trie_type)
{
  STORAGE_HANDLE *handle_cell;

  handle_cell = find_or_insert_storage_handle(name);
  /* new buckets are filled out with 0's by the calloc in hashtable_xsb.c */
  if (handle_cell->handle==(Cell)0) {
    /* initialize new handle */
    xsb_dbgmsg((LOG_STORAGE,
	       "GET_STORAGE_HANDLE: New trie created for %s\n",
	       string_val(name)));
    if (is_int(trie_type))
      handle_cell->handle= newtrie(CTXTc (int)p2c_int(trie_type));
    else
      xsb_abort("[GET_STORAGE_HANDLE] trie type (3d arg) must be an integer");

    /* Note: not necessary to initialize snapshot_number&changed: handle_cell
       was calloc()'ed
       handle_cell->snapshot_number=0;
       handle_cell->changed=FALSE;
    */
  }
  else
    xsb_dbgmsg((LOG_STORAGE,
	       "GET_STORAGE_HANDLE: Using existing trie for %s\n",
	       string_val(name)));
  return handle_cell;
}

STORAGE_HANDLE *storage_builtin(CTXTdeclc int builtin_number, Cell name, prolog_term trie_type)
{
  switch (builtin_number) {
  case GET_STORAGE_HANDLE:
    return get_storage_handle(CTXTc name, trie_type);
  case INCREMENT_STORAGE_SNAPSHOT:
    return increment_storage_snapshot(CTXTc name);
  case MARK_STORAGE_CHANGED:
    return mark_storage_changed(CTXTc name);
  case DESTROY_STORAGE_HANDLE: {
    xsb_dbgmsg((LOG_STORAGE,
	       "STORAGE_BUILTIN: Destroying storage handle for %s\n",
	       string_val(name)));
    destroy_storage_handle(name);
    return NULL;
  }
  case SHOW_TABLE_STATE: {
    show_table_state();
    return NULL;
  }
  default:
    xsb_abort("Unknown storage builtin");
    return NULL;
  }
}


static STORAGE_HANDLE *increment_storage_snapshot(CTXTdeclc Cell name)
{
  STORAGE_HANDLE *ptr = find_or_insert_storage_handle(name);
  ptr->snapshot_number++;
  ptr->changed = FALSE;
  return ptr;
}

static STORAGE_HANDLE *mark_storage_changed(CTXTdeclc Cell name)
{
  STORAGE_HANDLE *ptr = find_or_insert_storage_handle(name);
  ptr->changed = TRUE;
  return ptr;
}
