/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 173 $
 * $Date: 1998-08-26 12:34:42 -0700 (Wed, 26 Aug 1998) $
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
#define dbg_raw_tbl _CW_NS_ANY(dbg_raw_tbl)
#define dbg_raw_on _CW_NS_ANY(dbg_raw_on)

/* Array used to construct the debug table.  Order *IS* important.  These
 * should be in the same order as the externally visible *_R_* macros for
 * which they are named.  Also, make sure that all entries are terminated
 * by a -1.  The extra -1 at the end is also important. */
cw_sint32_t dbg_raw_tbl[] =
{
  _STASH_DBG_C_DBG, -1, /* CW_DBG_R_DBG */
  _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_FUNC */
  _STASH_DBG_C_ERROR, -1, /* _STASH_DBG_R_ERROR */
  _STASH_DBG_C_OH_SLOT, -1, /* _STASH_DBG_R_OH_SLOT */
  _STASH_DBG_C_RES_STATE, -1, /* _STASH_DBG_R_RES_STATE */
  _STASH_DBG_C_BR_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BR_FUNC */
  _STASH_DBG_C_BRBLK_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BRBLK_FUNC */
  _STASH_DBG_C_BRBS_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BRBS_FUNC */
  _STASH_DBG_C_BRBS_INIT, -1, /* _STASH_DBG_R_BRBS_INIT */
  _STASH_DBG_C_BRBS_ERROR, -1, /* _STASH_DBG_R_BRBS_ERROR */
  _STASH_DBG_C_BHP_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BHP_FUNC */
  _STASH_DBG_C_BUF_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BUF_FUNC */
  _STASH_DBG_C_BUFEL_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_BUFEL_FUNC */
  _STASH_DBG_C_LIST_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_LIST_FUNC */
  _STASH_DBG_C_MATRIX_FUNC, _STASH_DBG_C_FUNC, -1,/* _STASH_DBG_R_MATRIX_FUNC */
  _STASH_DBG_C_OH_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_OH_FUNC */
  _STASH_DBG_C_RES_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_RES_FUNC */
  _STASH_DBG_C_SOCK_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_SOCK_FUNC */
  _STASH_DBG_C_SOCKS_FUNC, _STASH_DBG_C_FUNC, -1, /* _STASH_DBG_R_SOCKS_FUNC */
  /* <ADD> */
  -1
};

/*
 * Debug flags that are turned on by default.
 */
cw_sint32_t dbg_raw_on[] =
{
  _STASH_DBG_C_DBG,
  _STASH_DBG_C_BRBS_ERROR,
  /* <ADD> */
  -1
};

#define dbg_build_tbl _CW_NS_ANY(dbg_build_tbl)
#define dbg_recalc_fpmatch _CW_NS_ANY(dbg_recalc_fpmatch)

void dbg_build_tbl(cw_dbg_t * a_dbg_o);
void dbg_recalc_fpmatch(cw_dbg_t * a_dbg_o);

#endif /* _DBG_PRIV_H_ */
