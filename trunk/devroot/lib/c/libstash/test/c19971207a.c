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
 * Current revision: $Revision: 5 $
 * Last modified: $Date: 1997-12-14 22:02:02 -0800 (Sun, 14 Dec 1997) $
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
