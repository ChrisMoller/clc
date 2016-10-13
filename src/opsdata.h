/***
    TODO:
	comparisons/conditionals
***/


  ENTRY ("+", SYM_PLUS,		gsl_complex_add,	NULL,
	 2, 0, "add or catenate", NULL),
  ENTRY ("-", SYM_MINUS,	gsl_complex_sub,	gsl_complex_negative,
	 2, 1, "subtract", "negate"),
  ENTRY ("*", SYM_STAR,		gsl_complex_mul,	NULL,
	 2, 1, "multiply", NULL),
  ENTRY ("/", SYM_SLASH,	gsl_complex_div,	gsl_complex_inverse,
	 2, 1, "divide", "invert"),
  ENTRY ("^", SYM_HAT,		gsl_complex_pow,	gsl_complex_exp,
	 2, 1, "power", "exp"),
  ENTRY ("?", SYM_QMARK,	NULL,			clc_random,
	 2, 1, "NULL",  "magnitude"),
  ENTRY ("|", SYM_BAR,		NULL,			clc_complex_abs,
	 2, 1, "NULL",  "magnitude"),
  ENTRY ("<", SYM_LT,		NULL,			clc_complex_arg,
	 2, 1, "NULL",  "phase"),
  ENTRY ("real", SYM_REAL,	NULL,			clc_complex_real,
	 2, 1, "NULL",  "real"),
  ENTRY ("imag", SYM_IMAG,	NULL,			clc_complex_imag,
	 2, 1, "NULL",  "imag"),
  ENTRY ("conj", SYM_CONJ,	NULL,			gsl_complex_conjugate,
	 2, 1, "NULL",  "complex conjugate"),

  ENTRY ("/+", SYM_SLASHPLUS,	NULL,			clc_sigma,
	 0, 1, NULL, "Sigma"),
  ENTRY ("/*", SYM_SLASHSTAR,	NULL,			clc_pi,
	 0, 1, NULL, "Pi"),
  ENTRY (",", SYM_COMMA,	clc_catenate,		clc_ravel,
	 2, 1, "catenate", "ravel"),
  ENTRY ("=", SYM_EQUAL,	clc_assign,		NULL,
	 2, 1, "assign", NULL),
  ENTRY ("<>", SYM_LTGT,	clc_reshape,		clc_shape,
	 2, 1, "reshape", "shape"),
  ENTRY ("><", SYM_GTLT,	clc_matrix_mul,		clc_transpose,
	 2, 1, "matrix multiply",      "transpose"),
  ENTRY ("plot", SYM_PLOT,	NULL,			clc_plot,
	 0, 1, NULL, "plot"),
  ENTRY ("print", SYM_PRINT,	NULL,			clc_print,
	 0, 1, NULL, "plot"),
  ENTRY ("file", SYM_FILE,	NULL,			clc_file,
	 0, 1, NULL, "file"),
  ENTRY ("sqrt", SYM_SQRT,	NULL,			gsl_complex_sqrt,
	 0, 1, NULL, "sin"),
  ENTRY ("::", SYM_COLCOL,	clc_range,		clc_index,
	 2, 1, "reshape", "shape"),
  ENTRY ("sin", SYM_SIN,	NULL,			gsl_complex_sin,
	 0, 1, NULL, "sin"),
  ENTRY ("cos", SYM_COS,	NULL,			gsl_complex_cos,
	 0, 1, NULL, "cos"),
  ENTRY ("tan", SYM_TAN,	NULL,			gsl_complex_tan,
	 0, 1, NULL, "tan"),
  ENTRY ("asin", SYM_ASIN,	NULL,			gsl_complex_arcsin,
	 0, 1, NULL, "asin"),
  ENTRY ("acos", SYM_ACOS,	NULL,			gsl_complex_arccos,
	 0, 1, NULL, "acos"),
  ENTRY ("atan", SYM_ATAN,	NULL,			gsl_complex_arctan,
	 0, 1, NULL, "atan"),
  ENTRY ("sinh", SYM_SINH,	NULL,			gsl_complex_sinh,
	 0, 1, NULL, "sinh"),
  ENTRY ("cosh", SYM_COSH,	NULL,			gsl_complex_cosh,
	 0, 1, NULL, "cosh"),
  ENTRY ("tanh", SYM_TANH,	NULL,			gsl_complex_tanh,
	 0, 1, NULL, "tanh"),
  ENTRY ("asinh", SYM_ASINH,	NULL,			gsl_complex_arcsinh,
	 0, 1, NULL, "asinh"),
  ENTRY ("acosh", SYM_ACOSH,	NULL,			gsl_complex_arccosh,
	 0, 1, NULL, "acosh"),
  ENTRY ("atanh", SYM_ATANH,	NULL,			gsl_complex_arctanh,
	 0, 1, NULL, "atanh"),
  ENTRY ("ln", SYM_LN,		NULL,			gsl_complex_log,
	 0, 1, NULL, "ln"),
  ENTRY ("log", SYM_LOG,	gsl_complex_log_b,	gsl_complex_log10,
	 2, 1, "log base n", "log base 10"),
  ENTRY (NULL, SYM_COUNT,	NULL,			NULL,
	 0, 0, NULL, NULL)
