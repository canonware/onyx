/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (C) 1996-1997 Jason Evans <jasone@canonware.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You can get a copy of the GNU General Public License at
 * http://www.fsf.org/copyleft/gpl.html, or by writing to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 2 $
 * Last modified: $Date: 1997-12-07 17:34:03 -0800 (Sun, 07 Dec 1997) $
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
