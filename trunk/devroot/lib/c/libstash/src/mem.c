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
 * $Revision: 41 $
 * $Date: 1998-04-26 20:06:13 -0700 (Sun, 26 Apr 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#define _INC_MEM_PRIV_H_
#include <config.h>

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
cw_mem_t *
mem_new()
{
  cw_mem_t * retval;

  /* Calling malloc() directly since we're bootstrapping. */
  retval = (cw_mem_t *) malloc(sizeof(cw_mem_t));
  _cw_check_ptr(retval);

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
mem_delete(cw_mem_t * a_mem_o)
{
  _cw_check_ptr(a_mem_o);

  /* Calling free() directly since we had to bootstrap the structure. */
  free(a_mem_o);
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_malloc(cw_mem_t * a_mem_o, size_t a_size)
{
  void * retval;
  
  _cw_check_ptr(a_mem_o);

  retval = malloc(a_size);
  _cw_check_ptr(retval);

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_calloc(cw_mem_t * a_mem_o, size_t a_number,
		  size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);

  retval = calloc(a_number, a_size);
  _cw_check_ptr(retval);

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void *
mem_realloc(cw_mem_t * a_mem_o, void * a_ptr, size_t a_size)
{
  void * retval;

  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  retval = realloc(a_ptr, a_size);
  _cw_check_ptr(retval);

  return retval;
}

/****************************************************************************
 * <<< Function >>>
 *
 *
 *
 * <<< Arguments >>>
 *
 *
 *
 * <<< Return Value >>>
 *
 *
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/
void
mem_free(cw_mem_t * a_mem_o, void * a_ptr)
{
  _cw_check_ptr(a_mem_o);
  _cw_check_ptr(a_ptr);

  free(a_ptr);
}
