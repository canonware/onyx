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

/* XXX Functions that need this should be moved to stilo.c. */
static void	stiloe_p_new(cw_stiloe_t *a_stiloe, cw_stilot_t a_type);

/* For stilo only. */
cw_stiloe_name_t *stiln_l_ref(cw_stilt_t *a_stilt, const char *a_str,
    cw_uint32_t a_len, cw_bool_t a_is_static);
cw_bool_t	stiln_l_gdict_insert(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt,
    const cw_stiloe_dict_t *a_dict);
cw_bool_t	stiln_l_gdict_search(cw_stiln_t *a_stiln, const cw_stiloe_dict_t
    *a_dict);
cw_bool_t	stiln_l_gdict_remove(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt,
    const cw_stiloe_dict_t *a_dict);
#define		stiln_l_val_get(a_stiln)	(a_stiln)->name
#define		stiln_l_len_get(a_stiln)	(a_stiln)->len

static void	stiln_p_new(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt,
    const cw_uint32_t *a_name, cw_uint32_t a_len, cw_bool_t a_is_static);
static void	stiln_p_delete(cw_stiln_t *a_stiln);
static cw_uint32_t stiln_p_hash(const void *a_key);
static cw_bool_t stiln_p_key_comp(const void *a_k1, const void *a_k2);

/* stilng. */
/* For stil only. */
cw_bool_t	stilng_l_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem);
void		stilng_l_delete(cw_stilng_t *a_stilng);

/* stilnt. */
/* For stilo only. */
cw_bool_t	stilnt_l_new(cw_stilnt_t *a_stilnt, cw_mem_t *a_mem,
    cw_stilng_t *a_stilng);
void		stilnt_l_delete(cw_stilnt_t *a_stilnt);

/* Return a reference to a stiln. */
cw_stiloe_name_t *
stiln_l_ref(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len, cw_bool_t
    a_is_static)
{
	cw_stiloe_name_t	*retval;
	cw_stiln_t		key;

	/* Fake up a key so that we can search the hash tables. */
	key.name = a_str;
	key.len = a_len;
	   
	if (stilt_currentglobal(a_stilt) == FALSE) {
		cw_stilnt_t	*stilnt;
		/*
		 * Look in the per-thread name cache for a cached reference to
		 * the name.  If there is no cached reference, check the global
		 * hash for the name.  Create the name in the global hash if
		 * necessary, then create a cached reference if necessary.
		 */
		stilnt = stilt_stilnt_get(a_stilt);
		if (dch_search(&stilnt->hash, (void *)&key, (void **)&retval)) {
			/* Not found in the per-thread name cache. */
			gname = stiln_p_gref(a_stilt, a_str, a_len,
			    a_is_static);

			retval = (cw_stiloe_name_t *)_cw_stilt_malloc(a_stilt,
			    sizeof(cw_stlioe_name_t));

			retval->val = gname;

			/* XXX stilo-internal initialization. */
			memset(&retval->e.i.stilo, 0, sizeof(cw_stilo_t));
			retval->e.i.stilo.type = _CW_STILOT_NAMETYPE;
			retval->e.i.stilo.stiloe = gname;

			/* XXX */
			stiloe_p_new(retval, _CW_STILOT_NAMETYPE);
			retval->stiloe.indirect = TRUE;

			/*
			 * Insert a cached entry for this thread.
			 */
			dch_insert(&stilnt->hash, (void *)&retval->key, (void
			    **)retval, _cw_stilt_chi_get(a_stilt));
		}
	} else
		retval = stiln_p_gref(a_stilt, a_str, a_len, a_is_static);

	return retval;
}

static cw_stiloe_name_t *
stiln_p_gref(cw_stilt_t *a_stilt, const char *a_str, cw_uint32_t a_len,
    cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*retval;
	cw_stiln_t		key;
	cw_stilng_t		*stilng;

	/* Fake up a key so that we can search the hash tables. */
	key.name = a_name;
	key.len = a_len;

	stilng = stil_stilng_get(stilt_stil_get(a_stilt));
	
	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(&stilng->lock);
	if (dch_search(&stilng->hash, (void *)&key, (void **)&retval)) {
		cw_stilag_t	*stilag;

		/*
		 * Not found in the global hash.  Create, initialize, and insert
		 * a new entry.
		 */
		stilag = stil_stilag_get(stilt_stil_get(a_stilt));
		retval = (cw_stiloe_name_t *)_cw_stilag_malloc(stilag,
		    sizeof(cw_stiloe_name_t));

		retval->val = retval;

		stiln_p_new(&retval->e.n.key, a_stilt, a_name, a_len,
		    a_is_static);

		/* XXX */
		stiloe_p_new(retval, _CW_STILOT_NAMETYPE);

		dch_insert(&stilng->hash, (void *)&retval->e.n.key, (void
		    **)retval, _cw_stilt_chi_get(a_stilt));
	}
	mtx_unlock(&stilng->lock);

	return retval;
}

/* Insert a keyed reference. */
cw_bool_t
stiln_l_gdict_insert(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt, const
    cw_stiloe_dict_t *a_dict)
{
	cw_bool_t	retval;

	mtx_lock(&a_stiln->lock);
	if (a_stiln->keyed_refs == NULL) {
		/* No keyed references.  Create the hash. */
		a_stiln->keyed_refs = dch_new(NULL,
		    stilag_mem_get(stil_stilag_get(stilt_stil_get(a_stilt))),
		    _CW_STILN_KREF_TABLE, _CW_STILN_KREF_GROW,
		    _CW_STILN_KREF_SHRINK, ch_hash_direct, ch_key_comp_direct);
		/* XXX Check dch_new() return. */
	}

	if (dch_insert(a_stiln->keyed_refs, (void *)a_dict, NULL,
	    _cw_stilt_chi_get(a_stilt))) {
		retval = TRUE;
		goto RETURN;
	}

	retval = FALSE;
	RETURN:
	mtx_unlock(&a_stiln->lock);
	return retval;
}

/* Search for a keyed reference matching a_dict. */
cw_bool_t
stiln_l_gdict_search(cw_stiln_t *a_stiln, const cw_stiloe_dict_t *a_dict)
{
	cw_bool_t	retval;

	mtx_lock(&a_stiln->lock);
	if ((a_stiln->keyed_refs == NULL) || dch_search(a_stiln->keyed_refs,
	    (void *)a_dict)) {
		/* Not found. */
		retval = TRUE;
	} else {
		/* Found. */
		retval = TRUE;
	}
	mtx_unlock(&a_stiln->lock);

	return retval;
}

/* Remove a keyed reference. */
cw_bool_t
stiln_l_gdict_remove(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt, const
    cw_stiloe_dict_t *a_dict)
{
	cw_bool_t	retval;
	cw_chi_t	*chi;

	mtx_lock(&a_stiln->lock);
	_cw_check_ptr(a_stiln->keyed_refs);

	if ((retval = dch_remove(a_stiln->keyed_refs, (void *)a_dict, NULL,
	    NULL, &chi)) == FALSE)
		/* XXX Need stilag, not a_stilt. */
		_cw_stilt_chi_put(a_stilt, chi);

	/* If there are no more keyed references, delete the hash. */
	if (dch_count(a_stiln->keyed_refs) == 0) {
		dch_delete(a_stiln->keyed_refs);
		a_stiln->keyed_refs = NULL;
	}
	mtx_unlock(&a_stiln->lock);

	return retval;
}

/* Constructor. */
static void
stiln_p_new(cw_stiln_t *a_stiln, cw_stilt_t *a_stilt, const cw_uint32_t *a_name,
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

/* Destructor. */
static void
stiln_p_delete(cw_stiln_t *a_stiln)
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

/* Hash {name string, length}. */
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

/* Compare keys {name string, length}. */
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
stilng_l_new(cw_stilng_t *a_stilng, cw_mem_t *a_mem)
{
	mtx_new(&a_stilng->lock);
	if (dch_new(&a_stilng->hash, a_mem, _CW_STILN_BASE_TABLE,
	    _CW_STILN_BASE_GROW, _CW_STILN_BASE_SHRINK, stiln_p_hash,
	    stiln_p_key_comp) == NULL)
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

	a_stilnt->stilng = a_stilng;

	return FALSE;
	OOM:
	return TRUE;
}

void
stilnt_l_delete(cw_stilnt_t *a_stilnt)
{
	dch_delete(&a_stilnt->hash);
}
