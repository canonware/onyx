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
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_name_l.h"

void
nxo_name_new(cw_nxo_t *a_nxo, cw_nx_t *a_nx, const cw_uint8_t *a_str,
	     cw_uint32_t a_len, cw_bool_t a_is_static)
{
    cw_nxoe_name_t *name, key;
#ifdef CW_THREADS
    cw_mtx_t *name_lock;
#endif
    cw_dch_t *name_hash;
    cw_nxa_t *nxa;
    cw_bool_t do_register;

    /* Fake up a key so that we can search the hash tables. */
    key.str = a_str;
    key.len = a_len;

#ifdef CW_THREADS
    name_lock = nx_l_name_lock_get(a_nx);
#endif
    name_hash = nx_l_name_hash_get(a_nx);

    /* Look in the global hash for the name.  If the name doesn't exist, create
     * it. */
#ifdef CW_THREADS
    mtx_lock(name_lock);
    thd_crit_enter();
#endif
    if (dch_search(name_hash, (void *) &key, (void **) &name))
    {
	/* Not found in the name hash.  Create, initialize, and insert a new
	 * entry. */
	nxa = nx_nxa_get(a_nx);
	name = (cw_nxoe_name_t *) nxa_malloc(nxa, sizeof(cw_nxoe_name_t));

	nxoe_l_new(&name->nxoe, NXOT_NAME, FALSE);
	name->nxoe.name_static = a_is_static;
	name->len = a_len;

	if (a_is_static == FALSE)
	{
	    name->str = nxa_malloc(nxa, a_len);
	    /* Cast away the const here; it's one of two places that the string
	     * is allowed to be modified, and this cast is better than dropping
	     * the const altogether. */
	    memcpy((cw_uint8_t *) name->str, a_str, a_len);
	}
	else
	{
	    name->str = a_str;
	}

	dch_insert(name_hash, (void *) name, (void **) name,
		   (cw_chi_t *) nxa_malloc(nx_nxa_get(a_nx), sizeof(cw_chi_t)));

	do_register = TRUE;
    }
    else
    {
	do_register = FALSE;
    }
    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) name;
    nxo_p_type_set(a_nxo, NXOT_NAME);
#ifdef CW_THREADS
    thd_crit_leave();
#endif

    /* Registration must be done outside the critical region to avoid
     * deadlock. */
    if (do_register)
    {
	nxa_l_gc_register(nx_nxa_get(a_nx), (cw_nxoe_t *) name);
    }
    else
    {
	nxa_l_gc_reregister(nx_nxa_get(a_nx), (cw_nxoe_t *) name);
    }

#ifdef CW_THREADS
    mtx_unlock(name_lock);
#endif
}

cw_bool_t
nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter)
{
    cw_nxoe_name_t *name;
#ifdef CW_THREADS
    cw_mtx_t *name_lock;
#endif
    cw_dch_t *name_hash;
    cw_chi_t *chi;
    cw_nx_t *nx;

    name = (cw_nxoe_name_t *) a_nxoe;

    nx = nxa_nx_get(a_nxa);

#ifdef CW_THREADS
    name_lock = nx_l_name_lock_get(nx);
#endif
    name_hash = nx_l_name_hash_get(nx);

#ifdef CW_THREADS
    mtx_lock(name_lock);
#endif
    /* Only delete the hash entry if this object hasn't been put back into
     * use. */
    if (name->nxoe.color != nxa_l_white_get(a_nxa))
    {
	/* Remove from hash table. */
	dch_remove(name_hash, (void *) name, NULL, NULL, &chi);
	nxa_free(a_nxa, chi, sizeof(cw_chi_t));

	if (name->nxoe.name_static == FALSE)
	{
	    /* Cast away the const here; it's one of two places that the string
	     * is allowed to be modified, and this cast is better than dropping
	     * the const altogether. */
	    nxa_free(a_nxa, (cw_uint8_t *) name->str, name->len);
	}

	nxa_free(a_nxa, name, sizeof(cw_nxoe_name_t));
    }
    else
    {
	/* Re-register. */
	a_nxoe->registered = FALSE;
	nxa_l_gc_register(a_nxa, a_nxoe);
    }
#ifdef CW_THREADS
    mtx_unlock(name_lock);
#endif

    return FALSE;
}

cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_name_t *name;

    name = (cw_nxoe_name_t *) a_nxoe;

    return NULL;
}

/* Hash {name string, length}. */
cw_uint32_t
nxo_l_name_hash(const void *a_key)
{
    cw_uint32_t retval, i;
    cw_nxoe_name_t *key = (cw_nxoe_name_t *) a_key;
    const cw_uint8_t *str;

    cw_check_ptr(a_key);

    for (i = 0, retval = 5381, str = key->str; i < key->len; i++, str++)
    {
	retval = ((retval << 5) + retval) + *str;
    }

    return retval;
}

/* Compare keys {name string, length}. */
cw_bool_t
nxo_l_name_key_comp(const void *a_k1, const void *a_k2)
{
    cw_bool_t retval;
    cw_nxoe_name_t *k1 = (cw_nxoe_name_t *) a_k1;
    cw_nxoe_name_t *k2 = (cw_nxoe_name_t *) a_k2;

    cw_check_ptr(a_k1);
    cw_check_ptr(a_k2);

    if (k1->len == k2->len)
    {
	if (strncmp((char *) k1->str, (char *) k2->str, k1->len) == 0)
	{
	    retval = TRUE;
	}
	else
	{
	    retval = FALSE;
	}
    }
    else
    {
	retval = FALSE;

    }

    return retval;
}

const cw_uint8_t *
nxo_name_str_get(const cw_nxo_t *a_nxo)
{
    const cw_uint8_t *retval;
    cw_nxoe_name_t *name;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_NAME);

    name = (cw_nxoe_name_t *) a_nxo->o.nxoe;

    cw_check_ptr(name);
    cw_dassert(name->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(name->nxoe.type == NXOT_NAME);

    retval = name->str;

    return retval;
}

cw_uint32_t
nxo_name_len_get(const cw_nxo_t *a_nxo)
{
    cw_uint32_t retval;
    cw_nxoe_name_t *name;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_NAME);

    name = (cw_nxoe_name_t *) a_nxo->o.nxoe;

    cw_check_ptr(name);
    cw_dassert(name->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(name->nxoe.type == NXOT_NAME);

    retval = name->len;

    return retval;
}
