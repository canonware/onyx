/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
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
static cw_bool_t
res_p_parse_res(cw_res_t * a_res, cw_bool_t a_is_file);

/****************************************************************************
 *
 * Returns the "type" of a_char.
 *
 ****************************************************************************/
static cw_uint32_t
res_p_char_type(char a_char);

/****************************************************************************
 *
 * Merges a resource into the hash table, taking care to clean up any
 * entry it replaces.  This is private, because we don't want to trust any
 * methods external to res to give us valid resources.
 *
 ****************************************************************************/
static cw_bool_t
res_p_merge_res(cw_res_t * a_res, const char * a_name, const char * a_val);
