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

#include <inc_common.h>

char * fallbacks1[] = 
{
  "foo.log", "5",
  "foo.Log", "4",
  NULL
};

int
main()
{
  res_t * context = _cw_malloc(sizeof(res_t));
  res_val_t * res_val;

  _cw_check_ptr(context);

  res_merge_list(context, fallbacks1);

  res_val = res_query(context, "foo.log");

  printf("foo.log --> %d\n", res_val_get_long(res_val));
  printf("foo.log --> %s\n", res_val_get_str(res_val));

  /* Try a bogus query */
  printf("bogus --> %d\n", res_val_get_long(&(res_query("bogus"))));
  printf("bogus --> %s\n", res_val_get_str(&(res_query("bogus"))));

  res_destroy_context(context);

  return 0;
}
