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

#ifdef CW_THREADS
/* Enumeration of message types for the GC thread event loop. */
typedef enum
{
    NXAM_NONE,
    NXAM_COLLECT,
    NXAM_RECONFIGURE,
    NXAM_SHUTDOWN
} cw_nxam_t;
#endif

#ifndef CW_THREADS
#endif

void
nxa_l_new(cw_nxa_t *a_nxa, cw_nx_t *a_nx);

void
nxa_l_shutdown(cw_nxa_t *a_nxa);

void
nxa_l_delete(cw_nxa_t *a_nxa);

void
nxa_l_gc_register(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);

void
nxa_l_gc_reregister(cw_nxa_t *a_nxa, cw_nxoe_t *a_nxoe);

cw_bool_t
nxa_l_white_get(cw_nxa_t *a_nxa);

#ifndef CW_USE_INLINES
void
nxa_l_count_adjust(cw_nxa_t *a_nxa, cw_nxoi_t a_adjust);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXA_C_))
CW_INLINE void
nxa_l_count_adjust(cw_nxa_t *a_nxa, cw_nxoi_t a_adjust)
{
    cw_check_ptr(a_nxa);
    cw_dassert(a_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&a_nxa->lock);
#endif

    /* Update count. */
    a_nxa->gcdict_count += a_adjust;

    if (a_adjust > 0)
    {
	if (a_nxa->gcdict_count > a_nxa->gcdict_maximum[0])
	{
	    /* Maximum amount of allocated memory seen. */
	    a_nxa->gcdict_maximum[0] = a_nxa->gcdict_count;
	}

	/* Note that allocation has been done. */
	a_nxa->gc_allocated = TRUE;

	/* Adjust the total allocation sum. */
	a_nxa->gcdict_sum[0] += a_adjust;

	/* Trigger a collection if the threshold was reached. */
	if (a_nxa->gcdict_count - a_nxa->gcdict_current[0]
	    >= a_nxa->gcdict_threshold && a_nxa->gcdict_active
	    && a_nxa->gcdict_threshold != 0)
	{
	    if (a_nxa->gc_pending == FALSE)
	    {
		a_nxa->gc_pending = TRUE;
#ifdef CW_PTHREADS
		mq_put(&a_nxa->gc_mq, NXAM_COLLECT);
#else
		if (a_nxa->gcdict_active)
		{
		    nxa_p_collect(a_nxa);
		}
#endif
	    }
	}
    }
#ifdef CW_THREADS
    mtx_unlock(&a_nxa->lock);
#endif
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXA_C_)) */
