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
 * $Revision: 65 $
 * $Date: 1998-05-01 21:20:37 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _DBG_H_
#define _DBG_H_

/* Debug flags (columns).  Use these with calls to dbg_turn_on() and
 * dbg_turn_off(). */
#define _CW_DBG_C_FUNC 0
#define _CW_DBG_C_ERROR 1
#define _CW_DBG_C_OH_FUNC 2
#define _CW_DBG_C_OH_SLOT 3
#define _CW_DBG_C_RES_FUNC 4
#define _CW_DBG_C_RES_ERROR 5
#define _CW_DBG_C_RES_STATE 6
#define _CW_DBG_C_BHP_FUNC 7
/* <ADD> */

/* Filters (rows).  Use these with calls to dbg_fmatch() and dbg_pmatch(). */
#define _CW_DBG_R_FUNC 0
#define _CW_DBG_R_ERROR 1
#define _CW_DBG_R_OH_FUNC 2
#define _CW_DBG_R_OH_SLOT 3
#define _CW_DBG_R_RES_FUNC 4
#define _CW_DBG_R_RES_ERROR 5
#define _CW_DBG_R_RES_STATE 6
#define _CW_DBG_R_BHP_FUNC 7
/* <ADD> */

/* Put these here only because they're related to the above macros.  They
 * aren't directly useful to the caller. */
#define _CW_DBG_C_MAX 7 /* Highest numbered column in table. */
#define _CW_DBG_R_MAX 7 /* Highest numbered row in table. */
/* <ADD> */

typedef struct cw_dbg_s cw_dbg_t;

#define dbg_new _CW_NS_CMN(dbg_new)
#define dbg_delete _CW_NS_CMN(dbg_delete)
#define dbg_fmatch _CW_NS_CMN(dbg_fmatch)
#define dbg_pmatch _CW_NS_CMN(dbg_pmatch)
#define dbg_turn_on _CW_NS_CMN(dbg_turn_on)
#define dbg_turn_off _CW_NS_CMN(dbg_turn_off)
#define dbg_clear _CW_NS_CMN(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * a_dbg_o);
cw_bool_t dbg_fmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
cw_bool_t dbg_pmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_turn_on(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_turn_off(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_clear(cw_dbg_t * a_dbg_o);

#endif /* _DBG_H_ */
