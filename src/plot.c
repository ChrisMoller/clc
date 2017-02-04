#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <malloc.h>
#include <math.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <values.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_blas.h>
#include <plplot/plplot.h>

#include "node.h"
#include "eval.h"
#include "printext.h"
#include "rgb.h"

#define MAX_PLOT_COLOURS 16

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)
	  
#define dest_offset(r,c)  (((r) * node_cpx_vector_cols (dest)) + c)
#define src_offset(s,r,c)  (((r) * (s)) + c)

typedef enum {
  MODE_XY,
  MODE_CPX,
  MODE_MULTI
} plot_mode_e;

typedef struct {
  FILE 		*out_stream;
  PLINT		 bg_colour[3];
  plot_mode_e	 mode;
  char		*xlabel;
  char		*ylabel;
  char		*toplabel;
  int		 width;
  int		 height;
  PLINT		 axes;
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
#define plot_options_mode(p)		((p)->mode)
#define plot_options_mode_xy(p)		((p)->mode == MODE_XY)
#define plot_options_mode_complex(p)	((p)->mode == MODE_CPX)
#define plot_options_mode_multi(p)	((p)->mode == MODE_MULTI)
#define plot_options_xlabel(p)		((p)->xlabel)
#define plot_options_ylabel(p)		((p)->ylabel)
#define plot_options_toplabel(p)	((p)->toplabel)
#define plot_options_width(p)		((p)->width)
#define plot_options_height(p)		((p)->height)
#define plot_options_axes(p)		((p)->axes)

plot_options_s plot_options = {
  .out_stream	= NULL,
  .bg_colour	= {0, 0, 0},
  .mode		= MODE_MULTI,
  .xlabel	= NULL,
  .ylabel	= NULL,
  .toplabel	= NULL,
  .width	= 512,
  .height	= 480
};

void
parseopts_set_bg_colour (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
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

void
parseopts_set_xlabel (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_LITERAL) {
    node_string_s *ls = node_string (arg);
    char *lv = node_string_value (ls);
    if (plot_options_xlabel (&plot_options))
      free (plot_options_xlabel (&plot_options));
    plot_options_xlabel (&plot_options) = strdup (lv);
  }
}

void
parseopts_set_ylabel (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_LITERAL) {
    node_string_s *ls = node_string (arg);
    char *lv = node_string_value (ls);
    if (plot_options_ylabel (&plot_options))
      free (plot_options_ylabel (&plot_options));
    plot_options_ylabel (&plot_options) = strdup (lv);
  }
}

void
parseopts_set_toplabel (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_LITERAL) {
    node_string_s *ls = node_string (arg);
    char *lv = node_string_value (ls);
    if (plot_options_toplabel (&plot_options))
      free (plot_options_toplabel (&plot_options));
    plot_options_toplabel (&plot_options) = strdup (lv);
  }
}

void
parseopts_set_width (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *ls = node_complex (arg);
    int val = (int)GSL_REAL (node_complex_value (ls));
    plot_options_width (&plot_options) = val;
  }
}

void
parseopts_set_height (node_u argi)
{
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *ls = node_complex (arg);
    int val = (int)GSL_REAL (node_complex_value (ls));
    plot_options_height (&plot_options) = val;
  }
}

void
parseopts_set_mode_xy (node_u argi)
{
  plot_options_mode (&plot_options)  = MODE_XY;
}

void
parseopts_set_mode_complex (node_u argi)
{
  plot_options_mode (&plot_options)  = MODE_CPX;
}

void
parseopts_set_mode_multi (node_u argi)
{
  plot_options_mode (&plot_options)  = MODE_MULTI;
}

void
parseopts_set_nogrid (node_u argi)
{
  plot_options_axes (&plot_options)  = -2;
}

void
parseopts_set_default (node_u argi)
{
  plot_options_axes (&plot_options)  = 0;
}

void
parseopts_set_grid (node_u argi)
{
  if (plot_options_axes (&plot_options) < 0)
    plot_options_axes (&plot_options) = 0;
  plot_options_axes (&plot_options) += 3;
}

void
parseopts_set_logx (node_u argi)
{
  if (plot_options_axes (&plot_options) < 0)
    plot_options_axes (&plot_options) = 0;
  if (plot_options_axes (&plot_options) < 10)
    plot_options_axes (&plot_options) += 10;
}

void
parseopts_set_linearx (node_u argi)
{
  if (plot_options_axes (&plot_options) < 0)
    plot_options_axes (&plot_options) = 0;
  if ((plot_options_axes (&plot_options) >= 10 &&
       plot_options_axes (&plot_options) <= 13) ||
      (plot_options_axes (&plot_options) >= 30 &&
       plot_options_axes (&plot_options) <= 33))
    plot_options_axes (&plot_options) -= 10;
}

void
parseopts_set_logy (node_u argi)
{
  if (plot_options_axes (&plot_options) < 0)
    plot_options_axes (&plot_options) = 0;
  if (plot_options_axes (&plot_options) < 20)
    plot_options_axes (&plot_options) += 20;
}

void
parseopts_set_lineary (node_u argi)
{
  if (plot_options_axes (&plot_options) < 0)
    plot_options_axes (&plot_options) = 0;
  if ((plot_options_axes (&plot_options) >= 20 &&
       plot_options_axes (&plot_options) <= 23) ||
      (plot_options_axes (&plot_options) >= 30 &&
       plot_options_axes (&plot_options) <= 33))
    plot_options_axes (&plot_options) -= 20;
}

void
parseopts_set_axes (node_u argi)
{
  printf ("in axes\n");
  node_u arg = do_eval (NULL, argi);
  if (get_type (arg) == TYPE_COMPLEX) {
    node_complex_s *ls = node_complex (arg);
    int val = (int)GSL_REAL (node_complex_value (ls));
    printf ("setting %d\n", val);
    plot_options_axes (&plot_options) = val;
  }
}

static const ENTRY plotopts[] = {
  {"bgcolour",	parseopts_set_bg_colour},
  {"bgcolor",	parseopts_set_bg_colour},
  {"xy",	parseopts_set_mode_xy},
  {"complex",	parseopts_set_mode_complex},
  {"multi",	parseopts_set_mode_multi},
  {"xlabel",	parseopts_set_xlabel},
  {"ylabel",	parseopts_set_ylabel},
  {"toplabel",	parseopts_set_toplabel},
  {"width",	parseopts_set_width},
  {"height",	parseopts_set_height},
  {"default",	parseopts_set_default},
  {"grid",	parseopts_set_grid},
  {"logx",	parseopts_set_logx},
  {"linearxx",	parseopts_set_linearx},
  {"logy",	parseopts_set_logy},
  {"linearxy",	parseopts_set_lineary},
  {"axes",	parseopts_set_axes},
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
  {
    char *str;
    asprintf (&str, "%dx%d",
	      plot_options_width (&plot_options),
	      plot_options_height (&plot_options));
    plsetopt ("geometry", str);
    free (str);
  }
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

static void
handle_option (char *lv, node_u arg)
{
  ENTRY look_for;
  ENTRY *found;
  found = NULL;
  look_for.key = lv;
  hsearch_r (look_for, FIND, &found, &commands_table);
  if (found) {
    void (*fcn)(node_u arg);
    fcn = found->data;
    if (fcn) (*fcn)(arg);
  }
}

typedef struct {
  PLFLT *xvec;
  PLFLT *yvec;
  PLFLT *zvec;
  int count;
} curves_s;
#define curves_xvec(c)  ((c)->xvec)
#define curves_yvec(c)  ((c)->yvec)
#define curves_zvec(c)  ((c)->zvec)
#define curves_count(c) ((c)->count)
static curves_s *curves = NULL;
static int curves_next = 0;
static int curves_max  = 0;
#define CURVE_INCR 8

static curves_s *
create_curve () {
  if (curves_max <= curves_next) {
    curves_max += CURVE_INCR;
    curves = realloc (curves, curves_max * sizeof(curves_s));
  }
  return &curves[curves_next++];
}

typedef struct {
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double min_z;
  double max_z;
} extremes_s;
#define min_x(e) ((e)->min_x)
#define max_x(e) ((e)->max_x)
#define min_y(e) ((e)->min_y)
#define max_y(e) ((e)->max_y)
#define min_z(e) ((e)->min_z)
#define max_z(e) ((e)->max_z)

static void
single_plot (node_cpx_vector_s *ls, extremes_s *extremes)
{
  if (node_cpx_vector_rho (ls)[0] > 1) {
    // ./clc 'plot {"xy", bgcolour = "purple"} (sin(::{.2}30))'
    int ct = node_cpx_vector_rho (ls)[0];
    curves_s *curve = create_curve ();
    curves_count (curve) = ct;
    curves_xvec (curve) = malloc (ct * sizeof(PLFLT));
    curves_yvec (curve) = malloc (ct * sizeof(PLFLT));
    curves_zvec (curve) = malloc (ct * sizeof(PLFLT));
    for (int i = 0; i < node_cpx_vector_next (ls); i++) {
      double xv = (double)i;
      double yv = GSL_REAL (node_cpx_vector_data (ls)[i]);
      double zv = GSL_IMAG (node_cpx_vector_data (ls)[i]);
      if (min_x (extremes) > xv)
	min_x (extremes) = xv;
      if (max_x (extremes) < xv)
	max_x (extremes) = xv;
      if (min_y (extremes) > yv)
	min_y (extremes) = yv;
      if (max_y (extremes) < yv)
	max_y (extremes) = yv;
      if (min_z (extremes) > zv)
	min_z (extremes) = zv;
      if (max_z (extremes) < zv)
	max_z (extremes) = zv;
      curves_xvec (curve)[i] = (PLFLT)xv;
      curves_yvec (curve)[i] = (PLFLT)yv;
      curves_zvec (curve)[i] = (PLFLT)zv;
    }
  }
}
  
node_u
clc_plot (node_u modifier, node_u argi)
{

  /***
      rhorho == 1, 2d real x vs index
                   3d complex x vs index
      rhorho == 2, 2d real y vs real x
                   3d real y on complex x plane, surface or mesh
                   2d multiple real x vs index 
      rhorho == 3, 3d real x, y, and z, surface or mesh
   ***/
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, argi);

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

  if (get_type (rm) == TYPE_LIST) {
    node_list_s *list   = node_list (rm);
    for (int i = 0; i < node_list_next (list); i++) {
      node_u ln = node_list_list (list)[i];
      if (get_type (ln) == TYPE_LITERAL) {
	node_string_s *ls = node_string (ln);
	char *lv = node_string_value (ls);
	handle_option (lv, NULL_NODE);
      }
      else if (get_type (ln) == TYPE_DYADIC) {
	do_eval (NULL, ln);
      }
    }
  }
  else if (get_type (rm) == TYPE_LITERAL) {
    node_string_s *ls = node_string (rm);
    char *lv = node_string_value (ls);
    handle_option (lv, NULL_NODE);
  }
  if (get_current_symtab ()) {
    twalk (get_current_symtab (), var_action);
  }
  
  init_plplot ();

  extremes_s extremes = {MAXDOUBLE, -MAXDOUBLE,
			 MAXDOUBLE, -MAXDOUBLE,
			 MAXDOUBLE, -MAXDOUBLE};
  if (get_type (arg) == TYPE_CPX_VECTOR) {
    node_cpx_vector_s *ls = node_cpx_vector (arg);

    if (node_cpx_vector_rhorho (ls) == 1) {		// just a simple plot
      single_plot (ls, &extremes);
    }
    else if (node_cpx_vector_rhorho (ls) == 2) {
      // ./clc 'plot ([2 30]<>(sin(::{.2}30)),(cos(::{.2}30)))'
      // ./clc 'plot {"xy"} ([2 30]<>(sin(::{.2}30)),(cos(::{.2}30)))'
      if (plot_options_mode_xy (&plot_options)) {
      }
      else if (plot_options_mode_complex (&plot_options)) {
      }
      else {	// multiplot
      }
    }
    else if (node_cpx_vector_rhorho (ls) == 3) {
    }
    else {
      // fixme don't know handle 4-space
    }

    if (min_z (&extremes) == 0.0 && max_z (&extremes) == 0.0) {
      plenv ((PLFLT)min_x (&extremes),
	     (PLFLT)max_x (&extremes),
	     (PLFLT)min_y (&extremes),
	     (PLFLT)max_y (&extremes),
	     0, plot_options_axes (&plot_options));
      pllab (plot_options_xlabel (&plot_options),
	     plot_options_ylabel (&plot_options),
	     plot_options_toplabel (&plot_options));

      for (int i = 0; i < curves_next; i++) {
	plcol0 ((PLINT)((i +1) % (MAX_PLOT_COLOURS - 1)));
	plline ((PLINT)curves_count (&curves[i]),
		curves_xvec (&curves[i]),
		curves_yvec (&curves[i]));
	free (curves_xvec (&curves[i]));
	free (curves_yvec (&curves[i]));
      }
    }
    else {
      pladv (0);
      plvpor( 0.0, 1.0, 0.0, 1.0 );
      plwind( -512.00, 512.0, -200.0, 600.0);
      
      plw3d (512.0, // basex,
	     512.0, // basey,
	     512.0, // height,
	     (PLFLT)min_x (&extremes),        // xmin,
	     (PLFLT)max_x (&extremes),        // xmax,
	     (PLFLT)min_y (&extremes),        // ymin,
	     (PLFLT)max_y (&extremes),        // ymax,
	     (PLFLT)min_z (&extremes),        // zmin,
	     (PLFLT)max_z (&extremes),        // zmax,
	     20.0,  // alt,
	     -30.0);        // az: pos cw from top, neg ccw from top

      plmtex ("t", -1.5, 0.5, 0.5, plot_options_toplabel (&plot_options));

      plbox3 ("bnstu",		// xopt,
	      "Index",		// xlabel,      
	      0.0,		// xtick,
	      0,		// nxsub,
	      
	      "bnstu",		// yopt,
	      "Imaginary",	// ylabel,      
	      0.0,		// ytick,
	      0,		// nysub,
	      
	      "bcdmnstuv",	// zopt,
	      "Real",		// zlabel,      
	      0.0,		// ztick,
	      0);		// nzsub
      
      for (int i = 0; i < curves_next; i++) {
	plline3 ((PLINT)curves_count (&curves[i]),
		 curves_xvec (&curves[i]),
		 curves_yvec (&curves[i]),
		 curves_zvec (&curves[i]));
	free (curves_xvec (&curves[i]));
	free (curves_yvec (&curves[i]));
	free (curves_zvec (&curves[i]));
      }
    }
    curves_next = 0;
  }
  plend ();

  plot_options_out_stream (&plot_options) = NULL;
  
  pop_symtab ();
  return rc;
}

#if 0
      
      if (node_cpx_vector_rho (ls)[0] > 1) {
	if (1) {
	  int r, off;
	  for (r = 1, off = lcols; r < lrows; r++) {
	    curves_s *curve = create_curve ();
	    curves_count (curve) = lcols;
	    curves_xvec (curve) = malloc (lcols * sizeof(PLFLT));
	    curves_yvec (curve) = malloc (lcols * sizeof(PLFLT));
	    for (int i = 0; i < lcols; i++, off++) {
	      double xv = GSL_REAL (node_cpx_vector_data (ls)[i]);
	      double yv = GSL_REAL (node_cpx_vector_data (ls)[off]);
	      if (min_x > xv) min_x = xv;
	      if (max_x < xv) max_x = xv;
	      if (min_y > yv) min_y = yv;
	      if (max_y < yv) max_y = yv;
	      curves_xvec (curve)[i] = (PLFLT)xv;
	      curves_yvec (curve)[i] = (PLFLT)yv;
	    }
	  }
	}
	else {
	  int r, off;
	  for (r = 0, off = 0; r < lrows; r++) {
	    curves_s *curve = create_curve ();
	    curves_count (curve) = lcols;
	    curves_xvec (curve) = malloc (lcols * sizeof(PLFLT));
	    curves_yvec (curve) = malloc (lcols * sizeof(PLFLT));
	    for (int i = 0; i < lcols; i++, off++) {
	      double xv = (double)i;
	      double yv = GSL_REAL (node_cpx_vector_data (ls)[off]);
	      if (min_x > xv) min_x = xv;
	      if (max_x < xv) max_x = xv;
	      if (min_y > yv) min_y = yv;
	      if (max_y < yv) max_y = yv;
	      curves_xvec (curve)[i] = (PLFLT)xv;
	      curves_yvec (curve)[i] = (PLFLT)yv;
	    }
	  }
	}
      }
    }
  }
  else {
    // fixme error
  }
#endif

  //     xmin xmax ymin ymax

// '[3 8]<>((sin(::{.2}8)), (cos(::{.2}8)), (8<>.6))'
