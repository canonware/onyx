/* -*- mode: c ; c-file-style: "canonware-c-style" -*-
 ******************************************************************************
 *
 * <Copyright = jasone>
 * <License>
 *
 ******************************************************************************
 *
 * Version: <Version>
 *
 ******************************************************************************/

typedef struct cw_nxoe_hook_s cw_nxoe_hook_t;

struct cw_nxoe_hook_s
{
    cw_nxoe_t nxoe;
    
    /* Used for remembering the current state of reference iteration. */
    cw_uint32_t ref_iter;
    
    cw_nxo_t tag;
    void *data;
    cw_nxo_hook_eval_t *eval_f;
    cw_nxo_hook_ref_iter_t *ref_iter_f;
    cw_nxo_hook_delete_t *delete_f;
};

cw_bool_t
nxoe_l_hook_delete(cw_nxoe_t *a_nxoe, cw_nxa_t *a_nxa, cw_uint32_t a_iter);

cw_nxoe_t *
nxoe_l_hook_ref_iter(cw_nxoe_t *a_nxoe, cw_bool_t a_reset);
