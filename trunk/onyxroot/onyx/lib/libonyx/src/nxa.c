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
#ifdef CW_OOP
#include "../include/libonyx/nxo_class_l.h"
#endif
#ifdef CW_THREADS
#include "../include/libonyx/nxo_condition_l.h"
#endif
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_file_l.h"
#ifdef CW_HANDLE
#include "../include/libonyx/nxo_handle_l.h"
#endif
#ifdef CW_OOP
#include "../include/libonyx/nxo_instance_l.h"
#endif
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

/* Prototypes for library-private functions that are only used in this file. */
void
nxo_name_l_init(void);
void
nxo_name_l_shutdown(void);

/* Prototypes. */
static void
nxa_p_collect(cw_bool_t a_shutdown);
#ifdef CW_PTHREADS
static void *
nxa_p_gc_entry(void *a_arg);
#endif

/* Global variables. */
#ifdef CW_DBG
cw_bool_t cw_g_nxa_initialized = FALSE;
#endif

cw_mema_t *cw_g_nxaa = NULL;

/* File-global variables. */
static cw_mema_t s_nxaa;

/* Head for list of nx's in the root set. */
static ql_head(cw_nx_t) s_nx_ql;

#ifdef CW_THREADS
/* Protects the gcdict_* fields and gc_pending. */
static cw_mtx_t s_lock;
#endif

/* Actual state of gcdict. */
static cw_bool_t s_gcdict_active;
#ifdef CW_PTHREADS
static cw_nxoi_t s_gcdict_period;
#endif
static cw_nxoi_t s_gcdict_threshold;
static cw_nxoi_t s_gcdict_collections;
static cw_nxoi_t s_gcdict_count;
static cw_nxoi_t s_gcdict_current[2];
static cw_nxoi_t s_gcdict_maximum[2];
static cw_nxoi_t s_gcdict_sum[2];

/* Sequence set. */
#ifdef CW_THREADS
/* Protects s_seq_set, s_white, and s_nx_ql. */
static cw_mtx_t s_seq_mtx;
#endif
static ql_head(cw_nxoe_t) s_seq_set;
static cw_bool_t s_white; /* Current value for white (alternates). */

/* Garbage. */
static cw_uint32_t s_iter;
static cw_nxoi_t s_target_count;
static cw_nxoe_t *s_garbage;
static cw_nxoe_t *s_deferred_garbage;

/* Message queue used to communicate with the GC thread. */
#ifdef CW_PTHREADS
static cw_mq_t s_gc_mq;
#endif
static cw_bool_t s_gc_pending;
static cw_bool_t s_gc_allocated;

/* GC thread. */
#ifdef CW_PTHREADS
static cw_thd_t *s_gc_thd;
#endif

/* Clean up enough unreferenced objects to drop below s_target_count.  This
 * function is entered/exited with s_lock locked. */
CW_P_INLINE void
nxa_p_sweep(void)
{
    cw_nxoe_t *nxoe, *next;
    cw_bool_t notyet;
    /* Sweep more than one object at a time, primarily in order to reduce the
     * number of times that s_lock is locked/unlocked. */
#define NSWEEP 8
    cw_uint32_t i;

    /* Iterate through the garbage objects and delete them.  If notyet is set to
     * TRUE, the object deletion is deferred until a later pass.  Repeatedly
     * iterate through undeleted objects until no objects defer deletion, or
     * s_gcdict_count has dropped below s_target_count. */
    while (1)
    {
	/* If all garbage is in s_deferred_garbage, start a new sweep iteration.
	 * If no garbage is left, return. */
	if (s_garbage == NULL)
	{
	    if (s_deferred_garbage == NULL)
	    {
		break;
	    }
	    else
	    {
		s_iter++;
		s_garbage = s_deferred_garbage;
		s_deferred_garbage = NULL;
	    }
	}

	/* Get the next NSWEEP objects and split them out of s_garbage. */
	for (i = 1, nxoe = s_garbage, s_garbage = qr_next(nxoe, link);
	     i < NSWEEP && s_garbage != nxoe;
	     i++, s_garbage = qr_next(s_garbage, link))
	{
	    /* Do nothing. */
	}

	if (s_garbage != nxoe)
	{
	    qr_split(s_garbage, nxoe, cw_nxoe_t, link);
	}
	else
	{
	    s_garbage = NULL;
	}

	/* Drop s_lock here to avoid recursive acquisition during object
	 * deletion. */
#ifdef CW_THREADS
	mtx_unlock(&s_lock);
#endif

	next = nxoe;
	do
	{
	    nxoe = next;
	    next = qr_next(next, link);
	    qr_remove(nxoe, link);

	    switch (nxoe_l_type_get(nxoe))
	    {
		case NXOT_ARRAY:
		{
		    notyet = nxoe_l_array_delete(nxoe, s_iter);
		    break;
		}
#ifdef CW_OOP
		case NXOT_CLASS:
		{
		    notyet = nxoe_l_class_delete(nxoe, s_iter);
		    break;
		}
#endif
#ifdef CW_THREADS
		case NXOT_CONDITION:
		{
		    notyet = nxoe_l_condition_delete(nxoe, s_iter);
		    break;
		}
#endif
		case NXOT_DICT:
		{
		    notyet = nxoe_l_dict_delete(nxoe, s_iter);
		    break;
		}
		case NXOT_FILE:
		{
		    notyet = nxoe_l_file_delete(nxoe, s_iter);
		    break;
		}
#ifdef CW_HANDLE
		case NXOT_HANDLE:
		{
		    notyet = nxoe_l_handle_delete(nxoe, s_iter);
		    break;
		}
#endif
#ifdef CW_OOP
		case NXOT_INSTANCE:
		{
		    notyet = nxoe_l_instance_delete(nxoe, s_iter);
		    break;
		}
#endif
#ifdef CW_THREADS
		case NXOT_MUTEX:
		{
		    notyet = nxoe_l_mutex_delete(nxoe, s_iter);
		    break;
		}
#endif
		case NXOT_NAME:
		{
		    notyet = nxoe_l_name_delete(nxoe, s_iter);
		    break;
		}
#ifdef CW_REGEX
		case NXOT_REGEX:
		{
		    notyet = nxoe_l_regex_delete(nxoe, s_iter);
		    break;
		}
		case NXOT_REGSUB:
		{
		    notyet = nxoe_l_regsub_delete(nxoe, s_iter);
		    break;
		}
#endif
		case NXOT_STACK:
		{
		    notyet = nxoe_l_stack_delete(nxoe, s_iter);
		    break;
		}
		case NXOT_STRING:
		{
		    notyet = nxoe_l_string_delete(nxoe, s_iter);
		    break;
		}
		case NXOT_THREAD:
		{
		    notyet = nxoe_l_thread_delete(nxoe, s_iter);
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
#ifdef CW_THREADS
		mtx_lock(&s_lock);
#endif
		/* Defer deletion of this object. */
		if (s_deferred_garbage != NULL)
		{
		    qr_before_insert(s_deferred_garbage, nxoe, link);
		}
		else
		{
		    s_deferred_garbage = nxoe;
		}
#ifdef CW_THREADS
		mtx_unlock(&s_lock);
#endif
	    }
	} while (next != nxoe);

	/* Check if enough garbage has been swept. */
#ifdef CW_THREADS
	mtx_lock(&s_lock);
#endif
	if (s_gcdict_count <= s_target_count)
	{
	    /* Enough garbage swept for now. */
	    break;
	}
    }
}

void
nxa_l_init(void)
{
#ifdef CW_PTHREADS
    sigset_t sig_mask, old_mask;
#endif

    cw_assert(cw_g_nxa_initialized == FALSE);

#ifdef CW_DBG
    cw_g_nxa_initialized = TRUE;
#endif
    /* Set up the mema to be used for allocation. */
    cw_g_nxaa = mema_new(&s_nxaa, (cw_opaque_alloc_t *) nxa_malloc_e,
			 (cw_opaque_calloc_t *) nxa_calloc_e,
			 (cw_opaque_realloc_t *) nxa_realloc_e,
			 (cw_opaque_dealloc_t *) nxa_free_e, NULL);

    ql_new(&s_nx_ql);

#ifdef CW_THREADS
    mtx_new(&s_lock);
#endif

#ifdef CW_THREADS
    mtx_new(&s_seq_mtx);
#endif

    ql_new(&s_seq_set);
    s_white = FALSE;

    s_garbage = NULL;
    s_deferred_garbage = NULL;

#ifdef CW_PTHREADS
    mq_new(&s_gc_mq, cw_g_mema, sizeof(cw_nxam_t));
#endif
    s_gc_pending = FALSE;
    s_gc_allocated = FALSE;

    /* Initialize gcdict state. */
    s_gcdict_active = FALSE;
#ifdef CW_PTHREADS
    s_gcdict_period = CW_LIBONYX_GCDICT_PERIOD;
#endif
    s_gcdict_threshold = CW_LIBONYX_GCDICT_THRESHOLD;
    s_gcdict_collections = 0;
    s_gcdict_count = 0;
    s_gcdict_current[0] = 0;
    s_gcdict_current[1] = 0;
    s_gcdict_maximum[0] = 0;
    s_gcdict_maximum[1] = 0;
    s_gcdict_sum[0] = 0;
    s_gcdict_sum[1] = 0;

    /* Initialize the name machinery before creating the GC thread.  If a GC
     * were to occur before the name machinery were initialized, bad things
     * would happen. */
    nxo_name_l_init();

#ifdef CW_PTHREADS
    /* Block all signals during thread creation, so that the GC thread does
     * not receive any signals.  Doing this here rather than in the GC
     * thread itself avoids a race condition where signals can be delivered
     * to the GC thread. */
    sigfillset(&sig_mask);
    thd_sigmask(SIG_BLOCK, &sig_mask, &old_mask);
    s_gc_thd = thd_new(nxa_p_gc_entry, NULL, FALSE);
    thd_sigmask(SIG_SETMASK, &old_mask, NULL);
#endif
}

void
nxa_l_shutdown(void)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
#ifdef CW_PTHREADS
    mq_put(&s_gc_mq, NXAM_SHUTDOWN);

    thd_join(s_gc_thd);
    mq_delete(&s_gc_mq);
#endif
#ifdef CW_PTH
    mtx_lock(&s_lock);
    nxa_p_collect(TRUE);
    s_target_count = 0;
    nxa_p_sweep();
    mtx_unlock(&s_lock);
#endif
    mtx_delete(&s_seq_mtx);
#else
    nxa_p_collect(TRUE);
    s_target_count = 0;
    nxa_p_sweep();
#endif
    /* nxo_name_l_shutdown() has to be called precisely here, because it uses
     * cw_g_nxaa for memory allocation. */
    nxo_name_l_shutdown();
#ifdef CW_THREADS
    mtx_delete(&s_lock);
#endif
#ifdef CW_DBG
    cw_g_nxa_initialized = FALSE;
#endif

    mema_delete(cw_g_nxaa);
    cw_g_nxaa = NULL;
}

void
nxa_l_nx_insert(cw_nx_t *a_nx)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_seq_mtx);
#endif
    ql_tail_insert(&s_nx_ql, a_nx, link);
#ifdef CW_THREADS
    mtx_unlock(&s_seq_mtx);
#endif
}

void
nxa_l_nx_remove(cw_nx_t *a_nx)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_seq_mtx);
#endif
    ql_remove(&s_nx_ql, a_nx, link);
#ifdef CW_THREADS
    mtx_unlock(&s_seq_mtx);
#endif
}

void *
nxa_malloc_e(void *a_arg, size_t a_size, const char *a_filename,
	     cw_uint32_t a_line_num)
{
    cw_assert(cw_g_nxa_initialized);

    nxa_l_count_adjust((cw_nxoi_t) a_size);

    return mem_malloc_e(NULL, a_size, a_filename, a_line_num);
}

void *
nxa_calloc_e(void *a_arg, size_t a_number, size_t a_size,
	     const char *a_filename, cw_uint32_t a_line_num)
{
    cw_assert(cw_g_nxa_initialized);

    nxa_l_count_adjust((cw_nxoi_t) (a_number * a_size));

    return mem_calloc_e(NULL, a_number, a_size, a_filename, a_line_num);
}

void *
nxa_realloc_e(void *a_arg, void *a_ptr, size_t a_size, size_t a_old_size,
	      const char *a_filename, cw_uint32_t a_line_num)
{
    cw_assert(cw_g_nxa_initialized);

    nxa_l_count_adjust((cw_nxoi_t) a_size - (cw_nxoi_t) a_old_size);

    return mem_realloc_e(NULL, a_ptr, a_size, a_old_size, a_filename,
			 a_line_num);
}

void
nxa_free_e(void *a_arg, void *a_ptr, size_t a_size, const char *a_filename,
	   cw_uint32_t a_line_num)
{
    cw_assert(cw_g_nxa_initialized);

    nxa_l_count_adjust(-(cw_nxoi_t)a_size);

    mem_free_e(NULL, a_ptr, a_size, a_filename, a_line_num);
}

void
nxa_collect(void)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif
    if (s_gc_pending == FALSE)
    {
	s_gc_pending = TRUE;
#ifdef CW_PTHREADS
	mq_put(&s_gc_mq, NXAM_COLLECT);
#else
	if (s_gcdict_active)
	{
	    nxa_p_collect(FALSE);
	}
#endif
    }
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif
}

cw_bool_t
nxa_active_get(void)
{
    cw_bool_t retval;

    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif
    retval = s_gcdict_active;
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif

    return retval;
}

void
nxa_active_set(cw_bool_t a_active)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif
    s_gcdict_active = a_active;
    if (a_active && s_gcdict_threshold > 0 && s_gcdict_threshold
	<= s_gcdict_count - s_gcdict_current[0])
    {
	if (s_gc_pending == FALSE)
	{
	    s_gc_pending = TRUE;
#ifdef CW_PTHREADS
	    mq_put(&s_gc_mq, NXAM_COLLECT);
#else
	    if (s_gcdict_active)
	    {
		nxa_p_collect(FALSE);
	    }
#endif
	}
    }
#ifdef CW_PTHREADS
    else
    {
	if (s_gc_pending == FALSE)
	{
	    mq_put(&s_gc_mq, NXAM_RECONFIGURE);
	}
    }
#endif
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif
}

#ifdef CW_PTHREADS
cw_nxoi_t
nxa_period_get(void)
{
    cw_nxoi_t retval;

    cw_assert(cw_g_nxa_initialized);

    mtx_lock(&s_lock);
    retval = s_gcdict_period;
    mtx_unlock(&s_lock);

    return retval;
}

void
nxa_period_set(cw_nxoi_t a_period)
{
    cw_assert(cw_g_nxa_initialized);
    cw_assert(a_period >= 0);

    mtx_lock(&s_lock);
    s_gcdict_period = a_period;
    mq_put(&s_gc_mq, NXAM_RECONFIGURE);
    mtx_unlock(&s_lock);
}
#endif

cw_nxoi_t
nxa_threshold_get(void)
{
    cw_nxoi_t retval;

    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif
    retval = s_gcdict_threshold;
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif

    return retval;
}

void
nxa_threshold_set(cw_nxoi_t a_threshold)
{
    cw_assert(cw_g_nxa_initialized);
    cw_assert(a_threshold >= 0);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif
    s_gcdict_threshold = a_threshold;
    if (a_threshold > 0
	&& a_threshold <= s_gcdict_count - s_gcdict_current[0]
	&& s_gcdict_active)
    {
	if (s_gc_pending == FALSE)
	{
	    s_gc_pending = TRUE;
#ifdef CW_PTHREADS
	    mq_put(&s_gc_mq, NXAM_COLLECT);
#else
	    if (s_gcdict_active)
	    {
		nxa_p_collect(FALSE);
	    }
#endif
	}
    }
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif
}

void
nxa_stats_get(cw_nxoi_t *r_collections, cw_nxoi_t *r_count,
	      cw_nxoi_t *r_ccount, cw_nxoi_t *r_cmark,
	      cw_nxoi_t *r_mcount, cw_nxoi_t *r_mmark,
	      cw_nxoi_t *r_scount, cw_nxoi_t *r_smark)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif

    /* collections. */
    if (r_collections != NULL)
    {
	*r_collections = s_gcdict_collections;
    }

    /* count. */
    if (r_count != NULL)
    {
	*r_count = s_gcdict_count;
    }

    /* current. */
    if (r_ccount != NULL)
    {
	*r_ccount = s_gcdict_current[0];
    }
    if (r_cmark != NULL)
    {
	*r_cmark = s_gcdict_current[1];
    }

    /* maximum. */
    if (r_mcount != NULL)
    {
	*r_mcount = s_gcdict_maximum[0];
    }
    if (r_mmark != NULL)
    {
	*r_mmark = s_gcdict_maximum[1];
    }

    /* sum. */
    if (r_scount != NULL)
    {
	*r_scount = s_gcdict_sum[0];
    }
    if (r_smark != NULL)
    {
	*r_smark = s_gcdict_sum[1];
    }

#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif
}

void
nxa_l_gc_register(cw_nxoe_t *a_nxoe)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_seq_mtx);
#endif
    cw_assert(nxoe_l_registered_get(a_nxoe) == FALSE);
    cw_assert(qr_next(a_nxoe, link) == a_nxoe);
    cw_assert(qr_prev(a_nxoe, link) == a_nxoe);

    /* Set the color to white, set the registered bit, and insert into the
     * object ring. */
    nxoe_l_color_set(a_nxoe, s_white);
    nxoe_l_registered_set(a_nxoe, TRUE);
    ql_tail_insert(&s_seq_set, a_nxoe, link);

#ifdef CW_THREADS
    mtx_unlock(&s_seq_mtx);
#endif
}

void
nxa_l_count_adjust(cw_nxoi_t a_adjust)
{
    cw_assert(cw_g_nxa_initialized);

#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif

    /* Update count. */
    s_gcdict_count += a_adjust;

    if (a_adjust > 0)
    {
	if (s_garbage != NULL || s_deferred_garbage != NULL)
	{
	    s_target_count -= a_adjust;

	    /* Sweep at least a_adjust bytes of garbage (or all garbage if there
	     * isn't enough) before going any farther.  This assures that
	     * garbage is swept at least as fast as it is generated, which keeps
	     * the mutators from outrunning the garbage collector. */
	    nxa_p_sweep();
	}

	if (s_gcdict_count > s_gcdict_maximum[0])
	{
	    /* Maximum amount of allocated memory seen. */
	    s_gcdict_maximum[0] = s_gcdict_count;
	}

	/* Note that allocation has been done. */
	s_gc_allocated = TRUE;

	/* Adjust the total allocation sum. */
	s_gcdict_sum[0] += a_adjust;

	/* Trigger a collection if the threshold was reached. */
	if (s_gcdict_count - s_gcdict_current[0]
	    >= s_gcdict_threshold && s_gcdict_active
	    && s_gcdict_threshold != 0)
	{
	    if (s_gc_pending == FALSE)
	    {
		s_gc_pending = TRUE;
#ifdef CW_PTHREADS
		mq_put(&s_gc_mq, NXAM_COLLECT);
#else
		if (s_gcdict_active)
		{
		    nxa_p_collect(FALSE);
		}
#endif
	    }
	}
    }
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif
}

CW_P_INLINE void
nxa_p_root_add(cw_nxoe_t *a_nxoe, cw_nxoe_t **r_gray, cw_bool_t *r_roots)
{
    /* If this object is registered and isn't already in the root set, paint it
     * gray and insert it into the root set.  It is very rare for an object to
     * be reported more than once during root set iteration.  The only way this
     * can normally happen is if multiple interpreters happen to be initializing
     * systemdict, and they happen to be initializing the same key/value pair.
     *
     * Explicitly allow a root to be reported more than once, since it is also
     * possible (if perhaps unusual) for the program to define one or more of
     * stdin/stdout/stderr to be the same across interpreters. */
    if (nxoe_l_registered_get(a_nxoe) && nxoe_l_color_get(a_nxoe) == s_white)
    {
	/* Paint object gray. */
	nxoe_l_color_set(a_nxoe, !s_white);
	if (*r_roots)
	{
	    qr_remove(a_nxoe, link);
	    qr_after_insert(*r_gray, a_nxoe, link);
	}
	else
	{
	    ql_first(&s_seq_set) = a_nxoe;
	    *r_roots = TRUE;
	}
	/* Set gray to a_nxoe, since we inserted at the head of the
	 * list. */
	*r_gray = a_nxoe;
    }
}

/* Find roots, if any.  Return TRUE if there are roots, FALSE otherwise.  Upon
 * return, s_seq_set points to the first object in the root set. */
CW_P_INLINE cw_bool_t
nxa_p_roots(cw_bool_t a_shutdown)
{
    cw_bool_t retval = FALSE;
    cw_nx_t *nx;
    cw_nxoe_t *nxoe, *gray;

    /* Iterate through the root set and mark it gray.
     *
     * Each set of *_ref_iter() calls on a particular object must start with a
     * call with (a_reset == TRUE), and repeated calls until NULL is
     * returned. */
    ql_foreach(nx, &s_nx_ql, link)
    {
	for (nxoe = nx_l_ref_iter(nx, TRUE);
	     nxoe != NULL;
	     nxoe = nx_l_ref_iter(nx, FALSE))
	{
	    nxa_p_root_add(nxoe, &gray, &retval);
	}
    }

    if (a_shutdown == FALSE)
    {
	/* Add argv to the root set. */
	nxa_p_root_add(nxo_nxoe_get(libonyx_argv_get()), &gray, &retval);

	/* Add envdict to the root set. */
#ifdef CW_POSIX
	nxa_p_root_add(nxo_nxoe_get(libonyx_envdict_get()), &gray, &retval);
#endif
	/* Add gcdict to the root set. */
	nxa_p_root_add(nxo_nxoe_get(libonyx_gcdict_get()), &gray, &retval);
    }

    return retval;
}

/* Mark.  Return a pointer to a ring of garbage, if any, otherwise NULL. */
CW_P_INLINE cw_nxoe_t *
nxa_p_mark(void)
{
    cw_nxoe_t *retval, *gray, *nxoe;
    cw_bool_t reset;

    /* Iterate through the gray objects and process them until only black and
     * white objects are left. */
    gray = ql_first(&s_seq_set);
    do
    {
	cw_assert(nxoe_l_color_get(gray) != s_white);

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
#ifdef CW_OOP
		case NXOT_CLASS:
		{
		    nxoe = nxoe_l_class_ref_iter(gray, reset);
		    break;
		}
#endif
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
#ifdef CW_HANDLE
		case NXOT_HANDLE:
		{
		    nxoe = nxoe_l_handle_ref_iter(gray, reset);
		    break;
		}
#endif
#ifdef CW_OOP
		case NXOT_INSTANCE:
		{
		    nxoe = nxoe_l_instance_ref_iter(gray, reset);
		    break;
		}
#endif
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
	    if (nxoe_l_color_get(nxoe) == s_white
		&& nxoe_l_registered_get(nxoe))
	    {
		nxoe_l_color_set(nxoe, !s_white);
		/* Move the object to the gray region, if it isn't already
		 * adjacent to (and thereby part of) it. */
		if (nxoe_l_color_get(qr_prev(nxoe, link)) == s_white)
		{
		    qr_remove(nxoe, link);
		    qr_after_insert(gray, nxoe, link);
		}
	    }
	}
	gray = qr_next(gray, link);
    } while (nxoe_l_color_get(gray) != s_white
	     && gray != ql_first(&s_seq_set));

    /* Split the white objects into a separate ring.  If there is garbage,
     * 'gray' points to the first garbage object in the ring. */
    if (gray != ql_first(&s_seq_set))
    {
	/* Split the ring. */
	qr_split(ql_first(&s_seq_set), gray, cw_nxoe_t, link);
	retval = gray;
    }
    else
    {
	retval = NULL;
    }

    return retval;
}

/* Collect garbage using a Baker's Treadmill.  s_lock is held upon entry
 * into this function. */
static void
nxa_p_collect(cw_bool_t a_shutdown)
{
    struct timeval t_tv;
    cw_nxoi_t start_us, mark_us;
#ifdef CW_THREADS
    cw_mtx_t *name_lock;
#endif

    /* Sweep any garbage that remains from the previous collection. */
    if (s_garbage != NULL || s_deferred_garbage != NULL)
    {
	s_target_count = 0;
	nxa_p_sweep();
    }

    /* Reset the pending flag. */
    s_gc_pending = FALSE;

    /* Reset the allocated flag. */
    s_gc_allocated = FALSE;

    /* Release the lock before entering the single section to avoid lock order
     * reversal due to mutators calling nxa_malloc() within critical sections.
     * We don't need the lock anyway, except to protect the GC statistics and
     * the s_gc_pending flag. */
#ifdef CW_THREADS
    mtx_unlock(&s_lock);
#endif

    /* Record the start time. */
    gettimeofday(&t_tv, NULL);
    start_us = t_tv.tv_sec;
    start_us *= 1000000;
    start_us += t_tv.tv_usec;

    /* Acquire name_lock before s_seq_mtx to avoid lock order reversal.
     * nxo_name_new() acquires the locks in this same order. */
#ifdef CW_THREADS
    name_lock = nxo_l_name_lock_get();
    mtx_lock(name_lock);
#endif

    /* Prevent new registrations until after the mark phase is completed. */
#ifdef CW_THREADS
    mtx_lock(&s_seq_mtx);
#endif

#ifdef CW_THREADS
    /* Stop mutator threads. */
    thd_single_enter();
#endif

    /* There shouldn't be any unswept garbage. */
    cw_assert(s_garbage == NULL);
    cw_assert(s_deferred_garbage == NULL);

    /* Mark the root set gray.  If there are any objects in the root set, mark
     * all objects reachable from the root set.  Otherwise, everything is
     * garbage. */
    if (nxa_p_roots(a_shutdown))
    {
	s_garbage = nxa_p_mark();
    }
    else
    {
	s_garbage = ql_first(&s_seq_set);
	ql_first(&s_seq_set) = NULL;
    }

    /* Prepare for incremental sweeping. */
    s_iter = 0;
    s_target_count = s_gcdict_count;

    /* Prune all garbage names before resuming the mutators or releasing
     * cw_g_nxa_name_lock.  Since the mutators may immediately start sweeping
     * objects once they are resumed, unreferenced name objects must already be
     * pruned; otherwise they could be deleted without being processed by the
     * pruning function, which would corrupt the name hash. */
    nxo_l_name_list_prune(s_white);

#ifdef CW_THREADS
    /* Allow mutator threads to run. */
    thd_single_leave();
#endif

    /* Flip the value of white. */
    s_white = !s_white;

#ifdef CW_THREADS
    mtx_unlock(name_lock);
#endif

    /* New registrations are safe again. */
#ifdef CW_THREADS
    mtx_unlock(&s_seq_mtx);
#endif

    /* Record the mark finish time and calculate mark_us. */
    gettimeofday(&t_tv, NULL);
    mark_us = t_tv.tv_sec;
    mark_us *= 1000000;
    mark_us += t_tv.tv_usec;
    mark_us -= start_us;

    /* Protect statistics updates. */
#ifdef CW_THREADS
    mtx_lock(&s_lock);
#endif

    /* Update statistics. */
    /* count.  Since sweeping occurs asynchronously, it is possible that the
     * current count is not an accurate reflection of what the lowest memory
     * usage since the mark phase completed.  We don't need to worry about
     * this too much, since, the race only exists for threaded versions of
     * onyx, and periodic collection will happen. */
    s_gcdict_current[0] = s_gcdict_count;

    /* mark. */
    s_gcdict_current[1] = mark_us;
    if (mark_us > s_gcdict_maximum[1])
    {
	s_gcdict_maximum[1] = mark_us;
    }
    s_gcdict_sum[1] += mark_us;

    /* Increment the collections counter. */
    s_gcdict_collections++;
}

#ifdef CW_PTHREADS
static void *
nxa_p_gc_entry(void *a_arg)
{
    struct timespec period;
    cw_nxam_t message;
    cw_bool_t allocated, sweep, shutdown;

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
	mtx_lock(&s_lock);
	period.tv_sec = s_gcdict_period;
	mtx_unlock(&s_lock);

	if (period.tv_sec > 0)
	{
	    if (mq_timedget(&s_gc_mq, &period, &message))
	    {
		message = NXAM_NONE;
	    }
	}
	else
	{
	    mq_get(&s_gc_mq, &message);
	}

	switch (message)
	{
	    case NXAM_NONE:
	    {
		mtx_lock(&s_lock);
		if (s_gcdict_active)
		{
		    if (s_gc_allocated)
		    {
			/* Record the fact that there has been allocation
			 * activity. */
			allocated = TRUE;
		    }

		    if (s_gc_allocated == FALSE)
		    {
			if (allocated)
			{
			    /* No additional registrations have happened since
			     * the last mq_timedget() timeout and some
			     * allocation has occurred; collect. */
			    nxa_p_collect(FALSE);
			    sweep = FALSE;
			    allocated = FALSE;
			}
			else
			{
			    sweep = TRUE;
			}
		    }
		    else
		    {
			/* Reset the allocated flag so that at the next timeout,
			 * we can tell if there has been any allocation
			 * activity. */
			s_gc_allocated = FALSE;
			sweep = FALSE;
		    }

		    /* If no collection was done, and no new data were
		     * allocated, finish sweeping any remaining garbage. */
		    if (sweep
			&& (s_garbage != NULL || s_deferred_garbage != NULL))
		    {
			s_target_count = 0;
			nxa_p_sweep();
		    }
		}
		mtx_unlock(&s_lock);

		break;
	    }
	    case NXAM_COLLECT:
	    {
		mtx_lock(&s_lock);
		nxa_p_collect(FALSE);
		allocated = FALSE;
		mtx_unlock(&s_lock);
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
		mtx_lock(&s_lock);
		nxa_p_collect(TRUE);
		s_target_count = 0;
		nxa_p_sweep();
		mtx_unlock(&s_lock);
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
