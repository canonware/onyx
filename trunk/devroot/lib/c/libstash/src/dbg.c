/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (c) 1996-1998
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
 * $Revision: 15 $
 * $Date: 1998-03-29 05:26:45 -0800 (Sun, 29 Mar 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_DBG_PRIV_H_
#define _INC_STRING_H_
#include <config.h>

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
cw_dbg_t *
dbg_new()
{
  cw_dbg_t * retval;
  
  retval = (cw_dbg_t *) _cw_malloc(sizeof(cw_dbg_t));

  /* Zero out the entire structure. */
  bzero(retval, sizeof(cw_dbg_t));

  dbg_build_tbl(retval);
  dbg_recalc_fpmatch(retval);

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_delete(cw_dbg_t * arg_dbg_obj)
{
  _cw_check_ptr(arg_dbg_obj);

  _cw_free(arg_dbg_obj);
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_fmatch(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_dbg_obj);

  if (arg_dbg_obj->is_current == FALSE)
  {
    dbg_recalc_fpmatch(arg_dbg_obj);
  }
  if (arg_flag <= _CW_DBG_T_MAX)
  {
    retval = arg_dbg_obj->fmatch[arg_flag];
  }
  else
  {
    retval = FALSE;
  }
  
  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_pmatch(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(arg_dbg_obj);

  if (arg_dbg_obj->is_current == FALSE)
  {
    dbg_recalc_fpmatch(arg_dbg_obj);
  }

  if (arg_flag <= _CW_DBG_T_MAX)
  {
    retval = arg_dbg_obj->pmatch[arg_flag];
  }
  else
  {
    retval = FALSE;
  }
  
  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_turn_on(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag)
{
  cw_bool_t retval;
  cw_uint32_t x;
  
  _cw_check_ptr(arg_dbg_obj);

  if (arg_flag <= _CW_DBG_T_MAX)
  {
    arg_dbg_obj->is_current = FALSE;
    for (x = 0; x <= _CW_DBG_C_MAX; x++)
    {
      if (arg_dbg_obj->tbl[x][arg_flag] == TRUE)
      {
	arg_dbg_obj->curr_settings[x] = TRUE;
      }
    }
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_turn_off(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag)
{
  cw_bool_t retval;
  cw_uint32_t x;
  
  _cw_check_ptr(arg_dbg_obj);

  if (arg_flag <= _CW_DBG_T_MAX)
  {
    arg_dbg_obj->is_current = FALSE;
    for (x = 0; x <= _CW_DBG_C_MAX; x++)
    {
      if (arg_dbg_obj->tbl[x][arg_flag] == TRUE)
      {
	arg_dbg_obj->curr_settings[x] = FALSE;
      }
    }
    retval = FALSE;
  }
  else
  {
    retval = TRUE;
  }

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_clear(cw_dbg_t * arg_dbg_obj)
{
  cw_uint32_t x;
  
  _cw_check_ptr(arg_dbg_obj);

  arg_dbg_obj->is_current = FALSE;
  
  for (x = 0; x <= _CW_DBG_C_MAX; x++)
  {
    arg_dbg_obj->curr_settings[x] = FALSE;
  }
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_build_tbl(cw_dbg_t * arg_dbg_obj)
{
  cw_uint32_t x, y, i;
  extern cw_sint32_t dbg_raw_tbl[];
  extern cw_sint32_t dbg_raw_on[];
  
  _cw_check_ptr(arg_dbg_obj);

  /* Build table. */
  for (i = 0, y = 0; y <= _CW_DBG_T_MAX; i++)
  {
    if (dbg_raw_tbl[i] == -1)
    {
      y++;
      if ((dbg_raw_tbl[i + 1] == -1)
	  && (y < _CW_DBG_T_MAX))
      {
	_cw_error("Raw debug table is inconsistent.");
      }
    }
    else
    {
      _cw_assert(dbg_raw_tbl[i] <= _CW_DBG_C_MAX);

      arg_dbg_obj->tbl[dbg_raw_tbl[i]][y] = TRUE;
    }
  }

  /* Set flags that are on by default. */
  for (i = 0; dbg_raw_on[i] != -1; i++)
  {
    _cw_assert(dbg_raw_tbl[i] <= _CW_DBG_T_MAX);

    for (x = 0; x <= _CW_DBG_T_MAX; x++)
    {
      if (arg_dbg_obj->tbl[x][dbg_raw_on[i]] == TRUE)
      {
	arg_dbg_obj->curr_settings[x] = TRUE;
      }
    }
  }
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
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
dbg_recalc_fpmatch(cw_dbg_t * arg_dbg_obj)
{
  cw_uint32_t x, y;
  cw_bool_t f, p;
  
  _cw_check_ptr(arg_dbg_obj);

  for (y = 0; y <= _CW_DBG_T_MAX; y++)
  {
    for (x = 0, f = TRUE, p = FALSE;
	 (x <= _CW_DBG_C_MAX)
	   && ((f == TRUE) || (p == FALSE));
	 x++)
    {
      if ((arg_dbg_obj->tbl[x][y] == TRUE)
	  && (arg_dbg_obj->curr_settings[x] == FALSE))
      {
	f = FALSE;
      }
      if ((arg_dbg_obj->tbl[x][y] == TRUE)
	  && (arg_dbg_obj->curr_settings[x] == TRUE))
      {
	p = TRUE;
      }
    }

    arg_dbg_obj->fmatch[y] = f;
    arg_dbg_obj->pmatch[y] = p;
  }
  
  arg_dbg_obj->is_current = TRUE;
}

