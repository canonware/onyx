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

static cw_stilso_t	*stils_p_alloc_stilso(cw_stils_t *a_stils);
static cw_stilsc_t	*stilsc_p_new(cw_pool_t *a_stilsc_pool);
static void		 stilso_p_new(cw_stilso_t *a_stilso);
static void		 stilsc_p_delete(cw_stilsc_t *a_stilsc);
static cw_uint32_t	 stilsc_p_get_nstilso(cw_stilsc_t *a_stilsc);
static cw_stilso_t	*stilsc_p_get_stilso(cw_stilsc_t *a_stilsc, cw_uint32_t
    a_index);
static void		 stilso_p_new(cw_stilso_t *a_stilso);

cw_stils_t *
stils_new(cw_stils_t *a_stils, cw_pool_t *a_stilsc_pool)
{
	cw_stils_t	*retval;

	_cw_check_ptr(a_stils);
	_cw_check_ptr(a_stilsc_pool);

	retval = a_stils;
	retval->stack = NULL;
	retval->count = 0;
	retval->spares = NULL;
	retval->nspares = 0;
	retval->stilsc_pool = a_stilsc_pool;
	ql_new(&retval->chunks);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILS_MAGIC;
#endif

	return retval;
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
		stils_pop(a_stils, a_stilt, a_stils->count);

	while (ql_first(&a_stils->chunks) != NULL) {
		stilsc = ql_first(&a_stils->chunks);
		ql_head_remove(&a_stils->chunks, cw_stilsc_t, link);
		stilsc_p_delete(stilsc);
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
	ql_head(cw_stilsc_t) old_chunks;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/*
	 * Move the old stack, spares, and stilsc's out of the way so that we
	 * can start fresh.
	 */
	old_stilso = a_stils->stack;
	old_count = a_stils->count;
	a_stils->stack = NULL;
	a_stils->count = 0;
	a_stils->spares = NULL;
	a_stils->nspares = 0;
	memcpy(&old_chunks, &a_stils->chunks, sizeof(old_chunks));
	ql_new(&a_stils->chunks);

	/*
	 * Iterate through the entire stack, moving stilso's to the new stack.
	 * Along the way, report any extended objects to the GC.
	 */
	for (i = 0; i < old_count; old_stilso = qr_next(old_stilso,
	    link), i++) {
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
	while (ql_first(&old_chunks) != NULL) {
		stilsc = ql_first(&old_chunks);
		ql_head_remove(&old_chunks, cw_stilsc_t, link);
		stilsc_p_delete(stilsc);
	}
}

cw_stilo_t *
stils_push(cw_stils_t *a_stils)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/* Get an unused stilso and link it into the stack. */
	stilso = stils_p_alloc_stilso(a_stils);
	if (a_stils->stack != NULL) 
		qr_meld(stilso, a_stils->stack, link);
	a_stils->stack = stilso;
	a_stils->count++;

	stilo_no_new(&stilso->stilo);
	return &stilso->stilo;
}

void
stils_pop(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_uint32_t a_count)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);
	
	if (a_count > a_stils->count)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, stilso = a_stils->stack; i < a_count; i++)
		stilso = qr_next(stilso, link);

	if (a_count != a_stils->count) {
		/*
		 * Split the ring.  stilso points to the top of the stack, and
		 * a_stils->stack points to the popped elements.
		 */
		qr_split(stilso, a_stils->stack, link);
	}

	if (a_stils->nspares > 0)
		qr_meld(a_stils->stack, a_stils->spares, link);
	a_stils->spares = a_stils->stack;
	a_stils->nspares += a_count;

	a_stils->stack = stilso;
	a_stils->count -= a_count;
}

void
stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t a_amount)
{
	cw_stilso_t	*stilso, *top;
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
	a_amount %= a_count;
	a_amount = a_amount + a_count;
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
	for (i = 0, stilso = a_stils->stack; i < a_amount; i++)
		stilso = qr_next(stilso, link);
	top = stilso;

	/*
	 * This portion of the code is not necessary (dangerous even) if we're
	 * rolling the entire stack.
	 */
	if (a_count < a_stils->count) {
		for (i = 0; i < a_count - a_amount; i++)
			stilso = qr_next(stilso, link);

		qr_split(stilso, a_stils->stack, link);
		qr_meld(top, stilso, link);
	}

	a_stils->stack = top;

	RETURN:
}

cw_uint32_t
stils_count(cw_stils_t *a_stils)
{
	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	return a_stils->count;
}

cw_stilo_t *
stils_get(cw_stils_t *a_stils, cw_uint32_t a_index)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_index >= a_stils->count)
		xep_throw(_CW_XEPV_STACKUNDERFLOW);

	for (i = 0, stilso = a_stils->stack; i < a_index; i++)
		stilso = qr_next(stilso, link);

	return &stilso->stilo;
}

cw_stilo_t *
stils_get_down(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilo_t	*retval;
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count <= 1) {
		retval = NULL;
		goto RETURN;
	}

	stilso = (cw_stilso_t *)a_stilo;
	stilso = qr_next(stilso, link);
	if (stilso == a_stils->stack) {
		/* Tried to get next element down while at the stack bottom. */
		retval = NULL;
		goto RETURN;
	}

	retval = &stilso->stilo;
	RETURN:
	return retval;
}

cw_stilo_t *
stils_get_up(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilo_t	*retval;
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count <= 1) {
		retval = NULL;
		goto RETURN;
	}

	stilso = (cw_stilso_t *)a_stilo;
	if ((stilso == a_stils->stack) || (a_stils->count <= 1)) {
		/* Tried to get next element up while at the stack top. */
		retval = NULL;
		goto RETURN;
	}
	stilso = qr_prev(stilso, link);

	retval = &stilso->stilo;
	RETURN:
	return retval;
}

cw_uint32_t
stils_get_index(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	for (i = 0, stilso = a_stils->stack; (stilso != (cw_stilso_t *)a_stilo)
	    && (i < a_stils->count); stilso = qr_next(stilso, link), i++);
	_cw_assert(i < a_stils->count);

	return i;
}

static cw_stilso_t *
stils_p_alloc_stilso(cw_stils_t *a_stils)
{
	cw_stilso_t	*retval;

	/*
	 * If there are no spares, create a new stilsc, add it to the stilsc
	 * slist, and add its stilso's to the spares ring.
	 */
	if (a_stils->nspares == 0) {
		cw_stilsc_t	*stilsc;

		stilsc = stilsc_p_new(a_stils->stilsc_pool);

		ql_head_insert(&a_stils->chunks, stilsc, link);
		a_stils->spares = stilsc_p_get_stilso(stilsc, 0);
		a_stils->nspares = stilsc_p_get_nstilso(stilsc);
	}

	retval = a_stils->spares;
	a_stils->spares = qr_next(retval, link);
	qr_remove(retval, link);
	a_stils->nspares--;

	stilo_no_new(&retval->stilo);
	return retval;
}

/*
 * Initialize all embedded stilso's and link them into a ring.
 */
static cw_stilsc_t *
stilsc_p_new(cw_pool_t *a_stilsc_pool)
{
	cw_stilsc_t	*retval;
	cw_uint32_t	nstilso, i;

	retval = (cw_stilsc_t *)pool_get(a_stilsc_pool);
	if (retval == NULL) {
		/* XXX Report error. */
	}

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILSC_MAGIC;
#endif
	retval->stilsc_pool = a_stilsc_pool;
	ql_elm_new(retval, link);

	stilso_p_new(&retval->objects[0]);
	for (i = 1, nstilso = stilsc_p_get_nstilso(retval); i < nstilso; i++) {
		stilso_p_new(&retval->objects[i]);
		qr_meld(&retval->objects[i], &retval->objects[i - 1], link);
	}

	return retval;
}

static void
stilsc_p_delete(cw_stilsc_t *a_stilsc)
{
	_cw_check_ptr(a_stilsc);
	_cw_assert(a_stilsc->magic == _CW_STILSC_MAGIC);

	pool_put(a_stilsc->stilsc_pool, a_stilsc);
}

static cw_uint32_t
stilsc_p_get_nstilso(cw_stilsc_t *a_stilsc)
{
	_cw_check_ptr(a_stilsc);
	_cw_assert(a_stilsc->magic == _CW_STILSC_MAGIC);

	return _CW_STILSC_SIZEOF2O(pool_buffer_size_get(a_stilsc->stilsc_pool));
}

static cw_stilso_t *
stilsc_p_get_stilso(cw_stilsc_t *a_stilsc, cw_uint32_t a_index)
{
	_cw_check_ptr(a_stilsc);
	_cw_assert(a_stilsc->magic == _CW_STILSC_MAGIC);
	_cw_assert(a_index < stilsc_p_get_nstilso(a_stilsc));

	return &a_stilsc->objects[a_index];
}

static void
stilso_p_new(cw_stilso_t *a_stilso)
{
	stilo_no_new(&a_stilso->stilo);
	qr_new(a_stilso, link);
}
