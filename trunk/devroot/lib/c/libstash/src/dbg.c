/****************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ****************************************************************************
 *
 * Version: <Version>
 *
 ****************************************************************************/

#include "../include/libstash/libstash.h"
#include "../include/libstash/mem_l.h"

struct cw_dbg_s {
	cw_mtx_t	lock;
	cw_ch_t		*flag_hash;
};

/* Number of slots in the flags hash table. */
#define _CW_DBG_TABLE_SIZE 64

cw_dbg_t *
dbg_new(void)
{
	cw_dbg_t	*retval;

	retval = (cw_dbg_t *)_cw_malloc(sizeof(cw_dbg_t));
	if (retval == NULL)
		goto RETURN;

	mtx_new(&retval->lock);

	retval->flag_hash = ch_new(NULL, _CW_DBG_TABLE_SIZE, ch_hash_string,
	    ch_key_comp_string);
	if (retval->flag_hash == NULL) {
		mtx_delete(&retval->lock);
		_cw_free(retval);
		retval = NULL;
		goto RETURN;
	}

	RETURN:
	return retval;
}

void
dbg_delete(cw_dbg_t *a_dbg)
{
	cw_chi_t	*chi;

	_cw_check_ptr(a_dbg);

	while (ch_remove_iterate(a_dbg->flag_hash, NULL, NULL, &chi) == FALSE)
		_cw_free(chi);

	ch_delete(a_dbg->flag_hash);
	mtx_delete(&a_dbg->lock);
	_cw_free(a_dbg);
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
			chi = (cw_chi_t *)_cw_malloc(sizeof(cw_chi_t));
			if (chi == NULL)
				retval = TRUE;
			else
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
			_cw_free(chi);
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
