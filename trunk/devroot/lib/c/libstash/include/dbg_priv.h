/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 92 $
 * $Date: 1998-06-26 01:34:11 -0700 (Fri, 26 Jun 1998) $
 *
 * <<< Description >>>
 *
 * Private header for debug spew code.  Additional debug flags can be added
 * here, in conjunction with dbg.h.
 *
 ****************************************************************************/

#ifndef _DBG_PRIV_H_
#define _DBG_PRIV_H_

/* These are globals so that we don't have to have multiple copies of them,
 * even if there are multiple dbg instances.  We mangle the names to keep
 * them from interfering with other code. */
#define dbg_raw_tbl _CW_NS_CMN(dbg_raw_tbl)
#define dbg_raw_on _CW_NS_CMN(dbg_raw_on)

/* Array used to construct the debug table.  Order *IS* important.  These
 * should be in the same order as the externally visible *_R_* macros for
 * which they are named.  Also, make sure that all entries are terminated
 * by a -1.  The extra -1 at the end is also important. */
cw_sint32_t dbg_raw_tbl[] =
{
  _CW_DBG_C_DBG, -1, /* CW_DBG_R_DBG */
  _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_FUNC */
  _CW_DBG_C_ERROR, -1, /* _CW_DBG_R_ERROR */
  _CW_DBG_C_OH_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_OH_FUNC */
  _CW_DBG_C_OH_SLOT, -1, /* _CW_DBG_R_OH_SLOT */
  _CW_DBG_C_RES_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_RES_FUNC */
  _CW_DBG_C_RES_ERROR, _CW_DBG_C_ERROR, -1, /* _CW_DBG_R_RES_ERROR */
  _CW_DBG_C_RES_STATE, -1, /* _CW_DBG_R_RES_STATE */
  _CW_DBG_C_BHP_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BHP_FUNC */
  _CW_DBG_C_LIST_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_LIST_FUNC */
  _CW_DBG_C_BR_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BR_FUNC */
  _CW_DBG_C_BRBLK_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BRBLK_FUNC */
  _CW_DBG_C_BRBS_FUNC, _CW_DBG_C_FUNC, -1, /* _CW_DBG_R_BRBS_FUNC */
  /* <ADD> */
  -1
};

/*
 * Debug flags that are turned on by default.
 */
cw_sint32_t dbg_raw_on[] =
{
  _CW_DBG_C_DBG,
  /* <ADD> */
  -1
};

struct cw_dbg_s
{
  cw_rwl_t rw_lock;
  cw_bool_t is_current;
  cw_bool_t curr_settings[_CW_DBG_C_MAX + 1];
  cw_bool_t fmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t pmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t tbl[_CW_DBG_C_MAX + 1][_CW_DBG_R_MAX + 1];
};

#define dbg_build_tbl _CW_NS_CMN(dbg_build_tbl)
#define dbg_recalc_fpmatch _CW_NS_CMN(dbg_recalc_fpmatch)

void dbg_build_tbl(cw_dbg_t * a_dbg_o);
void dbg_recalc_fpmatch(cw_dbg_t * a_dbg_o);

#endif /* _DBG_PRIV_H_ */
