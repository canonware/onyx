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

typedef struct cw_nxoe_dicto_s cw_nxoe_dicto_t;
typedef struct cw_nxoe_dict_s cw_nxoe_dict_t;

void
nxo_dict_new(cw_nxo_t *a_nxo, cw_bool_t a_locking, cw_uint32_t a_dict_size);

void
nxo_dict_copy(cw_nxo_t *a_to, cw_nxo_t *a_from);

void
nxo_dict_def(cw_nxo_t *a_nxo, cw_nxo_t *a_key, cw_nxo_t *a_val);

void
nxo_dict_undef(cw_nxo_t *a_nxo, const cw_nxo_t *a_key);

cw_bool_t
nxo_dict_lookup(const cw_nxo_t *a_nxo, const cw_nxo_t *a_key, cw_nxo_t *r_nxo);

cw_uint32_t
nxo_dict_count(const cw_nxo_t *a_nxo);

cw_bool_t
nxo_dict_iterate(cw_nxo_t *a_nxo, cw_nxo_t *r_nxo);
