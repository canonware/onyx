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
 * $Revision: 63 $
 * $Date: 1998-05-01 21:19:49 -0700 (Fri, 01 May 1998) $
 *
 * <<< Description >>>
 *
 * Header file for bhp class (binomial heap).
 *
 ****************************************************************************/

#ifndef _BHP_H_
#define _BHP_H_

/* Pseudo-opaque type. */
typedef struct cw_bhp_s cw_bhp_t;

typedef struct cw_bhpi_s
{
  void * priority;
  void * data;
  struct cw_bhpi_s * parent;
  struct cw_bhpi_s * child;
  struct cw_bhpi_s * sibling;
  cw_uint32_t degree;
} cw_bhpi_t;

struct cw_bhp_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
  cw_rwl_t rw_lock;
  cw_bhpi_t * head;
  cw_uint64_t num_nodes;
  cw_sint32_t (*priority_compare)(cw_bhpi_t *, cw_bhpi_t *);
};

#define bhp_new _CW_NS_CMN(bhp_new)
#define bhp_delete _CW_NS_CMN(bhp_delete)
#define bhp_dump _CW_NS_CMN(bhp_dump)
#define bhp_insert _CW_NS_CMN(bhp_insert)
#define bhp_find_min _CW_NS_CMN(bhp_find_min)
#define bhp_del_min _CW_NS_CMN(bhp_del_min)
#define bhp_get_size _CW_NS_CMN(bhp_get_size)
#define bhp_union _CW_NS_CMN(bhp_union)
#define bhp_set_priority_compare _CW_NS_CMN(bhp_set_priority_compare)

/* Typedefs to allow easy function pointer passing. */
typedef cw_sint32_t bhp_prio_comp_t(cw_bhpi_t *, cw_bhpi_t *);

cw_bhp_t * bhp_new(cw_bhp_t * a_bhp_o, cw_bool_t a_is_thread_safe);
void bhp_delete(cw_bhp_t * a_bhp_o);
void bhp_dump(cw_bhp_t * a_bhp_o);
void bhp_insert(cw_bhp_t * a_bhp_o, void * a_priority, void * a_data);
cw_bool_t bhp_find_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data);
cw_bool_t bhp_del_min(cw_bhp_t * a_bhp_o, void ** a_priority, void ** a_data);
cw_uint64_t bhp_get_size(cw_bhp_t * a_bhp_o);
void bhp_union(cw_bhp_t * a_bhp_o, cw_bhp_t * a_other);
void bhp_set_priority_compare(cw_bhp_t * a_bhp_o,
			      bhp_prio_comp_t * a_new_prio_comp);

#endif /* _BHP_H_ */
