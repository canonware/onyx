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
 * $Revision: 41 $
 * $Date: 1998-04-26 20:06:13 -0700 (Sun, 26 Apr 1998) $
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
dbg_delete(cw_dbg_t * a_dbg_o)
{
  _cw_check_ptr(a_dbg_o);

  _cw_free(a_dbg_o);
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
dbg_fmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dbg_o);

  if (a_dbg_o->is_current == FALSE)
  {
    dbg_recalc_fpmatch(a_dbg_o);
  }
  if (a_flag <= _CW_DBG_T_MAX)
  {
    retval = a_dbg_o->fmatch[a_flag];
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
dbg_pmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dbg_o);

  if (a_dbg_o->is_current == FALSE)
  {
    dbg_recalc_fpmatch(a_dbg_o);
  }

  if (a_flag <= _CW_DBG_T_MAX)
  {
    retval = a_dbg_o->pmatch[a_flag];
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
dbg_turn_on(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  cw_uint32_t x;
  
  _cw_check_ptr(a_dbg_o);

  if (a_flag <= _CW_DBG_T_MAX)
  {
    a_dbg_o->is_current = FALSE;
    for (x = 0; x <= _CW_DBG_C_MAX; x++)
    {
      if (a_dbg_o->tbl[x][a_flag] == TRUE)
      {
	a_dbg_o->curr_settings[x] = TRUE;
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
dbg_turn_off(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  cw_uint32_t x;
  
  _cw_check_ptr(a_dbg_o);

  if (a_flag <= _CW_DBG_T_MAX)
  {
    a_dbg_o->is_current = FALSE;
    for (x = 0; x <= _CW_DBG_C_MAX; x++)
    {
      if (a_dbg_o->tbl[x][a_flag] == TRUE)
      {
	a_dbg_o->curr_settings[x] = FALSE;
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
dbg_clear(cw_dbg_t * a_dbg_o)
{
  cw_uint32_t x;
  
  _cw_check_ptr(a_dbg_o);

  a_dbg_o->is_current = FALSE;
  
  for (x = 0; x <= _CW_DBG_C_MAX; x++)
  {
    a_dbg_o->curr_settings[x] = FALSE;
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
dbg_build_tbl(cw_dbg_t * a_dbg_o)
{
  cw_uint32_t x, y, i;
  extern cw_sint32_t dbg_raw_tbl[];
  extern cw_sint32_t dbg_raw_on[];
  
  _cw_check_ptr(a_dbg_o);

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

      a_dbg_o->tbl[dbg_raw_tbl[i]][y] = TRUE;
    }
  }

  /* Set flags that are on by default. */
  for (i = 0; dbg_raw_on[i] != -1; i++)
  {
    _cw_assert(dbg_raw_tbl[i] <= _CW_DBG_T_MAX);

    for (x = 0; x <= _CW_DBG_T_MAX; x++)
    {
      if (a_dbg_o->tbl[x][dbg_raw_on[i]] == TRUE)
      {
	a_dbg_o->curr_settings[x] = TRUE;
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
dbg_recalc_fpmatch(cw_dbg_t * a_dbg_o)
{
  cw_uint32_t x, y;
  cw_bool_t f, p;
  
  _cw_check_ptr(a_dbg_o);

  for (y = 0; y <= _CW_DBG_T_MAX; y++)
  {
    for (x = 0, f = TRUE, p = FALSE;
	 (x <= _CW_DBG_C_MAX)
	   && ((f == TRUE) || (p == FALSE));
	 x++)
    {
      if ((a_dbg_o->tbl[x][y] == TRUE)
	  && (a_dbg_o->curr_settings[x] == FALSE))
      {
	f = FALSE;
      }
      if ((a_dbg_o->tbl[x][y] == TRUE)
	  && (a_dbg_o->curr_settings[x] == TRUE))
      {
	p = TRUE;
      }
    }

    a_dbg_o->fmatch[y] = f;
    a_dbg_o->pmatch[y] = p;
  }
  
  a_dbg_o->is_current = TRUE;
}
