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
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_name_l.h"

void
stilo_name_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const cw_uint8_t
    *a_str, cw_uint32_t a_len, cw_bool_t a_is_static)
{
	cw_stiloe_name_t	*name, key;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;

	/* Fake up a key so that we can search the hash tables. */
	key.str = a_str;
	key.len = a_len;

	name_lock = stil_l_name_lock_get(a_stil);
	name_hash = stil_l_name_hash_get(a_stil);

	/*
	 * Look in the global hash for the name.  If the name doesn't exist,
	 * create it.
	 */
	mtx_lock(name_lock);
	thd_crit_enter();
	if (dch_search(name_hash, (void *)&key, (void **)&name)) {
		/*
		 * Not found in the name hash.  Create, initialize, and insert
		 * a new entry.
		 */
		name = (cw_stiloe_name_t *)_cw_malloc(sizeof(cw_stiloe_name_t));

		stiloe_l_new(&name->stiloe, STILOT_NAME, FALSE);
		name->stiloe.name_static = a_is_static;
		name->len = a_len;

		if (a_is_static == FALSE) {
			name->str = _cw_malloc(a_len);
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			memcpy((cw_uint8_t *)name->str, a_str, a_len);
		} else
			name->str = a_str;

		dch_insert(name_hash, (void *)name, (void **)name,
		    stila_chi_get(stil_stila_get(a_stil)));

		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)name;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_NAME;

		stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)name);
	} else {
		memset(a_stilo, 0, sizeof(cw_stilo_t));
		a_stilo->o.stiloe = (cw_stiloe_t *)name;
#ifdef _LIBSTIL_DBG
		a_stilo->magic = _CW_STILO_MAGIC;
#endif
		a_stilo->type = STILOT_NAME;
	}
	thd_crit_leave();
	mtx_unlock(name_lock);
}

void
stiloe_l_name_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_name_t	*name;
	cw_mtx_t		*name_lock;
	cw_dch_t		*name_hash;
	cw_chi_t		*chi;

	name = (cw_stiloe_name_t *)a_stiloe;

	name_lock = stil_l_name_lock_get(a_stil);
	name_hash = stil_l_name_hash_get(a_stil);

	mtx_lock(name_lock);
	/*
	 * Only delete the hash entry if this object hasn't been put back into
	 * use.
	 */
	if (name->stiloe.color != stila_l_white_get(stil_stila_get(a_stil))) {
		/*
		 * Remove from hash table.
		 */
		dch_remove(name_hash, (void *)name, NULL, NULL, &chi);
		stila_chi_put(stil_stila_get(a_stil), chi);

		if (name->stiloe.name_static == FALSE) {
			/*
			 * Cast away the const here; it's one of two places that
			 * the string is allowed to be modified, and this cast
			 * is better than dropping the const altogether.
			 */
			_CW_FREE((cw_uint8_t *)name->str);
		}

		_CW_STILOE_FREE(name);
	} else {
		/* Re-register. */
		a_stiloe->registered = FALSE;
		stila_l_gc_register(stil_stila_get(a_stil), a_stiloe);
	}
	mtx_unlock(name_lock);
}

cw_stiloe_t *
stiloe_l_name_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_name_t	*name;

	name = (cw_stiloe_name_t *)a_stiloe;

	return NULL;
}

cw_stilte_t
stilo_l_name_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t		retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);
	
	if (a_stilo->attrs == STILOA_LITERAL) {
		retval = stilo_file_output(a_file, "/");
		if (retval)
			goto RETURN;
	}

	retval = stilo_file_output_n(a_file, name->len, "[s]",
	    name->str);
	if (retval)
		goto RETURN;

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

/* Hash {name string, length}. */
cw_uint32_t
stilo_l_name_hash(const void *a_key)
{
	cw_uint32_t		retval, i;
	cw_stiloe_name_t	*key = (cw_stiloe_name_t *)a_key;
	const char		*str;

	_cw_check_ptr(a_key);

	for (i = 0, str = key->str, retval = 0; i < key->len;
	    i++, str++)
		retval = retval * 33 + *str;

	return retval;
}

/* Compare keys {name string, length}. */
cw_bool_t
stilo_l_name_key_comp(const void *a_k1, const void *a_k2)
{
	cw_stiloe_name_t	*k1 = (cw_stiloe_name_t *)a_k1;
	cw_stiloe_name_t	*k2 = (cw_stiloe_name_t *)a_k2;
	size_t			len;

	_cw_check_ptr(a_k1);
	_cw_check_ptr(a_k2);

	if (k1->len > k2->len)
		len = k1->len;
	else
		len = k2->len;

	return strncmp((char *)k1->str, (char *)k2->str, len) ? FALSE
	    : TRUE;
}

const cw_uint8_t *
stilo_name_str_get(cw_stilo_t *a_stilo)
{
	const cw_uint8_t	*retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);

	retval = name->str;

	return retval;
}

cw_uint32_t
stilo_name_len_get(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_name_t	*name;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_NAME);

	name = (cw_stiloe_name_t *)a_stilo->o.stiloe;

	_cw_check_ptr(name);
	_cw_assert(name->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(name->stiloe.type == STILOT_NAME);

	retval = name->len;

	return retval;
}
