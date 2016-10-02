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
#include <gsl/gsl_blas.h>

#include "node.h"
#include "memory.h"
#include "eval.h"
#include "printext.h"

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)
	  
#define dest_offset(r,c)  (((r) * node_cpx_vector_cols (dest)) + c)
#define src_offset(s,r,c)  (((r) * (s)) + c)

node_u
clc_sigma (node_u modifier, node_u arg)
{
  node_u rc = arg;
  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *ls = node_cpx_vector (arg);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    if (lrows == 0) {
      gsl_complex sum = {{0.0, 0.0}};
      for (int i = 0; i < lcols; i++)
	sum = gsl_complex_add (sum, node_cpx_vector_data (ls)[i]);
      rc = create_complex_node (sum);
    }
    else {
      int axis = 0;
      if (get_type (modifier) == TYPE_COMPLEX) {
	node_complex_s *mn = node_complex (modifier);
	gsl_complex mv = node_complex_value(mn);
	axis = (int)lrint (GSL_REAL (mv));
      }
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      if (axis == 0) {
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 
	  node_cpx_vector_cols (dest) = lrows;
	node_cpx_vector_data (dest) =
	  calloc (node_cpx_vector_max (dest), sizeof(gsl_complex));
	gsl_complex sum;
	for (int r = 0; r < lrows; r++) {
	  sum = gsl_complex_rect (0.0, 0.0);
	  for (int c = 0; c < lcols; c++) {
	    int off = src_offset (lcols, r, c);
	    sum = gsl_complex_add (sum, node_cpx_vector_data (ls)[off]);
	  }
	  node_cpx_vector_data (dest)[r] = sum;
	}
      }
      else {
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 
	  node_cpx_vector_cols (dest) = lcols;
	node_cpx_vector_data (dest) =
	  calloc (node_cpx_vector_max (dest), sizeof(gsl_complex));
	gsl_complex sum;
	for (int c = 0; c < lcols; c++) {
	  sum = gsl_complex_rect (0.0, 0.0);
	  for (int r = 0; r < lrows; r++) {
	    int off = src_offset (lcols, r, c);
	    sum = gsl_complex_add (sum, node_cpx_vector_data (ls)[off]);
	  }
	  node_cpx_vector_data (dest)[c] = sum;
	}
      }
    }
  }
  return rc;
}

node_u
clc_pi (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;
  return rc;
}

node_u
clc_matrix_mul (node_u modifier, node_u la, node_u ra)
{
  node_u rc = NULL_NODE;
  if (get_type (la) == TYPE_CPX_VECTOR &&
      get_type (ra) == TYPE_CPX_VECTOR) {
    // ./clc '([2 3]<>[.11 .12 .13 .21 .22 .23])><([3 2]<>[1011 1012 1021 1022 1031 1032])'
    // a b c     r s     ar+bt+cw   as+bu+cx   
    // d e f ><  t u  =  dr+et+fw	  ds+eu+fx
    // g h i     w x     gr+ht+iw	  gs+hu+ix
    // j k l             jr+kt+lw	  js+ku+lx
    node_cpx_vector_s *ls = node_cpx_vector (la);
    node_cpx_vector_s *rs = node_cpx_vector (ra);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    int rrows = node_cpx_vector_rows (rs);
    int rcols = node_cpx_vector_cols (rs);

    if (lrows == 0 && rrows == 0) {
    }
    else if (lrows == 0 && rrows != 0) {
      // ./clc '[1 2 3]><([3 2]<>[1011 1012 1021 1022 1031 1032])'
      // a b c     r s     ar+bt+cw  as+bu+cx   
      //       ><  t u  =  
      //           w x
      lrows = 1;
    }
    else if (lrows != 0 && rrows == 0) {
      // ./clc '([3 2]<>[1011 1012 1021 1022 1031 1032])><[1 2]'
      // a b c     r     ar+bs+ct
      // d e f ><  s  =  dr+es+ft
      // g h i     t     gr+hs+it
      // j k l           jr+ks+lt
      rrows = lcols;
      rcols = 1;
    }
    
    if (lcols == rrows) {
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      int drows = node_cpx_vector_rows (dest) = lrows;
      int dcols = node_cpx_vector_cols (dest) = rcols;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	lrows * rcols;
      node_cpx_vector_data (dest) =
	calloc (node_cpx_vector_max (dest), sizeof(gsl_complex));
      double *a = (double *)node_cpx_vector_data (ls);
      double *b = (double *)node_cpx_vector_data (rs);
      double *c = (double *)node_cpx_vector_data (dest);
      gsl_matrix_complex_view A =
	gsl_matrix_complex_view_array(a, lrows, lcols);
      gsl_matrix_complex_view B =
	gsl_matrix_complex_view_array(b, rrows, rcols);
      gsl_matrix_complex_view C =
	gsl_matrix_complex_view_array(c, drows, dcols);
      gsl_complex alpha = {{1.0, 0.0}};
      gsl_complex beta  = {{0.0, 0.0}};
      gsl_blas_zgemm (CblasNoTrans, CblasNoTrans,
		      alpha, &A.matrix, &B.matrix,
		      beta, &C.matrix);
    }
  }
  return rc;
}

node_u
clc_transpose (node_u modifier, node_u arg)
{
  node_u rc = arg;

  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *src  = node_cpx_vector (arg);
    int src_rows = node_cpx_vector_rows (src);
    if (src_rows == 0) {
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      int src_cols = node_cpx_vector_cols (src);
      node_cpx_vector_rows (dest) = src_cols;
      node_cpx_vector_cols (dest) = 1;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	node_cpx_vector_next (src);
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
      memmove (node_cpx_vector_data (dest), node_cpx_vector_data (src),
	       node_cpx_vector_max (dest) * sizeof(gsl_complex));
    }
    else if (src_rows > 0) {
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      int src_cols = node_cpx_vector_cols (src);
      node_cpx_vector_rows (dest) = src_cols;
      node_cpx_vector_cols (dest) = src_rows;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	node_cpx_vector_next (src);
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
  
#define xp_src_offset(r,c)  ((r) * src_cols + c)
#define xp_dst_offset(r,c) ((r) * src_rows + c)

      for (int r = 0; r < src_rows; r++) {
	for (int c = 0; c < src_cols; c++) {
	  int soff = xp_src_offset (r, c);
	  int doff = xp_dst_offset (c, r);
	  node_cpx_vector_data (dest)[doff] = node_cpx_vector_data (src)[soff];
	}
      }
    }
  }
  return rc;
}


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
clc_reshape (node_u modifier, node_u ln, node_u rn)
{
  node_u rc = NULL_NODE;
  node_u ra = rn;
  node_u la = ln;

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
clc_ravel (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;
  node_u la = arg;
  switch(get_type (la)) {
  case TYPE_COMPLEX:
    {
      node_complex_s *cn = node_complex (la);
      gsl_complex cv = node_complex_value(cn);
      rc = create_complex_node (cv);
    }
    break;
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *cn = node_cpx_vector (la);
      rc = create_complex_vector_node ();
      node_cpx_vector_s *vs = node_cpx_vector (rc);
      int ct = node_cpx_vector_next (cn);
      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = ct;
      node_cpx_vector_rows (vs) = 0;
      node_cpx_vector_cols (vs) = ct;
      node_cpx_vector_data (vs) = calloc (ct, sizeof(gsl_complex));
      memmove (node_cpx_vector_data (vs), node_cpx_vector_data (cn),
	       ct * sizeof(gsl_complex));
    }
    break;
  default:
    break;
  }
  return rc;
}

node_u
clc_shape (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;
  
  node_u la = arg;
  switch(get_type (la)) {
  case TYPE_COMPLEX:
    {
      gsl_complex cv = gsl_complex_rect (0.0, 0.0);
      rc = create_complex_node (cv);
    }
    break;
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *ls = node_cpx_vector (la);
      int rows = node_cpx_vector_rows (ls);
      int cols = node_cpx_vector_cols (ls);
      if (rows > 0) {
	gsl_complex cv = gsl_complex_rect ((double)cols, 0.0);
	gsl_complex rv = gsl_complex_rect ((double)rows, 0.0);
	rc = create_complex_vector_node ();
	node_cpx_vector_s *vs = node_cpx_vector (rc);
	node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = 2;
	node_cpx_vector_rows (vs) = 0;
	node_cpx_vector_cols (vs) = 2;
	node_cpx_vector_data (vs) =
	  calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
	node_cpx_vector_data (vs)[0] = rv;
	node_cpx_vector_data (vs)[1] = cv;
      }
      else {
	gsl_complex cv = gsl_complex_rect ((double)cols, 0.0);
	rc = create_complex_node (cv);
      }
    }
    break;
  case TYPE_LITERAL:
    {
      node_string_s *ls = node_string (la);
      char *value = node_string_value (ls);
      double len = (double)(value ? (int)strlen (value) : 0);
      gsl_complex cv = gsl_complex_rect (len, 0.0);
      rc = create_complex_node (cv);
    }
    break;
  default:
    break;
  }
  return rc;
}

node_u
clc_catenate (node_u modifier, node_u ln, node_u rn)
{
  node_u rc = NULL_NODE;
  node_u ra = rn;
  node_u la = ln;
  node_u mo = modifier;
  
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
  else if (get_type (la) == TYPE_CPX_VECTOR &&
	   get_type (ra) == TYPE_CPX_VECTOR ) {	// both vectors

    printf ("hhhhhhhhhhhhhhh\n");
    int axis = 0;
    if (get_type (mo) == TYPE_COMPLEX) {
      node_complex_s *mn = node_complex (mo);
      gsl_complex mv = node_complex_value(mn);
      axis = (int)lrint (GSL_REAL (mv));
    }

    node_cpx_vector_s *ls = node_cpx_vector (la);
    node_cpx_vector_s *rs = node_cpx_vector (ra);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    int rrows = node_cpx_vector_rows (rs);
    int rcols = node_cpx_vector_cols (rs);

    if (axis == 0) {				// axis == 0
      if (lrows == 0 && rrows == 0) {		// vec vec
	// a b c , x y z = a b c x y z
	rc = create_complex_vector_node ();
	node_cpx_vector_s *dest = node_cpx_vector (rc);
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	  node_cpx_vector_cols (dest) = lcols + rcols;
	node_cpx_vector_data (dest) =
	  malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	memmove (node_cpx_vector_data (dest),
		 node_cpx_vector_data (ls), lcols * sizeof(gsl_complex));
	memmove (&node_cpx_vector_data (dest)[lcols],
		 node_cpx_vector_data (rs), rcols * sizeof(gsl_complex));
      }
      else if (lrows == 0 && rrows != 0) {	// vec mtx
	// a b c , u v   a u v
	//         w x = b w x
	//         y z   c y z
	if (lcols == rrows) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = rrows;
	  node_cpx_vector_cols (dest) = rcols + 1;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  int r, c;
	  for (c = 0; c < lcols; c++)
	    node_cpx_vector_data (dest)[dest_offset (c, 0)] =
	      node_cpx_vector_data (ls)[c];
	  for (r = 0; r < rrows; r++) {
	    for (c = 0; c < rcols; c++) {
	      node_cpx_vector_data (dest)[dest_offset (r, c + 1)] =
		node_cpx_vector_data (rs)[src_offset (rcols, r, c)];
	    }
	  }
	}
	else {
	  // fixme -- dim mismatch
	}
      }
      else if (lrows != 0 && rrows == 0) {	// mtx vec
	// a b               a b x
	// c d  ,  x y z  =  c d y
	// e f               e f z
	if (lrows == rcols) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = lrows;
	  node_cpx_vector_cols (dest) = lcols + 1;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  int r, c;
	  for (r = 0; r < lrows; r++) {
	    for (c = 0; c < lcols; c++) {
	      node_cpx_vector_data (dest)[dest_offset (r, c)] =
		node_cpx_vector_data (ls)[src_offset (lcols, r, c)];
	    }
	  }
	  for (c = 0; c < rcols; c++)
	    node_cpx_vector_data (dest)[dest_offset (c, lcols)] =
	      node_cpx_vector_data (rs)[c];
	}
	else {
	  // fixme -- dim mismatch
	}
      }
      else if (lrows != 0 && rrows != 0) {	// mtx mtx
	// a b c     u v     a b c u v
	// d e f  ,  w x  =  d e f w x
	// g h i     y z     g h i y z
	if (lrows == rrows) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = lrows;
	  node_cpx_vector_cols (dest) = lcols + rcols;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  int r, c;
	  for (r = 0; r < lrows; r++) {
	    for (c = 0; c < lcols; c++) {
	      node_cpx_vector_data (dest)[dest_offset (r, c)] =
		node_cpx_vector_data (ls)[src_offset (lcols, r, c)];
	    }
	  }
	  for (r = 0; r < rrows; r++) {
	    for (c = 0; c < rcols; c++) {
	      node_cpx_vector_data (dest)[dest_offset (r, c+lcols)] =
		node_cpx_vector_data (rs)[src_offset (rcols, r, c)];
	    }
	  }
	}
	else {
	  // fixme -- dim mismatch
	}
      }
    }
    else {					// axis == 1
      if (lrows == 0 && rrows == 0) {		// vec vec
	// a b c ,{1} x y z = a b c
	//                    x y z
	if (lcols == rcols) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = 2;
	  node_cpx_vector_cols (dest) = rcols;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    lcols + rcols;
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  memmove (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls), lcols * sizeof(gsl_complex));
	  memmove (&node_cpx_vector_data (dest)[lcols],
		   node_cpx_vector_data (rs), rcols * sizeof(gsl_complex));
	}
	else {
	  // fixme -- dim mismatch
	}
      }
      else if (lrows == 0 && rrows != 0) {	// vec mtx
	// a b c ,{1} u v w   a b c
	//            x y z = u v w
	//                 x y z
	if (lcols == rcols) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = rrows + 1;
	  node_cpx_vector_cols (dest) = rcols;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  memmove (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memmove (&node_cpx_vector_data (dest)[lcols],
		   node_cpx_vector_data (rs),
		   node_cpx_vector_next (rs) * sizeof(gsl_complex));
	}
	else {
	  // fixme -- dim mismatch
	}
      }
      else if (lrows != 0 && rrows == 0) {	// mtx vec
	// a b c               a b c
	// d e f  ,  x y z  =  d e f
	//                     x y z
	if (lcols == rcols) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = lrows + 1;
	  node_cpx_vector_cols (dest) = lcols;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  memmove (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memmove (&node_cpx_vector_data (dest)[node_cpx_vector_next (ls)],
		   node_cpx_vector_data (rs),
		   node_cpx_vector_next (rs) * sizeof(gsl_complex));
	}
	else {
	  // fixme -- dim mismatch
	}
      }
      else if (lrows != 0 && rrows != 0) {	// mtx mtx
	// a b c     u v w     a b c
	// d e f  ,  x y z  =  d e f
	// g h i               g h i
	//                     u v w
	//                     x y z
	if (lcols == rcols) {
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rows (dest) = lrows + rrows;
	  node_cpx_vector_cols (dest) = lcols;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  memmove (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memmove (&node_cpx_vector_data (dest)[node_cpx_vector_next (ls)],
		   node_cpx_vector_data (rs),
		   node_cpx_vector_next (rs) * sizeof(gsl_complex));
	}
	else {
	  // fixme -- dim mismatch
	}
      }
    }
  }
  else if (get_type (la) == TYPE_CPX_VECTOR &&
	   get_type (ra) == TYPE_COMPLEX ) {	//
    // fixme
  }
  else if (get_type (la) == TYPE_COMPLEX &&
	   get_type (ra) == TYPE_CPX_VECTOR ) {	//
    // fixme
  }
  else if (get_type (la) == TYPE_COMPLEX &&
	   get_type (ra) == TYPE_COMPLEX ) {	//
    node_complex_s *ls = node_complex (la);
    gsl_complex lv = node_complex_value(ls);
    node_complex_s *rs = node_complex (ra);
    gsl_complex rv = node_complex_value(rs);
    rc = create_complex_vector_node ();
    node_cpx_vector_s *dest = node_cpx_vector (rc);
    node_cpx_vector_rows (dest) = 0;
    node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
      node_cpx_vector_cols (dest) = 2;
    node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
    node_cpx_vector_data (dest)[0] = lv;
    node_cpx_vector_data (dest)[1] = rv;
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