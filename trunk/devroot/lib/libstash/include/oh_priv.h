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
 * $Revision: 29 $
 * $Date: 1998-04-13 01:24:02 -0700 (Mon, 13 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_PRIV_H_
#define _OH_PRIV_H_

#include <oh.h>

#ifdef _SHADE_
#include <namespace.h>

#define oh_coalesce_priv EXTOS(oh_coalesce_priv)
#define oh_grow_priv EXTOS(oh_grow_priv)
#define oh_shrink_priv EXTOS(oh_shrink_priv)
#define oh_h1_priv EXTOS(oh_h1)
#define oh_insert_priv EXTOS(oh_insert_priv)
#define oh_search_priv EXTOS(oh_search_priv)
#define oh_rehash_priv EXTOS(oh_rehash_priv)

#endif

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before regrowing. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 230;

/* Maximum number of items allowable before we consider rehashing. */
#define _OH_BASE_REHASH_POINT 192;

typedef struct
{
  cw_bool_t is_valid;
  cw_uint32_t key;
  void * data;
} cw_oh_item_t;

struct oh_s
{
  cw_rwl_t rw_lock;
  cw_oh_item_t ** items;

  cw_uint32_t (*base_h1)(oh_t *, cw_uint32_t);
  cw_uint32_t (*curr_h1)(oh_t *, cw_uint32_t);

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

cw_bool_t oh_coalesce_priv(oh_t * arg_oh_obj);
cw_bool_t oh_grow_priv(oh_t * arg_oh_obj,
			  cw_sint32_t arg_num_doublings);
cw_bool_t oh_shrink_priv(oh_t * arg_oh_obj,
			    cw_sint32_t arg_num_halvings);

cw_uint32_t oh_h1_priv(oh_t * arg_oh_obj, cw_uint32_t arg_key);
cw_bool_t oh_item_insert_priv(oh_t * arg_oh_obj,
			      cw_oh_item_t * arg_item);
cw_bool_t oh_item_search_priv(oh_t * arg_oh_obj,
				 cw_uint32_t arg_key,
				 cw_uint32_t * arg_slot);
cw_bool_t oh_rehash_priv(oh_t * arg_oh_obj, cw_bool_t arg_force);


#endif /* _OH_PRIV_H_ */
