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

#ifndef _BR_H_
#define _BR_H_

/* Pseudo-opaque type. */
typedef struct cw_br_s cw_br_t;

struct cw_br_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;
  cw_bool_t is_open;
};

/* Namespace definitions for br. */
#define br_new _CW_NS_CMN(br_new)
#define br_delete _CW_NS_CMN(br_delete)
#define br_is_open _CW_NS_CMN(br_is_open)
#define br_open _CW_NS_CMN(br_open)
#define br_close _CW_NS_CMN(br_close)
#define br_get_block_size _CW_NS_CMN(br_get_block_size)
#define br_add_file _CW_NS_CMN(br_add_file)
#define br_rm_file _CW_NS_CMN(br_rm_file)
#define br_block_slock _CW_NS_CMN(br_slock)
#define br_block_tlock _CW_NS_CMN(br_tlock)
#define br_block_destroy _CW_NS_CMN(br_block_destroy)
/* #define br_ _CW_NS_CMN(br_) */
/* #define br_ _CW_NS_CMN(br_) */

/* Function prototypes. */
cw_br_t * br_new(cw_br_t * a_br_o);
void br_delete(cw_br_t * a_br_o);

cw_bool_t br_is_open(cw_br_t * a_br_o);

cw_bool_t br_open(cw_br_t * a_br_o, char * a_filename);
cw_bool_t br_close(cw_br_t * a_br_o);

cw_uint64_t br_get_block_size(cw_br_t * a_br_o);
			    
cw_bool_t br_add_file(cw_br_t * a_br_o, char * a_filename,
		      cw_bool_t a_is_raw, cw_bool_t a_can_overlap,
		      cw_bool_t a_is_dynamic,
		      cw_uint64_t a_base_addr, cw_uint64_t a_max_size);
cw_bool_t br_rm_file(cw_br_t * a_br_o, char * a_filename);

cw_brblk_t * br_block_slock(cw_br_t * a_br_o,
			     cw_uint64_t a_logical_addr);
cw_brblk_t * br_block_tlock(cw_br_t * a_br_o,
			     cw_uint64_t a_logical_addr);
cw_bool_t br_block_destroy(cw_br_t * a_br_o, cw_brblk_t * a_brblk_o);

#endif /* _BR_H_ */
