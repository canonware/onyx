/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 121 $
 * $Date: 1998-07-01 17:22:38 -0700 (Wed, 01 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Logging facility.
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
  FILE * fp;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);

  if (a_log_o->log_fp == NULL)
  {
    fp = stderr;
  }
  else
  {
    fp = a_log_o->log_fp;
  }

  va_start(ap, a_format);
  retval = vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

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
  FILE * fp;

  _cw_check_ptr(a_log_o);
  mtx_lock(&a_log_o->lock);
  
  if (a_log_o->log_fp == NULL)
  {
    fp = stderr;
  }
  else
  {
    fp = a_log_o->log_fp;
  }

  if (a_filename != NULL)
  {
    fprintf(fp,
	    "At %s, line %d: ",
	    a_filename,
	    a_line_num);
  }
  if (a_func_name != NULL)
  {
    fprintf(fp,
	    "%s(): ",
	    a_func_name);
  }

  va_start(ap, a_format);
  retval = vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

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
  FILE * fp;
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
  
  if (a_log_o->log_fp == NULL)
  {
    fp = stderr;
  }
  else
  {
    fp = a_log_o->log_fp;
  }

  fprintf(fp, "%s", time_str);
  va_start(ap, a_format);
  retval = vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

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
  FILE * fp;
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

  if (a_log_o->log_fp == NULL)
  {
    fp = stderr;
  }
  else
  {
    fp = a_log_o->log_fp;
  }

  if (a_filename != NULL)
  {
    fprintf(fp,
	    "%sAt %s, line %d: ",
	    time_str,
	    a_filename,
	    a_line_num);
  }
  if (a_func_name != NULL)
  {
    fprintf(fp,
	    "%s(): ",
	    a_func_name);
  }

  va_start(ap, a_format);
  retval = vfprintf(fp, a_format, ap);
  va_end(ap);
  fflush(fp);

  mtx_unlock(&a_log_o->lock);
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * a_val : Value to convert to a string.
 * a_base : Number base to convert to.
 * a_buf : A buffer of at least 65 bytes for base 2 conversion, 21 bytes
 *         for base 10 conversion, and 17 bytes for base 16 conversion.
 *
 * <<< Description >>>
 *
 * Converts a_val to a number string in base a_base and puts the result
 * into *a_buf.
 *
 ****************************************************************************/
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
    for (i = 0; i < 20; i++)
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
/*   else if (a_base == 8) */
/*   { */
/*   } */
  else
  {
    /* Unsupported base. */
    _cw_error("Unsupported base in log_print_uint64()");
    retval = NULL;
  }
  
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Does a base 10 addition of the strings a_a and a_b, and puts the result
 * into string a_result.
 *
 ****************************************************************************/
void
log_p_uint64_base10_add(char * a_result, char * a_a, char * a_b)
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
