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

#ifdef _LIBSTIL_DBG
#define _CW_STIL_MAGIC 0xae9678fd
#define _CW_STILN_MAGIC 0xaeb78944
#endif

/* Number of stil_bufc structures to allocate at a time via the pezz code. */
#define _CW_STIL_BUFC_CHUNK_COUNT	 16

/* Number of stack elements per memory chunk. */
#define _CW_STIL_STILSC_COUNT		 16

/*
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with.
 */
#define _CW_STIL_STILN_BASE_TABLE	512
#define _CW_STIL_STILN_BASE_GROW	400
#define _CW_STIL_STILN_BASE_SHRINK	128

/*
 * Size and fullness control of initial root set for global VM.  Global VM is
 * empty to begin with.
 */
#define _CW_STIL_ROOTS_BASE_TABLE	 32
#define _CW_STIL_ROOTS_BASE_GROW	 24
#define _CW_STIL_ROOTS_BASE_SHRINK	  8

static cw_stiln_t	*stil_p_stiln_new(cw_stil_t *a_stil);
static void		stil_p_stiln_delete(cw_stil_t *a_stil, cw_stiln_t
    *a_stiln);
static cw_bool_t	stil_p_stiln_kref(cw_stil_t *a_stil, cw_stiln_t
    *a_stiln, const void *a_key, const void *a_data);
static cw_uint32_t	stilnk_p_hash(const void *a_key);
static cw_bool_t	stilnk_p_key_comp(const void *a_k1, const void *a_k2);

cw_stil_t *
stil_new(cw_stil_t *a_stil)
{
	cw_stil_t	*retval;

	if (a_stil != NULL) {
		retval = a_stil;
		bzero(retval, sizeof(cw_stil_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stil_t *)_cw_malloc(sizeof(cw_stil_t));
		if (retval == NULL)
			goto OOM_1;
		bzero(retval, sizeof(cw_stil_t));
		retval->is_malloced = TRUE;
	}

	if (pezz_new(&retval->stil_bufc_pezz, sizeof(cw_stil_bufc_t),
	    _CW_STIL_BUFC_CHUNK_COUNT) == NULL)
		goto OOM_2;
	if (pezz_new(&retval->chi_pezz, sizeof(cw_chi_t),
	    _CW_STIL_STILN_BASE_GROW / 4) == NULL)
		goto OOM_3;
	if (pezz_new(&retval->stiln_pezz, sizeof(cw_stiln_t),
	    _CW_STIL_STILN_BASE_GROW / 4) == NULL)
		goto OOM_4;
	if (pezz_new(&retval->stilsc_pezz, sizeof(cw_stilsc_t),
	    _CW_STIL_STILSC_COUNT) == NULL)
		goto OOM_5;
	if (dch_new(&retval->stiln_dch, _CW_STIL_STILN_BASE_TABLE,
	    _CW_STIL_STILN_BASE_GROW, _CW_STIL_STILN_BASE_SHRINK, stilnk_p_hash,
	    stilnk_p_key_comp) == NULL)
		goto OOM_6;
	if (dch_new(&retval->roots_dch, _CW_STIL_ROOTS_BASE_TABLE,
	    _CW_STIL_ROOTS_BASE_GROW, _CW_STIL_ROOTS_BASE_SHRINK,
	    ch_hash_direct, ch_key_comp_direct) == NULL)
		goto OOM_7;
	mtx_new(&retval->lock);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STIL_MAGIC;
#endif

	return retval;

	OOM_7:
	dch_delete(&retval->stiln_dch);
	OOM_6:
	pezz_delete(&retval->stilsc_pezz);
	OOM_5:
	pezz_delete(&retval->stiln_pezz);
	OOM_4:
	pezz_delete(&retval->chi_pezz);
	OOM_3:
	pezz_delete(&retval->stil_bufc_pezz);
	OOM_2:
	if (retval->is_malloced)
		_cw_free(retval);
	OOM_1:
	return NULL;
}

void
stil_delete(cw_stil_t *a_stil)
{
	cw_stilnk_t	*key;
	cw_stiln_t	*data;
	cw_chi_t	*chi;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	/* This table should be empty by now. */
#ifdef _LIBSTIL_DBG
	if (dch_count(&a_stil->roots_dch) != 0) {
		_cw_out_put_e("roots_dch has [i] references (should be 0)\n",
		    dch_count(&a_stil->roots_dch));
	}
#endif
	dch_delete(&a_stil->roots_dch);

	while (dch_remove_iterate(&a_stil->stiln_dch, (void **)&key,
	    (void **)&data, &chi) == FALSE) {
		stil_p_stiln_delete(a_stil, data);
		_cw_pezz_put(&a_stil->chi_pezz, chi);
	}
	dch_delete(&a_stil->stiln_dch);
	pezz_delete(&a_stil->stilsc_pezz);
	pezz_delete(&a_stil->stiln_pezz);
	
	pezz_delete(&a_stil->chi_pezz);
	pezz_delete(&a_stil->stil_bufc_pezz);

	mtx_delete(&a_stil->lock);

	if (a_stil->is_malloced)
		_cw_free(a_stil);
#ifdef _LIBSTIL_DBG
	else
		memset(a_stil, 0x5a, sizeof(cw_stil_t));
#endif
}

cw_stil_bufc_t *
stil_get_stil_bufc(cw_stil_t *a_stil)
{
	cw_stil_bufc_t *retval;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);

	retval = (cw_stil_bufc_t *)_cw_pezz_get(&a_stil->stil_bufc_pezz);
	if (retval == NULL)
		goto RETURN;
	bufc_new(&retval->bufc, (cw_opaque_dealloc_t *)pezz_put,
	    &a_stil->stil_bufc_pezz);
	bzero(retval->buffer, sizeof(retval->buffer));
	bufc_set_buffer(&retval->bufc, retval->buffer, _CW_STIL_BUFC_SIZE, TRUE,
	    NULL, NULL);

	RETURN:
	return retval;
}

const cw_stiln_t *
stil_stiln_ref(cw_stil_t *a_stil, const cw_uint8_t *a_name, cw_uint32_t a_len,
    cw_bool_t a_force, cw_bool_t a_is_static, const void *a_key, const void
    *a_data)
{
	cw_stiln_t	*retval;
	cw_stiln_t	search_key, *data;
	cw_uint8_t	*name;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);
	_cw_check_ptr(a_name);
	_cw_assert(a_len > 0);

	/* Fake up a key. */
	stilnk_init(&search_key.key, a_name, a_len);

	mtx_lock(&a_stil->lock);

	/* Find the name. */
	if (dch_search(&a_stil->stiln_dch, &search_key, (void **)&data) ==
	    FALSE) {
		/* Add a keyed reference if necessary. */
		if (a_key != NULL) {
			mtx_lock(&data->lock);
			if (stil_p_stiln_kref(a_stil, data, a_key,
			    a_data)) {
				mtx_unlock(&data->lock);
				goto OOM_1;
			}
			mtx_unlock(&data->lock);
		}
		data->ref_count++;
		retval = data;
	} else if (a_force) {
		/* The name doesn't exist, and the caller wants it created. */
		data = stil_p_stiln_new(a_stil);
		if (data == NULL)
			goto OOM_1;
		if (a_is_static == FALSE) {
			/* Copy the name string. */
			name = _cw_malloc(sizeof(cw_uint8_t) * a_len);
			if (name == NULL)
				goto OOM_2;
			memcpy(name, a_name, a_len);
		} else {
			/*
			 * Cast away the const, though it gets picked right back
			 * up in the call to stilnk_init() below.
			 */
			name = (cw_uint8_t *)a_name;
			data->is_static_name = TRUE;
		}

		/* Initialize the key. */
		stilnk_init(&data->key, a_name, a_len);

		/* Increment the reference count for the caller. */
		data->ref_count++;

		/*
		 * Add a keyed reference if necessary.  We don't need to lock
		 * the stiln, because it hasn't been inserted into the hash
		 * table yet, so no other threads can get to it.
		 */
		if (a_key != NULL) {
			if (stil_p_stiln_kref(a_stil, data, a_key, a_data))
				goto OOM_3;
		}
		/* Finally, insert the stiln into the hash table. */
		if (dch_insert(&a_stil->stiln_dch, (void *)&data->key,
		    (void *)data, (cw_chi_t *)_cw_pezz_get(&a_stil->chi_pezz)))
			goto OOM_4;
		retval = data;
	} else
		retval = NULL;

	mtx_unlock(&a_stil->lock);
	return retval;

	OOM_4:
	if (data->keyed_refs != NULL)
		dch_delete(data->keyed_refs);
	OOM_3:
	if (a_is_static == FALSE)
		_cw_free(name);
	OOM_2:
	stil_p_stiln_delete(a_stil, data);
	OOM_1:
	mtx_unlock(&a_stil->lock);
	return NULL;
}

void
stil_stiln_unref(cw_stil_t *a_stil, const cw_stiln_t *a_stiln, const void
    *a_key)
{
	cw_stiln_t	*stiln = (cw_stiln_t *)a_stiln;
	cw_chi_t	*chi;

	_cw_check_ptr(a_stil);
	_cw_assert(a_stil->magic == _CW_STIL_MAGIC);
	_cw_check_ptr(a_stiln);
	_cw_assert(a_stiln->magic == _CW_STILN_MAGIC);

	mtx_lock(&a_stil->lock);
	mtx_lock(&stiln->lock);

	/* Remove a keyed reference if necessary. */
	if (a_key != NULL) {
		_cw_check_ptr(stiln->keyed_refs);

		if (dch_remove(stiln->keyed_refs, a_key, NULL, NULL, &chi)) {
#ifdef _LIBSTIL_DBG
			_cw_error("Trying to remove a non-existent keyed ref");
#endif
		}
		_cw_pezz_put(&a_stil->chi_pezz, chi);

		if (dch_count(stiln->keyed_refs) == 0) {
			dch_delete(stiln->keyed_refs);
			stiln->keyed_refs = NULL;
		}
	}
	stiln->ref_count--;

	if (stiln->ref_count != 0)
		mtx_unlock(&stiln->lock);
	else {
		mtx_unlock(&stiln->lock);
		dch_remove(&a_stil->stiln_dch, &stiln->key, NULL, NULL, &chi);
		_cw_pezz_put(&a_stil->chi_pezz, chi);
		stil_p_stiln_delete(a_stil, stiln);
	}

	mtx_unlock(&a_stil->lock);
}

const cw_stilnk_t *
stiln_get_stilnk(const cw_stiln_t *a_stiln)
{
	_cw_check_ptr(a_stiln);
	_cw_assert(a_stiln->magic == _CW_STILN_MAGIC);

	return &a_stiln->key;
}

void
stilnk_init(cw_stilnk_t *a_stilnk, const cw_uint8_t *a_name,
    cw_uint32_t a_len)
{
	_cw_check_ptr(a_stilnk);

	a_stilnk->name = a_name;
	a_stilnk->len = a_len;
}

void
stilnk_copy(cw_stilnk_t *a_to, const cw_stilnk_t *a_from)
{
	_cw_check_ptr(a_to);
	_cw_check_ptr(a_from);

	a_to->name = a_from->name;
	a_to->len = a_from->len;
}

const cw_uint8_t *
stilnk_get_val(cw_stilnk_t *a_stilnk)
{
	_cw_check_ptr(a_stilnk);

	return a_stilnk->name;
}

cw_uint32_t
stilnk_get_len(cw_stilnk_t *a_stilnk)
{
	_cw_check_ptr(a_stilnk);

	return a_stilnk->len;
}

static cw_stiln_t *
stil_p_stiln_new(cw_stil_t *a_stil)
{
	cw_stiln_t *retval;

	retval = (cw_stiln_t *)_cw_pezz_get(&a_stil->stiln_pezz);
	if (retval == NULL)
		goto RETURN;
	bzero(retval, sizeof(cw_stiln_t));

	mtx_new(&retval->lock);
#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILN_MAGIC;
#endif

	RETURN:
	return retval;
}

static void
stil_p_stiln_delete(cw_stil_t *a_stil, cw_stiln_t *a_stiln)
{
	_cw_check_ptr(a_stiln);
	_cw_assert(a_stiln->magic == _CW_STILN_MAGIC);
#ifdef _LIBSTIL_DBG
	if (a_stiln->is_static_name == FALSE) {
		_cw_out_put_e("Non-static name \"");
		_cw_out_put_n(a_stiln->key.len, "[s]", a_stiln->key.name);
		_cw_out_put("\" still exists with [i] reference[s]\n",
		    a_stiln->ref_count, (a_stiln->ref_count == 1) ? "" : "s");
	}
	if (a_stiln->keyed_refs != NULL) {
		cw_uint32_t	i;
		void		*key;

		_cw_out_put_e("Name \"");
		_cw_out_put_n(a_stiln->key.len, "[s]", a_stiln->key.name);
		_cw_out_put("\" still exists with [i] keyed reference[s]:",
		    dch_count(&a_stil->stiln_dch),
		    (dch_count(&a_stil->stiln_dch) == 1) ? "" : "s");
		for (i = 0; i < dch_count(&a_stil->stiln_dch); i++) {
			dch_get_iterate(&a_stil->stiln_dch, &key, NULL);
			_cw_out_put(" 0x[p]", key);
		}
		_cw_out_put("\n");
	}
#endif

	mtx_delete(&a_stiln->lock);
	_cw_pezz_put(&a_stil->stiln_pezz, a_stiln);
}

static cw_bool_t
stil_p_stiln_kref(cw_stil_t *a_stil, cw_stiln_t *a_stiln, const void *a_key,
    const void *a_data)
{
	cw_bool_t	retval, is_new_dch = FALSE;

	/*
	 * Assume that a_stiln is locked, or that no other threads can get to
	 * it.
	 */

	if (a_stiln->keyed_refs == NULL) {
		is_new_dch = TRUE;

		/* XXX Magic numbers here. */
		a_stiln->keyed_refs = dch_new(NULL, 4, 3, 1, ch_hash_direct,
		    ch_key_comp_direct);
		if (a_stiln->keyed_refs == NULL) {
			retval = TRUE;
			goto RETURN;
		}
	}
	if (dch_insert(a_stiln->keyed_refs, a_key, a_data,
	    (cw_chi_t *)_cw_pezz_get(&a_stil->chi_pezz))) {
		if (is_new_dch) {
			dch_delete(a_stiln->keyed_refs);
			a_stiln->keyed_refs = NULL;
		}
		retval = TRUE;
		goto RETURN;
	}
	retval = FALSE;

	RETURN:
	return retval;
}

static cw_uint32_t
stilnk_p_hash(const void *a_key)
{
	cw_uint32_t	retval, i;
	cw_stilnk_t	*key = (cw_stilnk_t *)a_key;
	const char	*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->name, retval = 0; i < key->len; i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

static cw_bool_t
stilnk_p_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stilnk_t	*k1 = (cw_stilnk_t *)a_k1;
	cw_stilnk_t	*k2 = (cw_stilnk_t *)a_k2;
	size_t		len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->name, (char *)k2->name, len) ? FALSE : TRUE;
}
