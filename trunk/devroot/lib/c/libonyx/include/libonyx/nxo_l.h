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

#if (0)
/*
 * Don't actually free nxoe's.  Instead, just reset the nxoe magic.  This way,
 * we should nxl core dump when we hit collected nxoe's, but can actually see
 * the old contents of the nxoe we tried to use.  This leaks tons of memory, so
 * should only be used for debugging.
 */
#define	_CW_FREE(a_nxo)
#define	_CW_NXOE_FREE(a_nxoe) (a_nxoe)->nxoe.magic = 0
#else
#define	_CW_FREE(a_nxo) _cw_free(a_nxo)
#define	_CW_NXOE_FREE(a_nxoe) _cw_free(a_nxoe)
#endif

/*
 * nxo.
 */
#define	nxo_p_type_set(a_nxo, a_type) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xffffffe0) | (a_type);	\
} while (0)

#define	nxo_p_opcode_get(a_nxo) (((a_nxo)->flags >> 8) & 0x3ff)
#define	nxo_p_opcode_set(a_nxo, a_opcode) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xfffc00ff) |		\
	    ((a_opcode) << 8);						\
} while (0)

#define	nxo_p_fastop_get(a_nxo) (((a_nxo)->flags >> 5) & 1)
#define	nxo_p_fastop_set(a_nxo, a_fastop) do {				\
	(a_nxo)->flags = ((a_nxo)->flags & 0xffffffdf) |		\
	    ((a_fastop) << 5);						\
} while (0)

/*
 * nxoe.
 */
void	nxoe_l_new(cw_nxoe_t *a_nxoe, cw_nxot_t a_type, cw_bool_t
    a_locking);
void	nxoe_l_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

#define	nxoe_l_color_get(a_nxoe) (a_nxoe)->color
#define	nxoe_l_color_set(a_nxoe, a_color) (a_nxoe)->color = (a_color)

#define	nxoe_l_registered_get(a_nxoe) (a_nxoe)->registered
#define	nxoe_l_registered_set(a_nxoe, a_registered)			\
	(a_nxoe)->registered = (a_registered)
