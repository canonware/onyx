/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 96 $
 * $Date: 1998-06-26 23:48:21 -0700 (Fri, 26 Jun 1998) $
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
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_brblk_t *
brblk_new(cw_brblk_t * a_brblk_o, cw_uint32_t a_block_size)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_new()");
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_new()");
  }
  return NULL; /* XXX */
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
brblk_delete(cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_delete()");
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_delete()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_slock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_tlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);

  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_s2dlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2rlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2wlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_2xlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_sunlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_tunlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_dunlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_runlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_wunlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_xunlock()");
  }
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
	       cw_uint8_t * a_byte)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_get_byte()");
  }
  _cw_check_ptr(a_brblk_o);
  rwl_rlock(&a_brblk_o->rw_lock);
  
  rwl_runlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_byte()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_bool_t
brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
	       cw_uint8_t a_byte)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_set_byte()");
  }
  _cw_check_ptr(a_brblk_o);
  rwl_wlock(&a_brblk_o->rw_lock);
  
  rwl_wunlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_set_byte()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_rlock(&a_brblk_o->rw_lock);

  retval = a_brblk_o->buf;
  
  rwl_runlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_buf_p()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
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
  rwl_rlock(&a_brblk_o->rw_lock);

  retval = a_brblk_o->is_dirty;
  
  rwl_runlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_get_is_dirty()");
  }
  return retval;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
brblk_set_is_dirty(cw_brblk_t * a_brblk_o, cw_bool_t a_is_dirty)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Enter brblk_set_is_dirty()");
  }
  _cw_check_ptr(a_brblk_o);
  rwl_rlock(&a_brblk_o->rw_lock);

  retval = a_brblk_o->is_dirty;
  
  rwl_runlock(&a_brblk_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBLK_FUNC))
  {
    _cw_marker("Exit brblk_set_is_dirty()");
  }
}
