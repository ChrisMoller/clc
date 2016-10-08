%top{
#define _GNU_SOURCE
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <values.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>    

#include "printext.h"
#include "node.h"
#include "parser.h"
#include "memory.h"

#define yyterminate() return EOS

// http://stackoverflow.com/questions/5418181/flex-lex-encoding-strings-with-escaped-characters

static int reading_file = 0;
static double cvt_r (const char *nptr, char **endptr);
static double cvt_i (const char *nptr, char **endptr);

static void yyunput (int c, register char * yy_bp ) __attribute__ ((unused));
static int input  (void) __attribute__ ((unused));
}

DIGIT       [0-9]
NUMBER_T1   [eE]([+-])?{DIGIT}+[dDrR]?
NUMBER_T2   {DIGIT}+(\.{DIGIT}*)?([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER_T3   \.{DIGIT}+([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER      {NUMBER_T1}|{NUMBER_T2}|{NUMBER_T3}
IMAGINARY   {NUMBER}[iIjJ]
COMPLEX     {NUMBER}[+-]{IMAGINARY}
SYMBOL      [[:alpha:]][[:alnum:]_]*
QSTR        [^\\\"]*

%option noyywrap
%x QUOTED
%x COMMENT


%%

{COMPLEX}      {
  char * ep1;
  char * ep2 = NULL;
  double real, imag;
  real = cvt_r (yytext, &ep1);
  imag = cvt_i (ep1, &ep2);
  if (ep2 && (*ep2 == 'd' || *ep2 == 'D'))
      real *= M_PI / 180.0;
  yylval.v = gsl_complex_rect (real, imag);
  return NUMBER;
}

{IMAGINARY}      {
  char * ep;
  yylval.v = gsl_complex_rect (0.0, cvt_r (yytext, &ep));
  return NUMBER;
}

{NUMBER}      {
  char * ep;
  double real = cvt_r (yytext, &ep);
  if (*ep == 'd' || *ep == 'D')
     real *= M_PI / 180.0;
  yylval.v = gsl_complex_rect (real, 0.0);
  return NUMBER;
}

asinh           { yylval.d = SYM_ASINH;  return BIF; }
acosh           { yylval.d = SYM_ACOSH;  return BIF; }
atanh           { yylval.d = SYM_ATANH;  return BIF; }
sinh            { yylval.d = SYM_SINH;   return BIF; }
cosh            { yylval.d = SYM_COSH;   return BIF; }
tanh            { yylval.d = SYM_TANH;   return BIF; }
asin            { yylval.d = SYM_ASIN;   return BIF; }
acos            { yylval.d = SYM_ACOS;   return BIF; }
atan            { yylval.d = SYM_ATAN;   return BIF; }
logb            { yylval.d = SYM_LOG;    return BIF; }
real            { yylval.d = SYM_REAL;   return CLC_BIF; }
imag            { yylval.d = SYM_IMAG;   return CLC_BIF; }
sqrt            { yylval.d = SYM_SQRT;   return BIF; }
plot            { yylval.d = SYM_PLOT;   return CLC_BIF; }
sin             { yylval.d = SYM_SIN;    return BIF; }
cos             { yylval.d = SYM_COS;    return BIF; }
tan             { yylval.d = SYM_TAN;    return BIF; }
log             { yylval.d = SYM_LOG;    return BIF; }
set             { return SET; }
ln              { yylval.d = SYM_LN;     return BIF; }
\<\>            { yylval.d = SYM_LTGT;   return RIGHT_CLC_DYADIC; }
\>\<            { yylval.d = SYM_GTLT;   return RIGHT_CLC_DYADIC; }
\:\:            { yylval.d = SYM_COLCOL; return RIGHT_CLC_DYADIC; }
\/\+            { yylval.d = SYM_SLASHPLUS; return RIGHT_CLC_DYADIC; }
\/\*            { yylval.d = SYM_SLASHSTAR; return RIGHT_CLC_DYADIC; }
\@\@            { printf ("start comment\n"); BEGIN (COMMENT); }
\+              { yylval.d = SYM_PLUS;   return RIGHT_DYADIC; }
\-              { yylval.d = SYM_MINUS;  return RIGHT_DYADIC; }
\*              { yylval.d = SYM_STAR;   return RIGHT_DYADIC; }
\/              { yylval.d = SYM_SLASH;  return RIGHT_DYADIC; }
\^              { yylval.d = SYM_HAT;    return RIGHT_DYADIC; }
\<              { yylval.d = SYM_LT;     return RIGHT_CLC_DYADIC; }
\|              { yylval.d = SYM_BAR;    return RIGHT_CLC_DYADIC; }
\,		{ yylval.d = SYM_COMMA;  return LEFT_CLC_DYADIC; }
\=		{ yylval.d = SYM_EQUAL;  return RIGHT_CLC_DYADIC; }
\?		{ yylval.d = SYM_QMARK;  return RIGHT_CLC_DYADIC; }

\(		return LEFT_PAREN;
\)		return RIGHT_PAREN;
\[		return LEFT_BRACKET;
\]		return RIGHT_BRACKET;
\{		return LEFT_BRACE;
\}		return RIGHT_BRACE;


pi		{ yylval.v = gsl_complex_rect (M_PI, 0.0); return NUMBER; }
e		{ yylval.v = gsl_complex_rect (M_E,  0.0); return NUMBER; }

{SYMBOL}        {
    {	// gets here only if not a function
	yylval.s = strdup (yytext);
	return SYMBOL;
    }
}

<COMMENT>.*		/* ignore everything */
<COMMENT>[\n]		{  BEGIN (INITIAL); }

\"                      {BEGIN (QUOTED); clear_qbuffer (); }
<QUOTED>{QSTR}          { cat_string_to_qbuffer (yytext); }
<QUOTED>\\\"            { cat_char_to_qbuffer ('"'); }
<QUOTED>\"              { BEGIN (INITIAL); 
                          yylval.s = strdup (get_qbuffer());
                          return QSTRING;}

\\[\n]      /* ignore terminal backslash */

[\;\n]         {return EOP; }

[ \t]+		/* ignore whitespace */


<<EOF>> {  yyterminate (); }

.           {  printf ("unknown \"%s\"\n", yytext); }

%%


static double
cvt_r (const char *nptr, char **endptr)
{
  double r;
  if (*nptr == 'e' || *nptr == 'E') {
    char *tt = alloca (1 + strlen (nptr));
    *tt = '1';
    strcpy (tt + 1, nptr);
    r = strtod (tt, endptr);
  } else r = strtod (nptr, endptr);
  return r;
}

static double
cvt_i (const char *nptr, char **endptr)
{
  double i;
  if (nptr[1] == 'e' || nptr[1] == 'E') {
    char *tt = alloca (1 + strlen (nptr));
    *tt = *nptr;
    tt[1] = '1';
    strcpy (tt + 2, nptr+1);
    i = strtod (tt, endptr);
  } else i = strtod (nptr, endptr);
  return i;
}

void
set_string (const char *s)
{
    yy_switch_to_buffer (yy_scan_string(s));
}

void
set_file (FILE *s)
{
  reading_file = 1;
  yy_switch_to_buffer (yy_create_buffer ( s, YY_BUF_SIZE ));
}

void
delete_buffer ()
{
  reading_file = 0;
  yy_delete_buffer(YY_CURRENT_BUFFER);
}

