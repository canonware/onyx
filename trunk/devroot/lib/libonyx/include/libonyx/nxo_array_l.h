/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

/* This is private, but is exposed here to make inlining nxo_array_el_get()
 * possible.  nxo_thread_loop() calls nxo_array_el_get() a lot, so this is
 * critical to performance. */
typedef struct cw_nxoe_array_s cw_nxoe_array_t;
struct cw_nxoe_array_s
{
    cw_nxoe_t nxoe;
#ifdef CW_THREADS
    /* Access is locked if this object has the locking bit set.  Indirect
     * arrays aren't locked, but their parents are. */
    cw_mtx_t lock;
#endif
    union
    {
	struct
	{
	    cw_nxo_t nxo;
	    cw_uint32_t beg_offset;
	    cw_uint32_t len;
	} i;
	struct
	{
	    cw_nxo_t *arr;
	    cw_uint32_t len;
	    cw_uint32_t alloc_len;
	} a;
    } e;
};

#ifdef CW_THREADS
/* Private, but defined here for the inline function. */
#define nxoe_p_array_lock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
	{								\
	    mtx_lock(&(a_nxoe)->lock);					\
	}								\
    } while (0)
#define nxoe_p_array_unlock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking && !(a_nxoe)->nxoe.indirect)		\
	{								\
	    mtx_unlock(&(a_nxoe)->lock);				\
	}								\
    } while (0)
#endif

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_array_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_array_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

#define nxo_l_array_el_get nxo_array_el_get

void
nxo_l_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el);

cw_bool_t
nxo_l_array_bound_get(const cw_nxo_t *a_nxo);

void
nxo_l_array_bound_set(cw_nxo_t *a_nxo);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_ARRAY_C_))
CW_INLINE cw_bool_t
nxoe_l_array_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_array_t *array;

    array = (cw_nxoe_array_t *) a_nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

    if (array->nxoe.indirect == FALSE && array->e.a.alloc_len > 0)
    {
	nxa_free(a_nxa, array->e.a.arr,
		 array->e.a.alloc_len * sizeof(cw_nxo_t));
    }

#ifdef CW_THREADS
    if (array->nxoe.locking && array->nxoe.indirect == FALSE)
    {
	mtx_delete(&array->lock);
    }
#endif

    nxa_free(a_nxa, array, sizeof(cw_nxoe_array_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_array_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_array_t *array;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so as long as two
     * interpreters aren't collecting simultaneously, using a static variable
     * works fine. */
    static cw_uint32_t ref_iter;

    array = (cw_nxoe_array_t *) a_nxoe;

    if (a_reset)
    {
	ref_iter = 0;
    }

    if (a_nxoe->indirect)
    {
	if (ref_iter == 0)
	{
	    retval = array->e.i.nxo.o.nxoe;
	    ref_iter++;
	}
	else
	{
	    retval = NULL;
	}
    }
    else
    {
	retval = NULL;
	while (retval == NULL && ref_iter < array->e.a.len)
	{
	    retval = nxo_nxoe_get(&array->e.a.arr[ref_iter]);
	    ref_iter++;
	}
    }

    return retval;
}

CW_INLINE void
nxo_l_array_el_get(const cw_nxo_t *a_nxo, cw_nxoi_t a_offset, cw_nxo_t *r_el)
{
    cw_nxoe_array_t *array;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);
    cw_check_ptr(r_el);

    array = (cw_nxoe_array_t *) a_nxo->o.nxoe;

    cw_check_ptr(array);
    cw_dassert(array->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(array->nxoe.type == NXOT_ARRAY);

#ifdef CW_THREADS
    nxoe_p_array_lock(array);
#endif
    if (array->nxoe.indirect == FALSE)
    {
	cw_assert(a_offset < array->e.a.len && a_offset >= 0);
	nxo_dup(r_el, &array->e.a.arr[a_offset]);
    }
    else
    {
	nxo_array_el_get(&array->e.i.nxo, a_offset + array->e.i.beg_offset,
			 r_el);
    }
#ifdef CW_THREADS
    nxoe_p_array_unlock(array);
#endif
}

CW_INLINE cw_bool_t
nxo_l_array_bound_get(const cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

    return ((a_nxo->flags >> 5) & 1);
}

CW_INLINE void
nxo_l_array_bound_set(cw_nxo_t *a_nxo)
{
    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_ARRAY);

    a_nxo->flags = (a_nxo->flags & 0xffffffdf) | (1 << 5);
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_ARRAY_C_)) */
