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

#define CW_NXO_HANDLE_C_

#include "../include/libonyx/libonyx.h"
#include "../include/libonyx/nxa_l.h"
#include "../include/libonyx/nxo_l.h"
#include "../include/libonyx/nxo_handle_l.h"

void
nxo_handle_new(cw_nxo_t *a_nxo, void *a_opaque, cw_nxo_handle_eval_t *a_eval_f,
	       cw_nxo_handle_ref_iter_t *a_ref_iter_f,
	       cw_nxo_handle_delete_t *a_delete_f)
{
    cw_nxoe_handle_t *handle;

    handle = (cw_nxoe_handle_t *) nxa_malloc(sizeof(cw_nxoe_handle_t));

    nxoe_l_new(&handle->nxoe, NXOT_HANDLE, FALSE);
    nxo_null_new(&handle->tag);
    handle->opaque = a_opaque;
    handle->eval_f = a_eval_f;
    handle->ref_iter_f = a_ref_iter_f;
    handle->delete_f = a_delete_f;

    nxo_no_new(a_nxo);
    a_nxo->o.nxoe = (cw_nxoe_t *) handle;
    nxo_p_type_set(a_nxo, NXOT_HANDLE);

    nxa_l_gc_register((cw_nxoe_t *) handle);
}

cw_nxo_t *
nxo_handle_tag_get(const cw_nxo_t *a_nxo)
{
    cw_nxo_t *retval;
    cw_nxoe_handle_t *handle;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    handle = (cw_nxoe_handle_t *) a_nxo->o.nxoe;

    cw_check_ptr(handle);
    cw_dassert(handle->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(handle->nxoe.type == NXOT_HANDLE);

    retval = &handle->tag;

    return retval;
}

void *
nxo_handle_opaque_get(const cw_nxo_t *a_nxo)
{
    void *retval;
    cw_nxoe_handle_t *handle;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    handle = (cw_nxoe_handle_t *) a_nxo->o.nxoe;

    cw_check_ptr(handle);
    cw_dassert(handle->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(handle->nxoe.type == NXOT_HANDLE);

    retval = handle->opaque;

    return retval;
}

void
nxo_handle_opaque_set(cw_nxo_t *a_nxo, void *a_opaque)
{
    cw_nxoe_handle_t *handle;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    handle = (cw_nxoe_handle_t *) a_nxo->o.nxoe;

    cw_check_ptr(handle);
    cw_dassert(handle->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(handle->nxoe.type == NXOT_HANDLE);

    handle->opaque = a_opaque;
}

void
nxo_handle_eval(cw_nxo_t *a_nxo, cw_nxo_t *a_thread)
{
    cw_nxoe_handle_t *handle;

    cw_check_ptr(a_nxo);
    cw_dassert(a_nxo->magic == CW_NXO_MAGIC);
    cw_assert(nxo_type_get(a_nxo) == NXOT_HANDLE);

    handle = (cw_nxoe_handle_t *) a_nxo->o.nxoe;

    cw_check_ptr(handle);
    cw_dassert(handle->nxoe.magic == CW_NXOE_MAGIC);
    cw_assert(handle->nxoe.type == NXOT_HANDLE);

    if (handle->eval_f != NULL)
    {
	handle->eval_f(handle->opaque, a_thread);
    }
    else
    {
	cw_nxo_t *ostack, *nxo;

	/* This handle can't be executed, so push it onto ostack just like a
	 * normal literal object would be. */
	ostack = nxo_thread_ostack_get(a_thread);
	nxo = nxo_stack_push(ostack);
	nxo_dup(nxo, a_nxo);
    }
}
