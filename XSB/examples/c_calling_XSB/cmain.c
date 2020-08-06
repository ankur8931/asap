/* File:      cmain.c
** Author(s): David S. Warren
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
** $Id: cmain.c,v 1.9 2010-08-19 15:03:38 spyrosh Exp $
** 
*/

/***   Simple example file showing how to call XSB from C   ***/

/*
 * This file contains the C "main()" function.  To create an executable,
 * link this with the XSB code:
 * 1. compile cmain to create cmain.o
 * 2. build the regular xsb system
 *      (really just interested in the object files)
 * 3. link together cmain.o, from step (1) above, with XSB's object
 *    files, from step (2) above, EXCEPT for the file xmain.o--this is
 *    used to create a "standard" XSB executable--into a new executable,
 *    say, cmain.  Remember to include any necessary linking options.
 *    For example, here's how one would create an executable for a
 *    Sun Microsystems machine running Solaris:
 *      gcc -o cmain cmain.o <XSB object files> -lm -lnsl -ldl -lsocket
 *
 * A good idea would be to look at the make file in this directory.
 * Note: the XSB executable must be in the directory
 *    	      <xsb install dir>/config/<your architecture>/bin/
 *
 */

/* on Windows, be sure to set the cinterf.h include file path
correctly, and also myargv[0].  When compiling this file, be sure to
include the XSB_DLL or XSB_DLL_C flag as was included when the xsb.dll
was compiled.  */

#include <stdio.h>

/* The following include is necessary to get the macros and routine
   headers */

#include "cinterf.h"
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

int main(int argc, char *argv[])
{ 
  int rcode;
  int myargc = 3;
  char *myargv[3];

  /* xsb_init relies on the calling program to pass the absolute or relative
     path name of the XSB installation directory. We assume that the current
     program is sitting in the directory .../examples/c_calling_xsb/
     To get installation directory, we strip 3 file names from the path. */
  myargv[0] = strip_names_from_path(xsb_executable_full_path(argv[0]), 3);
  myargv[1] = "-n";
  myargv[2] = "-e writeln(hello). writeln(kkk).";

  /* Initialize xsb */
  xsb_init(myargc,myargv);  /* depend on user to put in right options (-n) */

  /* Create command to consult a file: ctest.P, and send it. */
  c2p_functor("consult",1,reg_term(1));
  c2p_string("ctest",p2p_arg(reg_term(1),1));
  if (xsb_command()) {
    printf("Error consulting ctest.P.\n");
    fflush(stdout);
  }

  if (xsb_command_string("consult(basics).")) {
    printf("Error (string) consulting basics.\n");
    fflush(stdout);
  }

  /* Create the query p(300,X,Y) and send it. */
  c2p_functor("p",3,reg_term(1));
  c2p_int(300,p2p_arg(reg_term(1),1));

  rcode = xsb_query();

  /* Print out answer and retrieve next one. */
  while (!rcode) {
    if (!(is_string(p2p_arg(reg_term(2),1)) & 
	  is_string(p2p_arg(reg_term(2),2))))
       printf("2nd and 3rd subfields must be atoms\n");
    else
      printf("Answer: %d, %s(%s), %s(%s)\n",
	     (int)p2c_int(p2p_arg(reg_term(1),1)),
	     p2c_string(p2p_arg(reg_term(1),2)),
	     xsb_var_string(1),
	     p2c_string(p2p_arg(reg_term(1),3)),
	     xsb_var_string(2)
	     );
    fflush(stdout);
    rcode = xsb_next();
  }

  /* Create the string query p(300,X,Y) and send it, use higher-level
     routines. */

  xsb_make_vars(3);
  xsb_set_var_int(300,1);
  rcode = xsb_query_string("p(X,Y,Z).");

  /* Print out answer and retrieve next one. */
  while (!rcode) {
    if (!(is_string(p2p_arg(reg_term(2),2)) & 
	  is_string(p2p_arg(reg_term(2),3))))
       printf("2nd and 3rd subfields must be atoms\n");
    else
      printf("Answer: %d, %s, %s\n",
	     (int)xsb_var_int(1),
	     xsb_var_string(2),
	     xsb_var_string(3)
	     );
    fflush(stdout);
    rcode = xsb_next();
  }



  /* Close connection */
  xsb_close();
  printf("cmain exit\n");
  return(0);
}
