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
 * Don't actually free stiloe's if debugging GC.  Instead, just reset the
 * stiloe magic.  This way, we should still core dump when we hit collected
 * stiloe's, but can actually see the old contents of the stiloe we tried
 * to use.
 */
#if (0)
#define	_CW_FREE(a_stilo)
#define	_CW_STILOE_FREE(a_stiloe) (a_stiloe)->stiloe.magic = 0
#else
#define	_CW_FREE(a_stilo) _cw_free(a_stilo)
#define	_CW_STILOE_FREE(a_stiloe) _cw_free(a_stiloe)
#endif

/*
 * stilo.
 */
/* Call before other initialization. */
#ifdef _LIBSTIL_DBG
#define	stilo_l_new(a_stilo, a_type) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = (a_type);					\
	(a_stilo)->magic = _CW_STILO_MAGIC;				\
} while (0)
#else
#define	stilo_l_new(a_stilo, a_type) do {				\
	memset((a_stilo), 0, sizeof(cw_stilo_t));			\
	(a_stilo)->type = (a_type);					\
} while (0)
#endif

/*
 * stiloe.
 */
void	stiloe_l_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type, cw_bool_t
    a_locking);
void	stiloe_l_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);

#define	stiloe_l_color_get(a_stiloe) (a_stiloe)->color
#define	stiloe_l_color_set(a_stiloe, a_color) (a_stiloe)->color = (a_color)

#define	stiloe_l_registered_get(a_stiloe) (a_stiloe)->registered
#define	stiloe_l_registered_set(a_stiloe, a_registered)			\
	(a_stiloe)->registered = (a_registered)
