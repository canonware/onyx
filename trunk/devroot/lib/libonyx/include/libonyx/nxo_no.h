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
 * The code for nxo_dup() is fragile in that it relies on the internal layout
 * of cw_nxo_t.  In order to fix this fragility, we would need to convert the
 * bit fields to a manually managed 32 bit variable with bits that represent
 * various flags.
 *
 * The order of operations is important in order to avoid a GC race.
 */
#ifdef _LIBONYX_DBG
#define	nxo_no_new(a_nxo) do {						\
	struct {							\
		cw_uint32_t	magic;					\
		cw_uint32_t	flags;					\
		cw_nxoi_t	data;					\
	} *x_nxo;							\
	_cw_assert(sizeof(*x_nxo) == sizeof(cw_nxo_t));			\
									\
	x_nxo = (void *)(a_nxo);					\
	_cw_assert((a_nxo)->magic == x_nxo->magic);			\
	_cw_assert((a_nxo)->o.integer.i == x_nxo->data);		\
	x_nxo->flags = 0;						\
	x_nxo->magic = 0;						\
	x_nxo->data = 0;						\
	(a_nxo)->magic = _CW_NXO_MAGIC;					\
	(a_nxo)->type = NXOT_NO;					\
} while (0)
#else
#define	nxo_no_new(a_nxo) do {						\
	struct {							\
		cw_uint32_t	flags;					\
		cw_nxoi_t	data;					\
	} *x_nxo;							\
									\
	x_nxo = (void *)(a_nxo);					\
	x_nxo->flags = 0;						\
	x_nxo->data = 0;						\
	(a_nxo)->type = NXOT_NO;					\
} while (0)
#endif
