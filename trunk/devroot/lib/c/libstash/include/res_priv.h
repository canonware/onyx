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
#define res_p_char_type _CW_NS_STASH(res_p_char_type)
#define res_p_merge_res _CW_NS_STASH(res_p_merge_res)

cw_bool_t res_p_parse_res(cw_res_t * a_res_o, cw_bool_t a_is_file);
cw_uint32_t res_p_char_type(char a_char);
void res_p_merge_res(cw_res_t * a_res_o, char * a_nam, char * a_val);
