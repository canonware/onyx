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
 * nxa uses a Baker's Treadmill (an elegant form of mark and sweep) to
 * implement garbage collection.  The collector is atomic, which means that all
 * "mutator" threads must be suspended during the marking phase.  Although no
 * other threads can do any work during marking, the collector does not have to
 * (nor is it allowed to) do any locking, which is a significant performance
 * gain.  The total amount of work that the garbage collector has to do is
 * reduced, which improves overall program throughput.
 *
 * The downside of atomic collection is that if marking takes more than about
 * 100 milliseconds, the user may notice a lag.  In practice, the collector
 * appears to perform very well; marking is fast enough to stay well below the
 * threshold of user awareness, unless memory is low enough that system paging
 * becomes a factor.
 *
 * For situations where a significant amount of allocation is done over a short
 * period of time, or where no allocation whatsoever is happening, the collector
 * can be suspended/resumed/forced in order to improve performance or decrease
 * memory footprint.
 *
 * The following diagram depicts a treadmill as it exists during the mark phase:
 *
 *                                 -------------------\
 *                                                     \
 *                               /--\                   \
 *                               |W |                    \
 *                    /--\       \--/       /--\          \
 *                    |G |        ^         |W |           \
 *                    \--/        |         \--/            \
 *                                |                          \
 *                                white                       \
 *           /--\                                     /--\     \
 *           |G |<-- gray                             |W |      \
 *           \--/                                     \--/       \
 *                                                               |
 *                                                               |
 *                                                               |
 *        /--\                                           /--\   \|/
 *        |B |                                           |W |    V
 *        \--/                                           \--/
 *
 *
 *
 *           /--\                                     /--\
 *           |B |<-- black                            |W |
 *           \--/                                     \--/
 *
 *
 *                    /--\                  /--\
 *                    |W |                  |W |
 *                    \--/       /--\       \--/
 *                               |W |
 *                               \--/
 *
 * Black (fully scanned) objects are all the objects between 'black' (inclusive)
 * and 'gray' (exclusive).
 *
 * Gray (reached, not fully scanned) objects are all the objects
 * between 'gray' (inclusive) and 'white' (exclusive).
 *
 * White (unreached/unreachable) objects are all the objects between 'white'
 * (inclusive) and 'black' (exclusive).
 *
 ******************************************************************************/

#define	CW_NXA_C_

#include "../include/libonyx/libonyx.h"

#include <sys/time.h>

#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_array_l.h"
#ifdef CW_THREADS
#include "../include/libonyx/nxo_condition_l.h"
#endif
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_file_l.h"
#include "../include/libonyx/nxo_hook_l.h"
#ifdef CW_THREADS
#include "../include/libonyx/nxo_mutex_l.h"
#endif
#include "../include/libonyx/nxo_name_l.h"
#ifdef CW_REGEX
#include "../include/libonyx/nxo_regex_l.h"
#include "../include/libonyx/nxo_regsub_l.h"
#endif
#include "../include/libonyx/nxo_stack_l.h"
#include "../include/libonyx/nxo_string_l.h"
#include "../include/libonyx/nxo_thread_l.h"

static void
nxa_p_collect(void);
#ifdef CW_PTHREADS
static void *
nxa_p_gc_entry(void *a_arg);
#endif

cw_nxa_t *cw_g_nxa = NULL;
static cw_nxa_t nxa;

cw_mema_t *cw_g_nxaa = NULL;
static cw_mema_t nxaa;

void
nxa_l_init(void)
{
#ifdef CW_PTHREADS
    sigset_t sig_mask, old_mask;
#endif
    volatile cw_uint32_t try_stage = 0;

    cw_g_nxa = &nxa;
    memset(cw_g_nxa, 0, sizeof(cw_nxa_t));

    xep_begin();
    xep_try
    {
	/* Set up the mema to be used for allocation. */
	mema_new(&nxaa, (cw_opaque_alloc_t *) nxa_malloc_e,
		 (cw_opaque_calloc_t *) nxa_calloc_e,
		 (cw_opaque_realloc_t *) nxa_realloc_e,
		 (cw_opaque_dealloc_t *) nxa_free_e, cw_g_nxa);
	cw_g_nxaa = &nxaa;

	ql_new(&cw_g_nxa->nx_ql);

#ifdef CW_THREADS
	mtx_new(&cw_g_nxa->lock);
#endif

#ifdef CW_THREADS
	mtx_new(&cw_g_nxa->seq_mtx);
#endif

	ql_new(&cw_g_nxa->seq_set);

#ifdef CW_PTHREADS
	mq_new(&cw_g_nxa->gc_mq, cw_g_mema, sizeof(cw_nxam_t));
#endif
	try_stage = 1;

#ifdef CW_DBG
	cw_g_nxa->magic = CW_NXA_MAGIC;
#endif

	/* Initialize gcdict state. */
#ifdef CW_PTHREADS
	cw_g_nxa->gcdict_period = CW_LIBONYX_GCDICT_PERIOD;
#endif
	cw_g_nxa->gcdict_threshold = CW_LIBONYX_GCDICT_THRESHOLD;

#ifdef CW_PTHREADS
	/* Block all signals during thread creation, so that the GC thread does
	 * not receive any signals.  Doing this here rather than in the GC
	 * thread itself avoids a race condition where signals can be delivered
	 * to the GC thread. */
	sigfillset(&sig_mask);
	thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
	cw_g_nxa->gc_thd = thd_new(nxa_p_gc_entry, (void *) cw_g_nxa, FALSE);
	thd_sigmask(SIG_SETMASK, &old_mask, NULL);
#endif
	try_stage = 2;

	/* Initialize the global name cache. */
#ifdef CW_THREADS
	mtx_new(&cw_g_nxa->name_lock);
#endif
	dch_new(&cw_g_nxa->name_hash, cw_g_nxaa,
		CW_LIBONYX_NAME_HASH, CW_LIBONYX_NAME_HASH / 4 * 3,
		CW_LIBONYX_NAME_HASH / 4, nxo_l_name_hash,
		nxo_l_name_key_comp);
	try_stage = 1;
    }
    xep_catch(CW_ONYXX_OOM)
    {
	switch (try_stage)
	{
//	    case 3:
//	    {
//		dch_delete(&cw_g_nxa->name_hash);
//#ifdef CW_THREADS
//		mtx_delete(&cw_g_nxa->name_lock);
//#endif
//	    }
	    case 2:
	    {
#ifdef CW_PTHREADS
		mq_put(&cw_g_nxa->gc_mq, NXAM_SHUTDOWN);

		thd_join(cw_g_nxa->gc_thd);
#endif
	    }
	    case 1:
	    {
#ifdef CW_PTHREADS
		mq_delete(&cw_g_nxa->gc_mq);
#endif
#ifdef CW_THREADS
		mtx_delete(&cw_g_nxa->seq_mtx);
		mtx_delete(&cw_g_nxa->lock);
#endif
		memset(cw_g_nxa, 0x5a, sizeof(cw_nxa_t));
		cw_g_nxa = NULL;
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }
    xep_end();
}

void
nxa_l_shutdown(void)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
#ifdef CW_PTHREADS
    mq_put(&cw_g_nxa->gc_mq, NXAM_SHUTDOWN);

    thd_join(cw_g_nxa->gc_thd);
    mq_delete(&cw_g_nxa->gc_mq);
#endif
#ifdef CW_PTH
    mtx_lock(&cw_g_nxa->lock);
    nxa_p_collect(cw_g_nxa);
    mtx_unlock(&cw_g_nxa->lock);
#endif
    mtx_delete(&cw_g_nxa->seq_mtx);
#else
    nxa_p_collect();
#endif
    dch_delete(&cw_g_nxa->name_hash);
#ifdef CW_THREADS
    mtx_delete(&cw_g_nxa->name_lock);
    mtx_delete(&cw_g_nxa->lock);
#endif
    cw_g_nxa = NULL;

    mema_delete(cw_g_nxaa);
    cw_g_nxaa = NULL;
}

void
nxa_l_nx_insert(cw_nx_t *a_nx)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    ql_tail_insert(&cw_g_nxa->nx_ql, a_nx, link);
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

void
nxa_l_nx_remove(cw_nx_t *a_nx)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    ql_remove(&cw_g_nxa->nx_ql, a_nx, link);
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

void *
nxa_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    nxa_l_count_adjust((cw_nxoi_t) a_size);

    return mem_malloc_e(NULL, a_size, a_filename, a_line_num);
}

void *
nxa_calloc_e(void *a_arg, size_t a_number, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    nxa_l_count_adjust((cw_nxoi_t) (a_number * a_size));

    return mem_calloc_e(NULL, a_number, a_size, a_filename, a_line_num);
}

void *
nxa_realloc_e(void *a_arg, void *a_ptr, size_t a_size, size_t a_old_size,
	      const char *a_filename, cw_uint32_t a_line_num)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    nxa_l_count_adjust((cw_nxoi_t) a_size - (cw_nxoi_t) a_old_size);

    return mem_realloc_e(NULL, a_ptr, a_size, a_old_size, a_filename,
			 a_line_num);
}

void
nxa_free_e(void *a_arg, void *a_ptr, size_t a_size, const char *a_filename,
	   cw_uint32_t a_line_num)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    nxa_l_count_adjust(-(cw_nxoi_t)a_size);

    mem_free_e(NULL, a_ptr, a_size, a_filename, a_line_num);
}

void
nxa_collect(void)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    if (cw_g_nxa->gc_pending == FALSE)
    {
	cw_g_nxa->gc_pending = TRUE;
#ifdef CW_PTHREADS
	mq_put(&cw_g_nxa->gc_mq, NXAM_COLLECT);
#else
	if (cw_g_nxa->gcdict_active)
	{
	    nxa_p_collect();
	}
#endif
    }
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

cw_bool_t
nxa_active_get(void)
{
    cw_bool_t retval;

    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    retval = cw_g_nxa->gcdict_active;
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif

    return retval;
}

void
nxa_active_set(cw_bool_t a_active)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    cw_g_nxa->gcdict_active = a_active;
    if (a_active && cw_g_nxa->gcdict_threshold > 0 && cw_g_nxa->gcdict_threshold
	<= cw_g_nxa->gcdict_count - cw_g_nxa->gcdict_current[0])
    {
	if (cw_g_nxa->gc_pending == FALSE)
	{
	    cw_g_nxa->gc_pending = TRUE;
#ifdef CW_PTHREADS
	    mq_put(&cw_g_nxa->gc_mq, NXAM_COLLECT);
#else
	    if (cw_g_nxa->gcdict_active)
	    {
		nxa_p_collect();
	    }
#endif
	}
    }
#ifdef CW_PTHREADS
    else
    {
	if (cw_g_nxa->gc_pending == FALSE)
	{
	    mq_put(&cw_g_nxa->gc_mq, NXAM_RECONFIGURE);
	}
    }
#endif
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

#ifdef CW_PTHREADS
cw_nxoi_t
nxa_period_get(void)
{
    cw_nxoi_t retval;

    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    mtx_lock(&cw_g_nxa->lock);
    retval = cw_g_nxa->gcdict_period;
    mtx_unlock(&cw_g_nxa->lock);

    return retval;
}

void
nxa_period_set(cw_nxoi_t a_period)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);
    cw_assert(a_period >= 0);

    mtx_lock(&cw_g_nxa->lock);
    cw_g_nxa->gcdict_period = a_period;
    mq_put(&cw_g_nxa->gc_mq, NXAM_RECONFIGURE);
    mtx_unlock(&cw_g_nxa->lock);
}
#endif

cw_nxoi_t
nxa_threshold_get(void)
{
    cw_nxoi_t retval;

    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    retval = cw_g_nxa->gcdict_threshold;
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif

    return retval;
}

void
nxa_threshold_set(cw_nxoi_t a_threshold)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);
    cw_assert(a_threshold >= 0);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif
    cw_g_nxa->gcdict_threshold = a_threshold;
    if (a_threshold > 0
	&& a_threshold <= cw_g_nxa->gcdict_count - cw_g_nxa->gcdict_current[0]
	&& cw_g_nxa->gcdict_active)
    {
	if (cw_g_nxa->gc_pending == FALSE)
	{
	    cw_g_nxa->gc_pending = TRUE;
#ifdef CW_PTHREADS
	    mq_put(&cw_g_nxa->gc_mq, NXAM_COLLECT);
#else
	    if (cw_g_nxa->gcdict_active)
	    {
		nxa_p_collect();
	    }
#endif
	}
    }
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

void
nxa_stats_get(cw_nxoi_t *r_collections, cw_nxoi_t *r_count,
	      cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark, cw_nxoi_t *r_csweep,
	      cw_nxoi_t *r_mcount, cw_nxoi_t *r_mmark, cw_nxoi_t *r_msweep,
	      cw_nxoi_t *r_scount, cw_nxoi_t *r_smark, cw_nxoi_t *r_ssweep)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);
	
#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif

    /* collections. */
    if (r_collections != NULL)
    {
	*r_collections = cw_g_nxa->gcdict_collections;
    }

    /* count. */
    if (r_count != NULL)
    {
	*r_count = cw_g_nxa->gcdict_count;
    }

    /* current. */
    if (r_ccount != NULL)
    {
	*r_ccount = cw_g_nxa->gcdict_current[0];
    }
    if (r_cmark != NULL)
    {
	*r_cmark = cw_g_nxa->gcdict_current[1];
    }
    if (r_csweep != NULL)
    {
	*r_csweep = cw_g_nxa->gcdict_current[2];
    }

    /* maximum. */
    if (r_mcount != NULL)
    {
	*r_mcount = cw_g_nxa->gcdict_maximum[0];
    }
    if (r_mmark != NULL)
    {
	*r_mmark = cw_g_nxa->gcdict_maximum[1];
    }
    if (r_msweep != NULL)
    {
	*r_msweep = cw_g_nxa->gcdict_maximum[2];
    }

    /* sum. */
    if (r_scount != NULL)
    {
	*r_scount = cw_g_nxa->gcdict_sum[0];
    }
    if (r_smark != NULL)
    {
	*r_smark = cw_g_nxa->gcdict_sum[1];
    }
    if (r_ssweep != NULL)
    {
	*r_ssweep = cw_g_nxa->gcdict_sum[2];
    }

#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

void
nxa_l_gc_register(cw_nxoe_t *a_nxoe)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->seq_mtx);
#endif
    cw_assert(nxoe_l_registered_get(a_nxoe) == FALSE);
    cw_assert(qr_next(a_nxoe, link) == a_nxoe);
    cw_assert(qr_prev(a_nxoe, link) == a_nxoe);

    /* Set the color to white, set the registered bit, and insert into the
     * object ring. */
    nxoe_l_color_set(a_nxoe, cw_g_nxa->white);
    nxoe_l_registered_set(a_nxoe, TRUE);
    ql_tail_insert(&cw_g_nxa->seq_set, a_nxoe, link);

#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->seq_mtx);
#endif
}

void
nxa_l_gc_reregister(cw_nxoe_t *a_nxoe)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->seq_mtx);
#endif
    cw_assert(nxoe_l_registered_get(a_nxoe));

    /* This is only called in nxo_name_new() in the case that a name object is
     * already registered.  By setting the color to white, the collector thread
     * will notice that this object has been re-registered in
     * nxoe_l_name_delete(), and will finish the normal registration process.
     * If the object is not being swept, its color is already white, and this
     * function has no effect. */
    nxoe_l_color_set(a_nxoe, cw_g_nxa->white);

#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->seq_mtx);
#endif
}

void
nxa_l_count_adjust(cw_nxoi_t a_adjust)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif

    /* Update count. */
    cw_g_nxa->gcdict_count += a_adjust;

    if (a_adjust > 0)
    {
	if (cw_g_nxa->gcdict_count > cw_g_nxa->gcdict_maximum[0])
	{
	    /* Maximum amount of allocated memory seen. */
	    cw_g_nxa->gcdict_maximum[0] = cw_g_nxa->gcdict_count;
	}

	/* Note that allocation has been done. */
	cw_g_nxa->gc_allocated = TRUE;

	/* Adjust the total allocation sum. */
	cw_g_nxa->gcdict_sum[0] += a_adjust;

	/* Trigger a collection if the threshold was reached. */
	if (cw_g_nxa->gcdict_count - cw_g_nxa->gcdict_current[0]
	    >= cw_g_nxa->gcdict_threshold && cw_g_nxa->gcdict_active
	    && cw_g_nxa->gcdict_threshold != 0)
	{
	    if (cw_g_nxa->gc_pending == FALSE)
	    {
		cw_g_nxa->gc_pending = TRUE;
#ifdef CW_PTHREADS
		mq_put(&cw_g_nxa->gc_mq, NXAM_COLLECT);
#else
		if (cw_g_nxa->gcdict_active)
		{
		    nxa_p_collect();
		}
#endif
	    }
	}
    }
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif
}

cw_bool_t
nxa_l_white_get(void)
{
    cw_check_ptr(cw_g_nxa);
    cw_dassert(cw_g_nxa->magic == CW_NXA_MAGIC);

    /* This function is only called in nxoe_l_name_delete(), which is executed
     * in the context of the GC thread, so no locking is necessary. */
    return cw_g_nxa->white;
}

/* Find roots, if any.  Return TRUE if there are roots, FALSE otherwise.  Upon
 * return, cw_g_nxa->seq_set points to the first object in the root set. */
CW_P_INLINE cw_bool_t
nxa_p_roots(cw_uint32_t *r_nroot)
{
    cw_bool_t retval = FALSE;
    cw_nx_t *nx;
    cw_nxoe_t *nxoe, *gray;
    cw_uint32_t nroot = 0;

    /* Iterate through the root set and mark it gray.
     *
     * Each set of *_ref_iter() calls on a particular object must start with a
     * call with (a_reset == TRUE), and repeated calls until NULL is
     * returned. */
    ql_foreach(nx, &cw_g_nxa->nx_ql, link)
    {
	for (nxoe = nx_l_ref_iter(nx, TRUE);
	     nxoe != NULL;
	     nxoe = nx_l_ref_iter(nx, FALSE))
	{
	    if (nxoe_l_registered_get(nxoe))
	    {
		/* Paint object gray. */
		nroot++;
		cw_assert(nxoe_l_color_get(nxoe) == cw_g_nxa->white);
		nxoe_l_color_set(nxoe, !cw_g_nxa->white);
		if (retval)
		{
		    qr_remove(nxoe, link);
		    qr_after_insert(gray, nxoe, link);
		}
		else
		{
		    ql_first(&cw_g_nxa->seq_set) = nxoe;
		    retval = TRUE;
		}
		/* Set gray to nxoe, since we inserted at the head of the
		 * list. */
		gray = nxoe;
	    }
	}
    }

    *r_nroot = nroot;
    return retval;
}

/* Mark.  Return a pointer to a ring of garbage, if any, otherwise NULL. */
CW_P_INLINE cw_nxoe_t *
nxa_p_mark(cw_uint32_t *r_nreachable)
{
    cw_nxoe_t *retval, *gray, *nxoe;
    cw_uint32_t nreachable = 0;
    cw_bool_t reset;

    /* Iterate through the gray objects and process them until only black and
     * white objects are left. */
    gray = ql_first(&cw_g_nxa->seq_set);
    do
    {
	cw_assert(nxoe_l_color_get(gray) != cw_g_nxa->white);

	reset = TRUE;
	for (reset = TRUE;; reset = FALSE)
	{
	    switch (nxoe_l_type_get(gray))
	    {
		case NXOT_ARRAY:
		{
		    nxoe = nxoe_l_array_ref_iter(gray, reset);
		    break;
		}
#ifdef CW_THREADS
		case NXOT_CONDITION:
		{
		    nxoe = nxoe_l_condition_ref_iter(gray, reset);
		    break;
		}
#endif
		case NXOT_DICT:
		{
		    nxoe = nxoe_l_dict_ref_iter(gray, reset);
		    break;
		}
		case NXOT_FILE:
		{
		    nxoe = nxoe_l_file_ref_iter(gray, reset);
		    break;
		}
		case NXOT_HOOK:
		{
		    nxoe = nxoe_l_hook_ref_iter(gray, reset);
		    break;
		}
#ifdef CW_THREADS
		case NXOT_MUTEX:
		{
		    nxoe = nxoe_l_mutex_ref_iter(gray, reset);
		    break;
		}
#endif
		case NXOT_NAME:
		{
		    nxoe = nxoe_l_name_ref_iter(gray, reset);
		    break;
		}
#ifdef CW_REGEX
		case NXOT_REGEX:
		{
		    nxoe = nxoe_l_regex_ref_iter(gray, reset);
		    break;
		}
		case NXOT_REGSUB:
		{
		    nxoe = nxoe_l_regsub_ref_iter(gray, reset);
		    break;
		}
#endif
		case NXOT_STACK:
		{
		    nxoe = nxoe_l_stack_ref_iter(gray, reset);
		    break;
		}
		case NXOT_STRING:
		{
		    nxoe = nxoe_l_string_ref_iter(gray, reset);
		    break;
		}
		case NXOT_THREAD:
		{
		    nxoe = nxoe_l_thread_ref_iter(gray, reset);
		    break;
		}
		default:
		{
		    cw_not_reached();
		    break;
		}
	    }

	    if (nxoe == NULL)
	    {
		break;
	    }

	    /* If object is white and registered, color it. */
	    if (nxoe_l_color_get(nxoe) == cw_g_nxa->white
		&& nxoe_l_registered_get(nxoe))
	    {
		nxoe_l_color_set(nxoe, !cw_g_nxa->white);
		nreachable++;
		/* Move the object to the gray region, if it isn't already
		 * adjacent to (and thereby part of) it. */
		if (nxoe_l_color_get(qr_prev(nxoe, link)) == cw_g_nxa->white)
		{
		    qr_remove(nxoe, link);
		    qr_after_insert(gray, nxoe, link);
		}
	    }
	}
	gray = qr_next(gray, link);
    } while (nxoe_l_color_get(gray) != cw_g_nxa->white
	     && gray != ql_first(&cw_g_nxa->seq_set));

    /* Split the white objects into a separate ring.  If there is garbage,
     * 'gray' points to the first garbage object in the ring. */
    if (gray != ql_first(&cw_g_nxa->seq_set))
    {
	/* Split the ring. */
	qr_split(ql_first(&cw_g_nxa->seq_set), gray, cw_nxoe_t, link);
	retval = gray;
    }
    else
    {
	retval = NULL;
    }

    *r_nreachable = nreachable;
    return retval;
}

/* Clean up unreferenced objects. */
CW_P_INLINE void
nxa_p_sweep(cw_nxoe_t *a_garbage)
{
    cw_nxoe_t *last, *defer, *nxoe;
    cw_uint32_t i;
    cw_bool_t again, notyet;

    /* Iterate through the garbage objects and delete them.  If nxoe_l_delete()
     * returns TRUE, the object deletion is deferred until a later pass.
     * Repeatedly iterate through undeleted objects until no objects defer
     * deletion. */
    for (defer = a_garbage, nxoe = NULL, again = TRUE, i = 0;
	 again == TRUE;
	 i++)
    {
	again = FALSE;
	last = defer;
	do
	{
	    nxoe = qr_next(defer, link);
	    qr_remove(nxoe, link);
	    switch (nxoe_l_type_get(nxoe))
	    {
		case NXOT_ARRAY:
		{
		    notyet = nxoe_l_array_delete(nxoe, i);
		    break;
		}
#ifdef CW_THREADS
		case NXOT_CONDITION:
		{
		    notyet = nxoe_l_condition_delete(nxoe, i);
		    break;
		}
#endif
		case NXOT_DICT:
		{
		    notyet = nxoe_l_dict_delete(nxoe, i);
		    break;
		}
		case NXOT_FILE:
		{
		    notyet = nxoe_l_file_delete(nxoe, i);
		    break;
		}
		case NXOT_HOOK:
		{
		    notyet = nxoe_l_hook_delete(nxoe, i);
		    break;
		}
#ifdef CW_THREADS
		case NXOT_MUTEX:
		{
		    notyet = nxoe_l_mutex_delete(nxoe, i);
		    break;
		}
#endif
		case NXOT_NAME:
		{
		    notyet = nxoe_l_name_delete(nxoe, i);
		    break;
		}
#ifdef CW_REGEX
		case NXOT_REGEX:
		{
		    notyet = nxoe_l_regex_delete(nxoe, i);
		    break;
		}
		case NXOT_REGSUB:
		{
		    notyet = nxoe_l_regsub_delete(nxoe, i);
		    break;
		}
#endif
		case NXOT_STACK:
		{
		    notyet = nxoe_l_stack_delete(nxoe, i);
		    break;
		}
		case NXOT_STRING:
		{
		    notyet = nxoe_l_string_delete(nxoe, i);
		    break;
		}
		case NXOT_THREAD:
		{
		    notyet = nxoe_l_thread_delete(nxoe, i);
		    break;
		}
		default:
		{
		    cw_not_reached();
		    break;
		}
	    }

	    if (notyet)
	    {
		again = TRUE;
		qr_after_insert(defer, nxoe, link);
		defer = nxoe;
	    }
	} while (nxoe != last);
    }
}

/* Collect garbage using a Baker's Treadmill.  cw_g_nxa->lock is held upon entry
 * into this function. */
static void
nxa_p_collect(void)
{
    cw_uint32_t nroot, nreachable;
    cw_nxoe_t *garbage;
    struct timeval t_tv;
    cw_nxoi_t start_us, mark_us, sweep_us;

    /* Reset the pending flag. */
    cw_g_nxa->gc_pending = FALSE;

    /* Reset the allocated flag. */
    cw_g_nxa->gc_allocated = FALSE;

    /* Release the lock before entering the single section to avoid lock order
     * reversal due to mutators calling nxa_malloc() within critical sections.
     * We don't need the lock anyway, except to protect the GC statistics and
     * the gc_pending flag. */
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->lock);
#endif

    /* Record the start time. */
    gettimeofday(&t_tv, NULL);
    start_us = t_tv.tv_sec;
    start_us *= 1000000;
    start_us += t_tv.tv_usec;

    /* Prevent new registrations until after the mark phase is completed. */
#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->seq_mtx);
#endif

#ifdef CW_THREADS
    /* Stop mutator threads. */
    thd_single_enter();
#endif

    /* Mark the root set gray.  If there are any objects in the root set, mark
     * all objects reachable from the root set.  Otherwise, everything is
     * garbage. */
    if (nxa_p_roots(&nroot))
    {
	garbage = nxa_p_mark(&nreachable);
    }
    else
    {
	garbage = ql_first(&cw_g_nxa->seq_set);
	ql_first(&cw_g_nxa->seq_set) = NULL;
    }

#ifdef CW_THREADS
    /* Allow mutator threads to run. */
    thd_single_leave();
#endif

    /* Flip the value of white. */
    cw_g_nxa->white = !cw_g_nxa->white;

    /* New registrations are safe again. */
#ifdef CW_THREADS
    mtx_unlock(&cw_g_nxa->seq_mtx);
#endif

    /* Record the mark finish time and calculate mark_us. */
    gettimeofday(&t_tv, NULL);
    mark_us = t_tv.tv_sec;
    mark_us *= 1000000;
    mark_us += t_tv.tv_usec;
    mark_us -= start_us;

    /* If there is garbage, discard it. */
    if (garbage != NULL)
    {
	nxa_p_sweep(garbage);
    }

    /* Record the sweep finish time and calculate sweep_us. */
    gettimeofday(&t_tv, NULL);
    sweep_us = t_tv.tv_sec;
    sweep_us *= 1000000;
    sweep_us += t_tv.tv_usec;
    sweep_us -= start_us;
    sweep_us -= mark_us;

    /* Protect statistics updates. */
#ifdef CW_THREADS
    mtx_lock(&cw_g_nxa->lock);
#endif

    /* Update statistics. */
    /* count.  Since sweeping occurs asynchronously, it is possible that the
     * current count is not an accurate reflection of what the lowest memory
     * usage since the mark phase completed.  We don't need to worry about
     * this too much, since, the race only exists for threaded versions of
     * onyx, and periodic collection will happen. */
    cw_g_nxa->gcdict_current[0] = cw_g_nxa->gcdict_count;

    /* mark. */
    cw_g_nxa->gcdict_current[1] = mark_us;
    if (mark_us > cw_g_nxa->gcdict_maximum[1])
    {
	cw_g_nxa->gcdict_maximum[1] = mark_us;
    }
    cw_g_nxa->gcdict_sum[1] += mark_us;

    /* sweep. */
    cw_g_nxa->gcdict_current[2] = sweep_us;
    if (sweep_us > cw_g_nxa->gcdict_maximum[2])
    {
	cw_g_nxa->gcdict_maximum[2] = sweep_us;
    }
    cw_g_nxa->gcdict_sum[2] += sweep_us;

    /* Increment the collections counter. */
    cw_g_nxa->gcdict_collections++;
}

#ifdef CW_PTHREADS
static void *
nxa_p_gc_entry(void *a_arg)
{
    cw_nxa_t *nxa = (cw_nxa_t *) a_arg;
    struct timespec period;
    cw_nxam_t message;
    cw_bool_t allocated, shutdown;

    /* Any of the following conditions will cause a collection:
     *
     * 1) Enough allocation was done to trigger immediate collection.
     *
     * 2) Some registrations were done, followed by a period of no
     *    registrations for more than gcdict_period seconds.
     *
     * 3) Collection was explicitly requested.
     */
    period.tv_nsec = 0;
    for (allocated = shutdown = FALSE; shutdown == FALSE;)
    {
	mtx_lock(&nxa->lock);
	period.tv_sec = nxa->gcdict_period;
	mtx_unlock(&nxa->lock);

	if (period.tv_sec > 0)
	{
	    if (mq_timedget(&nxa->gc_mq, &period, &message))
	    {
		message = NXAM_NONE;
	    }
	}
	else
	{
	    mq_get(&nxa->gc_mq, &message);
	}

	switch (message)
	{
	    case NXAM_NONE:
	    {
		mtx_lock(&nxa->lock);
		if (nxa->gcdict_active)
		{
		    if (nxa->gc_allocated)
		    {
			/* Record the fact that there has been allocation
			 * activity. */
			allocated = TRUE;
		    }

		    if (nxa->gc_allocated == FALSE)
		    {
			if (allocated)
			{
			    /* No additional registrations have happened since
			     * the last mq_timedget() timeout and some
			     * allocation has occurred; collect. */
			    nxa_p_collect();
			    allocated = FALSE;
			}
		    }
		    else
		    {
			/* Reset the allocated flag so that at the next timeout,
			 * we can tell if there has been any allocation
			 * activity. */
			nxa->gc_allocated = FALSE;
		    }
		}
		mtx_unlock(&nxa->lock);

		break;
	    }
	    case NXAM_COLLECT:
	    {
		mtx_lock(&nxa->lock);
		nxa_p_collect();
		allocated = FALSE;
		mtx_unlock(&nxa->lock);
		break;
	    }
	    case NXAM_RECONFIGURE:
	    {
		/* Don't do anything here. */
		break;
	    }
	    case NXAM_SHUTDOWN:
	    {
		shutdown = TRUE;
		mtx_lock(&nxa->lock);
		nxa_p_collect();
		mtx_unlock(&nxa->lock);
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}
    }

    return NULL;
}
#endif
