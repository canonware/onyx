/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 * Stacks are implemented by the stils class.  Stack object space is allocated
 * in chunks (implemented by the stilsc class) in order to improve locality and
 * reduce memory fragmentation.  Freed objects within a chunk are kept in a
 * stack (LIFO ordering) and re-used.  This has the potential to cause adjacent
 * stack objects to be scattered throughout the stilsc's, but typical stack
 * operations have the same effect anyway, so little care is taken to keep stack
 * object re-allocation contiguous, or even local.
 *
 * Since the GC must traverse the entire stack at every collection, we use that
 * opportunity to tidy things up.  The entire stack is re-written contiguously,
 * and the old stilsc's are returned to the global pool.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

#ifdef _LIBSTIL_DBG
#define _CW_STILS_MAGIC 0x0ea67890
#define _CW_STILSC_MAGIC 0x543e2aff
#endif

static void	stils_p_spares_create(cw_stils_t *a_stils);

void
stils_new(cw_stils_t *a_stils, cw_pool_t *a_stilsc_pool)
{
	_cw_check_ptr(a_stils);
	_cw_check_ptr(a_stilsc_pool);

	ql_new(&a_stils->stack);
	a_stils->stilsc_pool = a_stilsc_pool;
	qs_new(&a_stils->chunks);

	a_stils->count = 0;

	ql_elm_new(&a_stils->under, link);
	ql_first(&a_stils->stack) = &a_stils->under;
	/* Fill spares. */
	stils_p_spares_create(a_stils);
	ql_first(&a_stils->stack) = qr_prev(ql_first(&a_stils->stack), link);

#ifdef _LIBSTIL_DBG
	a_stils->magic = _CW_STILS_MAGIC;
#endif
}

void
stils_delete(cw_stils_t *a_stils, cw_stilt_t *a_stilt)
{
	cw_stilsc_t	*stilsc;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/*
	 * Pop objects off the stack and delete them.  Then delete all the
	 * stilsc's.
	 */
	if (a_stils->count > 0)
		stils_npop(a_stils, a_stilt, a_stils->count);

	while (qs_top(&a_stils->chunks) != NULL) {
		stilsc = qs_top(&a_stils->chunks);
		qs_pop(&a_stils->chunks, link);
		pool_put(a_stils->stilsc_pool, stilsc);
	}

#ifdef _LIBSTILS_DBG
	a_stils->magic = 0;
#endif
}

void
stils_collect(cw_stils_t *a_stils, void (*a_add_root_func)
    (void *add_root_arg, cw_stilo_t *root), void *a_add_root_arg)
{
	cw_stilsc_t	*stilsc;
	cw_stilso_t	*old_stilso;
	cw_stilo_t	*new_stilo;
	cw_uint32_t	old_count, i;
	qs_head(cw_stilsc_t) old_chunks;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/*
	 * Move the old stack, spares, and stilsc's out of the way so that we
	 * can start fresh.
	 */
	old_stilso = ql_first(&a_stils->stack);
	old_count = a_stils->count;
	a_stils->count = 0;

	qr_remove(&a_stils->under, link);
	ql_first(&a_stils->stack) = &a_stils->under;
	/* Fill spares. */
	stils_p_spares_create(a_stils);
	ql_first(&a_stils->stack) = qr_prev(ql_first(&a_stils->stack), link);
	
	memcpy(&old_chunks, &a_stils->chunks, sizeof(old_chunks));
	qs_new(&a_stils->chunks);

	/*
	 * Iterate through the entire stack, moving stilso's to the new stack.
	 * Along the way, report any extended objects to the GC.
	 */
	for (i = 0; i < old_count; old_stilso = qr_next(old_stilso, link),
		 i++) {
		new_stilo = stils_push(a_stils, NULL);
		stilo_move(new_stilo, &old_stilso->stilo);

		switch (stilo_type_get(new_stilo)) {
		case STILOT_ARRAY:
		case STILOT_CONDITION:
		case STILOT_DICT:
		case STILOT_LOCK:
		case STILOT_STRING:
			a_add_root_func(a_add_root_arg, new_stilo);
			break;
		default:
			break;
		}
	}

	/*
	 * Now delete the old stilsc's.  We've moved everything important to new
	 * storage, so nothing more than deletion is necessary.
	 */
	while (qs_top(&old_chunks) != NULL) {
		stilsc = qs_top(&old_chunks);
		qs_pop(&old_chunks, link);
		pool_put(a_stils->stilsc_pool, stilsc);
	}
}

cw_stilo_t *
stils_push(cw_stils_t *a_stils, cw_stilt_t *a_stilt)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(ql_first(&a_stils->stack) != &a_stils->under);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qr_prev(ql_first(&a_stils->stack), link) == &a_stils->under)
		stils_p_spares_create(a_stils);
	ql_first(&a_stils->stack) = qr_prev(ql_first(&a_stils->stack), link);
	stilso = ql_first(&a_stils->stack);
	a_stils->count++;

	stilo_no_new(&stilso->stilo);

	return &stilso->stilo;
}

cw_stilo_t *
stils_under_push(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_check_ptr(a_stilo);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qr_prev(ql_first(&a_stils->stack), link) == &a_stils->under)
		stils_p_spares_create(a_stils);
	stilso = qr_prev(ql_first(&a_stils->stack), link);
	qr_remove(stilso, link);
	qr_after_insert((cw_stilso_t *)stilso, stilso, link);
	a_stils->count++;

	stilo_no_new(&stilso->stilo);

	return &stilso->stilo;
}

void
stils_pop(cw_stils_t *a_stils, cw_stilt_t *a_stilt)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	stilso = ql_first(&a_stils->stack);
	ql_first(&a_stils->stack) = qr_next(ql_first(&a_stils->stack), link);
	a_stils->count--;
}

void
stils_npop(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_uint32_t a_count)
{
	cw_stilso_t	*top;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);

	if (a_count > a_stils->count)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, top = ql_first(&a_stils->stack); i < a_count; i++)
		top = qr_next(top, link);

	ql_first(&a_stils->stack) = top;
	a_stils->count -= a_count;
}

void
stils_roll(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_uint32_t a_count,
    cw_sint32_t a_amount)
{
	cw_stilso_t	*top, *noroll;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_count < 1)
		stilt_error(a_stilt, STILTE_RANGECHECK);
	if (a_count > a_stils->count)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

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
	qr_split(ql_first(&a_stils->stack), noroll, link);
	qr_meld(top, noroll, link);
	ql_first(&a_stils->stack) = top;

	RETURN:
}

cw_stilo_t *
stils_get(cw_stils_t *a_stils, cw_stilt_t *a_stilt)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	stilso = ql_first(&a_stils->stack);

	return &stilso->stilo;
}

cw_stilo_t *
stils_nget(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_uint32_t a_index)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_index >= a_stils->count)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	for (i = 0, stilso = ql_first(&a_stils->stack); i < a_index; i++)
		stilso = qr_next(stilso, link);

	return &stilso->stilo;
}

cw_stilo_t *
stils_down_get(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count <= 1)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	stilso = (cw_stilso_t *)a_stilo;
	stilso = qr_next(stilso, link);
	if (stilso == &a_stils->under)
		stilt_error(a_stilt, STILTE_STACKUNDERFLOW);

	return &stilso->stilo;
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

static void
stils_p_spares_create(cw_stils_t *a_stils)
{
	cw_stilsc_t	*stilsc;
	cw_uint32_t	nstilso, i;

	/*
	 * create a new stilsc, add it to the stilsc qs, and add its stilso's to
	 * the stack.
	 */
	stilsc = (cw_stilsc_t *)pool_get(a_stils->stilsc_pool);

#ifdef _LIBSTIL_DBG
	stilsc->magic = _CW_STILSC_MAGIC;
#endif
	qs_elm_new(stilsc, link);

	qr_new(&stilsc->objects[0], link);
	for (i = 1, nstilso =
		 _CW_STILSC_SIZEOF2O(pool_buffer_size_get(a_stils->stilsc_pool));
	     i < nstilso; i++) {
		qr_new(&stilsc->objects[i], link);
		qr_after_insert(&stilsc->objects[i - 1],
		    &stilsc->objects[i], link);
	}
/*  	_cw_out_put_e("nstilso: [i]\n", nstilso); */
/*  	_cw_out_put_e("[i] --> [i] --> [i]\n", nstilso, */
/*  	    _CW_STILSC_O2SIZEOF(nstilso), */
/*  	    _CW_STILSC_SIZEOF2O(_CW_STILSC_O2SIZEOF(nstilso))); */

	qs_push(&a_stils->chunks, stilsc, link);
	qr_meld(ql_first(&a_stils->stack), &stilsc->objects[0], link);
}
