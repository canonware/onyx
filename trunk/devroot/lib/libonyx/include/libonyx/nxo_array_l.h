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
#ifdef CW_THREADS
	/*
	 * Access is locked if this object has the locking bit set.  Indirect
	 * arrays aren't locked, but their parents are.
	 */
	cw_mtx_t	lock;
#endif
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
			cw_uint32_t	alloc_len;
		}	a;
	}	e;
};

#ifdef CW_THREADS
/* Private, but defined here for the inline function. */
#define		nxoe_p_array_lock(a_nxoe) do {				\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
		mtx_lock(&(a_nxoe)->lock);				\
} while (0)
#define		nxoe_p_array_unlock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
		mtx_unlock(&(a_nxoe)->lock);				\
} while (0)
#endif

cw_bool_t nxoe_l_array_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t
    a_iter);
cw_nxoe_t *nxoe_l_array_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

#ifndef CW_USE_INLINES
#define	nxo_l_array_el_get nxo_array_el_get
void	nxo_l_array_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el);
cw_bool_t nxo_l_array_bound_get(cw_nxo_t *a_nxo);
void	nxo_l_array_bound_set(cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(_NXO_ARRAY_C_))
CW_INLINE void
nxo_l_array_el_get(cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el)
{
	cw_nxoe_array_t	*array;

	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);
	cw_check_ptr(r_el);

	array = (cw_nxoe_array_t *)a_nxo->o.nxoe;

	cw_check_ptr(array);
	cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
	cw_assert(array->nxoe.type == NXOT_ARRAY);

#ifdef CW_THREADS
	nxoe_p_array_lock(array);
#endif
	if (array->nxoe.indirect == FALSE) {
		cw_assert(a_offset < array->e.a.len && a_offset >= 0);
		nxo_dup(r_el, &array->e.a.arr[a_offset]);
	} else {
		nxo_array_el_get(&array->e.i.nxo, a_offset +
		    array->e.i.beg_offset, r_el);
	}
#ifdef CW_THREADS
	nxoe_p_array_unlock(array);
#endif
}
CW_INLINE cw_bool_t
nxo_l_array_bound_get(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

	return ((a_nxo->flags >> 8) & 1);
}

CW_INLINE void
nxo_l_array_bound_set(cw_nxo_t *a_nxo)
{
	cw_check_ptr(a_nxo);
	cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
	cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

	a_nxo->flags = (a_nxo->flags & 0xfffffeff) | (1 << 8);
}
#endif	/* (defined(CW_USE_INLINES) || defined(_NXO_ARRAY_C_)) */
