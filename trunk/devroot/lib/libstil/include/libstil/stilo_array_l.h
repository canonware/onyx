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
 * This is private, but is exposed here to make inlining stilo_array_el_get()
 * possible.  stilo_thread_loop() calls stilo_array_el_get() a lot, so this is
 * critical to performance.
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

/* Private, but defined here for the inline function. */
#define		stiloe_p_array_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_array_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking && !(a_stiloe)->stiloe.indirect)	\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

void	stiloe_l_array_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil);
cw_stiloe_t *stiloe_l_array_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset);
cw_stilo_threade_t stilo_l_array_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file,
    cw_uint32_t a_depth);

#ifndef _CW_USE_INLINES
void	stilo_l_array_el_get(cw_stilo_t *a_stilo, cw_stiloi_t a_offset,
    cw_stilo_t *r_el);
#endif

#if (defined(_CW_USE_INLINES) || defined(_STILO_ARRAY_C_))
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
