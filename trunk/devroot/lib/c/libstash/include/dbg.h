/* -*-mode:c-*-
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
 *
 *
 ****************************************************************************/

#ifndef _DBG_H_
#define _DBG_H_

/* Debug flags (columns).  Use these with calls to dbg_turn_on() and
 * dbg_turn_off(). */
#define _STASH_DBG_C_DBG 0
#define _STASH_DBG_C_FUNC 1
#define _STASH_DBG_C_ERROR 2
#define _STASH_DBG_C_OH_SLOT 3
#define _STASH_DBG_C_RES_STATE 4
#define _STASH_DBG_C_BR_FUNC 5
#define _STASH_DBG_C_BRBLK_FUNC 6
#define _STASH_DBG_C_BRBS_FUNC 7
#define _STASH_DBG_C_BRBS_INIT 8
#define _STASH_DBG_C_BRBS_ERROR 9
#define _STASH_DBG_C_BHP_FUNC 10
#define _STASH_DBG_C_BUF_FUNC 11
#define _STASH_DBG_C_BUFEL_FUNC 12
#define _STASH_DBG_C_LIST_FUNC 13
#define _STASH_DBG_C_MATRIX_FUNC 14
#define _STASH_DBG_C_OH_FUNC 15
#define _STASH_DBG_C_RES_FUNC 16
#define _STASH_DBG_C_SOCK_FUNC 17
#define _STASH_DBG_C_SOCKS_FUNC 18
#define _STASH_DBG_C_RES_ERROR 19
/* <ADD> */

/* Filters (rows).  Use these with calls to dbg_fmatch() and dbg_pmatch(). */
#define _STASH_DBG_R_DBG 0
#define _STASH_DBG_R_FUNC 1
#define _STASH_DBG_R_ERROR 2
#define _STASH_DBG_R_OH_SLOT 3
#define _STASH_DBG_R_RES_STATE 4
#define _STASH_DBG_R_BR_FUNC 5
#define _STASH_DBG_R_BRBLK_FUNC 6
#define _STASH_DBG_R_BRBS_FUNC 7
#define _STASH_DBG_R_BRBS_INIT 8
#define _STASH_DBG_R_BRBS_ERROR 9
#define _STASH_DBG_R_BHP_FUNC 10
#define _STASH_DBG_R_BUF_FUNC 11
#define _STASH_DBG_R_BUFEL_FUNC 12
#define _STASH_DBG_R_LIST_FUNC 13
#define _STASH_DBG_R_MATRIX_FUNC 14
#define _STASH_DBG_R_OH_FUNC 15
#define _STASH_DBG_R_RES_FUNC 16
#define _STASH_DBG_R_SOCK_FUNC 17
#define _STASH_DBG_R_SOCKS_FUNC 18
#define _STASH_DBG_R_RES_ERROR 19
/* <ADD> */

/* Put these here only because they're related to the above macros.  They
 * aren't directly useful to the caller. */
#define _STASH_DBG_C_MAX 19 /* Highest numbered column in table. */
#define _STASH_DBG_R_MAX 19 /* Highest numbered row in table. */
/* <ADD> */

typedef struct cw_dbg_s cw_dbg_t;

struct cw_dbg_s
{
  cw_rwl_t rw_lock;
  cw_bool_t curr_settings[_STASH_DBG_C_MAX + 1];
  cw_bool_t fmatch[_STASH_DBG_R_MAX + 1];
  cw_bool_t pmatch[_STASH_DBG_R_MAX + 1];
  cw_bool_t tbl[_STASH_DBG_C_MAX + 1][_STASH_DBG_R_MAX + 1];
};

#define dbg_new _CW_NS_ANY(dbg_new)
#define dbg_delete _CW_NS_ANY(dbg_delete)
#define dbg_turn_on _CW_NS_ANY(dbg_turn_on)
#define dbg_turn_off _CW_NS_ANY(dbg_turn_off)
#define dbg_clear _CW_NS_ANY(dbg_clear)

cw_dbg_t * dbg_new();
void dbg_delete(cw_dbg_t * a_dbg_o);
#ifdef _STASH_DBG
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
