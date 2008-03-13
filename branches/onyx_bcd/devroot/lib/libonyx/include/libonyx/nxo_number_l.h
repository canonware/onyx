/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

cw_bool_t
nxo_l_number_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_number_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

#define nxo_l_number_small_get(a_nxo) (((a_nxo)->flags >> 9) & 1)
#define nxo_l_number_small_set(a_nxo, a_small)				\
    do									\
    {									\
	(a_nxo)->flags = ((a_nxo)->flags & 0xfffffdff) |		\
	    ((a_small) << 9);						\
    } while (0)
