#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_blas.h>
#include <plplot/plplot.h>

#include "node.h"
#include "eval.h"
#include "printext.h"
#include "rgb.h"

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)
	  
#define dest_offset(r,c)  (((r) * node_cpx_vector_cols (dest)) + c)
#define src_offset(s,r,c)  (((r) * (s)) + c)

typedef struct {
  FILE 		*out_stream;
  PLINT		 bg_colour[3];
} plot_options_s;
#define plot_options_out_stream(p)	((p)->out_stream)
#define plot_options_bg_colour(p)	((p)->bg_colour)
#define COLOUR_IDX_RED		0
#define COLOUR_IDX_GREEN	1
#define COLOUR_IDX_BLUE		2
#define plot_options_bg_colour_component(p,i)	((p)->bg_colour[i])
#define plot_options_bg_colour_red(p)	((p)->bg_colour[COLOUR_IDX_RED])
#define plot_options_bg_colour_green(p)	((p)->bg_colour[COLOUR_IDX_GREEN])
#define plot_options_bg_colour_blue(p)	((p)->bg_colour[COLOUR_IDX_BLUE])

plot_options_s plot_options = {
  .out_stream = NULL,
  .bg_colour  = {0, 0, 0},
};

static void
parseopts_set_bg_colour (node_u arg)
{
  printf ("in bg colour\n");
  switch(get_type (arg)) {
  case TYPE_CPX_VECTOR:
    {
      node_cpx_vector_s *ls = node_cpx_vector (arg);
      for (int i = 0; i < node_cpx_vector_next (ls) && i < 3; i++) {
	double val = GSL_REAL (node_cpx_vector_data (ls)[i]);
	if (val >= 0.0) {
	  plot_options_bg_colour_component (&plot_options, i) =
	    (PLINT)((val <= 1.0) ? (val * 255.0) : val);
	}
      }
    }
    break;
  case TYPE_LITERAL:
    {
      int red, green, blue;
      node_string_s *ls = node_string (arg);
      char *lv = node_string_value (ls);
      int rc = lookup_colour (lv, &red, &green, &blue);
      if (rc) {
	plot_options_bg_colour_red (&plot_options)   = (PLINT)red;
	plot_options_bg_colour_green (&plot_options) = (PLINT)green;
	plot_options_bg_colour_blue (&plot_options)  = (PLINT)blue;
      }
      //printf ("colour = %s %d %d %d %d\n", lv, rc, red, green, blue);
    }
    break;
  default:
    // fixme can't parse it
    break;
  }
}

static const ENTRY plotopts[] = {
  {"bgcolour",          parseopts_set_bg_colour},
  {"bgcolor",           parseopts_set_bg_colour},
};
static const int plotopts_len = sizeof(plotopts) / sizeof(ENTRY);
static int commands_table_initialised = 0;
static struct hsearch_data commands_table;

static void
init_plplot ()
{
  int pipefd[2];

  pipe (pipefd);

  if (0 == fork ()) {
    /***
	child -- start a process which then starts the ImageMagick
	display utility, opens a pipe to it, and then exits.  This
	results in a detached window containing the plot but without
	stopping clc.  The plot will persist after clc closes so the
	user can do a one-liner like 'clc "<whatever>"' and keep the
	plot up.  The plot can be closed like any other window.
    ***/
    close (pipefd[1]);
#define CMD_BFR_LEN     64
    char cmd_bfr[CMD_BFR_LEN];
    snprintf (cmd_bfr, CMD_BFR_LEN, "display fd:%d", pipefd[0]);
    FILE *display = popen (cmd_bfr, "w");
    pclose (display);
    close (pipefd[0]);
    _exit (0);
  }
  else {
    /***
	parent -- opens its end of the pipe that the child has set up
	to the display utility, then does all the plotting
    ***/
    close (pipefd[0]);
    plot_options_out_stream (&plot_options) = fdopen (pipefd[1], "w");
  }

  plsfile (plot_options_out_stream (&plot_options));
  plscolbg (plot_options_bg_colour_red (&plot_options),
	    plot_options_bg_colour_green (&plot_options),
	    plot_options_bg_colour_blue (&plot_options));

  plsdev ("pngcairo");
  plsetopt ("geometry", "300x300");
  plinit ();
}

static void
var_action (const void *nodep, const VISIT which, const int depth)
{
  switch (which) {
  case preorder:
  case endorder:
    break;
  case postorder:
  case leaf:
    {
      ENTRY look_for;
      ENTRY *found;
      symbol_entry_s *vp = *(symbol_entry_s **)nodep;
      char  *lbl  = sym_lbl (vp);
      found = NULL;
      look_for.key = lbl;
      hsearch_r (look_for, FIND, &found, &commands_table);
      if (found) {
	node_u node = sym_node (vp);
	void (*fcn)(node_u arg);
	fcn = found->data;
	if (fcn) (*fcn)(node);
      }
#if 0
      print_node (0, nd);
      printf ("\n");
#endif
      break;
    }
  }
}

node_u
clc_plot (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;

  if (!commands_table_initialised) {
    int i;
    ENTRY *ret;
    commands_table_initialised = 1;
    bzero (&commands_table, sizeof (struct hsearch_data));
    hcreate_r ((size_t)plotopts_len, &commands_table);
    for (i = 0; i < plotopts_len; i++)
      hsearch_r (plotopts[i], ENTER, &ret, &commands_table);
    /*
    for (i = 0; i < MAX_PLOT_COLOURS; i++)
      plot_options_colour_red (&plot_options, i) = -1;
    */
    //    plot_options_device (&plot_options) = XSTRDUP (DEFAULT_DEVICE);
  }

  push_symtab ();
  node_u rm = do_eval (NULL, modifier);

  if (get_current_symtab ()) {
    twalk (get_current_symtab (), var_action);
  }
  if (get_type (rm) == TYPE_LIST) {
  }
  
  init_plplot ();

  PLFLT *xvec = NULL;
  PLFLT *yvec = NULL;
  int count = 0;


  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *ls = node_cpx_vector (arg);
    int lrows = node_cpx_vector_rows (ls);
    int lcols = node_cpx_vector_cols (ls);
    if (lrows == 0) {			// just a simple plot
      if (lcols > 1) {
	count = lcols;
	xvec = malloc (count * sizeof(PLFLT));
	yvec = malloc (count * sizeof(PLFLT));
	for (int i = 0; i < count; i++) {
	  xvec [i] = (PLFLT)i;
	  yvec [i] = (PLFLT)GSL_REAL (node_cpx_vector_data (ls)[i]);
	}
      }
    }
  }
  else {
    // fixme error
  }

  //     xmin xmax ymin ymax
  plenv (0.0, 30.0, -1.0, 1.0, 0, 0);

  plline ((PLINT)count, xvec, yvec);
  plend ();

  plot_options_out_stream (&plot_options) = NULL;
  
  pop_symtab ();
  return rc;
}
