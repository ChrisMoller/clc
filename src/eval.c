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
#include "vector.h"

typedef gsl_complex (*cpx_dyadic)(gsl_complex a, gsl_complex b);
typedef gsl_complex (*cpx_monadic)(gsl_complex a);
typedef node_u (*clc_dyadic)(node_u la, node_u ra);

typedef struct {
  void *dyadic;		/*cpx_dyadic*/
  void *monadic;	/*cpx_monadic*/
  int   nr_dyadic_args;
  int   nr_monadic_args;
} ops_s;

#undef ENTRY
#define ENTRY(l,s,d,m,dc,mc,dd,md) {d, m, dc, mc}
ops_s op_table[] = {
#include "opsdata.h"
};
#define op_dyadic(s)		op_table[s].dyadic
#define op_monadic(s)		op_table[s].monadic
#define op_nr_dyadic(s)		op_table[s].nr_dyadic_args
#define op_nr_monadic(s)	op_table[s].nr_monadic_args

static node_type_s null_node = { TYPE_NULL };


node_u
create_complex_vector_node ()
{
  node_cpx_vector_s *node = malloc (sizeof(node_cpx_vector_s));
  node_cpx_vector_type (node) = TYPE_CPX_VECTOR;
  node_cpx_vector_rows (node) = -1;
  node_cpx_vector_cols (node) = -1;
  node_cpx_vector_max (node)  = 0;
  node_cpx_vector_next (node) = 0;
  node_cpx_vector_data (node) = NULL;
  return (node_u)node;
}

node_u
append_complex_vector_node (node_u vector, gsl_complex v)
{
  // printf ("type = %d\n", get_type (vector));
  if (get_type (vector) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *node = node_cpx_vector (vector);
    if (node_cpx_vector_max (node) <= node_cpx_vector_next (node)) {
      node_cpx_vector_max (node) += NODE_CPX_VECTOR_INCR;
      node_cpx_vector_data (node) =
	realloc (node_cpx_vector_data (node),
		 node_cpx_vector_max (node) * sizeof(gsl_complex));
    }
    node_cpx_vector_data (node)[node_cpx_vector_next (node)++] = v;
    return (node_u)node;
  }
  return vector;
}

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
create_dyadic_node (node_u la, sym_e op, op_type_e op_type, node_u ra)
{
  node_dyadic_s *node = malloc (sizeof(node_dyadic_s));
  node_dyadic_type (node) = TYPE_DYADIC;
  node_dyadic_la (node) = la;
  node_dyadic_op (node) = op;
  node_dyadic_op_type (node) = op_type;
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
  case TYPE_LIST:
  case TYPE_CPX_VECTOR:
    rc = node;
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      node_u la = do_eval (node_dyadic_la (dyad));
      node_u ra = do_eval (node_dyadic_ra (dyad));
      sym_e sym = node_dyadic_op (dyad);
      op_type_e op_type = node_dyadic_op_type (dyad);

      if (op_type == OP_TYPE_CLC) {
	clc_dyadic op = op_dyadic (sym);
	if (op) rc = (*op)(la, ra);
	return rc;
      }

      switch(TYPE_GEN (get_type (la), get_type (ra))) {
      case TYPE_GEN (TYPE_COMPLEX, TYPE_CPX_VECTOR):
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_dyadic op = op_dyadic (sym);
	    if (op) {
	      node_complex_s *ls = node_complex (la);
	      node_cpx_vector_s *rs = node_cpx_vector (ra);
	      gsl_complex lv = node_complex_value (ls);
	      rc = create_complex_vector_node ();
	      node_cpx_vector_s *vs = node_cpx_vector (rc);
	      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		node_cpx_vector_next (rs);
	      node_cpx_vector_data (vs) =
		malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	      for (int i = 0; i < node_cpx_vector_next (rs); i++) {
		gsl_complex rv = node_cpx_vector_data (rs)[i];
		gsl_complex vv = (*op)(lv, rv);
		node_cpx_vector_data (vs)[i] = vv;
	      }
	    }
	  }
	}
	break;
      case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_COMPLEX):
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_dyadic op = op_dyadic (sym);
	    if (op) {
	      node_complex_s *rs = node_complex (ra);
	      node_cpx_vector_s *ls = node_cpx_vector (la);
	      gsl_complex rv = node_complex_value (rs);
	      rc = create_complex_vector_node ();
	      node_cpx_vector_s *vs = node_cpx_vector (rc);
	      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		node_cpx_vector_next (ls);
	      node_cpx_vector_data (vs) =
		malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	      for (int i = 0; i < node_cpx_vector_next (ls); i++) {
		gsl_complex lv = node_cpx_vector_data (ls)[i];
		gsl_complex vv = (*op)(lv, rv);
		node_cpx_vector_data (vs)[i] = vv;
	      }
	    }
	  }
	}
	break;
      case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_CPX_VECTOR):
	printf ("vec vec\n");
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
      case TYPE_LIST:
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    node_list_s *list = node_list (arg);
	    int nr_args = node_list_next(list);
	    switch(nr_args) {
	    case 2:
	      {
		if (op_nr_dyadic (sym) == 2) {
		  node_u a0 = do_eval (node_list_list (list)[0]);
		  node_u a1 = do_eval (node_list_list (list)[1]);
		  if (get_type (a0) == TYPE_COMPLEX &&
		      get_type (a1) == TYPE_COMPLEX) {
		    cpx_dyadic op = op_dyadic (sym);
		    node_complex_s *v0 = node_complex (a0);
		    node_complex_s *v1 = node_complex (a1);
		    gsl_complex vv = (*op)(node_complex_value (v0),
					   node_complex_value (v1));
		    rc = create_complex_node (vv);
		  }
		}
	      }
	      break;
	    default:
	      break;
	    }
	    
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
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *vs = node_cpx_vector (node);
      fprintf (stdout, "[ ");
      for (int i = 0; i < node_cpx_vector_next (vs); i++)
	fprintf (stdout, "%R ", &node_cpx_vector_data (vs)[i]);
      fprintf (stdout, "]\n");
    }
    break;
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
  case TYPE_LIST:
    {
      node_list_s *list = node_list (node);
      for (int i = 0; i < node_list_next (list); i++) 
	print_node (node_list_list (list)[i]);
    }
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
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *vs = node_cpx_vector (node);
      if (vs) {
	if (node_cpx_vector_data (vs)) free (node_cpx_vector_data (vs));
	free (vs);
      }
    }
    break;
  case TYPE_LIST:
    {
      node_list_s *list = node_list (node);
      if (list) {
	for (int i = 0; i < node_list_next (list); i++) 
	  free_node (node_list_list (list)[i]);
	free (list);
      }
    }
    break;
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
