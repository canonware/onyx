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

/* Initial parameters for the sequence set dch. */
#define _CW_STILA_SEQ_SET_BASE		32
#define _CW_STILA_SEQ_SET_BASE_GROW	24
#define _CW_STILA_SEQ_SET_BASE_SHRINK	 8

static cw_bool_t stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem);
static void	stila_p_delete(cw_stila_t *a_stila);
static cw_bool_t stila_p_gc_register(cw_stila_t *a_stila, cw_stilt_t *a_stilt,
    cw_stiloe_t *a_stiloe);

/* stilag. */
cw_bool_t
stilag_new(cw_stilag_t *a_stilag)
{
	mtx_new(&a_stilag->lock);
	qq_init(&a_stilag->head);

	if (mem_new(&a_stilag->mem, cw_g_mem) == NULL)
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

	if (pool_new(&a_stilag->dicto_pool, &a_stilag->mem,
	    sizeof(cw_stiloe_dicto_t)) == NULL)
		goto OOM_6;

	if (stila_p_new(&a_stilag->stila, &a_stilag->mem))
		goto OOM_7;

	return FALSE;
	OOM_7:
	pool_delete(&a_stilag->dicto_pool);
	OOM_6:
	pool_delete(&a_stilag->stilsc_pool);
	OOM_5:
	pool_delete(&a_stilag->stiln_pool);
	OOM_4:
	pool_delete(&a_stilag->chi_pool);
	OOM_3:
	pool_delete(&a_stilag->stil_bufc_pool);
	OOM_2:
	mem_delete(&a_stilag->mem);
	OOM_1:
	return TRUE;
}

void
stilag_delete(cw_stilag_t *a_stilag)
{
	stila_p_delete(&a_stilag->stila);
	pool_delete(&a_stilag->dicto_pool);
	pool_delete(&a_stilag->stilsc_pool);
	pool_delete(&a_stilag->stiln_pool);
	pool_delete(&a_stilag->chi_pool);
	pool_delete(&a_stilag->stil_bufc_pool);
	mem_delete(&a_stilag->mem);
}

void *
stilag_malloc(cw_stilag_t *a_stilag, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return mem_malloc(&a_stilag->mem, a_size, a_filename, a_line_num);
}

cw_bool_t
stilag_gc_register(cw_stilag_t *a_stilag, cw_stilt_t *a_stilt, cw_stiloe_t
    *a_stiloe)
{
	cw_bool_t	retval;

	mtx_lock(&a_stilag->lock);
	retval = stila_p_gc_register(&a_stilag->stila, a_stilt, a_stiloe);
	mtx_unlock(&a_stilag->lock);

	return retval;
}

void
stilag_free(cw_stilag_t *a_stilag, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
	mem_free(&a_stilag->mem, a_ptr, a_filename, a_line_num);
}

/* stilat. */
cw_bool_t
stilat_new(cw_stilat_t *a_stilat, cw_stilt_t *a_stilt, cw_stilag_t *a_stilag)
{
	a_stilat->global = FALSE;
	a_stilat->stilt = a_stilt;
	a_stilat->stilag = a_stilag;

	if (stila_p_new(&a_stilat->stila, stilag_mem_get(a_stilag)))
		goto OOM;

	/* Add this stilat to the stilag's list. */
	mtx_lock(&a_stilag->lock);
	qq_insert_head(&a_stilag->head, a_stilat, link);
	mtx_unlock(&a_stilag->lock);

	return FALSE;
	OOM:
	return TRUE;
}

void
stilat_delete(cw_stilat_t *a_stilat)
{
	/* Remove this stilat from the stilag's list. */
	mtx_lock(&a_stilat->stilag->lock);
	qq_remove(&a_stilat->stilag->head, a_stilat, cw_stilat_t, link);
	mtx_unlock(&a_stilat->stilag->lock);

	stila_p_delete(&a_stilat->stila);
}

void *
stilat_malloc(cw_stilat_t *a_stilat, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return stilag_malloc(a_stilat->stilag, a_size, a_filename, a_line_num);
}

cw_bool_t
stilat_gc_register(cw_stilat_t *a_stilat, cw_stiloe_t *a_stiloe)
{
	cw_bool_t	retval;

	if (a_stilat->global == FALSE) {
		retval = stila_p_gc_register(&a_stilat->stila, a_stilat->stilt,
		    a_stiloe);
	} else {
		retval = stilag_gc_register(a_stilat->stilag, a_stilat->stilt,
		    a_stiloe);
	}

	return retval;
}

void
stilat_free(cw_stilat_t *a_stilat, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
	stilag_free(a_stilat->stilag, a_ptr, a_filename, a_line_num);
}

/* stila. */
static cw_bool_t
stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem)
{
	if (dch_new(&a_stila->seq_set, a_mem, _CW_STILA_SEQ_SET_BASE,
	    _CW_STILA_SEQ_SET_BASE_GROW, _CW_STILA_SEQ_SET_BASE_SHRINK,
	    ch_hash_direct, ch_key_comp_direct) == NULL)
		goto OOM;

	return FALSE;
	OOM:
	return TRUE;
}

static void
stila_p_delete(cw_stila_t *a_stila)
{
	dch_delete(&a_stila->seq_set);
}

static cw_bool_t
stila_p_gc_register(cw_stila_t *a_stila, cw_stilt_t *a_stilt, cw_stiloe_t
    *a_stiloe)
{
	cw_chi_t	*chi;

	_cw_assert(dch_search(&a_stila->seq_set, (void *)a_stiloe, NULL));

	chi = _cw_stilt_chi_get(a_stilt);
	if (chi == NULL)
		goto OOM_1;

	if (dch_insert(&a_stila->seq_set, (void *)a_stiloe, NULL, chi))
		goto OOM_2;

	return FALSE;
	OOM_2:
	_cw_stilt_chi_put(a_stilt, chi);
	OOM_1:
	return TRUE;
}
