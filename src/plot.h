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
void parseopts_set_mode_noxy (node_u arg);

#endif  /* PLOT_H */
