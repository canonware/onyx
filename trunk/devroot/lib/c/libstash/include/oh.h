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
 * $Revision: 24 $
 * $Date: 1998-04-12 01:44:39 -0700 (Sun, 12 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_H_
#define _OH_H_

#define _OH_PERF_

typedef struct oh_s oh_t;

#define oh_new _CW_NS_CMN(oh_new)
#define oh_delete _CW_NS_CMN(oh_delete)
#define oh_rehash _CW_NS_CMN(oh_rehash)
#define oh_get_size _CW_NS_CMN(oh_get_size)
#define oh_get_num_items _CW_NS_CMN(oh_get_num_items)

#define oh_get_h1 _CW_NS_CMN(oh_get_h1)
#define oh_get_h2 _CW_NS_CMN(oh_get_h2)
#define oh_get_shrink_point _CW_NS_CMN(oh_get_shrink_point)
#define oh_get_grow_point _CW_NS_CMN(oh_get_grow_point)
#define oh_get_rehash_point _CW_NS_CMN(oh_get_rehash_point)

#define oh_set_h1 _CW_NS_CMN(oh_set_h1)
#define oh_set_h2 _CW_NS_CMN(oh_set_h2)
#define oh_set_shrink_point _CW_NS_CMN(oh_set_shrink_point)
#define oh_set_grow_point _CW_NS_CMN(oh_set_grow_point)
#define oh_set_rehash_point _CW_NS_CMN(oh_set_rehash_point)
#define oh_set_allowed_invalid _CW_NS_CMN(oh_set_allowed_invalid)

#define oh_item_insert _CW_NS_CMN(oh_insert)
#define oh_item_delete _CW_NS_CMN(oh_delete)
#define oh_item_search _CW_NS_CMN(oh_search)

#define oh_dump _CW_NS_CMN(oh_dump)

#ifdef _OH_PERF_
#  define oh_get_num_collisions _CW_NS_CMN(oh_get_num_collisions)
#  define oh_get_num_inserts _CW_NS_CMN(oh_get_num_inserts)
#  define oh_get_num_deletes _CW_NS_CMN(oh_get_num_deletes)
#  define oh_get_num_grows _CW_NS_CMN(oh_get_num_grows)
#  define oh_get_num_shrinks _CW_NS_CMN(oh_get_num_shrinks)
#  define oh_get_num_rehashes _CW_NS_CMN(oh_get_num_rehashes)
#endif

typedef cw_uint32_t oh_h1_t(oh_t *, cw_uint32_t);

oh_t * oh_new();
void oh_delete(oh_t * arg_oh_obj);
cw_bool_t oh_rehash(oh_t * arg_oh_obj);
cw_uint32_t oh_get_size(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_items(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_invalid(oh_t * arg_oh_obj);

oh_h1_t * oh_get_h1(oh_t * arg_oh_obj);
cw_sint32_t oh_get_base_h2(oh_t * arg_oh_obj);
cw_sint32_t oh_get_base_shrink_point(oh_t * arg_oh_obj);
cw_sint32_t oh_get_base_grow_point(oh_t * arg_oh_obj);
cw_sint32_t oh_get_base_rehash_point(oh_t * arg_oh_obj);

cw_bool_t oh_set_h1(oh_t * arg_oh_obj,
		 oh_h1_t * arg_new_h1);
cw_bool_t oh_set_base_h2(oh_t * arg_oh_obj,
		      cw_uint32_t arg_h2);
cw_bool_t oh_set_base_shrink_point(oh_t * arg_oh_obj,
				cw_sint32_t arg_shrink_point);
cw_bool_t oh_set_base_grow_point(oh_t * arg_oh_obj,
			      cw_sint32_t arg_grow_point);
cw_bool_t oh_set_base_rehash_point(oh_t * arg_oh_obj,
				cw_sint32_t arg_rehash_point);

cw_bool_t oh_item_insert(oh_t * arg_oh_obj, cw_uint32_t arg_key,
		      void * arg_data_addr);
cw_bool_t oh_item_delete(oh_t * arg_oh_obj, cw_uint32_t arg_key,
		      void ** arg_data);
cw_bool_t oh_item_search(oh_t * arg_oh_obj, cw_uint32_t arg_key,
		      void ** arg_data);

void oh_dump(oh_t * arg_oh_obj, cw_bool_t arg_all);

#ifdef _OH_PERF_
cw_uint32_t oh_get_num_collisions(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_inserts(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_deletes(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_grows(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_shrinks(oh_t * arg_oh_obj);
cw_uint32_t oh_get_num_rehashes(oh_t * arg_oh_obj);
#endif

#endif /* _OH_H_ */
