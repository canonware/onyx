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
 * Current revision: $Revision: 7 $
 * Last modified: $Date: 1998-01-16 00:20:54 -0800 (Fri, 16 Jan 1998) $
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

int set_g_error(char * arg_format, ...);
char * get_g_error();

int log_init(char * arg_logfile); /* Pass NULL to use stderr. */
int log_close();
int lprintf(char * arg_format, ...);
int leprintf(char * arg_filename, /* Optional, pass NULL if not used. */
	     int arg_line_num, /* Only used if (arg_filename != NULL) */
	     char * arg_func_name, /* Optional, pass NULL if not used. */
	     char * arg_format, 
	     ...);

/* 
 * My version of assert().  It's a bit prettier and cleaner, but the same idea.
 */

#define _cw_error(a) \
  { \
    leprintf(__FILE__, __LINE__, NULL, "Error: %s\n", a); \
    log_close(); \
    abort(); \
  }

#define _cw_assert(a) \
  { \
    if (!(a)) \
      { \
        leprintf(__FILE__, __LINE__, NULL, "Failed assertion: \"%s\"\n", #a); \
	log_close(); \
        abort(); \
      } \
  }

#define _cw_marker(a) \
  { \
    leprintf(__FILE__, __LINE__, NULL, a); \
  }

/* Macro to do the drudgery of checking whether a pointer is null. */
#define _cw_check_ptr(a) \
  { \
    if (a == NULL) \
      { \
	leprintf(__FILE__, __LINE__, NULL, "%s is a NULL pointer\n", #a); \
	log_close(); \
        abort(); \
      } \
  }

#endif /* _LOG_H_ */
