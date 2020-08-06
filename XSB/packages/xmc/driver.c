/* File:      driver.c		-- driver for the parser
** Author(s): Yifei Dong
** Contact:   lmc@cs.sunysb.edu
** 
** Copyright (C) SUNY at Stony Brook, 1998-2000
** 
** XMC is free software; you can redistribute it and/or modify it under the
** terms of the GNU Library General Public License as published by the Free
** Software Foundation; either version 2 of the License, or (at your option)
** any later version.
** 
** XMC is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License for
** more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with XMC; if not, write to the Free Software Foundation,
** Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** $Id: driver.c,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/

#include <stdio.h>
#include <string.h>

extern FILE *yyin;
extern FILE* error_file;
extern int yyparse();

extern int line_no, char_no;
extern char* yytext;

extern char* fn_source;
int num_errs;

void warning(char* msg, char* arg)
{
    fprintf(error_file, "%s:%d/%d: ",
	    fn_source, line_no, char_no);
    fprintf(error_file, msg, arg);
    fprintf(error_file, "\n");
    num_errs++;
}

int yyerror(char *errtype)
{
    fprintf(error_file, "%s:%d/%d: %s near token `%s'.\n",
	    fn_source, 
	    line_no, char_no, errtype, yytext);
    num_errs++;
    return 0;
}
