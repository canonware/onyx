/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * stila uses a Baker's Treadmill (an elegant form of mark and sweep) to
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
 * threshhold of user awareness, unless memory is low enough that system paging
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

#include "../include/libstil/libstil.h"
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stils_l.h"
#include "../include/libstil/stilt_l.h"

#ifdef _LIBSTIL_DBG
#define	_CW_STILA_MAGIC		0x63935743
#endif

/*
 * Doing any locking during GC is dangerous and can result in deadlock.  Even
 * using malloc() can cause a deadlock.  Therefore, some of the diagnostic
 * messages are dangerous in that they print, so if deadlocks occur while
 * they're on, don't be surprised.
 */
#ifdef _LIBSTIL_CONFESS
/* Print tree traversal information for root set acquisition if defined. */
#define	_LIBSTIL_STILA_REF_ITER

/* Print collection timing information if defined. */
#define	_LIBSTIL_STILA_TIME
#endif

#ifndef _LIBSTIL_STILA_TIME
/*  #define	_LIBSTIL_STILA_TIME */
#endif

/* Number of stack elements per memory chunk. */
#define	_CW_STIL_STILSC_COUNT	16

/*
 * Minimum period of registration inactivity before a periodic collection is
 * done (if any registrations have occured since the last collection).  On
 * average, the actual inactivity period will be 1.5 times this, but can range
 * from 1 to 2 times this.
 */
#define	_CW_STILA_INACTIVE	10

/*
 * Number of sequence set additions since last collection that will cause an
 * immediate collection.
 */
#define	_CW_STILA_THRESHHOLD	50000

typedef enum {
	STILAM_THRESHHOLD,
	STILAM_SUSPEND,
	STILAM_RESUME,
	STILAM_SHUTDOWN
} cw_stilam_t;

static void *stila_p_gc_entry(void *a_arg);

void
stila_new(cw_stila_t *a_stila, cw_stil_t *a_stil)
{
	sigset_t		sig_mask, old_mask;
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	xep_try {
		mtx_new(&a_stila->lock);
		try_stage = 1;

		pool_new(&a_stila->chi_pool, NULL, sizeof(cw_chi_t));
		try_stage = 2;

		pool_new(&a_stila->stilsc_pool, NULL,
		    _CW_STILSC_O2SIZEOF(_CW_STIL_STILSC_COUNT));
		try_stage = 3;

		pool_new(&a_stila->dicto_pool, NULL, sizeof(cw_stiloe_dicto_t));
		try_stage = 4;

		ql_new(&a_stila->seq_set);
		a_stila->seq_new = 0;
		a_stila->white = FALSE;
		mq_new(&a_stila->gc_mq, NULL, sizeof(cw_stilam_t));
		try_stage = 5;

		a_stila->stil = a_stil;

		/*
		 * Block all signals during thread creation, so that the GC
		 * thread does not receive any signals.  Doing this here rather
		 * than in the GC thread itself avoids a race condition where
		 * signals can be delivered to the GC thread.
		 */
		sigfillset(&sig_mask);
		thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
		a_stila->gc_thd = thd_new(stila_p_gc_entry, (void *)a_stila);
		thd_sigmask(SIG_SETMASK, &old_mask, NULL);
		try_stage = 6;

#ifdef _LIBSTIL_DBG
		a_stila->magic = _CW_STILA_MAGIC;
#endif
	}
	xep_catch(_CW_XEPV_OOM) {
		switch (try_stage) {
		case 5:
			mq_delete(&a_stila->gc_mq);
		case 4:
			pool_delete(&a_stila->dicto_pool);
		case 3:
			pool_delete(&a_stila->stilsc_pool);
		case 2:
			pool_delete(&a_stila->chi_pool);
		case 1:
			mtx_delete(&a_stila->lock);
			break;
		default:
			_cw_not_reached();
		}
	}
	xep_end();
}

void
stila_delete(cw_stila_t *a_stila)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mq_put(&a_stila->gc_mq, STILAM_SHUTDOWN);
	thd_join(a_stila->gc_thd);
	mq_delete(&a_stila->gc_mq);

	pool_delete(&a_stila->dicto_pool);
	pool_delete(&a_stila->stilsc_pool);
	pool_delete(&a_stila->chi_pool);
}

void
stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	stiloe_l_color_set(a_stiloe, a_stila->white);
	_cw_assert(stiloe_l_registered_get(a_stiloe) == FALSE);
	_cw_assert(qr_next(a_stiloe, link) == a_stiloe);
	_cw_assert(qr_prev(a_stiloe, link) == a_stiloe);
	stiloe_l_registered_set(a_stiloe, TRUE);
	ql_tail_insert(&a_stila->seq_set, a_stiloe, link);
	a_stila->seq_new++;
	if (a_stila->seq_new == _CW_STILA_THRESHHOLD)
		mq_put(&a_stila->gc_mq, STILAM_THRESHHOLD);
	mtx_unlock(&a_stila->lock);
}

void
stila_collect_set(cw_stila_t *a_stila, cw_bool_t a_collect)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mq_put(&a_stila->gc_mq, a_collect ? STILAM_RESUME : STILAM_SUSPEND);
}

cw_bool_t
stila_l_white_get(cw_stila_t *a_stila)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->white;
	mtx_unlock(&a_stila->lock);

	return retval;
}

/*
 * Find roots, if any.  Return TRUE if there are roots, FALSE otherwise.
 * Upon return, a_stila->seq_set points to the first object in the root set.
 */
_CW_INLINE cw_bool_t
stila_p_roots(cw_stila_t *a_stila)
{
	cw_bool_t	retval;
	cw_stilt_t	*stilt;
	cw_stils_t	*stils;
	cw_stiloe_t	*stiloe, *gray;
#ifdef _LIBSTIL_CONFESS
	cw_uint32_t	nroot = 0;
#endif

	/*
	 * Iterate through the root set and mark it gray.  This requires a 3
	 * level loop, due to the relationship:
	 *
	 * stil --> stilt --> stils --> stiloe
	 *
	 * Each set of *_ref_iter() calls on a particular object must start with
	 * a call with (a_reset == TRUE), and repeated calls until NULL is
	 * returned.
	 */
#ifdef _LIBSTIL_STILA_REF_ITER
	out_put_e(NULL, NULL, 0, __FUNCTION__, "v");
#endif

	/*
	 * Get a root object, so that we can create an invariant for the main
	 * iteration: 'gray' does not point to a white object.
	 */

	/*
	 * Iterate through stilt's.
	 */
	for (stilt = stil_l_ref_iter(a_stila->stil, TRUE); stilt != NULL; stilt
	    = stil_l_ref_iter(a_stila->stil, FALSE)) {
		/*
		 * Iterate through stils's.
		 */
#ifdef _LIBSTIL_STILA_REF_ITER
		out_put(NULL, "t");
#endif
		for (stils = stilt_l_ref_iter(stilt, TRUE); stils != NULL; stils
		    = stilt_l_ref_iter(stilt, FALSE)) {
#ifdef _LIBSTIL_STILA_REF_ITER
			out_put(NULL, "s");
#endif
			/*
			 * Iterate through stiloe's on the stils.
			 */
			for (stiloe = stils_l_ref_iter(stils, TRUE); stiloe !=
			    NULL; stiloe = stils_l_ref_iter(stils, FALSE)) {
				if (stiloe_l_registered_get(stiloe)) {
					/*
					 * Paint object gray.
					 */
#ifdef _LIBSTIL_CONFESS
					nroot++;
#endif
#ifdef _LIBSTIL_STILA_REF_ITER
					out_put(NULL, "<R>");
#endif
					_cw_assert(stiloe_l_color_get(stiloe) ==
					    a_stila->white);
					stiloe_l_color_set(stiloe,
					    !a_stila->white);
					ql_first(&a_stila->seq_set) = stiloe;
					gray = stiloe;
					goto HAVE_ROOT;
				}
			}
		}
	}
#ifdef _LIBSTIL_STILA_REF_ITER
	out_put(NULL, "\n");
#endif
	/*
	 * If we completed the above loop, there are no roots, and therefore we
	 * should not enter the main root set acquisition loop below.
	 */
#ifdef _LIBSTIL_CONFESS
	if (ql_first(&a_stila->seq_set) != NULL)
		out_put_e(NULL, NULL, 0, __FUNCTION__, "No objects\n");
	else
		out_put_e(NULL, NULL, 0, __FUNCTION__, "All garbage\n");
#endif
	retval = FALSE;
	goto RETURN;

	/*
	 * Main root set acquisition loop.
	 */
	HAVE_ROOT:

	/*
	 * Iterate through stilt's.
	 */
	for (stilt = stil_l_ref_iter(a_stila->stil, TRUE); stilt != NULL; stilt
	    = stil_l_ref_iter(a_stila->stil, FALSE)) {
		/*
		 * Iterate through stils's.
		 */
#ifdef _LIBSTIL_STILA_REF_ITER
		out_put(NULL, "t");
#endif
		for (stils = stilt_l_ref_iter(stilt, TRUE); stils != NULL; stils
		    = stilt_l_ref_iter(stilt, FALSE)) {
#ifdef _LIBSTIL_STILA_REF_ITER
			out_put(NULL, "s");
#endif
			/*
			 * Iterate through stiloe's on the stils.
			 */
			for (stiloe = stils_l_ref_iter(stils, TRUE); stiloe !=
			    NULL; stiloe = stils_l_ref_iter(stils, FALSE)) {
#ifdef _LIBSTIL_STILA_REF_ITER
				out_put(NULL, "+");
#endif
				if (stiloe_l_color_get(stiloe) ==
				    a_stila->white &&
				    stiloe_l_registered_get(stiloe)) {
#ifdef _LIBSTIL_CONFESS
					nroot++;
#endif
					/*
					 * Paint object gray.
					 */
					stiloe_l_color_set(stiloe,
					    !a_stila->white);
					if (stiloe != qr_next(gray, link)) {
#ifdef _LIBSTIL_STILA_REF_ITER
						out_put(NULL, "<C>");
#endif
						qr_remove(stiloe, link);
						qr_after_insert(gray, stiloe,
						    link);
					}
#ifdef _LIBSTIL_STILA_REF_ITER
					else
						out_put(NULL, "<CW>");
#endif
					gray = qr_next(gray, link);
				}
			}
		}
	}
#ifdef _LIBSTIL_STILA_REF_ITER
	out_put(NULL, "\n");
#endif

	retval = TRUE;
	RETURN:
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "[i] root object[s]\n", nroot,
	    nroot == 1 ? "" : "s");
#endif
	return retval;
}

/*
 * Mark and sweep.  Return a pointer to a ring of garbage, if any, otherwise
 * NULL.
 */
_CW_INLINE cw_stiloe_t *
stila_p_mark(cw_stila_t *a_stila)
{
	cw_stiloe_t	*retval, *gray, *stiloe;
#ifdef _LIBSTIL_CONFESS
	cw_uint32_t	nreachable = 0;
#endif

	/*
	 * Iterate through the gray objects and process them until only black
	 * and white objects are left.
	 */
	gray = ql_first(&a_stila->seq_set);
	do {
		_cw_assert(stiloe_l_color_get(gray) != a_stila->white);
		for (stiloe = stiloe_l_ref_iter(gray, TRUE); stiloe != NULL;
		     stiloe = stiloe_l_ref_iter(gray, FALSE)) {
			/* Unregistered stiloe's shouldn't be in the ring. */
			_cw_assert(stiloe_l_registered_get(stiloe));
			/*
			 * If object is white, color it.
			 */
			if (stiloe_l_color_get(stiloe) == a_stila->white) {
				stiloe_l_color_set(stiloe, !a_stila->white);
#ifdef _LIBSTIL_CONFESS
				nreachable++;
#endif
				/*
				 * Move the object to the gray region, if it
				 * isn't already adjacent to (and thereby part
				 * of) it.
				 */
				if (stiloe_l_color_get(qr_prev(stiloe, link)) ==
				    a_stila->white) {
					qr_remove(stiloe, link);
					qr_after_insert(gray, stiloe, link);
				}
			}
		}
		gray = qr_next(gray, link);
	} while (stiloe_l_color_get(gray) != a_stila->white && gray !=
	    ql_first(&a_stila->seq_set));
	
	/*
	 * Split the white objects into a separate ring.  If there is garbage,
	 * 'gray' points to the first garbage object in the ring.
	 */
	if (gray != ql_first(&a_stila->seq_set)) {
		/* Split the ring. */
		qr_split(ql_first(&a_stila->seq_set), gray, link);
		retval = gray;
	} else
		retval = NULL;

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[i] non-root reachable object[s]\n", nreachable, nreachable == 1 ?
	    "" : "s");
#endif
	return retval;
}

/*
 * Clean up unreferenced objects.
 */
_CW_INLINE void
stila_p_sweep(cw_stiloe_t *a_garbage, cw_stil_t *a_stil)
{
	cw_stiloe_t	*stiloe;
#ifdef _LIBSTIL_CONFESS
	cw_uint32_t	ngarbage = 0;
#endif

	do {
#ifdef _LIBSTIL_CONFESS
		ngarbage++;
#endif
		stiloe = qr_next(a_garbage, link);
		qr_remove(stiloe, link);
		stiloe_l_delete(stiloe, a_stil);
	} while (stiloe != a_garbage);

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[i] garbage object[s]\n", ngarbage, ngarbage == 1 ? "" : "s");
#endif
}

/*
 * Collect garbage using a Baker's Treadmill.
 */
_CW_INLINE void
stila_p_collect(cw_stila_t *a_stila)
{
	cw_stiloe_t	*garbage;
#ifdef _LIBSTIL_STILA_TIME
	struct timeval	start_tv, mark_tv, sweep_tv;
	cw_uint64_t	start_us, mark_us, sweep_us;

	gettimeofday(&start_tv, NULL);
#endif

	/*
	 * Stop the mutator threads.
	 */
	mtx_lock(&a_stila->lock);
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "---------> Start\n");
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[i] registration[s] since last collection\n", a_stila->seq_new,
	    a_stila->seq_new == 1 ? "" : "s");
	{
		cw_stiloe_t	*p;
		cw_uint32_t	nregistered;

		nregistered = 0;
		qr_foreach(p, ql_first(&a_stila->seq_set), link) {
			nregistered++;
		}
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[i] object[s] registered\n", nregistered, nregistered == 1
		    ? "" : "s");
		_cw_assert(a_stila->seq_new <= nregistered);
	}
#endif
	thd_single_enter();

	/*
	 * Mark the root set gray.  If there are any objects in the root set,
	 * mark all objects reachable from the root set.  Otherwise, everything
	 * is garbage.
	 */
	if (stila_p_roots(a_stila))
		garbage = stila_p_mark(a_stila);
	else {
		garbage = ql_first(&a_stila->seq_set);
		ql_first(&a_stila->seq_set) = NULL;
	}

	/* Flip the value of white. */
	a_stila->white = !a_stila->white;

	/* Reset the counter of new sequence set members since collection. */
	a_stila->seq_new = 0;

	/* Allow mutator threads to run. */
	thd_single_leave();
	mtx_unlock(&a_stila->lock);

#ifdef _LIBSTIL_STILA_TIME
	gettimeofday(&mark_tv, NULL);
#endif

	/* If there is garbage, discard it. */
	if (garbage != NULL)
		stila_p_sweep(garbage, a_stila->stil);

#ifdef _LIBSTIL_STILA_TIME
	gettimeofday(&sweep_tv, NULL);
	{

		start_us = start_tv.tv_sec;
		start_us *= 1000000;
		start_us += start_tv.tv_usec;
		
		mark_us = mark_tv.tv_sec;
		mark_us *= 1000000;
		mark_us += mark_tv.tv_usec;

		sweep_us = sweep_tv.tv_sec;
		sweep_us *= 1000000;
		sweep_us += sweep_tv.tv_usec;

		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[q] us mark + [q] us sweep = [q] us\n",
		    mark_us - start_us, sweep_us - mark_us, sweep_us - start_us);
	}
#endif
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "<--------- Finish\n");
#endif
}

static void *
stila_p_gc_entry(void *a_arg)
{
	cw_stila_t	*stila = (cw_stila_t *)a_arg;
	struct timespec	interval = {_CW_STILA_INACTIVE, 0};
	cw_stilam_t	message;
	cw_bool_t	shutdown, collect, suspend;
	cw_uint32_t	seq_new, prev_seq_new;

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "Start collector\n");
#endif

	/*
	 * Any of the following conditions will cause a collection:
	 *
	 * 1) Enough allocation was done to trigger immediate collection.
	 *
	 * 2) Some registrations were done, followed by a period of no
	 *    registrations for more than _CW_STILA_INACTIVE seconds.
	 *
	 * 3) Collection was resumed.  Even if collection wasn't previously
	 *    suspended, this will trigger a collection.
	 */
	prev_seq_new = 0;
	for (shutdown = FALSE, collect = FALSE, suspend = TRUE; shutdown ==
	    FALSE; collect = FALSE) {
		if (mq_timedget(&stila->gc_mq, &interval, &message) == FALSE) {
			switch (message) {
			case STILAM_THRESHHOLD:
				if (suspend == FALSE)
					collect = TRUE;
				break;
			case STILAM_SUSPEND:
				suspend = TRUE;
				break;
			case STILAM_RESUME:
				collect = TRUE;
				suspend = FALSE;
				break;
			case STILAM_SHUTDOWN:
				shutdown = TRUE;
				collect = TRUE;
				break;
			default:
				_cw_not_reached();
			}
		} else if (suspend == FALSE) {
			/*
			 * No messages.  Check to see if there have been any
			 * additions to the sequence set.
			 */
			mtx_lock(&stila->lock);
			seq_new = stila->seq_new;
			mtx_unlock(&stila->lock);

			if (seq_new > 0) {
				/*
				 * If no additional registrations have happened
				 * since the last mq_timedget() timeout,
				 * collect.
				 */
				if (prev_seq_new == seq_new)
					collect = TRUE;
				else
					prev_seq_new = stila->seq_new;
			}
		}

		if (collect) {
			stila_p_collect(stila);
			prev_seq_new = 0;
		}
	}

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "Shut down collector\n");
#endif
	return NULL;
}
