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

/* Calculate stilsc size, given the number of stilso's. */
#define _CW_STILSC_O2SIZEOF(n)						\
	(sizeof(cw_stilsc_t) + (((n) - 1) * sizeof(cw_stilso_t)))

/* Calculate number of stilso's per stilsc, given stilsc size. */
#define _CW_STILSC_SIZEOF2O(s)						\
	((((s) - sizeof(cw_stilsc_t)) / sizeof(cw_stilso_t)) + 1)

#ifdef _LIBSTIL_DBG
#define _CW_STILS_MAGIC 0x0ea67890
#define _CW_STILSC_MAGIC 0x543e2aff
#endif

typedef struct cw_stils_s cw_stils_t;
typedef struct cw_stilso_s cw_stilso_t;

struct cw_stilso_s {
	cw_stilo_t		stilo;	/* Payload.  Must be first field. */
	ql_elm(cw_stilso_t)	link;	/* Stack/spares linkage. */
};

struct cw_stilsc_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t		magic;
#endif
	qs_elm(cw_stilsc_t)	link;	/* Linkage for the stack of stilsc's. */

	/*
	 * Must be last field, since it is used for array indexing of
	 * stilso's beyond the end of the structure.
	 */
	cw_stilso_t		objects[1];
};

struct cw_stils_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t		magic;
#endif
	ql_head(cw_stilso_t)	stack;	/* Stack. */
	cw_uint32_t		count;	/* Number of stack elements. */
	cw_stilso_t		under;	/* Not used, just under stack bottom. */

	cw_pool_t		*stilsc_pool; /* Allocator for stilsc's. */

	qs_head(cw_stilsc_t)	chunks;	/* List of stilsc's. */
};

void		stils_new(cw_stils_t *a_stils, cw_pool_t *a_stilsc_pool);
void		stils_delete(cw_stils_t *a_stils);
void		stils_collect(cw_stils_t *a_stils, void (*a_add_root_func) (void
    *add_root_arg, cw_stilo_t *root), void *a_add_root_arg);
#define		stils_count(a_stils) (a_stils)->count
cw_uint32_t	stils_index_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
void		stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t
    a_amount);

#ifndef _CW_USE_INLINES
cw_stilo_t	*stils_push(cw_stils_t *a_stils);
cw_stilo_t	*stils_under_push(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
cw_bool_t	stils_pop(cw_stils_t *a_stils);
cw_bool_t	stils_npop(cw_stils_t *a_stils, cw_uint32_t a_count);

cw_stilo_t	*stils_get(cw_stils_t *a_stils);
cw_stilo_t	*stils_nget(cw_stils_t *a_stils, cw_uint32_t a_index);
cw_stilo_t	*stils_down_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
#endif

/* Private, but the inline functions need its prototype. */
void		stils_p_spares_create(cw_stils_t *a_stils);

#if (defined(_CW_USE_INLINES) || defined(_STILS_C_))
_CW_INLINE cw_stilo_t *
stils_push(cw_stils_t *a_stils)
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

_CW_INLINE cw_stilo_t *
stils_under_push(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qr_prev(ql_first(&a_stils->stack), link) == &a_stils->under)
		stils_p_spares_create(a_stils);
	if (a_stilo != NULL) {
		stilso = qr_prev(ql_first(&a_stils->stack), link);
		qr_remove(stilso, link);
		qr_after_insert((cw_stilso_t *)a_stilo, stilso, link);
	} else {
		/* Same as stils_push(). */
		ql_first(&a_stils->stack) = qr_prev(ql_first(&a_stils->stack),
		    link);
		stilso = ql_first(&a_stils->stack);
	}
	a_stils->count++;

	stilo_no_new(&stilso->stilo);

	return &stilso->stilo;
}

_CW_INLINE cw_bool_t
stils_pop(cw_stils_t *a_stils)
{
	cw_bool_t	retval;
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0) {
		retval = TRUE;
		goto RETURN;
	}

	stilso = ql_first(&a_stils->stack);
	ql_first(&a_stils->stack) = qr_next(ql_first(&a_stils->stack), link);
	a_stils->count--;

	retval = FALSE;
	RETURN:
	return retval;
}

_CW_INLINE cw_bool_t
stils_npop(cw_stils_t *a_stils, cw_uint32_t a_count)
{
	cw_bool_t	retval;
	cw_stilso_t	*top;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);

	if (a_count > a_stils->count) {
		retval = TRUE;
		goto RETURN;
	}

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, top = ql_first(&a_stils->stack); i < a_count; i++)
		top = qr_next(top, link);

	ql_first(&a_stils->stack) = top;
	a_stils->count -= a_count;

	retval = FALSE;
	RETURN:
	return retval;
}

_CW_INLINE cw_stilo_t *
stils_get(cw_stils_t *a_stils)
{
	cw_stilo_t	*retval;
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stils->count == 0) {
		retval = NULL;
		goto RETURN;
	}

	stilso = ql_first(&a_stils->stack);

	retval = &stilso->stilo;
	RETURN:
	return retval;
}

_CW_INLINE cw_stilo_t *
stils_nget(cw_stils_t *a_stils, cw_uint32_t a_index)
{
	cw_stilo_t	*retval;
	cw_stilso_t	*stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_index >= a_stils->count) {
		retval = NULL;
		goto RETURN;
	}

	for (i = 0, stilso = ql_first(&a_stils->stack); i < a_index; i++)
		stilso = qr_next(stilso, link);

	retval = &stilso->stilo;
	RETURN:
	return retval;
}

_CW_INLINE cw_stilo_t *
stils_down_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo)
{
	cw_stilo_t	*retval;
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	if (a_stilo != NULL) {
		if (a_stils->count <= 1) {
			retval = NULL;
			goto RETURN;
		}
		stilso = (cw_stilso_t *)a_stilo;
		stilso = qr_next(stilso, link);
		if (stilso == &a_stils->under) {
			retval = NULL;
			goto RETURN;
		}
	} else {
		/* Same as stils_get(). */
		if (a_stils->count == 0) {
			retval = NULL;
			goto RETURN;
		}

		stilso = ql_first(&a_stils->stack);
	}

	retval = &stilso->stilo;
	RETURN:
	return retval;
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_STILS_C_)) */

/*
 * Convenience wrapper macros for use where errors should cause an error and
 * immediate return.
 */
#define	STILS_POP(a_stils, a_stilt) do {				\
	if (stils_pop(a_stils)) {					\
		stilt_error((a_stilt), STILTE_STACKUNDERFLOW);		\
		return;							\
	}								\
} while (0)

#define	STILS_NPOP(a_stils, a_stilt, a_count) do {			\
	if (stils_npop((a_stils), (a_count)) {				\
		stilt_error((a_stilt), STILTE_STACKUNDERFLOW);		\
		return;							\
	}								\
} while (0)

#define	STILS_GET(r_stilo, a_stils, a_stilt) do {			\
	(r_stilo) = stils_get(a_stils);					\
	if ((r_stilo) == NULL) {					\
		stilt_error((a_stilt), STILTE_STACKUNDERFLOW);		\
		return;							\
	}								\
} while (0)

#define	STILS_NGET(r_stilo, a_stils, a_stilt, a_index) do {		\
	(r_stilo) = stils_nget((a_stils), (a_index));			\
	if ((r_stilo) == NULL) {					\
		stilt_error((a_stilt), STILTE_STACKUNDERFLOW);		\
		return;							\
	}								\
} while (0)

#define	STILS_DOWN_GET(r_stilo, a_stils, a_stilt, a_stilo) do {		\
	(r_stilo) = stils_down_get((a_stils), (a_stilo));		\
	if ((r_stilo) == NULL) {					\
		stilt_error((a_stilt), STILTE_STACKUNDERFLOW);		\
		return;							\
	}								\
} while (0)
