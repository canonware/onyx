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

    cw_chi_t chi;
    ql_elm(cw_nxoe_name_t) link;
};

/* Global variables. */
#ifdef CW_THREADS
extern cw_mtx_t cw_g_name_lock;
#endif
extern cw_dch_t cw_g_name_hash;
typedef ql_head(cw_nxoe_name_t) cw_name_list_t;
extern cw_name_list_t cw_g_name_list;

cw_uint32_t
nxo_l_name_hash(const void *a_key);

cw_bool_t
nxo_l_name_key_comp(const void *a_k1, const void *a_k2);

#ifndef CW_USE_INLINES
#ifdef CW_THREADS
cw_mtx_t *
nxo_l_name_lock_get(void);
#endif

void
nxo_l_name_list_prune(cw_bool_t a_white);

cw_bool_t
nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
#endif

#if (defined(CW_USE_INLINES) || defined(CW_NXO_NAME_C_))
#ifdef CW_THREADS
CW_INLINE cw_mtx_t *
nxo_l_name_lock_get(void)
{
    cw_assert(cw_g_nxa_initialized);

    return &cw_g_name_lock;
}
#endif

CW_INLINE void
nxo_l_name_list_prune(cw_bool_t a_white)
{
    cw_nxoe_name_t *name;

    for (name = ql_last(&cw_g_name_list, link);
	 name != NULL && nxoe_l_color_get(&name->nxoe) == a_white;
	 name = ql_last(&cw_g_name_list, link))
    {
	dch_chi_remove(&cw_g_name_hash, &name->chi);
	ql_remove(&cw_g_name_list, name, link);
    }
}

CW_INLINE cw_bool_t
nxoe_l_name_delete(cw_nxoe_t *a_nxoe, cw_uint32_t a_iter)
{
    cw_nxoe_name_t *name;

    name = (cw_nxoe_name_t *) a_nxoe;

    cw_check_ptr(name);
    cw_dassert(name->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(name->nxoe.type == NXOT_NAME);

    if (name->nxoe.name_static == FALSE)
    {
	/* Cast away the const here; it's one of two places that the string is
	 * allowed to be modified, and this cast is better than dropping the
	 * const altogether. */
	nxa_free((cw_uint8_t *) name->str, name->len);
    }

    nxa_free(name, sizeof(cw_nxoe_name_t));

    return FALSE;
}

CW_INLINE cw_nxoe_t *
nxoe_l_name_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset)
{
    cw_nxoe_name_t *name;

    name = (cw_nxoe_name_t *) a_nxoe;
    
    cw_check_ptr(name);
    cw_dassert(name->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(name->nxoe.type == NXOT_NAME);

    /* Move this name to the beginning of the name list, in order to facilitate
     * removal of all garbage names from the name hash at the end of the mark
     * phase. */
    ql_remove(&cw_g_name_list, name, link);
    ql_head_insert(&cw_g_name_list, name, link);

    return NULL;
}
#endif /* (defined(CW_USE_INLINES) || defined(CW_NXO_NAME_C_)) */
