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
  list_new(&retval->bufels, FALSE);
#else
  list_new(&retval->bufels);
#endif

  retval->size = 0;
  
  return retval;
}

void
buf_delete(cw_buf_t * a_buf)
{
  _cw_check_ptr(a_buf);

#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_delete(&a_buf->lock);
  }
#endif

  while (list_count(&a_buf->bufels) > 0)
  {
    bufel_delete(list_hpop(&a_buf->bufels));
  }
  list_delete(&a_buf->bufels);
}

cw_uint32_t
buf_get_size(cw_buf_t * a_buf)
{
  _cw_check_ptr(a_buf);
  return a_buf->size;
}

void
buf_prepend_buf(cw_buf_t * a_buf, cw_buf_t * a_other)
{
  cw_uint64_t i, count;
  
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_other);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_lock(&a_other->lock);
  }
#endif

  for (i = 0, count = list_count(&a_other->bufels);
       i < count;
       i++)
  {
    list_hpush(&a_buf->bufels, list_tpop(&a_other->bufels));
  }

  a_buf->size += a_other->size;
  a_other->size = 0;
  
#ifdef _CW_REENTRANT
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_other->lock);
  }
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

void
buf_append_buf(cw_buf_t * a_buf, cw_buf_t * a_other)
{
  cw_uint64_t i, count;
  
  _cw_check_ptr(a_buf);
  _cw_check_ptr(a_other);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_lock(&a_other->lock);
  }
#endif

  for (i = 0, count = list_count(&a_other->bufels);
       i < count;
       i++)
  {
    list_tpush(&a_buf->bufels, list_hpop(&a_other->bufels));
  }

  a_buf->size += a_other->size;
  a_other->size = 0;
  
#ifdef _CW_REENTRANT
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_other->lock);
  }
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

cw_bufel_t *
buf_rm_head_bufel(cw_buf_t * a_buf)
{
  cw_bufel_t * retval;

  _cw_check_ptr(a_buf);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (a_buf->size > 0) /* Make sure there is valid data. */
  {
    retval = (cw_bufel_t *) list_hpop(&a_buf->bufels);
    a_buf->size -= bufel_get_valid_data_size(retval);
    _cw_assert((cw_sint32_t) a_buf->size >= 0);
  }
  else
  {
    retval = NULL;
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
}

cw_bufel_t *
buf_rm_tail_bufel(cw_buf_t * a_buf)
{
  cw_bufel_t * retval;

  _cw_check_ptr(a_buf);
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf->lock);
  }
#endif

  if (a_buf->size > 0) /* Make sure there is valid data. */
  {
    retval = (cw_bufel_t *) list_tpop(&a_buf->bufels);
    a_buf->size -= bufel_get_valid_data_size(retval);
    _cw_assert((cw_sint32_t) a_buf->size >= 0);
  }
  else
  {
    retval = NULL;
  }
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
  return retval;
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

  list_hpush(&a_buf->bufels, a_bufel);
  a_buf->size += bufel_get_valid_data_size(a_bufel);
  
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

  list_tpush(&a_buf->bufels, a_bufel);
  a_buf->size += bufel_get_valid_data_size(a_bufel);
  
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
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bufel;
    retval->is_malloced = FALSE;
  }

  retval->buf_size = 0;
  retval->beg_offset = 0;
  retval->end_offset = 0;
  retval->buf = NULL;
  
  return retval;
}

void
bufel_delete(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);

  if (a_bufel->buf != NULL)
  {
    _cw_free(a_bufel->buf);
  }
  if (a_bufel->is_malloced == TRUE)
  {
    _cw_free(a_bufel);
  }
}

cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->buf_size;
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
    /* Release the memory for the buffer, if any is used. */
    if (a_bufel->buf != NULL)
    {
      _cw_free(a_bufel->buf);
    }
    a_bufel->buf = NULL;
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
bufel_append_bufel(cw_bufel_t * a_bufel, cw_bufel_t * a_other_bufel)
{
  _cw_check_ptr(a_bufel);
  _cw_check_ptr(a_other_bufel);

  if (a_bufel->end_offset + bufel_get_valid_data_size(a_other_bufel)
      > a_bufel->buf_size)
  {
    /* Need more space. */
    a_bufel->buf
      = (cw_uint32_t *) _cw_realloc(a_bufel->buf,
				    a_bufel->buf_size
				    + bufel_get_valid_data_size(a_other_bufel));

    memcpy(((cw_uint8_t *) a_bufel->buf) + a_bufel->end_offset,
	   ((cw_uint8_t *) a_other_bufel->buf) + a_other_bufel->beg_offset,
	   bufel_get_valid_data_size(a_other_bufel));

    a_bufel->buf_size += bufel_get_valid_data_size(a_other_bufel);
    a_bufel->end_offset += bufel_get_valid_data_size(a_other_bufel);
  }
  else
  {
    memcpy(((cw_uint8_t *) a_bufel->buf) + a_bufel->end_offset,
	   ((cw_uint8_t *) a_other_bufel->buf) + a_other_bufel->beg_offset,
	   bufel_get_valid_data_size(a_other_bufel));
    
    a_bufel->end_offset += bufel_get_valid_data_size(a_other_bufel);
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
