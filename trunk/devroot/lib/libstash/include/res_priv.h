/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 86 $
 * $Date: 1998-06-23 17:40:29 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Private definitions for the res class.
 *
 ****************************************************************************/

#ifndef _RES_PRIV_H_
#define _RES_PRIV_H_

#define res_parse_res _CW_NS_CMN(res_parse_res)

cw_bool_t res_parse_res(cw_res_t * a_res_o, cw_bool_t a_is_file);
cw_uint32_t res_char_type(char a_char);
void res_merge_res(cw_res_t * a_res_o, char * a_nam, char * a_val);

#endif /* _RES_PRIV_H_ */
