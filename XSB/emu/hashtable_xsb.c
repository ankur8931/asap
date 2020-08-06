/* File:      hashtable_xsb.c  -- a simple generic hash table ADT
** Author(s): Michael Kifer
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2002
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
** $Id: hashtable_xsb.c,v 1.20 2013-05-06 21:10:24 dwarren Exp $
** 
*/


#include "xsb_config.h"
#include "xsb_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "psc_xsb.h"
#include "cinterf.h"
#include "trie_internals.h"
#include "tab_structs.h"
#include "error_xsb.h"

#include "tr_utils.h"
#include "debug_xsb.h"
#include "flags_xsb.h"
#include "memory_xsb.h"
#include "heap_xsb.h"

/*
  A simple hashtable.
  The first bucket is integrated into the hashtable and the overflow bullers
  are calloc()'ed. This gives good performance on insertion and search.
  When a collision happens on insertion, it requires a calloc().
  But collisions should be rare. Otherwise, should use a bigger table.
*/


#define table_hash(val, length)    ((UInteger)(val) % (length))

#define get_top_bucket(htable,I) \
    	((xsbBucket *)(htable->table + (htable->bucket_size * I)))
/* these two make sense only for top buckets, because we deallocate overflow
   buckets when they become free. */
#define mark_bucket_free(bucket,size)   memset(bucket,(Cell)0,size)
#define is_free_bucket(bucket)          (bucket->name == (Cell)0)

static void init_hashtable(CTXTdeclc xsbHashTable *table);


xsbBucket *search_bucket(CTXTdeclc Cell name,
			 xsbHashTable *table,
			 enum xsbHashSearchOp search_op)
{
  xsbBucket *bucket, *prev;

  if (! table->initted) init_hashtable(CTXTc table);

  prev = NULL;
  bucket = get_top_bucket(table,table_hash(name,table->length));
  while (bucket && bucket->name) {
    if (bucket->name == name) {
      if (search_op == hashtable_delete) {
	if (!prev) {
	  /* if deleting a top bucket, copy the next bucket into the top one
	     and delete that next bucket. If no next, then just nullify name */
	  prev = bucket;
	  bucket=bucket->next;
	  if (bucket) {
	    /* use memcpy() because client bucket might have extra fields */
	    memcpy(prev, bucket, table->bucket_size);
	    mem_dealloc(bucket,table->bucket_size,HASH_SPACE);
	    //printf("dealloc bucket size: %d\n",table->bucket_size);
	  } else {
	    mark_bucket_free(prev,table->bucket_size);
	    xsb_dbgmsg((LOG_HASHTABLE,
		       "SEARCH_BUCKET: Destroying storage handle for %s\n",
		       string_val(name)));
	    xsb_dbgmsg((LOG_HASHTABLE, 
		       "SEARCH_BUCKET: Bucket nameptr is %p, next bucket %p\n",
		       prev->name, prev->next));
	  }
	} else {
	  /* Not top bucket: rearrange pointers & free space */
	  prev->next = bucket->next;
	  mem_dealloc(bucket,table->bucket_size,HASH_SPACE);
	  //printf("dealloc bucket size: %d\n",table->bucket_size);
	}
	return NULL;
      } else
	return bucket;
    }
    prev = bucket;
    bucket = bucket->next;
  }
  /* not found */
  if (search_op != hashtable_insert) return NULL;
  /* else create new bucket */
  /* calloc nullifies the allocated space; CLIENTS RELY ON THIS */
  if (!bucket) { /* i.e., it is not a top bucket */
    bucket = (xsbBucket *)mem_calloc(1,table->bucket_size,HASH_SPACE);
    //printf("calloc bucket, size: %d\n",table->bucket_size);
    prev->next = bucket;
    /* NOTE: not necessary to nullify bucket->next because of calloc() */
  }
  bucket->name = name;
  return bucket;
}


static void init_hashtable(CTXTdeclc xsbHashTable *table)
{
  /* calloc zeroes the allocated space; clients rely on this */
  table->table = (byte *)mem_calloc(table->length,table->bucket_size,HASH_SPACE);
  //printf("calloc table, size: %d\n",table->length*table->bucket_size);
  table->initted = TRUE;
}

void destroy_hashtable(xsbHashTable *table)
{
  int i;
  xsbBucket *bucket, *next;

  table->initted = FALSE;
  for (i=0; i < table->length; i++) {
    /* follow pointers and free up buckets */
    bucket=get_top_bucket(table,i)->next;
    while (bucket != NULL) {
      next = bucket->next;
      mem_dealloc(bucket,table->bucket_size,HASH_SPACE);
      //printf("dealloc bucket size: %d\n",table->bucket_size);
      bucket = next;
    }
  }
  mem_dealloc(table->table,table->length*table->bucket_size,HASH_SPACE);
  //printf("dealloc table size: %d\n",table->length*table->bucket_size);

}



void show_table_state(xsbHashTable *table)
{
  xsbBucket *bucket;
  int i;

  xsb_dbgmsg((LOG_DEBUG,"\nCell Status\tOverflow Count\n"));
  for (i=0; i < table->length; i++) {
    bucket = get_top_bucket(table,i);
    if (is_free_bucket(bucket)) {
      /* free cell */
      xsb_dbgmsg((LOG_DEBUG, "   ---\t\t   ---"));
    } else {
      int overflow_count=0;

      fprintf(stddbg, "  taken\t\t");
      bucket = bucket->next;
      while (bucket != NULL) {
	overflow_count++;
	bucket = bucket->next;
      }
      xsb_dbgmsg((LOG_DEBUG,"   %d", overflow_count));
    }
  }
}

#ifndef MULTI_THREAD
// only handles the one hashtable.
extern xsbHashTable bt_storage_hash_table;
#endif

void mark_hash_table_strings(CTXTdecl) {
  xsbBucket *bucket;
  int i;
  xsbHashTable *table;

  table = &bt_storage_hash_table;
  if (table->initted==TRUE) {
    for (i=0; i < table->length; i++) {
      bucket = get_top_bucket(table,i);
      if (!is_free_bucket(bucket)) {
	if (bucket->name && isstring(bucket->name)) {
	  mark_string(string_val(bucket->name),"hashtable");
	}
	bucket = bucket->next;
	while (bucket != NULL) {
	  if (bucket->name && isstring(bucket->name)) {
	    mark_string(string_val(bucket->name),"hashtable-b");
	  }
	  bucket = bucket->next;
	}
      }
    }
  }
}
