/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: Onyx <Version = onyx>
 *
 ******************************************************************************/

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_dict_l.h"
#include "../include/libonyx/nxo_name_l.h"

static cw_uint32_t
nxo_p_dict_hash(const void *a_key);
static cw_bool_t
nxo_p_dict_key_comp(const void *a_k1, const void *a_k2);

#ifdef CW_THREADS
#define nxoe_p_dict_lock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking)					\
	{								\
	    mtx_lock(&(a_nxoe)->lock);					\
	}								\
    } while (0)
#define nxoe_p_dict_unlock(a_nxoe)					\
    do									\
    {									\
	if ((a_nxoe)->nxoe.locking)					\
	{								\
	    mtx_unlock(&(a_nxoe)->lock);				\
	}								\
    } while (0)
#endif

void
nxo_dict_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_bool_t a_locking,
	     cw_uint32_t a_dict_size)
{
    cw_nxoe_dict_t *dict;
    cw_nxa_t *nxa;

    nxa = nx_nxa_get(a_nx);

    dict = (cw_nxoe_dict_t *) nxa_malloc(nxa, sizeof(cw_nxoe_dict_t));

    nxoe_l_new(&dict->nxoe, NXOT_DICT, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    mtx_new(&dict->lock);
#endif
    dict->dicto = NULL;

    /* Don't create a dict smaller than CW_LIBONYX_DICT_SIZE, since rounding
     * errors for calculating the grow/shrink points can cause severe
     * performance problems if the dict grows significantly.
     *
     * Don't let the table get more than 80% full, or less than 25% full, when
     * shrinking. */
    if (a_dict_size < CW_LIBONYX_DICT_SIZE)
    a_dict_size = CW_LIBONYX_DICT_SIZE;

    dch_new(&dict->hash, (cw_opaque_alloc_t *) nxa_malloc_e,
	    (cw_opaque_dealloc_t *) nxa_free_e, nxa, a_dict_size * 1.25,
	    a_dict_size, a_dict_size / 4, nxo_p_dict_hash, nxo_p_dict_key_comp);

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) dict;
    nxo_p_type_set(a_nxo, NXOT_DICT);

    nxa_l_gc_register(nxa, (cw_nxoe_t *) dict);
}

cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;
    cw_chi_t *chi;

    dict = (cw_nxoe_dict_t *) a_nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    if (dict->nxoe.locking)
    {
	mtx_delete(&dict->lock);
    }
#endif
    while (dch_remove_iterate(&dict->hash, NULL, (void **) &dicto, &chi)
	   == FALSE)
    {
	nxa_free(a_nxa, dicto, sizeof(cw_nxoe_dicto_t));
	nxa_free(a_nxa, chi, sizeof(cw_chi_t));
    }
    dch_delete(&dict->hash);

    nxa_free(a_nxa, dict, sizeof(cw_nxoe_dict_t));

    return FALSE;
}

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_t *retval;
    cw_nxoe_dict_t *dict;

    dict = (cw_nxoe_dict_t *) a_nxoe;

    if (a_reset)
    {
	dict->ref_iter = 0;
	dict->dicto = NULL;
    }

    retval = NULL;
    while (retval == NULL && dict->ref_iter < dch_count(&dict->hash))
    {
	if (dict->dicto == NULL)
	{
	    /* Key. */
	    dch_get_iterate(&dict->hash, NULL, (void **) &dict->dicto);
	    retval = nxo_nxoe_get(&dict->dicto->key);
	}
	else
	{
	    /* Value. */
	    retval = nxo_nxoe_get(&dict->dicto->val);
	    dict->ref_iter++;
	    dict->dicto = NULL;
	}
    }

    return retval;
}

void
nxo_dict_copy(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nx_t *a_nx)
{
    cw_nxoe_dict_t *to, *from;
    cw_uint32_t i, count;
    cw_nxoe_dicto_t *dicto_to, *dicto_from, *dicto_rm;
    cw_chi_t *chi, *chi_rm;
    cw_bool_t removed;
    cw_nxa_t *nxa;

    cw_check_ptr(a_to);
    cw_dassert(a_to->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_to) == NXOT_DICT);
    to = (cw_nxoe_dict_t *) a_to->o.nxoe;
    cw_check_ptr(to);
    cw_dassert(to->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(to->nxoe.type == NXOT_DICT);

    cw_check_ptr(a_from);
    cw_dassert(a_from->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_from) == NXOT_DICT);
    from = (cw_nxoe_dict_t *) a_from->o.nxoe;
    cw_check_ptr(from);
    cw_dassert(from->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(from->nxoe.type == NXOT_DICT);

    nxa = nx_nxa_get(a_nx);

    /* Deep (but not recursive) copy. */
#ifdef CW_THREADS
    nxoe_p_dict_lock(from);
    nxoe_p_dict_lock(to);
#endif
    for (i = 0, count = dch_count(&from->hash); i < count; i++)
    {
	/* Get a dicto. */
	dch_get_iterate(&from->hash, NULL, (void **) &dicto_from);

	/* Allocate and copy. */
	dicto_to = (cw_nxoe_dicto_t *) nxa_malloc(nxa, sizeof(cw_nxoe_dicto_t));
	nxo_no_new(&dicto_to->key);
	nxo_dup(&dicto_to->key, &dicto_from->key);
	nxo_no_new(&dicto_to->val);
	nxo_dup(&dicto_to->val, &dicto_from->val);
	chi = (cw_chi_t *) nxa_malloc(nxa, sizeof(cw_chi_t));

	/* Make sure the key is not defined, then insert. */
#ifdef CW_THREADS
	thd_crit_enter();
#endif
	removed = dch_remove(&to->hash, (void *) &dicto_to->key, NULL,
			     (void **) &dicto_rm, &chi_rm);
	dch_insert(&to->hash, &dicto_to->key, dicto_to, chi);
#ifdef CW_THREADS
	thd_crit_leave();
#endif

	if (removed == FALSE)
	{
	    nxa_free(nxa, dicto_rm, sizeof(cw_nxoe_dicto_t));
	    nxa_free(nxa, chi_rm, sizeof(cw_chi_t));
	}
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(to);
    nxoe_p_dict_unlock(from);
#endif
}

void
nxo_dict_def(cw_nxo_t *a_nxo, cw_nx_t *a_nx, cw_nxo_t *a_key, cw_nxo_t *a_val)
{
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
#endif
    if (dch_search(&dict->hash, (void *) a_key, (void **) &dicto) == FALSE)
    {
	/* a_key is already defined. */
	nxo_dup(&dicto->val, a_val);

	/* If (a_key == &dicto->val), things will break badly.  However, I can't
	 * think of a way that this could possibly happen in real use, so just
	 * assert. */
	cw_assert(a_key != &dicto->val);
    }
    else
    {
	cw_chi_t *chi;
	cw_nxa_t *nxa;

	nxa = nx_nxa_get(a_nx);

	/* Allocate and initialize. */
	dicto = (cw_nxoe_dicto_t *) nxa_malloc(nxa, sizeof(cw_nxoe_dicto_t));
	chi = (cw_chi_t *) nxa_malloc(nxa, sizeof(cw_chi_t));
	nxo_no_new(&dicto->key);
	nxo_dup(&dicto->key, a_key);
	nxo_no_new(&dicto->val);
	nxo_dup(&dicto->val, a_val);

	/* Insert. */
#ifdef CW_THREADS
	thd_crit_enter();
#endif
	dch_insert(&dict->hash, (void *) &dicto->key, (void *) dicto, chi);
#ifdef CW_THREADS
	thd_crit_leave();
#endif
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif
}

void
nxo_dict_undef(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_nxo_t *a_key)
{
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;
    cw_chi_t *chi;
    cw_bool_t error;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
    thd_crit_enter();
#endif
    error = dch_remove(&dict->hash, (void *) a_key, NULL, (void **) &dicto,
		       &chi);
#ifdef CW_THREADS
    thd_crit_leave();
    nxoe_p_dict_unlock(dict);
#endif

    if (error == FALSE)
    {
	cw_nxa_t *nxa;

	nxa = nx_nxa_get(a_nx);

	nxa_free(nxa, dicto, sizeof(cw_nxoe_dicto_t));
	nxa_free(nxa, chi, sizeof(cw_chi_t));
    }
}

cw_bool_t
nxo_dict_lookup(cw_nxo_t *a_nxo, const cw_nxo_t *a_key, cw_nxo_t *r_nxo)
{
    cw_bool_t retval;
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
#endif
    if (dch_search(&dict->hash, (void *) a_key, (void **) &dicto) == FALSE)
    {
	if (r_nxo != NULL)
	{
	    nxo_dup(r_nxo, &dicto->val);
	}
	retval = FALSE;
    }
    else
    {
	retval = TRUE;
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

/* This function is generally unsafe to use, since the return value can
 * disappear due to GC before the pointer is turned into a legitimate reference.
 * However, the GC itself needs to cache pointers to the actual values inside
 * the dict for performance reasons, so it uses this function. */
cw_nxo_t *
nxo_l_dict_lookup(cw_nxo_t *a_nxo, const cw_nxo_t *a_key)
{
    cw_nxo_t *retval;
    cw_nxoe_dict_t *dict;
    cw_nxoe_dicto_t *dicto;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
#endif
    if (dch_search(&dict->hash, (void *) a_key, (void **) &dicto) == FALSE)
    {
	retval = &dicto->val;
    }
    else
    {
	retval = NULL;
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

cw_uint32_t
nxo_dict_count(cw_nxo_t *a_nxo)
{
    cw_uint32_t retval;
    cw_nxoe_dict_t *dict;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
#endif
    retval = dch_count(&dict->hash);
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

cw_bool_t
nxo_dict_iterate(cw_nxo_t *a_nxo, cw_nxo_t *r_nxo)
{
    cw_bool_t retval;
    cw_nxoe_dict_t *dict;
    cw_nxo_t *nxo;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_DICT);

    dict = (cw_nxoe_dict_t *) a_nxo->o.nxoe;

    cw_check_ptr(dict);
    cw_dassert(dict->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(dict->nxoe.type == NXOT_DICT);

#ifdef CW_THREADS
    nxoe_p_dict_lock(dict);
#endif
    retval = dch_get_iterate(&dict->hash, (void **) &nxo, NULL);
    if (retval == FALSE)
    {
	nxo_dup(r_nxo, nxo);
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

/* Hash any nxo, but optimize for name hashing. */
static cw_uint32_t
nxo_p_dict_hash(const void *a_key)
{
    cw_uint32_t retval;
    cw_nxo_t *key = (cw_nxo_t *) a_key;

    cw_check_ptr(key);
    cw_dassert(key->magic == CW_NXO_MAGIC);

    switch (nxo_type_get(key))
    {
	case NXOT_ARRAY:
#ifdef CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
	case NXOT_HOOK:
#ifdef CW_THREADS
	case NXOT_MUTEX:
#endif
	case NXOT_NAME:
	case NXOT_STACK:
	case NXOT_THREAD:
	{
	    retval = ch_direct_hash((void *) key->o.nxoe);
	    break;
	}
	case NXOT_OPERATOR:
	{
	    retval = ch_direct_hash((void *) key->o.operator.f);
	    break;
	}
	case NXOT_STRING:
	{
	    cw_uint8_t *str;
	    cw_uint32_t i, len;

	    str = nxo_string_get(key);
	    len = nxo_string_len_get(key);
	    nxo_string_lock(key);
	    for (i = retval = 0; i < len; i++)
	    {
		retval = retval * 33 + str[i];
	    }
	    nxo_string_unlock(key);
	    break;
	}
	case NXOT_BOOLEAN:
	{
	    retval = (cw_uint32_t) key->o.boolean.val;
	    break;
	}
	case NXOT_INTEGER:
	{
	    retval = (cw_uint32_t) key->o.integer.i;
	    break;
	}
	case NXOT_MARK:
	case NXOT_NULL:
	case NXOT_PMARK:
	{
	    retval = UINT_MAX;
	    break;
	}
	default:
	{
	    cw_not_reached();
	}
    }

    return retval;
}

/* Compare nxo's, but optimize for name comparison. */
static cw_bool_t
nxo_p_dict_key_comp(const void *a_k1, const void *a_k2)
{
    cw_bool_t retval;
    cw_nxo_t *k1 = (cw_nxo_t *) a_k1;
    cw_nxo_t *k2 = (cw_nxo_t *) a_k2;

    cw_check_ptr(k1);
    cw_dassert(k1->magic == CW_NXO_MAGIC);
    cw_check_ptr(k2);
    cw_dassert(k2->magic == CW_NXO_MAGIC);

    if ((nxo_type_get(k1) == NXOT_NAME)
	&& (nxo_type_get(k1) == NXOT_NAME))
    {
	cw_nxoe_name_t *n1, *n2;

	n1 = (cw_nxoe_name_t *) k1->o.nxoe;
	n2 = (cw_nxoe_name_t *) k2->o.nxoe;

	retval = (n1 == n2) ? TRUE : FALSE;
    }
    else if (nxo_type_get(k1) != nxo_type_get(k2))
    {
	retval = FALSE;
    }
    else
    {
	retval = nxo_compare(k1, k2) ? FALSE : TRUE;
    }

    return retval;
}