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

/* stiln. */
/* For stilo only. */
cw_bool_t	stiln_l_gdict_insert(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);
cw_bool_t	stiln_l_gdict_search(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);
cw_bool_t	stiln_l_gdict_remove(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);

/* Hashing functions for stiln's. */
static cw_uint32_t	stiln_p_hash(const void *a_key);
static cw_bool_t	stiln_p_key_comp(const void *a_k1, const void *a_k2);

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
cw_bool_t	stilnt_l_name_ref(cw_stilnt_t *a_stilnt, cw_stilt_t * a_stilt,
    const cw_uint8_t *a_name, cw_uint32_t a_len, cw_stilo_t *r_stilo);

/* stiln. */
void
stiln_new(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt, const cw_uint32_t *a_name,
    cw_uint32_t a_len, cw_bool_t a_is_static)
{
	_cw_check_ptr(a_stiln);

	memset(a_stiln, 0, sizeof(cw_stiln_t));
	mtx_new(&a_stiln->lock);
	a_stiln->is_static_name = a_is_static;
	a_stiln->len = a_len;
	
	if (a_is_static == FALSE) {
		/* This should be allocated from global space. */
		a_stiln->name =
		    _cw_stilag_malloc(stil_stilag_get(stilt_stil_get(a_stilt)),
		    a_len);
		memcpy(a_stiln->name, a_name, a_len);
	} else
		a_stiln->name = a_name;
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

static cw_uint32_t
stiln_p_hash(const void *a_key)
{
	cw_uint32_t	retval, i;
	cw_stiln_t	*key = (cw_stiln_t *)a_key;
	const char	*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->name, retval = 0; i < key->len; i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

static cw_bool_t
stiln_p_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stiln_t	*k1 = (cw_stiln_t *)a_k1;
	cw_stiln_t	*k2 = (cw_stiln_t *)a_k2;
	size_t		len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->name, (char *)k2->name, len) ? FALSE : TRUE;
}

/* stilng. */
cw_bool_t
stilng_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem)
{
	if (dch_new(&a_stilng->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, stiln_p_hash,
	    stiln_p_key_comp) == NULL)
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
	cw_stiln_t		stiln;
	cw_stiloe_name_t	*name;

	/*
	 * XXX There's a race condition here.  We can find a name in the hash
	 * table, then have it deleted out of under us before we get around to
	 * setting r_stilo.
	 */

	/* Fake up a key so we can search through the global name cache. */
	stiln.name = a_name;
	stiln.len = a_len;

	/* Search. */
	if (dch_search(&a_stilng->hash, (void *)&stiln, (void **)&name)) {
		/* Not found.  Create a stiloe_name that can be inserted. */
		name = stiloe_

	} else {
		/* Found.  
		thd_crit_enter();
		r_stilo->o.stiloe = (cw_stiloe_t *)name;
		thd_crit_leave();
	}
	return TRUE;	/* XXX */
}

/* stilnt. */
cw_bool_t
stilnt_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem, cw_stilng_t *a_stilng)
{
	if (dch_new(&a_stilnt->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, ch_direct_hash,
	    ch_direct_key_comp) == NULL)
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
stilnt_l_name_ref(cw_stilnt_t *a_stilnt, cw_stilt_t * a_stilt, const cw_uint8_t
    *a_name, cw_uint32_t a_len, cw_stilo_t *r_stilo)
{
	return TRUE;	/* XXX */
}
