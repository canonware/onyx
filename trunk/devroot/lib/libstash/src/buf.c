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
 * $Revision: 151 $
 * $Date: 1998-07-29 16:53:57 -0700 (Wed, 29 Jul 1998) $
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
  _cw_check_ptr(a_buf_o);

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
    retval = a_spare;

    /* Delete the contents of retval's list. */
    while (list_count(&retval->bufels) > 0)
    {
      bufel_delete(list_hpop(&retval->bufels));
    }
  }
  else
  {
    retval = buf_new(NULL, a_buf_o->is_threadsafe);
  }
  
  /* Move bufels from a_buf_o to retval. */
  while (list_count(&a_buf_o->bufels) > 0)
  {
    list_tpush(&retval->bufels, list_hpop(&a_buf_o->bufels));
  }

  /* Copy over size and zero it for a_buf_o. */
  retval->size = a_buf_o->size;
  a_buf_o->size = 0;
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
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

  _cw_check_ptr(a_buf_o);
  _cw_check_ptr(a_other);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
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
buf_get_bufel(cw_buf_t * a_buf_o, cw_bufel_t * a_spare)
{
  cw_bufel_t * retval;

  _cw_check_ptr(a_buf_o);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
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

  _cw_check_ptr(a_buf_o);
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_lock(&a_buf_o->lock);
  }
  
  if (a_buf_o->is_threadsafe == TRUE)
  {
    mtx_unlock(&a_buf_o->lock);
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
  cw_uint32_t retval;

  return retval;
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
  cw_uint32_t retval;

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the begin pointer offset.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_beg_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_bool_t retval;

  return retval;
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
  cw_uint32_t retval;

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the end pointer offset.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_end_offset(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset)
{
  cw_bool_t retval;

  return retval;
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

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the uint8 at a_offset to a_val.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_uint8(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		cw_uint8_t a_val)
{
  cw_bool_t retval;

  return retval;
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

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Sets the uint32 at a_offset to a_val.
 *
 ****************************************************************************/
cw_bool_t
bufel_set_uint32(cw_bufel_t * a_bufel_o, cw_uint32_t a_offset,
		 cw_uint32_t a_val)
{
  cw_bool_t retval;

  return retval;
}
