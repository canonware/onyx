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

/* Calculate stilsc size, given the number of stilso's. */
#define _CW_STILSC_O2SIZEOF(n)						\
	(sizeof(cw_stilsc_t) + (((n) - 1) * sizeof(cw_stilso_t)))

/* Calculate number of stilso's per stilsc, given stilsc size. */
#define _CW_STILSC_SIZEOF2O(s)						\
	((((s) - sizeof(cw_stilsc_t)) / sizeof(cw_stilso_t)) + 1)

typedef struct cw_stils_s cw_stils_t;
typedef struct cw_stilso_s cw_stilso_t;

struct cw_stilso_s {
	cw_stilo_t		stilo;	/* Payload.  Must be first field. */
	ql_elm(cw_stilso_t)	link;	/* Stack/spares linkage. */
};

struct cw_stilsc_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
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
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
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

cw_stilo_t	*stils_push(cw_stils_t *a_stils);
cw_stilo_t	*stils_under_push(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
cw_bool_t	stils_pop(cw_stils_t *a_stils);
cw_bool_t	stils_npop(cw_stils_t *a_stils, cw_uint32_t a_count);
void		stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t
    a_amount);
#define		stils_count(a_stils) (a_stils)->count

cw_stilo_t	*stils_get(cw_stils_t *a_stils);
cw_stilo_t	*stils_nget(cw_stils_t *a_stils, cw_uint32_t a_index);
cw_stilo_t	*stils_down_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
cw_uint32_t	stils_index_get(cw_stils_t *a_stils, cw_stilo_t *a_stilo);

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
