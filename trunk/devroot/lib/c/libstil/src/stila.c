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

static cw_bool_t stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem);
static void	stila_p_delete(cw_stila_t *a_stila);
/*  static cw_bool_t stila_p_gc_register(cw_stila_t *a_stila, cw_stiloe_t */
/*      *a_stiloe); */

/* stilag. */
cw_bool_t
stilag_new(cw_stilag_t *a_stilag)
{
	mtx_new(&a_stilag->lock);
	qq_init(&a_stilag->head);

	if (mem_new(&a_stilag->mem, cw_g_mem))
		goto OOM_1;

	if (pool_new(&a_stilag->stil_bufc_pool, &a_stilag->mem,
	    sizeof(cw_stil_bufc_t)) == NULL)
		goto OOM_2;

	if (pool_new(&a_stilag->chi_pool, &a_stilag->mem, sizeof(cw_chi_t)) ==
	    NULL)
		goto OOM_3;

	if (pool_new(&a_stilag->stiln_pool, &a_stilag->mem, sizeof(cw_stiln_t))
	    == NULL)
		goto OOM_4;

	if (pool_new(&a_stilag->stilsc_pool, &a_stilag->mem,
	    sizeof(cw_stilsc_t)) == NULL)
		goto OOM_5;

	if (stila_p_new(&a_stilag->stila, &a_stilag->mem))
		goto OOM_6;

	return FALSE;
	OOM_6:
	OOM_5:
	OOM_4:
	OOM_3:
	OOM_2:
	OOM_1:
	return TRUE;
}

void
stilag_delete(cw_stilag_t *a_stilag)
{
	stila_p_delete(&a_stilag->stila);
}

void *
stilag_malloc(cw_stilag_t *a_stilag, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return NULL;	/* XXX */
}

void
stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
}

void
stilag_free(cw_stilag_t *a_stilag, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
}

/* stilat. */
cw_bool_t
stilat_new(cw_stilat_t *a_stilat, cw_stilt_t *a_stilt, cw_stilag_t *a_stilag)
{
	return TRUE;	/* XXX */
}

void
stilat_delete(cw_stilat_t *a_stilat)
{
}

void *
stilat_malloc(cw_stilat_t *a_stilat, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return NULL;	/* XXX */
}

void
stilat_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
}

void
stilat_free(cw_stilat_t *a_stilat, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
}

/* stila. */
static cw_bool_t
stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem)
{
	/* XXX Magic numbers. */
	if (dch_new(&a_stila->seq_set, a_mem, 16, 12, 4, ch_hash_direct,
	    ch_key_comp_direct))
		goto OOM_1;

#ifdef _LIBSTIL_DBG
	/* XXX Magic numbers. */
	if (dch_new(&a_stila->seq_complement, a_mem, 16, 12, 4, ch_hash_direct,
	    ch_key_comp_direct))
		goto OOM_2;
#endif

	return FALSE;
#ifdef _LIBSTIL_DBG
	OOM_2:
	dch_delete(&a_stila->seq_set);
#endif
	OOM_1:
	return TRUE;
}

static void
stila_p_delete(cw_stila_t *a_stila)
{
}

#if (0)
static cw_bool_t
stila_p_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
	return TRUE;	/* XXX */
}
#endif
