/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * nxa uses a Baker's Treadmill (an elegant form of mark and sweep) to
 * implement garbage collection.  The collector is atomic, which means that all
 * "mutator" threads must be suspended during the marking phase.  Although no
 * other threads can do any work during marking, the collector does not have to
 * (nor is it allowed to) do any locking, which is a significant performance
 * gain.  The total amount of work that the garbage collector has to do is
 * reduced, which improves overall program throughput.
 *
 * The downside of atomic collection is that if marking takes more than about
 * 100 milliseconds, the user may notice a lag.  In practice, the collector
 * appears to perform very well; marking is fast enough to stay well below the
 * threshold of user awareness, unless memory is low enough that system paging
 * becomes a factor.
 *
 * For situations where a significant amount of allocation is done over a short
 * period of time, or where no allocation whatsoever is happening, the collector
 * can be suspended/resumed/forced in order to improve performance or decrease
 * memory footprint.
 *
 * The following diagram depicts a treadmill as it exists during the mark phase:
 *
 *                                 -------------------\
 *                                                     \
 *                               /--\                   \
 *                               |W |                    \
 *                    /--\       \--/       /--\          \
 *                    |G |        ^         |W |           \
 *                    \--/        |         \--/            \
 *                                |                          \
 *                                white                       \
 *           /--\                                     /--\     \
 *           |G |<-- gray                             |W |      \
 *           \--/                                     \--/       \
 *                                                               |
 *                                                               |
 *                                                               |
 *        /--\                                           /--\   \|/
 *        |B |                                           |W |    V
 *        \--/                                           \--/
 *
 *
 *
 *           /--\                                     /--\
 *           |B |<-- black                            |W |
 *           \--/                                     \--/
 *
 *
 *                    /--\                  /--\
 *                    |W |                  |W |
 *                    \--/       /--\       \--/
 *                               |W |
 *                               \--/
 *
 * Black (fully scanned) objects are all the objects between 'black' (inclusive)
 * and 'gray' (exclusive).
 *
 * Gray (reached, not fully scanned) objects are all the objects
 * between 'gray' (inclusive) and 'white' (exclusive).
 *
 * White (unreached/unreachable) objects are all the objects between 'white'
 * (inclusive) and 'black' (exclusive).
 *
 ******************************************************************************/

#define	_NXA_C_

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>

#include "../include/libonyx/gcdict_l.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_stack_l.h"
#include "../include/libonyx/nxo_thread_l.h"

typedef enum {
	NXAM_NONE,
	NXAM_COLLECT,
	NXAM_RECONFIGURE,
	NXAM_SHUTDOWN
} cw_nxam_t;

static void *nxa_p_gc_entry(void *a_arg);

void
nxa_new(cw_nxa_t *a_nxa, cw_nx_t *a_nx)
{
	sigset_t		sig_mask, old_mask;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	xep_try {
		a_nxa->nx = a_nx;

		mtx_new(&a_nxa->lock);
		try_stage = 1;

		pool_new(&a_nxa->chi_pool, NULL, sizeof(cw_chi_t));
		try_stage = 2;

		pool_new(&a_nxa->dicto_pool, NULL, sizeof(cw_nxoe_dicto_t));
		try_stage = 3;

		pool_new(&a_nxa->stacko_pool, NULL, sizeof(cw_nxoe_stacko_t));
		try_stage = 4;

		ql_new(&a_nxa->seq_set);
		a_nxa->white = FALSE;
		mq_new(&a_nxa->gc_mq, NULL, sizeof(cw_nxam_t));
		try_stage = 5;

#ifdef _CW_DBG
		a_nxa->magic = _CW_NXA_MAGIC;
#endif

		/*
		 * Initialize gcdict state.
		 */
		a_nxa->gcdict_active = FALSE;
		a_nxa->gcdict_period = _CW_LIBONYX_GCDICT_PERIOD;
		a_nxa->gcdict_threshold = _CW_LIBONYX_GCDICT_THRESHOLD;
		a_nxa->gcdict_new = 0;
		memset(a_nxa->gcdict_current, 0, sizeof(cw_nxoi_t) * 3);
		memset(a_nxa->gcdict_maximum, 0, sizeof(cw_nxoi_t) * 3);
		memset(a_nxa->gcdict_sum, 0, sizeof(cw_nxoi_t) * 3);

		/* Initialize gcdict. */
		gcdict_l_populate(&a_nxa->gcdict, a_nxa);
		try_stage = 6;

		/*
		 * Block all signals during thread creation, so that the GC
		 * thread does not receive any signals.  Doing this here rather
		 * than in the GC thread itself avoids a race condition where
		 * signals can be delivered to the GC thread.
		 */
		sigfillset(&sig_mask);
		thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
		a_nxa->gc_thd = thd_new(nxa_p_gc_entry, (void *)a_nxa, TRUE);
		thd_sigmask(SIG_SETMASK, &old_mask, NULL);
		try_stage = 7;
	}
	xep_catch(_CW_STASHX_OOM) {
		switch (try_stage) {
		case 7:
		case 6:
		case 5:
			mq_delete(&a_nxa->gc_mq);
		case 4:
			pool_delete(&a_nxa->stacko_pool);
		case 3:
			pool_delete(&a_nxa->dicto_pool);
		case 2:
			pool_delete(&a_nxa->chi_pool);
		case 1:
			mtx_delete(&a_nxa->lock);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();
}

void
nxa_delete(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mq_put(&a_nxa->gc_mq, NXAM_SHUTDOWN);
	thd_join(a_nxa->gc_thd);
	mq_delete(&a_nxa->gc_mq);

	pool_delete(&a_nxa->stacko_pool);
	pool_delete(&a_nxa->dicto_pool);
	pool_delete(&a_nxa->chi_pool);
}

void
nxa_collect(cw_nxa_t *a_nxa)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mq_put(&a_nxa->gc_mq, NXAM_COLLECT);
}

cw_bool_t
nxa_active_get(cw_nxa_t *a_nxa)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	retval = a_nxa->gcdict_active;
	mtx_unlock(&a_nxa->lock);

	return retval;
}

void
nxa_active_set(cw_nxa_t *a_nxa, cw_bool_t a_active)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	a_nxa->gcdict_active = a_active;
	mq_put(&a_nxa->gc_mq, NXAM_RECONFIGURE);
	mtx_unlock(&a_nxa->lock);
}

cw_nxoi_t
nxa_period_get(cw_nxa_t *a_nxa)
{
	cw_nxoi_t	retval;

	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	retval = a_nxa->gcdict_period;
	mtx_unlock(&a_nxa->lock);

	return retval;
}

void
nxa_period_set(cw_nxa_t *a_nxa, cw_nxoi_t a_period)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	_cw_assert(a_period >= 0);

	mtx_lock(&a_nxa->lock);
	a_nxa->gcdict_period = a_period;
	mq_put(&a_nxa->gc_mq, NXAM_RECONFIGURE);
	mtx_unlock(&a_nxa->lock);
}

cw_nxoi_t
nxa_threshold_get(cw_nxa_t *a_nxa)
{
	cw_nxoi_t	retval;

	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	retval = a_nxa->gcdict_threshold;
	mtx_unlock(&a_nxa->lock);

	return retval;
}

void
nxa_threshold_set(cw_nxa_t *a_nxa, cw_nxoi_t a_threshold)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	_cw_assert(a_threshold >= 0);

	mtx_lock(&a_nxa->lock);
	a_nxa->gcdict_threshold = a_threshold;
	if (a_threshold <= a_nxa->gcdict_new)
		mq_put(&a_nxa->gc_mq, NXAM_COLLECT);
	else
		mq_put(&a_nxa->gc_mq, NXAM_RECONFIGURE);
	mtx_unlock(&a_nxa->lock);
}

void
nxa_stats_get(cw_nxa_t *a_nxa, cw_nxoi_t *r_collections, cw_nxoi_t *r_new,
    cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark, cw_nxoi_t *r_csweep, cw_nxoi_t
    *r_mcount, cw_nxoi_t *r_mmark, cw_nxoi_t *r_msweep, cw_nxoi_t *r_scount,
    cw_nxoi_t *r_smark, cw_nxoi_t *r_ssweep)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);
	
	mtx_lock(&a_nxa->lock);

	/* collections. */
	if (r_collections != NULL)
		*r_collections = a_nxa->gcdict_collections;

	/* new. */
	if (r_new != NULL)
		*r_new = a_nxa->gcdict_new;

	/* current. */
	if (r_ccount != NULL)
		*r_ccount = a_nxa->gcdict_current[0];
	if (r_cmark != NULL)
		*r_cmark = a_nxa->gcdict_current[1];
	if (r_csweep != NULL)
		*r_csweep = a_nxa->gcdict_current[2];

	/* maximum. */
	if (r_mcount != NULL)
		*r_mcount = a_nxa->gcdict_maximum[0];
	if (r_mmark != NULL)
		*r_mmark = a_nxa->gcdict_maximum[1];
	if (r_msweep != NULL)
		*r_msweep = a_nxa->gcdict_maximum[2];

	/* sum. */
	if (r_scount != NULL)
		*r_scount = a_nxa->gcdict_sum[0];
	if (r_smark != NULL)
		*r_smark = a_nxa->gcdict_sum[1];
	if (r_ssweep != NULL)
		*r_ssweep = a_nxa->gcdict_sum[2];

	mtx_unlock(&a_nxa->lock);
}

void
nxa_l_gc_register(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe)
{
	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	_cw_assert(nxoe_l_registered_get(a_nxoe) == FALSE);
	_cw_assert(qr_next(a_nxoe, link) == a_nxoe);
	_cw_assert(qr_prev(a_nxoe, link) == a_nxoe);

	/*
	 * Set the color to white, set the registered bit, and insert into the
	 * object ring.
	 */
	nxoe_l_color_set(a_nxoe, a_nxa->white);
	nxoe_l_registered_set(a_nxoe, TRUE);
	ql_tail_insert(&a_nxa->seq_set, a_nxoe, link);

	/* Update new. */
	a_nxa->gcdict_new++;

	/* Update current[0]. */
	a_nxa->gcdict_current[0]++;

	/* Update maximum[0]. */
	if (a_nxa->gcdict_maximum[0] < a_nxa->gcdict_current[0])
		a_nxa->gcdict_maximum[0] = a_nxa->gcdict_current[0];

	/* Update sum[0]. */
	a_nxa->gcdict_sum[0]++;

	/* Trigger a collection if the threshold was reached. */
	if (a_nxa->gcdict_new == a_nxa->gcdict_threshold &&
	    a_nxa->gcdict_active && a_nxa->gcdict_threshold != 0)
		mq_put(&a_nxa->gc_mq, NXAM_COLLECT);

	mtx_unlock(&a_nxa->lock);
}

cw_bool_t
nxa_l_white_get(cw_nxa_t *a_nxa)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_nxa);
	_cw_dassert(a_nxa->magic == _CW_NXA_MAGIC);

	mtx_lock(&a_nxa->lock);
	retval = a_nxa->white;
	mtx_unlock(&a_nxa->lock);

	return retval;
}

/*
 * Find roots, if any.  Return TRUE if there are roots, FALSE otherwise.
 * Upon return, a_nxa->seq_set points to the first object in the root set.
 */
_CW_INLINE cw_bool_t
nxa_p_roots(cw_nxa_t *a_nxa, cw_uint32_t *r_nroot)
{
	cw_bool_t	retval;
	cw_nxoe_t	*nxoe, *gray;
	cw_uint32_t	nroot = 0;

	/*
	 * Iterate through the root set and mark it gray.
	 *
	 * Each set of *_ref_iter() calls on a particular object must start with
	 * a call with (a_reset == TRUE), and repeated calls until NULL is
	 * returned.
	 */

	/*
	 * Get a root object, so that we can create an invariant for the main
	 * iteration: 'gray' does not point to a white object.
	 */
	for (nxoe = nx_l_ref_iter(a_nxa->nx, TRUE); nxoe != NULL; nxoe =
	    nx_l_ref_iter(a_nxa->nx, FALSE)) {
		
		if (nxoe_l_registered_get(nxoe)) {
			/*
			 * Paint object gray.
			 */
			nroot++;
			_cw_assert(nxoe_l_color_get(nxoe) == a_nxa->white);
			nxoe_l_color_set(nxoe, !a_nxa->white);
			ql_first(&a_nxa->seq_set) = nxoe;
			gray = nxoe;
			goto HAVE_ROOT;
		}
	}
	/*
	 * If we completed the above loop, there are no roots, and therefore we
	 * should not enter the main root set acquisition loop below.
	 */
	retval = FALSE;
	goto RETURN;

	/*
	 * Main root set acquisition loop.
	 */
	HAVE_ROOT:

	/*
	 * Iterate through nxoe's.
	 */
	for (nxoe = nx_l_ref_iter(a_nxa->nx, TRUE); nxoe != NULL; nxoe =
	    nx_l_ref_iter(a_nxa->nx, FALSE)) {
		if (nxoe_l_color_get(nxoe) == a_nxa->white &&
		    nxoe_l_registered_get(nxoe)) {
			nroot++;
			/*
			 * Paint object gray.
			 */
			nxoe_l_color_set(nxoe, !a_nxa->white);
			if (nxoe != qr_next(gray, link)) {
				qr_remove(nxoe, link);
				qr_after_insert(gray, nxoe, link);
			}
			gray = qr_next(gray, link);
		}
	}

	retval = TRUE;
	RETURN:
	*r_nroot = nroot;
	return retval;
}

/*
 * Mark.  Return a pointer to a ring of garbage, if any, otherwise NULL.
 */
_CW_INLINE cw_nxoe_t *
nxa_p_mark(cw_nxa_t *a_nxa, cw_uint32_t *r_nreachable)
{
	cw_nxoe_t	*retval, *gray, *nxoe;
	cw_uint32_t	nreachable = 0;

	/*
	 * Iterate through the gray objects and process them until only black
	 * and white objects are left.
	 */
	gray = ql_first(&a_nxa->seq_set);
	do {
		_cw_assert(nxoe_l_color_get(gray) != a_nxa->white);
		for (nxoe = nxoe_l_ref_iter(gray, TRUE); nxoe != NULL;
		    nxoe = nxoe_l_ref_iter(gray, FALSE)) {
			/* Unregistered nxoe's shouldn't be in the ring. */
			_cw_assert(nxoe_l_registered_get(nxoe));
			/*
			 * If object is white, color it.
			 */
			if (nxoe_l_color_get(nxoe) == a_nxa->white) {
				nxoe_l_color_set(nxoe, !a_nxa->white);
				nreachable++;
				/*
				 * Move the object to the gray region, if it
				 * isn't already adjacent to (and thereby part
				 * of) it.
				 */
				if (nxoe_l_color_get(qr_prev(nxoe, link)) ==
				    a_nxa->white) {
					qr_remove(nxoe, link);
					qr_after_insert(gray, nxoe, link);
				}
			}
		}
		gray = qr_next(gray, link);
	} while (nxoe_l_color_get(gray) != a_nxa->white && gray !=
	    ql_first(&a_nxa->seq_set));
	
	/*
	 * Split the white objects into a separate ring.  If there is garbage,
	 * 'gray' points to the first garbage object in the ring.
	 */
	if (gray != ql_first(&a_nxa->seq_set)) {
		/* Split the ring. */
		qr_split(ql_first(&a_nxa->seq_set), gray, link);
		retval = gray;
	} else
		retval = NULL;

	*r_nreachable = nreachable;
	return retval;
}

/*
 * Clean up unreferenced objects.
 */
_CW_INLINE void
nxa_p_sweep(cw_nxoe_t *a_garbage, cw_nx_t *a_nx)
{
	cw_nxoe_t	*nxoe;

	do {
		nxoe = qr_next(a_garbage, link);
		qr_remove(nxoe, link);
		nxoe_l_delete(nxoe, a_nx);
	} while (nxoe != a_garbage);
}

/*
 * Collect garbage using a Baker's Treadmill.
 */
_CW_INLINE void
nxa_p_collect(cw_nxa_t *a_nxa)
{
	cw_uint32_t	nroot, nreachable;
	cw_nxoe_t	*garbage;
	struct timeval	t_tv;
	cw_nxoi_t	start_us, mark_us, sweep_us;

	/* Record the start time. */
	gettimeofday(&t_tv, NULL);
	start_us = t_tv.tv_sec;
	start_us *= 1000000;
	start_us += t_tv.tv_usec;

	mtx_lock(&a_nxa->lock);

	/* Stop the mutator threads. */
	thd_single_enter();

	/*
	 * Mark the root set gray.  If there are any objects in the root set,
	 * mark all objects reachable from the root set.  Otherwise, everything
	 * is garbage.
	 */
	if (nxa_p_roots(a_nxa, &nroot))
		garbage = nxa_p_mark(a_nxa, &nreachable);
	else {
		garbage = ql_first(&a_nxa->seq_set);
		ql_first(&a_nxa->seq_set) = NULL;
	}

	/* Allow mutator threads to run. */
	thd_single_leave();

	/* Record the mark finish time and calculate mark_us. */
	gettimeofday(&t_tv, NULL);
	mark_us = t_tv.tv_sec;
	mark_us *= 1000000;
	mark_us += t_tv.tv_usec;
	mark_us -= start_us;

	/* Flip the value of white. */
	a_nxa->white = !a_nxa->white;

	/* Update statistics counters. */
	a_nxa->gcdict_new = 0;
	a_nxa->gcdict_current[0] = nroot + nreachable;

	mtx_unlock(&a_nxa->lock);

	/* If there is garbage, discard it. */
	if (garbage != NULL)
		nxa_p_sweep(garbage, a_nxa->nx);

	/* Drain the pools. */
	pool_drain(&a_nxa->chi_pool);
	pool_drain(&a_nxa->dicto_pool);
/*  	pool_drain(&a_nxa->stacko_pool); */

	/* Record the sweep finish time and calculate sweep_us. */
	gettimeofday(&t_tv, NULL);
	sweep_us = t_tv.tv_sec;
	sweep_us *= 1000000;
	sweep_us += t_tv.tv_usec;
	sweep_us -= start_us;
	sweep_us -= mark_us;

	/* Protect statistics updates. */
	mtx_lock(&a_nxa->lock);

	/* Update timing statistics. */
	/* mark. */
	a_nxa->gcdict_current[1] = mark_us;
	if (mark_us > a_nxa->gcdict_maximum[1])
		a_nxa->gcdict_maximum[1] = mark_us;
	a_nxa->gcdict_sum[1] += mark_us;

	/* sweep. */
	a_nxa->gcdict_current[2] = sweep_us;
	if (sweep_us > a_nxa->gcdict_maximum[2])
		a_nxa->gcdict_maximum[2] = sweep_us;
	a_nxa->gcdict_sum[2] += sweep_us;

	/* Increment the collections counter. */
	a_nxa->gcdict_collections++;

	mtx_unlock(&a_nxa->lock);
}

static void *
nxa_p_gc_entry(void *a_arg)
{
	cw_nxa_t	*nxa = (cw_nxa_t *)a_arg;
	struct timespec	period;
	cw_nxam_t	message;
	cw_bool_t	shutdown, collect;
	cw_nxoi_t	seq_new, prev_seq_new;

	/*
	 * Any of the following conditions will cause a collection:
	 *
	 * 1) Enough allocation was done to trigger immediate collection.
	 *
	 * 2) Some registrations were done, followed by a period of no
	 *    registrations for more than gcdict_period seconds.
	 *
	 * 3) Collection was explicitly requested.
	 */
	prev_seq_new = 0;
	period.tv_nsec = 0;
	for (shutdown = FALSE, collect = FALSE; shutdown == FALSE; collect =
	    FALSE) {
		mtx_lock(&nxa->lock);
		period.tv_sec = nxa->gcdict_period;
		mtx_unlock(&nxa->lock);

		if (period.tv_sec > 0) {
			if (mq_timedget(&nxa->gc_mq, &period, &message))
				message = NXAM_NONE;
		} else
			mq_get(&nxa->gc_mq, &period, &message);

		switch (message) {
		case NXAM_NONE:
			mtx_lock(&nxa->lock);
			if (nxa->gcdict_active) {
				/*
				 * No messages.  Check to see if there
				 * have been any additions to the
				 * sequence set.
				 */
				seq_new = nxa->gcdict_new;
			}
			mtx_unlock(&nxa->lock);

			if (seq_new > 0) {
				/*
				 * If no additional registrations have
				 * happened since the last mq_timedget()
				 * timeout, collect.
				 */
				if (prev_seq_new == seq_new) {
					nxa_p_collect(nxa);
					prev_seq_new = 0;
				} else
					prev_seq_new = seq_new;
			}
			break;
		case NXAM_COLLECT:
			nxa_p_collect(nxa);
			prev_seq_new = 0;
			break;
		case NXAM_RECONFIGURE:
			/* Don't do anything here. */
			break;
		case NXAM_SHUTDOWN:
			shutdown = TRUE;
			nxa_p_collect(nxa);
			break;
		default:
			_cw_not_reached();
		}
	}

	return NULL;
}
