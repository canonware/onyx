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
 * Current revision: $Revision: 42 $
 * Last modified: $Date: 1998-04-26 20:07:12 -0700 (Sun, 26 Apr 1998) $
 *
 * <<< Description >>>
 *
 * Very simple resources class.
 *
 ****************************************************************************
 */

#define _INC_STDARG_H_
#define _INC_RES_H_
#include <config.h>

cw_res_t *
res_new(cw_res_t * a_res_o)
{
  cw_res_t * retval;

  if (a_res_o == NULL)
  {
    retval = (cw_res_t *) _cw_malloc(sizeof(cw_res_t));
    retval->is_malloced = TRUE;
  }
  else
  {
    retval = a_res_o;
    retval->is_malloced = FALSE;
  }

  return retval;
}

void
res_delete(cw_res_t * a_res_o)
{
  _cw_check_ptr(a_res_o);

  /* XXX Free internals here. */
  
  if (a_res_o->is_malloced)
  {
    _cw_free(a_res_o);
  }
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error 
 *
 * <<< Description >>>
 *
 * Merges the resources contained in a_filename into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_file(cw_res_t * a_res_o, char * a_filename)
{
  _cw_check_ptr(a_res_o);
  
  _cw_error("Not implemented.");
  return TRUE;
}

/****************************************************************************
 * <<< Arguments >>>
 *
 * ... : NULL-terminated list of resource name/value pair strings.
 *
 * <<< Return Value >>>
 *
 * TRUE == error
 *
 * <<< Description >>>
 *
 * Merges the resources into the resource database.
 *
 ****************************************************************************/
cw_bool_t
res_merge_list(cw_res_t * a_res_o, ...)
{
  _cw_check_ptr(a_res_o);

  _cw_error("Not implemented.");
  return TRUE;
}

/****************************************************************************
 * <<< Return Value >>>
 *
 * NULL == error
 *
 * <<< Description >>>
 *
 * Returns the value associated with a_res_name if it exists.
 *
 ****************************************************************************/
char *
res_get_res_val(cw_res_t * a_res_o, char * a_res_name)
{
  _cw_check_ptr(a_res_o);
  _cw_check_ptr(a_res_name);

  _cw_error("Not implemented.");
  return NULL;
}
		
/****************************************************************************
 * <<< Description >>>
 *
 * Dumps the resource database.  Since the database is 
 *
 ****************************************************************************/
void
res_dump(cw_res_t * a_res_o)
{
  _cw_check_ptr(a_res_o);

  _cw_error("Not implemented.");
}

