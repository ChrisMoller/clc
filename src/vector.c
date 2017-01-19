#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <alloca.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_blas.h>

#include "node.h"
#include "memory.h"
#include "eval.h"
#include "printext.h"

/***


    A B C   U X   (AU + BV + CW)  (AX + BY + CZ)
    D E F * V y = (DU + EV + FW)  (DX + EY + FZ)
            W Z

    A = Aa Ab   B = Ba Bb   C = Ca Cb
        Ac Ad       Bc Bd       Cc Cd

    D = Da Db   E = Ea Eb   F = Fa Fb
        Dc Dd       Ec Ed       Fc Fd

	                 U = Ua Ub   X = Xa Xb
	                     Uc Ud       Xc Xd

			 V = Va Vb   Y = Ya Yb
			     Vc Vd       Yc Td

			 W = Wa Wb   Z = Za Zb
			     Wc Wd       Zc Zd

    A [2 3 4] = [a b]

    a =     
 1  2  3  4
 5  6  7  8
 9 10 11 12

    b =
13 14 15 16
17 18 19 20
21 22 23 24


    B [4 3 2] = [c d]
    b:   [4 3 2]

 1  2    [ 0 0 0]  [0 0 1]
 3  4    [ 0 1 0]  [0 1 1]
 5  6    [ 0 2 0]

 7  8    [ 1 0 0]
 9 10    [ 1 1 0]
11 12    [ 1 2 0]

13 14    [ 2 0 0]
15 16    [ 2 1 0]
17 18    [ 2 2 0]

19 20    [ 3 0 0]
21 22    [ 3 1 0]
23 24    [ 3 2 0]



   c:   [2 3 3 2]

 130  140	c[0;0;0;0]   c[0;0;0;1]
 150  160       c[0;0;1;0]   c[0;0;1;1]
 170  180       c[0;0;2;0]   c[0;0;2;1]

 290  316
 342  368
 394  420

 450  492
 534  576
 618  660


 610  668
 726  784
 842  900

 770  844
 918  992
1066 1140

 930 1020
1110 1200
1290 1380


 ***/

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)
	  
#define dest_offset(r,c)  (((r) * node_cpx_vector_cols (dest)) + c)
#define src_offset(s,r,c)  (((r) * (s)) + c)

node_u
clc_sigma (node_u modifier, node_u argi)
{
  node_u rc = NULL_NODE;
#if 0
  node_u arg = do_eval (NULL, argi);
  node_u rc = arg;
  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *ls = node_cpx_vector (arg);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    if (lrows == 0) {
      // ./clc 'a = 4; b = 3; /+(a,b)'
      gsl_complex sum = {{0.0, 0.0}};
      for (int i = 0; i < lcols; i++)
	sum = gsl_complex_add (sum, node_cpx_vector_data (ls)[i]);
      rc = create_complex_node (0, sum);
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
	// ./clc 'a = 4; b = 3; /+(a,b)<>1::(a*b)'
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
	// ./clc 'a = 4; b = 3; /+{1}(a,b)<>1::(a*b)'
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
#endif
  return rc;
}

node_u
clc_pi (node_u modifier, node_u argi)
{
  node_u rc = NULL_NODE;
#if 0
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *ls = node_cpx_vector (arg);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    if (lrows == 0) {
      // ./clc 'a = 4; b = 3; /*(a,b)'
      gsl_complex sum = {{1.0, 0.0}};
      for (int i = 0; i < lcols; i++)
	sum = gsl_complex_mul (sum, node_cpx_vector_data (ls)[i]);
      rc = create_complex_node (0, sum);
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
	// ./clc 'a = 4; b = 3; /*(a,b)<>1::(a*b)'
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 
	  node_cpx_vector_cols (dest) = lrows;
	node_cpx_vector_data (dest) =
	  calloc (node_cpx_vector_max (dest), sizeof(gsl_complex));
	gsl_complex sum;
	for (int r = 0; r < lrows; r++) {
	  sum = gsl_complex_rect (1.0, 0.0);
	  for (int c = 0; c < lcols; c++) {
	    int off = src_offset (lcols, r, c);
	    sum = gsl_complex_mul (sum, node_cpx_vector_data (ls)[off]);
	  }
	  node_cpx_vector_data (dest)[r] = sum;
	}
      }
      else {
	// ./clc 'a = 4; b = 3; /*{1}(a,b)<>1::(a*b)'
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 
	  node_cpx_vector_cols (dest) = lcols;
	node_cpx_vector_data (dest) =
	  calloc (node_cpx_vector_max (dest), sizeof(gsl_complex));
	gsl_complex sum;
	for (int c = 0; c < lcols; c++) {
	  sum = gsl_complex_rect (1.0, 0.0);
	  for (int r = 0; r < lrows; r++) {
	    int off = src_offset (lcols, r, c);
	    sum = gsl_complex_mul (sum, node_cpx_vector_data (ls)[off]);
	  }
	  node_cpx_vector_data (dest)[c] = sum;
	}
      }
    }
  }
#endif
  return rc;
}

node_u
clc_extract (node_u modifier, node_u lai, node_u rai)
{
  node_u rc = NULL_NODE;
#if 0
  node_u la = do_eval (NULL, lai);
  node_u ra = do_eval (NULL, rai);
  //* [a b c] <> 1 ==> b
  //
  //* [a b c] <> [2 0] ==> [c a], both rows = 0, rcols > 0, rcols <= lcols
  //
  // [ a b c
  //   d e f     <> [1 -1] ==> [d e f]
  //   g h i ]
  //
  // [ a b c
  //   d e f     <> [-1 1] ==> [b e h]
  //   g h i ]
  //
  // [ a b c
  //   d e f     <> [1 1] ==> e
  //   g h i ]
  switch(TYPE_GEN (get_type (la), get_type (ra))) {
  case TYPE_GEN (TYPE_LITERAL, TYPE_COMPLEX):
    break;
  case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_COMPLEX):
    {
      node_cpx_vector_s *ls = node_cpx_vector (la);
      int lrows = node_cpx_vector_rows (ls);
      if (lrows == 0 || lrows == 1) {
	int lcols = node_cpx_vector_cols (ls);
	node_complex_s *rs = node_complex (ra);
	int rv = (int)GSL_REAL (node_complex_value (rs));
	if (rv >= 0 && rv < lcols) {
	  gsl_complex lv = node_cpx_vector_data (ls)[rv];
	  rc = create_complex_node (0, lv);
	}
      }
    }
    break;
  case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_CPX_VECTOR):
    {
#define ex_dest_offset(r,c)  (((r) * node_cpx_vector_cols (ls)) + c)
      node_cpx_vector_s *ls = node_cpx_vector (la);
      node_cpx_vector_s *rs = node_cpx_vector (ra);
      int lrows = node_cpx_vector_rows (ls);
      int rrows = node_cpx_vector_rows (rs);
      int lcols = node_cpx_vector_cols (ls);
      int rcols = node_cpx_vector_cols (rs);
      if (lrows == 0 && rrows == 0) {
	rc = create_complex_vector_node ();
	node_cpx_vector_s *dest = node_cpx_vector (rc);
	node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	  node_cpx_vector_cols (dest) = rcols;
	node_cpx_vector_rows (dest) = 0;
	node_cpx_vector_data (dest) =
	  malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	for (int i = 0; i < rcols; i++) {
	  int offset = (int)GSL_REAL (node_cpx_vector_data (rs)[i]);
	  node_cpx_vector_data (dest)[i] = 
	    (offset >= 0 && offset < lcols) ?
	    node_cpx_vector_data (ls)[offset] : gsl_complex_rect (NAN, NAN);
	}
      }
      else if (lrows > 0 && rcols == 2) {
	if (GSL_REAL (node_cpx_vector_data (rs)[0]) < 0.0) {	// select col
	  //./clc '([4 3 ]#::12)<>[~1 2]'
	  int offset = (int)GSL_REAL (node_cpx_vector_data (rs)[1]);
	  if (offset > 0 && offset <= lcols) {
	    rc = create_complex_vector_node ();
	    node_cpx_vector_s *dest = node_cpx_vector (rc);
	    node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	      node_cpx_vector_cols (dest) = lrows;
	    node_cpx_vector_rows (dest) = 0;
	    node_cpx_vector_data (dest) =
	      malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	    for (int i = 0; i < lrows; i++)
	      node_cpx_vector_data (dest)[i] =
		node_cpx_vector_data (ls)[ex_dest_offset (i, offset)];
	  }
	}
	else if (GSL_REAL (node_cpx_vector_data (rs)[1]) < 0.0) { // select row
	  //./clc '([4 3 ]#::12)<>[2 ~1]'
	  int offset = (int)GSL_REAL (node_cpx_vector_data (rs)[0]);
	  if (offset > 0 && offset <= lrows) {
	    rc = create_complex_vector_node ();
	    node_cpx_vector_s *dest = node_cpx_vector (rc);
	    node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	      node_cpx_vector_cols (dest) = lcols;
	    node_cpx_vector_rows (dest) = 0;
	    node_cpx_vector_data (dest) =
	      malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	    for (int i = 0; i < lcols; i++)
	      node_cpx_vector_data (dest)[i] =
		node_cpx_vector_data (ls)[ex_dest_offset (offset, i)];
	  }
	}
	else {
	  // ./clc '([4 3 ]#::12)<>[2 1]'
	  int ir = (int)GSL_REAL (node_cpx_vector_data (rs)[0]);
	  int ic = (int)GSL_REAL (node_cpx_vector_data (rs)[1]);
	  if (ir >= 0 && ir < lrows &&
	      ic >= 0 && ic < lcols) {
	    gsl_complex vv =
	      node_cpx_vector_data (ls)[ex_dest_offset (ir, ic)];
	    rc = create_complex_node (0, vv);
	  }
	}
      }
    }
    break;
  }
#endif
  return rc;
}

/***
    https://en.wikipedia.org/wiki/Outer_product
***/

node_u
clc_matrix_mul (node_u modifier, node_u lai, node_u rai)
{
  node_u rc = NULL_NODE;
#if 0
  node_u la = do_eval (NULL, lai);
  node_u ra = do_eval (NULL, rai);
  if (get_type (la) == TYPE_CPX_VECTOR &&
      get_type (ra) == TYPE_CPX_VECTOR) {
    // ./clc '([2 3]<>[.11 .12 .13 .21 .22 .23])><([3 2]<>[1011 1012 1021 1022 1031 1032])'
    // a b c     r s     ar+bt+cw   as+bu+cx   
    // d e f ><  t u  =  dr+et+fw   ds+eu+fx
    // g h i     w x     gr+ht+iw   gs+hu+ix
    // j k l             jr+kt+lw   js+ku+lx
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
#endif
  return rc;
}

node_u
clc_transpose (node_u modifier, node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  node_u rc = arg;

  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *src  = node_cpx_vector (arg);
    int rhorho = node_cpx_vector_rhorho (src);
    int *rhop = malloc (rhorho * sizeof(int));
    memcpy (rhop, node_cpx_vector_rho (src), rhorho * sizeof(int));
    int *perm = alloca (rhorho * sizeof(int));
    for (int i = 0; i < rhorho; i++) perm[i] = i;


#if 1
    int mod_ok = 1;
    if (node_void (modifier)) {
      mod_ok = 0;
      if (get_type (modifier) == TYPE_CPX_VECTOR) {
	node_cpx_vector_s *mod  = node_cpx_vector (modifier);
	if (node_cpx_vector_rhorho (mod) == 1) {
	  if (node_cpx_vector_rho (mod)[0] == rhorho) {
	    for (int i = 0; i < rhorho; i++) 
	      perm[i] = (int)GSL_REAL (node_cpx_vector_data (mod)[i]);
	    mod_ok = 1;
	  }
	  else {
	    // fixme mod length != arg rhorho
	  }
	}
	else {
	  // fixme mod not a vector
	}
      }
      else {
	// fixme mod not a vector
      }
    }
    if (mod_ok) {
      for (int i = 0; i < rhorho; i++) 
	rhop[i] = node_cpx_vector_rho (src)[perm[i]];
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      node_cpx_vector_rhorho (dest) = node_cpx_vector_rhorho (src);
      node_cpx_vector_rho (dest) = rhop;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 
	node_cpx_vector_next (src);
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_next (dest) * sizeof(gsl_complex));
      int *dstride = alloca (rhorho * sizeof(int));
      dstride[0] = 1;
      for (int i = 1; i < rhorho; i++) 
	dstride[i] = dstride[i - 1] * rhop[i - 1];
      int *ix = alloca (rhorho * sizeof(int));
      bzero (ix, rhorho * sizeof(int));
      int *iy = alloca (rhorho * sizeof(int));
      int run = 1;
      while(run) {
	int offs, offd;
	for (int i = 0; i< rhorho; i++) iy[i] = ix[perm[i]];

	offd = 0;
	for (int i = 0; i< rhorho; i++) 
	  offd += iy[i] * dstride[i];

	node_cpx_vector_data (dest)[offd] =
	  node_cpx_vector_data (src)[offs++];

	int carry = 1;
	int i;
	for (i = 0; carry && i < rhorho; i++) {
	  if (carry) {
	    if (++ix[i] >= rhop[i]) ix[i] = 0;
	    else {
	      carry = 0;
	      break;
	    }
	  }
	}
	if (carry && i == rhorho) run = 0;
      }
    }
    else {
      // fixme mod error
    }
    
#else
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
      memcpy (node_cpx_vector_data (dest), node_cpx_vector_data (src),
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
#endif
  }
  return rc;
}


static node_u
build_vec (int len, node_cpx_vector_s *rs)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
#if 1
  node_cpx_vector_rhorho (vs) = 1;
  node_cpx_vector_rho (vs) = malloc (sizeof(int));
  node_cpx_vector_next (rs) = node_cpx_vector_rho (vs)[0] = len;
#else
  node_cpx_vector_rows (vs) = 0;
  if (rows == 0) {
    node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = 
      node_cpx_vector_cols (vs) = node_cpx_vector_next (rs);
  }
  else {
    node_cpx_vector_cols (vs) = rows;
    node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = rows;
  }
#endif
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
  if (len > node_cpx_vector_next (rs)) len = node_cpx_vector_next (rs);
  memcpy (node_cpx_vector_data (vs),
	  node_cpx_vector_data (rs),
	  len * sizeof(gsl_complex));
  return rc;
}

node_u
build_mtx (int len, gsl_complex rv)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
  node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = len;
#if 1
  node_cpx_vector_rhorho (vs) = 1;
  node_cpx_vector_rho (vs) = malloc (sizeof(int));
  node_cpx_vector_rho (vs)[0] = len;
#else
  node_cpx_vector_rows (vs) = 0;
  node_cpx_vector_cols (vs) = cols;
#endif
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
  for (int i = 0; i < len; i++) node_cpx_vector_data (vs)[i] = rv;
  return rc;
}

node_u
init_mtx (node_cpx_vector_s *ls)
{
  node_u rc = create_complex_vector_node ();
  node_cpx_vector_s *vs = node_cpx_vector (rc);
#if 1
  if (node_cpx_vector_rhorho (ls) == 1) {
    int i;
    int w;
    for(i = 0, w = 1; i < node_cpx_vector_rho (ls)[0]; i++) {
      w *= (int)GSL_REAL (node_cpx_vector_data (ls)[i]);
      if ((int)GSL_REAL (node_cpx_vector_data (ls)[i]) <= 0) break;
    }
    if (i == node_cpx_vector_rho (ls)[0]) {
      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = w;
      node_cpx_vector_rhorho (vs) = 1;
      node_cpx_vector_rho (vs) =
	malloc (node_cpx_vector_next (ls) * sizeof(int));
      node_cpx_vector_data (vs) = calloc (w, sizeof(gsl_complex));
    }
    else {
      // fixme 0 dim
    }
  }
  else {
    // fixme impossible
  }
#else
  int rows = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[0]));
  int cols = (int)lrint (GSL_REAL (node_cpx_vector_data (ls)[1]));
  node_cpx_vector_next (vs) =
    node_cpx_vector_max (vs) = rows * cols;
  node_cpx_vector_rows (vs) = rows;
  node_cpx_vector_cols (vs) = cols;
  node_cpx_vector_data (vs) =
    calloc (node_cpx_vector_next (vs), sizeof(gsl_complex));
#endif
  return rc;
}

node_u
clc_reshape (node_u modifier, node_u lni, node_u rni)
{
  node_u ln = do_eval (NULL, lni);
  node_u rn = do_eval (NULL, rni);
  node_u rc = NULL_NODE;
  node_u ra = rn;
  node_u la = ln;

  switch(TYPE_GEN (get_type (la), get_type (ra))) {
    case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_COMPLEX):
      {
	// [a b c d ...] # v
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
	// v # [a b c d ...]
	node_complex_s *ls = node_complex (la);
	node_cpx_vector_s *rs = node_cpx_vector (ra);
	gsl_complex lv = node_complex_value (ls);
	int cols = (int)lrint (GSL_REAL (lv));
	rc = build_vec (cols, rs);
      }
      break;
    case TYPE_GEN (TYPE_COMPLEX, TYPE_COMPLEX):
      {
	// [a b c d ...] # [z y x w ...]
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
	// v # w
#if 1
	node_cpx_vector_s *ls = node_cpx_vector (la);
	node_cpx_vector_s *rs = node_cpx_vector (ra);
	rc = init_mtx (ls);
	node_cpx_vector_s *vs = node_cpx_vector (rc);
	if (node_cpx_vector_rho (vs)) free (node_cpx_vector_rho (vs));
	node_cpx_vector_rhorho (vs) = node_cpx_vector_next (ls);
	node_cpx_vector_rho (vs) =
	  malloc (node_cpx_vector_next (ls) * sizeof(int));
	for (int i = 0; i < node_cpx_vector_next (ls); i++)
	  node_cpx_vector_rho (vs)[i] =
	    (int)GSL_REAL (node_cpx_vector_data (ls)[i]);
	int ct = node_cpx_vector_next (vs);
	if (ct > node_cpx_vector_next (rs))
	  ct = node_cpx_vector_next (rs);
	memcpy (node_cpx_vector_data (vs),
		node_cpx_vector_data (rs),
		ct * sizeof(gsl_complex));
#else
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
	    memcpy (node_cpx_vector_data (vs), node_cpx_vector_data (rs),
		     ct * sizeof(gsl_complex));
	  }
	  break;
	default:
	  // fixme invalid dims
	  break;
	}
#endif
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
      rc = create_complex_node (0, cv);
    }
    break;
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *cn = node_cpx_vector (la);
      rc = create_complex_vector_node ();
      node_cpx_vector_s *vs = node_cpx_vector (rc);
      int ct = node_cpx_vector_next (cn);
      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) = ct;
#if 1
      node_cpx_vector_rhorho (vs) = 1;
      node_cpx_vector_rho (vs) = malloc (sizeof(int));
      node_cpx_vector_rho (vs)[0] = ct;
#else
      node_cpx_vector_rows (vs) = 0;
      node_cpx_vector_cols (vs) = ct;
#endif
      node_cpx_vector_data (vs) = calloc (ct, sizeof(gsl_complex));
      memcpy (node_cpx_vector_data (vs), node_cpx_vector_data (cn),
	       ct * sizeof(gsl_complex));
    }
    break;
  default:
    break;
  }
  return rc;
}

node_u
clc_shape (node_u modifier, node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  node_u rc = NULL_NODE;
  
  node_u la = arg;
  switch(get_type (la)) {
  case TYPE_COMPLEX:
    {
      gsl_complex cv = gsl_complex_rect (0.0, 0.0);
      rc = create_complex_node (0, cv);
    }
    break;
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *ls = node_cpx_vector (la);
#if 1
      rc = create_complex_vector_node ();
      node_cpx_vector_s *vs = node_cpx_vector (rc);
      node_cpx_vector_rhorho (vs) = 1;
      node_cpx_vector_rho (vs) = malloc (sizeof(int));
      node_cpx_vector_max (vs) = node_cpx_vector_next (vs) = 
	node_cpx_vector_rho (vs)[0] = node_cpx_vector_rhorho (ls);
      node_cpx_vector_data (vs) =
	malloc (node_cpx_vector_max (vs) * sizeof(int));
      memcpy (node_cpx_vector_data (vs),
	       node_cpx_vector_rho (ls),
	       node_cpx_vector_max (vs) * sizeof(int));
#else
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
	rc = create_complex_node (0, cv);
      }
#endif
    }
    break;
  case TYPE_LITERAL:
    {
      node_string_s *ls = node_string (la);
      char *value = node_string_value (ls);
      double len = (double)(value ? (int)strlen (value) : 0);
      gsl_complex cv = gsl_complex_rect (len, 0.0);
      rc = create_complex_node (0, cv);
    }
    break;
  default:
    break;
  }
  return rc;
}

node_u
clc_catenate (node_u modifier, node_u lni, node_u rni)
{
  node_u ln = do_eval (NULL, lni);
  node_u rn = do_eval (NULL, rni);
  node_u rc = NULL_NODE;
  node_u ra = rn;
  node_u la = ln;
  node_u mo = modifier;
  if (get_type (la) == TYPE_LIST &&
      get_type (ra) == TYPE_LIST ) {	// both lists
#if 1
    node_list_s *list = malloc (sizeof(node_list_s));
    node_list_type (list) = TYPE_LIST;
    node_list_max (list) = NODE_LIST_INCR;
    node_list_list (list) = malloc (NODE_LIST_INCR * sizeof(node_u));
    node_list_list (list)[0] = la;
    node_list_list (list)[1] = ra;
    node_list_next (list) = 2;
    rc = (node_u)list;
#else
    node_list_s *tolist   = node_list (la);
    node_list_s *fromlist = node_list (ra);
    int needed = node_list_next (tolist) + node_list_next (fromlist);
    if (node_list_max (tolist) <= needed) {
      node_list_max (tolist) += node_list_next (fromlist);
      node_list_list (tolist) =
	realloc (node_list_list (tolist),
		 node_list_max (tolist) * sizeof(node_u)); 
    }
    memcpy (&node_list_list (tolist)[node_list_next (tolist)],
	     &node_list_list (fromlist)[0],
	     node_list_next (fromlist) * sizeof (node_u));
    node_list_next (tolist) += node_list_next (fromlist);
    rc = la;
#endif
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
    memcpy (&node_list_list (list)[1], &node_list_list (list)[0],
	     node_list_next (list) * sizeof (node_u));
    node_list_list (list)[0] = la;
    node_list_next (list)++;
    rc = ra;
  }
  else if (get_type (la) == TYPE_CPX_VECTOR &&
	   get_type (ra) == TYPE_CPX_VECTOR ) {	// both vectors

#if 1
    /***
ppL = 1          ppR = 1    ppV = 1       ppL = ppR && m < ppL 
pL =  3          pR  = 2    pV  = 5       pL[i != m] = pR
	a b c,{0} d e ==> a b c d e           ppV = ppL
	                                      pV[m] = pL[m] + pR[m]
					      pV[i != m] = pL

ppL = 1          ppR = 1    ppV = 2       ppL = ppR = m
pL  = 3          pR  = 3    pV  = 2 3     pL = pR
	a b c,[1] d e f ==> a b c             ppV = ppL + 1
	                    d e f             pV  = 2, pL

ppL = 2          ppR = 1    ppV = 2       ppL = ppR + 1 && m < ppL
pL  = 2 3        pR  = 2    pV  = 2 4     pL[i != m] = pR
	a b c             a b c g             ppV = ppL
	d e f,{0) g h ==> d e f h             pV = pL, pV[m] = pL[m] + 1

ppL = 2          ppR = 1    ppV = 2       ppL = ppR + 1 && m < ppL
pL  = 2 3        pR  = 3    pV  = 3 3     pL[i != m] = pR
	a b c               a b c             ppV = ppL
	d e f,{1} g h i ==> d e f             pV = pL, pV[m] = pL[m] + 1
	                    g h i

ppL = 2          ppR = 2    ppV = 2       ppL = ppR && m < ppL
pL  = 2 3        pR  = 2 2  pV  = 2 5     pL[i != m] = pR
	a b c     g h      a b c g h          ppV = ppL
	d e f,{0} i j  ==> d e f i j          pV[m] = pL[m] + pR[m]
	                                      pV[i != m] = pL

ppL = 2          ppR = 2    ppV = 2       ppL = ppR && m < ppL
pL  2 3          pR  = 3 3  pV  = 5 3     pL[i != m] = pR
	a b c     g h i     a b c             ppV = ppL
	d e f,{1} j k l ==> d e f             pV[m] = pL[m] + pR[m]
                  m n o     g h i             pV[i != m] = pL
			    j k l
	                    m n o

ppL = 2          ppR = 2     ppV = 3       ppL = ppR = m
Pl  = 2 3        pR  = 2 3   pV  = 2 3     pL = pR
	a b c     g h i      a b c             ppV = ppL + 1
	d e f,{2} j k l ==>  d e f             pV  = 2, pL

	                     g h i
			     j k l

ppL = 3         ppR = 2    ppV = 3         ppL = ppR + 1 && m < ppL
pL  = 2 4 3     pR  = 2 4  pV  = 2 4 4     pL[i != m] = pR
	a b c                 a b c 0          ppV = ppL
	d e f                 d e f 1          pV = pL, pV[m] = pL[m] + 1
	g h i                 g h i 2
	j k l     0 1 2 3     j k l 3
             ,{0} 4 5 6 7 ==> 
	m n o                 m n o 4
	p q r                 p q r 5
	s t u                 s t u 6
	v w x                 v w x 7

ppL = 3        ppR = 2      ppV = 3         ppL = ppR + 1 && m < ppL
Pl  = 2 4 3    pR  = 2 3    pV  = 2 5 3     pL[i != m] = pR
	a b c               a b c                ppV = ppL
	d e f               d e f                pV = pL, pV[m] = pL[m] + 1
	g h i               g h i
	j k l     0 1 2     j k l
             ,{1} 3 4 5 ==> 0 1 2
	m n o
	p q r               m n o
	s t u               p q r
	v w x               s t u
	                    v w x
                            3 4 5

ppL = 3        ppR = 2     ppV = 3         ppL = ppR + 1 && m < ppL
pL  = 2 4 3    pR  = 4 3   pV  = 3 4 3     pL[i != m] = pR
	a b c               a b c                ppV = ppL
	d e f               d e f                pV = pL, pV[m] = pL[m] + 1
	g h i     0 1 2     g h i
	j k l     3 4 5     j k l
             ,{2} 6 7 8 ==> 
	m n o     9 y z     m n o
	p q r               p q r
	s t u               s t u
	v w x               v w x

	                    0 1 2
			    3 4 5
			    6 7 8
			    9 y z


   ppL = ppR = m
   pL = pR
       ppV = ppL + 1
       pV  = 2, pL

   ppL = ppR && m < ppL 
   pL[i != m] = pR
       ppV = ppL
       pV[m] = pL[m] + pR[m]
	      pV[i != m] = pL

   ppL = ppR + 1 && m < ppL
   pL[i != m] = pR
       ppV = ppL
       pV = pL, pV[m] = pL[m] + 1
  
			    
     ***/
    int axis = 0;
    if (get_type (mo) == TYPE_COMPLEX) {
      node_complex_s *mn = node_complex (mo);
      gsl_complex mv = node_complex_value(mn);
      axis = (int)lrint (GSL_REAL (mv));
    }
    node_cpx_vector_s *ls = node_cpx_vector (la);
    node_cpx_vector_s *rs = node_cpx_vector (ra);
    int ppL = node_cpx_vector_rhorho (ls);
    int ppR = node_cpx_vector_rhorho (rs);
    int *pL = node_cpx_vector_rho (ls);
    int *pR = node_cpx_vector_rho (rs);

    if (ppL == ppR) {
      if (ppL == axis) {
	int i;
	for (i = 0; i < ppL; i++) {
	  if (pL[i] != pR[i]) break;
	}
	if (i == ppL) {
	  int ppV = ppL + 1;
	  int *pV = malloc (ppV * sizeof(int));
	  memcpy (pV, pL, ppL * sizeof(int));
	  pV[ppV - 1] = 2;
	  rc = create_complex_vector_node ();
	  node_cpx_vector_s *dest = node_cpx_vector (rc);
	  node_cpx_vector_rhorho (dest) = ppV;
	  node_cpx_vector_rho (dest) = pV;
	  node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	    node_cpx_vector_next (ls) + node_cpx_vector_next (rs);
	  node_cpx_vector_data (dest) =
	    malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
	  memcpy (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memcpy (&node_cpx_vector_data (dest)[node_cpx_vector_next (ls)],
		   node_cpx_vector_data (rs),
		   node_cpx_vector_next (rs) * sizeof(gsl_complex));
	}
	else {
	  // fixme dim mismatch
	}
      }
      else if (axis < ppL) {
      }
      else {
	// fixme out of range axis
      }
      
    }
#else
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
	memcpy (node_cpx_vector_data (dest),
		 node_cpx_vector_data (ls), lcols * sizeof(gsl_complex));
	memcpy (&node_cpx_vector_data (dest)[lcols],
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
	  memcpy (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls), lcols * sizeof(gsl_complex));
	  memcpy (&node_cpx_vector_data (dest)[lcols],
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
	  memcpy (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memcpy (&node_cpx_vector_data (dest)[lcols],
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
	  memcpy (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memcpy (&node_cpx_vector_data (dest)[node_cpx_vector_next (ls)],
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
	  memcpy (node_cpx_vector_data (dest),
		   node_cpx_vector_data (ls),
		   node_cpx_vector_next (ls) * sizeof(gsl_complex));
	  memcpy (&node_cpx_vector_data (dest)[node_cpx_vector_next (ls)],
		   node_cpx_vector_data (rs),
		   node_cpx_vector_next (rs) * sizeof(gsl_complex));
	}
	else {
	  // fixme -- dim mismatch
	}
      }
    }
#endif
  }
  else if (get_type (la) == TYPE_CPX_VECTOR &&
	   get_type (ra) == TYPE_COMPLEX ) {
    // ./clc -e '[4 5 6],7'
    node_cpx_vector_s *ls = node_cpx_vector (la);
    node_complex_s *rs = node_complex (ra);
#if 0
    int lrows = node_cpx_vector_rows (ls);
#endif
    if (node_cpx_vector_rhorho (ls) <= 1 /* lrows == 0 */) {
#if 0
      int lcols = node_cpx_vector_cols (ls);
#else
      int lcols = node_cpx_vector_next (ls);
#endif
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = lcols + 1;
#if 1
      if (node_cpx_vector_rhorho (ls) == 1)
	node_cpx_vector_rho (ls)[0] = lcols + 1;
#else
      node_cpx_vector_rows (dest) = 0;
      node_cpx_vector_cols (dest) = lcols + 1;
#endif
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
      memcpy (node_cpx_vector_data (dest),
	       node_cpx_vector_data (ls), lcols * sizeof(gsl_complex));
      node_cpx_vector_data (dest)[lcols] = node_complex_value(rs);
    }
    else {
      // fixme -- dim mismatch
    }
  }
  else if (get_type (la) == TYPE_COMPLEX &&
	   get_type (ra) == TYPE_CPX_VECTOR ) {	//
    // ./clc -e '7,[4 5 6]'
    node_complex_s *ls = node_complex (la);
    node_cpx_vector_s *rs = node_cpx_vector (ra);
#if 0
    int rrows = node_cpx_vector_rows (rs);
#endif
    if (node_cpx_vector_rhorho (rs) <= 1 /* rrows == 0*/) {
#if 0
      int rcols = node_cpx_vector_cols (rs);
#else
      int rcols = node_cpx_vector_next (rs);
#endif
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = rcols + 1;
#if 1
      if (node_cpx_vector_rhorho (rs) == 1)
	node_cpx_vector_rho (rs)[0] = rcols + 1;
#else
      node_cpx_vector_cols (dest) = rcols + 1;
      node_cpx_vector_rows (dest) = 0;
#endif
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
      node_cpx_vector_data (dest)[0] = node_complex_value(ls);
      memcpy (&node_cpx_vector_data (dest)[1],
	       node_cpx_vector_data (rs), rcols * sizeof(gsl_complex));
    }
    else {
      // fixme -- dim mismatch
    }
  }
  else if (get_type (la) == TYPE_COMPLEX &&
	   get_type (ra) == TYPE_COMPLEX ) {	//
    node_complex_s *ls = node_complex (la);
    gsl_complex lv = node_complex_value(ls);
    node_complex_s *rs = node_complex (ra);
    gsl_complex rv = node_complex_value(rs);
    rc = create_complex_vector_node ();
    node_cpx_vector_s *dest = node_cpx_vector (rc);
#if 1
    node_cpx_vector_rhorho (dest) = 1;
    node_cpx_vector_rho (dest) = malloc (sizeof(int));
    node_cpx_vector_rho (dest)[0] = 2;
#else
    node_cpx_vector_rows (dest) = 0;
    node_cpx_vector_cols (dest) = 2;
#endif
    node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = 2;
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
