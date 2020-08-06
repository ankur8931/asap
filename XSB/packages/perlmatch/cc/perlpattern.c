/* File:      perlpattern.c -- interface to Perl's match() and substitute()
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
** $Id: perlpattern.c,v 1.7 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


 
/*----------------------------------------------------------------------------
int match(SV *string, char *pattern)
Try to find the match pattern in the given string. 
If found, save the submatch strings in the global string array "matchResults".

     input: char *string: text string
            char *pattern: match pattern
     output: if match, return SUCCESS(1), otherwise return FAILURE(0).

----------------------------------------------------------------------------*/

int match(SV *string, char *pattern)
{

  SV *command = newSV(0), *retval;  /*allocate space for SV data*/
  SV *buffer = newSV(0);
  SV *string_buff = newSV(0);
  AV *matchArray;            	    /* AV storage for submatch lists */
  int number,i;
  int returnCode = FAILURE;         /*return code*/

  /*-------------------------------------------------------------------------
    allocate the space for the match pattern string, and initial it
    -----------------------------------------------------------------------*/
  if ( matchPattern!=NULL )free(matchPattern);
  matchPattern = malloc(strlen(pattern)+1);
  strcpy(matchPattern, pattern);  

  /*-------------------------------------------------------------------------
    initialize the seaching string 
    -----------------------------------------------------------------------*/
  /* sv_setpvf(command, "$__text__='%s'", SvPV(string,PL_na)); */
  sv_setpvf(command, "$__text__= <<'ENDj9yq6QC43b'; chop($__text__);\n%s\nENDj9yq6QC43b\n", SvPV(string,PL_na)); 
  my_perl_eval_sv(command, TRUE);     /* execute the perl command */
  
  /*-------------------------------------------------------------------------
    do the perl pattern matching
    ------------------------------------------------------------------------*/
  /* use an unlikely variable name __match__ to avoid name clashes*/
  sv_setpvf(command,
	    "$__text__=~%s; @__match__=%s",
	    matchPattern, subMatchSpec);
 
  retval=my_perl_eval_sv(command, TRUE);
  if (retval == NULL ) return returnCode;


  /*-------------------------------------------------------------------------
    get submatch from @__match__
    -----------------------------------------------------------------------*/
  matchArray = perl_get_av("__match__", FALSE);
  /* +1 because av_len gives the last index, and indices start with 0 */
  number = av_len(matchArray) + 1; 
  /* MAX_TOTAL_MATCH is also the size of subMatchSpec, so they must be equal */
  if ( number != MAX_TOTAL_MATCH ) {
    return returnCode;
  }

  for ( i=0;i<number;i++ ) {
    string_buff = av_shift(matchArray);
    if (matchResults[i] != NULL) free(matchResults[i]);
    matchResults[i] = (char *)malloc( strlen(SvPV(string_buff,PL_na))+1 ); 
    strcpy((char *)matchResults[i], SvPV(string_buff,PL_na) );   
  } 

  if ( strcmp(matchResults[0],"") ) returnCode=SUCCESS; 
                              /*if $& != "", match is found*/

  SvREFCNT_dec(command);      /* free space for the SV data */   
  SvREFCNT_dec(buffer);
  SvREFCNT_dec(string_buff);

  return returnCode;
}

/*----------------------------------------------------------------------------
int matchAgain( void )
Try to find the next occurrence of the match, 
where the string and pattern are same as those of last try_match__ function.

     return value: if match, return SUCCESS;
		   otherwise, FAILURE.
----------------------------------------------------------------------------*/

int match_again( void )
{

  SV *command = newSV(0), *retval; /*allocate space for SV data*/
  SV *buffer = newSV(0);
  SV *string_buff = newSV(0);
  AV *matchArray;                  /*AV storage for the submatch list*/
  int number,i;
  int returnCode=FAILURE;                /*return code*/  
    
  /*-------------------------------------------------------------------------
    do the perl pattern matching
   ------------------------------------------------------------------------*/
  sv_setpvf(command, "$__text__=~%s; @__match__=%s",
	    matchPattern,subMatchSpec);

  retval=my_perl_eval_sv(command, TRUE);
  if (retval == NULL ) return returnCode;

  /*-------------------------------------------------------------------------
    get submatches from @__match__
    -----------------------------------------------------------------------*/
  matchArray = perl_get_av("__match__", FALSE);
  /* +1 because av_len gives the last index, and indices start with 0 */
  number = av_len(matchArray) + 1;
  if ( number != MAX_TOTAL_MATCH ) {
    return returnCode;
  }

  for ( i=0;i<number;i++ ) {
    string_buff = av_shift(matchArray);
    if (matchResults[i] != NULL) free(matchResults[i]);
    matchResults[i] = (char *)malloc( strlen(SvPV(string_buff,PL_na))+1 ); 
    strcpy((char *)matchResults[i], SvPV(string_buff,PL_na) );   
  } 

  if ( strcmp(matchResults[0],"") ) returnCode=SUCCESS; 
                              /* if $&!="", match is found*/

  SvREFCNT_dec(command);      /* free space for the SV data */  
  SvREFCNT_dec(buffer);
  SvREFCNT_dec(string_buff);

  return returnCode;
} 

/*----------------------------------------------------------------------------
int substitute(SV **string, char *pattern)
Try to find the match pattern in the string, and substitute it by 
the expected pattern, the input string is ressigned with the substituted 
string. And return the number of the substituted substrings.

     input: SV **string: the pointer to the text string, it is ressigned when
                         returning.
            char *pattern: the pattern string.
     output: SV **string: the pointer to the text string, it is ressigned when
                          returning.  
     return value: number of the substituted substrings.
----------------------------------------------------------------------------*/

int substitute(SV **string, char *pattern)
{
  SV *command = newSV(0), *retval; /*allocate space for SV data */
  
  /*  sv_setpvf(command, "$__string__ = '%s'; ($__string__ =~ %s)",*/
  sv_setpvf(command, "$__string__ = <<'ENDj9yq6QC43b'; chop($__string__);\n%s\nENDj9yq6QC43b\n ($__string__ =~ %s)",
	    SvPV(*string,PL_na), pattern);

  retval = my_perl_eval_sv(command, TRUE);
  if (retval == 0 ) return FAILURE;

  SvREFCNT_dec(command);           /* release the space */

  *string = perl_get_sv("__string__", FALSE);
  return SvIV(retval);
}

/*----------------------------------------------------------------------------
int all_matches(SV *string, char *pattern, AV **match_list)
Try to find the global pattern match in the input string.
Store all matches in an array, then put the contents of the array 
into the AV storage match_list.

     input: char *string: the text string
            char *pattern: the match pattern
     output: AV **match_list: pointer to the AV storage for all matches
     return value: the number of the matches.
   
----------------------------------------------------------------------------*/

int all_matches(SV *string, char *pattern, AV **match_list)
{
  SV *command = newSV(0), *retval; /*allocate space for SV data */
  I32 num_matches;

  /*  sv_setpvf(command, "my $string = '%s'; @__array__ = ($string =~ %s)",*/
  sv_setpvf(command, "my $string = <<'ENDj9yq6QC43b'; chop($string);\n%s\nENDj9yq6QC43b\n @__array__ = ($string =~ %s)",
	    SvPV(string,PL_na), pattern);

  retval=my_perl_eval_sv(command, TRUE);
  if (retval == 0 ) return FAILURE;

  SvREFCNT_dec(command);  /*release the space */

  *match_list = perl_get_av("__array__", FALSE);
  num_matches = av_len(*match_list) + 1;
       /* +1 because av_len gives the last index, and indices start with 0 */
  
  return num_matches;

}

/*----------------------------------------------------------------------------
my_perl_eval_sv()
the function to interpret the input perl command and execute it
      input: SV* sv: the SV data containing the perl command string
             I32 croak_on_error: error handler flag
      return value: perl command executing results.
---------------------------------------------------------------------------*/ 

SV* my_perl_eval_sv(SV *sv, I32 croak_on_error)
{
    dSP;
    SV* retval;

    PUSHMARK(sp);      /*push in*/
    perl_eval_sv(sv, G_SCALAR); /*interpret the perl command*/

    SPAGAIN;
    retval = POPs;
    PUTBACK;           /*pop out*/

    if (croak_on_error && SvTRUE(GvSV(PL_errgv))) {
      printf("Warning: syntax error in the pattern expression!\n");
      /*
	printf("The entire Perl expression was: \n\t %s\n", SvPV(sv));
       */
      return FAILURE;
    }

    return retval;
}
