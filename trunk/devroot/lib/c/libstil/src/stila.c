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
#include "../include/libstil/stil_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilt_l.h"

/* Number of stack elements per memory chunk. */
#define	_CW_STIL_STILSC_COUNT	16

static void *stila_p_gc_entry(void *a_arg);

void
stila_new(cw_stila_t *a_stila, cw_stil_t *a_stil)
{
	/* XXX Handle OOM. */
	mtx_new(&a_stila->lock);
	mem_new(&a_stila->mem, NULL);
	ql_new(&a_stila->seq_set);
	pool_new(&a_stila->chi_pool, NULL, sizeof(cw_chi_t));
	pool_new(&a_stila->stilsc_pool, NULL,
	    _CW_STILSC_O2SIZEOF(_CW_STIL_STILSC_COUNT));
	pool_new(&a_stila->dicto_pool, NULL,
	    sizeof(cw_stiloe_dicto_t));

	a_stila->stil = a_stil;
	thd_new(&a_stila->gc_thd, stila_p_gc_entry, (void *)a_stila);
}

void
stila_delete(cw_stila_t *a_stila)
{
	pool_delete(&a_stila->dicto_pool);
	pool_delete(&a_stila->stilsc_pool);
	pool_delete(&a_stila->chi_pool);
	mem_delete(&a_stila->mem);

	thd_join(&a_stila->gc_thd);
}

void
stila_gc_register(cw_stila_t *a_stila, cw_stiloe_t *a_stiloe)
{
	mtx_lock(&a_stila->lock);
	ql_tail_insert(&a_stila->seq_set, a_stiloe, link);
	mtx_unlock(&a_stila->lock);
}

void
stila_gc_force(cw_stila_t *a_stila)
{

}

static void *
stila_p_gc_entry(void *a_arg)
{
/*  	cw_stila_t	*stila = (cw_stila_t *)a_arg; */

	return NULL;
}
