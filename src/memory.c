#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

node_u
create_complex_vector_node ()
{
  node_cpx_vector_s *node = malloc (sizeof(node_cpx_vector_s));
  node_cpx_vector_type (node) = TYPE_CPX_VECTOR;
  node_cpx_vector_rows (node) = 0;
  node_cpx_vector_cols (node) = 0;
  node_cpx_vector_max  (node) = 0;
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
    node_cpx_vector_rows (node) = 0;
    node_cpx_vector_cols (node)++;
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
create_dyadic_node (node_u la, sym_e op, op_type_e op_type,
		    node_u modifier, node_u ra)
{
  node_dyadic_s *node = malloc (sizeof(node_dyadic_s));
  node_dyadic_type (node) = TYPE_DYADIC;
  node_dyadic_la (node) = la;
  node_dyadic_op (node) = op;
  node_dyadic_op_type (node) = op_type;
  node_dyadic_modifier (node) = modifier;
  node_dyadic_ra (node) = ra;
  return (node_u)node;
}

node_u
create_monadic_node (sym_e op, op_type_e op_type,
		     node_u modifier, node_u arg)
{
  node_monadic_s *node = malloc (sizeof(node_monadic_s));
  node_monadic_type (node) = TYPE_MONADIC;
  node_monadic_op (node)   = op;
  node_monadic_op_type (node) = op_type;
  node_monadic_modifier (node) = modifier;
  node_monadic_arg (node)  = arg;
  return (node_u)node;
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
