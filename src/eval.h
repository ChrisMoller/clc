#ifndef EVAL_H
#define EVAL_H

typedef struct {
  char *lbl;
  node_u      node;
} symbol_entry_s;
#define sym_lbl(e)  ((e)->lbl)
#define sym_node(e) ((e)->node)

node_u  clc_lt (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_le (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_gt (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_ge (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_eq (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_ne (gsl_complex *retp, node_u modifier, node_u la, node_u ra);
node_u  clc_assign (node_u modifier, node_u la, node_u ra);
node_u  clc_range (node_u modifier, node_u la, node_u ra);
node_u  clc_index (node_u modifier, node_u arg);
node_u  clc_random (node_u modifier, node_u arg);
node_u  clc_file (node_u modifier, node_u arg);
node_u  clc_complex_real (node_u modifier, node_u arg);
node_u  clc_complex_imag (node_u modifier, node_u arg);
node_u  clc_complex_arg (node_u modifier, node_u arg);
node_u  clc_complex_abs (node_u modifier, node_u arg);
node_u  do_eval (int *noshow, node_u node);
void    print_node (int indent, node_u node);
void    push_symtab ();
void    pop_symtab ();
void   *get_current_symtab ();
void	do_assign (const char *name, node_u ra);
void   *get_op_dyadic (sym_e sym);

#endif /* EVAL_H */
