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
 * tsd : Thread-specific data.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_tsd_s cw_tsd_t;

struct cw_tsd_s
{
  cw_bool_t is_malloced;
  pthread_key_t key;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to space for a tsd, or NULL.
 *
 * a_func : Pointer to a cleanup function, or NULL.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a tsd.
 *
 * <<< Description >>>
 *
 * Constructor.
 *
 ****************************************************************************/
cw_tsd_t *
tsd_new(cw_tsd_t * a_tsd, void (*a_func)(void *));

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
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
tsd_delete(cw_tsd_t * a_tsd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to thread-specific data.
 *
 * <<< Description >>>
 *
 * Get thread-specific data pointer.
 *
 ****************************************************************************/
void *
tsd_get(cw_tsd_t * a_tsd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_tsd : Pointer to a tsd.
 *
 * a_val : Pointer to thread-specific data.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Set thread-specific data pointer.
 *
 ****************************************************************************/
void
tsd_set(cw_tsd_t * a_tsd, void * a_val);
