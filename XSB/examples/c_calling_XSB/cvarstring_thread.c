/* File:      cvarstring.c
** Author(s): Swift
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
** FOR
 A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XSB; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: cvarstring_thread.c,v 1.4 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

/*   Simple example file showing how to call XSB from C without varstrings  
 *   To make this file, see the instructions in ./README */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* cinterf.h is necessary for the XSB API, as well as the path manipulation routines*/
#include "cinterf.h"
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

/* context.h is necessary for the type of a thread context. */
#include "context.h"
#include "thread_xsb.h"

int main(int argc, char *argv[])
{ 

#ifdef MULTI_THREAD
  static th_context *p_th, *r_th;
#endif

  char init_string[MAXPATHLEN];
  int rcp, rcr;
  XSB_StrDefine(p_return_string);
  XSB_StrDefine(r_return_string);

  /* xsb_init_string relies on the calling program to pass the absolute or relative
     path name of the XSB installation directory. We assume that the current
     program is sitting in the directory ../examples/c_calling_xsb/
     To get installation directory, we strip 3 file names from the path. */
 
  strcpy(init_string,strip_names_from_path(xsb_executable_full_path(argv[0]),3));

  if (xsb_init_string(init_string)) {
    fprintf(stderr,"%s initializing XSB: %s\n",xsb_get_error_type(xsb_get_main_thread()),
	    xsb_get_error_message(xsb_get_main_thread()));
    exit(XSB_ERROR);
  }

  p_th = xsb_get_main_thread();

  /* Create command to consult a file: edb.P, and send it. */
  if (xsb_command_string(p_th, "consult('edb.P').") == XSB_ERROR)
    fprintf(stderr,"++Error consulting edb.P: %s/%s\n",xsb_get_error_type(p_th),
	    xsb_get_error_message(p_th));

  xsb_ccall_thread_create(p_th,&r_th);

 if (xsb_command_string(r_th,"import thread_exit/0 from thread.") == XSB_ERROR)
    fprintf(stderr,"++Error exiting r: %s/%s\n",xsb_get_error_type(r_th),
	    xsb_get_error_message(r_th));

  rcp = xsb_query_string_string(p_th,"p(X,Y,Z).",&p_return_string,"|");
  rcr = xsb_query_string_string(r_th,"r(X,Y,Z).",&r_return_string,"|");

  while (rcp == XSB_SUCCESS && rcr == XSB_SUCCESS) {

    printf("Return p %s\n",(p_return_string.string));
    rcp = xsb_next_string(p_th, &p_return_string,"|");

    printf("Return r %s\n",(r_return_string.string));
    rcr = xsb_next_string(r_th, &r_return_string,"|");
  }

 if (rcp == XSB_ERROR) 
   fprintf(stderr,"++Query Error p: %s/%s\n",xsb_get_error_type(p_th),xsb_get_error_message(p_th));
 if (rcr == XSB_ERROR) 
   fprintf(stderr,"++Query Error r: %s/%s\n",xsb_get_error_type(r_th),xsb_get_error_message(r_th));

 // xsb_kill_thread(r_th);

 xsb_close(xsb_get_main_thread());      /* Close connection */
  return(0);
}
