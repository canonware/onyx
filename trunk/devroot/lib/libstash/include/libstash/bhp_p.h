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
 * <<< Description >>>
 *
 * Private header for bhp class (binomial heap).
 *
 ****************************************************************************/

struct cw_bhpi_s
{
  void * priority;
  void * data;
  struct cw_bhpi_s * parent;
  struct cw_bhpi_s * child;
  struct cw_bhpi_s * sibling;
  cw_uint32_t degree;
};

static cw_bhpi_t *
bhp_p_dump(cw_bhpi_t * a_bhpi, cw_uint32_t a_depth, cw_bhpi_t * a_last_printed);

static void
bhp_p_bin_link(cw_bhpi_t * a_root, cw_bhpi_t * a_non_root);

static void
bhp_p_merge(cw_bhp_t * a_bhp, cw_bhp_t * a_other);

static cw_sint32_t
bhp_p_priority_compare(void * a_a, void * a_b);
