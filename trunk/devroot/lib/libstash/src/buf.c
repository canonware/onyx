/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 173 $
 * $Date: 1998-08-26 12:34:42 -0700 (Wed, 26 Aug 1998) $
 *
 * <<< Description >>>
 *
 * Simple extensible (mostly FIFO) buffer class.  Although this may be
 * useful for other purposes, buf and bufel are designed to support
 * asynchronous buffering of I/O for the sock (socket) class.
 *
 ****************************************************************************/

#define _INC_BUF_H_
#include <libstash.h>

#include <sys/param.h>

/****************************************************************************
 * <<< Description >>>
 *
 * buf constructor.
 *
 ****************************************************************************/
cw_buf_t *
buf_new(cw_buf_t * a_buf_o, cw_bool_t a_is_threadsafe)
{
  cw_buf_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_new()");
  }
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

  retval->is_threadsafe = a_is_threadsafe;
  if (retval->is_threadsafe == TRUE)
  {
    mtx_new(&retval->lock);
  }
  list_new(&retval->bufels, FALSE);
  retval->size = 0;
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_new()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * buf destructor.
 *
 ****************************************************************************/
void
buf_delete(cw_buf_t * a_buf_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_delete()");
  }
  _cw_check_ptr(a_buf_o);

  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_delete(&a_buf_o->lock);
  }

  while (list_count(&a_buf_o->bufels) > 0)
  {
    bufel_delete(list_hpop(&a_buf_o->bufels));
  }
  list_delete(&a_buf_o->bufels);
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_delete()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the amount of valid data in bytes.
 *
 ****************************************************************************/
cw_uint32_t
buf_get_size(cw_buf_t * a_buf_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_get_size()");
  }
  _cw_check_ptr(a_buf_o);

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_get_size()");
  }
  return a_buf_o->size;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns a buf that has had all of the valid data that was in a_buf_o
 * moved to it.  If a_spare is not NULL, it is used instead of malloc()ing
 * a new buf.
 *
 ****************************************************************************/
cw_buf_t *
buf_get_buf(cw_buf_t * a_buf_o, cw_buf_t * a_spare)
{
  cw_buf_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_get_buf()");
  }
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_spare);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }

  /* If we're using a_spare, assume that no one else is going to muck
   * around in a_spare while we're working here. */

  if (a_spare != NULL)
  {
    cw_uint64_t i, count;

    retval = a_spare;

    /* Delete the contents of retval's list. */
    for (i = 0, count = list_count(&retval->bufels);
	 i < count;
	 i++)
    {
      bufel_delete(list_hpop(&retval->bufels));
    }
  }
  else
  {
    retval = buf_new(NULL, a_buf_o->is_threadsafe);
  }
  
  /* Move bufels from a_buf_o to retval. */
  {
    cw_uint64_t i, count;

    for (i = 0, count = list_count(&a_buf_o->bufels);
	 i < count;
	 i++)
    {
      list_tpush(&retval->bufels, list_hpop(&a_buf_o->bufels));
    }
  }

  /* Copy over size and zero it for a_buf_o. */
  retval->size = a_buf_o->size;
  a_buf_o->size = 0;
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_get_buf()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Concatenates two bufs.  After this function call, a_other is empty, but
 * it still exists.
 *
 ****************************************************************************/
cw_bool_t
buf_put_buf(cw_buf_t * a_buf_o, cw_buf_t * a_other)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_put_buf()");
  }
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_other);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }

  /* Assume that we don't need to lock a_other. */
  {
    cw_uint64_t i, count;

    for (i = 0, count = list_count(&a_other->bufels);
	 i < count;
	 i++)
    {
      list_tpush(&a_buf_o->bufels, list_hpop(&a_other->bufels));
    }
  }

  a_buf_o->size += a_other->size;
  a_other->size = 0;
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_put_buf()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Removes the first bufel from a_buf_o and returns a pointer to it.
 *
 ****************************************************************************/
cw_bufel_t *
buf_get_bufel(cw_buf_t * a_buf_o)
{
  cw_bufel_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_get_bufel()");
  }
  _cw_check_ptr(a_buf_o);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }

  if (a_buf_o->size > 0) /* Make sure there is valid data. */
  {
    retval = (cw_bufel_t *) list_hpop(&a_buf_o->bufels);
    a_buf_o->size -= (retval->end_offset - retval->beg_offset);
  }
  else
  {
    retval = NULL;
  }
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_get_bufel()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Appends a bufel to a_buf_o.
 *
 ****************************************************************************/
cw_bool_t
buf_put_bufel(cw_buf_t * a_buf_o, cw_bufel_t * a_bufel_o)
{
  cw_bool_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter buf_put_bufel()");
  }
  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_bufel_o);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }

  list_tpush(&a_buf_o->bufels, a_bufel_o);
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
  }
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit buf_put_bufel()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * bufel constructor.
 *
 ****************************************************************************/
cw_bufel_t *
bufel_new(cw_bufel_t * a_bufel_o)
{
  cw_bufel_t * retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_new()");
  }
  if (a_bufel_o == NULL)
  {
    retval = (cw_bufel_t *) _cw_malloc(sizeof(a_bufel_o));
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
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_new()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * bufel destructor.
 *
 ****************************************************************************/
void
bufel_delete(cw_bufel_t * a_bufel_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_delete()");
  }
  _cw_check_ptr(a_bufel_o);

  if (a_bufel_o->buf != NULL)
  {
    _cw_free(a_bufel_o->buf);
  }
  if (a_bufel_o->is_malloced == TRUE)
  {
    _cw_free(a_bufel_o);
  }
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_delete()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns total size of a_bufel_o's internal buffer.  This is _not_
 * necessarily the same as the amount of valid data.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_size(cw_bufel_t * a_bufel_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_get_size()");
  }
  _cw_check_ptr(a_bufel_o);
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_get_size()");
  }
  return a_bufel_o->buf_size;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Either malloc()s or realloc()s the internal buffer to be of size a_size.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_size(cw_bufel_t * a_bufel_o, cw_uint32_t a_size)
{
  cw_bool_t retval;
  cw_uint32_t * t_buf;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_set_size()");
  }
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

    if (t_buf == NULL)
    {
      retval = TRUE;
    }
    else
    {
      a_bufel_o->buf = t_buf;
      a_bufel_o->buf_size = a_size;
      retval = FALSE;
    }
  }
  else
  {
    /* Allocate for the first time. */
    a_bufel_o->buf = (cw_uint32_t *) _cw_malloc(a_size);

    if (a_bufel_o->buf == NULL)
    {
      retval = TRUE;
    }
    else
    {
      a_bufel_o->buf_size = a_size;
      retval = FALSE;
    }
  }
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_set_size()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the offset to the begin pointer.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_beg_offset(cw_bufel_t * a_bufel_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_get_beg_offset()");
  }
  _cw_check_ptr(a_bufel_o);

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_get_beg_offset()");
  }
  return a_bufel_o->beg_offset;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the begin pointer offset.
 *
 ****************************************************************************/
void
bufel_set_beg_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_set_beg_offset()");
  }
  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset <= a_bufel_o->end_offset);

  a_bufel_o->beg_offset = a_offset;
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_set_beg_offset()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the offset to the end pointer.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_end_offset(cw_bufel_t * a_bufel_o)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_get_end_offset()");
  }
  _cw_check_ptr(a_bufel_o);

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_get_end_offset()");
  }
  return a_bufel_o->end_offset;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the end pointer offset.
 *
 ****************************************************************************/
void
bufel_set_end_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_set_end_offset()");
  }
  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset >= a_bufel_o->beg_offset);
  _cw_assert(a_offset < a_bufel_o->buf_size);

  a_bufel_o->end_offset = a_offset;
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_set_end_offset()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the uint8 at offset a_offset.
 *
 ****************************************************************************/
cw_uint8_t
bufel_get_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_uint8_t retval;
  cw_uint32_t t;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_get_uint8()");
  }
  _cw_check_ptr(a_bufel_o);
  _cw_assert(a_offset < a_bufel_o->buf_size);
  /* XXX This should be a compile-time check. */
  _cw_assert(sizeof(u_long) == sizeof(cw_uint32_t));

  /* XXX Does this make endianness assumptions? */
  t = a_bufel_o->buf[a_offset >> 2];
  t = htonl(t);
  t >>= (8 * (a_offset & 0x3));
  t &= 0xff;
  retval = t;
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_get_uint8()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
void
bufel_set_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		cw_uint8_t a_val)
{
  cw_uint32_t t_a, t_b, mask;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_set_uint8()");
  }
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
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_set_uint8()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the uint32 at offset a_offset.
 *
 ****************************************************************************/
cw_uint32_t
bufel_get_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_uint32_t retval;

  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_get_uint32()");
  }
  _cw_check_ptr(a_bufel_o);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel_o->buf_size);

  retval = a_bufel_o->buf[a_offset];
  retval = htonl(retval);
  
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_get_uint32()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
void
bufel_set_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		 cw_uint32_t a_val)
{
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Enter bufel_set_uint32()");
  }
  _cw_check_ptr(a_bufel_o);
  _cw_assert((a_offset & 0x3) == 0);
  _cw_assert (a_offset < a_bufel_o->buf_size);

  a_bufel_o->buf[a_offset] = ntohl(a_val);
  if (_cw_pmatch(_STASH_DBG_R_BUF_FUNC))
  {
    _cw_marker("Exit bufel_set_uint32()");
  }
}
