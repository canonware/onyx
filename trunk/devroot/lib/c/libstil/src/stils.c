/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
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

	qs_new(&a_stils->stack);
	a_stils->count = 0;
	qs_new(&a_stils->spares);
	a_stils->stilsc_pool = a_stilsc_pool;
	qs_new(&a_stils->chunks);

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
		stils_npop(a_stils, a_stils->count);

	while (qs_top(&a_stils->chunks) != NULL) {
		stilsc = qs_top(&a_stils->chunks);
		qs_pop(&a_stils->chunks, link);
		pool_put(stilsc->stilsc_pool, stilsc);
	}

#ifdef _LIBSTILS_DBG
	a_stils->magic = 0;
#endif
}

void
stils_collect(cw_stils_t *a_stils, void (*a_add_root_func) (void *add_root_arg,
    cw_stilo_t *root), void *a_add_root_arg)
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
	old_stilso = qs_top(&a_stils->stack);
	old_count = a_stils->count;
	qs_top(&a_stils->stack) = NULL;
	a_stils->count = 0;
	qs_top(&a_stils->spares) = NULL;
	memcpy(&old_chunks, &a_stils->chunks, sizeof(old_chunks));
	qs_new(&a_stils->chunks);

	/*
	 * Iterate through the entire stack, moving stilso's to the new stack.
	 * Along the way, report any extended objects to the GC.
	 */
	for (i = 0; i < old_count; old_stilso = qs_down(old_stilso, link),
		 i++) {
		new_stilo = stils_push(a_stils);
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
		pool_put(stilsc->stilsc_pool, stilsc);
	}
}

cw_stilo_t *
stils_push(cw_stils_t *a_stils)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qs_top(&a_stils->spares) == NULL)
		stils_p_spares_create(a_stils);
	stilso = qs_top(&a_stils->spares);
	qs_pop(&a_stils->spares, link);

	stilo_no_new(&stilso->stilo);

	/* Link the stilso into the stack. */
	qs_push(&a_stils->stack, stilso, link);
	a_stils->count++;

	return &stilso->stilo;
}

cw_stilo_t *
stils_under_push(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_check_ptr(a_stilo);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qs_top(&a_stils->spares) == NULL)
		stils_p_spares_create(a_stils);
	stilso = qs_top(&a_stils->spares);
	qs_pop(&a_stils->spares, link);

	stilo_no_new(&stilso->stilo);

	/* Link the stilso into the stack. */
	qs_under_push((cw_stilso_t *)a_stilo, stilso, link);
	a_stils->count++;

	return &stilso->stilo;
}

void
stils_pop(cw_stils_t *a_stils)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	stilso = qs_top(&a_stils->stack);
	qs_pop(&a_stils->stack, link);
	qs_push(&a_stils->spares, stilso, link);
	a_stils->count--;
}

void
stils_npop(cw_stils_t *a_stils, cw_uint32_t a_count)
{
	cw_stilso_t	*bottom, *top;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);

	if (a_count > a_stils->count)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, bottom = qs_top(&a_stils->stack); i < a_count - 1; i++)
		bottom = qs_down(bottom, link);
	top = qs_down(bottom, link);

	/*
	 * We now have:
	 *
	 * qs_top(&a_stils->stack) --> /----------\
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                  bottom --> \----------/
	 *                     top --> /----------\
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             \----------/
	 *
	 * 1) Link qs_top(&a_stils->spares) under bottom.
	 *
	 * 2) Move qs_top(&a_stils->stack) to qs_top(&a_stils->spares).
	 *
	 * 3) Set qs_top(&a_stils->stack) to top.
	 */
	qs_down(bottom, link) = qs_top(&a_stils->spares);
	qs_top(&a_stils->spares) = qs_top(&a_stils->stack);
	qs_top(&a_stils->stack) = top;
	
	a_stils->count -= a_count;
}

void
stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t a_amount)
{
	cw_stilso_t	*stilso, *top, *bottom, *noroll;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_count < 1)
		xep_throw(_CW_XEPV_RANGECHECK);
	if (a_count > a_stils->count)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

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
	 * Get a pointer to the new top of the stack, and save a pointer to the
	 * element just above it.  Then continue on to find the end of the roll
	 * region.
	 */
	for (i = 0, bottom = qs_top(&a_stils->stack); i < a_amount - 1; i++)
		bottom = qs_down(bottom, link);
	stilso = qs_down(bottom, link);
	top = stilso;

	for (i = 0; i < a_count - a_amount - 1; i++)
		stilso = qs_down(stilso, link);
	noroll = qs_down(stilso, link);

	/*
	 * We now have:
	 *
	 * qs_top(&a_stils->stack) --> /----------\ \  \
	 *                             |          | |  |
	 *                             |          | |   \
	 *                             |          | |   / a_amount
	 *                             |          | |  |
	 *                             |          | |  /
	 *                  bottom --> \----------/  \
	 *                     top --> /----------\  / a_count
	 *                             |          | |
	 *                             |          | |
	 *                             |          | |
	 *                             |          | |
	 *                             |          | |
	 *                  stilso --> \----------/ /
	 *                  noroll --> /----------\
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             |          |
	 *                             \----------/
	 *
	 * 1) Link qs_top(&a_stils->stack) under stilso.
	 *
	 * 2) Link noroll under bottom.
	 *
	 * 3) Set a_stils->stack to top.
	 */
	qs_down(stilso, link) = qs_top(&a_stils->stack);
	qs_down(bottom, link) = noroll;
	qs_top(&a_stils->stack) = top;

	RETURN:
}

cw_stilo_t *
stils_get(cw_stils_t *a_stils)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	stilso = qs_top(&a_stils->stack);

	return &stilso->stilo;
}

cw_stilo_t *
stils_nget(cw_stils_t *a_stils, cw_uint32_t a_index)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_index >= a_stils->count)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	for (i = 0, stilso = qs_top(&a_stils->stack); i < a_index; i++)
		stilso = qs_down(stilso, link);

	return &stilso->stilo;
}

cw_stilo_t *
stils_down_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count <= 1)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	stilso = (cw_stilso_t *)a_stilo;
	stilso = qs_down(stilso, link);
	if (stilso == NULL)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	return &stilso->stilo;
}

cw_uint32_t
stils_index_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	for (i = 0, stilso = qs_top(&a_stils->stack); (stilso != (cw_stilso_t
	    *)a_stilo) && (i < a_stils->count); stilso = qs_down(stilso, link),
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
	 * the spares qs.
	 */
	stilsc = (cw_stilsc_t *)pool_get(a_stils->stilsc_pool);

#ifdef _LIBSTIL_DBG
	stilsc->magic = _CW_STILSC_MAGIC;
#endif
	stilsc->stilsc_pool = a_stils->stilsc_pool;
	qs_elm_new(stilsc, link);

	qs_elm_new(&stilsc->objects[0], link);
	for (i = 1, nstilso =
		 _CW_STILSC_SIZEOF2O(pool_buffer_size_get(stilsc->stilsc_pool));
	     i < nstilso; i++) {
		qs_elm_new(&stilsc->objects[i], link);
		qs_under_push(&stilsc->objects[i],
		    &stilsc->objects[i - 1], link);
	}

	qs_push(&a_stils->chunks, stilsc, link);
	qs_top(&a_stils->spares) = &stilsc->objects[0];
}
