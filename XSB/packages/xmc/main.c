/* File:      main.c -- the main function for stand-alone parser
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
** $Id: main.c,v 1.4 2010-08-19 15:03:39 spyrosh Exp $
** 
*/


#include <stdio.h>

extern FILE* yyin;
extern FILE* yyout;
extern int yyparse();
extern int num_errs;

char* fn_source;
char* fn_target;
FILE* error_file;

int main(int argc, char* argv[])
{
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
	exit(1);
    }

    if (strcmp(argv[1], "-") == 0) {
	yyin = stdin;
	fn_source = "";
    } else if ((yyin = fopen(argv[1], "r")) == NULL) {
	fprintf(stderr, "Error in opening file %s!\n", argv[1]);
	exit(1);
    } else {
	fn_source = argv[1];
    }

    yyout = stdout;
    error_file = stderr;

    if (yyparse()) {
	printf("%s contains one or more syntax errors.\n", argv[1]);
    }

    return 0;
}
