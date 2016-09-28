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
//char *strdup(const char *s);


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
%left  <d> LEFT_DYADIC
%right <d> BIF
%right <d> BIF2
%right <d> BIF2R
%token     LEFT_PAREN
%token     RIGHT_PAREN
%token     COMMA
%type  <n> phrase
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
	| phrase LEFT_DYADIC phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| LEFT_DYADIC phrase { $$ = create_monadic_node ($1, $2); }
	| BIF LEFT_PAREN phrase RIGHT_PAREN { $$ = create_monadic_node ($1, $3); }
	| BIF2 LEFT_PAREN phrase COMMA phrase RIGHT_PAREN 
                 { $$ = create_dyadic_node ($3, $1, $5); }
	| BIF2R LEFT_PAREN phrase COMMA phrase RIGHT_PAREN 
                 { $$ = create_dyadic_node ($5, $1, $3); }
	| LEFT_PAREN phrase RIGHT_PAREN { $$ = $2; }
	;

%%


void
yyerror (char const *s)
{
  fprintf (stderr, "%s\n", s);
}

