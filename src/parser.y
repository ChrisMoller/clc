%{
#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_complex.h>

#include "printext.h"
#include "node.h"
#include "eval.h"

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

%token	   EOP
%token	   EOS
%token <v> NUMBER
%token <s> STRING
%token <s> QSTRING
%right <d> RIGHT_DYADIC
%right <d> RIGHT_CLC_DYADIC
%right <d> BIF
%right <d> CLC_BIF
%token     LEFT_PAREN
%token     RIGHT_PAREN
%token     LEFT_BRACKET
%token     RIGHT_BRACKET
%token     LEFT_BRACE
%token     RIGHT_BRACE
%type  <n> phrase
%type  <n> modifier
%type  <n> vector
%type  <i> eof
				
%debug
			
	/* %expect 17 */

%%

stmt	:	/* null */
	| stmt eof { if ($2) YYABORT; }
	| stmt phrase eof {
            node_u res = do_eval ($2);
            print_node (res);
            free_node (res);
            free_node ($2);
            if ($3) YYABORT;
          }
	;

eof     : EOS { $$ = 1; } 
	| EOP { $$ = 0; }
        ; 

phrase:   STRING  { $$ = create_string_node (TYPE_STRING, $1); }
	| QSTRING { $$ = create_string_node (TYPE_LITERAL, $1); }
	| NUMBER  { $$ = create_complex_node ($1); }
	| phrase RIGHT_DYADIC modifier phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_GSL, $3, $4); }
	| phrase RIGHT_CLC_DYADIC modifier phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_CLC, $3, $4); }
	| RIGHT_DYADIC modifier phrase
		{ $$ = create_monadic_node ($1, OP_TYPE_GSL, $2, $3); }
	| RIGHT_CLC_DYADIC modifier phrase
		{ $$ = create_monadic_node ($1, OP_TYPE_CLC, $2, $3); }
	| BIF modifier LEFT_PAREN phrase RIGHT_PAREN
		{ $$ = create_monadic_node ($1, OP_TYPE_GSL, $2, $4); }
	| CLC_BIF modifier LEFT_PAREN phrase RIGHT_PAREN
		{ $$ = create_monadic_node ($1, OP_TYPE_CLC, $2, $4); }
	| LEFT_PAREN phrase RIGHT_PAREN { $$ = $2; }
	| LEFT_BRACKET vector RIGHT_BRACKET { $$ = $2; }
	;

modifier : /* empty */ { $$ = NULL_NODE; }
	 | LEFT_BRACE phrase RIGHT_BRACE { $$ = $2; }
	 ;

vector  : /* empty */
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

