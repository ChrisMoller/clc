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

#include "node.h"
#include "eval.h"
#include "printext.h"

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)

static node_u
build_vec (int rows, node_cpx_vector_s *rs)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
  node_cpx_vector_next (vs) =
    node_cpx_vector_max (vs) = rows;
  node_cpx_vector_rows (vs) = 0;
  node_cpx_vector_cols (vs) = rows;
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
  int ct = node_cpx_vector_next (rs);
  if (ct > node_cpx_vector_next (vs)) ct = node_cpx_vector_next (vs);
  memmove (node_cpx_vector_data (vs), node_cpx_vector_data (rs),
	   ct * sizeof(gsl_complex));
  return rc;
}

node_u
build_mtx (int cols, gsl_complex rv)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
  node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = cols;
  node_cpx_vector_rows (vs) = 0;
  node_cpx_vector_cols (vs) = cols;
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
  for (int i = 0; i < node_cpx_vector_next (vs); i++)
    node_cpx_vector_data (vs)[i] = rv;
  return rc;
}

node_u
init_mtx (node_cpx_vector_s *ls)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
  int rows = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[0]));
  int cols = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[1]));
  node_cpx_vector_next (vs) =
    node_cpx_vector_max (vs) = rows * cols;
  node_cpx_vector_rows (vs) = rows;
  node_cpx_vector_cols (vs) = cols;
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
  return rc;
}

node_u
clc_reshape (node_u ln, node_u rn)
{
  node_u rc = NULL_NODE;
  node_u ra = do_eval (rn);
  node_u la = do_eval (ln);

  switch(TYPE_GEN (get_type (la), get_type (ra))) {
    case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_COMPLEX):
      {
	node_cpx_vector_s *ls = node_cpx_vector (la);
	node_complex_s *rs = node_complex (ra);
	gsl_complex rv = node_complex_value (rs);
	switch(node_cpx_vector_next (ls)) {
	case 1:
	  {
	    int cols = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[0]));
	    rc = build_mtx (cols, rv);
	  }
	  break;
	case 2:
	  {
	    rc = init_mtx (ls);
	    node_cpx_vector_s *vs = node_cpx_vector (rc);
	    for (int i = 0; i < node_cpx_vector_next (vs); i++)
	      node_cpx_vector_data (vs)[i] = rv;
	  }
	  break;
	default:
	  // fixme invalid dims
	  break;
	}
      }
      break;
    case TYPE_GEN (TYPE_COMPLEX, TYPE_CPX_VECTOR):
      {
	node_complex_s *ls = node_complex (la);
	node_cpx_vector_s *rs = node_cpx_vector (ra);
	gsl_complex lv = node_complex_value (ls);
	int cols = (int)lrint (GSL_REAL (lv));
	rc = build_vec (cols, rs);
      }
      break;
    case TYPE_GEN (TYPE_COMPLEX, TYPE_COMPLEX):
      {
	node_complex_s *ls = node_complex (la);
	node_complex_s *rs = node_complex (ra);
	gsl_complex lv = node_complex_value (ls);
	gsl_complex rv = node_complex_value (rs);
	int cols = (int)lrint (GSL_REAL (lv));
	rc = build_mtx (cols, rv);
      }
      break;
    case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_CPX_VECTOR):
      {
	node_cpx_vector_s *ls = node_cpx_vector (la);
	node_cpx_vector_s *rs = node_cpx_vector (ra);
	switch(node_cpx_vector_next (ls)) {
	case 1:
	  {
	    int cols = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[0]));
	    rc = build_vec (cols, rs);
	  }
	  break;
	case 2:
	  {
	    rc = init_mtx (ls);
	    node_cpx_vector_s *vs = node_cpx_vector (rc);
	    int ct = node_cpx_vector_next (rs);
	    if (ct > node_cpx_vector_next (vs)) ct = node_cpx_vector_next (vs);
	    memmove (node_cpx_vector_data (vs), node_cpx_vector_data (rs),
		     ct * sizeof(gsl_complex));
	  }
	  break;
	default:
	  // fixme invalid dims
	  break;
	}
      }
      break;
  }
  return rc;
}

node_u
clc_shape (node_u arg)
{
  node_u rc = NULL_NODE;
  printf ("shape\n");
  return rc;
}

node_u
clc_catenate (node_u la, node_u ra)
{
  node_u rc = NULL_NODE;
  if (get_type (la) == TYPE_LIST &&
      get_type (ra) == TYPE_LIST ) {	// both lists
    node_list_s *tolist   = node_list (la);
    node_list_s *fromlist = node_list (ra);
    int needed = node_list_next (tolist) + node_list_next (fromlist);
    if (node_list_max (tolist) <= needed) {
      node_list_max (tolist) += node_list_next (fromlist);
      node_list_list (tolist) =
	realloc (node_list_list (tolist),
		 node_list_max (tolist) * sizeof(node_u)); 
    }
    memmove (&node_list_list (tolist)[node_list_next (tolist)],
	     &node_list_list (fromlist)[0],
	     node_list_next (fromlist) * sizeof (node_u));
    node_list_next (tolist) += node_list_next (fromlist);
    rc = la;
  }
  else if (get_type (la) == TYPE_LIST) {	// left list, right not
    node_list_s *list = node_list (la);
    if (node_list_max (list) <= node_list_next (list)) {
      node_list_max (list) += NODE_LIST_INCR;
      node_list_list (list) =
	realloc (node_list_list (list),
		 node_list_max (list) * sizeof(node_u)); 
    }
    node_list_list (list)[node_list_next (list)++] = ra;
    rc = la;
  }
  else if (get_type (ra) == TYPE_LIST) {	// right list, left not
    node_list_s *list = node_list (ra);
    if (node_list_max (list) <= node_list_next (list)) {
      node_list_max (list) += NODE_LIST_INCR;
      node_list_list (list) =
	realloc (node_list_list (list),
		 node_list_max (list) * sizeof(node_u)); 
    }
    memmove (&node_list_list (list)[1], &node_list_list (list)[0],
	     node_list_next (list) * sizeof (node_u));
    node_list_list (list)[0] = la;
    node_list_next (list)++;
    rc = ra;
  }
  else {					// neither lists
    node_list_s *list = malloc (sizeof(node_list_s));
    node_list_type (list) = TYPE_LIST;
    node_list_max (list) = NODE_LIST_INCR;
    node_list_list (list) = malloc (NODE_LIST_INCR * sizeof(node_u));
    node_list_list (list)[0] = la;
    node_list_list (list)[1] = ra;
    node_list_next (list) = 2;
    rc = (node_u)list;
  }
 
  return rc;
}
