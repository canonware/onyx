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
	 * Dictionary permissions.  Only dictionaries share permissions accross
	 * all references.
	 */
	cw_stilop_t	perms:2;
	/*
	 * Since dictionaries share permissions across all references, and
	 * permissions can only ever decrease, it is possible to avoid locking
	 * even global dict's, if the permissions are unlimited.  In order to
	 * avoid a race condition, this field is set to TRUE when a dict is
	 * locked, so that it can be correctly unlocked, even if the dict's
	 * permissions change in the meanwhile.
	 */
	cw_bool_t	dict_locked:1;
	/*
	 * If TRUE, the string in the key is statically allocated, and should
	 * not be deallocated during destruction.
	 */
	cw_bool_t	name_static:1;
	/*
	 * All name creations toggle this value to TRUE, and the GC toggles
	 * this value to FALSE for garbage names.  During actual name deletion,
	 * we check to see if the value has been toggled back to TRUE, and abort
	 * deletion if so.
	 */
	cw_bool_t	name_referenced:1;
	/*
	 * If TRUE, there is a watchpoint set on this object.  In general, this
	 * field is not looked at unless the interpreter has been put into
	 * debugging mode. Note that setting a watchpoint on an extended type
	 * causes modification via *any* reference to be watched.
	 */
	cw_bool_t	watchpoint:1;
	/*
	 * The GC toggles this value at each collection in order to maintain
	 * state.
	 */
	cw_bool_t	color:1;
	/*
	 * TRUE if this object has been registered with the GC.
	 */
	cw_bool_t	registered:1;
	/*
	 * Allocated locally or globally?
	 */
	cw_bool_t	global:1;
	/*
	 * If TRUE, this stiloe is a reference to another stiloe.
	 */
	cw_bool_t	indirect:1;
};

/* This is private, but stila needs to know its size. */
struct cw_stiloe_dicto_s {
	cw_stilo_t	key;
	cw_stilo_t	val;
};

cw_stilte_t stiloe_l_print(cw_stiloe_t *a_stiloe, cw_stilo_t *a_file, cw_bool_t
    a_syntactic, cw_bool_t a_newline);

void	stiloe_l_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);

#define	stiloe_l_color_get(a_stiloe) (a_stiloe)->color
#define	stiloe_l_color_set(a_stiloe, a_color) (a_stiloe)->color = (a_color)

#define	stiloe_l_registered_get(a_stiloe) (a_stiloe)->registered
#define	stiloe_l_registered_set(a_stiloe, a_registered)			\
	(a_stiloe)->registered = (a_registered)

/*
 * name.
 */
cw_uint32_t	stilo_l_name_hash(const void *a_key);
cw_bool_t	stilo_l_name_key_comp(const void *a_k1, const void *a_k2);
