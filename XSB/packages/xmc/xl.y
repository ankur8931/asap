%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "attr.h"

  /* #define YYERROR_VERBOSE*/

extern FILE* yyout;

extern int yyerror(char *);     /* defined in driver.c */
 extern int yylex();             /* defined in lex.yy.c */
static char buffer[4096];
static int arity;		/* to count the predicate's arity */

void warning(char*, char*);

void set(AttrType*, char*, AttrType*, AttrType*);
void infix(char* op, AttrType* a0, AttrType* a1, AttrType* a2);
void comp1(AttrType*, AttrType*, AttrType*, char*, AttrType*);
void comp2(AttrType*, AttrType*, AttrType*, char*, AttrType*, AttrType*);
void compproc(AttrType*, AttrType*, AttrType*, char*, AttrType*, AttrType*);
void compproc3(AttrType*, AttrType*, AttrType*, char*, AttrType*, AttrType*, AttrType*);

/* variable management */
#define TYPE_UNDEFINED	((char*)0)
#define TYPE_TYPE	((char*)1)

void add_variable(char*, char*);
void dump_variables();

%}

/* Declare ALL tokens here... */
%type <attr> ','
%type <attr> ';'
%type <attr> ':'
%type <attr> DEFINE
%type <attr> '.'
%type <attr> '['
%type <attr> ']'
%type <attr> '('
%type <attr> ')'
%type <attr> '{'
%type <attr> '}'
%type <attr> '+'
%type <attr> '-'
%type <attr> '*'
%type <attr> MOD
%type <attr> EQ_EQ
%type <attr> NOT_EQ
%type <attr> '/'
%type <attr> PREF
%type <attr> '|'
%type <attr> '#'
%type <attr> IF
%type <attr> THEN
%type <attr> ELSE
%type <attr> '?'
%type <attr> '!'
%type <attr> ID
%type <attr> VAR
%type <attr> SYNTAX_ERROR
%type <attr> '='
%type <attr> ASSIGN
%type <attr> NOT_EQ_EQ
%type <attr> PLUS_EQ
%type <attr> MINUS_EQ
%type <attr> IS
%type <attr> INT_CONST
%type <attr> COL_HY
%type <attr> PREDICATE
%type <attr> FROM
%type <attr> '>'
%type <attr> '<'
%type <attr> GREATER_EQ
%type <attr> LESSER_EQ
%type <attr> '~'
%type <attr> AND
%type <attr> OR
%type <attr> TRUE
%type <attr> FALSE
%type <attr> CHANNEL
%type <attr> DIAM_ALL	/* <-> */
%type <attr> DIAM_MINUS	/* <- */

%token ',' ':' DEFINE
       '.' '['      /* CHECK USCORE*/
       ']' '(' ')'
       '{' '}' '+' '-'
       OP
       '*' MOD EQ_EQ NOT_EQ '/' DIV
       PREF ';'
	'|' '#'
	IF THEN
       ELSE '?' '!' ID VAR SYNTAX_ERROR
       '=' ASSIGN NOT_EQ_EQ PLUS_EQ MINUS_EQ
       IS INT_CONST COL_HY
       PREDICATE FROM
       '>' '<' GREATER_EQ LESSER_EQ
       AND OR TRUE FALSE CHANNEL
       DIAM_ALL DIAM_MINUS


%union{
    AttrType attr;
}

%type <attr> prog
%type <attr> speclist
%type <attr> spec
%type <attr> preddecl
%type <attr> predlist
%type <attr> predtype
%type <attr> predtypelist
%type <attr> cdecl
%type <attr> vardecllist
%type <attr> vardecl
%type <attr> type
%type <attr> typelist
%type <attr> pdefn
%type <attr> pname
%type <attr> pexp
%type <attr> cname
%type <attr> term
%type <attr> termlist
%type <attr> exp
%type <attr> explist
%type <attr> fdefn
%type <attr> fterm
%type <attr> fexp
%type <attr> fterm_ng
%type <attr> fexp_ng
%type <attr> modality
%type <attr> posmodal
%type <attr> unit
%type <attr> unitlist

/* Define operator precedences here... */
%nonassoc  ASSIGN IS

%left      OR
%left      AND
%nonassoc  '~'
/* 
%left      '{' '}' 
*/
%nonassoc      '[' ']' DIAM_ALL DIAM_MIMUS /* for box/diam formula */
/* 
%left      '(' ')' 
*/
%right     '#'
%right     '|'
%right     PREF ';'
%nonassoc  THEN
%nonassoc  ELSE
%nonassoc  EQ_EQ NOT_EQ_EQ '=' NOT_EQ '<' '>' LESSER_EQ GREATER_EQ
%left      '+' '-'
%left      '*' '/' MOD DIV
%nonassoc  '?' '!'
%left      ',' ':'

%%

prog:	  speclist
;

speclist: spec
	| speclist spec
;

spec:	  pdefn
	| preddecl
	| cdecl
	| fdefn
	| error '.'  /* catch parse errors and continue */ {}
;

/* -------------------- Prolog predicate directive -------------------- */
preddecl: PREDICATE predlist FROM ID '.'
		{
		    /* output import directive */
		    fprintf(yyout, ":- import %s from %s.\n",
			   $2.val.str, $4.val.str);
		}
	| PREDICATE predlist '.' 
		{
		    /* no import print out */
		}
;

predlist: predtype	{ $$ = $1; }
	| predlist ',' predtype
			{ comp2(&($$), &($1),&($3), "%s, %s", &($1),&($3)); }
;

predtype: ID		{ fprintf(yyout, "predicate(%s).\n", $1.val.lexeme);
			  comp1(&($$), &($1),&($1), "%s/0", &($1)); }
	| ID '(' predtypelist ')'
			{ /* output datatype directive */
			    fprintf(yyout, "predicate(%s(%s)).\n",
				   $1.val.lexeme, $3.val.str);
			    sprintf(buffer, "%s/%d",
				    $1.val.lexeme, arity);
			    set(&($$), buffer, &($1), &($4)); }
;

predtypelist: type	{ arity = 1; $$ = $1; }
	| predtypelist ',' type 
			{ arity++;
			  comp2(&($$), &($1),&($3), "%s,%s", &($1),&($3)); }
;

/* -------------------- channel definition -------------------- */
cdecl:	  CHANNEL ID '.'
		{
		    fprintf(yyout, "cdef(%s,[%d,%d],[]).\n\n",
			   $2.val.lexeme, $2.l1_no, $2.c1_no);
		}		    
	| CHANNEL ID '(' vardecllist  ')' '.'
		{
		    fprintf(yyout, "cdef(%s(%s),[%d,%d],",
			   $2.val.lexeme, $4.val.str, $2.l1_no, $2.c1_no);
		    dump_variables();
		}
;

vardecllist: vardecl	{ set(&($$), $1.val.str, &($1), &($1)); }
	| vardecllist ',' vardecl
			{ comp2(&($$), &($1),&($3), "%s,%s", &($1),&($3)); }
;

vardecl:  VAR		{ set(&($$), $1.val.lexeme, &($1), &($1));
			  add_variable($1.val.lexeme, TYPE_UNDEFINED); }
	| VAR ':' type	{ comp2(&($$), &($1),&($3), "%s:%s", &($1),&($3));
			  add_variable($1.val.lexeme, $3.val.str); }
;

type:	  ID		{ set(&($$), $1.val.str, &($1), &($1)); }
	| VAR		{ set(&($$), $1.val.lexeme, &($1), &($1));
			  add_variable($1.val.lexeme, TYPE_TYPE);
			}
	| ID '(' typelist ')'
			{ comp2(&($$), &($1),&($4), "%s(%s)", &($1),&($3)); }
;

typelist: type		{ set(&($$), $1.val.str, &($1), &($1)); }
	| typelist ',' type 
			{ comp2(&($$), &($1),&($3), "%s,%s", &($1),&($3)); }
;

/* -------------------- process definition -------------------- */

pdefn:	pname DEFINE pexp '.'
		{
		  fprintf(yyout, "pdef([%s,[%d,%d,%d,%d]],\n\t[%s,[%d,%d,%d,%d]],\n\t",
			 $1.val.str, $1.l1_no, $1.c1_no, $1.l2_no, $1.c2_no,
			 $3.val.str, $3.l1_no, $3.c1_no, $3.l2_no, $3.c2_no);
		  dump_variables();
                }
;

pname:	ID '(' vardecllist ')' 
			{ comp2(&($$), &($1),&($4), "%s(%s)", &($1),&($3));}
	| ID		{ set(&($$), $1.val.lexeme, &($1), &($1)); }
;

pexp:	  exp		{ set(&($$), $1.val.str, &($1), &($1)); }

	| cname '?' term {comp2(&($$), &($1),&($3), "in(%s,%s)", &($1),&($3));}
	| cname '?' '*'	 {comp1(&($$), &($1),&($3), "in(%s,*)", &($1)); }
	| cname '!' term {comp2(&($$), &($1),&($3), "out(%s,%s)",&($1),&($3));}
	| cname '!' '*'	 { comp1(&($$), &($1),&($3), "out(%s,*)", &($1)); }

	| IF exp 
	  THEN pexp 
	  ELSE pexp	{ compproc3(&($$),&($1),&($6),"if",&($2),&($4),&($6));}

	| IF exp 
	  THEN pexp	{ compproc(&($$), &($1),&($4),"if",&($2),&($4)); }

	| pexp PREF pexp
			{ compproc(&($$), &($1),&($3), "pref", &($1), &($3)); }
	| pexp ';' pexp { compproc(&($$), &($1),&($3), "pref", &($1), &($3)); }
	| pexp '#' pexp { compproc(&($$), &($1),&($3), "choice",&($1),&($3)); }
	| pexp '|' pexp { compproc(&($$), &($1),&($3), "par", &($1), &($3)); }
	| '{' pexp '}'	{ set(&($$), $2.val.str, &($1), &($3)); }
;

cname:	vardecl		{ set(&($$), $1.val.str, &($1), &($1)); }
	| ID		{ set(&($$), $1.val.str, &($1), &($1)); }
;

term:	ID '(' explist ')' { comp2(&($$),&($1),&($4),"%s(%s)",&($1),&($3)); }
/*	ID '(' termlist ')' { comp2(&($$),&($1),&($4),"%s(%s)",&($1),&($3)); } */
	| ID			{ set(&($$), $1.val.str, &($1), &($1)); }
	| vardecl               { set(&($$), $1.val.str, &($1), &($1)); }
	| INT_CONST		{ set(&($$), $1.val.lexeme, &($1), &($1)); }
	| '[' ']'		{ set(&($$), "[]", &($1), &($2)); }
	| '[' termlist ']'	{ comp1(&($$), &($1),&($3), "[%s]", &($2)); }
	| '[' termlist '|' term ']' {
				  comp2(&($$), &($1),&($5), 
					"[%s | %s]", &($2), &($4)); }
	| '(' term ',' termlist ')' {
				  comp2(&($$), &($1), &($5),
					"(%s,%s)", &($2), &($4)); }
;

termlist: term			{ set(&($$), $1.val.str, &($1), &($1)); }
	| termlist ',' term	{ comp2(&($$),&($1),&($3),"%s,%s",&($1),&($3));}
;

explist: exp			{ set(&($$), $1.val.str, &($1), &($1)); }
	| explist ',' exp	{ comp2(&($$),&($1),&($3),"%s,%s",&($1),&($3));}
;

exp:	term			{ $$ = $1; }
	| '+' exp		{ set(&($$), $1.val.str, &($1), &($1)); }
	| '-' exp		{ comp1(&($$), &($1),&($2), "(- %s)", &($2)); }
	| '~' exp		{ comp1(&($$), &($1),&($2), "not(%s)", &($2)); }
	| exp '+' exp		{ infix("+",   &($$), &($1), &($3)); }
	| exp '-' exp		{ infix("-",   &($$), &($1), &($3)); }
	| exp '*' exp		{ infix("*",   &($$), &($1), &($3)); }
	| exp MOD exp		{ infix("mod", &($$), &($1), &($3)); }
	| exp DIV exp		{ infix("//", &($$), &($1), &($3)); }
	| exp '/' exp		{ infix("/",   &($$), &($1), &($3)); }
	| exp '<' exp		{ infix("<",    &($$), &($1), &($3)); }
	| exp '>' exp		{ infix(">",    &($$), &($1), &($3)); }
	| exp GREATER_EQ exp	{ infix(">=",&($$), &($1), &($3)); }
	| exp LESSER_EQ exp	{ infix("=<", &($$), &($1), &($3)); }
	| exp '=' exp		{ infix("=",    &($$), &($1), &($3)); }
	| exp EQ_EQ  exp	{ infix("==",   &($$), &($1), &($3)); }
	| exp NOT_EQ exp	{ infix("\\=",  &($$), &($1), &($3)); }
	| exp NOT_EQ_EQ exp 
				{ infix("\\==", &($$), &($1), &($3)); }
	| exp AND exp		{ infix(",",    &($$), &($1), &($3)); }
				  /* note^ changed to Prolog format */
	| exp OR exp		{ infix(";",    &($$), &($1), &($3)); }
	| exp ASSIGN exp	{ infix(":=",  &($$), &($1), &($3)); }
	| exp IS exp		{ infix("is",  &($$), &($1), &($3)); }
	| '(' exp ')'		{ set(&($$),$2.val.str, &($1), &($1)); }
;

/*-------------------- Formula definition --------------------*/
fdefn:	fterm PLUS_EQ fexp '.'
		{
		    $$.l1_no=$1.l1_no;
		    $$.c1_no=$1.c1_no;
		    $$.l2_no=$4.l1_no;
		    $$.c2_no=$4.c1_no;
		    fprintf(yyout, "fdef(%s, %s).\n",
			   $1.val.str,$3.val.str);
		    fprintf(yyout, "fneg(%s, neg_form(%s)).\n",
			   $1.val.str,$1.val.str);
		}

	| fterm MINUS_EQ fexp_ng '.'
		{
		    $$.l1_no=$1.l1_no;
		    $$.c1_no=$1.c1_no;
		    $$.l2_no=$4.l1_no;
		    $$.c2_no=$4.c1_no;
		    /* neg_xxx = neg_form(neg_xxx) */
		    fprintf(yyout, "fdef(%s, neg_form(neg_%s)).\n",
			    $1.val.str,$1.val.str);
		    /* neg_xxx = form(neg. of expr) */
		    fprintf(yyout, "fdef(neg_%s, %s).\n",
			    $1.val.str, $3.val.str);
		    fprintf(yyout, "fneg(%s, form(neg_%s)).\n",
			   $1.val.str,$1.val.str);
		}
;

fterm:	ID '(' vardecllist ')'	
			{ comp2(&($$),&($1),&($4),"%s(%s)",&($1),&($3)); }
	| ID		{ set(&($$), $1.val.str, &($1), &($1));	}
;

fexp:	  fexp AND fexp	{ comp2(&($$),&($1),&($3), "and(%s,%s)",&($1),&($3)); }
	| fexp OR fexp	{ comp2(&($$),&($1),&($3), "or(%s,%s)", &($1),&($3)); }
	| '<' modality '>' fexp
			{ comp2(&($$),&($1),&($4),"diam%s,%s)",&($2),&($4)); }
	| DIAM_ALL fexp	{ comp1(&($$),&($1),&($2),"diamAll(%s)",&($2)); }
	| DIAM_MINUS modality '>' fexp
			{ comp2(&($$),&($1),&($2),"diamMinus%s,%s)",&($2),&($4));}
	| '[' modality ']' fexp
			{ comp2(&($$),&($1),&($4),"box%s,%s)",&($2),&($4)); }

	| TRUE		{ set(&($$), "tt", &($1), &($1)); }
	| FALSE		{ set(&($$), "ff", &($1), &($1)); }
	| fterm		{ comp1(&($$),&($1),&($1),"form(%s)",&($1)); }
	| '(' fexp ')'	{ set(&($$), $2.val.str, &($1), &($3));	}
;

fterm_ng: ID '(' vardecllist ')'	
			{ comp2(&($$),&($1),&($4),"%s(%s)",&($1),&($3)); }
	| ID		{ $$ = $1; }
;

fexp_ng:  fexp_ng AND fexp_ng
			{ comp2(&($$),&($1),&($3), "or(%s,%s)",&($1),&($3)); }
	| fexp_ng OR fexp_ng
			{ comp2(&($$),&($1),&($3), "and(%s,%s)",&($1),&($3)); }
	| '<' modality '>' fexp_ng
			{ comp2(&($$),&($1),&($4),"box%s,%s)",&($2),&($4)); }
	| DIAM_ALL fexp_ng
			{ comp1(&($$),&($1),&($2),"boxAll(%s)",&($2)); }
	| DIAM_MINUS modality '>' fexp_ng
			{ comp2(&($$),&($1),&($2),"boxMinus%s,%s)",&($2),&($4));}
	| '[' modality ']' fexp_ng
			{ comp2(&($$),&($1),&($4),"diam%s,%s)",&($2),&($4)); }
	| TRUE		{ set(&($$), "ff", &($1), &($1)); }
	| FALSE		{ set(&($$), "tt", &($1), &($1)); }
	| fterm_ng	{ comp1(&($$), &($1),&($1), "neg(%s)", &($1)); }
	| '(' fexp_ng ')' { set(&($$), $2.val.str, &($1), &($3));	}
;

modality: '-' posmodal	{ comp1(&($$),&($1),&($2),"Minus%s",&($2)); }
	| '-'		{ set(&($$), "MinusSet([]", &($1), &($1)); }
	| posmodal	{ set(&($$), $1.val.str, &($1), &($1)); }
;

posmodal: unit		{ comp1(&($$),&($1),&($1),"(%s",&($1)); }
	| '{' unitlist '}'
			{ comp1(&($$),&($1),&($3),"Set([%s]",&($2)); }

unit:	ID '(' unitlist ')'
			{
			    if (strcmp($1.val.str, "in") != 0 &&
				strcmp($1.val.str, "out") != 0) {
				comp2(&($$),&($1),&($4),"action(%s(%s))",
				      &($1),&($3));
			    } else
				comp2(&($$),&($1),&($4),"%s(%s)",&($1),&($3));
			}
	| ID		{
			    if (strcmp($1.val.str, "nop") != 0 &&
				strcmp($1.val.str, "tau") != 0) {
				comp1(&($$), &($1),&($1),"action(%s)",&($1));
			    } else
				$$ = $1;
			}
	| VAR		{ $$ = $1; }
	| INT_CONST	{ set(&($$),$1.val.lexeme, &($1), &($1)); }
;

unitlist:  unit		{ $$ = $1; }
	| unitlist ',' unit
			{ comp2(&($$),&($1),&($3),"%s,%s",&($1),&($3)); }
;

%%

void set(AttrType *nt, char* bd, AttrType* a1, AttrType* a2)
{
    if (bd == buffer)
	/* copy from temporary string */
	nt->val.str = strdup(bd);
    else
	nt->val.str = bd;
    nt->l1_no = a1->l1_no;
    nt->c1_no = a1->c1_no;
    nt->l2_no = a2->l2_no;
    nt->c2_no = a2->c2_no;
}

void infix(char* op, AttrType* a0, AttrType* a1, AttrType* a2)
{
    sprintf(buffer, "(%s %s %s)",
	    a1->val.str, op, a2->val.str);
    set(a0, buffer, a1, a2);
}


void comp1(AttrType* a0, AttrType* first, AttrType* last,
		char* format, AttrType* op1)
{
    sprintf(buffer, format, op1->val.str);
    set(a0, buffer, first, last);
}

void comp2(AttrType* a0, AttrType* first, AttrType* last,
		char* format, AttrType* op1, AttrType* op2)
{
    sprintf(buffer, format, op1->val.str, op2->val.str);
    set(a0, buffer, first, last);
}

void compproc(AttrType* a0, AttrType* first, AttrType* last,
		char* op, AttrType* op1, AttrType* op2)
{
    sprintf(buffer, "%s([%s,[%d,%d,%d,%d]],[%s,[%d,%d,%d,%d]])",
	    op,
	    op1->val.str, op1->l1_no, op1->c1_no, op1->l2_no, op1->c2_no,
	    op2->val.str, op2->l1_no, op2->c1_no, op2->l2_no, op2->c2_no);
    set(a0, buffer, first, last);
}

void compproc3(AttrType* a0, AttrType* first, AttrType* last,
		char* op, AttrType* op1, AttrType* op2, AttrType* op3)
{
    sprintf(buffer, "%s([%s,[%d,%d,%d,%d]],[%s,[%d,%d,%d,%d]],[%s,[%d,%d,%d,%d]])",
	    op,
	    op1->val.str, op1->l1_no, op1->c1_no, op1->l2_no, op1->c2_no,
	    op2->val.str, op2->l1_no, op2->c1_no, op2->l2_no, op2->c2_no,
	    op3->val.str, op3->l1_no, op3->c1_no, op3->l2_no, op3->c2_no);
    set(a0, buffer, first, last);
}

/* -------------------- variable management -------------------- */

typedef struct node
{
    char* varname;
    char* type;
    struct node* next;
} node;

static node* head = NULL;

void add_variable(char* newname, char* newtype)
{
    node* t;

    /* skip unnamed variable */
    if (newname[0] == '_' && newname[1] == '\0')
	return;

    /* check whether the variable is recorded */
    for (t = head; t != NULL; t = t->next)
	if (strcmp(newname, t->varname) == 0) {
	    if (t->type == TYPE_UNDEFINED) {
		if (newtype == TYPE_TYPE) 
		    warning("Data variable used as type", newname);
		else 
		    t->type = newtype;
	    } else if (t->type == TYPE_TYPE) {
		if (newtype != TYPE_TYPE)
		    warning("Type variable %s used as data", newname);
	    } else {
		if (newtype == TYPE_TYPE)
		    warning("Data variable %s used as type", newname);
		else if (newtype != TYPE_UNDEFINED 
			 && strcmp(t->type, newtype) != 0)
		    warning("Data variable %s's type defined twice", newname);
	    }
	    return;
	}

    /* add the variable at head */
    t = (node*)malloc(sizeof(node));
    t->varname = strdup(newname);
    t->type = newtype;
    t->next = head;
    head = t;
}

void dump_variables()
{
    node* t;

    fprintf(yyout, "[");
    /* print the first data variable */
    for (t = head; t != NULL; ) {
	head = t->next;
	if (t->type != TYPE_TYPE) {
	    fprintf(yyout, "(%s,'%s')", t->varname, t->varname);
	    free(t);
	    break;
	}
	free(t);
    }

    /* print the remaining data variables */
    for (t = head; t != NULL; ) {
	if (t->type != TYPE_TYPE)
	    fprintf(yyout, ", (%s,'%s')", t->varname, t->varname);
	head = t->next;
	free(t);
	t = head;
    }
    fprintf(yyout, "]).\n\n");
}
