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

#define _LIBSTASH_USE_BUF
#ifdef _CW_REENTRANT
#  include "libstash/libstash_r.h"
#else
#  include "libstash/libstash.h"
#endif

#include "libstash/buf_p.h"

#include <netinet/in.h>
#include <sys/param.h>

cw_bufpool_t *
bufpool_new(cw_bufpool_t * a_bufpool, cw_uint32_t a_buffer_size,
	    cw_uint32_t a_max_spare_buffers)
{
  cw_bufpool_t * retval;

  if (NULL == a_bufpool)
  {
    retval = (cw_bufpool_t *) _cw_malloc(sizeof(cw_bufpool_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bufpool;
    retval->is_malloced = FALSE;
  }

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_BUFPOOL_MAGIC;
#endif

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

  retval->buffer_size = a_buffer_size;
  retval->max_spare_buffers = a_max_spare_buffers;

#ifdef _CW_REENTRANT
  list_new(&retval->spare_buffers, FALSE);
#else
  list_new(&retval->spare_buffers);
#endif
  
  return retval;
}

void
bufpool_delete(cw_bufpool_t * a_bufpool)
{
  cw_uint32_t i, num_buffers;
  void * buffer;
  
  _cw_check_ptr(a_bufpool);
  _cw_assert(a_bufpool->magic == _CW_BUFPOOL_MAGIC);
  
#ifdef _CW_REENTRANT
  mtx_delete(&a_bufpool->lock);
#endif

  for (i = 0, num_buffers = list_count(&a_bufpool->spare_buffers);
       i < num_buffers;
       i++)
  {
    buffer = list_hpop(&a_bufpool->spare_buffers);
    _cw_free(buffer);
  }

#ifdef _LIBSTASH_DBG
  bzero(a_bufpool, sizeof(cw_bufpool_t));
#endif
  if (a_bufpool->is_malloced)
  {
    _cw_free(a_bufpool);
  }
}

cw_uint32_t
bufpool_get_buffer_size(cw_bufpool_t * a_bufpool)
{
  _cw_check_ptr(a_bufpool);
  _cw_assert(a_bufpool->magic == _CW_BUFPOOL_MAGIC);

  return a_bufpool->buffer_size;
}

cw_uint32_t
bufpool_get_max_spare_buffers(cw_bufpool_t * a_bufpool)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_bufpool);
  _cw_assert(a_bufpool->magic == _CW_BUFPOOL_MAGIC);

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufpool->lock);
#endif

  retval = a_bufpool->max_spare_buffers;
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufpool->lock);
#endif
  
  return retval;
}

void
bufpool_set_max_spare_buffers(cw_bufpool_t * a_bufpool,
			      cw_uint32_t a_max_spare_buffers)
{
  cw_uint32_t i, overflow;
  void * buffer;
  
  _cw_check_ptr(a_bufpool);
  _cw_assert(a_bufpool->magic == _CW_BUFPOOL_MAGIC);

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufpool->lock);
#endif

  if (list_count(&a_bufpool->spare_buffers) > a_max_spare_buffers)
  {
    for (i = 0,
	   overflow = (list_count(&a_bufpool->spare_buffers)
		       - a_max_spare_buffers);
	 i < overflow;
	 i++)
    {
      buffer = list_hpop(&a_bufpool->spare_buffers);
      _cw_free(buffer);
    }
  }

  a_bufpool->max_spare_buffers = a_max_spare_buffers;
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufpool->lock);
#endif
}

void *
bufpool_get_buffer(cw_bufpool_t * a_bufpool)
{
  void * retval;
  
  _cw_check_ptr(a_bufpool);
  _cw_assert(a_bufpool->magic == _CW_BUFPOOL_MAGIC);

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufpool->lock);
#endif

  retval = list_hpop(&a_bufpool->spare_buffers);
  if (NULL == retval)
  {
    retval = _cw_malloc(a_bufpool->buffer_size);
#ifdef _LIBSTASH_DBG
    bzero(retval, a_bufpool->buffer_size);
#endif
  }
  
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufpool->lock);
#endif
  return retval;
}

void
bufpool_put_buffer(void * a_bufpool, void * a_buffer)
{
  cw_bufpool_t * bufpool = (cw_bufpool_t *) a_bufpool;
  
  _cw_check_ptr(bufpool);
  _cw_assert(bufpool->magic == _CW_BUFPOOL_MAGIC);

#ifdef _CW_REENTRANT
  mtx_lock(&bufpool->lock);
#endif

#ifdef _LIBSTASH_DBG
    bzero(a_buffer, bufpool->buffer_size);
#endif
    
  if (list_count(&bufpool->spare_buffers) < bufpool->max_spare_buffers)
  {
    list_hpush(&bufpool->spare_buffers, a_buffer);
  }
  else
  {
    _cw_free(a_buffer);
  }

#ifdef _CW_REENTRANT
  mtx_unlock(&bufpool->lock);
#endif
}

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
  retval->array = (cw_bufel_array_el_t *)
    _cw_calloc(_LIBSTASH_BUF_ARRAY_MIN_SIZE, sizeof(cw_bufel_array_el_t));
  retval->iov = (struct iovec *) _cw_calloc(_LIBSTASH_BUF_ARRAY_MIN_SIZE,
					    sizeof(struct iovec));

#ifdef _LIBSTASH_DBG
  bzero(retval->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
			* sizeof(cw_bufel_array_el_t)));
  bzero(retval->iov, _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif
  
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

#ifdef _LIBSTASH_DBG
  bzero(a_buf, sizeof(cw_buf_t));
#endif
  if (a_buf->is_malloced)
  {
    _cw_free(a_buf);
  }
}

void
buf_dump(cw_buf_t * a_buf, const char * a_prefix)
{
  cw_uint32_t i;
  char * sub_prefix;
  
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_prefix);

  sub_prefix = _cw_malloc(strlen(a_prefix) + 1 + 2);
  strcpy(sub_prefix, a_prefix);
  strcat(sub_prefix, "| | ");
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  log_printf(cw_g_log,
	     "%s| buf_dump()\n",
	     a_prefix);
#ifdef _LIBSTASH_DBG
  log_printf(cw_g_log,
	     "%s|--> magic : 0x%x\n",
	     a_prefix, a_buf->magic);
#endif
  log_printf(cw_g_log,
	     "%s|--> is_malloced : %s\n",
	     a_prefix, (a_buf->is_malloced) ? "TRUE" : "FALSE");
#ifdef _CW_REENTRANT
  log_printf(cw_g_log,
	     "%s|--> is_threadsafe : %s\n",
	     a_prefix, (a_buf->is_threadsafe) ? "TRUE" : "FALSE");
#endif
  log_printf(cw_g_log,
	     "%s|--> size : %u\n",
	     a_prefix, a_buf->size);
  log_printf(cw_g_log,
	     "%s|--> array_size : %u\n",
	     a_prefix, a_buf->array_size);
  log_printf(cw_g_log,
	     "%s|--> array_num_valid : %u\n",
	     a_prefix, a_buf->array_num_valid);
  log_printf(cw_g_log,
	     "%s|--> array_start : %u\n",
	     a_prefix, a_buf->array_start);
  log_printf(cw_g_log,
	     "%s|--> array_end : %u\n",
	     a_prefix, a_buf->array_end);
  log_printf(cw_g_log,
	     "%s|--> is_cumulative_valid : %s\n",
	     a_prefix, (a_buf->is_cumulative_valid) ? "TRUE" : "FALSE");
  for (i = 0; i < a_buf->array_size; i++)
  {
    log_printf(cw_g_log,
	       "%s|\\\n"
	       "%s| |--> array[%d].bufel : \n"
	       "%s| |\\\n",
	       a_prefix, a_prefix, i, a_prefix);
    bufel_dump(&a_buf->array[i].bufel, sub_prefix);
    log_printf(cw_g_log,
	       "%s| \\--> array[%d].cumulative_size : %u\n",
	       a_prefix, i, a_buf->array[i].cumulative_size);
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  _cw_free(sub_prefix);
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

const struct iovec *
buf_get_iovec(cw_buf_t * a_buf, cw_uint32_t a_max_data, int * a_iovec_count)
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
    array_index = (a_buf->array_start + i) % a_buf->array_size;
    
    a_buf->iov[i].iov_base
      = (((char *) bufel_get_data_ptr(&a_buf->array[array_index].bufel))
	 + bufel_get_beg_offset(&a_buf->array[array_index].bufel));
    a_buf->iov[i].iov_len
      = (size_t) bufel_get_valid_data_size(&a_buf->array[array_index].bufel);
    
    num_bytes += a_buf->iov[i].iov_len;
  }
  
  /* Adjust the iovec size downward if necessary. */
  if (num_bytes > a_max_data)
  {
    a_buf->iov[i - 1].iov_len -= (num_bytes - a_max_data);
  }

  *a_iovec_count = i;

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  
  return a_buf->iov;
}

void
buf_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve)
{
  _cw_check_ptr(a_a);
  _cw_assert(a_a->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_b);
  _cw_assert(a_b->magic == _CW_BUF_MAGIC);
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

  buf_p_catenate_buf(a_a, a_b, a_preserve);
  
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
buf_split(cw_buf_t * a_a, cw_buf_t * a_b, cw_uint32_t a_offset)
{
  cw_uint32_t array_element, bufel_offset, num_bufels_to_move;
  cw_uint32_t i, a_a_index, a_b_index;
  cw_bool_t did_bufel_merge = FALSE;

  _cw_check_ptr(a_a);
  _cw_assert(a_a->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_b);
  _cw_assert(a_b->magic == _CW_BUF_MAGIC);
  _cw_assert(a_offset <= buf_get_size(a_b));

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

    /* Make sure that a_a's array is big enough. */
    buf_p_fit_array(a_a, a_a->array_num_valid + num_bufels_to_move);

    /* Try to merge first bufel of a_b and last bufel of a_a. */
    if ((num_bufels_to_move > 0) && (a_a->array_num_valid > 0))
    {
      cw_uint32_t last_element_index;
    
      last_element_index = ((a_a->array_end + a_a->array_size - 1)
			    % a_a->array_size);

      did_bufel_merge
	= ! bufel_p_merge_bufel(&a_a->array[last_element_index].bufel,
				&a_b->array[a_b->array_start].bufel);
      if (did_bufel_merge)
      {
	a_a->size
	  += bufel_get_valid_data_size(&a_b->array[a_b->array_start].bufel);
	a_a->array[last_element_index].cumulative_size = a_a->size;
      
	num_bufels_to_move--;
#ifdef _LIBSTASH_DBG
	bzero(&a_b->array[a_b->array_start], sizeof(cw_bufel_array_el_t));
#endif
	a_b->array_start = (a_b->array_start + 1) % a_b->array_size;
	a_b->array_num_valid--;
      }
    }
    
    /* Iterate through the bufel's in a_b and move them to a_a, up to and
     * including the bufel where the split occurs. */
    for (i = 0,
	   a_a_index = a_a->array_end,
	   a_b_index = a_b->array_start;
	 i < num_bufels_to_move;
	 i++)
    {
      a_a_index = (i + a_a->array_end) % a_a->array_size;
      a_b_index = (i + a_b->array_start) % a_b->array_size;
    
      memcpy(&a_a->array[a_a_index].bufel,
	     &a_b->array[a_b_index].bufel,
	     sizeof(cw_bufel_t));

      a_a->size += bufel_get_valid_data_size(&a_a->array[a_a_index].bufel);
      a_a->array[a_a_index].cumulative_size = a_a->size;
    
#ifdef _LIBSTASH_DBG
      bzero(&a_b->array[a_b_index], sizeof(cw_bufel_array_el_t));
#endif
    }

    /* Deal with the bufel that the split is in. */
    if (bufel_offset == 0)
    {
      /* The split is actually between bufel's. */
    }
    else
    {
#ifdef _LIBSTASH_DBG
      /* Copy the bufel back to a_b, since the data is split and the original
       * bufel must still remain valid. */
      memcpy(&a_b->array[a_b_index].bufel,
	     &a_a->array[a_a_index].bufel,
	     sizeof(cw_bufel_t));
#endif
      /* Decrement a_a->size, since we don't want the whole bufc. */
      a_a->size -= bufel_get_valid_data_size(&a_a->array[a_a_index].bufel);

      /* Increment the reference count for the buffer, and set the offsets
       * appropriately for both bufel's. */
      bufc_ref_increment(a_a->array[a_a_index].bufel.bufc);
		  
      bufel_set_end_offset(&a_a->array[a_a_index].bufel, bufel_offset);
      bufel_set_beg_offset(&a_b->array[a_b_index].bufel, bufel_offset);

      a_a->size += bufel_get_valid_data_size(&a_a->array[a_a_index].bufel);
      a_a->array[a_a_index].cumulative_size = a_a->size;
    }

    /* Make a_a's and a_b's states consistent. */
    a_a->array_num_valid += num_bufels_to_move;
    a_a->array_end = (a_a_index + 1) % a_a->array_size;
  
    a_b->array_num_valid -= num_bufels_to_move;
    if (bufel_offset != 0)
    {
      a_b->array_num_valid++;
      a_b->array_start = a_b_index;
    }
    else
    {
      a_b->array_start = (a_b_index + 1) % a_b->array_size;
    }
    a_b->is_cumulative_valid = FALSE;
    a_b->size -= a_offset;

    if ((a_b->array_num_valid == 0)
	&& (a_b->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
      /* Shrink the array back down. */
      a_b->array = (cw_bufel_array_el_t *)
	_cw_realloc(a_b->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
				 * sizeof(cw_bufel_array_el_t)));
      a_b->iov = (struct iovec *)
	_cw_realloc(a_b->iov, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
			       * sizeof(struct iovec)));

#ifdef _LIBSTASH_DBG
      bzero(a_b->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
			 * sizeof(cw_bufel_array_el_t)));
      bzero(a_b->iov, _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif
      
      a_b->array_size = _LIBSTASH_BUF_ARRAY_MIN_SIZE;
      a_b->array_start = 0;
      a_b->array_end = 0;
      a_b->is_cumulative_valid = TRUE;
    }
  }
  else if ((a_offset > 0) && (a_offset == a_b->size))
  {
    /* Same as catenation. */
    buf_p_catenate_buf(a_a, a_b, FALSE);
  }
  
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
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (NULL == bufel_get_data_ptr(a_bufel))
  {
    /* Do nothing, since a_bufel is empty. */
  }  /* Try to merge a_bufel into the first bufel in a_buf. */
  else if ((a_buf->array_num_valid > 0)
	   && (bufel_get_data_ptr(&a_buf->array[a_buf->array_start].bufel)
	       == bufel_get_data_ptr(a_bufel))
	   && (bufel_get_end_offset(&a_buf->array[a_buf->array_start].bufel)
	       == bufel_get_beg_offset(a_bufel)))
  {
    /* These two bufel's reference the same bufc, and the buffer regions they
     * refer to are consecutive and adjacent.  Merge the two bufel's
     * together. */
    bufel_set_end_offset(&a_buf->array[a_buf->array_start].bufel,
			 (bufel_get_end_offset(
			   &a_buf->array[a_buf->array_start].bufel)
			  + bufel_get_valid_data_size(
			    a_bufel)));
      
    a_buf->size
      += bufel_get_valid_data_size(a_bufel);
      
    a_buf->array[a_buf->array_start].cumulative_size = a_buf->size;
  }
  else
  {
    buf_p_fit_array(a_buf, a_buf->array_num_valid + 1);
  
    /* Now prepend the bufel. */
    a_buf->array_start = (((a_buf->array_start + a_buf->array_size) - 1)
			  % a_buf->array_size);
  
    a_buf->array_num_valid++;
    memcpy(&a_buf->array[a_buf->array_start].bufel,
	   a_bufel,
	   sizeof(cw_bufel_t));
    a_buf->array[a_buf->array_start].bufel.is_malloced = FALSE;

    a_buf->size += bufel_get_valid_data_size(a_bufel);
    a_buf->is_cumulative_valid = FALSE;

    bufc_ref_increment(a_bufel->bufc);
  }
  
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
  cw_bool_t did_bufel_merge = FALSE;
  
  _cw_check_ptr(a_buf);
  _cw_assert(a_buf->magic == _CW_BUF_MAGIC);
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (NULL == bufel_get_data_ptr(a_bufel))
  {
    /* Do nothing, since a_bufel is empty. */
  }
  /* Try to merge a_bufel into the last bufel in a_buf. */
  if (a_buf->array_num_valid > 0)
  {
    cw_uint32_t last_element_index;
    
    last_element_index = ((a_buf->array_end + a_buf->array_size - 1)
			  % a_buf->array_size);
  
    if ((a_buf->array_num_valid > 0)
	&& (bufel_get_data_ptr(&a_buf->array[last_element_index].bufel)
	    == bufel_get_data_ptr(a_bufel))
	&& (bufel_get_end_offset(&a_buf->array[last_element_index].bufel)
	    == bufel_get_beg_offset(a_bufel)))
    {
      /* These two bufel's reference the same bufc, and the buffer regions they
       * refer to are consecutive and adjacent.  Merge the two bufel's
       * together. */
      bufel_set_end_offset(&a_buf->array[last_element_index].bufel,
			   (bufel_get_end_offset(
			     &a_buf->array[last_element_index].bufel)
			    + bufel_get_valid_data_size(
			      a_bufel)));
      
      a_buf->size
	+= bufel_get_valid_data_size(a_bufel);
      
      a_buf->array[last_element_index].cumulative_size = a_buf->size;

      did_bufel_merge = TRUE;
    }
  }
  
  if (FALSE == did_bufel_merge)
  {
    buf_p_fit_array(a_buf, a_buf->array_num_valid + 1);
  
    /* Now append the bufel. */
    memcpy(&a_buf->array[a_buf->array_end].bufel,
	   a_bufel,
	   sizeof(cw_bufel_t));
    a_buf->array_num_valid++;
    a_buf->size += bufel_get_valid_data_size(a_bufel);
    a_buf->array[a_buf->array_end].cumulative_size = a_buf->size;
    a_buf->array[a_buf->array_end].bufel.is_malloced = FALSE;
	      
    a_buf->array_end = ((a_buf->array_end + 1) % a_buf->array_size);

    bufc_ref_increment(a_bufel->bufc);
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
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
	 array_index = (array_index + 1) % a_buf->array_size)
    {
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	bufel_delete(&a_buf->array[array_index].bufel);
	a_buf->array_start = (array_index + 1) % a_buf->array_size;
	a_buf->array_num_valid--;
	amount_left -= bufel_valid_data;
      }
      else /* if (bufel_valid_data > amount_left) */
      {
	/* This will finish things up. */
	bufel_set_beg_offset(&a_buf->array[array_index].bufel,
			     (bufel_get_beg_offset(
			       &a_buf->array[array_index].bufel)
			      + amount_left));
	amount_left = 0;
      }
    }

    /* Adjust the buf size. */
    a_buf->size -= a_amount;
      
#if (0)
    for (i = 0,
	   array_index = a_buf->array_start,
	   amount_left = a_amount;
	 amount_left > 0;
	 i++)
    {
      array_index = (i + a_buf->array_start) % a_buf->array_size;
      
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	amount_left -= bufel_valid_data;
	bufel_delete(&a_buf->array[array_index].bufel);
      }
      else if (bufel_valid_data > amount_left)
      {
	/* This will finish things up. */
	bufel_set_beg_offset(&a_buf->array[array_index].bufel,
			     (bufel_get_beg_offset(
			       &a_buf->array[array_index].bufel)
			      + amount_left));
	amount_left = 0;
      }
    }
    
    /* Adjust the array variables. */
    a_buf->size -= a_amount;
    a_buf->array_start = ((array_index + a_buf->array_size)
			  % a_buf->array_size);
    a_buf->array_num_valid -= (i - 1);

#endif
    if ((a_buf->array_num_valid == 0)
	&& (a_buf->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
      /* Shrink the array back down. */
      a_buf->array = (cw_bufel_array_el_t *)
	_cw_realloc(a_buf->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
				   * sizeof(cw_bufel_array_el_t)));
      a_buf->iov = (struct iovec *)
	_cw_realloc(a_buf->iov, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
				 * sizeof(struct iovec)));
      
#ifdef _LIBSTASH_DBG
      bzero(a_buf->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
			   * sizeof(cw_bufel_array_el_t)));
      bzero(a_buf->iov, _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif
  
      a_buf->array_size = _LIBSTASH_BUF_ARRAY_MIN_SIZE;
      a_buf->array_start = 0;
      a_buf->array_end = 0;
      a_buf->is_cumulative_valid = TRUE;
    }

    a_buf->is_cumulative_valid = FALSE;

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
	   = (a_buf->array_end + a_buf->array_size - 1) % a_buf->array_size,
	   amount_left = a_amount;
	 amount_left > 0;
	 array_index
	   = (array_index + a_buf->array_size - 1) % a_buf->array_size)
    {
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	bufel_delete(&a_buf->array[array_index].bufel);
	a_buf->array_end = array_index;
	a_buf->array_num_valid--;
	amount_left -= bufel_valid_data;
      }
      else /* if (bufel_valid_data > amount_left) */
      {
	/* This will finish things up. */
	bufel_set_end_offset(&a_buf->array[array_index].bufel,
			     (bufel_get_end_offset(
			       &a_buf->array[array_index].bufel)
			      - amount_left));
	amount_left = 0;
      }
    }

    /* Adjust the buf size. */
    a_buf->size -= a_amount;
    
#if (0)
    for (i = 0,
	   array_index = ((a_buf->array_size + a_buf->array_end - 1)
			  % a_buf->array_size),
	   amount_left = a_amount;
	 (amount_left > 0);
	 i++)
    {
      array_index = ((a_buf->array_size + a_buf->array_end - 1 - i)
		     % a_buf->array_size);
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= amount_left)
      {
	/* Need to get rid of the bufel. */
	amount_left -= bufel_valid_data;
	bufel_delete(&a_buf->array[array_index].bufel);
      }
      else if (bufel_valid_data > amount_left)
      {
	/* This will finish things up. */
	bufel_set_end_offset(&a_buf->array[array_index].bufel,
			     (bufel_get_end_offset(
			       &a_buf->array[array_index].bufel)
			      - amount_left));
	a_buf->array[array_index].cumulative_size -= amount_left;
	amount_left = 0;
      }
    }
    
    /* Adjust the array variables. */
    a_buf->size -= a_amount;
    a_buf->array_end = ((array_index + a_buf->array_size - 1)
			% a_buf->array_size);
    a_buf->array_num_valid -= (i - 1);
#endif

    if ((a_buf->array_num_valid == 0)
	&& (a_buf->array_size != _LIBSTASH_BUF_ARRAY_MIN_SIZE))
    {
      /* Shrink the array back down. */
      a_buf->array = (cw_bufel_array_el_t *)
	_cw_realloc(a_buf->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
				   * sizeof(cw_bufel_array_el_t)));
      a_buf->iov = (struct iovec *)
	_cw_realloc(a_buf->iov, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
				 * sizeof(struct iovec)));

#ifdef _LIBSTASH_DBG
      bzero(a_buf->array, (_LIBSTASH_BUF_ARRAY_MIN_SIZE
			   * sizeof(cw_bufel_array_el_t)));
      bzero(a_buf->iov, _LIBSTASH_BUF_ARRAY_MIN_SIZE * sizeof(struct iovec));
#endif
      
      a_buf->array_size = _LIBSTASH_BUF_ARRAY_MIN_SIZE;
      a_buf->array_start = 0;
      a_buf->array_end = 0;
      a_buf->is_cumulative_valid = TRUE;
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

  retval = *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
	     + bufel_offset);
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  return retval;
}

void
buf_set_uint8(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint8_t a_val)
{
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

  *(bufc_get_p(a_buf->array[array_element].bufel.bufc) + bufel_offset)
    = a_val;
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
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

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  if (bufel_offset + 3
      < 
      a_buf->array[array_element].bufel.end_offset)
  {
    retval = ((cw_uint32_t)
	      *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		+ a_buf->array[array_element].bufel.beg_offset + bufel_offset))
		  << 24;
    
    retval |= (((cw_uint32_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 1))
	       << 16) & 0x00ff0000;
    
    retval |= (((cw_uint32_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 2))
	       << 8) & 0x0000ff00;
    
    retval |= ((cw_uint32_t)
	       *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		 + a_buf->array[array_element].bufel.beg_offset
		 + bufel_offset + 3))
      & 0x000000ff;
  }
  else
  {
    /* The data is spread across two to four buffers. */
    retval = (*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		+ a_buf->array[array_element].bufel.beg_offset + bufel_offset)
	      << 24);
    
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= ((*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset + bufel_offset)
		<< 16) & 0x00ff0000);
    
    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= ((*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset + bufel_offset)
		<< 8) & 0x0000ff00);
    
    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= (*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		 + a_buf->array[array_element].bufel.beg_offset + bufel_offset)
	       & 0x000000ff);
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  
  return ntohl(retval);
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
      a_buf->array[array_element].bufel.end_offset)
  {
    retval = ((cw_uint64_t)
	      *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		+ a_buf->array[array_element].bufel.beg_offset + bufel_offset))
		  << 56;
    
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 1))
		<< 48) & (((cw_uint64_t) 0x00ff0000 << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 2))
		<< 40) & (((cw_uint64_t) 0x0000ff00 << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 3))
		<< 32) & (((cw_uint64_t) 0x000000ff << 32) | 0x00000000);

    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 4))
		<< 24) & (((cw_uint64_t) 0x00000000 << 32) | 0xff000000);

    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 5))
		<< 16) & (((cw_uint64_t) 0x00000000 << 32) | 0x00ff0000);

    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset + 6))
		<< 8) & (((cw_uint64_t) 0x00000000 << 32) | 0x0000ff00);

    retval |= ((cw_uint64_t)
	       *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		 + a_buf->array[array_element].bufel.beg_offset
		 + bufel_offset + 7))
	       & (((cw_uint64_t) 0x00000000 << 32) | 0x000000ff);
  }
  else
  {
    /* The data is spread across two to eight buffers. */
    retval = ((cw_uint64_t)
	      *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		+ a_buf->array[array_element].bufel.beg_offset + bufel_offset))
		  << 56;
    
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 48) & (((cw_uint64_t) 0x00ff0000 << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 40) & (((cw_uint64_t) 0x0000ff00 << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 32) & (((cw_uint64_t) 0x000000ff << 32) | 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 4, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 24) & (((cw_uint64_t) 0x00000000 << 32) | 0xff000000);

    buf_p_get_data_position(a_buf, a_offset + 5, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 16) & (((cw_uint64_t) 0x00000000 << 32) | 0x00ff0000);

    buf_p_get_data_position(a_buf, a_offset + 6, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		*(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		  + a_buf->array[array_element].bufel.beg_offset
		  + bufel_offset))
	       << 8) & (((cw_uint64_t) 0x00000000 << 32) | 0x0000ff00);

    buf_p_get_data_position(a_buf, a_offset + 7, &array_element, &bufel_offset);
    retval |= ((cw_uint64_t)
	       *(bufc_get_p(a_buf->array[array_element].bufel.bufc)
		 + a_buf->array[array_element].bufel.beg_offset
		 + bufel_offset))
      & (((cw_uint64_t) 0x00000000 << 32) | 0x000000ff);
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  return _cw_ntohq(retval);
}

static void
buf_p_rebuild_cumulative_index(cw_buf_t * a_buf)
{
  cw_uint32_t i, cumulative;
  
  for (i = cumulative = 0;
       i < a_buf->array_num_valid;
       i++)
  {
    cumulative
      += bufel_get_valid_data_size(&a_buf->array[(i + a_buf->array_start)
						% a_buf->array_size].bufel);
    a_buf->array[(i + a_buf->array_start)
		% a_buf->array_size].cumulative_size = cumulative;
  }
  a_buf->is_cumulative_valid = TRUE;
}

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset)
{
  /* First look to see if what we're looking for is in the first bufel. */
  if (a_offset
      < bufel_get_valid_data_size(&a_buf->array[a_buf->array_start].bufel))
  {
    *a_array_element = a_buf->array_start;
    *a_bufel_offset = a_offset;
  }
  else
  {
    cw_uint32_t i, adjust, index, index_prev;
    
    if (FALSE == a_buf->is_cumulative_valid)
    {
      buf_p_rebuild_cumulative_index(a_buf);
    }

    /* Do a binary search through the cumulative index to find the bufel we
     * want. */
    for (i = 1,
	   adjust = (((a_buf->array_num_valid >> i) > 0)
		     ? (a_buf->array_num_valid >> i) : 1),
	   index = (a_buf->array_start + adjust) % a_buf->array_size,
	   index_prev = (index + a_buf->array_size - 1) % a_buf->array_size;
	 !((a_buf->array[index].cumulative_size > a_offset)
	   && (a_buf->array[index_prev].cumulative_size <= a_offset));
	 i++,
	   adjust = ((adjust >> 1) > 0) ? (adjust >> 1) : 1,
	   index = ((a_buf->array[index].cumulative_size <= a_offset)
		    ? (index + adjust) % a_buf->array_size
		    : (index + a_buf->array_size - adjust) % a_buf->array_size),
	   index_prev = (index + a_buf->array_size - 1) % a_buf->array_size);
    
    *a_array_element = index;
    *a_bufel_offset = a_offset - a_buf->array[index_prev].cumulative_size;
  }
}

static void
buf_p_fit_array(cw_buf_t * a_buf, cw_uint32_t a_min_array_size)
{
  cw_uint32_t i;

  /* Make sure a_buf's array is big enough.  Even if we're trying to merge
   * bufel's, make the array big enough that it doesn't matter how successful
   * the bufel merging is. */
  if (a_min_array_size > a_buf->array_size)
  {
    /* Double i until it is big enough to accomodate our needs. */
    for (i = 2 * a_buf->array_size;
	 i < a_min_array_size;
	 i *= 2);

    a_buf->array
      = (cw_bufel_array_el_t *) _cw_realloc(a_buf->array,
					    i * sizeof(cw_bufel_array_el_t));
    a_buf->iov = (struct iovec *) _cw_realloc(a_buf->iov,
					      i * sizeof(struct iovec));
    
#ifdef _LIBSTASH_DBG
      bzero(&a_buf->array[a_buf->array_size],
			    ((i - a_buf->array_size)
			     * sizeof(cw_bufel_array_el_t)));
      bzero(&a_buf->iov[a_buf->array_size], ((i - a_buf->array_size)
					     * sizeof(struct iovec)));
#endif
      
    if ((a_buf->array_start >= a_buf->array_end)
	&& (a_buf->array_num_valid > 0)) /* array_num_valid check probably isn't
					  * necessary. */
    {
      /* The array was wrapped, so we need to move the wrapped part to sit
       * directly after where the end of the array used to be.  Since we at
       * least doubled the size of the array, there is no worry of writing past
       * the end of the array. */
      memcpy(&a_buf->array[a_buf->array_size],
	     a_buf->array,
	     a_buf->array_end * sizeof(cw_bufel_array_el_t));
#ifdef _LIBSTASH_DBG
      /* Zero the old copy to get rid of the bufel's' magic. */
      bzero(a_buf->array, (a_buf->array_end * sizeof(cw_bufel_array_el_t)));
#endif
      a_buf->array_end = a_buf->array_start + a_buf->array_num_valid;
    }

    /* This must happen last, since the old value is used for some calculations
     * above. */
    a_buf->array_size = i;
  }
}

static void
buf_p_catenate_buf(cw_buf_t * a_a, cw_buf_t * a_b, cw_bool_t a_preserve)
{
  cw_uint32_t i, a_a_index, a_b_index;
  cw_bool_t did_bufel_merge = FALSE;
  
  buf_p_fit_array(a_a, a_a->array_num_valid + a_b->array_num_valid);
    
  /* Try to merge the last bufel in a_a and the first bufel in a_b into one
   * bufel in a_a. */
  if ((a_a->array_num_valid > 0) && (a_b->array_num_valid > 0))
  {
    cw_uint32_t last_element_index;
    
    last_element_index = ((a_a->array_end + a_a->array_size - 1)
			  % a_a->array_size);

    did_bufel_merge
      = ! bufel_p_merge_bufel(&a_a->array[last_element_index].bufel,
			      &a_b->array[a_b->array_start].bufel);
    if (did_bufel_merge)
    {
      a_a->size
	+= bufel_get_valid_data_size(&a_b->array[a_b->array_start].bufel);
      a_a->array[last_element_index].cumulative_size = a_a->size;
      
      if (FALSE == a_preserve)
      {
	bufel_delete(&a_b->array[a_b->array_start].bufel);
      }
    }
  }
  
  /* Iterate through a_b's array, creating bufel's in a_a and adding references
   * to a_b's bufel data. */
  for (i = 0,
	 a_a_index = a_a->array_end,
	 a_b_index
	 = (a_b->array_start + (did_bufel_merge ? 1 : 0)) % a_b->array_size;
       i < a_b->array_num_valid - (did_bufel_merge ? 1 : 0);
       i++,
	 a_a_index = (a_a_index + 1) % a_a->array_size,
	 a_b_index = (a_b_index + 1) % a_b->array_size)
  {
    memcpy(&a_a->array[a_a_index].bufel,
	   &a_b->array[a_b_index].bufel,
	   sizeof(cw_bufel_t));

    a_a->size
      += bufel_get_valid_data_size(&a_a->array[a_a_index].bufel);

    a_a->array[a_a_index].cumulative_size = a_a->size;

    if (TRUE == a_preserve)
    {
      bufc_ref_increment(a_a->array[a_a_index].bufel.bufc);
    }
#ifdef _LIBSTASH_DBG
    else
    {
      bzero(&a_b->array[a_b_index], sizeof(cw_bufel_array_el_t));
    }
#endif
  }

  /* Finish making a_a's state consistent. */
  a_a->array_end = a_a_index;
  a_a->array_num_valid += i;

  /* Make a_b's state consistent if not preserving its state. */
  if (FALSE == a_preserve)
  {
    a_b->size = 0;
    a_b->array_num_valid = 0;
    a_b->array_start = 0;
    a_b->array_end = 0;
    a_b->is_cumulative_valid = FALSE;
  }
}

cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel,
	  void (*a_dealloc_func)(void * dealloc_arg, void * bufel),
	  void * a_dealloc_arg)
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

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_BUFEL_MAGIC;
#endif

  retval->dealloc_func = a_dealloc_func;
  retval->dealloc_arg = a_dealloc_arg;
  
  return retval;
}

void
bufel_delete(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);

  if (a_bufel->bufc != NULL)
  {
    bufc_ref_decrement(a_bufel->bufc);
  }

  if (a_bufel->is_malloced == TRUE)
  {
#ifdef _LIBSTASH_DBG
  bzero(a_bufel, sizeof(cw_bufel_t));
#endif
    _cw_free(a_bufel);
  }
  else if (NULL != a_bufel->dealloc_func)
  {
    a_bufel->dealloc_func(a_bufel->dealloc_arg, (void *) a_bufel);
  }
}

void
bufel_dump(cw_bufel_t * a_bufel, const char * a_prefix)
{
  char * sub_prefix;
  
  _cw_check_ptr(a_bufel);
  _cw_check_ptr(a_prefix);

  sub_prefix = _cw_malloc(strlen(a_prefix) + 1 + 2);
  strcpy(sub_prefix, a_prefix);
  strcat(sub_prefix, "  ");
  
  log_printf(cw_g_log,
	     "%s| bufel_dump()\n",
	     a_prefix);
#ifdef _LIBSTASH_DBG
  log_printf(cw_g_log,
	     "%s|--> magic : 0x%x\n",
	     a_prefix, a_bufel->magic);
#endif
  log_printf(cw_g_log,
	     "%s|--> is_malloced : %s\n",
	     a_prefix, (a_bufel->is_malloced) ? "TRUE" : "FALSE");
  log_printf(cw_g_log,
	     "%s|--> free_func : %p\n",
	     a_prefix, a_bufel->dealloc_func);
  log_printf(cw_g_log,
	     "%s|--> free_arg : %p\n",
	     a_prefix, a_bufel->dealloc_arg);
  log_printf(cw_g_log,
	     "%s|--> beg_offset : %u\n",
	     a_prefix, a_bufel->beg_offset);
  log_printf(cw_g_log,
	     "%s|--> end_offset : %u\n",
	     a_prefix, a_bufel->end_offset);
  if (NULL != a_bufel->bufc)
  {
    log_printf(cw_g_log,
	       "%s|--> bufc : 0x%x\n"
	       "%s \\\n",
	       a_prefix, a_bufel->bufc, a_prefix);
    bufc_dump(a_bufel->bufc, sub_prefix);
  }
  else
  {
    log_printf(cw_g_log,
	       "%s\\--> bufc : 0x%x\n",
	       a_prefix, a_bufel->bufc);
  }
  _cw_free(sub_prefix);
}

cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel)
{
  cw_uint32_t retval;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);

  if (NULL == a_bufel->bufc)
  {
    retval = 0;
  }
  else
  {
    retval = bufc_get_size(a_bufel->bufc);
  }
  
  return retval;
}

cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
  return a_bufel->beg_offset;
}

cw_bool_t
bufel_set_beg_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);

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
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
  return a_bufel->end_offset;
}

cw_bool_t
bufel_set_end_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);

  if (a_offset < a_bufel->beg_offset)
  {
    retval = TRUE;
  }
  else if (a_offset > bufc_get_size(a_bufel->bufc))
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
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
  _cw_assert(a_bufel->end_offset >= a_bufel->beg_offset);

  return a_bufel->end_offset - a_bufel->beg_offset;
}

const void *
bufel_get_data_ptr(cw_bufel_t * a_bufel)
{
  void * retval;
  
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
  /* Shouldn't have a bufc with a NULL buf. */
  _cw_assert(0 == ((a_bufel->bufc != NULL)
		   && (bufc_get_p(a_bufel->bufc) == NULL)));
  
  /* Since the user can potentially write to the buffer at any time after this,
   * we need to be sure that there's only one reference to the bufc, in order to
   * keep from messing up other bufel's. */
  if (a_bufel->bufc == NULL)
  {
    retval = NULL;
  }
  else
  {
    retval = (void *) bufc_get_p(a_bufel->bufc);
  }

  return retval;
}

void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size,
		   void (*a_dealloc_func)(void * dealloc_arg, void * buffer_p),
		   void * a_dealloc_arg)
{
  _cw_check_ptr(a_bufel);
  _cw_assert(a_bufel->magic == _CW_BUFEL_MAGIC);
  _cw_check_ptr(a_buf);
  _cw_assert(a_size > 0);

  if (a_bufel->bufc != NULL)
  {
    bufc_ref_decrement(a_bufel->bufc);
  }
  
  a_bufel->bufc = bufc_new(a_buf, a_size, a_dealloc_func, a_dealloc_arg);

  a_bufel->beg_offset = 0;
  a_bufel->end_offset = a_size;
}

static cw_bool_t
bufel_p_merge_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_a);
  _cw_assert(a_a->magic == _CW_BUFEL_MAGIC);
  _cw_check_ptr(a_b);
  _cw_assert(a_b->magic == _CW_BUFEL_MAGIC);
  
  if ((NULL != bufel_get_data_ptr(a_a))
      && (bufel_get_data_ptr(a_a) == bufel_get_data_ptr(a_b))
      && (bufel_get_end_offset(a_a) == bufel_get_beg_offset(a_b)))
  {
    /* These two bufel's reference the same bufc, and the buffer regions they
     * refer to are consecutive and adjacent.  Merge a_b into a_a. */
    bufel_set_end_offset(a_a, (bufel_get_end_offset(a_a)
			       + bufel_get_valid_data_size(a_b)));
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }
  
  return retval;
}

static cw_bufc_t *
bufc_new(void * a_buffer, cw_uint32_t a_size,
	 void (*a_dealloc_func)(void * dealloc_arg, void * buffer_p),
	 void * a_dealloc_arg)
{
  cw_bufc_t * retval;

  _cw_check_ptr(a_buffer);
  _cw_assert(a_size > 0);

  retval = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));

#ifdef _LIBSTASH_DBG
  retval->magic = _CW_BUFC_MAGIC;
#endif

#ifdef _CW_REENTRANT
  mtx_new(&retval->lock);
#endif

  retval->buf = (char *) a_buffer;
  retval->buf_size = a_size;
  retval->ref_count = 1;

  retval->dealloc_func = a_dealloc_func;
  retval->dealloc_arg = a_dealloc_arg;
  
  return retval;
}

static void
bufc_delete(cw_bufc_t * a_bufc)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);
  
#ifdef _CW_REENTRANT
  mtx_delete(&a_bufc->lock);
#endif

  if (NULL != a_bufc->dealloc_func)
  {
    a_bufc->dealloc_func(a_bufc->dealloc_arg, (void *) a_bufc->buf);
  }
  
#ifdef _LIBSTASH_DBG
  bzero(a_bufc, sizeof(cw_bufc_t));
#endif
  _cw_free(a_bufc);
}

static void
bufc_dump(cw_bufc_t * a_bufc, const char * a_prefix)
{
  _cw_check_ptr(a_bufc);
  _cw_check_ptr(a_prefix);

  log_printf(cw_g_log,
	     "%s| bufc_dump()\n",
	     a_prefix);
#ifdef _LIBSTASH_DBG
  log_printf(cw_g_log,
	     "%s|--> magic : 0x%x\n",
	     a_prefix, a_bufc->magic);
#endif
  log_printf(cw_g_log,
	     "%s|--> free_func : %p\n",
	     a_prefix, a_bufc->dealloc_func);
  log_printf(cw_g_log,
	     "%s|--> free_arg : %p\n",
	     a_prefix, a_bufc->dealloc_arg);
  log_printf(cw_g_log,
	     "%s|--> ref_count : %u\n",
	     a_prefix, a_bufc->ref_count);
  log_printf(cw_g_log,
	     "%s|--> buf_size : %u\n",
	     a_prefix, a_bufc->buf_size);
  log_printf(cw_g_log,
	     "%s\\--> buf : 0x%x\n",
	     a_prefix, a_bufc->buf);
}

static cw_uint32_t
bufc_get_size(cw_bufc_t * a_bufc)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);

  return a_bufc->buf_size;
}

static char *
bufc_get_p(cw_bufc_t * a_bufc)
{
  _cw_check_ptr(a_bufc);
  _cw_assert(a_bufc->magic == _CW_BUFC_MAGIC);

  return a_bufc->buf;
}

static void
bufc_ref_increment(cw_bufc_t * a_bufc)
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

static void
bufc_ref_decrement(cw_bufc_t * a_bufc)
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
    bufc_delete(a_bufc);
  }
#else
  a_bufc->ref_count--;
  if (0 == a_bufc->ref_count)
  {
    bufc_delete(a_bufc);
  }
#endif
}
