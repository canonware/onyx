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
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with.
 */
#define _CW_STIL_NAME_BASE_TABLE	512
#define _CW_STIL_NAME_BASE_GROW		400
#define _CW_STIL_NAME_BASE_SHRINK	128

/*
 * Size and fullness control of initial root set for global VM.  Global VM is
 * empty to begin with.
 */
#define _CW_STIL_ROOTS_BASE_TABLE	 32
#define _CW_STIL_ROOTS_BASE_GROW	 24
#define _CW_STIL_ROOTS_BASE_SHRINK	  8

/*
 * Initial size of globaldict.  This is a bit arbitrary, and some applications
 * could benefit from making it larger or smaller.
 */
#define	_CW_STIL_GLOBALDICT_SIZE	 64

cw_stil_t *
stil_new(cw_stil_t *a_stil)
{
	cw_stil_t	*retval;
	cw_stilt_t	stilt;

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

	mtx_new(&retval->name_lock);
	if (dch_new(&retval->name_hash, stilag_mem_get(&retval->stilag),
	    _CW_STIL_NAME_BASE_TABLE, _CW_STIL_NAME_BASE_GROW,
	    _CW_STIL_NAME_BASE_SHRINK, stilo_name_hash, stilo_name_key_comp) ==
	    NULL)
		goto OOM_3;
	
	/*
	 * Create a temporary thread in order to be able to initialize
	 * systemdict, and destroy the thread as soon as we're done.
	 */

	/* Initialize systemdict, since stilt_new() will access it. */
	stilo_no_new(&retval->systemdict);

	if (stilt_new(&stilt, retval) == NULL)
		goto OOM_4;

	stilt_setglobal(&stilt, TRUE);
	/* XXX No way to catch OOM here. */
	systemdict_populate(&retval->systemdict, &stilt);
	stilo_dict_new(&retval->globaldict, &stilt, _CW_STIL_GLOBALDICT_SIZE);
	stilt_delete(&stilt);

	mtx_new(&retval->lock);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STIL_MAGIC;
#endif

	return retval;
	OOM_4:
	dch_delete(&retval->name_hash);
	OOM_3:
	mtx_delete(&retval->name_lock);
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
	cw_stilt_t	stilt;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);
	
	/*
	 * Create a temporary thread in order to be able to destroy systemdict
	 * and globaldict.
	 */
	if (stilt_new(&stilt, a_stil) == NULL)
		_cw_error("XXX OOM unhandled");
	stilt_setglobal(&stilt, TRUE);
	/* XXX No way to catch OOM here. */
	stilo_delete(&a_stil->globaldict, &stilt);
	stilo_delete(&a_stil->systemdict, &stilt);
	stilt_delete(&stilt);

	/* XXX Run the GC one last time. */

	dch_delete(&a_stil->name_hash);
	mtx_delete(&a_stil->name_lock);
	stilag_delete(&a_stil->stilag);
	mtx_delete(&a_stil->lock);

	if (a_stil->is_malloced)
		_cw_free(a_stil);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stil, 0x5a, sizeof(cw_stil_t));
#endif
}
