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
%left  <d> RIGHT_DYADIC
%left  <d> RIGHT_CLC_DYADIC
%right <d> BIF
%token     LEFT_PAREN
%token     RIGHT_PAREN
%token     LEFT_BRACKET
%token     RIGHT_BRACKET
%token     COMMA
%type  <n> phrase
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
	| phrase RIGHT_DYADIC phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_GSL, $3); }
	| phrase RIGHT_CLC_DYADIC phrase
		{ $$ = create_dyadic_node ($1, $2, OP_TYPE_CLC, $3); }
	| RIGHT_DYADIC phrase { $$ = create_monadic_node ($1, $2); }
	| BIF LEFT_PAREN phrase RIGHT_PAREN
		{ $$ = create_monadic_node ($1, $3); }
	| LEFT_PAREN phrase RIGHT_PAREN { $$ = $2; }
	| LEFT_BRACKET vector RIGHT_BRACKET { $$ = $2; }
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

