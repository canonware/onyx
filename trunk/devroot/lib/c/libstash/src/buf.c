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
 * <<< Description >>>
 *
 * Simple extensible (mostly FIFO) buffer class.  Although this may be
 * useful for other purposes, buf and bufel are designed to support
 * asynchronous buffering of I/O for the sock (socket) class.
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

/****************************************************************************
 *
 * buf constructor.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * buf destructor.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Returns the amount of valid data in bytes.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_size(cw_buf_t * a_buf)
{
  _cw_check_ptr(a_buf);
  return a_buf->size;
}

/****************************************************************************
 *
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Removes the first bufel from a_buf and returns a pointer to it.
 *
 ****************************************************************************/
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
    a_buf->size -= (retval->end_offset - retval->beg_offset);
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

/****************************************************************************
 *
 * Prepends a bufel to a_buf.
 *
 ****************************************************************************/
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
  a_buf->size += (a_bufel->end_offset - a_bufel->beg_offset);
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

/****************************************************************************
 *
 * Appends a bufel to a_buf.
 *
 ****************************************************************************/
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
  a_buf->size += (a_bufel->end_offset - a_bufel->beg_offset);
  
#ifdef _CW_REENTRANT
  if (a_buf->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf->lock);
  }
#endif
}

/****************************************************************************
 *
 * bufel constructor.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * bufel destructor.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Returns total size of a_bufel's internal buffer.  This is _not_
 * necessarily the same as the amount of valid data.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->buf_size;
}

/****************************************************************************
 *
 * Either malloc()s or realloc()s the internal buffer to be of size a_size.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_size(cw_bufel_t * a_bufel, cw_uint32_t a_size)
{
  cw_bool_t retval;
  cw_uint32_t * t_buf;

  _cw_check_ptr(a_bufel);
  _cw_assert((a_size & 0x3) == 0);

  if (a_size <= a_bufel->end_offset)
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
    /* XXX These probably don't need set. */
    a_bufel->beg_offset = 0;
    a_bufel->end_offset = 0;

    retval = FALSE;
  }
  else if (a_bufel->buf != NULL)
  {
    /* Reallocate. */
    t_buf = (cw_uint32_t *) _cw_realloc(a_bufel->buf, a_size);

    a_bufel->buf = t_buf;
    a_bufel->buf_size = a_size;
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

/****************************************************************************
 *
 * Returns the offset to the begin pointer.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->beg_offset;
}

/****************************************************************************
 *
 * Sets the begin pointer offset.
 *
 ****************************************************************************/
void
bufel_set_beg_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  _cw_check_ptr(a_bufel);
  _cw_assert(a_offset <= a_bufel->end_offset);

  a_bufel->beg_offset = a_offset;
}

/****************************************************************************
 *
 * Returns the offset to the end pointer.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);
  return a_bufel->end_offset;
}

/****************************************************************************
 *
 * Sets the end pointer offset.
 *
 ****************************************************************************/
void
bufel_set_end_offset(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  _cw_check_ptr(a_bufel);
  _cw_assert(a_offset >= a_bufel->beg_offset);
  _cw_assert(a_offset <= a_bufel->buf_size);

  a_bufel->end_offset = a_offset;
}

/****************************************************************************
 *
 * Returns the number of bytes between the beginning offset marker and the end
 * offset marker.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_valid_data_size(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);

  return a_bufel->end_offset - a_bufel->beg_offset;
}

/****************************************************************************
 *
 * Returns a pointer to the internal buffer.
 *
 ****************************************************************************/
void *
bufel_get_data_ptr(cw_bufel_t * a_bufel)
{
  _cw_check_ptr(a_bufel);

  return (void *) a_bufel->buf;
}

/****************************************************************************
 *
 * Sets the internal buffer to a_buf, assumes the a_buf is a_size bytes long,
 * and sets the beginning offset to 0 and the end offset to a_size.  If there is
 * already an internal buffer, the old one is freed.  Note that a_buf _must_ be
 * allocated on the heap, since the bufel class will eventually free it.
 *
 ****************************************************************************/
void
bufel_set_data_ptr(cw_bufel_t * a_bufel, void * a_buf, cw_uint32_t a_size)
{
  _cw_check_ptr(a_bufel);
  _cw_check_ptr(a_buf);

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

/****************************************************************************
 *
 * Returns the uint8 at offset a_offset.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Sets the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
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

/****************************************************************************
 *
 * Returns the uint32 at offset a_offset.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_bufel);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel->buf_size);

  retval = a_bufel->buf[a_offset >> 2];
#ifndef WORDS_BIGENDIAN
  retval = htonl(retval);
#endif
  
  return retval;
}

/****************************************************************************
 *
 * Sets the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
void
bufel_set_uint32(cw_bufel_t * a_bufel, cw_uint32_t a_offset,
		 cw_uint32_t a_val)
{
  _cw_check_ptr(a_bufel);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel->buf_size);

#ifdef WORDS_BIGENDIAN
  a_bufel->buf[a_offset >> 2] = a_val;
#else
  a_bufel->buf[a_offset >> 2] = ntohl(a_val);
#endif
}
