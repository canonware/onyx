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
 * This is private, but is exposed here to make inlining nxo_array_el_get()
 * possible.  nxo_thread_loop() calls nxo_array_el_get() a lot, so this is
 * critical to performance.
 */
typedef struct cw_nxoe_array_s cw_nxoe_array_t;
struct cw_nxoe_array_s {
	cw_nxoe_t	nxoe;
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
			cw_nxo_t	nxo;
			cw_uint32_t	beg_offset;
			cw_uint32_t	len;
		}	i;
		struct {
			cw_nxo_t	*arr;
			cw_uint32_t	len;
		}	a;
	}	e;
};

/* Private, but defined here for the inline function. */
#define		nxoe_p_array_lock(a_nxoe) do {				\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
		mtx_lock(&(a_nxoe)->lock);				\
} while (0)
#define		nxoe_p_array_unlock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
		mtx_unlock(&(a_nxoe)->lock);				\
} while (0)

void	nxoe_l_array_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx);
cw_nxoe_t *nxoe_l_array_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
void nxo_l_array_print(cw_nxo_t *a_thread);

#ifndef _CW_USE_INLINES
void	nxo_l_array_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el);
#endif

#if (defined(_CW_USE_INLINES) || defined(_NXO_ARRAY_C_))
_CW_INLINE void
nxo_l_array_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el)
{
	cw_nxoe_array_t	*array;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(a_nxo->type == NXOT_ARRAY);
	_cw_check_ptr(r_el);

	array = (cw_nxoe_array_t *)a_nxo->o.nxoe;

	_cw_check_ptr(array);
	_cw_assert(array->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(array->nxoe.type == NXOT_ARRAY);

	nxoe_p_array_lock(array);
	if (array->nxoe.indirect == FALSE) {
		_cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		nxo_dup(r_el, &array->e.a.arr[a_offset]);
	} else {
		nxo_array_el_get(&array->e.i.nxo, a_offset +
		    array->e.i.beg_offset, r_el);
	}
	nxoe_p_array_unlock(array);
}
#endif	/* (defined(_CW_USE_INLINES) || defined(_NXO_ARRAY_C_)) */

cw_nxo_t *nxo_l_array_get(cw_nxo_t *a_nxo);

#define	nxo_l_array_bound_get(a_nxo) (a_nxo)->array_bound
#define	nxo_l_array_bound_set(a_nxo, a_bound) (a_nxo)->array_bound = (a_bound)
