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
 * Current revision: $Revision: 4 $
 * Last modified: $Date: 1997-12-14 22:01:41 -0800 (Sun, 14 Dec 1997) $
 *
 * Description: Functions for logging and error printing.
 *              
 ****************************************************************************
 */

#define _INC_STDARG_H_
/* #define _INC_STRING_H_ */
#include <inc_common.h>

#define G_ERROR_BUFF_SIZE 512

/* Pointer to error string used by set_g_error. */
char g_error[G_ERROR_BUFF_SIZE] = "";

/* Used to store file handle used for logging. */
FILE * g_log_fp = NULL;

int
set_g_error(char * arg_format, ...)
{
  va_list ap;
  int retval;

  va_start(ap, arg_format);
  retval = vsnprintf(g_error, G_ERROR_BUFF_SIZE, arg_format, ap);
  va_end(ap);

  return retval;
}

char *
get_g_error()
{
  return g_error;
}

int
log_init(char * arg_logfile)
{
  int retval = 0;

  if (g_log_fp != NULL)
    {
      retval = 1;
      set_g_error("Logging is already on");
    }
  else if (arg_logfile == NULL)
    {
      g_log_fp = stderr;
    }
  else
    {
      g_log_fp = fopen(arg_logfile, "a+");
      if (g_log_fp == NULL)
	{
	  g_log_fp = fopen(arg_logfile, "w");
	  if (g_log_fp == NULL)
	    {
	      retval = 1;
	      set_g_error("Unable to open \"%s\"", arg_logfile);
	    }
	}
    }

  return retval;
}

int
log_close()
{
  int retval = 0;

  if (g_log_fp != NULL)
    {
      if (g_log_fp != stderr) /* Don't want to close stderr. */
	{
	  retval = fclose(g_log_fp);
	  if (retval)
	    {
	      set_g_error("Error closing logfile");
	    }
	}
      g_log_fp = NULL; /* log_init() depends on this being NULL if invalid. */
    }
  else /* The logfile is already closed. */
    {
      set_g_error("Logfile is already closed");
      retval = 1;
    }

  return retval;
}

int
lprintf(char * arg_format, ...)
{
  va_list ap;
  int retval;

  if (g_log_fp != NULL)
    {
      va_start(ap, arg_format);
      retval = vfprintf(g_log_fp, arg_format, ap);
      va_end(ap);
    }
  else
    {
      /* Use stderr. */
      va_start(ap, arg_format);
      retval = vfprintf(stderr, arg_format, ap);
      va_end(ap);
    }

  return retval;
}

int
leprintf(char * arg_filename, 
	 int arg_line_num, 
	 char * arg_func_name,
	 char * arg_format, 
	 ...)
{
  va_list ap;
  int retval;

  if (g_log_fp != NULL)
    {
      if (arg_filename != NULL)
	{
	  fprintf(g_log_fp, 
		  "At %s, line %d: ", 
		  arg_filename, 
		  arg_line_num);
	}
      if (arg_func_name != NULL)
	{
	  fprintf(g_log_fp, 
		  "%s(): ", 
		  arg_func_name);
	}

      va_start(ap, arg_format);
      retval = vfprintf(g_log_fp, arg_format, ap);
      va_end(ap);
    }
  else
    {
      /* Use stderr. */
      if (arg_filename != NULL)
	{
	  fprintf(stderr, 
		  "At %s, line %d: ",
		  arg_filename, 
		  arg_line_num);
	}
      if (arg_func_name != NULL)
	{
	  fprintf(stderr, 
		  "%s(): ", 
		  arg_func_name);
	}

      va_start(ap, arg_format);
      retval = vfprintf(stderr, arg_format, ap);
      va_end(ap);
    }

  return retval;
}
