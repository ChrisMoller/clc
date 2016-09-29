#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#define _GNU_SOURCE
#include <math.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <gsl/gsl_complex.h>

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
  gsl_complex z = **((gsl_complex **) (args[0]));
  double rz = GSL_REAL (z);
  double iz = GSL_IMAG (z);
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
  gsl_complex z = **((gsl_complex **) (args[0]));
  double rz  = GSL_REAL (z);
  double iz  = GSL_IMAG (z);
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

static int
print_complex_arginfo (const struct printf_info *info,
		       size_t n,
		       int *argtypes, int *size)
{
  if (n > 0) argtypes[0] = PA_POINTER;
  return 1;
}

void
print_init ()
{
  register_printf_specifier ('R', print_complex_rect, print_complex_arginfo);
  register_printf_specifier ('P', print_complex_polar, print_complex_arginfo);
}
