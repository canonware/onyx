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

#include "../include/libstash/libstash.h"

struct cw_dbg_s {
	cw_mem_t	*mem;
	cw_mtx_t	lock;
	cw_ch_t		*flag_hash;
};

/* Number of slots in the flags hash table. */
#define _CW_DBG_TABLE_SIZE 64

cw_dbg_t *
dbg_new(cw_mem_t *a_mem)
{
	cw_dbg_t	*retval;

	retval = (cw_dbg_t *)mem_malloc(a_mem, sizeof(cw_dbg_t));

	retval->mem = a_mem;
	mtx_new(&retval->lock);

	xep_begin();
	xep_try {
		retval->flag_hash = ch_new(NULL, a_mem, _CW_DBG_TABLE_SIZE,
		    ch_string_hash, ch_string_key_comp);
	}
	xep_catch(_CW_XEPV_OOM) {
		mtx_delete(&retval->lock);
		mem_free(a_mem, retval);
	}
	xep_end();

	return retval;
}

void
dbg_delete(cw_dbg_t *a_dbg)
{
	cw_chi_t	*chi;

	_cw_check_ptr(a_dbg);

	while (ch_remove_iterate(a_dbg->flag_hash, NULL, NULL, &chi) == FALSE)
		mem_free(a_dbg->mem, chi);

	ch_delete(a_dbg->flag_hash);
	mtx_delete(&a_dbg->lock);
	mem_free(a_dbg->mem, a_dbg);
}

cw_bool_t
dbg_register(cw_dbg_t *a_dbg, const char *a_flag)
{
	cw_bool_t	retval = FALSE;
	cw_chi_t	*chi;

	if (a_dbg != NULL) {
		mtx_lock(&a_dbg->lock);
		if (ch_search(a_dbg->flag_hash, a_flag, NULL)) {
			/* Flag not registered. */
			xep_begin();
			xep_try {
				chi = (cw_chi_t *)mem_malloc(a_dbg->mem,
				    sizeof(cw_chi_t));
			}
			xep_catch(_CW_XEPV_OOM) {
				mtx_unlock(&a_dbg->lock);
			}
			xep_end();

			ch_insert(a_dbg->flag_hash, a_flag, NULL, chi);
		}
		mtx_unlock(&a_dbg->lock);
	} else
		retval = TRUE;

	return retval;
}

void
dbg_unregister(cw_dbg_t *a_dbg, const char *a_flag)
{
	cw_chi_t	*chi;

	if (a_dbg != NULL) {
		if (ch_remove(a_dbg->flag_hash, a_flag, NULL, NULL, &chi) ==
		    FALSE)
			mem_free(a_dbg->mem, chi);
	}
}

cw_bool_t
dbg_is_registered(cw_dbg_t *a_dbg, const char *a_flag)
{
	cw_bool_t	retval;

	_cw_check_ptr(a_flag);

	if (a_dbg != NULL)
		retval = !ch_search(a_dbg->flag_hash, a_flag, NULL);
	else
		retval = FALSE;

	return retval;
}
