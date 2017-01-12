%{
#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_complex.h>

#include "node.h"
#include "memory.h"
#include "printext.h"
#include "eval.h"
#include "set.h"

#define YYDEBUG 1

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)

int yylex ();
void yyerror (char const *);
%}

%union {
  int         i;
  node_u      n;
  char       *s;
  gsl_complex v;
  sym_e	      d;
}

%token	   PLOT
%token	   SET
%token	   EOP
%token	   EOS
%token <v> NUMBER
%token <s> SYMBOL
%token <s> QSTRING
%right     ASSIGN
%left  <d> CATENATE
%right <d> RIGHT_DYADIC
%right <d> RIGHT_CLC_DYADIC
%right <d> BIF
%right <d> CLC_BIF
%right     MONADIC
%token     LEFT_PAREN
%token     RIGHT_PAREN
%token     LEFT_BRACKET
%token     RIGHT_BRACKET
%token     LEFT_BRACE
%token     RIGHT_BRACE
%type  <n> phrase
%type  <n> modifier
%type  <n> vector
%type  <n> matrix
%type  <n> matrices
%type  <i> eof
				
%debug
			
	/* %expect 17 */

%%

stmt	:	/* null */
	| stmt eof { if ($2) YYABORT; }
	| stmt phrase eof {
            int noshow;
            node_u res = do_eval (&noshow, $2);
            if (!noshow) print_node (0, res);
/*
            walk_nodes ();       #ifdef DO_TREE
*/
            free_node (res);
            free_node ($2);
            if ($3) YYABORT;
          }
	| stmt SET SYMBOL phrase eof { do_set ($3, $4); }
	| stmt SYMBOL LEFT_PAREN phrase RIGHT_PAREN 
             LEFT_BRACE opteos phrase opteos RIGHT_BRACE eof
	        { create_function ($2, $4, $8); }
	| stmt SYMBOL LEFT_PAREN RIGHT_PAREN 
             LEFT_BRACE opteos phrase opteos RIGHT_BRACE eof
	        { create_function ($2, NULL_NODE, $7); }
	;

opteos : /* empty */
       | eof
       ;

eof     : EOS { $$ = 1; } 
	| EOP { $$ = 0; }
        ; 

phrase:   SYMBOL  { $$ = create_string_node (TYPE_SYMBOL, $1); }
	| QSTRING { $$ = create_string_node (TYPE_LITERAL, $1); }
	| NUMBER  { $$ = create_complex_node (0, $1); }
	| phrase CATENATE modifier phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_CLC, $3, $4); }
	| phrase RIGHT_DYADIC modifier phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_GSL, $3, $4); }
	| phrase RIGHT_CLC_DYADIC modifier phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_CLC, $3, $4); }
	| phrase ASSIGN modifier phrase %prec ASSIGN
		{ $$ = create_dyadic_node ($1, SYM_EQUAL,
					   OP_TYPE_CLC, $3, $4); }
	| RIGHT_DYADIC modifier phrase %prec MONADIC
		{ $$ = create_monadic_node ($1, OP_TYPE_GSL, $2, $3); }
	| RIGHT_CLC_DYADIC modifier phrase %prec MONADIC
		{ $$ = create_monadic_node ($1, OP_TYPE_CLC, $2, $3); }
	| BIF modifier LEFT_PAREN phrase RIGHT_PAREN %prec MONADIC
		{ $$ = create_monadic_node ($1, OP_TYPE_GSL, $2, $4); }
	| CLC_BIF modifier LEFT_PAREN phrase RIGHT_PAREN %prec MONADIC
		{ $$ = create_monadic_node ($1, OP_TYPE_CLC, $2, $4); }
	| LEFT_PAREN phrase RIGHT_PAREN { $$ = $2; }
	| matrix { $$ = $1; }
	| SYMBOL LEFT_PAREN phrase RIGHT_PAREN { $$ = create_call ($1, $3); }
	;

modifier : /* empty */ { $$ = NULL_NODE; }
	 | LEFT_BRACE  RIGHT_BRACE { $$ = NULL_NODE; }
	 | LEFT_BRACE phrase RIGHT_BRACE { $$ = $2; }
	 ;

matrix  : LEFT_BRACKET vector RIGHT_BRACKET { $$ = $2; }
	| LEFT_BRACKET matrices RIGHT_BRACKET { $$ = $2; }
	;

matrices : matrix  { $$ = $1; }
	 | matrices matrix { $$ = append_matrix ($1, $2); }
	 ;

vector  : /* NUMBER NUMBER */
		{ $$ = create_complex_vector_node (); }
	| vector NUMBER
		{ $$ = append_complex_vector_node ($1, $2); }
	;

%%


void
yyerror (char const *s)
{
  fprintf (stderr, "%s\n", s);
}

