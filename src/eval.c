#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "printext.h"
#include "node.h"

typedef gsl_complex (*cpx_dyadic)(gsl_complex a, gsl_complex b);
typedef gsl_complex (*cpx_monadic)(gsl_complex a);

typedef struct {
  cpx_dyadic  dyadic;
  cpx_monadic monadic;
} ops_s;

#undef ENTRY
#define ENTRY(l,s,d,m,dd,md) {d, m}
ops_s op_table[] = {
#include "opsdata.h"
};
#define op_dyadic(s)  op_table[s].dyadic
#define op_monadic(s) op_table[s].monadic

node_type_s null_node = { TYPE_NULL };


node_u
create_string_node (type_e type, const char *s)
{
  node_string_s *node = malloc (sizeof(node_string_s));
  node_string_type (node) = type;
  node_string_value (node) = s ? strdup (s) : NULL;
  return (node_u)node;
}

node_u
create_complex_node (gsl_complex v)
{
  node_complex_s *node = malloc (sizeof(node_complex_s));
  node_complex_type (node) = TYPE_COMPLEX;
  node_complex_value (node) = v;
  return (node_u)node;
}

node_u
create_dyadic_node (node_u la, sym_e op, node_u ra)
{
  node_dyadic_s *node = malloc (sizeof(node_dyadic_s));
  node_dyadic_type (node) = TYPE_DYADIC;
  node_dyadic_la (node) = la;
  node_dyadic_op (node) = op;
  node_dyadic_ra (node) = ra;
  return (node_u)node;
}

node_u
create_monadic_node (sym_e op, node_u arg)
{
  node_monadic_s *node = malloc (sizeof(node_monadic_s));
  node_monadic_type (node) = TYPE_MONADIC;
  node_monadic_op (node)   = op;
  node_monadic_arg (node)  = arg;
  return (node_u)node;
}

node_u
do_eval (node_u node)
{
  node_u rc = (node_u)(&null_node);
  switch(get_type (node)) {
  case TYPE_NULL:
    break;
  case TYPE_COMPLEX:
  case TYPE_LITERAL:
  case TYPE_STRING:
    rc = node;
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      node_u la = do_eval (node_dyadic_la (dyad));
      node_u ra = do_eval (node_dyadic_ra (dyad));
      sym_e sym = node_dyadic_op (dyad);

      switch(TYPE_GEN (get_type (la), get_type (ra))) {
      case TYPE_GEN (TYPE_COMPLEX, TYPE_COMPLEX):
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_dyadic op = op_dyadic (sym);
	    if (op) {
	      node_complex_s *ls = node_complex (la);
	      node_complex_s *rs = node_complex (ra);
	      gsl_complex lv = node_complex_value (ls);
	      gsl_complex rv = node_complex_value (rs);
	      gsl_complex vv = (*op)(lv, rv);
	      rc = create_complex_node (vv);
	    }
	  }
	}
	break;
      case TYPE_GEN (TYPE_LITERAL, TYPE_LITERAL):
	{
	  switch (sym) {
	  case SYM_PLUS:
	    {
	      node_string_s *ls = node_string (la);
	      node_string_s *rs = node_string (ra);
	      char *lv = node_string_value (ls);
	      char *rv = node_string_value (rs);
	      char *ss = NULL;
	      asprintf (&ss, "%s%s", lv, rv);
	      if (ss) {
		rc = create_string_node (TYPE_LITERAL, ss);
		free (ss);
	      }
	    }
	    break;
	  default:
	    break;
	  }
	}
	break;
      default:
	// fixme -- 
	break;
      }
    }
    break;
  case TYPE_MONADIC:
    {
      node_monadic_s *monad = node_monadic (node);
      node_u arg = do_eval (node_monadic_arg (monad));
      sym_e sym = node_monadic_op (monad);

      switch(get_type (arg)) {
      case TYPE_COMPLEX:
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_monadic op = op_monadic (sym);
	    node_complex_s *ls = node_complex (arg);
	    gsl_complex lv = node_complex_value (ls);
	    if (op) {
	      gsl_complex vv = (*op)(lv);
	      rc = create_complex_node (vv);
	    }
	    else rc = create_complex_node (lv);
	  }
	}
	break;
      default:
	// fixme -- 
	break;
      }
    }
    break;
  }
  return rc;
}

void
print_node (node_u node)
{
  switch(get_type (node)) {
  case TYPE_COMPLEX:
    {
      node_complex_s *vs = node_complex (node);
      gsl_complex vv = node_complex_value (vs);
      fprintf (stdout, "%R\n", &vv);
    }
    break;
  case TYPE_LITERAL:
    {
      node_string_s *ss = node_string (node);
      char *sv = node_string_value (ss);
      fprintf (stdout, "%s\n", sv);
    }
    break;
  case TYPE_NULL:
    fprintf (stdout, "''\n");
    break;
  case TYPE_STRING:
  case TYPE_DYADIC:
  case TYPE_MONADIC:
    break;
  }
}

void
free_node (node_u node)
{
  switch(get_type (node)) {
  case TYPE_STRING:
    // fixme -- free referred val here then fall through
  case TYPE_LITERAL:
    {
      node_string_s *ss = node_string (node);
      if (ss) {
	char *sv = node_string_value (ss);
	if (sv) free (sv);
	free (ss);
      }
    }
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      if (dyad) {
	free_node (node_dyadic_la (dyad));
	free_node (node_dyadic_ra (dyad));
	free (dyad);
      }
    }
    break;
  case TYPE_MONADIC:
    {
      node_monadic_s *monad = node_monadic (node);
      if (monad) {
	free_node (node_monadic_arg (monad));
	free (monad);
      }
    }
    break;
  case TYPE_COMPLEX:
    {
      node_complex_s *cpx = node_complex (node);
      if (cpx) free (cpx);
    }
    break;
  case TYPE_NULL:
    break;
  }
}
