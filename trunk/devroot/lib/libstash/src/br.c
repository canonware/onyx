/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1998
 * Jason Evans <jasone@canonware.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JASON EVANS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL JASON EVANS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 81 $
 * $Date: 1998-05-18 23:41:27 -0700 (Mon, 18 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_BR_H_
#include <config.h>

#include <br_priv.h>

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
cw_br_t *
br_new(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_new()");
  }

  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_new()");
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
br_delete(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_delete()");
  }
  _cw_check_ptr(a_br_o);
  
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_delete()");
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
br_is_open(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_is_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_is_open()");
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
br_open(cw_br_t * a_br_o, char * a_filename)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_open()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_open()");
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
br_close(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_close()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_close()");
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
cw_uint64_t
br_get_block_size(cw_br_t * a_br_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_get_block_size()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_get_block_size()");
  }
  return 0; /* XXX */
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
  rwl_wlock(&a_br_o->rw_lock);

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_add_file()");
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
br_rm_file(cw_br_t * a_br_o, char * a_filename)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_rm_file()");
  }
  _cw_check_ptr(a_br_o);
  rwl_wlock(&a_br_o->rw_lock);

  
  rwl_wunlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_rm_file()");
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
cw_brblk_t *
br_block_slock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_slock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_slock()");
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
cw_brblk_t *
br_block_tlock(cw_br_t * a_br_o,
	       cw_uint64_t a_logical_addr)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_tlock()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_tlock()");
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
cw_bool_t
br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o)
{
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Enter br_block_destroy()");
  }
  _cw_check_ptr(a_br_o);
  rwl_rlock(&a_br_o->rw_lock);

  
  rwl_runlock(&a_br_o->rw_lock);
  if (dbg_pmatch(g_dbg_o, _CW_DBG_R_BR_FUNC))
  {
    _cw_marker("Exit br_block_destroy()");
  }
  return TRUE; /* XXX */
}
