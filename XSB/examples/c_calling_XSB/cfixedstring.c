/* File:      cfixedstring.c
** Author(s): Terrance Swift
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
** 
*/

/*   Simple example file showing how to call XSB from C without varstrings  
 *   To make this file, see the instructions in ./README */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* cinterf.h is necessary for the XSB API, as well as the path manipulation routines*/
#include "cinterf.h"

/* context.h is necessary for the type of a thread context. */
#include "context.h"

/* The following include is necessary to get the macros and routine
   headers */
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

int main(int argc, char *argv[])
{ 

  int anslen,rc;

  int return_size = 15;
  char *return_string;
  return_string = malloc(return_size);

  int myargc = 1;
  char *myargv[1];

  myargv[0] = strip_names_from_path(xsb_executable_full_path(argv[0]),3);

  if (xsb_init(myargc,myargv)) {
    fprintf(stderr,"%s initializing XSB: %s\n",xsb_get_init_error_type(),xsb_get_init_error_message());
    exit(XSB_ERROR);
  }

#ifdef MULTI_THREAD
  th_context *th = xsb_get_main_thread();
#define xsb_get_main_thread_macro() xsb_get_main_thread()
#else
#define xsb_get_main_thread_macro() 
#endif

  /* Create command to consult a file: edb.P, and send it. */
  if (xsb_command_string(CTXTc "consult('edb.P').") == XSB_ERROR)
    fprintf(stderr,"++Error consulting edb.P: %s/%s\n",
	    xsb_get_error_type(xsb_get_main_thread_macro()),
	    xsb_get_error_message(xsb_get_main_thread_macro()));

  rc = xsb_query_string_string_b(CTXTc "p(X,Y,Z).",return_string,return_size,&anslen,"|");

  while (rc == XSB_SUCCESS || rc == XSB_OVERFLOW) {
  
    if (rc == XSB_OVERFLOW) {
      printf("reallocating (%d)\n",anslen);
      return_string = (char *) realloc(return_string,anslen);
      return_size = anslen;
      rc = xsb_get_last_answer_string(CTXTc return_string,return_size,&anslen);
    }    

    printf("Return %s %d %d\n",return_string,return_size,anslen);
    rc = xsb_next_string_b(CTXTc return_string,return_size,&anslen,"|");
  }

 if (rc == XSB_ERROR) 
   fprintf(stderr,"++Query Error: %s/%s\n",
	   xsb_get_error_type(xsb_get_main_thread_macro()),
	   xsb_get_error_message(xsb_get_main_thread_macro()));

  xsb_close(CTXT);
  return(0);
}
