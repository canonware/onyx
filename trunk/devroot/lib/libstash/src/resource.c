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
 * Current revision: $Revision: 6 $
 * Last modified: $Date: 1998-01-16 00:20:41 -0800 (Fri, 16 Jan 1998) $
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#define _INC_STDARG_H_
#define _INC_RESOURCE_H_
#include <inc_common.h>

/*
 ****************************************************************************
 * Function: res_t * res_create_context()
 *
 * Arguments: 
 *            
 * Return Value: Pointer to new resource context.
 *               
 * Description: Creates a new resource context and returns a pointer to it.
 *              
 ****************************************************************************
 */
res_t *
res_create_context()
{
  res_t * new_context;
  
  new_context = (res_t *) _cw_malloc(sizeof(res_t));
  _cw_check_ptr(new_context);

  return new_context;
}

/*
 ****************************************************************************
 * Function: void res_destroy_context(res_t * arg_res_context)
 *
 * Arguments: arg_res_context : Pointer to resource context.
 *            
 * Return Value: void
 *               
 * Description: Frees all memory associated with specified resource context.
 *              
 ****************************************************************************
 */
void
res_destroy_context(res_t * arg_res_context)
{
  _cw_free(arg_res_context);
}

/*
 ****************************************************************************
 * Function: int res_merge_file(res_t * arg_res_context, char * arg_filename)
 *
 * Arguments: arg_res_context : Pointer to resource context.
 *            arg_filename : Name of resource file to read.
 *            
 * Return Value: 0 == Okay.
 *               !0 == Error.
 *               
 * Description: Reads a resource file and merges it into the specified
 *              resource context.
 *              
 ****************************************************************************
 */
int
res_merge_file(res_t * arg_res_context, char * arg_filename)
{

  return 0;
}

/*
 ****************************************************************************
 * Function: int res_merge_list(res_t * arg_res_context, ...)
 *
 * Arguments: arg_res_context : Pointer to resource context.
 *            ... : NULL-terminated list of string pairs (name, value).
 *            
 * Return Value: 0 == Okay.
 *               !0 == Error.
 *               
 * Description: Merges a list of resources into a resource context.  Resource
 *              names with empty string values cause that name to be deleted,
 *              only in the case of an exact match.
 *              
 ****************************************************************************
 */
int
res_merge_list(res_t * arg_res_context, ...)
{

  return 0;
}

/*
 ****************************************************************************
 * Function: void res_dump(res_t * arg_res_context)
 *
 * Arguments: arg_res_context : Pointer to resource context.
 *            
 * Return Value: void
 *               
 * Description: Dumps the entire resource context to the log.
 *              
 ****************************************************************************
 */
void
res_dump(res_t * arg_res_context)
{

}

/*
 ****************************************************************************
 * Function: res_val_t res_query(res_t * arg_res_context,
 *                               char * arg_query_string)
 *
 * Arguments: arg_res_context : Pointer to resource context.
 *            arg_query_string : String to find closest match for.
 *            
 * Return Value: Resource value structure.  This structure should not
 *               be directly meddled with.  Use res_val_get_long() and 
 *               res_val_get_str() instead.
 *
 * Description: Finds closest match to arg_query_string and returns a 
 *              res_val_t structure.  The internal string pointer is aliased,
 *              so don't touch the internals.
 *              
 ****************************************************************************
 */
res_val_t
res_query(res_t * arg_res_context,
	  char * arg_query_string)
{
  res_val_t retval;


  return retval;
}

/*
 ****************************************************************************
 * Function: long res_val_get_long(res_val_t * arg_res_val)
 *
 * Arguments: arg_res_val : Resource value structure.
 *            
 * Return Value: Resource value string converted to a long.
 *               
 * Description: The only reason for this function and the extra members in 
 *              the arg_res_val struct is to avoid repeated conversion of the
 *              resource string to a long.  This is likely to be very common
 *              in the code.
 *
 ****************************************************************************
 */
long
res_val_get_long(res_val_t * arg_res_val)
{
  _cw_check_ptr(arg_res_val);

  if (!arg_res_val->valid_long)
    {
      if (arg_res_val->res_string != NULL)
	{
	  arg_res_val->long_val = strtol(arg_res_val->res_string, NULL, 0);
	  arg_res_val->valid_long = 1;
	}
    }

  return arg_res_val->long_val;
}

/*
 ****************************************************************************
 * Function: char * res_val_get_str(res_val_t * arg_res_val)
 *
 * Arguments: arg_res_val : Resource value structure.
 *            
 * Return Value: Pointer to resource value string.
 *               
 * Description: Simply returns a pointer to the internal value string.
 *              
 ****************************************************************************
 */
char *
res_val_get_str(res_val_t * arg_res_val)
{
  _cw_check_ptr(arg_res_val);

  return arg_res_val->res_string;
}
