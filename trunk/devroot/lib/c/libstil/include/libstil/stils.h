/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * Stacks are implemented by the stils class.  Stack object space is allocated
 * in chunks (implemented by the stilsc class) in order to improve locality and
 * reduce memory fragmentation.  Freed objects within a chunk are kept in a ring
 * (LIFO ordering) and re-used.  This has the potential to cause adjacent stack
 * objects to be scattered throughout the stilsc's, but typical stack operations
 * have the same effect anyway, so little care is taken to keep stack object
 * re-allocation contiguous, or even local.
 *
 * Since the GC must traverse the entire stack at every collection, we use that
 * opportunity to tidy things up.  The entire stack is re-written contiguously,
 * and the old stilsc's are returned to the global pool.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 ****************************************************************************/

/* Calculate stilsc size, given the number of stilo's. */
#define _CW_STILSC_O2SIZEOF(n)						\
	(sizeof(cw_stilsc_t) + (((n) - 1) * sizeof(cw_stilso_t)))

/* Calculate number of stilo's per stilsc, given stilsc size. */
#define _CW_STILSC_SIZEOF2O(s)						\
	((((s) - sizeof(cw_stilsc_t)) / sizeof(cw_stilso_t)) + 1)

typedef struct cw_stils_s cw_stils_t;
typedef struct cw_stilso_s cw_stilso_t;
typedef struct cw_stilsc_s cw_stilsc_t;

struct cw_stilso_s {
	cw_stilo_t	stilo;		/* Payload.  Must be first field. */
	qr_entry(cw_stilso_t) link;	/* Stack/spares ring linkage. */
};

struct cw_stilsc_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_pool_t	*stilsc_pool;	/* stilsc allocator. */

	qq_entry(cw_stilsc_t) link;	/* Linkage for the list of stilsc's. */

	/*
	 * Must be last field, since it is used for array indexing of
	 * stilso's beyond the end of the structure.
	 */
	cw_stilso_t	objects[1];
};

struct cw_stils_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t	magic;
#endif

	cw_stilso_t	*stack;		/* Pointer to the top of the stack. */
	cw_uint32_t	count;		/* Number of stack elements. */
	
	cw_stilso_t	*spares;	/* Pointer to a ring of spare slots. */
	cw_uint32_t	nspares;	/* Number of spares. */

	cw_pool_t	*stilsc_pool;	/* Allocator for stilsc's. */

	qq_head(cw_stilsc_t) chunks;	/* List of stilsc's. */
};

cw_stils_t	*stils_new(cw_stils_t *a_stils, cw_pool_t *a_stilsc_pool);

void		stils_delete(cw_stils_t *a_stils);

void		stils_collect(cw_stils_t *a_stils, void (*a_add_root_func)
    (void *add_root_arg, cw_stilo_t *root), void *a_add_root_arg);

cw_stilo_t	*stils_push(cw_stils_t *a_stils, cw_stilt_t *a_stilt,
    cw_stilot_t a_type, ...);

cw_bool_t	stils_pop(cw_stils_t *a_stils, cw_uint32_t a_count);

cw_bool_t	stils_roll(cw_stils_t *a_stils, cw_uint32_t a_count, cw_sint32_t
    a_amount);

cw_bool_t	stils_dup(cw_stils_t *a_stils, cw_stilt_t *a_stilt, cw_uint32_t
    a_count, cw_uint32_t a_index);

cw_uint32_t	stils_count(cw_stils_t *a_stils);

cw_stilo_t	*stils_get(cw_stils_t *a_stils, cw_uint32_t a_index);

cw_stilo_t	*stils_get_down(cw_stils_t *a_stils, cw_stilo_t *a_stilo);

cw_stilo_t	*stils_get_up(cw_stils_t *a_stils, cw_stilo_t *a_stilo);

cw_uint32_t	stils_get_index(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
