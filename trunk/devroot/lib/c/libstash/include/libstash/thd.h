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
 * thd : Thread.
 *
 ****************************************************************************/

/* Pseudo-opaque type. */
typedef struct cw_thd_s cw_thd_t;

struct cw_thd_s
{
  cw_bool_t is_malloced;
  pthread_t thread;
};

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to space for a thd, or NULL.
 *
 * a_start_func : Pointer to a start function.
 *
 * <<< Output(s) >>>
 *
 * retval : Pointer to a thd, or NULL.
 *          NULL : Memory allocation error.  Can only occur if (NULL == a_thd).
 *
 * <<< Description >>>
 *
 * Constructor (creates a new thread).
 *
 ****************************************************************************/
cw_thd_t *
thd_new(cw_thd_t * a_thd, void * (*a_start_func)(void *), void * a_arg);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to a thd.
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
thd_delete(cw_thd_t * a_thd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a_thd : Pointer to a thd.
 *
 * <<< Output(s) >>>
 *
 * retval : Return value from thread entry function.
 *
 * <<< Description >>>
 *
 * Join (wait for) the thread associated with a_thd.
 *
 ****************************************************************************/
void *
thd_join(cw_thd_t * a_thd);

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * None.
 *
 * <<< Output(s) >>>
 *
 * None.
 *
 * <<< Description >>>
 *
 * Give up the rest of this thread's time slice.
 *
 ****************************************************************************/
#define thd_yield() sched_yield()

/****************************************************************************
 *
 * <<< Input(s) >>>
 *
 * a : SIG_BLOCK   : Block signals in b.
 *     SIG_UNBLOCK : Unblock signals in b.
 *     SIG_SETMASK : Set signal mask to b.
 *
 * b : Pointer to a signal set (sigset_t *).
 *
 * <<< Output(s) >>>
 *
 * retval : Always zero, unless the arguments are invalid.
 *
 * <<< Description >>>
 *
 * Set the current thread's signal mask.
 *
 ****************************************************************************/
#define thd_sigmask(a, b) pthread_sigmask((a), (b), NULL)
