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
 * $Revision: 71 $
 * $Date: 1998-05-02 02:10:52 -0700 (Sat, 02 May 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _OH_PRIV_H_
#define _OH_PRIV_H_

#define oh_coalesce_priv _CW_NS_CMN(oh_coalesce_priv)
#define oh_grow_priv _CW_NS_CMN(oh_grow_priv)
#define oh_shrink_priv _CW_NS_CMN(oh_shrink_priv)
#define oh_h1_priv _CW_NS_CMN(oh_h1_priv)
#define oh_insert_priv _CW_NS_CMN(oh_insert_priv)
#define oh_search_priv _CW_NS_CMN(oh_search_priv)
#define oh_rehash_priv _CW_NS_CMN(oh_rehash_priv)

/* 2^(_OH_BASE_POWER) is the base table size used for calculations. */
#define _OH_BASE_POWER 8;
#define _OH_BASE_H2 7;

/* Minimum number of items allowable before regrowing. */
#define _OH_BASE_SHRINK_POINT 64;

/* Maximum number of items allowable before growing. */
#define _OH_BASE_GROW_POINT 230;

/* Maximum number of items allowable before we consider rehashing. */
#define _OH_BASE_REHASH_POINT 192;

cw_bool_t oh_coalesce_priv(cw_oh_t * a_oh_o);
cw_bool_t oh_grow_priv(cw_oh_t * a_oh_o);
cw_bool_t oh_shrink_priv(cw_oh_t * a_oh_o);

cw_uint64_t oh_h1_priv(cw_oh_t * a_oh_o, void * a_key);
cw_bool_t oh_key_compare_priv(void * a_k1, void * a_k2);
cw_bool_t oh_item_insert_priv(cw_oh_t * a_oh_o,
			      cw_oh_item_t * a_item);
cw_bool_t oh_item_search_priv(cw_oh_t * a_oh_o,
			      void * a_key,
			      cw_uint64_t * a_slot);
cw_bool_t oh_rehash_priv(cw_oh_t * a_oh_o, cw_bool_t a_force);

#endif /* _OH_PRIV_H_ */
