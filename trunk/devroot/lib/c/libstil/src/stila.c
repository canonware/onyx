/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
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

/* Print collection timing information if defined. */
/*  #define	_LIBSTIL_STILA_TIME */

/* Number of stack elements per memory chunk. */
#define	_CW_STIL_STILSC_COUNT	16

/*
 * Minimum period of registration inactivity before a periodic collection is
 * done (if any registrations have occured since the last collection).  On
 * average, the actual inactivity period will be 1.5 times this, but can range
 * from 1 to 2 times this.
 */
#define	_CW_STILA_INACTIVE	5

/*
 * Number of sequence set additions since last collection that will cause an
 * immediate collection.
 */
#define	_CW_STILA_THRESHHOLD	2500

typedef enum {
	STILAM_THRESHHOLD,
	STILAM_SUSPEND,
	STILAM_RESUME,
	STILAM_SHUTDOWN
} cw_stilam_t;

static void *stila_p_gc_entry(void *a_arg);
static void stila_p_collect(cw_stila_t *a_stila);

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
	for (shutdown = FALSE, collect = FALSE, suspend = FALSE; shutdown ==
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

/*
 * Collect garbage using a Baker's Treadmill.
 */
static void
stila_p_collect(cw_stila_t *a_stila)
{
	cw_stilt_t	*stilt;
	cw_stils_t	*stils;
	cw_stiloe_t	*stiloe, *black, *gray, *white;
#ifdef _LIBSTIL_STILA_TIME
	struct timeval	before, middle, after;

	gettimeofday(&before, NULL);
#endif

	mtx_lock(&a_stila->lock);
	thd_single_enter();

	/*
	 * Initialize 'gray' and 'white' for root set iteration.
	 */
	gray = white = ql_first(&a_stila->seq_set);

#ifdef _LIBSTIL_CONFESS
	out_put(NULL, "\n");
	out_put_e(NULL, NULL, 0, __FUNCTION__,
	    "[i] registration[s] since last collection\n", a_stila->seq_new,
	    a_stila->seq_new == 1 ? "" : "s");
	{
		cw_stiloe_t	*p;
		cw_uint32_t	nregistered;

		nregistered = 0;
		qr_foreach(p, white, link) {
			nregistered++;
		}
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[i] object[s] registered\n", nregistered, nregistered == 1
		    ? "" : "s");
		_cw_assert(a_stila->seq_new <= nregistered);
	}
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
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "v");
#endif

	/*
	 * Iterate through stilt's.
	 */
	for (stilt = stil_l_ref_iter(a_stila->stil, TRUE); stilt != NULL; stilt
	    = stil_l_ref_iter(a_stila->stil, FALSE)) {
		/*
		 * Iterate through stils's.
		 */
#ifdef _LIBSTIL_CONFESS
		out_put(NULL, "t");
#endif
		for (stils = stilt_l_ref_iter(stilt, TRUE); stils != NULL; stils
		    = stilt_l_ref_iter(stilt, FALSE)) {
#ifdef _LIBSTIL_CONFESS
			out_put(NULL, "s");
#endif
			/*
			 * Iterate through stiloe's on the stils.
			 */
			for (stiloe = stils_l_ref_iter(stils, TRUE); stiloe !=
			    NULL; stiloe = stils_l_ref_iter(stils, FALSE)) {
				/*
				 * Paint object gray.
				 */
#ifdef _LIBSTIL_CONFESS
				out_put(NULL, "+");
#endif
				if (stiloe_l_color_get(stiloe) ==
				    a_stila->white) {
					stiloe_l_color_set(stiloe,
					    !a_stila->white);
					if (stiloe != gray) {
						/*
						 * XXX This reverses the root
						 * set order every collection.
						 * Try to keep the order the
						 * same and avoid snaps.
						 */
						if (stiloe != white) {
#ifdef _LIBSTIL_CONFESS
							out_put(NULL, "<C>");
#endif
							qr_remove(stiloe, link);
							qr_before_insert(gray,
							    stiloe, link);
							gray = qr_prev(gray,
							    link);
						} else {
#ifdef _LIBSTIL_CONFESS
							out_put(NULL, "<CW>");
#endif
							white = qr_next(white,
							    link);
						}
					}
				}

			}
		}
	}
#ifdef _LIBSTIL_CONFESS
	out_put(NULL, "\n");
#endif

	/*
	 * Initialize 'black' to be the same as 'gray'.  There are no black
	 * objects yet.
	 */
	black = gray;

	/*
	 * Black objects are all the objects between 'black' (inclusive) and
	 * 'gray' (exclusive).
	 *
	 * Gray objects are all the objects between 'gray' (inclusive) and
	 * 'white' (exclusive).
	 *
	 * White objects are all the objects between 'white' (inclusive) and
	 * 'black' (exclusive).
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
	 */

#ifdef _LIBSTIL_CONFESS
	{
		cw_stiloe_t	*p;
		cw_uint32_t	nroot, nwhite;

		for (p = gray, nroot = 0; p != white; p = qr_next(p, link),
		 nroot++)
			; /* Do nothing. */

		if (nroot > 0) {
			for (p = white, nwhite = 0; p != black; p = qr_next(p,
			    link), nwhite++)
				; /* Do nothing. */

			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "[i] root + [i] white = [i] object[s]\n", nroot,
			    nwhite, nroot + nwhite, nroot + nwhite == 1 ? "" :
			    "s");

			/* Print root set. */
			for (p = gray; p != white; p = qr_next(p, link)) {
				out_put_e(NULL, NULL, 0, __FUNCTION__,
				    "Root 0x[p|w:8|p:0]: ", p);
			
				stiloe_l_print(p,
				    stil_stderr_get(a_stila->stil), TRUE, TRUE);
				stilo_file_buffer_flush(stil_stderr_get(
				    a_stila->stil));
			}

			/* Print non-root set. */
			for (p = white; p != black; p = qr_next(p, link)) {
				out_put_e(NULL, NULL, 0, __FUNCTION__,
				    "Not root 0x[p|w:8|p:0]: ", p);
				stiloe_l_print(p,
				    stil_stderr_get(a_stila->stil), TRUE, TRUE);
				stilo_file_buffer_flush(stil_stderr_get(
				    a_stila->stil));
			}
		} else {
			nwhite = 0;
			qr_foreach(p, white, link) {
				nwhite++;
			}
			out_put_e(NULL, NULL, 0, __FUNCTION__,
			    "[i] root + [i] white = [i] object[s]\n", nroot,
			    nwhite, nroot + nwhite, nroot + nwhite == 1 ? "" :
			    "s");

			/* Print non-root set (everything). */
			qr_foreach(p, white, link) {
				out_put_e(NULL, NULL, 0, __FUNCTION__,
				    "Not root: ");
				stiloe_l_print(p,
				    stil_stderr_get(a_stila->stil), TRUE, TRUE);
				stilo_file_buffer_flush(stil_stderr_get(
				    a_stila->stil));
			}
			
		}

	}
#endif

	/*
	 * Iterate through the gray objects and process them until only black
	 * and white objects are left.
	 */
	for (; gray != white; gray = qr_next(gray, link)) {
#ifdef _LIBSTIL_CONFESS
		out_put_e(NULL, NULL, 0, __FUNCTION__, "Gray: ");
		stiloe_l_print(gray, stil_stderr_get(a_stila->stil), TRUE,
		    FALSE);
		stilo_file_buffer_flush(stil_stderr_get(a_stila->stil));
		out_put(NULL, " :");
#endif
		_cw_assert(stiloe_l_color_get(gray) != a_stila->white);
		for (stiloe = stiloe_l_ref_iter(gray, TRUE); stiloe != NULL;
		     stiloe = stiloe_l_ref_iter(gray, FALSE)) {
#ifdef _LIBSTIL_CONFESS
			out_put(NULL, " ");
			stiloe_l_print(stiloe, stil_stderr_get(a_stila->stil),
			    TRUE, FALSE);
			stilo_file_buffer_flush(stil_stderr_get(a_stila->stil));
#endif

			/*
			 * If object is white (and registered), color it.
			 */
			if ((stiloe_l_color_get(stiloe) == a_stila->white) &&
			    stiloe_l_registered_get(stiloe)) {
				stiloe_l_color_set(stiloe, !a_stila->white);
				if (stiloe != white) {
#ifdef _LIBSTIL_CONFESS
					out_put(NULL, "<C>");
#endif
					qr_remove(stiloe, link);
					qr_before_insert(white, stiloe, link);
				} else {
#ifdef _LIBSTIL_CONFESS
					out_put(NULL, "<CW>");
#endif
					white = qr_next(white, link);
				}
			}
#ifdef _LIBSTIL_CONFESS
			else {
				out_put(NULL, "<S>");
			}
#endif
		}
#ifdef _LIBSTIL_CONFESS
		out_put(NULL, "\n");
#endif
	}

	/*
	 * Split the white objects into a separate ring before resuming other
	 * threads.
	 */
	if (black != white) {
		/* Split the ring. */
#ifdef _LIBSTIL_CONFESS
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "Split ring into white and black\n");
#endif
#ifdef _LIBSTIL_DBG
		/*
		 * Ring splitting and melding are idempotent, so make sure that
		 * we're about to split, rather than meld.
		 */
		{
			cw_stiloe_t	*p;
			cw_bool_t	same_ring = FALSE;

			qr_foreach(p, black, link) {
				if (p == white)
					same_ring = TRUE;
			}
			_cw_assert(same_ring);
		}
#endif

		qr_split(black, white, link);
		ql_first(&a_stila->seq_set) = black;
	} else {
		/*
		 * We either have all valid objects or all garbage.  Look at the
		 * color bit of an object to determine which is the case, and
		 * either keep all the objects, or throw out all the garbage.
		 */
		if (stiloe_l_color_get(black) == a_stila->white) {
			/* All garbage. */
#ifdef _LIBSTIL_CONFESS
			out_put_e(NULL, NULL, 0, __FUNCTION__, "All garbage\n");
#endif
			ql_first(&a_stila->seq_set) = NULL;
#ifdef _LIBSTIL_CONFESS
			black = NULL;
#endif
		} else {
			/* All valid objects. */
#ifdef _LIBSTIL_CONFESS
			out_put_e(NULL, NULL, 0, __FUNCTION__, "No garbage\n");
#endif
			ql_first(&a_stila->seq_set) = black;
			white = NULL;
		}
	}

#ifdef _LIBSTIL_CONFESS
	{
		cw_stiloe_t	*p;
		cw_uint32_t	nblack = 0, nwhite = 0;

		if (black != NULL) {
			qr_foreach(p, black, link) {
				out_put_e(NULL, NULL, 0, __FUNCTION__,
				    "Not garbage: 0x[p|w:8|p:0]: ", p);
				stiloe_l_print(p,
				    stil_stderr_get(a_stila->stil), TRUE, TRUE);
				stilo_file_buffer_flush(stil_stderr_get(a_stila->stil));
				nblack++;
			}
		}

		if (white != NULL) {
			qr_foreach(p, white, link) {
				out_put_e(NULL, NULL, 0, __FUNCTION__,
				    "Garbage: 0x[p|w:8|p:0]: ", p);
				stiloe_l_print(p,
				    stil_stderr_get(a_stila->stil), TRUE, TRUE);
				stilo_file_buffer_flush(stil_stderr_get(a_stila->stil));

				nwhite++;
			}
		}
		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[i] black + [i] white = [i] objects\n", nblack, nwhite,
		    nblack + nwhite);
	}
#endif

	/* Flip the value of white. */
	a_stila->white = !a_stila->white;

	/* Reset the counter of new sequence set members since collection. */
	a_stila->seq_new = 0;

	thd_single_leave();
	mtx_unlock(&a_stila->lock);

#ifdef _LIBSTIL_STILA_TIME
	gettimeofday(&middle, NULL);
#endif

	/*
	 * Now that we can safely call code that potentially does locking, clean
	 * up the unreferenced objects.
	 */
	if (white != NULL) {
		cw_stiloe_t	*p, *n;

		p = white;
		do {
			n = qr_next(p, link);

			stiloe_l_delete(p, a_stila->stil);

			p = n;
		} while (p != white);
	}
#ifdef _LIBSTIL_CONFESS
	out_put_e(NULL, NULL, 0, __FUNCTION__, "Done collecting\n");
#endif
#ifdef _LIBSTIL_STILA_TIME
	gettimeofday(&after, NULL);
	{
		cw_uint64_t	t1, t2, t3;

		t1 = before.tv_sec;
		t1 *= 1000000;
		t1 += before.tv_usec;
		
		t2 = middle.tv_sec;
		t2 *= 1000000;
		t2 += middle.tv_usec;

		t3 = after.tv_sec;
		t3 *= 1000000;
		t3 += after.tv_usec;

		out_put_e(NULL, NULL, 0, __FUNCTION__,
		    "[q] us mark&sweep + [q] us deallocate = [q] us\n",
		    t2 - t1, t3 - t2, t3 -t1);
	}
#endif
}
