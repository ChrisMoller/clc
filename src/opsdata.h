  ENTRY ("+", SYM_PLUS,    gsl_complex_add,	 NULL,
       "add or catenate", NULL),
  ENTRY ("-", SYM_MINUS,   gsl_complex_sub,   gsl_complex_negative,
	 "subtract", "negate"),
  ENTRY ("*", SYM_STAR,    gsl_complex_mul,   gsl_complex_conjugate,
	 "multiply", "complex conjugate"),
  ENTRY ("/", SYM_SLASH,   gsl_complex_div,   gsl_complex_inverse,
	 "divide", "invert"),
  ENTRY ("^", SYM_HAT,     gsl_complex_pow,   gsl_complex_exp,
	 "power", "exp"),
  ENTRY (",", SYM_COMMA,   clc_catenate,      NULL,
	 "power", "exp"),
  ENTRY ("sin", SYM_SIN,     NULL,              gsl_complex_sin,
	 NULL, "sin"),
  ENTRY ("cos", SYM_COS,     NULL,              gsl_complex_cos,
	 NULL, "cos"),
  ENTRY ("tan", SYM_TAN,     NULL,              gsl_complex_tan,
	 NULL, "tan"),
  ENTRY ("asin", SYM_ASIN,    NULL,              gsl_complex_arcsin,
	 NULL, "asin"),
  ENTRY ("acos", SYM_ACOS,    NULL,              gsl_complex_arccos,
	 NULL, "acos"),
  ENTRY ("atan", SYM_ATAN,    NULL,              gsl_complex_arctan,
	 NULL, "atan"),
  ENTRY ("sinh", SYM_SINH,    NULL,              gsl_complex_sinh,
	 NULL, "sinh"),
  ENTRY ("cosh", SYM_COSH,    NULL,              gsl_complex_cosh,
	 NULL, "cosh"),
  ENTRY ("tanh", SYM_TANH,    NULL,              gsl_complex_tanh,
	 NULL, "tanh"),
  ENTRY ("asinh", SYM_ASINH,   NULL,              gsl_complex_arcsinh,
	 NULL, "asinh"),
  ENTRY ("acosh", SYM_ACOSH,   NULL,              gsl_complex_arccosh,
	 NULL, "acosh"),
  ENTRY ("atanh", SYM_ATANH,   NULL,              gsl_complex_arctanh,
	 NULL, "atanh"),
  ENTRY ("ln", SYM_LN,      NULL,              gsl_complex_log,
	 NULL, "ln"),
  ENTRY ("log", SYM_LOG,     gsl_complex_log_b, gsl_complex_log10,
	 "log base n", "log base 10"),
  ENTRY (NULL, SYM_COUNT,   NULL, 		 NULL, NULL, NULL)
