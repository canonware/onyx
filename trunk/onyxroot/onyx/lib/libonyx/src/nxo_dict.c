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

#define CW_NXO_DICT_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nx_l.h"
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

    dict = (cw_nxoe_dict_t *) nxa_malloc(sizeof(cw_nxoe_dict_t));

    nxoe_l_new(&dict->nxoe, NXOT_DICT, a_locking);
#ifdef CW_THREADS
    if (a_locking)
    mtx_new(&dict->lock);
#endif

    if (a_dict_size < CW_LIBONYX_DICT_SIZE)
    {
	cw_uint32_t i;

	dict->is_hash = FALSE;
	dict->array_iter = 0;
	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    nxo_no_new(&dict->data.array[i].key);
	}
    }
    else
    {
	dict->is_hash = TRUE;

	/* Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking. */
	dch_new(&dict->data.hash, cw_g_nxaa, a_dict_size * 1.25,
		a_dict_size, a_dict_size / 4, nxo_p_dict_hash,
		nxo_p_dict_key_comp);
    }

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) dict;
    nxo_p_type_set(a_nxo, NXOT_DICT);

    nxa_l_gc_register((cw_nxoe_t *) dict);
}

CW_P_INLINE void
nxoe_p_dict_def(cw_nxoe_dict_t *a_dict, cw_nx_t *a_nx, cw_nxo_t *a_key,
		cw_nxo_t *a_val)
{
    cw_nxoe_dicto_t *dicto;

    if (a_dict->is_hash)
    {
	if (dch_search(&a_dict->data.hash, (void *) a_key, (void **) &dicto)
	    == FALSE)
	{
	    /* a_key is already defined. */
	    nxo_dup(&dicto->val, a_val);

	    /* If (a_key == &dicto->val), things will break badly.  However, I
	     * can't think of a way that this could possibly happen in real use,
	     * so just assert. */
	    cw_assert(a_key != &dicto->val);
	}
	else
	{
	    cw_chi_t *chi;

	    /* Allocate and initialize. */
	    dicto = (cw_nxoe_dicto_t *) nxa_malloc(sizeof(cw_nxoe_dicto_t));
	    chi = (cw_chi_t *) nxa_malloc(sizeof(cw_chi_t));
	    nxo_no_new(&dicto->key);
	    nxo_dup(&dicto->key, a_key);
	    nxo_no_new(&dicto->val);
	    nxo_dup(&dicto->val, a_val);

	    /* Insert. */
#ifdef CW_THREADS
	    thd_crit_enter();
#endif
	    dch_insert(&a_dict->data.hash, (void *) &dicto->key, (void *) dicto,
		       chi);
#ifdef CW_THREADS
	    thd_crit_leave();
#endif
	}
    }
    else
    {
	cw_bool_t done = FALSE;
	cw_uint32_t i;

	/* Search for an existing definition.  If one exists, replace the
	 * value.  Otherwise, insert into the array if there is room, or convert
	 * to a hash then insert if there is not room. */

	for (i = 0, dicto = NULL; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&a_dict->data.array[i].key) != NXOT_NO)
	    {
		if (nxo_compare(&a_dict->data.array[i].key, a_key) == 0)
		{
		    nxo_dup(&a_dict->data.array[i].val, a_val);
		    done = TRUE;
		    break;
		}
	    }
	    else if (dicto == NULL)
	    {
		dicto = &a_dict->data.array[i];
	    }
	}

	if (done == FALSE)
	{
	    if (dicto != NULL)
	    {
		nxo_dup(&dicto->key, a_key);
		nxo_no_new(&dicto->val);
		nxo_dup(&dicto->val, a_val);
	    }
	    else
	    {
		cw_nxoe_dicto_t tarray[CW_LIBONYX_DICT_SIZE];
		cw_chi_t *chi;

		memcpy(tarray, &a_dict->data.array, sizeof(tarray));

#ifdef CW_THREADS
		thd_crit_enter();
#endif
		/* Create a dch that initially has twice the capacity of what
		 * can fit in the array.  This has the advantage of avoiding a
		 * rehash when populating it below, but the disadvantage is that
		 * the dict can't ever shrink down below this size.  Oh well;
		 * converting from the array to a dch is a one way process
		 * anyway.
		 *
		 * Don't let the table get more than 80% full, or less than 25%
		 * full, when shrinking. */
		dch_new(&a_dict->data.hash, cw_g_nxaa,
			CW_LIBONYX_DICT_SIZE * 2.5, CW_LIBONYX_DICT_SIZE * 2,
			CW_LIBONYX_DICT_SIZE / 2,
			nxo_p_dict_hash, nxo_p_dict_key_comp);
		for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
		{
		    if (nxo_type_get(&tarray[i].key) != NXOT_NO)
		    {
			/* Allocate and initialize. */
			dicto = (cw_nxoe_dicto_t *)
			    nxa_malloc(sizeof(cw_nxoe_dicto_t));
			chi = (cw_chi_t *) nxa_malloc(sizeof(cw_chi_t));
			nxo_no_new(&dicto->key);
			nxo_dup(&dicto->key, &tarray[i].key);
			nxo_no_new(&dicto->val);
			nxo_dup(&dicto->val, &tarray[i].val);

			/* Insert. */
			dch_insert(&a_dict->data.hash, (void *) &dicto->key,
				   (void *) dicto, chi);
		    }
		}

		/* The conversion to a hash is complete. */
		a_dict->is_hash = TRUE;

		/* Finally, do the insertion. */

		/* Allocate and initialize. */
		dicto = (cw_nxoe_dicto_t *) nxa_malloc(sizeof(cw_nxoe_dicto_t));
		chi = (cw_chi_t *) nxa_malloc(sizeof(cw_chi_t));
		nxo_no_new(&dicto->key);
		nxo_dup(&dicto->key, a_key);
		nxo_no_new(&dicto->val);
		nxo_dup(&dicto->val, a_val);

		/* Insert. */
		dch_insert(&a_dict->data.hash, (void *) &dicto->key,
			   (void *) dicto, chi);

#ifdef CW_THREADS
		thd_crit_leave();
#endif
	    }
	}
    }
}

CW_P_INLINE cw_nxo_t *
nxoe_p_dict_lookup(cw_nxoe_dict_t *a_dict, const cw_nxo_t *a_key)
{
    cw_nxo_t *retval;

    if (a_dict->is_hash)
    {
	cw_nxoe_dicto_t *dicto;

	if (dch_search(&a_dict->data.hash, (void *) a_key, (void **) &dicto)
	    == FALSE)
	{
	    retval = &dicto->val;
	}
	else
	{
	    retval = NULL;
	}
    }
    else
    {
	cw_uint32_t i;

	retval = NULL;
	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&a_dict->data.array[i].key) != NXOT_NO
		&& nxo_compare(&a_dict->data.array[i].key, a_key) == 0)
	    {
		retval = &a_dict->data.array[i].val;
		break;
	    }
	}
    }

    return retval;
}

void
nxo_dict_copy(cw_nxo_t *a_to, cw_nxo_t *a_from, cw_nx_t *a_nx)
{
    cw_nxoe_dict_t *to, *from;
    cw_uint32_t i;
    cw_nxoe_dicto_t *dicto_from;

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

    /* Deep (but not recursive) copy. */
#ifdef CW_THREADS
    nxoe_p_dict_lock(from);
    nxoe_p_dict_lock(to);
#endif
    if (from->is_hash)
    {
	cw_uint32_t count;

	for (i = 0, count = dch_count(&from->data.hash); i < count; i++)
	{
	    /* Get a dicto. */
	    dch_get_iterate(&from->data.hash, NULL, (void **) &dicto_from);

	    nxoe_p_dict_def(to, a_nx, &dicto_from->key, &dicto_from->val);
	}
    }
    else
    {
	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&from->data.array[i].key) != NXOT_NO)
	    {
		dicto_from = &from->data.array[i];
		nxoe_p_dict_def(to, a_nx, &dicto_from->key, &dicto_from->val);
	    }
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
    nxoe_p_dict_def(dict, a_nx, a_key, a_val);
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif
}

void
nxo_dict_undef(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_nxo_t *a_key)
{
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
    if (dict->is_hash)
    {
	cw_nxoe_dicto_t *dicto;
	cw_chi_t *chi;
	cw_bool_t error;

#ifdef CW_THREADS
	thd_crit_enter();
#endif
	error = dch_remove(&dict->data.hash, (void *) a_key, NULL,
			   (void **) &dicto, &chi);
#ifdef CW_THREADS
	thd_crit_leave();
#endif

	if (error == FALSE)
	{
	    nxa_free(dicto, sizeof(cw_nxoe_dicto_t));
	    nxa_free(chi, sizeof(cw_chi_t));
	}
    }
    else
    {
	cw_uint32_t i;

	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&dict->data.array[i].key) != NXOT_NO
		&& nxo_compare(&dict->data.array[i].key, a_key) == 0)
	    {
		nxo_no_new(&dict->data.array[i].key);
		break;
	    }
	}
    }
#ifdef CW_THREADS
	nxoe_p_dict_unlock(dict);
#endif
}

cw_bool_t
nxo_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key, cw_nxo_t *r_nxo)
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
    nxo = nxoe_p_dict_lookup(dict, a_key);
    if (nxo != NULL)
    {
	if (r_nxo != NULL)
	{
	    nxo_dup(r_nxo, nxo);
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
nxo_l_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key)
{
    cw_nxo_t *retval;
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
    retval = nxoe_p_dict_lookup(dict, a_key);
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

cw_uint32_t
nxo_dict_count(const cw_nxo_t *a_nxo)
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
    if (dict->is_hash)
    {
	retval = dch_count(&dict->data.hash);
    }
    else
    {
	cw_uint32_t i;

	for (i = retval = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&dict->data.array[i].key) != NXOT_NO)
	    {
		retval++;
	    }
	}
    }
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
    if (dict->is_hash)
    {
	cw_nxo_t *nxo;

	retval = dch_get_iterate(&dict->data.hash, (void **) &nxo, NULL);
	if (retval == FALSE)
	{
	    nxo_dup(r_nxo, nxo);
	}
    }
    else
    {
	cw_uint32_t i;

	for (retval = TRUE, i = 0;
	     retval == TRUE && i < CW_LIBONYX_DICT_SIZE;
	     i++, dict->array_iter = (dict->array_iter + 1)
		 % CW_LIBONYX_DICT_SIZE)
	{
	    if (nxo_type_get(&dict->data.array[dict->array_iter].key)
		!= NXOT_NO)
	    {
		nxo_dup(r_nxo, &dict->data.array[dict->array_iter].key);
		retval = FALSE;
	    }
	}
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
#ifdef CW_REGEX
	case NXOT_REGEX:
	case NXOT_REGSUB:
#endif
	case NXOT_STACK:
	case NXOT_THREAD:
	{
	    retval = ch_direct_hash((void *) key->o.nxoe);
	    break;
	}
	case NXOT_OPERATOR:
	{
	    retval = ch_direct_hash((void *) key->o.oper.f);
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
#ifdef CW_REAL
	case NXOT_REAL:
	{
	    /* This could be vastly improved. */
	    retval = (cw_uint32_t) key->o.real.r;
	    break;
	}
#endif
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
