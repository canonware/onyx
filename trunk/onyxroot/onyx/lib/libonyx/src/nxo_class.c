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

#define CW_NXO_CLASS_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_class_l.h"

void
nxo_class_new(cw_nxo_t *a_nxo, void *a_opaque,
	      cw_nxo_class_ref_iter_t *a_ref_iter_f,
	      cw_nxo_class_delete_t *a_delete_f)
{
    cw_nxoe_class_t *class_;

    class_ = (cw_nxoe_class_t *) nxa_malloc(sizeof(cw_nxoe_class_t));

    nxoe_l_new(&class_->nxoe, NXOT_CLASS, FALSE);
    nxo_null_new(&class_->name);
    nxo_null_new(&class_->super);
    nxo_null_new(&class_->methods);
    nxo_null_new(&class_->data);
    class_->opaque = a_opaque;
    class_->ref_iter_f = a_ref_iter_f;
    class_->delete_f = a_delete_f;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) class_;
    nxo_p_type_set(a_nxo, NXOT_CLASS);

    nxa_l_gc_register((cw_nxoe_t *) class_);
}

cw_nxo_t *
nxo_class_name_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    retval = &class_->name;

    return retval;
}

cw_nxo_t *
nxo_class_super_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    retval = &class_->super;

    return retval;
}

cw_nxo_t *
nxo_class_methods_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    retval = &class_->methods;

    return retval;
}

cw_nxo_t *
nxo_class_data_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    retval = &class_->data;

    return retval;
}

void *
nxo_class_opaque_get(const cw_nxo_t *a_nxo)
{
    void *retval;
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    retval = class_->opaque;

    return retval;
}

void
nxo_class_opaque_set(cw_nxo_t *a_nxo, void *a_opaque)
{
    cw_nxoe_class_t *class_;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_CLASS);

    class_ = (cw_nxoe_class_t *) a_nxo->o.nxoe;

    cw_check_ptr(class_);
    cw_dassert(class_->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(class_->nxoe.type == NXOT_CLASS);

    class_->opaque = a_opaque;
}
