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

  retval->fd = 2;

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

  retval->nextensions = 0;
  retval->extensions = NULL;

#ifdef _LIBSTASH_DBG
  retval->magic = _LIBSTASH_OUT_MAGIC;
#endif

  RETURN:
  return retval;
}

void
out_delete(cw_out_t * a_out)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_out);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);

  for (i = 0; i < a_out->nextensions; i++)
  {
    _cw_free(a_out->extensions[i].type);
  }
  if (NULL != a_out->extensions)
  {
    _cw_free(a_out->extensions);
  }
  
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

  if (NULL == a_out->extensions)
  {
    a_out->extensions = (cw_out_ent_t *) _cw_malloc(sizeof(cw_out_ent_t));
    if (NULL == a_out->extensions)
    {
      retval = TRUE;
      goto RETURN;
    }
  }
  else
  {
    cw_out_ent_t * t_ptr;

    t_ptr = (cw_out_ent_t *) _cw_realloc(a_out->extensions,
					 ((a_out->nextensions + 1)
					  * sizeof(cw_out_ent_t)));
    if (NULL == t_ptr)
    {
      retval = TRUE;
      goto RETURN;
    }
    a_out->extensions = t_ptr;
  }
  
  a_out->extensions[a_out->nextensions].type
    = (char *) _cw_malloc(strlen(a_type) + 1);
  if (NULL == a_out->extensions[a_out->nextensions].type)
  {
    retval = TRUE;
    goto RETURN;
  }
  strcpy(a_out->extensions[a_out->nextensions].type, a_type);
  a_out->extensions[a_out->nextensions].size = a_size;
  a_out->extensions[a_out->nextensions].metric_func = a_metric_func;
  a_out->extensions[a_out->nextensions].render_func = a_render_func;

  a_out->nextensions++;

  retval = FALSE;
  
  RETURN:
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

/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "len == %d, output \"", out_size); */
/*    log_nprintf(cw_g_log, out_size, "%s", output); */
/*    log_printf(cw_g_log, "\"\n"); */
  i = 0;
  do
  {
    nwritten = write(a_fd, &output[i], out_size - i);
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
  
  if (0 >= (out_size = out_put_sva(a_out, &output, a_format, a_p)))
  {
    retval = out_size;
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
  if (0 <= retval)
  {
    a_str[retval] = '\0';
  }
  
  return retval;
}

cw_sint32_t
out_put_sva(cw_out_t * a_out, char ** r_str,
	    const char * a_format, va_list a_p)
{
  cw_sint32_t retval, out_size;
  char * output;

  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  out_size = out_p_metric(a_out, a_format, NULL, a_p);
  if (0 >= out_size)
  {
    retval = out_size;
    goto RETURN;
  }
  output = (char *) _cw_malloc(out_size + 1);
  if (NULL == output)
  {
    retval = -1;
    goto RETURN;
  }

  retval = out_put_svn(a_out, output, out_size, a_format, a_p);
  _cw_assert(out_size == retval);

  RETURN:
  if (0 <= retval)
  {
    output[retval] = '\0';
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
  cw_sint32_t retval, metric_size, size, format_len, i, j;
  char * format;

  _cw_check_ptr(a_str);
  _cw_assert(0 < a_size);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);

  metric_size = out_p_metric(a_out, a_format, &format, a_p);
  if (0 >= metric_size)
  {
    retval = metric_size;
    goto RETURN;
  }

  /* Choose the smaller of two possible sizes. */
  if (metric_size < a_size)
  {
    size = metric_size;
  }
  else
  {
    size = (cw_sint32_t) a_size;
  }
  
  for (i = j = 0, format_len = strlen(a_format);
       (i < format_len) && (j < size);
       )
  {
/*      log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		"i == %d, j == %d\n", i, j); */
    switch (format[i])
    {
      case _LIBSTASH_OUT_DES_NORMAL:
      {
	a_str[j] = a_format[i];
	j++;
	i++;
	break;
      }
      case _LIBSTASH_OUT_DES_SPECIFIER:
      {
	cw_sint32_t spec_len, type_len;
	cw_uint32_t metric;
	const char * type;
	cw_out_ent_t * ent;
	void * arg;
	
	/* Calculate the specifier length.  We're guaranteed that there is a
	 * whiteout character following the specifier. */
	for (spec_len = 0;
	     format[i + spec_len] == _LIBSTASH_OUT_DES_SPECIFIER;
	     spec_len++);

	/* Find the type string. */
	type_len = spec_get_val(&a_format[i], spec_len, "t", &type);
	_cw_assert(0 <= type_len);

	ent = out_p_get_ent(a_out, type, type_len);
	_cw_assert(NULL != ent);
	
	switch (ent->size)
	{
	  case 1:
	  {
	    arg = (void *) &va_arg(a_p, cw_uint8_t);
	    break;
	  }
	  case 2:
	  {
	    arg = (void *) &va_arg(a_p, cw_uint16_t);
	    break;
	  }
	  case 4:
	  {
	    arg = (void *) &va_arg(a_p, cw_uint32_t);
	    break;
	  }
	  case 8:
	  {
	    arg = (void *) &va_arg(a_p, cw_uint64_t);
	    break;
	  }
/*  	  case 12: */
/*  	  { */
/*  	    arg = (void *) va_arg(a_p, s_12); */
/*  	    break; */
/*  	  } */
/*  	  case 16: */
/*  	  { */
/*  	    arg = (void *) va_arg(a_p, s_16); */
/*  	    break; */
/*  	  } */
	  default:
	  {
	    _cw_error("Programming error");
	  }
	}

/*  	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		    "metric == %lu\n", ent->metric_func(&a_format[i], */
/*  							spec_len, arg)); */

	metric = ent->metric_func(&a_format[i], spec_len, arg);
	if (j + metric <= size)
	{
	  /* The printout of this item will fit in the output string. */
/*  	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		      "Fit\n"); */

	  /* XXX */
	  ent->render_func(&a_format[i], spec_len, arg, &a_str[j]);
	  
/*  	  memset(&a_str[j], 'X', metric); */
	}
	else
	{
	  /* The printout of this item will not fit in the string.  Therefore,
	   * allocate a temporary buffer, render the item there, then copy as
	   * much as will fit into the output string. */
	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__,
		      "Unfit\n");

	  /* XXX */
	  memset(&a_str[j], 'X', size - j);
	}
	
	j += metric;
	i += spec_len;
	break;
      }
      case _LIBSTASH_OUT_DES_WHITEOUT:
      {
	i++;
	break;
      }
      default:
      {
	_cw_error("Programming error");
      }
    }
  }
  
  /* XXX */
  retval = size;

  RETURN:
/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "Output: \""); */
/*    log_nprintf(cw_g_log, retval, "%s", a_str); */
/*    log_printf(cw_g_log, "\"\n"); */
  if (NULL != format)
  {
    _cw_free(format);
  }
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
	if ('|' == a_spec[i])
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
	else if (i == a_spec_len - 1)
	{
	  /* End of the value, and end of specifier.  Add one to val_len. */
	  val_len++;
	  
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

  for (i = out_size = 0, state = START;
       i < format_len;
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
/*  	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		      "out_size++ (%lu --> %lu)\n", */
/*  		      out_size, out_size + 1); */
	  out_size++;
	}
	
	break;
      }
      case BRACKET:
      {
	if ('[' == a_format[i])
	{
	  format[i] = _LIBSTASH_OUT_DES_NORMAL;
/*  	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		      "out_size++ (%lu --> %lu)\n", */
/*  		      out_size, out_size + 1); */
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
	  cw_out_ent_t * ent;
	  
	  format[i] = _LIBSTASH_OUT_DES_WHITEOUT;
	  state = START;

	  /* Successful completion of parsing this specifier.  Call the
	   * corresponding metric function. */
/*  	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		      "spec_len == %lu, \"", spec_len); */
/*  	  log_nprintf(cw_g_log, spec_len, "%s", &a_format[i - spec_len]); */
/*  	  log_printf(cw_g_log, "\"\n"); */

	  val_len = spec_get_val(&a_format[i - spec_len], spec_len,
				 "t", &val);
	  if (-1 == val_len)
	  {
	    _cw_marker("Error");
	    retval = -2;
	    goto RETURN;
	  }
/*  	  log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		      "val_len == %d, \"", val_len); */
/*  	  log_nprintf(cw_g_log, val_len, "%s", val); */
/*  	  log_printf(cw_g_log, "\"\n"); */

	  ent = out_p_get_ent(a_out, val, val_len);
	  if (NULL == ent)
	  {
	    /* No handler. */
	    _cw_marker("Error");
	    retval = -2;
	    goto RETURN;
	  }
	  
	  {
	    void * arg;

	    switch (ent->size)
	    {
	      case 1:
	      {
		arg = (void *) &va_arg(a_p, cw_uint8_t);
		break;
	      }
	      case 2:
	      {
		arg = (void *) &va_arg(a_p, cw_uint16_t);
		break;
	      }
	      case 4:
	      {
		arg = (void *) &va_arg(a_p, cw_uint32_t);
		break;
	      }
	      case 8:
	      {
		arg = (void *) &va_arg(a_p, cw_uint64_t);
		break;
	      }
/*  	      case 12: */
/*  	      { */
/*  		arg = (void *) va_arg(a_p, s_12); */
/*  		break; */
/*  	      } */
/*  	      case 16: */
/*  	      { */
/*  		arg = (void *) va_arg(a_p, s_16); */
/*  		break; */
/*  	      } */
	      default:
	      {
		_cw_marker("Error");
/*  		log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  			    "ent->size == %lu\n", ent->size); */
		retval = -2;
		goto RETURN;
	      }
	    }

/*  	    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  			"out_size += %lu\n", */
/*  			ent->metric_func(&a_format[i - spec_len], */
/*  					 spec_len, arg)); */
	    out_size += ent->metric_func(&a_format[i - spec_len],
					 spec_len, arg);
	  }
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

  retval = out_size;

  RETURN:
/*    _cw_marker("Before"); */
/*    log_printf(cw_g_log, "\"%s\"\n\"%s\"\nretval == %d\n", */
/*  	     a_format, format, retval); */
/*    _cw_marker("After"); */
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

static cw_out_ent_t *
out_p_get_ent(cw_out_t * a_out, const char * a_format, cw_uint32_t a_len)
{
  cw_out_ent_t * retval;
  cw_uint32_t i;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);

  /* Find a match for the type and call the corresponding metric function.  Use
   * the first match found by searching the extended types, then the built in
   * types.  If there is no match, return an error, since we have no way of
   * knowing the size of argument to use. */
  if (NULL != a_out)
  {
    for (i = 0;
	 i < a_out->nextensions;
	 i++)
    {
      if (0 == strncmp(a_format, a_out->extensions[i].type, a_len)
	  && (a_len == strlen(a_out->extensions[i].type)))
      {
/*  	log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		    "Type: \"%s\", i == %d\n", */
/*  		    a_out->extensions[i].type, i); */
	retval = &a_out->extensions[i];
	goto RETURN;
      }
    }
  }

  for (i = 0;
       i < (sizeof(cw_g_out_builtins) / sizeof(struct cw_out_ent_s));
       i++)
  {
    if (0 == strncmp(a_format, cw_g_out_builtins[i].type, a_len)
	&& (a_len == strlen(cw_g_out_builtins[i].type)))
    {
/*        log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  		  "Type: \"%s\", i == %d\n", */
/*  		  cw_g_out_builtins[i].type, i); */
      retval = &cw_g_out_builtins[i];
      goto RETURN;
    }
  }

  retval = NULL;

  RETURN:
/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "Search for \""); */
/*    log_nprintf(cw_g_log, a_len, "%s", a_format); */
/*    log_printf(cw_g_log, "\" --> %010p\n", retval); */
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
out_p_metric_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg, cw_uint32_t a_nbits)
{
  cw_uint32_t retval, width, base, i;
  cw_sint32_t val_len;
  cw_uint64_t arg = a_arg;
  cw_bool_t is_negative, show_sign;
  const char * val;
  char * zero, s_zero[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";
  char * curr_add, s_curr_add[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000001";
  char * result, s_result[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";
  char * temp, s_temp[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";

  _cw_assert((8 == a_nbits) || (16 == a_nbits)
	     || (32 == a_nbits) || (64 == a_nbits));

  /* Move the pointers forward so that unnecessary digits can be ignored. */
  zero = &s_zero[64 - a_nbits];
  curr_add = &s_curr_add[64 - a_nbits];
  result = &s_result[64 - a_nbits];
  temp = &s_temp[64 - a_nbits];

  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", &val)))
  {
    /* Width specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    width = strtoul(val, NULL, 10);
  }
  else
  {
    width = 0;
  }

  /* Determine sign. */
  if ((-1 != (val_len = spec_get_val(a_format, a_len, "s", &val)))
      && ('s' == val[0])
      && (0 != (arg & (((cw_uint64_t) 1) << (a_nbits - 1)))))
  {
    is_negative = TRUE;
    /* Convert two's complement to positive. */
    arg ^= ((cw_uint64_t) 0xffffffff << 32) + 0xffffffff;
    arg++;
  }
  else
  {
    is_negative = FALSE;
  }

  /* Should we show the sign if the number is positive? */
  if (((-1 != (val_len = spec_get_val(a_format, a_len, "+", &val)))
       && ('+' == val[0]))
      || (TRUE == is_negative))
  {
    show_sign = TRUE;
  }
  else
  {
    show_sign = FALSE;
  }
    
  if (-1 != (val_len = spec_get_val(a_format, a_len, "b", &val)))
  {
    /* Base specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    base = strtoul(val, NULL, 10);
    _cw_assert(2 <= base);
    _cw_assert(36 >= base);
  }
  else
  {
    /* Default to base 10. */
    base = 10;
  }
  
  for (i = 0; i < a_nbits; i++)
  {
    if ((arg >> i) & 1)
    {
      /* Copy the result for use in the next call. */
      out_p_add(base, a_nbits, temp, result, zero);
      
      /* Add this digit into the result. */
      out_p_add(base, a_nbits, result, temp, curr_add);
    }
    /* Copy curr_add for use in the next call. */
    out_p_add(base, a_nbits, temp, curr_add, zero);

    /* Double curr_add. */
    out_p_add(base, a_nbits, curr_add, temp, temp);
  }

  /* Find the first non-zero digit. */
  for (i = 0; i < (a_nbits - 1); i++)
  {
    if (result[i] != '0')
    {
      break;
    }
  }

  retval = a_nbits - i;
  if (TRUE == show_sign)
  {
    retval++;
  }
  if (width > retval)
  {
    retval = width;
  }

/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "\"%s\"_%lu, metric %lu\n", &result[i], base, retval); */
  return retval;
}

static char *
out_p_render_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg, char * r_buf, cw_uint32_t a_nbits)
{
  char * retval;
  cw_uint32_t base, width, out_len, i;
  cw_sint32_t val_len;
  cw_uint64_t arg = a_arg;
  cw_bool_t is_negative, show_sign;
  const char * val;
  char * zero, s_zero[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";
  char * curr_add, s_curr_add[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000001";
  char * result, s_result[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";
  char * temp, s_temp[65] =
    "00000000000000000000000000000000"
    "00000000000000000000000000000000";

  _cw_assert((8 == a_nbits) || (16 == a_nbits)
	     || (32 == a_nbits) || (64 == a_nbits));

  /* Move the pointers forward so that unnecessary digits can be ignored. */
  zero = &s_zero[64 - a_nbits];
  curr_add = &s_curr_add[64 - a_nbits];
  result = &s_result[64 - a_nbits];
  temp = &s_temp[64 - a_nbits];

  if (-1 != (val_len = spec_get_val(a_format, a_len, "b", &val)))
  {
    /* Base specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    base = strtoul(val, NULL, 10);
    _cw_assert(2 <= base);
    _cw_assert(36 >= base);
  }
  else
  {
    /* Default to base 10. */
    base = 10;
  }
  
  /* Determine sign. */
  if ((-1 != (val_len = spec_get_val(a_format, a_len, "s", &val)))
      && ('s' == val[0])
      && (0 != (arg & (((cw_uint64_t) 1) << (a_nbits - 1)))))
  {
    is_negative = TRUE;
    /* Convert two's complement to positive. */
    arg ^= ((cw_uint64_t) 0xffffffff << 32) + 0xffffffff;
    arg++;
  }
  else
  {
    is_negative = FALSE;
  }

  /* Should we show the sign if the number is positive? */
  if (((-1 != (val_len = spec_get_val(a_format, a_len, "+", &val)))
       && ('+' == val[0]))
      || (TRUE == is_negative))
  {
    show_sign = TRUE;
  }
  else
  {
    show_sign = FALSE;
  }
    
  for (i = 0; i < a_nbits; i++)
  {
    if ((arg >> i) & 1)
    {
      /* Copy the result for use in the next call. */
      out_p_add(base, a_nbits, temp, result, zero);
      
      /* Add this digit into the result. */
      out_p_add(base, a_nbits, result, temp, curr_add);
    }
    /* Copy curr_add for use in the next call. */
    out_p_add(base, a_nbits, temp, curr_add, zero);

    /* Double curr_add. */
    out_p_add(base, a_nbits, curr_add, temp, temp);
  }

  /* Find the first non-zero digit. */
  for (i = 0; i < (a_nbits - 1); i++)
  {
    if (result[i] != '0')
    {
      break;
    }
  }

  width = out_p_metric_int(a_format, a_len, a_arg, a_nbits);
  out_len = (a_nbits - i) + (show_sign ? 1 : 0);

  if (width > out_len)
  {
    char pad, justify, * output;
    
    /* Padding needed.  memset() the output string to the padding character,
     * then determine where to render the integer based on justification. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "p", &val)))
    {
      pad = val[0];
    }
    else
    {
      pad = ' ';
    }
    memset(r_buf, pad, width);

    if (-1 != (val_len = spec_get_val(a_format, a_len, "j", &val)))
    {
      justify = val[0];
    }
    else
    {
      justify = 'r';
    }

    switch (justify)
    {
      case 'r':
      {
	output = &r_buf[width - out_len];
	break;
      }
      case 'l':
      {
	output = r_buf;
	break;
      }
      case 'c':
      {
	output = &r_buf[(width - out_len) / 2];
	break;
      }
      default:
      {
	_cw_error("Unknown justification");
      }
    }

    if (TRUE == show_sign)
    {
      output[0] = (is_negative) ? '-' : '+';
      memcpy(&output[1], &result[i], a_nbits - i);
    }
    else
    {
      memcpy(output, &result[i], a_nbits - i);
    }
  }
  else
  {
    if (TRUE == show_sign)
    {
      r_buf[0] = (is_negative) ? '-' : '+';
      memcpy(&r_buf[1], &result[i], a_nbits - i);
    }
    else
    {
      memcpy(r_buf, &result[i], a_nbits - i);
    }
  }

  retval = r_buf;

/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "r_buf \""); */
/*    log_nprintf(cw_g_log, width, "%s", r_buf); */
/*    log_printf(cw_g_log, "\"\n"); */
  return retval;
}

static cw_uint32_t
out_p_metric_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg)
{
  cw_uint32_t retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint8_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 8);

  return retval;
}

static char *
out_p_render_int8(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint8_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 8);

  return retval;
}

static cw_uint32_t
out_p_metric_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint16_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 16);

  return retval;
}

static char *
out_p_render_int16(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint16_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 16);

  return retval;
}

static cw_uint32_t
out_p_metric_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint32_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 32);

  return retval;
}

static char *
out_p_render_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint32_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 32);

  return retval;
}

static cw_uint32_t
out_p_metric_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_uint32_t retval;
  cw_uint64_t arg = *(const cw_uint64_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 64);

  return retval;
}

static char *
out_p_render_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = *(const cw_uint64_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 64);

  return retval;
}

static cw_uint32_t
out_p_metric_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg)
{
  cw_uint32_t retval, len, width;
  cw_sint32_t val_len;
  const char * val, * str = *(const char **) a_arg;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);

  len = strlen(str);
  
  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", &val)))
  {
    /* Width specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    width = strtoul(val, NULL, 10);
    if (width < len)
    {
      retval = width;
      goto RETURN;
    }
  }
  
  retval = len;

  RETURN:
/*    log_eprintf(cw_g_log, __FILE__, __LINE__, __FUNCTION__, */
/*  	      "str == %010p, retval == %lu: \"", str, retval); */
/*    log_nprintf(cw_g_log, retval, "%s", str); */
/*    log_printf(cw_g_log, "\"\n"); */
  return retval;
}

static char *
out_p_render_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg, char * r_buf)
{
  char * retval;
  const char * str = *(const char **) a_arg;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);
  _cw_check_ptr(r_buf);

  /* XXX Add p: w: j: */

  memcpy(r_buf, str, out_p_metric_string(a_format, a_len, a_arg));

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
