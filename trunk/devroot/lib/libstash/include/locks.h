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
 * Locking classes that are built using the primitives in thread.h, as well 
 * as the list class (the main reason these needed split out of thread.h).
 *
 ****************************************************************************/

#ifndef _LOCKS_H_
#define _LOCKS_H_

/* Pseudo-opaque types. */
typedef struct cw_lwq_s cw_lwq_t;
typedef struct cw_rwl_s cw_rwl_t;
typedef struct cw_rwq_s cw_rwq_t;
typedef struct cw_btl_s cw_btl_t;

struct cw_lwq_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_uint32_t num_lockers;
  cw_uint32_t num_lock_waiters;
  cw_list_t list;
};

struct cw_rwl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t read_wait;
  cw_cnd_t write_wait;
  cw_uint32_t num_readers;
  cw_uint32_t num_writers;
  cw_uint32_t read_waiters;
  cw_uint32_t write_waiters;
};

struct cw_rwq_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;
  cw_cnd_t read_wait;
  cw_uint32_t num_readers;
  cw_uint32_t read_waiters;

  cw_lwq_t write_waiters;
};

struct cw_btl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;

  cw_rwq_t stlock;

  cw_uint32_t max_dlocks;
  cw_sem_t dlock_sem;

  cw_rwl_t rxlock;

  cw_sem_t wlock_sem;
};

/* Namespace definitions. */
#define lwq_new _CW_NS_CMN(lwq_new)
#define lwq_delete _CW_NS_CMN(lwq_delete)
#define lwq_lock _CW_NS_CMN(lwq_lock)
#define lwq_unlock _CW_NS_CMN(lwq_unlock)
#define lwq_num_waiters _CW_NS_CMN(lwq_num_waiters)
#define lwq_purge_spares _CW_NS_CMN(lwq_purge_spares)

#define rwl_new _CW_NS_CMN(rwl_new)
#define rwl_delete _CW_NS_CMN(rwl_delete)
#define rwl_rlock _CW_NS_CMN(rwl_rlock)
#define rwl_runlock _CW_NS_CMN(rwl_runlock)
#define rwl_wlock _CW_NS_CMN(rwl_wlock)
#define rwl_wunlock _CW_NS_CMN(rwl_wunlock)

#define rwq_new _CW_NS_CMN(rwq_new)
#define rwq_delete _CW_NS_CMN(rwq_delete)
#define rwq_rlock _CW_NS_CMN(rwq_rlock)
#define rwq_runlock _CW_NS_CMN(rwq_runlock)
#define rwq_wlock _CW_NS_CMN(rwq_wlock)
#define rwq_wunlock _CW_NS_CMN(rwq_wunlock)

#define btl_new _CW_NS_CMN(btl_new)
#define btl_delete _CW_NS_CMN(btl_delete)
#define btl_slock _CW_NS_CMN(btl_slock)
#define btl_tlock _CW_NS_CMN(btl_tlock)
#define btl_s2dlock _CW_NS_CMN(btl_s2dlock)
#define btl_s2rlock _CW_NS_CMN(btl_s2rlock)
#define btl_s2wlock _CW_NS_CMN(btl_s2wlock)
#define btl_s2xlock _CW_NS_CMN(btl_s2xlock)
#define btl_t2rlock _CW_NS_CMN(btl_t2rlock)
#define btl_t2wlock _CW_NS_CMN(btl_t2wlock)
#define btl_t2xlock _CW_NS_CMN(btl_t2xlock)
#define btl_sunlock _CW_NS_CMN(btl_sunlock)
#define btl_tunlock _CW_NS_CMN(btl_tunlock)
#define btl_dunlock _CW_NS_CMN(btl_dunlock)
#define btl_runlock _CW_NS_CMN(btl_runlock)
#define btl_wunlock _CW_NS_CMN(btl_wunlock)
#define btl_xunlock _CW_NS_CMN(btl_xunlock)
#define btl_get_dlocks _CW_NS_CMN(btl_get_dlocks)
#define btl_set_dlocks _CW_NS_CMN(btl_set_dlocks)

/* Function prototypes. */
cw_lwq_t * lwq_new(cw_lwq_t * a_lwq_o);
void lwq_delete(cw_lwq_t * a_lwq_o);
void lwq_lock(cw_lwq_t * a_lwq_o);
void lwq_unlock(cw_lwq_t * a_lwq_o);
cw_sint32_t lwq_num_waiters(cw_lwq_t * a_lwq_o);
void lwq_purge_spares(cw_lwq_t * a_lwq_o);

cw_rwl_t * rwl_new(cw_rwl_t * a_rwl_o);
void rwl_delete(cw_rwl_t * a_rwl_o);
void rwl_rlock(cw_rwl_t * a_rwl_o);
void rwl_runlock(cw_rwl_t * a_rwl_o);
void rwl_wlock(cw_rwl_t * a_rwl_o);
void rwl_wunlock(cw_rwl_t * a_rwl_o);

cw_rwq_t * rwq_new(cw_rwq_t * a_rwq_o);
void rwq_delete(cw_rwq_t * a_rwq_o);
void rwq_rlock(cw_rwq_t * a_rwq_o);
void rwq_runlock(cw_rwq_t * a_rwq_o);
void rwq_wlock(cw_rwq_t * a_rwq_o);
void rwq_wunlock(cw_rwq_t * a_rwq_o);

cw_btl_t * btl_new(cw_btl_t * a_btl_o);
void btl_delete(cw_btl_t * a_btl_o);
void btl_slock(cw_btl_t * a_btl_o);
void btl_tlock(cw_btl_t * a_btl_o);
void btl_s2dlock(cw_btl_t * a_btl_o);
void btl_s2rlock(cw_btl_t * a_btl_o);
void btl_s2wlock(cw_btl_t * a_btl_o);
void btl_s2xlock(cw_btl_t * a_btl_o);
void btl_t2rlock(cw_btl_t * a_btl_o);
void btl_t2wlock(cw_btl_t * a_btl_o);
void btl_t2xlock(cw_btl_t * a_btl_o);
void btl_sunlock(cw_btl_t * a_btl_o);
void btl_tunlock(cw_btl_t * a_btl_o);
void btl_dunlock(cw_btl_t * a_btl_o);
void btl_runlock(cw_btl_t * a_btl_o);
void btl_wunlock(cw_btl_t * a_btl_o);
void btl_xunlock(cw_btl_t * a_btl_o);
cw_uint32_t btl_get_dlocks(cw_btl_t * a_btl_o);
cw_uint32_t btl_set_dlocks(cw_btl_t * a_btl_o, cw_uint32_t a_dlocks);

#endif /* _LOCKS_H_ */
