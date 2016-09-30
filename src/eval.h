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

#endif /* EVAL_H */
