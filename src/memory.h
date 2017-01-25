#ifndef MEMORY_H
#define MEMORY_H

char  *get_qbuffer ();
void   clear_qbuffer ();
void   cat_string_to_qbuffer (char *str);
void   cat_char_to_qbuffer (char chr);

void   free_node (node_u node);
node_u create_string_node (type_e type, const char *s);
node_u create_complex_node (int negate, gsl_complex v);
node_u create_dyadic_node (node_u la, sym_e op, op_type_e op_type,
			   node_u modifier, node_u ra);
node_u create_monadic_node (sym_e op, op_type_e op_type,
			    node_u modifier, node_u arg);
node_u create_composite_node (node_u la, sym_e lop, sym_e op,
			      node_u modifier, node_u arg);
node_u append_matrix (node_u lv, node_u rv);
node_u create_complex_vector_node ();
node_u append_complex_vector_node (node_u vector, gsl_complex v);
node_u create_call (char *fcn, node_u args);
void   node_incref (node_u node);
void   node_decref (node_u node);
void   create_function (const char *name, node_u params, node_u body);
#ifdef DO_TREE
void   walk_nodes ();
#endif

#endif /* MEMORY_H */
