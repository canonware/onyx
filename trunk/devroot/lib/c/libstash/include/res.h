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
 * Current revision: $Revision: 60 $
 * Last modified: $Date: 1998-05-01 14:50:34 -0700 (Fri, 01 May 1998) $
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#ifndef _RESOURCE_H_
#  define _RESOURCE_H_

/* Pseudo-opaque type. */
typedef struct cw_res_s cw_res_t;

struct cw_res_s
{
  cw_bool_t is_malloced;
  cw_rwl_t rw_lock;
  cw_oh_t hash_o;
  FILE * fd;
  char * str;
};

/*
 * Namespace definitions.
 */
#define res_new _CW_NS_CMN(res_new)
#define res_delete _CW_NS_CMN(res_delete)
#define res_merge_file _CW_NS_CMN(res_merge_file)
#define res_merge_list _CW_NS_CMN(res_merge_list)
#define res_get_res_val _CW_NS_CMN(res_get_res_val)
#define res_dump _CW_NS_CMN(res_dump)

cw_res_t * res_new(cw_res_t * a_res_o);
void res_delete(cw_res_t * a_res_o);
cw_bool_t res_merge_file(cw_res_t * a_res_o, char * a_filename);
cw_bool_t res_merge_list(cw_res_t * a_res_o, ...);
char * res_get_res_val(cw_res_t * a_res_o, char * a_res_name);
void res_dump(cw_res_t * a_res_o);

#endif /* _RESOURCE_H_ */
