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
 * Current revision: $Revision: 9 $
 * Last modified: $Date: 1998-02-08 22:18:43 -0800 (Sun, 08 Feb 1998) $
 *
 * Description: 
 *              
 *              
 *              
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
