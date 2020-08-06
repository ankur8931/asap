/* File:      interface.h - the header file for the XSB to Perl interface
** Author(s): Jin Yu
** Contact:   xsb-contact@cs.sunysb.edu
** 
** Copyright (C) The Research Foundation of SUNY, 1998
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
** $Id: interface.h,v 1.5 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include <EXTERN.h>               /*perlembed c function's header file*/ 
#include <perl.h> 
#include "cinterf.h"	    	  /*for c and XSB interface functions*/

/*----------------------------------------------------------------------------
definitions for the XSB perl interface
----------------------------------------------------------------------------*/

#define MATCH -1
#define PREMATCH -2               
#define POSTMATCH -3
#define LAST_PAREN_MATCH -4

/*---------------------------------------------------------------------------
the following can be modified
---------------------------------------------------------------------------*/

#define MAX_SUB_MATCH 20   /*how many perl variables $1, $2.. will be loaded */
#define MAX_TOTAL_MATCH (MAX_SUB_MATCH+4) 
                           /*the total sub match number + pre,post,match... */
#define FIXEDSUBMATCHSPEC "($&,$`,$',$+"  
                           /* the fixed part of sub match specs*/

#define SUCCESS 1          /* return code when success */  
#define FAILURE 0          /* return code when failed */
#define LOADED 1           /* the flag when Perl interpretor is loaded */
#define UNLOADED 0         /* the flag when Perl interpretor is not loaded */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*----------------------------------------------------------------------------
 *global variables
 *--------------------------------------------------------------------------*/

char*  matchResults[MAX_TOTAL_MATCH]; /*$1-$20,$`,$'...*/
char** bulkMatchList;     /*the pointer to the bulk match results*/ 
int    preBulkMatchNumber;/*the number of the old bulk match results*/
char*  matchPattern;      /*match pattern string*/
int    perlObjectStatus;  /*if UNLOADED, must load perl interpreter*/  
char*  substituteString;  /*the substituted string result*/
char*  subMatchSpec;      /*the string of submatch arguments ($&,$`...$digit)*/

/*----------------------------------------------------------------------------
function declarations
----------------------------------------------------------------------------*/

static PerlInterpreter *my_perl;
extern int matchAgain( void );   /*perl interface for next match*/
extern int loadPerl(void);       /*load perl interpreter*/  
extern int unloadPerl( void );   /*unload perl interpreter*/
extern SV* my_perl_eval_sv(SV *sv, I32 croak_on_error); /*call perl compiler*/
extern int match(SV *string, char *pattern); /*perl match function*/
extern int substitute(SV **string, char *pattern);/*perl substitute function*/
extern int all_matches(SV *string, char *pattern, AV **match_list);
                                 /*perl global match function: all matches*/
extern void buildSubMatchSpec(void); 
                                 /*build the submatch arguments list string*/
                           

