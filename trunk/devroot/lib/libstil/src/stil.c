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

#include "../include/libstil/libstil.h"

#ifdef _LIBSTIL_DBG
#define _CW_STIL_MAGIC 0xae9678fd
#endif

/* Number of stack elements per memory chunk. */
#define _CW_STIL_STILSC_COUNT		 16

/*
 * Size and fullness control of initial root set for global VM.  Global VM is
 * empty to begin with.
 */
#define _CW_STIL_ROOTS_BASE_TABLE	 32
#define _CW_STIL_ROOTS_BASE_GROW	 24
#define _CW_STIL_ROOTS_BASE_SHRINK	  8

/* Used only by stil. */
cw_bool_t	stilng_l_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem);
void		stilng_l_delete(cw_stilng_t *a_stilng);

cw_stil_t *
stil_new(cw_stil_t *a_stil)
{
	cw_stil_t	*retval;

	if (a_stil != NULL) {
		retval = a_stil;
		memset(retval, 0, sizeof(cw_stil_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stil_t *)_cw_malloc(sizeof(cw_stil_t));
		if (retval == NULL)
			goto OOM_1;
		memset(retval, 0, sizeof(cw_stil_t));
		retval->is_malloced = TRUE;
	}

	if (stilag_new(&retval->stilag))
		goto OOM_2;
	if (stilng_l_new(&retval->stilng, stilag_mem_get(&retval->stilag)))
		goto OOM_3;
	mtx_new(&retval->lock);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STIL_MAGIC;
#endif

	return retval;
	OOM_3:
	stilag_delete(&retval->stilag);
	OOM_2:
	if (retval->is_malloced)
		_cw_free(retval);
	OOM_1:
	return NULL;
}

void
stil_delete(cw_stil_t *a_stil)
{
	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	stilng_l_delete(&a_stil->stilng);
	stilag_delete(&a_stil->stilag);
	mtx_delete(&a_stil->lock);

	if (a_stil->is_malloced)
		_cw_free(a_stil);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stil, 0x5a, sizeof(cw_stil_t));
#endif
}

cw_stil_bufc_t *
stil_stil_bufc_get(cw_stil_t *a_stil)
{
	cw_stil_bufc_t *retval;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	retval = _cw_stilag_stil_bufc_get(&a_stil->stilag);
	if (retval == NULL)
		goto RETURN;
	bufc_new(&retval->bufc, stilag_mem_get(&a_stil->stilag),
	    (cw_opaque_dealloc_t *)pool_put,
	    stilag_stil_bufc_pool_get(&a_stil->stilag));
	memset(retval->buffer, 0, sizeof(retval->buffer));
	bufc_buffer_set(&retval->bufc, retval->buffer, _CW_STIL_BUFC_SIZE, TRUE,
	    NULL, NULL);

	RETURN:
	return retval;
}
