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
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_name_l.h"

static cw_uint32_t nxo_p_dict_hash(const void *a_key);
static cw_bool_t nxo_p_dict_key_comp(const void *a_k1, const void *a_k2);

#define		nxoe_p_dict_lock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking)					\
		mtx_lock(&(a_nxoe)->lock);				\
} while (0)
#define		nxoe_p_dict_unlock(a_nxoe) do {			\
	if ((a_nxoe)->nxoe.locking)					\
		mtx_unlock(&(a_nxoe)->lock);				\
} while (0)

void
nxo_dict_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking, cw_uint32_t
    a_dict_size)
{
	cw_nxoe_dict_t	*dict;

	dict = (cw_nxoe_dict_t *)_cw_malloc(sizeof(cw_nxoe_dict_t));

	nxoe_l_new(&dict->nxoe, NXOT_DICT, a_locking);
	if (a_locking)
		mtx_new(&dict->lock);
	dict->dicto = NULL;

	/*
	 * Don't create a dict smaller than _CW_LIBONYX_DICT_SIZE, since
	 * rounding errors for calculating the grow/shrink points can cause
	 * severe performance problems if the dict grows significantly.
	 *
	 * Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking.
	 */
	if (a_dict_size < _CW_LIBONYX_DICT_SIZE)
		a_dict_size = _CW_LIBONYX_DICT_SIZE;

	dch_new(&dict->hash, NULL, a_dict_size * 1.25, a_dict_size, a_dict_size
	    / 4, nxo_p_dict_hash, nxo_p_dict_key_comp);

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *)dict;
	nxo_p_type_set(a_nxo, NXOT_DICT);

	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *)dict);
}

void
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nx_t *a_nx)
{
	cw_nxoe_dict_t	*dict;
	cw_nxoe_dicto_t	*dicto;
	cw_chi_t	*chi;

	dict = (cw_nxoe_dict_t *)a_nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	if (dict->nxoe.locking)
		mtx_delete(&dict->lock);
	while (dch_remove_iterate(&dict->hash, NULL, (void **)&dicto, &chi) ==
	    FALSE) {
		nxa_l_dicto_put(nx_nxa_get(a_nx), dicto);
		nxa_l_chi_put(nx_nxa_get(a_nx), chi);
	}

	_CW_NXOE_FREE(dict);
}

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
	cw_nxoe_t	*retval;
	cw_nxoe_dict_t	*dict;

	dict = (cw_nxoe_dict_t *)a_nxoe;

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
			retval = nxo_nxoe_get(&dict->dicto->key);
		} else {
			/* Value. */
			retval = nxo_nxoe_get(&dict->dicto->val);
			dict->ref_iter++;
			dict->dicto = NULL;
		}
	}

	return retval;
}

void
nxo_l_dict_print(cw_nxo_t *a_thread)
{
	cw_nxo_t		*ostack, *depth, *dict, *stdout_nxo;
	cw_nxo_threade_t	error;

	ostack = nxo_thread_ostack_get(a_thread);
	NXO_STACK_GET(depth, ostack, a_thread);
	NXO_STACK_DOWN_GET(dict, ostack, a_thread, depth);
	if (nxo_type_get(depth) != NXOT_INTEGER || nxo_type_get(dict) !=
	    NXOT_DICT) {
		nxo_thread_error(a_thread, NXO_THREADE_TYPECHECK);
		return;
	}
	stdout_nxo = nxo_thread_stdout_get(a_thread);

	if (nxo_integer_get(depth) > 0) {
		cw_nxo_t	*tstack, *key, *val, *nxo;
		cw_uint32_t	count, i;

		tstack = nxo_thread_tstack_get(a_thread);
		key = nxo_stack_push(tstack);
		val = nxo_stack_push(tstack);

		error = nxo_file_output(stdout_nxo, "<");
		if (error) {
			nxo_stack_npop(tstack, 2);
			nxo_thread_error(a_thread, error);
			return;
		}

		for (i = 0, count = nxo_dict_count(dict); i < count; i++) {
			/* Get key and val. */
			nxo_dict_iterate(dict, key);
			nxo_dict_lookup(dict, key, val);

			/* Print key. */
			nxo = nxo_stack_push(ostack);
			nxo_dup(nxo, key);
			nxo = nxo_stack_push(ostack);
			nxo_integer_new(nxo, nxo_integer_get(depth) - 1);
			_cw_onyx_code(a_thread,
			    "1 index type sprintdict exch get eval");
			
			error = nxo_file_output(stdout_nxo, " ");
			if (error) {
				nxo_thread_error(a_thread, error);
				return;
			}

			/* Print val. */
			nxo = nxo_stack_push(ostack);
			nxo_dup(nxo, val);
			nxo = nxo_stack_push(ostack);
			nxo_integer_new(nxo, nxo_integer_get(depth) - 1);
			_cw_onyx_code(a_thread,
			    "1 index type sprintdict exch get eval");

			if (i < count - 1) {
				error = nxo_file_output(stdout_nxo, " ");
				if (error) {
					nxo_stack_npop(tstack, 2);
					nxo_thread_error(a_thread, error);
					return;
				}
			}
		}
		error = nxo_file_output(stdout_nxo, ">");
		if (error) {
			nxo_stack_npop(tstack, 2);
			nxo_thread_error(a_thread, error);
			return;
		}

		nxo_stack_npop(tstack, 2);
	} else {
		error = nxo_file_output(stdout_nxo, "-dict-");
		if (error) {
			nxo_thread_error(a_thread, error);
			return;
		}
	}

	nxo_stack_npop(ostack, 2);
}

void
nxo_dict_copy(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nx_t *a_nx)
{
	cw_nxoe_dict_t	*to, *from;
	cw_uint32_t	i, count;
	cw_nxoe_dicto_t	*dicto_to, *dicto_from, *dicto_rm;
	cw_chi_t	*chi, *chi_rm;
	cw_bool_t	removed;

	_cw_check_ptr(a_to);
	_cw_assert(a_to->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_to) == NXOT_DICT);
	to = (cw_nxoe_dict_t *)a_to->o.nxoe;
	_cw_check_ptr(to);
	_cw_assert(to->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(to->nxoe.type == NXOT_DICT);

	_cw_check_ptr(a_from);
	_cw_assert(a_from->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_from) == NXOT_DICT);
	from = (cw_nxoe_dict_t *)a_from->o.nxoe;
	_cw_check_ptr(from);
	_cw_assert(from->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(from->nxoe.type == NXOT_DICT);

	/* Deep (but not recursive) copy. */
	nxoe_p_dict_lock(from);
	nxoe_p_dict_lock(to);
	for (i = 0, count = dch_count(&from->hash); i < count; i++) {
		/* Get a dicto. */
		dch_get_iterate(&from->hash, NULL, (void **)&dicto_from);

		/* Allocate and copy. */
		dicto_to = nxa_l_dicto_get(nx_nxa_get(a_nx));
		nxo_no_new(&dicto_to->key);
		nxo_dup(&dicto_to->key, &dicto_from->key);
		nxo_no_new(&dicto_to->val);
		nxo_dup(&dicto_to->val, &dicto_from->val);
		chi = nxa_l_chi_get(nx_nxa_get(a_nx));

		/* Make sure the key is not defined, then insert. */
		thd_crit_enter();
		removed = dch_remove(&to->hash, (void *)&dicto_to->key, NULL,
		    (void **)&dicto_rm, &chi_rm);
		dch_insert(&to->hash, &dicto_to->key, dicto_to, chi);
		thd_crit_leave();

		if (removed == FALSE) {
			cw_nxa_t	*nxa;

			nxa = nx_nxa_get(a_nx);

			nxa_l_dicto_put(nxa, dicto_rm);
			nxa_l_chi_put(nxa, chi_rm);
		}
	}
	nxoe_p_dict_unlock(to);
	nxoe_p_dict_unlock(from);
}

void
nxo_dict_def(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_nxo_t *a_key, cw_nxo_t *a_val)
{
	cw_nxoe_dict_t	*dict;
	cw_nxoe_dicto_t	*dicto;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		/* a_key is already defined. */
		nxo_dup(&dicto->val, a_val);

		/*
		 * If (a_key == &dicto->val), things will break badly.  However,
		 * I can't think of a way that this could possibly happen in
		 * real use, so just assert.
		 */
		_cw_assert(a_key != &dicto->val);
	} else {
		cw_chi_t	*chi;

		/* Allocate and initialize. */
		dicto = nxa_l_dicto_get(nx_nxa_get(a_nx));
		chi = nxa_l_chi_get(nx_nxa_get(a_nx));
		nxo_no_new(&dicto->key);
		nxo_dup(&dicto->key, a_key);
		nxo_no_new(&dicto->val);
		nxo_dup(&dicto->val, a_val);

		/* Insert. */
		thd_crit_enter();
		dch_insert(&dict->hash, (void *)&dicto->key, (void *)dicto,
		    chi);
		thd_crit_leave();
	}
	nxoe_p_dict_unlock(dict);
}

void
nxo_dict_undef(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_nxo_t *a_key)
{
	cw_nxoe_dict_t	*dict;
	cw_nxoe_dicto_t	*dicto;
	cw_chi_t	*chi;
	cw_bool_t	error;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	thd_crit_enter();
	error = dch_remove(&dict->hash, (void *)a_key, NULL, (void **)&dicto,
	    &chi);
	thd_crit_leave();
	nxoe_p_dict_unlock(dict);

	if (error == FALSE) {
		cw_nxa_t	*nxa;

		nxa = nx_nxa_get(a_nx);

		nxa_l_dicto_put(nxa, dicto);
		nxa_l_chi_put(nxa, chi);
	}
}

cw_bool_t
nxo_dict_lookup(cw_nxo_t *a_nxo, const cw_nxo_t *a_key, cw_nxo_t *r_nxo)
{
	cw_bool_t	retval;
	cw_nxoe_dict_t	*dict;
	cw_nxoe_dicto_t	*dicto;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		if (r_nxo != NULL)
			nxo_dup(r_nxo, &dicto->val);
		retval = FALSE;
	} else
		retval = TRUE;
	nxoe_p_dict_unlock(dict);

	return retval;
}

/*
 * This function is generally unsafe to use, since the return value can
 * disappear due to GC before the pointer is turned into a legitimate reference.
 * However, the GC itself needs to cache pointers to the actual values inside
 * the dict for performance reasons, so it uses this function.
 */
cw_nxo_t *
nxo_l_dict_lookup(cw_nxo_t *a_nxo, const cw_nxo_t *a_key)
{
	cw_nxo_t	*retval;
	cw_nxoe_dict_t	*dict;
	cw_nxoe_dicto_t	*dicto;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	if (dch_search(&dict->hash, (void *)a_key, (void **)&dicto) == FALSE) {
		retval = &dicto->val;
	} else
		retval = NULL;
	nxoe_p_dict_unlock(dict);

	return retval;
}

cw_uint32_t
nxo_dict_count(cw_nxo_t *a_nxo)
{
	cw_uint32_t	retval;
	cw_nxoe_dict_t	*dict;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	retval = dch_count(&dict->hash);
	nxoe_p_dict_unlock(dict);

	return retval;
}

cw_bool_t
nxo_dict_iterate(cw_nxo_t *a_nxo, cw_nxo_t *r_nxo)
{
	cw_bool_t	retval;
	cw_nxoe_dict_t	*dict;
	cw_nxo_t	*nxo;

	_cw_check_ptr(a_nxo);
	_cw_assert(a_nxo->magic == _CW_NXO_MAGIC);
	_cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

	dict = (cw_nxoe_dict_t *)a_nxo->o.nxoe;

	_cw_check_ptr(dict);
	_cw_assert(dict->nxoe.magic == _CW_NXOE_MAGIC);
	_cw_assert(dict->nxoe.type == NXOT_DICT);

	nxoe_p_dict_lock(dict);
	retval = dch_get_iterate(&dict->hash, (void **)&nxo, NULL);
	if (retval == FALSE)
		nxo_dup(r_nxo, nxo);
	nxoe_p_dict_unlock(dict);

	return retval;
}

/* Hash any nxo, but optimize for name hashing. */
static cw_uint32_t
nxo_p_dict_hash(const void *a_key)
{
	cw_uint32_t	retval;
	cw_nxo_t	*key = (cw_nxo_t *)a_key;

	_cw_check_ptr(key);
	_cw_assert(key->magic == _CW_NXO_MAGIC);

	switch (nxo_type_get(key)) {
	case NXOT_ARRAY:
	case NXOT_CONDITION:
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
	case NXOT_MUTEX:
	case NXOT_NAME:
	case NXOT_STACK:
	case NXOT_THREAD:
		retval = ch_direct_hash((void *)key->o.nxoe);
		break;
	case NXOT_OPERATOR:
		retval = ch_direct_hash((void *)key->o.operator.f);
		break;
	case NXOT_STRING: {
		cw_uint8_t	*str;
		cw_uint32_t	i, len;

		str = nxo_string_get(key);
		len = nxo_string_len_get(key);
		nxo_string_lock(key);
		for (i = retval = 0; i < len; i++)
			retval = retval * 33 + str[i];
		nxo_string_unlock(key);
		break;
	}
	case NXOT_BOOLEAN:
		retval = (cw_uint32_t)key->o.boolean.val;
		break;
	case NXOT_INTEGER:
		retval = (cw_uint32_t)key->o.integer.i;
		break;
	case NXOT_MARK:
	case NXOT_NULL:
	case NXOT_PMARK:
		retval = UINT_MAX;
		break;
	default:
		_cw_not_reached();
	}

	return retval;
}

/* Compare nxo's, but optimize for name comparison. */
static cw_bool_t
nxo_p_dict_key_comp(const void *a_k1, const void *a_k2)
{
	cw_bool_t	retval;
	cw_nxo_t	*k1 = (cw_nxo_t *)a_k1;
	cw_nxo_t	*k2 = (cw_nxo_t *)a_k2;

	_cw_check_ptr(k1);
	_cw_assert(k1->magic == _CW_NXO_MAGIC);
	_cw_check_ptr(k2);
	_cw_assert(k2->magic == _CW_NXO_MAGIC);

	if ((nxo_type_get(k1) == NXOT_NAME) && (nxo_type_get(k1) ==
	    NXOT_NAME)) {
		cw_nxoe_name_t	*n1, *n2;

		n1 = (cw_nxoe_name_t *)k1->o.nxoe;
		n2 = (cw_nxoe_name_t *)k2->o.nxoe;

		retval = (n1 == n2) ? TRUE : FALSE;
	} else if (nxo_type_get(k1) != nxo_type_get(k2))
		retval = FALSE;
	else
		retval = nxo_compare(k1, k2) ? FALSE : TRUE;

	return retval;
}
