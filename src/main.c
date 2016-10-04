#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <getopt.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "printext.h"
#include "node.h"
#include "eval.h"
#include "parser.h"

typedef struct {
  const char *symbol;
  const char *dyadic_desc;
  const char *monadic_desc;
} desc_table_s;
#undef ENTRY
#define ENTRY(l,s,d,m,dc,mc,dd,md) {l, dd, md}
desc_table_s desc_table[] = {
#include "opsdata.h"
};
#define desc_symbol(d)  desc_table[d].symbol
#define desc_dyadic(d)  desc_table[d].dyadic_desc
#define desc_monadic(d) desc_table[d].monadic_desc

void set_string (const char *s);
void set_file (FILE *s);
void delete_buffer();
int yyparse (void);

int
main (int ac, char *av[])
{
  char **exprs =  NULL;
  int    exprs_next = 0;
  int    exprs_max  = 0;
  {
    int c;
    int option_index = 0;
#define EXPRS_INC	16
    static int show_ops =  0;
    static struct option long_options[] = {
      {"yydebug", no_argument,       &yydebug, 1},
      {"showops", no_argument,       &show_ops, 1},
      {0, 0, 0, 0}
    };
    while (-1 != (c = getopt_long (ac, av, "e:h",
                                   long_options, &option_index))) {
      switch(c) {
      case 'e':
	{
	  if (exprs_max <= exprs_next) {
	    exprs_max += EXPRS_INC;
	    exprs = realloc (exprs, exprs_max * sizeof(char *));
	  }
	  exprs[exprs_next++] = optarg;
	}
	break;
      case 'h':
	printf ("\nformat:\n");
	printf ("\n");
	printf ("\t%s [options] expression expression ...\n", av[0]);
	printf ("\nwhere options] are one or more of:\n");
	printf ("\n");
	printf ("\t-h		show this brief help\n");
	printf ("\t--showops	show a list ofsupported operators, \
then exit\n");
	printf ("\n");
	printf ("expressions are clc expressions, usually quoted, such as:\n");
	printf ("\n");
	printf ("\t\"[2 3]<>[1 2 3 4 5 6]\"\n");
	printf ("\t\"sin(45d}\"\n");
	break;
      }
    }
    if (show_ops) {
      fprintf (stdout, "\n%-8s %-35s\t%-35s\n\n",
	       "Operator", "Dyadic", "Monadic");
      for (int i = 0; i < SYM_COUNT; i++) {
	fprintf (stdout, "%-8s %-35s\t%-35s\n",
		 desc_symbol (i) ? : "<unused>",
		 desc_dyadic (i) ? : "<unused>",
		 desc_monadic (i) ? : "<unused>");
      }
      return 0;
    }
  }
  
  print_init ();		// sets up printf extensions

  if (exprs) {
    for (int e = 0; e < exprs_next; e++) {
      set_string (exprs[e]);
      yyparse ();
      delete_buffer ();
    }
  }
  
  for (int i = optind; i < ac; i++) {
    FILE *exec_file;
    exec_file = fopen (av[i], "r");
    if (exec_file) {                                // report errors
      set_file (exec_file);
      yyparse ();
      delete_buffer();
      fclose (exec_file);
    }
    else {	// not a file, so try to interpret it
      set_string (av[i]);
      yyparse ();
      delete_buffer ();
    }
  }


  return 0;
}
