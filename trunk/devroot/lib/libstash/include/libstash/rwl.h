/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Implementation of some complex locking classes.
 *
 * rwl : Read/write lock.  Multiple simultaneous readers are allowed, but
 * only one locker (with no readers) is allowed.  This implementation
 * toggles back and forth between read locks and write locks to assure
 * deterministic locking.
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_rwl_s cw_rwl_t;

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

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to space for a rwl, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a rwl.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_rwl_t *
rwl_new(cw_rwl_t * a_rwl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to a rwl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Destructor.
 *
 ****************************************************************************/
void
rwl_delete(cw_rwl_t * a_rwl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to a rwl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a r-lock.
 *
 ****************************************************************************/
void
rwl_rlock(cw_rwl_t * a_rwl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to a rwl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a r-lock.
 *
 ****************************************************************************/
void
rwl_runlock(cw_rwl_t * a_rwl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to a rwl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Got w-lock.
 *
 ****************************************************************************/
void
rwl_wlock(cw_rwl_t * a_rwl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_rwl : Pointer to a rwl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a w-lock.
 *
 ****************************************************************************/
void
rwl_wunlock(cw_rwl_t * a_rwl);
