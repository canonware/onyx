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

/* Number of stack elements per memory chunk. */
#define	_CW_STIL_STILSC_COUNT	16

/*
 * Interval (in seconds) at which collection occurs, if not triggered sooner.
 */
#define	_CW_STILA_INTERVAL	 2

/*
 * Number of sequence set additions since last collection that will cause an
 * immediate collection.
 */
#define	_CW_STILA_THRESHHOLD	2500

typedef enum {
	STILAM_SHUTDOWN,
	STILAM_FORCE
} cw_stilam_t;

static void *stila_p_gc_entry(void *a_arg);
static void stila_p_collect(cw_stila_t *a_stila);

void
stila_new(cw_stila_t *a_stila, cw_stil_t *a_stil)
{
	volatile cw_uint32_t	try_stage = 0;

	xep_begin();
	xep_try {
		mtx_new(&a_stila->lock);
		try_stage = 1;

		mem_new(&a_stila->mem, NULL);
		try_stage = 2;

		pool_new(&a_stila->chi_pool, NULL, sizeof(cw_chi_t));
		try_stage = 3;

		pool_new(&a_stila->stilsc_pool, NULL,
		    _CW_STILSC_O2SIZEOF(_CW_STIL_STILSC_COUNT));
		try_stage = 4;

		pool_new(&a_stila->dicto_pool, NULL, sizeof(cw_stiloe_dicto_t));
		try_stage = 5;

		ql_new(&a_stila->seq_set);
		a_stila->seq_new = 0;
		a_stila->white = FALSE;
		mq_new(&a_stila->gc_mq, &a_stila->mem, sizeof(cw_stilam_t));
		try_stage = 6;

		a_stila->stil = a_stil;
		thd_new(&a_stila->gc_thd, stila_p_gc_entry, (void *)a_stila);
		try_stage = 7;
	}
	xep_catch(_CW_XEPV_OOM) {
		switch (try_stage) {
		case 6:
			mq_delete(&a_stila->gc_mq);
		case 5:
			pool_delete(&a_stila->dicto_pool);
		case 4:
			pool_delete(&a_stila->stilsc_pool);
		case 3:
			pool_delete(&a_stila->chi_pool);
		case 2:
			mem_delete(&a_stila->mem);
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
	mq_put(&a_stila->gc_mq, STILAM_SHUTDOWN);
	thd_join(&a_stila->gc_thd);
	mq_delete(&a_stila->gc_mq);

	pool_delete(&a_stila->dicto_pool);
	pool_delete(&a_stila->stilsc_pool);
	pool_delete(&a_stila->chi_pool);

	mem_delete(&a_stila->mem);
}

void
stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
	mtx_lock(&a_stila->lock);
/*  	out_put_e(cw_g_out, NULL, 0, __FUNCTION__, */
/*  	    "0x[p|w:8|p:0] : [i]\n", a_stiloe, a_stiloe->type); */
	stiloe_l_color_set(a_stiloe, a_stila->white);
	ql_tail_insert(&a_stila->seq_set, a_stiloe, link);
	a_stila->seq_new++;
	if (a_stila->seq_new >= _CW_STILA_THRESHHOLD)
		stila_gc_force(a_stila);
	mtx_unlock(&a_stila->lock);
}

static void *
stila_p_gc_entry(void *a_arg)
{
	cw_stila_t	*stila = (cw_stila_t *)a_arg;
	struct timespec	interval = {_CW_STILA_INTERVAL, 0};
	cw_stilam_t	message;
	cw_bool_t	shutdown, collect;

	/*
	 * Any of the following conditions will cause a collection:
	 *
	 * 1) Someone called stila_gc_force().
	 *
	 * 2) Enough allocation was done to trigger immediate collection.
	 *
	 * 3) Some allocation was done, and it is time for periodic collection.
	 */
	for (shutdown = FALSE, collect = FALSE; shutdown == FALSE; collect =
	    FALSE) {
		if (mq_timedget(&stila->gc_mq, &interval, &message) == FALSE) {
			switch (message) {
			case STILAM_SHUTDOWN:
				shutdown = TRUE;
				/* Fall through. */
			case STILAM_FORCE:
				collect = TRUE;
				break;
			default:
				_cw_not_reached();
			}
		} else {
			/*
			 * No messages.  Check to see if there have been any
			 * additions to the sequence set.
			 */
			mtx_lock(&stila->lock);
			if (stila->seq_new > 0)
				collect = TRUE;
			mtx_unlock(&stila->lock);
		}

		if (collect)
			stila_p_collect(stila);
	}

	return NULL;
}

#define	stila_p_gray(a_stila, a_stiloe) do {				\
	stiloe_l_color_set((a_stiloe), !(a_stila)->white);
	

/*
 * Collect garbage using a Baker's Treadmill.
 */
static void
stila_p_collect(cw_stila_t *a_stila)
{
	cw_stilt_t	*stilt;
	cw_stils_t	*stils;
	cw_stiloe_t	*stiloe, *black, *gray, *white;

	mtx_lock(&a_stila->lock);
	thd_single_enter();

	/*
	 * Set up the object regions.
	 */
	black = gray = white = ql_first(&a_stila->seq_set);

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
	_cw_out_put("\n");
	out_put_e(cw_g_out, NULL, 0, __FUNCTION__, "v");
	for (stilt = stil_l_ref_iter(a_stila->stil, TRUE); stilt != NULL; stilt
	    = stil_l_ref_iter(a_stila->stil, FALSE)) {
		_cw_out_put("t");
		for (stils = stilt_l_ref_iter(stilt, TRUE); stils != NULL; stils
		    = stilt_l_ref_iter(stilt, FALSE)) {
			_cw_out_put("s");
			for (stiloe = stils_l_ref_iter(stils, TRUE); stiloe !=
			    NULL; stiloe = stils_l_ref_iter(stils, FALSE)) {
				if (stiloe_l_color_get(stiloe) ==
				    a_stila->white) {
					_cw_out_put("+");
					/* Make object gray. */
					stila_p_gray(stila, stiloe);
					/* XXX Move. */
				}
			}
		}
	}
	_cw_out_put("\n");

	/*
	 * Iterate through the gray objects and process them until only black
	 * and white objects are left.
	 */

	/*
	 * Move black forward one, since it is actually pointing to a white
	 * object.
	 */
	black = qr_next(black, link);

	/*
	 * Split the white objects into a separate ring before resuming other
	 * threads.
	 */

	/* Flip the value of white. */
	a_stila->white = !a_stila->white;

	/* Reset the counter of new sequence set members since collection. */
	a_stila->seq_new = 0;

	thd_single_leave();
	mtx_unlock(&a_stila->lock);

	/*
	 * Now that we can safely call code that potentially does locking, clean
	 * up the unreferenced objects.
	 */
}
