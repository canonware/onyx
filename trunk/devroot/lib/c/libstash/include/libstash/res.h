/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
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

/* Pseudo-opaque type. */
typedef struct cw_res_s cw_res_t;

struct cw_res_s
{
  cw_bool_t is_malloced;
#ifdef _CW_REENTRANT
  cw_rwl_t rw_lock;
#endif
  cw_oh_t hash;
  FILE * fd;
  char * str;
};

/*
 * Namespace definitions.
 */
#define res_new _CW_NS_STASH(res_new)
cw_res_t *
res_new(cw_res_t * a_res);

#define res_delete _CW_NS_STASH(res_delete)
void
res_delete(cw_res_t * a_res);

#define res_clear _CW_NS_STASH(res_clear)
void
res_clear(cw_res_t * a_res);

#define res_is_equal _CW_NS_STASH(res_is_equal)
cw_bool_t
res_is_equal(cw_res_t * a_res, cw_res_t * a_resther);

#define res_merge_file _CW_NS_STASH(res_merge_file)
cw_bool_t
res_merge_file(cw_res_t * a_res, char * a_filename);

#define res_merge_list _CW_NS_STASH(res_merge_list)
cw_bool_t
res_merge_list(cw_res_t * a_res, ...);

#define res_get_res_val _CW_NS_STASH(res_get_res_val)
char *
res_get_res_val(cw_res_t * a_res, char * a_res_name);

#define res_extract_res _CW_NS_STASH(res_extract_res)
cw_bool_t
res_extract_res(cw_res_t * a_res, char * a_res_key,
		char ** a_res_name, char ** a_res_val);

#define res_dump _CW_NS_STASH(res_dump)
cw_bool_t
res_dump(cw_res_t * a_res, char * a_filename);
