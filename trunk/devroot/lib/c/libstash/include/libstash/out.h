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
 ****************************************************************************/

/*
 * Typedef's to allow easy function pointer passing.
 */
typedef cw_sint32_t
cw_out_metric_t(const char * a_format, cw_uint32_t a_format_len,
		const void * a_arg);

typedef char *
cw_out_render_t(const char * a_format, cw_uint32_t a_format_len,
		const void * a_arg, char * r_str);

/* Maximum type string length, including NULL termination. */
#define _LIBSTASH_OUT_MAX_TYPE 16

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
  char type[_LIBSTASH_OUT_MAX_TYPE];
  cw_uint32_t len;
  cw_uint32_t size;
  cw_out_metric_t * metric_func;
  cw_out_render_t * render_func;
};

cw_out_t *
out_new(cw_out_t * a_out);

void
out_delete(cw_out_t * a_out);

cw_bool_t
out_register(cw_out_t * a_out,
	     const char * a_type,
	     cw_uint32_t a_size,
	     cw_out_metric_t * a_metric_func,
	     cw_out_render_t * a_render_func);

cw_bool_t
out_merge(cw_out_t * a_a, cw_out_t * a_b);

cw_sint32_t 
out_get_default_fd(cw_out_t * a_out);

void
out_set_default_fd(cw_out_t * a_out, cw_sint32_t a_fd);

cw_sint32_t
out_put(cw_out_t * a_out, const char * a_format, ...);

cw_sint32_t
out_put_e(cw_out_t * a_out,
	  const char * a_file_name,
	  cw_uint32_t a_line_num,
	  const char * a_func_name,
	  const char * a_format,
	  ...);

cw_sint32_t
out_put_l(cw_out_t * a_out, const char * a_format, ...);

cw_sint32_t
out_put_le(cw_out_t * a_out,
	   const char * a_file_name,
	   cw_uint32_t a_line_num,
	   const char * a_func_name,
	   const char * a_format,
	   ...);

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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * Print formatted output, with "[yy/mm/dd hh:mm:ss (zz)]: " prepended to the
 * output.
 *
 ****************************************************************************/
cw_sint32_t
out_put_fl(cw_out_t * a_out, cw_sint32_t a_fd, const char * a_format, ...);

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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_format : Pointer to a formatting specifier string.
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
 * a_name_len : Length of a_name.
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
	     const char * a_name,  cw_uint32_t a_name_len,
	     const char ** r_val);
