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

#include "../include/libstil/libstil.h"

#include <sys/time.h>

#include "../include/libstil/gcdict_l.h"
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_dict_l.h"
#include "../include/libstil/stilo_stack_l.h"
#include "../include/libstil/stilt_l.h"

#ifdef _LIBSTIL_DBG
#define	_CW_STILA_MAGIC		0x63935743
#endif

typedef enum {
	STILAM_NONE,
	STILAM_COLLECT,
	STILAM_RECONFIGURE,
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
		a_stila->stil = a_stil;

		mtx_new(&a_stila->lock);
		try_stage = 1;

		pool_new(&a_stila->chi_pool, NULL, sizeof(cw_chi_t));
		try_stage = 2;

		pool_new(&a_stila->dicto_pool, NULL, sizeof(cw_stiloe_dicto_t));
		try_stage = 3;

		pool_new(&a_stila->stackc_pool, NULL,
		    sizeof(cw_stiloe_stackc_t));
		try_stage = 4;

		ql_new(&a_stila->seq_set);
		a_stila->white = FALSE;
		mq_new(&a_stila->gc_mq, NULL, sizeof(cw_stilam_t));
		try_stage = 5;

#ifdef _LIBSTIL_DBG
		a_stila->magic = _CW_STILA_MAGIC;
#endif

		/*
		 * Initialize gcdict state.
		 */
		a_stila->gcdict_active = FALSE;
		a_stila->gcdict_period = _LIBSTIL_GCDICT_PERIOD;
		a_stila->gcdict_threshold = _LIBSTIL_GCDICT_THRESHOLD;
		a_stila->gcdict_new = 0;
		memset(a_stila->gcdict_current, 0, sizeof(cw_stiloi_t) * 3);
		memset(a_stila->gcdict_maximum, 0, sizeof(cw_stiloi_t) * 3);
		memset(a_stila->gcdict_sum, 0, sizeof(cw_stiloi_t) * 3);

		/* Initialize gcdict. */
		gcdict_l_populate(&a_stila->gcdict, a_stila);
		try_stage = 6;

		/*
		 * Block all signals during thread creation, so that the GC
		 * thread does not receive any signals.  Doing this here rather
		 * than in the GC thread itself avoids a race condition where
		 * signals can be delivered to the GC thread.
		 */
		sigfillset(&sig_mask);
		thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
		a_stila->gc_thd = thd_new(stila_p_gc_entry, (void *)a_stila,
		    TRUE);
		thd_sigmask(SIG_SETMASK, &old_mask, NULL);
		try_stage = 7;
	}
	xep_catch(_CW_STASHX_OOM) {
		switch (try_stage) {
		case 7:
		case 6:
		case 5:
			mq_delete(&a_stila->gc_mq);
		case 4:
			pool_delete(&a_stila->stackc_pool);
		case 3:
			pool_delete(&a_stila->dicto_pool);
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

	pool_delete(&a_stila->stackc_pool);
	pool_delete(&a_stila->dicto_pool);
	pool_delete(&a_stila->chi_pool);
}

void
stila_collect(cw_stila_t *a_stila)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mq_put(&a_stila->gc_mq, STILAM_COLLECT);
}

void
stila_dump(cw_stila_t *a_stila, cw_stilt_t *a_stilt)
{
	cw_stilo_t	*file;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	file = stil_stdout_get(a_stila->stil);

	/*
	 * Print out the contents of gcdict while under the protection of the
	 * stila lock, so that we are sure of printing a consistent snapshot.
	 */
	mtx_lock(&a_stila->lock);
	stilo_file_output(file, "active: [s|w:10]\n", a_stila->gcdict_active ?
	    "true" : "false");
#if (_CW_STILOI_SIZEOF == 8)
	stilo_file_output(file, "period: [q|w:10]\n", a_stila->gcdict_period);
	stilo_file_output(file, "threshold: [q|w:7]\n",
	    a_stila->gcdict_threshold);
	stilo_file_output(file, "collections: [q|w:5]\n",
	    a_stila->gcdict_collections);
	stilo_file_output(file, "new:     [q|w:9]\n", a_stila->gcdict_new);
	stilo_file_output(file,
	    "current: [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    a_stila->gcdict_current[0],
	    a_stila->gcdict_current[1] / 1000000,
	    a_stila->gcdict_current[1] % 1000000,
	    a_stila->gcdict_current[2] / 1000000,
	    a_stila->gcdict_current[2] % 1000000);
	stilo_file_output(file,
	    "maximum: [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    a_stila->gcdict_maximum[0],
	    a_stila->gcdict_maximum[1] / 1000000,
	    a_stila->gcdict_maximum[1] % 1000000,
	    a_stila->gcdict_maximum[2] / 1000000,
	    a_stila->gcdict_maximum[2] % 1000000);
	stilo_file_output(file,
	    "sum:     [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    a_stila->gcdict_sum[0],
	    a_stila->gcdict_sum[1] / 1000000,
	    a_stila->gcdict_sum[1] % 1000000,
	    a_stila->gcdict_sum[2] / 1000000,
	    a_stila->gcdict_sum[2] % 1000000);
	stilo_file_buffer_flush(file);
#else
	stilo_file_output(file, "period: [i|w:10]\n", a_stila->gcdict_period);
	stilo_file_output(file, "threshold: [i|w:7]\n",
	    a_stila->gcdict_threshold);
	stilo_file_output(file, "collections: [i|w:5]\n",
	    a_stila->gcdict_collections);
	stilo_file_output(file, "new:     [i|w:9]\n", a_stila->gcdict_new);
	stilo_file_output(file,
	    "current: [i|w:9] [i|w:5].[i|w:6|p:0] [i|w:5].[i|w:6|p:0]\n",
	    a_stila->gcdict_current[0],
	    a_stila->gcdict_current[1] / 1000000,
	    a_stila->gcdict_current[1] % 1000000,
	    a_stila->gcdict_current[2] / 1000000,
	    a_stila->gcdict_current[2] % 1000000);
	stilo_file_output(file,
	    "maximum: [i|w:9] [i|w:5].[i|w:6|p:0] [i|w:5].[i|w:6|p:0]\n",
	    a_stila->gcdict_maximum[0],
	    a_stila->gcdict_maximum[1] / 1000000,
	    a_stila->gcdict_maximum[1] % 1000000,
	    a_stila->gcdict_maximum[2] / 1000000,
	    a_stila->gcdict_maximum[2] % 1000000);
	stilo_file_output(file,
	    "sum:     [i|w:9] [i|w:5].[i|w:6|p:0] [i|w:5].[i|w:6|p:0]\n",
	    a_stila->gcdict_sum[0],
	    a_stila->gcdict_sum[1] / 1000000,
	    a_stila->gcdict_sum[1] % 1000000,
	    a_stila->gcdict_sum[2] / 1000000,
	    a_stila->gcdict_sum[2] % 1000000);
	stilo_file_buffer_flush(file);
#endif
	mtx_unlock(&a_stila->lock);
}

cw_bool_t
stila_active_get(cw_stila_t *a_stila)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->gcdict_active;
	mtx_unlock(&a_stila->lock);

	return retval;
}

void
stila_active_set(cw_stila_t *a_stila, cw_bool_t a_active)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	a_stila->gcdict_active = a_active;
	mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
	mtx_unlock(&a_stila->lock);
}

cw_stiloi_t
stila_period_get(cw_stila_t *a_stila)
{
	cw_stiloi_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->gcdict_period;
	mtx_unlock(&a_stila->lock);

	return retval;
}

void
stila_period_set(cw_stila_t *a_stila, cw_stiloi_t a_period)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);
	_cw_assert(a_period >= 0);

	mtx_lock(&a_stila->lock);
	a_stila->gcdict_period = a_period;
	mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
	mtx_unlock(&a_stila->lock);
}

cw_stiloi_t
stila_threshold_get(cw_stila_t *a_stila)
{
	cw_stiloi_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->gcdict_threshold;
	mtx_unlock(&a_stila->lock);

	return retval;
}

void
stila_threshold_set(cw_stila_t *a_stila, cw_stiloi_t a_threshold)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);
	_cw_assert(a_threshold >= 0);

	mtx_lock(&a_stila->lock);
	a_stila->gcdict_threshold = a_threshold;
	if (a_threshold <= a_stila->gcdict_new)
		mq_put(&a_stila->gc_mq, STILAM_COLLECT);
	else
		mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
	mtx_unlock(&a_stila->lock);
}

cw_stiloi_t
stila_collections_get(cw_stila_t *a_stila)
{
	cw_stiloi_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->gcdict_collections;
	mtx_unlock(&a_stila->lock);

	return retval;
}

cw_stiloi_t
stila_new_get(cw_stila_t *a_stila)
{
	cw_stiloi_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = a_stila->gcdict_new;
	mtx_unlock(&a_stila->lock);

	return retval;
}

void
stila_current_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t
    *r_mark, cw_stiloi_t *r_sweep)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);
	_cw_check_ptr(r_count);
	_cw_check_ptr(r_mark);
	_cw_check_ptr(r_sweep);
	
	mtx_lock(&a_stila->lock);
	*r_count = a_stila->gcdict_current[0];
	*r_mark = a_stila->gcdict_current[1];
	*r_sweep = a_stila->gcdict_current[2];
	mtx_unlock(&a_stila->lock);
}

void
stila_maximum_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t
    *r_mark, cw_stiloi_t *r_sweep)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);
	_cw_check_ptr(r_count);
	_cw_check_ptr(r_mark);
	_cw_check_ptr(r_sweep);
	
	mtx_lock(&a_stila->lock);
	*r_count = a_stila->gcdict_maximum[0];
	*r_mark = a_stila->gcdict_maximum[1];
	*r_sweep = a_stila->gcdict_maximum[2];
	mtx_unlock(&a_stila->lock);
}

void
stila_sum_get(cw_stila_t *a_stila, cw_stiloi_t *r_count, cw_stiloi_t *r_mark,
    cw_stiloi_t *r_sweep)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);
	_cw_check_ptr(r_count);
	_cw_check_ptr(r_mark);
	_cw_check_ptr(r_sweep);
	
	mtx_lock(&a_stila->lock);
	*r_count = a_stila->gcdict_sum[0];
	*r_mark = a_stila->gcdict_sum[1];
	*r_sweep = a_stila->gcdict_sum[2];
	mtx_unlock(&a_stila->lock);
}

void
stila_l_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	_cw_assert(stiloe_l_registered_get(a_stiloe) == FALSE);
	_cw_assert(qr_next(a_stiloe, link) == a_stiloe);
	_cw_assert(qr_prev(a_stiloe, link) == a_stiloe);

	/*
	 * Set the color to white, set the registered bit, and insert into the
	 * object ring.
	 */
	stiloe_l_color_set(a_stiloe, a_stila->white);
	stiloe_l_registered_set(a_stiloe, TRUE);
	ql_tail_insert(&a_stila->seq_set, a_stiloe, link);

	/* Update new. */
	a_stila->gcdict_new++;

	/* Update current[0]. */
	a_stila->gcdict_current[0]++;

	/* Update maximum[0]. */
	if (a_stila->gcdict_maximum[0] < a_stila->gcdict_current[0])
		a_stila->gcdict_maximum[0] = a_stila->gcdict_current[0];

	/* Update sum[0]. */
	a_stila->gcdict_sum[0]++;

	/* Trigger a collection if the threshold was reached. */
	if (a_stila->gcdict_new == a_stila->gcdict_threshold &&
	    a_stila->gcdict_active && a_stila->gcdict_threshold != 0)
		mq_put(&a_stila->gc_mq, STILAM_COLLECT);

	mtx_unlock(&a_stila->lock);
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
stila_p_roots(cw_stila_t *a_stila, cw_uint32_t *r_nroot)
{
	cw_bool_t	retval;
	cw_stilt_t	*stilt;
	cw_stiloe_t	*stiloe, *gray;
	cw_uint32_t	nroot = 0;

	/*
	 * Iterate through the root set and mark it gray.  This requires a 2
	 * level loop, due to the relationship:
	 *
	 * stil --> stilt --> stiloe
	 *
	 * Each set of *_ref_iter() calls on a particular object must start with
	 * a call with (a_reset == TRUE), and repeated calls until NULL is
	 * returned.
	 */

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
		 * Iterate through stiloe's on the stilt.
		 */
		for (stiloe = stilt_l_ref_iter(stilt, TRUE); stiloe !=
			 NULL; stiloe = stilt_l_ref_iter(stilt, FALSE)) {
			if (stiloe_l_registered_get(stiloe)) {
				/*
				 * Paint object gray.
				 */
				nroot++;
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
	 * Iterate through stilt's.
	 */
	for (stilt = stil_l_ref_iter(a_stila->stil, TRUE); stilt != NULL; stilt
	    = stil_l_ref_iter(a_stila->stil, FALSE)) {
		/*
		 * Iterate through stiloe's on the stilt.
		 */
		for (stiloe = stilt_l_ref_iter(stilt, TRUE); stiloe !=
			 NULL; stiloe = stilt_l_ref_iter(stilt, FALSE)) {
			if (stiloe_l_color_get(stiloe) ==
			    a_stila->white &&
			    stiloe_l_registered_get(stiloe)) {
				nroot++;
				/*
				 * Paint object gray.
				 */
				stiloe_l_color_set(stiloe,
				    !a_stila->white);
				if (stiloe != qr_next(gray, link)) {
					qr_remove(stiloe, link);
					qr_after_insert(gray, stiloe,
					    link);
				}
				gray = qr_next(gray, link);
			}
		}
	}

	retval = TRUE;
	RETURN:
	*r_nroot = nroot;
	return retval;
}

/*
 * Mark and sweep.  Return a pointer to a ring of garbage, if any, otherwise
 * NULL.
 */
_CW_INLINE cw_stiloe_t *
stila_p_mark(cw_stila_t *a_stila, cw_uint32_t *r_nreachable)
{
	cw_stiloe_t	*retval, *gray, *stiloe;
	cw_uint32_t	nreachable = 0;

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
				nreachable++;
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

	*r_nreachable = nreachable;
	return retval;
}

/*
 * Clean up unreferenced objects.
 */
_CW_INLINE void
stila_p_sweep(cw_stiloe_t *a_garbage, cw_stil_t *a_stil)
{
	cw_stiloe_t	*stiloe;

	do {
		stiloe = qr_next(a_garbage, link);
		qr_remove(stiloe, link);
		stiloe_l_delete(stiloe, a_stil);
	} while (stiloe != a_garbage);
}

/*
 * Collect garbage using a Baker's Treadmill.
 */
_CW_INLINE void
stila_p_collect(cw_stila_t *a_stila)
{
	cw_uint32_t	nroot, nreachable;
	cw_stiloe_t	*garbage;
	struct timeval	t_tv;
	cw_stiloi_t	start_us, mark_us, sweep_us;

	/* Record the start time. */
	gettimeofday(&t_tv, NULL);
	start_us = t_tv.tv_sec;
	start_us *= 1000000;
	start_us += t_tv.tv_usec;

	mtx_lock(&a_stila->lock);

	/* Stop the mutator threads. */
	thd_single_enter();

	/*
	 * Mark the root set gray.  If there are any objects in the root set,
	 * mark all objects reachable from the root set.  Otherwise, everything
	 * is garbage.
	 */
	if (stila_p_roots(a_stila, &nroot))
		garbage = stila_p_mark(a_stila, &nreachable);
	else {
		garbage = ql_first(&a_stila->seq_set);
		ql_first(&a_stila->seq_set) = NULL;
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
	a_stila->white = !a_stila->white;

	/* Update statistics counters. */
	a_stila->gcdict_new = 0;
	a_stila->gcdict_current[0] = nroot + nreachable;

	mtx_unlock(&a_stila->lock);

	/* If there is garbage, discard it. */
	if (garbage != NULL)
		stila_p_sweep(garbage, a_stila->stil);

	/* Drain the pools. */
	pool_drain(&a_stila->chi_pool);
	pool_drain(&a_stila->dicto_pool);
	pool_drain(&a_stila->stackc_pool);

	/* Record the sweep finish time and calculate sweep_us. */
	gettimeofday(&t_tv, NULL);
	sweep_us = t_tv.tv_sec;
	sweep_us *= 1000000;
	sweep_us += t_tv.tv_usec;
	sweep_us -= start_us;
	sweep_us -= mark_us;

	/* Protect statistics updates. */
	mtx_lock(&a_stila->lock);

	/* Update timing statistics. */
	/* mark. */
	a_stila->gcdict_current[1] = mark_us;
	if (mark_us > a_stila->gcdict_maximum[1])
		a_stila->gcdict_maximum[1] = mark_us;
	a_stila->gcdict_sum[1] += mark_us;

	/* sweep. */
	a_stila->gcdict_current[2] = sweep_us;
	if (sweep_us > a_stila->gcdict_maximum[2])
		a_stila->gcdict_maximum[2] = sweep_us;
	a_stila->gcdict_sum[2] += sweep_us;

	/* Increment the collections counter. */
	a_stila->gcdict_collections++;

	mtx_unlock(&a_stila->lock);
}

static void *
stila_p_gc_entry(void *a_arg)
{
	cw_stila_t	*stila = (cw_stila_t *)a_arg;
	struct timespec	period;
	cw_stilam_t	message;
	cw_bool_t	shutdown, collect;
	cw_stiloi_t	seq_new, prev_seq_new;

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
		mtx_lock(&stila->lock);
		period.tv_sec = stila->gcdict_period;
		mtx_unlock(&stila->lock);

		if (period.tv_sec > 0) {
			if (mq_timedget(&stila->gc_mq, &period, &message))
				message = STILAM_NONE;
		} else
			mq_get(&stila->gc_mq, &period, &message);

		switch (message) {
		case STILAM_NONE:
			mtx_lock(&stila->lock);
			if (stila->gcdict_active) {
				/*
				 * No messages.  Check to see if there
				 * have been any additions to the
				 * sequence set.
				 */
				seq_new = stila->gcdict_new;
			}
			mtx_unlock(&stila->lock);

			if (seq_new > 0) {
				/*
				 * If no additional registrations have
				 * happened since the last mq_timedget()
				 * timeout, collect.
				 */
				if (prev_seq_new == seq_new) {
					stila_p_collect(stila);
					prev_seq_new = 0;
				} else
					prev_seq_new = seq_new;
			}
			break;
		case STILAM_COLLECT:
			stila_p_collect(stila);
			prev_seq_new = 0;
			break;
		case STILAM_RECONFIGURE:
			/* Don't do anything here. */
			break;
		case STILAM_SHUTDOWN:
			shutdown = TRUE;
			stila_p_collect(stila);
			break;
		default:
			_cw_not_reached();
		}
	}

	return NULL;
}
