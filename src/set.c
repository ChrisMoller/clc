#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <malloc.h>
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "node.h"
#include "plot.h"
#include "eval.h"

static void
set_seed (node_u arg)
{
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *ls = node_complex (arg);
    long int val = (long int)GSL_REAL (node_complex_value (ls));
    srand48 (val);
  }
}

static const ENTRY plotopts[] = {
  {"plotbgcolour",	parseopts_set_bg_colour},
  {"plotbgcolor",	parseopts_set_bg_colour},
  {"plotxy",		parseopts_set_mode_xy},
  {"plotnoxy",		parseopts_set_mode_noxy},
  {"plotxlabel",	parseopts_set_xlabel},
  {"plotylabel",	parseopts_set_ylabel},
  {"plottoplabel",	parseopts_set_toplabel},
  {"plotwidth",		parseopts_set_width},
  {"plotheight",	parseopts_set_height},
  {"seed",		set_seed},
};
static const int plotopts_len = sizeof(plotopts) / sizeof(ENTRY);
static int commands_table_initialised = 0;
static struct hsearch_data commands_table;

void
do_set (char *lv, node_u argi)
{
  ENTRY look_for;
  ENTRY *found;
  node_u arg = do_eval (NULL, argi);

  if (!commands_table_initialised) {
    int i;
    ENTRY *ret;
    commands_table_initialised = 1;
    bzero (&commands_table, sizeof (struct hsearch_data));
    hcreate_r ((size_t)plotopts_len, &commands_table);
    for (i = 0; i < plotopts_len; i++)
      hsearch_r (plotopts[i], ENTER, &ret, &commands_table);
  }

  found = NULL;
  look_for.key = lv;
  hsearch_r (look_for, FIND, &found, &commands_table);
  if (found) {
    void (*fcn)(node_u arg);
    fcn = found->data;
    if (fcn) (*fcn)(arg);
  }
}


