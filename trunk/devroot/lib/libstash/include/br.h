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
 * $Revision: 76 $
 * $Date: 1998-05-02 22:34:39 -0700 (Sat, 02 May 1998) $
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

typedef struct
{
  cw_btl_t bt_lock;
  cw_bool_t is_valid;
  cw_bool_t is_dirty;
  cw_uint64_t logical_addr;
  cw_uint8_t * block;
} cw_brblk_t;

struct cw_br_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;
  cw_bool_t is_open;
  
};

/* Namespace definitions for brblk. */
#define brblk_new _CW_NS_CMN(brblk_new)
#define brblk_delete _CW_NS_CMN(brblk_delete)
#define brblk_slock _CW_NS_CMN(brblk_slock)
#define brblk_tlock _CW_NS_CMN(brblk_tlock)
#define brblk_s2dlock _CW_NS_CMN(brblk_s2dlock)
#define brblk_s2rlock _CW_NS_CMN(brblk_s2rlock)
#define brblk_s2wlock _CW_NS_CMN(brblk_s2wlock)
#define brblk_s2xlock _CW_NS_CMN(brblk_s2xlock)
#define brblk_t2rlock _CW_NS_CMN(brblk_t2rlock)
#define brblk_t2wlock _CW_NS_CMN(brblk_t2wlock)
#define brblk_t2xlock _CW_NS_CMN(brblk_t2xlock)
#define brblk_sunlock _CW_NS_CMN(brblk_sunlock)
#define brblk_tunlock _CW_NS_CMN(brblk_tunlock)
#define brblk_dunlock _CW_NS_CMN(brblk_dunlock)
#define brblk_runlock _CW_NS_CMN(brblk_runlock)
#define brblk_wunlock _CW_NS_CMN(brblk_wunlock)
#define brblk_xunlock _CW_NS_CMN(brblk_xunlock)
#define brblk_get_byte _CW_NS_CMN(brblk_get_byte)
#define brblk_set_byte _CW_NS_CMN(brblk_set_byte)
#define brblk_is_dirty _CW_NS_CMN(brblk_is_dirty)
#define brblk_flush _CW_NS_CMN(brblk_flush)
/* #define brblk_ _CW_NS_CMN(brblk_) */
/* #define brblk_ _CW_NS_CMN(brblk_) */

/* brblk methods. */
cw_brblk_t * brblk_new(cw_brblk_t * a_brblk_o);
void brblk_delete(cw_brblk_t * a_brblk_o);

void brblk_slock(cw_brblk_t * a_brblk_o);
void brblk_tlock(cw_brblk_t * a_brblk_o);

void brblk_s2dlock(cw_brblk_t * a_brblk_o);
void brblk_s2rlock(cw_brblk_t * a_brblk_o);
void brblk_s2wlock(cw_brblk_t * a_brblk_o);
void brblk_s2xlock(cw_brblk_t * a_brblk_o);
void brblk_t2rlock(cw_brblk_t * a_brblk_o);
void brblk_t2wlock(cw_brblk_t * a_brblk_o);
void brblk_t2xlock(cw_brblk_t * a_brblk_o);

void brblk_sunlock(cw_brblk_t * a_brblk_o);
void brblk_tunlock(cw_brblk_t * a_brblk_o);
void brblk_dunlock(cw_brblk_t * a_brblk_o);
void brblk_runlock(cw_brblk_t * a_brblk_o);
void brblk_wunlock(cw_brblk_t * a_brblk_o);
void brblk_xunlock(cw_brblk_t * a_brblk_o);

cw_bool_t brblk_get_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
			 cw_uint8_t * a_byte);
cw_bool_t brblk_set_byte(cw_brblk_t * a_brblk_o, cw_uint32_t a_offset,
			 cw_uint8_t a_byte);

cw_bool_t brblk_is_dirty(cw_brblk_t * a_brblk_o);
cw_bool_t brblk_flush(cw_brblk_t * a_brblk_o);

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

/* br methods. */
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
