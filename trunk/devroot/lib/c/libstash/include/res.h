/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * Current revision: $Revision: 86 $
 * Last modified: $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
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
