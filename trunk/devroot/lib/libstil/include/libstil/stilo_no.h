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

/*
 * The code for stilo_dup() is fragile in that it relies on the internal layout
 * of cw_stilo_t.  In order to fix this fragility, we would need to convert the
 * bit fields to a manually managed 32 bit variable with bits that represent
 * various flags.
 *
 * The order of operations is important in order to avoid a GC race.
 */
#ifdef _LIBSTIL_DBG
#define	stilo_no_new(a_stilo) do {					\
	struct {							\
		cw_uint32_t	magic;					\
		cw_uint32_t	flags;					\
		cw_stiloi_t	data;					\
	} *x_stilo;							\
	_cw_assert(sizeof(*x_stilo) == sizeof(cw_stilo_t));		\
									\
	x_stilo = (void *)(a_stilo);					\
	_cw_assert((a_stilo)->magic == x_stilo->magic);			\
	_cw_assert((a_stilo)->o.integer.i == x_stilo->data);		\
	x_stilo->flags = 0;						\
	x_stilo->magic = 0;						\
	x_stilo->data = 0;						\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
	(a_stilo)->type = STILOT_NO;					\
} while (0)
#else
#define	stilo_no_new(a_stilo) do {					\
	struct {							\
		cw_uint32_t	flags;					\
		cw_stiloi_t	data;					\
	} *x_stilo;							\
									\
	x_stilo = (void *)(a_stilo);					\
	x_stilo->flags = 0;						\
	x_stilo->data = 0;						\
	(a_stilo)->type = STILOT_NO;					\
} while (0)
#endif
