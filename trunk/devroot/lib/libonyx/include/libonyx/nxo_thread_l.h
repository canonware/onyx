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
#ifdef CW_REGEX
cw_nxo_t *
nxo_l_thread_regex_input_get(cw_nxo_t *a_nxo);

void
nxo_l_thread_regex_input_set(cw_nxo_t *a_nxo, cw_nxo_t *a_input);

cw_uint32_t
nxo_l_thread_regex_cont_get(cw_nxo_t *a_nxo);

void
nxo_l_thread_regex_cont_set(cw_nxo_t *a_nxo, cw_uint32_t a_cont);
#endif

cw_bool_t
nxoe_l_thread_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_thread_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_THREAD_C_))
#ifdef CW_REGEX
CW_INLINE cw_nxo_t *
nxo_l_thread_regex_input_get(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return &thread->regex_input;
}

CW_INLINE void
nxo_l_thread_regex_input_set(cw_nxo_t *a_nxo, cw_nxo_t *a_input)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    cw_check_ptr(a_input);
    cw_assert(a_input->magic == CW_NXO_MAGIC);

    nxo_dup(&thread->regex_input, a_input);
}

CW_INLINE cw_uint32_t
nxo_l_thread_regex_cont_get(cw_nxo_t *a_nxo)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    return thread->regex_cont;
}

CW_INLINE void
nxo_l_thread_regex_cont_set(cw_nxo_t *a_nxo, cw_uint32_t a_cont)
{
    cw_nxoe_thread_t *thread;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);

    thread = (cw_nxoe_thread_t *) a_nxo->o.nxoe;
    cw_dassert(thread->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(thread->nxoe.type == NXOT_THREAD);

    thread->regex_cont = a_cont;
}
#endif

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
    /* Used for remembering the current state of reference iteration.  This
     * function is only called by the garbage collector, so as long as two
     * interpreters aren't collecting simultaneously, using a static variable
     * works fine. */
    static cw_uint32_t ref_iter;

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
#ifdef CW_REGEX
	    case 8:
	    {
		retval = nxo_nxoe_get(&thread->regex_matches);
		break;
	    }
	    case 9:
	    {
		retval = nxo_nxoe_get(&thread->regex_input);
		break;
	    }
#endif
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
