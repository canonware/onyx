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
 * $Revision: 62 $
 * $Date: 1998-05-01 16:48:14 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_STDARG_H_
#define _INC_STRING_H_
#define _INC_TIME_H_
#include <config.h>
#include <log_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * log constructor.
 *
 ****************************************************************************/
cw_log_t *
log_new()
{
  cw_log_t * retval;

  retval = (cw_log_t *) _cw_malloc(sizeof(cw_log_t));
  
  mtx_new(&retval->lock);
  retval->is_logfile_open = FALSE;
  retval->logfile_name = NULL;
  retval->log_fp = NULL;
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * log destructor.
 *
 ****************************************************************************/
void
log_delete(cw_log_t * a_log_o)
{

  _cw_check_ptr(a_log_o);
  
  if ((a_log_o->log_fp != NULL) && (a_log_o->log_fp != stderr))
  {
    fflush(a_log_o->log_fp);
    fclose(a_log_o->log_fp);
  }

  if (a_log_o->logfile_name != NULL)
  {
    _cw_free(a_log_o->logfile_name);
  }

  mtx_delete(&a_log_o->lock);
  
  _cw_free(a_log_o);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Opens file for logging.  If another file is already being used, it is
 * first closed.
 *
 ****************************************************************************/
cw_bool_t
log_set_logfile(cw_log_t * a_log_o,
		char * a_logfile,
		cw_bool_t a_overwrite)
{
  cw_bool_t retval;
  FILE * temp_fp;
  
  _cw_check_ptr(a_log_o);
  _cw_check_ptr(a_logfile);
  mtx_lock(&a_log_o->lock);
  
  if ((a_log_o->log_fp != NULL) && (a_log_o->log_fp != stderr))
  {
    fflush(a_log_o->log_fp);
    fclose(a_log_o->log_fp);
  }

  if (a_overwrite == TRUE)
  {
    temp_fp = fopen(a_logfile, "w");
  }
  else
  {
    temp_fp = fopen(a_logfile, "a+");
    if (temp_fp == NULL)
    {
      temp_fp = fopen(a_logfile, "w");
    }
  }
  
  if (temp_fp != NULL)
  {
    retval = FALSE;
    a_log_o->log_fp = temp_fp;
    if (a_log_o->logfile_name != NULL)
    {
      _cw_free(a_log_o->logfile_name);
    }
    a_log_o->logfile_name = (char *) _cw_malloc(strlen(a_logfile) + 1);
    strcpy(a_log_o->logfile_name, a_logfile);
  }
  else
  {
    retval = TRUE;
  }

  mtx_unlock(&a_log_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Run-of-the-mill printf()-alike.
 *
 ****************************************************************************/
int
log_printf(cw_log_t * a_log_o, char * a_format, ...)
{
  va_list ap;
  int retval;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);

  if ((a_log_o == NULL) || (a_log_o->log_fp == NULL))
  {
    /* Use stderr. */
    va_start(ap, a_format);
    retval = vfprintf(stderr, a_format, ap);
    va_end(ap);
  }
  else
  {
    va_start(ap, a_format);
    retval = vfprintf(a_log_o->log_fp, a_format, ap);
    va_end(ap);
    fflush(a_log_o->log_fp);
  }

  mtx_unlock(&a_log_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Optional arguments prepend filename, line number, and function name.
 * Otherwise, this still acts like printf().
 *
 ****************************************************************************/
int
log_eprintf(cw_log_t * a_log_o,
	    char * a_filename,
	    int a_line_num,
	    char * a_func_name,
	    char * a_format,
	    ...)
{
  va_list ap;
  int retval;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);
  
  if ((a_log_o == NULL) || (a_log_o->log_fp == NULL))
  {
    /* Use stderr. */
    if (a_filename != NULL)
    {
      fprintf(stderr,
	      "At %s, line %d: ",
	      a_filename,
	      a_line_num);
    }
    if (a_func_name != NULL)
    {
      fprintf(stderr,
	      "%s(): ",
	      a_func_name);
    }

    va_start(ap, a_format);
    retval = vfprintf(stderr, a_format, ap);
    va_end(ap);
  }
  else
  {
    if (a_filename != NULL)
    {
      fprintf(a_log_o->log_fp,
	      "At %s, line %d: ",
	      a_filename,
	      a_line_num);
    }
    if (a_func_name != NULL)
    {
      fprintf(a_log_o->log_fp,
	      "%s(): ",
	      a_func_name);
    }

    va_start(ap, a_format);
    retval = vfprintf(a_log_o->log_fp, a_format, ap);
    va_end(ap);
    fflush(a_log_o->log_fp);
  }

  mtx_unlock(&a_log_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * printf()-alike that prepends log message foo.
 *
 ****************************************************************************/
int
log_lprintf(cw_log_t * a_log_o, char * a_format, ...)
{
  va_list ap;
  int retval;
  char time_str[29];
  time_t curr_time;
  struct tm * cts;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);

  /* Create time string. */
  curr_time = time(NULL);
  cts = localtime(&curr_time);
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, cts->tm_zone);
  
  if ((a_log_o == NULL) || (a_log_o->log_fp == NULL))
  {
    /* Use stderr. */
    fprintf(stderr, "%s", time_str);
    va_start(ap, a_format);
    retval = vfprintf(stderr, a_format, ap);
    va_end(ap);
  }
  else
  {
    fprintf(a_log_o->log_fp, "%s", time_str);
    va_start(ap, a_format);
    retval = vfprintf(a_log_o->log_fp, a_format, ap);
    va_end(ap);
    fflush(a_log_o->log_fp);
  }

  mtx_unlock(&a_log_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Prepended log message foo.  'Optional' arguments prepend filename, line
 * number, and function name.
 *
 ****************************************************************************/
int
log_leprintf(cw_log_t * a_log_o,
	     char * a_filename,
	     int a_line_num,
	     char * a_func_name,
	     char * a_format,
	     ...)
{
  va_list ap;
  int retval;
  char time_str[29];
  time_t curr_time;
  struct tm * cts;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);
  
  /* Create time string. */
  curr_time = time(NULL);
  cts = localtime(&curr_time);
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, cts->tm_zone);

  if ((a_log_o == NULL) || (a_log_o->log_fp == NULL))
  {
    /* Use stderr. */
    if (a_filename != NULL)
    {
      fprintf(stderr,
	      "%sAt %s, line %d: ",
	      time_str,
	      a_filename,
	      a_line_num);
    }
    if (a_func_name != NULL)
    {
      fprintf(stderr,
	      "%s(): ",
	      a_func_name);
    }

    va_start(ap, a_format);
    retval = vfprintf(stderr, a_format, ap);
    va_end(ap);
  }
  else
  {
    if (a_filename != NULL)
    {
      fprintf(a_log_o->log_fp,
	      "%sAt %s, line %d: ",
	      time_str,
	      a_filename,
	      a_line_num);
    }
    if (a_func_name != NULL)
    {
      fprintf(a_log_o->log_fp,
	      "%s(): ",
	      a_func_name);
    }

    va_start(ap, a_format);
    retval = vfprintf(a_log_o->log_fp, a_format, ap);
    va_end(ap);
    fflush(a_log_o->log_fp);
  }

  mtx_unlock(&a_log_o->lock);
  return retval;
}
