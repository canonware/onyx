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
#include "../include/libstil/gcdict_l.h"
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stils_l.h"
#include "../include/libstil/stilt_l.h"

#ifdef _LIBSTIL_DBG
#define	_CW_STILA_MAGIC		0x63935743
#endif

/*
 * Doing any locking during GC is dangerous and can result in deadlock.  Even
 * using malloc() can cause a deadlock.  Therefore, the diagnostic messages are
 * dangerous in that they print, so if deadlocks occur while they're on, don't
 * be surprised.
 */
#ifdef _LIBSTIL_CONFESS
/* Print tree traversal information for root set acquisition if defined. */
#define	_LIBSTIL_STILA_REF_ITER
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
	cw_stilo_t		key, *array;
	cw_stilo_t		t_new, t_current, t_maximum;
	cw_stilo_t		t_active, t_threshold;
	cw_stilo_t		sum_count;
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

		ql_new(&a_stila->seq_set);
		a_stila->white = FALSE;
		mq_new(&a_stila->gc_mq, NULL, sizeof(cw_stilam_t));
		try_stage = 4;

#ifdef _LIBSTIL_DBG
		a_stila->magic = _CW_STILA_MAGIC;
#endif

		/*
		 * Create temporary variables for bootstrapping, since gcdict
		 * initialization will hit them.
		 */
		stilo_integer_new(&t_new, 0);
		a_stila->gcdict_new = &t_new;

		stilo_integer_new(&t_current, 0);
		a_stila->gcdict_current = &t_current;

		stilo_integer_new(&t_maximum, 0);
		a_stila->gcdict_maximum = &t_maximum;

		stilo_integer_new(&sum_count, 0);
		a_stila->gcdict_sum = &sum_count;

		stilo_boolean_new(&t_active, FALSE);
		a_stila->gcdict_active = &t_active;

		stilo_integer_new(&t_threshold, 0);
		a_stila->gcdict_threshold = &t_threshold;

		/* Initialize gcdict. */
		gcdict_l_populate(&a_stila->gcdict, a_stila);
		try_stage = 5;

		/*
		 * Cache pointers to gcdict objects and set their initial
		 * values.
		 */
		/* collections. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_collections),
		    stiln_len(STILN_collections), TRUE);
		a_stila->gcdict_collections =
		    stilo_l_dict_lookup(&a_stila->gcdict, &key);

		/* new. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_new),
		    stiln_len(STILN_new), TRUE);
		a_stila->gcdict_new = stilo_l_dict_lookup(&a_stila->gcdict,
		    &key);
		stilo_integer_set(a_stila->gcdict_new,
		    stilo_integer_get(&t_new));

		/* current. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_current),
		    stiln_len(STILN_current), TRUE);
		array = stilo_l_dict_lookup(&a_stila->gcdict, &key);
		a_stila->gcdict_current = stilo_l_array_get(array);
		stilo_integer_set(a_stila->gcdict_current,
		    stilo_integer_get(&t_current));

		/* maximum. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_maximum),
		    stiln_len(STILN_maximum), TRUE);
		array = stilo_l_dict_lookup(&a_stila->gcdict, &key);
		a_stila->gcdict_maximum = stilo_l_array_get(array);
		stilo_integer_set(a_stila->gcdict_maximum,
		    stilo_integer_get(&t_maximum));

		/* sum. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_sum),
		    stiln_len(STILN_sum), TRUE);
		array = stilo_l_dict_lookup(&a_stila->gcdict, &key);
		a_stila->gcdict_sum = stilo_l_array_get(array);
		stilo_integer_set(a_stila->gcdict_sum,
		    stilo_integer_get(&sum_count));

		/* active. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_active),
		    stiln_len(STILN_active), TRUE);
		a_stila->gcdict_active = stilo_l_dict_lookup(&a_stila->gcdict,
		    &key);

		/* period. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_period),
		    stiln_len(STILN_period), TRUE);
		a_stila->gcdict_period = stilo_l_dict_lookup(&a_stila->gcdict,
		    &key);

		/* threshold. */
		stilo_name_new(&key, a_stil, stiln_str(STILN_threshold),
		    stiln_len(STILN_threshold), TRUE);
		a_stila->gcdict_threshold =
		    stilo_l_dict_lookup(&a_stila->gcdict, &key);

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
	}
	xep_catch(_CW_XEPV_OOM) {
		switch (try_stage) {
		case 6:
		case 5:
		case 4:
			mq_delete(&a_stila->gc_mq);
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
	stilo_file_output(file, "active: [s|w:10]\n",
	    stilo_boolean_get(a_stila->gcdict_active) ? "true" : "false");
	stilo_file_output(file, "period: [q|w:10]\n",
	    stilo_integer_get(a_stila->gcdict_period));
	stilo_file_output(file, "threshold: [q|w:7]\n",
	    stilo_integer_get(a_stila->gcdict_threshold));
	stilo_file_output(file, "collections: [q|w:5]\n",
	    stilo_integer_get(a_stila->gcdict_collections));
	stilo_file_output(file, "new:     [q|w:9]\n",
	    stilo_integer_get(a_stila->gcdict_new));
	stilo_file_output(file,
	    "current: [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    stilo_integer_get(&a_stila->gcdict_current[0]),
	    stilo_integer_get(&a_stila->gcdict_current[1]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_current[1]) % 1000000,
	    stilo_integer_get(&a_stila->gcdict_current[2]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_current[2]) % 1000000);
	stilo_file_output(file,
	    "maximum: [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    stilo_integer_get(&a_stila->gcdict_maximum[0]),
	    stilo_integer_get(&a_stila->gcdict_maximum[1]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_maximum[1]) % 1000000,
	    stilo_integer_get(&a_stila->gcdict_maximum[2]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_maximum[2]) % 1000000);
	stilo_file_output(file,
	    "sum:     [q|w:9] [q|w:5].[q|w:6|p:0] [q|w:5].[q|w:6|p:0]\n",
	    stilo_integer_get(&a_stila->gcdict_sum[0]),
	    stilo_integer_get(&a_stila->gcdict_sum[1]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_sum[1]) % 1000000,
	    stilo_integer_get(&a_stila->gcdict_sum[2]) / 1000000,
	    stilo_integer_get(&a_stila->gcdict_sum[2]) % 1000000);
	stilo_file_buffer_flush(file);
	mtx_unlock(&a_stila->lock);
}

cw_bool_t
stila_active_get(cw_stila_t *a_stila)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = stilo_boolean_get(a_stila->gcdict_active);
	mtx_unlock(&a_stila->lock);
	
	return retval;
}

void
stila_active_set(cw_stila_t *a_stila, cw_bool_t a_active)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	/*
	 * Under normal circumstances, calling stilo_dict_def() while holding
	 * the stila lock would be very dangerous, but since we know that we're
	 * just replacing the value of an existing definition, there will be no
	 * new allocation, which would cause GC deadlock.
	 */
	mtx_lock(&a_stila->lock);
	stilo_boolean_set(a_stila->gcdict_active, a_active);
	mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
	mtx_unlock(&a_stila->lock);
}

cw_uint32_t
stila_period_get(cw_stila_t *a_stila)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = stilo_integer_get(a_stila->gcdict_period);
	mtx_unlock(&a_stila->lock);
	
	return retval;
}

void
stila_period_set(cw_stila_t *a_stila, cw_uint32_t a_period)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	/*
	 * Under normal circumstances, calling stilo_dict_def() while holding
	 * the stila lock would be very dangerous, but since we know that we're
	 * just replacing the value of an existing definition, there will be no
	 * new allocation, which would cause GC deadlock.
	 */
	mtx_lock(&a_stila->lock);
	stilo_integer_set(a_stila->gcdict_period, a_period);
	mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
	mtx_unlock(&a_stila->lock);
}

cw_uint32_t
stila_threshold_get(cw_stila_t *a_stila)
{
	cw_uint32_t	retval;

	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	mtx_lock(&a_stila->lock);
	retval = stilo_integer_get(a_stila->gcdict_threshold);
	mtx_unlock(&a_stila->lock);
	
	return retval;
}

void
stila_threshold_set(cw_stila_t *a_stila, cw_uint32_t a_threshold)
{
	_cw_check_ptr(a_stila);
	_cw_assert(a_stila->magic == _CW_STILA_MAGIC);

	/*
	 * Under normal circumstances, calling stilo_dict_def() while holding
	 * the stila lock would be very dangerous, but since we know that we're
	 * just replacing the value of an existing definition, there will be no
	 * new allocation, which would cause GC deadlock.
	 */
	mtx_lock(&a_stila->lock);
	stilo_integer_set(a_stila->gcdict_threshold, a_threshold);
	if (a_threshold <= stilo_integer_get(a_stila->gcdict_new))
		mq_put(&a_stila->gc_mq, STILAM_COLLECT);
	else
		mq_put(&a_stila->gc_mq, STILAM_RECONFIGURE);
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
	stilo_integer_set(a_stila->gcdict_new,
	    stilo_integer_get(a_stila->gcdict_new) + 1);

	/* Update current[0]. */
	stilo_integer_set(&a_stila->gcdict_current[0],
	    stilo_integer_get(&a_stila->gcdict_current[0]) + 1);

	/* Update maximum[0]. */
	if (stilo_integer_get(&a_stila->gcdict_maximum[0]) <
	    stilo_integer_get(&a_stila->gcdict_current[0])) {
		stilo_integer_set(&a_stila->gcdict_maximum[0],
		    stilo_integer_get(&a_stila->gcdict_current[0]));
	}

	/* Update sum[0]. */
	stilo_integer_set(&a_stila->gcdict_sum[0],
	    stilo_integer_get(&a_stila->gcdict_sum[0]) + 1);

	/* Trigger a collection if the threshold was reached. */
	if (stilo_integer_get(a_stila->gcdict_new) ==
	    stilo_integer_get(a_stila->gcdict_threshold) &&
	    stilo_boolean_get(a_stila->gcdict_active) &&
	    stilo_integer_get(a_stila->gcdict_threshold) != 0)
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
	cw_stils_t	*stils;
	cw_stiloe_t	*stiloe, *gray;
	cw_uint32_t	nroot = 0;

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
					nroot++;
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
					nroot++;
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

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[i] non-root reachable object[s]\n", nreachable, nreachable == 1 ?
	    "" : "s");
#endif
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
stila_p_collect(cw_stila_t *a_stila, cw_bool_t a_shutdown)
{
	cw_uint32_t	nroot, nreachable;
	cw_stiloe_t	*garbage;
	struct timeval	t_tv;
	cw_sint64_t	start_us, mark_us, sweep_us;

	/* Record the start time. */
	gettimeofday(&t_tv, NULL);
	start_us = t_tv.tv_sec;
	start_us *= 1000000;
	start_us += t_tv.tv_usec;

	mtx_lock(&a_stila->lock);

#ifdef _LIBSTIL_CONFESS
	/*
	 * Do this before calling thd_single_enter() to avoid potential memory
	 * allocation deadlock.
	 */
	{
		cw_stiloe_t	*p;
		cw_uint32_t	nregistered;

		out_put_e(NULL, NULL, 0, __FUNCTION__, "---------> Start\n");
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[q] registration[s] since last collection\n",
		    stilo_integer_get(a_stila->gcdict_current),
		    stilo_integer_get(a_stila->gcdict_current) == 1 ? "" : "s");

		nregistered = 0;
		qr_foreach(p, ql_first(&a_stila->seq_set), link) {
			nregistered++;
		}
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[i] object[s] registered\n", nregistered, nregistered == 1
		    ? "" : "s");
		_cw_assert(stilo_integer_get(a_stila->gcdict_current) <=
		    nregistered);
	}
#endif
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
	stilo_integer_set(a_stila->gcdict_new, 0);
	stilo_integer_set(a_stila->gcdict_current, nroot + nreachable);

	mtx_unlock(&a_stila->lock);

	/* If there is garbage, discard it. */
	if (garbage != NULL)
		stila_p_sweep(garbage, a_stila->stil);

	if (a_shutdown) {
		/*
		 * All objects were just swept away, so don't update the
		 * statistics, since the storage is already gone for them.
		 */
		goto RETURN;
	}

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
	stilo_integer_set(&a_stila->gcdict_current[1], mark_us);
	if (mark_us > stilo_integer_get(&a_stila->gcdict_maximum[1]))
		stilo_integer_set(&a_stila->gcdict_maximum[1], mark_us);
	stilo_integer_set(&a_stila->gcdict_sum[1],
	    stilo_integer_get(&a_stila->gcdict_sum[1]) + mark_us);

	/* sweep. */
	stilo_integer_set(&a_stila->gcdict_current[2], sweep_us);
	if (sweep_us > stilo_integer_get(&a_stila->gcdict_maximum[2]))
		stilo_integer_set(&a_stila->gcdict_maximum[2], sweep_us);
	stilo_integer_set(&a_stila->gcdict_sum[2],
	    stilo_integer_get(&a_stila->gcdict_sum[2]) + sweep_us);

	/* Increment the collections counter. */
	stilo_integer_set(a_stila->gcdict_collections,
	    stilo_integer_get(a_stila->gcdict_collections) + 1);

	mtx_unlock(&a_stila->lock);

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[q] us mark + [q] us sweep\n", mark_us, sweep_us);
#endif
	RETURN:
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "<--------- Finish\n");
#endif
}

static void *
stila_p_gc_entry(void *a_arg)
{
	cw_stila_t	*stila = (cw_stila_t *)a_arg;
	struct timespec	period;
	cw_stilam_t	message;
	cw_bool_t	shutdown, collect;
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
	 *    registrations for more than gcdict_period seconds.
	 *
	 * 3) Collection was explicitly requested.
	 */
	prev_seq_new = 0;
	period.tv_nsec = 0;
	for (shutdown = FALSE, collect = FALSE; shutdown == FALSE; collect =
	    FALSE) {
		mtx_lock(&stila->lock);
		period.tv_sec = stilo_integer_get(stila->gcdict_period);
		mtx_unlock(&stila->lock);

		if (period.tv_sec > 0) {
			if (mq_timedget(&stila->gc_mq, &period, &message))
				message = STILAM_NONE;
		} else
			mq_get(&stila->gc_mq, &period, &message);

		switch (message) {
		case STILAM_NONE:
			mtx_lock(&stila->lock);
			if (stilo_boolean_get(stila->gcdict_active)) {
				/*
				 * No messages.  Check to see if there
				 * have been any additions to the
				 * sequence set.
				 */
				seq_new = stilo_integer_get(stila->gcdict_new);
			}
			mtx_unlock(&stila->lock);

			if (seq_new > 0) {
				/*
				 * If no additional registrations have
				 * happened since the last mq_timedget()
				 * timeout, collect.
				 */
				if (prev_seq_new == seq_new) {
					stila_p_collect(stila, FALSE);
					prev_seq_new = 0;
				} else
					prev_seq_new = seq_new;
			}
			break;
		case STILAM_COLLECT:
			stila_p_collect(stila, FALSE);
			prev_seq_new = 0;
			break;
		case STILAM_RECONFIGURE:
			/* Don't do anything here. */
			break;
		case STILAM_SHUTDOWN:
			shutdown = TRUE;
			stila_p_collect(stila, TRUE);
			break;
		default:
			_cw_not_reached();
		}
	}

#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "Shut down collector\n");
#endif
	return NULL;
}
