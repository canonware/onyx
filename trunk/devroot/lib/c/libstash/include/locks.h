/* -*-mode:c-*-
 ****************************************************************************
 *
 * <Copyright>
 *
 ****************************************************************************
 *
 * $Source$
 * $Author: jasone $
 * $Revision: 90 $
 * $Date: 1998-06-24 23:45:26 -0700 (Wed, 24 Jun 1998) $
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
typedef struct cw_jtl_s cw_jtl_t;

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

struct cw_jtl_s
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

#define jtl_new _CW_NS_CMN(jtl_new)
#define jtl_delete _CW_NS_CMN(jtl_delete)
#define jtl_slock _CW_NS_CMN(jtl_slock)
#define jtl_tlock _CW_NS_CMN(jtl_tlock)
#define jtl_s2dlock _CW_NS_CMN(jtl_s2dlock)
#define jtl_s2rlock _CW_NS_CMN(jtl_s2rlock)
#define jtl_s2wlock _CW_NS_CMN(jtl_s2wlock)
#define jtl_s2xlock _CW_NS_CMN(jtl_s2xlock)
#define jtl_t2rlock _CW_NS_CMN(jtl_t2rlock)
#define jtl_t2wlock _CW_NS_CMN(jtl_t2wlock)
#define jtl_t2xlock _CW_NS_CMN(jtl_t2xlock)
#define jtl_sunlock _CW_NS_CMN(jtl_sunlock)
#define jtl_tunlock _CW_NS_CMN(jtl_tunlock)
#define jtl_dunlock _CW_NS_CMN(jtl_dunlock)
#define jtl_runlock _CW_NS_CMN(jtl_runlock)
#define jtl_wunlock _CW_NS_CMN(jtl_wunlock)
#define jtl_xunlock _CW_NS_CMN(jtl_xunlock)
#define jtl_get_dlocks _CW_NS_CMN(jtl_get_dlocks)
#define jtl_set_dlocks _CW_NS_CMN(jtl_set_dlocks)

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

cw_jtl_t * jtl_new(cw_jtl_t * a_jtl_o);
void jtl_delete(cw_jtl_t * a_jtl_o);
void jtl_slock(cw_jtl_t * a_jtl_o);
void jtl_tlock(cw_jtl_t * a_jtl_o);
void jtl_s2dlock(cw_jtl_t * a_jtl_o);
void jtl_s2rlock(cw_jtl_t * a_jtl_o);
void jtl_s2wlock(cw_jtl_t * a_jtl_o);
void jtl_s2xlock(cw_jtl_t * a_jtl_o);
void jtl_t2rlock(cw_jtl_t * a_jtl_o);
void jtl_t2wlock(cw_jtl_t * a_jtl_o);
void jtl_t2xlock(cw_jtl_t * a_jtl_o);
void jtl_sunlock(cw_jtl_t * a_jtl_o);
void jtl_tunlock(cw_jtl_t * a_jtl_o);
void jtl_dunlock(cw_jtl_t * a_jtl_o);
void jtl_runlock(cw_jtl_t * a_jtl_o);
void jtl_wunlock(cw_jtl_t * a_jtl_o);
void jtl_xunlock(cw_jtl_t * a_jtl_o);
cw_uint32_t jtl_get_dlocks(cw_jtl_t * a_jtl_o);
cw_uint32_t jtl_set_dlocks(cw_jtl_t * a_jtl_o, cw_uint32_t a_dlocks);

#endif /* _LOCKS_H_ */
