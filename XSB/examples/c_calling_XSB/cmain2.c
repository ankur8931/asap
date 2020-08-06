/* File:      cmain2.c
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
** $Id: cmain2.c,v 1.8 2010-08-19 15:03:38 spyrosh Exp $
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

#include <stdio.h>


/* The following include is necessary to get the macros and routine
   headers */

#include "cinterf.h"
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

int getline(s, lim)
char s[];
int lim;
{
  int c=0, i;
  for (i=0; i<lim-1 && (c=getc(stdin))>=0 && c!='\n'; ++i)
    s[i] = c;
  if (c == '\n') {
    s[i] = c;
    ++i;
  }
  s[i] = '\0';
  return(i);
}

void printpterm(prolog_term term)
{
  int i;

  if (is_var(term)) fprintf(stdout,"_%p", (void *)term);
  else if (is_int(term)) fprintf(stdout,"%d",p2c_int(term));
  else if (is_float(term)) fprintf(stdout,"%f",p2c_float(term));
  else if (is_nil(term)) fprintf(stdout,"[]");
  else if (is_string(term)) fprintf(stdout,"%s",p2c_string(term));
  else if (is_list(term)) {
    fprintf(stdout,"[");
    printpterm(p2p_car(term));
    term = p2p_cdr(term);
    while (is_list(term)) {
      fprintf(stdout,","),
      printpterm(p2p_car(term));
      term = p2p_cdr(term);
    }
    if (!is_nil(term)) {
      fprintf(stdout,"|");
      printpterm(term);
    }
    fprintf(stdout,"]");
  } else if (is_functor(term)) {
    fprintf(stdout,"%s",p2c_functor(term));
    if (p2c_arity(term) > 0) {
      fprintf(stdout,"(");
      printpterm(p2p_arg(term,1));
      for (i = 2; i <= p2c_arity(term); i++) {
	fprintf(stdout,",");
	printpterm(p2p_arg(term,i));
      }
      fprintf(stdout,")");
    }
  } else fprintf(stdout,"error, unrecognized type");
}

int main(int argc, char *argv[])
{ 
  int rcode;
  int myargc = 2;
  char *myargv[2];
  char query[256];

  /* xsb_init relies on the calling program to pass the absolute or relative
     path name of the XSB installation directory. We assume that the current
     program is sitting in the directory
     .../examples/c_calling_xsb/
     To get installation directory, we strip 3 file names from the path. */
  myargv[0] = strip_names_from_path(xsb_executable_full_path(argv[0]), 3);
  myargv[1] = "-n";

  /* Initialize xsb */
  xsb_init(myargc,myargv);  /* depend on user to put in right options (-n) */

  /* Create command to consult a file: ctest.P, and send it. */
  c2p_functor("consult",1,reg_term(1));
  c2p_string("ctest",p2p_arg(reg_term(1),1));
  if (xsb_command()) printf("Error consulting ctest.P.\n");

  if (xsb_command_string("consult(basics).")) 
    printf("Error (string) consulting basics.\n");

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
	     p2c_int(p2p_arg(reg_term(1),1)),
	     p2c_string(p2p_arg(reg_term(1),2)),
	     xsb_var_string(1),
	     p2c_string(p2p_arg(reg_term(1),3)),
	     xsb_var_string(2)
	     );
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
	     xsb_var_int(1),
	     xsb_var_string(2),
	     xsb_var_string(3)
	     );
    rcode = xsb_next();
  }

  /* read query from user, call, and print answers */
  fprintf(stdout,"Enter Prolog queries (^d for eof to exit)\n");
  fprintf(stdout,"| ?- ");
  getline(query,255);
  while (query[0] != '\0') {
    rcode = xsb_query_string(query);
    while (!rcode) {
      printpterm(reg_term(2));
      fprintf(stdout,"\n");
      rcode = xsb_next();
    }
    fprintf(stdout,"| ?- ");
    getline(query,255);
  }


  /* Close connection */
  xsb_close();
  printf("cmain exit\n");
  return(0);
}
