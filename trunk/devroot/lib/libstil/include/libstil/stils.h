/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
 * opportunity to tidy things up.  The entire stack is re-written contiguously
 * in place, and freed stilsc's are returned to the global pool.
 *
 * By keeping the re-allocation algorithm simple, we are able to make common
 * stack operations very fast.
 *
 ****************************************************************************/

/* Calculate stilsc size, given the number of stilo's. */
#define _CW_STILSC_O2SIZEOF(n)

/* Calculate number of stilo's per stilsc, given stilsc size. */
#define _CW_STILSC_SIZEOF2O(s)

typedef struct cw_stils_s cw_stils_t;
typedef struct cw_stilso_s cw_stilso_t;
typedef struct cw_stilsc_s cw_stilsc_t;

struct cw_stilso_s {
	/*
	 * The payload.  This must be first in the structure, since pointers
	 * are cast between (cw_stilso_t *) and (cw_stilo_t *).
	 */
	cw_stilo_t stilo;

#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	/*
	 * Stack linkage.  If a spare slot, this field is used to link into
	 * the stils-wide spares ring.  If this stilso is empty/invalid, the
	 * least significant bit of the data pointer is 1.  This allows the
	 * GC to deallocate completely unused stilsc's by iteratively
	 * unlinking all stilso's in an empty stilsc.
	 */
	cw_ring_t link;
};

struct cw_stilsc_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	/* Pointer to the stils's spares linkage. */
	cw_ring_t *spares;

	/*
	 * Linkage into a ring of stilsc's in the same stils.  The GC uses
	 * this ring to iterate through the stils, and potentially
	 * deallocate empty stilsc's.
	 */
	cw_ring_t link;

	/* stils this stilsc is part of. */
	cw_stils_t *stils;

	/*
	 * Must be last field, since it is used for array indexing of
	 * stilso's beyond the end of the structure.
	 */
	cw_stilso_t objects[1];
};

struct cw_stils_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	/* Linkage to the top of the stack. */
	cw_ring_t top;

	/* Linkage to the ring of spare slots. */
	cw_ring_t spares;

	cw_pezz_t *stilsc_pezz;

	/* Number of stack elements. */
	cw_uint32_t count;
};

cw_stils_t *stils_new(cw_stils_t *a_stils, cw_pezz_t *a_chunk_pezz);

void    stils_delete(cw_stils_t *a_stils);

void    stils_collect(cw_stils_t *a_stils, void (*a_add_root_func)
    (void *add_root_arg, cw_stiloe_t *root), void *a_add_root_arg);

cw_stilo_t *stils_push(cw_stils_t *a_stils);

cw_bool_t stils_pop(cw_stils_t *a_stils, cw_uint32_t a_count);

cw_bool_t stils_roll(cw_stils_t *a_stils, cw_sint32_t a_count);

cw_bool_t stils_dup(cw_stils_t *a_stils, cw_uint32_t a_count, cw_uint32_t
    a_index);

cw_uint32_t stils_count(cw_stils_t *a_stils);

cw_stilo_t *stils_get(cw_stils_t *a_stils, cw_uint32_t a_index);

cw_stilo_t *stils_get_down(cw_stils_t *a_stils, cw_stilo_t *a_stilo);

cw_stilo_t *stils_get_up(cw_stils_t *a_stils, cw_stilo_t *a_stilo);

cw_bool_t stils_get_index(cw_stils_t *a_stils, cw_stilo_t *a_stilo);
