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
 * <<< Description >>>
 *
 * Private definitions for the res class.
 *
 ****************************************************************************/

/****************************************************************************
 * <<< Return Value >>>
 *
 * TRUE == error
 *
 * <<< Description >>>
 *
 * Private method.  Parses the resources contained either in a string or in 
 * a file and inserts the results into the hash table.
 *
 ****************************************************************************/
#define res_p_parse_res _CW_NS_STASH(res_p_parse_res)
cw_bool_t
res_p_parse_res(cw_res_t * a_res, cw_bool_t a_is_file);

/****************************************************************************
 * <<< Description >>>
 *
 * Returns the "type" of a_char.
 *
 ****************************************************************************/
#define res_p_char_type _CW_NS_STASH(res_p_char_type)
cw_uint32_t
res_p_char_type(char a_char);

/****************************************************************************
 * <<< Description >>>
 *
 * Merges a resource into the hash table, taking care to clean up any
 * entry it replaces.  This is private, because we don't want to trust any
 * methods external to res to give us valid resources.
 *
 ****************************************************************************/
#define res_p_merge_res _CW_NS_STASH(res_p_merge_res)
void
res_p_merge_res(cw_res_t * a_res, char * a_name, char * a_val);
