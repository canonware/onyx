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

/* Various include files needed in order to see the prototypes for their
 * printing functions. */
#define _LIBSTASH_USE_BUF

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
  _cw_check_ptr(a_out);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_out->magic);

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
  _cw_assert(_LIBSTASH_OUT_MAX_TYPE >= strlen(a_type));
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

  memcpy(a_out->extensions[a_out->nextensions].type, a_type, strlen(a_type));
  a_out->extensions[a_out->nextensions].len = strlen(a_type);
  a_out->extensions[a_out->nextensions].size = a_size;
  a_out->extensions[a_out->nextensions].metric_func = a_metric_func;
  a_out->extensions[a_out->nextensions].render_func = a_render_func;

  a_out->nextensions++;

  retval = FALSE;
  
  RETURN:
  return retval;
}

cw_bool_t
out_merge(cw_out_t * a_a, cw_out_t * a_b)
{
  cw_bool_t retval;
  cw_sint32_t i;
  
  _cw_check_ptr(a_a);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_a->magic);
  _cw_check_ptr(a_b);
  _cw_assert(_LIBSTASH_OUT_MAGIC == a_b->magic);

  if (0 < a_b->nextensions)
  {
    if (NULL == a_a->extensions)
    {
      a_a->extensions = (cw_out_ent_t *) _cw_calloc(a_b->nextensions,
						    sizeof(cw_out_ent_t));
      if (NULL == a_a->extensions)
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      cw_out_ent_t * t_ptr;

      t_ptr = (cw_out_ent_t *) _cw_realloc(a_a->extensions,
					   ((a_a->nextensions
					     + a_b->nextensions)
					    * sizeof(cw_out_ent_t)));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }

      a_a->extensions = t_ptr;
    }

    memcpy(&a_a->extensions[a_a->nextensions],
	   a_b->extensions,
	   a_b->nextensions * sizeof(cw_out_ent_t));

    /* Make copies of the type strings. */
    for (i = 0; i < a_b->nextensions; i++)
    {
      memcpy(a_b->extensions[i].type,
	     a_a->extensions[i + a_a->nextensions].type,
	     _LIBSTASH_OUT_MAX_TYPE);
    }
    
    a_a->nextensions += a_b->nextensions;
  }
  
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
  retval = out_p_put_fvle(a_out, fd, FALSE, a_file_name, a_line_num,
			  a_func_name, a_format, ap);
  va_end(ap);
  
  return retval;
}

cw_sint32_t
out_put_l(cw_out_t * a_out, const char * a_format, ...)
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
  retval = out_p_put_fvle(a_out, fd, TRUE, NULL, 0, NULL, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_le(cw_out_t * a_out,
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
  retval = out_p_put_fvle(a_out, fd, TRUE, a_file_name, a_line_num,
			  a_func_name, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_n(cw_out_t * a_out, cw_uint32_t a_size, const char * a_format, ...)
{
  cw_sint32_t retval, fd, metric;
  va_list ap;
  cw_out_key_t key;

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
  metric = out_p_metric(a_out, a_format, &key, ap);
  if (0 >= metric)
  {
    retval = metric;
    goto RETURN;
  }
  
  retval = out_p_put_fvn(a_out, fd, a_size, &key, a_format, ap);
  va_end(ap);

  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
  }
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
  retval = out_p_put_fvle(a_out, a_fd, FALSE, a_file_name, a_line_num,
			  a_func_name, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_fl(cw_out_t * a_out, cw_sint32_t a_fd, const char * a_format, ...)
{
  cw_sint32_t retval;
  va_list ap;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_p_put_fvle(a_out, a_fd, TRUE, NULL, 0, NULL, a_format, ap);
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

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  retval = out_p_put_fvle(a_out, a_fd, TRUE, a_file_name, a_line_num,
			  a_func_name, a_format, ap);
  va_end(ap);

  return retval;
}

cw_sint32_t
out_put_fn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	   const char * a_format, ...)
{
  cw_sint32_t retval, metric;
  va_list ap;
  cw_out_key_t key;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  
  va_start(ap, a_format);
  metric = out_p_metric(a_out, a_format, &key, ap);
  if (0 >= metric)
  {
    retval = metric;
    goto RETURN;
  }
  
  retval = out_p_put_fvn(a_out, a_fd, a_size, &key, a_format, ap);
  va_end(ap);

  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
  }
  return retval;
}

cw_sint32_t
out_put_fv(cw_out_t * a_out, cw_sint32_t a_fd,
	   const char * a_format, va_list a_p)
{
  cw_sint32_t retval, metric;
  cw_out_key_t key;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);

  metric = out_p_metric(a_out, a_format, &key, a_p);
  if (0 >= metric)
  {
    retval = metric;
    goto RETURN;
  }

  retval = out_p_put_fvn(a_out, a_fd, metric, &key, a_format, a_p);
  
  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
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
  cw_sint32_t retval, metric;
  cw_out_key_t key;

  _cw_check_ptr(a_str);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  metric = out_p_metric(a_out, a_format, &key, a_p);
  if (0 >= metric)
  {
    retval = metric;
    goto RETURN;
  }
  
  retval = out_p_put_svn(a_out, a_str, metric, &key, a_format, a_p);
  if (0 > retval)
  {
    goto RETURN;
  }
  
  a_str[retval] = '\0';

  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
  }
  return retval;
}

cw_sint32_t
out_put_sva(cw_out_t * a_out, char ** r_str,
	    const char * a_format, va_list a_p)
{
  cw_sint32_t retval, metric;
  cw_out_key_t key;

  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);
  
  metric = out_p_metric(a_out, a_format, &key, a_p);
  if (0 > metric)
  {
    retval = metric;
    goto RETURN;
  }

  retval = out_p_put_sva(a_out, r_str, &key, a_format, a_p);

  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
  }
  return retval;
}

cw_sint32_t
out_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	    const char * a_format, va_list a_p)
{
  cw_sint32_t retval, metric;
  cw_out_key_t key;

  _cw_check_ptr(a_str);
  _cw_assert(0 <= a_size);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);

  metric = out_p_metric(a_out, a_format, &key, a_p);
  if (0 >= metric)
  {
    retval = metric;
    goto RETURN;
  }

  retval = out_p_put_svn(a_out, a_str, a_size, &key, a_format, a_p);
  
  RETURN:
  if (key.format_key_buf != key.format_key)
  {
    /* out_p_metric() allocated a new spec key. */
    _cw_free(key.format_key);
  }
  return retval;
}

cw_sint32_t
spec_get_type(const char * a_spec, cw_uint32_t a_spec_len, const char ** r_val)
{
  cw_sint32_t retval, i;

  _cw_check_ptr(a_spec);
  _cw_assert(0 < a_spec_len);
  _cw_check_ptr(r_val);
  
  for (i = 0; i < a_spec_len; i++)
  {
    if ('|' == a_spec[i])
    {
      break;
    }
  }

  *r_val = a_spec;
  retval = i;

  return retval;
}

cw_sint32_t
spec_get_val(const char * a_spec, cw_uint32_t a_spec_len,
	     const char * a_name, cw_uint32_t a_name_len,
	     const char ** r_val)
{
  cw_sint32_t retval, i, curr_name_len, val_len;
  cw_bool_t match;
  enum
  {
    NAME,
    VALUE
  } state;

  _cw_check_ptr(a_spec);
  _cw_assert(0 < a_spec_len);
  _cw_check_ptr(a_name);
  _cw_check_ptr(r_val);

  curr_name_len = 0; /* Shut up the optimizer warnings. */

  for (i = val_len = 0, match = FALSE, state = VALUE;
       i < a_spec_len;
       i++)
  {
    switch (state)
    {
      case NAME:
      {
	if (':' == a_spec[i])
	{
	  if (a_name_len != curr_name_len)
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

static cw_sint32_t
out_p_put_fvle(cw_out_t * a_out, cw_sint32_t a_fd,
	       cw_bool_t a_time_stamp,
	       const char * a_file_name,
	       cw_uint32_t a_line_num,
	       const char * a_func_name,
	       const char * a_format,
	       va_list a_p)
{
  cw_sint32_t retval;
  char * format = NULL, timestamp[128];

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);

  if (TRUE == a_time_stamp)
  {
    time_t curr_time;
    struct tm * cts;
    
    curr_time = time(NULL);
    cts = localtime(&curr_time);
    if (0 == strftime(timestamp, sizeof(timestamp), "[[%Y/%m/%d %T %Z]: ", cts))
    {
      /* Wow, this locale must be *really* verbose about displaying time.
       * Terminate the string, since there's no telling what's there. */
      timestamp[0] = '\0';
    }
  }
  else
  {
    timestamp[0] = '\0';
  }
    
  if (NULL != a_file_name)
  {
    if (NULL != a_func_name)
    {
      /* Print filename, line number, and function name. */
      if (-1 == out_put_sa(a_out, &format,
			   "[s]At [s], line [i]: [s](): [s]",
			   timestamp,
			   a_file_name, a_line_num, a_func_name,
			   a_format))
      {
	retval = -1;
	goto RETURN;
      }
    }
    else
    {
      /* Print filename and line number. */
      if (-1 == out_put_sa(a_out, &format,
			   "[s]At [s], line [i]: [s]",
			   timestamp,
			   a_file_name, a_line_num,
			   a_format))
      {
	retval = -1;
	goto RETURN;
      }
    }
  }
  else if (NULL != a_func_name)
  {
    /* Print function name. */
    if (-1 == out_put_sa(a_out, &format,
			 "[s][s](): [s]",
			 timestamp,
			 a_func_name,
			 a_format))
    {
      retval = -1;
      goto RETURN;
    }
  }
  else
  {
    /* Make no modifications. */
    if (-1 == out_put_sa(a_out, &format,
			 "[s][s]",
			 timestamp,
			 a_format))
    {
      retval = -1;
      goto RETURN;
    }
  }

  retval = out_put_fv(a_out, a_fd, format, a_p);
  
  RETURN:
  if (NULL != format)
  {
    /* This string was allocated using the mem class.  Free it as such. */
#ifdef _LIBSTASH_DBG
    mem_free(cw_g_mem, format, __FILE__, __LINE__);
#else
    mem_free(cw_g_mem, format);
#endif
  }
  return retval;
}

static cw_sint32_t
out_p_put_fvn(cw_out_t * a_out, cw_sint32_t a_fd, cw_uint32_t a_size,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p)
{
  cw_sint32_t retval, i, out_size, nwritten;
  cw_bool_t malloced_output;
  char output_buf[_LIBSTASH_OUT_PRINT_BUF];
  char * output;

  _cw_assert(0 <= a_fd);
  _cw_check_ptr(a_format);

  if (TRUE == a_key->raw)
  {
    malloced_output = FALSE;
    out_size = a_size;
    output = (char *) a_format;
  }
  else
  {
    if (_LIBSTASH_OUT_PRINT_BUF >= a_size)
    {
      malloced_output = FALSE;
      output = output_buf;
    }
    else
    {
      malloced_output = TRUE;
      output = (char *) _cw_malloc(a_size);
      if (NULL == output)
      {
	retval = -1;
	goto RETURN;
      }
    }
  
    if (0 > (out_size = out_p_put_svn(a_out, output, a_size, a_key,
				      a_format, a_p)))
    {
      retval = -1;
      goto RETURN;
    }
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
/*    fsync(a_fd); */

  retval = i;
  
  RETURN:
  if ((TRUE == malloced_output) && (NULL != output))
  {
    _cw_free(output);
  }
  return retval;
}

cw_sint32_t
out_p_put_sva(cw_out_t * a_out, char ** r_str,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p)
{
  cw_sint32_t retval;
  char * output = NULL;

  /* Since this string will get passed out to the user, we must allocate it
   * using the mem class.  Otherwise, if the user tries to free the string with
   * mem_free(), an error will occur. */
#ifdef _LIBSTASH_DBG
  output = (char *) mem_malloc(cw_g_mem, a_key->metric + 1, __FILE__, __LINE__);
#else
  output = (char *) mem_malloc(cw_g_mem, a_key->metric + 1);
#endif
  if (NULL == output)
  {
    retval = -1;
    goto RETURN;
  }

  retval = out_p_put_svn(a_out, output, a_key->metric, a_key, a_format, a_p);
  _cw_assert(a_key->metric == retval);

  RETURN:
  if (0 <= retval)
  {
    if (NULL != output)
    {
      output[retval] = '\0';
    }
    *r_str = output;
  }
  else
  {
    *r_str = NULL;
  }
  return retval;
}

cw_sint32_t
out_p_put_svn(cw_out_t * a_out, char * a_str, cw_uint32_t a_size,
	      cw_out_key_t * a_key,
	      const char * a_format, va_list a_p)
{
  cw_sint32_t retval, size, format_len, i, j, enti;
  cw_uint32_t metric;

  _cw_check_ptr(a_str);
  _cw_assert(0 <= a_size);
  _cw_check_ptr(a_format);
  _cw_check_ptr(a_p);

  /* Choose the smaller of two possible sizes. */
  if (a_key->metric < a_size)
  {
    size = a_key->metric;
  }
  else
  {
    size = (cw_sint32_t) a_size;
  }

  if (TRUE == a_key->raw)
  {
    memcpy(a_str, a_format, size);
  }
  else
  {
    for (i = j = enti = 0, format_len = strlen(a_format);
	 (i < format_len) && (j < size);
	 )
    {
      switch (a_key->format_key[i])
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
	  const char * type;
	  cw_out_ent_t * ent;
	  void * arg = NULL; /* Shut up the optimizer. */
	
	  if (_LIBSTASH_OUT_ENT_CACHE > enti)
	  {
	    spec_len = a_key->ents[enti].spec_len;
	    ent = a_key->ents[enti].ent;
	  }
	  else
	  {
	    /* Calculate the specifier length.  We're guaranteed that there is a
	     * whiteout character following the specifier. */
	    for (spec_len = 0;
		 a_key->format_key[i + spec_len] == _LIBSTASH_OUT_DES_SPECIFIER;
		 spec_len++);
	  
	    /* Find the type string. */
	    type_len = spec_get_type(&a_format[i], spec_len, &type);
	    _cw_assert(0 <= type_len);
	  
	    ent = out_p_get_ent(a_out, type, type_len);
	    _cw_assert(NULL != ent);
	  }
	
	  switch (ent->size)
	  {
	    case 1:
	    case 2:
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
#ifdef _TYPE_FP96_DEFINED
	    case 12:
	    {
	      arg = (void *) &va_arg(a_p, cw_fp96_t);
	      break;
	    }
#endif
#ifdef _TYPE_FP128_DEFINED
	    case 16:
	    {
	      arg = (void *) &va_arg(a_p, cw_fp128_t);
	      break;
	    }
#endif
	    default:
	    {
	      _cw_error("Programming error");
	    }
	  }

	  if (_LIBSTASH_OUT_ENT_CACHE > enti)
	  {
	    metric = a_key->ents[enti].metric;
	  }
	  else
	  {
	    metric = ent->metric_func(&a_format[i], spec_len, arg);
	    if (0 > metric)
	    {
	      retval = metric;
	      goto RETURN;
	    }
	  }
	  
	  if (j + metric <= size)
	  {
	    /* The printout of this item will fit in the output string. */
	    if (NULL == ent->render_func(&a_format[i], spec_len, arg,
					 &a_str[j]))
	    {
	      retval = -1;
	      goto RETURN;
	    }
	  }
	  else
	  {
	    char * t_buf;
	  
	    /* The printout of this item will not fit in the string.  Therefore,
	     * allocate a temporary buffer, render the item there, then copy as
	     * much as will fit into the output string. */
	    t_buf = (char *) _cw_malloc(metric);
	    if (NULL == t_buf)
	    {
	      retval = -1;
	      goto RETURN;
	    }

	    if (NULL == ent->render_func(&a_format[i], spec_len, arg, t_buf))
	    {
	      _cw_free(t_buf);
	      retval = -1;
	      goto RETURN;
	    }
	    memcpy(&a_str[j], t_buf, size - j);
	  
	    _cw_free(t_buf);
	  }
	
	  j += metric;
	  i += spec_len;
	  enti++;
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
  }
  
  retval = size;

  RETURN:
  return retval;
}

static cw_sint32_t
out_p_metric(cw_out_t * a_out, const char * a_format,
	     cw_out_key_t * a_key,
	     va_list a_p)
{
  cw_sint32_t retval, spec_metric;
  cw_uint32_t i, metric; /* Total number of bytes to be printed. */
  cw_uint32_t spec_len = 0; /* Shut up the optimizer warning. */
  cw_uint32_t next_ent = 0;
  enum
  {
    NORMAL,
    BRACKET,
    NAME,
    VALUE
  } state;

  a_key->format_key = a_key->format_key_buf;
  a_key->raw = TRUE;
  
  for (i = metric = 0, state = NORMAL;
       a_format[i] != '\0';
       i++)
  {
    if ((_LIBSTASH_OUT_SPEC_BUF == i)
	&& (a_key->format_key_buf == a_key->format_key))
    {
      cw_uint32_t format_len;

      /* We just ran out of space in the statically allocated buffer.  Time to
       * face cold hard reality, get the specifier length, allocate a buffer,
       * and copy the static buffer's contents over. */
      format_len = strlen(a_format);
      a_key->format_key = (char *) _cw_malloc(format_len);
      if (NULL == a_key->format_key)
      {
	retval = -1;
	goto RETURN;
      }
#ifdef _LIBSTASH_DBG
      bzero(a_key->format_key, format_len);
#endif
      memcpy(a_key->format_key, a_key->format_key_buf, _LIBSTASH_OUT_SPEC_BUF);
#ifdef _LIBSTASH_DBG
      bzero(a_key->format_key_buf, _LIBSTASH_OUT_SPEC_BUF);
#endif
    }
    
    switch (state)
    {
      case NORMAL:
      {
	if ('[' == a_format[i])
	{
	  /* We can unconditionally white this character out.  If the next
	   * character is a `[', we can leave that one intact. */
	  a_key->format_key[i] = _LIBSTASH_OUT_DES_WHITEOUT;
	  state = BRACKET;
	}
	else
	{
	  a_key->format_key[i] = _LIBSTASH_OUT_DES_NORMAL;
	  metric++;
	}
	
	break;
      }
      case BRACKET:
      {
	a_key->raw = FALSE;
	
	if ('[' == a_format[i])
	{
	  a_key->format_key[i] = _LIBSTASH_OUT_DES_NORMAL;
	  metric++;
	  state = NORMAL;
	}
	else
	{
	  a_key->format_key[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	  spec_len = 1;
	  state = VALUE;
	}
	
	break;
      }
      case NAME:
      {
	a_key->format_key[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	spec_len++;
	
	if (':' == a_format[i])
	{
	  state = VALUE;
	}
	  
	break;
      }
      case VALUE:
      {
	a_key->format_key[i] = _LIBSTASH_OUT_DES_SPECIFIER;
	
	if ('|' == a_format[i])
	{
	  spec_len++;
	  state = NAME;
	}
	else if (']' == a_format[i])
	{
	  const char * val;
	  cw_sint32_t val_len;
	  cw_out_ent_t * ent;
	  
	  a_key->format_key[i] = _LIBSTASH_OUT_DES_WHITEOUT;
	  state = NORMAL;

	  /* Successful completion of parsing this specifier.  Call the
	   * corresponding metric function. */
	  val_len = spec_get_type(&a_format[i - spec_len], spec_len, &val);
	  if (-1 == val_len)
	  {
	    retval = -2;
	    goto RETURN;
	  }

	  ent = out_p_get_ent(a_out, val, val_len);
	  if (NULL == ent)
	  {
	    /* No handler. */
	    retval = -2;
	    goto RETURN;
	  }
	  
	  {
	    void * arg;

	    switch (ent->size)
	    {
	      case 1:
	      case 2:
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
#ifdef _TYPE_FP96_DEFINED
	      case 12:
	      {
		arg = (void *) &va_arg(a_p, cw_fp96_t);
		break;
	      }
#endif
#ifdef _TYPE_FP128_DEFINED
	      case 16:
	      {
		arg = (void *) &va_arg(a_p, cw_fp128_t);
		break;
	      }
#endif
	      default:
	      {
		retval = -2;
		goto RETURN;
	      }
	    }

	    spec_metric = ent->metric_func(&a_format[i - spec_len],
					   spec_len, arg);
	    if (0 > spec_metric)
	    {
	      retval = spec_metric;
	      goto RETURN;
	    }
	    
	    metric += spec_metric;
	  }
	  
	  if (_LIBSTASH_OUT_ENT_CACHE > next_ent)
	  {
	    a_key->ents[next_ent].metric = spec_metric;
	    a_key->ents[next_ent].spec_len = spec_len;
	    a_key->ents[next_ent].ent = ent;
	    next_ent++;
	  }
	}
	else
	{
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
  if (NORMAL != state)
  {
    retval = -2;
    goto RETURN;
  }

  a_key->metric = metric;
  a_key->format_len = i;
  retval = metric;
  
  RETURN:
  return retval;
}

static cw_out_ent_t *
out_p_get_ent(cw_out_t * a_out, const char * a_format, cw_uint32_t a_len)
{
  cw_out_ent_t * retval;
  cw_uint32_t i;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);

  /* Find a match for the type.  Use the first match found by searching the
   * extended types, then the built in types.  If there is no match, return an
   * error, since we have no way of knowing the size of argument to use. */
  if (NULL != a_out)
  {
    for (i = 0;
	 i < a_out->nextensions;
	 i++)
    {
      if (0 == strncmp(a_format, a_out->extensions[i].type,
		       a_out->extensions[i].len)
	  && (a_len == a_out->extensions[i].len))
      {
	retval = &a_out->extensions[i];
	goto RETURN;
      }
    }
  }

  for (i = 0;
       i < (sizeof(cw_g_out_builtins) / sizeof(struct cw_out_ent_s));
       i++)
  {
    if (0 == strncmp(a_format, cw_g_out_builtins[i].type,
		     cw_g_out_builtins[i].len)
	&& (a_len == cw_g_out_builtins[i].len))
    {
      retval = &cw_g_out_builtins[i];
      goto RETURN;
    }
  }

  retval = NULL;

  RETURN:
  return retval;
}

static cw_sint32_t
out_p_metric_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg,
		 cw_uint32_t a_nbits, cw_uint32_t a_default_base)
{
  cw_sint32_t retval, val_len;
  cw_uint32_t width, base, i;
  cw_uint64_t arg = a_arg;
  cw_bool_t is_negative, show_sign;
  const char * val;
  char * syms = "0123456789abcdefghijklmnopqrstuvwxyz";
  char * result, s_result[65] =
    "0000000000000000000000000000000000000000000000000000000000000000";

  _cw_assert((8 == a_nbits) || (16 == a_nbits)
	     || (32 == a_nbits) || (64 == a_nbits));

  /* Move the pointer forward so that unnecessary digits can be ignored. */
  result = &s_result[64 - a_nbits];

  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", 1, &val)))
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
  if ((-1 != (val_len = spec_get_val(a_format, a_len, "s", 1, &val)))
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
  if (((-1 != (val_len = spec_get_val(a_format, a_len, "+", 1, &val)))
       && ('+' == val[0]))
      || (TRUE == is_negative))
  {
    show_sign = TRUE;
  }
  else
  {
    show_sign = FALSE;
  }
    
  if (-1 != (val_len = spec_get_val(a_format, a_len, "b", 1, &val)))
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
    base = a_default_base;
  }

  /* Treat 64 bit numbers separately, since they're much slower on 32 bit
   * architectures. */
  if (64 != a_nbits)
  {
    cw_uint32_t rval = (cw_uint32_t) arg;
    
    for (i = a_nbits - 1; rval != 0; i--)
    {
      result[i] = syms[rval % base];
      rval /= base;
    }
  }
  else
  {
    cw_uint64_t rval = (cw_uint64_t) arg;
    
    for (i = a_nbits - 1; rval != 0; i--)
    {
      result[i] = syms[rval % base];
      rval /= base;
    }
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

  return retval;
}

static char *
out_p_render_int(const char * a_format, cw_uint32_t a_len,
		 cw_uint64_t a_arg, char * r_buf,
		 cw_uint32_t a_nbits, cw_uint32_t a_default_base)
{
  char * retval;
  cw_uint32_t base, width, out_len, i;
  cw_sint32_t val_len;
  cw_uint64_t arg = a_arg;
  cw_bool_t is_negative, show_sign;
  const char * val;
  char * syms = "0123456789abcdefghijklmnopqrstuvwxyz";
  char * result, s_result[65] =
    "0000000000000000000000000000000000000000000000000000000000000000";

  _cw_assert((8 == a_nbits) || (16 == a_nbits)
	     || (32 == a_nbits) || (64 == a_nbits));

  /* Move the pointer forward so that unnecessary digits can be ignored. */
  result = &s_result[64 - a_nbits];

  if (-1 != (val_len = spec_get_val(a_format, a_len, "b", 1, &val)))
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
    base = a_default_base;
  }
  
  /* Determine sign. */
  if ((-1 != (val_len = spec_get_val(a_format, a_len, "s", 1, &val)))
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
  if (((-1 != (val_len = spec_get_val(a_format, a_len, "+", 1, &val)))
       && ('+' == val[0]))
      || (TRUE == is_negative))
  {
    show_sign = TRUE;
  }
  else
  {
    show_sign = FALSE;
  }

  /* Treat 64 bit numbers separately, since they're much slower on 32 bit
   * architectures. */
  if (64 != a_nbits)
  {
    cw_uint32_t rval = (cw_uint32_t) arg;
    
    for (i = a_nbits - 1; rval != 0; i--)
    {
      result[i] = syms[rval % base];
      rval /= base;
    }
  }
  else
  {
    cw_uint64_t rval = arg;

    for (i = a_nbits - 1; rval != 0; i--)
    {
      result[i] = syms[rval % base];
      rval /= base;
    }
  }
  
  /* Find the first non-zero digit. */
  for (i = 0; i < (a_nbits - 1); i++)
  {
    if (result[i] != '0')
    {
      break;
    }
  }

  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", 1, &val)))
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
  
  out_len = (a_nbits - i) + (show_sign ? 1 : 0);

  if (width > out_len)
  {
    char pad, justify, * output;
    
    /* Padding needed.  memset() the output string to the padding character,
     * then determine where to render the integer based on justification. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "p", 1, &val)))
    {
      pad = val[0];
    }
    else
    {
      pad = ' ';
    }
    memset(r_buf, pad, width);

    if (-1 != (val_len = spec_get_val(a_format, a_len, "j", 1, &val)))
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

  return retval;
}

static cw_sint32_t
out_p_metric_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_sint32_t retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint32_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 32, 10);

  return retval;
}

static char *
out_p_render_int32(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = (cw_uint64_t) *(const cw_uint32_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 32, 10);

  return retval;
}

static cw_sint32_t
out_p_metric_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg)
{
  cw_sint32_t retval;
  cw_uint64_t arg = *(const cw_uint64_t *) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 64, 10);

  return retval;
}

static char *
out_p_render_int64(const char * a_format, cw_uint32_t a_len,
		   const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint64_t arg = *(const cw_uint64_t *) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 64, 10);

  return retval;
}

static cw_sint32_t
out_p_metric_char(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg)
{
  cw_sint32_t retval, val_len;
  cw_uint32_t width;
  const char * val;

  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", 1, &val)))
  {
    /* Width specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    width = strtoul(val, NULL, 10);
    if (width > 1)
    {
      retval = width;
      goto RETURN;
    }
  }

  retval = 1;

  RETURN:
  return retval;
}

static char *
out_p_render_char(const char * a_format, cw_uint32_t a_len,
		  const void * a_arg, char * r_buf)
{
  char * retval;
  cw_uint32_t width;
  cw_sint32_t val_len;
  const char * val;
  cw_uint8_t pad, c = *(const cw_uint32_t *) a_arg;

  width = out_p_metric_char(a_format, a_len, a_arg);

  if (1 < width)
  {
    /* Padding character. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "p", 1, &val)))
    {
      pad = val[0];
    }
    else
    {
      pad = ' ';
    }

    memset(r_buf, pad, width);

    /* Justification. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "j", 1, &val)))
    {
      switch (val[0])
      {
	case 'r':
	{
	  r_buf[width - 1] = c;
	  break;
	}
	case 'l':
	{
	  r_buf[0] = c;
	  break;
	}
	case 'c':
	{
	  r_buf[width / 2] = c;
	  break;
	}
	default:
	{
	  _cw_error("Unknown justification");
	}
      }
    }
    else
    {
      /* Default to right justification. */
      r_buf[width - 1] = c;
    }
  }
  else
  {
    r_buf[0] = c;
  }

  retval = r_buf;
  
  return retval;
}

static cw_sint32_t
out_p_metric_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg)
{
  cw_sint32_t retval, val_len;
  cw_uint32_t len, width;
  const char * val, * str = *(const char **) a_arg;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);

  len = strlen(str);
  
  if (-1 != (val_len = spec_get_val(a_format, a_len, "w", 1, &val)))
  {
    /* Width specified. */
    /* The next character after val is either `|' or `]', so we don't have to
     * worry about terminating the string that val points to. */
    width = strtoul(val, NULL, 10);
    if (width > len)
    {
      retval = width;
      goto RETURN;
    }
  }
  
  retval = len;

  RETURN:
  return retval;
}

static char *
out_p_render_string(const char * a_format, cw_uint32_t a_len,
		    const void * a_arg, char * r_buf)
{
  char * retval, pad;
  cw_sint32_t val_len;
  const char * val;
  cw_uint32_t len, width;
  const char * str = *(const char **) a_arg;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);
  _cw_check_ptr(r_buf);

  len = strlen(str);
  
  width = out_p_metric_string(a_format, a_len, a_arg);

  if (len < width)
  {
    /* Padding character. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "p", 1, &val)))
    {
      pad = val[0];
    }
    else
    {
      pad = ' ';
    }

    memset(r_buf, pad, width);

    /* Justification. */
    if (-1 != (val_len = spec_get_val(a_format, a_len, "j", 1, &val)))
    {
      switch (val[0])
      {
	case 'r':
	{
	  memcpy(&r_buf[width - len], str, len);
	  break;
	}
	case 'l':
	{
	  memcpy(r_buf, str, len);
	  break;
	}
	case 'c':
	{
	  memcpy(&r_buf[(width - len) / 2], str, len);
	  break;
	}
	default:
	{
	  _cw_error("Unknown justification");
	}
      }
    }
    else
    {
      /* Default to right justification. */
      memcpy(&r_buf[width - len], str, len);
    }
  }
  else
  {
    memcpy(r_buf, str, len);
  }

  retval = r_buf;
  
  return retval;
}

static cw_sint32_t
out_p_metric_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg)
{
  cw_sint32_t retval;
  /* Assumes 32 bit pointer. */
  cw_uint64_t arg = (cw_uint64_t) (cw_uint32_t) *(const void **) a_arg;
  
  retval = out_p_metric_int(a_format, a_len, arg, 32, 16);

  return retval;
}

static char *
out_p_render_pointer(const char * a_format, cw_uint32_t a_len,
		     const void * a_arg, char * r_buf)
{
  char * retval;
  /* Assumes 32 bit pointer. */
  cw_uint64_t arg = (cw_uint64_t) (cw_uint32_t) *(const void **) a_arg;

  retval = out_p_render_int(a_format, a_len, arg, r_buf, 32, 16);

  return retval;
}
