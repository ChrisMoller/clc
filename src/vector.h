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

#endif  /* VECTOR_H */
