/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Logging facility.
 *
 ****************************************************************************/

#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include <stdarg.h>
#include <time.h>

#include "libstash/log_p.h"
#include "libstash/mem_l.h"

cw_log_t *
log_new(void)
{
  cw_log_t * retval;

  retval = (cw_log_t *) _cw_malloc(sizeof(cw_log_t));
  if (NULL == retval)
  {
    goto RETURN;
  }
  
#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif
  retval->is_logfile_open = FALSE;
  retval->logfile_name = NULL;
  retval->log_fp = NULL;

  RETURN:
  return retval;
}

void
log_delete(cw_log_t * a_log)
{
  _cw_check_ptr(a_log);
  
  if ((a_log->log_fp != NULL) && (a_log->log_fp != stderr))
  {
    fflush(a_log->log_fp);
    fclose(a_log->log_fp);
  }

  if (a_log->logfile_name != NULL)
  {
    _cw_free(a_log->logfile_name);
  }

#ifdef _CW_REENTRANT
  mtx_delete(&a_log->lock);
#endif
  
  _cw_free(a_log);
}

cw_bool_t
log_set_logfile(cw_log_t * a_log,
		const char * a_logfile,
		cw_bool_t a_overwrite)
{
  cw_bool_t retval;
  FILE * temp_fp;
  char * t_str;
  
  _cw_check_ptr(a_log);
  _cw_check_ptr(a_logfile);
#ifdef _CW_REENTRANT
  mtx_lock(&a_log->lock);
#endif

  t_str = (char *) _cw_malloc(strlen(a_logfile) + 1);
  if (NULL == t_str)
  {
    retval = TRUE;
    goto RETURN;
  }
  
  if ((a_log->log_fp != NULL) && (a_log->log_fp != stderr))
  {
    if (fflush(a_log->log_fp))
    {
      _cw_free(t_str);
      retval = TRUE;
      goto RETURN;
    }
    if (fclose(a_log->log_fp))
    {
      _cw_free(t_str);
      retval = TRUE;
      goto RETURN;
    }
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
    a_log->log_fp = temp_fp;
    if (a_log->logfile_name != NULL)
    {
      _cw_free(a_log->logfile_name);
    }
    a_log->logfile_name = t_str;
    strcpy(a_log->logfile_name, a_logfile);
  }
  else
  {
    _cw_free(t_str);
    retval = TRUE;
    goto RETURN;
  }

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  mtx_unlock(&a_log->lock);
#endif
  return retval;
}

int
log_printf(cw_log_t * a_log, const char * a_format, ...)
{
  va_list ap;
  int retval;
  FILE * fp;

  if (a_log == NULL)
  {
    fp = stderr;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_log->lock);
#endif
    if (a_log->log_fp == NULL)
    {
      fp = stderr;
    }
    else
    {
      fp = a_log->log_fp;
    }
  }
  
  va_start(ap, a_format);
  retval = vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

#ifdef _CW_REENTRANT
  if (a_log != NULL)
  {
    mtx_unlock(&a_log->lock);
  }
#endif
  return retval;
}

int
log_eprintf(cw_log_t * a_log,
	    const char * a_filename,
	    int a_line_num,
	    const char * a_func_name,
	    const char * a_format,
	    ...)
{
  va_list ap;
  int retval = 0;
  FILE * fp;

  if (a_log == NULL)
  {
    fp = stderr;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_log->lock);
#endif
  
    if (a_log->log_fp == NULL)
    {
      fp = stderr;
    }
    else
    {
      fp = a_log->log_fp;
    }
  }
  
  if (a_filename != NULL)
  {
    retval += fprintf(fp,
		      "At %s, line %d: ",
		      a_filename,
		      a_line_num);
  }
  if (a_func_name != NULL)
  {
    retval += fprintf(fp,
		      "%s(): ",
		      a_func_name);
  }

  va_start(ap, a_format);
  retval += vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

#ifdef _CW_REENTRANT
  if (a_log != NULL)
  {
    mtx_unlock(&a_log->lock);
  }
#endif
  
  return retval;
}

int
log_nprintf(cw_log_t * a_log,
	    cw_uint32_t a_size,
	    const char * a_format,
	    ...)
{
  va_list ap;
  int retval;
  char * t_buf;
  FILE * fp;

  if (a_log == NULL)
  {
    fp = stderr;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_log->lock);
#endif
    if (a_log->log_fp == NULL)
    {
      fp = stderr;
    }
    else
    {
      fp = a_log->log_fp;
    }
  }

  t_buf = (char *) _cw_malloc(a_size + 1);
  if (NULL == t_buf)
  {
    retval = -1;
    goto RETURN;
  }
  
  va_start(ap, a_format);
  vsnprintf(t_buf, (size_t) a_size + 1, a_format, ap);
  va_end(ap);

  retval = fprintf(fp, "%s", t_buf);
  fflush(fp);

  _cw_free(t_buf);

  RETURN:
#ifdef _CW_REENTRANT
  if (a_log != NULL)
  {
    mtx_unlock(&a_log->lock);
  }
#endif
  return retval;
}

int
log_lprintf(cw_log_t * a_log, const char * a_format, ...)
{
  va_list ap;
  int retval = 0;
  FILE * fp;
  char time_str[29];
  time_t curr_time;
  struct tm * cts;

  if (a_log == NULL)
  {
    fp = stderr;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_log->lock);
#endif
    if (a_log->log_fp == NULL)
    {
      fp = stderr;
    }
    else
    {
      fp = a_log->log_fp;
    }
  }

  /* Create time string. */
  curr_time = time(NULL);
  cts = localtime(&curr_time);
#if (defined(_CW_OS_FREEBSD))
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, cts->tm_zone);
#elif (defined(_CW_OS_SOLARIS) || defined(_CW_OS_LINUX))
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, tzname[0]);
#else
#  error "Unsupported OS"
#endif

  retval += fprintf(fp, "%s", time_str);
  va_start(ap, a_format);
  retval += vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

#ifdef _CW_REENTRANT
  if (a_log != NULL)
  {
    mtx_unlock(&a_log->lock);
  }
#endif
  return retval;
}

int
log_leprintf(cw_log_t * a_log,
	     const char * a_filename,
	     int a_line_num,
	     const char * a_func_name,
	     const char * a_format,
	     ...)
{
  va_list ap;
  int retval = 0;
  FILE * fp;
  char time_str[29];
  time_t curr_time;
  struct tm * cts;

  if (a_log == NULL)
  {
    fp = stderr;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_log->lock);
#endif

    if (a_log->log_fp == NULL)
    {
      fp = stderr;
    }
    else
    {
      fp = a_log->log_fp;
    }
  }
  
  /* Create time string. */
  curr_time = time(NULL);
  cts = localtime(&curr_time);
#if (defined(_CW_OS_FREEBSD))
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, cts->tm_zone);
#elif (defined(_CW_OS_SOLARIS) || defined(_CW_OS_LINUX))
  sprintf(time_str, "[%4d/%02d/%02d %02d:%02d:%02d (%s)]: ",
	  cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
	  cts->tm_hour, cts->tm_min, cts->tm_sec, tzname[0]);
#else
#  error "Unsupported OS"
#endif
  
  retval += fprintf(fp, "%s", time_str);
    
  if (a_filename != NULL)
  {
    retval += fprintf(fp,
		      "At %s, line %d: ",
		      a_filename,
		      a_line_num);
  }
  if (a_func_name != NULL)
  {
    retval += fprintf(fp,
		      "%s(): ",
		      a_func_name);
  }

  va_start(ap, a_format);
  retval += vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

#ifdef _CW_REENTRANT
  if (a_log != NULL)
  {
    mtx_unlock(&a_log->lock);
  }
#endif
  return retval;
}

char *
log_print_uint64(cw_uint64_t a_val, cw_uint32_t a_base, char * a_buf)
{
  char * retval;
  
  _cw_check_ptr(a_buf);

  retval = a_buf;
  
  if (a_base == 16)
  {
    cw_uint32_t i, digit;
    
    for (i = 0; i < 16; i++)
    {
      digit = ((a_val >> (60 - (4 * i))) & 0x0000000f);

      if ((digit >= 0) && (digit <= 9))
      {
	a_buf[i] = '0' + digit;
      }
      else
      {
	a_buf[i] = 'a' + (digit - 10);
      }
    }
    a_buf[16] = '\0';
  }
  else if (a_base == 10)
  {
    char zero[21] = "00000000000000000000",
      curr_add[21]= "00000000000000000001",
      result[21] = "00000000000000000000",
      temp[21] = "00000000000000000000";
    
    cw_uint32_t i;

    for (i = 0; i < 64; i++)
    {
      if ((a_val >> i) & 0x00000001)
      {
	/* Copy the result for use in the next call. */
	log_p_uint64_base10_add(temp, result, zero);

	/* Add this digit into the result. */
	log_p_uint64_base10_add(result, temp, curr_add);
      }
      /* Copy curr_add for use in the next call. */
      log_p_uint64_base10_add(temp, curr_add, zero);

      /* Double curr_add. */
      log_p_uint64_base10_add(curr_add, temp, temp);
    }
    
    /* Find the first non-zero digit. */
    for (i = 0; i < 19; i++)
    {
      if (result[i] != '0')
      {
	break;
      }
    }
    strcpy(a_buf, result + i);
  }
  else if (a_base == 2)
  {
    cw_uint32_t i;
    cw_uint8_t digit;
    
    for (i = 0; i < 64; i++)
    {
      digit = ((a_val >> (63 - i)) & 0x00000001);

      if (digit == 0)
      {
	a_buf[i] = '0';
      }
      else
      {
	a_buf[i] = '1';
      }
    }
    a_buf[64] = '\0';
  }
  else
  {
    /* Unsupported base. */
    _cw_error("Unsupported base");
    retval = NULL;
  }
  
  return retval;
}

static void
log_p_uint64_base10_add(char * a_result, const char * a_a, const char * a_b)
{
  cw_sint32_t i;
  cw_uint8_t digit, carry;

  for (i = 19, carry = 0; i >= 0; i--)
  {
    digit = (a_a[i] - '0') + (a_b[i] - '0') + carry;
    if (digit > 9)
    {
      digit -= 10;
      carry = 1;
    }
    else
    {
      carry = 0;
    }
    a_result[i] = digit + '0';
  }
}
