/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

#ifdef _LIBSTIL_DBG
#define _CW_STILS_MAGIC 0x0ea67890
#define _CW_STILSC_MAGIC 0x543e2aff
#endif

static cw_stilso_t	*stils_p_alloc_stilso(cw_stils_t *a_stils);
static cw_stilsc_t	*stilsc_p_new(cw_pezz_t *a_stilsc_pezz);
static void		 stilso_p_new(cw_stilso_t *a_stilso);
static void		 stilsc_p_delete(cw_stilsc_t *a_stilsc);
static cw_uint32_t	 stilsc_p_get_nstilso(cw_stilsc_t *a_stilsc);
static cw_stilso_t	*stilsc_p_get_stilso(cw_stilsc_t *a_stilsc, cw_uint32_t
    a_index);
static void		 stilso_p_new(cw_stilso_t *a_stilso);

cw_stils_t *
stils_new(cw_stils_t *a_stils, cw_pezz_t *a_stilsc_pezz)
{
	cw_stils_t	*retval;

	_cw_check_ptr(a_stils);
	_cw_check_ptr(a_stilsc_pezz);

	retval = a_stils;
	retval->stack = NULL;
	retval->count = 0;
	retval->spares = NULL;
	retval->nspares = 0;
	retval->stilsc_pezz = a_stilsc_pezz;
	qq_init(&retval->chunks);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILS_MAGIC;
#endif

	return retval;
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
		stils_pop(a_stils, a_stils->count);

	while (!qq_empty(&a_stils->chunks)) {
		stilsc = qq_first(&a_stils->chunks);
		qq_remove_head(&a_stils->chunks, link);
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
	qq_head(cw_stilsc_t) old_chunks;

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
	qq_init(&a_stils->chunks);

	/*
	 * Iterate through the entire stack, moving stilso's to the new stack.
	 * Along the way, report any extended objects to the GC.
	 */
	for (i = 0; i < old_count; old_stilso = qr_next(old_stilso,
	    link), i++) {
		new_stilo = stils_push(a_stils);
		stilo_move(new_stilo, &old_stilso->stilo);

		switch (stilo_type(new_stilo)) {
		case _CW_STILOT_ARRAYTYPE:
		case _CW_STILOT_CONDITIONTYPE:
		case _CW_STILOT_DICTTYPE:
		case _CW_STILOT_LOCKTYPE:
		case _CW_STILOT_MSTATETYPE:
		case _CW_STILOT_NUMBERTYPE:
		case _CW_STILOT_PACKEDARRAYTYPE:
		case _CW_STILOT_STRINGTYPE:
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
	while (!qq_empty(&old_chunks)) {
		stilsc = qq_first(&old_chunks);
		qq_remove_head(&old_chunks, link);
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
	qr_meld(stilso, a_stils->stack, link);
	a_stils->stack = stilso;
	a_stils->count++;

	return &stilso->stilo;
}

cw_bool_t
stils_pop(cw_stils_t *a_stils, cw_uint32_t a_count)
{
	cw_bool_t	retval;
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);
	
	if (a_count > a_stils->count) {
		retval = TRUE;
		goto RETURN;
	}

	/*
	 * Get a pointer to what will be the new stack top, and delete the
	 * stilo's on the way down.
	 */
	for (i = 0, stilso = a_stils->stack; i < a_count; i++) {
		stilo_delete(&stilso->stilo);
		stilso = qr_next(stilso, link);
	}

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

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t a_amount)
{
	cw_bool_t	retval;
	cw_stilso_t	*stilso, *top;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if ((a_count < 1) || (a_count > a_stils->count)) {
		retval = TRUE;
		goto RETURN;
	}

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
		/* Noop.  Return success. */
		retval = FALSE;
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

	retval = FALSE;
	RETURN:
	return retval;
}

cw_bool_t
stils_dup(cw_stils_t *a_stils, cw_uint32_t a_count, cw_uint32_t a_index)
{
	cw_bool_t	retval;
	cw_stilso_t	*orig, *dup;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0) {
		retval = TRUE;
		goto RETURN;
	}

	/* Keep track of the original object, since we're about to bury it. */
	orig = a_stils->stack;

	/* Push an unused stilso. */
	dup = stils_p_alloc_stilso(a_stils);
	qr_meld(dup, a_stils->stack, link);
	a_stils->stack = dup;
	a_stils->count++;

	/* Copy the object contents. */
	stilo_copy((cw_stilo_t *)dup, (cw_stilo_t *)orig);

	retval = FALSE;
	RETURN:
	return retval;
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

	if (a_index >= a_stils->count) {
		stilso = NULL;
		goto RETURN;
	}

	for (i = 0, stilso = a_stils->stack; i < a_index; i++)
		stilso = qr_next(stilso, link);

	RETURN:
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

		stilsc = stilsc_p_new(a_stils->stilsc_pezz);

		qq_insert_head(&a_stils->chunks, stilsc, link);
		a_stils->spares = stilsc_p_get_stilso(stilsc, 0);
		a_stils->nspares = stilsc_p_get_nstilso(stilsc);
	 }

	retval = a_stils->spares;
	a_stils->spares = qr_next(retval, link);
	qr_remove(retval, link);
	a_stils->nspares--;

	return retval;
}

/*
 * Initialize all embedded stilso's and link them into a ring.
 */
static cw_stilsc_t *
stilsc_p_new(cw_pezz_t *a_stilsc_pezz)
{
	cw_stilsc_t	*retval;
	cw_uint32_t	nstilso, i;

	retval = (cw_stilsc_t *)_cw_pezz_get(a_stilsc_pezz);
	if (retval == NULL) {
		/* XXX Report error. */
	}

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILSC_MAGIC;
#endif
	retval->allocator = a_stilsc_pezz;

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

	_cw_pezz_put(a_stilsc->allocator, a_stilsc);
}

static cw_uint32_t
stilsc_p_get_nstilso(cw_stilsc_t *a_stilsc)
{
	_cw_check_ptr(a_stilsc);
	_cw_assert(a_stilsc->magic == _CW_STILSC_MAGIC);

	return _CW_STILSC_SIZEOF2O(pezz_get_buffer_size(a_stilsc->allocator));
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
	stilo_new(&a_stilso->stilo);
	qr_new(a_stilso, link);
}
