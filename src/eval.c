#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "node.h"
#include "memory.h"
#include "printext.h"
#include "eval.h"
#include "vector.h"
#include "plot.h"

typedef gsl_complex (*cpx_dyadic)(gsl_complex a, gsl_complex b);
typedef gsl_complex (*cpx_monadic)(gsl_complex a);
typedef node_u (*clc_dyadic)(node_u modifier, node_u la, node_u ra);
typedef node_u (*clc_monadic)(node_u modifier, node_u arg);

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
#define NULL_NODE (node_u)(&null_node)

static void *global_root = NULL;

static node_u
do_range (node_u modifier, node_u la, node_u ra)
{
  node_u rc = NULL_NODE;
  node_u rn = do_eval (NULL, ra);
  
  if (get_type (rn) == TYPE_COMPLEX) {
    gsl_complex incr = gsl_complex_rect (1.0, 0.0);
    gsl_complex init = gsl_complex_rect (0.0, 0.0);
    
    node_u mod = do_eval (NULL, modifier);
    if (get_type (mod) == TYPE_COMPLEX) {
      node_complex_s *mn = node_complex (mod);
      incr = node_complex_value (mn);
    }

    node_u ln = do_eval (NULL, la);
    if (get_type (ln) == TYPE_COMPLEX) {
      node_complex_s *ls = node_complex (ln);
      init = node_complex_value (ls);
    }

    node_complex_s *rs = node_complex (rn);
    gsl_complex rv = node_complex_value (rs);
    int ct = lrint (GSL_REAL (rv));
    if (ct < 0) {
      ct = -ct;
      incr = gsl_complex_negative (incr);
    }
    if (ct > 0) {
      rc = create_complex_vector_node ();
      node_cpx_vector_s *dest = node_cpx_vector (rc);
      node_cpx_vector_rows (dest) = 0;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) =
	node_cpx_vector_cols (dest) = ct;
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
      for (int i =  0; i < ct; i++) {
	node_cpx_vector_data (dest)[i] = init;
	init = gsl_complex_add (init, incr);
      }
    }
  }
  return rc;
}

node_u
clc_index (node_u modifier, node_u ra)
{
  return do_range (modifier, NULL_NODE, ra);
}

node_u
clc_range (node_u modifier, node_u la, node_u ra)
{
  return do_range (modifier, la, ra);
}

typedef struct {
  char *lbl;
  node_u      node;
} symbol_entry_s;
#define sym_lbl(e)  ((e)->lbl)
#define sym_node(e) ((e)->node)

static int
var_compare (const void *a, const void *b)
{
  const symbol_entry_s *sa = a;
  const symbol_entry_s *sb = b;
  int rc = strcmp (sym_lbl (sa), sym_lbl (sb));
  return rc;
}

#if 0
static void
var_action (const void *nodep, const VISIT which, const int depth)
{
  //  int *datap;
  symbol_entry_s *vp;

  switch (which) {
  case preorder:
    break;
  case postorder:
    vp = *(symbol_entry_s **)nodep;
    printf("post \"%s\"\n", sym_lbl (vp));
    break;
  case endorder:
    break;
  case leaf:
    vp = *(symbol_entry_s **)nodep;
    printf("leaf \"%s\"\n", sym_lbl (vp));
    break;
  }
}
#endif

node_u
clc_assign (node_u modifier, node_u la, node_u ra)
{
  node_u rc =  ra;
  
  switch(get_type (la)) {
  case TYPE_NULL:
    break;
  case TYPE_SYMBOL:	// unquoted string
    {
      node_string_s *lv = node_string (la);
      symbol_entry_s *sa = malloc (sizeof(symbol_entry_s));
      sym_lbl (sa) = strdup (node_string_value (lv));
      sym_node (sa) = ra;
      node_incref (ra);
      void *found = tfind (sa, &global_root, var_compare);
      if (found) {
	symbol_entry_s *vp = *(symbol_entry_s **)found;
	node_decref (sym_node (vp));
	free (sym_lbl (sa));
	sym_node (vp) = ra;
	node_incref (ra);
      }
      else tsearch (sa, &global_root, var_compare);
      rc = NULL_NODE;	// fixme this migh not be the right thing to do
#if 0
      printf ("\nvars\n");
      twalk(global_root, var_action);
      printf ("end of vars\n\n");
#endif
    }
    break;
  case TYPE_LITERAL:	// quoted string
  case TYPE_COMPLEX:
  case TYPE_LIST:
  case TYPE_CPX_VECTOR:
  case TYPE_DYADIC:
  case TYPE_MONADIC:
    // err unassignable
    break;
  }
  return rc;
}

node_u
clc_complex_real (node_u modifier, node_u iarg)
{
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, iarg);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *la = node_complex (arg);
    gsl_complex aa = node_complex_value (la);
    double vv = GSL_REAL (aa);
    gsl_complex rr = gsl_complex_rect (vv, 0.0);
    rc = create_complex_node (rr);
  }
  return rc;
}

node_u
clc_complex_imag (node_u modifier, node_u iarg)
{
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, iarg);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *la = node_complex (arg);
    gsl_complex aa = node_complex_value (la);
    double vv = GSL_IMAG (aa);
    gsl_complex rr = gsl_complex_rect (vv, 0.0);
    rc = create_complex_node (rr);
  }
  return rc;
}

node_u
clc_complex_abs (node_u modifier, node_u iarg)
{
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, iarg);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *la = node_complex (arg);
    gsl_complex aa = node_complex_value (la);
    double vv = gsl_complex_abs (aa);
    gsl_complex rr = gsl_complex_rect (vv, 0.0);
    rc = create_complex_node (rr);
  }
  return rc;
}

node_u
clc_complex_arg (node_u modifier, node_u iarg)
{
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, iarg);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *la = node_complex (arg);
    gsl_complex aa = node_complex_value (la);
    double vv = gsl_complex_arg (aa);
    gsl_complex rr = gsl_complex_rect (vv, 0.0);
    rc = create_complex_node (rr);
  }
  return rc;
}

node_u
do_eval (int *noshow, node_u node)
{
  if (noshow) *noshow = 0;
  node_u rc = NULL_NODE;
  switch(get_type (node)) {
  case TYPE_NULL:
    break;
  case TYPE_SYMBOL:
    {
      symbol_entry_s sa;
      node_string_s *lv = node_string (node);
      sym_lbl (&sa) = node_string_value (lv);
      void *found = tfind (&sa, &global_root, var_compare);
      if (found) {
	symbol_entry_s *vp = *(symbol_entry_s **)found;
	rc = sym_node (vp);
      }
      else rc = node;
    }
    break;
  case TYPE_COMPLEX:
  case TYPE_LITERAL:
  case TYPE_LIST:
  case TYPE_CPX_VECTOR:
    rc = node;
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      node_u ra = do_eval (NULL, node_dyadic_ra (dyad));
      sym_e sym = node_dyadic_op (dyad);
      node_u la = node_dyadic_la (dyad);
      if (sym != SYM_EQUAL || get_type (la) != TYPE_SYMBOL)
	la = do_eval (NULL, la);
      node_u modifier = node_dyadic_modifier (dyad);
      op_type_e op_type = node_dyadic_op_type (dyad);

      if (op_type == OP_TYPE_CLC) {
	if (noshow && sym == SYM_EQUAL) *noshow = 1;
	clc_dyadic op = op_dyadic (sym);
	if (op) rc = (*op)(modifier, la, ra);
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
	      node_cpx_vector_rows (vs) = node_cpx_vector_rows (rs);
	      node_cpx_vector_cols (vs) = node_cpx_vector_cols (rs);
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
	      node_cpx_vector_rows (vs) = node_cpx_vector_rows (ls);
	      node_cpx_vector_cols (vs) = node_cpx_vector_cols (ls);
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
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_dyadic op = op_dyadic (sym);
	    if (op) {
	      node_cpx_vector_s *rs = node_cpx_vector (ra);
	      node_cpx_vector_s *ls = node_cpx_vector (la);
	      if ((node_cpx_vector_next (ls) ==
		   node_cpx_vector_next (rs)) &&
		  (node_cpx_vector_rows (ls) ==
		   node_cpx_vector_rows (rs)) &&
		  (node_cpx_vector_cols (ls) ==
		   node_cpx_vector_cols (rs))) {
		rc = create_complex_vector_node ();
		node_cpx_vector_s *vs = node_cpx_vector (rc);
		node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		  node_cpx_vector_next (ls);
		node_cpx_vector_rows (vs) = node_cpx_vector_rows (ls);
		node_cpx_vector_cols (vs) = node_cpx_vector_cols (ls);
		node_cpx_vector_data (vs) =
		  malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
		for (int i = 0; i < node_cpx_vector_next (ls); i++) {
		  gsl_complex rv = node_cpx_vector_data (rs)[i];
		  gsl_complex lv = node_cpx_vector_data (ls)[i];
		  gsl_complex vv = (*op)(lv, rv);
		  node_cpx_vector_data (vs)[i] = vv;
		}
	      }
	      else {
		// fixme dim mismatch
	      }
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
      node_u arg = do_eval (NULL, node_monadic_arg (monad));
      sym_e sym = node_monadic_op (monad);
      op_type_e op_type = node_monadic_op_type (monad);
      node_u modifier = node_monadic_modifier (monad);

      if (op_type == OP_TYPE_CLC) {
	clc_monadic op = op_monadic (sym);
	if (op) rc = (*op)(modifier, arg);
	return rc;
      }

      switch(get_type (arg)) {
      case TYPE_CPX_VECTOR:
	{
	  if (sym >= 0 && sym < SYM_COUNT) {
	    cpx_monadic op = op_monadic (sym);
	    if (op) {
	      rc = create_complex_vector_node ();
	      node_cpx_vector_s *ls = node_cpx_vector (arg);
	      node_cpx_vector_s *vs = node_cpx_vector (rc);
	      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		node_cpx_vector_next (ls);
	      node_cpx_vector_rows (vs) = node_cpx_vector_rows (ls);
	      node_cpx_vector_cols (vs) = node_cpx_vector_cols (ls);
	      node_cpx_vector_data (vs) =
		malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	      for (int i = 0; i < node_cpx_vector_next (ls); i++) {
		gsl_complex lv = node_cpx_vector_data (ls)[i];
		gsl_complex vv = (*op)(lv);
		node_cpx_vector_data (vs)[i] = vv;
	      }
	    }
	  } 
	}
	break;
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
		  node_u a0 = do_eval (NULL, node_list_list (list)[0]);
		  node_u a1 = do_eval (NULL, node_list_list (list)[1]);
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
      int cols = node_cpx_vector_cols (vs);
      int i;
      fprintf (stdout, "[ ");
      for (i = 0; i < node_cpx_vector_next (vs); i++) {
	fprintf (stdout, "%R ", &node_cpx_vector_data (vs)[i]);
	if (((i + 1) % cols == 0) &&
	    ((i + 1) <  node_cpx_vector_next (vs)))
	  fprintf (stdout, "\n  ");
      }
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
  case TYPE_SYMBOL:
  case TYPE_DYADIC:
  case TYPE_MONADIC:
    break;
  }
}

#if 0
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
#endif
