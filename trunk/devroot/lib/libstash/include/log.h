/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 156 $
 * Last modified: $Date: 1998-07-29 16:59:01 -0700 (Wed, 29 Jul 1998) $
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#ifndef _LOG_H_
#define _LOG_H_

typedef struct cw_log_s cw_log_t;

#define log_new _CW_NS_ANY(log_new)
#define log_delete _CW_NS_ANY(log_delete)
#define log_set_logfile _CW_NS_ANY(log_set_logfile)
#define log_printf _CW_NS_ANY(log_printf)
#define log_eprintf _CW_NS_ANY(log_eprintf)
#define log_lprintf _CW_NS_ANY(log_eprintf)
#define log_leprintf _CW_NS_ANY(log_leprintf)
#define log_print_uint64 _CW_NS_ANY(log_print_uint64)

cw_log_t * log_new();
void log_delete(cw_log_t * a_log_o);

cw_bool_t log_set_logfile(cw_log_t * a_log_o,
			  char * a_logfile,
			  cw_bool_t a_overwrite);
int log_printf(cw_log_t * a_log_o, char * a_format, ...);
int log_eprintf(cw_log_t * a_log_o, 
		char * a_filename, /* Optional, pass NULL if not used. */
		int a_line_num, /* Only used if (a_filename != NULL) */
		char * a_func_name, /* Optional, pass NULL if not used. */
		char * a_format, 
		...);
int log_lprintf(cw_log_t * a_log_o, 
		char * a_format, 
		...);
int log_leprintf(cw_log_t * a_log_o, 
		 char * a_filename, /* Optional, pass NULL if not used. */
		 int a_line_num, /* Only used if (a_filename != NULL) */
		 char * a_func_name, /* Optional, pass NULL if not used. */
		 char * a_format, 
		 ...);
char * log_print_uint64(cw_uint64_t a_val, cw_uint32_t a_base, char * a_buf);

/* 
 * My version of assert().  It's a bit prettier and cleaner, but the same idea.
 */

#define _cw_error(a) \
  { \
    log_eprintf(g_log_o, __FILE__, __LINE__, NULL, "Error: %s\n", a); \
    abort(); \
  }

#ifdef _CW_DEBUG
#define _cw_assert(a) \
  { \
    if (!(a)) \
      { \
        log_eprintf(g_log_o, __FILE__, __LINE__, NULL, \
		    "Failed assertion: \"%s\"\n", #a); \
        abort(); \
      } \
  }

#define _cw_marker(a) \
  { \
    log_eprintf(g_log_o, __FILE__, __LINE__, NULL, "%s\n", a); \
  }

/* Macro to do the drudgery of checking whether a pointer is null. */
#define _cw_check_ptr(x) \
  { \
    if ((x) == NULL) \
      { \
	log_eprintf(g_log_o, __FILE__, __LINE__, NULL, \
		    "%s is a NULL pointer\n", #x); \
        abort(); \
      } \
  }
#else
#  define _cw_assert(a)
#  define _cw_marker(a)
#  define _cw_check_ptr(a)
#endif

#endif /* _LOG_H_ */
