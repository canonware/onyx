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

static uint32_t
nxo_p_dict_hash(const void *a_key);
static bool
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
nxo_dict_new(cw_nxo_t *a_nxo, bool a_locking, uint32_t a_dict_size)
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
	uint32_t i;

	dict->is_hash = false;
	dict->array_iter = 0;
	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    nxo_no_new(&dict->data.a.array[i].key);
	}
    }
    else
    {
	dict->is_hash = true;

	/* Don't let the table get more than 80% full, or less than 25% full,
	 * when shrinking. */
	dch_new(&dict->data.h.hash, cw_g_nxaa, a_dict_size * 1.25,
		a_dict_size, a_dict_size / 4, nxo_p_dict_hash,
		nxo_p_dict_key_comp);
	ql_new(&dict->data.h.list);
    }

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) dict;
    nxo_p_type_set(a_nxo, NXOT_DICT);

    nxa_l_gc_register((cw_nxoe_t *) dict);
}

CW_P_INLINE void
nxoe_p_dict_def(cw_nxoe_dict_t *a_dict, cw_nxo_t *a_key, cw_nxo_t *a_val)
{
    if (a_dict->is_hash)
    {
	cw_nxoe_dicth_t *dicth;

	if (dch_search(&a_dict->data.h.hash, (void *) a_key, (void **) &dicth)
	    == false)
	{
	    /* a_key is already defined. */
	    nxo_dup(&dicth->val, a_val);

	    /* If (a_key == &dicth->val), things will break badly.  However, I
	     * can't think of a way that this could possibly happen in real use,
	     * so just assert. */
	    cw_assert(a_key != &dicth->val);
	}
	else
	{
	    /* Allocate and initialize. */
	    dicth = (cw_nxoe_dicth_t *) nxa_malloc(sizeof(cw_nxoe_dicth_t));
	    ql_elm_new(dicth, link);
	    nxo_no_new(&dicth->key);
	    nxo_no_new(&dicth->val);
	    nxo_dup(&dicth->key, a_key);
	    nxo_dup(&dicth->val, a_val);

	    /* This insertion is GC-safe because the order of pointer
	     * assignments is done such that the list can always be traversed
	     * in forward order, which is what reference iteration does.
	     *
	     * Insertion into the list must happen before insertion into the
	     * dch, since dch_count() is used to limit reference iteration, but
	     * the list is used to actually iterate. */
	    mb_write(); /* Make sure item is initialized before insertion. */
	    ql_tail_insert(&a_dict->data.h.list, dicth, link);
	    mb_write(); /* Make sure list is updated before dch. */

	    /* Insert. */
	    dch_insert(&a_dict->data.h.hash, (void *) &dicth->key,
		       (void *) dicth, &dicth->chi);
	}
    }
    else
    {
	cw_nxoe_dicta_t *dicta;
	bool done = false;
	uint32_t key_hash, i;

	/* Search for an existing definition.  If one exists, replace the
	 * value.  Otherwise, insert into the array if there is room, or convert
	 * to a hash then insert if there is not room. */

	key_hash = nxo_p_dict_hash(a_key);

	for (i = 0, dicta = NULL; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&a_dict->data.a.array[i].key) != NXOT_NO)
	    {
		if (nxo_p_dict_hash(&a_dict->data.a.array[i].key) == key_hash
		    && nxo_p_dict_key_comp(&a_dict->data.a.array[i].key, a_key))
		{
		    nxo_dup(&a_dict->data.a.array[i].val, a_val);
		    done = true;
		    break;
		}
	    }
	    else if (dicta == NULL)
	    {
		dicta = &a_dict->data.a.array[i];
	    }
	}

	if (done == false)
	{
	    if (dicta != NULL)
	    {
		/* Take care to set val to a known state before setting
		 * key, since the GC may look at val after key is set, but
		 * before we get a chance to set the final val. */
		nxo_no_new(&dicta->val);
		nxo_dup(&dicta->key, a_key);
		nxo_dup(&dicta->val, a_val);
	    }
	    else
	    {
		cw_nxoe_dicta_t tarray[CW_LIBONYX_DICT_SIZE];
		cw_nxoe_dicth_t *dicth;

		memcpy(tarray, &a_dict->data.a.array, sizeof(tarray));

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
		dch_new(&a_dict->data.h.hash, cw_g_nxaa,
			CW_LIBONYX_DICT_SIZE * 2.5, CW_LIBONYX_DICT_SIZE * 2,
			CW_LIBONYX_DICT_SIZE / 2,
			nxo_p_dict_hash, nxo_p_dict_key_comp);
		ql_new(&a_dict->data.h.list);
		for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
		{
		    if (nxo_type_get(&tarray[i].key) != NXOT_NO)
		    {
			/* Allocate and initialize. */
			dicth = (cw_nxoe_dicth_t *)
			    nxa_malloc(sizeof(cw_nxoe_dicth_t));
			ql_elm_new(dicth, link);
			nxo_no_new(&dicth->key);
			nxo_no_new(&dicth->val);
			nxo_dup(&dicth->key, &tarray[i].key);
			nxo_dup(&dicth->val, &tarray[i].val);

			/* Insert. */
			dch_insert(&a_dict->data.h.hash, (void *) &dicth->key,
				   (void *) dicth, &dicth->chi);
			ql_tail_insert(&a_dict->data.h.list, dicth, link);
		    }
		}

		/* The conversion to a hash is complete. */
		a_dict->is_hash = true;

		/* Finally, do the insertion. */

		/* Allocate and initialize. */
		dicth = (cw_nxoe_dicth_t *) nxa_malloc(sizeof(cw_nxoe_dicth_t));
		ql_elm_new(dicth, link);
		nxo_no_new(&dicth->key);
		nxo_no_new(&dicth->val);
		nxo_dup(&dicth->key, a_key);
		nxo_dup(&dicth->val, a_val);

		/* Insert. */
		dch_insert(&a_dict->data.h.hash, (void *) &dicth->key,
			   (void *) dicth, &dicth->chi);
		ql_tail_insert(&a_dict->data.h.list, dicth, link);

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
	cw_nxoe_dicth_t *dicth;

	if (dch_search(&a_dict->data.h.hash, (void *) a_key, (void **) &dicth)
	    == false)
	{
	    retval = &dicth->val;
	}
	else
	{
	    retval = NULL;
	}
    }
    else
    {
	uint32_t key_hash, i;

	key_hash = nxo_p_dict_hash(a_key);

	retval = NULL;
	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&a_dict->data.a.array[i].key) != NXOT_NO
		&& nxo_p_dict_hash(&a_dict->data.a.array[i].key) == key_hash
		&& nxo_p_dict_key_comp(&a_dict->data.a.array[i].key, a_key))
	    {
		retval = &a_dict->data.a.array[i].val;
		break;
	    }
	}
    }

    return retval;
}

void
nxo_dict_copy(cw_nxo_t *a_to, cw_nxo_t *a_from)
{
    cw_nxoe_dict_t *to, *from;

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
	cw_nxoe_dicth_t *dicth_from;

	for (dicth_from = ql_first(&from->data.h.list);
	     dicth_from != NULL;
	     dicth_from = ql_next(&from->data.h.list, dicth_from, link))
	{
	    nxoe_p_dict_def(to, &dicth_from->key, &dicth_from->val);
	}
    }
    else
    {
	cw_nxoe_dicta_t *dicta_from;
	uint32_t i;

	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&from->data.a.array[i].key) != NXOT_NO)
	    {
		dicta_from = &from->data.a.array[i];
		nxoe_p_dict_def(to, &dicta_from->key, &dicta_from->val);
	    }
	}

    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(to);
    nxoe_p_dict_unlock(from);
#endif
}

void
nxo_dict_def(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t *a_val)
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
    nxoe_p_dict_def(dict, a_key, a_val);
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif
}

void
nxo_dict_undef(cw_nxo_t *a_nxo, const cw_nxo_t *a_key)
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
	cw_nxoe_dicth_t *dicth;
	bool error;

	error = dch_remove(&dict->data.h.hash, (void *) a_key, NULL,
			   (void **) &dicth, NULL);

	if (error == false)
	{
	    /* This removal is GC-safe because the order of pointer assignments
	     * is done such that the list can always be traversed in forward
	     * order, which is what reference iteration does. */
	    ql_remove(&dict->data.h.list, dicth, link);
	    mb_write();
	    nxa_free(dicth, sizeof(cw_nxoe_dicth_t));
	}
    }
    else
    {
	uint32_t key_hash, i;

	key_hash = nxo_p_dict_hash(a_key);

	for (i = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&dict->data.a.array[i].key) != NXOT_NO
		&& nxo_p_dict_hash(&dict->data.a.array[i].key) == key_hash
		&& nxo_p_dict_key_comp(&dict->data.a.array[i].key, a_key))
	    {
		nxo_no_new(&dict->data.a.array[i].key);
		break;
	    }
	}
    }
#ifdef CW_THREADS
	nxoe_p_dict_unlock(dict);
#endif
}

bool
nxo_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key, cw_nxo_t *r_nxo)
{
    bool retval;
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
	retval = false;
    }
    else
    {
	retval = true;
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

uint32_t
nxo_dict_count(const cw_nxo_t *a_nxo)
{
    uint32_t retval;
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
	retval = dch_count(&dict->data.h.hash);
    }
    else
    {
	uint32_t i;

	for (i = retval = 0; i < CW_LIBONYX_DICT_SIZE; i++)
	{
	    if (nxo_type_get(&dict->data.a.array[i].key) != NXOT_NO)
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

bool
nxo_dict_iterate(cw_nxo_t *a_nxo, cw_nxo_t *r_nxo)
{
    bool retval;
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
	cw_nxoe_dicth_t *dicth;

	dicth = ql_first(&dict->data.h.list);
	if (dicth != NULL)
	{
	    ql_first(&dict->data.h.list) = qr_next(dicth, link);
	    nxo_dup(r_nxo, &dicth->key);
	    retval = false;
	}
	else
	{
	    retval = true;
	}
    }
    else
    {
	uint32_t i;

	for (retval = true, i = 0;
	     retval == true && i < CW_LIBONYX_DICT_SIZE;
	     i++, dict->array_iter = (dict->array_iter + 1)
		 % CW_LIBONYX_DICT_SIZE)
	{
	    if (nxo_type_get(&dict->data.a.array[dict->array_iter].key)
		!= NXOT_NO)
	    {
		nxo_dup(r_nxo, &dict->data.a.array[dict->array_iter].key);
		retval = false;
	    }
	}
    }
#ifdef CW_THREADS
    nxoe_p_dict_unlock(dict);
#endif

    return retval;
}

/* Hash any nxo, but optimize for name hashing. */
static uint32_t
nxo_p_dict_hash(const void *a_key)
{
    uint32_t retval;
    cw_nxo_t *key = (cw_nxo_t *) a_key;

    cw_check_ptr(key);
    cw_dassert(key->magic == CW_NXO_MAGIC);

    switch (nxo_type_get(key))
    {
	case NXOT_ARRAY:
#ifdef CW_OOP
	case NXOT_CLASS:
#endif
#ifdef CW_THREADS
	case NXOT_CONDITION:
#endif
	case NXOT_DICT:
	case NXOT_FILE:
#ifdef CW_HANDLE
	case NXOT_HANDLE:
#endif
#ifdef CW_OOP
	case NXOT_INSTANCE:
#endif
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
	    unsigned char *str;
	    uint32_t i, len;

	    str = nxo_string_get(key);
	    len = nxo_string_len_get(key);
	    nxo_string_lock(key);
	    for (i = 0, retval = 5381; i < len; i++, str++)
	    {
		retval = ((retval << 5) + retval) + *str;
	    }
	    nxo_string_unlock(key);
	    break;
	}
	case NXOT_BOOLEAN:
	{
	    retval = (uint32_t) key->o.boolean.val;
	    break;
	}
	case NXOT_INTEGER:
	{
	    retval = (uint32_t) key->o.integer.i;
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
	    retval = (uint32_t) key->o.real.r;
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
static bool
nxo_p_dict_key_comp(const void *a_k1, const void *a_k2)
{
    bool retval;
    cw_nxo_t *k1 = (cw_nxo_t *) a_k1;
    cw_nxo_t *k2 = (cw_nxo_t *) a_k2;

    cw_check_ptr(k1);
    cw_dassert(k1->magic == CW_NXO_MAGIC);
    cw_check_ptr(k2);
    cw_dassert(k2->magic == CW_NXO_MAGIC);

    if ((nxo_type_get(k1) == NXOT_NAME) && (nxo_type_get(k1) == NXOT_NAME))
    {
	cw_nxoe_name_t *n1, *n2;

	n1 = (cw_nxoe_name_t *) k1->o.nxoe;
	n2 = (cw_nxoe_name_t *) k2->o.nxoe;

	retval = (n1 == n2) ? true : false;
    }
    else if (nxo_type_get(k1) != nxo_type_get(k2))
    {
	retval = false;
    }
    else
    {
	retval = nxo_compare(k1, k2) ? false : true;
    }

    return retval;
}
