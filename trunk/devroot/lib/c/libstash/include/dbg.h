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
#define _CW_DBG_C_OH_FUNC 3
#define _CW_DBG_C_OH_SLOT 4
#define _CW_DBG_C_RES_FUNC 5
#define _CW_DBG_C_RES_ERROR 6
#define _CW_DBG_C_RES_STATE 7
#define _CW_DBG_C_BHP_FUNC 8
#define _CW_DBG_C_LIST_FUNC 9
#define _CW_DBG_C_BR_FUNC 10
#define _CW_DBG_C_BRBLK_FUNC 11
/* <ADD> */

/* Filters (rows).  Use these with calls to dbg_fmatch() and dbg_pmatch(). */
#define _CW_DBG_R_DBG 0
#define _CW_DBG_R_FUNC 1
#define _CW_DBG_R_ERROR 2
#define _CW_DBG_R_OH_FUNC 3
#define _CW_DBG_R_OH_SLOT 4
#define _CW_DBG_R_RES_FUNC 5
#define _CW_DBG_R_RES_ERROR 6
#define _CW_DBG_R_RES_STATE 7
#define _CW_DBG_R_BHP_FUNC 8
#define _CW_DBG_R_LIST_FUNC 9
#define _CW_DBG_R_BR_FUNC 10
#define _CW_DBG_R_BRBLK_FUNC 11
/* <ADD> */

/* Put these here only because they're related to the above macros.  They
 * aren't directly useful to the caller. */
#define _CW_DBG_C_MAX 11 /* Highest numbered column in table. */
#define _CW_DBG_R_MAX 11 /* Highest numbered row in table. */
/* <ADD> */

typedef struct cw_dbg_s cw_dbg_t;

#define dbg_new _CW_NS_CMN(dbg_new)
#define dbg_delete _CW_NS_CMN(dbg_delete)
#define dbg_fmatch _CW_NS_CMN(dbg_fmatch)
#define dbg_pmatch _CW_NS_CMN(dbg_pmatch)
#define dbg_turn_on _CW_NS_CMN(dbg_turn_on)
#define dbg_turn_off _CW_NS_CMN(dbg_turn_off)
#define dbg_clear _CW_NS_CMN(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * a_dbg_o);
cw_bool_t dbg_fmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
cw_bool_t dbg_pmatch(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_turn_on(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_turn_off(cw_dbg_t * a_dbg_o, cw_uint32_t a_flag);
void dbg_clear(cw_dbg_t * a_dbg_o);

#endif /* _DBG_H_ */
