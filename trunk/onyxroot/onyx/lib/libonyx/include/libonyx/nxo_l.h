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
/* Call before other initialization. */
#ifdef _LIBONYX_DBG
#define	nxo_l_new(a_nxo, a_type) do {					\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->type = (a_type);					\
	(a_nxo)->magic = _CW_NXO_MAGIC;					\
} while (0)
#else
#define	nxo_l_new(a_nxo, a_type) do {					\
	memset((a_nxo), 0, sizeof(cw_nxo_t));				\
	(a_nxo)->type = (a_type);					\
} while (0)
#endif

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
