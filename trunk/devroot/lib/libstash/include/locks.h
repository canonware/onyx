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
 * Locking classes that are built using the primitives in thread.h., as well 
 * as the list class (the main reason these needed split out of thread.h).
 *
 ****************************************************************************/

#ifndef _LOCKS_H_
#define _LOCKS_H_

/* Pseudo-opaque types. */
typedef struct cw_rwl_s cw_rwl_t;
typedef struct cw_jtl_s cw_jtl_t;
typedef struct cw_jtl_tq_el_s cw_jtl_tq_el_t;

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

struct cw_jtl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;

  cw_uint32_t slock_holders;
  cw_uint32_t slock_waiters;
  cw_cnd_t slock_wait;

  cw_uint32_t tlock_holders;
  cw_uint32_t tlock_waiters;
  cw_list_t tlock_wait;

  cw_uint32_t max_dlocks;
  cw_uint32_t dlock_holders;
  cw_uint32_t dlock_waiters;
  cw_cnd_t dlock_wait;

  cw_uint32_t qlock_holders;
  cw_uint32_t qlock_waiters;
  cw_cnd_t qlock_wait;

  cw_uint32_t rlock_holders;
  cw_uint32_t rlock_waiters;
  cw_cnd_t rlock_wait;

  cw_uint32_t wlock_holders;
  cw_uint32_t wlock_waiters;
  cw_cnd_t wlock_wait;

  cw_uint32_t xlock_holders;
  cw_uint32_t xlock_waiters;
  cw_cnd_t xlock_wait;
};

struct cw_jtl_tq_el_s
{
  cw_bool_t is_blocked;
  cw_cnd_t tlock_wait;
};

/* Namespace definitions. */
#define rwl_new _CW_NS_ANY(rwl_new)
#define rwl_delete _CW_NS_ANY(rwl_delete)
#define rwl_rlock _CW_NS_ANY(rwl_rlock)
#define rwl_runlock _CW_NS_ANY(rwl_runlock)
#define rwl_wlock _CW_NS_ANY(rwl_wlock)
#define rwl_wunlock _CW_NS_ANY(rwl_wunlock)

#define jtl_new _CW_NS_ANY(jtl_new)
#define jtl_delete _CW_NS_ANY(jtl_delete)
#define jtl_slock _CW_NS_ANY(jtl_slock)
#define jtl_get_tq_el _CW_NS_ANY(jtl_get_tq_el)
#define jtl_tlock _CW_NS_ANY(jtl_tlock)
#define jtl_s2dlock _CW_NS_ANY(jtl_s2dlock)
#define jtl_2qlock _CW_NS_ANY(jtl_2qlock)
#define jtl_2rlock _CW_NS_ANY(jtl_2rlock)
#define jtl_2wlock _CW_NS_ANY(jtl_2wlock)
#define jtl_2xlock _CW_NS_ANY(jtl_2xlock)
#define jtl_sunlock _CW_NS_ANY(jtl_sunlock)
#define jtl_tunlock _CW_NS_ANY(jtl_tunlock)
#define jtl_dunlock _CW_NS_ANY(jtl_dunlock)
#define jtl_qunlock _CW_NS_ANY(jtl_qunlock)
#define jtl_runlock _CW_NS_ANY(jtl_runlock)
#define jtl_wunlock _CW_NS_ANY(jtl_wunlock)
#define jtl_xunlock _CW_NS_ANY(jtl_xunlock)
#define jtl_get_max_dlocks _CW_NS_ANY(jtl_get_max_dlocks)
#define jtl_set_max_dlocks _CW_NS_ANY(jtl_set_max_dlocks)

/* Function prototypes. */
cw_rwl_t * rwl_new(cw_rwl_t * a_rwl_o);
void rwl_delete(cw_rwl_t * a_rwl_o);
void rwl_rlock(cw_rwl_t * a_rwl_o);
void rwl_runlock(cw_rwl_t * a_rwl_o);
void rwl_wlock(cw_rwl_t * a_rwl_o);
void rwl_wunlock(cw_rwl_t * a_rwl_o);

cw_jtl_t * jtl_new(cw_jtl_t * a_jtl_o);
void jtl_delete(cw_jtl_t * a_jtl_o);
void jtl_slock(cw_jtl_t * a_jtl_o);
cw_jtl_tq_el_t * jtl_get_tq_el(cw_jtl_t * a_jtl_o);
void jtl_tlock(cw_jtl_t * a_jtl_o, cw_jtl_tq_el_t * a_tq_el);
void jtl_s2dlock(cw_jtl_t * a_jtl_o);
void jtl_2qlock(cw_jtl_t * a_jtl_o);
void jtl_2rlock(cw_jtl_t * a_jtl_o);
void jtl_2wlock(cw_jtl_t * a_jtl_o);
void jtl_2xlock(cw_jtl_t * a_jtl_o);
void jtl_sunlock(cw_jtl_t * a_jtl_o);
void jtl_tunlock(cw_jtl_t * a_jtl_o);
void jtl_dunlock(cw_jtl_t * a_jtl_o);
void jtl_qunlock(cw_jtl_t * a_jtl_o);
void jtl_runlock(cw_jtl_t * a_jtl_o);
void jtl_wunlock(cw_jtl_t * a_jtl_o);
void jtl_xunlock(cw_jtl_t * a_jtl_o);
cw_uint32_t jtl_get_max_dlocks(cw_jtl_t * a_jtl_o);
cw_uint32_t jtl_set_max_dlocks(cw_jtl_t * a_jtl_o, cw_uint32_t a_dlocks);

#endif /* _LOCKS_H_ */
