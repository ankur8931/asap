/* File:      xsb_wildmatch.c
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
** $Id: xsb_wildmatch.c,v 1.20 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include "xsb_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fnmatch.h>
#include <glob.h>

/* context.h is necessary for the type of a thread context. */
#include "context.h"

#include "auxlry.h"
#include "cell_xsb.h"
#include "error_xsb.h"
#include "cinterf.h"

/* Casefolding seems to be defined in some versions of gcc, but not in
   others. So, it is really not that portable.
*/
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0
#define CASEFOLD_UNSUPPORTED 1
#else
#define CASEFOLD_UNSUPPORTED 0
#endif

extern char *p_charlist_to_c_string(CTXTdeclc prolog_term term, VarString *outstring,
				    char *in_func, char *where);
extern void c_string_to_p_charlist(CTXTdeclc char *name, prolog_term list,
			    int regs_to_protect, char *in_func, char *where);
static prolog_term wild_term, input_string_term;

static char *lowercase_string(char *str);

static XSB_StrDefine(wild_buffer);
static XSB_StrDefine(input_string_buffer);

#ifdef MULTI_THREAD
	static th_context *th = NULL;
#endif


/* XSB wildcard matcher entry point 
** Arg1: wildcard, Arg2: string to be matched, Arg3: IgnoreCase flag */
int do_wildmatch__(void)
{
  int ignorecase=FALSE;
  int flags = 0; /* passed to wildcard matcher */
  char *wild_ptr=NULL, *input_string=NULL;
  int ret_code;
  
#ifdef MULTI_THREAD
  if( NULL == th)
	th = xsb_get_main_thread();
#endif

  wild_term = reg_term(CTXTc 1); /* Arg1: wildcard */
  input_string_term = reg_term(CTXTc 2); /* Arg2: string to find matches in */
  /* If arg 3 is bound to anything, then consider this as ignore case flag */
  if (! is_var(reg_term(CTXTc 3)))
    ignorecase = TRUE;

  flags = (ignorecase ? FNM_CASEFOLD : 0);

  /* check wildcard expression */
  if (is_string(wild_term))
    wild_ptr = string_val(wild_term);
  else if (is_list(wild_term))
    wild_ptr = p_charlist_to_c_string(CTXTc wild_term, &wild_buffer,"WILDMATCH", "wildcard");
  else
    xsb_abort("[WILDMATCH] Wildcard (Arg 1) must be an atom or a character list");

  /* check string to be matched */
  if (is_string(input_string_term))
    input_string = string_val(input_string_term);
  else if (is_list(input_string_term)) {
    input_string = p_charlist_to_c_string(CTXTc input_string_term,
					  &input_string_buffer,
					  "WILDMATCH", "input string");
  } else
    xsb_abort("[WILDMATCH] Input string (Arg 2) must be an atom or a character list");

  /* if the FNM_CASEFOLD flag is not supported,
     convert everything to lowercase before matching */
  if (CASEFOLD_UNSUPPORTED && ignorecase) {
    wild_ptr = lowercase_string(wild_ptr);
    input_string = lowercase_string(input_string);
  }

  ret_code = fnmatch(wild_ptr, input_string, flags);

  /* if we used lowercase_string, we must free up space to avoid memory leak */
  if (CASEFOLD_UNSUPPORTED && ignorecase) {
    free(input_string);
    free(wild_ptr);
  }
  if (ret_code == 0)
    return TRUE;
  return FALSE;
}

#ifndef GLOB_ABORTED
#define	GLOB_ABORTED	(-2)	/* Unignored error. */
#endif
#ifndef GLOB_NOMATCH
#define	GLOB_NOMATCH	(-3)	/* No match and GLOB_NOCHECK not set. */
#endif
#ifndef GLOB_NOSYS
#define	GLOB_NOSYS	(-4)	/* Function not supported. */
#endif

#define GLOB_ABEND	GLOB_ABORTED


/* XSB glob matcher: match files in current directory according to a wildcard.
** Arg1: wildcard, Arg2: Mark directories with `/' flag, Arg3: variable that
** gets the list of matched files.
** Arg4 tells if conversion into a charlist is required. */
int do_glob_directory__(void)
{
  glob_t file_vector;
  prolog_term listOfMatches, listHead, listTail;
  int markdirs=FALSE; /* flag: whether to append '/' to directories */
  int flags = 0;      /* passed to glob matcher */
  char *wild_ptr=NULL;
  int conversion_required, return_code, i;

#ifdef MULTI_THREAD
  if( NULL == th)
	th = xsb_get_main_thread();
#endif

  wild_term = reg_term(CTXTc 1); /* Arg1: wildcard */
  /* If arg 3 is bound to anything, then consider this as ignore case flag */
  if (! is_var(reg_term(CTXTc 2)))
    markdirs = TRUE;

  flags = (markdirs ? GLOB_MARK : 0);

  conversion_required = ptoc_int(CTXTc 4);

  /* check wildcard expression */
  if (is_string(wild_term))
    wild_ptr = string_val(wild_term);
  else if (is_list(wild_term)) {
    wild_ptr = p_charlist_to_c_string(CTXTc wild_term, &wild_buffer,
				      "GLOB_DIRECTORY", "wildcard");
  }
  else
    xsb_abort("[GLOB_DIRECTORY] Wildcard (Arg 1) must be an atom or a character list");

  file_vector.gl_offs = 0; /* put results right in the first element of
			     file_vector */
  return_code = glob(wild_ptr, flags, NULL, &file_vector);

#if defined(__APPLE__)

  if (0 != return_code)
  	{
    globfree(&file_vector);
    xsb_abort("[GLOB_DIRECTORY] Can't read directory or out of memory");
  	}
  else if (0 == file_vector.gl_matchc)	// Case GLOB_NOMATCH:
  	{
    globfree(&file_vector); /* glob allocates a long string, which must be freed to avoid memory leak */
    return FALSE;
  	}

#else

  switch (return_code) {
  case GLOB_NOMATCH:
    globfree(&file_vector); /* glob allocates a long string, which must be
			       freed to avoid memory leak */
    return FALSE;
  case 0: break;
  default:
    globfree(&file_vector);
    xsb_abort("[GLOB_DIRECTORY] Can't read directory or out of memory");
  }

#endif

  /* matched successfully: now retrieve results */
  listTail = listOfMatches = reg_term(CTXTc 3);
  if (! is_var(listTail))
    xsb_abort("[GLOB_DIRECTORY] Argument 7 (list of matches) must be an unbound variable");

  for (i=0; i<file_vector.gl_pathc; i++) {
    c2p_list(CTXTc listTail); /* make it into a list */
    listHead = p2p_car(listTail); /* get head of the list */

    if (conversion_required)
      c_string_to_p_charlist(CTXTc file_vector.gl_pathv[i], listHead, 4,
			     "GLOB_DIRECTORY", "arg 3");
    else
      c2p_string(CTXTc file_vector.gl_pathv[i], listHead);

    listTail = p2p_cdr(listTail);
  }

  c2p_nil(CTXTc listTail); /* bind tail to nil */
  globfree(&file_vector);
  return TRUE;
}

/* gets string, converts to lowercase, returns result; allocates space 
   so don't forget to clean up */
static char *lowercase_string(char *str)
{
  int i, len=strlen(str)+1;
  char *newstr = (char *) malloc(len);

  for (i=0; i<len; i++)
    *(newstr+i) = tolower(*(str+i));
  return newstr;
}

/* gets string, converts to uppercase, returns result; allocates space 
   so don't forget to clean up */
static char *uppercase_string(char *str)
{
  int i, len=strlen(str)+1;
  char *newstr = (char *) malloc(len);

  for (i=0; i<len; i++)
    *(newstr+i) = toupper(*(str+i));
  return newstr;
}


int do_convert_string__(void)
{
  char *output_ptr=NULL, *input_string=NULL, *conversion_flag=NULL;
  prolog_term conversion_flag_term, output_term;
  int to_string_conversion_required=FALSE;

#ifdef MULTI_THREAD
  if( NULL == th)
	th = xsb_get_main_thread();
#endif

  input_string_term = reg_term(CTXTc 1); /* Arg1: string to convert */

  output_term = reg_term(CTXTc 2);
  if (! is_var(output_term))
    xsb_abort("[CONVERT_STRING] Output string (Arg 2) must be a variable");

  /* If arg 3 is bound to anything, then consider this as ignore case flag */
  conversion_flag_term = reg_term(CTXTc 3);
  if (! is_string(conversion_flag_term))
    xsb_abort("[CONVERT_STRING] Conversion flag (Arg 3) must be an atom");

  conversion_flag = string_val(conversion_flag_term);

  /* check string to be converted */
  if (is_string(input_string_term))
    input_string = string_val(input_string_term);
  else if (is_list(input_string_term)) {
    input_string = p_charlist_to_c_string(CTXTc input_string_term,
					  &input_string_buffer,
					  "STRING_CONVERT", "input string");
    to_string_conversion_required = TRUE;
  } else
    xsb_abort("[CONVERT_STRING] Input string (Arg 1) must be an atom or a character list");

  if (0==strcmp(conversion_flag,"tolower"))
    output_ptr = lowercase_string(input_string);
  else if (0==strcmp(conversion_flag,"toupper"))
    output_ptr = uppercase_string(input_string);
  else
    xsb_abort("[CONVERT_STRING] Valid conversion flags (Arg 3): `tolower', `toupper'");

  if (to_string_conversion_required)
    c_string_to_p_charlist(CTXTc output_ptr,output_term, 3, "CONVERT_STRING","Arg 2");
  else
    c2p_string(CTXTc output_ptr, output_term);

  /* free up space to avoid memory leak */
  free(output_ptr);

  return TRUE;
}

