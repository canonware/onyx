/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ****************************************************************************
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
#define _CW_STIL_BUFC_CHUNK_COUNT 16

/*
 * Size and fullness control of initial name cache hash table.  We know for sure
 * that there will be about 175 names referenced by systemdict and threaddict to
 * begin with.
 */
#define _CW_STIL_STILN_BASE_TABLE  512
#define _CW_STIL_STILN_BASE_GROW   400
#define _CW_STIL_STILN_BASE_SHRINK 128

/*
 * Size and fullness control of initial root set for global VM.  Global VM is
 * empty to begin with.
 */
#define _CW_STIL_ROOTS_BASE_TABLE  32
#define _CW_STIL_ROOTS_BASE_GROW   24
#define _CW_STIL_ROOTS_BASE_SHRINK  8

cw_stiln_t * stil_p_stiln_new(cw_stil_t *a_stil);
void	stil_p_stiln_delete(cw_stil_t *a_stil, cw_stiln_t *a_stiln);
cw_bool_t stil_p_stiln_kref(cw_stil_t *a_stil, cw_stiln_t *a_stiln, const void
    *a_key, const void *a_data);
static cw_uint32_t stilnk_p_hash(const void *a_key);
static cw_bool_t stilnk_p_key_comp(const void *a_k1, const void *a_k2);

cw_stil_t *
stil_new(cw_stil_t *a_stil)
{
	cw_stil_t *retval;

	if (NULL != a_stil) {
		retval = a_stil;
		bzero(retval, sizeof(cw_stil_t));
		retval->is_malloced = FALSE;
	} else {
		retval = (cw_stil_t *)_cw_malloc(sizeof(cw_stil_t));
		if (NULL == retval)
			goto OOM_1;
		bzero(retval, sizeof(cw_stil_t));
		retval->is_malloced = TRUE;
	}

	if (NULL == pezz_new(&retval->stil_bufc_pezz, sizeof(cw_stil_bufc_t),
	    _CW_STIL_BUFC_CHUNK_COUNT))
		goto OOM_2;
	if (NULL == pezz_new(&retval->chi_pezz, sizeof(cw_chi_t),
	    _CW_STIL_STILN_BASE_GROW / 4))
		goto OOM_3;
	if (NULL == pezz_new(&retval->stiln_pezz, sizeof(cw_stiln_t),
	    _CW_STIL_STILN_BASE_GROW / 4))
		goto OOM_4;
	if (NULL == dch_new(&retval->stiln_dch, _CW_STIL_STILN_BASE_TABLE,
	    _CW_STIL_STILN_BASE_GROW, _CW_STIL_STILN_BASE_SHRINK,
	    &retval->chi_pezz, stilnk_p_hash, stilnk_p_key_comp))
		goto OOM_5;
	if (NULL == dch_new(&retval->roots_dch, _CW_STIL_ROOTS_BASE_TABLE,
	    _CW_STIL_ROOTS_BASE_GROW, _CW_STIL_ROOTS_BASE_SHRINK,
	    &retval->chi_pezz, ch_hash_direct, ch_key_comp_direct))
		goto OOM_6;
	mtx_new(&retval->lock);

#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STIL_MAGIC;
#endif

	return retval;

OOM_6:
	dch_delete(&retval->stiln_dch);
OOM_5:
	pezz_delete(&retval->stiln_pezz);
OOM_4:
	pezz_delete(&retval->chi_pezz);
OOM_3:
	pezz_delete(&retval->stil_bufc_pezz);
OOM_2:
	if (TRUE == retval->is_malloced)
		_cw_free(retval);
OOM_1:
	return NULL;
}

void
stil_delete(cw_stil_t *a_stil)
{
	cw_stilnk_t *key;
	cw_stiln_t *data;

	_cw_check_ptr(a_stil);
	_cw_assert(_CW_STIL_MAGIC == a_stil->magic);

	/* This table should be empty by now. */
#ifdef _LIBSTIL_DBG
	if (0 != dch_count(&a_stil->roots_dch)) {
		_cw_out_put_e("roots_dch has [i] references (should be 0)\n",
		    dch_count(&a_stil->roots_dch));
	}
#endif
	dch_delete(&a_stil->roots_dch);

	while (FALSE == dch_remove_iterate(&a_stil->stiln_dch, (void **)&key,
	    (void **)&data)) {
		stil_p_stiln_delete(a_stil, data);
	}
	dch_delete(&a_stil->stiln_dch);
	pezz_delete(&a_stil->stiln_pezz);

	pezz_delete(&a_stil->stil_bufc_pezz);

	mtx_delete(&a_stil->lock);

	if (TRUE == a_stil->is_malloced)
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
	_cw_assert(_CW_STIL_MAGIC == a_stil->magic);

	retval = (cw_stil_bufc_t *) _cw_pezz_get(&a_stil->stil_bufc_pezz);
	if (NULL == retval)
		goto RETURN;
	bufc_new(&retval->bufc, pezz_put, &a_stil->stil_bufc_pezz);
	bzero(retval->buffer, sizeof(retval->buffer));
	bufc_set_buffer(&retval->bufc, retval->buffer, _CW_STIL_BUFC_SIZE, TRUE,
	    NULL, NULL);

RETURN:
	return retval;
}

const cw_stiln_t *
stil_stiln_ref(cw_stil_t *a_stil, const cw_uint8_t *a_name, cw_uint32_t a_len,
    cw_bool_t a_force, cw_bool_t a_is_static, const void *a_key,
    const void *a_data)
{
	cw_stiln_t *retval;
	cw_stiln_t search_key, *data;
	cw_uint8_t *name;

	_cw_check_ptr(a_stil);
	_cw_assert(_CW_STIL_MAGIC == a_stil->magic);
	_cw_check_ptr(a_name);
	_cw_assert(0 < a_len);

	/* Fake up a key. */
	stilnk_init(&search_key.key, a_name, a_len);

	mtx_lock(&a_stil->lock);

	/* Find the name. */
	if (FALSE == dch_search(&a_stil->stiln_dch, &search_key, (void
	    **)&data)) {
		/* Add a keyed reference if necessary. */
		if (NULL != a_key) {
			mtx_lock(&data->lock);
			if (TRUE == stil_p_stiln_kref(a_stil, data, a_key,
			    a_data)) {
				mtx_unlock(&data->lock);
				goto OOM_1;
			}
			mtx_unlock(&data->lock);
		}
		data->ref_count++;
		retval = data;
	} else if (TRUE == a_force) {
		/* The name doesn't exist, and the caller wants it created. */
		data = stil_p_stiln_new(a_stil);
		if (NULL == data)
			goto OOM_1;
		if (FALSE == a_is_static) {
			/* Copy the name string. */
			name = _cw_malloc(sizeof(cw_uint8_t) * a_len);
			if (NULL == name)
				goto OOM_2;
			memcpy(name, a_name, a_len);
		} else {
			/*
			 * Cast away the const, though it gets picked right
			 * back up in the call to stilnk_init() below.
			 */
			name = (cw_uint8_t *)a_name;
			data->is_static_name = TRUE;
		}

		/* Initialize the key. */
		stilnk_init(&data->key, a_name, a_len);

		/* Increment the reference count for the caller. */
		data->ref_count++;

		/*
		 * Add a keyed reference if necessary.  We don't need to
		 * lock the stiln, because it hasn't been inserted into the
		 * hash table yet, so no other threads can get to it.
		 */
		if (NULL != a_key) {
			if (TRUE == stil_p_stiln_kref(a_stil, data, a_key,
			a_data))
				goto OOM_3;
		}
		/* Finally, insert the stiln into the hash table. */
		if (TRUE == dch_insert(&a_stil->stiln_dch, (void *)&data->key,
		    (void *)data))
			goto OOM_4;
		retval = data;
	} else
		retval = NULL;

	mtx_unlock(&a_stil->lock);
	return retval;

OOM_4:
	if (NULL != data->keyed_refs)
		dch_delete(data->keyed_refs);
OOM_3:
	if (FALSE == a_is_static)
		_cw_free(name);
OOM_2:
	stil_p_stiln_delete(a_stil, data);
OOM_1:
	mtx_unlock(&a_stil->lock);
	return NULL;
}

void
stil_stiln_unref(cw_stil_t *a_stil, const cw_stiln_t *a_stiln,
    const void *a_key)
{
	cw_stiln_t *stiln = (cw_stiln_t *)a_stiln;

	_cw_check_ptr(a_stil);
	_cw_assert(_CW_STIL_MAGIC == a_stil->magic);
	_cw_check_ptr(a_stiln);
	_cw_assert(_CW_STILN_MAGIC == a_stiln->magic);

	mtx_lock(&a_stil->lock);
	mtx_lock(&stiln->lock);

	/* Remove a keyed reference if necessary. */
	if (NULL != a_key) {
		_cw_check_ptr(stiln->keyed_refs);

		if (TRUE == dch_remove(stiln->keyed_refs, a_key, NULL, NULL)) {
#ifdef _LIBSTIL_DBG
			_cw_error("Trying to remove a non-existent keyed ref");
#endif
		}
		if (0 == dch_count(stiln->keyed_refs)) {
			dch_delete(stiln->keyed_refs);
			stiln->keyed_refs = NULL;
		}
	}
	stiln->ref_count--;

	if (0 != stiln->ref_count)
		mtx_unlock(&stiln->lock);
	else {
		mtx_unlock(&stiln->lock);
		dch_remove(&a_stil->stiln_dch, &stiln->key, NULL, NULL);
		stil_p_stiln_delete(a_stil, stiln);
	}

	mtx_unlock(&a_stil->lock);
}

const cw_stilnk_t *
stiln_get_stilnk(const cw_stiln_t *a_stiln)
{
	_cw_check_ptr(a_stiln);
	_cw_assert(_CW_STILN_MAGIC == a_stiln->magic);

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

cw_stiln_t *
stil_p_stiln_new(cw_stil_t *a_stil)
{
	cw_stiln_t *retval;

	retval = _cw_pezz_get(&a_stil->stiln_pezz);
	if (NULL == retval)
		goto RETURN;
	bzero(retval, sizeof(cw_stiln_t));

	mtx_new(&retval->lock);
#ifdef _LIBSTIL_DBG
	retval->magic = _CW_STILN_MAGIC;
#endif

RETURN:
	return retval;
}

void
stil_p_stiln_delete(cw_stil_t *a_stil, cw_stiln_t *a_stiln)
{
	_cw_check_ptr(a_stiln);
	_cw_assert(_CW_STILN_MAGIC == a_stiln->magic);
#ifdef _LIBSTIL_DBG
	if (FALSE == a_stiln->is_static_name) {
		_cw_out_put_e("Non-static name \"");
		_cw_out_put_n(a_stiln->key.len, "[s]", a_stiln->key.name);
		_cw_out_put("\" still exists with [i] reference[s]\n",
		    a_stiln->ref_count, (1 == a_stiln->ref_count) ? "" : "s");
	}
	if (NULL != a_stiln->keyed_refs) {
		cw_uint32_t i;
		void   *key;

		_cw_out_put_e("Name \"");
		_cw_out_put_n(a_stiln->key.len, "[s]", a_stiln->key.name);
		_cw_out_put("\" still exists with [i] keyed reference[s]:",
		    dch_count(&a_stil->stiln_dch),
		    (1 == dch_count(&a_stil->stiln_dch)) ? "" : "s");
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

cw_bool_t
stil_p_stiln_kref(cw_stil_t *a_stil, cw_stiln_t *a_stiln, const void *a_key,
    const void *a_data)
{
	cw_bool_t retval, is_new_dch = FALSE;

	/*
	 * Assume that a_stiln is locked, or that no other threads can get
	 * to it.
	 */

	if (NULL == a_stiln->keyed_refs) {
		is_new_dch = TRUE;

		/* XXX Magic numbers here. */
		a_stiln->keyed_refs = dch_new(NULL, 4, 3, 1, &a_stil->chi_pezz,
		    ch_hash_direct, ch_key_comp_direct);
		if (NULL == a_stiln->keyed_refs) {
			retval = TRUE;
			goto RETURN;
		}
	}
	if (TRUE == dch_insert(a_stiln->keyed_refs, a_key, a_data)) {
		if (TRUE == is_new_dch) {
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
	cw_uint32_t retval, i;
	cw_stilnk_t *key = (cw_stilnk_t *)a_key;
	const char *str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->name, retval = 0; i < key->len; i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

static cw_bool_t
stilnk_p_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stilnk_t *k1 = (cw_stilnk_t *)a_k1;
	cw_stilnk_t *k2 = (cw_stilnk_t *)a_k2;
	size_t  len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->name, (char *)k2->name, len) ? FALSE : TRUE;
}
