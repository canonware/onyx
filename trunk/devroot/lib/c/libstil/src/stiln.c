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

/*
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with.
 */
#define _CW_STILN_BASE_TABLE	512
#define _CW_STILN_BASE_GROW	400
#define _CW_STILN_BASE_SHRINK	128

/* stiln. */
/* For stilo only. */
cw_bool_t	stiln_l_gdict_insert(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);
cw_bool_t	stiln_l_gdict_search(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);
cw_bool_t	stiln_l_gdict_remove(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);

/* stilng. */
/* For stil only. */
cw_bool_t	stilng_l_name_sref(cw_stilng_t *a_stilng, const cw_uint8_t
    *a_name, cw_uint32_t a_len, const void *a_key, const void *a_data,
    cw_stilo_t *r_stilo);
/* For stilo only. */
cw_bool_t	stilng_l_name_ref(cw_stilng_t *a_stilng, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_stilo_t *r_stilo);
cw_bool_t	stilng_l_gdict_search(cw_stilng_t *a_stilng, const cw_uint8_t
    *a_name, cw_uint32_t a_len);

/* stilnt. */
/* For stilo only. */
cw_bool_t	stilnt_l_name_ref(cw_stilnt_t *a_stilnt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_stilo_t *r_stilo);


/* stiln. */
void
stiln_new(cw_stiln_t *a_stiln)
{
	_cw_check_ptr(a_stiln);

	memset(a_stiln, 0, sizeof(cw_stiln_t));
	mtx_new(&a_stiln->lock);
}

void
stiln_delete(cw_stiln_t *a_stiln)
{
#ifdef _LIBSTIL_DBG
	if (a_stiln->keyed_refs != NULL) {
		cw_uint32_t	i;
		void		*key;

		_cw_out_put_e("Name \"");
		_cw_out_put_n(a_stiln->len, "[s]", a_stiln->name);
		_cw_out_put("\" still exists with [i] keyed reference[s]:",
		    dch_count(a_stiln->keyed_refs),
		    (dch_count(a_stiln->keyed_refs) == 1) ? "" : "s");
		for (i = 0; i < dch_count(a_stiln->keyed_refs); i++) {
			dch_get_iterate(a_stiln->keyed_refs, &key, NULL);
			_cw_out_put(" 0x[p]", key);
		}
		_cw_out_put("\n");
		abort();
	}
#endif
	mtx_delete(&a_stiln->lock);
}

cw_bool_t
stiln_l_gdict_insert(cw_stiln_t *a_stiln, const cw_stiloe_dict_t *a_dict)
{
	return TRUE;	/* XXX */
}
cw_bool_t
stiln_l_gdict_search(cw_stiln_t *a_stiln, const cw_stiloe_dict_t *a_dict)
{
	return TRUE;	/* XXX */
}

cw_bool_t
stiln_l_gdict_remove(cw_stiln_t *a_stiln, const cw_stiloe_dict_t *a_dict)
{
	return TRUE;	/* XXX */
}

/* stilng. */
cw_bool_t
stilng_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem)
{
	if (dch_new(&a_stilng->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, ch_hash_direct,
	    ch_key_comp_direct) == NULL)
		goto OOM;

	return FALSE;
	OOM:
	return TRUE;
}

void
stilng_delete(cw_stilng_t *a_stilng)
{
}

cw_bool_t
stilng_l_name_sref(cw_stilng_t *a_stilng, const cw_uint8_t *a_name, cw_uint32_t
    a_len, const void *a_key, const void *a_data, cw_stilo_t *r_stilo)
{
	return TRUE;	/* XXX */
}

cw_bool_t
stilng_l_name_ref(cw_stilng_t *a_stilng, const cw_uint8_t *a_name, cw_uint32_t
    a_len, cw_stilo_t *r_stilo)
{
	return TRUE;	/* XXX */
}

/* stilnt. */
cw_bool_t
stilnt_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem, cw_stilng_t *a_stilng)
{
	if (dch_new(&a_stilnt->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, ch_hash_direct,
	    ch_key_comp_direct) == NULL)
		goto OOM;

	/* XXX Need handle for allocation.  stilat, mem, or stilt? */
/*  	a_stilnt->stilng = a_stilng; */

	return FALSE;
	OOM:
	return TRUE;
}

void
stilnt_delete(cw_stilnt_t *a_stilnt)
{
}

cw_bool_t
stilnt_l_name_ref(cw_stilnt_t *a_stilnt, const cw_uint8_t *a_name, cw_uint32_t
    a_len, cw_stilo_t *r_stilo)
{
	return TRUE;	/* XXX */
}
