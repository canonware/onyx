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
 * $Revision: 26 $
 * $Date: 1998-04-12 04:09:42 -0700 (Sun, 12 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_STDARG_H_
#define _INC_STRING_H_
#define _INC_LOG_PRIV_H_
#include <config.h>

cw_log_t *
log_new()
{
  cw_log_t * retval;

  retval = (cw_log_t *) _cw_malloc(sizeof(cw_log_t));
  
/*   pthread_mutexattr_init(&retval->mutex); */
  retval->is_logfile_open = FALSE;
  retval->logfile_name = NULL;
  retval->log_fp = NULL;
  
  return retval;
}

void
log_delete(cw_log_t * arg_log_obj)
{

  _cw_check_ptr(arg_log_obj);
  
  if ((arg_log_obj->log_fp != NULL) && (arg_log_obj->log_fp != stderr))
  {
    fclose(arg_log_obj->log_fp);
  }

  if (arg_log_obj->logfile_name != NULL)
  {
    _cw_free(arg_log_obj->logfile_name);
  }

  _cw_free(arg_log_obj);
}

cw_bool_t
log_set_logfile(cw_log_t * arg_log_obj,
		char * arg_logfile,
		cw_bool_t arg_overwrite)
{
  cw_bool_t retval;
  FILE * temp_fp;
  
  _cw_check_ptr(arg_log_obj);
  _cw_check_ptr(arg_logfile);
  
  if ((arg_log_obj->log_fp != NULL) && (arg_log_obj->log_fp != stderr))
  {
    fclose(arg_log_obj->log_fp);
  }

  if (arg_overwrite == TRUE)
  {
    temp_fp = fopen(arg_logfile, "w");
  }
  else
  {
    temp_fp = fopen(arg_logfile, "a+");
    if (temp_fp == NULL)
    {
      temp_fp = fopen(arg_logfile, "w");
    }
  }
  
  if (temp_fp != NULL)
  {
    retval = FALSE;
    arg_log_obj->log_fp = temp_fp;
    if (arg_log_obj->logfile_name != NULL)
    {
      _cw_free(arg_log_obj->logfile_name);
    }
    arg_log_obj->logfile_name = (char *) _cw_malloc(strlen(arg_logfile) + 1);
    strcpy(arg_log_obj->logfile_name, arg_logfile);
  }
  else
  {
    retval = TRUE;
  }

  return retval;
}

int
log_printf(cw_log_t * arg_log_obj, char * arg_format, ...)
{
  va_list ap;
  int retval;

  _cw_check_ptr(arg_log_obj);

  if (arg_log_obj->log_fp != NULL)
    {
      va_start(ap, arg_format);
      retval = vfprintf(arg_log_obj->log_fp, arg_format, ap);
      va_end(ap);
      fflush(arg_log_obj->log_fp);
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
log_eprintf(cw_log_t * arg_log_obj,
	    char * arg_filename,
	    int arg_line_num,
	    char * arg_func_name,
	    char * arg_format,
	    ...)
{
  va_list ap;
  int retval;

  _cw_check_ptr(arg_log_obj);

  if (arg_log_obj->log_fp != NULL)
    {
      if (arg_filename != NULL)
	{
	  fprintf(arg_log_obj->log_fp,
		  "At %s, line %d: ",
		  arg_filename,
		  arg_line_num);
	}
      if (arg_func_name != NULL)
	{
	  fprintf(arg_log_obj->log_fp,
		  "%s(): ",
		  arg_func_name);
	}

      va_start(ap, arg_format);
      retval = vfprintf(arg_log_obj->log_fp, arg_format, ap);
      va_end(ap);
      fflush(arg_log_obj->log_fp);
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
