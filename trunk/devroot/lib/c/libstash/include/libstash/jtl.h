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
 * jtl : JOE-tree lock.  These are used by the block repository to provide
 * the necessary locking semantics for concurrent JOE-trees.  The following
 * lock types are encapsulated by jtl:
 *   s : Non-serialized place holder lock.
 *   t : Serialized place holder lock.
 *   d : Potential deletion lock (only needed when holding an s lock).
 *   q : Non-exclusive read lock.
 *   r : Non-exclusive read lock.
 *   w : Write lock that allows simultaneous q locks.
 *   x : Exclusive write lock.
 *
 * jtl lock compatibility matrix:
 *
 * (X == compatible)
 * (q == queued, incompatible)
 *
 * | s | t | d | q | r | w | x |
 * +---+---+---+---+---+---+---+--
 * | X |   | X | X | X | X | X | s
 * +---+---+---+---+---+---+---+--
 *     | q |   | X | X | X | X | t
 *     +---+---+---+---+---+---+--
 *         | X | X | X | X | X | d
 *         +---+---+---+---+---+--
 *             | X | X |   |   | q
 *             +---+---+---+---+--
 *                 | X | X |   | r
 *                 +---+---+---+--
 *                     |   |   | w
 *                     +---+---+--
 *                         |   | x
 *                         +---+--
 *
 ****************************************************************************/

/* Pseudo-opaque types. */
typedef struct cw_jtl_s cw_jtl_t;
typedef struct cw_jtl_tq_el_s cw_jtl_tq_el_t;

struct cw_jtl_s
{
  cw_bool_t is_malloced;
  cw_mtx_t lock;

  cw_uint32_t slock_holders;
  cw_uint32_t slock_waiters;
  cw_cnd_t slock_wait;

  cw_uint32_t tlock_holders;
  cw_uint32_t tlock_waiters;
  cw_ring_t * tlock_wait_ring;
  cw_uint32_t tlock_wait_count;

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
  cw_ring_t ring_item;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to space for a jtl, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a jtl.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_jtl_t *
jtl_new(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
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
jtl_delete(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a s-lock.
 *
 ****************************************************************************/
void
jtl_slock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a jtl_tq_el (place holder), or NULL.
 *          NULL : Memory allocation error.
 *
 * <<< Description >>>
 *
 * Reserve a place in line for a t-lock.
 *
 ****************************************************************************/
cw_jtl_tq_el_t *
jtl_get_tq_el(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * a_tq_el : Pointer to a place holder.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a t-lock, using the place holder returned by jtl_get_tq_el().
 *
 ****************************************************************************/
void
jtl_tlock(cw_jtl_t * a_jtl, cw_jtl_tq_el_t * a_tq_el);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Convert an s-lock to an sd-lock.
 *
 ****************************************************************************/
void
jtl_s2dlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a q-lock.
 *
 ****************************************************************************/
void
jtl_2qlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
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
jtl_2rlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a w-lock.
 *
 ****************************************************************************/
void
jtl_2wlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Get a x-lock.
 *
 ****************************************************************************/
void
jtl_2xlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a s-lock.
 *
 ****************************************************************************/
void
jtl_sunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a t-lock.
 *
 ****************************************************************************/
void
jtl_tunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a d-lock.
 *
 ****************************************************************************/
void
jtl_dunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a q-lock.
 *
 ****************************************************************************/
void
jtl_qunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
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
jtl_runlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
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
jtl_wunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Release a x-lock.
 *
 ****************************************************************************/
void
jtl_xunlock(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * retval : Maximum number of d-locks.
 *
 * <<< Description >>>
 *
 * Return the maximum number of d-locks this a_jtl will grant.
 *
 ****************************************************************************/
cw_uint32_t
jtl_get_max_dlocks(cw_jtl_t * a_jtl);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_jtl : Pointer to a jtl.
 *
 * <<< Output(s) >>>
 *
 * retval : Previous maximum number of d-locks.
 *
 * <<< Description >>>
 *
 * Set the maximum number of d-locks a_jtl will grant to a_dlocks and return
 * the old value.
 *
 ****************************************************************************/
cw_uint32_t
jtl_set_max_dlocks(cw_jtl_t * a_jtl, cw_uint32_t a_dlocks);
