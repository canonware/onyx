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
 * Implementation of thread locking primitives.
 *
 * mtx : Mutex.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_mtx_s cw_mtx_t;

struct cw_mtx_s
{
  cw_bool_t is_malloced;
  pthread_mutex_t mutex;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * Pointer to space for a mtx, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a mtx.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_mtx_t *
mtx_new(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
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
mtx_delete(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Lock a_mtx.
 *
 ****************************************************************************/
void
mtx_lock(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * retval : FALSE == success, TRUE == failure.
 *
 * <<< Description >>>
 *
 * Try to lock a_mtx, but return immediately instead of blocking if a_mtx is
 * already locked.
 *
 ****************************************************************************/
cw_bool_t
mtx_trylock(cw_mtx_t * a_mtx);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_mtx : Pointer to a mtx.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Unlock a_mtx.
 *
 ****************************************************************************/
void
mtx_unlock(cw_mtx_t * a_mtx);
