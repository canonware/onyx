/******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

#include "../include/libstil/libstil.h"

/* Initial parameters for the sequence set dch. */
#define _CW_STILA_SEQ_SET_BASE		32
#define _CW_STILA_SEQ_SET_BASE_GROW	24
#define _CW_STILA_SEQ_SET_BASE_SHRINK	 8

/* Number of stack elements per memory chunk. */
#define _CW_STIL_STILSC_COUNT		16

static void	stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem);
static void	stila_p_delete(cw_stila_t *a_stila);
static void	stila_p_gc_register(cw_stila_t *a_stila, cw_stilt_t *a_stilt,
    cw_stiloe_t *a_stiloe);

/* stilag. */
void
stilag_new(cw_stilag_t *a_stilag)
{
	mtx_new(&a_stilag->lock);
	ql_new(&a_stilag->head);

	mem_new(&a_stilag->mem, NULL);
	pool_new(&a_stilag->chi_pool, NULL, sizeof(cw_chi_t));
	pool_new(&a_stilag->stilsc_pool, NULL,
	    _CW_STILSC_O2SIZEOF(_CW_STIL_STILSC_COUNT));
	pool_new(&a_stilag->dicto_pool, NULL,
	    sizeof(cw_stiloe_dicto_t));
	stila_p_new(&a_stilag->stila, &a_stilag->mem);
}

void
stilag_delete(cw_stilag_t *a_stilag)
{
	stila_p_delete(&a_stilag->stila);
	pool_delete(&a_stilag->dicto_pool);
	pool_delete(&a_stilag->stilsc_pool);
	pool_delete(&a_stilag->chi_pool);
	mem_delete(&a_stilag->mem);
}

void *
stilag_malloc_e(cw_stilag_t *a_stilag, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return mem_malloc_e(&a_stilag->mem, a_size, a_filename, a_line_num);
}

void
stilag_free_e(cw_stilag_t *a_stilag, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
	mem_free_e(&a_stilag->mem, a_ptr, a_filename, a_line_num);
}

void *
stilag_gc_malloc_e(cw_stilag_t *a_stilag, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	/* XXX Not at all right. */
	return mem_malloc_e(&a_stilag->mem, a_size, a_filename, a_line_num);
}

void
stilag_gc_register(cw_stilag_t *a_stilag, cw_stilt_t *a_stilt, cw_stiloe_t
    *a_stiloe)
{
	mtx_lock(&a_stilag->lock);
	stila_p_gc_register(&a_stilag->stila, a_stilt, a_stiloe);
	mtx_unlock(&a_stilag->lock);
}

/* stilat. */
void
stilat_new(cw_stilat_t *a_stilat, cw_stilt_t *a_stilt, cw_stilag_t *a_stilag)
{
	ql_elm_new(a_stilat, link);
	a_stilat->global = FALSE;
	a_stilat->stilt = a_stilt;
	a_stilat->stilag = a_stilag;

	stila_p_new(&a_stilat->stila, stilag_mem_get(a_stilag));

	/* Add this stilat to the stilag's list. */
	mtx_lock(&a_stilag->lock);
	ql_head_insert(&a_stilag->head, a_stilat, link);
	mtx_unlock(&a_stilag->lock);
}

void
stilat_delete(cw_stilat_t *a_stilat)
{
	/* Remove this stilat from the stilag's list. */
	mtx_lock(&a_stilat->stilag->lock);
	ql_remove(&a_stilat->stilag->head, a_stilat, link);
	mtx_unlock(&a_stilat->stilag->lock);

	stila_p_delete(&a_stilat->stila);
}

void *
stilat_malloc_e(cw_stilat_t *a_stilat, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	return stilag_malloc_e(a_stilat->stilag, a_size, a_filename,
	    a_line_num);
}

void *
stilat_gc_malloc_e(cw_stilat_t *a_stilat, size_t a_size, const char *a_filename,
    cw_uint32_t a_line_num)
{
	/* XXX Not at all right. */
	return stilag_malloc_e(a_stilat->stilag, a_size, a_filename,
	    a_line_num);
}

void
stilat_gc_register(cw_stilat_t *a_stilat, cw_stiloe_t *a_stiloe)
{
	if (a_stilat->global == FALSE) {
		stila_p_gc_register(&a_stilat->stila, a_stilat->stilt,
		    a_stiloe);
	} else
		stilag_gc_register(a_stilat->stilag, a_stilat->stilt, a_stiloe);
}

void
stilat_free_e(cw_stilat_t *a_stilat, void *a_ptr, const char *a_filename,
    cw_uint32_t a_line_num)
{
	stilag_free_e(a_stilat->stilag, a_ptr, a_filename, a_line_num);
}

cw_chi_t *
stilat_chi_get_e(cw_stilat_t *a_stilat, const char *a_filename, cw_uint32_t
    a_line_num)
{
	return (cw_chi_t *)pool_get_e(&a_stilat->stilag->chi_pool, a_filename,
	    a_line_num);
}

cw_stilsc_t *
stilat_stilsc_get_e(cw_stilat_t *a_stilat, const char *a_filename, cw_uint32_t
    a_line_num)
{
	return (cw_stilsc_t *)pool_get_e(&a_stilat->stilag->stilsc_pool,
	    a_filename, a_line_num);
}

cw_stiloe_dicto_t *
stilat_dicto_get_e(cw_stilat_t *a_stilat, const char *a_filename, cw_uint32_t
    a_line_num)
{
	return (cw_stiloe_dicto_t *)pool_get_e(&a_stilat->stilag->dicto_pool,
	    a_filename, a_line_num);
}

/* stila. */
static void
stila_p_new(cw_stila_t *a_stila, cw_mem_t *a_mem)
{
	dch_new(&a_stila->seq_set, a_mem, _CW_STILA_SEQ_SET_BASE,
	    _CW_STILA_SEQ_SET_BASE_GROW, _CW_STILA_SEQ_SET_BASE_SHRINK,
	    ch_direct_hash, ch_direct_key_comp);
}

static void
stila_p_delete(cw_stila_t *a_stila)
{
	/* XXX Check whether the sequence set is empty. */
	dch_delete(&a_stila->seq_set);
}

static void
stila_p_gc_register(cw_stila_t *a_stila, cw_stilt_t *a_stilt, cw_stiloe_t
    *a_stiloe)
{
	cw_chi_t	*chi;

	_cw_assert(dch_search(&a_stila->seq_set, (void *)a_stiloe, NULL));

	chi = stilt_chi_get(a_stilt);
	xep_begin();
	xep_try {
		dch_insert(&a_stila->seq_set, (void *)a_stiloe, NULL, chi);
	}
	xep_catch(_CW_XEPV_OOM) {
		stilt_chi_put(a_stilt, chi);
	}
	xep_end();
}
