/****************************************************************************
 *
 * <Copyright = "jasone">
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstil/libstil.h"

cw_bool_t
stila_gnew(cw_stila_t *a_stila, cw_pool_t *a_stil_bufc_pool, cw_pool_t
    *a_chi_pool, cw_pool_t *a_stiln_pool, cw_pool_t *a_stilsc_pool, cw_pool_t
    *a_dicto_pool)
{
	if (mem_new(&a_stila->mem, NULL))
		goto OOM_1;
	a_stila->global = TRUE;
	qq_init(&a_stila->l.g.head);

	/* XXX Magic numbers. */
	if (dch_new(&a_stila->seq_set, &a_stila->mem, 16, 12, 4, ch_hash_direct,
	    ch_key_comp_direct))
		goto OOM_2;

#ifdef _LIBSTIL_DBG
	/* XXX Magic numbers. */
	if (dch_new(&a_stila->seq_complement, &a_stila->mem, 16, 12, 4,
	    ch_hash_direct, ch_key_comp_direct))
		goto OOM_3;
#endif

	a_stila->stil_bufc_pool = a_stil_bufc_pool;
	if (pool_new(a_stila->stil_bufc_pool, &a_stila->mem,
	    sizeof(cw_stil_bufc_t)) == NULL)
		goto OOM_4;

	a_stila->chi_pool = a_chi_pool;
	if (pool_new(a_stila->chi_pool, &a_stila->mem, sizeof(cw_chi_t)) ==
	    NULL)
		goto OOM_5;

	a_stila->stiln_pool = a_stiln_pool;
	if (pool_new(a_stila->stiln_pool, &a_stila->mem, sizeof(cw_stiln_t)) ==
	    NULL)
		goto OOM_6;

	a_stila->stilsc_pool = a_stilsc_pool;
	if (pool_new(a_stila->stilsc_pool, &a_stila->mem, sizeof(cw_stilsc_t))
	    == NULL)
		goto OOM_7;

	/* XXX Set up oom handler. */

	return FALSE;
	OOM_7:
	OOM_6:
	OOM_5:
	OOM_4:
#ifdef _LIBSTIL_DBG
	OOM_3:
#endif
	OOM_2:
	OOM_1:
	return TRUE;
}

void
stila_tnew(cw_stila_t *a_stila, cw_stila_t *a_other, cw_stilt_t *a_stilt)
{
}

void
stila_delete(cw_stila_t *a_stila)
{
}

void *
stila_malloc(cw_stila_t *a_stila, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return NULL;	/* XXX */
}

void
stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
}

void
stila_free(cw_stila_t *a_stila, void *a_ptr, const char *a_filename, cw_uint32_t
    a_line_num)
{
}
