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

/* This is private, but stila needs to know its size. */
struct cw_stiloe_dicto_s {
	cw_stilo_t	key;
	cw_stilo_t	val;
};

/*
 * This is private, but is exposed here to make inlining stilo_array_el_get()
 * possible.  stilt_loop() calls stilo_array_el_get() a lot, so this is critical
 * to performance.
 */
typedef struct cw_stiloe_array_s cw_stiloe_array_t;
struct cw_stiloe_array_s {
	cw_stiloe_t	stiloe;
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * arrays aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
	/*
	 * Used for remembering the current state of reference iteration.
	 */
	cw_uint32_t	ref_iter;
	union {
		struct {
			cw_stilo_t	stilo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_stilo_t	*arr;
			cw_uint32_t	len;
		}	a;
	}	e;
};

/*
 * stiloe.
 */
void	stiloe_l_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);

#define	stiloe_l_color_get(a_stiloe) (a_stiloe)->color
#define	stiloe_l_color_set(a_stiloe, a_color) (a_stiloe)->color = (a_color)

#define	stiloe_l_registered_get(a_stiloe) (a_stiloe)->registered
#define	stiloe_l_registered_set(a_stiloe, a_registered)			\
	(a_stiloe)->registered = (a_registered)

/*
 * array.
 */
#define		stiloe_p_array_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_array_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

#ifndef _CW_USE_INLINES
void	stilo_l_array_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset,
    cw_stilo_t *r_el);
#endif

#if (defined(_CW_USE_INLINES) || defined(_STILO_C_))
_CW_INLINE void
stilo_l_array_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset, cw_stilo_t
    *r_el)
{
	cw_stiloe_array_t	*array;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_ARRAY);
	_cw_check_ptr(r_el);

	array = (cw_stiloe_array_t *)a_stilo->o.stiloe;

	_cw_check_ptr(array);
	_cw_assert(array->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(array->stiloe.type == STILOT_ARRAY);

	stiloe_p_array_lock(array);
	if (array->stiloe.indirect == FALSE) {
		_cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		stilo_dup(r_el, &array->e.a.arr[a_offset]);
	} else {
		stilo_array_el_get(&array->e.i.stilo, a_offset +
		    array->e.i.beg_offset, r_el);
	}
	stiloe_p_array_unlock(array);
}
#endif

cw_stilo_t *stilo_l_array_get(cw_stilo_t *a_stilo);

#define	stilo_l_array_bound_get(a_stilo) (a_stilo)->array_bound
#define	stilo_l_array_bound_set(a_stilo, a_bound)			\
	(a_stilo)->array_bound = (a_bound);

/*
 * dict.
 */
cw_stilo_t *stilo_l_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t
    *a_key);

/*
 * name.
 */
cw_uint32_t stilo_l_name_hash(const void *a_key);
cw_bool_t stilo_l_name_key_comp(const void *a_k1, const void *a_k2);

/*
 * operator.
 */
#define	stilo_l_operator_fast_op_get(a_stilo) (a_stilo)->fast_op
#define	stilo_l_operator_fast_op_set(a_stilo, a_op_code) do {		\
	(a_stilo)->fast_op = TRUE;					\
	(a_stilo)->op_code = (a_op_code);				\
} while (0)
#define	stilo_l_operator_fast_op_stiln(a_stilo) (a_stilo)->op_code
