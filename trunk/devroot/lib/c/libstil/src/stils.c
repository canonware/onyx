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
 * that it is possible that additional cache misses result.
 *
 * XXX A reasonably simple solution to the ring re-ordering problem is to
 * maintain rings of free elements on a per-chunk basis.  This will cause re-use
 * to occur mainly in the first-listed chunk, so that other chunks will tend to
 * become entirely free.  This allows other chunks to be freed, as well as
 * causing adjacent stack elements to be more contiguous in memory.
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

void
stils_new(cw_stils_t *a_stils)
{
	_cw_check_ptr(a_stils);

	ql_new(&a_stils->stack);
	qs_new(&a_stils->chunks);

	a_stils->count = 0;

	ql_elm_new(&a_stils->under, link);
	ql_first(&a_stils->stack) = &a_stils->under;
	/* Fill spares. */
	stils_p_spares_create(a_stils);

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

	while (qs_top(&a_stils->chunks) != NULL) {
		stilsc = qs_top(&a_stils->chunks);
		qs_pop(&a_stils->chunks, link);
		_cw_free(stilsc);
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
	cw_uint32_t	nstilso, i;

	/*
	 * create a new stilsc, add it to the stilsc qs, and add its stilso's to
	 * the stack.
	 */
	stilsc = (cw_stilsc_t
	    *)_cw_malloc(_CW_STILSC_O2SIZEOF(_LIBSTIL_STILSC_COUNT));

#ifdef _LIBSTIL_DBG
	stilsc->magic = _CW_STILSC_MAGIC;
#endif
	qs_elm_new(stilsc, link);

	qr_new(&stilsc->objects[0], link);
	for (i = 1, nstilso = _LIBSTIL_STILSC_COUNT; i < nstilso; i++) {
		qr_new(&stilsc->objects[i], link);
		qr_after_insert(&stilsc->objects[i - 1],
		    &stilsc->objects[i], link);
	}

	qs_push(&a_stils->chunks, stilsc, link);
	qr_meld(ql_first(&a_stils->stack), &stilsc->objects[0], link);
}
