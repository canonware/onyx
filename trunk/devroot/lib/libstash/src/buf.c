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

#define _INC_BUF_H_
#ifdef _CW_REENTRANT
#  include "libstash_r.h"
#else
#  include "libstash.h"
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
buf_new(cw_buf_t * a_buf_o, cw_bool_t a_is_threadsafe)
#else
buf_new(cw_buf_t * a_buf_o)
#endif
{
  cw_buf_t * retval;

  if (a_buf_o == NULL)
  {
    retval = (cw_buf_t *) _cw_malloc(sizeof(cw_buf_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_buf_o;
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
buf_delete(cw_buf_t * a_buf_o)
{
  _cw_check_ptr(a_buf_o);

#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_delete(&a_buf_o->lock);
  }
#endif

  while (list_count(&a_buf_o->bufels) > 0)
  {
    bufel_delete(list_hpop(&a_buf_o->bufels));
  }
  list_delete(&a_buf_o->bufels);
}

/****************************************************************************
 *
 * Returns the amount of valid data in bytes.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_size(cw_buf_t * a_buf_o)
{
  _cw_check_ptr(a_buf_o);
  return a_buf_o->size;
}

/****************************************************************************
 *
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
void
buf_prepend_buf(cw_buf_t * a_buf_o, cw_buf_t * a_other)
{
  cw_uint64_t i, count;
  
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_other);
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
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
    list_hpush(&a_buf_o->bufels, list_tpop(&a_other->bufels));
  }

  a_buf_o->size += a_other->size;
  a_other->size = 0;
  
#ifdef _CW_REENTRANT
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_other->lock);
  }
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
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
buf_append_buf(cw_buf_t * a_buf_o, cw_buf_t * a_other)
{
  cw_uint64_t i, count;
  
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_other);
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
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
    list_tpush(&a_buf_o->bufels, list_hpop(&a_other->bufels));
  }

  a_buf_o->size += a_other->size;
  a_other->size = 0;
  
#ifdef _CW_REENTRANT
  if (a_other->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_other->lock);
  }
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
#endif
}

/****************************************************************************
 *
 * Removes the first bufel from a_buf_o and returns a pointer to it.
 *
 ****************************************************************************/
cw_bufel_t *
buf_rm_head_bufel(cw_buf_t * a_buf_o)
{
  cw_bufel_t * retval;

  _cw_check_ptr(a_buf_o);
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }
#endif

  if (a_buf_o->size > 0) /* Make sure there is valid data. */
  {
    retval = (cw_bufel_t *) list_hpop(&a_buf_o->bufels);
    a_buf_o->size -= (retval->end_offset - retval->beg_offset);
  }
  else
  {
    retval = NULL;
  }
  
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
#endif
  return retval;
}

/****************************************************************************
 *
 * Appends a bufel to a_buf_o.
 *
 ****************************************************************************/
void
buf_append_bufel(cw_buf_t * a_buf_o, cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_bufel_o);
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }
#endif

  list_tpush(&a_buf_o->bufels, a_bufel_o);
  a_buf_o->size += (a_bufel_o->end_offset - a_bufel_o->beg_offset);
  
#ifdef _CW_REENTRANT
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
#endif
}

/****************************************************************************
 *
 * bufel constructor.
 *
 ****************************************************************************/
cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel_o)
{
  cw_bufel_t * retval;

  if (a_bufel_o == NULL)
  {
    retval = (cw_bufel_t *) _cw_malloc(sizeof(cw_bufel_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_bufel_o;
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
bufel_delete(cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_bufel_o);

  if (a_bufel_o->buf != NULL)
  {
    _cw_free(a_bufel_o->buf);
  }
  if (a_bufel_o->is_malloced == TRUE)
  {
    _cw_free(a_bufel_o);
  }
}

/****************************************************************************
 *
 * Returns total size of a_bufel_o's internal buffer.  This is _not_
 * necessarily the same as the amount of valid data.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_bufel_o);
  return a_bufel_o->buf_size;
}

/****************************************************************************
 *
 * Either malloc()s or realloc()s the internal buffer to be of size a_size.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_size(cw_bufel_t * a_bufel_o, cw_uint32_t a_size)
{
  cw_bool_t retval;
  cw_uint32_t * t_buf;

  _cw_check_ptr(a_bufel_o);
  _cw_assert((a_size & 0x3) == 0);

  if (a_size <= a_bufel_o->end_offset)
  {
    /* We would chop off valid data if we did this. */
    retval = TRUE;
  }
  else if (a_bufel_o->buf != NULL)
  {
    /* Reallocate. */
    t_buf = (cw_uint32_t *) _cw_realloc(a_bufel_o->buf, a_size);

    a_bufel_o->buf = t_buf;
    a_bufel_o->buf_size = a_size;
    retval = FALSE;
  }
  else
  {
    /* Allocate for the first time. */
    a_bufel_o->buf = (cw_uint32_t *) _cw_malloc(a_size);

    a_bufel_o->buf_size = a_size;
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
bufel_get_beg_offset(cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_bufel_o);
  return a_bufel_o->beg_offset;
}

/****************************************************************************
 *
 * Sets the begin pointer offset.
 *
 ****************************************************************************/
void
bufel_set_beg_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset <= a_bufel_o->end_offset);

  a_bufel_o->beg_offset = a_offset;
}

/****************************************************************************
 *
 * Returns the offset to the end pointer.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_bufel_o);
  return a_bufel_o->end_offset;
}

/****************************************************************************
 *
 * Sets the end pointer offset.
 *
 ****************************************************************************/
void
bufel_set_end_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset >= a_bufel_o->beg_offset);
  _cw_assert(a_offset <= a_bufel_o->buf_size);

  a_bufel_o->end_offset = a_offset;
}

/****************************************************************************
 *
 * Returns a pointer to the internal buffer.
 *
 ****************************************************************************/
void *
bufel_get_data_ptr(cw_bufel_t * a_bufel_o)
{
  _cw_check_ptr(a_bufel_o);

  return (void *) a_bufel_o->buf;
}

/****************************************************************************
 *
 * Returns the uint8 at offset a_offset.
 *
 ****************************************************************************/
cw_uint8_t
bufel_get_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_uint8_t retval;
  cw_uint32_t t;

  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset < a_bufel_o->buf_size);

  t = a_bufel_o->buf[a_offset >> 2];
  t = ntohl(t);
  t >>= (8 * (a_offset & 0x3));
  t &= 0xff;
  retval = t;
  
  return retval;
}

/****************************************************************************
 *
 * Sets the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
void
bufel_set_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		cw_uint8_t a_val)
{
  cw_uint32_t t_a, t_b, mask;

  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset < a_bufel_o->buf_size);

  t_a = a_bufel_o->buf[a_offset >> 2];
  t_a = htonl(t_a);

  mask = 0xff << (8 * (a_offset & 0x3));
  mask ^= 0xffffffff;

  t_a &= mask;

  t_b = a_val;
  t_b <<= (8 * (a_offset & 0x3));

  t_a |= t_b;
  t_a = ntohl(t_a);
  a_bufel_o->buf[a_offset >> 2] = t_a;
}

/****************************************************************************
 *
 * Returns the uint32 at offset a_offset.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_uint32_t retval;

  _cw_check_ptr(a_bufel_o);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel_o->buf_size);

  retval = a_bufel_o->buf[a_offset >> 2];
  retval = htonl(retval);
  
  return retval;
}

/****************************************************************************
 *
 * Sets the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
void
bufel_set_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		 cw_uint32_t a_val)
{
  _cw_check_ptr(a_bufel_o);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel_o->buf_size);

  a_bufel_o->buf[a_offset >> 2] = ntohl(a_val);
}
