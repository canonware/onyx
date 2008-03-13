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

#define CW_NXO_INSTANCE_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_instance_l.h"

void
nxo_instance_new(cw_nxo_t *a_nxo, void *a_opaque,
	      cw_nxo_instance_ref_iter_t *a_ref_iter_f,
	      cw_nxo_instance_delete_t *a_delete_f)
{
    cw_nxoe_instance_t *instance;

    instance = (cw_nxoe_instance_t *) nxa_malloc(sizeof(cw_nxoe_instance_t));

    nxoe_l_new(&instance->nxoe, NXOT_INSTANCE, false);
    nxo_null_new(&instance->isa);
    nxo_null_new(&instance->data);
    instance->opaque = a_opaque;
    instance->ref_iter_f = a_ref_iter_f;
    instance->delete_f = a_delete_f;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) instance;
    nxo_p_type_set(a_nxo, NXOT_INSTANCE);

    nxa_l_gc_register((cw_nxoe_t *) instance);
}

cw_nxo_t *
nxo_instance_isa_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_instance_t *instance;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_INSTANCE);

    instance = (cw_nxoe_instance_t *) a_nxo->o.nxoe;

    cw_check_ptr(instance);
    cw_dassert(instance->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(instance->nxoe.type == NXOT_INSTANCE);

    retval = &instance->isa;

    return retval;
}

cw_nxo_t *
nxo_instance_data_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_instance_t *instance;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_INSTANCE);

    instance = (cw_nxoe_instance_t *) a_nxo->o.nxoe;

    cw_check_ptr(instance);
    cw_dassert(instance->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(instance->nxoe.type == NXOT_INSTANCE);

    retval = &instance->data;

    return retval;
}

void *
nxo_instance_opaque_get(const cw_nxo_t *a_nxo)
{
    void *retval;
    cw_nxoe_instance_t *instance;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_INSTANCE);

    instance = (cw_nxoe_instance_t *) a_nxo->o.nxoe;

    cw_check_ptr(instance);
    cw_dassert(instance->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(instance->nxoe.type == NXOT_INSTANCE);

    retval = instance->opaque;

    return retval;
}

void
nxo_instance_opaque_set(cw_nxo_t *a_nxo, void *a_opaque)
{
    cw_nxoe_instance_t *instance;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_INSTANCE);

    instance = (cw_nxoe_instance_t *) a_nxo->o.nxoe;

    cw_check_ptr(instance);
    cw_dassert(instance->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(instance->nxoe.type == NXOT_INSTANCE);

    instance->opaque = a_opaque;
}
