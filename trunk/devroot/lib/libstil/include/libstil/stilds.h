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
 ****************************************************************************/

typedef struct cw_stilds_s cw_stilds_t;

struct cw_stilds_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	cw_stils_t stack;

	cw_ch_t hash;
};

struct cw_stildsi_s {
#if (defined(_LIBSTIL_DBG) || defined(_LIBSTIL_DEBUG))
	cw_uint32_t magic;
#endif

	cw_stilo_t name;

	/* Linkage into a ring of stildo's that all have the same name. */
	cw_ring_t link;
};
