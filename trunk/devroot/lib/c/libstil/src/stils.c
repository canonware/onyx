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
	 * Pop objects off the stack and delete them.  Then delete all the
	 * stilsc's.
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

	retval = NULL;
	if (a_reset) {
		a_stils->ref_stilso = ql_first(&a_stils->stack);
		if (a_stils->ref_stilso != &a_stils->under)
			retval = stilo_stiloe_get(&a_stils->ref_stilso->stilo);
	}

	for (; retval == NULL && a_stils->ref_stilso != &a_stils->under;
	    a_stils->ref_stilso = qr_next(a_stils->ref_stilso, link))
		retval = stilo_stiloe_get(&a_stils->ref_stilso->stilo);

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
stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t a_amount)
{
	cw_stilso_t	*top, *noroll;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);
	_cw_assert(a_count <= a_stils->count);

	/*
	 * Calculate the current index of the element that will end up on top of
	 * the stack.  This allows us to save a pointer to it as we iterate down
	 * the stack on the way to the bottom of the roll region.  This code
	 * also has the side effect of 'mod'ing the roll amount, so that we
	 * don't spend a bunch of time rolling the stack if the user specifies a
	 * roll amount larger than the roll region.  A decent program will never
	 * do this, so it's not worth specifically optimizing, but it falls out
	 * of these calculations with no extra work, since we already have to
	 * deal with upward versus downward rolling calculations.
	 */
	if (a_amount < 0) {
		/* Convert a_amount to a positive equivalent. */
		a_amount += ((a_amount - a_count) / a_count) * a_count;
	}
	a_amount %= a_count;
	a_amount += a_count;
	a_amount %= a_count;

	/*
	 * Do this check after the above calculations, just in case the roll
	 * amount is an even multiple of the roll region.
	 */
	if (a_amount == 0) {
		/* Noop. */
		goto RETURN;
	}

	/*
	 * Get a pointer to the new top of the stack.  Then continue on to find
	 * the end of the roll region.
	 */
	for (i = 0, top = ql_first(&a_stils->stack); i < a_amount; i++)
		top = qr_next(top, link);
	noroll = top;

	for (i = 0; i < a_count - a_amount; i++)
		noroll = qr_next(noroll, link);

	/*
	 * We now have:
	 *
	 * ql_first(&a_stils->stack) --> /----------\ \  \
	 *                               |          | |  |
	 *                               |          | |   \
	 *                               |          | |   / a_amount
	 *                               |          | |  |
	 *                               |          | |  /
	 *                               \----------/  \
	 *                       top --> /----------\  / a_count
	 *                               |          | |
	 *                               |          | |
	 *                               |          | |
	 *                               |          | |
	 *                               |          | |
	 *                               \----------/ /
	 *                    noroll --> /----------\
	 *                               |          |
	 *                               |          |
	 *                               |          |
	 *                               |          |
	 *                               |          |
	 *                               \----------/
	 */
	thd_crit_enter();
	qr_split(ql_first(&a_stils->stack), noroll, link);
	qr_meld(top, noroll, link);
	ql_first(&a_stils->stack) = top;
	thd_crit_leave();

	RETURN:
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
