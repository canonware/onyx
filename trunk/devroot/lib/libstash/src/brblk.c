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
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_BRBLK_H_
#include <config.h>

#include <brblk_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_brblk_t *
brblk_new(cw_brblk_t * a_brblk_o, cw_uint32_t a_block_size)
{
  cw_brblk_t * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_new()");
  }

  if (a_brblk_o == NULL)
  {
    retval = (cw_brblk_t *) _cw_malloc(sizeof(cw_brblk_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_brblk_o;
    retval->is_malloced = FALSE;
  }

  jtl_new(&retval->jt_lock);
  retval->is_dirty = FALSE;
  retval->logical_addr = 0;
  retval->buf = (cw_uint8_t *) _cw_malloc(a_block_size);
  retval->buf_size = a_block_size;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_new()");
  }
  return NULL; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
brblk_delete(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_delete()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_delete(&a_brblk_o->jt_lock);
  _cw_free(a_brblk_o->buf);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_delete()");
  }
}

#ifdef _CW_DEBUG
/****************************************************************************
 * <<< Description >>>
 *
 * Grab an slock on the block.
 *
 ****************************************************************************/
void
brblk_slock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_slock()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_slock(&a_brblk_o->jt_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_slock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Grab a tlock on the block.
 *
 ****************************************************************************/
void
brblk_tlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_tlock()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_tlock(&a_brblk_o->jt_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_tlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return whether the buffer is dirty.
 *
 ****************************************************************************/
cw_bool_t
brblk_get_is_dirty(cw_brblk_t * a_brblk_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_is_dirty()");
  }
  _cw_check_ptr(a_brblk_o);

  retval = a_brblk_o->is_dirty;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_is_dirty()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Return whether the buffer is dirty.
 *
 ****************************************************************************/
void
brblk_set_is_dirty(cw_brblk_t * a_brblk_o, cw_bool_t a_is_dirty)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_is_dirty()");
  }
  _cw_check_ptr(a_brblk_o);

  a_brblk_o->is_dirty = a_is_dirty;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_is_dirty()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Convert an slock to a dlock on the block.
 *
 ****************************************************************************/
void
brblk_s2dlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_s2dlock()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_s2dlock(&a_brblk_o->jt_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_s2dlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Convert a[n]* [st]lock to an rlock.
 *
 ****************************************************************************/
void
brblk_2rlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_2rlock()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_2rlock(&a_brblk_o->jt_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2rlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Convert a[n]* [st]lock to a wlock.
 *
 ****************************************************************************/
void
brblk_2wlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_2wlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_2wlock(&a_brblk_o->jt_lock);
  a_brblk_o->is_dirty = TRUE;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2wlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Convert a[n]* [st]lock to an xlock.
 *
 ****************************************************************************/
void
brblk_2xlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_2xlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_2xlock(&a_brblk_o->jt_lock);
  a_brblk_o->is_dirty = TRUE;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2xlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release an slock.
 *
 ****************************************************************************/
void
brblk_sunlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_sunlock()");
  }
  _cw_check_ptr(a_brblk_o);

  jtl_sunlock(&a_brblk_o->jt_lock);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_sunlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release a tlock.
 *
 ****************************************************************************/
void
brblk_tunlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_tunlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_tunlock(&a_brblk_o->jt_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_tunlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release a dlock.
 *
 ****************************************************************************/
void
brblk_dunlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_dunlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_dunlock(&a_brblk_o->jt_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_dunlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release an rlock.
 *
 ****************************************************************************/
void
brblk_runlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_runlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_runlock(&a_brblk_o->jt_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_runlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release a wlock.
 *
 ****************************************************************************/
void
brblk_wunlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_wunlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_wunlock(&a_brblk_o->jt_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_wunlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Release an xlock.
 *
 ****************************************************************************/
void
brblk_xunlock(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_xunlock()");
  }
  _cw_check_ptr(a_brblk_o);
  
  jtl_xunlock(&a_brblk_o->jt_lock);

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_xunlock()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Read a byte from the buffer.
 *
 ****************************************************************************/
cw_uint8_t
brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset)
{
  cw_uint8_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_byte()");
  }
  _cw_check_ptr(a_brblk_o);

  _cw_assert(a_offset < a_brblk_o->buf_size);
  
  retval = a_brblk_o->buf[a_offset];
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_byte()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Write a byte to the buffer.
 *
 ****************************************************************************/
void
brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
	       cw_uint8_t a_byte)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_set_byte()");
  }
  _cw_check_ptr(a_brblk_o);

  _cw_assert(a_offset < a_brblk_o->buf_size);
  
  a_brblk_o->buf[a_offset] = a_byte;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_set_byte()");
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Get a pointer to the buffer.
 *
 ****************************************************************************/
cw_uint8_t *
brblk_get_buf_p(cw_brblk_t * a_brblk_o)
{
  cw_uint8_t * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_buf_p()");
  }
  _cw_check_ptr(a_brblk_o);

  retval = a_brblk_o->buf;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_buf_p()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Get the buffer size
 *
 ****************************************************************************/
cw_uint32_t
brblk_get_buf_size(cw_brblk_t * a_brblk_o)
{
  cw_uint32_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_buf_size()");
  }
  _cw_check_ptr(a_brblk_o);

  retval = a_brblk_o->buf_size;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_buf_size()");
  }
  return retval;
}
#endif /* _CW_DEBUG */
