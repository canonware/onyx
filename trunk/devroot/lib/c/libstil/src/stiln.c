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

/*
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with.
 */
#define _CW_STILN_BASE_TABLE	512
#define _CW_STILN_BASE_GROW	400
#define _CW_STILN_BASE_SHRINK	128

/* stilng. */
cw_bool_t
stilng_l_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem)
{
	mtx_new(&a_stilng->lock);
	if (dch_new(&a_stilng->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, stilo_name_hash,
	    stilo_name_key_comp) == NULL)
		goto OOM;

	return FALSE;
	OOM:
	return TRUE;
}

void
stilng_l_delete(cw_stilng_t *a_stilng)
{
	/* XXX Clean up the hash table if it isn't empty. */
	dch_delete(&a_stilng->hash);
	mtx_delete(&a_stilng->lock);
}

/* stilnt. */
cw_bool_t
stilnt_l_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem, cw_stilng_t *a_stilng)
{
	if (dch_new(&a_stilnt->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, ch_direct_hash,
	    ch_direct_key_comp) == NULL)
		goto OOM;

	return FALSE;
	OOM:
	return TRUE;
}

void
stilnt_l_delete(cw_stilnt_t *a_stilnt)
{
	dch_delete(&a_stilnt->hash);
}
