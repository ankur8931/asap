/* File:      varstring.c
** Author(s): kifer
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
** $Id: varstring.c,v 1.29 2011-05-18 19:21:41 dwarren Exp $
** 
*/


/*
  Usage:

  XSB_StrDefine(foo);  // Declare foo as a variable length string
  XSB_StrDefine(boo);  // Declare boo as a variable length string
  VarString *goo;    // Declare a pointer to a varstring.

  foo.op->set(&foo,"abc");        // or XSB_StrSet(&foo, "abc");
  foo.op->append(&foo,"123");     // or XSB_StrAppend(&foo, "123");
  // shrink foo, set the increment to 5
  foo.op->shrink(&foo,5);         // or XSB_StrShrink(&foo,5);
  foo.op->prepend(&foo,"098");    // or XSB_StrPrepend(&foo,"098");

  boo.op->prepend("pasddsf");

  goo = &boo;
  goo->op->strcmp(goo, "jdshdd"); // or XSB_StrStrCmp(goo,"jdshdd");

  if (foo.length > 0)
     printf("%s   %d\n", foo.string, foo.length);

  printf("boo: %s\n", goo->string);

*/


/* To test: define the string below and compile: cc varstring.c; a.out */
#undef DEBUG_VARSTRING


#include "xsb_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>

#include "wind2unix.h"

#include "auxlry.h"
#include "context.h"
#include "cell_xsb.h"
#include "memory_xsb.h"
#ifndef DEBUG_VARSTRING
#include "error_xsb.h"
#endif

#include "varstring_xsb.h"


#define DEFAULT_VARSTR_INCREMENT     128
#define NEWLENGTH(vstr,additional_size) \
    	    	    	    	vstr->length + additional_size +1


static void  vs_init(VarString*, int);
static void  vs_set(VarString*, char*);
static void  vs_setv(VarString*, VarString*);
static void  vs_append(VarString*, char*);
static void  vs_prepend(VarString*, char*);
static inline void  vs_appendv(VarString*, VarString*);
static inline void  vs_appendc(VarString*, char);
static inline void  vs_prependv(VarString*, VarString*);
static inline int   vs_compare(VarString*, VarString*);
static inline int   vs_strcmp(VarString*, char*);
static inline void  vs_destroy(VarString*);
static inline void  vs_shrink(VarString*, int);
static inline void  vs_ensure_size(VarString*, int);

/* add char to the end of string, don't null-terminate */
static void  vs_appendblk(VarString *vstr,char*,int);
static void  vs_prependblk(VarString *vstr,char*,int);
static void  vs_adjust_size(VarString *vstr, int add_size);
static inline void vs_null_terminate(VarString *vstr);

/* VarStrOps does not get exported properly in Windows due to the peculiarities
   of the MS C++ compiler */
DllExport struct varstr_ops VarStrOps = {vs_set,vs_setv,
					 vs_append,vs_prepend, 
					 vs_appendv,vs_appendc,vs_prependv,
					 vs_compare,vs_strcmp,
					 vs_appendblk,vs_prependblk,
					 vs_null_terminate,
					 vs_ensure_size,
					 vs_shrink,vs_destroy};


DllExport void call_conv varstring_init(VarString *vstr)
{
  vstr->size = 0;
  vstr->increment = 0;
  vstr->length =0;
  vstr->string = NULL;
  vstr->op = &VarStrOps;
}

DllExport void call_conv varstring_create(VarString **vstr)
{
  *vstr = (VarString *) mem_alloc(sizeof(VarString),OTHER_SPACE); // never released!
  varstring_init(*vstr);
}


/* initialize a var string. This is the only function that isn't a member of
   the data structure, because somebody must assign functions to the function
   pointers inside struct varstr */
static void vs_init(VarString *vstr, int increment)
{
  if (vstr->string != NULL)
    return;

  if (increment < 1)
    increment = DEFAULT_VARSTR_INCREMENT;

  vstr->string = (char *)mem_calloc(1, increment,OTHER_SPACE);

  vstr->increment   = increment;
  vstr->size        = increment;
  vstr->length      = 0;
  vstr->string[0]   = '\0';
}


/* change the increment and also shrink the allocated space to the minimum */
static inline void vs_shrink(VarString *vstr, int increment)
{
  if (vstr->string == NULL)
    vs_init(vstr,increment);
  else { /* already initialized */
    vstr->increment = increment;
    /* make sure we don't clobber existing stuff */
    vs_adjust_size(vstr, vstr->length+1);
  }
}


/* ensure that the VarString has room for minsize bytes */
/* dsw changed, so it will never shrink the buffer */
static inline void vs_ensure_size(VarString *vstr, int minsize)
{
  vs_init(vstr,0);

  if (minsize > vstr->size) vs_adjust_size(vstr, minsize+1);
}


static void vs_set(VarString *vstr, char *str)
{
  size_t newlength;

  vs_init(vstr,0); /* conditional init */

  if (str == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "Assigning a NULL pointer to a variable-length string\n");
    return;
#else
    xsb_bug("Assigning a NULL pointer to a variable-length string");
#endif
  }

  newlength = strlen(str);

  //  vs_adjust_size(vstr, newlength+1); %% dsw changed to avoid too much realloc
  vs_ensure_size(vstr, (int)newlength+1);

  strcpy(vstr->string, str);
  vstr->length = (int)newlength;
}

static void vs_setv(VarString *vstr, VarString *vstr1)
{
  vs_set(vstr, vstr1->string);
}

static void vs_append(VarString *vstr, char *str)
{
  if (str == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "Appending a NULL string\n");
    return;
#else
    xsb_bug("Appending a NULL string");
#endif
  }
  vs_appendblk(vstr, str, (int)strlen(str));
  vs_null_terminate(vstr);
}

static inline void vs_appendc(VarString *vstr, char code) {
  if (vstr->size < vstr->length+2) // \0 + new one to add
    vs_adjust_size(vstr,vstr->length+2);
  *(vstr->string+vstr->length) = code;
  vstr->length++;
  *(vstr->string+vstr->length) = '\0';
}

static void vs_prepend(VarString *vstr, char *str)
{
  if (str == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "Appending a NULL string\n");
    return;
#else
    xsb_bug("Appending a NULL string");
#endif
  }
  vs_prependblk(vstr, str, (int)strlen(str));
}

static inline void vs_appendv(VarString *vstr, VarString *vstr1)
{
  vs_append(vstr, vstr1->string);
}

static inline void vs_prependv(VarString *vstr, VarString *vstr1)
{
  vs_prepend(vstr, vstr1->string);
}

static inline int vs_compare(VarString *vstr, VarString *vstr1)
{
  /* conditional init */
  vs_init(vstr,0);
  vs_init(vstr1,0);

  return strcmp(vstr->string, vstr1->string);
}

static inline int vs_strcmp(VarString *vstr, char *str)
{
  vs_init(vstr,0); /* conditional init */

  if (str == NULL) {
#ifdef DEBUG_VARSTRING
  fprintf(stderr, "Comparing string with a NULL pointer\n");
  return 0;
#else
  xsb_bug("Comparing string with a NULL pointer");
#endif
  }

  return strcmp(vstr->string, str);
}

/* destruction is necessary for automatic VarString's,
   or else there will be a memory leak */
static inline void  vs_destroy(VarString *vstr)
{
  if (vstr->string == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr,
	    "Attempt to deallocate uninitialized variable-length string\n");
    return;
#else
    return;
    //    xsb_bug("Attempt to deallocate uninitialized variable-length string");
#endif
  }
#ifdef DEBUG_VARSTRING
  fprintf(stderr,
	    "Deallocating a variable-length string\n");
#endif
  mem_dealloc(vstr->string,vstr->size,OTHER_SPACE);
  vstr->string    = NULL;
  vstr->size        = 0;
  vstr->length      = 0;
  vstr->increment   = 0;
}

/* append block of chars, don't NULL-terminate */
static void vs_appendblk(VarString *vstr, char *blk, int blk_size)
{
  int newlength;

  vs_init(vstr,0); /* conditional init */

  if (blk == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "Appending a NULL string\n");
    return;
#else
    xsb_bug("Appending a NULL string");
#endif
  }

  newlength = NEWLENGTH(vstr,blk_size);

  if (newlength > vstr->size)
    vs_adjust_size(vstr, newlength);

  strncpy(vstr->string+vstr->length, blk, blk_size);
  vstr->length=vstr->length + blk_size;
}


/* append block of chars */
static void vs_prependblk(VarString *vstr, char *blk, int blk_size)
{
  int newlength;

  vs_init(vstr,0); /* conditional init */

  if (blk == NULL) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "Prepending a NULL string\n");
    return;
#else
    xsb_bug("Prepending a NULL string");
#endif
  }

  newlength = NEWLENGTH(vstr,blk_size);

  if (newlength > vstr->size)
    vs_adjust_size(vstr, newlength);

  memmove(vstr->string+blk_size, vstr->string, vstr->length+1);
  strncpy(vstr->string, blk, blk_size);
  vstr->length=vstr->length + blk_size;
}


static inline void vs_null_terminate(VarString *vstr)
{
  int newlength;

  vs_init(vstr,0); /* conditional init */

  newlength = NEWLENGTH(vstr,0);

  if (newlength > vstr->size)
    vs_adjust_size(vstr, newlength);

  vstr->string[vstr->length] = '\0';
}


/* Adjust size to the next multiple of the increment after minsize;
   This can enlarge as well as shrink a VarString. 
   The caller must make sure that the existing data isn't clobbered by
   shrinking. */
static void vs_adjust_size(VarString *vstr, int minsize)
{
  int newsize;

  vs_init(vstr,0); /* conditional init */

  newsize = (minsize/vstr->increment +1) * (vstr->increment);

  if (NULL == (vstr->string = (char *)mem_realloc_nocheck(vstr->string, vstr->size, 
							  newsize,OTHER_SPACE))) {
#ifdef DEBUG_VARSTRING
    fprintf(stderr, "No room to expand a variable-length string\n");
    return;
#else
    vstr->size        = 0;
    vstr->length      = 0;
    xsb_memory_error("memory","No room to expand a variable-length string");
#endif
  }

#ifdef DEBUG_VARSTRING
  if (newsize > vstr->size)
    fprintf(stderr, "Expanding a VarString from %d to %d\n",
	    vstr->size, newsize);
  else if (newsize < vstr->size)
    fprintf(stderr, "Shrinking a VarString from %d to %d\n",
	    vstr->size, newsize);
#endif

  vstr->size = newsize;
}


#ifdef DEBUG_VARSTRING

int main (int argc, char** argv)
{
  XSB_StrDefine(foo);
  XSB_StrDefine(boo);
  XSB_StrDefine(goo);
  VarString *loo;

  XSB_StrSet(&foo, "abc");
  printf("foo1: %s   %d/%d\n", foo.string, foo.length, foo.size);
  foo.op->append(&foo, "123");
  printf("foo2: %s   %d/%d\n", foo.string, foo.length, foo.size);
  foo.op->prepend(&foo, "098");
  printf("foo3: %s   %d/%d\n", foo.string, foo.length, foo.size);

  boo.op->prepend(&boo, "booooooo");
  printf("boo: %s     %d/%d\n", boo.string, boo.length, boo.size);

  boo.op->shrink(&boo, 3);
  boo.op->appendv(&boo, &foo);

  loo = &boo;
  printf("boo2: %s     %d/%d\n", loo->string, boo.length, boo.size);

  foo.op->shrink(&foo,60);
  if (foo.length > 0)
      printf("foo4: %s   %d/%d\n", foo.string, foo.length, foo.size);

  boo.op->set(&boo,"123");
  XSB_StrAppend(loo,"---");
  printf("boo3: %s     %d/%d\n", loo->string, boo.length, boo.size);

  boo.op->append(&boo,"4567");
  printf("boo4: %s     %d/%d\n", loo->string, boo.length, boo.size);

  printf("initialized: boo=%p  foo=%p  goo=%p\n",
	 boo.string, foo.string, goo.string);

  XSB_StrSetV(loo, &foo);
  printf("boo5: %s     %d/%d\n", loo->string, boo.length, boo.size);

  XSB_StrEnsureSize(loo, 1000);

  XSB_StrAppendBlk(loo,NULL,5);
  XSB_StrAppend(loo,NULL);

  XSB_StrDestroy(&foo);
  XSB_StrDestroy(loo);
  goo.op->destroy(&goo);

}

#endif
