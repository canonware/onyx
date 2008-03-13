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
 * nxo.
 */
#define	nxo_p_type_set(a_nxo, a_type) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xffffffe0) | (a_type);	\
} while (0)

/*
 * nxoe.
 */
void	nxoe_l_new(cw_nxoe_t *a_nxoe, cw_nxot_t a_type, cw_bool_t
    a_locking);
void	nxoe_l_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa);
cw_nxoe_t *nxoe_l_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

#define	nxoe_l_color_get(a_nxoe) (a_nxoe)->color
#define	nxoe_l_color_set(a_nxoe, a_color) (a_nxoe)->color = (a_color)

#define	nxoe_l_registered_get(a_nxoe) (a_nxoe)->registered
#define	nxoe_l_registered_set(a_nxoe, a_registered)			\
	(a_nxoe)->registered = (a_registered)
