/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Block repository implementation.
 *
 ****************************************************************************/

#define _INC_BR_H_
#include <config.h>

#include <br_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * br constructor.
 *
 ****************************************************************************/
cw_br_t *
br_new(cw_br_t * a_br_o, cw_bool_t a_is_thread_safe)
{
  cw_br_t * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_new()");
  }

  if (a_br_o == NULL)
  {
    retval = (cw_br_t *) _cw_malloc(sizeof(cw_br_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_br_o;
    retval->is_malloced = FALSE;
  }

  if (a_is_thread_safe)
  {
    retval->is_thread_safe = TRUE;
    rwl_new(&retval->rw_lock);
  }
  else
  {
    retval->is_thread_safe = FALSE;
  }
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_new()");
  }
  return NULL; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * br destructor.
 *
 ****************************************************************************/
void
br_delete(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_delete()");
  }
  _cw_check_ptr(a_br_o);

  if (a_br_o->is_thread_safe)
  {
    rwl_delete(&a_br_o->rw_lock);
  }

  if (a_br_o->is_malloced)
  {
    _cw_free(a_br_o);
  }
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_delete()");
  }
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == repository open.
 * FALSE == repository closed.
 *
 ****************************************************************************/
cw_bool_t
br_is_open(cw_br_t * a_br_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_is_open()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }

  retval = a_br_o->is_open;
  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_is_open()");
  }
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Opens the backing store for a br instance.  The main file in the
 * repository must be a plain file.
 *
 ****************************************************************************/
cw_bool_t
br_open(cw_br_t * a_br_o, char * a_filename)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_open()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_wlock(&a_br_o->rw_lock);
  }

  /* Open a_filename. */
  /* Get size of config space. */
  /* Parse config space (res format). */
  /* Create and initialize internal data structures. */
  
  
  if (a_br_o->is_thread_safe)
  {
    rwl_wunlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_open()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Closes the backing store for the repository.  All dirty buffers are
 * flushed.
 *
 ****************************************************************************/
cw_bool_t
br_close(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_close()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_wlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_wunlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_close()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * Size of block in bytes.
 *
 ****************************************************************************/
cw_uint64_t
br_get_block_size(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_block_size()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_block_size()");
  }
  return 0; /* XXX */
}
			    
/****************************************************************************
 * <<< Description >>>
 *
 * Adds a file or device as backing store for the block repository.
 *
 ****************************************************************************/
cw_bool_t
br_add_file(cw_br_t * a_br_o, char * a_filename,
	    cw_bool_t a_is_raw, cw_bool_t a_can_overlap,
	    cw_bool_t a_is_dynamic,
	    cw_uint64_t a_base_addr, cw_uint64_t a_max_size)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_add_file()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_wlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_wunlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_add_file()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Removes a file or device from the repository.  First though, all valid
 * blocks are moved off of the file or device.  If there is not enough
 * space to do so, the call will eventually fail.
 *
 ****************************************************************************/
cw_bool_t
br_rm_file(cw_br_t * a_br_o, char * a_filename)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_rm_file()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_wlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_wunlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_rm_file()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Allocates a block as used and points *a_brblk_o to it.
 *
 ****************************************************************************/
cw_bool_t
br_block_create(cw_br_t * a_br_o, cw_brblk_t ** a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * Permanently removes a block from the repository.  The physical block is
 * reclaimed for future use.
 *
 ****************************************************************************/
cw_bool_t
br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return TRUE; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * s-locks a block, as well as pulling it into cache.
 *
 ****************************************************************************/
cw_brblk_t *
br_block_slock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_slock()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_slock()");
  }
  return NULL; /* XXX */
}

/****************************************************************************
 * <<< Description >>>
 *
 * t-locks a block, as well as pulling it into cache.
 *
 ****************************************************************************/
cw_brblk_t *
br_block_tlock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_tlock()");
  }
  _cw_check_ptr(a_br_o);
  if (a_br_o->is_thread_safe)
  {
    rwl_rlock(&a_br_o->rw_lock);
  }
  

  
  if (a_br_o->is_thread_safe)
  {
    rwl_runlock(&a_br_o->rw_lock);
  }
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_tlock()");
  }
  return NULL; /* XXX */
}
