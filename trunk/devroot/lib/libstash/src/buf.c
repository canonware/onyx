/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#define _STASH_USE_BUF
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include <netinet/in.h>
#include <sys/param.h>

cw_buf_t *
#ifdef _CW_REENTRANT
buf_new(cw_buf_t * a_buf, cw_bool_t a_is_threadsafe)
#else
  buf_new(cw_buf_t * a_buf)
#endif
{
  cw_buf_t * retval;

  if (a_buf == NULL)
  {
    retval = (cw_buf_t *) _cw_malloc(sizeof(cw_buf_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_buf;
    retval->is_malloced = FALSE;
  }

#ifdef _CW_REENTRANT
  retval->is_threadsafe = a_is_threadsafe;
  if (retval->is_threadsafe == TRUE)
  {
    mtx_new(&retval->lock);
  }
#endif

  retval->size = 0;

  retval->array_size = 2;
  retval->array_num_valid = 0;
  retval->array_start = 0;
  retval->array_end = 0;
  retval->is_cumulative_valid = FALSE;
  retval->array = _cw_malloc(2 * sizeof(cw_bufel_array_el_t));
  
  return retval;
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
    bufel_delete(&a_buf->array[(i + a_buf->array_start)
			     % a_buf->array_size].bufel);
  }
}

cw_uint32_t
buf_get_size(cw_buf_t * a_buf)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_buf);
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

void
buf_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_try_bufel_merge)
{
  cw_uint32_t i;
  
  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);
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

  /* Make sure a_a's array is big enough.  Even if we're trying to merge
   * bufel's, make the array big enough that it doesn't matter how successful
   * the bufel merging is. */
  if (a_a->array_num_valid + a_b->array_num_valid > a_a->array_size)
  {
    /* Double i until it is big enough to accomodate our needs. */
    for (i = 2 * a_a->array_size;
	 i < a_a->array_num_valid + a_b->array_num_valid;
	 i *= 2);

    a_a->array
      = (cw_bufel_array_el_t *) _cw_realloc(a_a,
					    i * sizeof(cw_bufel_array_el_t *));
    
    if ((a_a->array_start >= a_a->array_end) && (a_a->array_num_valid > 0))
    {
      /* The array was wrapped, so we need to move the wrapped part to sit
       * directly after where the end of the array used to be.  Since we at
       * least doubled the size of the array, there is no worry of writing past
       * the end of the array. */
      memcpy(&a_a->array[a_a->array_size],
	     a_a->array,
	     a_a->array_end * sizeof(cw_bufel_array_el_t *));
      a_a->array_end += a_a->array_size;
    }

    /* This must happen last, since the old value is used for some calculations
     * above. */
    a_a->array_size = i;
  }

  if (TRUE == a_try_bufel_merge)
  {
    /* This looks ugly because we have to be careful to not decrement past the
     * beginning of the array. */
    cw_uint32_t last_element_index = (((a_a->array_end + a_a->array_size) - 1)
				      % a_a->array_size);

    if ((bufel_get_size(&a_a->array[last_element_index].bufel)
	 -
	 bufel_get_end_offset(&a_a->array[last_element_index].bufel))
	>= bufel_get_valid_data_size(&a_b->array[a_b->array_start].bufel))
    {
      /* Woohoo, there's enough space to merge a_b's first bufel into a_a's last
       * bufel. */
      bufel_catenate_bufel(&a_a->array[last_element_index].bufel,
			   &a_b->array[a_b->array_start].bufel);
      
      a_a->size
	+= bufel_get_valid_data_size(&a_b->array[a_b->array_start].bufel);

      a_a->array[last_element_index].cumulative_size = a_a->size;
    }
  }
  
  /* Iterate through a_b's array, creating bufel's in a_a and adding references
   * to a_b's bufel data. */
  for (i = 0; i < a_b->array_num_valid; i++)
  {
    memcpy(&a_a->array[(i + a_a->array_start) % a_a->array_size].bufel,
	   &a_b->array[(i + a_b->array_start) % a_b->array_size].bufel,
	   sizeof(cw_bufel_t));

    a_a->size
      += bufel_get_valid_data_size(&a_a->array[(i + a_a->array_start)
					      % a_a->array_size].bufel);

    a_a->array[(i + a_a->array_start) % a_a->array_size].cumulative_size
      = a_a->size;

    a_a->array_num_valid++;

#ifdef _CW_REENTRANT
    mtx_lock(&a_a->array[(i + a_a->array_start)
			% a_a->array_size].bufel.bufc->lock);
#endif

    a_a->array[(i + a_a->array_start)
	      % a_a->array_size].bufel.bufc->ref_count++;
    
#ifdef _CW_REENTRANT
    mtx_unlock(&a_a->array[(i + a_a->array_start)
			  % a_a->array_size].bufel.bufc->lock);
#endif
  }

  /* Finish making a_a's state consistent. */
  a_a->array_end = (i + a_a->array_start) % a_a->array_size;
  
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
}

void
buf_prepend_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_bufel);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  /* Make sure there is space in a_buf's array. */
  if (a_buf->array_size <= a_buf->array_num_valid)
  {
    a_buf->array
      = (cw_bufel_array_el_t *) _cw_realloc(a_buf,
					    2 * sizeof(cw_bufel_array_el_t *));
    
    if (a_buf->array_start >= a_buf->array_end)
    {
      /* The array was wrapped, so we need to move the wrapped part to sit
       * directly after where the end of the array used to be.  Since we doubled
       * the size of the array, there is no worry of writing past the end of the
       * array. */
      memcpy(&a_buf->array[a_buf->array_size],
	     a_buf->array,
	     a_buf->array_end * sizeof(cw_bufel_array_el_t *));
      a_buf->array_end += a_buf->array_size;
    }

    /* This must happen last, since the old value is used for some calculations
     * above. */
    a_buf->array_size *= 2;
  }
  
  /* Now prepend the bufel. */
  a_buf->array_start = (((a_buf->array_end + a_buf->array_size) - 1)
			% a_buf->array_size);
  a_buf->array_num_valid++;
  memcpy(&a_buf->array[a_buf->array_start].bufel,
	 a_bufel,
	 sizeof(cw_bufel_t));

  a_buf->size += bufel_get_valid_data_size(a_bufel);
  a_buf->is_cumulative_valid = FALSE;

#ifdef _CW_REENTRANT
  mtx_lock(a_bufel->bufc->lock);
#endif

  a_bufel->bufc->ref_count++;
    
#ifdef _CW_REENTRANT
  mtx_unlock(a_bufel->bufc->lock);
#endif
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

void
buf_append_bufel(cw_buf_t * a_buf, cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_bufel);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel)
{
  cw_bufel_t * retval;

  if (a_bufel == NULL)
  {
    retval = (cw_bufel_t *) _cw_malloc(sizeof(cw_bufel_t));
    bzero(retval, sizeof(cw_bufel_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bufel;
    bzero(retval, sizeof(cw_bufel_t));
    retval->is_malloced = FALSE;
  }
  
  return retval;
}

void
bufel_delete(cw_bufel_t * a_bufel)
{
#ifdef _CW_REENTRANT
  cw_bool_t should_delete_bufc;
#endif
  
  _cw_check_ptr(a_bufel);

  if (a_bufel->bufc != NULL)
  {
#ifdef _CW_REENTRANT
    mtx_lock(a_bufel->bufc->lock);
#endif
    a_bufel->bufc->ref_count--;

    if (0 == a_bufel->bufc->ref_count)
    {
#ifdef _CW_REENTRANT
      /* Make a note that we should delete the bufc once we've released the
       * mutex. */
      should_delete_bufc = TRUE;
#else
      _cw_free(a_bufel->bufc);
#endif
    }
#ifdef _CW_REENTRANT
    else
    {
      should_delete_bufc = FALSE;
    }
#endif

#ifdef _CW_REENTRANT
    mtx_unlock(a_bufel->bufc->lock);

    if (TRUE == should_delete_bufc)
    {
      mtx_delete(a_bufel->bufc->lock);
      _cw_free(a_bufel->bufc);
    }
#endif
    
    _cw_free(a_bufel->bufc);
  }
  if (a_bufel->is_malloced == TRUE)
  {
    _cw_free(a_bufel);
  }
}

cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_bufel);

  if (NULL == a_bufel->bufc)
  {
    retval = 0;
  }
  else
  {
    retval = a_bufel->bufc->buf_size;
  }
  
  return retval;
}

cw_bool_t
bufel_set_size(cw_bufel_t * a_bufel, cw_uint32_t a_size)
{
  cw_bool_t retval;
  cw_uint32_t * t_buf;

  _cw_check_ptr(a_bufel);

  if ((a_size < a_bufel->end_offset)
      && (a_bufel->beg_offset != a_bufel->end_offset))
  {
    /* We would chop off valid data if we did this. */
    retval = TRUE;
  }
  else if (a_size == 0)
  {
    if (a_bufel->bufc != NULL)
    {
      /* Decrement the reference count for the bufc and free it if necessary. */
      
      a_bufel->bufc = NULL;
    }
    
    /* Release the memory for the buffer, if any is used. */
    a_bufel->buf_size = 0;
    a_bufel->beg_offset = 0;
    a_bufel->end_offset = 0;

    retval = FALSE;
  }
  else if (a_bufel->buf != NULL)
  {
    if (a_bufel->buf_size != a_size)
    {
      /* Reallocate. */
      t_buf = (cw_uint32_t *) _cw_realloc(a_bufel->buf, a_size);

      a_bufel->buf = t_buf;
      a_bufel->buf_size = a_size;
      if (a_bufel->beg_offset > a_bufel->buf_size)
      {
	a_bufel->beg_offset = a_bufel->buf_size;
	a_bufel->end_offset = a_bufel->buf_size;
      }
    }
    retval = FALSE;
  }
  else
  {
    /* Allocate for the first time. */
    a_bufel->buf = (cw_uint32_t *) _cw_malloc(a_size);

    a_bufel->buf_size = a_size;
    retval = FALSE;
  }
  
  return retval;
}

cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->beg_offset;
}

cw_bool_t
bufel_set_beg_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_bufel);

  if (a_offset > a_bufel->end_offset)
  {
    retval = TRUE;
  }
  else
  {
    a_bufel->beg_offset = a_offset;
    retval = FALSE;
  }
  
  return retval;
}

cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->end_offset;
}

cw_bool_t
bufel_set_end_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_bufel);

  if (a_offset < a_bufel->beg_offset)
  {
    retval = TRUE;
  }
  else if (a_offset > a_bufel->buf_size)
  {
    retval = TRUE;
  }
  else
  {
    a_bufel->end_offset = a_offset;
    retval = FALSE;
  }

  return retval;
}

cw_uint32_t
bufel_get_valid_data_size(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  _cw_assert((a_bufel->end_offset - a_bufel->beg_offset) >= 0);

  return a_bufel->end_offset - a_bufel->beg_offset;
}

void *
bufel_get_data_ptr(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);

  return (void *) a_bufel->buf;
}

void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size)
{
  _cw_check_ptr(a_bufel);

  if (a_bufel->buf != NULL)
  {
    /* Release the memory used by the current buffer. */
    _cw_free(a_bufel->buf);
  }

  a_bufel->buf = (cw_uint32_t *) a_buf;
  a_bufel->buf_size = a_size;
  a_bufel->beg_offset = 0;
  a_bufel->end_offset = a_size;
}

void
bufel_catenate_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b)
{
  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a_a->end_offset + bufel_get_valid_data_size(a_b)
      > a_a->buf_size)
  {
    /* Need more space. */
    a_a->buf
      = (cw_uint32_t *) _cw_realloc(a_a->buf,
				    a_a->buf_size
				    + bufel_get_valid_data_size(a_b));

    memcpy(((cw_uint8_t *) a_a->buf) + a_a->end_offset,
	   ((cw_uint8_t *) a_b->buf) + a_b->beg_offset,
	   bufel_get_valid_data_size(a_b));

    a_a->buf_size += bufel_get_valid_data_size(a_b);
    a_a->end_offset += bufel_get_valid_data_size(a_b);
  }
  else
  {
    memcpy(((cw_uint8_t *) a_a->buf) + a_a->end_offset,
	   ((cw_uint8_t *) a_b->buf) + a_b->beg_offset,
	   bufel_get_valid_data_size(a_b));
    
    a_a->end_offset += bufel_get_valid_data_size(a_b);
  }
}

cw_uint8_t
bufel_get_uint8(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_uint8_t retval;
  cw_uint8_t * data;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_offset < a_bufel->buf_size);

  data = (cw_uint8_t *) a_bufel->buf;

  retval = data[a_offset];
  
  return retval;
}

void
bufel_set_uint8(cw_bufel_t * a_bufel, cw_uint32_t a_offset,
		cw_uint8_t a_val)
{
  cw_uint8_t * data;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_offset < a_bufel->buf_size);

  data = (cw_uint8_t *) a_bufel->buf;

  data[a_offset] = a_val;
}

cw_uint32_t
bufel_get_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_bufel);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert ((a_offset + 4) <= a_bufel->buf_size);

  retval = a_bufel->buf[a_offset >> 2];
#ifndef WORDS_BIGENDIAN
  retval = htonl(retval);
#endif
  
  return retval;
}

void
bufel_set_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset,
		 cw_uint32_t a_val)
{
  _cw_check_ptr(a_bufel);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert((a_offset + 4) <= a_bufel->buf_size);

#ifdef WORDS_BIGENDIAN
  a_bufel->buf[a_offset >> 2] = a_val;
#else
  a_bufel->buf[a_offset >> 2] = ntohl(a_val);
#endif
}
