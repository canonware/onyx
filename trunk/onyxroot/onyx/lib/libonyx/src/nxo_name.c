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

#define CW_NXO_NAME_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nx_l.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_name_l.h"

/* Global variables. */
#ifdef CW_THREADS
cw_mtx_t cw_g_name_lock;
#endif
/* Hash of names (key: {name, len}, value: (nxoe_name *)).  This hash table
 * keeps track of *all* name "values" in the virtual machine.  When a name
 * object is created, it actually adds a reference to a nxoe_name in this hash
 * and uses a pointer to that nxoe_name as a unique key. */
cw_dch_t cw_g_name_hash;
/* List of names, corresponding to the entries in cw_g_name_hash. */
cw_name_list_t cw_g_name_list;

void
nxo_name_l_init(void)
{
    /* Initialize the global name cache. */
    dch_new(&cw_g_name_hash, cw_g_nxaa,
	    CW_LIBONYX_NAME_HASH, CW_LIBONYX_NAME_HASH / 4 * 3,
	    CW_LIBONYX_NAME_HASH / 4, nxo_l_name_hash,
	    nxo_l_name_key_comp);
#ifdef CW_THREADS
    mtx_new(&cw_g_name_lock);
#endif
    ql_new(&cw_g_name_list);
}

void
nxo_name_l_shutdown(void)
{
    dch_delete(&cw_g_name_hash);
#ifdef CW_THREADS
    mtx_delete(&cw_g_name_lock);
#endif
}

void
nxo_name_new(cw_nxo_t *a_nxo, const char *a_str, uint32_t a_len,
	     bool a_is_static)
{
    cw_nxoe_name_t *name, key;

    /* Fake up a key so that we can search the hash tables. */
    key.str = a_str;
    key.len = a_len;

    /* Look in the global hash for the name.  If the name doesn't exist, create
     * it. */
#ifdef CW_THREADS
    mtx_lock(&cw_g_name_lock);
#endif
    if (dch_search(&cw_g_name_hash, (void *) &key, (void **) &name))
    {
	/* Not found in the name hash.  Create, initialize, and insert a new
	 * entry. */
	name = (cw_nxoe_name_t *) nxa_malloc(sizeof(cw_nxoe_name_t));

	nxoe_l_new(&name->nxoe, NXOT_NAME, false);
	name->nxoe.name_static = a_is_static;
	name->len = a_len;

	if (a_is_static == false)
	{
	    name->str = nxa_malloc(a_len);
	    /* Cast away the const here; it's one of two places that the string
	     * is allowed to be modified, and this cast is better than dropping
	     * the const altogether. */
	    memcpy((char *) name->str, a_str, a_len);
	}
	else
	{
	    name->str = a_str;
	}
	ql_elm_new(name, link);

	dch_insert(&cw_g_name_hash, (void *) name, (void **) name,
		   &name->chi);
	ql_head_insert(&cw_g_name_list, name, link);

	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *) name;
	nxo_p_type_set(a_nxo, NXOT_NAME);
	nxa_l_gc_register((cw_nxoe_t *) name);
    }
    else
    {
	nxo_no_new(a_nxo);
	a_nxo->o.nxoe = (cw_nxoe_t *) name;
	nxo_p_type_set(a_nxo, NXOT_NAME);
    }

#ifdef CW_THREADS
    mtx_unlock(&cw_g_name_lock);
#endif
}

/* Hash {name string, length}. */
uint32_t
nxo_l_name_hash(const void *a_key)
{
    uint32_t retval, i;
    cw_nxoe_name_t *key = (cw_nxoe_name_t *) a_key;
    const unsigned char *str;

    cw_check_ptr(a_key);

    for (i = 0, retval = 5381, str = key->str; i < key->len; i++, str++)
    {
	retval = ((retval << 5) + retval) + *str;
    }

    return retval;
}

/* Compare keys {name string, length}. */
bool
nxo_l_name_key_comp(const void *a_k1, const void *a_k2)
{
    bool retval;
    cw_nxoe_name_t *k1 = (cw_nxoe_name_t *) a_k1;
    cw_nxoe_name_t *k2 = (cw_nxoe_name_t *) a_k2;

    cw_check_ptr(a_k1);
    cw_check_ptr(a_k2);

    if (k1->len == k2->len)
    {
	if (strncmp((char *) k1->str, (char *) k2->str, k1->len) == 0)
	{
	    retval = true;
	}
	else
	{
	    retval = false;
	}
    }
    else
    {
	retval = false;

    }

    return retval;
}

const char *
nxo_name_str_get(const cw_nxo_t *a_nxo)
{
    const char *retval;
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

uint32_t
nxo_name_len_get(const cw_nxo_t *a_nxo)
{
    uint32_t retval;
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
