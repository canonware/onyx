/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Stacks are implemented by the stils class.  Stack object space is allocated
 * in chunks (implemented by the stilsc class) in order to improve locality and
 * reduce memory fragmentation.  Freed objects within a chunk are kept in the
 * same ring as the actual stack and re-used in LIFO order.  This has the
 * potential to cause adjacent stack objects to be scattered throughout the
 * stilsc's, but typical stack operations have the same effect anyway, so little
 * care is taken to keep stack object re-allocation contiguous, or even local.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 * Using a ring makes it relatively simple to make all the stack operations
 * GC-safe.  One disadvantage of using rings, however, is that the stils_roll()
 * re-orders stack elements, and over time, the elements become jumbled enough
 * that it is possible that additional cache misses result.  However, since only
 * a relatively small number of spare elements is kept, the cache effects of
 * jumbling should be negligible under normal conditions.
 *
 ******************************************************************************/

/* Compile non-inlined functions if not using inlines. */
#define	_STILS_C_

#include "../include/libstil/libstil.h"

void
stils_new(cw_stils_t *a_stils, cw_stilt_t *a_stilt)
{
	_cw_check_ptr(a_stils);

	a_stils->stilt = a_stilt;
	ql_new(&a_stils->stack);
	ql_new(&a_stils->chunks);

	a_stils->count = 0;
	a_stils->nspare = 0;

	ql_elm_new(&a_stils->under, link);
	ql_head_insert(&a_stils->stack, &a_stils->under, link);

	a_stils->noroll = NULL;
#ifdef _LIBSTIL_DBG
	a_stils->magic = _CW_STILS_MAGIC;
#endif
}

void
stils_delete(cw_stils_t *a_stils)
{
	cw_stilsc_t	*stilsc;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/*
	 * Pop objects off the stack.  Then delete all the stilsc's.
	 */
	if (a_stils->count > 0)
		stils_npop(a_stils, a_stils->count);

	while (ql_first(&a_stils->chunks) != NULL) {
		stilsc = ql_first(&a_stils->chunks);
		ql_remove(&a_stils->chunks, stilsc, link);
		stila_stilsc_put(stil_stila_get(stilt_stil_get(a_stils->stilt)),
		    stilsc);
	}

#ifdef _LIBSTILS_DBG
	a_stils->magic = 0;
#endif
}

cw_stiloe_t *
stils_l_ref_iter(cw_stils_t *a_stils, cw_bool_t a_reset)
{
	cw_stiloe_t	*retval;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_reset) {
		if (a_stils->noroll != NULL) {
			/*
			 * We're in the middle of a roll operation, so need to
			 * handle the noroll region specially.  It's entirely
			 * possible that we'll end up reporting some/all stack
			 * elements twice, but that doesn't cause a correctness
			 * problem, whereas not reporting them at all does.
			 */
			a_stils->ref_stage = 0;
		} else
			a_stils->ref_stage = 2;
	}

	retval = NULL;
	switch (a_stils->ref_stage) {
	case 0:
		/* Set up for stage 1. */
		a_stils->ref_stilso = a_stils->noroll;
		a_stils->ref_stage++;
		/* Fall through. */
	case 1:
		/* noroll region stack iteration. */
		for (; retval == NULL && a_stils->ref_stilso != &a_stils->under;
		    a_stils->ref_stilso = qr_next(a_stils->ref_stilso, link))
			retval = stilo_stiloe_get(&a_stils->ref_stilso->stilo);

		if (retval != NULL)
			break;
		a_stils->ref_stage++;
		/* Fall through. */
	case 2:
		/* First roll region iteration. */
		a_stils->ref_stilso = ql_first(&a_stils->stack);
		if (a_stils->ref_stilso != &a_stils->under)
			retval = stilo_stiloe_get(&a_stils->ref_stilso->stilo);

		a_stils->ref_stage++;
		if (retval != NULL)
			break;
		/* Fall through. */
	case 3:
		/* Set up for stage 4. */
		if (a_stils->ref_stilso != &a_stils->under) {
			a_stils->ref_stilso = qr_next(a_stils->ref_stilso,
			    link);
		}
		a_stils->ref_stage++;
		/* Fall through. */
	case 4:
		/* Main roll region iteration. */
		for (; retval == NULL && a_stils->ref_stilso != &a_stils->under
		    && a_stils->ref_stilso != ql_first(&a_stils->stack);
		    a_stils->ref_stilso = qr_next(a_stils->ref_stilso, link))
			retval = stilo_stiloe_get(&a_stils->ref_stilso->stilo);

		if (retval != NULL)
			break;
		a_stils->ref_stage++;
		/* Fall through. */
	default:
		retval = NULL;
	}
	return retval;
}

cw_uint32_t
stils_index_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	for (i = 0, stilso = ql_first(&a_stils->stack); (stilso != (cw_stilso_t
	    *)a_stilo) && (i < a_stils->count); stilso = qr_next(stilso, link),
	     i++);
	_cw_assert(i < a_stils->count);

	return i;
}

void
stils_p_spares_create(cw_stils_t *a_stils)
{
	cw_stilsc_t	*stilsc;
	cw_uint32_t	i;

	/*
	 * create a new stilsc, add it to the stilsc ql, and add its stilso's to
	 * the stack.
	 */
	stilsc =
	    stila_stilsc_get(stil_stila_get(stilt_stil_get(a_stils->stilt)));

	ql_elm_new(stilsc, link);

	stilsc->nused = 0;

	qr_new(&stilsc->objects[0], link);
	stilsc->objects[0].stilsc = stilsc;
	for (i = 1; i < _LIBSTIL_STILSC_COUNT; i++) {
		qr_new(&stilsc->objects[i], link);
		qr_after_insert(&stilsc->objects[i - 1],
		    &stilsc->objects[i], link);
		stilsc->objects[i].stilsc = stilsc;
	}

	ql_tail_insert(&a_stils->chunks, stilsc, link);
	qr_meld(ql_first(&a_stils->stack), &stilsc->objects[0], link);

	a_stils->nspare += _LIBSTIL_STILSC_COUNT;
}

void
stils_p_spares_destroy(cw_stils_t *a_stils, cw_stilsc_t *a_stilsc)
{
	cw_uint32_t	i;

	/*
	 * Iterate through the objects and remove them from the stils-wide
	 * object ring.
	 */
	for (i = 0; i < _LIBSTIL_STILSC_COUNT; i++)
		qr_remove(&a_stilsc->objects[i], link);

	/* Remove the stilsc from the stils's list of stilsc's. */
	ql_remove(&a_stils->chunks, a_stilsc, link);

	/* Deallocate. */
	stila_stilsc_put(stil_stila_get(stilt_stil_get(a_stils->stilt)),
	    a_stilsc);

	a_stils->nspare -= _LIBSTIL_STILSC_COUNT;
}
