#ifndef EVAL_H
#define EVAL_H

node_u clc_range (node_u modifier, node_u la, node_u ra);
node_u clc_index (node_u modifier, node_u arg);
node_u clc_complex_real (node_u modifier, node_u arg);
node_u clc_complex_imag (node_u modifier, node_u arg);
node_u clc_complex_arg (node_u modifier, node_u arg);
node_u clc_complex_abs (node_u modifier, node_u arg);
node_u do_eval (node_u node);
void   print_node (node_u node);
void   free_node (node_u node);
node_u create_string_node (type_e type, const char *s);
node_u create_complex_node (gsl_complex v);
node_u create_dyadic_node (node_u la, sym_e op, op_type_e op_type,
			   node_u modifier, node_u ra);
node_u create_monadic_node (sym_e op, op_type_e op_type,
			    node_u modifier, node_u arg);
node_u create_complex_vector_node ();
node_u append_complex_vector_node (node_u vector, gsl_complex v);

#endif /* EVAL_H */
