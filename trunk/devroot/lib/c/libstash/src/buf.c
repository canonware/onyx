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

#define _LIBSTASH_USE_BUF
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/buf_p.h"

#ifdef _CW_OS_FREEBSD
#  include <sys/types.h>
#  include <sys/uio.h>
#endif

cw_buf_t *
buf_new(cw_buf_t * a_buf)
{
  return buf_p_new(a_buf, FALSE);
}

cw_buf_t *
buf_new_r(cw_buf_t * a_buf)
{
  return buf_p_new(a_buf, TRUE);
}

void
buf_delete(cw_buf_t * a_buf)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_buf);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_delete(&a_buf->lock);
  }
#endif

  for (i = 0;
       i < a_buf->array_num_valid;
       i++)
  {
    if (NULL != a_buf->bufel_array[(i + a_buf->array_start)
				  & (a_buf->array_size - 1)].bufc)
    {
      bufc_delete(a_buf->bufel_array[(i + a_buf->array_start)
				    & (a_buf->array_size - 1)].bufc);
    }
#ifdef _LIBSTASH_DBG
/*      memset(&a_buf->bufel_array[(i + a_buf->array_start) */
/*  			     & (a_buf->array_size - 1)], */
/*  	   0x5a, */
/*  	   sizeof(cw_bufel_t)); */
    bzero(&a_buf->bufel_array[(i + a_buf->array_start)
			     & (a_buf->array_size - 1)], sizeof(cw_bufel_t));
#endif
  }

  if (a_buf->bufel_array != a_buf->static_bufel_array)
  {
    _cw_free(a_buf->bufel_array);
  }
  
  if (a_buf->cumulative_index != a_buf->static_cumulative_index)
  {
    _cw_free(a_buf->cumulative_index);
  }

  if (a_buf->iov != a_buf->static_iov)
  {
    _cw_free(a_buf->iov);
  }
  
  if (a_buf->is_malloced)
  {
    _cw_free(a_buf);
  }
}

void
buf_dump(cw_buf_t * a_buf, const char * a_prefix)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_prefix);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  out_put(cw_g_out,
	  "[s]| buf_dump()\n",
	  a_prefix);
#ifdef _LIBSTASH_DBG
  out_put(cw_g_out,
	  "[s]|--> magic : 0x[i|b:16]\n",
	  a_prefix, a_buf->magic);
#endif
  out_put(cw_g_out,
	  "[s]|--> is_malloced : [s]\n",
	  a_prefix, (a_buf->is_malloced) ? "TRUE" : "FALSE");
#ifdef _CW_REENTRANT
  out_put(cw_g_out,
	  "[s]|--> is_threadsafe : [s]\n",
	  a_prefix, (a_buf->is_threadsafe) ? "TRUE" : "FALSE");
#endif
  out_put(cw_g_out,
	  "[s]|--> size : [i]\n",
	  a_prefix, a_buf->size);
  out_put(cw_g_out,
	  "[s]|--> array_size : [i]\n",
	  a_prefix, a_buf->array_size);
  out_put(cw_g_out,
	  "[s]|--> array_num_valid : [i]\n",
	  a_prefix, a_buf->array_num_valid);
  out_put(cw_g_out,
	  "[s]|--> array_start : [i]\n",
	  a_prefix, a_buf->array_start);
  out_put(cw_g_out,
	  "[s]|--> array_end : [i]\n",
	  a_prefix, a_buf->array_end);
  out_put(cw_g_out,
	  "[s]|--> is_cumulative_valid : [s]\n",
	  a_prefix, (a_buf->is_cumulative_valid) ? "TRUE" : "FALSE");
  out_put(cw_g_out,
	  "[s]|--> is_cached_bufel_valid : [s]\n",
	  a_prefix, (a_buf->is_cached_bufel_valid) ? "TRUE" : "FALSE");
  out_put(cw_g_out,
	  "[s]|--> cached_bufel : [i]\n",
	  a_prefix, a_buf->cached_bufel);
  
  for (i = 0; i < a_buf->array_size; i++)
  {
    out_put(cw_g_out,
	    "[s]|\\\n"
	    "[s]| |--> cumulative_index[[[i]] : [i]\n"
	    "[s]| |--> bufel_array[[[i]] : \n"
	    "[s]|  \\\n",
	    a_prefix,
	    a_prefix, i, a_buf->cumulative_index[i],
	    a_prefix, i,
	    a_prefix);

    /* Dump bufel. */
#ifdef _LIBSTASH_DBG
    out_put(cw_g_out,
	    "[s]|   |--> magic : 0x[i|b:16]\n",
	    a_prefix, a_buf->bufel_array[i].magic);
#endif
    out_put(cw_g_out,
	    "[s]|   |--> beg_offset : [i]\n",
	    a_prefix, a_buf->bufel_array[i].beg_offset);
    out_put(cw_g_out,
	    "[s]|   |--> end_offset : [i]\n",
	    a_prefix, a_buf->bufel_array[i].end_offset);
#ifdef _LIBSTASH_DBG
    if ((NULL != a_buf->bufel_array[i].bufc)
	&& (_CW_BUFEL_MAGIC == a_buf->bufel_array[i].magic))
#else
    if (NULL != a_buf->bufel_array[i].bufc)
#endif
    {
      char * sub_prefix;
      
      out_put(cw_g_out,
	      "[s]|   |--> bufc : 0x[i|b:16]\n"
	      "[s]|    \\\n",
	      a_prefix, a_buf->bufel_array[i].bufc, a_prefix);
      
      sub_prefix = _cw_malloc(strlen(a_prefix) + 7);
      if (NULL == sub_prefix)
      {
	bufc_p_dump(a_buf->bufel_array[i].bufc, "...");
      }
      else
      {
	out_put_s(cw_g_out, sub_prefix, "[s]|     ", a_prefix);
	bufc_p_dump(a_buf->bufel_array[i].bufc, sub_prefix);
	_cw_free(sub_prefix);
      }
    }
    else
    {
      out_put(cw_g_out,
	      "[s]|   \\--> bufc : 0x[i|b:16] (invalid)\n",
	      a_prefix, a_buf->bufel_array[i].bufc);
    }
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

cw_sint32_t
buf_out_metric(const char * a_format, cw_uint32_t a_len, const void * a_arg)
{
  cw_sint32_t retval, val_len;
  cw_uint32_t len, width;
  const char * val;
  cw_buf_t * buf;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);

  buf = *(cw_buf_t **) a_arg;
  _cw_check_ptr(buf);
  _cw_assert(buf->magic == _CW_BUF_MAGIC);
  
  len = buf_get_size(buf);
  
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

char *
buf_out_render(const char * a_format, cw_uint32_t a_len, const void * a_arg,
	       char * r_buf)
{
  char * retval, * pos, pad;
  cw_sint32_t val_len;
  const char * val;
  cw_uint32_t len, width;
  cw_buf_t * buf;
  const struct iovec * iov;
  int iov_cnt, i;

  _cw_check_ptr(a_format);
  _cw_assert(0 < a_len);
  _cw_check_ptr(a_arg);
  _cw_check_ptr(r_buf);

  buf = *(cw_buf_t **) a_arg;
  _cw_check_ptr(buf);
  _cw_assert(buf->magic == _CW_BUF_MAGIC);
  
  len = buf_get_size(buf);
  
  width = buf_out_metric(a_format, a_len, a_arg);

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
	  pos = &r_buf[width - len];
	  break;
	}
	case 'l':
	{
	  pos = &r_buf[0];
	  break;
	}
	case 'c':
	{
	  pos = &r_buf[(width - len) / 2];
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
      pos = &r_buf[width - len];
    }
  }
  else
  {
    pos = &r_buf[0];
  }

  /* Copy bytes from the buf to the output string.  Use the buf's iovec and
   * memcpy for efficiency. */
  iov = buf_get_iovec(buf, buf_get_size(buf), FALSE, &iov_cnt);
  for (i = 0; i < iov_cnt; i++)
  {
    memcpy(pos, iov[i].iov_base, iov[i].iov_len);
    pos += iov[i].iov_len;
  }

  retval = r_buf;
  
  return retval;
}

cw_uint32_t
buf_get_size(cw_buf_t * a_buf)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  retval = a_buf->size;

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_uint32_t
buf_get_num_bufels(cw_buf_t * a_buf)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  retval = a_buf->array_num_valid;

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

const struct iovec *
buf_get_iovec(cw_buf_t * a_buf, cw_uint32_t a_max_data,
	      cw_bool_t a_is_sys_iovec, int * r_iovec_count)
{
  cw_uint32_t array_index, num_bytes;
  int i;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  for (i = num_bytes = 0;
       (i < a_buf->array_num_valid) && (num_bytes < a_max_data);
       i++)
  {
    array_index = (a_buf->array_start + i) & (a_buf->array_size - 1);
    
    a_buf->iov[i].iov_base
      = (((char *) a_buf->bufel_array[array_index].bufc->buf)
	 + a_buf->bufel_array[array_index].beg_offset);
    a_buf->iov[i].iov_len
      = (size_t) (a_buf->bufel_array[array_index].end_offset
		  - a_buf->bufel_array[array_index].beg_offset);
    
    num_bytes += a_buf->iov[i].iov_len;
  }
  
  /* Adjust the iovec size downward if necessary. */
  if (num_bytes > a_max_data)
  {
    a_buf->iov[i - 1].iov_len -= (num_bytes - a_max_data);
  }

  if ((TRUE == a_is_sys_iovec) && (i > _LIBSTASH_MAX_IOV))
  {
    *r_iovec_count = _LIBSTASH_MAX_IOV;
  }
  else
  {
    *r_iovec_count = i;
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  
  return a_buf->iov;
}

cw_bool_t
buf_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_a);
  _cw_assert(a_a->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_b);
  _cw_assert(a_b->magic == _CW_BUF_MAGIC);
  _cw_assert(a_a != a_b);
  
#ifdef _CW_REENTRANT
  if (a_a->is_threadsafe == TRUE)
  {
    mtx_lock(&a_a->lock);
  }
  if (a_b->is_threadsafe == TRUE)
  {
    mtx_lock(&a_b->lock);
  }
#endif

  retval = buf_p_catenate_buf(a_a, a_b, a_preserve);
  
#ifdef _CW_REENTRANT
  if (a_b->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_b->lock);
  }
  if (a_a->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_a->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_split(cw_buf_t * a_a, cw_buf_t * a_b, cw_uint32_t a_offset)
{
  cw_bool_t retval;
  cw_uint32_t array_element, bufel_offset, num_bufels_to_move;
  cw_uint32_t i, a_a_index, a_b_index;

  _cw_check_ptr(a_a);
  _cw_assert(a_a->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_b);
  _cw_assert(a_b->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= buf_get_size(a_b));
  _cw_assert(a_a != a_b);

#ifdef _CW_REENTRANT
  if (a_a->is_threadsafe == TRUE)
  {
    mtx_lock(&a_a->lock);
  }
  if (a_b->is_threadsafe == TRUE)
  {
    mtx_lock(&a_b->lock);
  }
#endif
    
  if ((a_offset > 0) && (a_offset < a_b->size))
  {
    buf_p_get_data_position(a_b, a_offset, &array_element, &bufel_offset);

    num_bufels_to_move = (((array_element >= a_b->array_start)
			   ? array_element - a_b->array_start
			   : (array_element + a_b->array_size
			      - a_b->array_start)));
    if (bufel_offset != 0)
    {
      num_bufels_to_move++;
    }

    /* Make sure that a_a's array is big enough.  Doing this here instead of
     * after trying to do a bufel merge means that we might be expanding the
     * arrays unnecessarily.  However, error recovery after the bufel merge is
     * particularly nasty. */
    if (buf_p_fit_array(a_a, a_a->array_num_valid + num_bufels_to_move))
    {
      retval = TRUE;
      goto RETURN;
    }

    /* Try to merge first bufel of a_b and last bufel of a_a. */
    if ((num_bufels_to_move > 1) && (a_a->array_num_valid > 0))
    {
      cw_uint32_t last_element_index;
    
      last_element_index = ((a_a->array_end + a_a->array_size - 1)
			    & (a_a->array_size - 1));

      if ((NULL != a_a->bufel_array[last_element_index].bufc->buf)
	  && (a_a->bufel_array[last_element_index].bufc->buf
	      == a_b->bufel_array[a_b->array_start].bufc->buf)
	  && (a_a->bufel_array[last_element_index].end_offset
	      == a_b->bufel_array[a_b->array_start].beg_offset))
      {
	/* These two bufel's reference the same bufc, and the buffer regions
	 * they refer to are consecutive and adjacent.  Merge. */
	a_a->bufel_array[last_element_index].end_offset
	  = (a_a->bufel_array[last_element_index].end_offset
	     + (a_b->bufel_array[a_b->array_start].end_offset
		- a_b->bufel_array[a_b->array_start].beg_offset));

	a_a->size
	  += (a_b->bufel_array[a_b->array_start].end_offset
	      - a_b->bufel_array[a_b->array_start].beg_offset);
	a_a->cumulative_index[last_element_index] = a_a->size;
      
	num_bufels_to_move--;

	/* Need to decrement the bufc's reference count. */
	bufc_delete(a_b->bufel_array[a_b->array_start].bufc);
	
#ifdef _LIBSTASH_DBG
	bzero(&a_b->bufel_array[a_b->array_start], sizeof(cw_bufel_t));
	bzero(&a_b->cumulative_index[a_b->array_start],
	      sizeof(cw_uint32_t));
#endif
	a_b->array_start = (a_b->array_start + 1) & (a_b->array_size - 1);
	a_b->array_num_valid--;
      }
    }
    else if ((num_bufels_to_move == 1) && (a_a->array_num_valid > 0))
    {
      cw_uint32_t last_element_index;

      last_element_index = ((a_a->array_end + a_a->array_size - 1)
			    & (a_a->array_size - 1));

      if ((NULL != a_a->bufel_array[last_element_index].bufc->buf)
	  && (a_a->bufel_array[last_element_index].bufc->buf
	      == a_b->bufel_array[a_b->array_start].bufc->buf)
	  && (a_a->bufel_array[last_element_index].end_offset
	      == a_b->bufel_array[a_b->array_start].beg_offset))
      {
	/* These two bufel's reference the same bufc, and the buffer regions
	 * they refer to are consecutive and adjacent.  Merge a_b into a_a. */
	a_a->bufel_array[last_element_index].end_offset
	  = (a_a->bufel_array[last_element_index].end_offset
	     + (a_b->bufel_array[a_b->array_start].end_offset
		- a_b->bufel_array[a_b->array_start].beg_offset));

	a_a->size += a_offset;
	a_a->cumulative_index[last_element_index] = a_a->size;
      
	num_bufels_to_move--;

	if ((a_b->bufel_array[a_b->array_start].end_offset
	     - a_b->bufel_array[a_b->array_start].beg_offset)
	    == a_offset)
	{
	  /* Need to decrement the bufc's reference count. */
	  bufc_delete(a_b->bufel_array[a_b->array_start].bufc);
#ifdef _LIBSTASH_DBG
	  bzero(&a_b->bufel_array[a_b->array_start], sizeof(cw_bufel_t));
	  bzero(&a_b->cumulative_index[a_b->array_start],
		sizeof(cw_uint32_t));
#endif
	  a_b->array_start = (a_b->array_start + 1) & (a_b->array_size - 1);
	  a_b->array_num_valid--;
	}
	else
	{
	  a_b->bufel_array[a_b->array_start].beg_offset
	    = a_b->bufel_array[a_b->array_start].beg_offset + a_offset;
	  a_a->bufel_array[last_element_index].end_offset
	    = (a_a->bufel_array[last_element_index].end_offset
	       - (a_b->bufel_array[a_b->array_start].end_offset
		  - a_b->bufel_array[a_b->array_start].beg_offset));
	}
      }
    }

    if (num_bufels_to_move > 0)
    {
#ifdef _LIBSTASH_DBG
      /* Non-destructively copy all the bufel's we care about. */
      buf_p_copy_array(a_a,
		       a_b,
		       num_bufels_to_move,
		       a_a->array_end,
		       a_b->array_start,
		       FALSE);
      /* Destructively copy all but perhaps the last bufel, in order to zero out
       * a_b's copy. */
      buf_p_copy_array(a_a,
		       a_b,
		       num_bufels_to_move - (bufel_offset == 0 ? 0 : 1),
		       a_a->array_end,
		       a_b->array_start,
		       TRUE);
#else
      buf_p_copy_array(a_a,
		       a_b,
		       num_bufels_to_move,
		       a_a->array_end,
		       a_b->array_start);
#endif

      /* Iterate through the bufel's in a_b and move them to a_a, up to and
       * including the bufel where the split occurs. */
      for (i = 0,
	     a_a_index = a_a->array_end,
	     a_b_index = a_b->array_start;
	   i < num_bufels_to_move;
	   i++)
      {
	a_a_index = (i + a_a->array_end) & (a_a->array_size - 1);
	a_b_index = (i + a_b->array_start) & (a_b->array_size - 1);

	a_a->size += (a_a->bufel_array[a_a_index].end_offset
		      - a_a->bufel_array[a_a_index].beg_offset);
	a_a->cumulative_index[a_a_index] = a_a->size;
      }

      /* Deal with the bufel that the split is in. */
      if (bufel_offset != 0)
      {
#ifdef _LIBSTASH_DBG
	/* Copy the bufel back to a_b, since the data is split and the original
	 * bufel must still remain valid. */
	memcpy(&a_b->bufel_array[a_b_index],
	       &a_a->bufel_array[a_a_index],
	       sizeof(cw_bufel_t));
#endif
	/* Decrement a_a->size, since we don't want the whole bufc. */
	a_a->size -= (a_a->bufel_array[a_a_index].end_offset
		      - a_a->bufel_array[a_a_index].beg_offset);

	/* Increment the reference count for the buffer, and set the offsets
	 * appropriately for both bufel's. */
	bufc_p_ref_increment(a_a->bufel_array[a_a_index].bufc);
		  
	a_a->bufel_array[a_a_index].end_offset = bufel_offset;
	a_b->bufel_array[a_b_index].beg_offset = bufel_offset;

	a_a->size += (a_a->bufel_array[a_a_index].end_offset
		      - a_a->bufel_array[a_a_index].beg_offset);
	a_a->cumulative_index[a_a_index] = a_a->size;
      }

      /* Make a_a's and a_b's states consistent. */
      a_a->array_num_valid += num_bufels_to_move;
      a_a->array_end = (a_a_index + 1) & (a_a->array_size - 1);
  
      a_b->array_num_valid -= num_bufels_to_move;
      if (bufel_offset != 0)
      {
	a_b->array_num_valid++;
	a_b->array_start = a_b_index;
      }
      else
      {
	a_b->array_start = (a_b_index + 1) & (a_b->array_size - 1);
      }
    }
    
    a_b->size -= a_offset;
    a_b->is_cumulative_valid = FALSE;
    a_b->is_cached_bufel_valid = FALSE;
      
    if ((a_b->array_num_valid == 0)
	&& (a_b->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
/*        a_b->array_start = 0; */
/*        a_b->array_end = 0; */
      a_b->is_cumulative_valid = TRUE;
      a_b->is_cached_bufel_valid = FALSE;
/*        a_b->cached_bufel = 0; */
    }
  }
  else if ((a_offset > 0) && (a_offset == a_b->size))
  {
    /* Same as catenation. */
    if (buf_p_catenate_buf(a_a, a_b, FALSE))
    {
      retval = TRUE;
      goto RETURN;
    }
  }
  
  retval = FALSE;
  
  RETURN:
#ifdef _CW_REENTRANT
  if (a_b->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_b->lock);
  }
  if (a_a->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_a->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_prepend_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		 cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  _cw_assert(a_end_offset <= a_bufc->buf_size);
  _cw_assert(a_beg_offset <= a_end_offset);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (NULL != a_bufc->buf)
  {
    /* Try to merge a_bufc into the first bufel in a_buf. */
    if ((a_buf->array_num_valid > 0)
	&& (a_buf->bufel_array[a_buf->array_start].bufc->buf
	    == a_bufc->buf)
	&& (a_buf->bufel_array[a_buf->array_start].end_offset
	    == a_beg_offset))
    {
      /* Two references to the same bufc, and the buffer regions they refer to
       * are consecutive and adjacent.  Merge. */
      a_buf->bufel_array[a_buf->array_start].end_offset
	= (a_buf->bufel_array[a_buf->array_start].end_offset
	   + (a_end_offset - a_beg_offset));
      
      a_buf->size += (a_end_offset - a_beg_offset);
      
      a_buf->cumulative_index[a_buf->array_start] = a_buf->size;
    }
    else
    {
      if (buf_p_fit_array(a_buf, a_buf->array_num_valid + 1))
      {
	retval = TRUE;
	goto RETURN;
      }
  
      /* Now prepend the bufel. */
      a_buf->array_start = (((a_buf->array_start + a_buf->array_size) - 1)
			    & (a_buf->array_size - 1));
      a_buf->array_num_valid++;

#ifdef _LIBSTASH_DBG
      a_buf->bufel_array[a_buf->array_start].magic = _CW_BUFEL_MAGIC;
#endif
      a_buf->bufel_array[a_buf->array_start].beg_offset = a_beg_offset;
      a_buf->bufel_array[a_buf->array_start].end_offset = a_end_offset;
      a_buf->bufel_array[a_buf->array_start].bufc = a_bufc;
      bufc_p_ref_increment(a_bufc);

      a_buf->size += (a_end_offset - a_beg_offset);
      a_buf->is_cumulative_valid = FALSE;
      a_buf->is_cached_bufel_valid = FALSE;
    }
  }
  
  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_append_bufc(cw_buf_t * a_buf, cw_bufc_t * a_bufc,
		cw_uint32_t a_beg_offset, cw_uint32_t a_end_offset)
{
  cw_bool_t retval;
  cw_bool_t did_bufel_merge = FALSE;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  _cw_assert(a_end_offset <= a_bufc->buf_size);
  _cw_assert(a_beg_offset <= a_end_offset);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (NULL != a_bufc->buf)
  {
    if (a_buf->array_num_valid > 0)
    {
      cw_uint32_t last_element_index;
    
      last_element_index = ((a_buf->array_end + a_buf->array_size - 1)
			    & (a_buf->array_size - 1));
  
      if ((a_buf->array_num_valid > 0)
	  && (a_buf->bufel_array[last_element_index].bufc->buf
	      == a_bufc->buf)
	  && (a_buf->bufel_array[last_element_index].end_offset
	      == a_beg_offset))
      {
	/* Two references to the same bufc, and the buffer regions they refer to
	 * are consecutive and adjacent.  Merge. */
	a_buf->bufel_array[last_element_index].end_offset
	  = (a_buf->bufel_array[last_element_index].end_offset
	     + (a_end_offset - a_beg_offset));
      
	a_buf->size += (a_end_offset - a_beg_offset);
      
	a_buf->cumulative_index[last_element_index] = a_buf->size;

	did_bufel_merge = TRUE;
      }
    }
  
    if (FALSE == did_bufel_merge)
    {
      if (buf_p_fit_array(a_buf, a_buf->array_num_valid + 1))
      {
	retval = TRUE;
	goto RETURN;
      }
  
      /* Now append the bufel. */
#ifdef _LIBSTASH_DBG
      a_buf->bufel_array[a_buf->array_end].magic = _CW_BUFEL_MAGIC;
#endif
      a_buf->bufel_array[a_buf->array_end].beg_offset = a_beg_offset;
      a_buf->bufel_array[a_buf->array_end].end_offset = a_end_offset;
      a_buf->bufel_array[a_buf->array_end].bufc = a_bufc;
      bufc_p_ref_increment(a_bufc);
      
      a_buf->array_num_valid++;
      a_buf->size += (a_end_offset - a_beg_offset);
      a_buf->cumulative_index[a_buf->array_end] = a_buf->size;

      a_buf->array_end = ((a_buf->array_end + 1) & (a_buf->array_size - 1));
    }
  }

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_release_head_data(cw_buf_t * a_buf, cw_uint32_t a_amount)
{
  cw_bool_t retval;
  cw_uint32_t array_index, bufel_valid_data, amount_left;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (a_amount == 0)
  {
    retval = FALSE;
  }
  else if (a_amount > a_buf->size)
  {
    retval = TRUE;
  }
  else
  {
    for (array_index = a_buf->array_start,
	   amount_left = a_amount;
	 amount_left > 0;
	 array_index = (array_index + 1) & (a_buf->array_size - 1))
    {
      bufel_valid_data
	= (a_buf->bufel_array[array_index].end_offset
	   - a_buf->bufel_array[array_index].beg_offset);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	if (NULL != a_buf->bufel_array[array_index].bufc)
	{
	  bufc_delete(a_buf->bufel_array[array_index].bufc);
	}
#ifdef _LIBSTASH_DBG
	bzero(&a_buf->bufel_array[array_index], sizeof(cw_bufel_t));
#endif
	
	a_buf->array_start = (array_index + 1) & (a_buf->array_size - 1);
	a_buf->array_num_valid--;
	amount_left -= bufel_valid_data;
      }
      else /* if (bufel_valid_data > amount_left) */
      {
	/* This will finish things up. */
	a_buf->bufel_array[array_index].beg_offset
	  = a_buf->bufel_array[array_index].beg_offset + amount_left;
	amount_left = 0;
      }
    }

    /* Adjust the buf size. */
    a_buf->size -= a_amount;
      
    if ((a_buf->array_num_valid == 0)
	&& (a_buf->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
/*        a_buf->array_start = 0; */
/*        a_buf->array_end = 0; */
      a_buf->is_cumulative_valid = TRUE;
/*        a_buf->cached_bufel = 0; */
    }
    else
    {
      a_buf->is_cumulative_valid = FALSE;
    }
    
    a_buf->is_cached_bufel_valid = FALSE;

    retval = FALSE;
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_release_tail_data(cw_buf_t * a_buf, cw_uint32_t a_amount)
{
  cw_bool_t retval;
  cw_uint32_t array_index, bufel_valid_data, amount_left;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (a_amount == 0)
  {
    retval = FALSE;
  }
  else if (a_amount > a_buf->size)
  {
    retval = TRUE;
  }
  else
  {
    for (array_index
	   = (a_buf->array_end + a_buf->array_size - 1)
	   & (a_buf->array_size - 1),
	   amount_left = a_amount;
	 amount_left > 0;
	 array_index
	   = (array_index + a_buf->array_size - 1) & (a_buf->array_size - 1))
    {
      bufel_valid_data
	= (a_buf->bufel_array[array_index].end_offset
	   - a_buf->bufel_array[array_index].beg_offset);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	if (NULL != a_buf->bufel_array[array_index].bufc)
	{
	  bufc_delete(a_buf->bufel_array[array_index].bufc);
	}
#ifdef _LIBSTASH_DBG
	bzero(&a_buf->bufel_array[array_index], sizeof(cw_bufel_t));
#endif
	
	a_buf->array_end = array_index;
	a_buf->array_num_valid--;
	amount_left -= bufel_valid_data;
      }
      else /* if (bufel_valid_data > amount_left) */
      {
	/* This will finish things up. */
	a_buf->bufel_array[array_index].end_offset
	  = (a_buf->bufel_array[array_index].end_offset - amount_left);
	amount_left = 0;
      }
    }

    /* Adjust the buf size. */
    a_buf->size -= a_amount;
    
    if ((a_buf->array_num_valid == 0)
	&& (a_buf->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
/*        a_buf->array_start = 0; */
/*        a_buf->array_end = 0; */
      a_buf->is_cumulative_valid = TRUE;
      a_buf->is_cached_bufel_valid = FALSE;
/*        a_buf->cached_bufel = 0; */
    }

    retval = FALSE;
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif

  return retval;
}

cw_uint8_t
buf_get_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset)
{
  cw_uint8_t retval;
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  retval = *(a_buf->bufel_array[array_element].bufc->buf + bufel_offset);
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_uint32_t
buf_get_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset)
{
  cw_uint32_t retval, array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert((a_offset + 3) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  /* Prepare a byte for logical or into retval.
   * o: Offset from bufel_offset.
   * s: Number of bytes to left shift. */
#define _LIBSTASH_BUF_OR_BYTE(o, s) \
  (((cw_uint32_t) *(a_buf->bufel_array[array_element].bufc->buf \
                    + bufel_offset + (o)) << ((s) << 3)) \
   & (0xff << ((s) << 3)))

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  if (bufel_offset + 3
      < 
      a_buf->bufel_array[array_element].end_offset)
  {
#ifdef WORDS_BIGENDIAN
    retval = _LIBSTASH_BUF_OR_BYTE(0, 3);
    retval |= _LIBSTASH_BUF_OR_BYTE(1, 2);
    retval |= _LIBSTASH_BUF_OR_BYTE(2, 1);
    retval |= _LIBSTASH_BUF_OR_BYTE(3, 0);
#else
    retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
    retval |= _LIBSTASH_BUF_OR_BYTE(1, 1);
    retval |= _LIBSTASH_BUF_OR_BYTE(2, 2);
    retval |= _LIBSTASH_BUF_OR_BYTE(3, 3);
#endif
  }
  else
  {
#ifdef WORDS_BIGENDIAN
    retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 3);
#else
    retval = _LIBSTASH_BUF_OR_BYTE(0, 0);
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 1);
    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 2);
    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= _LIBSTASH_BUF_OR_BYTE(0, 3);
#endif
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  
  return retval;
}

cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset)
{
  cw_uint64_t retval;
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert((a_offset + 7) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  if (bufel_offset + 7
      < 
      a_buf->bufel_array[array_element].end_offset)
  {
    retval = (((cw_uint64_t)
	       *(a_buf->bufel_array[array_element].bufc->buf
		 + bufel_offset))
	      & 0xff);
    
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 1))
	       << 8) & (((cw_uint64_t) 0x00000000 << 32) | 0x0000ff00);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 2))
	       << 16) & (((cw_uint64_t) 0x00000000 << 32) | 0x00ff0000);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 3))
	       << 24) & (((cw_uint64_t) 0x00000000 << 32) | 0xff000000);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 4))
	       << 32) & (((cw_uint64_t) 0x000000ff << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 5))
	       << 40) & (((cw_uint64_t) 0x0000ff00 << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 6))
	       << 48) & (((cw_uint64_t) 0x00ff0000 << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset + 7))
	       << 56) & (((cw_uint64_t) 0xff000000 << 32) | 0x00000000);
  }
  else
  {
    /* The data is spread across two to eight buffers. */
    retval = (((cw_uint64_t)
	       *(a_buf->bufel_array[array_element].bufc->buf
		 + bufel_offset))
	      & 0xff);
    
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 8) & (((cw_uint64_t) 0x00000000 << 32) | 0x0000ff00);

    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 16) & (((cw_uint64_t) 0x00000000 << 32) | 0x00ff0000);

    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 24) & (((cw_uint64_t) 0x00000000 << 32) | 0xff000000);

    buf_p_get_data_position(a_buf, a_offset + 4, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 32) & (((cw_uint64_t) 0x000000ff << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 5, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 40) & (((cw_uint64_t) 0x0000ff00 << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 6, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 48) & (((cw_uint64_t) 0x00ff0000 << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 7, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(a_buf->bufel_array[array_element].bufc->buf
		  + bufel_offset))
	       << 56) & (((cw_uint64_t) 0xff000000 << 32) | 0x00000000);
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_set_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint8_t a_val)
{
  cw_bool_t retval;
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  
  if (buf_p_make_range_writeable(a_buf, a_offset, sizeof(cw_uint8_t)))
  {
    retval = TRUE;
    goto RETURN;
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  a_buf->bufel_array[array_element].bufc->buf[bufel_offset] = a_val;

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_set_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_val)
{
  cw_bool_t retval;
  cw_uint32_t array_element, bufel_offset;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  
  if (buf_p_make_range_writeable(a_buf, a_offset, sizeof(cw_uint32_t)))
  {
    retval = TRUE;
    goto RETURN;
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);
  a_buf->bufel_array[array_element].bufc->buf[bufel_offset] = a_val & 0xff;
  
  if (bufel_offset + 3
      < 
      a_buf->bufel_array[array_element].end_offset)
  {
    /* The whole thing is in one bufel. */
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
      = (a_val >> 8) & 0xff;
    
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
      = (a_val >> 16) & 0xff;
    
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
      = (a_val >> 24) & 0xff;
  }
  else
  {
    /* Split across two or more bufels. */
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 8) & 0xff;
    
    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 16) & 0xff;
    
    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 24) & 0xff;
  }

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_set_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint64_t a_val)
{
  cw_bool_t retval;
  cw_uint32_t array_element, bufel_offset;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  
  if (buf_p_make_range_writeable(a_buf, a_offset, sizeof(cw_uint64_t)))
  {
    retval = TRUE;
    goto RETURN;
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);
  a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
    = a_val & 0xff;
  if (bufel_offset + 7
      <
      a_buf->bufel_array[array_element].end_offset)
  {
    /* The whole thing is in one bufel. */
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 1]
      = (a_val >> 8) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 2]
      = (a_val >> 16) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 3]
      = (a_val >> 24) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 4]
      = (a_val >> 32) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 5]
      = (a_val >> 40) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 6]
      = (a_val >> 48) & 0xff;

    a_buf->bufel_array[array_element].bufc->buf[bufel_offset + 7]
      = (a_val >> 56) & 0xff;
  }
  else
  {
    /* Split across two or more bufels. */
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 8) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 16) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 24) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 4, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 32) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 5, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 40) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 6, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 48) & 0xff;

    buf_p_get_data_position(a_buf, a_offset + 7, &array_element, &bufel_offset);
    a_buf->bufel_array[array_element].bufc->buf[bufel_offset]
      = (a_val >> 56) & 0xff;
  }

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bool_t
buf_set_range(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_length,
	      cw_uint8_t * a_val, cw_bool_t a_is_writeable)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  /* If a_val is writeable and it's being appended to the buf, tack it on to the
   * end of the buf, rather than copying. */
  if (a_is_writeable && (a_offset == a_buf->size))
  {
    cw_bufc_t * bufc;

    bufc = bufc_new(NULL, NULL, NULL);
    if (NULL == bufc)
    {
      retval = TRUE;
      goto RETURN;
    }

    bufc_set_buffer(bufc, (void *) a_val, a_length, TRUE, NULL, NULL);

    if (buf_p_fit_array(a_buf, a_buf->array_num_valid + 1))
    {
      bufc_delete(bufc);
      retval = TRUE;
      goto RETURN;
    }

    /* Initialize bufel. */
/*      bzero(&a_buf->bufel_array[a_buf->array_end], sizeof(cw_bufel_t)); */
#ifdef _LIBSTASH_DBG
    a_buf->bufel_array[a_buf->array_end].magic = _CW_BUFEL_MAGIC;
#endif
    a_buf->bufel_array[a_buf->array_end].beg_offset = 0;
    a_buf->bufel_array[a_buf->array_end].end_offset = a_length;
    a_buf->bufel_array[a_buf->array_end].bufc = bufc;
/*      bufc_p_ref_increment(bufc); */
    
    a_buf->size += a_length;
    a_buf->array_num_valid++;
    /* Do this in case the cumulative index is valid. */
    a_buf->cumulative_index[a_buf->array_end] = a_buf->size;
    a_buf->array_end = ((a_buf->array_end + 1) & (a_buf->array_size - 1));
  }
  else
  {
    cw_uint32_t bytes_copied, array_element, bufel_offset;
    
    if (buf_p_make_range_writeable(a_buf, a_offset, a_length))
    {
      retval = TRUE;
      goto RETURN;
    }
    /* March through the bufel_array and memcpy in a_val. */
    bytes_copied = 0;
    while (bytes_copied < a_length)
    {
      buf_p_get_data_position(a_buf, a_offset + bytes_copied, &array_element,
			      &bufel_offset);
      if (((a_buf->bufel_array[array_element].end_offset
	    - bufel_offset) + bytes_copied) > a_length)
      {
	/* There's more than enough room to finish up with the current bufel. */
	memcpy(a_buf->bufel_array[array_element].bufc->buf + bufel_offset,
	       a_val + bytes_copied,
	       a_length - bytes_copied);
	bytes_copied = a_length;
      }
      else
      {
	/* Completely re-write the current bufel. */
	memcpy(a_buf->bufel_array[array_element].bufc->buf + bufel_offset,
	       a_val + bytes_copied,
	       (a_buf->bufel_array[array_element].end_offset
		- bufel_offset));
	bytes_copied += (a_buf->bufel_array[array_element].end_offset
			 - bufel_offset);
      }
    }
  }

  retval = FALSE;

  RETURN:
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

static cw_buf_t *
buf_p_new(cw_buf_t * a_buf, cw_bool_t a_is_threadsafe)
{
  cw_buf_t * retval;

  if (a_buf == NULL)
  {
    retval = (cw_buf_t *) _cw_malloc(sizeof(cw_buf_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_buf;
    retval->is_malloced = FALSE;
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_BUF_MAGIC;
#endif

#ifdef _CW_REENTRANT
  retval->is_threadsafe = a_is_threadsafe;
  if (retval->is_threadsafe == TRUE)
  {
    mtx_new(&retval->lock);
  }
#endif

  retval->size = 0;

  retval->array_size = _LIBSTASH_BUF_ARRAY_MIN_SIZE;
  retval->array_num_valid = 0;
  retval->array_start = 0;
  retval->array_end = 0;
  retval->is_cumulative_valid = TRUE;
  retval->is_cached_bufel_valid = FALSE;
/*    retval->cached_bufel = 0; */

  retval->bufel_array = retval->static_bufel_array;
  retval->cumulative_index = retval->static_cumulative_index;
  retval->iov = retval->static_iov;

#ifdef _LIBSTASH_DBG
  bzero(retval->bufel_array,
	_LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(cw_bufel_t));
  bzero(retval->cumulative_index,
	_LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(cw_uint32_t));
  bzero(retval->iov,
	_LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif

  RETURN:
  return retval;
}

static void
buf_p_rebuild_cumulative_index(cw_buf_t * a_buf)
{
  cw_uint32_t i, cumulative, index;
  
  for (i = cumulative = 0;
       i < a_buf->array_num_valid;
       i++)
  {
    index = (i + a_buf->array_start) & (a_buf->array_size - 1);
    
    cumulative += (a_buf->bufel_array[index].end_offset
		   - a_buf->bufel_array[index].beg_offset);
    a_buf->cumulative_index[index] = cumulative;
  }
  a_buf->is_cumulative_valid = TRUE;
}

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset)
{
  if ((a_buf->is_cached_bufel_valid)
      && (a_offset < a_buf->cumulative_index[a_buf->cached_bufel])
      && (a_offset
	  >= (a_buf->cumulative_index[a_buf->cached_bufel]
	      - (a_buf->bufel_array[a_buf->cached_bufel].end_offset
		 - a_buf->bufel_array[a_buf->cached_bufel].beg_offset))))
  {
    /* The cached data position info is valid and useful. */
    *a_array_element = a_buf->cached_bufel;
    *a_bufel_offset
      = (a_buf->bufel_array[a_buf->cached_bufel].end_offset
	 - (a_buf->cumulative_index[a_buf->cached_bufel] - a_offset));
  }
  else
  {
    cw_uint32_t first, last, index;
    
    if (FALSE == a_buf->is_cumulative_valid)
    {
      buf_p_rebuild_cumulative_index(a_buf);
    }
    
    if (a_buf->array_start < a_buf->array_end)
    {
      /* bufel_array is not wrapped, which means we can do a standard binary
       * search without lots of modulus operations. */
      first = a_buf->array_start;
      last = a_buf->array_end;
    }
    else if (a_buf->array_end == 0)
    {
      /* bufel_array is not wrapped, which means we can do a standard binary
       * search without lots of modulus operations. */
      first = a_buf->array_start;
      last = a_buf->array_size;
    }
    else if (a_buf->cumulative_index[a_buf->array_size - 1] > a_offset)
    {
      /* bufel_array is wrapped, but the byte we want is not in the wrapped
       * portion. */
      first = a_buf->array_start;
      last = a_buf->array_size;
    }
    else
    {
      /* bufel_array is wrapped, and the byte we want is in the wrapped
       * portion. */
      first = 0;
      last = a_buf->array_end;
    }
    
    /* Binary search, where "first" is the index of the first element in the
     * range to search, and "last" is one plus the index of the last element in
     * the range to search. */
    if (a_offset < a_buf->cumulative_index[first])
    {
      index = first;
    }
    else
    {
      while (1)
      {
	index = (first + last) >> 1;
	  
	if (a_buf->cumulative_index[index] <= a_offset)
	{
	  first = index + 1;
	}
	else if (a_buf->cumulative_index[index - 1] > a_offset)
	{
	  last = index;
	}
	else
	{
	  break;
	}
      }
    }
	    
    *a_array_element = index;
    *a_bufel_offset = (a_buf->bufel_array[index].end_offset
		       - (a_buf->cumulative_index[index] - a_offset));

    a_buf->is_cached_bufel_valid = TRUE;
    a_buf->cached_bufel = index;
  }
}

static cw_bool_t
buf_p_fit_array(cw_buf_t * a_buf, cw_uint32_t a_min_array_size)
{
  cw_bool_t retval;
  cw_uint32_t i;
  void * t_ptr;

  /* Make sure a_buf's array is big enough.  Even if we're trying to merge
   * bufel's, make the array big enough that it doesn't matter how successful
   * the bufel merging is. */
  if (a_min_array_size > a_buf->array_size)
  {
    /* Double i until it is big enough to accomodate our needs. */
    for (i = a_buf->array_size << 1;
	 i < a_min_array_size;
	 i <<= 1);

    if (a_buf->bufel_array != a_buf->static_bufel_array)
    {
      t_ptr = _cw_realloc(a_buf->bufel_array, i * sizeof(cw_bufel_t)); /* XXX */
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      t_ptr = _cw_calloc(i, sizeof(cw_bufel_t));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
      
      memcpy(t_ptr, a_buf->bufel_array,
	     (size_t) (a_buf->array_size * sizeof(cw_bufel_t)));
#ifdef _LIBSTASH_DBG
      memset(a_buf->bufel_array, 0x5a,
	     (a_buf->array_size * sizeof(cw_bufel_t)));
#endif
    }
    a_buf->bufel_array = (cw_bufel_t *) t_ptr;

    if (a_buf->cumulative_index != a_buf->static_cumulative_index)
    {
      t_ptr = _cw_realloc(a_buf->cumulative_index, i * sizeof(cw_uint32_t));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      t_ptr = _cw_calloc(i, sizeof(cw_uint32_t));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
      memcpy(t_ptr, a_buf->cumulative_index,
	     (a_buf->array_size * sizeof(cw_uint32_t)));
#ifdef _LIBSTASH_DBG
      memset(a_buf->cumulative_index, 0x5a,
	     (a_buf->array_size * sizeof(cw_uint32_t)));
#endif
    }
    a_buf->cumulative_index = (cw_uint32_t *) t_ptr;

    if (a_buf->iov != a_buf->static_iov)
    {
      t_ptr = _cw_realloc(a_buf->iov, i * sizeof(struct iovec));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
    }
    else
    {
      t_ptr = _cw_calloc(i, sizeof(struct iovec));
      if (NULL == t_ptr)
      {
	retval = TRUE;
	goto RETURN;
      }
#ifdef _LIBSTASH_DBG
      memset(a_buf->iov, 0x5a,
	     (a_buf->array_size * sizeof(struct iovec)));
#endif
    }
    a_buf->iov = (struct iovec *) t_ptr;
    
#ifdef _LIBSTASH_DBG
    bzero(&a_buf->bufel_array[a_buf->array_size],
	  ((i - a_buf->array_size) * sizeof(cw_bufel_t)));
    bzero(&a_buf->cumulative_index[a_buf->array_size],
	  ((i - a_buf->array_size) * sizeof(cw_uint32_t)));
    bzero(&a_buf->iov[a_buf->array_size],
	  ((i - a_buf->array_size) * sizeof(struct iovec)));
#endif
      
    if ((a_buf->array_start >= a_buf->array_end)
	&& (a_buf->array_num_valid > 0)) /* array_num_valid check probably isn't
					  * necessary. */
    {
      /* The array was wrapped, so we need to move the wrapped part to sit
       * directly after where the end of the array used to be.  Since we at
       * least doubled the size of the array, there is no worry of writing past
       * the end of the array. */
      memcpy(&a_buf->bufel_array[a_buf->array_size],
	     a_buf->bufel_array,
	     a_buf->array_end * sizeof(cw_bufel_t));
      memcpy(&a_buf->cumulative_index[a_buf->array_size],
	     a_buf->cumulative_index,
	     a_buf->array_end * sizeof(cw_uint32_t));
      
#ifdef _LIBSTASH_DBG
      /* Zero the old copy to get rid of the bufel's' magic. */
      bzero(a_buf->bufel_array,
	    (a_buf->array_end * sizeof(cw_bufel_t)));
      bzero(a_buf->cumulative_index,
	    (a_buf->array_end * sizeof(cw_uint32_t)));
#endif
      a_buf->array_end = a_buf->array_start + a_buf->array_num_valid;
    }

    /* This must happen last, since the old value is used for some calculations
     * above. */
    a_buf->array_size = i;
  }

  retval = FALSE;
  
  RETURN:
  return retval;
}

static cw_bool_t
buf_p_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve)
{
  cw_bool_t retval;
  cw_uint32_t i, a_a_index, a_b_index;
  cw_uint32_t did_bufel_merge = 0;
  
  if (buf_p_fit_array(a_a, a_a->array_num_valid + a_b->array_num_valid))
  {
    retval = TRUE;
    goto RETURN;
  }
    
  /* Try to merge the last bufel in a_a and the first bufel in a_b into one
   * bufel in a_a. */
  if ((a_a->array_num_valid > 0) && (a_b->array_num_valid > 0))
  {
    cw_uint32_t last_element_index;
    
    last_element_index = ((a_a->array_end + a_a->array_size - 1)
			  & (a_a->array_size - 1));

    if ((NULL != a_a->bufel_array[last_element_index].bufc->buf)
	&& (a_a->bufel_array[last_element_index].bufc->buf
	    == a_b->bufel_array[a_b->array_start].bufc->buf)
	&& (a_a->bufel_array[last_element_index].end_offset
	    == a_b->bufel_array[a_b->array_start].beg_offset))
    {
      /* These two bufel's reference the same bufc, and the buffer regions they
       * refer to are consecutive and adjacent.  Merge them. */
      did_bufel_merge = TRUE;
      
      a_a->bufel_array[last_element_index].end_offset
	= (a_a->bufel_array[last_element_index].end_offset
	   + (a_b->bufel_array[a_b->array_start].end_offset
	      - a_b->bufel_array[a_b->array_start].beg_offset));
      
      a_a->size
	+= (a_b->bufel_array[a_b->array_start].end_offset
	    - a_b->bufel_array[a_b->array_start].beg_offset);
      a_a->cumulative_index[last_element_index] = a_a->size;
      
      if (FALSE == a_preserve)
      {
	if (NULL != a_b->bufel_array[a_b->array_start].bufc)
	{
	  bufc_delete(a_b->bufel_array[a_b->array_start].bufc);
	}
#ifdef _LIBSTASH_DBG
	bzero(&a_b->bufel_array[a_b->array_start], sizeof(cw_bufel_t));
#endif
      }
    }
  }
  
#ifdef _LIBSTASH_DBG
  if (did_bufel_merge)
  {
    buf_p_copy_array(a_a, a_b, a_b->array_num_valid - 1, a_a->array_end,
		     (a_b->array_start + 1) & (a_b->array_size - 1),
		     !a_preserve);
  }
  else
  {
    buf_p_copy_array(a_a, a_b, a_b->array_num_valid, a_a->array_end,
		     a_b->array_start, !a_preserve);
  }
#else
  if (did_bufel_merge)
  {
    buf_p_copy_array(a_a, a_b, a_b->array_num_valid - 1, a_a->array_end,
		     (a_b->array_start + 1) & (a_b->array_size - 1));
  }
  else
  {
    buf_p_copy_array(a_a, a_b, a_b->array_num_valid, a_a->array_end,
		     a_b->array_start);
  }
#endif
      
  /* Iterate through a_b's array, creating bufel's in a_a and adding references
   * to a_b's bufel data. */
  for (i = 0,
	 a_a_index = a_a->array_end,
	 a_b_index
	 = ((a_b->array_start + did_bufel_merge)
	    & (a_b->array_size - 1));
       i < a_b->array_num_valid - did_bufel_merge;
       i++,
	 a_a_index = (a_a_index + 1) & (a_a->array_size - 1),
	 a_b_index = (a_b_index + 1) & (a_b->array_size - 1))
  {
    a_a->size
      += (a_a->bufel_array[a_a_index].end_offset
	  - a_a->bufel_array[a_a_index].beg_offset);

    a_a->cumulative_index[a_a_index] = a_a->size;

    if (TRUE == a_preserve)
    {
      bufc_p_ref_increment(a_a->bufel_array[a_a_index].bufc);
    }
  }

  /* Finish making a_a's state consistent. */
  a_a->array_end = a_a_index;
  a_a->array_num_valid += i;

  /* Make a_b's state consistent if not preserving its state. */
  if (FALSE == a_preserve)
  {
    a_b->array_start = 0;
    a_b->array_end = 0;
    a_b->size = 0;
    a_b->array_num_valid = 0;
    a_b->is_cumulative_valid = TRUE;
    a_b->is_cached_bufel_valid = FALSE;
/*      a_b->cached_bufel = 0; */
  }

  retval = FALSE;
  
  RETURN:
  return retval;
}

#ifdef _LIBSTASH_DBG
static void
buf_p_copy_array(cw_buf_t * a_a, cw_buf_t * a_b,
		 cw_uint32_t a_num_elements,
		 cw_uint32_t a_a_start, cw_uint32_t a_b_start,
		 cw_bool_t a_is_destructive)
#else
  static void
buf_p_copy_array(cw_buf_t * a_a, cw_buf_t * a_b,
		 cw_uint32_t a_num_elements,
		 cw_uint32_t a_a_start, cw_uint32_t a_b_start)
#endif
{
  cw_bool_t is_a_wrapped, is_b_wrapped;
  cw_uint32_t first_chunk_size, second_chunk_size, third_chunk_size;

  /* Do 1 to 3 memcpy()'s of a_b's array to a_a's array, depending on array
   * alignments. */
  is_a_wrapped = ((a_a_start + a_num_elements)
		  > a_a->array_size) ? TRUE : FALSE;
  is_b_wrapped = ((a_b_start + a_num_elements)
		  > a_b->array_size) ? TRUE : FALSE;
  
  if ((FALSE == is_a_wrapped) && (FALSE == is_b_wrapped))
  {
    /* Simple case; one memcpy() suffices.
     *
     *   a_b (src)                           a_a (dest)
     * /------------\ 0                    /------------\ 0
     * |            |                      |            |
     * |            |                      |            |
     * |------------|                      |------------|
     * |DDDDDDDDDDDD| \                    |            |
     * |DDDDDDDDDDDD| |                    |            |
     * |------------| |                    |------------|
     * |DDDDDDDDDDDD| |                  / |DDDDDDDDDDDD|
     * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
     * |------------|  >------\          | |------------|
     * |DDDDDDDDDDDD| |        \         | |DDDDDDDDDDDD|
     * |DDDDDDDDDDDD| |         \        | |DDDDDDDDDDDD|
     * |------------| |          \------<  |------------|
     * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
     * |DDDDDDDDDDDD| /                  | |DDDDDDDDDDDD|
     * |------------|                    | |------------|
     * |            |                    | |DDDDDDDDDDDD|
     * |            |                    \ |DDDDDDDDDDDD|
     * |------------|                      |------------|
     * |            |                      |            |
     * |            |                      |            |
     * |------------|                      |------------| 
     * |            |                      |            |
     * |            |                      |            |
     * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
     */
    if (a_num_elements > 0)
    {
      memcpy(&a_a->bufel_array[a_a_start],
	     &a_b->bufel_array[a_b_start],
	     a_num_elements * sizeof(cw_bufel_t));
    }
    
#ifdef _LIBSTASH_DBG
    if (a_is_destructive)
    {
      bzero(&a_b->bufel_array[a_b_start],
	    a_num_elements * sizeof(cw_bufel_t));
    }
#endif
  }
  else if ((TRUE == is_a_wrapped) && (FALSE == is_b_wrapped))
  {
    /* Two memcpy()'s, since a_b wraps into a_a.
     *
     *   a_b (src)                           a_a (dest)
     * /------------\ 0                    /------------\ 0
     * |            |                    / |DDDDDDDDDDDD|
     * |            |                    | |DDDDDDDDDDDD|
     * |------------| a_b_start     /---<  |------------|
     * |DDDDDDDDDDDD| \            /     | |DDDDDDDDDDDD|
     * |DDDDDDDDDDDD| |           /      \ |DDDDDDDDDDDD|
     * |------------|  >-\       /         |------------|
     * |DDDDDDDDDDDD| |   \     /          |            |
     * |DDDDDDDDDDDD| /    \   /           |            |
     * |------------|       \ /            |------------|
     * |DDDDDDDDDDDD| \      X             |            |
     * |DDDDDDDDDDDD| |     / \            |            |
     * |------------|  >---/   \           |------------|
     * |DDDDDDDDDDDD| |         \          |            |
     * |DDDDDDDDDDDD| /          \         |            |
     * |------------|             \        |------------|
     * |            |              \       |            |
     * |            |               \      |            |
     * |------------|               |      |------------| a_a_start
     * |            |               |    / |DDDDDDDDDDDD|
     * |            |               |    | |DDDDDDDDDDDD|
     * |------------|               \---<  |------------| 
     * |            |                    | |DDDDDDDDDDDD|
     * |            |                    \ |DDDDDDDDDDDD|
     * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
     */

    first_chunk_size = a_a->array_size - a_a_start;
    second_chunk_size = a_num_elements - first_chunk_size;
    
    memcpy(&a_a->bufel_array[a_a_start],
	   &a_b->bufel_array[a_b_start],
	   first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
    if (a_is_destructive)
    {
      bzero(&a_b->bufel_array[a_b_start],
	    first_chunk_size * sizeof(cw_bufel_t));
    }
#endif

    memcpy(&a_a->bufel_array[0],
	   &a_b->bufel_array[a_b_start + first_chunk_size],
	   second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
    if (a_is_destructive)
    {
      bzero(&a_b->bufel_array[a_b_start + first_chunk_size],
	    second_chunk_size * sizeof(cw_bufel_t));
    }
#endif
  }
  else
  {
    if ((is_a_wrapped)
	&& ((a_b->array_size - a_b_start) < (a_a->array_size - a_a_start)))
    {
      /* The first chunk of a_b wraps into a_a.
       *
       *   a_b (src)                           a_a (dest)
       * /------------\ 0                    /------------\ 0
       * |DDDDDDDDDDDD| \               ___/ |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| |              /   \ |DDDDDDDDDDDD|
       * |------------|  >------\     |      |------------|
       * |DDDDDDDDDDDD| |        \    |    / |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /         \   |    | |DDDDDDDDDDDD|
       * |------------|            \--+---<  |------------|
       * |            |               |    | |DDDDDDDDDDDD|
       * |            |               |    \ |DDDDDDDDDDDD|
       * |------------|               |      |------------|
       * |            |               |      |            |
       * |            |               |      |            |
       * |------------|               |      |------------|
       * |            |               |      |            |
       * |            |               |      |            |
       * |------------|               |      |------------|
       * |            |               |      |            |
       * |            |               |      |            |
       * |------------|              /       |------------|
       * |DDDDDDDDDDDD| \___        /        |            |
       * |DDDDDDDDDDDD| /   \      /         |            |
       * |------------|      \----/---\      |------------| 
       * |DDDDDDDDDDDD| \________/     \___/ |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
       * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
       */
      first_chunk_size = a_a->array_size - a_a_start;
      second_chunk_size = a_b->array_size - a_b_start - first_chunk_size;
      third_chunk_size = a_num_elements - second_chunk_size - first_chunk_size;
      
      memcpy(&a_a->bufel_array[a_a_start],
	     &a_b->bufel_array[a_b_start],
	     first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[a_b_start],
	      first_chunk_size * sizeof(cw_bufel_t));
      }
#endif
    
      memcpy(&a_a->bufel_array[0],
	     &a_b->bufel_array[a_b_start + first_chunk_size],
	     second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[a_b_start + first_chunk_size],
	      second_chunk_size * sizeof(cw_bufel_t));
      }
#endif

      memcpy(&a_a->bufel_array[second_chunk_size],
	     &a_b->bufel_array[0],
	     third_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[0],
	      third_chunk_size * sizeof(cw_bufel_t));
      }
#endif
    }
    else if ((is_a_wrapped)
	     && ((a_b->array_size - a_b_start) > (a_a->array_size - a_a_start)))
    {
      /* The second chunk of a_b wraps into a_a.
       *
       *   a_b (src)                           a_a (dest)
       * /------------\ 0                    /------------\ 0
       * |DDDDDDDDDDDD| \___        _______/ |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /   \      /       \ |DDDDDDDDDDDD|
       * |------------|     |     /          |------------|
       * |DDDDDDDDDDDD| \___|____/           |            |
       * |DDDDDDDDDDDD| /   |                |            |
       * |------------|     |                |------------|
       * |            |     |                |            |
       * |            |     |                |            |
       * |------------|     |                |------------|
       * |            |     |                |            |
       * |            |     |                |            |
       * |------------|     |                |------------|
       * |            |     |                |            |
       * |            |     |                |            |
       * |------------|     |                |------------|
       * |            |     |              / |DDDDDDDDDDDD|
       * |            |     |              | |DDDDDDDDDDDD|
       * |------------|     |      /------<  |------------|
       * |DDDDDDDDDDDD| \   |     /        | |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| |   |    /         \ |DDDDDDDDDDDD|
       * |------------|  >--|---/            |------------| 
       * |DDDDDDDDDDDD| |    \_____________/ |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
       * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
       */
      first_chunk_size = a_b->array_size - a_b_start;
      second_chunk_size = a_a->array_size - a_a_start - first_chunk_size;
      third_chunk_size = a_num_elements - second_chunk_size - first_chunk_size;

      memcpy(&a_a->bufel_array[a_a_start],
	     &a_a->bufel_array[a_b_start],
	     first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_a->bufel_array[a_b_start],
	      first_chunk_size * sizeof(cw_bufel_t));
      }
#endif

      memcpy(&a_a->bufel_array[a_a_start + first_chunk_size],
	     &a_b->bufel_array[0],
	     second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[0],
	      second_chunk_size * sizeof(cw_bufel_t));
      }
#endif

      memcpy(&a_a->bufel_array[0],
	     &a_b->bufel_array[second_chunk_size],
	     third_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[second_chunk_size],
	      third_chunk_size * sizeof(cw_bufel_t));
      }
#endif
    }
    else
    {
      /* Either a_b unwraps into a_a, or the two chunks of a_b's array that are
       * being copied wrap at the same point in a_b as when copied to a_a.
       * These two cases can be treated the same way in the code. 
       *
       *   a_b (src)                           a_a (dest)
       * /------------\ 0                    /------------\ 0
       * |DDDDDDDDDDDD| \                  / |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
       * |------------|  >----------------<  |------------|
       * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
       * |------------|                      |------------|
       * |            |                      |            |
       * |            |                      |            |
       * |------------|                      |------------|
       * |            |                      |            |
       * |            |                      |            |
       * |------------|                      |------------|
       * |            |                      |            |
       * |            |                      |            |
       * |------------|                      |------------|
       * |            |                      |            |
       * |            |                      |            |
       * |------------|                      |------------|
       * |DDDDDDDDDDDD| \                  / |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
       * |------------|  >----------------<  |------------| 
       * |DDDDDDDDDDDD| |                  | |DDDDDDDDDDDD|
       * |DDDDDDDDDDDD| /                  \ |DDDDDDDDDDDD|
       * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
       *
       *   a_b (src)                           a_a (dest)
       * /------------\ 0                    /------------\ 0
       * |DDDDDDDDDDDD| \                    |            |
       * |DDDDDDDDDDDD| |                    |            |
       * |------------|  >-\                 |------------|
       * |DDDDDDDDDDDD| |   \                |            |
       * |DDDDDDDDDDDD| /    \               |            |
       * |------------|       \              |------------|
       * |            |        \           / |DDDDDDDDDDDD|
       * |            |         \          | |DDDDDDDDDDDD|
       * |------------|          \     /--<  |------------|
       * |            |           \   /    | |DDDDDDDDDDDD|
       * |            |            \ /     \ |DDDDDDDDDDDD|
       * |------------|             X        |------------|
       * |            |            / \     / |DDDDDDDDDDDD|
       * |            |           /   \    | |DDDDDDDDDDDD|
       * |------------|          /     \--<  |------------|
       * |            |         /          | |DDDDDDDDDDDD|
       * |            |        /           \ |DDDDDDDDDDDD|
       * |------------|       /              |------------|
       * |DDDDDDDDDDDD| \    /               |            |
       * |DDDDDDDDDDDD| |   /                |            |
       * |------------|  >-/                 |------------| 
       * |DDDDDDDDDDDD| |                    |            |
       * |DDDDDDDDDDDD| /                    |            |
       * \------------/ 8 (a_b->array_size)  \------------/ 8 (a_a->array_size)
       */
      first_chunk_size = a_b->array_size - a_b_start;
      second_chunk_size = a_num_elements - first_chunk_size;

      memcpy(&a_a->bufel_array[a_a_start],
	     &a_b->bufel_array[a_b_start],
	     first_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[a_b_start],
	      first_chunk_size * sizeof(cw_bufel_t));
      }
#endif

      memcpy(&a_a->bufel_array[(a_a_start + first_chunk_size)
			      & (a_a->array_size - 1)],
	     &a_b->bufel_array[0],
	     second_chunk_size * sizeof(cw_bufel_t));
#ifdef _LIBSTASH_DBG
      if (a_is_destructive)
      {
	bzero(&a_b->bufel_array[0],
	      second_chunk_size * sizeof(cw_bufel_t));
      }
#endif
    }
  }
}

static cw_bool_t
buf_p_make_range_writeable(cw_buf_t * a_buf, cw_uint32_t a_offset,
			   cw_uint32_t a_length)
{
  cw_bool_t retval;
  cw_uint32_t first_array_element, last_array_element, bufel_offset;
  cw_uint32_t i, num_iterations;
  cw_bufel_t * bufel;
  cw_bufc_t * bufc;
  void * buffer;

  _cw_assert(a_length > 0);

  /* Add extra buffer space to the end of the buf if the writeable range we're
   * creating extends past the current end of the buf. */
  if (a_offset + a_length > a_buf->size)
  {
    bufc = bufc_new(NULL, mem_dealloc, cw_g_mem);
    if (NULL == bufc)
    {
      retval = TRUE;
      goto RETURN;
    }
    
    buffer = _cw_malloc((a_offset + a_length) - a_buf->size);
    if (NULL == buffer)
    {
      bufc_delete(bufc);
      retval = TRUE;
      goto RETURN;
    }

    bufc_set_buffer(bufc, buffer, (a_offset + a_length) - a_buf->size,
		    TRUE, mem_dealloc, cw_g_mem);

    if (buf_p_fit_array(a_buf, a_buf->array_num_valid + 1))
    {
      bufc_delete(bufc);
      retval = TRUE;
      goto RETURN;
    }

    /* Initialize bufel. */
/*      bzero(&a_buf->bufel_array[a_buf->array_end], sizeof(cw_bufel_t)); */
#ifdef _LIBSTASH_DBG
    a_buf->bufel_array[a_buf->array_end].magic = _CW_BUFEL_MAGIC;
#endif
    a_buf->bufel_array[a_buf->array_end].beg_offset = 0;
    a_buf->bufel_array[a_buf->array_end].end_offset = ((a_offset + a_length)
						       - a_buf->size);
    a_buf->bufel_array[a_buf->array_end].bufc = bufc;

    a_buf->size = a_offset + a_length;
    a_buf->array_num_valid++;
    /* Do this in case the cumulative index is valid. */
    a_buf->cumulative_index[a_buf->array_end] = a_buf->size;
    a_buf->array_end = ((a_buf->array_end + 1) & (a_buf->array_size - 1));
  }
  
  /* March through the bufel's we need our own copy of and make them writeable.
   *
   * Note that this algorithm has the potential to fragment memory, but the
   * functions that use this facility are not normally meant to be used in
   * situations where a bufc is unwriteable (reference count greater than one or
   * marked unwriteable). */
  buf_p_get_data_position(a_buf, a_offset,
			  &first_array_element, &bufel_offset);
  buf_p_get_data_position(a_buf, a_offset + a_length - 1,
			  &last_array_element, &bufel_offset);

  num_iterations = (((last_array_element + a_buf->array_size)
		     - first_array_element)
		    & (a_buf->array_size - 1)) + 1;
  for (i = 0; i < num_iterations; i++)
  {
    bufel = &a_buf->bufel_array[((first_array_element + i)
				 & (a_buf->array_size - 1))];
    if ((FALSE == bufc_p_get_is_writeable(bufel->bufc))
	|| (1 < bufc_p_get_ref_count(bufel->bufc)))
    {
      buffer = _cw_malloc(bufel->end_offset - bufel->beg_offset);
      if (NULL == buffer)
      {
	retval = TRUE;
	goto RETURN;
      }

      bufc = bufc_new(NULL, NULL, NULL);
      if (NULL == bufc)
      {
	_cw_free(buffer);
	retval = TRUE;
	goto RETURN;
      }

      bufc_set_buffer(bufc, buffer, bufel->end_offset - bufel->beg_offset,
		      TRUE, mem_dealloc, cw_g_mem);

      memcpy(buffer,
	     bufel->bufc->buf + bufel->beg_offset,
	     bufel->end_offset - bufel->beg_offset);

      bufc_delete(bufel->bufc);
      bufel->bufc = bufc;
      bufel->end_offset -= bufel->beg_offset;
      bufel->beg_offset = 0;
    }
    
    _cw_assert(TRUE == bufel->bufc->is_writeable);
    _cw_assert(1 == bufc_p_get_ref_count(bufel->bufc));
  }

  retval = FALSE;

  RETURN:
  return retval;
}

cw_bufc_t *
bufc_new(cw_bufc_t * a_bufc,
	 void (*a_dealloc_func)(void * dealloc_arg, void * bufc),
	 void * a_dealloc_arg)
{
  cw_bufc_t * retval;

  if (NULL == a_bufc)
  {
    retval = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
    if (NULL == retval)
    {
      goto RETURN;
    }
    bzero(retval, sizeof(cw_bufc_t));
    retval->dealloc_func = mem_dealloc;
    retval->dealloc_arg = cw_g_mem;
  }
  else
  {
    retval = a_bufc;
    bzero(retval, sizeof(cw_bufc_t));
    retval->dealloc_func = a_dealloc_func;
    retval->dealloc_arg = a_dealloc_arg;
  }
  retval->ref_count = 1;

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_BUFC_MAGIC;
#endif

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif
  
  RETURN:
  return retval;
}

void
bufc_delete(cw_bufc_t * a_bufc)
{
#ifdef _CW_REENTRANT
  cw_bool_t should_delete;
#endif
      
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
      
#ifdef _CW_REENTRANT
  mtx_lock(&a_bufc->lock);
    
  a_bufc->ref_count--;
  if (0 == a_bufc->ref_count)
  {
    /* Make a note that we should delete the bufc once we've released the
     * mutex. */
    should_delete = TRUE;
  }
  else
  {
    should_delete = FALSE;
  }

  mtx_unlock(&a_bufc->lock);

  if (TRUE == should_delete)
  {
    mtx_delete(&a_bufc->lock);

    if (NULL != a_bufc->buffer_dealloc_func)
    {
      a_bufc->buffer_dealloc_func(a_bufc->buffer_dealloc_arg,
				  (void *) a_bufc->buf);
    }
    
    if (NULL != a_bufc->dealloc_func)
    {
      a_bufc->dealloc_func(a_bufc->dealloc_arg, (void *) a_bufc);
    }
  }
#else
  a_bufc->ref_count--;
  if (0 == a_bufc->ref_count)
  {
    if (NULL != a_bufc->buffer_dealloc_func)
    {
      a_bufc->buffer_dealloc_func(a_bufc->buffer_dealloc_arg,
				  (void *) a_bufc->buf);
    }
    
    if (NULL != a_bufc->dealloc_func)
    {
      a_bufc->dealloc_func(a_bufc->dealloc_arg, (void *) a_bufc);
    }
  }
#endif
}

void
bufc_set_buffer(cw_bufc_t * a_bufc, void * a_buffer, cw_uint32_t a_size,
		cw_bool_t a_is_writeable,
		void (*a_dealloc_func)(void * dealloc_arg, void * buffer),
		void * a_dealloc_arg)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  _cw_check_ptr(a_buffer);
  _cw_assert(a_size > 0);
  
  a_bufc->buf = (cw_uint8_t *) a_buffer;
  a_bufc->buf_size = a_size;
  a_bufc->is_writeable = a_is_writeable;
  a_bufc->buffer_dealloc_func = a_dealloc_func;
  a_bufc->buffer_dealloc_arg = a_dealloc_arg;
}

cw_uint32_t
bufc_get_size(cw_bufc_t * a_bufc)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  
  retval = a_bufc->buf_size;
  
  return retval;
}

static void
bufc_p_dump(cw_bufc_t * a_bufc, const char * a_prefix)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  _cw_check_ptr(a_prefix);

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufc->lock);
#endif
  
  out_put(cw_g_out,
	  "[s]| bufc_dump()\n",
	  a_prefix);
#ifdef _LIBSTASH_DBG
  out_put(cw_g_out,
	  "[s]|--> magic : 0x[i|b:16]\n",
	  a_prefix, a_bufc->magic);
#endif
  out_put(cw_g_out,
	  "[s]|--> free_func : 0x[p]\n",
	  a_prefix, a_bufc->dealloc_func);
  out_put(cw_g_out,
	  "[s]|--> free_arg : 0x[p]\n",
	  a_prefix, a_bufc->dealloc_arg);
  out_put(cw_g_out,
	  "[s]|--> ref_count : [i]\n",
	  a_prefix, a_bufc->ref_count);
  out_put(cw_g_out,
	  "[s]|--> is_writeable : [s]\n",
	  a_prefix, a_bufc->is_writeable ? "TRUE" : "FALSE");
  out_put(cw_g_out,
	  "[s]|--> buf_size : [i]\n",
	  a_prefix, a_bufc->buf_size);
  out_put(cw_g_out,
	  "[s]\\--> buf (0x[i|w:8|p:0|b:16]) : ",
	  a_prefix, a_bufc->buf);
  
  for (i = 0; i < a_bufc->buf_size; i++)
  {
    if (i % 16 == 0)
    {
      out_put(cw_g_out, "\n[s]         [[[i|w:4|b:16]] ", a_prefix, i);
    }
    out_put(cw_g_out, "[i|w:2|p:0|b:16] ", a_bufc->buf[i]);
  }
  out_put(cw_g_out, "\n");
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufc->lock);
#endif
}

static cw_bool_t
bufc_p_get_is_writeable(cw_bufc_t * a_bufc)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);

  return a_bufc->is_writeable;
}

static cw_uint32_t
bufc_p_get_ref_count(cw_bufc_t * a_bufc)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
      
#ifdef _CW_REENTRANT
  mtx_lock(&a_bufc->lock);
#endif

  retval = a_bufc->ref_count;
    
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufc->lock);
#endif
  return retval;
}

static void
bufc_p_ref_increment(cw_bufc_t * a_bufc)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
      
#ifdef _CW_REENTRANT
  mtx_lock(&a_bufc->lock);
#endif

  a_bufc->ref_count++;
    
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufc->lock);
#endif
}
