#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <alloca.h>
#include <ctype.h>
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "node.h"
#include "memory.h"
#include "printext.h"
#include "eval.h"
#include "vector.h"
#include "plot.h"

#if 0
typedef gsl_complex (*cpx_dyadic)(gsl_complex a, gsl_complex b);
typedef gsl_complex (*cpx_monadic)(gsl_complex a);
typedef node_u (*clcx_dyadic)(node_u modifier, node_u la, node_u ra);
typedef node_u (*clc_dyadic)(gsl_complex *retp, node_u modifier,
			     node_u la, node_u ra);
typedef node_u (*clc_monadic)(node_u modifier, node_u arg);
#endif

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

void *
get_op_dyadic (sym_e sym)
{
  return op_dyadic (sym);
}

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)

static void *global_symtab = NULL;
static void *current_symtab = NULL;
static void **symtab_stack = NULL;
static int   symtab_stack_next = 0;
static int   symtab_stack_max  = 0;
#define SYMTAB_INCR 16

void
push_symtab ()
{
  if (symtab_stack_max <= symtab_stack_next) {
    symtab_stack_max += SYMTAB_INCR;
    symtab_stack = realloc (symtab_stack, symtab_stack_max * sizeof(void *));
  }
  symtab_stack[symtab_stack_next++] = current_symtab;
  current_symtab = NULL;
}

static void
free_symbol (void *ptr)
{
  symbol_entry_s *vp = (symbol_entry_s *)ptr;
  if (sym_lbl (vp)) free (sym_lbl (vp));
  free_node (sym_node (vp));
}

void *
get_current_symtab ()
{
  return current_symtab;
}

void
pop_symtab ()
{
  if (symtab_stack_next > 0) {
    if (current_symtab) tdestroy (current_symtab, free_symbol);
    current_symtab = symtab_stack[--symtab_stack_next];
  }
  else current_symtab = global_symtab;
}

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
      node_cpx_vector_rhorho (dest) = 1;
      node_cpx_vector_rho (dest) = malloc (sizeof(int));
      node_cpx_vector_rho (dest)[0] = ct;
      node_cpx_vector_next (dest) = node_cpx_vector_max (dest) = ct;
      node_cpx_vector_data (dest) =
	malloc (node_cpx_vector_max (dest) * sizeof(gsl_complex));
      for (int i =  0; i < ct; i++) {
	node_cpx_vector_data (dest)[i] = init;
	init = gsl_complex_add (init, incr);
      }
    }
  }
  else {
    // fixme
  }
  return rc;
}

node_u
clc_random (node_u modifier, node_u ra)
{
  node_u rc = NULL_NODE;
  node_u rn = do_eval (NULL, ra);
  node_u rm = do_eval (NULL, modifier);
  enum {RT_SEQ, RT_IND} rt = RT_IND;

  if (get_type (rm) == TYPE_LITERAL) {
    node_string_s *sm = node_string (rm);
    char *vm = node_string_value (sm);
    if (vm && (*vm == 's' || *vm == 'S')) rt = RT_SEQ;
  }

  switch(get_type (rn)) {
  case TYPE_COMPLEX:
    {
      node_complex_s *rn = node_complex (ra);
      gsl_complex scale = node_complex_value (rn);
      double rr = drand48 () * GSL_REAL (scale);
      double ri = drand48 () * GSL_IMAG (scale);
      gsl_complex rv = gsl_complex_rect (rr, ri);
      rc = create_complex_node (0, rv);
    }
    break;
  case TYPE_CPX_VECTOR:	// rands within given ranges or random sequence
    {
      node_cpx_vector_s *rs = node_cpx_vector (ra);
      int len = node_cpx_vector_next (rs);
      if (len > 0) {
	rc = create_complex_vector_node ();
	node_cpx_vector_s *vs = node_cpx_vector (rc);
	node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
	  node_cpx_vector_next (rs);
	node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (rs);
	node_cpx_vector_rho (vs) =
	  malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	memmove (node_cpx_vector_rho (vs),
		 node_cpx_vector_rho (rs),
		 node_cpx_vector_rhorho (vs) * sizeof(int));
	node_cpx_vector_data (vs) =
	  malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	if (rt == RT_SEQ) {			// randomise sequence
	  int *indices = alloca ((len + 1) * sizeof(int));
      
	  for (int i = 0; i < len; i++) indices[i] = i;
	  for (int i = 0; len > 0; len --, i++) {
	    long ix = (long) ((double)len * drand48 ());
	    node_cpx_vector_data (vs)[i] =
	      node_cpx_vector_data (rs)[indices[ix]];
	    if (len > 1)
	      memmove (&indices[ix], &indices[ix + 1], (len - 1) * sizeof(int));
	  }
	}
	else {					// create vec of randoms
	  for (int i = 0; i < len; i++) {
	    gsl_complex scale = node_cpx_vector_data (rs)[i];
	    double rr = drand48 () * GSL_REAL (scale);
	    double ri = drand48 () * GSL_IMAG (scale);
	    node_cpx_vector_data (vs)[i] = gsl_complex_rect (rr, ri);
	  }
	}
      }
    }
    break;
  case TYPE_LITERAL:	// random sequence
    {
      node_string_s *rm = node_string (ra);
      char *rs = node_string_value (rm);
      int len = strlen (rs);
      if (len > 0) {
	char *vs = alloca (1+len);
	if (rt == RT_SEQ) {			// randomise sequence
	  int *indices = alloca ((len + 1) * sizeof(int));
	
	  for (int i = 0; i < len; i++) indices[i] = i;
	  for (int i = 0; len > 0; len --, i++) {
	    long ix = (long) ((double)len * drand48 ());
	    vs[i] = rs[indices[ix]];
	    if (len > 1)
	      memmove (&indices[ix], &indices[ix + 1], (len - 1) * sizeof(int));
	  }
	  vs[len] = 0;
	  rc = create_string_node (TYPE_LITERAL, vs);
	}
	else {			// random string
	  for (int i = 0; i < len; i++) {
	    int chr = 0;
	    switch(rs[i]) {
	    case 'a':  // alpha
	      do {chr = (int)(drand48 () * 255.0);} while(!isalpha (chr));
	      break;
	    case 'A':  // alphanumeric
	      do {chr = (int)(drand48 () * 255.0);} while(!isalnum (chr));
	      break;
	    case 'i':  // ascii
	      do {chr = (int)(drand48 () * 255.0);} while(!isascii (chr));
	      break;
	    case 'd':  // digit
	      do {chr = (int)(drand48 () * 255.0);} while(!isdigit (chr));
	      break;
	    case 'g':  // graph
	      do {chr = (int)(drand48 () * 255.0);} while(!isgraph (chr));
	      break;
	    case 'l':  // lower
	      do {chr = (int)(drand48 () * 255.0);} while(!islower (chr));
	      break;
	    case 'p':  // print
	      do {chr = (int)(drand48 () * 255.0);} while(!isprint (chr));
	      break;
	    case '!':  // punct
	      do {chr = (int)(drand48 () * 255.0);} while(!ispunct (chr));
	      break;
	    case 'u':  // upper
	      do {chr = (int)(drand48 () * 255.0);} while(!isupper (chr));
	      break;
	    }
	    vs[i] = chr;
	  }
	  vs[len] = 0;
	  rc = create_string_node (TYPE_LITERAL, vs);
	}
      }
    }
    break;
  default:
    break;
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

node_u
clc_file (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;
  if (get_type (arg) == TYPE_LITERAL) {
  }
  return rc;
}

typedef double (*compare_op)(double x, double y);

static inline double do_lt (double x, double y) {return (x <  y) ? 1.0 : 0.0;}
static inline double do_le (double x, double y) {return (x <= y) ? 1.0 : 0.0;}
static inline double do_gt (double x, double y) {return (x >  y) ? 1.0 : 0.0;}
static inline double do_ge (double x, double y) {return (x >= y) ? 1.0 : 0.0;}
static inline double do_eq (double x, double y) {return (x == y) ? 1.0 : 0.0;}
static inline double do_ne (double x, double y) {return (x != y) ? 1.0 : 0.0;}

static node_u
clc_compare (gsl_complex *retp, compare_op cpr,
	     node_u modifier, node_u la, node_u ra)
{
  /***
      modifier mag, pha, real, imag
   ***/
  node_u rc = NULL_NODE;
  node_u ln = do_eval (NULL, la);
  node_u rn = do_eval (NULL, ra);
  node_u mn = do_eval (NULL, modifier);
  
  if (get_type (ln) == TYPE_COMPLEX &&
      get_type (rn) == TYPE_COMPLEX) {

    typedef enum {MOD_MAG, MOD_PHASE, MOD_REAL, MOD_IMAG, MOD_CPX} mod_mode_e;
    mod_mode_e mod_mode = MOD_CPX;
    
    if (get_type (mn) == TYPE_SYMBOL) {
      node_string_s *mv = node_string (mn);
      char *mx = node_string_value (mv);
      switch(*mx) {
      case 'm':
      case 'M':
	mod_mode = MOD_MAG;
	break;
      case 'p':
      case 'P':
	mod_mode = MOD_PHASE;
	break;
      case 'r':
      case 'R':
	mod_mode = MOD_REAL;
	break;
      case 'i':
      case 'I':
	mod_mode = MOD_IMAG;
	break;
      case 'c':
      case 'C':
	mod_mode = MOD_CPX;
	break;
      }
    }

    
    node_complex_s *lv = node_complex (ln);
    node_complex_s *rv = node_complex (rn);
    gsl_complex lx = node_complex_value (lv);
    gsl_complex rx = node_complex_value (rv);
    if (GSL_IMAG (lx) == 0.0 && GSL_IMAG (rx) == 0.0) mod_mode = MOD_MAG;
    double lm = 0.0, rm = 0.0, ry = 0.0, iy = 0.0;
    switch(mod_mode) {
    case MOD_MAG:
      lm = gsl_complex_abs (lx);
      rm = gsl_complex_abs (rx);
      ry = (*cpr)(lm, rm);
      iy = 0.0;
      break;
    case MOD_PHASE:
      lm = gsl_complex_arg (lx);
      rm = gsl_complex_arg (rx);
      ry = (*cpr)(lm, rm);
      iy = 0.0;
      break;
    case MOD_REAL:
      lm = GSL_REAL (lx);
      rm = GSL_REAL (rx);
      ry = (*cpr)(lm, rm);
      iy = 0.0;
      break;
    case MOD_IMAG:
      lm = GSL_IMAG (lx);
      rm = GSL_IMAG (rx);
      ry = (*cpr)(lm, rm);
      iy = 0.0;
      break;
    case MOD_CPX:
      lm = GSL_REAL (lx);
      rm = GSL_REAL (rx);
      ry = (*cpr)(lm, rm);
      lm = GSL_IMAG (lx);
      rm = GSL_IMAG (rx);
      iy = (*cpr)(lm, rm);
      break;
    }
    gsl_complex rr = gsl_complex_rect (ry, iy);
    if (retp) *retp = rr;
    else rc = create_complex_node (0, rr);
  }
  return rc;
}

node_u
clc_lt (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_lt, modifier, la, ra);
}

node_u
clc_le (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_le, modifier, la, ra);
}

node_u
clc_gt (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_gt, modifier, la, ra);
}

node_u
clc_ge (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_ge, modifier, la, ra);
}

node_u
clc_eq (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_eq, modifier, la, ra);
}

node_u
clc_ne (gsl_complex *retp, node_u modifier, node_u la, node_u ra)
{
  return clc_compare (retp, do_ne, modifier, la, ra);
}

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

void
do_assign (const char *name, node_u ra)
{
  symbol_entry_s *sa = malloc (sizeof(symbol_entry_s));
  sym_lbl (sa) = strdup (name);
  sym_node (sa) = ra;
  node_incref (ra);
  void *found = tfind (sa, &current_symtab, var_compare);
  if (found) {
    symbol_entry_s *vp = *(symbol_entry_s **)found;
    node_decref (sym_node (vp));
    free (sym_lbl (sa));
    sym_node (vp) = ra;
    node_incref (ra);
  }
  else tsearch (sa, &current_symtab, var_compare);
}

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
      do_assign (node_string_value (lv), ra);
      rc = NULL_NODE;	// fixme this migh not be the right thing to do
#if 0
      printf ("\nvars\n");
      twalk (current_symtab, var_action);
      printf ("end of vars\n\n");
#endif
    }
    break;
  case TYPE_LITERAL:	// quoted string
  case TYPE_COMPLEX:
  case TYPE_LIST:
  case TYPE_FUNCTION:
  case TYPE_CPX_VECTOR:
  case TYPE_DYADIC:
  case TYPE_MONADIC:
  case TYPE_COMPOSITE:
  case TYPE_CALL:
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
    rc = create_complex_node (0, rr);
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
    rc = create_complex_node (0, rr);
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
    rc = create_complex_node (0, rr);
  }
  return rc;
}

static node_u
lookup_symbol (char *ls)
{
  node_u rc = NULL_NODE;
  symbol_entry_s sa;
  sym_lbl (&sa) = ls;
  void *found = NULL;
  if (symtab_stack_next > 0) {
    for (int i = symtab_stack_next; !found && i > 0; i--) {
      void *this_symtab = symtab_stack[i-1];
      found = tfind (&sa, &this_symtab, var_compare);
    }
  }
  if (!found) found = tfind (&sa, &current_symtab, var_compare);
  if (found) {
#if 0
    rc = *found;
#else
    symbol_entry_s *vp = *(symbol_entry_s **)found;
    rc = sym_node (vp);
#endif
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
    rc = create_complex_node (0, rr);
  }
  return rc;
}

static void
append_to_list (node_list_s **list_p, node_u node)
{
  node_list_s *list = *list_p;
  if (!list) {
    list = malloc (sizeof(node_list_s));
    node_list_type (list) = TYPE_LIST;
    node_function_refcnt (list) = 0;
    node_list_max (list) = NODE_LIST_INCR;
    node_list_next (list) = 0;
    node_list_list (list) = calloc (node_list_max (list), sizeof(node_u));
    *list_p = list;
  }
  if (node_list_max (list) <= node_list_next (list)) {
    node_list_max (list) += NODE_LIST_INCR;
    node_list_list (list) =
      realloc (node_list_list (list),
	       node_list_max (list) * sizeof(node_u));
  }	
  node_list_list (list)[node_list_next (list)++] = node;
}

static void
flatten (node_list_s **list_p, node_u node)
{
  if (!list_p) return;
  switch(get_type (node)) {
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      sym_e sym = node_dyadic_op (dyad);
      if (sym != SYM_COMMA) append_to_list (list_p, node);
      else {
	flatten (list_p, node_dyadic_la (dyad));
	flatten (list_p, node_dyadic_ra (dyad));
      }
    }
    break;
  case TYPE_MONADIC:
    {
      node_monadic_s *monad = node_monadic (node);
      flatten (list_p, node_monadic_arg (monad));
    }
    break;
  default:
    append_to_list (list_p, node);
    break;
  }
}

node_u
do_composite (int *noshow, node_u node)
{
  /***
      https://en.wikipedia.org/wiki/Tensor_contraction

      
      a +\* b		inner
      a  \* b		outer
         \* b		scanner
   ***/
  node_u rc = NULL_NODE;
  node_composite_s *comp = node_composite (node);
  node_u la = do_eval (NULL, node_composite_la (comp));
  node_u ra = do_eval (NULL, node_composite_ra (comp));
  node_u mo = do_eval (NULL, node_composite_modifier (comp));

  if (get_type (ra) == TYPE_CPX_VECTOR) {
    sym_e lsym = node_composite_left_op (comp);
    sym_e rsym = node_composite_right_op (comp);
    if (rsym != SYM_NULL) {
      if (lsym == SYM_NULL) {				// outer or scanner
	if (get_type (la) == TYPE_CPX_VECTOR) {		// outer
	  rc = do_outer (rsym, la, ra, mo);
	}
	else {
	  // fixme bad la
	}
      }
      else {						// inner op
	if (get_type (la) == TYPE_CPX_VECTOR) {
	  rc = do_inner (lsym, rsym, la, ra, mo);
	}
	else {
	  // fixme missing arg
	}
      }
    }
    {
      // fixme -- bad rsym
    }
  }
  else {
    // fixme bad ra
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
  case TYPE_COMPOSITE:
    rc = do_composite (noshow, node);
    break;
  case TYPE_CALL:
    {
      node_call_s *call = node_call (node);
      node_list_s *arg_list = NULL;
      flatten (&arg_list, node_call_args (call));
      char *fcn_name  = node_call_fcn (call);
      node_u fcn_node = lookup_symbol (fcn_name);
      if (get_type (fcn_node) == TYPE_FUNCTION) {
	node_function_s *node = node_function (fcn_node);
	node_u body = node_function_body (node);
	node_list_s *param_list = NULL;
	flatten (&param_list, node_function_params (node));
	push_symtab ();
	for (int i = 0; i < node_list_next (param_list); i++) {
	  do_eval (NULL, node_list_list (param_list)[i]);
#if 0
	  printf ("param node %d\n", i);
	  print_node (0, node_list_list (param_list)[i]);
#endif
	}
	for (int i = 0; i < node_list_next (arg_list); i++) {
	  do_eval (NULL, node_list_list (arg_list)[i]);
#if 0
	  printf ("arg node %d\n", i);
	  print_node (0, node_list_list (arg_list)[i]);
#endif
	}
	rc = do_eval (NULL, body);
	pop_symtab ();
      }
#if 0
      push_symtab ();
      do_eval (NULL, params);		// fixme -- doesn't do positionals
      rc = do_eval (NULL, body);
      pop_symtab ();
#endif
    }
    break;
  case TYPE_SYMBOL:
    {
      symbol_entry_s sa;
      node_string_s *lv = node_string (node);
      sym_lbl (&sa) = node_string_value (lv);
      node_u val = lookup_symbol (node_string_value (lv));
      if (get_type (val) != TYPE_NULL && get_type (val) != TYPE_CALL) {
	rc = val;
      }
      else rc = node;
#if 0
      lookup_symbol (char *ls)
      void *found = NULL;
      if (symtab_stack_next > 0) {
	for (int i = symtab_stack_next; !found && i > 0; i--) {
	  void *this_symtab = symtab_stack[i-1];
	  found = tfind (&sa, &this_symtab, var_compare);
	}
      }
      if (!found) found = tfind (&sa, &current_symtab, var_compare);
      if (found) {
	symbol_entry_s *vp = *(symbol_entry_s **)found;
	rc = sym_node (vp);
      }
      else rc = node;
#endif
    }
    break;
  case TYPE_COMPLEX:
  case TYPE_LITERAL:
  case TYPE_LIST:
  case TYPE_FUNCTION:
  case TYPE_CPX_VECTOR:
    rc = node;
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      sym_e sym = node_dyadic_op (dyad);
      node_u ra = node_dyadic_ra (dyad);
      if (sym != SYM_COMMA) ra = do_eval (NULL, ra);
      node_u la = node_dyadic_la (dyad);
      /***
	  (sym != SYM_EQUAL || get_type (la) != TYPE_SYMBOL) ==
	  !(sym == SYM_EQUAL  && get_type (la) == TYPE_SYMBOL
       ***/
      if (!(sym == SYM_EQUAL  && get_type (la) == TYPE_SYMBOL) &&
	  sym != SYM_COMMA)
	  la = do_eval (NULL, la);
      node_u modifier = do_eval (NULL, node_dyadic_modifier (dyad));
      op_type_e op_type = node_dyadic_op_type (dyad);


      if (op_type == OP_TYPE_CLC) {
	if (noshow && sym == SYM_EQUAL) *noshow = 1;
	clc_dyadic op = op_dyadic (sym);

	if (sym == SYM_EQUAL)
	  return clc_assign (modifier, la, ra);

	if (sym == SYM_POUND || sym == SYM_COMMA ||
	    sym == SYM_COLCOL) {
	  clcx_dyadic op = op_dyadic (sym);
	  rc = (*op)(modifier, la, ra);
	  return rc;
	}
	if (op) {
	  switch(TYPE_GEN (get_type (la), get_type (ra))) {
	  case TYPE_GEN (TYPE_COMPLEX, TYPE_CPX_VECTOR):
	    {
	      node_complex_s node;
	      node_refcnt (&node) = 0;
	      node_complex_type (&node) = TYPE_COMPLEX;
	      node_cpx_vector_s *rs = node_cpx_vector (ra);
	      rc = create_complex_vector_node ();
	      node_cpx_vector_s *vs = node_cpx_vector (rc);
	      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		node_cpx_vector_next (rs);
	      node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (rs);
	      node_cpx_vector_rho (vs) =
		malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	      memmove (node_cpx_vector_rho (vs),
		       node_cpx_vector_rho (rs),
		       node_cpx_vector_rhorho (vs) * sizeof(int));
	      node_cpx_vector_data (vs) =
		malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	      for (int i = 0; i < node_cpx_vector_next (rs); i++) {
		node_complex_value (&node) = node_cpx_vector_data (rs)[i];
		gsl_complex vvv;
	        (*op)(&vvv, modifier, la, (node_u)&node);
		node_cpx_vector_data (vs)[i] = vvv;
	      }
	    }
	    break;
	  case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_COMPLEX):
	    {
	      node_complex_s node;
	      node_refcnt (&node) = 0;
	      node_complex_type (&node) = TYPE_COMPLEX;
	      node_cpx_vector_s *ls = node_cpx_vector (la);
	      rc = create_complex_vector_node ();
	      node_cpx_vector_s *vs = node_cpx_vector (rc);
	      node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		node_cpx_vector_next (ls);;
	      node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (ls);
	      node_cpx_vector_rho (vs) =
		malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	      memmove (node_cpx_vector_rho (vs),
		       node_cpx_vector_rho (ls),
		       node_cpx_vector_rhorho (vs) * sizeof(int));
	      node_cpx_vector_data (vs) =
		malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
	      for (int i = 0; i < node_cpx_vector_next (ls); i++) {
		node_complex_value (&node) = node_cpx_vector_data (ls)[i];
		gsl_complex vvv;
	        (*op)(&vvv, modifier, (node_u)&node, ra);
		node_cpx_vector_data (vs)[i] = vvv;
	      }
	    }
	    break;
	  case TYPE_GEN (TYPE_CPX_VECTOR, TYPE_CPX_VECTOR):
	    {
	      node_complex_s nodel;
	      node_complex_s noder;
	      node_refcnt (&nodel) = 0;
	      node_refcnt (&noder) = 0;
	      node_complex_type (&nodel) = TYPE_COMPLEX;
	      node_complex_type (&noder) = TYPE_COMPLEX;
	      node_cpx_vector_s *ls = node_cpx_vector (la);
	      node_cpx_vector_s *rs = node_cpx_vector (ra);
	      if (node_cpx_vector_next (ls) ==
		  node_cpx_vector_next (rs)) {
		rc = create_complex_vector_node ();
		node_cpx_vector_s *vs = node_cpx_vector (rc);
		node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		  node_cpx_vector_next (ls);
		node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (ls);
		node_cpx_vector_rho (vs) =
		  malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
		memmove (node_cpx_vector_rho (vs),
			 node_cpx_vector_rho (ls),
			 node_cpx_vector_rhorho (vs) * sizeof(int));
		node_cpx_vector_data (vs) =
		  malloc (node_cpx_vector_max (vs) * sizeof(gsl_complex));
		for (int i = 0; i < node_cpx_vector_next (ls); i++) {
		  node_complex_value (&nodel) = node_cpx_vector_data (ls)[i];
		  node_complex_value (&noder) = node_cpx_vector_data (rs)[i];
		  gsl_complex vvv;
		  (*op)(&vvv, modifier, (node_u)&nodel, (node_u)&noder);
		  node_cpx_vector_data (vs)[i] = vvv;
		}
	      }
	      else {
		// fixme err dim mismatch. also check row/col mismatch
	      }
	    }
	    break;
	  case TYPE_GEN (TYPE_COMPLEX, TYPE_COMPLEX):
	    rc = (*op)(NULL, modifier, la, ra);
	    break;
	  }
	}
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
	      node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (rs);
	      node_cpx_vector_rho (vs) =
		malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	      memmove (node_cpx_vector_rho (vs),
		       node_cpx_vector_rho (rs),
		       node_cpx_vector_rhorho (vs) * sizeof(int));
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
	      node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (ls);
	      node_cpx_vector_rho (vs) =
		malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	      memmove (node_cpx_vector_rho (vs),
		       node_cpx_vector_rho (ls),
		       node_cpx_vector_rhorho (vs) * sizeof(int));
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
		  (node_cpx_vector_rhorho (ls) ==
		   node_cpx_vector_rhorho (rs))
		  ) {
		int p;
		for (p = 0; p < node_cpx_vector_rhorho (rs); p++) {
		  if (node_cpx_vector_rho (ls)[p] !=
		      node_cpx_vector_rho (rs)[p]) break;
		}
		if (p == node_cpx_vector_rhorho (rs)) {
		  rc = create_complex_vector_node ();
		  node_cpx_vector_s *vs = node_cpx_vector (rc);
		  node_cpx_vector_next (vs) = node_cpx_vector_max (vs) =
		    node_cpx_vector_next (ls);
		  node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (ls);
		  node_cpx_vector_rho (vs) =
		    malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
		  memmove (node_cpx_vector_rho (vs),
			   node_cpx_vector_rho (ls),
			   node_cpx_vector_rhorho (vs) * sizeof(int));
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
	      rc = create_complex_node (0, vv);
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

      if (op_type == OP_TYPE_CLC) {
	clc_monadic op = op_monadic (sym);
	if (op) rc = (*op)(node_monadic_modifier (monad), arg);
	return rc;
      }
      
      //      node_u modifier = do_eval (NULL, node_monadic_modifier (monad));

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
	      node_cpx_vector_rhorho (vs) = node_cpx_vector_rhorho (ls);
	      node_cpx_vector_rho (vs) =
		malloc (node_cpx_vector_rhorho (vs) * sizeof(int));
	      memmove (node_cpx_vector_rho (vs),
		       node_cpx_vector_rho (ls),
		       node_cpx_vector_rhorho (vs) * sizeof(int));
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
	      rc = create_complex_node (0, vv);
	    }
	    else rc = create_complex_node (0, lv);
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
		    rc = create_complex_node (0, vv);
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


static void
print_indices (int rhorho, int *ix)
{
  int i;
  printf ("\n[");
  for (i = rhorho - 1; i >= 2; i--) printf ("%d ", ix[i]);
  printf (" * *]\n");
}

#undef ENTRY
#define ENTRY(l,s,d,m,dc,mc,dd,md) #s
static char *sym_names[] = {
#include "opsdata.h"
};

void
print_node (int indent, node_u node)
{
  switch(get_type (node)) {
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *vs = node_cpx_vector (node);
      int rhorho = node_cpx_vector_rhorho (vs);
      int *rhop = node_cpx_vector_rho (vs);
      gsl_complex *datap = node_cpx_vector_data (vs);
      int ct = node_cpx_vector_next (vs);
      int *ix = alloca (rhorho * sizeof(int));
      bzero (ix, rhorho * sizeof(int));
      int i, k;
      for (i = 0; i < ct; i++) {
	if (rhorho > 2 && ix[0] == 0 && ix[1] == 0)
	  print_indices (rhorho, ix);
	printf ("%R ", datap[i]);
	if (ix[0] == rhop[0] - 1)	printf ("\n");
	int carry = 1;
	for (k = 0; carry && k < rhorho; k++) {
	  if (carry) {
	    if (++ix[k] >= rhop[k]) ix[k] = 0;
	    else {
	      carry = 0;
	      break;
	    }
	  }
	}
      }
    }
    break;
  case TYPE_COMPLEX:
    {
      node_complex_s *vs = node_complex (node);
      gsl_complex vv = node_complex_value (vs);
      fprintf (stdout, "%*s%R\n", indent, " ", GSL_REAL (vv), GSL_IMAG (vv));
    }
    break;
  case TYPE_LITERAL:
    {
      node_string_s *ss = node_string (node);
      char *sv = node_string_value (ss);
      fprintf (stdout, "%*s%s\n", indent, " ", sv);
    }
    break;
  case TYPE_NULL:
    //fprintf (stdout, "%*s''\n", indent, " ");
    break;
  case TYPE_LIST:
    {
      node_list_s *list = node_list (node);
      fprintf (stdout, "%*slist %d elements\n", indent, " ",
	       node_list_next (list));
      for (int i = 0; i < node_list_next (list); i++) 
	print_node (indent+2, node_list_list (list)[i]);
    }
    break;
  case TYPE_DYADIC:
    {
      node_dyadic_s *dyad = node_dyadic (node);
      sym_e sym = node_dyadic_op (dyad);
      fprintf (stdout, "%*sdyadic sym = %s (%d)\n",
	       indent, " ", sym_names[sym]);
      fprintf (stdout, "%*sleft arg:\n", indent, " ");
      print_node (indent+2, node_dyadic_la (dyad));
      fprintf (stdout, "%*sright arg:\n", indent, " ");
      print_node (indent+2, node_dyadic_ra (dyad));
    }
    break;
  case TYPE_SYMBOL:
    {
      node_string_s *lv = node_string (node);
      fprintf (stdout, "%*ssymbol %s\n", indent, " ", node_string_value (lv));
    }
    break;
  case TYPE_MONADIC:
  case TYPE_COMPOSITE:
  case TYPE_FUNCTION:
  case TYPE_CALL:
    // fixme
    break;
  }
}

