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

typedef struct cw_nxoe_name_s cw_nxoe_name_t;

struct cw_nxoe_name_s
{
    cw_nxoe_t nxoe;
    /* name is not required to be NULL-terminated, so we keep track of the
     * length. */
    const cw_uint8_t *str;
    cw_uint32_t len;
};

cw_uint32_t
nxo_l_name_hash(const void *a_key);

cw_bool_t
nxo_l_name_key_comp(const void *a_k1, const void *a_k2);

#ifndef CW_USE_INLINES
cw_bool_t
nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_NAME_C_))
CW_INLINE cw_bool_t
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

CW_INLINE cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_name_t *name;

    name = (cw_nxoe_name_t *) a_nxoe;

    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_NAME_C_)) */
