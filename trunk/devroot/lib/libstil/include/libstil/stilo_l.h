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

#ifdef _LIBSTIL_DBG
#define _CW_STILOE_MAGIC	0x0fa6e798
#endif

/*
 * All extended type objects contain a stiloe.  This provides a poor man's
 * inheritance.  Since stil's type system is static, this idiom is adequate.
 */
struct cw_stiloe_s {
#ifdef _LIBSTIL_DBG
	cw_uint32_t	magic;
#endif

	/*
	 * Linkage for GC.  All stiloe's are in a single ring, which the GC uses
	 * to implement a Baker's Treadmill collector.
	 */
	qr(cw_stiloe_t)	link;
	/*
	 * Object type.  We store this in stiloe's as well as stilo's, since
	 * various functions access stiloe's directly, rather than going through
	 * a referring stilo.
	 */
	cw_stilot_t	type:4;
	/*
	 * If TRUE, the string in the key is statically allocated, and should
	 * not be deallocated during destruction.
	 */
	cw_bool_t	name_static:1;
	/*
	 * The GC toggles this value at each collection in order to maintain
	 * state.
	 */
	cw_bool_t	color:1;
	/*
	 * TRUE if this object has been registered with the GC.  It is possible
	 * for an object to be reachable by the GC (on a stack, for instance),
	 * but not be registered yet.
	 */
	cw_bool_t	registered:1;
	/*
	 * If true, accesses to this object are locked.  This applies to arrays,
	 * dictionaries, files, and strings.
	 */
	cw_bool_t	locking:1;
	/*
	 * If TRUE, this stiloe is a reference to another stiloe.
	 */
	cw_bool_t	indirect:1;
};

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
