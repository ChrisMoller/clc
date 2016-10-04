#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <math.h>
#ifdef _PARSE_OPT_STRINGS
#include <regex.h>
#endif
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
//#include "memory.h"
#include "eval.h"
#include "printext.h"

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
#define plot_options_bg_colour_red(p)	((p)->bg_colour[COLOUR_IDX_RED])
#define plot_options_bg_colour_green(p)	((p)->bg_colour[COLOUR_IDX_GREEN])
#define plot_options_bg_colour_blue(p)	((p)->bg_colour[COLOUR_IDX_BLUE])

plot_options_s plot_options = {
  .out_stream = NULL
};

static void
parseopts_set_bg_colour (plot_options_s *plot_options, char *arg)
{
#if 0
  PLINT red = -1, green = -1, blue = -1;
  if (parse_colour (arg, &red, &green, &blue, COLOUR_BG)) {
    plot_options_bg_colour_red (plot_options)   = red;
    plot_options_bg_colour_green (plot_options) = green;
    plot_options_bg_colour_blue (plot_options)  = blue;
  }
#endif
}

static const ENTRY plotopts[] = {
  {"bgcolour",          parseopts_set_bg_colour},
  {"bgcolor",           parseopts_set_bg_colour},
};

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
  plsdev ("pngcairo");
  plsetopt ("geometry", "300x300");
  plinit ();
}

#ifdef _PARSE_OPT_STRINGS
/***
    strip off leading spaces and tabs [[*space]]*
    identify kwd                      ([[:alpha:]][[:alnum:]_]*)
    skip spaces                       [[*space]]*
    look for optional assignment      (=
    skip spaces                       [[*space]]*
    quoted string                     \"([^\"]*)\"
    remainder                         (.*)
 ***/
#define RE \
  "[[:space:]]*([[:alpha:]][[:alnum:]_]*)[[:space:]]*(=[[:space:]]*\"([^\"]*)\")?(.*)"
#endif

static void
parse_opts_list (node_list_s *es)
{
  int nr_elems = node_list_next (es);
  if (nr_elems > 0) {
    node_u kwd_node = node_list_list (es)[0];
    if (get_type (kwd_node) == TYPE_LITERAL) {
      node_string_s *kns = node_string (kwd_node);
      char *kwd = node_string_value (kns);
      printf ("kwd = \"%s\"\n", kwd);
    }
  }
}

node_u
clc_plot (node_u modifier, node_u arg)
{
  node_u rc = NULL_NODE;

  init_plplot ();

  PLFLT *xvec = NULL;
  PLFLT *yvec = NULL;
  int count = 0;
#ifdef _PARSE_OPT_STRINGS
  static regex_t preg;
  static int preg_set = 0;

  if (!preg_set) {
    regcomp (&preg, RE, REG_ICASE | REG_EXTENDED);
    preg_set = 1;
  }
#endif


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
  else if (get_type (arg) == TYPE_LIST) {
    // ./clc -e 'plot("bg = \"pale red\" fg = \"green\"",  sin(::{.2}30))'
    node_list_s *ls = node_list (arg);
    int elements = node_list_next (ls);
    for (int e = 0; e < elements; e++) {
      node_u elem = node_list_list (ls)[e];
      switch(get_type (elem)) {
      case TYPE_LIST:
	{
	  node_list_s *es = node_list (elem);
	  parse_opts_list (es);
	}
	break;
      case TYPE_CPX_VECTOR:
	{
	  node_cpx_vector_s *ls = node_cpx_vector (elem);
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
	break;
      case TYPE_LITERAL:
	{
#ifndef _PARSE_OPT_STRINGS
#else
	  node_string_s *ns = node_string (elem);
	  char *str = strdup (node_string_value (ns));
	  char *str_base = str;
	  if (str) {
#define MATCH_KWD 1
#define MATCH_VALUE 3
#define MATCH_REMAINDER 4
	    regmatch_t *match =
	      malloc ((1 + preg.re_nsub) * sizeof(regmatch_t));
	    while (*str) {
	      int rc = regexec (&preg, str, 1 + preg.re_nsub, match, 0);
	      if (rc == 0) {
		str[match[MATCH_KWD].rm_eo] = 0;
		char *kwd = str + match[MATCH_KWD].rm_so;
		char *val = NULL;
		if (match[MATCH_VALUE].rm_eo != -1) {
		  str[match[MATCH_VALUE].rm_eo] = 0;
		  val = str + match[MATCH_VALUE].rm_so;
		}
		printf ("%s = \"%s\"\n", kwd, val);
		str += match[MATCH_REMAINDER].rm_so;
	      }
	      else *str = 0;	// stop scanning
	    }
	    if (match) free (match);
	    free (str_base);
	  }
#endif
	}
	break;
      default:
	//fixme huh?
	break;
      }
    }
  }
  else {
    // fixme huh?
  }


  //     xmin xmax ymin ymax
  plenv (0.0, 30.0, -1.0, 1.0, 0, 0);

  plline ((PLINT)count, xvec, yvec);
  plend ();

  plot_options_out_stream (&plot_options) = NULL;
  
  
  return rc;
}
