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
 * $Revision: 181 $
 * $Date: 1998-08-29 21:02:22 -0700 (Sat, 29 Aug 1998) $
 *
 * <<< Description >>>
 *
 *
 *
 ****************************************************************************/

#ifndef _BRF_H_
#define _BRF_H_

/* Pseudo-opaque type. */
typedef struct cw_brf_s cw_brf_t;

struct cw_brf_s
{
  cw_bool_t is_malloced;
  
};

#define brf_new _CW_NS_ANY(brf_new)
#define brf_delete _CW_NS_ANY(brf_delete)
#define brf_destroy _CW_NS_ANY(brf_destroy)
#define brf_open _CW_NS_ANY(brf_open)
#define brf_close _CW_NS_ANY(brf_close)
#define brf_read _CW_NS_ANY(brf_read)
#define brf_write _CW_NS_ANY(brf_write)
#define brf_truncate _CW_NS_ANY(brf_truncate)

#define brf_rlock _CW_NS_ANY(brf_rlock)
#define brf_runlock _CW_NS_ANY(brf_runlock)
#define brf_r2wlock _CW_NS_ANY(brf_r2wlock)
#define brf_w2rlock _CW_NS_ANY(brf_w2rlock)

#define brf_wlock _CW_NS_ANY(brf_wlock)
#define brf_wunlock _CW_NS_ANY(brf_wunlock)

/* #define brf_ _CW_NS_ANY(brf_) */

/* Flags we need to be able to pass into brf_open():
   rlock
   wlock
   create
   truncate (if already exists)
*/

cw_brf_t * brf_new(cw_brf_t * a_brf_o);
void brf_delete(cw_brf_t * a_brf_o);

cw_bool_t brf_destroy(cw_brf_t * a_brf_o);
cw_bool_t brf_open(cw_brf_t * a_brf_o, cw_bool_t a_open_rlocked,
		   cw_bool_t a_open_wlocked, cw_bool_t a_create,
		   cw_bool_t a_truncate);
cw_bool_t brf_close(cw_brf_t * a_brf_o);
brf_read
brf_write
brf_truncate

brf_rlock
brf_runlock
brf_r2wlock
brf_w2rlock

brf_wlock
brf_wunlock


#endif /* _BRF_H_ */
