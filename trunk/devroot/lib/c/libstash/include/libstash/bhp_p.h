/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 * <<< Description >>>
 *
 * Private header for bhp class (binomial heap).
 *
 ****************************************************************************/

#ifdef _LIBSTASH_DBG
#  define _LIBSTASH_BHP_MAGIC 0xbf917ca1
#  define _LIBSTASH_BHPI_MAGIC 0xbf90ec15
#endif

static cw_bhpi_t *
bhp_p_dump(cw_bhpi_t * a_bhpi, cw_uint32_t a_depth, cw_bhpi_t * a_last_printed);

static void
bhp_p_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root);

static void
bhp_p_merge(cw_bhp_t * a_bhp, cw_bhp_t * a_other);

static void
bhp_p_union(cw_bhp_t * a_bhp, cw_bhp_t * a_other);
