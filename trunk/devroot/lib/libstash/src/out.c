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
 ****************************************************************************/

#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include "libstash/out_p.h"
/* Uncomment this once the mem class uses `out' instead of `log'. */
/*  #include "libstash/mem_l.h" */

cw_out_t *
out_new(cw_out_t * a_out)
{
  cw_out_t * retval;

  if (NULL != a_out)
  {
    retval = a_out;
    retval->is_malloced = FALSE;
  }
  else
  {
    retval = (cw_out_t *) _cw_malloc(sizeof(cw_out_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

#ifdef _LIBSTASH_DBG
  retval->magic = _LIBSTASH_OUT_MAGIC;
#endif

  RETURN:
  return retval;
}

void
out_delete(cw_out_t * a_out)
{
  _cw_check_ptr(a_out);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);

#ifdef _CW_REENTRANT
  mtx_delete(&a_out->lock);
#endif
  
  if (TRUE == a_out->is_malloced)
  {
    _cw_free(a_out);
  }
#ifdef _LIBSTASH_DBG
  else
  {
    memset(a_out, 0x5a, sizeof(cw_out_t));
  }
#endif
}

cw_bool_t
out_register(cw_out_t * a_out,
	     const char * a_type,
	     cw_uint32_t a_size,
	     cw_out_metric_t * a_metric_func,
	     cw_out_render_t * a_render_func)
{
  cw_bool_t retval;

  _cw_check_ptr(a_out);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);
  _cw_check_ptr(a_type);
  _cw_assert((1 == a_size) || (2 == a_size) || (4 == a_size) || (8 == a_size)
	     || (12 == a_size) || (16 == a_size));
  _cw_check_ptr(a_metric_func);
  _cw_check_ptr(a_render_func);
  
  return retval;
}

cw_sint32_t
out_put_f(cw_out_t * a_out, cw_sint32_t a_fd, const char * a_format, ...)
{
  cw_sint32_t retval;
  va_list ap;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);

  va_start(ap, a_format);
  retval = out_put_fv(a_out, a_fd, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_fe(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_file_name,
	   cw_uint32_t a_line_num,
	   const char * a_func_name,
	   const char * a_format,
	   ...)
{
  cw_sint32_t retval;
  va_list ap;
  char * format = NULL, * line_num = NULL;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);

  if (NULL != a_file_name)
  {
    if (NULL != a_func_name)
    {
      /* Print filename, line number, and function name. */
      if (-1 == out_put_sa(a_out, &format,
			   "At [t:s], line [t:i32]: [t:s](): [t:s]",
			   a_file_name, a_line_num,
			   a_func_name, a_format))
      {
	retval = -1;
	goto RETURN;
      }
      retval = out_put_fv(a_out, a_fd, format, ap);
    }
    else
    {
      /* Print filename and line number. */
      if (-1 == out_put_sa(a_out, &format,
			   "At [t:s], line [t:i32]: [t:s]",
			   a_file_name, a_line_num, a_format))
      {
	retval = -1;
	goto RETURN;
      }
      retval = out_put_fv(a_out, a_fd, format, ap);
    }
  }
  else if (NULL != a_func_name)
  {
    /* Print function name. */
    if (-1 == out_put_sa(a_out, &format,
			 "[t:s](): [t:s]",
			 a_func_name, a_format))
    {
      retval = -1;
      goto RETURN;
    }
    retval = out_put_fv(a_out, a_fd, format, ap);
  }
  else
  {
    /* Make no modifications. */
    retval = out_put_fv(a_out, a_fd, a_format, ap);
  }
  
  RETURN:
  va_end(ap);
  if (NULL != format)
  {
    _cw_free(format);
  }
  if (NULL != line_num)
  {
    _cw_free(line_num);
  }
  return retval;
}

cw_sint32_t
out_put_fle(cw_out_t * a_out, cw_sint32_t a_fd,
	    const char * a_file_name,
	    cw_uint32_t a_line_num,
	    const char * a_func_name,
	    const char * a_format,
	    ...)
{
  cw_sint32_t retval;
  va_list ap;
  char * format = NULL;
  time_t curr_time;
  struct tm * cts;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  curr_time = time(NULL);
  cts = localtime(&curr_time);
  if (-1 == out_put_sa(a_out, &format,
		       "\[[t:i32|w:4]/[t:i32|w:2|p:0]/[t:i32|w:2|p:0] "
		       "[t:i32|w:2|p:0]:[t:i32|w:2|p:0]:[t:i32|w:2|p:0] "
		       "([t:s])]: [t:s]",
		       cts->tm_year + 1900, cts->tm_mon + 1, cts->tm_mday,
		       cts->tm_hour, cts->tm_min, cts->tm_sec, tzname[0],
		       a_format))
  {
    retval = -1;
    goto RETURN;
  }

  va_start(ap, a_format);
  retval = out_put_fe(a_out, a_fd, a_file_name, a_line_num, a_func_name,
		      format, ap);
  va_end(ap);

  RETURN:
  if (NULL != format)
  {
    _cw_free(format);
  }
  return retval;
}

cw_sint32_t
out_put_fn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	   const char * a_format, ...)
{
  cw_sint32_t retval, i, out_size, nwritten;
  va_list ap;
  char * output;

  _cw_assert(0 <= a_fd);
  _cw_assert(0 < a_size);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  
  output = (char *) _cw_malloc(a_size);
  if (NULL == output)
  {
    retval = -1;
    goto RETURN;
  }

  if (-1 == (out_size = out_put_svn(a_out, output, a_size, a_format, ap)))
  {
    retval = -1;
    goto RETURN;
  }

#ifdef _CW_REENTRANT
  if (NULL != a_out)
  {
    mtx_lock(&a_out->lock);
  }
#endif

  i = 0;
  do
  {
    nwritten = write(a_fd, &output[i], retval - i);
    if (-1 != nwritten)
    {
      i += nwritten;
    }
  } while ((i < out_size) && (-1 == nwritten) && (EAGAIN == errno));

#ifdef _CW_REENTRANT
  if (NULL != a_out)
  {
    mtx_unlock(&a_out->lock);
  }
#endif

  retval = i;
  
  RETURN:
  va_end(ap);
  if (NULL != output)
  {
    _cw_free(output);
  }
  return retval;
}

cw_sint32_t
out_put_fv(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_format, va_list a_p)
{
  cw_sint32_t retval, out_size;
  char * output = NULL;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  if (-1 == (out_size = out_put_sva(a_out, &output, a_format, a_p)))
  {
    retval = -1;
    goto RETURN;
  }

  retval = out_put_fn(a_out, a_fd, (cw_uint32_t) out_size, "[t:s]", output);
  
  RETURN:
  if (NULL != output)
  {
    _cw_free(output);
  }
  return retval;
}

cw_sint32_t
out_put_s(cw_out_t * a_out, char * a_str, const char * a_format, ...)
{
  cw_sint32_t retval;
  va_list ap;

  _cw_check_ptr(a_str);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_put_sv(a_out, a_str, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_sa(cw_out_t * a_out, char ** r_str, const char * a_format, ...)
{
  cw_sint32_t retval;
  va_list ap;
  
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_put_sva(a_out, r_str, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_sn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	   const char * a_format, ...)
{
  cw_sint32_t retval;
  va_list ap;

  _cw_check_ptr(a_str);
  _cw_assert(0 < a_size);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_put_svn(a_out, a_str, a_size, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_sv(cw_out_t * a_out, char * a_str,
	   const char * a_format, va_list a_p)
{
  cw_sint32_t retval;
  cw_uint32_t out_size;

  _cw_check_ptr(a_str);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  out_size = out_p_metric(a_out, a_format, a_p);

  retval = out_put_svn(a_out, a_str, out_size, a_format, a_p);
  
  return retval;
}

cw_sint32_t
out_put_sva(cw_out_t * a_out, char ** r_str,
	    const char * a_format, va_list a_p)
{
  cw_sint32_t retval;
  cw_uint32_t out_size;
  char * output;

  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  out_size = out_p_metric(a_out, a_format, a_p);
  output = (char *) _cw_malloc(out_size);
  if (NULL == output)
  {
    retval = -1;
    goto RETURN;
  }

  retval = out_put_svn(a_out, output, out_size, a_format, a_p);

  RETURN:
  if (-1 != retval)
  {
    *r_str = output;
  }
  else
  {
    *r_str = NULL;
  }
  return retval;
}

cw_sint32_t
out_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	    const char * a_format, va_list a_p)
{
  cw_sint32_t retval;

  _cw_check_ptr(a_str);
  _cw_assert(0 < a_size);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  /* XXX */

  return retval;
}

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format, va_list a_p)
{
  cw_sint32_t retval;
  cw_uint32_t i, format_len, nsegments;
  cw_uint32_t out_size; /* Total number of bytes to be printed. */
  char * format;
  cw_uint8_t hex[17] = "0123456789abcdef";
  enum
  {
    S_START,           /* Initial vanilla state. */
    S_NORMAL,          /* Vanilla state. */
    S_BACKSLASH,       /* Backslash-protected. */
    S_BACKSLASH_HEX_A, /* First digit of a hex specifier (ex: "\xff"). */
    S_BACKSLASH_HEX_B, /* Second digit of a hex specifier. */
    S_NAME_NO_T,       /* Name ("t" name hasn't been specified yet. */
    S_NAME_INITIAL_T,  /* First character of name is 't'. */
    S_NAME_CHAR,       /* Name character ("t" name hasn't been specified yet. */
    S_VALUE,           /* Value ("t" name hasn't been specified yet. */
    S_VALUE_T,         /* Value ("t" name has been specified. */
    S_NAME_T,          /* Name ("t" name has been specified. */
    S_NAME_T_CHAR      /* Name character ("t" name has been specified). */
  } state;

/* Designator values.  Each segment in format (with the likely exception of the
 * first segment is preceded by a designator code. */
#define _LIBSTASH_OUT_DES_NORMAL    0x81
#define _LIBSTASH_OUT_DES_PROTECTED 0x82
#define _LIBSTASH_OUT_DES_SPECIFIER 0x83

  format_len = strlen(a_format);
  format = (char *) _cw_malloc(out_size + 1);
  if (NULL == format)
  {
    retval = -1;
    goto RETURN;
  }
  memcpy(format, a_format, format_len + 1);

  for (i = out_size = nsegments = 0,
	 state = S_START;
       i < format_len + 1;
       i++)
  {
    switch (state)
    {
      case S_START:
      {
	if ('\\' == format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_PROTECTED;
	  state = S_BACKSLASH;
	}
	else if ('[' == format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  state = S_NAME_NO_T;
	}
	else if (0 == isprint)
	{
	  retval = -2;
	  goto RETURN;
	}
	else
	{
	  out_size++;
	}

	nsegments++;
	
	break;
      }
      case S_NORMAL:
      {
	if ('\\' == format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_PROTECTED;
	  nsegments++;
	  state = S_BACKSLASH;
	}
	else if ('[' == format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  nsegments++;
	  state = S_NAME_NO_T;
	}
	else if (0 == isprint)
	{
	  retval = -2;
	  goto RETURN;
	}
	else
	{
	  out_size++;
	}
	
	break;
      }
      case S_BACKSLASH:
      {
	switch (format[i])
	{
	  case 'a':
	  {
	    format[i] = '\a';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'b':
	  {
	    format[i] = '\t';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 't':
	  {
	    format[i] = '\t';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'n':
	  {
	    format[i] = '\n';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'v':
	  {
	    format[i] = '\v';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'f':
	  {
	    format[i] = '\f';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'r':
	  {
	    format[i] = '\r';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case '\\':
	  {
	    format[i] = '\\';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case '[':
	  {
	    format[i] = '[';
	    out_size++;
	    state = S_NORMAL;
	    break;
	  }
	  case 'x':
	  {
	    format[i] = '\0';
	    state = S_NORMAL;
	    break;
	  }
	  default:
	  {
	    retval = -2;
	    goto RETURN;
	  }
	}
	
	break;
      }
      case S_BACKSLASH_HEX_A:
      {
	cw_uint32_t j;

	for (j = 0; j < 16; j++)
	{
	  if (hex[j] == format[i])
	  {
	    format[i] = (j << 4);
	    break;
	  }
	}
	
	if (16 == j)
	{
	  /* The character wasn't [0-9a-f]. */
	  retval = -2;
	  goto RETURN;
	}
	
	break;
      }
      case S_BACKSLASH_HEX_B:
      {
	cw_uint32_t j;

	for (j = 0; j < 16; j++)
	{
	  if (hex[j] == format[i])
	  {
	    format[i - 1] |= j;
	    out_size++;
	    format[i] = '\0';
	    break;
	  }
	}
	
	if (16 == j)
	{
	  /* The character wasn't [0-9a-f]. */
	  retval = -2;
	  goto RETURN;
	}
	
	break;
      }
      case S_NAME_NO_T:
      {
	if ('t' == format[i])
	{
	  state = S_NAME_INITIAL_T;
	}
	else if ((':' == format[i])
		 || (']' == format[i])
		 || ('|' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}
	else
	{
	  state = S_NAME_CHAR;
	}
	
	break;
      }
      case S_NAME_INITIAL_T:
      {
	if (':' == format[i])
	{
	  state = S_VALUE_T;
	}
	else if ((']' == format[i])
		 || ('|' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}
	else
	{
	  state = S_NAME_CHAR;
	}
	
	break;
      }
      case S_NAME_CHAR:
      {	
	if (':' == format[i])
	{
	  state = S_VALUE;
	}
	else if ((']' == format[i])
		 || ('|' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}
	
	break;
      }
      case S_VALUE:
      {
	if ('|' == format[i])
	{
	  state = S_NAME_NO_T;
	}
	else if ((']' == format[i])
		 || (':' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}

	break;
      }
      case S_VALUE_T:
      {
	if ('|' == format[i])
	{
	  state = S_NAME_T;
	}
	else if (']' == format[i])
	{
	  state = S_NORMAL;
	}
	else if ((':' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}

	break;
      }
      case S_NAME_T:
      {
	if ((']' == format[i])
	    || ('|' == format[i])
	    || (':' == format[i])
	    || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}
	else
	{
	  state = S_NAME_T_CHAR;
	}
	
	break;
      }
      case S_NAME_T_CHAR:
      {
	if (':' == format[i])
	{
	  state = S_VALUE_T;
	}
	else if ((']' == format[i])
		 || ('|' == format[i])
		 || (0 == isprint(format[i])))
	{
	  retval = -2;
	  goto RETURN;
	}
	
	break;
      }
      default:
      {
	break;
      }
    }
  }

  /* XXX */

  RETURN:
  if (NULL != format)
  {
    _cw_free(format);
  }
  return retval;
}

static void
out_p_add(cw_uint32_t a_base, cw_uint32_t a_ndigits,
	  char * r_result, const char * a_a, const char * a_b)
{
  cw_sint32_t i, j, k;
  int a, b;
  cw_uint32_t digit, carry;
  cw_uint8_t * syms = "0123456789abcdefghijklmnopqrstuvwxyz";

  _cw_assert(a_base >= 2);
  _cw_assert(a_base <= 36);
  _cw_check_ptr(r_result);
  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  for (i = a_ndigits - 1, carry = 0; i >= 0; i--)
  {
    /* This is slower than it would be if ascii were assumed, but it always
     * works. */
    for (j = k = 0, a = a_a[i], b = a_b[i], digit = carry;
	 k < 2 && j < 36;
	 j++)
    {
      if (a == syms[j])
      {
	digit += j;
	k++;
      }
      if (b == syms[j])
      {
	digit += j;
	k++;
      }
    }
    _cw_assert(2 == k);

    if (digit > (a_base - 1))
    {
      digit -= a_base;
      carry = 1;
    }
    else
    {
      carry = 0;
    }
    r_result[i] = syms[digit];
  }
}

static cw_uint32_t
out_p_metric_int8(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int8(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int16(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int16(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int32(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int32(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int64(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int64(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_string(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_string(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_pointer(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_pointer(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_undef(const char * a_format, const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_undef(const char * a_format, const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}
