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
 * $Revision: 82 $
 * $Date: 1998-05-18 23:41:54 -0700 (Mon, 18 May 1998) $
 *
 * <<< Description >>>
 *
 * Private header for debug spew code.  Additional debug flags can be added
 * here, in conjunction with dbg.h.
 *
 ****************************************************************************/

#ifndef _DBG_PRIV_H_
#define _DBG_PRIV_H_

/* These are globals so that we don't have to have multiple copies of them,
 * even if there are multiple dbg instances.  We mangle the names to keep
 * them from interfering with other code. */
#define dbg_raw_tbl _CW_NS_CMN(dbg_raw_tbl)
#define dbg_raw_on _CW_NS_CMN(dbg_raw_on)

/* Array used to construct the debug table.  Make sure to add *_T_* here.
 * Order *IS* important.  These should be in the same order as the
 * externally visible *_R_* macros for which they are named.  Also, make
 * sure that all entries are terminated by a -1.  The extra -1 at the end
 * is also important. */
cw_sint32_t dbg_raw_tbl[] =
{
  _CW_DBG_C_DBG, -1, /* CW_DBG_R_DBG */
  _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_FUNC */
  _CW_DBG_C_ERROR, -1, /* _CW_DBG_R_ERROR */
  _CW_DBG_C_OH_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_OH_FUNC */
  _CW_DBG_C_OH_SLOT, -1, /* _CW_DBG_R_OH_SLOT */
  _CW_DBG_C_RES_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_RES_FUNC */
  _CW_DBG_C_RES_ERROR, _CW_DBG_C_ERROR, -1, /* _CW_DBG_R_RES_ERROR */
  _CW_DBG_C_RES_STATE, -1, /* _CW_DBG_R_RES_STATE */
  _CW_DBG_C_BHP_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BHP_FUNC */
  _CW_DBG_C_LIST_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_LIST_FUNC */
  _CW_DBG_C_BR_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BR_FUNC */
  _CW_DBG_C_BRBLK_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BRBLK_FUNC */
  /* <ADD> */
  -1
};

/*
 * Debug flags that are turned on by default.
 */
cw_sint32_t dbg_raw_on[] =
{
  _CW_DBG_C_DBG,
  /* <ADD> */
  -1
};

struct cw_dbg_s
{
  cw_rwl_t rw_lock;
  cw_bool_t is_current;
  cw_bool_t curr_settings[_CW_DBG_C_MAX + 1];
  cw_bool_t fmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t pmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t tbl[_CW_DBG_C_MAX + 1][_CW_DBG_R_MAX + 1];
};

#define dbg_build_tbl _CW_NS_CMN(dbg_build_tbl)
#define dbg_recalc_fpmatch _CW_NS_CMN(dbg_recalc_fpmatch)

void dbg_build_tbl(cw_dbg_t * a_dbg_o);
void dbg_recalc_fpmatch(cw_dbg_t * a_dbg_o);

#endif /* _DBG_PRIV_H_ */
