/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 87 $
 * $Date: 1998-06-23 17:41:44 -0700 (Tue, 23 Jun 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _DEPG_H_
#define _DEPG_H_

/* Pseudo-opaque type. */
typedef struct cw_depg_s cw_depg_t;

struct cw_depg_s
{
  cw_bool_t is_malloced;
  cw_bool_t is_thread_safe;
};

#define depg_new _CW_NS_CMN(depg_new)
#define depg_delete _CW_NS_CMN(depg_delete)
#define depg_ _CW_NS_CMN(depg_)

cw_depg_t * depg_new(cw_depg_t * a_depg_o, cw_bool_t a_is_thread_safe);
void depg_delete(cw_depg_t * a_depg_o);

cw_bool_t depg_update(cw_depg_t * a_depg_o);

cw_bool_t depg_insert_parent_leaf(cw_depg_t * a_depg_o, cw_depgn_t * a_depgn_o);

#endif /* _DEPG_H_ */
