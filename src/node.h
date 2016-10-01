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
struct _node_list_s;
struct _node_cpx_vector_s;

typedef struct _node_type_type		node_type_s;
typedef struct _node_string_type	node_string_s;
typedef struct _node_complex_type	node_complex_s;
typedef struct _node_dyadic_type	node_dyadic_s;
typedef struct _node_monadic_type	node_monadic_s;
typedef struct _node_list_type		node_list_s;
typedef struct _node_cpx_vector_type	node_cpx_vector_s;


typedef union _node_u {
  node_type_s		*t;
  node_complex_s	*c;
  node_string_s		*s;
  node_dyadic_s		*d;
  node_monadic_s	*m;
  node_list_s		*l;
  node_cpx_vector_s	*v;
  void	                *vp;
  const void	        *vcp;
} node_u;
#define node_type(n)		((n).t)
#define node_complex(n)		((n).c)
#define node_string(n)		((n).s)
#define node_dyadic(n)		((n).d)
#define node_monadic(n)		((n).m)
#define node_list(n)		((n).l)
#define node_cpx_vector(n)	((n).v)
#define node_void(n)		((n).vp)

typedef enum {
  TYPE_NULL,
  TYPE_COMPLEX,
  TYPE_LITERAL,
  TYPE_STRING,
  TYPE_DYADIC,
  TYPE_MONADIC,
  TYPE_LIST,
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
#define node_monadic_refcnt(n)	((n)->refcnt)
#define node_monadic_op(n)		((n)->op)
#define node_monadic_op_type(n)		((n)->op_type)
#define node_monadic_modifier(n)	((n)->modifier)
#define node_monadic_arg(n)		((n)->arg)

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

struct _node_cpx_vector_type {
  type_e	 type;
  int		 refcnt;
  int		 rows;
  int		 cols;
  int		 max;
  int		 next;
  gsl_complex	*data;
};
#define node_cpx_vector_type(n)	((n)->type)
#define node_cpx_vector_refcnt(n)	((n)->refcnt)
#define node_cpx_vector_rows(n)	((n)->rows)
#define node_cpx_vector_cols(n)	((n)->cols)
#define node_cpx_vector_max(n)	((n)->max)
#define node_cpx_vector_next(n)	((n)->next)
#define node_cpx_vector_data(n)	((n)->data)
#define NODE_CPX_VECTOR_INCR 16

#endif /* NODE_H */
