/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 121 $
 * $Date: 1998-07-01 17:22:38 -0700 (Wed, 01 Jul 1998) $
 *
 * <<< Description >>>
 *
 * Private definitions for the res class.
 *
 ****************************************************************************/

#ifndef _RES_PRIV_H_
#define _RES_PRIV_H_

#define res_p_parse_res _CW_NS_CMN(res_p_parse_res)
#define res_p_char_type _CW_NS_CMN(res_p_char_type)
#define res_p_merge_res _CW_NS_CMN(res_p_merge_res)

cw_bool_t res_p_parse_res(cw_res_t * a_res_o, cw_bool_t a_is_file);
cw_uint32_t res_p_char_type(char a_char);
void res_p_merge_res(cw_res_t * a_res_o, char * a_nam, char * a_val);

#endif /* _RES_PRIV_H_ */
