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

#define res_p_parse_res _CW_NS_STASH(res_p_parse_res)
cw_bool_t
res_p_parse_res(cw_res_t * a_res, cw_bool_t a_is_file);

#define res_p_char_type _CW_NS_STASH(res_p_char_type)
cw_uint32_t
res_p_char_type(char a_char);

#define res_p_merge_res _CW_NS_STASH(res_p_merge_res)
void
res_p_merge_res(cw_res_t * a_res, char * a_name, char * a_val);
