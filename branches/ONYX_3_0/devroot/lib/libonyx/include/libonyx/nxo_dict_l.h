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

/* This is private, but nxa needs to know its size. */
struct cw_nxoe_dicto_s
{
    cw_nxo_t key;
    cw_nxo_t val;
};

struct cw_nxoe_dict_s
{
    cw_nxoe_t nxoe;

#ifdef CW_THREADS
    /* Access is locked if this object has the locking bit set. */
    cw_mtx_t lock;
#endif

    /* Used for remembering the current state of reference iteration. */
    cw_uint32_t ref_iter;

    /* If non-NULL, the previous reference iteration returned the key of this
     * dicto, so the value of this dicto is the next reference to check. */
    cw_nxoe_dicto_t *dicto;

    /* Name/value pairs.  The keys are (cw_nxo_t *), and the values are
     * (cw_nxoe_dicto_t *).  The nxo that the key points to resides in the
     * nxoe_dicto (value) structure. */
    cw_dch_t hash;
};

cw_bool_t
nxoe_l_dict_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_dict_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);

cw_nxo_t *
nxo_l_dict_lookup(cw_nxo_t *a_nxo, const cw_nxo_t *a_key);
