/* File:      dynamic_stack.c
** Author(s): Ernie Johnson
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
** $Id: dynamic_stack.c,v 1.15 2011-08-06 19:14:20 tswift Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include "debugs/debug_tries.h"

#include <stdio.h>
#include <stdlib.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "tst_aux.h"  /* needs cell_xsb.h */
#include "error_xsb.h"
#include "tries.h"
#include "debug_xsb.h"
#include "flags_xsb.h"
#include "memory_xsb.h"

/*-------------------------------------------------------------------------*/

/*
 * Prints the fields of the given DynamicStack.  `comment' is useful during
 * debugging, e.g. "just after initialization".
 */

void dsPrint(DynamicStack ds, char *comment) {

  xsb_dbgmsg((LOG_DEBUG, "Dynamic Stack: %s (%s)\n"
	     "  Stack Base:    %8p\tFrame Size:   %u bytes\n"
	     "  Stack Top:     %8p\tCurrent Size: %u frames\n"
	     "  Stack Ceiling: %8p\tInitial Size: %u frames",
	     DynStk_Name(ds), comment,
	     DynStk_Base(ds), DynStk_FrameSize(ds),
	     DynStk_Top(ds), DynStk_CurSize(ds),
	     DynStk_Ceiling(ds), DynStk_InitSize(ds)));
}

/*-------------------------------------------------------------------------*/

/*
 * Initialize a stack for use.  Allocates a block of memory for the stack
 * area and sets the fields of the DynamicStack structure.
 */

void dsInit(DynamicStack *ds, size_t stack_size, size_t frame_size,
	    char *name) {

  size_t total_bytes;

  xsb_dbgmsg((LOG_TRIE_STACK, "Initializing %s", name));

  total_bytes = stack_size * frame_size;
  if (total_bytes > 0) {
    DynStk_Base(*ds) = mem_alloc(total_bytes,TABLE_SPACE);
    if ( IsNULL(DynStk_Base(*ds)) )
      xsb_resource_error_nopred("memory","Ran out of memory in allocation of %s", DynStk_Name(*ds));
  } else DynStk_Base(*ds) = NULL;
  DynStk_Top(*ds) = DynStk_Base(*ds);
  DynStk_Ceiling(*ds) = (char *)DynStk_Base(*ds) + total_bytes;
  DynStk_FrameSize(*ds) = frame_size;
  DynStk_InitSize(*ds) = DynStk_CurSize(*ds) = stack_size;
  DynStk_Name(*ds) = name;
}

/*-------------------------------------------------------------------------*/

/*
 * `num_frames' are the number of frames that are needed immediately.
 * Here we make sure that the expanded size can accommodate this need.
 */

void dsExpand(DynamicStack *ds, size_t num_frames) {

  size_t new_size, total_bytes;
  char *new_base;

  if ( num_frames < 1 )
    return;
  if ( DynStk_CurSize(*ds) > 0 )
    new_size = 2 * DynStk_CurSize(*ds);
  else
    new_size = DynStk_InitSize(*ds);
  if ( new_size < DynStk_CurSize(*ds) + num_frames )
    new_size = new_size + num_frames;

  xsb_dbgmsg((LOG_TRIE_STACK, "Expanding %s: %d -> %d", DynStk_Name(*ds),
	     DynStk_CurSize(*ds), new_size));
  dbg_dsPrint(LOG_TRIE_STACK, *ds, "Before expansion");

  total_bytes = new_size * DynStk_FrameSize(*ds);
  new_base = mem_realloc(DynStk_Base(*ds),DynStk_CurSize(*ds)*DynStk_FrameSize(*ds),total_bytes,TABLE_SPACE);
  if ( IsNULL(new_base) && total_bytes > 0)
    xsb_resource_error_nopred("memory","Ran out of memory during expansion of %s", DynStk_Name(*ds));
  DynStk_Top(*ds) =
    new_base + ((char *)DynStk_Top(*ds) - (char *)DynStk_Base(*ds));
  DynStk_Base(*ds) = new_base;
  DynStk_Ceiling(*ds) = new_base + total_bytes;
  DynStk_CurSize(*ds) = new_size;

  dbg_dsPrint(LOG_TRIE_STACK, *ds, "After expansion");
}

/*-------------------------------------------------------------------------*/

/*
 * Reduces the size of the memory block allocated back to the initial size
 * specified during initialization.
 */

void dsShrink(DynamicStack *ds) {

  size_t total_bytes;
  char *new_base;

  if ( DynStk_CurSize(*ds) <= DynStk_InitSize(*ds) )
    return;
  total_bytes = DynStk_InitSize(*ds) * DynStk_FrameSize(*ds);
  new_base = mem_realloc(DynStk_Base(*ds),DynStk_CurSize(*ds)*DynStk_FrameSize(*ds),total_bytes,TABLE_SPACE);

  xsb_dbgmsg((LOG_TRIE_STACK, "Shrinking %s: %d -> %d", DynStk_Name(*ds),
	     DynStk_CurSize(*ds), DynStk_InitSize(*ds)));

  if ( IsNULL(new_base) && total_bytes > 0 )
    xsb_resource_error_nopred("memory","Ran out of memory during expansion of %s", DynStk_Name(*ds));
  DynStk_Top(*ds) =
    new_base + ((char *)DynStk_Top(*ds) - (char *)DynStk_Base(*ds));
  DynStk_Base(*ds) = new_base;
  DynStk_Ceiling(*ds) = new_base + total_bytes;
  DynStk_CurSize(*ds) = DynStk_InitSize(*ds);
}
