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

#ifdef _LIBSTIL_DBG
#define _CW_STILS_MAGIC 0x0ea67890
#endif

typedef struct cw_stils_s cw_stils_t;
typedef struct cw_stilso_s cw_stilso_t;

struct cw_stilso_s {
	cw_stilo_t		stilo;	/* Payload.  Must be first field. */
	ql_elm(cw_stilso_t)	link;	/* Stack/spares linkage. */
	cw_stilsc_t		*stilsc; /* Container stilsc. */
};

struct cw_stilsc_s {
	ql_elm(cw_stilsc_t)	link;	/* Linkage for the stack of stilsc's. */

	cw_uint32_t		nused;	/* Number of objects in use. */

	cw_stilso_t		objects[_LIBSTIL_STILSC_COUNT];
};

struct cw_stils_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t		magic;
#endif
	cw_stilt_t		*stilt;
	ql_head(cw_stilso_t)	stack;	/* Stack. */
	cw_uint32_t		count;	/* Number of stack elements. */
	cw_uint32_t		nspare;	/* Number of spare elements. */
	cw_stilso_t		under;	/* Not used, just under stack bottom. */

	ql_head(cw_stilsc_t)	chunks;	/* List of stilsc's. */

	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_stilso_t		*ref_stilso;
};

void		stils_new(cw_stils_t *a_stils, cw_stilt_t *a_stilt);
void		stils_delete(cw_stils_t *a_stils);
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

/* Private, but the inline functions need their prototypes. */
void		stils_p_spares_create(cw_stils_t *a_stils);
void		stils_p_spares_destroy(cw_stils_t *a_stils, cw_stilsc_t
    *a_stilsc);

#if (defined(_CW_USE_INLINES) || defined(_STILS_C_))
_CW_INLINE cw_stilo_t *
stils_push(cw_stils_t *a_stils)
{
	cw_stilso_t	*stilso;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);

	/* Get an unused stilso.  If there are no spares, create some first. */
	if (qr_prev(ql_first(&a_stils->stack), link) == &a_stils->under)
		stils_p_spares_create(a_stils);
	stilso = qr_prev(ql_first(&a_stils->stack), link);
	stilo_no_new(&stilso->stilo);
	ql_first(&a_stils->stack) = stilso;
	stilso->stilsc->nused++;
	a_stils->count++;
	a_stils->nspare--;

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
		stilo_no_new(&stilso->stilo);
		qr_remove(stilso, link);
		qr_after_insert((cw_stilso_t *)a_stilo, stilso, link);
	} else {
		/* Same as stils_push(). */
		stilso = qr_prev(ql_first(&a_stils->stack), link);
		stilo_no_new(&stilso->stilo);
		ql_first(&a_stils->stack) = stilso;
	}
	stilso->stilsc->nused++;
	a_stils->count++;
	a_stils->nspare--;

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
	stilso->stilsc->nused--;
	a_stils->count--;
	a_stils->nspare++;
	if (stilso->stilsc->nused == 0 && a_stils->nspare >
	    2 * _LIBSTIL_STILSC_COUNT)
		stils_p_spares_destroy(a_stils, stilso->stilsc);

	retval = FALSE;
	RETURN:
	return retval;
}

_CW_INLINE cw_bool_t
stils_npop(cw_stils_t *a_stils, cw_uint32_t a_count)
{
	cw_bool_t	retval;
	cw_stilso_t	*top, *stilso;
	cw_uint32_t	i;

	_cw_check_ptr(a_stils);
	_cw_assert(a_stils->magic == _CW_STILS_MAGIC);
	_cw_assert(a_count > 0);

	if (a_count > a_stils->count) {
		retval = TRUE;
		goto RETURN;
	}

	/* Get a pointer to what will be the new stack top. */
	for (i = 0, top = ql_first(&a_stils->stack); i < a_count; i++) {
		stilso = top;
		top = qr_next(top, link);

		stilso->stilsc->nused--;
		a_stils->nspare++;
		if (stilso->stilsc->nused == 0 && a_stils->nspare > 2 *
		    _LIBSTIL_STILSC_COUNT)
			stils_p_spares_destroy(a_stils, stilso->stilsc);
	}

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
