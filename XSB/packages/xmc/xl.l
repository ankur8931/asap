%{
#include <stdio.h>
#include <string.h>
#include "attr.h"
#include "xl.tab.h"

int line_no = 1;
int char_no = 0;
extern FILE* error_file;
extern int num_errs;
%}

%option noyywrap

digit	[0-9]
lcase	[a-z]
ucase	[A-Z]
alnum	[a-zA-Z0-9_] 
ucaseus	[A-Z_]
oprtr	[+\-*/?!@#\\\^&=<>:]

%%
%{
#define set_position {	\
			yylval.attr.l1_no = line_no; \
			yylval.attr.c1_no = char_no; \
			char_no += yyleng; \
			yylval.attr.l2_no = line_no; \
			yylval.attr.c2_no = char_no-1; \
		     }

int input_count()
{
    int c = input();
    if (c == '\n') {
	line_no++; char_no = 1;
    } else {
	char_no++;
    }
    return c;
}

typedef struct mapping {
    char* keyword;
    int   token;
} mapping;

static mapping keywords[] = {
    { "o",	PREF },
    { "is",	IS },
    { "channel",CHANNEL },
    { "predicate", PREDICATE },
    { "from",	FROM },
    { "mod",	MOD },
    { "if",	IF },
    { "then",	THEN },
    { "else",	ELSE },
    { "tt",	TRUE },
    { "ff",	FALSE },
    { NULL,	0 }
};

static mapping operators[] = {
    { ":=",	ASSIGN },
    { "::=",	DEFINE },
    { ":-",	COL_HY },
    { "+=",	PLUS_EQ },
    { "-=",	MINUS_EQ },
    { "==",	EQ_EQ },
    { "\\=",	NOT_EQ },
    { "\\==",	NOT_EQ_EQ },
    { ">=",	GREATER_EQ },
    { "<=",	LESSER_EQ },
    { "=<",	LESSER_EQ },
    { "//",	DIV },
    { "/\\",	AND },
    { "\\/",	OR },
    { "<->",	DIAM_ALL },
    { "<-",	DIAM_MINUS }
};

int lookup(mapping* map, char* text)
{
    for ( ; map->keyword != NULL; map++ )
	if (strcmp(map->keyword, text) == 0)
	    return map->token;
    return 0;
}
%}

[ \t]			{ char_no++; }
\n			{ line_no++; char_no = 1; }

"%".*\n			{ /* eol comment */
			    line_no++; char_no = 1; 
			}

"{*"			{ /* inline code */
			    int start_line = line_no;
			    int c;

			    char_no += 2;
			    for (;;) {
				if ((c = input_count()) == '*') {
				    /* possible end */
				    if ((c = input_count()) == '}') {
					/* end of inline */
					/* start new line to make it safe */
					fputc('\n', yyout);
					break;
				    } else 
					fputc('*', yyout);
				}

				if ( c == EOF ) {
				    fprintf(error_file, 
					    "unterminated inline starting at line %d\n",
					    start_line);
				    num_errs++;
				    yyterminate();
				    break;
				} else
				    /* copy the inline text to output */
				    fputc(c, yyout);
			    }
			}

"/*"			{ /* comment */
			    int start_line = line_no;
			    int c;

			    for (;;) {
				while ( (c = input_count()) != '*' 
					&& c != EOF )
				    /* eat up text of comment */
				    ;
				
				if ( c == '*' ) {
				    while ( (c = input_count()) == '*' ) 
					;
				    if ( c == '/' )
					break;    /* found the end */
				}
				
				if ( c == EOF ) {
				    fprintf(error_file,
					    "unterminated comment starting at line %d\n",
					    start_line);
				    num_errs++;
				    yyterminate();
				    break;
				}
			    }
			}

{ucaseus}{alnum}*	{ /* variable */
			    set_position;
			    yylval.attr.val.lexeme = strdup(yytext);
			    return VAR;
			}

{lcase}{alnum}*		{ /* identifier */
			    int token;
			    set_position;
			    if ((token = lookup(keywords, yytext)) != 0)
				return token;
			    yylval.attr.val.lexeme = strdup(yytext);
			    return ID;
			}

\'[^\']*\'		{ /* quoted identifier */
			    set_position;
			    yylval.attr.val.lexeme = strdup(yytext);
			    return ID;
			}

{oprtr}+		{ /* operators */
			    int token;
			    set_position;
			    if (yytext[1] == '\0')
				return yytext[0];
			    if ((token = lookup(operators, yytext)) != 0)
				return token;
			    yylval.attr.val.lexeme = strdup(yytext);
			    return OP;
			}

{digit}+		{ /* number */
			    yylval.attr.val.lexeme = strdup(yytext);
			    set_position;
			    return INT_CONST;
			}

.			{ /* other characters */
			    set_position;
			    return yytext[0];
			}

%%
