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
 * $Revision: 56 $
 * $Date: 1998-05-01 03:14:47 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 * Dynamic debug spew class.  The idea is to be able to turn various types
 * of debug spew on and off on the fly, without recompilation, without even
 * restarting the program.
 *
 * dbg works by building a table as such:
 *
 *           | 0 | 1 | 2 | 3 | 4 | . |   |   |   |   |   | x |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * Row 0     |   |   |   |   |   | . |   |   |   |   |   |   |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * Row 1     |   |   |   |   |   | . |   |   |   |   |   |   |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * ...........................................................
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * Row y -1  |   |   |   |   |   | . |   |   |   |   |   |   |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * Row y     |   |   |   |   |   | . |   |   |   |   |   |   |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 *
 * The set of columns turned on looks like:
 *
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 * Turned on |   |   |   |   |   | . |   |   |   |   |   |   |
 * ----------+---+---+---+---+---+ . +---+---+---+---+---+---+
 *
 * The functions dbg_turn_on() and dbg_turn_off() affect the settings in
 * the turned on columns set.
 *
 * The function dbg_fmatch() returns true if the row in question is a
 * subset of the turned on columns.
 *
 * The function dbg_pmatch() returns true if the intersection of the turned
 * on columns and the row in question exists.
 *
 ****************************************************************************/

#define _INC_STRING_H_
#include <config.h>
#include <dbg_priv.h>

/****************************************************************************
 * <<< Description >>>
 *
 * dbg constructor.
 *
 ****************************************************************************/
cw_dbg_t *
dbg_new()
{
  cw_dbg_t * retval;

  retval = (cw_dbg_t *) _cw_malloc(sizeof(cw_dbg_t));

  /* Zero out the entire structure. */
  bzero(retval, sizeof(cw_dbg_t));

  rwl_new(&retval->rw_lock);
  dbg_build_tbl(retval);
  dbg_recalc_fpmatch(retval);

  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * dbg destructor.
 *
 ****************************************************************************/
void
dbg_delete(cw_dbg_t * a_dbg_o)
{
  _cw_check_ptr(a_dbg_o);

  rwl_delete(&a_dbg_o->rw_lock);

  _cw_free(a_dbg_o);
}

/****************************************************************************
 * <<< Description >>>
 *
 * See if row a_flag is a subset of curr_settings.
 *
 ****************************************************************************/
cw_bool_t
dbg_fmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dbg_o);
  rwl_rlock(&a_dbg_o->rw_lock);
  
  if (a_dbg_o->is_current == FALSE)
  {
    dbg_recalc_fpmatch(a_dbg_o);
  }
  if (a_flag <= _CW_DBG_R_MAX)
  {
    retval = a_dbg_o->fmatch[a_flag];
  }
  else
  {
    retval = FALSE;
  }
  
  rwl_runlock(&a_dbg_o->rw_lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * See if the intersection of row a_flag and curr_settings exists.
 *
 ****************************************************************************/
cw_bool_t
dbg_pmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  cw_bool_t retval;
  
  _cw_check_ptr(a_dbg_o);
  rwl_rlock(&a_dbg_o->rw_lock);

  if (a_dbg_o->is_current == FALSE)
  {
    dbg_recalc_fpmatch(a_dbg_o);
  }

  if (a_flag <= _CW_DBG_R_MAX)
  {
    retval = a_dbg_o->pmatch[a_flag];
  }
  else
  {
    retval = FALSE;
  }
  
  rwl_runlock(&a_dbg_o->rw_lock);
  return retval;
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set column a_flag in curr_settings to true.
 *
 ****************************************************************************/
void
dbg_turn_on(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  _cw_check_ptr(a_dbg_o);
  _cw_assert(a_flag <= _CW_DBG_R_MAX);
  rwl_wlock(&a_dbg_o->rw_lock);

  a_dbg_o->is_current = FALSE;
  a_dbg_o->curr_settings[a_flag] = TRUE;

  rwl_wunlock(&a_dbg_o->rw_lock);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Set column a_flag in curr_settings to false.
 *
 ****************************************************************************/
void
dbg_turn_off(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag)
{
  _cw_check_ptr(a_dbg_o);
  _cw_assert(a_flag <= _CW_DBG_R_MAX);
  rwl_wlock(&a_dbg_o->rw_lock);

  a_dbg_o->is_current = FALSE;
  a_dbg_o->curr_settings[a_flag] = FALSE;

  rwl_wunlock(&a_dbg_o->rw_lock);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Clear curr_settings.
 *
 ****************************************************************************/
void
dbg_clear(cw_dbg_t * a_dbg_o)
{
  cw_uint32_t x;
  
  _cw_check_ptr(a_dbg_o);
  rwl_wlock(&a_dbg_o->rw_lock);

  a_dbg_o->is_current = FALSE;
  
  for (x = 0; x <= _CW_DBG_C_MAX; x++)
  {
    a_dbg_o->curr_settings[x] = FALSE;
  }
  
  rwl_wunlock(&a_dbg_o->rw_lock);
}

/****************************************************************************
 * <<< Description >>>
 *
 * Build the matrix from the raw array defined in dbg_priv.h.
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
  for (i = 0, y = 0; y <= _CW_DBG_R_MAX; i++)
  {
    if (dbg_raw_tbl[i] == -1)
    {
      y++;
      if ((dbg_raw_tbl[i + 1] == -1)
	  && (y < _CW_DBG_R_MAX))
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
    _cw_assert(dbg_raw_tbl[i] <= _CW_DBG_R_MAX);

    for (x = 0; x <= _CW_DBG_R_MAX; x++)
    {
      if (a_dbg_o->tbl[x][dbg_raw_on[i]] == TRUE)
      {
	a_dbg_o->curr_settings[x] = TRUE;
      }
    }
  }
}

/****************************************************************************
 * <<< Description >>>
 *
 * Recalculate full and partial matches for all rows, given curr_settings.
 *
 ****************************************************************************/
void
dbg_recalc_fpmatch(cw_dbg_t * a_dbg_o)
{
  cw_uint32_t x, y;
  cw_bool_t f, p;
  
  _cw_check_ptr(a_dbg_o);

  /* Iterate through rows. */
  for (y = 0; y <= _CW_DBG_R_MAX; y++)
  {
    /* Iterate through columns. */
    for (x = 0, f = TRUE, p = FALSE;
	 (x <= _CW_DBG_C_MAX)
	   && ((f == TRUE) || (p == FALSE));
	 x++)
    {
      /* Checking for a full match against the current settings. */
      if ((a_dbg_o->tbl[x][y] == TRUE)
	  && (a_dbg_o->curr_settings[x] == FALSE))
      {
	f = FALSE;
      }
      /* Checking for a partial match against the current settings. */
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
