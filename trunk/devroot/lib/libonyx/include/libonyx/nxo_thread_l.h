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

uint32_t
nxo_l_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp,
		   const uint8_t *a_str, uint32_t a_len);

#ifndef CW_USE_INLINES
cw_nxo_t *
nxo_l_thread_trapped_arg_get(cw_nxo_t *a_nxo);

#ifdef CW_REGEX
cw_nxo_regex_cache_t *
nxo_l_thread_regex_cache_get(cw_nxo_t *a_nxo);
#endif

bool
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter);

cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_THREAD_C_))
CW_INLINE cw_nxo_t *
nxo_l_thread_trapped_arg_get(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return &thread->trapped_arg;
}

#ifdef CW_REGEX
CW_INLINE cw_nxo_regex_cache_t *
nxo_l_thread_regex_cache_get(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return &thread->regex_cache;
}
#endif

CW_INLINE bool
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, uint32_t a_iter)
{
    cw_nxoe_thread_t *thread;

    thread = (cw_nxoe_thread_t *) a_nxoe;

    cw_check_ptr(thread);
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    if (thread->tok_str != thread->buffer)
    {
	/* This shouldn't happen, since it indicates that there is an unaccepted
	 * token.  However, it's really the caller's fault, so just clean up. */
	nxa_free(thread->tok_str, thread->buffer_len);
    }

#ifdef CW_REGEX
    nxo_l_regex_cache_delete(&thread->regex_cache);
#endif

    nxa_free(thread, sizeof(cw_nxoe_thread_t));

    return false;
}

CW_INLINE cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, bool a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_thread_t *thread;
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so using a static
     * variable works fine. */
    static uint32_t ref_iter;

    thread = (cw_nxoe_thread_t *) a_nxoe;

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
		retval = nxo_nxoe_get(&thread->estack);
		break;
	    }
	    case 1:
	    {
		retval = nxo_nxoe_get(&thread->istack);
		break;
	    }
	    case 2:
	    {
		retval = nxo_nxoe_get(&thread->ostack);
		break;
	    }
	    case 3:
	    {
		retval = nxo_nxoe_get(&thread->dstack);
		break;
	    }
#ifdef CW_OOP
#define CW_REF_ITER_CSTACK 1
	    case 4:
	    {
		retval = nxo_nxoe_get(&thread->cstack);
		break;
	    }
#else
#define CW_REF_ITER_CSTACK 0
#endif
	    case (4 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_nxoe_get(&thread->tstack);
		break;
	    }
	    case (5 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_nxoe_get(&thread->stdin_nxo);
		break;
	    }
	    case (6 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_nxoe_get(&thread->stdout_nxo);
		break;
	    }
	    case (7 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_nxoe_get(&thread->stderr_nxo);
		break;
	    }
	    case (8 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_nxoe_get(&thread->trapped_arg);
		break;
	    }
#ifdef CW_REGEX
	    case (9 + CW_REF_ITER_CSTACK):
	    {
		retval = nxo_l_regex_cache_ref_iter(&thread->regex_cache, true);
		if (retval == NULL)
		{
		    /* Avoid looping and hitting the default case. */
		    goto RETURN;
		}
		break;
	    }
#endif
#undef CW_REF_ITER_CSTACK
	    default:
	    {
#ifdef CW_REGEX
		retval = nxo_l_regex_cache_ref_iter(&thread->regex_cache,
						    false);
#else
		retval = NULL;
#endif
		goto RETURN;
	    }
	}
    }

    RETURN:
    return retval;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_THREAD_C_)) */
