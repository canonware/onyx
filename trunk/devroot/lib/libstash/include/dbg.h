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
 * $Revision: 125 $
 * $Date: 1998-07-02 16:55:52 -0700 (Thu, 02 Jul 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _DBG_H_
#define _DBG_H_

/* Debug flags (columns).  Use these with calls to dbg_turn_on() and
 * dbg_turn_off(). */
#define _CW_DBG_C_DBG 0
#define _CW_DBG_C_FUNC 1
#define _CW_DBG_C_ERROR 2
#define _CW_DBG_C_OH_SLOT 3
#define _CW_DBG_C_RES_STATE 4
#define _CW_DBG_C_BR_FUNC 5
#define _CW_DBG_C_BRBLK_FUNC 6
#define _CW_DBG_C_BRBS_FUNC 7
#define _CW_DBG_C_BRBS_INIT 8
#define _CW_DBG_C_BRBS_ERROR 9
/* <ADD> */

/* Filters (rows).  Use these with calls to dbg_fmatch() and dbg_pmatch(). */
#define _CW_DBG_R_DBG 0
#define _CW_DBG_R_FUNC 1
#define _CW_DBG_R_ERROR 2
#define _CW_DBG_R_OH_SLOT 3
#define _CW_DBG_R_RES_STATE 4
#define _CW_DBG_R_BR_FUNC 5
#define _CW_DBG_R_BRBLK_FUNC 6
#define _CW_DBG_R_BRBS_FUNC 7
#define _CW_DBG_R_BRBS_INIT 8
#define _CW_DBG_R_BRBS_ERROR 9
/* <ADD> */

/* Put these here only because they're related to the above macros.  They
 * aren't directly useful to the caller. */
#define _CW_DBG_C_MAX 9 /* Highest numbered column in table. */
#define _CW_DBG_R_MAX 9 /* Highest numbered row in table. */
/* <ADD> */

typedef struct cw_dbg_s cw_dbg_t;

struct cw_dbg_s
{
  cw_rwl_t rw_lock;
  cw_bool_t curr_settings[_CW_DBG_C_MAX + 1];
  cw_bool_t fmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t pmatch[_CW_DBG_R_MAX + 1];
  cw_bool_t tbl[_CW_DBG_C_MAX + 1][_CW_DBG_R_MAX + 1];
};

#define dbg_new _CW_NS_CMN(dbg_new)
#define dbg_delete _CW_NS_CMN(dbg_delete)
#define dbg_turn_on _CW_NS_CMN(dbg_turn_on)
#define dbg_turn_off _CW_NS_CMN(dbg_turn_off)
#define dbg_clear _CW_NS_CMN(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * a_dbg_o);
#ifdef _CW_DEBUG
#  define dbg_fmatch(a, b) (a)->fmatch[(b)]
#  define dbg_pmatch(a, b) (a)->pmatch[(b)]
#else
#  define dbg_fmatch(a, b) (0)
#  define dbg_pmatch(a, b) (0)
#endif
void dbg_turn_on(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_turn_off(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_clear(cw_dbg_t * a_dbg_o);

#endif /* _DBG_H_ */
