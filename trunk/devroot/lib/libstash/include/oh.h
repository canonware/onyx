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
 * $Revision: 57 $
 * $Date: 1998-05-01 03:17:44 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_H_
#define _OH_H_

#define _OH_PERF_

/* Pseudo-opaque type. */
typedef struct cw_oh_s cw_oh_t;

typedef struct
{
  cw_bool_t is_valid;
  void * key;
  void * data;
} cw_oh_item_t;

struct cw_oh_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;
  cw_oh_item_t ** items;

  cw_uint32_t (*base_h1)(cw_oh_t *, void *);
  cw_uint32_t (*curr_h1)(cw_oh_t *, void *);
  cw_bool_t (*key_compare)(void *, void *);

  cw_uint32_t size;
  cw_uint32_t num_items;
  cw_uint32_t num_invalid;

  cw_uint32_t base_power;
  cw_uint32_t base_h2;
  cw_uint32_t base_shrink_point;
  cw_uint32_t base_grow_point;
  cw_uint32_t base_rehash_point;

  cw_uint32_t curr_power;
  cw_uint32_t curr_h2;
  cw_uint32_t curr_shrink_point;
  cw_uint32_t curr_grow_point;
  cw_uint32_t curr_rehash_point;

#ifdef _OH_PERF_  
  /* Counters used to get an idea of performance. */
  cw_uint32_t num_collisions;
  cw_uint32_t num_inserts;
  cw_uint32_t num_deletes;
  cw_uint32_t num_grows;
  cw_uint32_t num_shrinks;
  cw_uint32_t num_rehashes;
#endif
};

#define oh_new _CW_NS_CMN(oh_new)
#define oh_delete _CW_NS_CMN(oh_delete)
#define oh_rehash _CW_NS_CMN(oh_rehash)
#define oh_get_size _CW_NS_CMN(oh_get_size)
#define oh_get_num_items _CW_NS_CMN(oh_get_num_items)

#define oh_get_h1 _CW_NS_CMN(oh_get_h1)
#define oh_get_key_compare _CW_NS_CMN(oh_get_key_compare)
#define oh_get_h2 _CW_NS_CMN(oh_get_h2)
#define oh_get_shrink_point _CW_NS_CMN(oh_get_shrink_point)
#define oh_get_grow_point _CW_NS_CMN(oh_get_grow_point)
#define oh_get_rehash_point _CW_NS_CMN(oh_get_rehash_point)

#define oh_set_h1 _CW_NS_CMN(oh_set_h1)
#define oh_set_key_compare _CW_NS_CMN(oh_set_key_compare)
#define oh_set_h2 _CW_NS_CMN(oh_set_h2)
#define oh_set_shrink_point _CW_NS_CMN(oh_set_shrink_point)
#define oh_set_grow_point _CW_NS_CMN(oh_set_grow_point)
#define oh_set_rehash_point _CW_NS_CMN(oh_set_rehash_point)
#define oh_set_allowed_invalid _CW_NS_CMN(oh_set_allowed_invalid)

#define oh_item_insert _CW_NS_CMN(oh_insert)
#define oh_item_delete _CW_NS_CMN(oh_delete)
#define oh_item_search _CW_NS_CMN(oh_search)

#define oh_item_delete_iterate _CW_NS_CMN(oh_item_delete_iterate)
#define oh_dump _CW_NS_CMN(oh_dump)

#ifdef _OH_PERF_
#  define oh_get_num_collisions _CW_NS_CMN(oh_get_num_collisions)
#  define oh_get_num_inserts _CW_NS_CMN(oh_get_num_inserts)
#  define oh_get_num_deletes _CW_NS_CMN(oh_get_num_deletes)
#  define oh_get_num_grows _CW_NS_CMN(oh_get_num_grows)
#  define oh_get_num_shrinks _CW_NS_CMN(oh_get_num_shrinks)
#  define oh_get_num_rehashes _CW_NS_CMN(oh_get_num_rehashes)
#endif

/* Typedefs to allow easy function pointer passing. */
typedef cw_uint32_t oh_h1_t(cw_oh_t *, void *);
typedef cw_bool_t oh_key_comp_t(void *, void *);

cw_oh_t * oh_new(cw_oh_t * a_oh_o);
void oh_delete(cw_oh_t * a_oh_o);
cw_bool_t oh_rehash(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_size(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_items(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_invalid(cw_oh_t * a_oh_o);

oh_h1_t * oh_get_h1(cw_oh_t * a_oh_o);
oh_key_comp_t * oh_get_key_compare(cw_oh_t * a_oh_o);
cw_sint32_t oh_get_base_h2(cw_oh_t * a_oh_o);
cw_sint32_t oh_get_base_shrink_point(cw_oh_t * a_oh_o);
cw_sint32_t oh_get_base_grow_point(cw_oh_t * a_oh_o);
cw_sint32_t oh_get_base_rehash_point(cw_oh_t * a_oh_o);

cw_bool_t oh_set_h1(cw_oh_t * a_oh_o,
		    oh_h1_t * a_new_h1);
void oh_set_key_compare(cw_oh_t * a_oh_o,
			oh_key_comp_t * a_new_key_compare);
cw_bool_t oh_set_base_h2(cw_oh_t * a_oh_o,
			 cw_uint32_t a_h2);
cw_bool_t oh_set_base_shrink_point(cw_oh_t * a_oh_o,
				   cw_sint32_t a_shrink_point);
cw_bool_t oh_set_base_grow_point(cw_oh_t * a_oh_o,
				 cw_sint32_t a_grow_point);
cw_bool_t oh_set_base_rehash_point(cw_oh_t * a_oh_o,
				   cw_sint32_t a_rehash_point);

cw_bool_t oh_item_insert(cw_oh_t * a_oh_o, void * a_key,
			 void * a_data);
cw_bool_t oh_item_delete(cw_oh_t * a_oh_o, void * a_search_key, void ** a_key,
			 void ** a_data);
cw_bool_t oh_item_search(cw_oh_t * a_oh_o, void * a_key,
			 void ** a_data);

cw_bool_t oh_item_delete_iterate(cw_oh_t * a_oh_o, void ** a_key,
				 void ** a_data);
void oh_dump(cw_oh_t * a_oh_o, cw_bool_t a_all);

#ifdef _OH_PERF_
cw_uint32_t oh_get_num_collisions(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_inserts(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_deletes(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_grows(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_shrinks(cw_oh_t * a_oh_o);
cw_uint32_t oh_get_num_rehashes(cw_oh_t * a_oh_o);
#endif

#endif /* _OH_H_ */
