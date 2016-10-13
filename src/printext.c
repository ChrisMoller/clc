#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <ffi.h>
#include <math.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>

#include "node.h"
#include "eval.h"

static node_type_s null_node = { TYPE_NULL };
#define NULL_NODE (node_u)(&null_node)

/***
    predefined:

        h l L q j z t

        d i o u x X e E f F g G a A c s C S p n

    mine:

        P R
***/

static int
print_complex_rect (FILE *stream,
		    const struct printf_info *info,
		    const void *const *args)
{
  char *buffer;
  int len;
  double rz;
  double iz;
  if (info->alt) {
    gsl_complex z = **((gsl_complex **) (args[0]));
    rz = GSL_REAL (z);
    iz = GSL_IMAG (z);
  }
  else {
    rz = *(double *)args[0];
    iz = *(double *)args[1];
  }
  char *units = "";
  if (iz == 0.0 && info->alt) {
    rz *= 180.0 / M_PI;
    units = "d";
  }
  len = (iz == 0.0)
    ? asprintf (&buffer, "%g%s", rz, units)
    : asprintf (&buffer, "%g%+gi", rz, iz);
  if (len == -1) return -1;

  len = fprintf (stream, "%*s",
                 (info->left ? -info->width : info->width),
		 buffer);

  free (buffer);
  return len;
}

static int
print_complex_polar (FILE *stream,
		    const struct printf_info *info,
		    const void *const *args)
{
  char *buffer;
  int len;
  double rz;
  double iz;
  if (info->alt) {
    gsl_complex z = **((gsl_complex **) (args[0]));
    rz = GSL_REAL (z);
    iz = GSL_IMAG (z);
  }
  else {
    rz = *(double *)args[0];
    iz = *(double *)args[1];
  }
  double pha = atan2 (iz, rz);
  if (pha < 0.0) pha += 2.0 * M_PI;
  double mag = hypot (iz, rz);
  char *units = "";
  if (info->alt) {
    pha *= 180.0 / M_PI;
    units = "d";
  }
  len = asprintf (&buffer, "%g<%g%s", mag, pha, units);
  if (len == -1) return -1;

  len = fprintf (stream, "%*s",
                 (info->left ? -info->width : info->width),
		 buffer);

  free (buffer);
  return len;
}

#if 0
static int
my_printf(const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  char *s = va_arg (ap, char *);
  printf ("fmt = %s s = %s\n", format, s);
  return 6;
}
#endif

static int
print_complex_arginfo (const struct printf_info *info,
		       size_t n,
		       int *argtypes, int *size)
{
  if (n > 0) {
    argtypes[0] = PA_DOUBLE;
    argtypes[1] = PA_DOUBLE;
  }
  
  return 2;
}

void
print_init ()
{
  // https://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html#Customizing-Printf
  // https://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html
  register_printf_specifier ('R', print_complex_rect, print_complex_arginfo);
  register_printf_specifier ('P', print_complex_polar, print_complex_arginfo);
}

node_u
clc_print (node_u modifier, node_u argi)
{
  node_u rc = NULL_NODE;
  node_u arg = do_eval (NULL, argi);

  switch(get_type (arg)) {
  case TYPE_LIST:
    {
      node_list_s *list = node_list (arg);
      if (node_list_next (list) > 0) {
	ffi_cif cif;
	ffi_type *rtype = &ffi_type_sint;
	ffi_type **argtypes =
	  calloc (2 * node_list_next (list), sizeof(ffi_type *));
	void **argvalues =
	  calloc (2 * node_list_next (list), sizeof(void *));
	int i;
	int c = 0;
	for (i = 0; i < node_list_next (list); i++) {
	  node_u node = node_list_list (list)[i];
	  switch(get_type (node)) {
	  case TYPE_LITERAL:
	    {
	      node_string_s *ss = node_string (node);
	      argtypes[c]    = &ffi_type_pointer;
	      argvalues[c++] = &node_string_value (ss);
	    }
	    break;
	  case TYPE_COMPLEX:
	    {
	      node_complex_s *vs = node_complex (node);
	      argtypes[c]    = &ffi_type_double;
	      argvalues[c++] = &GSL_REAL (node_complex_value (vs));
	      argtypes[c]    = &ffi_type_double;
	      argvalues[c++] = &GSL_IMAG (node_complex_value (vs));
	    }
	    break;
	  default:
	    break;
	  }
	}
	if (ffi_prep_cif_var (&cif,
			      FFI_DEFAULT_ABI,
			      1,
			      c,
			      rtype,
			      argtypes) == FFI_OK) {
	  ffi_arg result; 
	  ffi_call (&cif, (void (*) (void)) &printf, &result, argvalues);
	}
	if (argtypes)  free (argtypes);
	if (argvalues) free (argvalues);
      }
    }
    break;
  case TYPE_LITERAL:
    {
      node_string_s *ss = node_string (arg);
      char *sv = node_string_value (ss);
      printf ("%s", sv);
    }
    break;
  default:
    break;
  }
  
  return rc;
}
