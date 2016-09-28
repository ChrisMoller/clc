#ifndef EVAL_H
#define EVAL_H

node_u do_eval (node_u node);
void   print_node (node_u node);
void   free_node (node_u node);
node_u create_string_node (type_e type, const char *s);
node_u create_complex_node (gsl_complex v);
node_u create_dyadic_node (node_u la, sym_e op, node_u ra);
node_u create_monadic_node (sym_e op, node_u arg);

#endif /* EVAL_H */
