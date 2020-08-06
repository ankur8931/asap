/*
** File: packages/pcre/cc/pcre4pl.c
** Author: Mandar Pathak
** Contact: xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 2010
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
*/

#include "deprecation.h"

#include "xsb_config.h"

#ifdef WIN_NT
#define XSB_DLL
#include <windows.h>
#include <malloc.h>
#ifndef alloca
#define alloca _malloca
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pcre.h>

#include "cinterf.h"
#include "error_xsb.h"
#include "auxlry.h"
#include "heap_xsb.h"

#ifdef MULTI_THREAD
#define xsb_get_main_thread_macro xsb_get_main_thread()
#define  check_thread_context th = xsb_get_main_thread();
#else
#define xsb_get_main_thread_macro
#define check_thread_context
#endif

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define OVECCOUNT 300 
// Stores the offsets of substring matches. Must be multiple of 3.

#define MAX_PATTERN_LENGTH 4000
#define MAX_SUBJECT_LENGTH 4000

#ifdef MULTI_THREAD
static th_context *th = NULL;
#endif


/* 
   MATCH
   Arg1: regex
   Arg2: subject
   Arg3: OUTPUT variable (must be unbound)
   Arg4: mode. must be an atom, either 'bulk' or 'one'. case insensitive.
*/

DllExport int call_conv pl_trymatch()
{
  pcre *re;
  const char *error;
  char *pattern;
  char *subject;
  char *substring_start;
  char *findall_flag;
  char *errmsg;
  int erroffset;
  int ovector[OVECCOUNT];
  int subject_length;
  int rc;
  int firstmatch;       
  int options, start_offset;
  size_t substring_length;
  int findall;  
        
        
  // Define prolog terms to handle input and output data
  prolog_term regex, ip_str, result, allmatches;
  prolog_term op_term;
  prolog_term listHead, listTail;
        
  check_thread_context
    regex = reg_term(CTXTc 1);
  ip_str = reg_term(CTXTc 2);
  result = reg_term(CTXTc 3);
  allmatches = reg_term(CTXTc 4);
        
  pattern = (char*)malloc(MAX_PATTERN_LENGTH * sizeof(char));
  subject = (char*)malloc(MAX_SUBJECT_LENGTH * sizeof(char));
  findall_flag = (char*)malloc(8*sizeof(char));
        
  // Extract reg ex from Arg 1 passed from the Prolog 
  if(is_string(regex))
    pattern = ptoc_string(CTXTc 1);
  else
    xsb_abort("[PCRE:MATCH] Arg 1 (the regular expression) must be an atom");
        
  // Extract subject string from Arg 2 passed from the Prolog 
  if(is_string(ip_str))
    subject = ptoc_string(CTXTc 2);
  else
    xsb_abort("[PCRE:MATCH] Arg 2 (the subject string) must be an atom");
        
  // Check if Arg 3 is a valid Prolog variable or not
  if(!is_var(result))
    xsb_abort("[PCRE:MATCH] Arg 3 (the output) must be an unbound variable");
        
  // See if all matches are required or not.
  if(!is_string(allmatches))
    xsb_abort("[PCRE:MATCH] Arg 4 (the matching mode) must be an atom");
  else
    findall_flag = ptoc_string(CTXTc 4);

  if(strcmp(findall_flag,"bulk")!=0)
    findall = 0;
  else
    findall = 1;        
                        
  subject_length = (int)strlen(subject);
  errmsg = (char *)malloc(sizeof(char)*128);
        
  re = pcre_compile(
                    pattern,              // the pattern 
                    0,                    // default options 
                    &error,               // for error message 
                    &erroffset,           // for error offset 
                    NULL);                // use default character tables 

  // Compilation failed: print the error message and exit 

  if (re == NULL)
    {
      sprintf(errmsg,"[PCRE:MATCH] Regular expression compilation failed at offset %d: %s\n", erroffset, error);
      xsb_abort(errmsg);
      return FALSE;
    }
        
  //    PCRE compilation succeeded.
        
  options = 0;
  start_offset = 0;
  firstmatch = 1;
        
  op_term = p2p_new(CTXT);
  listTail = op_term;
        
  while(TRUE)
    {
      char *match, *prematch, *postmatch;
      int i;
      prolog_term substrTail, substrList;
        
      rc = pcre_exec(
                     re, 
                     NULL, 
                     subject, 
                     subject_length,
                     start_offset,
                     options,
                     ovector,
                     OVECCOUNT
                     );
                        
      // PCRE_EXEC Succeeded.
                        
      if(rc==PCRE_ERROR_NOMATCH && firstmatch)
        {
          // No match found.
          pcre_free(re);
          c2p_nil(CTXTc listTail);
          return p2p_unify(CTXTc op_term, reg_term(CTXTc 3));
        }
      else if(rc==PCRE_ERROR_NOMATCH && !firstmatch)
        {
          if(options==0)
            break;
          ovector[1]=start_offset+1;
          continue;
        }
      else if(rc==0)
        {
          rc = OVECCOUNT/3;
        }
      else if(rc<0)
        {
          sprintf(errmsg,"[PCRE:MATCH] Matching error %d\n",rc);
          pcre_free(re);
          xsb_abort(errmsg);
          return FAILURE;
        }
                
      // Match succeeded 
      firstmatch = 0;
                
      match = (char *)alloca(strlen(subject));
      prematch = (char *)alloca(strlen(subject));
      postmatch = (char *)alloca(strlen(subject));
                
      substring_start = subject + ovector[0];
      SQUASH_LINUX_COMPILER_WARN(substring_start);  // to pacify the compiler
      substring_length = ovector[1] - ovector[0];
                
      strncpy(match,subject+ovector[0],substring_length);
      match[substring_length]='\0';
                
      strcpy(prematch, " ");
      strcpy(postmatch, " ");
                

      if(ovector[0]==0)
        strcpy(prematch,"");
      else {
        strncpy(prematch, subject, ovector[0]);
        prematch[ovector[0]]='\0';
      }
                
        
      if((ovector[0]+substring_length)>=strlen(subject))
        strcpy(postmatch,"");
      else {
        strncpy(postmatch,
                subject+ovector[0]+substring_length,
                strlen(subject)-ovector[0]-substring_length);
        postmatch[strlen(subject)-ovector[0]-substring_length]='\0';
      }
                
      substrList = p2p_new(CTXT);
      substrTail = substrList;
                
      for (i=1;i<rc;i++) {
        int sc;
        const char *substr;
        char *temp;
        sc = pcre_get_substring(subject, ovector, rc,i, &substr);
        temp = (char*)malloc(strlen(substr)*sizeof(char));
        strcpy(temp,substr);
        if(sc<0)
          xsb_abort("[PCRE:MATCH] Substrings could not be retrieved");
        else {                  
          c2p_list(CTXTc substrTail);
          c2p_string(CTXTc temp,p2p_car(substrTail));
          substrTail = p2p_cdr(substrTail);
                    
          pcre_free_substring(substr);
        }
      }         
      c2p_nil(CTXTc substrTail);
                        
      c2p_list(CTXTc listTail);
      listHead = p2p_car(listTail);
        
      c2p_functor(CTXTc "match",4,listHead);
      c2p_string(CTXTc match, p2p_arg(listHead,1));
      c2p_string(CTXTc prematch, p2p_arg(listHead,2));
      c2p_string(CTXTc postmatch, p2p_arg(listHead,3));
      p2p_unify(CTXTc substrList,p2p_arg(listHead,4));
                
      listTail = p2p_cdr(listTail);
                
      start_offset = ovector[0]+1;
        
      if(!findall)
        break;
    }
        
  pcre_free(re);
  c2p_nil(CTXTc listTail);
  return p2p_unify(CTXTc op_term, reg_term(CTXTc 3));
}


/* 
   SUBSTITUTE
   Arg1: pattern
   Arg2: subject string
   Arg3: substitution string
   Arg4: OUTPUT variable. Must be unbound.
       
   NOTE: Substitute finds all substrings in the SUBJECT which match the
   PATTERN and replaces them with SUBSTITUTION STRING. The result is
   returned in the unbound variable.
*/


DllExport int call_conv pl_substitute()
{
  pcre *re;
  const char *error;
  char *substring_start;
  char *errmsg;
  int erroffset;
  int ovector[OVECCOUNT];
  int subject_length;
  int rc;
  int firstmatch;       
  int options, start_offset;
  int substring_length;
  size_t copyIndex;
        
  prolog_term term_subject, term_regex, term_subst, term_op;
  char *subject, *pattern, *subst;
  char op[1024];
  char *temp;   
        
  check_thread_context
    term_subject = reg_term(CTXTc 1);
  term_regex = reg_term(CTXTc 2);
  term_subst = reg_term(CTXTc 3);
  term_op = reg_term(CTXTc 4);
        
  pattern = (char*)malloc(MAX_PATTERN_LENGTH*sizeof(char));
  subject = (char*)malloc(MAX_SUBJECT_LENGTH*sizeof(char));
  subst = (char*)malloc(MAX_SUBJECT_LENGTH*sizeof(char));
        
  if(is_string(term_regex))
    pattern = ptoc_string(CTXTc 1);
  else
    xsb_abort("[PCRE:SUBSTITUTE] Arg 1 (the pattern) must be an atom");
        
  if(is_string(term_subject))
    subject = ptoc_string(CTXTc 2);
  else
    xsb_abort("[PCRE:SUBSTITUTE] Arg 2 (the subject) must be an atom");
        
  if(is_string(term_subst))
    subst = ptoc_string(CTXTc 3);
  else
    xsb_abort("[PCRE:SUBSTITUTE] Arg 3 (the substitution) must be an atom");
        
  if(!is_var(term_op))
    xsb_abort("[PCRE:SUBSTITUTE] Arg 4 (the output) must be an unbound variable.");


  subject_length = (int)strlen(subject);
  errmsg = (char *)malloc(sizeof(char)*128);
        
  re = pcre_compile(
                    pattern,              // the pattern 
                    0,                    // default options 
                    &error,               // for error message 
                    &erroffset,           // for error offset 
                    NULL);                // use default character tables 

  // Compilation failed: print the error message and exit 

  if (re == NULL) {
    sprintf(errmsg,
            "[PCRE:SUBSTITUTE] Regular expression error at position %d: %s\n",
            erroffset, error);
    xsb_abort(errmsg);
    return FALSE;
  }
        
  // PCRE compilation succeeded
        
  options = 0;
  start_offset = 0;
  firstmatch = 1;
  copyIndex=0;

  while(strlen(subject)) {      
    char *match;
                
    rc = pcre_exec(
                   re, 
                   NULL, 
                   subject, 
                   subject_length,
                   start_offset,
                   options,
                   ovector,
                   OVECCOUNT
                   );
                                                
    if(rc==PCRE_ERROR_NOMATCH && firstmatch) {
      // No match found.
      pcre_free(re);
      strncpy(op,subject,strlen(subject));
      op[strlen(subject)]='\0';
      ctop_string(CTXTc 4, op);
      return SUCCESS;
    }
    else if(rc==PCRE_ERROR_NOMATCH && !firstmatch) {
      if(options==0)
        break;
      ovector[1]=start_offset+1;
      continue;
    }
    else if(rc==0) {
      rc = OVECCOUNT/3;
    }
    else if(rc<0) {
      sprintf(errmsg,"Matching error %d\n",rc);
      pcre_free(re);
      xsb_abort(errmsg);
      return FAILURE;
    }
                
    // Match succeeded 
    firstmatch = 0;
          
    match = (char *)alloca(strlen(subject));
          
    substring_start = subject + ovector[0];
    SQUASH_LINUX_COMPILER_WARN(substring_start);  // to pacify the compiler
    substring_length = ovector[1] - ovector[0];         
    strncpy(match,subject+ovector[0],substring_length);
    match[substring_length]='\0';
                
    strncpy(op+copyIndex,subject,ovector[0]);
    copyIndex+=ovector[0];
    strncpy(op+copyIndex,subst,strlen(subst));
    copyIndex+=strlen(subst);
          
                
    temp = (char *)alloca(strlen(subject)*sizeof(char));
    strcpy(temp,subject+ovector[0]+substring_length);
    strcpy(subject,temp);
    subject_length=(int)strlen(subject);
  }     
        
  strncpy(op+copyIndex,subject,strlen(subject));
  copyIndex+=strlen(subject);
  op[copyIndex]='\0';
  ctop_string(CTXTc 4, op);
        
  pcre_free(re);

  return SUCCESS;
}



