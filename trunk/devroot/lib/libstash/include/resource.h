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
 * Current revision: $Revision: 7 $
 * Last modified: $Date: 1998-01-16 00:20:54 -0800 (Fri, 16 Jan 1998) $
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

typedef struct
{
  

} res_t;

typedef struct
{
  char valid_long;
  int long_val;
  char * res_string;
} res_val_t;

res_t * res_create_context();
void res_destroy_context(res_t * arg_res_context);
int res_merge_file(res_t * arg_res_context, char * arg_filename);
int res_merge_list(res_t * arg_res_context, ...);
void res_dump(res_t * arg_res_context);
res_val_t res_query(res_t * arg_res_context, char * arg_query_string);
long res_val_get_long(res_val_t * arg_res_val);
char * res_val_get_str(res_val_t * arg_res_val);

#endif /* _RESOURCE_H_ */
