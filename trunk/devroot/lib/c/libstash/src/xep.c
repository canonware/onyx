/******************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstash/libstash.h"

#ifdef _LIBSTASH_DBG
cw_bool_t	cw_g_xep_initialized = FALSE;
#endif
cw_tsd_t	cw_g_xep_key;

void
xep_l_init(void)
{
	_cw_assert(cw_g_xep_initialized == FALSE);

	tsd_new(&cw_g_xep_key, NULL);
#ifdef _LIBSTASH_DBG
	cw_g_xep_initialized = TRUE;
#endif
}

void
xep_l_shutdown(void)
{
	_cw_assert(cw_g_xep_initialized);

	tsd_delete(&cw_g_xep_key);
#ifdef _LIBSTASH_DBG
	cw_g_xep_initialized = FALSE;
#endif
}	

void
xep_raise_e(cw_xepv_t a_value, const char *a_filename, cw_uint32_t a_line_num)
{
	cw_xep_t	*xep_first, *xep;

	_cw_assert(cw_g_xep_initialized);
	_cw_assert(a_value < _CW_XEPS_FINALLY);

	/*
	 * Iterate backward through the exception handlers until the exception
	 * is handled or there are no more exception handlers.
	 */
	xep = xep_first = (cw_xep_t *)tsd_get(&cw_g_xep_key);
	if (xep_first != NULL)
		xep = qr_prev(xep_first, link);
	else {
		/* No exception handlers at all. */
		_cw_out_put("[s](): Unhandled exception [i|s:s] "
		    "raised at [s], line [i]\n",
		    __FUNCTION__, a_value, a_filename, a_line_num);
		abort();
	}

	do {
		xep->is_handled = FALSE;
		xep->filename = a_filename;
		xep->line_num = a_line_num;

		switch (xep->state) {
		case _CW_XEPS_TRY:
			/* Execute the handler. */
			xep->value = a_value;
			xep->state = _CW_XEPS_CATCH;
			longjmp(xep->context, (int)a_value);
			/* Not reached. */
			_cw_error("Programming error");
		case _CW_XEPS_CATCH:
			/* Re-raise, do finally first. */
			xep->value = a_value;
			xep->state = _CW_XEPS_FINALLY;
			longjmp(xep->context, (int)_CW_XEPV_FINALLY);
			/* Not reached. */
			_cw_error("Programming error");
		case _CW_XEPS_FINALLY:
			/* Exception raised within finally; propagate. */
			break;
		default:
			_cw_error("Programming error");
		}

		xep = qr_prev(xep, link);
	} while (xep != xep_first);

	/* No more exception handlers. */
	_cw_out_put("[s](): Unhandled exception [i|s:s] "
	    "raised at [s], line [i]\n",
	    __FUNCTION__, a_value, xep->filename, xep->line_num);
	abort();
}

void
xep_retry(void)
{
	cw_xep_t	*xep;

	_cw_assert(cw_g_xep_initialized);

	xep = qr_prev((cw_xep_t *)tsd_get(&cw_g_xep_key), link);
	xep->value = _CW_XEPV_NONE;
	xep->state = _CW_XEPS_TRY;
	xep->is_handled = TRUE;
	longjmp(xep->context, (int)_CW_XEPV_CODE);
	/* Not reached. */
	_cw_error("Programming error");
}

void
xep_handled(void)
{
	cw_xep_t	*xep;

	_cw_assert(cw_g_xep_initialized);

	xep = qr_prev((cw_xep_t *)tsd_get(&cw_g_xep_key), link);
#ifdef _LIBSTASH_DBG
	switch (xep->state) {
	case _CW_XEPS_CATCH:
		break;
	case _CW_XEPS_TRY:
	case _CW_XEPS_FINALLY:
		_cw_error("Exception handled outside handler");
	default:
		_cw_error("Programming error");
	}
#endif

	xep->value = _CW_XEPV_NONE;
	xep->state = _CW_XEPS_TRY;
	xep->is_handled = TRUE;
}

void
xep_p_link(cw_xep_t *a_xep)
{
	cw_xep_t	*xep_first;

	_cw_assert(cw_g_xep_initialized);

	xep_first = (cw_xep_t *)tsd_get(&cw_g_xep_key);

	/* Link into the xep ring, if it exists. */
	qr_new(a_xep, link);
	if (xep_first != NULL)
		qr_before_insert(a_xep, xep_first, link);
	else
		tsd_set(&cw_g_xep_key, (void *)a_xep);

	a_xep->value = _CW_XEPV_NONE;
	a_xep->state = _CW_XEPS_TRY;
	a_xep->is_handled = TRUE;
}

cw_bool_t
xep_p_unlink(cw_xep_t *a_xep)
{
	cw_bool_t	retval;
	cw_xep_t	*xep_first, *xep;

	_cw_assert(cw_g_xep_initialized);

	xep_first = (cw_xep_t *)tsd_get(&cw_g_xep_key);
	xep = qr_prev(xep_first, link);

	switch (xep->state) {
	case _CW_XEPS_TRY:	/* No exception. */
	case _CW_XEPS_CATCH:	/* Exception now handled. */
		xep->state = _CW_XEPS_FINALLY;
		longjmp(xep->context, (int)_CW_XEPV_FINALLY);
		/* Not reached. */
		_cw_error("Programming error");
	case _CW_XEPS_FINALLY:	/* Done. */
		/* Remove handler from ring. */
		if (xep != xep_first)
			qr_remove(xep, link);
		else
			tsd_set(&cw_g_xep_key, NULL);

		if (xep->is_handled == FALSE) {
			if (xep != xep_first) {
				/* Propagate exception. */
				xep_raise_e(xep->value, xep->filename,
				    xep->line_num);
			} else {
				retval = TRUE;
				goto RETURN;
			}
		}
		break;
	default:
		_cw_error("Programming error");
	}

	retval = FALSE;
	RETURN:
	return retval;
}
