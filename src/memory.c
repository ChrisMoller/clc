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

#include "node.h"
#include "printext.h"
#include "memory.h"

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
create_complex_vector_node ()
{
  node_cpx_vector_s *node = malloc (sizeof(node_cpx_vector_s));
  node_refcnt (node) = 0;
  node_cpx_vector_type (node) = TYPE_CPX_VECTOR;
  node_cpx_vector_rows (node) = 0;
  node_cpx_vector_cols (node) = 0;
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
  node_refcnt (node) = 0;
  node_string_type (node) = type;
  node_string_value (node) = s ? strdup (s) : NULL;
#ifdef DO_TREE
  insert_node ((node_u)node);
#endif
  return (node_u)node;
}

node_u
create_complex_node (gsl_complex v)
{
  node_complex_s *node = malloc (sizeof(node_complex_s));
  node_refcnt (node) = 0;
  node_complex_type (node) = TYPE_COMPLEX;
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
