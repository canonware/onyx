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

cw_uint32_t
nxo_l_thread_token(cw_nxo_t *a_nxo, cw_nxo_threadp_t *a_threadp,
		   const cw_uint8_t *a_str, cw_uint32_t a_len);

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_THREAD_C_))
CW_INLINE cw_bool_t
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
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
	nxa_free(a_nxa, thread->tok_str, thread->buffer_len);
    }

    nxa_free(a_nxa, thread, sizeof(cw_nxoe_thread_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_thread_t *thread;

    thread = (cw_nxoe_thread_t *) a_nxoe;

    if (a_reset)
    {
	thread->ref_iter = 0;
    }

    for (retval = NULL; retval == NULL; thread->ref_iter++)
    {
	switch (thread->ref_iter)
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
	    case 4:
	    {
		retval = nxo_nxoe_get(&thread->tstack);
		break;
	    }
	    case 5:
	    {
		retval = nxo_nxoe_get(&thread->stdin_nxo);
		break;
	    }
	    case 6:
	    {
		retval = nxo_nxoe_get(&thread->stdout_nxo);
		break;
	    }
	    case 7:
	    {
		retval = nxo_nxoe_get(&thread->stderr_nxo);
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
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_THREAD_C_)) */
