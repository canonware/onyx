/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 86 $
 * Last modified: $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
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

#define log_new _CW_NS_CMN(log_new)
#define log_delete _CW_NS_CMN(log_delete)
#define log_set_logfile _CW_NS_CMN(log_set_logfile)
#define log_printf _CW_NS_CMN(log_printf)
#define log_eprintf _CW_NS_CMN(log_eprintf)
#define log_lprintf _CW_NS_CMN(log_eprintf)
#define log_leprintf _CW_NS_CMN(log_leprintf)

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

/* 
 * My version of assert().  It's a bit prettier and cleaner, but the same idea.
 */

#define _cw_error(a) \
  { \
    log_eprintf(g_log_o, __FILE__, __LINE__, NULL, "Error: %s\n", a); \
    abort(); \
  }

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

#endif /* _LOG_H_ */
