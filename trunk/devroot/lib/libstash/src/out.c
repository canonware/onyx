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
#include "libstash/mem_l.h"

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

  retval->fd = 2;

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
out_get_default_fd(cw_out_t * a_out)
{
  cw_sint32_t retval;
  
  if (NULL != a_out)
  {
    retval = a_out->fd;
  }
  else
  {
    retval = 2;
  }

  return retval;
}

void
out_set_default_fd(cw_out_t * a_out, cw_sint32_t a_fd)
{
  _cw_check_ptr(a_out);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);
  _cw_assert(0 <= a_fd);

  a_out->fd = a_fd;
}

cw_sint32_t
out_put(cw_out_t * a_out, const char * a_format, ...)
{
  cw_sint32_t retval, fd;
  va_list ap;

  _cw_check_ptr(a_format);

  if (NULL != a_out)
  {
    _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);
    fd = a_out->fd;
  }
  else
  {
    fd = 2;
  }

  va_start(ap, a_format);
  retval = out_put_fv(a_out, fd, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_e(cw_out_t * a_out,
	  const char * a_file_name,
	  cw_uint32_t a_line_num,
	  const char * a_func_name,
	  const char * a_format,
	  ...)
{
  cw_sint32_t retval, fd;
  va_list ap;

  _cw_check_ptr(a_format);
  
  if (NULL != a_out)
  {
    _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);
    fd = a_out->fd;
  }
  else
  {
    fd = 2;
  }
  
  va_start(ap, a_format);
  retval = out_p_put_vfe(a_out, fd, a_file_name, a_line_num, a_func_name,
			 a_format, ap);
  va_end(ap);
  
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

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_p_put_vfe(a_out, a_fd, a_file_name, a_line_num, a_func_name,
			 a_format, ap);
  va_end(ap);

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
  retval = out_p_put_vfe(a_out, a_fd, a_file_name, a_line_num, a_func_name,
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
  
  out_size = out_p_metric(a_out, a_format, NULL, a_p);

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
  
  out_size = out_p_metric(a_out, a_format, NULL, a_p);
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

cw_sint32_t
spec_get_val(const char * a_spec, cw_uint32_t a_spec_len,
	     const char * a_name, const char ** r_val)
{
  cw_sint32_t retval, i, name_len, curr_name_len, val_len;
  cw_bool_t match;
  enum
  {
    NAME,
    VALUE
  } state;

  _cw_check_ptr(a_name);
  _cw_check_ptr(r_val);

  name_len = strlen(a_name);
  
  for (i = curr_name_len = 0, state = NAME, match = TRUE;
       i < a_spec_len;
       i++)
  {
/*      log_eprintf(cw_g_log, NULL, 0, __FUNCTION__, */
/*  		"state: %s, curr_name_len: %d, val_len: %d, match: %s, i: %d\n", */
/*  		(state == NAME) ? " NAME" : "VALUE", */
/*  		curr_name_len, */
/*  		val_len, */
/*  		match ? " TRUE" : "FALSE", */
/*  		i); */
    
    switch (state)
    {
      case NAME:
      {
	if (':' == a_spec[i])
	{
	  if (name_len != curr_name_len)
	  {
	    /* Too short. */
	    match = FALSE;
	  }

	  if (TRUE == match)
	  {
	    /* Set the return pointer.  We'll figure out how long the value is
	     * later. */
	    *r_val = &a_spec[i + 1];
	  }

	  val_len = 0;
	  state = VALUE;
	}
	else if (a_name[curr_name_len] != a_spec[i])
	{
	  curr_name_len++;
	  match = FALSE;
	}
	else
	{
	  curr_name_len++;
	}
	
	break;
      }
      case VALUE:
      {
	if (('|' == a_spec[i])
	    || (i == a_spec_len - 1))
	{
	  /* End of the value. */
	  if (TRUE == match)
	  {
	    retval = val_len;
	    goto RETURN;
	  }
	  else
	  {
	    curr_name_len = 0;
	    match = TRUE;
	    state = NAME;
	  }
	}
	else
	{
	  val_len++;
	}
	
	break;
      }
      default:
      {
	_cw_error("Programming error");
      }
    }
  }
  
  retval = -1;

  RETURN:
  return retval;
}

cw_sint32_t
out_p_put_vfe(cw_out_t * a_out, cw_sint32_t a_fd,
	      const char * a_file_name,
	      cw_uint32_t a_line_num,
	      const char * a_func_name,
	      const char * a_format,
	      va_list a_p)
{
  cw_sint32_t retval;
  char * format = NULL;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
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
      retval = out_put_fv(a_out, a_fd, format, a_p);
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
      retval = out_put_fv(a_out, a_fd, format, a_p);
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
    retval = out_put_fv(a_out, a_fd, format, a_p);
  }
  else
  {
    /* Make no modifications. */
    retval = out_put_fv(a_out, a_fd, a_format, a_p);
  }
  
  RETURN:
  if (NULL != format)
  {
    _cw_free(format);
  }
  return retval;
}

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format, char ** r_format,
	     va_list a_p)
{
  cw_sint32_t retval;
  cw_uint32_t i, format_len, spec_len;
  cw_uint32_t out_size; /* Total number of bytes to be printed. */
  char * format; /* After parsing, each byte contains a code. */
  enum
  {
    START,   /* Initial vanilla state. */
    BRACKET, /* `[' seen. */
    PAIR,    /* Beginning of a name/value pair. */
    NAME,    /* Name. */
    VALUE,   /* Value. */
    T,       /* Current name starts with `t'. */
    NAME_T,  /* Name.  "t" name has already been seen. */
    VALUE_T  /* Value. "t" name has already been seen. */
  } state;

/* Designator values.  `format' contains codes that indicate the type of each
 * byte in a_format. */
#define _LIBSTASH_OUT_DES_NORMAL    'n'
#define _LIBSTASH_OUT_DES_SPECIFIER 's'
#define _LIBSTASH_OUT_DES_WHITEOUT  'w'

  format_len = strlen(a_format);
  if (0 == format_len)
  {
    retval = 0;
    goto RETURN;
  }
  
  format = (char *) _cw_malloc(format_len + 1);
  if (NULL == format)
  {
    _cw_marker("Error");
    retval = -1;
    goto RETURN;
  }
  bzero(format, format_len + 1);

  for (i = out_size = 0,
	 state = START;
       i < format_len + 1;
       i++)
  {
    switch (state)
    {
      case START:
      {
	if ('[' == a_format[i])
	{
	  /* We can unconditionally white this character out.  If the next
	   * character is a `[', we can leave that one intact. */
	  format[i] = _LIBSTASH_OUT_DES_WHITEOUT;
	  state = BRACKET;
	}
	else
	{
	  format[i] = _LIBSTASH_OUT_DES_NORMAL;
	  out_size++;
	}
	
	break;
      }
      case BRACKET:
      {
	if ('[' == a_format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_NORMAL;
	  out_size++;
	  state = START;
	}
	else if ('t' == a_format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  spec_len = 1;
	  state = T;
	}
	else
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  spec_len = 1;
	  state = NAME;
	}
	
	break;
      }
      case PAIR:
      {
	format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if ('t' == a_format[i])
	{
	  state = T;
	}
	
	break;
      }
      case NAME:
      {
	format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if (':' == a_format[i])
	{
	  state = VALUE;
	}
	  
	break;
      }
      case VALUE:
      {
	format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if ('|' == a_format[i])
	{
	  state = PAIR;
	}
	  
	break;
      }
      case T:
      {
	format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if (':' == a_format[i])
	{
	  state = VALUE_T;
	}
	else
	{
	  state = NAME;
	}
	
	break;
      }
      case NAME_T:
      {
	format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if (':' == a_format[i])
	{
	  state = VALUE_T;
	}
	
	break;
      }
      case VALUE_T:
      {
	if ('|' == a_format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  spec_len++;
	  state = NAME_T;
	}
	else if (']' == a_format[i])
	{
	  const char * val;
	  cw_sint32_t val_len;
	  
	  format[i] = _LIBSTASH_OUT_DES_WHITEOUT;
	  state = START;

	  /* Successful completion of parsing this specifier.  Call the
	   * corresponding metric function. */
	  /* XXX Accept. */
	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		      "spec_len == %lu, \"", spec_len);
	  log_nprintf(cw_g_log, spec_len, "%s", &a_format[i - spec_len]);
	  log_printf(cw_g_log, "\"\n");

	  val_len = spec_get_val(&a_format[i - spec_len], spec_len,
				 "t", &val);
	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		      "val_len == %d\n", val_len);
	  if (-1 == val_len)
	  {
	    _cw_marker("Error");
	    retval = -2;
	    goto RETURN;
	  }

	  /* Find a match for the type and call the corresponding metric
	   * function. */
	  

	  /* XXX */
	}
	else
	{
	  format[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  spec_len++;
	}
	
	break;
      }
      default:
      {
	_cw_error("Programming error");
      }
    }
  }
  if (START != state)
  {
    _cw_marker("Error");
    retval = -2;
    goto RETURN;
  }


  /* XXX */

  retval = out_size;

  RETURN:
  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
	      "\"%s\" retval == %d\n",
	      a_format, retval);
  if (0 > retval)
  {
    if (NULL != format)
    {
      _cw_free(format);
    }
  }
  else
  {
    if (NULL != r_format)
    {
      *r_format = format;
    }
    else
    {
      _cw_free(format);
    }
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
out_p_metric_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}

static cw_uint32_t
out_p_metric_undef(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;

  return retval;
}

static char *
out_p_render_undef(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;

  return retval;
}
