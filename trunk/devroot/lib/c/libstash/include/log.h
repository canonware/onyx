/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1996-1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 62 $
 * Last modified: $Date: 1998-05-01 16:48:14 -0700 (Fri, 01 May 1998) $
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
