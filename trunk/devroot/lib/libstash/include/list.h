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
 * $Revision: 33 $
 * $Date: 1998-04-19 01:43:20 -0700 (Sun, 19 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _LIST_H_
#define _LIST_H_

typedef struct cw_list_item_s cw_list_item_t;
typedef struct cw_list_s cw_list_t;

/*
 * Namespace definitions.
 */

#define list_item_new _CW_NS_CMN(list_item_new)
#define list_item_delete _CW_NS_CMN(list_item_delete)
#define list_item_get _CW_NS_CMN(list_item_get)
#define list_item_set _CW_NS_CMN(list_item_set)

#define list_new _CW_NS_CMN(list_new)
#define list_delete _CW_NS_CMN(list_delete)
#define list_count _CW_NS_CMN(list_count)
#define list_hpush _CW_NS_CMN(list_hpush)
#define list_hpop _CW_NS_CMN(list_hpop)
#define list_tpush _CW_NS_CMN(list_tpush)
#define list_tpop _CW_NS_CMN(list_tpop)
#define list_insert_after _CW_NS_CMN(list_insert_after)
#define list_remove _CW_NS_CMN(list_remove)

cw_list_item_t * list_item_new();
void list_item_delete(cw_list_item_t * a_cont);
void * list_item_get(cw_list_item_t * a_cont);
void list_item_set(cw_list_item_t * a_cont, void * a_item);

cw_list_t * list_new();
void list_delete(cw_list_t * a_list);
cw_sint32_t list_count(cw_list_t * a_list);
void list_hpush(cw_list_t * a_list, cw_list_item_t * a_item);
cw_list_item_t * list_hpop(cw_list_t * a_list);
void list_tpush(cw_list_t * a_list, cw_list_item_t * a_item);
cw_list_item_t * list_tpop(cw_list_t * a_list);
void list_insert_after(cw_list_t * a_list,
		       cw_list_item_t * a_in_list,
		       cw_list_item_t * a_to_insert);
cw_list_item_t * list_remove(cw_list_t * a_list,
			     cw_list_item_t * a_to_remove);

#endif /* _LIST_H_ */
