/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 92 $
 * $Date: 1998-06-26 01:34:11 -0700 (Fri, 26 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_BRBS_H_
#include <config.h>

#include <brbs_priv.h>

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
cw_brbs_t * 
brbs_new(cw_brbs_t * a_brbs_o)
{
  cw_brbs_t * retval;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_new()");
  }
  if (a_brbs_o == NULL)
  {
    retval = (cw_brbs_t *) _cw_malloc(sizeof(cw_brbs_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_brbs_o;
    retval->is_malloced = FALSE;
  }

  rwl_new(&retval->rw_lock);
  retval->is_open = FALSE;
  retval->filename = NULL;
  retval->is_raw = FALSE;
  retval->is_dynamic = FALSE;
  retval->max_size = NULL;

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_new()");
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
brbs_delete(cw_brbs_t * a_brbs_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_delete()");
  }
  _cw_check_ptr(a_brbs_o);

  if (a_brbs_o->is_open)
  {
    if (brbs_close(a_brbs_o))
    {
      log_leprintf(g_log_o, __FILE__, __LINE__, "brbs_delete",
		   "Error in brbs_close()\n");
    }
  }
  
  rwl_delete(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_malloced == TRUE)
  {
    _cw_free(a_brbs_o);
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_delete()");
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
brbs_is_open(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_is_open()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  retval = a_brbs_o->is_open;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_is_open()");
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
brbs_open(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_open()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_open)
  {
    retval = TRUE;
  }
  else
  {
    /* Figure out whether this is a normal file, or a raw device. */
    
  }
  

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_open()");
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
brbs_close(cw_brbs_t * a_brbs_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_close()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);



  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_close()");
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
char * 
brbs_get_filename(cw_brbs_t * a_brbs_o)
{
  char * retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_filename()");
  }
  _cw_check_ptr(a_brbs_o);
/*   rwl_rlock(&a_brbs_o->rw_lock); */

  /* Note that the value can change at any time! */
  retval = a_brbs_o->filename;

/*   rwl_runlock(&a_brbs_o->rw_lock); */
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_filename()");
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
brbs_set_filename(cw_brbs_t * a_brbs_o, char * a_filename)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_filename()");
  }
  _cw_check_ptr(a_brbs_o);
  _cw_check_ptr(a_filename);
  rwl_wlock(&a_brbs_o->rw_lock);

  if (a_brbs_o->is_open == TRUE)
  {
    retval = TRUE;
  }
  else
  {
    retval = FALSE;
    a_brbs_o->filename = (char *) _cw_realloc(a_brbs_o->filename,
					      strlen(a_filename) + 1);
    strcpy(a_brbs_o->filename, a_filename);
  }

  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_filename()");
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
brbs_get_is_dynamic(cw_brbs_t * a_brbs_o)
{
  cw_bool_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_is_dynamic()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);

  retval = a_brbs_o->is_dynamic;

  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_is_dynamic()");
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
brbs_set_is_dynamic(cw_brbs_t * a_brbs_o, cw_bool_t a_is_dynamic)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_is_dynamic()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);



  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_is_dynamic()");
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
cw_uint64_t 
brbs_get_max_size(cw_brbs_t * a_brbs_o)
{
  cw_uint64_t retval;
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_get_max_size()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);

  retval = a_brbs_o->max_size;

  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_get_max_size()");
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
brbs_set_max_size(cw_brbs_t * a_brbs_o, cw_uint64_t a_max_size)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_set_max_size()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);



  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_set_max_size()");
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
brbs_block_read(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		cw_brblk_t ** a_block)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_read()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_rlock(&a_brbs_o->rw_lock);



  rwl_runlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_read()");
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
brbs_block_write(cw_brbs_t * a_brbs_o, cw_uint64_t a_offset,
		 cw_brblk_t * a_block)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Enter brbs_block_write()");
  }
  _cw_check_ptr(a_brbs_o);
  rwl_wlock(&a_brbs_o->rw_lock);



  rwl_wunlock(&a_brbs_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BRBS_FUNC))
  {
    _cw_marker("Exit brbs_block_write()");
  }
}
