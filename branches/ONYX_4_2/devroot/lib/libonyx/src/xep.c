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

#include "../include/libonyx/libonyx.h"

#ifdef CW_DBG
static cw_bool_t cw_g_xep_initialized = FALSE;
#endif
#ifdef CW_THREADS
static cw_tsd_t cw_g_xep_key;
#else
static cw_xep_t *cw_g_xep_first;
#endif

void
xep_l_init(void)
{
    cw_assert(cw_g_xep_initialized == FALSE);

#ifdef CW_THREADS
    tsd_new(&cw_g_xep_key, NULL);
#else
    cw_g_xep_first = NULL;
#endif
#ifdef CW_DBG
    cw_g_xep_initialized = TRUE;
#endif
}

void
xep_l_shutdown(void)
{
    cw_assert(cw_g_xep_initialized);

#ifdef CW_THREADS
    tsd_delete(&cw_g_xep_key);
#endif
#ifdef CW_DBG
    cw_g_xep_initialized = FALSE;
#endif
}	

void
xep_throw_e(cw_xepv_t a_value, volatile const char *a_filename,
	    cw_uint32_t a_line_num)
{
    cw_xep_t *xep_first, *xep;

    cw_assert(cw_g_xep_initialized);
    cw_assert(a_value > CW_XEPS_CATCH);

    /* Iterate backward through the exception handlers until the exception is
     * handled or there are no more exception handlers. */
#ifdef CW_THREADS
    xep = xep_first = (cw_xep_t *) tsd_get(&cw_g_xep_key);
#else
    xep = xep_first = cw_g_xep_first;
#endif
    if (xep_first != NULL)
    {
	xep = qr_prev(xep_first, link);
    }
    else
    {
	/* No exception handlers at all. */
	fprintf(stderr, "%s(): Unhandled exception %u thrown at %s:%u\n",
		__FUNCTION__, a_value, a_filename, a_line_num);
	abort();
    }

    do
    {
	xep->is_handled = FALSE;
	xep->filename = a_filename;
	xep->line_num = a_line_num;

	switch (xep->state)
	{
	    case CW_XEPS_TRY:
	    {
		/* Execute the handler. */
		xep->value = a_value;
		xep->state = CW_XEPS_CATCH;
		longjmp(xep->context, (int)a_value);
		cw_not_reached();
	    }
	    case CW_XEPS_CATCH:
	    {
		/* Exception thrown within handler; propagate. */
		break;
	    }
	    default:
	    {
		cw_not_reached();
	    }
	}

	xep = qr_prev(xep, link);
    } while (xep != xep_first);

    /* No more exception handlers. */
    fprintf(stderr, "%s(): Unhandled exception %u thrown at %s:%u\n",
	    __FUNCTION__, a_value, xep->filename, xep->line_num);
    abort();
}

void
xep_p_retry(cw_xep_t *a_xep)
{
    cw_assert(cw_g_xep_initialized);

#ifdef CW_DBG
    switch (a_xep->state)
    {
	case CW_XEPS_CATCH:
	{
	    break;
	}
	case CW_XEPS_TRY:
	{
	    cw_error("Exception retry outside handler");
	}
	default:
	{
	    cw_not_reached();
	}
    }
#endif
    a_xep->value = CW_XEPV_NONE;
    a_xep->state = CW_XEPS_TRY;
    a_xep->is_handled = TRUE;
    longjmp(a_xep->context, (int)CW_XEPV_CODE);
    cw_not_reached();
}

void
xep_p_handled(cw_xep_t *a_xep)
{
    cw_assert(cw_g_xep_initialized);

#ifdef CW_DBG
    switch (a_xep->state)
    {
	case CW_XEPS_CATCH:
	{
	    break;
	}
	case CW_XEPS_TRY:
	{
	    cw_error("Exception handled outside handler");
	}
	default:
	{
	    cw_not_reached();
	}
    }
#endif

    a_xep->is_handled = TRUE;
    xep_p_unlink(a_xep);
}

void
xep_p_link(cw_xep_t *a_xep)
{
    cw_xep_t *xep_first;

    cw_assert(cw_g_xep_initialized);

#ifdef CW_THREADS
    xep_first = (cw_xep_t *) tsd_get(&cw_g_xep_key);
#else
    xep_first = cw_g_xep_first;
#endif

    /* Link into the xep ring, if it exists. */
    qr_new(a_xep, link);
    if (xep_first != NULL)
    {
	cw_check_ptr(qr_prev(xep_first, link));
	cw_check_ptr(qr_next(xep_first, link));

	qr_before_insert(xep_first, a_xep, link);
    }
    else
    {
#ifdef CW_THREADS
	tsd_set(&cw_g_xep_key, (void *) a_xep);
#else
	cw_g_xep_first = a_xep;
#endif
    }

    a_xep->value = CW_XEPV_NONE;
    a_xep->state = CW_XEPS_TRY;
    a_xep->is_handled = TRUE;
    a_xep->is_linked = TRUE;
}

void
xep_p_unlink(cw_xep_t *a_xep)
{
    cw_xep_t *xep_first;

    cw_assert(cw_g_xep_initialized);

    if (a_xep->is_linked)
    {
#ifdef CW_THREADS
	xep_first = (cw_xep_t *) tsd_get(&cw_g_xep_key);
#else
	xep_first = cw_g_xep_first;
#endif
	cw_check_ptr(qr_prev(xep_first, link));
	cw_check_ptr(qr_next(xep_first, link));

	/* Remove handler from ring. */
	if (a_xep != xep_first)
	{
	    qr_remove(a_xep, link);
	}
	else
	{
#ifdef CW_THREADS
	    tsd_set(&cw_g_xep_key, NULL);
#else
	    cw_g_xep_first = NULL;
#endif
	}
	a_xep->is_linked = FALSE;

	if (a_xep->is_handled == FALSE)
	{
	    if (a_xep != xep_first)
	    {
		/* Propagate exception. */
		xep_throw_e(a_xep->value, a_xep->filename,
			    a_xep->line_num);
	    }
	    else
	    {
		/* No more exception handlers. */
		fprintf(stderr, "%s(): Unhandled exception "
			"%u thrown at %s:%u\n", __FUNCTION__,
			a_xep->value, a_xep->filename,
			a_xep->line_num);
		abort();
	    }
	}
    }
}
