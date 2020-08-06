/* File:      parse.c -- Prolog interface to front-end parser
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
** $Id: xlparse.c,v 1.3 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include <stdio.h>
#include <string.h>

/*
#include <cinterf.h>
*/
#define TRUE  1
#define FALSE 0
extern char* ptoc_string(int);

extern FILE* yyin;
extern FILE* yyout;

extern int yyparse();
extern int line_no, char_no;
extern int num_errs;

char* fn_source;
char* fn_target;
char* fn_error;

FILE* error_file;

int parse()
{
    fn_source = (char*)ptoc_string(1);
    fn_target = (char*)ptoc_string(2);
    fn_error  = (char*)ptoc_string(3);

    if (strcmp(fn_error, "stderr") == 0)
	error_file = stderr;
    else
	error_file = fopen(fn_error, "w");

    if ((yyin = fopen(fn_source, "r")) == NULL) {
	fprintf(error_file,
		"Can not open source file %s.\n",
		fn_source);
	return FALSE;
    }

    if ((yyout = fopen(fn_target, "w")) == NULL) {
	fprintf(error_file, 
		"Can not open target file %s.\n",
		fn_target);
	return FALSE;
    }

    line_no = 1; char_no = 1;
    num_errs = 0;

    yyparse();

    fclose(yyin);
    fclose(yyout);

    if (num_errs > 0) {
	fprintf(error_file,
		"%s contains syntax %d errors.\n", 
		fn_source, num_errs);
    }

    if (error_file != stderr)
	fclose(error_file);
    return (num_errs == 0) ? TRUE : FALSE ;
}
