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

#include "libstash/buf_p.h"

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
  a_buf->array_start = (((a_buf->array_start + a_buf->array_size) - 1)
			% a_buf->array_size);
  a_buf->array_num_valid++;
  memcpy(&a_buf->array[a_buf->array_start].bufel,
	 a_bufel,
	 sizeof(cw_bufel_t));

  a_buf->size += bufel_get_valid_data_size(a_bufel);
  a_buf->is_cumulative_valid = FALSE;

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufel->bufc->lock);
#endif

  a_bufel->bufc->ref_count++;
    
#ifdef _CW_REENTRANT
  mtx_unlock(&a_bufel->bufc->lock);
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
  
  /* Now append the bufel. */
  memcpy(&a_buf->array[a_buf->array_end].bufel,
	 a_bufel,
	 sizeof(cw_bufel_t));
  a_buf->array_num_valid++;
  a_buf->size += bufel_get_valid_data_size(a_bufel);
  a_buf->array[a_buf->array_end].cumulative_size = a_buf->size;
	      
  a_buf->array_end = ((a_buf->array_end + 1) % a_buf->array_size);

#ifdef _CW_REENTRANT
  mtx_lock(&a_bufel->bufc->lock);
#endif

  a_bufel->bufc->ref_count++;
  
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
  cw_uint32_t array_index, bufel_valid_data, i;
  
  _cw_check_ptr(a_buf);
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
    for (i = 0; (a_amount > 0) && (i < a_buf->array_num_valid); i++)
    {
      array_index = (i + a_buf->array_start) % a_buf->array_size;
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= a_amount)
      {
	/* Need to get rid of the bufel. */
	a_amount -= bufel_valid_data;
	bufel_delete(&a_buf->array[array_index].bufel);
      }
      else if (bufel_valid_data > a_amount)
      {
	/* This will finish things up. */
	a_buf->array[array_index].bufel.beg_offset += a_amount;
	a_amount = 0;
      }
    }
    
    /* Adjust the array variables. */
    a_buf->array_start = (a_buf->array_start + i) % a_buf->array_size;
    a_buf->array_num_valid -= i;

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
  cw_uint32_t array_index, bufel_valid_data, i;
  
  _cw_check_ptr(a_buf);
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
    for (i = 0; (a_amount > 0) && (i < a_buf->array_num_valid); i++)
    {
      array_index = ((a_buf->array_size + a_buf->array_end - 1 - i)
		     % a_buf->array_size);
      bufel_valid_data
	= bufel_get_valid_data_size(&a_buf->array[array_index].bufel);

      if (bufel_valid_data <= a_amount)
      {
	/* Need to get rid of the bufel. */
	a_amount -= bufel_valid_data;
	bufel_delete(&a_buf->array[array_index].bufel);
      }
      else if (bufel_valid_data > a_amount)
      {
	/* This will finish things up. */
	a_buf->array[array_index].bufel.end_offset -= a_amount;
	a_buf->array[array_index].cumulative_size -= a_amount;
	a_amount = 0;
      }
    }
    
    /* Adjust the array variables. */
    a_buf->array_end = ((a_buf->array_end + a_buf->array_size - i)
			% a_buf->array_size);
    a_buf->array_num_valid -= i;

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
  _cw_assert(a_offset < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  retval = a_buf->array[array_element].bufel.bufc->buf[bufel_offset];
  
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
  _cw_assert(a_offset < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  a_buf->array[array_element].bufel.bufc->buf[bufel_offset] = a_val;
  
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
  _cw_assert((a_offset + 3) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  if (bufel_offset + 3
      < 
      a_buf->array[array_element].bufel.end_offset)
  {
    cw_uint32_t a, b, bit_alignment;

    /* Yay, all of the data is in one bufel. */

    /* XXX Assumes 32 bit addresses. */
/*      a = *((a_buf->array[array_element].bufel.bufc->buf + bufel_offset) */
/*  	 & (0xfffffffc)); */
    a = *(char *) ((cw_uint32_t)
		   (a_buf->array[array_element].bufel.bufc->buf + bufel_offset)
		   & ((cw_uint32_t) 0xfffffffc));
    b = *(char *)
      ((cw_uint32_t)
       (a_buf->array[array_element].bufel.bufc->buf + bufel_offset + 4)
       & (0xfffffffc));
    
    bit_alignment
      = ((cw_uint32_t)
	 (a_buf->array[array_element].bufel.bufc->buf[bufel_offset])
	 & (0x3)) * 8;

    retval = (a << (32 - bit_alignment));
    retval |= (b >> bit_alignment);
  }
  else
  {
    /* The data is spread across two to four buffers.  Go into paranoid schizo
     * mode and make this work, no matter how ugly it gets. */

    retval = (a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
	      << 24);
    
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= ((a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 16) & 0x00ff0000);
    
    buf_p_get_data_position(a_buf, a_offset + 2, &array_element, &bufel_offset);
    retval |= ((a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 8) & 0x0000ff00);
    
    buf_p_get_data_position(a_buf, a_offset + 3, &array_element, &bufel_offset);
    retval |= (a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
	       & 0x000000ff);
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  
  return retval;
}

void
buf_set_uint32(cw_buf_t * a_buf, cw_uint32_t a_offset, cw_uint32_t a_val)
{
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert((a_offset + 3) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  /* XXX */

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
}

cw_uint64_t
buf_get_uint64(cw_buf_t * a_buf, cw_uint32_t a_offset)
{
  cw_uint64_t retval;
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert((a_offset + 7) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  if (bufel_offset + 3
      < 
      a_buf->array[array_element].bufel.end_offset)
  {
    cw_uint64_t a, b;
    cw_uint32_t bit_alignment;

    /* Yay, all of the data is in one bufel. */

    /* XXX Assumes 32 bit addresses. */
    a = *(char *)
      ((cw_uint32_t)
       (&a_buf->array[array_element].bufel.bufc->buf[bufel_offset])
       & (cw_uint32_t) 0xfffffff8);
    b = *(char *)
      ((cw_uint32_t) (&a_buf->array[array_element].bufel.bufc->buf +
		      bufel_offset + 8)
       & (cw_uint32_t) 0xfffffff8);
    
    bit_alignment =
      ((cw_uint32_t)
       (&a_buf->array[array_element].bufel.bufc->buf + bufel_offset)
	 & (0x7)) * 8;

    retval = (a << (64 - bit_alignment));
    retval |= (b >> bit_alignment);
  }
  else
  {
    /* The data is spread across two to four buffers.  Go into paranoid schizo
     * mode and make this work, no matter how ugly it gets. */

    retval = ((cw_uint64_t)
	      a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
	      << 56);
    
    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 48) & ((cw_uint64_t) 0x00ff0000 << 32) & 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 40) & ((cw_uint64_t) 0x0000ff00 << 32) & 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 32) & ((cw_uint64_t) 0x000000ff << 32) & 0x00000000);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 24) & ((cw_uint64_t) 0x00000000 << 32) & 0xff000000);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 16) & ((cw_uint64_t) 0x00000000 << 32) & 0x00ff0000);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= (((cw_uint64_t)
		a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
		<< 8) & ((cw_uint64_t) 0x00000000 << 32) & 0x0000ff00);

    buf_p_get_data_position(a_buf, a_offset + 1, &array_element, &bufel_offset);
    retval |= ((cw_uint64_t)
	       a_buf->array[array_element].bufel.bufc->buf[bufel_offset]
	       & ((cw_uint64_t) 0x00000000 << 32) & 0x000000ff);
  }

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
  return retval;
}

void
buf_set_uint64(cw_buf_t * a_buf, cw_uint64_t a_offset, cw_uint32_t a_val)
{
  cw_uint32_t array_element, bufel_offset;

  _cw_check_ptr(a_buf);
  _cw_assert((a_offset + 7) < a_buf->size);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if ((FALSE == a_buf->is_cumulative_valid)
      && (a_offset >= a_buf->array[a_buf->array_start].bufel.bufc->buf_size))
  {
    buf_p_rebuild_cumulative_index(a_buf);
  }

  buf_p_get_data_position(a_buf, a_offset, &array_element, &bufel_offset);

  /* XXX */

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe)
  {
    mtx_lock(&a_buf->lock);
  }
#endif
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
}

static void
buf_p_get_data_position(cw_buf_t * a_buf,
			cw_uint32_t a_offset,
			cw_uint32_t * a_array_element,
			cw_uint32_t * a_bufel_offset)
{
  /* XXX */
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
  _cw_check_ptr(a_bufel);

  if (a_bufel->bufc != NULL)
  {
#ifdef _CW_REENTRANT
    cw_bool_t should_delete_bufc;
    
    mtx_lock(&a_bufel->bufc->lock);
    
    a_bufel->bufc->ref_count--;
    if (0 == a_bufel->bufc->ref_count)
    {
      /* Make a note that we should delete the bufc once we've released the
       * mutex. */
      should_delete_bufc = TRUE;
    }
    else
    {
      should_delete_bufc = FALSE;
    }

    mtx_unlock(&a_bufel->bufc->lock);

    if (TRUE == should_delete_bufc)
    {
      mtx_delete(&a_bufel->bufc->lock);
      _cw_free(a_bufel->bufc);
    }
#else
    a_bufel->bufc->ref_count--;
    if (0 == a_bufel->bufc->ref_count)
    {
      _cw_free(a_bufel->bufc);
    }
#endif
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
  cw_bool_t retval = FALSE;

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
#ifdef _CW_REENTRANT
      cw_bool_t should_delete_bufc;
      mtx_lock(&a_bufel->bufc->lock);
    
      a_bufel->bufc->ref_count--;
      if (0 == a_bufel->bufc->ref_count)
      {
	/* Make a note that we should delete the bufc once we've released the
	 * mutex. */
	should_delete_bufc = TRUE;
      }
      else
      {
	should_delete_bufc = FALSE;
      }

      mtx_unlock(&a_bufel->bufc->lock);

      if (TRUE == should_delete_bufc)
      {
	mtx_delete(&a_bufel->bufc->lock);
	_cw_free(a_bufel->bufc->buf);
	_cw_free(a_bufel->bufc);
      }
#else
      a_bufel->bufc->ref_count--;
      if (0 == a_bufel->bufc->ref_count)
      {
	_cw_free(a_bufel->bufc->buf);
	_cw_free(a_bufel->bufc);
      }
#endif
      a_bufel->bufc = NULL;
    }
    
    a_bufel->beg_offset = 0;
    a_bufel->end_offset = 0;
  }
  else if (a_bufel->bufc != NULL)
  {
    if (a_bufel->bufc->buf_size != a_size)
    {
#ifdef _CW_REENTRANT
      mtx_lock(&a_bufel->bufc->lock);
#endif
      if (a_bufel->bufc->ref_count > 1)
      {
	cw_bufc_t * t_bufc;
	
	/* Someone else has a reference, so we need to make our own copy. */

	t_bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
#ifdef _CW_REENTRANT
	mtx_new(&t_bufc->lock);
#endif
	t_bufc->buf = (char *) _cw_malloc(a_size);
	t_bufc->buf_size = a_size;
	t_bufc->ref_count = 1;

	memcpy(t_bufc->buf + a_bufel->beg_offset,
	       a_bufel->bufc->buf + a_bufel->beg_offset,
	       a_bufel->end_offset - a_bufel->beg_offset);

	a_bufel->bufc->ref_count--;
      }
      else
      {
	/* No one else has a reference, so muck around freely with the current
	 * bufc. */
	a_bufel->bufc->buf = (char *) _cw_realloc(a_bufel->bufc->buf,
						  a_size);
	a_bufel->bufc->buf_size = a_size;
	if (a_bufel->beg_offset > a_bufel->bufc->buf_size)
	{
	  a_bufel->beg_offset = a_bufel->bufc->buf_size;
	  a_bufel->end_offset = a_bufel->bufc->buf_size;
	}
      }
#ifdef _CW_REENTRANT
	  mtx_unlock(&a_bufel->bufc->lock);
#endif
    }
  }
  else
  {
    /* Allocate for the first time. */
    a_bufel->bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
    a_bufel->bufc->buf = (char *) _cw_malloc(a_size);
#ifdef _CW_REENTRANT
    mtx_new(&a_bufel->bufc->lock);
#endif
    a_bufel->bufc->buf_size = a_size;
    a_bufel->bufc->ref_count = 1;
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
  else if (a_offset > a_bufel->bufc->buf_size)
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
  _cw_assert(a_bufel->end_offset >= a_bufel->beg_offset);

  return a_bufel->end_offset - a_bufel->beg_offset;
}

void *
bufel_get_data_ptr(cw_bufel_t * a_bufel)
{
  void * retval;
  
  _cw_check_ptr(a_bufel);
  /* Shouldn't have a bufc with a NULL buf. */
  _cw_assert(0 == ((a_bufel->bufc != NULL) && (a_bufel->bufc->buf == NULL)));
  
  /* Since the user can potentially write to the buffer at any time after this,
   * we need to be sure that there's only one reference to the bufc, in order to
   * keep from messing up other bufel's. */
  if (a_bufel->bufc == NULL)
  {
    retval = NULL;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_bufel->bufc->lock);
#endif
    if (a_bufel->bufc->ref_count > 1)
    {
      cw_bufc_t * t_bufc;
      
      /* Need to make our own copy. */
      t_bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
#ifdef _CW_REENTRANT
      mtx_new(&t_bufc->lock);
#endif
      t_bufc->buf = (char *) _cw_malloc(a_bufel->bufc->buf_size);
      t_bufc->buf_size = a_bufel->bufc->buf_size;
      t_bufc->ref_count = 1;

      memcpy(t_bufc->buf + a_bufel->beg_offset,
	     a_bufel->bufc->buf + a_bufel->beg_offset,
	     a_bufel->end_offset - a_bufel->beg_offset);

      /* Disengage ourselves from the old bufc. */
      a_bufel->bufc->ref_count--;
#ifdef _CW_REENTRANT
      mtx_unlock(&a_bufel->bufc->lock);
#endif
      a_bufel->bufc = t_bufc;
    }
#ifdef _CW_REENTRANT
    else
    {
      mtx_unlock(&a_bufel->bufc->lock);
    }
#endif
    
    retval = a_bufel->bufc->buf;
  }

  return retval;
}

void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size)
{
  _cw_check_ptr(a_bufel);
  _cw_check_ptr(a_buf);
  _cw_assert(a_size > 0);

  if (a_bufel->bufc == NULL)
  {
    /* Do first time allocation. */
    a_bufel->bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
    a_bufel->bufc->buf = a_buf;
    a_bufel->bufc->buf_size = a_size;
    a_bufel->bufc->ref_count = 1;
  }
  else
  {
#ifdef _CW_REENTRANT
    mtx_lock(&a_bufel->bufc->lock);
#endif
    if (a_bufel->bufc->ref_count > 1)
    {
      cw_bufc_t * t_bufc;
      
      /* Need to make our own bufc. */
      t_bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
#ifdef _CW_REENTRANT
      mtx_new(&t_bufc->lock);
#endif
      t_bufc->buf = (char *) a_buf;
      t_bufc->buf_size = a_size;
      t_bufc->ref_count = 1;

      /* Disengage ourselves from the old bufc. */
      a_bufel->bufc->ref_count--;
#ifdef _CW_REENTRANT
      mtx_unlock(&a_bufel->bufc->lock);
#endif
      a_bufel->bufc = t_bufc;
    }
#ifdef _CW_REENTRANT
    else
    {
      /* We're the only one with a reference, so muck with the existing bufc. */
      _cw_free(a_bufel->bufc->buf);
      a_bufel->bufc->buf = (char *) a_buf;
      a_bufel->bufc->buf_size = a_size;
      
      mtx_unlock(&a_bufel->bufc->lock);
    }
#endif
  }

  a_bufel->beg_offset = 0;
  a_bufel->end_offset = a_size;
}

void
bufel_catenate_bufel(cw_bufel_t * a_a, cw_bufel_t * a_b)
{
  _cw_check_ptr(a_a);
  _cw_check_ptr(a_b);

  if (a_b->bufc == NULL)
  {
    /* Nothing to catenate. */
    goto RETURN;
  }
  else if (a_a->bufc == NULL)
  {
    /* Need a new bufc. */
    a_a->bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
    a_a->bufc->buf = (char *) _cw_malloc(a_b->end_offset - a_b->beg_offset);
    a_a->bufc->buf_size = a_b->end_offset - a_b->beg_offset;
#ifdef _CW_REENTRANT
    mtx_new(&a_a->bufc->lock);
#endif
    a_a->bufc->ref_count = 1;

    a_a->end_offset = a_a->bufc->buf_size;
    
    memcpy(a_a->bufc->buf,
	   a_b->bufc->buf + a_b->beg_offset,
	   a_a->bufc->buf_size);
  }
  else
  {
    cw_uint32_t new_size;

    if (a_a->end_offset + bufel_get_valid_data_size(a_b) > a_a->bufc->buf_size)
    {
      /* Need more space. */
      new_size = (a_a->bufc->buf_size + bufel_get_valid_data_size(a_b));
    }
    else
    {
      /* Current size is adequate. */
      new_size = a_a->bufc->buf_size;
    }
    
#ifdef _CW_REENTRANT
    mtx_lock(&a_a->bufc->lock);
#endif

    if (a_a->bufc->ref_count > 1)
    {
      cw_bufc_t * t_bufc;
      
      /* We need our own bufc. */
      t_bufc = (cw_bufc_t *) _cw_malloc(sizeof(cw_bufc_t));
      t_bufc->buf = (char *) _cw_malloc(new_size);
#ifdef _CW_REENTRANT
      mtx_new(&t_bufc->lock);
#endif
      t_bufc->buf_size = new_size;
      t_bufc->ref_count = 1;

      memcpy(t_bufc->buf + a_a->beg_offset,
	     a_a->bufc->buf + a_a->beg_offset,
	     a_a->end_offset - a_a->beg_offset);

      /* Extricate ourselves from the old bufc. */
      a_a->bufc->ref_count--;
#ifdef _CW_REENTRANT
      mtx_unlock(&a_a->bufc->lock);
#endif
      a_a->bufc = t_bufc;
    }
    else if (new_size != a_a->bufc->buf_size)
    {
      /* Need more space. */
      a_a->bufc->buf = (char *) _cw_realloc(a_a->bufc->buf, new_size);
      a_a->bufc->buf_size = new_size;

#ifdef _CW_REENTRANT
      mtx_unlock(&a_a->bufc->lock);
#endif
    }
#ifdef _CW_REENTRANT
    else
    {
      /* We have our own buffer, and it's already big enough. */
      mtx_unlock(&a_a->bufc->lock);
    }
#endif

    memcpy(a_a->bufc->buf + a_a->end_offset,
	   a_b->bufc->buf + a_b->beg_offset,
	   a_b->end_offset - a_b->beg_offset);
    a_a->end_offset += bufel_get_valid_data_size(a_b);
  }
  
  RETURN:
}
