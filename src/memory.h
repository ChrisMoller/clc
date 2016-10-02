#ifndef MEMORY_H
#define MEMORY_H

void   free_node (node_u node);
node_u create_string_node (type_e type, const char *s);
node_u create_complex_node (gsl_complex v);
node_u create_dyadic_node (node_u la, sym_e op, op_type_e op_type,
			   node_u modifier, node_u ra);
node_u create_monadic_node (sym_e op, op_type_e op_type,
			    node_u modifier, node_u arg);
node_u create_complex_vector_node ();
node_u append_complex_vector_node (node_u vector, gsl_complex v);
void node_incref (node_u node);
void node_decref (node_u node);
#ifdef DO_TREE
void   walk_nodes ();
#endif

#endif /* MEMORY_H */