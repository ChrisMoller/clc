#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#ifdef DO_TREE
#include <search.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "node.h"
#include "printext.h"
#include "memory.h"
#include "eval.h"

static char *qbuffer      = NULL;
static int   qbuffer_max  = 0;
static int   qbuffer_next = 0;
#define QBUFFER_INCR 64

#ifdef DO_TREE
static void *node_root = NULL;

static int
node_compare (const void *a, const void *b)
{
  return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

static void
action (const void *nodep, const VISIT which, const int depth)
{
  //  int *datap;
  node_u vp;

  switch (which) {
  case preorder:
    break;
  case postorder:
    vp = *(node_u *)nodep;
    //    datap = *(int **) nodep;
    printf("%p %d %d post\n", node_void (vp), node_refcnt (vp), get_type (vp));
    break;
  case endorder:
    break;
  case leaf:
    vp = *(node_u *)nodep;
    //    datap = *(int **) nodep;
    printf("%p %d %d leaf\n", node_void (vp), node_refcnt (vp), get_type (vp));
    break;
  }
}
  
static void
insert_node (node_u node)
{
  tsearch(node_void (node), &node_root, node_compare);
}

void
walk_nodes ()
{
  twalk(node_root, action);
}
#endif

char *
get_qbuffer ()
{
  return qbuffer;
}

void
clear_qbuffer ()
{
  qbuffer_next = 0;
}

void
cat_string_to_qbuffer (char *str)
{
  size_t len = strlen (str);
  if (qbuffer_max <= qbuffer_next + len + 4) {
    qbuffer_max += len + QBUFFER_INCR;
    qbuffer = realloc (qbuffer, qbuffer_max);
  }
  memmove(&qbuffer[qbuffer_next], str, len);
  qbuffer_next += len;
  qbuffer[qbuffer_next] = 0;
}

void
cat_char_to_qbuffer (char chr)
{
  if (qbuffer_max <= qbuffer_next + 1) {
    qbuffer_max += QBUFFER_INCR;
    qbuffer = realloc (qbuffer, qbuffer_max);
  }
  qbuffer[qbuffer_next++] = chr;
  qbuffer[qbuffer_next] = 0;
}

void
node_incref (node_u node)
{
  node_refcnt (node)++;
  switch(get_type (node)) {
  case TYPE_LIST:
    {
      node_list_s *list = node_list (node);
      for (int i = 0; i < node_list_next (list); i++) 
	node_incref (node_list_list (list)[i]);
    }
    break;
  default:
    break;
  }
}

void
node_decref (node_u node)
{
  if (--node_refcnt (node) <= 0) free_node (node);
  switch(get_type (node)) {
  case TYPE_LIST:
    {
      node_list_s *list = node_list (node);
      for (int i = 0; i < node_list_next (list); i++) 
	node_decref (node_list_list (list)[i]);
    }
    break;
  default:
    break;
  }
}

node_u
append_matrix (node_u lv, node_u rv)
{
#if 0
  node_cpx_vector_s *ln = node_cpx_vector (lv);
  node_cpx_vector_s *rn = node_cpx_vector (rv);

  if (node_cpx_vector_cols (ln) ==
      node_cpx_vector_cols (rn)) {
    if (node_cpx_vector_max (ln) <
	(node_cpx_vector_next (ln) + node_cpx_vector_next (rn))) {
      node_cpx_vector_max (ln) += node_cpx_vector_next (rn);
      node_cpx_vector_data (ln) =
	realloc (node_cpx_vector_data (ln),
		 node_cpx_vector_max (ln) * sizeof(gsl_complex));
    }
    memmove (&node_cpx_vector_data (ln)[node_cpx_vector_next (ln)],
	     node_cpx_vector_data (rn),
	     node_cpx_vector_next (rn) * sizeof(gsl_complex));
    node_cpx_vector_next (ln) += node_cpx_vector_next (rn);
    node_cpx_vector_rows (ln)++;
    node_decref ((node_u)rn);
  }
  else {
    // fixme dim mismatch
  }
#endif
  return lv;
}

node_u
create_complex_vector_node ()
{
  node_cpx_vector_s *node = malloc (sizeof(node_cpx_vector_s));
  node_refcnt (node) = 0;
  node_cpx_vector_type (node) = TYPE_CPX_VECTOR;
  node_cpx_vector_rhorho (node) = 0;
  node_cpx_vector_rho (node) = NULL;
  node_cpx_vector_max  (node) = 0;
  node_cpx_vector_next (node) = 0;
  node_cpx_vector_data (node) = NULL;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
append_complex_vector_node (node_u vector, gsl_complex v)
{
  if (get_type (vector) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *node = node_cpx_vector (vector);
    if (node_cpx_vector_rhorho (node) == 0) {
      node_cpx_vector_rhorho (node) = 1;
      node_cpx_vector_rho (node) = malloc (sizeof(int));;
      node_cpx_vector_rho (node)[0] = 0;
    }
    if (node_cpx_vector_rhorho (node) == 1) {
      if (node_cpx_vector_max (node) <= node_cpx_vector_next (node)) {
	node_cpx_vector_max (node) += NODE_CPX_VECTOR_INCR;
	node_cpx_vector_data (node) =
	  realloc (node_cpx_vector_data (node),
		   node_cpx_vector_max (node) * sizeof(gsl_complex));
      }
      node_cpx_vector_data (node)[node_cpx_vector_next (node)++] = v;
      node_cpx_vector_rho (node)[0]++;
    }
    return (node_u)node;
  }
  return vector;
}

node_u
create_string_node (type_e type, const char *s)
{
  node_string_s *node = malloc (sizeof(node_string_s));
  node_refcnt (node) = 0;
  node_string_type (node) = type;
  node_string_value (node) = s ? strdup (s) : NULL;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
create_complex_node (int negate, gsl_complex v)
{
  node_complex_s *node = malloc (sizeof(node_complex_s));
  node_refcnt (node) = 0;
  node_complex_type (node) = TYPE_COMPLEX;
  if (negate) v = gsl_complex_negative (v);
  node_complex_value (node) = v;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
create_dyadic_node (node_u la, sym_e op, op_type_e op_type,
		    node_u modifier, node_u ra)
{
  node_dyadic_s *node = malloc (sizeof(node_dyadic_s));
  node_refcnt (node) = 0;
  node_dyadic_type (node) = TYPE_DYADIC;
  node_dyadic_la (node) = la;
  node_dyadic_op (node) = op;
  node_dyadic_op_type (node) = op_type;
  node_dyadic_modifier (node) = modifier;
  node_dyadic_ra (node) = ra;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
create_monadic_node (sym_e op, op_type_e op_type,
		     node_u modifier, node_u arg)
{
  node_monadic_s *node = malloc (sizeof(node_monadic_s));
  node_refcnt (node) = 0;
  node_monadic_type (node) = TYPE_MONADIC;
  node_monadic_op (node)   = op;
  node_monadic_op_type (node) = op_type;
  node_monadic_modifier (node) = modifier;
  node_monadic_arg (node)  = arg;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
create_composite_node (node_u la, sym_e lop, sym_e rop,
		       node_u modifier, node_u ra)
{
  node_composite_s *node = malloc (sizeof(node_composite_s));
  node_refcnt (node) = 0;
  node_composite_type (node) = TYPE_COMPOSITE;
  node_composite_la (node) = la;
  node_composite_left_op (node)  = lop;
  node_composite_right_op (node) = rop;
  node_composite_modifier (node) = modifier;
  node_composite_ra (node) = ra;
  
  return (node_u)node;
}

void
create_function (const char *name, node_u params, node_u body)
{
  node_function_s *node = malloc (sizeof(node_function_s));
  node_function_type (node) = TYPE_FUNCTION;
  node_function_refcnt (node) = 1;
  node_function_params (node) = params;
  node_incref (params);
  node_function_body (node) = body;
  node_incref (body);
  node_incref ((node_u)node);
  do_assign (name, (node_u)node);
}

node_u
create_call (char *fcn, node_u args)
{
  node_call_s *node = malloc (sizeof(node_call_s));
  node_call_type (node) = TYPE_CALL;
  node_call_refcnt (node) = 0;
  node_call_fcn (node) = fcn;
  node_call_args (node) = args;
  return (node_u)node;
}

void
free_node (node_u node)
{
  if (node_refcnt (node) > 0) return;
  switch(get_type (node)) {
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *vs = node_cpx_vector (node);
      if (vs) {
	if (node_cpx_vector_data (vs)) free (node_cpx_vector_data (vs));
	if (node_cpx_vector_rho (vs))  free (node_cpx_vector_rho (vs));
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
  case TYPE_SYMBOL:
    // fixme -- free referred val here then fall through
    break;
  case TYPE_CALL:
    {
      node_call_s *call = node_call (node);
      char *fcn  = node_call_fcn (call);
      node_u args = node_call_args (call);
      if (fcn) free (fcn);
      free_node (args);
    }
    break;
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
  case TYPE_COMPOSITE:
    break;
  case TYPE_COMPLEX:
    {
      node_complex_s *cpx = node_complex (node);
      if (cpx) free (cpx);
    }
    break;
  case TYPE_NULL:
    break;
  case TYPE_FUNCTION:
    // fixme 
    break;
  }
}
