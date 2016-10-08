#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <malloc.h>
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>

#include "node.h"

void
do_set (const char *kwd, node_u val)
{
  printf ("setting %s\n", kwd);
}

void
do_set_bif (int kwd, node_u val)
{
  printf ("setting %d\n", kwd);
}
