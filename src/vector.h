#ifndef VECTOR_H
#define VECTOR_H

node_u clc_matrix_mul (node_u modifier, node_u la, node_u ra);
node_u clc_catenate (node_u modifier, node_u la, node_u ra);
node_u clc_extract (node_u modifier, node_u la, node_u ra);
node_u clc_reshape (node_u modifier, node_u la, node_u ra);
node_u clc_shape (node_u modifier, node_u arg);
node_u clc_ravel (node_u modifier, node_u arg);
node_u clc_transpose (node_u modifier, node_u arg);
node_u clc_sigma (node_u modifier, node_u arg);
node_u clc_pi (node_u modifier, node_u arg);
node_u do_inner (sym_e lsym, sym_e rsym, node_u la, node_u ra, node_u mo);
node_u do_outer (sym_e rsym, node_u la, node_u ra, node_u mo);

#endif  /* VECTOR_H */
