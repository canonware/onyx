/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Description: 
 *              
 *              
 *              
 *              
 ****************************************************************************
 */

#ifndef _RES_H_
#  define _RES_H_

/* Pseudo-opaque type. */
typedef struct cw_res_s cw_res_t;

struct cw_res_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_rwl_t rw_lock;
#endif
  cw_oh_t hash_o;
  FILE * fd;
  char * str;
};

/*
 * Namespace definitions.
 */
#define res_new _CW_NS_ANY(res_new)
#define res_delete _CW_NS_ANY(res_delete)
#define res_clear _CW_NS_ANY(res_clear)
#define res_is_equal _CW_NS_ANY(res_is_equal)
#define res_merge_file _CW_NS_ANY(res_merge_file)
#define res_merge_list _CW_NS_ANY(res_merge_list)
#define res_get_res_val _CW_NS_ANY(res_get_res_val)
#define res_dump _CW_NS_ANY(res_dump)

cw_res_t * res_new(cw_res_t * a_res_o);
void res_delete(cw_res_t * a_res_o);
void res_clear(cw_res_t * a_res_o);
cw_bool_t res_is_equal(cw_res_t * a_res_o, cw_res_t * a_res_other);
cw_bool_t res_merge_file(cw_res_t * a_res_o, char * a_filename);
cw_bool_t res_merge_list(cw_res_t * a_res_o, ...);
char * res_get_res_val(cw_res_t * a_res_o, char * a_res_name);
cw_bool_t res_extract_res(cw_res_t * a_res_o, char * a_res_key,
			  char ** a_res_name, char ** a_res_val);
cw_bool_t res_dump(cw_res_t * a_res_o, char * a_filename);

#endif /* _RES_H_ */
