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

#define log_new _CW_NS_STASH(log_new)
cw_log_t *
log_new();

#define log_delete _CW_NS_STASH(log_delete)
void
log_delete(cw_log_t * a_log);

#define log_set_logfile _CW_NS_STASH(log_set_logfile)
cw_bool_t
log_set_logfile(cw_log_t * a_log, char * a_logfile, cw_bool_t a_overwrite);

#define log_printf _CW_NS_STASH(log_printf)
int
log_printf(cw_log_t * a_log, char * a_format, ...);

#define log_eprintf _CW_NS_STASH(log_eprintf)
int
log_eprintf(cw_log_t * a_log, 
	    char * a_filename, /* Optional, pass NULL if not used. */
	    int a_line_num, /* Only used if (a_filename != NULL) */
	    char * a_func_name, /* Optional, pass NULL if not used. */
	    char * a_format, 
	    ...);

#define log_nprintf _CW_NS_STASH(log_nprintf)
int
log_nprintf(cw_log_t * a_log,
	    cw_uint32_t a_size,
	    char * a_format,
	    ...);

#define log_lprintf _CW_NS_STASH(log_lprintf)
int
log_lprintf(cw_log_t * a_log, char * a_format, ...);

#define log_leprintf _CW_NS_STASH(log_leprintf)
int
log_leprintf(cw_log_t * a_log,
	     char * a_filename, /* Optional, pass NULL if not used. */
	     int a_line_num, /* Only used if (a_filename != NULL) */
	     char * a_func_name, /* Optional, pass NULL if not used. */
	     char * a_format, 
	     ...);

#define log_print_uint64 _CW_NS_STASH(log_print_uint64)
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

#if (defined(_STASH_DBG) || defined(_STASH_DEBUG))
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
