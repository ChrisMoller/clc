#ifndef NODE_H
#define NODE_H

#include <gsl/gsl_matrix_complex_double.h>

#undef ENTRY
#define ENTRY(l,s,d,m,dc,mc,dd,md) s
typedef enum {
#include "opsdata.h"
} sym_e;

struct _node_type_s;
struct _node_string_s;
struct _node_complex_s;
struct _node_dyadic_s;
struct _node_mondic_s;
struct _node_composite_s;
struct _node_list_s;
struct _node_function_s;
struct _node_call_s;
struct _node_cpx_vector_s;

typedef struct _node_type_type		node_type_s;
typedef struct _node_string_type	node_string_s;
typedef struct _node_complex_type	node_complex_s;
typedef struct _node_dyadic_type	node_dyadic_s;
typedef struct _node_monadic_type	node_monadic_s;
typedef struct _node_composite_type	node_composite_s;
typedef struct _node_list_type		node_list_s;
typedef struct _node_function_type	node_function_s;
typedef struct _node_call_type		node_call_s;
typedef struct _node_cpx_vector_type	node_cpx_vector_s;


typedef union _node_u {
  node_type_s		*t;
  node_complex_s	*c;
  node_string_s		*s;
  node_dyadic_s		*d;
  node_monadic_s	*m;
  node_composite_s	*x;
  node_list_s		*l;
  node_function_s	*f;
  node_call_s		*e;
  node_cpx_vector_s	*v;
  void	                *vp;
  const void	        *vcp;
} node_u;
#define node_type(n)		((n).t)
#define node_complex(n)		((n).c)
#define node_string(n)		((n).s)
#define node_dyadic(n)		((n).d)
#define node_monadic(n)		((n).m)
#define node_composite(n)	((n).x)
#define node_list(n)		((n).l)
#define node_function(n)	((n).f)
#define node_call(n)		((n).e)
#define node_cpx_vector(n)	((n).v)
#define node_void(n)		((n).vp)

typedef enum {
  TYPE_NULL,
  TYPE_COMPLEX,
  TYPE_LITERAL,
  TYPE_SYMBOL,
  TYPE_DYADIC,
  TYPE_MONADIC,
  TYPE_COMPOSITE,
  TYPE_LIST,
  TYPE_FUNCTION,
  TYPE_CALL,
  TYPE_CPX_VECTOR,
} type_e;

#define TYPE_GEN(l,r)  ((100 * (l)) + (r))

struct _node_type_type {
  type_e	type;
  int		refcnt;
};
#define node_type_type(n)	((n)->type)
#define node_type_refcnt(n)	((n)->refcnt)
#define get_type(n) (node_type_type (node_type (n)))
#define node_refcnt(n) (node_type_refcnt (node_type ((node_u)n)))

struct _node_complex_type {
  type_e	type;
  int		refcnt;
  gsl_complex	value;
};
#define node_complex_type(n)	((n)->type)
#define node_complex_refcnt(n)	((n)->refcnt)
#define node_complex_value(n)	((n)->value)

struct _node_string_type {
  type_e	type;
  int		refcnt;
  char 		*value;
};
#define node_string_type(n)	((n)->type)
#define node_string_refcnt(n)	((n)->refcnt)
#define node_string_value(n)	((n)->value)

typedef enum {
  OP_TYPE_GSL,
  OP_TYPE_CLC,
} op_type_e;
struct _node_dyadic_type {
  type_e	type;
  int		refcnt;
  node_u	left_arg;
  sym_e		op;
  op_type_e	op_type;
  node_u	modifier;
  node_u	right_arg;
};
#define node_dyadic_type(n)	((n)->type)
#define node_dyadic_refcnt(n)	((n)->refcnt)
#define node_dyadic_la(n)	((n)->left_arg)
#define node_dyadic_op(n)	((n)->op)
#define node_dyadic_op_type(n)	((n)->op_type)
#define node_dyadic_modifier(n)	((n)->modifier)
#define node_dyadic_ra(n)	((n)->right_arg)

struct _node_monadic_type {
  type_e	type;
  int		refcnt;
  sym_e		op;
  op_type_e	op_type;
  node_u	modifier;
  node_u	arg;
};
#define node_monadic_type(n)		((n)->type)
#define node_monadic_refcnt(n)		((n)->refcnt)
#define node_monadic_op(n)		((n)->op)
#define node_monadic_op_type(n)		((n)->op_type)
#define node_monadic_modifier(n)	((n)->modifier)
#define node_monadic_arg(n)		((n)->arg)

struct _node_composite_type {
  type_e	type;
  int		refcnt;
  node_u	left_arg;
  sym_e		left_op;
  sym_e		right_op;
  node_u	modifier;
  node_u	right_arg;
};
#define node_composite_type(n)		((n)->type)
#define node_composite_refcnt(n)	((n)->refcnt)
#define node_composite_la(n)		((n)->left_arg)
#define node_composite_left_op(n)	((n)->left_op)
#define node_composite_right_op(n)	((n)->right_op)
#define node_composite_modifier(n)	((n)->modifier)
#define node_composite_ra(n)		((n)->right_arg)

struct _node_list_type {
  type_e	 type;
  int		 refcnt;
  node_u	*list;
  int		 max;
  int		 next;
};
#define node_list_type(n)	((n)->type)
#define node_list_refcnt(n)	((n)->refcnt)
#define node_list_list(n)	((n)->list)
#define node_list_max(n)	((n)->max)
#define node_list_next(n)	((n)->next)
#define NODE_LIST_INCR	16

struct _node_function_type {
  type_e	 type;
  int		 refcnt;
  node_u	 params;
  node_u	 body;
};
#define node_function_type(n)	((n)->type)
#define node_function_refcnt(n)	((n)->refcnt)
#define node_function_params(n)	((n)->params)
#define node_function_body(n)	((n)->body)

struct _node_call_type {
  type_e	 type;
  int		 refcnt;
  char		*fcn;
  node_u	 args;
};
#define node_call_type(n)	((n)->type)
#define node_call_refcnt(n)	((n)->refcnt)
#define node_call_fcn(n)	((n)->fcn)
#define node_call_args(n)	((n)->args)

struct _node_cpx_vector_type {
  type_e	 type;
  int		 refcnt;
  int		 rhorho;
  int		*rho;
  int		 max;
  int		 next;
  gsl_complex	*data;
};
#define node_cpx_vector_type(n)	((n)->type)
#define node_cpx_vector_refcnt(n)	((n)->refcnt)
#define node_cpx_vector_rhorho(n)	((n)->rhorho)
#define node_cpx_vector_rho(n)		((n)->rho)
#define node_cpx_vector_max(n)	((n)->max)
#define node_cpx_vector_next(n)	((n)->next)
#define node_cpx_vector_data(n)	((n)->data)
#define NODE_CPX_VECTOR_INCR 16
typedef gsl_complex (*cpx_dyadic)(gsl_complex a, gsl_complex b);
typedef gsl_complex (*cpx_monadic)(gsl_complex a);
typedef node_u (*clcx_dyadic)(node_u modifier, node_u la, node_u ra);
typedef node_u (*clc_dyadic)(gsl_complex *retp, node_u modifier,
			     node_u la, node_u ra);
typedef node_u (*clc_monadic)(node_u modifier, node_u arg);

#endif /* NODE_H */
