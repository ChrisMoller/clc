#ifndef NODE_H
#define NODE_H


#undef ENTRY
#define ENTRY(l,s,d,m,dd,md) s
typedef enum {
#include "opsdata.h"
} sym_e;

struct _node_type_s;
struct _node_string_s;
struct _node_complex_s;
struct _node_dyadic_s;
struct _node_mondic_s;

typedef struct _node_type_type		node_type_s;
typedef struct _node_string_type	node_string_s;
typedef struct _node_complex_type	node_complex_s;
typedef struct _node_dyadic_type	node_dyadic_s;
typedef struct _node_monadic_type	node_monadic_s;


typedef union _node_u {
  node_type_s		*t;
  node_complex_s	*c;
  node_string_s		*s;
  node_dyadic_s		*d;
  node_monadic_s	*m;
} node_u;
#define node_type(n)	((n).t)
#define node_complex(n)	((n).c)
#define node_string(n)	((n).s)
#define node_dyadic(n)	((n).d)
#define node_monadic(n)	((n).m)

typedef enum {
  TYPE_NULL,
  TYPE_COMPLEX,
  TYPE_LITERAL,
  TYPE_STRING,
  TYPE_DYADIC,
  TYPE_MONADIC,
} type_e;

#define TYPE_GEN(l,r)  ((100 * (l)) + (r))

struct _node_type_type {
  type_e	type;
};
#define node_type_type(n)	((n)->type)
#define get_type(n) (node_type_type (node_type (n)))

struct _node_complex_type {
  type_e	type;
  gsl_complex	value;
};
#define node_complex_type(n)	((n)->type)
#define node_complex_value(n)	((n)->value)

struct _node_string_type {
  type_e	type;
  char 		*value;
};
#define node_string_type(n)	((n)->type)
#define node_string_value(n)	((n)->value)

struct _node_dyadic_type {
  type_e	type;
  node_u	left_arg;
  sym_e		op;
  node_u	right_arg;
};
#define node_dyadic_type(n)	((n)->type)
#define node_dyadic_la(n)	((n)->left_arg)
#define node_dyadic_op(n)	((n)->op)
#define node_dyadic_ra(n)	((n)->right_arg)

struct _node_monadic_type {
  type_e	type;
  sym_e		op;
  node_u	arg;
};
#define node_monadic_type(n)	((n)->type)
#define node_monadic_op(n)	((n)->op)
#define node_monadic_arg(n)	((n)->arg)

#endif /* NODE_H */
