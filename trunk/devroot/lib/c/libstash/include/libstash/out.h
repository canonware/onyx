/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Public interface for the out (formatted output) class, and the spec helper
 * functions.
 *
 * The out class provides functionality similar to the printf family of
 * functions, with the additional capability of dynamically adding handlers for
 * non-builtin types.
 *
 * The syntax of formatting specifiers is significantly different than what
 * printf uses, since arbitrary flags must be supported.  Formatting specifiers
 * are delimited by `[' and `]'.  Following are some examples of formatting
 * strings, followed by explanations.
 *
 * "[["
 * Print a `[' character.  Due to the way C strings and `\' protection work,
 * it is necessary to use a different escape character (`[').
 * 
 * "[s]"
 * Print a string.
 *
 * "[s|w:10]"
 *  Print a string, padded to be at least 10 bytes long.
 *
 * "[i32|b:16|p:0|w:8]"
 * Print a 32 bit integer in base 16.  Pad the output to 8 bytes, using `0' for
 * the padding character.
 *
 * As can be seen above, flags are specified as name/value pairs.  Each name and
 * value is separated by `:', and name/value pairs are separated from each other
 * (and the type specifier) by `|'.  Names and values can be of arbitrary
 * length, and can contain any characters except (in some cases) `]', `|', `:',
 * and `\0'.  The parser may let these characters slip through in some cases,
 * but such behavior should not be relied on.  Some or all of the following
 * flags are supported for the builtin types:
 *
 * "w" : Minimum number of bytes of output.
 
 * "j" : Justification.  Legal values:
 *         "r" : Right.
 *         "l" : Left.
 *         "c" : Center.
 *
 * "p" : Padding character.
 *
 * "b" : Numerical base.  Legal bases are (2 <= base <= 36).
 *
 * "s" : Signed/unsigned.  Legal values:
 *         "u" : Unsigned.
 *         "s" : Signed.
 *
 * "+" : Show sign.  Legal values:
 *         "-" : Only print sign if output is negative.
 *         "+" : Always print sign.
 *
 * Below is a matrix of the builtin output types, supported flags, and flag
 * defaults:
 *
 * Field key:
 * +------------+
 * | Supported? |
 * | Default    |
 * +------------+
 *
 * Type     | w   | j     | p   | b   | s        | +             |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * int8     | Yes | Yes   | Yes | Yes | Yes      | Yes           |
 * i8       | Fit | Right | ` ' | 10  | Unsigned | Negative only |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * int16    | Yes | Yes   | Yes | Yes | Yes      | Yes           |
 * i16      | Fit | Right | ` ' | 10  | Unsigned | Negative only |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * int32    | Yes | Yes   | Yes | Yes | Yes      | Yes           |
 * i32      | Fit | Right | ` ' | 10  | Unsigned | Negative only |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * int64    | Yes | Yes   | Yes | Yes | Yes      | Yes           |
 * i64      | Fit | Right | ` ' | 10  | Unsigned | Negative only |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * char     | Yes | Yes   | Yes | No  | No       | No            |
 * c        | Fit | Right | ` ' |     |          |               |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * string   | Yes | Yes   | Yes | No  | No       | No            |
 * s        | Fit | Right | ` ' |     |          |               |
 * ---------+-----+-------+-----+-----+----------+---------------+
 * pointer  | Yes | Yes   | Yes | Yes | Yes      | Yes           |
 * p        | Fit | Right | ` ' | 16  | Unsigned | Negative only |
 * ---------+-----+-------+-----+-----+----------+---------------+
 *
 ****************************************************************************/

/*
 * Typedef's to allow easy function pointer passing.
 */

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_format : Pointer to a formatting specification.
 *
 * a_format_len : Length of a_format.
 *
 * a_arg : Pointer to variable to be output.
 *
 * <<< Output(s) >>>
 *
 * retval : Number of bytes of output that the corresponding rendering function
 *          needs, or -1.
 *          -1 : Memory error.
 *          -2 : Other error.
 *
 * <<< Description >>>
 *
 * Return the number of bytes needed to output a_arg, given the specification
 * pointed to by a_format.
 *
 ****************************************************************************/
typedef cw_sint32_t
cw_out_metric_t(const char * a_format, cw_uint32_t a_format_len,
		const void * a_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_format : Pointer to a formatting specification.
 *
 * a_format_len : Length of a_format.
 *
 * a_arg : Pointer to variable to be output.
 *
 * r_str : Pointer to output string.
 *
 * <<< Output(s) >>>
 *
 * retval : r_str, or NULL.
 *          NULL : Memory allocation error.
 *
 * *r_str : Formatted output.
 *
 * <<< Description >>>
 *
 * Print a_arg, given the specification pointed to by a_format.
 *
 ****************************************************************************/
typedef char *
cw_out_render_t(const char * a_format, cw_uint32_t a_format_len,
		const void * a_arg, char * r_str);

/* Pseudo-opaque types. */
typedef struct cw_out_s cw_out_t;
typedef struct cw_out_ent_s cw_out_ent_t;

struct cw_out_s
{
#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
  cw_uint32_t magic;
#endif

  cw_bool_t is_malloced;
  cw_sint32_t fd;

#ifdef _CW_REENTRANT
  cw_mtx_t lock;
#endif

  cw_uint32_t nextensions;
  cw_out_ent_t * extensions;
};

struct cw_out_ent_s
{
  char * type;
  cw_uint32_t size;
  cw_out_metric_t * metric_func;
  cw_out_render_t * render_func;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to space for an out, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to an out, or NULL.
 *          NULL : Memory allocation error.  Can only occur if (NULL == a_out).
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_out_t *
out_new(cw_out_t * a_out);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
out_delete(cw_out_t * a_out);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_type : Pointer to a NULL-terminated string that represents a data type
 *          specifier.
 *
 * a_size : sizeof(<data type>).  In almost all cases this will be
 *          sizeof(void *).  a_size must be 1, 2, 4, or 8.
 *
 * a_metric_func : Pointer to a metric function.
 *
 * a_render_func : Pointer to a rendering function.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Register a new type with a_out, so that non-builtin type specifiers can be
 * embedded in formatting strings.
 *
 ****************************************************************************/
cw_bool_t
out_register(cw_out_t * a_out,
	     const char * a_type,
	     cw_uint32_t a_size,
	     cw_out_metric_t * a_metric_func,
	     cw_out_render_t * a_render_func);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_a : Pointer to an out.
 *
 * a_b : Pointer to an out.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == error.
 *          TRUE : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Merge a_b's extended type handlers into a_a.
 *
 ****************************************************************************/
cw_bool_t
out_merge(cw_out_t * a_a, cw_out_t * a_b);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : File descriptor number.
 *
 * <<< Description >>>
 *
 * Return a_out's default file descriptor.
 *
 ****************************************************************************/
cw_sint32_t 
out_get_default_fd(cw_out_t * a_out);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set a_out's default file descriptor number.
 *
 ****************************************************************************/
void
out_set_default_fd(cw_out_t * a_out, cw_sint32_t a_fd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_out's default file descriptor (2 if (NULL == a_out)).
 * 
 * <<< Description >>>
 *
 * Print formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put(cw_out_t * a_out, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_file_name : Pointer to a string that represents the source file name,
 *               or NULL.
 *
 * a_line_num : Source file line number.  Ignored if (NULL == a_file_name).
 *
 * a_func_name : Pointer to a string that represents the source function name,
 *               or NULL.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_out's default file descriptor (2 if (NULL == a_out)).
 *
 * <<< Description >>>
 *
 * Print formatted output, with optional
 * "At <file>, line <line>: <function>(): "
 * prepended to the output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_e(cw_out_t * a_out,
	  const char * a_file_name,
	  cw_uint32_t a_line_num,
	  const char * a_func_name,
	  const char * a_format,
	  ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_file_name : Pointer to a string that represents the source file name,
 *               or NULL.
 *
 * a_line_num : Source file line number.  Ignored if (NULL == a_file_name).
 *
 * a_func_name : Pointer to a string that represents the source function name,
 *               or NULL.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_out's default file descriptor (2 if (NULL == a_out)).
 *
 * <<< Description >>>
 *
 * Print formatted output, with "[yy/mm/dd hh:mm:ss (zz)]: "
 * and optional "At <file>, line <line>: <function>(): " prepended to the
 * output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_le(cw_out_t * a_out,
	   const char * a_file_name,
	   cw_uint32_t a_line_num,
	   const char * a_func_name,
	   const char * a_format,
	   ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_size : Maximum number of characters out output.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_out's default file descriptor (2 if (NULL == a_out)).
 *
 * <<< Description >>>
 *
 * Print at most a_size bytes of formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_n(cw_out_t * a_out, cw_uint32_t a_size,
	  const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_fd.
 *
 * <<< Description >>>
 *
 * Print formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_f(cw_out_t * a_out, cw_sint32_t a_fd, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * a_file_name : Pointer to a string that represents the source file name,
 *               or NULL.
 *
 * a_line_num : Source file line number.  Ignored if (NULL == a_file_name).
 *
 * a_func_name : Pointer to a string that represents the source function name,
 *               or NULL.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_fd.
 *
 * <<< Description >>>
 *
 * Print formatted output, with optional
 * "At <file>, line <line>: <function>(): "
 * prepended to the output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_fe(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_file_name,
	   cw_uint32_t a_line_num,
	   const char * a_func_name,
	   const char * a_format,
	   ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * a_file_name : Pointer to a string that represents the source file name,
 *               or NULL.
 *
 * a_line_num : Source file line number.  Ignored if (NULL == a_file_name).
 *
 * a_func_name : Pointer to a string that represents the source function name,
 *               or NULL.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_fd.
 *
 * <<< Description >>>
 *
 * Print formatted output, with "[yy/mm/dd hh:mm:ss (zz)]: "
 * and optional "At <file>, line <line>: <function>(): " prepended to the
 * output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_fle(cw_out_t * a_out, cw_sint32_t a_fd,
	    const char * a_file_name,
	    cw_uint32_t a_line_num,
	    const char * a_func_name,
	    const char * a_format,
	    ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * a_size : Maximum number of characters out output.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_fd.
 *
 * <<< Description >>>
 *
 * Print at most a_size bytes of formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_fn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	   const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_fd : File descriptor number.
 *
 * a_format : Pointer to a formatting string.
 *
 * a_p : Variable argument list.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * Output printed to a_fd.
 *
 * <<< Description >>>
 *
 * Print formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_fv(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_format, va_list a_p);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_str : Pointer to output string.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * a_str : Formatted output.
 *
 * <<< Description >>>
 *
 * Print formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_s(cw_out_t * a_out, char * a_str, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * r_str : Pointer to a string pointer.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * *r_str : Formatted output, or undefined if error.
 *
 * <<< Description >>>
 *
 * Allocate space, then print formatted output to it.
 *
 ****************************************************************************/
cw_sint32_t
out_put_sa(cw_out_t * a_out, char ** r_str, const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_str : Pointer to output string.
 *
 * a_size : Maximum number of characters out output.
 *
 * a_format : Pointer to a formatting string.
 *
 * ... : Arguments that correspond to the specifiers in a_format.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * a_str : Formatted output.
 *
 * <<< Description >>>
 *
 * Print at most a_size bytes of formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_sn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	   const char * a_format, ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_str : Pointer to output string.
 *
 * a_format : Pointer to a formatting string.
 *
 * a_p : Variable argument list.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * a_str : Formatted output.
 *
 * <<< Description >>>
 *
 * Print formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_sv(cw_out_t * a_out, char * a_str,
	   const char * a_format, va_list a_p);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * *r_str : Formatted output, or undefined if error.
 *
 * a_format : Pointer to a formatting string.
 *
 * a_p : Variable argument list.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * *r_str : Formatted output, or undefined if error.
 *
 * <<< Description >>>
 *
 * Allocate space, then print formatted output to it.
 *
 ****************************************************************************/
cw_sint32_t
out_put_sva(cw_out_t * a_out, char ** r_str,
	    const char * a_format, va_list a_p);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_out : Pointer to an out, or NULL.
 *
 * a_str : Pointer to output string.
 *
 * a_size : Maximum number of characters out output.
 *
 * a_format : Pointer to a formatting string.
 *
 * a_p : Variable argument list.
 *
 * <<< Output(s) >>>
 *
 * retval : >= 0 : Number of bytes output
 *            -1 : Memory allocation error.
 *            -2 : Parse error.
 *
 * a_str : Formatted output.
 *
 * <<< Description >>>
 *
 * Print at most a_size bytes of formatted output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	    const char * a_format, va_list a_p);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_spec : Pointer to a specifier string.
 *
 * a_spec_len : Length of a_spec.
 *
 * r_val : Pointer to a string pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Length of type value string.
 *
 * *r_val : Pointer to type value string.
 *
 * <<< Description >>>
 *
 * Return the length of the type value string, and set *r_val to point to it.
 *
 ****************************************************************************/
cw_sint32_t
spec_get_type(const char * a_spec, cw_uint32_t a_spec_len, const char ** r_val);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_spec : Pointer to a specifier string.
 *
 * a_spec_len : Length of a_spec.
 *
 * a_name : Pointer to a NULL-terminated name.
 *
 * r_val : Pointer to a string pointer.
 *
 * <<< Output(s) >>>
 *
 * retval : Length of value string, or -1.
 *          -1 : No such name.
 *
 * *r_val : Pointer to value string.
 *
 * <<< Description >>>
 *
 * Return the length of the value string that corresponds to a_spec, and set
 * *r_val to point to it.
 *
 ****************************************************************************/
cw_sint32_t
spec_get_val(const char * a_spec, cw_uint32_t a_spec_len,
	     const char * a_name, const char ** r_val);
