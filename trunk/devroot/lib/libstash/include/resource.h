/* -*-mode:c-*-
 ****************************************************************************
 *
 * Copyright (C) 1996-1997 Jason Evans <jasone@canonware.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You can get a copy of the GNU General Public License at
 * http://www.fsf.org/copyleft/gpl.html, or by writing to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 3 $
 * Last modified: $Date: 1997-12-14 22:01:05 -0800 (Sun, 14 Dec 1997) $
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

#endif _RESOURCE_H_
