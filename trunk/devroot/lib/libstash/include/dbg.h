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
 * $Revision: 14 $
 * $Date: 1998-03-29 05:26:20 -0800 (Sun, 29 Mar 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _DBG_H_
#define _DBG_H_

/*
 * Debug flags.
 */

#define _CW_DBG_FUNC_ENTRY 0
#define _CW_DBG_FUNC_EXIT 1
#define _CW_DBG_MISC 2
#define _CW_DBG_FUNC 3

typedef struct cw_dbg_s cw_dbg_t;

#define dbg_new _CW_NS_CMN(dbg_new)
#define dbg_delete _CW_NS_CMN(dbg_delete)
#define dbg_fmatch _CW_NS_CMN(dbg_fmatch)
#define dbg_pmatch _CW_NS_CMN(dbg_pmatch)
#define dbg_turn_on _CW_NS_CMN(dbg_turn_on)
#define dbg_turn_off _CW_NS_CMN(dbg_turn_off)
#define dbg_clear _CW_NS_CMN(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * arg_dbg_obj);
cw_bool_t dbg_fmatch(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag);
cw_bool_t dbg_pmatch(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag);
cw_bool_t dbg_turn_on(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag);
cw_bool_t dbg_turn_off(cw_dbg_t * arg_dbg_obj, cw_uint32_t arg_flag);
void dbg_clear(cw_dbg_t * arg_dbg_obj);

#endif /* _DBG_H_ */
