/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************/

typedef struct cw_log_s cw_log_t;

/****************************************************************************
 * <<< Description >>>
 *
 * log constructor.
 *
 ****************************************************************************/
#define log_new _CW_NS_LIBSTASH(log_new)
cw_log_t *
log_new();

/****************************************************************************
 * <<< Description >>>
 *
 * log destructor.
 *
 ****************************************************************************/
#define log_delete _CW_NS_LIBSTASH(log_delete)
void
log_delete(cw_log_t * a_log);

/****************************************************************************
 * <<< Description >>>
 *
 * Opens file for logging.  If another file is already being used, it is
 * first closed.
 *
 ****************************************************************************/
#define log_set_logfile _CW_NS_LIBSTASH(log_set_logfile)
cw_bool_t
log_set_logfile(cw_log_t * a_log, const char * a_logfile,
		cw_bool_t a_overwrite);

/****************************************************************************
 * <<< Description >>>
 *
 * Run-of-the-mill printf()-alike.
 *
 ****************************************************************************/
#define log_printf _CW_NS_LIBSTASH(log_printf)
int
log_printf(cw_log_t * a_log, const char * a_format, ...);

/****************************************************************************
 * <<< Description >>>
 *
 * Optional arguments prepend filename, line number, and function name.
 * Otherwise, this still acts like printf().
 *
 ****************************************************************************/
#define log_eprintf _CW_NS_LIBSTASH(log_eprintf)
int
log_eprintf(cw_log_t * a_log, 
	    const char * a_filename, /* Optional, pass NULL if not used. */
	    int a_line_num, /* Only used if (a_filename != NULL) */
	    const char * a_func_name, /* Optional, pass NULL if not used. */
	    const char * a_format, 
	    ...);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 *
 *
 * <<< Output(s) >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
#define log_nprintf _CW_NS_LIBSTASH(log_nprintf)
int
log_nprintf(cw_log_t * a_log,
	    cw_uint32_t a_size,
	    const char * a_format,
	    ...);

/****************************************************************************
 * <<< Description >>>
 *
 * printf()-alike that prepends log message foo.
 *
 ****************************************************************************/
#define log_lprintf _CW_NS_LIBSTASH(log_lprintf)
int
log_lprintf(cw_log_t * a_log, const char * a_format, ...);

/****************************************************************************
 * <<< Description >>>
 *
 * Prepended log message foo.  'Optional' arguments prepend filename, line
 * number, and function name.
 *
 ****************************************************************************/
#define log_leprintf _CW_NS_LIBSTASH(log_leprintf)
int
log_leprintf(cw_log_t * a_log,
	     const char * a_filename, /* Optional, pass NULL if not used. */
	     int a_line_num, /* Only used if (a_filename != NULL) */
	     const char * a_func_name, /* Optional, pass NULL if not used. */
	     const char * a_format,
	     ...);

/****************************************************************************
 * <<< Arguments >>>
 *
 * a_val : Value to convert to a string.
 * a_base : Number base to convert to.
 * a_buf : A buffer of at least 65 bytes for base 2 conversion, 21 bytes
 *         for base 10 conversion, and 17 bytes for base 16 conversion.
 *
 * <<< Description >>>
 *
 * Converts a_val to a number string in base a_base and puts the result
 * into *a_buf.
 *
 ****************************************************************************/
#define log_print_uint64 _CW_NS_LIBSTASH(log_print_uint64)
char *
log_print_uint64(cw_uint64_t a_val, cw_uint32_t a_base, char * a_buf);

/* 
 * My version of assert().  It's a bit prettier and cleaner, but the same idea.
 */

#define _cw_error(a) \
  { \
    log_eprintf(g_log, __FILE__, __LINE__, __FUNCTION__, "Error: %s\n", a); \
    abort(); \
  }

#if (defined(_LIBSTASH_DBG) || defined(_LIBSTASH_DEBUG))
#define _cw_assert(a) \
  { \
    if (!(a)) \
      { \
        log_eprintf(g_log, __FILE__, __LINE__, __FUNCTION__, \
		    "Failed assertion: \"%s\"\n", #a); \
        abort(); \
      } \
  }

#define _cw_marker(a) \
  { \
    log_eprintf(g_log, __FILE__, __LINE__, __FUNCTION__, "%s\n", a); \
  }

/* Macro to do the drudgery of checking whether a pointer is null. */
#define _cw_check_ptr(x) \
  { \
    if ((x) == NULL) \
      { \
	log_eprintf(g_log, __FILE__, __LINE__, __FUNCTION__, \
		    "%s is a NULL pointer\n", #x); \
        abort(); \
      } \
  }
#else
#  define _cw_assert(a)
#  define _cw_marker(a)
#  define _cw_check_ptr(a)
#endif
