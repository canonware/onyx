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

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_name_l.h"

void
nxo_name_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_str,
    cw_uint32_t a_len, cw_bool_t a_is_static)
{
	cw_nxoe_name_t		*name, key;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;

	/* Fake up a key so that we can search the hash tables. */
	key.str = a_str;
	key.len = a_len;

	name_lock = nx_l_name_lock_get(a_nx);
	name_hash = nx_l_name_hash_get(a_nx);

	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(name_lock);
	thd_crit_enter();
	if (dch_search(name_hash, (void *)&key, (void **)&name)) {
		cw_nxa_t	*nxa;

		/*
		 * Not found in the name hash.  Create, initialize, and insert
		 * a new entry.
		 */
		nxa = nx_nxa_get(a_nx);
		name = (cw_nxoe_name_t *)nxa_malloc(nxa,
		    sizeof(cw_nxoe_name_t));

		nxoe_l_new(&name->nxoe, NXOT_NAME, FALSE);
		name->nxoe.name_static = a_is_static;
		name->len = a_len;

		if (a_is_static == FALSE) {
			name->str = nxa_malloc(nxa, a_len);
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			memcpy((cw_uint8_t *)name->str, a_str, a_len);
		} else
			name->str = a_str;

		dch_insert(name_hash, (void *)name, (void **)name,
		    nxa_l_chi_get(nx_nxa_get(a_nx)));

		nxo_no_new(a_nxo);
		a_nxo->o.nxoe = (cw_nxoe_t *)name;
		nxo_p_type_set(a_nxo, NXOT_NAME);

		nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)name);
	} else {
		nxo_no_new(a_nxo);
		a_nxo->o.nxoe = (cw_nxoe_t *)name;
		nxo_p_type_set(a_nxo, NXOT_NAME);
	}
	thd_crit_leave();
	mtx_unlock(name_lock);
}

void
nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_name_t	*name;
	cw_mtx_t	*name_lock;
	cw_dch_t	*name_hash;
	cw_chi_t	*chi;

	name = (cw_nxoe_name_t *)a_nxoe;

	name_lock = nx_l_name_lock_get(a_nx);
	name_hash = nx_l_name_hash_get(a_nx);

	mtx_lock(name_lock);
	/*
	 * Only delete the hash entry if this object hasn't been put back into
	 * use.
	 */
	if (name->nxoe.color != nxa_l_white_get(nx_nxa_get(a_nx))) {
		/*
		 * Remove from hash table.
		 */
		dch_remove(name_hash, (void *)name, NULL, NULL, &chi);
		nxa_l_chi_put(nx_nxa_get(a_nx), chi);

		if (name->nxoe.name_static == FALSE) {
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			_CW_FREE((cw_uint8_t *)name->str);
		}

		_CW_NXOE_FREE(name);
	} else {
		/* Re-register. */
		a_nxoe->registered = FALSE;
		nxa_l_gc_register(nx_nxa_get(a_nx), a_nxoe);
	}
	mtx_unlock(name_lock);
}

cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_name_t	*name;

	name = (cw_nxoe_name_t *)a_nxoe;

	return NULL;
}

/* Hash {name string, length}. */
cw_uint32_t
nxo_l_name_hash(const void *a_key)
{
	cw_uint32_t	retval, i;
	cw_nxoe_name_t	*key = (cw_nxoe_name_t *)a_key;
	const char	*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->str, retval = 0; i < key->len; i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

/* Compare keys {name string, length}. */
cw_bool_t
nxo_l_name_key_comp(const void *a_k1, const void *a_k2)
{
	cw_nxoe_name_t	*k1 = (cw_nxoe_name_t *)a_k1;
	cw_nxoe_name_t	*k2 = (cw_nxoe_name_t *)a_k2;
	size_t		len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->str, (char *)k2->str, len) ? FALSE : TRUE;
}

const cw_uint8_t *
nxo_name_str_get(cw_nxo_t *a_nxo)
{
	const cw_uint8_t	*retval;
	cw_nxoe_name_t		*name;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_NAME);

	name = (cw_nxoe_name_t *)a_nxo->o.nxoe;

	_cw_check_ptr(name);
	_cw_dassert(name->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(name->nxoe.type == NXOT_NAME);

	retval = name->str;

	return retval;
}

cw_uint32_t
nxo_name_len_get(cw_nxo_t *a_nxo)
{
	cw_uint32_t	retval;
	cw_nxoe_name_t	*name;

	_cw_check_ptr(a_nxo);
	_cw_dassert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_NAME);

	name = (cw_nxoe_name_t *)a_nxo->o.nxoe;

	_cw_check_ptr(name);
	_cw_dassert(name->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(name->nxoe.type == NXOT_NAME);

	retval = name->len;

	return retval;
}
