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
#include "../include/libstil/stila_l.h"
#include "../include/libstil/stilo_l.h"
#include "../include/libstil/stilo_dict_l.h"
#include "../include/libstil/stilo_name_l.h"

static cw_uint32_t stilo_p_dict_hash(const void *a_key);
static cw_bool_t stilo_p_dict_key_comp(const void *a_k1, const void *a_k2);

#define		stiloe_p_dict_lock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_lock(&(a_stiloe)->lock);				\
} while (0)
#define		stiloe_p_dict_unlock(a_stiloe) do {			\
	if ((a_stiloe)->stiloe.locking)					\
		mtx_unlock(&(a_stiloe)->lock);				\
} while (0)

void
stilo_dict_new(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_bool_t a_locking,
    cw_uint32_t a_dict_size)
{
	cw_stiloe_dict_t	*dict;

	dict = (cw_stiloe_dict_t *)_cw_malloc(sizeof(cw_stiloe_dict_t));

	stiloe_l_new(&dict->stiloe, STILOT_DICT, a_locking);
	if (a_locking)
		mtx_new(&dict->lock);
	dict->dicto = NULL;

	/*
	 * Don't create a dict smaller than _LIBSTIL_DICT_SIZE, since rounding
	 * errors for calculating the grow/shrink points can cause severe
	 * performance problems if the dict grows significantly.
	 *
	 * Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking.
	 */
	if (a_dict_size < _LIBSTIL_DICT_SIZE)
		a_dict_size = _LIBSTIL_DICT_SIZE;

	dch_new(&dict->hash, NULL, a_dict_size * 1.25, a_dict_size, a_dict_size
	    / 4, stilo_p_dict_hash, stilo_p_dict_key_comp);

	memset(a_stilo, 0, sizeof(cw_stilo_t));
	a_stilo->o.stiloe = (cw_stiloe_t *)dict;
#ifdef _LIBSTIL_DBG
	a_stilo->magic = _CW_STILO_MAGIC;
#endif
	a_stilo->type = STILOT_DICT;

	stila_l_gc_register(stil_stila_get(a_stil), (cw_stiloe_t *)dict);
}

void
stiloe_l_dict_delete(cw_stiloe_t *a_stiloe, cw_stil_t *a_stil)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;
	cw_chi_t		*chi;

	dict = (cw_stiloe_dict_t *)a_stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	if (dict->stiloe.locking)
		mtx_delete(&dict->lock);
	while (dch_remove_iterate(&dict->hash, NULL, (void **)&dicto, &chi) ==
	    FALSE) {
		stila_dicto_put(stil_stila_get(a_stil), dicto);
		stila_chi_put(stil_stila_get(a_stil), chi);
	}

	_CW_STILOE_FREE(dict);
}

cw_stiloe_t *
stiloe_l_dict_ref_iter(cw_stiloe_t *a_stiloe, cw_bool_t a_reset)
{
	cw_stiloe_t		*retval;
	cw_stiloe_dict_t	*dict;

	dict = (cw_stiloe_dict_t *)a_stiloe;

	if (a_reset) {
		dict->ref_iter = 0;
		dict->dicto = NULL;
	}

	retval = NULL;
	while (retval == NULL && dict->ref_iter < dch_count(&dict->hash)) {
		if (dict->dicto == NULL) {
			/* Key. */
			dch_get_iterate(&dict->hash, NULL, (void
			    **)&dict->dicto);
			retval = stilo_stiloe_get(&dict->dicto->key);
		} else {
			/* Value. */
			retval = stilo_stiloe_get(&dict->dicto->val);
			dict->ref_iter++;
			dict->dicto = NULL;
		}
	}

	return retval;
}

cw_stilte_t
stilo_l_dict_print(cw_stilo_t *a_stilo, cw_stilo_t *a_file, cw_uint32_t
    a_depth)
{
	cw_stilte_t	retval;

	if (a_depth > 0) {
		cw_stilo_t	key, val;
		cw_uint32_t	count, i;

		retval = stilo_file_output(a_file, "<<");
		if (retval)
			goto RETURN;

		for (i = 0, count = stilo_dict_count(a_stilo); i < count; i++) {
			/* Get key and val. */
			stilo_dict_iterate(a_stilo, &key);
			stilo_dict_lookup(a_stilo, &key, &val);

			/* Print key. */
			retval = stilo_print(&key, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;
			retval = stilo_file_output(a_file, " ");
			if (retval)
				goto RETURN;

			/* Print val. */
			retval = stilo_print(&val, a_file, a_depth - 1, FALSE);
			if (retval)
				goto RETURN;
			if (i < count - 1) {
				retval = stilo_file_output(a_file, " ");
				if (retval)
					goto RETURN;
			}
		}
		retval = stilo_file_output(a_file, ">>");
	} else {
		retval = stilo_file_output(a_file, "-dict-");
		if (retval)
			goto RETURN;
	}

	retval = STILTE_NONE;
	RETURN:
	return retval;
}

void
stilo_dict_copy(cw_stilo_t *a_to, cw_stilo_t *a_from, cw_stil_t *a_stil,
    cw_bool_t a_locking)
{
	cw_stiloe_dict_t	*to, *from;
	cw_uint32_t		i, count;
	cw_stiloe_dicto_t	*dicto_to, *dicto_from;
	cw_chi_t		*chi;

	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_STILO_MAGIC);
	_cw_assert(a_to->type == STILOT_DICT);
	_cw_assert(stilo_dict_count(a_to) == 0);

	from = (cw_stiloe_dict_t *)a_from->o.stiloe;

	_cw_check_ptr(from);
	_cw_assert(from->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(from->stiloe.type == STILOT_DICT);

	/* Deep (but not recursive) copy. */
	count = dch_count(&from->hash);
	stilo_dict_new(a_to, a_stil, a_locking, count);
	to = (cw_stiloe_dict_t *)a_to->o.stiloe;

	stiloe_p_dict_lock(from);
	for (i = 0, count = dch_count(&from->hash); i < count; i++) {
		/* Get a dicto. */
		dch_get_iterate(&from->hash, NULL, (void **)&dicto_from);

		/* Allocate and copy. */
		dicto_to = stila_dicto_get(stil_stila_get(a_stil));
		stilo_no_new(&dicto_to->key);
		stilo_dup(&dicto_to->key, &dicto_from->key);
		stilo_no_new(&dicto_to->val);
		stilo_dup(&dicto_to->val, &dicto_from->val);
		chi = stila_chi_get(stil_stila_get(a_stil));

		/* Insert. */
		dch_insert(&to->hash, &dicto_to->key, dicto_to, chi);
	}
	stiloe_p_dict_unlock(from);
}

void
stilo_dict_def(cw_stilo_t *a_stilo, cw_stil_t *a_stil, cw_stilo_t *a_key,
    cw_stilo_t *a_val)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		/* a_key is already defined. */
		stilo_no_new(&dicto->val);
		stilo_dup(&dicto->val, a_val);

		/*
		 * If (a_key == &dicto->val), things will break badly.  However,
		 * I can't think of a way that this could possibly happen in
		 * real use, so just assert.
		 */
		_cw_assert(a_key != &dicto->val);
	} else {
		cw_chi_t	*chi;

		/* Allocate and initialize. */
		dicto = stila_dicto_get(stil_stila_get(a_stil));
		chi = stila_chi_get(stil_stila_get(a_stil));
		stilo_no_new(&dicto->key);
		stilo_dup(&dicto->key, a_key);
		stilo_no_new(&dicto->val);
		stilo_dup(&dicto->val, a_val);

		/* Insert. */
		thd_crit_enter();
		dch_insert(&dict->hash, (void *)&dicto->key, (void *)dicto,
		    chi);
		thd_crit_leave();
	}
	stiloe_p_dict_unlock(dict);
}

void
stilo_dict_undef(cw_stilo_t *a_stilo, cw_stil_t *a_stil, const cw_stilo_t
    *a_key)
{
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;
	cw_chi_t		*chi;
	cw_bool_t		error;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	thd_crit_enter();
	error = dch_remove(&dict->hash, (void *)a_key, NULL, (void **)&dicto,
	    &chi);
	thd_crit_leave();
	stiloe_p_dict_unlock(dict);

	if (error == FALSE) {
		stila_dicto_put(stil_stila_get(a_stil), dicto);
		stila_chi_put(stil_stila_get(a_stil), chi);
	}
}

cw_bool_t
stilo_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key, cw_stilo_t
    *r_stilo)
{
	cw_bool_t		retval;
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		if (r_stilo != NULL)
			stilo_dup(r_stilo, &dicto->val);
		retval = FALSE;
	} else
		retval = TRUE;
	stiloe_p_dict_unlock(dict);

	return retval;
}

/*
 * This function is generally unsafe to use, since the return value can
 * disappear due to GC before the pointer is turned into a legitimate reference.
 * However, the GC itself needs to cache pointers to the actual values inside
 * the dict for performance reasons, so it uses this function.
 */
cw_stilo_t *
stilo_l_dict_lookup(cw_stilo_t *a_stilo, const cw_stilo_t *a_key)
{
	cw_stilo_t		*retval;
	cw_stiloe_dict_t	*dict;
	cw_stiloe_dicto_t	*dicto;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		retval = &dicto->val;
	} else
		retval = NULL;
	stiloe_p_dict_unlock(dict);

	return retval;
}

cw_uint32_t
stilo_dict_count(cw_stilo_t *a_stilo)
{
	cw_uint32_t		retval;
	cw_stiloe_dict_t	*dict;

	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	retval = dch_count(&dict->hash);
	stiloe_p_dict_unlock(dict);

	return retval;
}

void
stilo_dict_iterate(cw_stilo_t *a_stilo, cw_stilo_t *r_stilo)
{
	cw_stiloe_dict_t	*dict;
	cw_stilo_t		*stilo;
	
	_cw_check_ptr(a_stilo);
	_cw_assert(a_stilo->magic == _CW_STILO_MAGIC);
	_cw_assert(a_stilo->type == STILOT_DICT);

	dict = (cw_stiloe_dict_t *)a_stilo->o.stiloe;

	_cw_check_ptr(dict);
	_cw_assert(dict->stiloe.magic == _CW_STILOE_MAGIC);
	_cw_assert(dict->stiloe.type == STILOT_DICT);

	stiloe_p_dict_lock(dict);
	dch_get_iterate(&dict->hash, (void **)&stilo, NULL);
	stilo_dup(r_stilo, stilo);
	stiloe_p_dict_unlock(dict);
}

/* Hash any stilo, but optimize for name hashing. */
static cw_uint32_t
stilo_p_dict_hash(const void *a_key)
{
	cw_uint32_t	retval;
	cw_stilo_t	*key = (cw_stilo_t *)a_key;

	_cw_check_ptr(key);
	_cw_assert(key->magic == _CW_STILO_MAGIC);

	switch (key->type) {
	case STILOT_ARRAY:
	case STILOT_CONDITION:
	case STILOT_DICT:
	case STILOT_FILE:
	case STILOT_HOOK:
	case STILOT_MUTEX:
	case STILOT_NAME:
		retval = ch_direct_hash((void *)key->o.stiloe);
		break;
	case STILOT_OPERATOR:
		retval = ch_direct_hash((void *)key->o.operator.f);
		break;
	case STILOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	i, len;

		str = stilo_string_get(key);
		len = stilo_string_len_get(key);
		stilo_string_lock(key);
		for (i = retval = 0; i < len; i++)
			retval = retval * 33 + str[i];
		stilo_string_unlock(key);
		break;
	}
	case STILOT_BOOLEAN:
		retval = (cw_uint32_t)key->o.boolean.val;
		break;
	case STILOT_INTEGER:
		retval = (cw_uint32_t)key->o.integer.i;
		break;
	case STILOT_MARK:
	case STILOT_NULL:
		retval = UINT_MAX;
		break;
	default:
		_cw_not_reached();
	}

	return retval;
}

/* Compare stilo's, but optimize for name comparison. */
static cw_bool_t
stilo_p_dict_key_comp(const void *a_k1, const void *a_k2)
{
	cw_bool_t	retval;
	cw_stilo_t	*k1 = (cw_stilo_t *)a_k1;
	cw_stilo_t	*k2 = (cw_stilo_t *)a_k2;

	_cw_check_ptr(k1);
	_cw_assert(k1->magic == _CW_STILO_MAGIC);
	_cw_check_ptr(k2);
	_cw_assert(k2->magic == _CW_STILO_MAGIC);

	if ((k1->type == STILOT_NAME) && (k1->type == STILOT_NAME)) {
		cw_stiloe_name_t	*n1, *n2;

		n1 = (cw_stiloe_name_t *)k1->o.stiloe;
		n2 = (cw_stiloe_name_t *)k2->o.stiloe;

		retval = (n1 == n2) ? TRUE : FALSE;
	} else if (k1->type != k2->type)
		retval = FALSE;
	else
		retval = stilo_compare(k1, k2) ? FALSE : TRUE;

	return retval;
}
