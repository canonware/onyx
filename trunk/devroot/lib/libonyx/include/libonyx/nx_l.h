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

#ifndef CW_USE_INLINES
cw_nxoe_t *
nx_l_ref_iter(cw_nx_t *a_nx, cw_bool_t a_reset);

void
nx_l_thread_insert(cw_nx_t *a_nx, cw_nxo_t *a_thread);

void
nx_l_thread_remove(cw_nx_t *a_nx, cw_nxo_t *a_thread);

#ifdef CW_THREADS
cw_mtx_t *
nx_l_name_lock_get(cw_nx_t *a_nx);
#endif

cw_dch_t *
nx_l_name_hash_get(cw_nx_t *a_nx);

cw_op_t *
nx_l_thread_init(cw_nx_t *a_nx);

cw_thread_start_t *
nx_l_thread_start(cw_nx_t *a_nx);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NX_C_))
CW_INLINE cw_nxoe_t *
nx_l_ref_iter(cw_nx_t *a_nx, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static cw_uint32_t ref_iter;

    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    if (a_reset)
    {
	ref_iter = 0;
    }

    for (retval = NULL; retval == NULL; ref_iter++)
    {
	switch (ref_iter)
	{
	    case 0:
	    {
		retval = nxo_nxoe_get(&a_nx->threadsdict);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&a_nx->systemdict);
		break;
	    }
	    case 2:
	    {
		retval = nxo_nxoe_get(&a_nx->globaldict);
		break;
	    }
	    case 3:
	    {
		retval = nxo_nxoe_get(&a_nx->gcdict);
		break;
	    }
	    case 4:
	    {
		retval = nxo_nxoe_get(&a_nx->stdin_nxo);
		break;
	    }
	    case 5:
	    {
		retval = nxo_nxoe_get(&a_nx->stdout_nxo);
		break;
	    }
	    case 6:
	    {
		retval = nxo_nxoe_get(&a_nx->stderr_nxo);
		break;
	    }
	    default:
	    {
		retval = NULL;
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}

CW_INLINE void
nx_l_thread_insert(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
    cw_nxo_t nxo;

    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    cw_check_ptr(a_thread);
    cw_assert(nxo_type_get(a_thread) == NXOT_THREAD);

    nxo_null_new(&nxo);
    nxo_dict_def(&a_nx->threadsdict, a_thread, &nxo);
}

CW_INLINE void
nx_l_thread_remove(cw_nx_t *a_nx, cw_nxo_t *a_thread)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    cw_check_ptr(a_thread);
    cw_assert(nxo_type_get(a_thread) == NXOT_THREAD);

    nxo_dict_undef(&a_nx->threadsdict, a_thread);
}

CW_INLINE cw_op_t *
nx_l_thread_init(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return a_nx->thread_init;
}

CW_INLINE cw_thread_start_t *
nx_l_thread_start(cw_nx_t *a_nx)
{
    cw_check_ptr(a_nx);
    cw_dassert(a_nx->magic == CW_NX_MAGIC);

    return a_nx->thread_start;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NX_C_)) */
