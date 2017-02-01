#ifndef PLOT_H
#define PLOT_H

node_u clc_plot (node_u modifier, node_u arg);
void parseopts_set_bg_colour (node_u arg);
void parseopts_set_xlabel (node_u arg);
void parseopts_set_ylabel (node_u arg);
void parseopts_set_toplabel (node_u arg);
void parseopts_set_width (node_u arg);
void parseopts_set_height (node_u arg);
void parseopts_set_mode_xy (node_u arg);
void parseopts_set_mode_complex (node_u arg);
void parseopts_set_mode_multi (node_u arg);
void parseopts_set_default (node_u arg);
void parseopts_set_grid (node_u arg);
void parseopts_set_logx (node_u arg);
void parseopts_set_linearx (node_u arg);
void parseopts_set_logy (node_u arg);
void parseopts_set_lineary (node_u arg);
void parseopts_set_axes (node_u arg);

#endif  /* PLOT_H */
